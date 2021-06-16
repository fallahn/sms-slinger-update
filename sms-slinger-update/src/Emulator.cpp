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

#include "Config.hpp"
#include "Emulator.hpp"
#include "LogMessages.hpp"

#include <memory.h>
#include <cstdio>
#include <cassert>
#include <cstring>

namespace
{
    BYTE readByte(WORD address)
    {
        return Emulator::getSingleton()->readMemory(address);
    }

    void writeByte(WORD address, BYTE data)
    {
        Emulator::getSingleton()->writeMemory(address, data);
    }

    BYTE readIOByte(BYTE address)
    {
        return Emulator::getSingleton()->readIOMemory(address);
    }

    void writeIOByte(BYTE address, BYTE data)
    {
        Emulator::getSingleton()->writeIOMemory(address, data);
    }
}

std::unique_ptr<Emulator> Emulator::m_instance;

Emulator::Emulator()
    : m_cyclesThisUpdate(0),
    m_FPS               (60),
    m_isPAL             (false),
    m_isCodeMasters     (false),
    m_oneMegCartridge   (false),
    m_clockInfo         (0),
    m_firstBankPage     (0),
    m_secondBankPage    (0),
    m_thirdBankPage     (0),
    m_currentRam        (0)
{
    reset();
}

//public
Emulator* Emulator::createInstance()
{
    if (m_instance == nullptr)
    {
        m_instance = std::make_unique<Emulator>();
    }
    return m_instance.get();
}

Emulator* Emulator::getSingleton()
{
    if (m_instance == nullptr)
    {
        LogMessage::GetSingleton()->DoLogMessage("Trying to get the singleton of Emulator when m_instance is NULL", true);
        assert(false);
    }
    return m_instance.get();
}

void Emulator::reset()
{
    m_clockInfo = 0;

    CONTEXTZ80* context = m_Z80.GetContext();
    std::memset(&context->m_CartridgeMemory, 0, sizeof(context->m_CartridgeMemory));
    std::memset(&context->m_InternalMemory, 0, sizeof(context->m_InternalMemory));
   
    std::memset(&m_ramBank, 0, sizeof(m_ramBank));

    // these may have to be set to none 0 values so for now i'll set them to all
    // 0 instead of doing a memset so it becomes easier to edit later if necessary

    context->m_RegisterAF.reg = 0x0; // unofficial z80
    context->m_RegisterBC.reg = 0x0 ;// unofficial z80
    context->m_RegisterDE.reg = 0x0 ;// unofficial z80
    context->m_RegisterHL.reg = 0x0 ;// unofficial z80
    context->m_RegisterAFPrime.reg = 0x0 ;// unofficial z80
    context->m_RegisterBCPrime.reg = 0x0 ;// unofficial z80
    context->m_RegisterDEPrime.reg = 0x0 ;// unofficial z80
    context->m_RegisterHLPrime.reg = 0x0 ;// unofficial z80
    context->m_RegisterIX.reg = 0 ;
    context->m_RegisterIY.reg = 0 ;
    context->m_RegisterI = 0;
    context->m_RegisterR = 0;
    context->m_ProgramCounter = 0;
    context->m_OpcodeCycle = 0;
    context->m_StackPointer.reg = 0xDFF0;
    context->m_InternalMemory[0xFFFF] = 2; // official sega doc
    context->m_InternalMemory[0xFFFE] = 1; // official sega doc
    context->m_FuncPtrRead = &readByte;
    context->m_FuncPtrWrite = &writeByte;
    context->m_FuncPtrIORead = &readIOByte;
    context->m_FuncPtrIOWrite = &writeIOByte;
    context->m_IFF1 = false;
    context->m_IFF2 = false;
    context->m_Halted = false;
    context->m_InteruptMode = 1;
    context->m_NMI = false;
    context->m_NMIServicing = false;
    context->m_EIPending = false;

    std::fill(m_keyboardPorts.begin(), m_keyboardPorts.end(), 0xFF);

    m_cyclesThisUpdate = 0;
    m_oneMegCartridge = false;
    m_currentRam = -1;

    m_soundChip.reset();
}

