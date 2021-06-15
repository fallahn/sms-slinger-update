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
#include "Z80.Opcodes.h"
#include "LogMessages.h"

#include <cassert>
#include <cstdio>

void Z80::IncreaseRReg()
{
    if ((m_ContextZ80.m_RegisterR & 127) == 127)
    {
        m_ContextZ80.m_RegisterR = m_ContextZ80.m_RegisterR & 128;
    }
    else
    {
        m_ContextZ80.m_RegisterR++;
    }
}

void Z80::ExecuteOpcode(const BYTE& opcode)
{
    IncreaseRReg();
    

//  char buffer[255];
//  sprintf(buffer, "Executing Opcode %x",opcode);
    //LogMessage::GetSingleton()->DoLogMessage(buffer,true);


    switch(opcode)
    {
        //no-op
        case 0x00: m_ContextZ80.m_OpcodeCycle=4; break;

        // 8-Bit Loads
        case 0x06: CPU_8BIT_LOAD_IMMEDIATE(m_ContextZ80.m_RegisterBC.hi); break;
        case 0x0E: CPU_8BIT_LOAD_IMMEDIATE(m_ContextZ80.m_RegisterBC.lo); break;
        case 0x16: CPU_8BIT_LOAD_IMMEDIATE(m_ContextZ80.m_RegisterDE.hi); break;
        case 0x1E: CPU_8BIT_LOAD_IMMEDIATE(m_ContextZ80.m_RegisterDE.lo); break;
        case 0x26: CPU_8BIT_LOAD_IMMEDIATE(m_ContextZ80.m_RegisterHL.hi); break;
        case 0x2E: CPU_8BIT_LOAD_IMMEDIATE(m_ContextZ80.m_RegisterHL.lo); break;
        case 0x3E: CPU_8BIT_LOAD_IMMEDIATE(m_ContextZ80.m_RegisterAF.hi); break;

        // 8-Bit Reg Loads
        case 0x7F: CPU_REG_LOAD(m_ContextZ80.m_RegisterAF.hi, m_ContextZ80.m_RegisterAF.hi); break;
        case 0x78: CPU_REG_LOAD(m_ContextZ80.m_RegisterAF.hi, m_ContextZ80.m_RegisterBC.hi); break;
        case 0x79: CPU_REG_LOAD(m_ContextZ80.m_RegisterAF.hi, m_ContextZ80.m_RegisterBC.lo); break;
        case 0x7A: CPU_REG_LOAD(m_ContextZ80.m_RegisterAF.hi, m_ContextZ80.m_RegisterDE.hi); break;
        case 0x7B: CPU_REG_LOAD(m_ContextZ80.m_RegisterAF.hi, m_ContextZ80.m_RegisterDE.lo); break;
        case 0x7C: CPU_REG_LOAD(m_ContextZ80.m_RegisterAF.hi, m_ContextZ80.m_RegisterHL.hi); break;
        case 0x7D: CPU_REG_LOAD(m_ContextZ80.m_RegisterAF.hi, m_ContextZ80.m_RegisterHL.lo); break;
        case 0x40: CPU_REG_LOAD(m_ContextZ80.m_RegisterBC.hi, m_ContextZ80.m_RegisterBC.hi); break;
        case 0x41: CPU_REG_LOAD(m_ContextZ80.m_RegisterBC.hi, m_ContextZ80.m_RegisterBC.lo); break;
        case 0x42: CPU_REG_LOAD(m_ContextZ80.m_RegisterBC.hi, m_ContextZ80.m_RegisterDE.hi); break;
        case 0x43: CPU_REG_LOAD(m_ContextZ80.m_RegisterBC.hi, m_ContextZ80.m_RegisterDE.lo); break;
        case 0x44: CPU_REG_LOAD(m_ContextZ80.m_RegisterBC.hi, m_ContextZ80.m_RegisterHL.hi); break;
        case 0x45: CPU_REG_LOAD(m_ContextZ80.m_RegisterBC.hi, m_ContextZ80.m_RegisterHL.lo); break;
        case 0x48: CPU_REG_LOAD(m_ContextZ80.m_RegisterBC.lo, m_ContextZ80.m_RegisterBC.hi); break;
        case 0x49: CPU_REG_LOAD(m_ContextZ80.m_RegisterBC.lo, m_ContextZ80.m_RegisterBC.lo); break;
        case 0x4A: CPU_REG_LOAD(m_ContextZ80.m_RegisterBC.lo, m_ContextZ80.m_RegisterDE.hi); break;
        case 0x4B: CPU_REG_LOAD(m_ContextZ80.m_RegisterBC.lo, m_ContextZ80.m_RegisterDE.lo); break;
        case 0x4C: CPU_REG_LOAD(m_ContextZ80.m_RegisterBC.lo, m_ContextZ80.m_RegisterHL.hi); break;
        case 0x4D: CPU_REG_LOAD(m_ContextZ80.m_RegisterBC.lo, m_ContextZ80.m_RegisterHL.lo); break;
        case 0x50: CPU_REG_LOAD(m_ContextZ80.m_RegisterDE.hi, m_ContextZ80.m_RegisterBC.hi); break;
        case 0x51: CPU_REG_LOAD(m_ContextZ80.m_RegisterDE.hi, m_ContextZ80.m_RegisterBC.lo); break;
        case 0x52: CPU_REG_LOAD(m_ContextZ80.m_RegisterDE.hi, m_ContextZ80.m_RegisterDE.hi); break;
        case 0x53: CPU_REG_LOAD(m_ContextZ80.m_RegisterDE.hi, m_ContextZ80.m_RegisterDE.lo); break;
        case 0x54: CPU_REG_LOAD(m_ContextZ80.m_RegisterDE.hi, m_ContextZ80.m_RegisterHL.hi); break;
        case 0x55: CPU_REG_LOAD(m_ContextZ80.m_RegisterDE.hi, m_ContextZ80.m_RegisterHL.lo); break;
        case 0x58: CPU_REG_LOAD(m_ContextZ80.m_RegisterDE.lo, m_ContextZ80.m_RegisterBC.hi); break;
        case 0x59: CPU_REG_LOAD(m_ContextZ80.m_RegisterDE.lo, m_ContextZ80.m_RegisterBC.lo); break;
        case 0x5A: CPU_REG_LOAD(m_ContextZ80.m_RegisterDE.lo, m_ContextZ80.m_RegisterDE.hi); break;
        case 0x5B: CPU_REG_LOAD(m_ContextZ80.m_RegisterDE.lo, m_ContextZ80.m_RegisterDE.lo); break;
        case 0x5C: CPU_REG_LOAD(m_ContextZ80.m_RegisterDE.lo, m_ContextZ80.m_RegisterHL.hi); break;
        case 0x5D: CPU_REG_LOAD(m_ContextZ80.m_RegisterDE.lo, m_ContextZ80.m_RegisterHL.lo); break;
        case 0x60: CPU_REG_LOAD(m_ContextZ80.m_RegisterHL.hi, m_ContextZ80.m_RegisterBC.hi); break;
        case 0x61: CPU_REG_LOAD(m_ContextZ80.m_RegisterHL.hi, m_ContextZ80.m_RegisterBC.lo); break;
        case 0x62: CPU_REG_LOAD(m_ContextZ80.m_RegisterHL.hi, m_ContextZ80.m_RegisterDE.hi); break;
        case 0x63: CPU_REG_LOAD(m_ContextZ80.m_RegisterHL.hi, m_ContextZ80.m_RegisterDE.lo); break;
        case 0x64: CPU_REG_LOAD(m_ContextZ80.m_RegisterHL.hi, m_ContextZ80.m_RegisterHL.hi); break;
        case 0x65: CPU_REG_LOAD(m_ContextZ80.m_RegisterHL.hi, m_ContextZ80.m_RegisterHL.lo); break;
        case 0x68: CPU_REG_LOAD(m_ContextZ80.m_RegisterHL.lo, m_ContextZ80.m_RegisterBC.hi); break;
        case 0x69: CPU_REG_LOAD(m_ContextZ80.m_RegisterHL.lo, m_ContextZ80.m_RegisterBC.lo); break;
        case 0x6A: CPU_REG_LOAD(m_ContextZ80.m_RegisterHL.lo, m_ContextZ80.m_RegisterDE.hi); break;
        case 0x6B: CPU_REG_LOAD(m_ContextZ80.m_RegisterHL.lo, m_ContextZ80.m_RegisterDE.lo); break;
        case 0x6C: CPU_REG_LOAD(m_ContextZ80.m_RegisterHL.lo, m_ContextZ80.m_RegisterHL.hi); break;
        case 0x6D: CPU_REG_LOAD(m_ContextZ80.m_RegisterHL.lo, m_ContextZ80.m_RegisterHL.lo); break;
        case 0x47: CPU_REG_LOAD(m_ContextZ80.m_RegisterBC.hi, m_ContextZ80.m_RegisterAF.hi); break;
        case 0x4F: CPU_REG_LOAD(m_ContextZ80.m_RegisterBC.lo, m_ContextZ80.m_RegisterAF.hi); break;
        case 0x57: CPU_REG_LOAD(m_ContextZ80.m_RegisterDE.hi, m_ContextZ80.m_RegisterAF.hi); break;
        case 0x5F: CPU_REG_LOAD(m_ContextZ80.m_RegisterDE.lo, m_ContextZ80.m_RegisterAF.hi); break;
        case 0x67: CPU_REG_LOAD(m_ContextZ80.m_RegisterHL.hi, m_ContextZ80.m_RegisterAF.hi); break;
        case 0x6F: CPU_REG_LOAD(m_ContextZ80.m_RegisterHL.lo, m_ContextZ80.m_RegisterAF.hi); break;

        // write reg to memory
        case 0x70: m_ContextZ80.m_FuncPtrWrite(m_ContextZ80.m_RegisterHL.reg, m_ContextZ80.m_RegisterBC.hi); m_ContextZ80.m_OpcodeCycle=7; break;
        case 0x71: m_ContextZ80.m_FuncPtrWrite(m_ContextZ80.m_RegisterHL.reg, m_ContextZ80.m_RegisterBC.lo); m_ContextZ80.m_OpcodeCycle=7;break;
        case 0x72: m_ContextZ80.m_FuncPtrWrite(m_ContextZ80.m_RegisterHL.reg, m_ContextZ80.m_RegisterDE.hi); m_ContextZ80.m_OpcodeCycle=7;break;
        case 0x73: m_ContextZ80.m_FuncPtrWrite(m_ContextZ80.m_RegisterHL.reg, m_ContextZ80.m_RegisterDE.lo); m_ContextZ80.m_OpcodeCycle=7;break;
        case 0x74: m_ContextZ80.m_FuncPtrWrite(m_ContextZ80.m_RegisterHL.reg, m_ContextZ80.m_RegisterHL.hi); m_ContextZ80.m_OpcodeCycle=7;break;
        case 0x75: m_ContextZ80.m_FuncPtrWrite(m_ContextZ80.m_RegisterHL.reg, m_ContextZ80.m_RegisterHL.lo); m_ContextZ80.m_OpcodeCycle=7;break;
        case 0x02: m_ContextZ80.m_FuncPtrWrite(m_ContextZ80.m_RegisterBC.reg, m_ContextZ80.m_RegisterAF.hi); m_ContextZ80.m_OpcodeCycle=7; break;
        case 0x12: m_ContextZ80.m_FuncPtrWrite(m_ContextZ80.m_RegisterDE.reg, m_ContextZ80.m_RegisterAF.hi); m_ContextZ80.m_OpcodeCycle=7; break;
        case 0x77: m_ContextZ80.m_FuncPtrWrite(m_ContextZ80.m_RegisterHL.reg, m_ContextZ80.m_RegisterAF.hi); m_ContextZ80.m_OpcodeCycle=7; break;

        // write memory to reg
        case 0x7E: CPU_REG_LOAD_ROM(m_ContextZ80.m_RegisterAF.hi, m_ContextZ80.m_RegisterHL.reg); break;
        case 0x46: CPU_REG_LOAD_ROM(m_ContextZ80.m_RegisterBC.hi, m_ContextZ80.m_RegisterHL.reg); break;
        case 0x4E: CPU_REG_LOAD_ROM(m_ContextZ80.m_RegisterBC.lo, m_ContextZ80.m_RegisterHL.reg); break;
        case 0x56: CPU_REG_LOAD_ROM(m_ContextZ80.m_RegisterDE.hi, m_ContextZ80.m_RegisterHL.reg); break;
        case 0x5E: CPU_REG_LOAD_ROM(m_ContextZ80.m_RegisterDE.lo, m_ContextZ80.m_RegisterHL.reg); break;
        case 0x66: CPU_REG_LOAD_ROM(m_ContextZ80.m_RegisterHL.hi, m_ContextZ80.m_RegisterHL.reg); break;
        case 0x6E: CPU_REG_LOAD_ROM(m_ContextZ80.m_RegisterHL.lo, m_ContextZ80.m_RegisterHL.reg); break;
        case 0x0A: CPU_REG_LOAD_ROM(m_ContextZ80.m_RegisterAF.hi, m_ContextZ80.m_RegisterBC.reg); break;
        case 0x1A: CPU_REG_LOAD_ROM(m_ContextZ80.m_RegisterAF.hi, m_ContextZ80.m_RegisterDE.reg); break;

        // 16 bit loads
        case 0x01: CPU_16BIT_LOAD(m_ContextZ80.m_RegisterBC.reg); break;
        case 0x11: CPU_16BIT_LOAD(m_ContextZ80.m_RegisterDE.reg);break;
        case 0x21: CPU_16BIT_LOAD(m_ContextZ80.m_RegisterHL.reg);break;
        case 0x31: CPU_16BIT_LOAD(m_ContextZ80.m_StackPointer.reg);break;
        case 0xF9: m_ContextZ80.m_StackPointer.reg = m_ContextZ80.m_RegisterHL.reg; m_ContextZ80.m_OpcodeCycle=2; break;

        // push word onto stack
        case 0xF5: PushWordOntoStack(m_ContextZ80.m_RegisterAF.reg);  m_ContextZ80.m_OpcodeCycle=11;break;
        case 0xC5: PushWordOntoStack(m_ContextZ80.m_RegisterBC.reg);  m_ContextZ80.m_OpcodeCycle=11;break;
        case 0xD5: PushWordOntoStack(m_ContextZ80.m_RegisterDE.reg);  m_ContextZ80.m_OpcodeCycle=11;break;
        case 0xE5: PushWordOntoStack(m_ContextZ80.m_RegisterHL.reg);  m_ContextZ80.m_OpcodeCycle=11; break;

        // pop word from stack into reg
        case 0xF1: m_ContextZ80.m_RegisterAF.reg = PopWordOffStack();  m_ContextZ80.m_OpcodeCycle=10;break;
        case 0xC1: m_ContextZ80.m_RegisterBC.reg = PopWordOffStack();  m_ContextZ80.m_OpcodeCycle=10;break;
        case 0xD1: m_ContextZ80.m_RegisterDE.reg = PopWordOffStack();  m_ContextZ80.m_OpcodeCycle=10;break;
        case 0xE1: m_ContextZ80.m_RegisterHL.reg = PopWordOffStack();  m_ContextZ80.m_OpcodeCycle=10; break;

            // 8-bit add
        case 0x87: CPU_8BIT_ADD(m_ContextZ80.m_RegisterAF.hi, m_ContextZ80.m_RegisterAF.hi,4,false,false); break;
        case 0x80: CPU_8BIT_ADD(m_ContextZ80.m_RegisterAF.hi, m_ContextZ80.m_RegisterBC.hi,4,false,false); break;
        case 0x81: CPU_8BIT_ADD(m_ContextZ80.m_RegisterAF.hi, m_ContextZ80. m_RegisterBC.lo,4,false,false); break;
        case 0x82: CPU_8BIT_ADD(m_ContextZ80.m_RegisterAF.hi, m_ContextZ80.m_RegisterDE.hi,4,false,false); break;
        case 0x83: CPU_8BIT_ADD(m_ContextZ80.m_RegisterAF.hi, m_ContextZ80.m_RegisterDE.lo,4,false,false); break;
        case 0x84: CPU_8BIT_ADD(m_ContextZ80.m_RegisterAF.hi, m_ContextZ80.m_RegisterHL.hi,4,false,false); break;
        case 0x85: CPU_8BIT_ADD(m_ContextZ80.m_RegisterAF.hi, m_ContextZ80.m_RegisterHL.lo,4,false,false); break;
        case 0x86: CPU_8BIT_ADD(m_ContextZ80.m_RegisterAF.hi, m_ContextZ80.m_FuncPtrRead(m_ContextZ80.m_RegisterHL.reg),7,false,false); break;
        case 0xC6: CPU_8BIT_ADD(m_ContextZ80.m_RegisterAF.hi, 0,8,true,false); break;

            // 8-bit add + carry
        case 0x8F: CPU_8BIT_ADD(m_ContextZ80.m_RegisterAF.hi, m_ContextZ80.m_RegisterAF.hi,4,false,true); break;
        case 0x88: CPU_8BIT_ADD(m_ContextZ80.m_RegisterAF.hi, m_ContextZ80.m_RegisterBC.hi,4,false,true); break;
        case 0x89: CPU_8BIT_ADD(m_ContextZ80.m_RegisterAF.hi, m_ContextZ80.m_RegisterBC.lo,4,false,true); break;
        case 0x8A: CPU_8BIT_ADD(m_ContextZ80.m_RegisterAF.hi, m_ContextZ80.m_RegisterDE.hi,4,false,true); break;
        case 0x8B: CPU_8BIT_ADD(m_ContextZ80.m_RegisterAF.hi, m_ContextZ80.m_RegisterDE.lo,4,false,true); break;
        case 0x8C: CPU_8BIT_ADD(m_ContextZ80.m_RegisterAF.hi, m_ContextZ80.m_RegisterHL.hi,4,false,true); break;
        case 0x8D: CPU_8BIT_ADD(m_ContextZ80.m_RegisterAF.hi, m_ContextZ80.m_RegisterHL.lo,4,false,true); break;
        case 0x8E: CPU_8BIT_ADD(m_ContextZ80.m_RegisterAF.hi, m_ContextZ80.m_FuncPtrRead(m_ContextZ80.m_RegisterHL.reg),7,false,true); break;
        case 0xCE: CPU_8BIT_ADD(m_ContextZ80.m_RegisterAF.hi, 0,7,true,true); break;

        // 8-bit subtract
        case 0x97: CPU_8BIT_SUB(m_ContextZ80.m_RegisterAF.hi, m_ContextZ80.m_RegisterAF.hi,4,false,false); break;
        case 0x90: CPU_8BIT_SUB(m_ContextZ80.m_RegisterAF.hi, m_ContextZ80.m_RegisterBC.hi,4,false,false); break;
        case 0x91: CPU_8BIT_SUB(m_ContextZ80.m_RegisterAF.hi, m_ContextZ80.m_RegisterBC.lo,4,false,false); break;
        case 0x92: CPU_8BIT_SUB(m_ContextZ80.m_RegisterAF.hi, m_ContextZ80.m_RegisterDE.hi,4,false,false); break;
        case 0x93: CPU_8BIT_SUB(m_ContextZ80.m_RegisterAF.hi, m_ContextZ80.m_RegisterDE.lo,4,false,false); break;
        case 0x94: CPU_8BIT_SUB(m_ContextZ80.m_RegisterAF.hi, m_ContextZ80.m_RegisterHL.hi,4,false,false); break;
        case 0x95: CPU_8BIT_SUB(m_ContextZ80.m_RegisterAF.hi, m_ContextZ80.m_RegisterHL.lo,4,false,false); break;
        case 0x96: CPU_8BIT_SUB(m_ContextZ80.m_RegisterAF.hi, m_ContextZ80.m_FuncPtrRead(m_ContextZ80.m_RegisterHL.reg),7,false,false); break;
        case 0xD6: CPU_8BIT_SUB(m_ContextZ80.m_RegisterAF.hi, 0,7,true,false); break;

        // 8-bit subtract + carry
        case 0x9F: CPU_8BIT_SUB(m_ContextZ80.m_RegisterAF.hi, m_ContextZ80.m_RegisterAF.hi,4,false,true); break;
        case 0x98: CPU_8BIT_SUB(m_ContextZ80.m_RegisterAF.hi, m_ContextZ80.m_RegisterBC.hi,4,false,true); break;
        case 0x99: CPU_8BIT_SUB(m_ContextZ80.m_RegisterAF.hi, m_ContextZ80.m_RegisterBC.lo,4,false,true); break;
        case 0x9A: CPU_8BIT_SUB(m_ContextZ80.m_RegisterAF.hi, m_ContextZ80.m_RegisterDE.hi,4,false,true); break;
        case 0x9B: CPU_8BIT_SUB(m_ContextZ80.m_RegisterAF.hi, m_ContextZ80.m_RegisterDE.lo,4,false,true); break;
        case 0x9C: CPU_8BIT_SUB(m_ContextZ80.m_RegisterAF.hi, m_ContextZ80.m_RegisterHL.hi,4,false,true); break;
        case 0x9D: CPU_8BIT_SUB(m_ContextZ80.m_RegisterAF.hi, m_ContextZ80.m_RegisterHL.lo,4,false,true); break;
        case 0x9E: CPU_8BIT_SUB(m_ContextZ80.m_RegisterAF.hi, m_ContextZ80.m_FuncPtrRead(m_ContextZ80.m_RegisterHL.reg),7,false,true); break;
        case 0xDE: CPU_8BIT_SUB(m_ContextZ80.m_RegisterAF.hi, 0,7,true,true); break;

        // 8-bit AND reg with reg
        case 0xA7: CPU_8BIT_AND(m_ContextZ80.m_RegisterAF.hi, m_ContextZ80.m_RegisterAF.hi,4, false); break;
        case 0xA0: CPU_8BIT_AND(m_ContextZ80.m_RegisterAF.hi, m_ContextZ80.m_RegisterBC.hi,4, false); break;
        case 0xA1: CPU_8BIT_AND(m_ContextZ80.m_RegisterAF.hi, m_ContextZ80.m_RegisterBC.lo,4, false); break;
        case 0xA2: CPU_8BIT_AND(m_ContextZ80.m_RegisterAF.hi, m_ContextZ80.m_RegisterDE.hi,4, false); break;
        case 0xA3: CPU_8BIT_AND(m_ContextZ80.m_RegisterAF.hi, m_ContextZ80.m_RegisterDE.lo,4, false); break;
        case 0xA4: CPU_8BIT_AND(m_ContextZ80.m_RegisterAF.hi, m_ContextZ80.m_RegisterHL.hi,4, false); break;
        case 0xA5: CPU_8BIT_AND(m_ContextZ80.m_RegisterAF.hi, m_ContextZ80.m_RegisterHL.lo,4, false); break;
        case 0xA6: CPU_8BIT_AND(m_ContextZ80.m_RegisterAF.hi, m_ContextZ80.m_FuncPtrRead(m_ContextZ80.m_RegisterHL.reg),7, false); break;
        case 0xE6: CPU_8BIT_AND(m_ContextZ80.m_RegisterAF.hi, 0,7, true); break;

        // 8-bit OR reg with reg
        case 0xB7: CPU_8BIT_OR(m_ContextZ80.m_RegisterAF.hi, m_ContextZ80.m_RegisterAF.hi,4, false); break;
        case 0xB0: CPU_8BIT_OR(m_ContextZ80.m_RegisterAF.hi, m_ContextZ80.m_RegisterBC.hi,4, false); break;
        case 0xB1: CPU_8BIT_OR(m_ContextZ80.m_RegisterAF.hi, m_ContextZ80.m_RegisterBC.lo,4, false); break;
        case 0xB2: CPU_8BIT_OR(m_ContextZ80.m_RegisterAF.hi, m_ContextZ80.m_RegisterDE.hi,4, false); break;
        case 0xB3: CPU_8BIT_OR(m_ContextZ80.m_RegisterAF.hi, m_ContextZ80.m_RegisterDE.lo,4, false); break;
        case 0xB4: CPU_8BIT_OR(m_ContextZ80.m_RegisterAF.hi, m_ContextZ80.m_RegisterHL.hi,4, false); break;
        case 0xB5: CPU_8BIT_OR(m_ContextZ80.m_RegisterAF.hi, m_ContextZ80.m_RegisterHL.lo,4, false); break;
        case 0xB6: CPU_8BIT_OR(m_ContextZ80.m_RegisterAF.hi, m_ContextZ80.m_FuncPtrRead(m_ContextZ80.m_RegisterHL.reg),7, false); break;
        case 0xF6: CPU_8BIT_OR(m_ContextZ80.m_RegisterAF.hi, 0,7, true); break;

        // 8-bit XOR reg with reg
        case 0xAF: CPU_8BIT_XOR(m_ContextZ80.m_RegisterAF.hi, m_ContextZ80.m_RegisterAF.hi,4, false); break;
        case 0xA8: CPU_8BIT_XOR(m_ContextZ80.m_RegisterAF.hi, m_ContextZ80.m_RegisterBC.hi,4, false); break;
        case 0xA9: CPU_8BIT_XOR(m_ContextZ80.m_RegisterAF.hi, m_ContextZ80.m_RegisterBC.lo,4, false); break;
        case 0xAA: CPU_8BIT_XOR(m_ContextZ80.m_RegisterAF.hi, m_ContextZ80.m_RegisterDE.hi,4, false); break;
        case 0xAB: CPU_8BIT_XOR(m_ContextZ80.m_RegisterAF.hi, m_ContextZ80.m_RegisterDE.lo,4, false); break;
        case 0xAC: CPU_8BIT_XOR(m_ContextZ80.m_RegisterAF.hi, m_ContextZ80.m_RegisterHL.hi,4, false); break;
        case 0xAD: CPU_8BIT_XOR(m_ContextZ80.m_RegisterAF.hi, m_ContextZ80.m_RegisterHL.lo,4, false); break;
        case 0xAE: CPU_8BIT_XOR(m_ContextZ80.m_RegisterAF.hi, m_ContextZ80.m_FuncPtrRead(m_ContextZ80.m_RegisterHL.reg),7, false); break;
        case 0xEE: CPU_8BIT_XOR(m_ContextZ80.m_RegisterAF.hi, 0,7, true); break;

        // 8-Bit compare
        case 0xBF: CPU_8BIT_COMPARE(m_ContextZ80.m_RegisterAF.hi, m_ContextZ80.m_RegisterAF.hi,4, false); break;
        case 0xB8: CPU_8BIT_COMPARE(m_ContextZ80.m_RegisterAF.hi, m_ContextZ80.m_RegisterBC.hi,4, false); break;
        case 0xB9: CPU_8BIT_COMPARE(m_ContextZ80.m_RegisterAF.hi, m_ContextZ80.m_RegisterBC.lo,4, false); break;
        case 0xBA: CPU_8BIT_COMPARE(m_ContextZ80.m_RegisterAF.hi, m_ContextZ80.m_RegisterDE.hi,4, false); break;
        case 0xBB: CPU_8BIT_COMPARE(m_ContextZ80.m_RegisterAF.hi, m_ContextZ80.m_RegisterDE.lo,4, false); break;
        case 0xBC: CPU_8BIT_COMPARE(m_ContextZ80.m_RegisterAF.hi, m_ContextZ80.m_RegisterHL.hi,4, false); break;
        case 0xBD: CPU_8BIT_COMPARE(m_ContextZ80.m_RegisterAF.hi, m_ContextZ80.m_RegisterHL.lo,4, false); break;
        case 0xBE: CPU_8BIT_COMPARE(m_ContextZ80.m_RegisterAF.hi, m_ContextZ80.m_FuncPtrRead(m_ContextZ80.m_RegisterHL.reg),7, false); break;
        case 0xFE: CPU_8BIT_COMPARE(m_ContextZ80.m_RegisterAF.hi, 0,7, true); break;

        // 8-bit inc
        case 0x3C: CPU_8BIT_INC(m_ContextZ80.m_RegisterAF.hi,4); break;
        case 0x04: CPU_8BIT_INC(m_ContextZ80.m_RegisterBC.hi,4); break;
        case 0x0C: CPU_8BIT_INC(m_ContextZ80.m_RegisterBC.lo,4); break;
        case 0x14: CPU_8BIT_INC(m_ContextZ80.m_RegisterDE.hi,4); break;
        case 0x1C: CPU_8BIT_INC(m_ContextZ80.m_RegisterDE.lo,4); break;
        case 0x24: CPU_8BIT_INC(m_ContextZ80.m_RegisterHL.hi,4); break;
        case 0x2C: CPU_8BIT_INC(m_ContextZ80.m_RegisterHL.lo,4); break;
        case 0x34: CPU_8BIT_MEMORY_INC(m_ContextZ80.m_RegisterHL.reg,11); break;

        // 8-bit dec
        case 0x3D: CPU_8BIT_DEC(m_ContextZ80.m_RegisterAF.hi,4); break;
        case 0x05: CPU_8BIT_DEC(m_ContextZ80.m_RegisterBC.hi,4); break;
        case 0x0D: CPU_8BIT_DEC(m_ContextZ80.m_RegisterBC.lo,4); break;
        case 0x15: CPU_8BIT_DEC(m_ContextZ80.m_RegisterDE.hi,4); break;
        case 0x1D: CPU_8BIT_DEC(m_ContextZ80.m_RegisterDE.lo,4); break;
        case 0x25: CPU_8BIT_DEC(m_ContextZ80.m_RegisterHL.hi,4); break;
        case 0x2D: CPU_8BIT_DEC(m_ContextZ80.m_RegisterHL.lo,4); break;
        case 0x35: CPU_8BIT_MEMORY_DEC(m_ContextZ80.m_RegisterHL.reg,11); break;

        // 16-bit add
        case 0x09: CPU_16BIT_ADD(m_ContextZ80.m_RegisterHL.reg,m_ContextZ80.m_RegisterBC.reg,11,false); break;
        case 0x19: CPU_16BIT_ADD(m_ContextZ80.m_RegisterHL.reg,m_ContextZ80.m_RegisterDE.reg,11,false); break;
        case 0x29: CPU_16BIT_ADD(m_ContextZ80.m_RegisterHL.reg,m_ContextZ80.m_RegisterHL.reg,11,false); break;
        case 0x39: CPU_16BIT_ADD(m_ContextZ80.m_RegisterHL.reg,m_ContextZ80.m_StackPointer.reg,11,false); break;

        // inc 16-bit register
        case 0x03: CPU_16BIT_INC(m_ContextZ80.m_RegisterBC.reg, 6); break;
        case 0x13: CPU_16BIT_INC(m_ContextZ80.m_RegisterDE.reg, 6); break;
        case 0x23: CPU_16BIT_INC(m_ContextZ80.m_RegisterHL.reg, 6); break;
        case 0x33: CPU_16BIT_INC(m_ContextZ80.m_StackPointer.reg, 6); break;

        // dec 16-bit register
        case 0x0B: CPU_16BIT_DEC(m_ContextZ80.m_RegisterBC.reg, 6); break;
        case 0x1B: CPU_16BIT_DEC(m_ContextZ80.m_RegisterDE.reg, 6); break;
        case 0x2B: CPU_16BIT_DEC(m_ContextZ80.m_RegisterHL.reg, 6); break;
        case 0x3B: CPU_16BIT_DEC(m_ContextZ80.m_StackPointer.reg, 6); break;

        // jumps
        case 0xE9: m_ContextZ80.m_OpcodeCycle=4; m_ContextZ80.m_ProgramCounter = m_ContextZ80.m_RegisterHL.reg; break;
        case 0xC3: CPU_JUMP(false, 0, false); break;
        case 0xC2: CPU_JUMP(true, FLAG_Z, false); break;
        case 0xCA: CPU_JUMP(true, FLAG_Z, true); break;
        case 0xD2: CPU_JUMP(true, FLAG_C, false); break;
        case 0xDA: CPU_JUMP(true, FLAG_C, true); break;
        case 0xFA: CPU_JUMP(true, FLAG_S, true); break;
        case 0xF2: CPU_JUMP(true, FLAG_S, false); break;
        case 0xE2: CPU_JUMP(true, FLAG_PV,false); break;
        case 0xEA: CPU_JUMP(true, FLAG_PV,true); break;
        case 0x10: CPU_DJNZ();break;

        // jump with immediate data
        case 0x18 : CPU_JUMP_IMMEDIATE(false, 0, false); break;
        case 0x20 : CPU_JUMP_IMMEDIATE(true, FLAG_Z, false);break;
        case 0x28 : CPU_JUMP_IMMEDIATE(true, FLAG_Z, true);break;
        case 0x30 : CPU_JUMP_IMMEDIATE(true, FLAG_C, false);break;
        case 0x38 : CPU_JUMP_IMMEDIATE(true, FLAG_C, true);break;

        // calls
        case 0xCD : CPU_CALL(false, 0, false); break;
        case 0xC4 : CPU_CALL(true, FLAG_Z, false);break;
        case 0xCC : CPU_CALL(true, FLAG_Z, true);break;
        case 0xD4 : CPU_CALL(true, FLAG_C, false);break;
        case 0xDC : CPU_CALL(true, FLAG_C, true); break;
        case 0xE4 : CPU_CALL(true, FLAG_PV, false); break;
        case 0xEC : CPU_CALL(true, FLAG_PV, true); break;
        case 0xF4 : CPU_CALL(true, FLAG_S, false); break;
        case 0xFC : CPU_CALL(true, FLAG_S, true); break;

        // returns
        case 0xC9: CPU_RETURN(false, 0, false); m_ContextZ80.m_OpcodeCycle = 10; break;
        case 0xC0: CPU_RETURN(true, FLAG_Z, false); break;
        case 0xC8: CPU_RETURN(true, FLAG_Z, true); break;
        case 0xD0: CPU_RETURN(true, FLAG_C, false); break;
        case 0xD8: CPU_RETURN(true, FLAG_C, true); break;
        case 0xF8: CPU_RETURN(true, FLAG_S, true); break;
        case 0xE8: CPU_RETURN(true, FLAG_PV, true); break;
        case 0xE0: CPU_RETURN(true, FLAG_PV, false);break;
        case 0xF0: CPU_RETURN(true, FLAG_S, false);break;

        // restarts
        case 0xC7: CPU_RESTARTS(0x00); break;
        case 0xCF: CPU_RESTARTS(0x08); break;
        case 0xD7: CPU_RESTARTS(0x10); break;
        case 0xDF: CPU_RESTARTS(0x18); break;
        case 0xE7: CPU_RESTARTS(0x20); break;
        case 0xEF: CPU_RESTARTS(0x28); break;
        case 0xF7: CPU_RESTARTS(0x30); break;
        case 0xFF: CPU_RESTARTS(0x38); break;

        // rotates
        case 0x07:CPU_RLC(m_ContextZ80.m_RegisterAF.hi,true); break;
        case 0x0F:CPU_RRC(m_ContextZ80.m_RegisterAF.hi, true); break;
        case 0x17:CPU_RL(m_ContextZ80.m_RegisterAF.hi, true); break;
        case 0x1F:CPU_RR(m_ContextZ80.m_RegisterAF.hi, true); break;

        case 0xCB: ExecuteCBOpcode(); break;
        case 0xED: ExecuteEDOpcode(); break;
        case 0xF3: m_ContextZ80.m_IFF1 = false; m_ContextZ80.m_IFF2 = false; m_ContextZ80.m_OpcodeCycle = 4; break;

        case 0xD3: CPU_OUT_IMMEDIATE(m_ContextZ80.m_RegisterAF.hi); break;
        case 0xDB: CPU_IN_IMMEDIATE(m_ContextZ80.m_RegisterAF.hi); break;

        // exchanges
        case 0xEB: CPU_EXCHANGE(m_ContextZ80.m_RegisterDE.reg, m_ContextZ80.m_RegisterHL.reg); break;
        case 0x08: CPU_EXCHANGE(m_ContextZ80.m_RegisterAF.reg, m_ContextZ80.m_RegisterAFPrime.reg); break;

        case 0xFB: m_ContextZ80.m_EIPending = true; m_ContextZ80.m_OpcodeCycle = 4; break;

        case 0x22: CPU_LOAD_NNN(m_ContextZ80.m_RegisterHL.reg); break;

        case 0x76: m_ContextZ80.m_Halted = true; m_ContextZ80.m_OpcodeCycle = 1; break;

        case 0xE3:
        {
            BYTE nhi = m_ContextZ80.m_FuncPtrRead(m_ContextZ80.m_StackPointer.reg+1);
            BYTE nlo = m_ContextZ80.m_FuncPtrRead(m_ContextZ80.m_StackPointer.reg);
            BYTE h = m_ContextZ80.m_RegisterHL.hi;
            BYTE l = m_ContextZ80.m_RegisterHL.lo;
            m_ContextZ80.m_RegisterHL.hi = nhi;
            m_ContextZ80.m_RegisterHL.lo = nlo;
            m_ContextZ80.m_FuncPtrWrite(m_ContextZ80.m_StackPointer.reg+1, h);
            m_ContextZ80.m_FuncPtrWrite(m_ContextZ80.m_StackPointer.reg, l);
            m_ContextZ80.m_OpcodeCycle = 19;
        }
        break;


        case 0xDD: ExecuteDDFDOpcode(true); break;
        case 0xFD: ExecuteDDFDOpcode(false); break;

        case 0xD9:
        {
            WORD bcTemp = m_ContextZ80.m_RegisterBC.reg;
            WORD deTemp = m_ContextZ80.m_RegisterDE.reg;
            WORD hlTemp = m_ContextZ80.m_RegisterHL.reg;
            m_ContextZ80.m_RegisterBC.reg = m_ContextZ80.m_RegisterBCPrime.reg;
            m_ContextZ80.m_RegisterDE.reg = m_ContextZ80.m_RegisterDEPrime.reg;
            m_ContextZ80.m_RegisterHL.reg = m_ContextZ80.m_RegisterHLPrime.reg;
            m_ContextZ80.m_RegisterBCPrime.reg = bcTemp;
            m_ContextZ80.m_RegisterDEPrime.reg = deTemp;
            m_ContextZ80.m_RegisterHLPrime.reg = hlTemp;
            m_ContextZ80.m_OpcodeCycle = 4;

        } break;

        case 0x2A: CPU_REG_LOAD_NNN(m_ContextZ80.m_RegisterHL.reg); break;

        case 0x2F:
        {
            m_ContextZ80.m_OpcodeCycle = 4;
            m_ContextZ80.m_RegisterAF.hi ^= 0xFF;
            m_ContextZ80.m_RegisterAF.lo = BitSet(m_ContextZ80.m_RegisterAF.lo, FLAG_N);
            m_ContextZ80.m_RegisterAF.lo = BitSet(m_ContextZ80.m_RegisterAF.lo, FLAG_H);
//          if (TestBit(m_ContextZ80.m_RegisterAF.hi,5))
//              m_ContextZ80.m_RegisterAF.lo = BitSet(m_ContextZ80.m_RegisterAF.lo, FLAG_B5);
//          else
//              m_ContextZ80.m_RegisterAF.lo = BitReset(m_ContextZ80.m_RegisterAF.lo, FLAG_B5);
//          if (TestBit(m_ContextZ80.m_RegisterAF.hi,3))
//              m_ContextZ80.m_RegisterAF.lo = BitSet(m_ContextZ80.m_RegisterAF.lo, FLAG_B3);
//          else
//              m_ContextZ80.m_RegisterAF.lo = BitReset(m_ContextZ80.m_RegisterAF.lo, FLAG_B3);
        }
        break;

        case 0x3A:
        {
            WORD nn = ReadWord();
            m_ContextZ80.m_ProgramCounter+=2;
            m_ContextZ80.m_OpcodeCycle = 13;
            m_ContextZ80.m_RegisterAF.hi = m_ContextZ80.m_FuncPtrRead(nn);
        }break;

        case 0x32:
        {
            WORD nn = ReadWord();
            m_ContextZ80.m_ProgramCounter+=2;
            m_ContextZ80.m_FuncPtrWrite(nn, m_ContextZ80.m_RegisterAF.hi);
            m_ContextZ80.m_OpcodeCycle = 13;
        }break;

        case 0x36:
        {
            m_ContextZ80.m_OpcodeCycle = 10;
            BYTE n = m_ContextZ80.m_FuncPtrRead(m_ContextZ80.m_ProgramCounter);
            m_ContextZ80.m_ProgramCounter++;
            m_ContextZ80.m_FuncPtrWrite(m_ContextZ80.m_RegisterHL.reg, n);

        } break;

        // complement the carry flag
        case 0x3F:
        {
            m_ContextZ80.m_OpcodeCycle = 4;
            if (TestBit(m_ContextZ80.m_RegisterAF.lo, FLAG_C))
            {
                m_ContextZ80.m_RegisterAF.lo = BitReset(m_ContextZ80.m_RegisterAF.lo, FLAG_C);
                m_ContextZ80.m_RegisterAF.lo = BitSet(m_ContextZ80.m_RegisterAF.lo, FLAG_H);
            }
            else
            {
                m_ContextZ80.m_RegisterAF.lo = BitSet(m_ContextZ80.m_RegisterAF.lo, FLAG_C);
                m_ContextZ80.m_RegisterAF.lo = BitReset(m_ContextZ80.m_RegisterAF.lo, FLAG_H);
            }

            m_ContextZ80.m_RegisterAF.lo = BitReset(m_ContextZ80.m_RegisterAF.lo, FLAG_N);
        }break;

        case 0x27: CPU_DAA(); break;

        case 0x37:
        {
            m_ContextZ80.m_RegisterAF.lo = BitSet(m_ContextZ80.m_RegisterAF.lo, FLAG_C); 
            m_ContextZ80.m_OpcodeCycle = 4;
            m_ContextZ80.m_RegisterAF.lo = BitReset(m_ContextZ80.m_RegisterAF.lo, FLAG_H);
            m_ContextZ80.m_RegisterAF.lo = BitReset(m_ContextZ80.m_RegisterAF.lo, FLAG_N); 
        }
        break;

        default:
        {
            char buffer[255];
            sprintf(buffer, "Unhandled opcode %x", opcode);
            LogMessage::GetSingleton()->DoLogMessage(buffer, true);
            assert(false);
        }
        break;
    }
}

