#include "../public/ppu.h"

// #define SDL_MAIN_HANDLED

Ppu::Ppu(Memory *memory, std::function<void(uint8_t *RawPixels, uint8_t row)> UpdateDisplay) : memory(memory), UpdateDisplay(UpdateDisplay)
{
    BackgroundPixelFetcher = new PixelFetcher(memory, &BackgroundPixelFIFO, PixelFetcher::FetchingMode::BACKGROUND);
    ObjectPixelFetcher = new PixelFetcher(memory, &ObjectPixelFIFO, PixelFetcher::FetchingMode::SPRITE);
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
        if (getPPUMode() != PpuMode::VBLANK)
            setPPUMode(PpuMode::VBLANK);
    }
    else if (DotCounter < 80)
    {
        if (DotCounter == 0)
        {
            setPPUMode(PpuMode::OAMSCAN);
            OAMBuffer.clear();
        }
        OAMScanStep();
    }
    else if (hPixelDrawing <= 159)
    {
        if (DotCounter == 80)
        {
            setPPUMode(PpuMode::DRAW);

            StallCounter += 4;

            BackgroundPixelFIFO = {};
            ObjectPixelFIFO = {};
        }

        if (!SpriteFetch)
            BackgroundPixelFetcher->Step();

        if (memory->readMemory(0xFF40) & 0b0000010)
        {
            bool ObjectInLine = false;
            for (size_t i = 0; i < OAMBuffer.size(); i++)
            {
                if (OAMBuffer[i].XPostion <= hPixelDrawing + 8)
                {
                    ObjectInLine = true;
                    break;
                }
            }
            if (ObjectInLine)
            {
                SpriteFetch = true;
                BackgroundPixelFetcher->ResetPhase();
            }
            else
            {
                // TODO Cancel Object Fetch
            }
        }

        if (SpriteFetch)
        {
            if (ObjectPixelFetcher->GetStep() < 3 && BackgroundPixelFIFO.empty())
                ObjectPixelFetcher->Step();
            else
                SpriteFetch = false;
            // TODO Cancel Object Fetch
        }

        if (!SpriteFetch)
            renderPixel(hPixelDrawing, getLY());

        if (hPixelDrawing == 160)
        {
            UpdateDisplay(Display, getLY());
            BackgroundPixelFetcher->ResetCounter();
            BackgroundPixelFetcher->ResetPhase();
        }
    }
    else
    {
        if (getPPUMode() != PpuMode::HBLANK)
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
    uint8_t SCX = memory->readMemory(0xFF43);
    if (BackgroundPixelFIFO.empty())
        return;
    if (LX < SCX)
    {
        BackgroundPixelFIFO.pop();
    }

    uint8_t ColorID = BackgroundPixelFIFO.front().Color;
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

void Ppu::OAMScanStep()
{
    if (DotCounter & 1)
    {
        uint8_t ObjectOffset = ((DotCounter - 1) / 2) * 4;

        uint8_t YPosition = memory->readMemory(0xFE00 + ObjectOffset, true);
        uint8_t XPosition = memory->readMemory(0xFE00 + ObjectOffset + 1, true);
        uint8_t TileIndex = memory->readMemory(0xFE00 + ObjectOffset + 2, true);
        uint8_t Attributes = memory->readMemory(0xFE00 + ObjectOffset + 3, true);

        bool ObjectSize = memory->readMemory(0xFF40) & 0b00000100;

        if ((XPosition > 0) && ((getLY() + 16) >= YPosition) && ((getLY() + 16) < (YPosition + (ObjectSize ? 16 : 8))) && OAMBuffer.size() < 10)
            OAMBuffer.push_back(OAMData(YPosition, XPosition, TileIndex, Attributes));
    }
}

void PixelFetcher::Step()
{

    if (FetchPhase == 1)
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
    else if (FetchPhase == 7 || (FetchPhase == 6 && Mode == FetchingMode::SPRITE))
    { // TODO Push to FIFO Wait???
        Push();
    }
    else
    {
        // StallCounter++;
    }

    FetchPhase = (FetchPhase + 1) % 8;
}

void PixelFetcher::FetchTileNo()
{
    uint16_t TileMapBaseAdress;
    if (Mode == FetchingMode::BACKGROUND)
    {
        if (memory->readMemory(0xFF40) & 0b00001000)
        {
            TileMapBaseAdress = 0x9C00;
        }
        else
        {
            TileMapBaseAdress = 0x9800;
        }

        if (FetchCounterX == 0)
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
    else if (Mode == FetchingMode::SPRITE)
    {
        TileMapNo = OAMBuffer->front().TileIndex;
        // TODO Not front but the one overlapping
    }
}

void PixelFetcher::FetchTileDataLow()
{
    uint16_t TileDataBaseAdress;

    if ((memory->readMemory(0xFF40) & 0b00010000) || Mode == FetchingMode::SPRITE)
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

    if (memory->readMemory(0xFF40) & 0b00010000 || Mode == FetchingMode::SPRITE)
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
    if (Mode == FetchingMode::BACKGROUND)
    {
        if (PixelFIFO->size() == 0)
        {
            for (size_t i = 0; i < 8; i++)
            {
                uint8_t Color = ((TileDataHigh & (1 << (7 - i))) >> (6 - i)) + ((TileDataLow & (1 << (7 - i))) >> (7 - i));
                PixelFIFO->push(Pixel(Color));
            }
            FetchCounterX++;
        }
    }
    else if (Mode == FetchingMode::SPRITE)
    {
        for (size_t i = 0; i < 8; i++)
        {
            if(i < PixelFIFO->size() || i < (8 - OAMBuffer->front().XPostion))
                continue;
            uint8_t Color = ((TileDataHigh & (1 << (7 - i))) >> (6 - i)) + ((TileDataLow & (1 << (7 - i))) >> (7 - i));
            PixelFIFO->push(Pixel(Color));
        }
    }
}