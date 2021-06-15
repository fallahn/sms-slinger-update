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

#include "Config.h"
#include "Z80.h"


// load immediate byte into reg
void Z80::CPU_8BIT_LOAD_IMMEDIATE(BYTE& reg)
{
    m_ContextZ80.m_OpcodeCycle = 7;
    BYTE n = m_ContextZ80.m_FuncPtrRead(m_ContextZ80.m_ProgramCounter);
    m_ContextZ80.m_ProgramCounter++;
    reg = n;
}

///////////////////////////////////////////////////////////////////////

void Z80::CPU_REG_LOAD(BYTE& reg, BYTE load)
{
    m_ContextZ80.m_OpcodeCycle = 4;
    reg = load;
}

///////////////////////////////////////////////////////////////////////

void Z80::CPU_REG_LOAD_ROM(BYTE& reg, WORD address)
{
    m_ContextZ80.m_OpcodeCycle = 7;
    reg = m_ContextZ80.m_FuncPtrRead(address);
}

///////////////////////////////////////////////////////////////////////

void Z80::CPU_16BIT_LOAD(WORD& reg)
{
    m_ContextZ80.m_OpcodeCycle = 10;
    WORD n = ReadWord();
    m_ContextZ80.m_ProgramCounter+=2;
    reg = n;
}

///////////////////////////////////////////////////////////////////////

void Z80::CPU_8BIT_ADD(BYTE& reg, BYTE toAdd, int cycles, bool useImmediate, bool addCarry)
{
    m_ContextZ80.m_OpcodeCycle = cycles;
    BYTE before = reg;
    unsigned int adding = 0; // must be unsigned int not byte. Because adding carry to 0xFF would cause overflow and the carry flag wouldnt get set
    BYTE nonMod = toAdd;
    int res = 0;

    // are we adding immediate data or the second param?
    if (useImmediate)
    {
        BYTE n = m_ContextZ80.m_FuncPtrRead(m_ContextZ80.m_ProgramCounter);
        m_ContextZ80.m_ProgramCounter++;
        adding = n;
        nonMod = n;
    }
    else
    {
        adding = toAdd;
    }

    // are we also adding the carry flag?
    if (addCarry)
    {
        if (TestBit(m_ContextZ80.m_RegisterAF.lo, FLAG_C))
            adding++;
    }

    res = reg + adding;
    reg+=adding;

    // set the flags
    m_ContextZ80.m_RegisterAF.lo = 0;

    // correct
    if (TestBit(reg,7))
        m_ContextZ80.m_RegisterAF.lo = BitSet(m_ContextZ80.m_RegisterAF.lo, FLAG_S);

    //  if (TestBit(reg,5))
    //      m_ContextZ80.m_RegisterAF.lo = BitSet(m_ContextZ80.m_RegisterAF.lo, FLAG_B5);
    //
    //  if (TestBit(reg,3))
    //      m_ContextZ80.m_RegisterAF.lo = BitSet(m_ContextZ80.m_RegisterAF.lo, FLAG_B3);

    // correct
    if (reg == 0)
        m_ContextZ80.m_RegisterAF.lo = BitSet(m_ContextZ80.m_RegisterAF.lo, FLAG_Z);

    // correct
    if(((nonMod ^ before ^ 0x80) & (nonMod ^ res) & 0x80))
        m_ContextZ80.m_RegisterAF.lo = BitSet(m_ContextZ80.m_RegisterAF.lo, FLAG_PV);

    // correct
    if ((before ^ res ^ nonMod) & 0x10)
        m_ContextZ80.m_RegisterAF.lo = BitSet(m_ContextZ80.m_RegisterAF.lo, FLAG_H);

    // correct
    if ((before + adding) > 0xFF)
        m_ContextZ80.m_RegisterAF.lo = BitSet(m_ContextZ80.m_RegisterAF.lo, FLAG_C);
}

///////////////////////////////////////////////////////////////////////

void Z80::CPU_8BIT_SUB(BYTE& reg, BYTE subtracting, int cycles, bool useImmediate, bool subCarry)
{
    m_ContextZ80.m_OpcodeCycle = cycles;
    BYTE before = reg;
    unsigned int toSubtract = 0;
    BYTE nonMod = subtracting;
    int res = 0;

    if (useImmediate)
    {
        BYTE n = m_ContextZ80.m_FuncPtrRead(m_ContextZ80.m_ProgramCounter);
        m_ContextZ80.m_ProgramCounter++;
        toSubtract = n;
        nonMod = n;
    }
    else
    {
        toSubtract = subtracting;
    }

    if (subCarry)
    {
        if (TestBit(m_ContextZ80.m_RegisterAF.lo, FLAG_C))
            toSubtract++;
    }

    res = reg - toSubtract;
    reg -= toSubtract;

    m_ContextZ80.m_RegisterAF.lo = 0;

    // correct
    if (reg == 0)
        m_ContextZ80.m_RegisterAF.lo = BitSet(m_ContextZ80.m_RegisterAF.lo, FLAG_Z);

    // correct
    m_ContextZ80.m_RegisterAF.lo = BitSet(m_ContextZ80.m_RegisterAF.lo, FLAG_N);

    // correct
    if (before < toSubtract)
        m_ContextZ80.m_RegisterAF.lo = BitSet(m_ContextZ80.m_RegisterAF.lo, FLAG_C);

    SIGNED_WORD htest = (before & 0xF);
    htest -= (toSubtract & 0xF);

    // correct
    if((before ^ res ^ nonMod) & 0x10)
        m_ContextZ80.m_RegisterAF.lo = BitSet(m_ContextZ80.m_RegisterAF.lo, FLAG_H);

    // correct
    if (TestBit(reg,7))
        m_ContextZ80.m_RegisterAF.lo = BitSet(m_ContextZ80.m_RegisterAF.lo, FLAG_S);

    //  if (TestBit(reg,5))
    //      m_ContextZ80.m_RegisterAF.lo = BitSet(m_ContextZ80.m_RegisterAF.lo, FLAG_B5);
    //
    //  if (TestBit(reg,3))
    //      m_ContextZ80.m_RegisterAF.lo = BitSet(m_ContextZ80.m_RegisterAF.lo, FLAG_B3);

    // v is calculated not p

    if((nonMod ^ before) & (before ^ res) & 0x80)
        m_ContextZ80.m_RegisterAF.lo = BitSet(m_ContextZ80.m_RegisterAF.lo, FLAG_PV);

}


///////////////////////////////////////////////////////////////////////

void Z80::CPU_8BIT_AND(BYTE& reg, BYTE toAnd, int cycles, bool useImmediate)
{
    m_ContextZ80.m_OpcodeCycle=cycles;
    BYTE myand = 0;

    if (useImmediate)
    {
        BYTE n = m_ContextZ80.m_FuncPtrRead(m_ContextZ80.m_ProgramCounter);
        m_ContextZ80.m_ProgramCounter++;
        myand = n;
    }
    else
    {
        myand = toAnd;
    }

    reg &= myand;

    m_ContextZ80.m_RegisterAF.lo = 0;

    // correct
    if (reg == 0)
        m_ContextZ80.m_RegisterAF.lo = BitSet(m_ContextZ80.m_RegisterAF.lo, FLAG_Z);

    // correct
    m_ContextZ80.m_RegisterAF.lo = BitSet(m_ContextZ80.m_RegisterAF.lo, FLAG_H);

    // correct
    if (TestBit(reg,7))
        m_ContextZ80.m_RegisterAF.lo = BitSet(m_ContextZ80.m_RegisterAF.lo, FLAG_S);

    //  if (TestBit(reg,5))
    //      m_ContextZ80.m_RegisterAF.lo = BitSet(m_ContextZ80.m_RegisterAF.lo, FLAG_B5);
    //
    //  if (TestBit(reg,3))
    //      m_ContextZ80.m_RegisterAF.lo = BitSet(m_ContextZ80.m_RegisterAF.lo, FLAG_B3);


    int pcount = 0;
    for (int i = 0; i < 8; i++)
    {
        if (TestBit(reg,i))
            pcount++;
    }

    // unsure
    if ((pcount == 0) || ((pcount % 2) == 0))
        m_ContextZ80.m_RegisterAF.lo = BitSet(m_ContextZ80.m_RegisterAF.lo, FLAG_PV);

}

//////////////////////////////////////////////////////////////////////////////////

void Z80::CPU_8BIT_OR(BYTE& reg, BYTE toOr, int cycles, bool useImmediate)
{
    m_ContextZ80.m_OpcodeCycle=cycles;
    BYTE myor = 0;

    if (useImmediate)
    {
        BYTE n = m_ContextZ80.m_FuncPtrRead(m_ContextZ80.m_ProgramCounter);
        m_ContextZ80.m_ProgramCounter++;
        myor = n;
    }
    else
    {
        myor = toOr;
    }

    reg |= myor;

    m_ContextZ80.m_RegisterAF.lo = 0;

    // correct
    if (reg == 0)
        m_ContextZ80.m_RegisterAF.lo = BitSet(m_ContextZ80.m_RegisterAF.lo, FLAG_Z);


    // correct
    if (TestBit(reg,7))
        m_ContextZ80.m_RegisterAF.lo = BitSet(m_ContextZ80.m_RegisterAF.lo, FLAG_S);

    //  if (TestBit(reg,5))
    //      m_ContextZ80.m_RegisterAF.lo = BitSet(m_ContextZ80.m_RegisterAF.lo, FLAG_B5);
    //
    //  if (TestBit(reg,3))
    //      m_ContextZ80.m_RegisterAF.lo = BitSet(m_ContextZ80.m_RegisterAF.lo, FLAG_B3);


    int pcount = 0;
    for (int i = 0; i < 8; i++)
    {
        if (TestBit(reg,i))
            pcount++;
    }

    // unsure
    if ((pcount == 0) || ((pcount % 2) == 0))
        m_ContextZ80.m_RegisterAF.lo = BitSet(m_ContextZ80.m_RegisterAF.lo, FLAG_PV);

}

//////////////////////////////////////////////////////////////////////////////////

void Z80::CPU_8BIT_XOR(BYTE& reg, BYTE toXOr, int cycles, bool useImmediate)
{
    m_ContextZ80.m_OpcodeCycle=cycles;
    BYTE myxor = 0;

    if (useImmediate)
    {
        BYTE n = m_ContextZ80.m_FuncPtrRead(m_ContextZ80.m_ProgramCounter);
        m_ContextZ80.m_ProgramCounter++;
        myxor = n;
    }
    else
    {
        myxor = toXOr;
    }

    reg ^= myxor;

    m_ContextZ80.m_RegisterAF.lo = 0;

    // correct
    if (reg == 0)
        m_ContextZ80.m_RegisterAF.lo = BitSet(m_ContextZ80.m_RegisterAF.lo, FLAG_Z);

    // correct
    if (TestBit(reg,7))
        m_ContextZ80.m_RegisterAF.lo = BitSet(m_ContextZ80.m_RegisterAF.lo, FLAG_S);

    //  if (TestBit(reg,5))
    //      m_ContextZ80.m_RegisterAF.lo = BitSet(m_ContextZ80.m_RegisterAF.lo, FLAG_B5);
    //
    //  if (TestBit(reg,3))
    //      m_ContextZ80.m_RegisterAF.lo = BitSet(m_ContextZ80.m_RegisterAF.lo, FLAG_B3);


    int pcount = 0;
    for (int i = 0; i < 8; i++)
    {
        if (TestBit(reg,i))
            pcount++;
    }

    // unsure
    if ((pcount == 0) || ((pcount % 2) == 0))
        m_ContextZ80.m_RegisterAF.lo = BitSet(m_ContextZ80.m_RegisterAF.lo, FLAG_PV);

}

//////////////////////////////////////////////////////////////////////////////////

// this does not affect any registers, hence why im not passing a reference

void Z80::CPU_8BIT_COMPARE(BYTE reg, BYTE subtracting, int cycles, bool useImmediate)
{
    // the CPI function uses this function and the CPI function is correct according to zexall.
    // if there are any problems with this function it must be to do with the flags that CPI sets itself like
    // the carry flag and the PV flag.

    m_ContextZ80.m_OpcodeCycle= cycles;
    BYTE before = reg;
    BYTE toSubtract = 0;
    BYTE nonMod = subtracting;
    int res = 0;

    if (useImmediate)
    {
        BYTE n = m_ContextZ80.m_FuncPtrRead(m_ContextZ80.m_ProgramCounter);
        m_ContextZ80.m_ProgramCounter++;
        toSubtract = n;
        nonMod = n;
    }
    else
    {
        toSubtract = subtracting;
    }

    res = reg - toSubtract;
    reg -= toSubtract;

    m_ContextZ80.m_RegisterAF.lo = 0;

    // correct
    if (reg == 0)
        m_ContextZ80.m_RegisterAF.lo = BitSet(m_ContextZ80.m_RegisterAF.lo, FLAG_Z);

    // correct
    m_ContextZ80.m_RegisterAF.lo = BitSet(m_ContextZ80.m_RegisterAF.lo, FLAG_N);

    // correct
    if (before < toSubtract)
    {
        m_ContextZ80.m_RegisterAF.lo = BitSet(m_ContextZ80.m_RegisterAF.lo, FLAG_C);
    }


    SIGNED_WORD htest = (before & 0xF);
    htest -= (toSubtract & 0xF);

    // correct
    if ((before ^ res ^ nonMod) & 0x10)
        m_ContextZ80.m_RegisterAF.lo = BitSet(m_ContextZ80.m_RegisterAF.lo, FLAG_H);

    // correct
    if (TestBit(reg,7))
        m_ContextZ80.m_RegisterAF.lo = BitSet(m_ContextZ80.m_RegisterAF.lo, FLAG_S);

    //  if (TestBit(reg,5))
    //      m_ContextZ80.m_RegisterAF.lo = BitSet(m_ContextZ80.m_RegisterAF.lo, FLAG_B5);
    //
    //  if (TestBit(reg,3))
    //      m_ContextZ80.m_RegisterAF.lo = BitSet(m_ContextZ80.m_RegisterAF.lo, FLAG_B3);

    // v is calculated not p

    if((nonMod ^ before) & (before ^ res) & 0x80)
        m_ContextZ80.m_RegisterAF.lo = BitSet(m_ContextZ80.m_RegisterAF.lo, FLAG_PV);
}

