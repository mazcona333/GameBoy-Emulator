#ifndef PPU_H
#define PPU_H

#include "memory.h"
#include <queue>
#include <functional>

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
    PixelFetcher(Memory *mem, std::queue<Pixel>* FIFO, bool Background = false);
    std::queue<Pixel> *PixelFIFO;
    void Step();
    void Reset() { FetchCounterX = 0; FetchPhase = 0; };

private:
    Memory *memory;
    bool IsBackground = false;
    uint8_t SCX;
    uint8_t SCY;
    uint8_t LY;
    uint8_t FetchPhase = 0;
    uint8_t FetchCounterX = 0;
    uint8_t TileMapNo = 0;
    uint8_t TileDataLow = 0;
    uint8_t TileDataHigh = 0;
    
    void FetchTileNo();
    void FetchTileDataLow();
    void FetchTileDataHigh();
    void Push();
};

class Ppu
{
public:
    Ppu(Memory *mem, std::function<void(uint8_t *RawPixels, uint8_t row)> UpdateDisplayFunction);
    void Tick();
    uint8_t *getDisplay();

private:
    Memory *memory;
    std::function<void(uint8_t *RawPixels, uint8_t row)> UpdateDisplay;

    uint8_t Display[RES_W * RES_H * 4] = {0};
    uint8_t hPixelDrawing = 0;

    uint16_t DotCounter = 0;
    uint8_t StallCounter = 0;

    PpuMode getPPUMode();
    void setPPUMode(PpuMode Mode);
    bool Disabled = false;
    bool WaitFrame = false;

    uint8_t getLY();
    void setLY(uint8_t LY);

    uint8_t getLCDC();

    PixelFetcher *BackgroundPixelFetcher;
    std::queue<Pixel> BackgroundPixelFIFO;
    // std::queue<std::function<void()>> ObjectPixelFIFO;
    void renderPixel(uint8_t LX, uint8_t LY);
};

#endif // PPU_H