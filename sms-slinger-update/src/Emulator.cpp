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

///////////////////////////////////////////////////////////////////////////////////

std::unique_ptr<Emulator> Emulator::m_Instance;

Emulator* Emulator::CreateInstance()
{
    if (m_Instance == nullptr)
    {
        m_Instance = std::make_unique<Emulator>();
    }
    return m_Instance.get();
}

///////////////////////////////////////////////////////////////////////////////////

Emulator* Emulator::GetSingleton()
{
    if (m_Instance == nullptr)
    {
        LogMessage::GetSingleton()->DoLogMessage("Trying to get the singleton of Emulator when m_Instance is NULL", true);
        assert(false);
    }
    return m_Instance.get();
}

///////////////////////////////////////////////////////////////////////////////////

BYTE ReadByte(WORD address)
{
    return Emulator::GetSingleton()->ReadMemory(address);
}

///////////////////////////////////////////////////////////////////////////////////

void WriteByte(WORD address, BYTE data)
{
    Emulator::GetSingleton()->WriteMemory(address,data);
}

///////////////////////////////////////////////////////////////////////////////////

BYTE ReadIOByte(BYTE address)
{
    return Emulator::GetSingleton()->ReadIOMemory(address);
}

///////////////////////////////////////////////////////////////////////////////////

void WriteIOByte(BYTE address, BYTE data)
{
    Emulator::GetSingleton()->WriteIOMemory(address,data);
}

///////////////////////////////////////////////////////////////////////////////////

Emulator::Emulator()
    : m_FPS         (60),
    m_IsPAL         (false),
    m_IsCodeMasters (false)
{
    Reset();
}

///////////////////////////////////////////////////////////////////////////////////

void Emulator::Reset()
{
    m_ClockInfo = 0;

    CONTEXTZ80* context = m_Z80.GetContext();
    std::memset(&context->m_CartridgeMemory,0,sizeof(context->m_CartridgeMemory));
    std::memset(&context->m_InternalMemory,0,sizeof(context->m_InternalMemory));
   
    std::memset(&m_RamBank,0, sizeof(m_RamBank));

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
    context->m_FuncPtrRead = &ReadByte;
    context->m_FuncPtrWrite = &WriteByte;
    context->m_FuncPtrIORead = &ReadIOByte;
    context->m_FuncPtrIOWrite = &WriteIOByte;
    context->m_IFF1 = false;
    context->m_IFF2 = false;
    context->m_Halted = false;
    context->m_InteruptMode = 1;
    context->m_NMI = false;
    context->m_NMIServicing = false;
    context->m_EIPending = false;

    m_KeyboardPort1 = 0xFF;
    m_KeyboardPort2 = 0xFF;

    m_CyclesThisUpdate = 0;
    m_OneMegCartridge = false;
    m_CurrentRam = -1;

    m_SoundChip.Reset();
}

///////////////////////////////////////////////////////////////////////////////////

void Emulator::InsertCartridge(const char* path)
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
    m_OneMegCartridge = (endPos > 0x80000)?true:false;
    
    std::memcpy(&context->m_InternalMemory[0x0], &context->m_CartridgeMemory[0x0], 0xC000);

    context->m_InternalMemory[0xFFFE] = 0x01;
    context->m_InternalMemory[0xFFFF] = 0x02;

    m_FirstBankPage = 0;
    m_SecondBankPage = 1;
    m_ThirdBankPage = 2;

    m_IsPAL = false;
    m_GraphicsChip.Reset(IsPAL());
    m_FPS = IsPAL() ? 50 : 60;
    m_IsCodeMasters = IsCodeMasters();


    BYTE a0 = context->m_InternalMemory[0x99];

    // codemasters games are initialized with banks 0,1,0 in slots 0,1,2
    if (m_IsCodeMasters)
    {
        DoMemPageCM(0x0,0);
        DoMemPageCM(0x4000,1);
        DoMemPageCM(0x8000,0);
    }
}

///////////////////////////////////////////////////////////////////////////////////

void Emulator::Update()
{
    unsigned long int targetCPU = MACHINE_CLICKS;
    targetCPU /= m_FPS;

    m_CyclesThisUpdate = 0;
    m_GraphicsChip.ResetScreen();
    while (!m_GraphicsChip.GetRefresh())
    //while (m_CyclesThisUpdate < targetCPU)
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
        CheckInterupts();
        
        //http://www.smspower.org/forums/viewtopic.php?p=44198      
                
        // convert from clock cycles to machine cycles
        // FIX ME! THIS SHOULD BE * 3, not * 2. HOWEVER WITH * 3 SOME GAMES FEEL REALLY CRAP AND SLOW
        // I BELIEVE ITS A VSYNC INTERRUPT ISSUE

        //potentially related to the (fixed) typo in TMS9918A::GetHCount()? M
        //cycles *= 2;
        cycles *= 3;

        m_CyclesThisUpdate += cycles;
        m_ClockInfo += cycles;

        // graphics chips clock is half of that of the sms machine clock
        float vdpClock = static_cast<float>(cycles);
        vdpClock /= 2;
        m_GraphicsChip.Update(vdpClock);

        float soundCycles = static_cast<float>(cycles);
        soundCycles /= CPU_CYCLES_TO_MACHINE_CLICKS;
        m_SoundChip.Update(soundCycles);       
    }
}