//////////////////////////////////////////////////////////////////////////////////

void Z80::CPU_8BIT_INC(BYTE& reg, int cycles)
{
    // WHEN EDITING THIS FUNCTION DONT FORGET TO MAKE THE SAME CHANGES TO CPU_8BIT_MEMORY_INC

    m_ContextZ80.m_OpcodeCycle= cycles;

    BYTE before = reg;

    reg++;

    if (reg == 0)
        m_ContextZ80.m_RegisterAF.lo = BitSet(m_ContextZ80.m_RegisterAF.lo, FLAG_Z);
    else
        m_ContextZ80.m_RegisterAF.lo = BitReset(m_ContextZ80.m_RegisterAF.lo, FLAG_Z);

    m_ContextZ80.m_RegisterAF.lo = BitReset(m_ContextZ80.m_RegisterAF.lo, FLAG_N);

    if ((before & 0xF) == 0xF)
        m_ContextZ80.m_RegisterAF.lo = BitSet(m_ContextZ80.m_RegisterAF.lo, FLAG_H);
    else
        m_ContextZ80.m_RegisterAF.lo = BitReset(m_ContextZ80.m_RegisterAF.lo, FLAG_H);

    if (TestBit(reg,7))
        m_ContextZ80.m_RegisterAF.lo = BitSet(m_ContextZ80.m_RegisterAF.lo, FLAG_S);
    else
        m_ContextZ80.m_RegisterAF.lo = BitReset(m_ContextZ80.m_RegisterAF.lo, FLAG_S);

    //  if (TestBit(reg,5))
    //      m_ContextZ80.m_RegisterAF.lo = BitSet(m_ContextZ80.m_RegisterAF.lo, FLAG_B5);
    //  else
    //      m_ContextZ80.m_RegisterAF.lo = BitReset(m_ContextZ80.m_RegisterAF.lo, FLAG_B5);
    //
    //  if (TestBit(reg,3))
    //      m_ContextZ80.m_RegisterAF.lo = BitSet(m_ContextZ80.m_RegisterAF.lo, FLAG_B3);
    //  else
    //      m_ContextZ80.m_RegisterAF.lo = BitReset(m_ContextZ80.m_RegisterAF.lo, FLAG_B3);

    // v is calculated not p
    if (before == 127)
        m_ContextZ80.m_RegisterAF.lo = BitSet(m_ContextZ80.m_RegisterAF.lo, FLAG_PV);
    else
        m_ContextZ80.m_RegisterAF.lo = BitReset(m_ContextZ80.m_RegisterAF.lo, FLAG_PV);

}

//////////////////////////////////////////////////////////////////////////////////

void Z80::CPU_8BIT_MEMORY_INC(WORD address, int cycles)
{
    // WHEN EDITING THIS FUNCTION DONT FORGET TO MAKE THE SAME CHANGES TO CPU_8BIT_INC

    m_ContextZ80.m_OpcodeCycle= cycles;

    BYTE before = m_ContextZ80.m_FuncPtrRead(address);
    m_ContextZ80.m_FuncPtrWrite(address, (before+1));
    BYTE now =  before+1;

    if (now == 0)
        m_ContextZ80.m_RegisterAF.lo = BitSet(m_ContextZ80.m_RegisterAF.lo, FLAG_Z);
    else
        m_ContextZ80.m_RegisterAF.lo = BitReset(m_ContextZ80.m_RegisterAF.lo, FLAG_Z);

    m_ContextZ80.m_RegisterAF.lo = BitReset(m_ContextZ80.m_RegisterAF.lo, FLAG_N);

    if ((before & 0xF) == 0xF)
        m_ContextZ80.m_RegisterAF.lo = BitSet(m_ContextZ80.m_RegisterAF.lo, FLAG_H);
    else
        m_ContextZ80.m_RegisterAF.lo = BitReset(m_ContextZ80.m_RegisterAF.lo, FLAG_H);

    if (TestBit(now,7))
        m_ContextZ80.m_RegisterAF.lo = BitSet(m_ContextZ80.m_RegisterAF.lo, FLAG_S);
    else
        m_ContextZ80.m_RegisterAF.lo = BitReset(m_ContextZ80.m_RegisterAF.lo, FLAG_S);

    //  if (TestBit(now,5))
    //      m_ContextZ80.m_RegisterAF.lo = BitSet(m_ContextZ80.m_RegisterAF.lo, FLAG_B5);
    //  else
    //      m_ContextZ80.m_RegisterAF.lo = BitReset(m_ContextZ80.m_RegisterAF.lo, FLAG_B5);
    //
    //  if (TestBit(now,3))
    //      m_ContextZ80.m_RegisterAF.lo = BitSet(m_ContextZ80.m_RegisterAF.lo, FLAG_B3);
    //  else
    //      m_ContextZ80.m_RegisterAF.lo = BitReset(m_ContextZ80.m_RegisterAF.lo, FLAG_B3);

    // v is calculated not p
    if (before == 127)
        m_ContextZ80.m_RegisterAF.lo = BitSet(m_ContextZ80.m_RegisterAF.lo, FLAG_PV);
    else
        m_ContextZ80.m_RegisterAF.lo = BitReset(m_ContextZ80.m_RegisterAF.lo, FLAG_PV);


}

//////////////////////////////////////////////////////////////////////////////////

void Z80::CPU_8BIT_DEC(BYTE& reg, int cycles)
{
    // WHEN EDITING THIS FUNCTION DONT FORGET TO MAKE THE SAME CHANGES TO CPU_8BIT_MEMORY_DEC

    m_ContextZ80.m_OpcodeCycle= cycles;

    BYTE before = reg;

    reg--;

    if (reg == 0)
        m_ContextZ80.m_RegisterAF.lo = BitSet(m_ContextZ80.m_RegisterAF.lo, FLAG_Z);
    else
        m_ContextZ80.m_RegisterAF.lo = BitReset(m_ContextZ80.m_RegisterAF.lo, FLAG_Z);

    if ((before & 0x0F) == 0)
        m_ContextZ80.m_RegisterAF.lo = BitSet(m_ContextZ80.m_RegisterAF.lo, FLAG_H);
    else
        m_ContextZ80.m_RegisterAF.lo = BitReset(m_ContextZ80.m_RegisterAF.lo, FLAG_H);

    SIGNED_BYTE vtest = before;

    // v is calculated not p
    if (vtest == -128)
        m_ContextZ80.m_RegisterAF.lo = BitSet(m_ContextZ80.m_RegisterAF.lo, FLAG_PV);
    else
        m_ContextZ80.m_RegisterAF.lo = BitReset(m_ContextZ80.m_RegisterAF.lo, FLAG_PV);

    m_ContextZ80.m_RegisterAF.lo = BitSet(m_ContextZ80.m_RegisterAF.lo, FLAG_N);


    if (TestBit(reg,7))
        m_ContextZ80.m_RegisterAF.lo = BitSet(m_ContextZ80.m_RegisterAF.lo, FLAG_S);
    else
        m_ContextZ80.m_RegisterAF.lo = BitReset(m_ContextZ80.m_RegisterAF.lo, FLAG_S);
    //
    //  if (TestBit(reg,5))
    //      m_ContextZ80.m_RegisterAF.lo = BitSet(m_ContextZ80.m_RegisterAF.lo, FLAG_B5);
    //  else
    //      m_ContextZ80.m_RegisterAF.lo = BitReset(m_ContextZ80.m_RegisterAF.lo, FLAG_B5);
    //
    //  if (TestBit(reg,3))
    //      m_ContextZ80.m_RegisterAF.lo = BitSet(m_ContextZ80.m_RegisterAF.lo, FLAG_B3);
    //  else
    //      m_ContextZ80.m_RegisterAF.lo = BitReset(m_ContextZ80.m_RegisterAF.lo, FLAG_B3);



}

//////////////////////////////////////////////////////////////////////////////////

void Z80::CPU_8BIT_MEMORY_DEC(WORD address, int cycles)
{
    // WHEN EDITING THIS FUNCTION DONT FORGET TO MAKE THE SAME CHANGES TO CPU_8BIT_DEC

    m_ContextZ80.m_OpcodeCycle= cycles;
    BYTE before = m_ContextZ80.m_FuncPtrRead(address);
    m_ContextZ80.m_FuncPtrWrite(address, (before-1));
    BYTE now = before-1;

    if (now == 0)
        m_ContextZ80.m_RegisterAF.lo = BitSet(m_ContextZ80.m_RegisterAF.lo, FLAG_Z);
    else
        m_ContextZ80.m_RegisterAF.lo = BitReset(m_ContextZ80.m_RegisterAF.lo, FLAG_Z);

    m_ContextZ80.m_RegisterAF.lo = BitSet(m_ContextZ80.m_RegisterAF.lo, FLAG_N);

    if ((before & 0x0F) == 0)
        m_ContextZ80.m_RegisterAF.lo = BitSet(m_ContextZ80.m_RegisterAF.lo, FLAG_H);
    else
        m_ContextZ80.m_RegisterAF.lo = BitReset(m_ContextZ80.m_RegisterAF.lo, FLAG_H);

    if (TestBit(now,7))
        m_ContextZ80.m_RegisterAF.lo = BitSet(m_ContextZ80.m_RegisterAF.lo, FLAG_S);
    else
        m_ContextZ80.m_RegisterAF.lo = BitReset(m_ContextZ80.m_RegisterAF.lo, FLAG_S);

    //  if (TestBit(now,5))
    //      m_ContextZ80.m_RegisterAF.lo = BitSet(m_ContextZ80.m_RegisterAF.lo, FLAG_B5);
    //  else
    //      m_ContextZ80.m_RegisterAF.lo = BitReset(m_ContextZ80.m_RegisterAF.lo, FLAG_B5);
    //
    //  if (TestBit(now,3))
    //      m_ContextZ80.m_RegisterAF.lo = BitSet(m_ContextZ80.m_RegisterAF.lo, FLAG_B3);
    //  else
    //      m_ContextZ80.m_RegisterAF.lo = BitReset(m_ContextZ80.m_RegisterAF.lo, FLAG_B3);


    SIGNED_BYTE vtest = before;

    // v is calculated not p
    if (vtest == -128)
        m_ContextZ80.m_RegisterAF.lo = BitSet(m_ContextZ80.m_RegisterAF.lo, FLAG_PV);
    else
        m_ContextZ80.m_RegisterAF.lo = BitReset(m_ContextZ80.m_RegisterAF.lo, FLAG_PV);
}

//////////////////////////////////////////////////////////////////////////////////

void Z80::CPU_16BIT_ADD(WORD& reg, WORD myAdd, int cycles, bool addCarry)
{
    m_ContextZ80.m_OpcodeCycle= cycles;
    WORD before = reg;
    unsigned int toAdd = myAdd;

    if (addCarry)
    {
        if (TestBit(m_ContextZ80.m_RegisterAF.lo, FLAG_C))
            toAdd++;
    }

    unsigned long result = before + toAdd;

    // correct
    m_ContextZ80.m_RegisterAF.lo = BitReset(m_ContextZ80.m_RegisterAF.lo, FLAG_N); 

    // correct
    if (result & 0xFFFF0000)
        m_ContextZ80.m_RegisterAF.lo = BitSet(m_ContextZ80.m_RegisterAF.lo, FLAG_C);
    else
        m_ContextZ80.m_RegisterAF.lo = BitReset(m_ContextZ80.m_RegisterAF.lo, FLAG_C);


    if (((before & 0x0FFF) + (toAdd & 0x0FFF)) > 0x0FFF)
        m_ContextZ80.m_RegisterAF.lo = BitSet(m_ContextZ80.m_RegisterAF.lo, FLAG_H);
    else
        m_ContextZ80.m_RegisterAF.lo = BitReset(m_ContextZ80.m_RegisterAF.lo, FLAG_H);

    reg = result & 0xFFFF;


    // unsure above the following two flags... should it be 13 and 11 or 5 and 3?

    //  if (TestBit(reg,13))
    //      m_ContextZ80.m_RegisterAF.lo = BitSet(m_ContextZ80.m_RegisterAF.lo, FLAG_B5);
    //  else
    //      m_ContextZ80.m_RegisterAF.lo = BitReset(m_ContextZ80.m_RegisterAF.lo, FLAG_B5);
    //
    //  if (TestBit(reg,11))
    //      m_ContextZ80.m_RegisterAF.lo = BitSet(m_ContextZ80.m_RegisterAF.lo, FLAG_B3);
    //  else
    //      m_ContextZ80.m_RegisterAF.lo = BitReset(m_ContextZ80.m_RegisterAF.lo, FLAG_B3);

    if (addCarry)
    {
        // 0x8000 is the top bit of the 16-bit number. So for 8 bit it would be 0x80 (128)
        if(!((before ^ toAdd) & 0x8000) && ((before ^ reg) & 0x8000))
            m_ContextZ80.m_RegisterAF.lo = BitSet(m_ContextZ80.m_RegisterAF.lo, FLAG_PV);
        else
            m_ContextZ80.m_RegisterAF.lo = BitReset(m_ContextZ80.m_RegisterAF.lo, FLAG_PV);

        // correct
        if (reg == 0)
            m_ContextZ80.m_RegisterAF.lo = BitSet(m_ContextZ80.m_RegisterAF.lo, FLAG_Z);
        else
            m_ContextZ80.m_RegisterAF.lo = BitReset(m_ContextZ80.m_RegisterAF.lo, FLAG_Z);

        // correct
        if (TestBit(reg,15))
            m_ContextZ80.m_RegisterAF.lo = BitSet(m_ContextZ80.m_RegisterAF.lo, FLAG_S);
        else
            m_ContextZ80.m_RegisterAF.lo = BitReset(m_ContextZ80.m_RegisterAF.lo, FLAG_S);
    }


}

