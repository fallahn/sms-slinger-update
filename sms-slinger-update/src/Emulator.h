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

#include "Z80.h"
#include "TMS9918A.h"
#include "SN79489.h"

class Emulator
{
public:
    static  Emulator* CreateInstance();
    static  Emulator* GetSingleton();

    void                Reset();
    void                InsertCartridge(const char* path);
    void                Update();

    BYTE                ReadMemory(const WORD& address);
    void                WriteMemory(const WORD& address, const BYTE& data);
    BYTE                ReadIOMemory(const BYTE& address);
    void                WriteIOMemory(const BYTE& address, const BYTE& data);
    TMS9918A&           GetGraphicChip() { return m_GraphicsChip; }

    void                SetKeyPressed(int player, int key);
    void                SetKeyReleased(int player, int key);
    void                ResetButton();
    void                DumpClockInfo();
    void                SetGFXOpt(bool useGFXOpt) { m_GraphicsChip.SetGFXOpt(useGFXOpt); }
    void                CheckInterupts();


    static       const  long long   MACHINE_CLICKS = 10738635;
    static       const  int         CPU_CYCLES_TO_MACHINE_CLICKS = 3;

private:
    static      Emulator* m_Instance;
    Emulator(void);

    bool                IsPAL() const;
    bool                IsCodeMasters();
    void                DoMemPage(WORD address, BYTE data);
    void                DoMemPageCM(WORD address, BYTE data);


    unsigned long int   m_CyclesThisUpdate;
    int                 m_FPS;
    TMS9918A            m_GraphicsChip;
    SN79489             m_SoundChip;

    Z80                 m_Z80;

    BYTE                m_RamBank[0x2][0x4000];

    BYTE                m_KeyboardPort1;
    BYTE                m_KeyboardPort2;
    bool                m_IsPAL;
    bool                m_IsCodeMasters;
    bool                m_OneMegCartridge;
    unsigned long int   m_ClockInfo;
    BYTE                m_FirstBankPage;
    BYTE                m_SecondBankPage;
    BYTE                m_ThirdBankPage;
    int                 m_CurrentRam;
};