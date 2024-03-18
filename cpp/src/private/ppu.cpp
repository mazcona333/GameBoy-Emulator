#include "../public/ppu.h"

// #define SDL_MAIN_HANDLED

Ppu::Ppu(Memory *mem, std::function<void(uint8_t *RawPixels, uint8_t row)> UpdateDisplayFunction)
{
    memory = mem;
    BackgroundPixelFetcher = new PixelFetcher(memory, true);
    UpdateDisplay = UpdateDisplayFunction;
}

void Ppu::Tick()
{
    if ((memory->readMemory(0xFF40) >> 7) == 0)
    {
        Disabled = true;
        return;
    }
    else
    {
        if (Disabled)
        {
            WaitFrame = true;
            Disabled = false;
        }
    }

    if (!WaitFrame)
    {
    }
    else if (StallCounter > 0)
    {
        StallCounter--;
    }
    else if (getLY() >= 144)
    {
        setPPUMode(PpuMode::VBLANK);
    }
    else if (DotCounter < 80)
    {
        setPPUMode(PpuMode::OAMSCAN);
    }
    else if (hPixelDrawing <= 159)
    {
        setPPUMode(PpuMode::DRAW);

        if (DotCounter == 80)
        {
            uint8_t SCX = memory->readMemory(0xFF43);
            StallCounter = SCX;
            BackgroundPixelFIFO.empty();
            // TODO Clear ObjectFIFO
        }
        FetchBackgroundPixel();
        renderPixel(hPixelDrawing, getLY());
        if (hPixelDrawing == 160)
            UpdateDisplay(Display, getLY());
    }
    else
    {
        setPPUMode(PpuMode::HBLANK);
    }
    DotCounter = (DotCounter + 1) % 456;
    if (DotCounter == 0)
    {
        setLY((getLY() + 1) % 154);
        hPixelDrawing = 0;
    }
}

void Ppu::FetchBackgroundPixel()
{
    uint8_t FetchPhase = DotCounter % 8;
    if (FetchPhase == 0 || FetchPhase == 2 || FetchPhase == 4 || FetchPhase == 6)
    {
        // StallCounter++;
    }
    else if (FetchPhase == 1)
    { // TODO Fetch Tile Number
        BackgroundPixelFetcher->FetchTileNo();
    }
    else if (FetchPhase == 3)
    { // TODO Fetch Tile Data Low
        BackgroundPixelFetcher->FetchTileDataLow();
    }
    else if (FetchPhase == 5)
    { // TODO Fetch Tile Data High
        BackgroundPixelFetcher->FetchTileDataHigh();
    }
    else if (FetchPhase == 7)
    { // TODO Push to FIFO Wait???
        BackgroundPixelFetcher->Push(&BackgroundPixelFIFO);
    }
}

void Ppu::renderPixel(uint8_t LX, uint8_t LY)
{
    if (BackgroundPixelFIFO.size() == 0)
        return;

    uint8_t ColorID = BackgroundPixelFIFO.front().GetColor();
    uint8_t Color = (memory->readMemory(0xFF47) & (3 << (2 * ColorID))) >> (2 * ColorID);

    Display[(LY * RES_W + LX) * 4 + 0] = 0xFF;
    Display[(LY * RES_W + LX) * 4 + 1] = 0xFF * (3 - Color) / 3;
    Display[(LY * RES_W + LX) * 4 + 2] = 0xFF * (3 - Color) / 3;
    Display[(LY * RES_W + LX) * 4 + 3] = 0xFF * (3 - Color) / 3;
    BackgroundPixelFIFO.pop();
    hPixelDrawing++;
}

uint8_t *Ppu::getDisplay()
{
    return Display;
}

PpuMode Ppu::getPPUMode()
{
    switch (memory->readMemory(0xFF41))
    {
    case 0:
        return PpuMode::HBLANK;
        break;
    case 1:
        return PpuMode::VBLANK;
        break;
    case 2:
        return PpuMode::OAMSCAN;
        break;
    case 3:
        return PpuMode::DRAW;
        break;

    default:
        break;
    };
}