//////////////////////////////////////////////////////////////////////////////////

void Z80::CPU_16BIT_SUB(WORD& reg, WORD mySub, int cycles, bool subCarry)
{
    m_ContextZ80.m_OpcodeCycle= cycles;
    WORD before = reg;
    int c = TestBit(m_ContextZ80.m_RegisterAF.lo,FLAG_C)?1:0;
    int res = reg - mySub - c;

    m_ContextZ80.m_RegisterAF.lo = 0;

    if (res & 0x10000)
        m_ContextZ80.m_RegisterAF.lo = BitSet(m_ContextZ80.m_RegisterAF.lo, FLAG_C);

    if (((before ^ res ^ mySub) >> 8) & 0x10)
        m_ContextZ80.m_RegisterAF.lo = BitSet(m_ContextZ80.m_RegisterAF.lo, FLAG_H);

    m_ContextZ80.m_RegisterAF.lo = BitSet(m_ContextZ80.m_RegisterAF.lo, FLAG_N);

    if (!(res & 0xFFFF))
        m_ContextZ80.m_RegisterAF.lo = BitSet(m_ContextZ80.m_RegisterAF.lo, FLAG_Z);

    if ((mySub ^ before) & (before ^ res) & 0x8000)
        m_ContextZ80.m_RegisterAF.lo = BitSet(m_ContextZ80.m_RegisterAF.lo, FLAG_PV);

    reg = res & 0xFFFF;

    if (TestBit(reg,15))
        m_ContextZ80.m_RegisterAF.lo = BitSet(m_ContextZ80.m_RegisterAF.lo, FLAG_S);
}

//////////////////////////////////////////////////////////////////////////////////

void Z80::CPU_16BIT_INC(WORD& word, int cycles)
{
    m_ContextZ80.m_OpcodeCycle= cycles;
    word++;
}

//////////////////////////////////////////////////////////////////////////////////

void Z80::CPU_16BIT_DEC(WORD& word, int cycles)
{
    m_ContextZ80.m_OpcodeCycle= cycles;
    word--;
}

//////////////////////////////////////////////////////////////////////////////////

void Z80::CPU_JUMP(bool useCondition, int flag, bool condition)
{
    m_ContextZ80.m_OpcodeCycle= 10;        

    if (!useCondition)
    {
        WORD nn = ReadWord();
        m_ContextZ80.m_ProgramCounter += 2;
        m_ContextZ80.m_ProgramCounter = nn;
        return;
    }

    if (TestBit(m_ContextZ80.m_RegisterAF.lo, flag) == condition)
    {
        WORD nn = ReadWord();     
        m_ContextZ80.m_ProgramCounter = nn;
    }
    else
    {
        m_ContextZ80.m_ProgramCounter += 2;
    }
}

//////////////////////////////////////////////////////////////////////////////////

void Z80::CPU_JUMP_IMMEDIATE(bool useCondition, int flag, bool condition)
{
    m_ContextZ80.m_OpcodeCycle= 12;

    if (!useCondition)
    {
        SIGNED_BYTE n = (SIGNED_BYTE)m_ContextZ80.m_FuncPtrRead(m_ContextZ80.m_ProgramCounter);

        m_ContextZ80.m_ProgramCounter += n;
    }
    else if (TestBit(m_ContextZ80.m_RegisterAF.lo, flag) == condition)
    {
        SIGNED_BYTE n = (SIGNED_BYTE)m_ContextZ80.m_FuncPtrRead(m_ContextZ80.m_ProgramCounter);

        m_ContextZ80.m_ProgramCounter += n;
    }
    else
    {
        m_ContextZ80.m_OpcodeCycle= 7;
    }

    m_ContextZ80.m_ProgramCounter++;
}

//////////////////////////////////////////////////////////////////////////////////

void Z80::CPU_CALL(bool useCondition, int flag, bool condition)
{
    m_ContextZ80.m_OpcodeCycle= 17;


    if (!useCondition)
    {
        WORD nn = ReadWord();
        m_ContextZ80.m_ProgramCounter += 2;
        PushWordOntoStack(m_ContextZ80.m_ProgramCounter);
        m_ContextZ80.m_ProgramCounter = nn;
        return;
    }

    if (TestBit(m_ContextZ80.m_RegisterAF.lo, flag)==condition)
    {
        WORD nn = ReadWord();
        m_ContextZ80.m_ProgramCounter += 2;
        PushWordOntoStack(m_ContextZ80.m_ProgramCounter);
        m_ContextZ80.m_ProgramCounter = nn;
    }
    else
    {
        m_ContextZ80.m_OpcodeCycle= 10;
        m_ContextZ80.m_ProgramCounter += 2;
    }
}

//////////////////////////////////////////////////////////////////////////////////

void Z80::CPU_RETURN(bool useCondition, int flag, bool condition)
{
    m_ContextZ80.m_OpcodeCycle = 11;
    if (!useCondition)
    {
        m_ContextZ80.m_ProgramCounter = PopWordOffStack();
        return;
    }

    if (TestBit(m_ContextZ80.m_RegisterAF.lo, flag) == condition)
    {
        m_ContextZ80.m_ProgramCounter = PopWordOffStack();
    }
    else
    {
        m_ContextZ80.m_OpcodeCycle = 5;
    }
}

//////////////////////////////////////////////////////////////////////////////////

void Z80::CPU_RESTARTS(BYTE n)
{
    PushWordOntoStack(m_ContextZ80.m_ProgramCounter);
    m_ContextZ80.m_OpcodeCycle = 11;
    m_ContextZ80.m_ProgramCounter = n;
}

//////////////////////////////////////////////////////////////////////////////////

// rotate right through carry
void Z80::CPU_RR(BYTE& reg, bool isAReg)
{
    // WHEN EDITING THIS ALSO EDIT CPU_RR_MEMORY
    if (isAReg)
        m_ContextZ80.m_OpcodeCycle = 4;
    else
        m_ContextZ80.m_OpcodeCycle = 8;

    bool isCarrySet = TestBit(m_ContextZ80.m_RegisterAF.lo, FLAG_C);
    bool isLSBSet = TestBit(reg, 0);

    reg >>= 1;

    if (isLSBSet)
        m_ContextZ80.m_RegisterAF.lo = BitSet(m_ContextZ80.m_RegisterAF.lo, FLAG_C);
    else
        m_ContextZ80.m_RegisterAF.lo = BitReset(m_ContextZ80.m_RegisterAF.lo, FLAG_C);

    if (isCarrySet)
        reg = BitSet(reg, 7);

    // when passing the A reg through these flags remain unchanged
    if (!isAReg)
    {
        if (reg == 0)
            m_ContextZ80.m_RegisterAF.lo = BitSet(m_ContextZ80.m_RegisterAF.lo, FLAG_Z);
        else
            m_ContextZ80.m_RegisterAF.lo = BitReset(m_ContextZ80.m_RegisterAF.lo, FLAG_Z);

        if (TestBit(reg,7))
            m_ContextZ80.m_RegisterAF.lo = BitSet(m_ContextZ80.m_RegisterAF.lo, FLAG_S);
        else
            m_ContextZ80.m_RegisterAF.lo = BitReset(m_ContextZ80.m_RegisterAF.lo, FLAG_S);

        int pcount = 0;
        for (int i = 0; i < 8; i++)
        {
            if (TestBit(reg,i))
                pcount++;
        }

        if ((pcount == 0) || ((pcount % 2) == 0))
            m_ContextZ80.m_RegisterAF.lo = BitSet(m_ContextZ80.m_RegisterAF.lo, FLAG_PV);
        else
            m_ContextZ80.m_RegisterAF.lo = BitReset(m_ContextZ80.m_RegisterAF.lo, FLAG_PV);
    }


    m_ContextZ80.m_RegisterAF.lo = BitReset(m_ContextZ80.m_RegisterAF.lo, FLAG_H);
    m_ContextZ80.m_RegisterAF.lo = BitReset(m_ContextZ80.m_RegisterAF.lo, FLAG_N);

    //  if (TestBit(reg,5))
    //      m_ContextZ80.m_RegisterAF.lo = BitSet(m_ContextZ80.m_RegisterAF.lo, FLAG_B5);
    //  else
    //      m_ContextZ80.m_RegisterAF.lo = BitReset(m_ContextZ80.m_RegisterAF.lo, FLAG_B5);
    //
    //  if (TestBit(reg,3))
    //      m_ContextZ80.m_RegisterAF.lo = BitSet(m_ContextZ80.m_RegisterAF.lo, FLAG_B3);
    //  else
    //      m_ContextZ80.m_RegisterAF.lo = BitReset(m_ContextZ80.m_RegisterAF.lo, FLAG_B3);


}

//////////////////////////////////////////////////////////////////////////////////

// rotate right through carry
void Z80::CPU_RR_MEMORY(WORD address, bool isAReg)
{
    // WHEN EDITING THIS ALSO EDIT CPU_RR
    m_ContextZ80.m_OpcodeCycle = 15;

    BYTE reg = m_ContextZ80.m_FuncPtrRead(address);

    bool isCarrySet = TestBit(m_ContextZ80.m_RegisterAF.lo, FLAG_C);
    bool isLSBSet = TestBit(reg, 0);

    reg >>= 1;

    if (isLSBSet)
        m_ContextZ80.m_RegisterAF.lo = BitSet(m_ContextZ80.m_RegisterAF.lo, FLAG_C);
    else
        m_ContextZ80.m_RegisterAF.lo = BitReset(m_ContextZ80.m_RegisterAF.lo, FLAG_C);

    if (isCarrySet)
        reg = BitSet(reg, 7);

    // when passing the A reg through these flags remain unchanged
    if (!isAReg)
    {
        if (reg == 0)
            m_ContextZ80.m_RegisterAF.lo = BitSet(m_ContextZ80.m_RegisterAF.lo, FLAG_Z);
        else
            m_ContextZ80.m_RegisterAF.lo = BitReset(m_ContextZ80.m_RegisterAF.lo, FLAG_Z);

        if (TestBit(reg,7))
            m_ContextZ80.m_RegisterAF.lo = BitSet(m_ContextZ80.m_RegisterAF.lo, FLAG_S);
        else
            m_ContextZ80.m_RegisterAF.lo = BitReset(m_ContextZ80.m_RegisterAF.lo, FLAG_S);

        int pcount = 0;
        for (int i = 0; i < 8; i++)
        {
            if (TestBit(reg,i))
                pcount++;
        }

        if ((pcount == 0) || ((pcount % 2) == 0))
            m_ContextZ80.m_RegisterAF.lo = BitSet(m_ContextZ80.m_RegisterAF.lo, FLAG_PV);
        else
            m_ContextZ80.m_RegisterAF.lo = BitReset(m_ContextZ80.m_RegisterAF.lo, FLAG_PV);
    }


    m_ContextZ80.m_RegisterAF.lo = BitReset(m_ContextZ80.m_RegisterAF.lo, FLAG_H);
    m_ContextZ80.m_RegisterAF.lo = BitReset(m_ContextZ80.m_RegisterAF.lo, FLAG_N);

    //  if (TestBit(reg,5))
    //      m_ContextZ80.m_RegisterAF.lo = BitSet(m_ContextZ80.m_RegisterAF.lo, FLAG_B5);
    //  else
    //      m_ContextZ80.m_RegisterAF.lo = BitReset(m_ContextZ80.m_RegisterAF.lo, FLAG_B5);
    //
    //  if (TestBit(reg,3))
    //      m_ContextZ80.m_RegisterAF.lo = BitSet(m_ContextZ80.m_RegisterAF.lo, FLAG_B3);
    //  else
    //      m_ContextZ80.m_RegisterAF.lo = BitReset(m_ContextZ80.m_RegisterAF.lo, FLAG_B3);

    m_ContextZ80.m_FuncPtrWrite(address, reg);
}

//////////////////////////////////////////////////////////////////////////////////

// rotate left
void Z80::CPU_RLC(BYTE& reg, bool isAReg)
{
    //WHEN EDITING THIS FUNCTION ALSO EDIT CPU_RLC_MEMORY

    if (isAReg)
        m_ContextZ80.m_OpcodeCycle = 4;
    else
        m_ContextZ80.m_OpcodeCycle = 8;

    bool isMSBSet = TestBit(reg, 7);

    reg <<= 1;

    if (isMSBSet)
    {
        m_ContextZ80.m_RegisterAF.lo = BitSet(m_ContextZ80.m_RegisterAF.lo, FLAG_C);
        reg = BitSet(reg,0);
    }
    else
    {
        m_ContextZ80.m_RegisterAF.lo = BitReset(m_ContextZ80.m_RegisterAF.lo, FLAG_C);
    }

    if (!isAReg)
    {
        if (reg == 0)
            m_ContextZ80.m_RegisterAF.lo = BitSet(m_ContextZ80.m_RegisterAF.lo, FLAG_Z);
        else
            m_ContextZ80.m_RegisterAF.lo = BitReset(m_ContextZ80.m_RegisterAF.lo, FLAG_Z);

        if (TestBit(reg,7))
            m_ContextZ80.m_RegisterAF.lo = BitSet(m_ContextZ80.m_RegisterAF.lo, FLAG_S);
        else
            m_ContextZ80.m_RegisterAF.lo = BitReset(m_ContextZ80.m_RegisterAF.lo, FLAG_S);

        int pcount = 0;
        for (int i = 0; i < 8; i++)
        {
            if (TestBit(reg,i))
                pcount++;
        }

        if ((pcount == 0) || ((pcount % 2) == 0))
            m_ContextZ80.m_RegisterAF.lo = BitSet(m_ContextZ80.m_RegisterAF.lo, FLAG_PV);
        else
            m_ContextZ80.m_RegisterAF.lo = BitReset(m_ContextZ80.m_RegisterAF.lo, FLAG_PV);
    }

    m_ContextZ80.m_RegisterAF.lo = BitReset(m_ContextZ80.m_RegisterAF.lo, FLAG_H);
    m_ContextZ80.m_RegisterAF.lo = BitReset(m_ContextZ80.m_RegisterAF.lo, FLAG_N);

    //  if (TestBit(reg,5))
    //      m_ContextZ80.m_RegisterAF.lo = BitSet(m_ContextZ80.m_RegisterAF.lo, FLAG_B5);
    //  else
    //      m_ContextZ80.m_RegisterAF.lo = BitReset(m_ContextZ80.m_RegisterAF.lo, FLAG_B5);
    //
    //  if (TestBit(reg,3))
    //      m_ContextZ80.m_RegisterAF.lo = BitSet(m_ContextZ80.m_RegisterAF.lo, FLAG_B3);
    //  else
    //      m_ContextZ80.m_RegisterAF.lo = BitReset(m_ContextZ80.m_RegisterAF.lo, FLAG_B3);

}