//////////////////////////////////////////////////////////////////////////////////

void Z80::ExecuteCBOpcode()
{
    IncreaseRReg();

    BYTE opcode = m_ContextZ80.m_FuncPtrRead(m_ContextZ80.m_ProgramCounter);

    LogInstInfo(opcode, "CB", false);

    m_ContextZ80.m_ProgramCounter++;




 //   char buffer[255];
//  sprintf(buffer, "Executing CB Opcode %x",opcode);
    //LogMessage::GetSingleton()->DoLogMessage(buffer,true);


    switch(opcode)
    {
        // rotate left through carry
        case 0x0 : CPU_RLC(m_ContextZ80.m_RegisterBC.hi,false); break;
        case 0x1 : CPU_RLC(m_ContextZ80.m_RegisterBC.lo,false); break;
        case 0x2 : CPU_RLC(m_ContextZ80.m_RegisterDE.hi,false); break;
        case 0x3 : CPU_RLC(m_ContextZ80.m_RegisterDE.lo,false); break;
        case 0x4 : CPU_RLC(m_ContextZ80.m_RegisterHL.hi,false); break;
        case 0x5 : CPU_RLC(m_ContextZ80.m_RegisterHL.lo,false); break;
        case 0x6 : CPU_RLC_MEMORY(m_ContextZ80.m_RegisterHL.reg,false); break;
        case 0x7 : CPU_RLC(m_ContextZ80.m_RegisterAF.hi,false); break;

        // rotate right through carry
        case 0x8 : CPU_RRC(m_ContextZ80.m_RegisterBC.hi,false); break;
        case 0x9 : CPU_RRC(m_ContextZ80.m_RegisterBC.lo,false); break;
        case 0xA : CPU_RRC(m_ContextZ80.m_RegisterDE.hi,false); break;
        case 0xB : CPU_RRC(m_ContextZ80.m_RegisterDE.lo,false); break;
        case 0xC : CPU_RRC(m_ContextZ80.m_RegisterHL.hi,false); break;
        case 0xD : CPU_RRC(m_ContextZ80.m_RegisterHL.lo,false); break;
        case 0xE : CPU_RRC_MEMORY(m_ContextZ80.m_RegisterHL.reg,false); break;
        case 0xF : CPU_RRC(m_ContextZ80.m_RegisterAF.hi,false); break;

        // rotate left
        case 0x10: CPU_RL(m_ContextZ80.m_RegisterBC.hi,false); break;
        case 0x11: CPU_RL(m_ContextZ80.m_RegisterBC.lo,false); break;
        case 0x12: CPU_RL(m_ContextZ80.m_RegisterDE.hi,false); break;
        case 0x13: CPU_RL(m_ContextZ80.m_RegisterDE.lo,false); break;
        case 0x14: CPU_RL(m_ContextZ80.m_RegisterHL.hi,false); break;
        case 0x15: CPU_RL(m_ContextZ80.m_RegisterHL.lo,false); break;
        case 0x16: CPU_RL_MEMORY(m_ContextZ80.m_RegisterHL.reg,false); break;
        case 0x17: CPU_RL(m_ContextZ80.m_RegisterAF.hi,false); break;

        // rotate right
        case 0x18: CPU_RR(m_ContextZ80.m_RegisterBC.hi,false); break;
        case 0x19: CPU_RR(m_ContextZ80.m_RegisterBC.lo,false); break;
        case 0x1A: CPU_RR(m_ContextZ80.m_RegisterDE.hi,false); break;
        case 0x1B: CPU_RR(m_ContextZ80.m_RegisterDE.lo,false); break;
        case 0x1C: CPU_RR(m_ContextZ80.m_RegisterHL.hi,false); break;
        case 0x1D: CPU_RR(m_ContextZ80.m_RegisterHL.lo,false); break;
        case 0x1E: CPU_RR_MEMORY(m_ContextZ80.m_RegisterHL.reg,false); break;
        case 0x1F: CPU_RR(m_ContextZ80.m_RegisterAF.hi,false); break;

        case 0x20 : CPU_SLA(m_ContextZ80.m_RegisterBC.hi);break;
        case 0x21 : CPU_SLA(m_ContextZ80.m_RegisterBC.lo);break;
        case 0x22 : CPU_SLA(m_ContextZ80.m_RegisterDE.hi);break;
        case 0x23 : CPU_SLA(m_ContextZ80.m_RegisterDE.lo);break;
        case 0x24 : CPU_SLA(m_ContextZ80.m_RegisterHL.hi);break;
        case 0x25 : CPU_SLA(m_ContextZ80.m_RegisterHL.lo);break;
        case 0x26 : CPU_SLA_MEMORY(m_ContextZ80.m_RegisterHL.reg);break;
        case 0x27 : CPU_SLA(m_ContextZ80.m_RegisterAF.hi);break;

        case 0x28 : CPU_SRA(m_ContextZ80.m_RegisterBC.hi); break;
        case 0x29 : CPU_SRA(m_ContextZ80.m_RegisterBC.lo); break;
        case 0x2A : CPU_SRA(m_ContextZ80.m_RegisterDE.hi); break;
        case 0x2B : CPU_SRA(m_ContextZ80.m_RegisterDE.lo); break;
        case 0x2C : CPU_SRA(m_ContextZ80.m_RegisterHL.hi); break;
        case 0x2D : CPU_SRA(m_ContextZ80.m_RegisterHL.lo); break;
        case 0x2E : CPU_SRA_MEMORY(m_ContextZ80.m_RegisterHL.reg); break;
        case 0x2F : CPU_SRA(m_ContextZ80.m_RegisterAF.hi); break;

        // shift left logical
        case 0x30 : CPU_SLL(m_ContextZ80.m_RegisterBC.hi); break;
        case 0x31 : CPU_SLL(m_ContextZ80.m_RegisterBC.lo); break;
        case 0x32 : CPU_SLL(m_ContextZ80.m_RegisterDE.hi); break;
        case 0x33 : CPU_SLL(m_ContextZ80.m_RegisterDE.lo); break;
        case 0x34 : CPU_SLL(m_ContextZ80.m_RegisterHL.hi); break;
        case 0x35 : CPU_SLL(m_ContextZ80.m_RegisterHL.lo); break;
        case 0x36 : CPU_SLL_MEMORY(m_ContextZ80.m_RegisterHL.reg); break;
        case 0x37 : CPU_SLL(m_ContextZ80.m_RegisterAF.hi); break;


        case 0x38 : CPU_SRL(m_ContextZ80.m_RegisterBC.hi); break;
        case 0x39 : CPU_SRL(m_ContextZ80.m_RegisterBC.lo); break;
        case 0x3A : CPU_SRL(m_ContextZ80.m_RegisterDE.hi); break;
        case 0x3B : CPU_SRL(m_ContextZ80.m_RegisterDE.lo); break;
        case 0x3C : CPU_SRL(m_ContextZ80.m_RegisterHL.hi); break;
        case 0x3D : CPU_SRL(m_ContextZ80.m_RegisterHL.lo); break;
        case 0x3E : CPU_SRL_MEMORY(m_ContextZ80.m_RegisterHL.reg); break;
        case 0x3F : CPU_SRL(m_ContextZ80.m_RegisterAF.hi); break;

        // test bit
        case 0x40 : CPU_TEST_BIT(m_ContextZ80.m_RegisterBC.hi, 0 , 8); break;
        case 0x41 : CPU_TEST_BIT(m_ContextZ80.m_RegisterBC.lo, 0 , 8); break;
        case 0x42 : CPU_TEST_BIT(m_ContextZ80.m_RegisterDE.hi, 0 , 8); break;
        case 0x43 : CPU_TEST_BIT(m_ContextZ80.m_RegisterDE.lo, 0 , 8); break;
        case 0x44 : CPU_TEST_BIT(m_ContextZ80.m_RegisterHL.hi, 0 , 8); break;
        case 0x45 : CPU_TEST_BIT(m_ContextZ80.m_RegisterHL.lo, 0 , 8); break;
        case 0x46 : CPU_TEST_BIT(m_ContextZ80.m_FuncPtrRead(m_ContextZ80.m_RegisterHL.reg), 0 , 12); break;
        case 0x47 : CPU_TEST_BIT(m_ContextZ80.m_RegisterAF.hi, 0 , 8); break;
        case 0x48 : CPU_TEST_BIT(m_ContextZ80.m_RegisterBC.hi, 1 , 8); break;
        case 0x49 : CPU_TEST_BIT(m_ContextZ80.m_RegisterBC.lo, 1 , 8); break;
        case 0x4A : CPU_TEST_BIT(m_ContextZ80.m_RegisterDE.hi, 1 , 8); break;
        case 0x4B : CPU_TEST_BIT(m_ContextZ80.m_RegisterDE.lo, 1 , 8); break;
        case 0x4C : CPU_TEST_BIT(m_ContextZ80.m_RegisterHL.hi, 1 , 8); break;
        case 0x4D : CPU_TEST_BIT(m_ContextZ80.m_RegisterHL.lo, 1 , 8); break;
        case 0x4E : CPU_TEST_BIT(m_ContextZ80.m_FuncPtrRead(m_ContextZ80.m_RegisterHL.reg), 1 , 12); break;
        case 0x4F : CPU_TEST_BIT(m_ContextZ80.m_RegisterAF.hi, 1 , 8); break;
        case 0x50 : CPU_TEST_BIT(m_ContextZ80.m_RegisterBC.hi, 2 , 8); break;
        case 0x51 : CPU_TEST_BIT(m_ContextZ80.m_RegisterBC.lo, 2 , 8); break;
        case 0x52 : CPU_TEST_BIT(m_ContextZ80.m_RegisterDE.hi, 2 , 8); break;
        case 0x53 : CPU_TEST_BIT(m_ContextZ80.m_RegisterDE.lo, 2 , 8); break;
        case 0x54 : CPU_TEST_BIT(m_ContextZ80.m_RegisterHL.hi, 2 , 8); break;
        case 0x55 : CPU_TEST_BIT(m_ContextZ80.m_RegisterHL.lo, 2 , 8); break;
        case 0x56 : CPU_TEST_BIT(m_ContextZ80.m_FuncPtrRead(m_ContextZ80.m_RegisterHL.reg), 2 , 12); break;
        case 0x57 : CPU_TEST_BIT(m_ContextZ80.m_RegisterAF.hi, 2 , 8); break;
        case 0x58 : CPU_TEST_BIT(m_ContextZ80.m_RegisterBC.hi, 3 , 8); break;
        case 0x59 : CPU_TEST_BIT(m_ContextZ80.m_RegisterBC.lo, 3 , 8); break;
        case 0x5A : CPU_TEST_BIT(m_ContextZ80.m_RegisterDE.hi, 3 , 8); break;
        case 0x5B : CPU_TEST_BIT(m_ContextZ80.m_RegisterDE.lo, 3 , 8); break;
        case 0x5C : CPU_TEST_BIT(m_ContextZ80.m_RegisterHL.hi, 3 , 8); break;
        case 0x5D : CPU_TEST_BIT(m_ContextZ80.m_RegisterHL.lo, 3 , 8); break;
        case 0x5E : CPU_TEST_BIT(m_ContextZ80.m_FuncPtrRead(m_ContextZ80.m_RegisterHL.reg), 3 , 12); break;
        case 0x5F : CPU_TEST_BIT(m_ContextZ80.m_RegisterAF.hi, 3 , 8); break;
        case 0x60 : CPU_TEST_BIT(m_ContextZ80.m_RegisterBC.hi, 4 , 8); break;
        case 0x61 : CPU_TEST_BIT(m_ContextZ80.m_RegisterBC.lo, 4 , 8); break;
        case 0x62 : CPU_TEST_BIT(m_ContextZ80.m_RegisterDE.hi, 4 , 8); break;
        case 0x63 : CPU_TEST_BIT(m_ContextZ80.m_RegisterDE.lo, 4 , 8); break;
        case 0x64 : CPU_TEST_BIT(m_ContextZ80.m_RegisterHL.hi, 4 , 8); break;
        case 0x65 : CPU_TEST_BIT(m_ContextZ80.m_RegisterHL.lo, 4 , 8); break;
        case 0x66 : CPU_TEST_BIT(m_ContextZ80.m_FuncPtrRead(m_ContextZ80.m_RegisterHL.reg), 4 , 12); break;
        case 0x67 : CPU_TEST_BIT(m_ContextZ80.m_RegisterAF.hi, 4 , 8); break;
        case 0x68 : CPU_TEST_BIT(m_ContextZ80.m_RegisterBC.hi, 5 , 8); break;
        case 0x69 : CPU_TEST_BIT(m_ContextZ80.m_RegisterBC.lo, 5 , 8); break;
        case 0x6A : CPU_TEST_BIT(m_ContextZ80.m_RegisterDE.hi, 5 , 8); break;
        case 0x6B : CPU_TEST_BIT(m_ContextZ80.m_RegisterDE.lo, 5 , 8); break;
        case 0x6C : CPU_TEST_BIT(m_ContextZ80.m_RegisterHL.hi, 5 , 8); break;
        case 0x6D : CPU_TEST_BIT(m_ContextZ80.m_RegisterHL.lo, 5 , 8); break;
        case 0x6E : CPU_TEST_BIT(m_ContextZ80.m_FuncPtrRead(m_ContextZ80.m_RegisterHL.reg), 5 , 12); break;
        case 0x6F : CPU_TEST_BIT(m_ContextZ80.m_RegisterAF.hi, 5 , 8); break;
        case 0x70 : CPU_TEST_BIT(m_ContextZ80.m_RegisterBC.hi, 6 , 8); break;
        case 0x71 : CPU_TEST_BIT(m_ContextZ80.m_RegisterBC.lo, 6 , 8); break;
        case 0x72 : CPU_TEST_BIT(m_ContextZ80.m_RegisterDE.hi, 6 , 8); break;
        case 0x73 : CPU_TEST_BIT(m_ContextZ80.m_RegisterDE.lo, 6 , 8); break;
        case 0x74 : CPU_TEST_BIT(m_ContextZ80.m_RegisterHL.hi, 6 , 8); break;
        case 0x75 : CPU_TEST_BIT(m_ContextZ80.m_RegisterHL.lo, 6 , 8); break;
        case 0x76 : CPU_TEST_BIT(m_ContextZ80.m_FuncPtrRead(m_ContextZ80.m_RegisterHL.reg), 6 , 12); break;
        case 0x77 : CPU_TEST_BIT(m_ContextZ80.m_RegisterAF.hi, 6 , 8); break;
        case 0x78 : CPU_TEST_BIT(m_ContextZ80.m_RegisterBC.hi, 7 , 8); break;
        case 0x79 : CPU_TEST_BIT(m_ContextZ80.m_RegisterBC.lo, 7 , 8); break;
        case 0x7A : CPU_TEST_BIT(m_ContextZ80.m_RegisterDE.hi, 7 , 8); break;
        case 0x7B : CPU_TEST_BIT(m_ContextZ80.m_RegisterDE.lo, 7 , 8); break;
        case 0x7C : CPU_TEST_BIT(m_ContextZ80.m_RegisterHL.hi, 7 , 8); break;
        case 0x7D : CPU_TEST_BIT(m_ContextZ80.m_RegisterHL.lo, 7 , 8); break;
        case 0x7E : CPU_TEST_BIT(m_ContextZ80.m_FuncPtrRead(m_ContextZ80.m_RegisterHL.reg), 7 , 12); break;
        case 0x7F : CPU_TEST_BIT(m_ContextZ80.m_RegisterAF.hi, 7 , 8); break;

        // reset bit
        case 0x80 : CPU_RESET_BIT(m_ContextZ80.m_RegisterBC.hi, 0); break;
        case 0x81 : CPU_RESET_BIT(m_ContextZ80.m_RegisterBC.lo, 0); break;
        case 0x82 : CPU_RESET_BIT(m_ContextZ80.m_RegisterDE.hi, 0); break;
        case 0x83 : CPU_RESET_BIT(m_ContextZ80.m_RegisterDE.lo, 0); break;
        case 0x84 : CPU_RESET_BIT(m_ContextZ80.m_RegisterHL.hi, 0); break;
        case 0x85 : CPU_RESET_BIT(m_ContextZ80.m_RegisterHL.lo, 0); break;
        case 0x86 : CPU_RESET_BIT_MEMORY(m_ContextZ80.m_RegisterHL.reg, 0); break;
        case 0x87 : CPU_RESET_BIT(m_ContextZ80.m_RegisterAF.hi, 0); break;
        case 0x88 : CPU_RESET_BIT(m_ContextZ80.m_RegisterBC.hi, 1 ); break;
        case 0x89 : CPU_RESET_BIT(m_ContextZ80.m_RegisterBC.lo, 1); break;
        case 0x8A : CPU_RESET_BIT(m_ContextZ80.m_RegisterDE.hi, 1); break;
        case 0x8B : CPU_RESET_BIT(m_ContextZ80.m_RegisterDE.lo, 1); break;
        case 0x8C : CPU_RESET_BIT(m_ContextZ80.m_RegisterHL.hi, 1); break;
        case 0x8D : CPU_RESET_BIT(m_ContextZ80.m_RegisterHL.lo, 1); break;
        case 0x8E : CPU_RESET_BIT_MEMORY(m_ContextZ80.m_RegisterHL.reg, 1); break;
        case 0x8F : CPU_RESET_BIT(m_ContextZ80.m_RegisterAF.hi, 1 ); break;
        case 0x90 : CPU_RESET_BIT(m_ContextZ80.m_RegisterBC.hi, 2 ); break;
        case 0x91 : CPU_RESET_BIT(m_ContextZ80.m_RegisterBC.lo, 2 ); break;
        case 0x92 : CPU_RESET_BIT(m_ContextZ80.m_RegisterDE.hi, 2 ); break;
        case 0x93 : CPU_RESET_BIT(m_ContextZ80.m_RegisterDE.lo, 2 ); break;
        case 0x94 : CPU_RESET_BIT(m_ContextZ80.m_RegisterHL.hi, 2 ); break;
        case 0x95 : CPU_RESET_BIT(m_ContextZ80.m_RegisterHL.lo, 2 ); break;
        case 0x96 : CPU_RESET_BIT_MEMORY(m_ContextZ80.m_RegisterHL.reg, 2); break;
        case 0x97 : CPU_RESET_BIT(m_ContextZ80.m_RegisterAF.hi, 2 ); break;
        case 0x98 : CPU_RESET_BIT(m_ContextZ80.m_RegisterBC.hi, 3 ); break;
        case 0x99 : CPU_RESET_BIT(m_ContextZ80.m_RegisterBC.lo, 3 ); break;
        case 0x9A : CPU_RESET_BIT(m_ContextZ80.m_RegisterDE.hi, 3 ); break;
        case 0x9B : CPU_RESET_BIT(m_ContextZ80.m_RegisterDE.lo, 3 ); break;
        case 0x9C : CPU_RESET_BIT(m_ContextZ80.m_RegisterHL.hi, 3 ); break;
        case 0x9D : CPU_RESET_BIT(m_ContextZ80.m_RegisterHL.lo, 3 ); break;
        case 0x9E : CPU_RESET_BIT_MEMORY(m_ContextZ80.m_RegisterHL.reg, 3 ); break;
        case 0x9F : CPU_RESET_BIT(m_ContextZ80.m_RegisterAF.hi, 3 ); break;
        case 0xA0 : CPU_RESET_BIT(m_ContextZ80.m_RegisterBC.hi, 4 ); break;
        case 0xA1 : CPU_RESET_BIT(m_ContextZ80.m_RegisterBC.lo, 4 ); break;
        case 0xA2 : CPU_RESET_BIT(m_ContextZ80.m_RegisterDE.hi, 4 ); break;
        case 0xA3 : CPU_RESET_BIT(m_ContextZ80.m_RegisterDE.lo, 4 ); break;
        case 0xA4 : CPU_RESET_BIT(m_ContextZ80.m_RegisterHL.hi, 4 ); break;
        case 0xA5 : CPU_RESET_BIT(m_ContextZ80.m_RegisterHL.lo, 4 ); break;
        case 0xA6 : CPU_RESET_BIT_MEMORY(m_ContextZ80.m_RegisterHL.reg, 4); break;
        case 0xA7 : CPU_RESET_BIT(m_ContextZ80.m_RegisterAF.hi, 4); break;
        case 0xA8 : CPU_RESET_BIT(m_ContextZ80.m_RegisterBC.hi, 5); break;
        case 0xA9 : CPU_RESET_BIT(m_ContextZ80.m_RegisterBC.lo, 5); break;
        case 0xAA : CPU_RESET_BIT(m_ContextZ80.m_RegisterDE.hi, 5); break;
        case 0xAB : CPU_RESET_BIT(m_ContextZ80.m_RegisterDE.lo, 5); break;
        case 0xAC : CPU_RESET_BIT(m_ContextZ80.m_RegisterHL.hi, 5); break;
        case 0xAD : CPU_RESET_BIT(m_ContextZ80.m_RegisterHL.lo, 5); break;
        case 0xAE : CPU_RESET_BIT_MEMORY(m_ContextZ80.m_RegisterHL.reg, 5); break;
        case 0xAF : CPU_RESET_BIT(m_ContextZ80.m_RegisterAF.hi, 5 ); break;
        case 0xB0 : CPU_RESET_BIT(m_ContextZ80.m_RegisterBC.hi, 6 ); break;
        case 0xB1 : CPU_RESET_BIT(m_ContextZ80.m_RegisterBC.lo, 6 ); break;
        case 0xB2 : CPU_RESET_BIT(m_ContextZ80.m_RegisterDE.hi, 6 ); break;
        case 0xB3 : CPU_RESET_BIT(m_ContextZ80.m_RegisterDE.lo, 6 ); break;
        case 0xB4 : CPU_RESET_BIT(m_ContextZ80.m_RegisterHL.hi, 6 ); break;
        case 0xB5 : CPU_RESET_BIT(m_ContextZ80.m_RegisterHL.lo, 6 ); break;
        case 0xB6 : CPU_RESET_BIT_MEMORY(m_ContextZ80.m_RegisterHL.reg, 6); break;
        case 0xB7 : CPU_RESET_BIT(m_ContextZ80.m_RegisterAF.hi, 6 ); break;
        case 0xB8 : CPU_RESET_BIT(m_ContextZ80.m_RegisterBC.hi, 7 ); break;
        case 0xB9 : CPU_RESET_BIT(m_ContextZ80.m_RegisterBC.lo, 7 ); break;
        case 0xBA : CPU_RESET_BIT(m_ContextZ80.m_RegisterDE.hi, 7 ); break;
        case 0xBB : CPU_RESET_BIT(m_ContextZ80.m_RegisterDE.lo, 7 ); break;
        case 0xBC : CPU_RESET_BIT(m_ContextZ80.m_RegisterHL.hi, 7 ); break;
        case 0xBD : CPU_RESET_BIT(m_ContextZ80.m_RegisterHL.lo, 7 ); break;
        case 0xBE : CPU_RESET_BIT_MEMORY(m_ContextZ80.m_RegisterHL.reg, 7); break;
        case 0xBF : CPU_RESET_BIT(m_ContextZ80.m_RegisterAF.hi, 7); break;


        // set bit
        case 0xC0 : CPU_SET_BIT(m_ContextZ80.m_RegisterBC.hi, 0); break;
        case 0xC1 : CPU_SET_BIT(m_ContextZ80.m_RegisterBC.lo, 0); break;
        case 0xC2 : CPU_SET_BIT(m_ContextZ80.m_RegisterDE.hi, 0); break;
        case 0xC3 : CPU_SET_BIT(m_ContextZ80.m_RegisterDE.lo, 0); break;
        case 0xC4 : CPU_SET_BIT(m_ContextZ80.m_RegisterHL.hi, 0); break;
        case 0xC5 : CPU_SET_BIT(m_ContextZ80.m_RegisterHL.lo, 0); break;
        case 0xC6 : CPU_SET_BIT_MEMORY(m_ContextZ80.m_RegisterHL.reg, 0); break;
        case 0xC7 : CPU_SET_BIT(m_ContextZ80.m_RegisterAF.hi, 0); break;
        case 0xC8 : CPU_SET_BIT(m_ContextZ80.m_RegisterBC.hi, 1 ); break;
        case 0xC9 : CPU_SET_BIT(m_ContextZ80.m_RegisterBC.lo, 1); break;
        case 0xCA : CPU_SET_BIT(m_ContextZ80.m_RegisterDE.hi, 1); break;
        case 0xCB : CPU_SET_BIT(m_ContextZ80.m_RegisterDE.lo, 1); break;
        case 0xCC : CPU_SET_BIT(m_ContextZ80.m_RegisterHL.hi, 1); break;
        case 0xCD : CPU_SET_BIT(m_ContextZ80.m_RegisterHL.lo, 1); break;
        case 0xCE : CPU_SET_BIT_MEMORY(m_ContextZ80.m_RegisterHL.reg, 1); break;
        case 0xCF : CPU_SET_BIT(m_ContextZ80.m_RegisterAF.hi, 1 ); break;
        case 0xD0 : CPU_SET_BIT(m_ContextZ80.m_RegisterBC.hi, 2 ); break;
        case 0xD1 : CPU_SET_BIT(m_ContextZ80.m_RegisterBC.lo, 2 ); break;
        case 0xD2 : CPU_SET_BIT(m_ContextZ80.m_RegisterDE.hi, 2 ); break;
        case 0xD3 : CPU_SET_BIT(m_ContextZ80.m_RegisterDE.lo, 2 ); break;
        case 0xD4 : CPU_SET_BIT(m_ContextZ80.m_RegisterHL.hi, 2 ); break;
        case 0xD5 : CPU_SET_BIT(m_ContextZ80.m_RegisterHL.lo, 2 ); break;
        case 0xD6 : CPU_SET_BIT_MEMORY(m_ContextZ80.m_RegisterHL.reg, 2); break;
        case 0xD7 : CPU_SET_BIT(m_ContextZ80.m_RegisterAF.hi, 2 ); break;
        case 0xD8 : CPU_SET_BIT(m_ContextZ80.m_RegisterBC.hi, 3 ); break;
        case 0xD9 : CPU_SET_BIT(m_ContextZ80.m_RegisterBC.lo, 3 ); break;
        case 0xDA : CPU_SET_BIT(m_ContextZ80.m_RegisterDE.hi, 3 ); break;
        case 0xDB : CPU_SET_BIT(m_ContextZ80.m_RegisterDE.lo, 3 ); break;
        case 0xDC : CPU_SET_BIT(m_ContextZ80.m_RegisterHL.hi, 3 ); break;
        case 0xDD : CPU_SET_BIT(m_ContextZ80.m_RegisterHL.lo, 3 ); break;
        case 0xDE : CPU_SET_BIT_MEMORY(m_ContextZ80.m_RegisterHL.reg, 3 ); break;
        case 0xDF : CPU_SET_BIT(m_ContextZ80.m_RegisterAF.hi, 3 ); break;
        case 0xE0 : CPU_SET_BIT(m_ContextZ80.m_RegisterBC.hi, 4 ); break;
        case 0xE1 : CPU_SET_BIT(m_ContextZ80.m_RegisterBC.lo, 4 ); break;
        case 0xE2 : CPU_SET_BIT(m_ContextZ80.m_RegisterDE.hi, 4 ); break;
        case 0xE3 : CPU_SET_BIT(m_ContextZ80.m_RegisterDE.lo, 4 ); break;
        case 0xE4 : CPU_SET_BIT(m_ContextZ80.m_RegisterHL.hi, 4 ); break;
        case 0xE5 : CPU_SET_BIT(m_ContextZ80.m_RegisterHL.lo, 4 ); break;
        case 0xE6 : CPU_SET_BIT_MEMORY(m_ContextZ80.m_RegisterHL.reg, 4); break;
        case 0xE7 : CPU_SET_BIT(m_ContextZ80.m_RegisterAF.hi, 4); break;
        case 0xE8 : CPU_SET_BIT(m_ContextZ80.m_RegisterBC.hi, 5); break;
        case 0xE9 : CPU_SET_BIT(m_ContextZ80.m_RegisterBC.lo, 5); break;
        case 0xEA : CPU_SET_BIT(m_ContextZ80.m_RegisterDE.hi, 5); break;
        case 0xEB : CPU_SET_BIT(m_ContextZ80.m_RegisterDE.lo, 5); break;
        case 0xEC : CPU_SET_BIT(m_ContextZ80.m_RegisterHL.hi, 5); break;
        case 0xED : CPU_SET_BIT(m_ContextZ80.m_RegisterHL.lo, 5); break;
        case 0xEE : CPU_SET_BIT_MEMORY(m_ContextZ80.m_RegisterHL.reg, 5); break;
        case 0xEF : CPU_SET_BIT(m_ContextZ80.m_RegisterAF.hi, 5 ); break;
        case 0xF0 : CPU_SET_BIT(m_ContextZ80.m_RegisterBC.hi, 6 ); break;
        case 0xF1 : CPU_SET_BIT(m_ContextZ80.m_RegisterBC.lo, 6 ); break;
        case 0xF2 : CPU_SET_BIT(m_ContextZ80.m_RegisterDE.hi, 6 ); break;
        case 0xF3 : CPU_SET_BIT(m_ContextZ80.m_RegisterDE.lo, 6 ); break;
        case 0xF4 : CPU_SET_BIT(m_ContextZ80.m_RegisterHL.hi, 6 ); break;
        case 0xF5 : CPU_SET_BIT(m_ContextZ80.m_RegisterHL.lo, 6 ); break;
        case 0xF6 : CPU_SET_BIT_MEMORY(m_ContextZ80.m_RegisterHL.reg, 6); break;
        case 0xF7 : CPU_SET_BIT(m_ContextZ80.m_RegisterAF.hi, 6); break;
        case 0xF8 : CPU_SET_BIT(m_ContextZ80.m_RegisterBC.hi, 7 ); break;
        case 0xF9 : CPU_SET_BIT(m_ContextZ80.m_RegisterBC.lo, 7 ); break;
        case 0xFA : CPU_SET_BIT(m_ContextZ80.m_RegisterDE.hi, 7 ); break;
        case 0xFB : CPU_SET_BIT(m_ContextZ80.m_RegisterDE.lo, 7 ); break;
        case 0xFC : CPU_SET_BIT(m_ContextZ80.m_RegisterHL.hi, 7 ); break;
        case 0xFD : CPU_SET_BIT(m_ContextZ80.m_RegisterHL.lo, 7 ); break;
        case 0xFE : CPU_SET_BIT_MEMORY(m_ContextZ80.m_RegisterHL.reg, 7); break;
        case 0xFF : CPU_SET_BIT(m_ContextZ80.m_RegisterAF.hi, 7); break;

        default:
        {
            char buffer[255];
            sprintf(buffer, "Unhandled CB opcode %x", opcode);
            LogMessage::GetSingleton()->DoLogMessage(buffer, true);
            assert(false);
        }
        break;
    }
}

