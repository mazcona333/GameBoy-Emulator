#include "../public/ppu.h"

// #define SDL_MAIN_HANDLED

Ppu::Ppu(Memory *mem, std::function<void(uint8_t *RawPixels, uint8_t row)> UpdateDisplayFunction)
{
    memory = mem;
    BackgroundPixelFetcher = new PixelFetcher(memory, &BackgroundPixelFIFO, true);
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

    if (StallCounter > 0)
    {
        StallCounter--;
    }
    else if (getLY() >= 144)
    {
        if(getPPUMode() != PpuMode::VBLANK)
            setPPUMode(PpuMode::VBLANK);
    }
    else if (DotCounter < 80)
    {
        if(getPPUMode() != PpuMode::OAMSCAN)
            setPPUMode(PpuMode::OAMSCAN);
    }
    else if (hPixelDrawing <= 159)
    {
        if(getPPUMode() != PpuMode::DRAW)
            setPPUMode(PpuMode::DRAW);

        if (DotCounter == 80)
        {
            uint8_t SCX = memory->readMemory(0xFF43);
            StallCounter += SCX + 4;
            BackgroundPixelFIFO.empty();
            // TODO Clear ObjectFIFO
        }
        BackgroundPixelFetcher->Step();
        renderPixel(hPixelDrawing, getLY());

        if (hPixelDrawing == 160)
        {
            UpdateDisplay(Display, getLY());
            BackgroundPixelFetcher->Reset();
        }
    }
    else
    {
        if(getPPUMode() != PpuMode::HBLANK)
            setPPUMode(PpuMode::HBLANK);
    }
    
    DotCounter = (DotCounter + 1) % 456;
    if (DotCounter == 0)
    {
        setLY((getLY() + 1) % 154);
        hPixelDrawing = 0;

        if (getLY() == 0 && !Disabled && WaitFrame)
            WaitFrame = false;
    }
}

void Ppu::renderPixel(uint8_t LX, uint8_t LY)
{
    if (BackgroundPixelFIFO.size() == 0)
        return;

    uint8_t ColorID = BackgroundPixelFIFO.front().GetColor();
    uint8_t Color = (memory->readMemory(0xFF47) & (3 << (2 * ColorID))) >> (2 * ColorID);

    if (!Disabled && !WaitFrame)
    {
        Display[(LY * RES_W + LX) * 4 + 0] = 0xFF;
        Display[(LY * RES_W + LX) * 4 + 1] = 0xF0 * (3 - Color) / 3;
        Display[(LY * RES_W + LX) * 4 + 2] = 0xF0 * (3 - Color) / 3;
        Display[(LY * RES_W + LX) * 4 + 3] = 0xF0 * (3 - Color) / 3;
    }
    else
    {
        Display[(LY * RES_W + LX) * 4 + 0] = 0xFF;
        Display[(LY * RES_W + LX) * 4 + 1] = 0xFF;
        Display[(LY * RES_W + LX) * 4 + 2] = 0xFF;
        Display[(LY * RES_W + LX) * 4 + 3] = 0xFF;
    }
    BackgroundPixelFIFO.pop();
    hPixelDrawing++;
}

uint8_t *Ppu::getDisplay()
{
    return Display;
}

PpuMode Ppu::getPPUMode()
{
    switch (memory->readMemory(0xFF41) & 0x00000011)
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

PixelFetcher::PixelFetcher(Memory *mem, std::queue<Pixel>* FIFO, bool Background)
{
    memory = mem;
    IsBackground = Background;
    PixelFIFO = FIFO;
}

void PixelFetcher::Step()
{
    if (FetchPhase == 0 || FetchPhase == 2 || FetchPhase == 4 || FetchPhase == 6)
    {
        // StallCounter++;
    }
    else if (FetchPhase == 1)
    { // TODO Fetch Tile Number
        FetchTileNo();
    }
    else if (FetchPhase == 3)
    { // TODO Fetch Tile Data Low
        FetchTileDataLow();
    }
    else if (FetchPhase == 5)
    { // TODO Fetch Tile Data High
        FetchTileDataHigh();
    }
    else if (FetchPhase == 7)
    { // TODO Push to FIFO Wait???
        Push();
    }

    FetchPhase = (FetchPhase + 1) % 8;
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
    if(FetchCounterX == 0)
        SCX = memory->readMemory(0xFF43);
    else
        SCX = (memory->readMemory(0xFF43) & 0x11111000) + (SCX & 0x00000111);

    uint16_t TileMapOffset = (FetchCounterX + (SCX / 8)) & 0x1F;

    LY = memory->readMemory(0xFF44);
    SCY = memory->readMemory(0xFF42);
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
        uint16_t TileAdressOffset = 2 * ((LY + SCY) % 8);
        TileDataLow = memory->readMemory(TileAdress + TileAdressOffset, true);
    }
    else
    {
        TileDataBaseAdress = 0x9000;
        uint16_t TileAdress = TileDataBaseAdress + ((int8_t)TileMapNo * 0x10);
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
        uint16_t TileAdressOffset = 2 * ((LY + SCY) % 8);
        TileDataLow = memory->readMemory(TileAdress + TileAdressOffset + 1, true);
    }
    else
    {
        TileDataBaseAdress = 0x9000;
        uint16_t TileAdress = TileDataBaseAdress + ((int8_t)TileMapNo * 0x10);
        uint16_t TileAdressOffset = 2 * ((LY + SCY) % 8);
        TileDataLow = memory->readMemory(TileAdress + TileAdressOffset + 1, true);
    }
}

void PixelFetcher::Push()
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