//////////////////////////////////////////////////////////////////////////////////

// rotate left
void Z80::CPU_RLC_MEMORY(WORD address, bool isAReg)
{
    //WHEN EDITING THIS FUNCTION ALSO EDIT CPU_RLC

    m_ContextZ80.m_OpcodeCycle = 15;

    BYTE reg = m_ContextZ80.m_FuncPtrRead(address);

    bool isMSBSet = TestBit(reg, 7);

    reg <<= 1;

    if (isMSBSet)
    {
        m_ContextZ80.m_RegisterAF.lo = BitSet(m_ContextZ80.m_RegisterAF.lo, FLAG_C);
        reg = BitSet(reg,0);
    }
    else
    {
        m_ContextZ80.m_RegisterAF.lo = BitReset(m_ContextZ80.m_RegisterAF.lo, FLAG_C);
    }

    if (!isAReg)
    {
        if (reg == 0)
            m_ContextZ80.m_RegisterAF.lo = BitSet(m_ContextZ80.m_RegisterAF.lo, FLAG_Z);
        else
            m_ContextZ80.m_RegisterAF.lo = BitReset(m_ContextZ80.m_RegisterAF.lo, FLAG_Z);

        if (TestBit(reg,7))
            m_ContextZ80.m_RegisterAF.lo = BitSet(m_ContextZ80.m_RegisterAF.lo, FLAG_S);
        else
            m_ContextZ80.m_RegisterAF.lo = BitReset(m_ContextZ80.m_RegisterAF.lo, FLAG_S);

        int pcount = 0;
        for (int i = 0; i < 8; i++)
        {
            if (TestBit(reg,i))
                pcount++;
        }

        if ((pcount == 0) || ((pcount % 2) == 0))
            m_ContextZ80.m_RegisterAF.lo = BitSet(m_ContextZ80.m_RegisterAF.lo, FLAG_PV);
        else
            m_ContextZ80.m_RegisterAF.lo = BitReset(m_ContextZ80.m_RegisterAF.lo, FLAG_PV);
    }

    m_ContextZ80.m_RegisterAF.lo = BitReset(m_ContextZ80.m_RegisterAF.lo, FLAG_H);
    m_ContextZ80.m_RegisterAF.lo = BitReset(m_ContextZ80.m_RegisterAF.lo, FLAG_N);

    //  if (TestBit(reg,5))
    //      m_ContextZ80.m_RegisterAF.lo = BitSet(m_ContextZ80.m_RegisterAF.lo, FLAG_B5);
    //  else
    //      m_ContextZ80.m_RegisterAF.lo = BitReset(m_ContextZ80.m_RegisterAF.lo, FLAG_B5);
    //
    //  if (TestBit(reg,3))
    //      m_ContextZ80.m_RegisterAF.lo = BitSet(m_ContextZ80.m_RegisterAF.lo, FLAG_B3);
    //  else
    //      m_ContextZ80.m_RegisterAF.lo = BitReset(m_ContextZ80.m_RegisterAF.lo, FLAG_B3);

    m_ContextZ80.m_FuncPtrWrite(address, reg);

}

//////////////////////////////////////////////////////////////////////////////////

// rotate right
void Z80::CPU_RRC(BYTE& reg, bool isAReg)
{
    // WHEN EDITING THIS FUNCTION ALSO EDIT CPU_RRC_MEMORY
    if (isAReg)
        m_ContextZ80.m_OpcodeCycle = 4;
    else
        m_ContextZ80.m_OpcodeCycle = 8;

    bool isLSBSet = TestBit(reg, 0);

    reg >>= 1;

    if (isLSBSet)
    {
        m_ContextZ80.m_RegisterAF.lo = BitSet(m_ContextZ80.m_RegisterAF.lo, FLAG_C);
        reg = BitSet(reg,7);
    }
    else
    {
        m_ContextZ80.m_RegisterAF.lo = BitReset(m_ContextZ80.m_RegisterAF.lo, FLAG_C);
    }

    if (!isAReg)
    {
        if (reg == 0)
            m_ContextZ80.m_RegisterAF.lo = BitSet(m_ContextZ80.m_RegisterAF.lo, FLAG_Z);
        else
            m_ContextZ80.m_RegisterAF.lo = BitReset(m_ContextZ80.m_RegisterAF.lo, FLAG_Z);

        if (TestBit(reg,7))
            m_ContextZ80.m_RegisterAF.lo = BitSet(m_ContextZ80.m_RegisterAF.lo, FLAG_S);
        else
            m_ContextZ80.m_RegisterAF.lo = BitReset(m_ContextZ80.m_RegisterAF.lo, FLAG_S);

        int pcount = 0;
        for (int i = 0; i < 8; i++)
        {
            if (TestBit(reg,i))
                pcount++;
        }

        if ((pcount == 0) || ((pcount % 2) == 0))
            m_ContextZ80.m_RegisterAF.lo = BitSet(m_ContextZ80.m_RegisterAF.lo, FLAG_PV);
        else
            m_ContextZ80.m_RegisterAF.lo = BitReset(m_ContextZ80.m_RegisterAF.lo, FLAG_PV);
    }

    m_ContextZ80.m_RegisterAF.lo = BitReset(m_ContextZ80.m_RegisterAF.lo, FLAG_H);
    m_ContextZ80.m_RegisterAF.lo = BitReset(m_ContextZ80.m_RegisterAF.lo, FLAG_N);

    //  if (TestBit(reg,5))
    //      m_ContextZ80.m_RegisterAF.lo = BitSet(m_ContextZ80.m_RegisterAF.lo, FLAG_B5);
    //  else
    //      m_ContextZ80.m_RegisterAF.lo = BitReset(m_ContextZ80.m_RegisterAF.lo, FLAG_B5);
    //
    //  if (TestBit(reg,3))
    //      m_ContextZ80.m_RegisterAF.lo = BitSet(m_ContextZ80.m_RegisterAF.lo, FLAG_B3);
    //  else
    //      m_ContextZ80.m_RegisterAF.lo = BitReset(m_ContextZ80.m_RegisterAF.lo, FLAG_B3);
}

//////////////////////////////////////////////////////////////////////////////////

// rotate right
void Z80::CPU_RRC_MEMORY(WORD address, bool isAReg)
{
    // WHEN EDITING THIS FUNCTION ALSO EDIT CPU_RRC_MEMORY

    m_ContextZ80.m_OpcodeCycle = 15;

    BYTE reg = m_ContextZ80.m_FuncPtrRead(address);

    bool isLSBSet = TestBit(reg, 0);

    reg >>= 1;

    if (isLSBSet)
    {
        m_ContextZ80.m_RegisterAF.lo = BitSet(m_ContextZ80.m_RegisterAF.lo, FLAG_C);
        reg = BitSet(reg,7);
    }
    else
    {
        m_ContextZ80.m_RegisterAF.lo = BitReset(m_ContextZ80.m_RegisterAF.lo, FLAG_C);
    }

    if (!isAReg)
    {
        if (reg == 0)
            m_ContextZ80.m_RegisterAF.lo = BitSet(m_ContextZ80.m_RegisterAF.lo, FLAG_Z);
        else
            m_ContextZ80.m_RegisterAF.lo = BitReset(m_ContextZ80.m_RegisterAF.lo, FLAG_Z);

        if (TestBit(reg,7))
            m_ContextZ80.m_RegisterAF.lo = BitSet(m_ContextZ80.m_RegisterAF.lo, FLAG_S);
        else
            m_ContextZ80.m_RegisterAF.lo = BitReset(m_ContextZ80.m_RegisterAF.lo, FLAG_S);

        int pcount = 0;
        for (int i = 0; i < 8; i++)
        {
            if (TestBit(reg,i))
                pcount++;
        }

        if ((pcount == 0) || ((pcount % 2) == 0))
            m_ContextZ80.m_RegisterAF.lo = BitSet(m_ContextZ80.m_RegisterAF.lo, FLAG_PV);
        else
            m_ContextZ80.m_RegisterAF.lo = BitReset(m_ContextZ80.m_RegisterAF.lo, FLAG_PV);
    }

    m_ContextZ80.m_RegisterAF.lo = BitReset(m_ContextZ80.m_RegisterAF.lo, FLAG_H);
    m_ContextZ80.m_RegisterAF.lo = BitReset(m_ContextZ80.m_RegisterAF.lo, FLAG_N);

    //  if (TestBit(reg,5))
    //      m_ContextZ80.m_RegisterAF.lo = BitSet(m_ContextZ80.m_RegisterAF.lo, FLAG_B5);
    //  else
    //      m_ContextZ80.m_RegisterAF.lo = BitReset(m_ContextZ80.m_RegisterAF.lo, FLAG_B5);
    //
    //  if (TestBit(reg,3))
    //      m_ContextZ80.m_RegisterAF.lo = BitSet(m_ContextZ80.m_RegisterAF.lo, FLAG_B3);
    //  else
    //      m_ContextZ80.m_RegisterAF.lo = BitReset(m_ContextZ80.m_RegisterAF.lo, FLAG_B3);

    m_ContextZ80.m_FuncPtrWrite(address, reg);
}

//////////////////////////////////////////////////////////////////////////////////

void Z80::CPU_RL(BYTE& reg, bool isAReg)
{
    // WHEN EDITING THIS FUNCTION ALSO EDIT CPU_RL_MEMORY
    if (isAReg)
        m_ContextZ80.m_OpcodeCycle = 4;
    else
        m_ContextZ80.m_OpcodeCycle = 8;

    bool isCarrySet = TestBit(m_ContextZ80.m_RegisterAF.lo, FLAG_C);
    bool isMSBSet = TestBit(reg, 7);

    reg <<= 1;

    if (isMSBSet)
        m_ContextZ80.m_RegisterAF.lo = BitSet(m_ContextZ80.m_RegisterAF.lo, FLAG_C);
    else
        m_ContextZ80.m_RegisterAF.lo = BitReset(m_ContextZ80.m_RegisterAF.lo, FLAG_C);

    if (isCarrySet)
        reg = BitSet(reg, 0);

    if (!isAReg)
    {
        if (reg == 0)
            m_ContextZ80.m_RegisterAF.lo = BitSet(m_ContextZ80.m_RegisterAF.lo, FLAG_Z);
        else
            m_ContextZ80.m_RegisterAF.lo = BitReset(m_ContextZ80.m_RegisterAF.lo, FLAG_Z);

        if (TestBit(reg,7))
            m_ContextZ80.m_RegisterAF.lo = BitSet(m_ContextZ80.m_RegisterAF.lo, FLAG_S);
        else
            m_ContextZ80.m_RegisterAF.lo = BitReset(m_ContextZ80.m_RegisterAF.lo, FLAG_S);

        int pcount = 0;
        for (int i = 0; i < 8; i++)
        {
            if (TestBit(reg,i))
                pcount++;
        }

        if ((pcount == 0) || ((pcount % 2) == 0))
            m_ContextZ80.m_RegisterAF.lo = BitSet(m_ContextZ80.m_RegisterAF.lo, FLAG_PV);
        else
            m_ContextZ80.m_RegisterAF.lo = BitReset(m_ContextZ80.m_RegisterAF.lo, FLAG_PV);
    }

    m_ContextZ80.m_RegisterAF.lo = BitReset(m_ContextZ80.m_RegisterAF.lo, FLAG_H);
    m_ContextZ80.m_RegisterAF.lo = BitReset(m_ContextZ80.m_RegisterAF.lo, FLAG_N);

    //  if (TestBit(reg,5))
    //      m_ContextZ80.m_RegisterAF.lo = BitSet(m_ContextZ80.m_RegisterAF.lo, FLAG_B5);
    //  else
    //      m_ContextZ80.m_RegisterAF.lo = BitReset(m_ContextZ80.m_RegisterAF.lo, FLAG_B5);
    //
    //  if (TestBit(reg,3))
    //      m_ContextZ80.m_RegisterAF.lo = BitSet(m_ContextZ80.m_RegisterAF.lo, FLAG_B3);
    //  else
    //      m_ContextZ80.m_RegisterAF.lo = BitReset(m_ContextZ80.m_RegisterAF.lo, FLAG_B3);
}

//////////////////////////////////////////////////////////////////////////////////