void Emulator::insertCartridge(const char* path)
{
    CONTEXTZ80* context = m_Z80.GetContext();

    FILE *in = nullptr;

    // get the file size
    in = fopen(path, "rb");
    fseek(in, 0L, SEEK_END);
    long endPos = ftell(in);
    fclose(in);

    in = fopen(path, "rb");

    endPos = endPos % 16384;

    if ((endPos == 512) || (endPos == 64))
    {
        // we need to strip off the old header in order for the rom to load
        // http://www.smspower.org/forums/viewtopic.php?t=7999&highlight=header+512
        char header[1000];
        if (endPos == 512)
        {
            fread(header, 1, 512, in);
        }
        else
        {
            assert(false); // im not sure if this is correct because the post says the last 64 bytes not the first
            fread(header, 1, 64, in);
        }
    }

    size_t size = fread(context->m_CartridgeMemory, 1, 0x100000, in);
    m_oneMegCartridge = (endPos > 0x80000)?true:false;
    
    std::memcpy(&context->m_InternalMemory[0x0], &context->m_CartridgeMemory[0x0], 0xC000);

    context->m_InternalMemory[0xFFFE] = 0x01;
    context->m_InternalMemory[0xFFFF] = 0x02;

    m_firstBankPage = 0;
    m_secondBankPage = 1;
    m_thirdBankPage = 2;

    m_isPAL = false;
    m_graphicsChip.Reset(m_isPAL);
    m_FPS = m_isPAL ? 50 : 60;
    m_isCodeMasters = isCodeMasters();


    BYTE a0 = context->m_InternalMemory[0x99];

    // codemasters games are initialized with banks 0,1,0 in slots 0,1,2
    if (m_isCodeMasters)
    {
        doMemPageCM(0x0,0);
        doMemPageCM(0x4000,1);
        doMemPageCM(0x8000,0);
    }
}

void Emulator::update()
{
    unsigned long int targetCPU = MACHINE_CLICKS;
    targetCPU /= m_FPS;

    m_cyclesThisUpdate = 0;
    m_graphicsChip.ResetScreen();
    while (!m_graphicsChip.GetRefresh())
    //while (m_cyclesThisUpdate < targetCPU)
    { 
        int cycles = 0;
        if (m_Z80.GetContext()->m_Halted)
        {
            cycles = 4;
        }
        else
        {
            cycles = m_Z80.ExecuteNextOpcode();
        }
        checkInterupts();
        
        //http://www.smspower.org/forums/viewtopic.php?p=44198      
                
        // convert from clock cycles to machine cycles
        // FIX ME! THIS SHOULD BE * 3, not * 2. HOWEVER WITH * 3 SOME GAMES FEEL REALLY CRAP AND SLOW
        // I BELIEVE ITS A VSYNC INTERRUPT ISSUE

        //potentially related to the (fixed) typo in TMS9918A::GetHCount()? - M
        //cycles *= 2;
        cycles *= 3;

        m_cyclesThisUpdate += cycles;
        m_clockInfo += cycles;

        // graphics chips clock is half of that of the sms machine clock
        float vdpClock = static_cast<float>(cycles);
        vdpClock /= 2;
        m_graphicsChip.Update(vdpClock);

        float soundCycles = static_cast<float>(cycles);
        soundCycles /= CPU_CYCLES_TO_MACHINE_CLICKS;
        m_soundChip.update(soundCycles);       
    }
}

BYTE Emulator::readMemory(const WORD& address)
{
    CONTEXTZ80* context = m_Z80.GetContext();
    WORD addr = address;

    if (addr >= 0xFFFC)
    {
        addr -= 0x2000;
    }

    // the fixed memory address
    if (!m_isCodeMasters && (addr < 0x400))
    {
        return context->m_InternalMemory[addr];
    }
    // bank 0
    else if (addr < 0x4000)
    {
        unsigned int bankaddr = addr + (0x4000 * m_firstBankPage);
        return context->m_CartridgeMemory[bankaddr];
    }
    // bank 1
    else if (addr < 0x8000)
    {
        unsigned int bankaddr = addr + (0x4000 * m_secondBankPage);
        bankaddr-=0x4000;
        return context->m_CartridgeMemory[bankaddr];
    }
    // bank 2
    else if (addr < 0xC000)
    {
        // is ram banking mapped in this slot?
        if (m_currentRam > -1)
        {
            return m_ramBank[m_currentRam][addr-0x8000];
        }
        else
        {
            unsigned int bankaddr = addr + (0x4000 * m_thirdBankPage);
            bankaddr-=0x8000;
            return context->m_CartridgeMemory[bankaddr];
        }
    }

    return context->m_InternalMemory[addr];
}

