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

class TMS9918A
{
public:
    static const    int         NUM_RES_HORIZONTAL = 256;
    static const    int         NUM_RES_VERTICAL = 192;
    static const    int         NUM_NTSC_VERTICAL = 262;
    static const    int         NUM_PAL_VERTICAL = 313;
    static const    int         NUM_RES_VERT_MED = 224;
    static const    int         NUM_RES_VERT_HIGH = 240;
    static const    int         SCREENBLANKCOLOUR = 1;

    static const    int         MACHINE_CLICKS_PER_SCANLINE = 684;

                                TMS9918A();
                                ~TMS9918A();

                    void        Update(float cycles);
                    void        Reset(bool isPAL);
                    BYTE        ReadMemory(BYTE address);
                    void        WriteMemory(BYTE address, BYTE data);
                    void        WriteVDPAddress(BYTE data);
                    BYTE        ReadDataPort();
                    void        WriteDataPort(BYTE data);
                    BYTE        GetStatus();
                    void        ResetScreen();
                    BYTE        GetHCounter() const;
                    BYTE        GetVCounter() const { return m_VCounter; }
                    bool        IsRequestingInterupt() const { return m_RequestInterupt; }
                    WORD        GetHeight() const { return m_Height; }
                    WORD        GetWidth() const { return m_Width; }
                    bool        GetRefresh();
                    void        DumpClockInfo();
                    void        SetGFXOpt(bool useGFXOpt) { m_UseGFXOpt = useGFXOpt; }
                    
                    BYTE        m_ScreenStandard[NUM_RES_VERTICAL][NUM_RES_HORIZONTAL][3];
                    BYTE        m_ScreenMed[NUM_RES_VERT_MED][NUM_RES_HORIZONTAL][3];
                    BYTE        m_ScreenHigh[NUM_RES_VERT_HIGH][NUM_RES_HORIZONTAL][3];

    static          bool        m_ScreenDisabled;
    static          bool        m_FrameToggle;
private:

    

                    WORD        GetAddressRegister() const;
                    BYTE        GetCodeRegister() const;
                    void        IncrementAddress();
                    void        SetRegData();
                    void        Render();
                    void        RenderOpt();
                    void        RenderSpritesMode2();
                    void        RenderSpritesMode4();
                    void        RenderBackgroundMode2();
                    void        RenderBackgroundMode4();
                    bool        IsRegBitSet(int reg, BYTE bit);
                    void        SetSpriteOverflow();
                    void        SetSpriteCollision();
                    WORD        GetSATBase() const;
                    WORD        GetNameBase() const;
            inline  BYTE        GetColourShade(BYTE val) const;

                    BYTE        GetVDPMode() const;
            inline  void        WriteToScreen(BYTE x, BYTE y, BYTE red, BYTE blue, BYTE green);
            inline  BYTE        GetScreenPixelColour(BYTE x, BYTE y, int index ) const;
                    BYTE        GetVJump() const;
                    BYTE        GetVJumpTo() const;
                    void        DumpVRAM();
                    void        DrawMode2Sprite(const WORD& address, BYTE xpos, BYTE line, BYTE colour);
                    void        SetMode2IllegalSprites(BYTE sprite);
                    
                    float       m_RunningCycles;
                    unsigned int long m_ClockInfo;


                    BYTE        m_VRAM[0x4000];
                    BYTE        m_CRAM[32];
                    bool        m_IsPAL;
                    int         m_NumScanlines;
                    bool        m_IsVBlank;
                    BYTE        m_Status;
                    WORD        m_ControlWord;
                    WORD        m_TempWord;
                //  WORD        m_NewControlWord ;
                    bool        m_IsSecondControlWrite;
                    BYTE        m_VDPRegisters[16];
                    bool        m_RequestInterupt;
                    bool        m_UseGFXOpt;

                    BYTE        m_VCounter;
                    WORD        m_HCounter;
                    bool        m_VCounterFirst;
                    BYTE        m_LineInterupt;
                    BYTE        m_VScroll;
                    BYTE        m_ReadBuffer;
                    WORD        m_Height;
                    WORD        m_Width;
                    bool        m_Refresh;
                    int         m_RefreshRatePerSecond;        
};