///////////////////////////////////////////////////////////////////////////////////

void Emulator::CheckInterupts()
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

    if (m_GraphicsChip.IsRequestingInterupt())
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

///////////////////////////////////////////////////////////////////////////////////

BYTE Emulator::ReadMemory(const WORD& address)
{
    CONTEXTZ80* context = m_Z80.GetContext();
    WORD addr = address;

    if (addr >= 0xFFFC)
    {
        addr -= 0x2000;
    }

    // the fixed memory address
    if (!m_IsCodeMasters && (addr < 0x400))
    {
        return context->m_InternalMemory[addr];
    }
    // bank 0
    else if (addr < 0x4000)
    {
        unsigned int bankaddr = addr + (0x4000 * m_FirstBankPage);
        return context->m_CartridgeMemory[bankaddr];
    }
    // bank 1
    else if (addr < 0x8000)
    {
        unsigned int bankaddr = addr + (0x4000 * m_SecondBankPage);
        bankaddr-=0x4000;
        return context->m_CartridgeMemory[bankaddr];
    }
    // bank 2
    else if (addr < 0xC000)
    {
        // is ram banking mapped in this slot?
        if (m_CurrentRam > -1)
        {
            return m_RamBank[m_CurrentRam][addr-0x8000];
        }
        else
        {
            unsigned int bankaddr = addr + (0x4000 * m_ThirdBankPage);
            bankaddr-=0x8000;
            return context->m_CartridgeMemory[bankaddr];
        }
    }

    return context->m_InternalMemory[addr];
}

///////////////////////////////////////////////////////////////////////////////////

void Emulator::WriteMemory(const WORD& address, const BYTE& data)
{
    CONTEXTZ80* context = m_Z80.GetContext();

    if (m_IsCodeMasters)
    {
        if (address == 0x0)
        {
            DoMemPageCM(address,data);
        }
        else if (address == 0x4000)
        {
            DoMemPageCM(address,data);
        }
        else if (address == 0x8000)
        {
            DoMemPageCM(address,data);
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
        if (m_CurrentRam > -1)
        {
            m_RamBank[m_CurrentRam][address-0x8000] = data;
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
        if (!m_IsCodeMasters)
        {
            DoMemPage(address, data);
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


///////////////////////////////////////////////////////////////////////////////////

void Emulator::DoMemPageCM(WORD address, BYTE data)
{
    CONTEXTZ80* context = m_Z80.GetContext();
    BYTE page = BitReset(data, 7);
    page = BitReset(page, 6);
    page = BitReset(page, 5);

    switch(address)
    {
        case 0x0: m_FirstBankPage = page; break; //memcpy(&context->m_InternalMemory[0x0], &context->m_CartridgeMemory[(0x4000*page)], 0x4000); break;
        case 0x4000: m_SecondBankPage = page; break;//memcpy(&context->m_InternalMemory[0x4000], &context->m_CartridgeMemory[(0x4000*page)], 0x4000); break;
        case 0x8000: m_ThirdBankPage = page; break;//memcpy(&context->m_InternalMemory[0x8000], &context->m_CartridgeMemory[(0x4000*page)], 0x4000); break;
    }
}

///////////////////////////////////////////////////////////////////////////////////

void Emulator::DoMemPage(WORD address, BYTE data)
{
    CONTEXTZ80* context = m_Z80.GetContext();

    // memory paging. ROXOR!!!
    if (address >= 0xFFFC)
    {
        // I think the seventh bit is never used in page mirroring.
        BYTE page = m_OneMegCartridge ? (data & 0x3F) : (data & 0x1F);

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
                if (TestBit(data,3))
                {
                    // which of the two ram banks are we swapping in?
                    bool secondBank = TestBit(data,2);

                    if (secondBank)
                    {
                        m_CurrentRam = 1;//memcpy(&context->m_InternalMemory[0x8000], &m_RamBank[1], 0x4000);
                    }
                    else
                    {
                        m_CurrentRam = 0;//memcpy(&context->m_InternalMemory[0x8000], &m_RamBank[0], 0x4000);
                    }
                }
                else
                {
                    m_CurrentRam = -1;
                }

                // apparently no games use ram banking in address 0xC000
                assert(TestBit(data,4) == 0);
            }
            break;

            case 0xFFFD: m_FirstBankPage = page; break;// memcpy(&context->m_InternalMemory[0x400], &context->m_CartridgeMemory[(0x4000*page)+0x400], 0x3C00); break;
    
            case 0xFFFE: m_SecondBankPage = page; break; // memcpy(&context->m_InternalMemory[0x4000], &context->m_CartridgeMemory[0x4000*page], 0x4000);break;
            case 0xFFFF:
            {
                // only allow rom banking in slot 2 if ram is not mapped there!
                if (false == TestBit(context->m_InternalMemory[0xFFFC],3))
                {
                    m_ThirdBankPage = page;
                    //memcpy(&context->m_InternalMemory[0x8000], &context->m_CartridgeMemory[0x4000*page], 0x4000);
                }
            }
            break;
        }
    }
}

///////////////////////////////////////////////////////////////////////////////////

BYTE Emulator::ReadIOMemory(const BYTE& address)
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
            return m_GraphicsChip.GetVCounter();
        }
        return 0; // h counter
    }

    if ((address >= 0x80) && (address <= 0xBF))
    {
        //Even locations are data port, odd locations are control port.
        if ((address % 2)== 0)
        {
            return m_GraphicsChip.ReadDataPort();
        }
        return m_GraphicsChip.GetStatus();    
    }

    switch (address)
    {
        case 0xBE: return m_GraphicsChip.ReadDataPort(); break;
        case 0xBF: return m_GraphicsChip.GetStatus(); break;
        case 0xBD: return m_GraphicsChip.GetStatus(); break; // mirror of 0xBF
        case 0x7F: return m_GraphicsChip.GetHCounter() ; break; // hblank
        case 0x7E: return m_GraphicsChip.GetVCounter(); break; 
        case 0xDC: return m_KeyboardPort1; break;
        case 0xC0: return m_KeyboardPort1; break; // mirror of 0xDC
        case 0xDD: return m_KeyboardPort2; break;
        case 0xC1: return m_KeyboardPort2; break; // mirror of 0xDD

        default: return 0xFF; break;
    }
}