void Emulator::writeMemory(const WORD& address, const BYTE& data)
{
    CONTEXTZ80* context = m_Z80.GetContext();

    if (m_isCodeMasters)
    {
        if (address == 0x0)
        {
            doMemPageCM(address,data);
        }
        else if (address == 0x4000)
        {
            doMemPageCM(address,data);
        }
        else if (address == 0x8000)
        {
            doMemPageCM(address,data);
        }
    }

    // cant write to rom
    if (address < 0x8000)
    {
        return;
    }

    // only allow writing to here if a ram bank is mapped into this slot
    else if (address < 0xC000)
    {
        BYTE controlMap = context->m_InternalMemory[0xFFFC];
        if (m_currentRam > -1)
        {
            m_ramBank[m_currentRam][address-0x8000] = data;
            return;
        }
        else
        {
            // this is rom so lets return
            return;
        }
    }

    context->m_InternalMemory[address] = data;

    if (address >= 0xFFFC)
    {
        if (!m_isCodeMasters)
        {
            doMemPage(address, data);
        }
    }

    //  if you uncomment the following crap, you need to find out what happens to the rom/ram banking with mirroring
    // for example if address == 0xDFFF then mirroring with overwrite 0xFFFF which is a ram bank. Should I allow this?
    if (address >= 0xC000 && address < 0xDFFC)
    {
        context->m_InternalMemory[address + 0x2000] = data;
    }

    if (address >= 0xE000)
    {
        context->m_InternalMemory[address - 0x2000] = data;
    }
}

BYTE Emulator::readIOMemory(const BYTE& address)
{
    CONTEXTZ80* context = m_Z80.GetContext();

    if (address < 0x40)
    {
        return 0xFF;
    }

    if ((address >= 0x40) && (address <= 0x7F))
    {
        // even addresses are v counter, odd are h counter
        if ((address % 2)== 0)
        {
            return m_graphicsChip.GetVCounter();
        }
        return 0; // h counter
    }

    if ((address >= 0x80) && (address <= 0xBF))
    {
        //Even locations are data port, odd locations are control port.
        if ((address % 2)== 0)
        {
            return m_graphicsChip.ReadDataPort();
        }
        return m_graphicsChip.GetStatus();    
    }

    switch (address)
    {
        case 0xBE: return m_graphicsChip.ReadDataPort(); break;
        case 0xBF: return m_graphicsChip.GetStatus(); break;
        case 0xBD: return m_graphicsChip.GetStatus(); break; // mirror of 0xBF
        case 0x7F: return m_graphicsChip.GetHCounter() ; break; // hblank
        case 0x7E: return m_graphicsChip.GetVCounter(); break; 
        case 0xDC: return m_keyboardPorts[0]; break;
        case 0xC0: return m_keyboardPorts[0]; break; // mirror of 0xDC
        case 0xDD: return m_keyboardPorts[1]; break;
        case 0xC1: return m_keyboardPorts[1]; break; // mirror of 0xDD

        default: return 0xFF; break;
    }
}

void Emulator::writeIOMemory(const BYTE& address, const BYTE& data)
{
    CONTEXTZ80* context = m_Z80.GetContext();

    if (address < 0x40)
    {
        return;
    }

    if ((address >=0x40) && (address < 0x80))
    {
        // sound
        m_soundChip.writeData(m_cyclesThisUpdate , data);
        return;
    }

    switch (address)
    {
        case 0xBE: m_graphicsChip.WriteDataPort(data); break;
        case 0xBF: 
        {
//          char buffer[0x200];
//          sprintf(buffer, "PC is %x", context->m_ProgramCounterStart);
//          LogMessage::GetSingleton()->DoLogMessage(buffer, false);
            m_graphicsChip.WriteVDPAddress(data);
        }break;
        case 0xBD: 
            {
//              char buffer[0x200];
//              sprintf(buffer, "PC is %x", context->m_ProgramCounterStart);
//              LogMessage::GetSingleton()->DoLogMessage(buffer, false);
                m_graphicsChip.WriteVDPAddress(data);
            }
            break;
        default:  break;
    }
}

void Emulator::setKeyPressed(int player, int key)
{
    assert(player < 2);
    BYTE& port = m_keyboardPorts[player];
    port = bitReset(port, key);
}

void Emulator::setKeyReleased(int player, int key)
{
    assert(player < 2);
    BYTE& port = m_keyboardPorts[player];
    port = bitSet(port, key);
}

void Emulator::resetButton()
{
    CONTEXTZ80* context = m_Z80.GetContext();

    if (!context->m_NMIServicing)
    {
        context->m_NMI = true;
    }

    setKeyPressed(1, 4);
}

void Emulator::dumpClockInfo()
{
//  return;
//  DWORD tickCount = GetTickCount();
// 
//  char buffer[255];
//  memset(buffer,0,sizeof(buffer));
//  sprintf(buffer, "Tick Count is %lu", tickCount);
//  LogMessage::GetSingleton()->DoLogMessage(buffer, true);
// 
//  memset(buffer,0,sizeof(buffer));
//  sprintf(buffer, "Machine Clicks Per Second: %lu", m_clockInfo);
//  LogMessage::GetSingleton()->DoLogMessage(buffer, true);
// 
//  m_graphicsChip.DumpClockInfo();
//  m_soundChip.DumpClockInfo();

    m_clockInfo = 0;
}