// rotate left through carry flag
void Z80::CPU_RL_MEMORY(WORD address, bool isAReg)
{
    // WHEN EDITING THIS FUNCTION ALSO EDIT CPU_RL_MEMORY
    m_ContextZ80.m_OpcodeCycle = 15;

    BYTE reg = m_ContextZ80.m_FuncPtrRead(address);

    bool isCarrySet = TestBit(m_ContextZ80.m_RegisterAF.lo, FLAG_C);
    bool isMSBSet = TestBit(reg, 7);

    reg <<= 1;

    if (isMSBSet)
        m_ContextZ80.m_RegisterAF.lo = BitSet(m_ContextZ80.m_RegisterAF.lo, FLAG_C);
    else
        m_ContextZ80.m_RegisterAF.lo = BitReset(m_ContextZ80.m_RegisterAF.lo, FLAG_C);

    if (isCarrySet)
        reg = BitSet(reg, 0);

    if (!isAReg)
    {
        if (reg == 0)
            m_ContextZ80.m_RegisterAF.lo = BitSet(m_ContextZ80.m_RegisterAF.lo, FLAG_Z);
        else
            m_ContextZ80.m_RegisterAF.lo = BitReset(m_ContextZ80.m_RegisterAF.lo, FLAG_Z);

        if (TestBit(reg,7))
            m_ContextZ80.m_RegisterAF.lo = BitSet(m_ContextZ80.m_RegisterAF.lo, FLAG_S);
        else
            m_ContextZ80.m_RegisterAF.lo = BitReset(m_ContextZ80.m_RegisterAF.lo, FLAG_S);

        int pcount = 0;
        for (int i = 0; i < 8; i++)
        {
            if (TestBit(reg,i))
                pcount++;
        }

        if ((pcount == 0) || ((pcount % 2) == 0))
            m_ContextZ80.m_RegisterAF.lo = BitSet(m_ContextZ80.m_RegisterAF.lo, FLAG_PV);
        else
            m_ContextZ80.m_RegisterAF.lo = BitReset(m_ContextZ80.m_RegisterAF.lo, FLAG_PV);
    }

    m_ContextZ80.m_RegisterAF.lo = BitReset(m_ContextZ80.m_RegisterAF.lo, FLAG_H);
    m_ContextZ80.m_RegisterAF.lo = BitReset(m_ContextZ80.m_RegisterAF.lo, FLAG_N);

    //  if (TestBit(reg,5))
    //      m_ContextZ80.m_RegisterAF.lo = BitSet(m_ContextZ80.m_RegisterAF.lo, FLAG_B5);
    //  else
    //      m_ContextZ80.m_RegisterAF.lo = BitReset(m_ContextZ80.m_RegisterAF.lo, FLAG_B5);
    //
    //  if (TestBit(reg,3))
    //      m_ContextZ80.m_RegisterAF.lo = BitSet(m_ContextZ80.m_RegisterAF.lo, FLAG_B3);
    //  else
    //      m_ContextZ80.m_RegisterAF.lo = BitReset(m_ContextZ80.m_RegisterAF.lo, FLAG_B3);

    m_ContextZ80.m_FuncPtrWrite(address, reg);
}

//////////////////////////////////////////////////////////////////////////////////

// shift left arithmetically (basically bit 0 gets set to 0) (bit 7 goes into carry)
void Z80::CPU_SLA(BYTE& reg)
{
    // WHEN EDITING THIS ALSO EDIT CPU_SLA_MEMORY

    m_ContextZ80.m_OpcodeCycle = 8;

    bool isMSBSet = TestBit(reg, 7);

    reg <<= 1;

    m_ContextZ80.m_RegisterAF.lo = 0;

    if (isMSBSet)
        m_ContextZ80.m_RegisterAF.lo = BitSet(m_ContextZ80.m_RegisterAF.lo, FLAG_C);

    if (reg == 0)
        m_ContextZ80.m_RegisterAF.lo = BitSet(m_ContextZ80.m_RegisterAF.lo, FLAG_Z);

    if (TestBit(reg,7))
        m_ContextZ80.m_RegisterAF.lo = BitSet(m_ContextZ80.m_RegisterAF.lo, FLAG_S);

    //  if (TestBit(reg,5))
    //      m_ContextZ80.m_RegisterAF.lo = BitSet(m_ContextZ80.m_RegisterAF.lo, FLAG_B5);
    //
    //  if (TestBit(reg,3))
    //      m_ContextZ80.m_RegisterAF.lo = BitSet(m_ContextZ80.m_RegisterAF.lo, FLAG_B3);

    int pcount = 0;
    for (int i = 0; i < 8; i++)
    {
        if (TestBit(reg,i))
            pcount++;
    }

    if ((pcount == 0) || ((pcount % 2) == 0))
        m_ContextZ80.m_RegisterAF.lo = BitSet(m_ContextZ80.m_RegisterAF.lo, FLAG_PV);

}

//////////////////////////////////////////////////////////////////////////////////

void Z80::CPU_SLA_MEMORY(WORD address)
{
    // WHEN EDITING THIS ALSO EDIT CPU_SLA

    m_ContextZ80.m_OpcodeCycle = 15;

    BYTE reg = m_ContextZ80.m_FuncPtrRead(address);

    bool isMSBSet = TestBit(reg, 7);

    reg <<= 1;

    m_ContextZ80.m_RegisterAF.lo = 0;

    if (isMSBSet)
        m_ContextZ80.m_RegisterAF.lo = BitSet(m_ContextZ80.m_RegisterAF.lo, FLAG_C);

    if (reg == 0)
        m_ContextZ80.m_RegisterAF.lo = BitSet(m_ContextZ80.m_RegisterAF.lo, FLAG_Z);

    if (TestBit(reg,7))
        m_ContextZ80.m_RegisterAF.lo = BitSet(m_ContextZ80.m_RegisterAF.lo, FLAG_S);

    //  if (TestBit(reg,5))
    //      m_ContextZ80.m_RegisterAF.lo = BitSet(m_ContextZ80.m_RegisterAF.lo, FLAG_B5);
    //
    //  if (TestBit(reg,3))
    //      m_ContextZ80.m_RegisterAF.lo = BitSet(m_ContextZ80.m_RegisterAF.lo, FLAG_B3);

    int pcount = 0;
    for (int i = 0; i < 8; i++)
    {
        if (TestBit(reg,i))
            pcount++;
    }

    if ((pcount == 0) || ((pcount % 2) == 0))
        m_ContextZ80.m_RegisterAF.lo = BitSet(m_ContextZ80.m_RegisterAF.lo, FLAG_PV);

    m_ContextZ80.m_FuncPtrWrite(address,reg);
}
//////////////////////////////////////////////////////////////////////////////////

void Z80::CPU_SRA(BYTE& reg)
{
    // WHEN EDITING THIS FUNCTION ALSO EDIT CPU_SRA_MEMORY

    m_ContextZ80.m_OpcodeCycle = 8;

    bool isLSBSet = TestBit(reg,0);
    bool isMSBSet = TestBit(reg,7);

    m_ContextZ80.m_RegisterAF.lo = 0;

    reg >>= 1;

    if (isMSBSet)
        reg = BitSet(reg,7);
    if (isLSBSet)
        m_ContextZ80.m_RegisterAF.lo = BitSet(m_ContextZ80.m_RegisterAF.lo, FLAG_C);

    if (reg == 0)
        m_ContextZ80.m_RegisterAF.lo = BitSet(m_ContextZ80.m_RegisterAF.lo, FLAG_Z);

    if (TestBit(reg,7))
        m_ContextZ80.m_RegisterAF.lo = BitSet(m_ContextZ80.m_RegisterAF.lo, FLAG_S);

    //  if (TestBit(reg,5))
    //      m_ContextZ80.m_RegisterAF.lo = BitSet(m_ContextZ80.m_RegisterAF.lo, FLAG_B5);
    //
    //  if (TestBit(reg,3))
    //      m_ContextZ80.m_RegisterAF.lo = BitSet(m_ContextZ80.m_RegisterAF.lo, FLAG_B3);

    int pcount = 0;
    for (int i = 0; i < 8; i++)
    {
        if (TestBit(reg,i))
            pcount++;
    }

    if ((pcount == 0) || ((pcount % 2) == 0))
        m_ContextZ80.m_RegisterAF.lo = BitSet(m_ContextZ80.m_RegisterAF.lo, FLAG_PV);
}

//////////////////////////////////////////////////////////////////////////////////

// shift right. LSB into carry. bit 7 doesn't change
void Z80::CPU_SRA_MEMORY(WORD address)
{
    // WHEN EDITING THIS FUNCTION ALSO EDIT CPU_SRA

    m_ContextZ80.m_OpcodeCycle = 15;

    BYTE reg = m_ContextZ80.m_FuncPtrRead(address);

    bool isLSBSet = TestBit(reg,0);
    bool isMSBSet = TestBit(reg,7);

    m_ContextZ80.m_RegisterAF.lo = 0;

    reg >>= 1;

    if (isMSBSet)
        reg = BitSet(reg,7);
    if (isLSBSet)
        m_ContextZ80.m_RegisterAF.lo = BitSet(m_ContextZ80.m_RegisterAF.lo, FLAG_C);

    if (reg == 0)
        m_ContextZ80.m_RegisterAF.lo = BitSet(m_ContextZ80.m_RegisterAF.lo, FLAG_Z);

    if (TestBit(reg,7))
        m_ContextZ80.m_RegisterAF.lo = BitSet(m_ContextZ80.m_RegisterAF.lo, FLAG_S);

    //  if (TestBit(reg,5))
    //      m_ContextZ80.m_RegisterAF.lo = BitSet(m_ContextZ80.m_RegisterAF.lo, FLAG_B5);
    //
    //  if (TestBit(reg,3))
    //      m_ContextZ80.m_RegisterAF.lo = BitSet(m_ContextZ80.m_RegisterAF.lo, FLAG_B3);

    int pcount = 0;
    for (int i = 0; i < 8; i++)
    {
        if (TestBit(reg,i))
            pcount++;
    }

    if ((pcount == 0) || ((pcount % 2) == 0))
        m_ContextZ80.m_RegisterAF.lo = BitSet(m_ContextZ80.m_RegisterAF.lo, FLAG_PV);

    m_ContextZ80.m_FuncPtrWrite(address, reg);
}

//////////////////////////////////////////////////////////////////////////////////

void Z80::CPU_SRL(BYTE& reg)
{
    //WHEN EDITING THIS FUNCTION ALSO EDIT CPU_SRL_MEMORY

    m_ContextZ80.m_OpcodeCycle = 8;

    bool isLSBSet = TestBit(reg,0);

    m_ContextZ80.m_RegisterAF.lo = 0;

    reg >>= 1;

    if (isLSBSet)
        m_ContextZ80.m_RegisterAF.lo = BitSet(m_ContextZ80.m_RegisterAF.lo, FLAG_C);

    if (reg == 0)
        m_ContextZ80.m_RegisterAF.lo = BitSet(m_ContextZ80.m_RegisterAF.lo, FLAG_Z);

    if (TestBit(reg,7))
        m_ContextZ80.m_RegisterAF.lo = BitSet(m_ContextZ80.m_RegisterAF.lo, FLAG_S);

    //  if (TestBit(reg,5))
    //      m_ContextZ80.m_RegisterAF.lo = BitSet(m_ContextZ80.m_RegisterAF.lo, FLAG_B5);
    //
    //  if (TestBit(reg,3))
    //      m_ContextZ80.m_RegisterAF.lo = BitSet(m_ContextZ80.m_RegisterAF.lo, FLAG_B3);

    int pcount = 0;
    for (int i = 0; i < 8; i++)
    {
        if (TestBit(reg,i))
            pcount++;
    }

    if ((pcount == 0) || ((pcount % 2) == 0))
        m_ContextZ80.m_RegisterAF.lo = BitSet(m_ContextZ80.m_RegisterAF.lo, FLAG_PV);

}

//////////////////////////////////////////////////////////////////////////////////

// shift right. bit 0 into carry
void Z80::CPU_SRL_MEMORY(WORD address)
{
    //WHEN EDITING THIS FUNCTION ALSO EDIT CPU_SRL_MEMORY

    m_ContextZ80.m_OpcodeCycle = 15;
    BYTE reg = m_ContextZ80.m_FuncPtrRead(address);

    bool isLSBSet = TestBit(reg,0);

    m_ContextZ80.m_RegisterAF.lo = 0;

    reg >>= 1;

    if (isLSBSet)
        m_ContextZ80.m_RegisterAF.lo = BitSet(m_ContextZ80.m_RegisterAF.lo, FLAG_C);

    if (reg == 0)
        m_ContextZ80.m_RegisterAF.lo = BitSet(m_ContextZ80.m_RegisterAF.lo, FLAG_Z);

    if (TestBit(reg,7))
        m_ContextZ80.m_RegisterAF.lo = BitSet(m_ContextZ80.m_RegisterAF.lo, FLAG_S);

    //  if (TestBit(reg,5))
    //      m_ContextZ80.m_RegisterAF.lo = BitSet(m_ContextZ80.m_RegisterAF.lo, FLAG_B5);
    //
    //  if (TestBit(reg,3))
    //      m_ContextZ80.m_RegisterAF.lo = BitSet(m_ContextZ80.m_RegisterAF.lo, FLAG_B3);

    int pcount = 0;
    for (int i = 0; i < 8; i++)
    {
        if (TestBit(reg,i))
            pcount++;
    }

    if ((pcount == 0) || ((pcount % 2) == 0))
        m_ContextZ80.m_RegisterAF.lo = BitSet(m_ContextZ80.m_RegisterAF.lo, FLAG_PV);

    m_ContextZ80.m_FuncPtrWrite(address,reg);

}

//////////////////////////////////////////////////////////////////////////////////

