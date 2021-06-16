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

#include "Z80.hpp"
#include "TMS9918A.hpp"
#include "SN79489.hpp"

#include <memory>
#include <array>

class Emulator final
{
public:
    Emulator();

    static Emulator* createInstance();
    static Emulator* getSingleton();

    void reset();
    void insertCartridge(const char* path);
    void update();

    BYTE readMemory(const WORD& address);
    void writeMemory(const WORD& address, const BYTE& data);
    BYTE readIOMemory(const BYTE& address);
    void writeIOMemory(const BYTE& address, const BYTE& data);
    TMS9918A& getGraphicChip() { return m_graphicsChip; }

    void setKeyPressed(int player, int key);
    void setKeyReleased(int player, int key);
    void resetButton();
    void dumpClockInfo();
    void setGFXOpt(bool useGFXOpt) { m_graphicsChip.setGFXOpt(useGFXOpt); }
    void checkInterupts();


    static constexpr long long MACHINE_CLICKS = 10738635;
    static constexpr int CPU_CYCLES_TO_MACHINE_CLICKS = 3;

private:
    static std::unique_ptr<Emulator> m_instance;

    unsigned long int m_cyclesThisUpdate;
    int m_FPS;
    TMS9918A m_graphicsChip;
    SN79489 m_soundChip;

    Z80 m_Z80;

    BYTE m_ramBank[0x2][0x4000];

    std::array<BYTE, 2u> m_keyboardPorts = {};
    bool m_isPAL;
    bool m_isCodeMasters;
    bool m_oneMegCartridge;
    unsigned long int m_clockInfo;
    BYTE m_firstBankPage;
    BYTE m_secondBankPage;
    BYTE m_thirdBankPage;
    int m_currentRam;

    bool isCodeMasters();
    void doMemPage(WORD address, BYTE data);
    void doMemPageCM(WORD address, BYTE data);
};