void Emulator::checkInterupts()
{
    if  (m_Z80.GetContext()->m_NMI && (m_Z80.GetContext()->m_NMIServicing == false))
    {
        m_Z80.IncreaseRReg();
        m_Z80.GetContext()->m_NMIServicing = true;
        m_Z80.GetContext()->m_NMI = false;

        CONTEXTZ80* context = m_Z80.GetContext();
        context->m_IFF1 = false;
        context->m_Halted = false;
        m_Z80.PushWordOntoStack(context->m_ProgramCounter);
        context->m_ProgramCounter = 0x66;
    }

    if (m_graphicsChip.IsRequestingInterupt())
    {
        m_Z80.IncreaseRReg();
        CONTEXTZ80* context = m_Z80.GetContext();
        if (context->m_IFF1 && context->m_InteruptMode == 1)
        {
            context->m_Halted = false;

            m_Z80.PushWordOntoStack(context->m_ProgramCounter);
            context->m_ProgramCounter = 0x38;
            context->m_IFF1 = false;
            context->m_IFF2 = false;
        }
    }
}

//private
bool Emulator::isCodeMasters()
{
    // a code masters rom header has a checksum. So if the checksum is correct it must be a code masters game
    CONTEXTZ80* context = m_Z80.GetContext();

    WORD checksum = context->m_InternalMemory[0x7fe7] << 8;
    checksum |= context->m_InternalMemory[0x7fe6];

    if (checksum == 0x0)
    {
        return false;
    }

    WORD compute = 0x10000 - checksum;

    WORD answer = context->m_InternalMemory[0x7fe9] << 8;
    answer |= context->m_InternalMemory[0x7fe8];

    return (compute == answer);
}

void Emulator::doMemPageCM(WORD address, BYTE data)
{
    CONTEXTZ80* context = m_Z80.GetContext();
    BYTE page = bitReset(data, 7);
    page = bitReset(page, 6);
    page = bitReset(page, 5);

    switch(address)
    {
        case 0x0: m_firstBankPage = page; break; //memcpy(&context->m_InternalMemory[0x0], &context->m_CartridgeMemory[(0x4000*page)], 0x4000); break;
        case 0x4000: m_secondBankPage = page; break;//memcpy(&context->m_InternalMemory[0x4000], &context->m_CartridgeMemory[(0x4000*page)], 0x4000); break;
        case 0x8000: m_thirdBankPage = page; break;//memcpy(&context->m_InternalMemory[0x8000], &context->m_CartridgeMemory[(0x4000*page)], 0x4000); break;
    }
}

void Emulator::doMemPage(WORD address, BYTE data)
{
    CONTEXTZ80* context = m_Z80.GetContext();

    // memory paging. ROXOR!!!
    if (address >= 0xFFFC)
    {
        // I think the seventh bit is never used in page mirroring.
        BYTE page = m_oneMegCartridge ? (data & 0x3F) : (data & 0x1F);

        context->m_InternalMemory[address-0x2000] = data; // ram mirror

        if (false)
        {
            char buffer[200];
            sprintf (buffer, "Mem Paging address %x datd %x page %x", address, data, page);
            LogMessage::GetSingleton()->DoLogMessage(buffer, false);
        }

        switch(address)
        {
            case 0xFFFC:
            {
                // check for slot 2 ram banking
                if (testBit(data,3))
                {
                    // which of the two ram banks are we swapping in?
                    bool secondBank = testBit(data,2);

                    if (secondBank)
                    {
                        m_currentRam = 1;//memcpy(&context->m_InternalMemory[0x8000], &m_ramBank[1], 0x4000);
                    }
                    else
                    {
                        m_currentRam = 0;//memcpy(&context->m_InternalMemory[0x8000], &m_ramBank[0], 0x4000);
                    }
                }
                else
                {
                    m_currentRam = -1;
                }

                // apparently no games use ram banking in address 0xC000
                assert(testBit(data,4) == 0);
            }
            break;

            case 0xFFFD: m_firstBankPage = page; break;// memcpy(&context->m_InternalMemory[0x400], &context->m_CartridgeMemory[(0x4000*page)+0x400], 0x3C00); break;
    
            case 0xFFFE: m_secondBankPage = page; break; // memcpy(&context->m_InternalMemory[0x4000], &context->m_CartridgeMemory[0x4000*page], 0x4000);break;
            case 0xFFFF:
            {
                // only allow rom banking in slot 2 if ram is not mapped there!
                if (false == testBit(context->m_InternalMemory[0xFFFC],3))
                {
                    m_thirdBankPage = page;
                    //memcpy(&context->m_InternalMemory[0x8000], &context->m_CartridgeMemory[0x4000*page], 0x4000);
                }
            }
            break;
        }
    }
}