//////////////////////////////////////////////////////////////////////////////////

void Z80::ExecuteDDFDCBOpcode(bool isDD)
{

    SIGNED_BYTE displacement = (SIGNED_BYTE)m_ContextZ80.m_FuncPtrRead(m_ContextZ80.m_ProgramCounter);

    LogInstInfo(displacement, "DDFDCB displacement", false);

    m_ContextZ80.m_ProgramCounter++;


    BYTE opcode = m_ContextZ80.m_FuncPtrRead(m_ContextZ80.m_ProgramCounter);

    LogInstInfo(opcode, "DDFDCB opcode", false);

    m_ContextZ80.m_ProgramCounter++;

    REGISTERZ80& reg = (isDD) ? m_ContextZ80.m_RegisterIX : m_ContextZ80.m_RegisterIY;

    switch(opcode)
    {
        case 0x0 : CPU_DDFD_RLC(m_ContextZ80.m_RegisterBC.hi, reg.reg, displacement); break;
        case 0x1 : CPU_DDFD_RLC(m_ContextZ80.m_RegisterBC.lo, reg.reg, displacement); break;
        case 0x2 : CPU_DDFD_RLC(m_ContextZ80.m_RegisterDE.hi, reg.reg, displacement); break;
        case 0x3 : CPU_DDFD_RLC(m_ContextZ80.m_RegisterDE.lo, reg.reg, displacement); break;
        case 0x4 : CPU_DDFD_RLC(m_ContextZ80.m_RegisterHL.hi, reg.reg, displacement); break;
        case 0x5 : CPU_DDFD_RLC(m_ContextZ80.m_RegisterHL.lo, reg.reg, displacement); break;
        case 0x6 : CPU_RLC_MEMORY(reg.reg + displacement,false); m_ContextZ80.m_OpcodeCycle =23; break;
        case 0x7 : CPU_DDFD_RLC(m_ContextZ80.m_RegisterAF.hi, reg.reg, displacement); break;

        // rotate right through carry
        case 0x8 : CPU_DDFD_RRC(m_ContextZ80.m_RegisterBC.hi, reg.reg, displacement); break;
        case 0x9 : CPU_DDFD_RRC(m_ContextZ80.m_RegisterBC.lo, reg.reg, displacement); break;
        case 0xA : CPU_DDFD_RRC(m_ContextZ80.m_RegisterDE.hi, reg.reg, displacement); break;
        case 0xB : CPU_DDFD_RRC(m_ContextZ80.m_RegisterDE.lo, reg.reg, displacement); break;
        case 0xC : CPU_DDFD_RRC(m_ContextZ80.m_RegisterHL.hi, reg.reg, displacement); break;
        case 0xD : CPU_DDFD_RRC(m_ContextZ80.m_RegisterHL.lo, reg.reg, displacement); break;
        case 0xE : CPU_RRC_MEMORY(reg.reg + displacement,false); m_ContextZ80.m_OpcodeCycle = 23; break;
        case 0xF : CPU_DDFD_RRC(m_ContextZ80.m_RegisterAF.hi, reg.reg, displacement); break;

        // rotate left
        case 0x10: CPU_DDFD_RL(m_ContextZ80.m_RegisterBC.hi, reg.reg, displacement); break;
        case 0x11: CPU_DDFD_RL(m_ContextZ80.m_RegisterBC.lo, reg.reg, displacement); break;
        case 0x12: CPU_DDFD_RL(m_ContextZ80.m_RegisterDE.hi, reg.reg, displacement); break;
        case 0x13: CPU_DDFD_RL(m_ContextZ80.m_RegisterDE.lo, reg.reg, displacement); break;
        case 0x14: CPU_DDFD_RL(m_ContextZ80.m_RegisterHL.hi, reg.reg, displacement); break;
        case 0x15: CPU_DDFD_RL(m_ContextZ80.m_RegisterHL.lo, reg.reg, displacement); break;
        case 0x16: CPU_RL_MEMORY(reg.reg+displacement,false); m_ContextZ80.m_OpcodeCycle=23; break;
        case 0x17: CPU_DDFD_RL(m_ContextZ80.m_RegisterAF.hi, reg.reg, displacement); break;

        // rotate right
        case 0x18: CPU_DDFD_RR(m_ContextZ80.m_RegisterBC.hi, reg.reg, displacement); break;
        case 0x19: CPU_DDFD_RR(m_ContextZ80.m_RegisterBC.lo, reg.reg, displacement); break;
        case 0x1A: CPU_DDFD_RR(m_ContextZ80.m_RegisterDE.hi, reg.reg, displacement); break;
        case 0x1B: CPU_DDFD_RR(m_ContextZ80.m_RegisterDE.lo, reg.reg, displacement); break;
        case 0x1C: CPU_DDFD_RR(m_ContextZ80.m_RegisterHL.hi, reg.reg, displacement); break;
        case 0x1D: CPU_DDFD_RR(m_ContextZ80.m_RegisterHL.lo, reg.reg, displacement); break;
        case 0x1E: CPU_RR_MEMORY(reg.reg + displacement,false); m_ContextZ80.m_OpcodeCycle=23;break;
        case 0x1F: CPU_DDFD_RR(m_ContextZ80.m_RegisterAF.hi, reg.reg, displacement); break;

        case 0x20 : CPU_DDFD_SLA(m_ContextZ80.m_RegisterBC.hi, reg.reg, displacement);break;
        case 0x21 : CPU_DDFD_SLA(m_ContextZ80.m_RegisterBC.lo, reg.reg, displacement);break;
        case 0x22 : CPU_DDFD_SLA(m_ContextZ80.m_RegisterDE.hi, reg.reg, displacement);break;
        case 0x23 : CPU_DDFD_SLA(m_ContextZ80.m_RegisterDE.lo, reg.reg, displacement);break;
        case 0x24 : CPU_DDFD_SLA(m_ContextZ80.m_RegisterHL.hi, reg.reg, displacement);break;
        case 0x25 : CPU_DDFD_SLA(m_ContextZ80.m_RegisterHL.lo, reg.reg, displacement);break;
        case 0x26 : CPU_SLA_MEMORY(reg.reg + displacement); m_ContextZ80.m_OpcodeCycle=23;break;
        case 0x27 : CPU_DDFD_SLA(m_ContextZ80.m_RegisterAF.hi, reg.reg, displacement);break;

        case 0x28 : CPU_DDFD_SRA(m_ContextZ80.m_RegisterBC.hi, reg.reg, displacement); break;
        case 0x29 : CPU_DDFD_SRA(m_ContextZ80.m_RegisterBC.lo, reg.reg, displacement); break;
        case 0x2A : CPU_DDFD_SRA(m_ContextZ80.m_RegisterDE.hi, reg.reg, displacement); break;
        case 0x2B : CPU_DDFD_SRA(m_ContextZ80.m_RegisterDE.lo, reg.reg, displacement); break;
        case 0x2C : CPU_DDFD_SRA(m_ContextZ80.m_RegisterHL.hi, reg.reg, displacement); break;
        case 0x2D : CPU_DDFD_SRA(m_ContextZ80.m_RegisterHL.lo, reg.reg, displacement); break;
        case 0x2E : CPU_SRA_MEMORY(reg.reg + displacement);m_ContextZ80.m_OpcodeCycle=23; break;
        case 0x2F : CPU_DDFD_SRA(m_ContextZ80.m_RegisterAF.hi, reg.reg, displacement); break;

        // shift left logical
        case 0x30 : CPU_DDFD_SLL(m_ContextZ80.m_RegisterBC.hi, reg.reg,displacement); break;
        case 0x31 : CPU_DDFD_SLL(m_ContextZ80.m_RegisterBC.lo, reg.reg,displacement); break;
        case 0x32 : CPU_DDFD_SLL(m_ContextZ80.m_RegisterDE.hi, reg.reg,displacement); break;
        case 0x33 : CPU_DDFD_SLL(m_ContextZ80.m_RegisterDE.lo, reg.reg,displacement); break;
        case 0x34 : CPU_DDFD_SLL(m_ContextZ80.m_RegisterHL.hi, reg.reg,displacement); break;
        case 0x35 : CPU_DDFD_SLL(m_ContextZ80.m_RegisterHL.lo, reg.reg,displacement); break;
        case 0x36 : CPU_SLL_MEMORY(reg.reg + displacement); m_ContextZ80.m_OpcodeCycle = 23; break;
        case 0x37 : CPU_DDFD_SLL(m_ContextZ80.m_RegisterAF.hi, reg.reg,displacement); break;


        case 0x38 : CPU_DDFD_SRL(m_ContextZ80.m_RegisterBC.hi, reg.reg, displacement); break;
        case 0x39 : CPU_DDFD_SRL(m_ContextZ80.m_RegisterBC.lo, reg.reg, displacement); break;
        case 0x3A : CPU_DDFD_SRL(m_ContextZ80.m_RegisterDE.hi, reg.reg, displacement); break;
        case 0x3B : CPU_DDFD_SRL(m_ContextZ80.m_RegisterDE.lo, reg.reg, displacement); break;
        case 0x3C : CPU_DDFD_SRL(m_ContextZ80.m_RegisterHL.hi, reg.reg, displacement); break;
        case 0x3D : CPU_DDFD_SRL(m_ContextZ80.m_RegisterHL.lo, reg.reg, displacement); break;
        case 0x3E : CPU_SRL_MEMORY(reg.reg + displacement); m_ContextZ80.m_OpcodeCycle=23; break;
        case 0x3F : CPU_DDFD_SRL(m_ContextZ80.m_RegisterAF.hi, reg.reg, displacement); break;


        
        



        // test bit
        case 0x40 : CPU_DDFD_TEST_BIT(m_ContextZ80.m_RegisterBC.hi, 0 ,   reg.reg, displacement); break;
        case 0x41 : CPU_DDFD_TEST_BIT(m_ContextZ80.m_RegisterBC.lo, 0 ,   reg.reg, displacement); break;
        case 0x42 : CPU_DDFD_TEST_BIT(m_ContextZ80.m_RegisterDE.hi, 0 ,   reg.reg, displacement); break;
        case 0x43 : CPU_DDFD_TEST_BIT(m_ContextZ80.m_RegisterDE.lo, 0 ,   reg.reg, displacement); break;
        case 0x44 : CPU_DDFD_TEST_BIT(m_ContextZ80.m_RegisterHL.hi, 0 ,   reg.reg, displacement); break;
        case 0x45 : CPU_DDFD_TEST_BIT(m_ContextZ80.m_RegisterHL.lo, 0 ,   reg.reg, displacement); break;
        case 0x46 : CPU_TEST_BIT(m_ContextZ80.m_FuncPtrRead(reg.reg+displacement), 0 , 4); m_ContextZ80.m_OpcodeCycle = 20; break;
        case 0x47 : CPU_DDFD_TEST_BIT(m_ContextZ80.m_RegisterAF.hi, 0 ,   reg.reg, displacement); break;
        case 0x48 : CPU_DDFD_TEST_BIT(m_ContextZ80.m_RegisterBC.hi, 1 ,   reg.reg, displacement); break;
        case 0x49 : CPU_DDFD_TEST_BIT(m_ContextZ80.m_RegisterBC.lo, 1 ,   reg.reg, displacement); break;
        case 0x4A : CPU_DDFD_TEST_BIT(m_ContextZ80.m_RegisterDE.hi, 1 ,   reg.reg, displacement); break;
        case 0x4B : CPU_DDFD_TEST_BIT(m_ContextZ80.m_RegisterDE.lo, 1 ,   reg.reg, displacement); break;
        case 0x4C : CPU_DDFD_TEST_BIT(m_ContextZ80.m_RegisterHL.hi, 1 ,   reg.reg, displacement); break;
        case 0x4D : CPU_DDFD_TEST_BIT(m_ContextZ80.m_RegisterHL.lo, 1 ,   reg.reg, displacement); break;
        case 0x4E : CPU_TEST_BIT(m_ContextZ80.m_FuncPtrRead(reg.reg+displacement), 1 , 4); m_ContextZ80.m_OpcodeCycle = 20; break;
        case 0x4F : CPU_DDFD_TEST_BIT(m_ContextZ80.m_RegisterAF.hi, 1 ,   reg.reg, displacement); break;
        case 0x50 : CPU_DDFD_TEST_BIT(m_ContextZ80.m_RegisterBC.hi, 2 ,   reg.reg, displacement); break;
        case 0x51 : CPU_DDFD_TEST_BIT(m_ContextZ80.m_RegisterBC.lo, 2 ,   reg.reg, displacement); break;
        case 0x52 : CPU_DDFD_TEST_BIT(m_ContextZ80.m_RegisterDE.hi, 2 ,   reg.reg, displacement); break;
        case 0x53 : CPU_DDFD_TEST_BIT(m_ContextZ80.m_RegisterDE.lo, 2 ,   reg.reg, displacement); break;
        case 0x54 : CPU_DDFD_TEST_BIT(m_ContextZ80.m_RegisterHL.hi, 2 ,   reg.reg, displacement); break;
        case 0x55 : CPU_DDFD_TEST_BIT(m_ContextZ80.m_RegisterHL.lo, 2 ,   reg.reg, displacement); break;
        case 0x56 : CPU_TEST_BIT(m_ContextZ80.m_FuncPtrRead(reg.reg+displacement), 2 , 4); m_ContextZ80.m_OpcodeCycle = 20;  break;
        case 0x57 : CPU_DDFD_TEST_BIT(m_ContextZ80.m_RegisterAF.hi, 2 ,   reg.reg, displacement); break;
        case 0x58 : CPU_DDFD_TEST_BIT(m_ContextZ80.m_RegisterBC.hi, 3 ,   reg.reg, displacement); break;
        case 0x59 : CPU_DDFD_TEST_BIT(m_ContextZ80.m_RegisterBC.lo, 3 ,   reg.reg, displacement); break;
        case 0x5A : CPU_DDFD_TEST_BIT(m_ContextZ80.m_RegisterDE.hi, 3 ,   reg.reg, displacement); break;
        case 0x5B : CPU_DDFD_TEST_BIT(m_ContextZ80.m_RegisterDE.lo, 3 ,   reg.reg, displacement); break;
        case 0x5C : CPU_DDFD_TEST_BIT(m_ContextZ80.m_RegisterHL.hi, 3 ,   reg.reg, displacement); break;
        case 0x5D : CPU_DDFD_TEST_BIT(m_ContextZ80.m_RegisterHL.lo, 3 ,   reg.reg, displacement); break;
        case 0x5E : CPU_TEST_BIT(m_ContextZ80.m_FuncPtrRead(reg.reg+displacement), 3 , 4); m_ContextZ80.m_OpcodeCycle = 20; break;
        case 0x5F : CPU_DDFD_TEST_BIT(m_ContextZ80.m_RegisterAF.hi, 3 ,   reg.reg, displacement); break;
        case 0x60 : CPU_DDFD_TEST_BIT(m_ContextZ80.m_RegisterBC.hi, 4 ,   reg.reg, displacement); break;
        case 0x61 : CPU_DDFD_TEST_BIT(m_ContextZ80.m_RegisterBC.lo, 4 ,   reg.reg, displacement); break;
        case 0x62 : CPU_DDFD_TEST_BIT(m_ContextZ80.m_RegisterDE.hi, 4 ,   reg.reg, displacement); break;
        case 0x63 : CPU_DDFD_TEST_BIT(m_ContextZ80.m_RegisterDE.lo, 4 ,   reg.reg, displacement); break;
        case 0x64 : CPU_DDFD_TEST_BIT(m_ContextZ80.m_RegisterHL.hi, 4 ,   reg.reg, displacement); break;
        case 0x65 : CPU_DDFD_TEST_BIT(m_ContextZ80.m_RegisterHL.lo, 4 ,   reg.reg, displacement); break;
        case 0x66 : CPU_TEST_BIT(m_ContextZ80.m_FuncPtrRead(reg.reg+displacement), 4 , 4); m_ContextZ80.m_OpcodeCycle = 20; break;
        case 0x67 : CPU_DDFD_TEST_BIT(m_ContextZ80.m_RegisterAF.hi, 4 ,   reg.reg, displacement); break;
        case 0x68 : CPU_DDFD_TEST_BIT(m_ContextZ80.m_RegisterBC.hi, 5 ,   reg.reg, displacement); break;
        case 0x69 : CPU_DDFD_TEST_BIT(m_ContextZ80.m_RegisterBC.lo, 5 ,   reg.reg, displacement); break;
        case 0x6A : CPU_DDFD_TEST_BIT(m_ContextZ80.m_RegisterDE.hi, 5 ,   reg.reg, displacement); break;
        case 0x6B : CPU_DDFD_TEST_BIT(m_ContextZ80.m_RegisterDE.lo, 5 ,   reg.reg, displacement); break;
        case 0x6C : CPU_DDFD_TEST_BIT(m_ContextZ80.m_RegisterHL.hi, 5 ,   reg.reg, displacement); break;
        case 0x6D : CPU_DDFD_TEST_BIT(m_ContextZ80.m_RegisterHL.lo, 5 ,   reg.reg, displacement); break;
        case 0x6E : CPU_TEST_BIT(m_ContextZ80.m_FuncPtrRead(reg.reg+displacement), 5 , 4); m_ContextZ80.m_OpcodeCycle = 20; break;
        case 0x6F : CPU_DDFD_TEST_BIT(m_ContextZ80.m_RegisterAF.hi, 5 ,   reg.reg, displacement); break;
        case 0x70 : CPU_DDFD_TEST_BIT(m_ContextZ80.m_RegisterBC.hi, 6 ,   reg.reg, displacement); break;
        case 0x71 : CPU_DDFD_TEST_BIT(m_ContextZ80.m_RegisterBC.lo, 6 ,   reg.reg, displacement); break;
        case 0x72 : CPU_DDFD_TEST_BIT(m_ContextZ80.m_RegisterDE.hi, 6 ,   reg.reg, displacement); break;
        case 0x73 : CPU_DDFD_TEST_BIT(m_ContextZ80.m_RegisterDE.lo, 6 ,   reg.reg, displacement); break;
        case 0x74 : CPU_DDFD_TEST_BIT(m_ContextZ80.m_RegisterHL.hi, 6 ,   reg.reg, displacement); break;
        case 0x75 : CPU_DDFD_TEST_BIT(m_ContextZ80.m_RegisterHL.lo, 6 ,   reg.reg, displacement); break;
        case 0x76 : CPU_TEST_BIT(m_ContextZ80.m_FuncPtrRead(reg.reg+displacement), 6 , 4); m_ContextZ80.m_OpcodeCycle = 20;  break;
        case 0x77 : CPU_DDFD_TEST_BIT(m_ContextZ80.m_RegisterAF.hi, 6 ,   reg.reg, displacement); break;
        case 0x78 : CPU_DDFD_TEST_BIT(m_ContextZ80.m_RegisterBC.hi, 7 ,   reg.reg, displacement); break;
        case 0x79 : CPU_DDFD_TEST_BIT(m_ContextZ80.m_RegisterBC.lo, 7 ,   reg.reg, displacement); break;
        case 0x7A : CPU_DDFD_TEST_BIT(m_ContextZ80.m_RegisterDE.hi, 7 ,   reg.reg, displacement); break;
        case 0x7B : CPU_DDFD_TEST_BIT(m_ContextZ80.m_RegisterDE.lo, 7 ,   reg.reg, displacement); break;
        case 0x7C : CPU_DDFD_TEST_BIT(m_ContextZ80.m_RegisterHL.hi, 7 ,   reg.reg, displacement); break;
        case 0x7D : CPU_DDFD_TEST_BIT(m_ContextZ80.m_RegisterHL.lo, 7 ,   reg.reg, displacement); break;
        case 0x7E : CPU_TEST_BIT(m_ContextZ80.m_FuncPtrRead(reg.reg+displacement), 7 , 4); m_ContextZ80.m_OpcodeCycle = 20; break;
        case 0x7F : CPU_DDFD_TEST_BIT(m_ContextZ80.m_RegisterAF.hi, 7 ,   reg.reg, displacement); break;

        // reset bit
        case 0x80 : CPU_DDFD_RESET_BIT(m_ContextZ80.m_RegisterBC.hi, 0 ,  reg.reg, displacement); break;
        case 0x81 : CPU_DDFD_RESET_BIT(m_ContextZ80.m_RegisterBC.lo, 0 ,  reg.reg, displacement); break;
        case 0x82 : CPU_DDFD_RESET_BIT(m_ContextZ80.m_RegisterDE.hi, 0 ,  reg.reg, displacement); break;
        case 0x83 : CPU_DDFD_RESET_BIT(m_ContextZ80.m_RegisterDE.lo, 0 ,  reg.reg, displacement); break;
        case 0x84 : CPU_DDFD_RESET_BIT(m_ContextZ80.m_RegisterHL.hi, 0 ,  reg.reg, displacement); break;
        case 0x85 : CPU_DDFD_RESET_BIT(m_ContextZ80.m_RegisterHL.lo, 0 ,  reg.reg, displacement); break;
        case 0x86 : CPU_RESET_BIT_MEMORY(reg.reg + displacement, 0); m_ContextZ80.m_OpcodeCycle = 23;break;
        case 0x87 : CPU_DDFD_RESET_BIT(m_ContextZ80.m_RegisterAF.hi, 0 ,  reg.reg, displacement); break;
        case 0x88 : CPU_DDFD_RESET_BIT(m_ContextZ80.m_RegisterBC.hi, 1  ,  reg.reg, displacement); break;
        case 0x89 : CPU_DDFD_RESET_BIT(m_ContextZ80.m_RegisterBC.lo, 1 ,  reg.reg, displacement); break;
        case 0x8A : CPU_DDFD_RESET_BIT(m_ContextZ80.m_RegisterDE.hi, 1 ,  reg.reg, displacement); break;
        case 0x8B : CPU_DDFD_RESET_BIT(m_ContextZ80.m_RegisterDE.lo, 1 ,  reg.reg, displacement); break;
        case 0x8C : CPU_DDFD_RESET_BIT(m_ContextZ80.m_RegisterHL.hi, 1 ,  reg.reg, displacement); break;
        case 0x8D : CPU_DDFD_RESET_BIT(m_ContextZ80.m_RegisterHL.lo, 1 ,  reg.reg, displacement); break;
        case 0x8E : CPU_RESET_BIT_MEMORY(reg.reg + displacement, 1); m_ContextZ80.m_OpcodeCycle =23;break;
        case 0x8F : CPU_DDFD_RESET_BIT(m_ContextZ80.m_RegisterAF.hi, 1  ,  reg.reg, displacement); break;
        case 0x90 : CPU_DDFD_RESET_BIT(m_ContextZ80.m_RegisterBC.hi, 2  ,  reg.reg, displacement); break;
        case 0x91 : CPU_DDFD_RESET_BIT(m_ContextZ80.m_RegisterBC.lo, 2  ,  reg.reg, displacement); break;
        case 0x92 : CPU_DDFD_RESET_BIT(m_ContextZ80.m_RegisterDE.hi, 2  ,  reg.reg, displacement); break;
        case 0x93 : CPU_DDFD_RESET_BIT(m_ContextZ80.m_RegisterDE.lo, 2  ,  reg.reg, displacement); break;
        case 0x94 : CPU_DDFD_RESET_BIT(m_ContextZ80.m_RegisterHL.hi, 2  ,  reg.reg, displacement); break;
        case 0x95 : CPU_DDFD_RESET_BIT(m_ContextZ80.m_RegisterHL.lo, 2  ,  reg.reg, displacement); break;
        case 0x96 : CPU_RESET_BIT_MEMORY(reg.reg+displacement, 2); m_ContextZ80.m_OpcodeCycle = 23;  break;
        case 0x97 : CPU_DDFD_RESET_BIT(m_ContextZ80.m_RegisterAF.hi, 2  ,  reg.reg, displacement); break;
        case 0x98 : CPU_DDFD_RESET_BIT(m_ContextZ80.m_RegisterBC.hi, 3  ,  reg.reg, displacement); break;
        case 0x99 : CPU_DDFD_RESET_BIT(m_ContextZ80.m_RegisterBC.lo, 3  ,  reg.reg, displacement); break;
        case 0x9A : CPU_DDFD_RESET_BIT(m_ContextZ80.m_RegisterDE.hi, 3  ,  reg.reg, displacement); break;
        case 0x9B : CPU_DDFD_RESET_BIT(m_ContextZ80.m_RegisterDE.lo, 3  ,  reg.reg, displacement); break;
        case 0x9C : CPU_DDFD_RESET_BIT(m_ContextZ80.m_RegisterHL.hi, 3  ,  reg.reg, displacement); break;
        case 0x9D : CPU_DDFD_RESET_BIT(m_ContextZ80.m_RegisterHL.lo, 3  ,  reg.reg, displacement); break;
        case 0x9E : CPU_RESET_BIT_MEMORY(reg.reg+displacement, 3 ); m_ContextZ80.m_OpcodeCycle = 23; break;
        case 0x9F : CPU_DDFD_RESET_BIT(m_ContextZ80.m_RegisterAF.hi, 3  ,  reg.reg, displacement); break;
        case 0xA0 : CPU_DDFD_RESET_BIT(m_ContextZ80.m_RegisterBC.hi, 4  ,  reg.reg, displacement); break;
        case 0xA1 : CPU_DDFD_RESET_BIT(m_ContextZ80.m_RegisterBC.lo, 4  ,  reg.reg, displacement); break;
        case 0xA2 : CPU_DDFD_RESET_BIT(m_ContextZ80.m_RegisterDE.hi, 4  ,  reg.reg, displacement); break;
        case 0xA3 : CPU_DDFD_RESET_BIT(m_ContextZ80.m_RegisterDE.lo, 4  ,  reg.reg, displacement); break;
        case 0xA4 : CPU_DDFD_RESET_BIT(m_ContextZ80.m_RegisterHL.hi, 4  ,  reg.reg, displacement); break;
        case 0xA5 : CPU_DDFD_RESET_BIT(m_ContextZ80.m_RegisterHL.lo, 4  ,  reg.reg, displacement); break;
        case 0xA6 : CPU_RESET_BIT_MEMORY(reg.reg+displacement, 4); m_ContextZ80.m_OpcodeCycle = 23;  break;
        case 0xA7 : CPU_DDFD_RESET_BIT(m_ContextZ80.m_RegisterAF.hi, 4 ,  reg.reg, displacement); break;
        case 0xA8 : CPU_DDFD_RESET_BIT(m_ContextZ80.m_RegisterBC.hi, 5 ,  reg.reg, displacement); break;
        case 0xA9 : CPU_DDFD_RESET_BIT(m_ContextZ80.m_RegisterBC.lo, 5 ,  reg.reg, displacement); break;
        case 0xAA : CPU_DDFD_RESET_BIT(m_ContextZ80.m_RegisterDE.hi, 5 ,  reg.reg, displacement); break;
        case 0xAB : CPU_DDFD_RESET_BIT(m_ContextZ80.m_RegisterDE.lo, 5 ,  reg.reg, displacement); break;
        case 0xAC : CPU_DDFD_RESET_BIT(m_ContextZ80.m_RegisterHL.hi, 5 ,  reg.reg, displacement); break;
        case 0xAD : CPU_DDFD_RESET_BIT(m_ContextZ80.m_RegisterHL.lo, 5 ,  reg.reg, displacement); break;
        case 0xAE : CPU_RESET_BIT_MEMORY(reg.reg+displacement, 5); m_ContextZ80.m_OpcodeCycle = 23;  break;
        case 0xAF : CPU_DDFD_RESET_BIT(m_ContextZ80.m_RegisterAF.hi, 5  ,  reg.reg, displacement); break;
        case 0xB0 : CPU_DDFD_RESET_BIT(m_ContextZ80.m_RegisterBC.hi, 6  ,  reg.reg, displacement); break;
        case 0xB1 : CPU_DDFD_RESET_BIT(m_ContextZ80.m_RegisterBC.lo, 6  ,  reg.reg, displacement); break;
        case 0xB2 : CPU_DDFD_RESET_BIT(m_ContextZ80.m_RegisterDE.hi, 6  ,  reg.reg, displacement); break;
        case 0xB3 : CPU_DDFD_RESET_BIT(m_ContextZ80.m_RegisterDE.lo, 6  ,  reg.reg, displacement); break;
        case 0xB4 : CPU_DDFD_RESET_BIT(m_ContextZ80.m_RegisterHL.hi, 6  ,  reg.reg, displacement); break;
        case 0xB5 : CPU_DDFD_RESET_BIT(m_ContextZ80.m_RegisterHL.lo, 6  ,  reg.reg, displacement); break;
        case 0xB6 : CPU_RESET_BIT_MEMORY(reg.reg+displacement, 6); m_ContextZ80.m_OpcodeCycle = 23;  break;
        case 0xB7 : CPU_DDFD_RESET_BIT(m_ContextZ80.m_RegisterAF.hi, 6  ,  reg.reg, displacement); break;
        case 0xB8 : CPU_DDFD_RESET_BIT(m_ContextZ80.m_RegisterBC.hi, 7  ,  reg.reg, displacement); break;
        case 0xB9 : CPU_DDFD_RESET_BIT(m_ContextZ80.m_RegisterBC.lo, 7  ,  reg.reg, displacement); break;
        case 0xBA : CPU_DDFD_RESET_BIT(m_ContextZ80.m_RegisterDE.hi, 7  ,  reg.reg, displacement); break;
        case 0xBB : CPU_DDFD_RESET_BIT(m_ContextZ80.m_RegisterDE.lo, 7  ,  reg.reg, displacement); break;
        case 0xBC : CPU_DDFD_RESET_BIT(m_ContextZ80.m_RegisterHL.hi, 7  ,  reg.reg, displacement); break;
        case 0xBD : CPU_DDFD_RESET_BIT(m_ContextZ80.m_RegisterHL.lo, 7  ,  reg.reg, displacement); break;
        case 0xBE : CPU_RESET_BIT_MEMORY(reg.reg+displacement, 7); m_ContextZ80.m_OpcodeCycle = 23;  break;
        case 0xBF : CPU_DDFD_RESET_BIT(m_ContextZ80.m_RegisterAF.hi, 7 ,  reg.reg, displacement); break;


        // set bit
        case 0xC0 : CPU_DDFD_SET_BIT(m_ContextZ80.m_RegisterBC.hi, 0 ,  reg.reg, displacement); break;
        case 0xC1 : CPU_DDFD_SET_BIT(m_ContextZ80.m_RegisterBC.lo, 0 ,  reg.reg, displacement); break;
        case 0xC2 : CPU_DDFD_SET_BIT(m_ContextZ80.m_RegisterDE.hi, 0 ,  reg.reg, displacement); break;
        case 0xC3 : CPU_DDFD_SET_BIT(m_ContextZ80.m_RegisterDE.lo, 0 ,  reg.reg, displacement); break;
        case 0xC4 : CPU_DDFD_SET_BIT(m_ContextZ80.m_RegisterHL.hi, 0 ,  reg.reg, displacement); break;
        case 0xC5 : CPU_DDFD_SET_BIT(m_ContextZ80.m_RegisterHL.lo, 0 ,  reg.reg, displacement); break;
        case 0xC6 : CPU_SET_BIT_MEMORY(reg.reg + displacement, 0); m_ContextZ80.m_OpcodeCycle = 23; break;
        case 0xC7 : CPU_DDFD_SET_BIT(m_ContextZ80.m_RegisterAF.hi, 0 ,  reg.reg, displacement); break;
        case 0xC8 : CPU_DDFD_SET_BIT(m_ContextZ80.m_RegisterBC.hi, 1  ,  reg.reg, displacement); break;
        case 0xC9 : CPU_DDFD_SET_BIT(m_ContextZ80.m_RegisterBC.lo, 1 ,  reg.reg, displacement); break;
        case 0xCA : CPU_DDFD_SET_BIT(m_ContextZ80.m_RegisterDE.hi, 1 ,  reg.reg, displacement); break;
        case 0xCB : CPU_DDFD_SET_BIT(m_ContextZ80.m_RegisterDE.lo, 1 ,  reg.reg, displacement); break;
        case 0xCC : CPU_DDFD_SET_BIT(m_ContextZ80.m_RegisterHL.hi, 1 ,  reg.reg, displacement); break;
        case 0xCD : CPU_DDFD_SET_BIT(m_ContextZ80.m_RegisterHL.lo, 1 ,  reg.reg, displacement); break;
        case 0xCE : CPU_SET_BIT_MEMORY(reg.reg + displacement, 1); m_ContextZ80.m_OpcodeCycle = 23; break;
        case 0xCF : CPU_DDFD_SET_BIT(m_ContextZ80.m_RegisterAF.hi, 1  ,  reg.reg, displacement); break;
        case 0xD0 : CPU_DDFD_SET_BIT(m_ContextZ80.m_RegisterBC.hi, 2  ,  reg.reg, displacement); break;
        case 0xD1 : CPU_DDFD_SET_BIT(m_ContextZ80.m_RegisterBC.lo, 2  ,  reg.reg, displacement); break;
        case 0xD2 : CPU_DDFD_SET_BIT(m_ContextZ80.m_RegisterDE.hi, 2  ,  reg.reg, displacement); break;
        case 0xD3 : CPU_DDFD_SET_BIT(m_ContextZ80.m_RegisterDE.lo, 2  ,  reg.reg, displacement); break;
        case 0xD4 : CPU_DDFD_SET_BIT(m_ContextZ80.m_RegisterHL.hi, 2  ,  reg.reg, displacement); break;
        case 0xD5 : CPU_DDFD_SET_BIT(m_ContextZ80.m_RegisterHL.lo, 2  ,  reg.reg, displacement); break;
        case 0xD6 : CPU_SET_BIT_MEMORY(reg.reg+displacement, 2); m_ContextZ80.m_OpcodeCycle = 23;  break;
        case 0xD7 : CPU_DDFD_SET_BIT(m_ContextZ80.m_RegisterAF.hi, 2  ,  reg.reg, displacement); break;
        case 0xD8 : CPU_DDFD_SET_BIT(m_ContextZ80.m_RegisterBC.hi, 3  ,  reg.reg, displacement); break;
        case 0xD9 : CPU_DDFD_SET_BIT(m_ContextZ80.m_RegisterBC.lo, 3  ,  reg.reg, displacement); break;
        case 0xDA : CPU_DDFD_SET_BIT(m_ContextZ80.m_RegisterDE.hi, 3  ,  reg.reg, displacement); break;
        case 0xDB : CPU_DDFD_SET_BIT(m_ContextZ80.m_RegisterDE.lo, 3  ,  reg.reg, displacement); break;
        case 0xDC : CPU_DDFD_SET_BIT(m_ContextZ80.m_RegisterHL.hi, 3  ,  reg.reg, displacement); break;
        case 0xDD : CPU_DDFD_SET_BIT(m_ContextZ80.m_RegisterHL.lo, 3  ,  reg.reg, displacement); break;
        case 0xDE : CPU_SET_BIT_MEMORY(reg.reg+displacement, 3 ); m_ContextZ80.m_OpcodeCycle = 23; break;
        case 0xDF : CPU_DDFD_SET_BIT(m_ContextZ80.m_RegisterAF.hi, 3  ,  reg.reg, displacement); break;
        case 0xE0 : CPU_DDFD_SET_BIT(m_ContextZ80.m_RegisterBC.hi, 4  ,  reg.reg, displacement); break;
        case 0xE1 : CPU_DDFD_SET_BIT(m_ContextZ80.m_RegisterBC.lo, 4  ,  reg.reg, displacement); break;
        case 0xE2 : CPU_DDFD_SET_BIT(m_ContextZ80.m_RegisterDE.hi, 4  ,  reg.reg, displacement); break;
        case 0xE3 : CPU_DDFD_SET_BIT(m_ContextZ80.m_RegisterDE.lo, 4  ,  reg.reg, displacement); break;
        case 0xE4 : CPU_DDFD_SET_BIT(m_ContextZ80.m_RegisterHL.hi, 4  ,  reg.reg, displacement); break;
        case 0xE5 : CPU_DDFD_SET_BIT(m_ContextZ80.m_RegisterHL.lo, 4  ,  reg.reg, displacement); break;
        case 0xE6 : CPU_SET_BIT_MEMORY(reg.reg+displacement, 4); m_ContextZ80.m_OpcodeCycle = 23;  break;
        case 0xE7 : CPU_DDFD_SET_BIT(m_ContextZ80.m_RegisterAF.hi, 4 ,  reg.reg, displacement); break;
        case 0xE8 : CPU_DDFD_SET_BIT(m_ContextZ80.m_RegisterBC.hi, 5 ,  reg.reg, displacement); break;
        case 0xE9 : CPU_DDFD_SET_BIT(m_ContextZ80.m_RegisterBC.lo, 5 ,  reg.reg, displacement); break;
        case 0xEA : CPU_DDFD_SET_BIT(m_ContextZ80.m_RegisterDE.hi, 5 ,  reg.reg, displacement); break;
        case 0xEB : CPU_DDFD_SET_BIT(m_ContextZ80.m_RegisterDE.lo, 5 ,  reg.reg, displacement); break;
        case 0xEC : CPU_DDFD_SET_BIT(m_ContextZ80.m_RegisterHL.hi, 5 ,  reg.reg, displacement); break;
        case 0xED : CPU_DDFD_SET_BIT(m_ContextZ80.m_RegisterHL.lo, 5 ,  reg.reg, displacement); break;
        case 0xEE : CPU_SET_BIT_MEMORY(reg.reg+displacement, 5); m_ContextZ80.m_OpcodeCycle = 23;  break;
        case 0xEF : CPU_DDFD_SET_BIT(m_ContextZ80.m_RegisterAF.hi, 5  ,  reg.reg, displacement); break;
        case 0xF0 : CPU_DDFD_SET_BIT(m_ContextZ80.m_RegisterBC.hi, 6  ,  reg.reg, displacement); break;
        case 0xF1 : CPU_DDFD_SET_BIT(m_ContextZ80.m_RegisterBC.lo, 6  ,  reg.reg, displacement); break;
        case 0xF2 : CPU_DDFD_SET_BIT(m_ContextZ80.m_RegisterDE.hi, 6  ,  reg.reg, displacement); break;
        case 0xF3 : CPU_DDFD_SET_BIT(m_ContextZ80.m_RegisterDE.lo, 6  ,  reg.reg, displacement); break;
        case 0xF4 : CPU_DDFD_SET_BIT(m_ContextZ80.m_RegisterHL.hi, 6  ,  reg.reg, displacement); break;
        case 0xF5 : CPU_DDFD_SET_BIT(m_ContextZ80.m_RegisterHL.lo, 6  ,  reg.reg, displacement); break;
        case 0xF6 : CPU_SET_BIT_MEMORY(reg.reg+displacement, 6); m_ContextZ80.m_OpcodeCycle = 23;  break;
        case 0xF7 : CPU_DDFD_SET_BIT(m_ContextZ80.m_RegisterAF.hi, 6 ,  reg.reg, displacement); break;
        case 0xF8 : CPU_DDFD_SET_BIT(m_ContextZ80.m_RegisterBC.hi, 7  ,  reg.reg, displacement); break;
        case 0xF9 : CPU_DDFD_SET_BIT(m_ContextZ80.m_RegisterBC.lo, 7  ,  reg.reg, displacement); break;
        case 0xFA : CPU_DDFD_SET_BIT(m_ContextZ80.m_RegisterDE.hi, 7  ,  reg.reg, displacement); break;
        case 0xFB : CPU_DDFD_SET_BIT(m_ContextZ80.m_RegisterDE.lo, 7  ,  reg.reg, displacement); break;
        case 0xFC : CPU_DDFD_SET_BIT(m_ContextZ80.m_RegisterHL.hi, 7  ,  reg.reg, displacement); break;
        case 0xFD : CPU_DDFD_SET_BIT(m_ContextZ80.m_RegisterHL.lo, 7  ,  reg.reg, displacement); break;
        case 0xFE : CPU_SET_BIT_MEMORY(reg.reg+displacement, 7); m_ContextZ80.m_OpcodeCycle = 23;  break;
        case 0xFF : CPU_DDFD_SET_BIT(m_ContextZ80.m_RegisterAF.hi, 7 ,  reg.reg, displacement); break;


        default:
        {
            char buffer[255];
            sprintf(buffer, "Unhandled DDFDCB opcode. Displacement %x opcode %x",displacement, opcode);
            LogMessage::GetSingleton()->DoLogMessage(buffer, true);
            assert(false);
        }
        break;
    }
}