void Z80::CPU_SLL(BYTE& reg)
{
    //WHEN EDITING THIS FUNCTION ALSO EDIT CPU_SLL_MEMORY

    m_ContextZ80.m_OpcodeCycle = 8;

    bool isMSBSet = TestBit(reg,7);

    m_ContextZ80.m_RegisterAF.lo = 0;

    reg <<= 1;

    reg = BitSet(reg,0); // apparently lsb is 1

    if (isMSBSet)
        m_ContextZ80.m_RegisterAF.lo = BitSet(m_ContextZ80.m_RegisterAF.lo, FLAG_C);

    if (reg == 0)
        m_ContextZ80.m_RegisterAF.lo = BitSet(m_ContextZ80.m_RegisterAF.lo, FLAG_Z);

    if (TestBit(reg,7))
        m_ContextZ80.m_RegisterAF.lo = BitSet(m_ContextZ80.m_RegisterAF.lo, FLAG_S);

    //  if (TestBit(reg,5))
    //      m_ContextZ80.m_RegisterAF.lo = BitSet(m_ContextZ80.m_RegisterAF.lo, FLAG_B5);
    //
    //  if (TestBit(reg,3))
    //      m_ContextZ80.m_RegisterAF.lo = BitSet(m_ContextZ80.m_RegisterAF.lo, FLAG_B3);

    int pcount = 0;
    for (int i = 0; i < 8; i++)
    {
        if (TestBit(reg,i))
            pcount++;
    }

    if ((pcount == 0) || ((pcount % 2) == 0))
        m_ContextZ80.m_RegisterAF.lo = BitSet(m_ContextZ80.m_RegisterAF.lo, FLAG_PV);

}

////////////////////////////////////////////////////////////////////////

void Z80::CPU_SLL_MEMORY(WORD address)
{
    //WHEN EDITING THIS FUNCTION ALSO EDIT CPU_SLL

    m_ContextZ80.m_OpcodeCycle = 15;
    BYTE reg = m_ContextZ80.m_FuncPtrRead(address);


    bool isMSBSet = TestBit(reg,7);

    m_ContextZ80.m_RegisterAF.lo = 0;

    reg <<= 1;

    reg = BitSet(reg,0); // apparently lsb is 1

    if (isMSBSet)
        m_ContextZ80.m_RegisterAF.lo = BitSet(m_ContextZ80.m_RegisterAF.lo, FLAG_C);

    if (reg == 0)
        m_ContextZ80.m_RegisterAF.lo = BitSet(m_ContextZ80.m_RegisterAF.lo, FLAG_Z);

    if (TestBit(reg,7))
        m_ContextZ80.m_RegisterAF.lo = BitSet(m_ContextZ80.m_RegisterAF.lo, FLAG_S);

    //  if (TestBit(reg,5))
    //      m_ContextZ80.m_RegisterAF.lo = BitSet(m_ContextZ80.m_RegisterAF.lo, FLAG_B5);
    //
    //  if (TestBit(reg,3))
    //      m_ContextZ80.m_RegisterAF.lo = BitSet(m_ContextZ80.m_RegisterAF.lo, FLAG_B3);

    int pcount = 0;
    for (int i = 0; i < 8; i++)
    {
        if (TestBit(reg,i))
            pcount++;
    }

    if ((pcount == 0) || ((pcount % 2) == 0))
        m_ContextZ80.m_RegisterAF.lo = BitSet(m_ContextZ80.m_RegisterAF.lo, FLAG_PV);

    m_ContextZ80.m_FuncPtrWrite(address,reg);
}

//////////////////////////////////////////////////////////////////////////////////////

void Z80::CPU_TEST_BIT(BYTE reg, int bit, int cycles)
{
    bool isSet = false;
    if (TestBit(reg, bit))
    {
        m_ContextZ80.m_RegisterAF.lo = BitReset(m_ContextZ80.m_RegisterAF.lo, FLAG_Z);
        m_ContextZ80.m_RegisterAF.lo = BitReset(m_ContextZ80.m_RegisterAF.lo, FLAG_PV);
        isSet = true;
    }
    else
    {
        m_ContextZ80.m_RegisterAF.lo = BitSet(m_ContextZ80.m_RegisterAF.lo, FLAG_Z);
        m_ContextZ80.m_RegisterAF.lo = BitSet(m_ContextZ80.m_RegisterAF.lo, FLAG_PV);
    }

    m_ContextZ80.m_RegisterAF.lo = BitReset(m_ContextZ80.m_RegisterAF.lo, FLAG_N);
    m_ContextZ80.m_RegisterAF.lo = BitSet(m_ContextZ80.m_RegisterAF.lo, FLAG_H);

    if ((bit == 7) && isSet)
        m_ContextZ80.m_RegisterAF.lo = BitSet(m_ContextZ80.m_RegisterAF.lo, FLAG_S);
    else
        m_ContextZ80.m_RegisterAF.lo = BitReset(m_ContextZ80.m_RegisterAF.lo, FLAG_S);

    //  if((bit == 5) && isSet)
    //      m_ContextZ80.m_RegisterAF.lo = BitSet(m_ContextZ80.m_RegisterAF.lo, FLAG_B5);
    //  else
    //      m_ContextZ80.m_RegisterAF.lo = BitReset(m_ContextZ80.m_RegisterAF.lo, FLAG_B5);
    //
    //  if((bit == 3) && isSet)
    //      m_ContextZ80.m_RegisterAF.lo = BitSet(m_ContextZ80.m_RegisterAF.lo, FLAG_B3);
    //  else
    //      m_ContextZ80.m_RegisterAF.lo = BitReset(m_ContextZ80.m_RegisterAF.lo, FLAG_B3);

    m_ContextZ80.m_OpcodeCycle = cycles;
}

//////////////////////////////////////////////////////////////////////////////////

void Z80::CPU_SET_BIT(BYTE& reg, int bit)
{
    // WHEN EDITING THIS ALSO EDIT CPU_SET_BIT_MEMORY
    reg = BitSet(reg, bit);
    m_ContextZ80.m_OpcodeCycle = 8;
}

//////////////////////////////////////////////////////////////////////////////////

void Z80::CPU_SET_BIT_MEMORY(WORD address, int bit)
{
    // WHEN EDITING THIS ALSO EDIT CPU_SET_BIT
    BYTE mem = m_ContextZ80.m_FuncPtrRead(address);
    mem = BitSet(mem, bit);
    m_ContextZ80.m_FuncPtrWrite(address, mem);
    m_ContextZ80.m_OpcodeCycle = 15;
}

//////////////////////////////////////////////////////////////////////////////////

void Z80::CPU_RESET_BIT(BYTE& reg, int bit)
{
    // WHEN EDITING THIS ALSO EDIT CPU_RESET_BIT_MEMORY
    reg = BitReset(reg, bit);
    m_ContextZ80.m_OpcodeCycle = 8;
}

//////////////////////////////////////////////////////////////////////////////////

void Z80::CPU_RESET_BIT_MEMORY(WORD address, int bit)
{
    // WHEN EDITING THIS ALSO EDIT CPU_RESET_BIT
    BYTE mem = m_ContextZ80.m_FuncPtrRead(address);
    mem = BitReset(mem, bit);
    m_ContextZ80.m_FuncPtrWrite(address, mem);
    m_ContextZ80.m_OpcodeCycle = 15;
}

//////////////////////////////////////////////////////////////////////////////////

void Z80::CPU_IN(BYTE& data)
{
    m_ContextZ80.m_OpcodeCycle = 12;
    data = m_ContextZ80.m_FuncPtrIORead(m_ContextZ80.m_RegisterBC.lo);

    if (TestBit(data,7))
        m_ContextZ80.m_RegisterAF.lo = BitSet(m_ContextZ80.m_RegisterAF.lo,FLAG_S);
    else
        m_ContextZ80.m_RegisterAF.lo = BitReset(m_ContextZ80.m_RegisterAF.lo,FLAG_S);

    if (data == 0)
        m_ContextZ80.m_RegisterAF.lo = BitSet(m_ContextZ80.m_RegisterAF.lo,FLAG_Z);
    else
        m_ContextZ80.m_RegisterAF.lo = BitReset(m_ContextZ80.m_RegisterAF.lo,FLAG_Z);

    //  if (TestBit(data,5))
    //      m_ContextZ80.m_RegisterAF.lo = BitSet(m_ContextZ80.m_RegisterAF.lo,FLAG_B5);
    //  else
    //      m_ContextZ80.m_RegisterAF.lo = BitReset(m_ContextZ80.m_RegisterAF.lo,FLAG_B5);
    //
    //  if (TestBit(data,3))
    //      m_ContextZ80.m_RegisterAF.lo = BitSet(m_ContextZ80.m_RegisterAF.lo,FLAG_B3);
    //  else
    //      m_ContextZ80.m_RegisterAF.lo = BitReset(m_ContextZ80.m_RegisterAF.lo,FLAG_B3);

    m_ContextZ80.m_RegisterAF.lo = BitReset(m_ContextZ80.m_RegisterAF.lo,FLAG_H);
    m_ContextZ80.m_RegisterAF.lo = BitReset(m_ContextZ80.m_RegisterAF.lo,FLAG_N);

    int pcount = 0;
    for (int i = 0; i < 8; i++)
    {
        if (TestBit(data,i))
            pcount++;
    }

    if ((pcount == 0) || ((pcount % 2) == 0))
        m_ContextZ80.m_RegisterAF.lo = BitSet(m_ContextZ80.m_RegisterAF.lo, FLAG_PV);
    else
        m_ContextZ80.m_RegisterAF.lo = BitReset(m_ContextZ80.m_RegisterAF.lo, FLAG_PV);

}

//////////////////////////////////////////////////////////////////////////////////

void Z80::CPU_OUT(const BYTE& address, const BYTE& data)
{
    m_ContextZ80.m_OpcodeCycle = 12;
    m_ContextZ80.m_FuncPtrIOWrite(address, data);
}

//////////////////////////////////////////////////////////////////////////////////

void Z80::CPU_IN_IMMEDIATE(BYTE& data)
{
    m_ContextZ80.m_OpcodeCycle = 11;
    BYTE n = m_ContextZ80.m_FuncPtrRead(m_ContextZ80.m_ProgramCounter);
    m_ContextZ80.m_ProgramCounter++;
    data = m_ContextZ80.m_FuncPtrIORead(n);
}

//////////////////////////////////////////////////////////////////////////////////

void Z80::CPU_OUT_IMMEDIATE(const BYTE& data)
{
    m_ContextZ80.m_OpcodeCycle = 11;
    BYTE n = m_ContextZ80.m_FuncPtrRead(m_ContextZ80.m_ProgramCounter);
    m_ContextZ80.m_ProgramCounter++;
    m_ContextZ80.m_FuncPtrIOWrite(n, data);
}

//////////////////////////////////////////////////////////////////////////////////

void Z80::CPU_OUTI()
{
    BYTE hldata = m_ContextZ80.m_FuncPtrRead(m_ContextZ80.m_RegisterHL.reg);
    m_ContextZ80.m_FuncPtrIOWrite(m_ContextZ80.m_RegisterBC.lo, hldata);

    // increment hl
    CPU_16BIT_INC(m_ContextZ80.m_RegisterHL.reg,0);

    // decrement b, and will set the appropriate flags
    CPU_8BIT_DEC(m_ContextZ80.m_RegisterBC.hi,0);


    m_ContextZ80.m_OpcodeCycle = 16;
}

//////////////////////////////////////////////////////////////////////////////////

void Z80::CPU_INI()
{
    m_ContextZ80.m_FuncPtrWrite(m_ContextZ80.m_RegisterHL.reg, m_ContextZ80.m_FuncPtrIORead(m_ContextZ80.m_RegisterBC.lo));

    // increment hl
    CPU_16BIT_INC(m_ContextZ80.m_RegisterHL.reg,0);

    // decrement b, and will set the appropriate flags
    CPU_8BIT_DEC(m_ContextZ80.m_RegisterBC.hi,0);


    m_ContextZ80.m_OpcodeCycle = 16;
}

//////////////////////////////////////////////////////////////////////////////////

void Z80::CPU_INIR()
{
    CPU_INI();
    // keep calling this function until b == 0
    if (m_ContextZ80.m_RegisterBC.hi != 0)
    {
        m_ContextZ80.m_OpcodeCycle = 21;
        m_ContextZ80.m_ProgramCounter-=2;
    }
    else
    {
        m_ContextZ80.m_OpcodeCycle = 16;
    }

}

//////////////////////////////////////////////////////////////////////////////////

void Z80::CPU_IND()
{
    m_ContextZ80.m_FuncPtrWrite(m_ContextZ80.m_RegisterHL.reg, m_ContextZ80.m_FuncPtrIORead(m_ContextZ80.m_RegisterBC.lo));

    // increment hl
    CPU_16BIT_DEC(m_ContextZ80.m_RegisterHL.reg,0);

    // decrement b, and will set the appropriate flags
    CPU_8BIT_DEC(m_ContextZ80.m_RegisterBC.hi,0);


    m_ContextZ80.m_OpcodeCycle = 16;
}

//////////////////////////////////////////////////////////////////////////////////

void Z80::CPU_INDR()
{
    CPU_IND();
    // keep calling this function until b == 0
    if (m_ContextZ80.m_RegisterBC.hi != 0)
    {
        m_ContextZ80.m_OpcodeCycle = 21;
        m_ContextZ80.m_ProgramCounter-=2;
    }
    else
    {
        m_ContextZ80.m_OpcodeCycle = 16;
    }
}

//////////////////////////////////////////////////////////////////////////////////

void Z80::CPU_OUTD()
{
    BYTE hldata = m_ContextZ80.m_FuncPtrRead(m_ContextZ80.m_RegisterHL.reg);
    m_ContextZ80.m_FuncPtrIOWrite(m_ContextZ80.m_RegisterBC.lo, hldata);

    // increment hl
    CPU_16BIT_DEC(m_ContextZ80.m_RegisterHL.reg,0);

    // decrement b, and will set the appropriate flags
    CPU_8BIT_DEC(m_ContextZ80.m_RegisterBC.hi,0);


    m_ContextZ80.m_OpcodeCycle = 16;
}

