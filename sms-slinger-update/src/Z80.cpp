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

#include "Config.h"
#include "Z80.h"
#include "LogMessages.h"
#include "Z80.Mnemonics.h"

#include <cassert>
#include <cstdio>
#include <memory.h>


///////////////////////////////////////////////////////////////////////

Z80::Z80(void)
{
    memset(&m_DAATable,0,sizeof(m_DAATable));
    memset(&m_ZSPTable,0,sizeof(m_ZSPTable));
    InitDAATable();
}

///////////////////////////////////////////////////////////////////////

Z80::~Z80(void)
{

}

WORD Z80::GetIXIYAddress(WORD ixiy)
{
    SIGNED_BYTE offset = (SIGNED_BYTE)m_ContextZ80.m_FuncPtrRead(m_ContextZ80.m_ProgramCounter);
    m_ContextZ80.m_ProgramCounter++;

    return ixiy+offset;
}

///////////////////////////////////////////////////////////////////////

WORD Z80::ReadWord() const
{
    WORD res = m_ContextZ80.m_FuncPtrRead(m_ContextZ80.m_ProgramCounter+1);
    res = res << 8;
    res |= m_ContextZ80.m_FuncPtrRead(m_ContextZ80.m_ProgramCounter);
    return res;
}

//////////////////////////////////////////////////////////////////

void Z80::PushWordOntoStack(WORD word)
{
    BYTE hi = word >> 8;
    BYTE lo = word & 0xFF;
    m_ContextZ80.m_StackPointer.reg--;
    m_ContextZ80.m_FuncPtrWrite(m_ContextZ80.m_StackPointer.reg, hi);
    m_ContextZ80.m_StackPointer.reg--;
    m_ContextZ80.m_FuncPtrWrite(m_ContextZ80.m_StackPointer.reg, lo);
}

//////////////////////////////////////////////////////////////////

WORD Z80::PopWordOffStack()
{
    WORD word = m_ContextZ80.m_FuncPtrRead(m_ContextZ80.m_StackPointer.reg+1) << 8;
    word |= m_ContextZ80.m_FuncPtrRead(m_ContextZ80.m_StackPointer.reg);
    m_ContextZ80.m_StackPointer.reg+=2;
    return word;
}

///////////////////////////////////////////////////////////////////////

int Z80::ExecuteNextOpcode()
{
    m_ContextZ80.m_OpcodeCycle = 0;

    BYTE opcode = m_ContextZ80.m_FuncPtrRead(m_ContextZ80.m_ProgramCounter);

    LogInstInfo(opcode, "", false);

    //if (m_ContextZ80.m_ProgramCounterStart == 0x88)
    //  _asm int 3;
    
    m_ContextZ80.m_ProgramCounterStart = m_ContextZ80.m_ProgramCounter;
    m_ContextZ80.m_ProgramCounter++;


    ExecuteOpcode(opcode);

    // if the last opcode we executed wasnt EI (0xFB) but we are pending the enable of interupts
    // then enable them.
    if ((opcode != 0xFB) && m_ContextZ80.m_EIPending)
    {
        m_ContextZ80.m_EIPending = false;
        m_ContextZ80.m_IFF1 = true;
        m_ContextZ80.m_IFF2 = true;
    }


    if (m_ContextZ80.m_OpcodeCycle == 0)
    {
        char buffer[255];
        sprintf(buffer, "Opcode %x didnt add to m_CyclesThisUpdate", opcode);
        LogMessage::GetSingleton()->DoLogMessage(buffer,true);
        assert(false);
    }

    return m_ContextZ80.m_OpcodeCycle;
}

///////////////////////////////////////////////////////////////////////

void Z80::LogInstInfo(BYTE opcode, const char* subset, bool showmnemonic)
{
    if (false)
    {
        char buffer[255];
        sprintf(buffer, "%s%x %s. PC: %x. AF: %x. BC: %x. DE: %x. HL: %x.", subset, opcode,(showmnemonic)?Z80MNEMONICSSTANDARD[opcode]:"",m_ContextZ80.m_ProgramCounter, m_ContextZ80.m_RegisterAF.reg,m_ContextZ80.m_RegisterBC.reg,m_ContextZ80.m_RegisterDE.reg,m_ContextZ80.m_RegisterHL.reg);
        LogMessage::GetSingleton()->DoLogMessage(buffer,false);
    }
}

///////////////////////////////////////////////////////////////////////





//////////////////////////////////////////////////////////////////////////////////

void Z80::InitDAATable()
{

    for (int i = 0; i < 256; ++i) 
    {
        BYTE zFlag = (i == 0) ? 0x40 : 0;
        BYTE sFlag = i & 0x80;
        BYTE vFlag = 0x04;
        for (int v = 128; v != 0; v >>= 1) 
        {
            if (i & v) 
                vFlag ^= 0x04;
        }
        m_ZSPTable[i] = zFlag | sFlag | vFlag;
    }



    for (int x = 0; x < 0x800; ++x) 
    {
        bool nf = x & 0x400;
        bool hf = x & 0x200;
        bool cf = x & 0x100;
        BYTE a = x & 0xFF;
        BYTE hi = a / 16;
        BYTE lo = a & 15;
        BYTE diff;
        if (cf) 
        {
            diff = ((lo <= 9) && !hf) ? 0x60 : 0x66;
        } 
        else 
        {
            if (lo >= 10) 
            {
                diff = (hi <= 8) ? 0x06 : 0x66;
            } else 
            {
                if (hi >= 10) 
                {
                    diff = hf ? 0x66 : 0x60;
                } else 
                {
                    diff = hf ? 0x06 : 0x00;
                }
            }
        }
        BYTE res_a = nf ? a - diff : a + diff;
        BYTE res_f = m_ZSPTable[res_a] | (nf ? 0x02 : 0);
        if (cf || ((lo <= 9) ? (hi >= 10) : (hi >= 9))) 
        {
            res_f |= 0x01;
        }
        if (nf ? (hf && (lo <= 5)) : (lo >= 10)) 
        {
            res_f |= 0x10;
        }
        m_DAATable[x] = (res_a << 8) + res_f;
    }

}