//////////////////////////////////////////////////////////////////////////////////

void Z80::ExecuteEDOpcode()
{
    BYTE opcode = m_ContextZ80.m_FuncPtrRead(m_ContextZ80.m_ProgramCounter);

    IncreaseRReg();

    LogInstInfo(opcode, "ED", false);

    m_ContextZ80.m_ProgramCounter++;

  //  char buffer[255];
//  sprintf(buffer, "Executing ED Opcode %x",opcode);
//  LogMessage::GetSingleton()->DoLogMessage(buffer,true);




    switch(opcode)
    {
        case 0x49 : CPU_OUT(m_ContextZ80.m_RegisterBC.lo, m_ContextZ80.m_RegisterBC.lo);break;
        case 0x41 : CPU_OUT(m_ContextZ80.m_RegisterBC.lo, m_ContextZ80.m_RegisterBC.hi);break;
        case 0x51 : CPU_OUT(m_ContextZ80.m_RegisterBC.lo, m_ContextZ80.m_RegisterDE.hi);break;
        case 0x59 : CPU_OUT(m_ContextZ80.m_RegisterBC.lo, m_ContextZ80.m_RegisterDE.lo);break;
        case 0x61 : CPU_OUT(m_ContextZ80.m_RegisterBC.lo, m_ContextZ80.m_RegisterHL.hi);break;
        case 0x71 : CPU_OUT(m_ContextZ80.m_RegisterBC.lo, 0); break; // UNOFFICIAL
        case 0x69 : CPU_OUT(m_ContextZ80.m_RegisterBC.lo, m_ContextZ80.m_RegisterHL.lo);break;
        case 0x79 : CPU_OUT(m_ContextZ80.m_RegisterBC.lo, m_ContextZ80.m_RegisterAF.hi); break;
        case 0xA3 : CPU_OUTI(); break;
        case 0xB3 : CPU_OTIR(); break;

        case 0x4A : CPU_16BIT_ADD(m_ContextZ80.m_RegisterHL.reg, m_ContextZ80.m_RegisterBC.reg, 15, true);break;
        case 0x5A : CPU_16BIT_ADD(m_ContextZ80.m_RegisterHL.reg, m_ContextZ80.m_RegisterDE.reg, 15, true);break;
        case 0x6A : CPU_16BIT_ADD(m_ContextZ80.m_RegisterHL.reg, m_ContextZ80.m_RegisterHL.reg, 15, true);break;
        case 0x7A : CPU_16BIT_ADD(m_ContextZ80.m_RegisterHL.reg, m_ContextZ80.m_StackPointer.reg, 15, true);break;


        case 0x42 : CPU_16BIT_SUB(m_ContextZ80.m_RegisterHL.reg, m_ContextZ80.m_RegisterBC.reg, 15, true); break;
        case 0x52 : CPU_16BIT_SUB(m_ContextZ80.m_RegisterHL.reg, m_ContextZ80.m_RegisterDE.reg, 15, true); break;
        case 0x62 : CPU_16BIT_SUB(m_ContextZ80.m_RegisterHL.reg, m_ContextZ80.m_RegisterHL.reg, 15, true); break;
        case 0x72 : CPU_16BIT_SUB(m_ContextZ80.m_RegisterHL.reg, m_ContextZ80.m_StackPointer.reg, 15, true); break;

        case 0xAB: CPU_OUTD(); break;
        case 0xBB: CPU_OTDR(); break;
        case 0xA0: CPU_LDI(); break;
        case 0xB0: CPU_LDIR(); break;
        case 0xA1: CPU_CPI();break;
        case 0xB1: CPU_CPIR();break;
        case 0xA9: CPU_CPD(); break;
        case 0xB9: CPU_CPDR(); break;
        case 0xA2: CPU_INI(); break;
        case 0xB2: CPU_INIR(); break;
        case 0xAA: CPU_IND(); break;
        case 0xBA: CPU_INDR(); break;

        case 0x43: CPU_LOAD_NNN(m_ContextZ80.m_RegisterBC.reg); break;
        case 0x53: CPU_LOAD_NNN(m_ContextZ80.m_RegisterDE.reg); break;
        case 0x63: CPU_LOAD_NNN(m_ContextZ80.m_RegisterHL.reg); break;
        case 0x73: CPU_LOAD_NNN(m_ContextZ80.m_StackPointer.reg); break;

        case 0x45: m_ContextZ80.m_ProgramCounter = PopWordOffStack(); m_ContextZ80.m_IFF1 = m_ContextZ80.m_IFF2; m_ContextZ80.m_NMIServicing = false;m_ContextZ80.m_OpcodeCycle = 4;break; // iff1 = iff2 is correct (look at sean youngs undocumented)

        case 0x4b: CPU_REG_LOAD_NNN(m_ContextZ80.m_RegisterBC.reg); m_ContextZ80.m_OpcodeCycle = 20; break;
        case 0x5b: CPU_REG_LOAD_NNN(m_ContextZ80.m_RegisterDE.reg); m_ContextZ80.m_OpcodeCycle = 20;break;
        case 0x6b: CPU_REG_LOAD_NNN(m_ContextZ80.m_RegisterHL.reg); m_ContextZ80.m_OpcodeCycle = 20;break;
        case 0x7b: CPU_REG_LOAD_NNN(m_ContextZ80.m_StackPointer.reg); m_ContextZ80.m_OpcodeCycle = 20;break;

        case 0x40: CPU_IN(m_ContextZ80.m_RegisterBC.hi); break;
        case 0x48: CPU_IN(m_ContextZ80.m_RegisterBC.lo); break;
        case 0x50: CPU_IN(m_ContextZ80.m_RegisterDE.hi); break;
        case 0x58: CPU_IN(m_ContextZ80.m_RegisterDE.lo); break;
        case 0x60: CPU_IN(m_ContextZ80.m_RegisterHL.hi); break;
        case 0x68: CPU_IN(m_ContextZ80.m_RegisterHL.lo); break;
        case 0x78: CPU_IN(m_ContextZ80.m_RegisterAF.hi); break;

        case 0xA8: CPU_LDD(); break;
        case 0xB8: CPU_LDDR(); break;
        case 0x44: CPU_NEG(); break;
        case 0x67: CPU_RRD();m_ContextZ80.m_OpcodeCycle = 18; break;
        case 0x6F: CPU_RLD();m_ContextZ80.m_OpcodeCycle = 18; break;

        case 0x4D:
        {
            m_ContextZ80.m_ProgramCounter = PopWordOffStack();
            m_ContextZ80.m_IFF1 = m_ContextZ80.m_IFF2;// iff1 = iff2 is correct (look at sean youngs undocumented)
            m_ContextZ80.m_NMIServicing = false;
            m_ContextZ80.m_OpcodeCycle = 14;
        }
        break;

        case 0x47: m_ContextZ80.m_RegisterI = m_ContextZ80.m_RegisterAF.hi; m_ContextZ80.m_OpcodeCycle = 8; break;
        case 0x4F: m_ContextZ80.m_RegisterR = m_ContextZ80.m_RegisterAF.hi; m_ContextZ80.m_OpcodeCycle = 8; break;
        case 0x57: CPU_LDA_I(); break; 
        case 0x5F: CPU_LDA_R(); break;


        case 0x46: LogMessage::GetSingleton()->DoLogMessage("changing to interupt mode 0", true);assert(false);m_ContextZ80.m_InteruptMode = 0;m_ContextZ80.m_OpcodeCycle=8;break;
        case 0x5E: LogMessage::GetSingleton()->DoLogMessage("changing to interupt mode 2", true);assert(false);m_ContextZ80.m_InteruptMode = 2;m_ContextZ80.m_OpcodeCycle=8;break;

        case 0x56: m_ContextZ80.m_InteruptMode = 1;m_ContextZ80.m_OpcodeCycle=8;break;

        
        default:
        {
            char buffer[255];
            sprintf(buffer, "Unhandled ED opcode %x", opcode);
            LogMessage::GetSingleton()->DoLogMessage(buffer, true);
            assert(false);
        }
        break;
    }
}