void Ppu::setPPUMode(PpuMode Mode)
{
    memory->setPPUMode(Mode);
}

uint8_t Ppu::getLY()
{
    return memory->readMemory(0xFF44);
}

void Ppu::setLY(uint8_t LY)
{
    memory->setLY(LY);
}

uint8_t Ppu::getLCDC()
{
    return memory->readMemory(0xFF40);
}

PixelFetcher::PixelFetcher(Memory *mem, bool Background)
{
    memory = mem;
    IsBackground = Background;
}

void PixelFetcher::FetchTileNo()
{
    uint16_t TileMapBaseAdress;
    if (memory->readMemory(0xFF40) & 0b00001000)
    {
        TileMapBaseAdress = 0x9C00;
    }
    else
    {
        TileMapBaseAdress = 0x9800;
    }
    uint8_t SCX = memory->readMemory(0xFF43);
    uint16_t TileMapOffset = (FetchCounterX + (SCX / 8)) & 0x1F;

    uint8_t LY = memory->readMemory(0xFF44);
    uint8_t SCY = memory->readMemory(0xFF42);
    TileMapOffset += (((LY + SCY) & 0xFF) / 8) * 32;
    TileMapOffset = TileMapOffset & 0x3FF;

    TileMapNo = memory->readMemory(TileMapBaseAdress + TileMapOffset, true);
}

void PixelFetcher::FetchTileDataLow()
{
    uint16_t TileDataBaseAdress;

    if (memory->readMemory(0xFF40) & 0b00010000)
    {
        TileDataBaseAdress = 0x8000;
        uint16_t TileAdress = TileDataBaseAdress + (TileMapNo * 0x10);
        uint8_t LY = memory->readMemory(0xFF44);
        uint8_t SCY = memory->readMemory(0xFF42);
        uint16_t TileAdressOffset = 2 * ((LY + SCY) % 8);
        TileDataLow = memory->readMemory(TileAdress + TileAdressOffset, true);
    }
    else
    {
        TileDataBaseAdress = 0x9000;
        uint16_t TileAdress = TileDataBaseAdress + ((int8_t)TileMapNo * 0x10);
        uint8_t LY = memory->readMemory(0xFF44);
        uint8_t SCY = memory->readMemory(0xFF42);
        uint16_t TileAdressOffset = 2 * ((LY + SCY) % 8);
        TileDataLow = memory->readMemory(TileAdress + TileAdressOffset, true);
    }
}

void PixelFetcher::FetchTileDataHigh()
{
    uint16_t TileDataBaseAdress;

    if (memory->readMemory(0xFF40) & 0b00010000)
    {
        TileDataBaseAdress = 0x8000;
        uint16_t TileAdress = TileDataBaseAdress + (TileMapNo * 0x10);
        uint8_t LY = memory->readMemory(0xFF44);
        uint8_t SCY = memory->readMemory(0xFF42);
        uint16_t TileAdressOffset = 2 * ((LY + SCY) % 8);
        TileDataLow = memory->readMemory(TileAdress + TileAdressOffset + 1, true);
    }
    else
    {
        TileDataBaseAdress = 0x9000;
        uint16_t TileAdress = TileDataBaseAdress + ((int8_t)TileMapNo * 0x10);
        uint8_t LY = memory->readMemory(0xFF44);
        uint8_t SCY = memory->readMemory(0xFF42);
        uint16_t TileAdressOffset = 2 * ((LY + SCY) % 8);
        TileDataLow = memory->readMemory(TileAdress + TileAdressOffset + 1, true);
    }
}

void PixelFetcher::Push(std::queue<Pixel> *PixelFIFO)
{
    if (PixelFIFO->size() == 0)
    {
        for (size_t i = 0; i < 8; i++)
        {
            PixelFIFO->push(Pixel((TileDataHigh & (1 << (7 - i))) >> (7 - i), (TileDataLow & (1 << (7 - i))) >> (7 - i)));
        }
        FetchCounterX++;
    }
}