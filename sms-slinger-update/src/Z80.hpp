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

#include "Config.hpp"

typedef BYTE (*Z80ReadMemory)(WORD);
typedef void (*Z80WriteMemory)(WORD, BYTE);
typedef BYTE (*Z80IOReadMemory)(BYTE);
typedef void (*Z80IOWriteMemory)(BYTE, BYTE);


#define FLAG_S 7
#define FLAG_Z 6
//#define FLAG_B5 5
#define FLAG_H 4
//#define FLAG_B3 3
#define FLAG_PV 2
#define FLAG_N 1
#define FLAG_C 0

union REGISTERZ80
{
    WORD reg;
    struct
    {
        BYTE lo;
        BYTE hi;
    };
};

struct CONTEXTZ80
{
    REGISTERZ80         m_RegisterAF;
    REGISTERZ80         m_RegisterBC;
    REGISTERZ80         m_RegisterDE;
    REGISTERZ80         m_RegisterHL;
    REGISTERZ80         m_RegisterAFPrime;
    REGISTERZ80         m_RegisterBCPrime;
    REGISTERZ80         m_RegisterDEPrime;
    REGISTERZ80         m_RegisterHLPrime;
    REGISTERZ80         m_StackPointer;
    REGISTERZ80         m_RegisterIX;
    REGISTERZ80         m_RegisterIY;
    BYTE                m_RegisterI;
    BYTE                m_RegisterR;
    WORD                m_ProgramCounter;
    WORD                m_ProgramCounterStart;
    BYTE                m_CartridgeMemory[0x100000];
    BYTE                m_InternalMemory[0x10000];
    BYTE                m_OpcodeCycle;
    Z80ReadMemory       m_FuncPtrRead;
    Z80WriteMemory      m_FuncPtrWrite;
    Z80IOReadMemory     m_FuncPtrIORead;
    Z80IOWriteMemory    m_FuncPtrIOWrite;

    bool                m_Halted;
    bool                m_IFF1;
    bool                m_IFF2;
    bool                m_EIPending;
    int                 m_InteruptMode;
    bool                m_NMI;
    bool                m_NMIServicing;
};

class Z80 final
{
public:
                        Z80();

        int             ExecuteNextOpcode();
        void            PushWordOntoStack(WORD address);
        void            IncreaseRReg();

        CONTEXTZ80*     GetContext() { return &m_ContextZ80; }
private:
        void            ExecuteOpcode(const BYTE& opcode);
        WORD            ReadWord() const;

        WORD            PopWordOffStack();
        void            LogInstInfo(BYTE opcode, const char* subset, bool showmnemonic);

        CONTEXTZ80      m_ContextZ80;


        inline  void            CPU_NEG();
        inline  void            CPU_8BIT_LOAD_IMMEDIATE(BYTE& reg);
        inline  void            CPU_REG_LOAD(BYTE& reg, BYTE load);
        inline  void            CPU_REG_LOAD_ROM(BYTE& reg, WORD address);
        inline  void            CPU_16BIT_LOAD(WORD& reg);
        inline  void            CPU_8BIT_ADD(BYTE& reg, BYTE toAdd, int cycles, bool useImmediate, bool addCarry);
        inline  void            CPU_8BIT_SUB(BYTE& reg, BYTE toSub, int cycles, bool useImmediate, bool subCarry);
        inline  void            CPU_8BIT_AND(BYTE& reg, BYTE toAnd, int cycles, bool useImmediate);
        inline  void            CPU_8BIT_OR(BYTE& reg, BYTE toOr, int cycles, bool useImmediate);
        inline  void            CPU_8BIT_XOR(BYTE& reg, BYTE toXOr, int cycles, bool useImmediate);
        inline  void            CPU_8BIT_COMPARE(BYTE reg, BYTE toSubtract, int cycles, bool useImmediate); //dont pass a reference
        inline  void            CPU_8BIT_INC(BYTE& reg, int cycles);
        inline  void            CPU_8BIT_DEC(BYTE& reg, int cycles);
        inline  void            CPU_8BIT_MEMORY_INC(WORD address, int cycles);
        inline  void            CPU_8BIT_MEMORY_DEC(WORD address, int cycles);
        inline  void            CPU_LOAD_NNN(WORD reg);

        inline  void            CPU_16BIT_DEC(WORD& word, int cycles);
        inline  void            CPU_16BIT_INC(WORD& word, int cycles);
        inline  void            CPU_16BIT_ADD(WORD& reg, WORD toAdd, int cycles, bool addCarry);
        inline  void            CPU_16BIT_SUB(WORD& reg, WORD toSub, int cycles, bool subCarry);

        inline  void            CPU_JUMP(bool useCondition, int flag, bool condition);
        inline  void            CPU_JUMP_IMMEDIATE(bool useCondition, int flag, bool condition);
        inline  void            CPU_CALL(bool useCondition, int flag, bool condition);
        inline  void            CPU_RETURN(bool useCondition, int flag, bool condition);
        inline  void            CPU_RESTARTS(BYTE n);

