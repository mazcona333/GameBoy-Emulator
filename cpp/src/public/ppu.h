#ifndef PPU_H
#define PPU_H

#include "memory.h"
#include <queue>

#define RES_W 160
#define RES_H 144

enum PpuMode
{
    HBLANK,
    VBLANK,
    OAMSCAN,
    DRAW
};

struct Pixel
{
    Pixel(bool High, bool Low)
    {
        HighBit = High;
        LowBit = Low;
    }
    bool LowBit;
    bool HighBit;
    uint8_t GetColor() { return (HighBit << 1) + LowBit; }
};

class PixelFetcher
{
public:
    PixelFetcher(Memory *mem, bool Background = false);
    void FetchTileNo();
    void FetchTileDataLow();
    void FetchTileDataHigh();
    void Push(std::queue<Pixel> *PixelFIFO);

private:
    Memory *memory;
    bool IsBackground = false;
    uint8_t FetchCounterX = 0;
    uint8_t TileMapNo = 0;
    uint8_t TileDataLow = 0;
    uint8_t TileDataHigh = 0;
};

class Ppu
{
public:
    Ppu(Memory *mem);
    void Tick();
    uint8_t *getDisplay();

private:
    Memory *memory;

    uint8_t Display[RES_W * RES_H * 3] = {0};
    uint8_t hPixelDrawing = 0;

    uint8_t DotCounter = 0;
    uint8_t StallCounter = 0;

    PpuMode getPPUMode();
    void setPPUMode(PpuMode Mode);

    uint8_t getLY();
    void setLY(uint8_t LY);

    uint8_t getLCDC();

    PixelFetcher *BackgroundPixelFetcher;
    std::queue<Pixel> BackgroundPixelFIFO;
    void FetchBackgroundPixel();
    // std::queue<std::function<void()>> ObjectPixelFIFO;
    void renderPixel(uint8_t LX, uint8_t LY);
};

#endif // PPU_H