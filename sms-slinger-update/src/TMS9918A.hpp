/*
    http://www.codeslinger.co.uk/pages/projects/mastersystem.html

    Copyright(c) 2008 < copyright holders > (sic)
    Modified 2021 Matt Marchant https://github.com/fallahn

    This software is provided 'as-is', without any express or implied
    warranty.In no event will the authors be held liable for any damages
    arising from the use of this software.

    Permission is granted to anyone to use this software for any purpose,
    including commercial applications, and to alter itand redistribute it
    freely, subject to the following restrictions :

    1. The origin of this software must not be misrepresented; you must not
    claim that you wrote the original software.If you use this software
    in a product, an acknowledgment in the product documentation would be
    appreciated but is not required.

    2. Altered source versions must be plainly marked as such, and must not be
    misrepresented as being the original software.

    3. This notice may not be removed or altered from any source distribution.
*/

#pragma once

#include <array>

class TMS9918A final
{
public:
    static constexpr int NUM_RES_HORIZONTAL = 256;
    static constexpr int NUM_RES_VERTICAL = 192;
    static constexpr int NUM_NTSC_VERTICAL = 262;
    static constexpr int NUM_PAL_VERTICAL = 313;
    static constexpr int NUM_RES_VERT_MED = 224;
    static constexpr int NUM_RES_VERT_HIGH = 240;
    static constexpr int SCREENBLANKCOLOUR = 1;
    static constexpr int BYTES_PER_CHANNEL = 3;

    static constexpr int MACHINE_CLICKS_PER_SCANLINE = 684;

    TMS9918A();

    void update(float cycles);
    void reset(bool isPAL);
    BYTE readMemory(BYTE address);
    void writeMemory(BYTE address, BYTE data);
    void writeVDPAddress(BYTE data);
    BYTE readDataPort();
    void writeDataPort(BYTE data);
    BYTE getStatus();
    void resetScreen();
    BYTE getHCounter() const;
    BYTE getVCounter() const { return m_VCounter; }
    bool isRequestingInterupt() const { return m_requestInterrupt; }
    WORD getWidth() const { return m_width; }
    WORD getHeight() const { return m_height; }
    bool getRefresh();
    void dumpClockInfo();
    void setGFXOpt(bool useGFXOpt) { m_useGFXOpt = useGFXOpt; }

    const BYTE* getPixelBuffer() const { return m_buffer.data(); }

    static bool screenDisabled;
    static bool frameToggle;

private:
    std::array<BYTE, 0x4000> m_VRAM = {};
    std::array<BYTE, 32> m_CRAM = {};
    std::array<BYTE, 16> m_VDPRegisters = {};

    //we only need to make one buffer big enough to accept all modes
    std::array<BYTE, NUM_RES_VERT_HIGH * NUM_RES_HORIZONTAL * BYTES_PER_CHANNEL> m_buffer = {};

    float m_runningCycles;
    unsigned int long m_clockInfo;
    bool m_isPAL;
    int m_numScanlines;
    bool m_isVBlank;
    BYTE m_status;
    WORD m_controlWord;
    WORD m_tempWord;
//  WORD m_newControlWord;
    bool m_isSecondControlWrite;
    bool m_requestInterrupt;
    bool m_useGFXOpt;

    BYTE m_VCounter;
    WORD m_HCounter;
    bool m_VCounterFirst;
    BYTE m_lineInterrupt;
    BYTE m_VScroll;
    BYTE m_readBuffer;
    WORD m_width;
    WORD m_height;
    bool m_refresh;
    int m_refreshRatePerSecond;
    

    WORD getAddressRegister() const;
    BYTE getCodeRegister() const;
    void incrementAddress();
    void setRegData();
    void render();
    void renderOpt();
    void renderSpritesMode2();
    void renderSpritesMode4();
    void renderBackgroundMode2();
    void renderBackgroundMode4();
    bool isRegBitSet(int reg, BYTE bit);
    void setSpriteOverflow();
    void setSpriteCollision();
    WORD getSATBase() const;
    WORD getNameBase() const;
    inline BYTE getColourShade(BYTE val) const;

    BYTE getVDPMode() const;
    inline void writeToScreen(BYTE x, BYTE y, BYTE red, BYTE blue, BYTE green);
    inline BYTE getScreenPixelColour(BYTE x, BYTE y, int index ) const;
    BYTE getVJump() const;
    BYTE getVJumpTo() const;
    void dumpVRAM();
    void drawMode2Sprite(const WORD& address, BYTE xpos, BYTE line, BYTE colour);
    void setMode2IllegalSprites(BYTE sprite);   
};