///////////////////////////////////////////////////////////////////////////////////

void Emulator::WriteIOMemory(const BYTE& address, const BYTE& data)
{
    CONTEXTZ80* context = m_Z80.GetContext();

    if (address < 0x40)
    {
        return;
    }

    if ((address >=0x40) && (address < 0x80))
    {
        // sound
        m_SoundChip.WriteData(m_CyclesThisUpdate , data);
        return;
    }

    switch (address)
    {
        case 0xBE: m_GraphicsChip.WriteDataPort(data); break;
        case 0xBF: 
        {
//          char buffer[0x200];
//          sprintf(buffer, "PC is %x", context->m_ProgramCounterStart);
//          LogMessage::GetSingleton()->DoLogMessage(buffer, false);
            m_GraphicsChip.WriteVDPAddress(data);
        }break;
        case 0xBD: 
            {
//              char buffer[0x200];
//              sprintf(buffer, "PC is %x", context->m_ProgramCounterStart);
//              LogMessage::GetSingleton()->DoLogMessage(buffer, false);
                m_GraphicsChip.WriteVDPAddress(data);
            }
            break;
        default:  break;
    }
}

///////////////////////////////////////////////////////////////////////////////////

bool Emulator::IsPAL() const
{
    return m_IsPAL;
}

///////////////////////////////////////////////////////////////////////////////////

void Emulator::SetKeyPressed(int player, int key)
{
    BYTE& port = (player == 1) ? m_KeyboardPort1 : m_KeyboardPort2;
    port = BitReset(port, key);
}

///////////////////////////////////////////////////////////////////////////////////

void Emulator::SetKeyReleased(int player, int key)
{
    BYTE& port = (player == 1) ? m_KeyboardPort1 : m_KeyboardPort2;
    port = BitSet(port, key);
}
///////////////////////////////////////////////////////////////////////////////////


// a code masters rom header has a checksum. So if the checksum is correct it must be a code masters game
bool Emulator::IsCodeMasters()
{
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

///////////////////////////////////////////////////////////////////////////////////

void Emulator::ResetButton()
{
    CONTEXTZ80* context = m_Z80.GetContext();

    if (!context->m_NMIServicing)
    {
        context->m_NMI = true;
    }

    SetKeyPressed(2, 4);
}

///////////////////////////////////////////////////////////////////////////////////

void Emulator::DumpClockInfo()
{
//  return;
//  DWORD tickCount = GetTickCount();
// 
//  char buffer[255];
//  memset(buffer,0,sizeof(buffer));
//  sprintf(buffer, "Tick Count is %u", tickCount);
//  LogMessage::GetSingleton()->DoLogMessage(buffer, true);
// 
//  memset(buffer,0,sizeof(buffer));
//  sprintf(buffer, "Machine Clicks Per Second: %u", m_ClockInfo);
//  LogMessage::GetSingleton()->DoLogMessage(buffer, true);
// 
//  m_GraphicsChip.DumpClockInfo();
//  m_SoundChip.DumpClockInfo();

    m_ClockInfo = 0;
}

///////////////////////////////////////////////////////////////////////////////////