//////////////////////////////////////////////////////////////////////////////////

void Z80::CPU_OTDR()
{
    CPU_OUTD();
    // keep calling this function until b == 0
    if (m_ContextZ80.m_RegisterBC.hi != 0)
    {
        m_ContextZ80.m_ProgramCounter-=2;
        m_ContextZ80.m_OpcodeCycle = 21;
    }
    else
    {
        m_ContextZ80.m_OpcodeCycle = 16;
        m_ContextZ80.m_RegisterAF.lo = BitReset(m_ContextZ80.m_RegisterAF.lo, FLAG_S);
        m_ContextZ80.m_RegisterAF.lo = BitSet(m_ContextZ80.m_RegisterAF.lo, FLAG_Z);
    }
}

//////////////////////////////////////////////////////////////////////////////////


void Z80::CPU_OTIR()
{
    CPU_OUTI();
    // keep calling this function until b == 0
    if (m_ContextZ80.m_RegisterBC.hi != 0)
    {
        m_ContextZ80.m_ProgramCounter-=2;
        m_ContextZ80.m_OpcodeCycle = 21;
    }
    else
    {
        m_ContextZ80.m_RegisterAF.lo = BitReset(m_ContextZ80.m_RegisterAF.lo, FLAG_S);
        m_ContextZ80.m_RegisterAF.lo = BitSet(m_ContextZ80.m_RegisterAF.lo, FLAG_Z);
        m_ContextZ80.m_OpcodeCycle = 16;
        //   m_ContextZ80.m_RegisterAF.lo = BitReset(m_ContextZ80.m_RegisterAF.lo, FLAG_B5);
        //    m_ContextZ80.m_RegisterAF.lo = BitReset(m_ContextZ80.m_RegisterAF.lo, FLAG_B3);

    }
}

//////////////////////////////////////////////////////////////////////////////////

void Z80::CPU_DJNZ()
{
    m_ContextZ80.m_RegisterBC.hi--; // dont think this affects flags
    m_ContextZ80.m_OpcodeCycle = 8;

    if (m_ContextZ80.m_RegisterBC.hi!=0)
    {
        SIGNED_BYTE n = (SIGNED_BYTE)m_ContextZ80.m_FuncPtrRead(m_ContextZ80.m_ProgramCounter);
        m_ContextZ80.m_ProgramCounter += n;
        m_ContextZ80.m_OpcodeCycle = 13;
    }

    m_ContextZ80.m_ProgramCounter++;   
}

//////////////////////////////////////////////////////////////////////////////////

void Z80::CPU_LDI()
{
    BYTE hldata =  m_ContextZ80.m_FuncPtrRead(m_ContextZ80.m_RegisterHL.reg);
    m_ContextZ80.m_FuncPtrWrite(m_ContextZ80.m_RegisterDE.reg,hldata);
    CPU_16BIT_INC(m_ContextZ80.m_RegisterDE.reg,0);
    CPU_16BIT_INC(m_ContextZ80.m_RegisterHL.reg,0);
    CPU_16BIT_DEC(m_ContextZ80.m_RegisterBC.reg,0);

    m_ContextZ80.m_RegisterAF.lo = BitReset(m_ContextZ80.m_RegisterAF.lo, FLAG_N);
    m_ContextZ80.m_RegisterAF.lo = BitReset(m_ContextZ80.m_RegisterAF.lo, FLAG_H);

    if (m_ContextZ80.m_RegisterBC.reg == 0)
        m_ContextZ80.m_RegisterAF.lo = BitReset(m_ContextZ80.m_RegisterAF.lo, FLAG_PV);
    else
        m_ContextZ80.m_RegisterAF.lo = BitSet(m_ContextZ80.m_RegisterAF.lo, FLAG_PV);

    hldata+=m_ContextZ80.m_RegisterAF.hi;

    //      if (TestBit(hldata,1))
    //          m_ContextZ80.m_RegisterAF.lo = BitSet(m_ContextZ80.m_RegisterAF.lo, FLAG_B5);
    //      else
    //          m_ContextZ80.m_RegisterAF.lo = BitReset(m_ContextZ80.m_RegisterAF.lo, FLAG_B5);
    //
    //      if (TestBit(hldata,3))
    //          m_ContextZ80.m_RegisterAF.lo = BitSet(m_ContextZ80.m_RegisterAF.lo, FLAG_B3);
    //      else
    //          m_ContextZ80.m_RegisterAF.lo = BitReset(m_ContextZ80.m_RegisterAF.lo, FLAG_B3);

    m_ContextZ80.m_OpcodeCycle = 16;
}

//////////////////////////////////////////////////////////////////////////////////

void Z80::CPU_LDIR()
{
    CPU_LDI();
    // keep calling this function until bc == 0
    if (m_ContextZ80.m_RegisterBC.reg != 0)
    {
        m_ContextZ80.m_ProgramCounter-=2;
        m_ContextZ80.m_OpcodeCycle = 21;
    }
    else
    {
        m_ContextZ80.m_OpcodeCycle = 16;
        m_ContextZ80.m_RegisterAF.lo = BitReset(m_ContextZ80.m_RegisterAF.lo, FLAG_PV);
    }
}

//////////////////////////////////////////////////////////////////////////////////

void Z80::CPU_EXCHANGE(WORD& reg1, WORD& reg2)
{
    m_ContextZ80.m_OpcodeCycle = 4;
    WORD temp = reg1;
    reg1 = reg2;
    reg2 = temp;
}

//////////////////////////////////////////////////////////////////////////////////

void Z80::CPU_LOAD_NNN(WORD reg)
{
    WORD nn = ReadWord();
    m_ContextZ80.m_ProgramCounter+=2;
    m_ContextZ80.m_OpcodeCycle = 16;
    m_ContextZ80.m_FuncPtrWrite(nn, reg&0xFF);
    m_ContextZ80.m_FuncPtrWrite(nn+1, reg>>8);
}

//////////////////////////////////////////////////////////////////////////////////

void Z80::CPU_REG_LOAD_NNN(WORD& reg)
{
    WORD nn = ReadWord();
    m_ContextZ80.m_ProgramCounter+=2;
    m_ContextZ80.m_OpcodeCycle = 16;
    reg = m_ContextZ80.m_FuncPtrRead(nn+1) << 8;
    reg |= m_ContextZ80.m_FuncPtrRead(nn);
}

//////////////////////////////////////////////////////////////////////////////////

void Z80::CPU_LDD()
{
    BYTE hlData = m_ContextZ80.m_FuncPtrRead(m_ContextZ80.m_RegisterHL.reg);
    m_ContextZ80.m_FuncPtrWrite(m_ContextZ80.m_RegisterDE.reg, hlData);

    m_ContextZ80.m_RegisterDE.reg--;
    m_ContextZ80.m_RegisterHL.reg--;
    m_ContextZ80.m_RegisterBC.reg--;

    // the following flags are not used in the master system
    //  BYTE flagTest = m_ContextZ80.m_RegisterAF.hi + hlData;
    //  if (TestBit(flagTest,1))
    //      m_ContextZ80.m_RegisterAF.lo = BitSet(m_ContextZ80.m_RegisterAF.lo, FLAG_B5);
    //  else
    //      m_ContextZ80.m_RegisterAF.lo = BitReset(m_ContextZ80.m_RegisterAF.lo, FLAG_B5);
    //  if (TestBit(flagTest,3))
    //      m_ContextZ80.m_RegisterAF.lo = BitSet(m_ContextZ80.m_RegisterAF.lo, FLAG_B3);
    //  else
    //      m_ContextZ80.m_RegisterAF.lo = BitReset(m_ContextZ80.m_RegisterAF.lo, FLAG_B3);

    if (m_ContextZ80.m_RegisterBC.reg == 0)
        m_ContextZ80.m_RegisterAF.lo = BitReset(m_ContextZ80.m_RegisterAF.lo, FLAG_PV);
    else
        m_ContextZ80.m_RegisterAF.lo = BitSet(m_ContextZ80.m_RegisterAF.lo, FLAG_PV);

    m_ContextZ80.m_RegisterAF.lo = BitReset(m_ContextZ80.m_RegisterAF.lo, FLAG_H);
    m_ContextZ80.m_RegisterAF.lo = BitReset(m_ContextZ80.m_RegisterAF.lo, FLAG_N);


    m_ContextZ80.m_OpcodeCycle = 16;
}

//////////////////////////////////////////////////////////////////////////////////

void Z80::CPU_LDDR()
{
    CPU_LDD();
    // keep calling this function until bc == 0
    if (m_ContextZ80.m_RegisterBC.reg != 0)
    {
        m_ContextZ80.m_OpcodeCycle = 21;
        m_ContextZ80.m_ProgramCounter-=2;
    }
    else
    {
        m_ContextZ80.m_RegisterAF.lo = BitReset(m_ContextZ80.m_RegisterAF.lo, FLAG_PV);
        m_ContextZ80.m_OpcodeCycle = 16;
    }
}

//////////////////////////////////////////////////////////////////////////////////

void Z80::CPU_DDFD_RLC(BYTE& reg, WORD& ixiyreg, SIGNED_BYTE& displacement)
{
    WORD address = ixiyreg + displacement;
    reg = m_ContextZ80.m_FuncPtrRead(address);
    CPU_RLC(reg,false);
    m_ContextZ80.m_FuncPtrWrite(address, reg);
    m_ContextZ80.m_OpcodeCycle = 23;
}

//////////////////////////////////////////////////////////////////////////////////

void Z80::CPU_DDFD_RRC(BYTE& reg, WORD& ixiyreg, SIGNED_BYTE& displacement)
{
    WORD address = ixiyreg + displacement;
    reg = m_ContextZ80.m_FuncPtrRead(address);
    CPU_RRC(reg,false);
    m_ContextZ80.m_FuncPtrWrite(address, reg);
    m_ContextZ80.m_OpcodeCycle = 23;

}

//////////////////////////////////////////////////////////////////////////////////

void Z80::CPU_DDFD_RL(BYTE& reg, WORD& ixiyreg, SIGNED_BYTE& displacement)
{
    WORD address = ixiyreg + displacement;
    reg = m_ContextZ80.m_FuncPtrRead(address);
    CPU_RL(reg,false);
    m_ContextZ80.m_FuncPtrWrite(address, reg);
    m_ContextZ80.m_OpcodeCycle = 23;

}

//////////////////////////////////////////////////////////////////////////////////

void Z80::CPU_DDFD_RR(BYTE& reg, WORD& ixiyreg, SIGNED_BYTE& displacement)
{
    WORD address = ixiyreg + displacement;
    reg = m_ContextZ80.m_FuncPtrRead(address);
    CPU_RR(reg,false);
    m_ContextZ80.m_FuncPtrWrite(address, reg);
    m_ContextZ80.m_OpcodeCycle = 23;

}

//////////////////////////////////////////////////////////////////////////////////

void Z80::CPU_DDFD_SLA (BYTE& reg, WORD& ixiyreg, SIGNED_BYTE& displacement)
{
    WORD address = ixiyreg + displacement;
    reg = m_ContextZ80.m_FuncPtrRead(address);
    CPU_SLA(reg);
    m_ContextZ80.m_FuncPtrWrite(address, reg);
    m_ContextZ80.m_OpcodeCycle = 23;

}

//////////////////////////////////////////////////////////////////////////////////

void Z80::CPU_DDFD_SRA (BYTE& reg, WORD& ixiyreg, SIGNED_BYTE& displacement)
{
    WORD address = ixiyreg + displacement;
    reg = m_ContextZ80.m_FuncPtrRead(address);
    CPU_SRA(reg);
    m_ContextZ80.m_FuncPtrWrite(address, reg);
    m_ContextZ80.m_OpcodeCycle = 23;
}

//////////////////////////////////////////////////////////////////////////////////

void Z80::CPU_DDFD_SRL (BYTE& reg, WORD& ixiyreg, SIGNED_BYTE& displacement)
{
    WORD address = ixiyreg + displacement;
    reg = m_ContextZ80.m_FuncPtrRead(address);
    CPU_SRL(reg);
    m_ContextZ80.m_FuncPtrWrite(address, reg);
    m_ContextZ80.m_OpcodeCycle = 23;

}

///////////////////////////////////////////////////////////////////////////////////

void Z80::CPU_DDFD_SLL (BYTE& reg, WORD& ixiyreg, SIGNED_BYTE& displacement)
{
    WORD address = ixiyreg + displacement;
    reg = m_ContextZ80.m_FuncPtrRead(address);
    CPU_SLL(reg);
    m_ContextZ80.m_FuncPtrWrite(address, reg);
    m_ContextZ80.m_OpcodeCycle = 23;
}
//////////////////////////////////////////////////////////////////////////////////

void Z80::CPU_NEG()
{
    BYTE before = m_ContextZ80.m_RegisterAF.hi;

    m_ContextZ80.m_RegisterAF.lo = 0;

    m_ContextZ80.m_RegisterAF.hi = 0 - m_ContextZ80.m_RegisterAF.hi;

    m_ContextZ80.m_OpcodeCycle = 8;

    BYTE a = m_ContextZ80.m_RegisterAF.hi;

    // correct
    m_ContextZ80.m_RegisterAF.lo = BitSet(m_ContextZ80.m_RegisterAF.lo, FLAG_N); 

    // correct
    if (a == 0)
        m_ContextZ80.m_RegisterAF.lo = BitSet(m_ContextZ80.m_RegisterAF.lo, FLAG_Z);

    // correct
    if (a > 127)
        m_ContextZ80.m_RegisterAF.lo = BitSet(m_ContextZ80.m_RegisterAF.lo, FLAG_S);

    SIGNED_WORD htest = 0;
    htest -= (before & 0xF);

    // correct
    if (htest < 0)
        m_ContextZ80.m_RegisterAF.lo = BitSet(m_ContextZ80.m_RegisterAF.lo, FLAG_H);

    // correct
    if (before == 128)
        m_ContextZ80.m_RegisterAF.lo = BitSet(m_ContextZ80.m_RegisterAF.lo, FLAG_PV);

    // correct
    if (before != 0)
        m_ContextZ80.m_RegisterAF.lo = BitSet(m_ContextZ80.m_RegisterAF.lo, FLAG_C);

}

