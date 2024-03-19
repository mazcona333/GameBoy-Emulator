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
    Pixel(uint8_t Color) : Color(Color) {}
    Pixel(uint8_t Color, uint8_t Palette, bool BackgroundPriority) : Color(Color), Palette(Palette), BackgroundPriority(BackgroundPriority) {}
    uint8_t Color;
    uint8_t Palette = -1;
    uint8_t SpritePriority = -1;
    bool BackgroundPriority;
};

struct OAMData
{
    OAMData(uint8_t YPostion, uint8_t XPostion, uint8_t TileIndex, uint8_t Attrigutes) : YPostion(YPostion), XPostion(XPostion), TileIndex(TileIndex), Attributes(Attributes) {}
    uint8_t YPostion;
    uint8_t XPostion;
    uint8_t TileIndex;
    uint8_t Attributes;
};

class PixelFetcher
{
public:
    enum FetchingMode
    {
        BACKGROUND,
        SPRITE,
        WINDOW
    };

public:
    PixelFetcher(Memory *memory, std::queue<Pixel> *PixelFIFO, FetchingMode Mode, std::vector<OAMData>* OAMBuffer = nullptr) : memory(memory), PixelFIFO(PixelFIFO), Mode(Mode), OAMBuffer(OAMBuffer){};
    std::queue<Pixel> *PixelFIFO;
    void Step();
    void ResetCounter()
    {
        FetchCounterX = 0;
    };
    void ResetPhase()
    {
        FetchPhase = 0;
    };
    uint8_t GetStep() { return (FetchPhase & 1 ? FetchPhase - 1 : FetchPhase) / 2; }

private:
    Memory *memory;
    FetchingMode Mode;
    uint8_t SCX;
    uint8_t SCY;
    uint8_t LY;
    uint8_t FetchPhase = 0;
    uint8_t FetchCounterX = 0;
    uint8_t TileMapNo = 0;
    uint8_t TileDataLow = 0;
    uint8_t TileDataHigh = 0;

    std::vector<OAMData>* OAMBuffer;

    void FetchTileNo();
    void FetchTileDataLow();
    void FetchTileDataHigh();
    void Push();
};

class Ppu
{
public:
    Ppu(Memory *memory, std::function<void(uint8_t *RawPixels, uint8_t row)> UpdateDisplay);
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

    void OAMScanStep();

    PixelFetcher *BackgroundPixelFetcher;
    std::queue<Pixel> BackgroundPixelFIFO;

    std::vector<OAMData> OAMBuffer;
    bool SpriteFetch = false;
    PixelFetcher *ObjectPixelFetcher;
    std::queue<Pixel> ObjectPixelFIFO;
    void renderPixel(uint8_t LX, uint8_t LY);
};

#endif // PPU_H