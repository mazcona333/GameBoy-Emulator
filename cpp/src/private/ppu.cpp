#include "../public/ppu.h"

// #define SDL_MAIN_HANDLED

Ppu::Ppu(Memory *mem)
{
    memory = mem;
    BackgroundPixelFetcher = new PixelFetcher(memory, true);
}

void Ppu::Tick()
{
    /*
    if (StallCounter > 0)
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
            BackgroundPixelFIFO.empty();
            // TODO Clear ObjectFIFO
        }
        FetchBackgroundPixel();
        renderPixel(hPixelDrawing, getLY());
    }
    else
    {
        setPPUMode(PpuMode::HBLANK);
    }
    DotCounter = (DotCounter + 1) % 456;
    if (DotCounter == 0)
        setLY((getLY() + 1) % 154);
        */

    //	reset SCY, SCX for correct scrolling values
    uint8_t SCY = memory->readMemory(0xff42);
    uint8_t SCX = memory->readMemory(0xff43);

    uint8_t tilemap = (((memory->readMemory(0xff40) >> 3) & 1) == 1) ? 0x9c00 : 0x9800;  //	check which location was set in LCDC for BG Map
    uint8_t tiledata = (((memory->readMemory(0xff40) >> 4) & 1) == 1) ? 0x8000 : 0x8800; //	check which location was set in LCDC for BG / Window Tile Data (Catalog)
    unsigned char p = memory->readMemory(0xff47);
    for (int j = 0; j < 256; j++)
    {

        //	handle wrapping
        uint8_t offY = getLY() + SCY;
        uint8_t offX = j + SCX;

        //	which tile no. is wanted from tiledata
        uint8_t tilenr = memory->readMemory(tilemap + ((offY / 8 * 32) + (offX / 8)));
        //	get color value for the current pixel (00, 01, 10, 11)
        //	if 0x8800 we adress as signed tilenr from 0x9000 being tile 0 (overwrite the original value)
        uint8_t colorval;
        if (tiledata == 0x8800)
        {
            colorval = (memory->readMemory(tiledata + 0x800 + ((int8_t)tilenr * 0x10) + (offY % 8 * 2)) >> (7 - (offX % 8)) & 0x1) + ((memory->readMemory(tiledata + 0x800 + ((int8_t)tilenr * 0x10) + (offY % 8 * 2) + 1) >> (7 - (offX % 8)) & 0x1) * 2);
        }
        else
        {
            colorval = (memory->readMemory(tiledata + (tilenr * 0x10) + (offY % 8 * 2)) >> (7 - (offX % 8)) & 0x1) + (memory->readMemory(tiledata + (tilenr * 0x10) + (offY % 8 * 2) + 1) >> (7 - (offX % 8)) & 0x1) * 2;
        }

        unsigned char bgmap[160 * 144 * 3];

        //	get real color from palette
        bgmap[(getLY() * 256 * 3) + (j * 3)] = 0xFF * colorval / 3;
        bgmap[(getLY() * 256 * 3) + (j * 3) + 1] = 0xFF * colorval / 3;
        bgmap[(getLY() * 256 * 3) + (j * 3) + 2] = 0xFF * colorval / 3;

        //	print by line, so image effects are possible
        for (int r = 0; r < 144; r++)
        {
            for (int col = 0; col < 160; col++)
            {
                uint8_t yoffA = (r * 256 * 3);
                uint8_t xoffA = (col * 3);
                Display[(r * 160 * 3) + (col * 3)] = bgmap[yoffA + xoffA];
                Display[(r * 160 * 3) + (col * 3) + 1] = bgmap[yoffA + xoffA + 1];
                Display[(r * 160 * 3) + (col * 3) + 2] = bgmap[yoffA + xoffA + 2];
                // Display[getLY() * RES_W + j] = 0xFFFFFFFF & (0b11111111111111111111111111111100 + colorval);
            }
        }
        // Display[getLY() * RES_W + j] = 0xFFFFFFFF & (0b11111111111111111111111111111100 + colorval);
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
    Display[LY * RES_W + LX] = 0xFFFFFFFF & (0b11111111111111111111111111111100 + BackgroundPixelFIFO.front().GetColor());
    BackgroundPixelFIFO.pop();
    hPixelDrawing = (hPixelDrawing + 1) % 160;
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
    uint8_t offY = memory->readMemory(0xFF44) + memory->readMemory(0xFF42);
    uint8_t offX = FetchCounterX + memory->readMemory(0xFF43);
    // uint8_t TileMapOffset = (FetchCounterX + (memory->readMemory(0xFF43) / 8)) & 0x1F;
    uint8_t TileMapOffset = offX / 8;
    // TileMapOffset += 32 * (((memory->readMemory(0xFF44) + memory->readMemory(0xFF42)) & 0xFF) / 8);
    TileMapOffset += 32 * (offY / 8);
    // TileMapOffset = TileMapOffset & 0x3FF;

    TileMapNo = TileMapBaseAdress + TileMapOffset;
}

void PixelFetcher::FetchTileDataLow()
{
    uint16_t TileDataBaseAdress;

    uint8_t offY = memory->readMemory(0xFF44) + memory->readMemory(0xFF42);
    uint8_t offX = FetchCounterX + memory->readMemory(0xFF43);

    if (memory->readMemory(0xFF40) & 0b00010000)
    {
        TileDataBaseAdress = 0x8000;
        // TileDataLow = memory->readMemory(TileDataBaseAdress + TileMapNo + (2 * ((memory->readMemory(0xFF44) + memory->readMemory(0xFF42)) % 8)));
        TileDataLow = memory->readMemory(TileDataBaseAdress + (TileMapNo * 0x10) + (2 * (offY % 8)));
    }
    else
    {
        TileDataBaseAdress = 0x9000;
        TileDataLow = memory->readMemory(TileDataBaseAdress + (int8_t)TileMapNo + (2 * ((memory->readMemory(0xFF44) + memory->readMemory(0xFF42)) % 8)));
    }
}

void PixelFetcher::FetchTileDataHigh()
{
    uint16_t TileDataBaseAdress;

    uint8_t offY = memory->readMemory(0xFF44) + memory->readMemory(0xFF42);
    uint8_t offX = FetchCounterX + memory->readMemory(0xFF43);

    if (memory->readMemory(0xFF40) & 0b00010000)
    {
        TileDataBaseAdress = 0x8000;
        // TileDataLow = memory->readMemory(TileDataBaseAdress + TileMapNo + (2 * ((memory->readMemory(0xFF44) + memory->readMemory(0xFF42)) % 8)));
        TileDataLow = memory->readMemory(TileDataBaseAdress + (TileMapNo * 0x10) + (2 * (offY % 8)) + 1);
    }
    else
    {
        TileDataBaseAdress = 0x9000;
        TileDataLow = memory->readMemory(TileDataBaseAdress + (int8_t)TileMapNo + (2 * ((memory->readMemory(0xFF44) + memory->readMemory(0xFF42)) % 8)) + 1);
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