//////////////////////////////////////////////////////////////////////////////////

void Z80::CPU_DDFD_RESET_BIT(BYTE& reg, int bit, WORD& ixiyreg, SIGNED_BYTE& displacement)
{
    WORD address = ixiyreg + displacement;
    reg = m_ContextZ80.m_FuncPtrRead(address);
    CPU_RESET_BIT(reg,bit);
    m_ContextZ80.m_FuncPtrWrite(address, reg);
    m_ContextZ80.m_OpcodeCycle = 23;
}

//////////////////////////////////////////////////////////////////////////////////

void Z80::CPU_DDFD_SET_BIT(BYTE& reg, int bit, WORD& ixiyreg, SIGNED_BYTE& displacement)
{
    WORD address = ixiyreg + displacement;
    reg = m_ContextZ80.m_FuncPtrRead(address);
    CPU_SET_BIT(reg,bit);
    m_ContextZ80.m_FuncPtrWrite(address, reg);
    m_ContextZ80.m_OpcodeCycle = 23;
}

//////////////////////////////////////////////////////////////////////////////////

void Z80::CPU_DDFD_TEST_BIT(BYTE reg, int bit, WORD& ixiyreg, SIGNED_BYTE& displacement)
{
    WORD address = ixiyreg + displacement;
    reg = m_ContextZ80.m_FuncPtrRead(address);
    CPU_TEST_BIT(reg,bit,0);
    m_ContextZ80.m_FuncPtrWrite(address, reg);
    m_ContextZ80.m_OpcodeCycle = 20;
}

//////////////////////////////////////////////////////////////////////////////////

void Z80::CPU_RLD()
{
    m_ContextZ80.m_OpcodeCycle = 2;
    BYTE hldata = m_ContextZ80.m_FuncPtrRead(m_ContextZ80.m_RegisterHL.reg);
    BYTE nibbleloA = m_ContextZ80.m_RegisterAF.hi & 0xF;
    BYTE nibbleloHL = hldata & 0xF;
    BYTE nibblehiHL = hldata >> 4;

    m_ContextZ80.m_RegisterAF.hi &= 0xF0;
    m_ContextZ80.m_RegisterAF.hi |= nibblehiHL;
    hldata = nibbleloHL << 4;
    hldata |= nibbleloA;

    m_ContextZ80.m_FuncPtrWrite(m_ContextZ80.m_RegisterHL.reg, hldata);


    BYTE a = m_ContextZ80.m_RegisterAF.hi;
    BYTE& f = m_ContextZ80.m_RegisterAF.lo;

    if (TestBit(a,7))
        f = BitSet(f, FLAG_S);
    else
        f = BitReset(f,FLAG_S);

    if (a==0)
        f = BitSet(f, FLAG_Z);
    else
        f = BitReset(f,FLAG_Z);

    f = BitReset(f,FLAG_H);
    f = BitReset(f, FLAG_N);

    int pcount = 0;
    for (int i = 0; i < 8; i++)
    {
        if (TestBit(a,i))
            pcount++;
    }

    if ((pcount == 0) || ((pcount % 2) == 0))
        f = BitSet(f, FLAG_PV);
    else
        f = BitReset(f, FLAG_PV);
}

//////////////////////////////////////////////////////////////////////////////////

void Z80::CPU_RRD()
{
    m_ContextZ80.m_OpcodeCycle = 2;
    BYTE hldata = m_ContextZ80.m_FuncPtrRead(m_ContextZ80.m_RegisterHL.reg);
    BYTE nibbleloA = m_ContextZ80.m_RegisterAF.hi & 0xF;
    BYTE nibbleloHL = hldata & 0xF;
    BYTE nibblehiHL = hldata >> 4;

    m_ContextZ80.m_RegisterAF.hi &= 0xF0;
    m_ContextZ80.m_RegisterAF.hi |= nibbleloHL;
    hldata = nibbleloA << 4;
    hldata |= nibblehiHL;


    m_ContextZ80.m_FuncPtrWrite(m_ContextZ80.m_RegisterHL.reg, hldata);

    BYTE a = m_ContextZ80.m_RegisterAF.hi;
    BYTE& f = m_ContextZ80.m_RegisterAF.lo;

    if (TestBit(a,7))
        f = BitSet(f, FLAG_S);
    else
        f = BitReset(f,FLAG_S);

    if (a==0)
        f = BitSet(f, FLAG_Z);
    else
        f = BitReset(f,FLAG_Z);

    f = BitReset(f,FLAG_H);
    f = BitReset(f, FLAG_N);

    int pcount = 0;
    for (int i = 0; i < 8; i++)
    {
        if (TestBit(a,i))
            pcount++;
    }

    if ((pcount == 0) || ((pcount % 2) == 0))
        f = BitSet(f, FLAG_PV);
    else
        f = BitReset(f, FLAG_PV);

}

//////////////////////////////////////////////////////////////////////////////////

BYTE Z80::CPU_CPI()
{
    bool carry = TestBit(m_ContextZ80.m_RegisterAF.lo, FLAG_C);
    BYTE res =  m_ContextZ80.m_FuncPtrRead(m_ContextZ80.m_RegisterHL.reg);

    CPU_8BIT_COMPARE(m_ContextZ80.m_RegisterAF.hi,res,0,false);
    CPU_16BIT_INC(m_ContextZ80.m_RegisterHL.reg, 0);
    CPU_16BIT_DEC(m_ContextZ80.m_RegisterBC.reg,0);

    if (m_ContextZ80.m_RegisterBC.reg == 0)
        m_ContextZ80.m_RegisterAF.lo = BitReset(m_ContextZ80.m_RegisterAF.lo,FLAG_PV);
    else
        m_ContextZ80.m_RegisterAF.lo = BitSet(m_ContextZ80.m_RegisterAF.lo,FLAG_PV);

    if (!carry)
        m_ContextZ80.m_RegisterAF.lo = BitReset(m_ContextZ80.m_RegisterAF.lo,FLAG_C);
    else
        m_ContextZ80.m_RegisterAF.lo = BitSet(m_ContextZ80.m_RegisterAF.lo,FLAG_C);

    m_ContextZ80.m_OpcodeCycle = 16;
    return res;
}

//////////////////////////////////////////////////////////////////////////////////

void Z80::CPU_CPIR()
{
    BYTE hladdress = CPU_CPI();
    // keep calling this function until b == 0
    if ((m_ContextZ80.m_RegisterBC.reg != 0) && (hladdress != m_ContextZ80.m_RegisterAF.hi))
    {
        m_ContextZ80.m_OpcodeCycle = 21;
        m_ContextZ80.m_ProgramCounter-=2;
    }
    else
    {
        m_ContextZ80.m_OpcodeCycle = 16;
    }
}

//////////////////////////////////////////////////////////////////////////////////

BYTE Z80::CPU_CPD()
{
    bool carry = TestBit(m_ContextZ80.m_RegisterAF.lo, FLAG_C);
    BYTE res = m_ContextZ80.m_FuncPtrRead(m_ContextZ80.m_RegisterHL.reg);

    CPU_8BIT_COMPARE(m_ContextZ80.m_RegisterAF.hi,res ,0,false);
    CPU_16BIT_DEC(m_ContextZ80.m_RegisterHL.reg, 0);
    CPU_16BIT_DEC(m_ContextZ80.m_RegisterBC.reg,0);

    if (m_ContextZ80.m_RegisterBC.reg == 0)
        m_ContextZ80.m_RegisterAF.lo = BitReset(m_ContextZ80.m_RegisterAF.lo,FLAG_PV);
    else
        m_ContextZ80.m_RegisterAF.lo = BitSet(m_ContextZ80.m_RegisterAF.lo,FLAG_PV);

    if (!carry)
        m_ContextZ80.m_RegisterAF.lo = BitReset(m_ContextZ80.m_RegisterAF.lo,FLAG_C);
    else
        m_ContextZ80.m_RegisterAF.lo = BitSet(m_ContextZ80.m_RegisterAF.lo,FLAG_C);

    m_ContextZ80.m_OpcodeCycle = 16;
    return res;
}

//////////////////////////////////////////////////////////////////////////////////

void Z80::CPU_CPDR()
{
    BYTE hladdress = CPU_CPD();
    // keep calling this function until bc == 0
    // or a == hladdress
    if ((m_ContextZ80.m_RegisterBC.reg != 0) && (hladdress != m_ContextZ80.m_RegisterAF.hi))
    {
        m_ContextZ80.m_OpcodeCycle = 21;
        m_ContextZ80.m_ProgramCounter-=2;
    }
    else
    {
        m_ContextZ80.m_OpcodeCycle = 16;
    }
}

//////////////////////////////////////////////////////////////////////////////////

void Z80::CPU_8BIT_IXIY_LOAD(BYTE& store , const REGISTERZ80& reg)
{

    CPU_REG_LOAD_ROM(store, GetIXIYAddress(reg.reg)) ;
    m_ContextZ80.m_OpcodeCycle = 11;
}

//////////////////////////////////////////////////////////////////////////////////

void Z80::CPU_8BIT_MEM_IXIY_LOAD(BYTE store , const REGISTERZ80& reg)
{
    m_ContextZ80.m_FuncPtrWrite(GetIXIYAddress(reg.reg), store);
    m_ContextZ80.m_OpcodeCycle=19;
}

//////////////////////////////////////////////////////////////////////////////////

void Z80::CPU_DAA()
{
    m_ContextZ80.m_OpcodeCycle = 4;

    int i = m_ContextZ80.m_RegisterAF.hi;
    bool cSet = TestBit(m_ContextZ80.m_RegisterAF.lo, FLAG_C);
    bool hSet = TestBit(m_ContextZ80.m_RegisterAF.lo, FLAG_H);
    bool nSet = TestBit(m_ContextZ80.m_RegisterAF.lo, FLAG_N);

    if (cSet) 
        i |= 0x100;
    if (hSet) 
        i |= 0x200;
    if (nSet) 
        i |= 0x400;

    m_ContextZ80.m_RegisterAF.reg = m_DAATable[i];

}


//////////////////////////////////////////////////////////////////////////////////

void Z80::CPU_LDA_I()
{
    m_ContextZ80.m_OpcodeCycle = 9;
    m_ContextZ80.m_RegisterAF.hi = m_ContextZ80.m_RegisterI;

    if (m_ContextZ80.m_RegisterAF.hi == 0)
        m_ContextZ80.m_RegisterAF.lo = BitSet(m_ContextZ80.m_RegisterAF.lo, FLAG_Z);
    else
        m_ContextZ80.m_RegisterAF.lo = BitReset(m_ContextZ80.m_RegisterAF.lo, FLAG_Z);

    m_ContextZ80.m_RegisterAF.lo = BitReset(m_ContextZ80.m_RegisterAF.lo, FLAG_H);
    m_ContextZ80.m_RegisterAF.lo = BitReset(m_ContextZ80.m_RegisterAF.lo, FLAG_N);

    if (m_ContextZ80.m_IFF2)
        m_ContextZ80.m_RegisterAF.lo = BitSet(m_ContextZ80.m_RegisterAF.lo, FLAG_PV);
    else
        m_ContextZ80.m_RegisterAF.lo = BitReset(m_ContextZ80.m_RegisterAF.lo, FLAG_PV);

    if (TestBit(m_ContextZ80.m_RegisterAF.hi, 7))
        m_ContextZ80.m_RegisterAF.lo = BitSet(m_ContextZ80.m_RegisterAF.lo, FLAG_S);
    else
        m_ContextZ80.m_RegisterAF.lo = BitReset(m_ContextZ80.m_RegisterAF.lo, FLAG_S);
}

//////////////////////////////////////////////////////////////////////////////////

void Z80::CPU_LDA_R()
{
    m_ContextZ80.m_OpcodeCycle = 9;
    m_ContextZ80.m_RegisterAF.hi = m_ContextZ80.m_RegisterR;

    if (m_ContextZ80.m_RegisterAF.hi == 0)
        m_ContextZ80.m_RegisterAF.lo = BitSet(m_ContextZ80.m_RegisterAF.lo, FLAG_Z);
    else
        m_ContextZ80.m_RegisterAF.lo = BitReset(m_ContextZ80.m_RegisterAF.lo, FLAG_Z);

    m_ContextZ80.m_RegisterAF.lo = BitReset(m_ContextZ80.m_RegisterAF.lo, FLAG_H);
    m_ContextZ80.m_RegisterAF.lo = BitReset(m_ContextZ80.m_RegisterAF.lo, FLAG_N);

    if (m_ContextZ80.m_IFF2)
        m_ContextZ80.m_RegisterAF.lo = BitSet(m_ContextZ80.m_RegisterAF.lo, FLAG_PV);
    else
        m_ContextZ80.m_RegisterAF.lo = BitReset(m_ContextZ80.m_RegisterAF.lo, FLAG_PV);

    if (TestBit(m_ContextZ80.m_RegisterAF.hi, 7))
        m_ContextZ80.m_RegisterAF.lo = BitSet(m_ContextZ80.m_RegisterAF.lo, FLAG_S);
    else
        m_ContextZ80.m_RegisterAF.lo = BitReset(m_ContextZ80.m_RegisterAF.lo, FLAG_S);
}