        inline  void            CPU_RLC(BYTE& reg, bool isAReg);
        inline  void            CPU_RLC_MEMORY(WORD address, bool isAReg);
        inline  void            CPU_RRC(BYTE& reg, bool isAReg);
        inline  void            CPU_RRC_MEMORY(WORD address, bool isAReg);
        inline  void            CPU_DDFD_RLC(BYTE& reg, WORD& ixiyreg, SIGNED_BYTE& displacement);
        inline  void            CPU_DDFD_RRC(BYTE& reg, WORD& ixiyreg, SIGNED_BYTE& displacement);

        inline  void            CPU_DAA();

        inline  void            CPU_RL(BYTE& reg, bool isAReg);
        inline  void            CPU_RL_MEMORY(WORD address, bool isAReg);
        inline  void            CPU_RR(BYTE& reg, bool isAReg);
        inline  void            CPU_RR_MEMORY(WORD address, bool isAReg);
        inline  void            CPU_DDFD_RL(BYTE& reg, WORD& ixiyreg, SIGNED_BYTE& displacement);
        inline  void            CPU_DDFD_RR(BYTE& reg, WORD& ixiyreg, SIGNED_BYTE& displacement);

        inline  void            CPU_RLD();
        inline  void            CPU_RRD();

        inline  void            CPU_SLA(BYTE& reg);
        inline  void            CPU_SLA_MEMORY(WORD address);
        inline  void            CPU_SRA(BYTE& reg);
        inline  void            CPU_SRA_MEMORY(WORD address);
        inline  void            CPU_SRL(BYTE& reg);
        inline  void            CPU_SRL_MEMORY(WORD address);
        inline  void            CPU_SLL(BYTE& reg);
        inline  void            CPU_SLL_MEMORY(WORD address);

        inline  void            CPU_DDFD_SLA(BYTE& reg, WORD& ixiyreg, SIGNED_BYTE& displacement);
        inline  void            CPU_DDFD_SRA(BYTE& reg, WORD& ixiyreg, SIGNED_BYTE& displacement);
        inline  void            CPU_DDFD_SRL(BYTE& reg, WORD& ixiyreg, SIGNED_BYTE& displacement);
        inline  void            CPU_DDFD_SLL(BYTE& reg, WORD& ixiyreg, SIGNED_BYTE& displacement);


        inline  void            CPU_RESET_BIT(BYTE& reg, int bit);
        inline  void            CPU_DDFD_RESET_BIT(BYTE& reg, int bit, WORD& ixiyreg, SIGNED_BYTE& displacement);
        inline  void            CPU_RESET_BIT_MEMORY( WORD address, int bit);
        inline  void            CPU_TEST_BIT(BYTE reg, int bit, int cycles);
        inline  void            CPU_DDFD_TEST_BIT(BYTE reg, int bit, WORD& ixiyreg, SIGNED_BYTE& displacement);
        inline  void            CPU_SET_BIT(BYTE& reg, int bit);
        inline  void            CPU_DDFD_SET_BIT(BYTE& reg, int bit, WORD& ixiyreg, SIGNED_BYTE& displacement);
        inline  void            CPU_SET_BIT_MEMORY(WORD address, int bit);

        inline  void            CPU_IN(BYTE& data);
        inline  void            CPU_IN_IMMEDIATE(BYTE& data);
        inline  void            CPU_OUT(const BYTE& address, const BYTE& data);
        inline  void            CPU_OUT_IMMEDIATE(const BYTE& data);
        inline  void            CPU_OUTI();
        inline  void            CPU_OTIR();
        inline  void            CPU_OUTD();
        inline  void            CPU_OTDR();
        inline  BYTE            CPU_CPI();
        inline  void            CPU_CPIR();
        inline  BYTE            CPU_CPD();
        inline  void            CPU_CPDR();
        inline  void            CPU_INI();
        inline  void            CPU_INIR();
        inline  void            CPU_IND();
        inline  void            CPU_INDR();

        inline  void            CPU_LDI();
        inline  void            CPU_LDIR();

        inline  void            CPU_DJNZ();
        inline  void            CPU_LDD();
        inline  void            CPU_LDDR();
        inline  void            CPU_LDA_I();
        inline  void            CPU_LDA_R();

        inline  void            CPU_REG_LOAD_NNN(WORD& reg);

        void            ExecuteCBOpcode();
        void            ExecuteDDFDCBOpcode(bool isDD);
        void            ExecuteEDOpcode();
        void            ExecuteDDFDOpcode(bool isDD);

        inline  void            CPU_EXCHANGE(WORD& reg1, WORD& reg2);


        inline  void            CPU_8BIT_IXIY_LOAD(BYTE& store , const REGISTERZ80& reg);
        inline  void            CPU_8BIT_MEM_IXIY_LOAD(BYTE store , const REGISTERZ80& reg);

        WORD            GetIXIYAddress(WORD value);
        void            InitDAATable();
        
        WORD            m_DAATable[0x800];
        BYTE            m_ZSPTable[256];
};