//////////////////////////////////////////////////////////////////////////////////

void Z80::ExecuteDDFDOpcode(bool isDD)
{
    IncreaseRReg();
    BYTE opcode = m_ContextZ80.m_FuncPtrRead(m_ContextZ80.m_ProgramCounter);

    LogInstInfo(opcode, "DDFD", false);

    m_ContextZ80.m_ProgramCounter++;

    REGISTERZ80& reg = isDD?m_ContextZ80.m_RegisterIX:m_ContextZ80.m_RegisterIY;

    switch(opcode)
    {
        case 0xE1: reg.reg = PopWordOffStack();  m_ContextZ80.m_OpcodeCycle=14;break;
        case 0xE5: PushWordOntoStack(reg.reg);  m_ContextZ80.m_OpcodeCycle=15; break;
        case 0x21: CPU_16BIT_LOAD(reg.reg);m_ContextZ80.m_OpcodeCycle=14;break;
        case 0xCB: ExecuteDDFDCBOpcode(isDD); break;
        case 0x2A: CPU_REG_LOAD_NNN(reg.reg); m_ContextZ80.m_OpcodeCycle=20;break;
        case 0x26: CPU_8BIT_LOAD_IMMEDIATE(reg.hi); m_ContextZ80.m_OpcodeCycle=11;break;
        case 0x2E: CPU_8BIT_LOAD_IMMEDIATE(reg.lo); m_ContextZ80.m_OpcodeCycle=11;break;
        case 0x22: CPU_LOAD_NNN(reg.reg); m_ContextZ80.m_OpcodeCycle=20;break;

        case 0x09: CPU_16BIT_ADD(reg.reg, m_ContextZ80.m_RegisterBC.reg, 15, false); break;
        case 0x19: CPU_16BIT_ADD(reg.reg, m_ContextZ80.m_RegisterDE.reg, 15, false); break;
        case 0x29: CPU_16BIT_ADD(reg.reg, reg.reg, 15, false); break;
        case 0x39: CPU_16BIT_ADD(reg.reg, m_ContextZ80.m_StackPointer.reg, 15, false); break;

        case 0x46: CPU_8BIT_IXIY_LOAD(m_ContextZ80.m_RegisterBC.hi, reg); break;
        case 0x4E: CPU_8BIT_IXIY_LOAD(m_ContextZ80.m_RegisterBC.lo, reg); break;
        case 0x56: CPU_8BIT_IXIY_LOAD(m_ContextZ80.m_RegisterDE.hi, reg); break;
        case 0x5E: CPU_8BIT_IXIY_LOAD(m_ContextZ80.m_RegisterDE.lo, reg); break;
        case 0x66: CPU_8BIT_IXIY_LOAD(m_ContextZ80.m_RegisterHL.hi, reg); break;
        case 0x6E: CPU_8BIT_IXIY_LOAD(m_ContextZ80.m_RegisterHL.lo, reg); break;
        case 0x7E: CPU_8BIT_IXIY_LOAD(m_ContextZ80.m_RegisterAF.hi, reg); break;

        case 0x70: CPU_8BIT_MEM_IXIY_LOAD(m_ContextZ80.m_RegisterBC.hi, reg); break;
        case 0x71: CPU_8BIT_MEM_IXIY_LOAD(m_ContextZ80.m_RegisterBC.lo, reg); break;
        case 0x72: CPU_8BIT_MEM_IXIY_LOAD(m_ContextZ80.m_RegisterDE.hi, reg); break;
        case 0x73: CPU_8BIT_MEM_IXIY_LOAD(m_ContextZ80.m_RegisterDE.lo, reg); break;
        case 0x74: CPU_8BIT_MEM_IXIY_LOAD(m_ContextZ80.m_RegisterHL.hi, reg); break;
        case 0x75: CPU_8BIT_MEM_IXIY_LOAD(m_ContextZ80.m_RegisterHL.lo, reg); break;
        case 0x77: CPU_8BIT_MEM_IXIY_LOAD(m_ContextZ80.m_RegisterAF.hi, reg); break;

        case 0x86: CPU_8BIT_ADD(m_ContextZ80.m_RegisterAF.hi, m_ContextZ80.m_FuncPtrRead(GetIXIYAddress(reg.reg)),19,false,false); break;
        case 0x8E: CPU_8BIT_ADD(m_ContextZ80.m_RegisterAF.hi, m_ContextZ80.m_FuncPtrRead(GetIXIYAddress(reg.reg)),19,false,true); break;
        case 0x34: CPU_8BIT_MEMORY_INC(GetIXIYAddress(reg.reg),23); break;
        case 0x35: CPU_8BIT_MEMORY_DEC(GetIXIYAddress(reg.reg),23); break;
        case 0x96: CPU_8BIT_SUB(m_ContextZ80.m_RegisterAF.hi,m_ContextZ80.m_FuncPtrRead(GetIXIYAddress(reg.reg)),19, false,false); break;
        case 0x9E: CPU_8BIT_SUB(m_ContextZ80.m_RegisterAF.hi,m_ContextZ80.m_FuncPtrRead(GetIXIYAddress(reg.reg)),19, false,true); break;
        case 0xA6: CPU_8BIT_AND(m_ContextZ80.m_RegisterAF.hi,m_ContextZ80.m_FuncPtrRead(GetIXIYAddress(reg.reg)),19, false); break;
        case 0xAE: CPU_8BIT_XOR(m_ContextZ80.m_RegisterAF.hi,m_ContextZ80.m_FuncPtrRead(GetIXIYAddress(reg.reg)),19, false); break;
        case 0xB6: CPU_8BIT_OR(m_ContextZ80.m_RegisterAF.hi,m_ContextZ80.m_FuncPtrRead(GetIXIYAddress(reg.reg)),19, false); break;
        case 0xBE: CPU_8BIT_COMPARE(m_ContextZ80.m_RegisterAF.hi,m_ContextZ80.m_FuncPtrRead(GetIXIYAddress(reg.reg)),19, false); break;

        case 0x23: CPU_16BIT_INC(reg.reg, 10); break;
        case 0x2B: CPU_16BIT_DEC(reg.reg, 10); break;
        case 0x24: CPU_8BIT_INC(reg.hi , 10); break;
        case 0x25: CPU_8BIT_DEC(reg.hi , 10); break;
        case 0x2C: CPU_8BIT_INC(reg.lo , 10); break;
        case 0x2D: CPU_8BIT_DEC(reg.lo , 10); break;


        case 0x44: CPU_REG_LOAD(m_ContextZ80.m_RegisterBC.hi, reg.hi); m_ContextZ80.m_OpcodeCycle = 8; break;
        case 0x45: CPU_REG_LOAD(m_ContextZ80.m_RegisterBC.hi, reg.lo); m_ContextZ80.m_OpcodeCycle = 8;break;
        case 0x4C: CPU_REG_LOAD(m_ContextZ80.m_RegisterBC.lo, reg.hi); m_ContextZ80.m_OpcodeCycle = 8; break;
        case 0x4D: CPU_REG_LOAD(m_ContextZ80.m_RegisterBC.lo, reg.lo); m_ContextZ80.m_OpcodeCycle = 8;break;
        case 0x54: CPU_REG_LOAD(m_ContextZ80.m_RegisterDE.hi, reg.hi); m_ContextZ80.m_OpcodeCycle = 8; break;
        case 0x55: CPU_REG_LOAD(m_ContextZ80.m_RegisterDE.hi, reg.lo); m_ContextZ80.m_OpcodeCycle = 8;break;
        case 0x5C: CPU_REG_LOAD(m_ContextZ80.m_RegisterDE.lo, reg.hi); m_ContextZ80.m_OpcodeCycle = 8; break;
        case 0x5D: CPU_REG_LOAD(m_ContextZ80.m_RegisterDE.lo, reg.lo); m_ContextZ80.m_OpcodeCycle = 8;break;
        case 0x64: CPU_REG_LOAD(reg.hi, reg.hi); m_ContextZ80.m_OpcodeCycle = 8; break;
        case 0x65: CPU_REG_LOAD(reg.hi, reg.lo); m_ContextZ80.m_OpcodeCycle = 8;break;
        case 0x6C: CPU_REG_LOAD(reg.lo, reg.hi); m_ContextZ80.m_OpcodeCycle = 8; break;
        case 0x6D: CPU_REG_LOAD(reg.lo, reg.lo); m_ContextZ80.m_OpcodeCycle = 8;break;
        case 0x7C: CPU_REG_LOAD(m_ContextZ80.m_RegisterAF.hi, reg.hi); m_ContextZ80.m_OpcodeCycle = 8; break;
        case 0x7D: CPU_REG_LOAD(m_ContextZ80.m_RegisterAF.hi, reg.lo); m_ContextZ80.m_OpcodeCycle = 8;break;

        case 0x84: CPU_8BIT_ADD(m_ContextZ80.m_RegisterAF.hi, reg.hi,8,false,false); break;
        case 0x85: CPU_8BIT_ADD(m_ContextZ80.m_RegisterAF.hi, reg.lo,8,false,false); break;
        case 0x8C: CPU_8BIT_ADD(m_ContextZ80.m_RegisterAF.hi, reg.hi,8,false,true); break;
        case 0x8D: CPU_8BIT_ADD(m_ContextZ80.m_RegisterAF.hi, reg.lo,8,false,true); break;
        case 0x94: CPU_8BIT_SUB(m_ContextZ80.m_RegisterAF.hi, reg.hi,8,false,false); break;
        case 0x95: CPU_8BIT_SUB(m_ContextZ80.m_RegisterAF.hi, reg.lo,8,false,false); break;
        case 0x9C: CPU_8BIT_SUB(m_ContextZ80.m_RegisterAF.hi, reg.hi,8,false,true); break;
        case 0x9D: CPU_8BIT_SUB(m_ContextZ80.m_RegisterAF.hi, reg.lo,8,false,true); break;
        case 0xA4: CPU_8BIT_AND(m_ContextZ80.m_RegisterAF.hi, reg.hi,8,false); break;
        case 0xA5: CPU_8BIT_AND(m_ContextZ80.m_RegisterAF.hi, reg.lo,8,false); break;
        case 0xAC: CPU_8BIT_XOR(m_ContextZ80.m_RegisterAF.hi, reg.hi,8,false); break;
        case 0xAD: CPU_8BIT_XOR(m_ContextZ80.m_RegisterAF.hi, reg.lo,8,false); break;
        case 0xB4: CPU_8BIT_OR(m_ContextZ80.m_RegisterAF.hi, reg.hi,8,false); break;
        case 0xB5: CPU_8BIT_OR(m_ContextZ80.m_RegisterAF.hi, reg.lo,8,false); break;
        case 0xBC: CPU_8BIT_COMPARE(m_ContextZ80.m_RegisterAF.hi, reg.hi,8,false); break;
        case 0xBD: CPU_8BIT_COMPARE(m_ContextZ80.m_RegisterAF.hi, reg.lo,8,false); break;


        // IF YOU HAVE TO DO A JUMP INSTURCTION LIKE THIS JP (IX) WHERE THE ORIGINAL INSTRUCTION WAS JP (HL) YOU DO NOT ADD THE
        // DISPLACEMENT TO THE IX ADDRESS. I.E. IT IS JP (IX) NOT JP (IX+d)

        // EX DE, HL IS UNAFFECTED


        case 0x40: CPU_REG_LOAD(m_ContextZ80.m_RegisterBC.hi, m_ContextZ80.m_RegisterBC.hi);  m_ContextZ80.m_OpcodeCycle = 8; break;
        case 0x41: CPU_REG_LOAD(m_ContextZ80.m_RegisterBC.hi, m_ContextZ80.m_RegisterBC.lo);  m_ContextZ80.m_OpcodeCycle = 8;break;
        case 0x42: CPU_REG_LOAD(m_ContextZ80.m_RegisterBC.hi, m_ContextZ80.m_RegisterDE.hi);  m_ContextZ80.m_OpcodeCycle = 8;break;
        case 0x43: CPU_REG_LOAD(m_ContextZ80.m_RegisterBC.hi, m_ContextZ80.m_RegisterDE.lo);  m_ContextZ80.m_OpcodeCycle = 8;break;
        case 0x47: CPU_REG_LOAD(m_ContextZ80.m_RegisterBC.hi, m_ContextZ80.m_RegisterAF.hi);  m_ContextZ80.m_OpcodeCycle = 8;break;
        case 0x48: CPU_REG_LOAD(m_ContextZ80.m_RegisterBC.lo, m_ContextZ80.m_RegisterBC.hi);  m_ContextZ80.m_OpcodeCycle = 8;break;
        case 0x49: CPU_REG_LOAD(m_ContextZ80.m_RegisterBC.lo, m_ContextZ80.m_RegisterBC.lo);  m_ContextZ80.m_OpcodeCycle = 8;break;
        case 0x4A: CPU_REG_LOAD(m_ContextZ80.m_RegisterBC.lo, m_ContextZ80.m_RegisterDE.hi);  m_ContextZ80.m_OpcodeCycle = 8;break;
        case 0x4B: CPU_REG_LOAD(m_ContextZ80.m_RegisterBC.lo, m_ContextZ80.m_RegisterDE.lo);  m_ContextZ80.m_OpcodeCycle = 8;break;
        case 0x4F: CPU_REG_LOAD(m_ContextZ80.m_RegisterBC.lo, m_ContextZ80.m_RegisterAF.hi);  m_ContextZ80.m_OpcodeCycle = 8;break;
        case 0x50: CPU_REG_LOAD(m_ContextZ80.m_RegisterDE.hi, m_ContextZ80.m_RegisterBC.hi);  m_ContextZ80.m_OpcodeCycle = 8;break;
        case 0x51: CPU_REG_LOAD(m_ContextZ80.m_RegisterDE.hi, m_ContextZ80.m_RegisterBC.lo);  m_ContextZ80.m_OpcodeCycle = 8;break;
        case 0x52: CPU_REG_LOAD(m_ContextZ80.m_RegisterDE.hi, m_ContextZ80.m_RegisterDE.hi);  m_ContextZ80.m_OpcodeCycle = 8;break;
        case 0x53: CPU_REG_LOAD(m_ContextZ80.m_RegisterDE.hi, m_ContextZ80.m_RegisterDE.lo);  m_ContextZ80.m_OpcodeCycle = 8;break;
        case 0x57: CPU_REG_LOAD(m_ContextZ80.m_RegisterDE.hi, m_ContextZ80.m_RegisterAF.hi);  m_ContextZ80.m_OpcodeCycle = 8;break;
        case 0x58: CPU_REG_LOAD(m_ContextZ80.m_RegisterDE.lo, m_ContextZ80.m_RegisterBC.hi);  m_ContextZ80.m_OpcodeCycle = 8;break;
        case 0x59: CPU_REG_LOAD(m_ContextZ80.m_RegisterDE.lo, m_ContextZ80.m_RegisterBC.lo);  m_ContextZ80.m_OpcodeCycle = 8;break;
        case 0x5A: CPU_REG_LOAD(m_ContextZ80.m_RegisterDE.lo, m_ContextZ80.m_RegisterDE.hi);  m_ContextZ80.m_OpcodeCycle = 8;break;
        case 0x5B: CPU_REG_LOAD(m_ContextZ80.m_RegisterDE.lo, m_ContextZ80.m_RegisterDE.lo);  m_ContextZ80.m_OpcodeCycle = 8;break;
        case 0x5F: CPU_REG_LOAD(m_ContextZ80.m_RegisterDE.lo, m_ContextZ80.m_RegisterAF.hi);  m_ContextZ80.m_OpcodeCycle = 8;break;
        case 0x60: CPU_REG_LOAD(reg.hi, m_ContextZ80.m_RegisterBC.hi);  m_ContextZ80.m_OpcodeCycle = 8;break;
        case 0x61: CPU_REG_LOAD(reg.hi, m_ContextZ80.m_RegisterBC.lo);  m_ContextZ80.m_OpcodeCycle = 8;break;
        case 0x62: CPU_REG_LOAD(reg.hi, m_ContextZ80.m_RegisterDE.hi);  m_ContextZ80.m_OpcodeCycle = 8;break;
        case 0x63: CPU_REG_LOAD(reg.hi, m_ContextZ80.m_RegisterDE.lo);  m_ContextZ80.m_OpcodeCycle = 8;break;
        case 0x67: CPU_REG_LOAD(reg.hi, m_ContextZ80.m_RegisterAF.hi);  m_ContextZ80.m_OpcodeCycle = 8;break;
        case 0x68: CPU_REG_LOAD(reg.lo, m_ContextZ80.m_RegisterBC.hi);  m_ContextZ80.m_OpcodeCycle = 8;break;
        case 0x69: CPU_REG_LOAD(reg.lo, m_ContextZ80.m_RegisterBC.lo);  m_ContextZ80.m_OpcodeCycle = 8;break;
        case 0x6A: CPU_REG_LOAD(reg.lo, m_ContextZ80.m_RegisterDE.hi);  m_ContextZ80.m_OpcodeCycle = 8;break;
        case 0x6B: CPU_REG_LOAD(reg.lo, m_ContextZ80.m_RegisterDE.lo);  m_ContextZ80.m_OpcodeCycle = 8;break;
        case 0x6F: CPU_REG_LOAD(reg.lo, m_ContextZ80.m_RegisterAF.hi);  m_ContextZ80.m_OpcodeCycle = 8;break;
        case 0x78: CPU_REG_LOAD(m_ContextZ80.m_RegisterAF.hi, m_ContextZ80.m_RegisterBC.hi);  m_ContextZ80.m_OpcodeCycle = 8;break;
        case 0x79: CPU_REG_LOAD(m_ContextZ80.m_RegisterAF.hi, m_ContextZ80.m_RegisterBC.lo);  m_ContextZ80.m_OpcodeCycle = 8;break;
        case 0x7A: CPU_REG_LOAD(m_ContextZ80.m_RegisterAF.hi, m_ContextZ80.m_RegisterDE.hi);  m_ContextZ80.m_OpcodeCycle = 8;break;
        case 0x7B: CPU_REG_LOAD(m_ContextZ80.m_RegisterAF.hi, m_ContextZ80.m_RegisterDE.lo);  m_ContextZ80.m_OpcodeCycle = 8;break;
        case 0x7F: CPU_REG_LOAD(m_ContextZ80.m_RegisterAF.hi, m_ContextZ80.m_RegisterAF.hi);  m_ContextZ80.m_OpcodeCycle = 8;break;

        //case 0xC1: m_ContextZ80.m_RegisterBC.reg = PopWordOffStack(); m_ContextZ80.m_OpcodeCycle = 4; break;
        //case 0xF4 : CPU_CALL(true, FLAG_S, false); m_ContextZ80.m_OpcodeCycle = 6;break;
        //case 0xFC : CPU_CALL(true, FLAG_S, true); m_ContextZ80.m_OpcodeCycle = 6;break;
        
        

        case 0x36:
        {
            m_ContextZ80.m_OpcodeCycle = 19;
            WORD address = GetIXIYAddress(reg.reg);

            BYTE n = m_ContextZ80.m_FuncPtrRead(m_ContextZ80.m_ProgramCounter);
            m_ContextZ80.m_ProgramCounter++;
            m_ContextZ80.m_FuncPtrWrite(address, n);
        } break;


        case 0xE3:
        {
            BYTE nhi = m_ContextZ80.m_FuncPtrRead(m_ContextZ80.m_StackPointer.reg+1);
            BYTE nlo = m_ContextZ80.m_FuncPtrRead(m_ContextZ80.m_StackPointer.reg);
            BYTE h = reg.hi;
            BYTE l = reg.lo;
            reg.hi = nhi;
            reg.lo = nlo;
            m_ContextZ80.m_FuncPtrWrite(m_ContextZ80.m_StackPointer.reg+1, h);
            m_ContextZ80.m_FuncPtrWrite(m_ContextZ80.m_StackPointer.reg, l);
            m_ContextZ80.m_OpcodeCycle = 23;
        }
        break;

        case 0xE9: m_ContextZ80.m_OpcodeCycle=8; m_ContextZ80.m_ProgramCounter = reg.reg; break;

        case 0xF9: m_ContextZ80.m_OpcodeCycle=10; m_ContextZ80.m_StackPointer.reg = reg.reg; break;


        default:
        {
            char buffer[255];
            sprintf(buffer, "Unhandled DD opcode %x", opcode);
            LogMessage::GetSingleton()->DoLogMessage(buffer, true);
            assert(false);
        }
        break;
    }

}


//////////////////////////////////////////////////////////////////////////////////

