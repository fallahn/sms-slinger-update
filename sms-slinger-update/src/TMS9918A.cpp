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
#include "TMS9918A.h"
#include "LogMessages.h"

#include <memory.h>
#include <cassert>
#include <cmath>
#include <fstream>

bool TMS9918A::m_ScreenDisabled = true;
bool TMS9918A::m_FrameToggle = true;

/////////////////////////////////////////////////////////////////////////

void GetOldStyleColour(BYTE colNum, BYTE& red, BYTE& green, BYTE& blue)
{
    switch (colNum)
    {
    case 0: red = 0; green = 0; blue = 0; break; // transparent
    case 1: red = 0; green = 0; blue = 0; break; // black
    case 2: red = 33; green = 200; blue = 66; break; // medium green
    case 3: red = 94; green = 220; blue = 120; break; // light green
    case 4: red = 84; green = 85; blue = 237; break; // dark blue
    case 5: red = 125; green = 118; blue = 252; break; // light blue
    case 6: red = 212; green = 82; blue = 77; break; // dark red
    case 7: red = 66; green = 235; blue = 245; break; // cyan
    case 8: red = 252; green = 85; blue = 84; break; // medium red
    case 9: red = 255; green = 121; blue = 120; break; // light red
    case 0xA: red = 212; green = 193; blue = 84; break; // dark yellow
    case 0xB: red = 230; green = 206; blue = 84; break; // light yellow
    case 0xC: red = 33; green = 176; blue = 59; break; // dark green
    case 0xD: red = 201; green = 91; blue = 186; break; // Magenta
    case 0xE: red = 204; green = 204; blue = 204; break; // Gray
    case 0xF: red = 255; green = 255; blue = 255; break; // White
    default: LogMessage::GetSingleton()->DoLogMessage("Invalid mode 2 colour", true); assert(false); break;
    }
}


/////////////////////////////////////////////////////////////////////////

TMS9918A::TMS9918A()
    : m_IsPAL               (true),
    m_NumScanlines          (NUM_NTSC_VERTICAL),
    m_IsVBlank              (false),
    m_IsSecondControlWrite  (false),
    m_Status                (0),
    m_RequestInterupt       (false),
    m_VScroll               (0),
    m_ReadBuffer            (0),
    m_Width                 (NUM_RES_HORIZONTAL),
    m_Height                (NUM_RES_VERTICAL),
    m_TempWord              (0),
    m_Refresh               (false),
    m_UseGFXOpt             (false)
{
    Reset(false);
}

/////////////////////////////////////////////////////////////////////////

void TMS9918A::Reset(bool isPAL)
{
    m_RefreshRatePerSecond = 0;
    m_ClockInfo = 0;
    std::memset(&m_VRAM, 0, sizeof(m_VRAM));
    std::memset(&m_CRAM, 0, sizeof(m_CRAM));
    std::memset(&m_VDPRegisters, 0, sizeof(m_VDPRegisters));

    m_VDPRegisters[0x2] = 0xFF; // will deafualt name table to 0x3800
    m_VDPRegisters[0x3] = 0xFF; // must set all bits
    m_VDPRegisters[0x4] = 0x07; // bits 2-0 should be set
    m_VDPRegisters[0x5] = 0xFF; // will default sprite table to 0x3F00;
    m_VDPRegisters[0xA] = 0xFF; // no line interupts to start with please
    m_RequestInterupt = false;
    m_ControlWord = 0;
    m_TempWord = 0;
    m_HCounter = 0;
    m_VCounter = 0;
    m_LineInterupt = 0xFF;
    m_VScroll = 0;
    m_ScreenDisabled = true;

    // the rest of the vdp registers are unused

    m_IsPAL = isPAL;
    m_VCounterFirst = true;

    m_NumScanlines = m_IsPAL ? NUM_PAL_VERTICAL : NUM_NTSC_VERTICAL;

    m_Height = NUM_RES_VERTICAL;
    m_RunningCycles = 0;

    ResetScreen();
}

/////////////////////////////////////////////////////////////////////////

void TMS9918A::Update(float nextCycle)
{
    m_RequestInterupt = TestBit(m_Status,7) && IsRegBitSet(1,5);

    WORD hcount = m_HCounter;
    bool nextline = false;
    m_IsVBlank = false;
    m_Refresh = false;

    m_RunningCycles += nextCycle;

    int clockInfo = static_cast<int>(floorf(m_RunningCycles));

    m_ClockInfo += clockInfo;

    // the hcounter moves on by the same number of
    // machine cycles not the graphic cards clock cycle. The machine cycle is twice the speed of the vdp
    int cycles = clockInfo * 2;

    // are we moving off this scanline onto the next?
    if ((hcount + cycles) > MACHINE_CLICKS_PER_SCANLINE)
    {
        nextline = true;
    }

    m_HCounter = (m_HCounter + cycles) % (MACHINE_CLICKS_PER_SCANLINE + 1);

    if (nextline)
    {
        BYTE vcount = m_VCounter;
        m_VCounter++;

        // are we coming to the end of the vertical refresh?
        if (vcount == 255)
        {
            m_RefreshRatePerSecond++;
            m_VCounter = 0;
            m_VCounterFirst = true;
            
            if (!m_UseGFXOpt)
            {
                Render();
            }
            m_Refresh = true;
        }
        else if ((m_VCounter == GetVJump()) && m_VCounterFirst) 
        {
            m_VCounterFirst = false;
            m_VCounter = GetVJumpTo();
        }

        // are we just about to enter vertical refresh?
        else if (m_VCounter == m_Height)
        {
            if (m_UseGFXOpt)
            {
                RenderOpt();
            }
            m_IsVBlank = true;
            m_Status = BitSet(m_Status, 7);
        }

        if (m_VCounter >= m_Height)
        {
            // do not reload the line interupt until we are past the FIRST line of the none active display period
            if (m_VCounter != m_Height)
            {
                m_LineInterupt = m_VDPRegisters[0xA];
            }

            m_VScroll = m_VDPRegisters[0x9];
            BYTE mode = GetVDPMode();
            if (mode == 11)
            {
                m_Height = NUM_RES_VERT_MED;
            }
            else if (mode == 14)
            {
                m_Height = NUM_RES_VERT_HIGH;
            }
            else
            {
                m_Height = NUM_RES_VERTICAL;
            }
        }

        // else if we are still drawing the screen then draw next scanline
        if (m_VCounter < m_Height)
        {
            m_ScreenDisabled = !IsRegBitSet(1,6);
            
            if (!m_UseGFXOpt)
            {
                Render();
            }
        }

        // decrement the line interupt counter during the active display period
        // including the first line of the none active display period
        if (m_VCounter <= m_Height)
        {
            bool underflow = false;
            if (m_LineInterupt == 0)
            {
                underflow = true;
            }
            m_LineInterupt--;

            // it is going to underflow
            if (underflow)
            {
                m_LineInterupt = m_VDPRegisters[0xA];
                if (IsRegBitSet(0, 4))
                {
                    m_RequestInterupt = true;
                }
            }
        }
    }
    if (TestBit(m_Status, 7) && IsRegBitSet(1, 5))
    {
        m_RequestInterupt = true;
    }

    m_RunningCycles -= clockInfo;
}

/////////////////////////////////////////////////////////////////////////

BYTE TMS9918A::ReadMemory(BYTE address)
{
    return m_VRAM[address];
}

/////////////////////////////////////////////////////////////////////////

void TMS9918A::WriteMemory(BYTE address, BYTE data)
{
    m_VRAM[address] = data;
}

/////////////////////////////////////////////////////////////////////////

BYTE TMS9918A::GetStatus()
{
    BYTE res = m_Status;
    if (GetVDPMode() == 2)
    {
        m_Status &= 0x2F; // turn off bits 7 and 5
    }
    else
    {
        m_Status &= 0x1F; // turn off top 3 bits
    }
    m_IsSecondControlWrite = false;
    m_RequestInterupt = false;

    return res;
}

/////////////////////////////////////////////////////////////////////////

void TMS9918A::WriteVDPAddress(BYTE data)
{
//  char buffer[200];
//  sprintf(buffer, "Value before write is %x", GetAddressRegister());
//  LogMessage::GetSingleton()->DoLogMessage(buffer,false);
    if (m_IsSecondControlWrite)
    {
        m_ControlWord &= 0xFF;
        m_ControlWord |= data << 8;
        m_TempWord &= 0xFF;
        m_TempWord |= data << 8;
    //  m_ControlWord = m_TempWord;
        m_IsSecondControlWrite = false;

        switch (GetCodeRegister())
        {
            case 0: m_ReadBuffer = m_VRAM[GetAddressRegister()]; IncrementAddress(); break;
            case 1: break;
            case 2: SetRegData();break;
            case 3: break;
        }
    }

    else
    {
        m_TempWord &= 0xFF00;
        m_TempWord |= data;
        m_IsSecondControlWrite = true;
        m_ControlWord &= 0xFF00;
        m_ControlWord |= data;
    }
//  sprintf(buffer, "Value after write is %x", GetAddressRegister());
//  LogMessage::GetSingleton()->DoLogMessage(buffer,false);
}

/////////////////////////////////////////////////////////////////////////

BYTE TMS9918A::ReadDataPort()
{
    m_IsSecondControlWrite = false;

    BYTE res = m_ReadBuffer;

    switch (GetCodeRegister())
    {
        case 0: m_ReadBuffer = m_VRAM[GetAddressRegister()]; break; // not sure about this one
        case 1: m_ReadBuffer = m_VRAM[GetAddressRegister()]; break;
        default: assert(false); break;
    }

    IncrementAddress();

    return res;
}

/////////////////////////////////////////////////////////////////////////

void TMS9918A::WriteDataPort(BYTE data)
{

    m_IsSecondControlWrite = false;
    BYTE code = GetCodeRegister();

    switch (code)
    {
        case 0: m_VRAM[GetAddressRegister()] = data; break; // not sure about this one
        case 1: m_VRAM[GetAddressRegister()] = data; break;
        case 2: m_VRAM[GetAddressRegister()] = data; break;
        case 3: m_CRAM[GetAddressRegister() & 31] = data; break; // write to CRAM
        default: assert(false); break;
    }

    m_ReadBuffer = data;

    IncrementAddress();
}

/////////////////////////////////////////////////////////////////////////

void TMS9918A::IncrementAddress()
{
    
    if (GetAddressRegister() == 0x3FFF)
    {
        m_ControlWord &= 0xC000;
    }
    else
    {
        m_ControlWord++;
    }
}

/////////////////////////////////////////////////////////////////////////

WORD TMS9918A::GetAddressRegister() const
{
    return m_ControlWord & 0x3FFF;
}

/////////////////////////////////////////////////////////////////////////

BYTE TMS9918A::GetCodeRegister() const
{
    WORD w = m_ControlWord >> 14;
    return (BYTE)w;
}

/////////////////////////////////////////////////////////////////////////

void TMS9918A::SetRegData()
{
    BYTE data = m_ControlWord & 0xFF;
    BYTE reg = m_ControlWord >> 8;
    reg &= 0xF;

    if (reg > 11)
    {
        return;
    }

    m_VDPRegisters[reg] = data;

    if (reg == 5)
    {
        if (TestBit(m_Status, 7) && IsRegBitSet(1, 5))
        {
            m_RequestInterupt = true;
        }
    }
}

/////////////////////////////////////////////////////////////////////////

void TMS9918A::RenderOpt()
{
    if (!m_FrameToggle)
    {
        return;
    }

    BYTE vCounterBackup = m_VCounter;
    BYTE mode = GetVDPMode();

    for (int i = 0; i < m_Height; i++)
    {
        m_VCounter = i;    
        // this may seem strange rendering sprites before background, however
        // it makes it easier to detect sprite collisions and get the background
        // priority working.

        if (mode == 2)
        {
            RenderSpritesMode2();
            RenderBackgroundMode2();          
        }
        else
        {
            RenderSpritesMode4();
            RenderBackgroundMode4();
        }
    }
    m_VCounter = vCounterBackup;
}

/////////////////////////////////////////////////////////////////////////

void TMS9918A::Render()
{
    if (!m_FrameToggle)
    {
        return;
    }

    BYTE mode = GetVDPMode();
        
    // this may seem strange rendering sprites before background, however
    // it makes it easier to detect sprite collisions and get the background
    // priority working.

    if (mode == 2)
    {
        RenderSpritesMode2();
        RenderBackgroundMode2();          
    }
    else
    {
        RenderSpritesMode4();
        RenderBackgroundMode4();
    }
}

/////////////////////////////////////////////////////////////////////////

void TMS9918A::RenderSpritesMode2()
{
    WORD satbase = GetSATBase();
    WORD sgtable = m_VDPRegisters[0x6] & 7;
    sgtable <<= 11;
    
    int size = IsRegBitSet(1,1)?16:8;
    bool isZoomed = IsRegBitSet(1,0);

    int spriteCount = 0;

    for (int sprite = 0; sprite < 32; sprite++)
    {
        WORD location = satbase + (sprite * 4);
        int y = m_VRAM[location];

        // d0 means dont draw this sprite and all remaining sprites
        if (y == 0xD0)
        {
            // if there is no illegal sprites then store this sprite in the status
            if (!TestBit(m_Status,6))
            {
                m_Status &= 0xE0; // turn off last 5 bits
                m_Status |= sprite; // puts sprite into last 5 bits
            }
            return; // need to return not break otherwise outside of for loop will overwrite last 5 bits of status
        }

        if (y > 0xD0)
        {
            y -= 0x100;
        }

        y++;

        if ((m_VCounter>=y) && (m_VCounter < (y+size)))
        {
            BYTE x = m_VRAM[location+1];
            BYTE pattern = m_VRAM[location+2];
            BYTE colour = m_VRAM[location+3];
            bool ec = TestBit(colour,7);

            if (ec)
            {
                x -= 32;
            }

            colour &= 0xF;

            if (colour == 0)
            {
                continue;
            }

            spriteCount++;

            if (spriteCount > 4)
            {
                SetMode2IllegalSprites(sprite);
                return; // need to return otherwise outside of for loop will reset last 5 bits of sprite
            }
            else
            {
                m_Status = BitReset(m_Status, 6); // not an illegal sprite
            }

            int line = m_VCounter - y;
        
            if (size == 8)
            {
                DrawMode2Sprite(sgtable + (pattern * 8), x, line, colour);
            }
            else
            {
                WORD address = sgtable + ((pattern & 252) * 8);
                DrawMode2Sprite(address, x, line, colour);
                DrawMode2Sprite(address, x+8, line+16, colour);
            }
        }
    }

    m_Status &= 0xE0; // turn off last 5 bits
    m_Status |= 31; // puts last sprite into last 5 bits   
}

/////////////////////////////////////////////////////////////////////////

void TMS9918A::SetMode2IllegalSprites(BYTE sprite)
{
    m_Status = BitSet(m_Status, 6);
    m_Status &= 0xE0; // turn off last 5 bits
    m_Status |= sprite; // puts sprite into last 5 bits
}

/////////////////////////////////////////////////////////////////////////

void TMS9918A::DrawMode2Sprite(const WORD& address, BYTE x, BYTE line, BYTE colour)
{
    BYTE red = 0;
    BYTE green = 0;
    BYTE blue = 0;

    GetOldStyleColour(colour, red,green,blue);
    BYTE invert = 7;
    for (int i = 0; i < 8; i++, invert--)
    {
        BYTE drawLine = m_VRAM[address+line];
        BYTE xpos = x + i;

        // is this a sprite collision?
        if (GetScreenPixelColour(xpos,m_VCounter,0) != SCREENBLANKCOLOUR)
        {
            m_Status = BitSet(m_Status,5);
            continue;
        }

        if (!TestBit(drawLine, invert))
        {
            continue;
        }

        WriteToScreen(xpos, m_VCounter, red,green,blue);
    }
}

/////////////////////////////////////////////////////////////////////////

void TMS9918A::RenderSpritesMode4()
{
    int vCounter = m_VCounter;
    int spriteCount = 0;
    WORD satbase = GetSATBase();

    bool is8x16 = false;
    bool isZoomed = false;
    int size = 8;      

    bool shiftX = IsRegBitSet(0,3);
    bool useSecondPattern = IsRegBitSet(6,2);

    // is it 8x16 sprite?
    if (IsRegBitSet(1, 1))
    {
        is8x16 = true;
        size = 16;
    }

    if (IsRegBitSet(1,0))
    {
        isZoomed = true;
        size = 16;
    }

    for (int sprite = 0; sprite < 64; sprite++)
    {
        int y = m_VRAM[satbase+sprite];

        if ((m_Height == NUM_RES_VERTICAL) && (y == 0xD0))
        {
            break;
        }

        if (y > 0xD0)
        {
            y -= 0x100;
        }

        y++;       

        if ((vCounter>=y) && (vCounter < (y+size)))
        {
            spriteCount++;
            int x = m_VRAM[satbase+128+(sprite*2)];
            WORD tileNumber = m_VRAM[satbase+129+(sprite*2)];

            if (spriteCount >8)
            {
                SetSpriteOverflow();
                break;
            }

            // if bit 3 of reg0 is set, x -= 8
            if (shiftX)
            {
                x -= 8;
            }

            // are we using first sprite patterns or second
            if (useSecondPattern)
            {
                tileNumber += 256;
            }

            // i believe this also affects tileNumber
            if (is8x16)
            {
                if (y < (vCounter + 9))
                {
                    tileNumber = BitReset(tileNumber, 0);
                }
            }

            int i;
            int col;

            int startAddress = tileNumber * 32;
            startAddress += (4 * (vCounter-y));

            BYTE data1 = m_VRAM[startAddress];
            BYTE data2 = m_VRAM[startAddress+1];
            BYTE data3 = m_VRAM[startAddress+2];
            BYTE data4 = m_VRAM[startAddress+3];

            for (i = 0, col = 7; i < 8; i++, col--)
            {
                if ((x+i)>= NUM_RES_HORIZONTAL)
                {
                    continue;
                }

                // is this a sprite collision?
                if (GetScreenPixelColour(x+i,vCounter,0) != SCREENBLANKCOLOUR)
                {
                    SetSpriteCollision();
                    continue;
                }
                BYTE palette = 0;
                BYTE bit = BitGetVal(data4,col);
                palette = (bit << 3);
                bit = BitGetVal(data3,col);
                palette |= (bit << 2);
                bit = BitGetVal(data2,col);
                palette |= (bit << 1);
                bit = BitGetVal(data1, col);
                palette |= bit;

                // sprites can only use the second palette, i think.

                // palette 0 is transparency
                if (palette == 0)
                {
                    continue;
                }

                BYTE colour = m_CRAM[palette+16];

                BYTE red = colour & 0x3;
                colour >>=2;
                BYTE green = colour & 0x3;
                colour >>=2;
                BYTE blue = colour & 0x3;

                WriteToScreen(x+i, vCounter,GetColourShade(red),GetColourShade(green),GetColourShade(blue));
            }
        }
    }
}

/////////////////////////////////////////////////////////////////////////

bool TMS9918A::IsRegBitSet(int reg, BYTE bit)
{
    return TestBit(m_VDPRegisters[reg], bit);
}

/////////////////////////////////////////////////////////////////////////

void TMS9918A::SetSpriteOverflow()
{
    m_Status = BitSet(m_Status, 6);
}

/////////////////////////////////////////////////////////////////////////

void TMS9918A::SetSpriteCollision()
{
    m_Status = BitSet(m_Status, 5);
}

/////////////////////////////////////////////////////////////////////////

void TMS9918A::RenderBackgroundMode2()
{
    WORD nameBase = GetNameBase();

    // reg 3 contains colour table info
    BYTE reg3 = m_VDPRegisters[0x3];

    // reg 4 contains the pattern table info
    BYTE reg4 = m_VDPRegisters[0x4];

    // if bit 2 is set of reg 4 the pattern table address starts at 0x2000 otherwise 0x0
    WORD pgBase = TestBit(reg4, 2) ? 0x2000 : 0x0;

    // if bit 7 is set of reg 3 the colour table address starts at 0x2000 otherwise 0x0
    WORD colBase = TestBit(reg3, 7) ? 0x2000 : 0x0;

    // bits 0 to 6 of reg 3 get anded over bits 1-7 of the character number
    BYTE colAnd = reg3 & 127;

    // bit 0-6 get anded over bit 1-7 of character number so shift left 1
    colAnd <<= 1;

    // make sure bit 0 is set to it doesnt affect bit 0 of character number as we only want to affect 1-7
    colAnd = BitSet(colAnd, 0);
    
    int row = m_VCounter / 8;

    //the pattern table is comprised of 3 tables (0-2), which one are we using
    int pgTable = 0;

    // are we drawing part of the lower 2 thirds of the screen?
    if (row > 7)
    {
        // if we are drawing bottom third
        if (row > 15)
        {
            // then use table 3 if bit 1 of reg 4 is set else use table 0
            if (TestBit(reg4, 1))
            {
                pgTable = 2;
            }
        }
        // we must be drawing the middle third
        else
        {
            // then use table 2 if bit 0 of reg 4 is set else use table 0
            if (TestBit(reg4, 0))
            {
                pgTable = 1;
            }
        }
    }

    // pg offset points us to the correct pg table to use (this is also the same for the colour table)
    WORD pgOffset = 0;

    if (pgTable == 1)
    {
        pgOffset = 256 * 8;
    }
    else if (pgTable == 2)
    {
        pgOffset = 256 * 2 * 8;
    }

    int line = m_VCounter % 8;

    for (int column = 0; column < 32; column++)
    {       
        WORD nameBaseCopy = nameBase + ((row*32)+column);       

        BYTE pattern = m_VRAM[nameBaseCopy];
        WORD pgAddress = pgBase + (pattern * 8);
        
        pgAddress += pgOffset;
        pgAddress += line;

        BYTE pixelLine = m_VRAM[pgAddress];
        BYTE colIndex = pattern & colAnd;

        BYTE colour = m_VRAM[colBase + (colIndex * 8) + pgOffset + line];
        BYTE fore = colour >> 4;
        BYTE back = colour & 0xF;

        int invert = 7;
        for (int x = 0; x < 8; x++, invert--)
        {
            BYTE colNum = TestBit(pixelLine, invert)?fore:back;

            if (colNum == 0)
            {
                continue;
            }

            BYTE red = 0;
            BYTE green = 0;
            BYTE blue = 0;

            GetOldStyleColour(colNum, red,green,blue);
            int xpos = (column * 8) + x;

            if (GetScreenPixelColour(xpos, m_VCounter, 0) != SCREENBLANKCOLOUR)
            {
                continue;
            }

            if (xpos >= NUM_RES_HORIZONTAL)
            {
                continue;
            }

            WriteToScreen(xpos, m_VCounter, red,green,blue);       
        }
    }
}

/////////////////////////////////////////////////////////////////////////

void TMS9918A::RenderBackgroundMode4()
{
    WORD nameBase = GetNameBase();
    int vCounter = m_VCounter;
    BYTE vScroll = m_VScroll; // v scrolling only gets updated outside active display
    BYTE hScroll = m_VDPRegisters[0x8];

    BYTE vStartingRow = vScroll >> 3;
    BYTE vFineScroll = vScroll & 0x7;
    BYTE hStartingCol = hScroll >> 3;
    BYTE hFineScroll = hScroll & 0x7;

    bool limitVScroll = IsRegBitSet(0, 7);
    bool limitHScroll = IsRegBitSet(0, 6);

    int row = vCounter;
    row /= 8;

    bool maskFirstColumn = IsRegBitSet(0,5);

    // draw all 32 columns
    for (int column = 0; column < 32; column++)
    {
        // draw all 8 pixels in the column
        int invert = 7; // this is used for the colour index
        for (int x = 0; x < 8; x++, invert--)
        {
            int xpixel = x;            
            
            bool allowhscroll = ((row > 1) || !limitHScroll)?true:false;

            int i = x;
            i += column * 8;

            int xpos = i;

            if (allowhscroll)
            {
                xpos = hStartingCol;
                xpos *= 8;
                xpos += xpixel + hFineScroll;
                xpos = xpos % m_Width;
            }           

            bool allowvscroll = (((xpos/8) > 23) && limitVScroll)?false:true;

            int vrow = row;

            if (allowvscroll)
            {
                vrow += vStartingRow;
                
                int bumpRow = vCounter % 8;
                if ((bumpRow + vFineScroll) > 7)
                {
                    vrow++;
                }

                int mod = (m_Height == NUM_RES_VERTICAL) ? 28 : 32;
                vrow = vrow % mod;
            }           

            int col = column;
            
            WORD nameBaseOffset = nameBase;
            nameBaseOffset += vrow * 64; //each scanline has 32 tiles (1 tile per column) but 1 tile is 2 bytes in memory
            nameBaseOffset += col * 2; // each tile is two bytes in memory

            WORD tileData = m_VRAM[nameBaseOffset+1] << 8;
            tileData |= m_VRAM[nameBaseOffset];

            bool hiPriority = TestBit(tileData,12);
            bool useSpritePalette = TestBit(tileData,11);
            bool vertFlip = TestBit(tileData,10);
            bool horzFlip = TestBit(tileData,9);
            WORD tileDefinition = tileData & 0x1FF;

            int offset = vCounter;;            

            if (allowvscroll)
            {
                offset += vScroll;
            }
            offset = offset % 8;

            if (vertFlip)
            {
                offset *= -1;
                offset += 7;
            }

            tileDefinition *= 32;
            tileDefinition += 4 * offset;  
  
            BYTE data1 = m_VRAM[tileDefinition];
            BYTE data2 = m_VRAM[tileDefinition+1];
            BYTE data3 = m_VRAM[tileDefinition+2];
            BYTE data4 = m_VRAM[tileDefinition+3];

            int colourbit = invert;
            if (horzFlip)
            {
                colourbit = x;
            }                   

            BYTE palette = 0;
            BYTE bit = BitGetVal(data4,colourbit);
            palette = (bit << 3);
            bit = BitGetVal(data3,colourbit);
            palette |= (bit << 2);
            bit = BitGetVal(data2,colourbit);
            palette |= (bit << 1);
            bit = BitGetVal(data1, colourbit);
            palette |= bit;

            bool masking = false;

            if ((xpos < 8) && maskFirstColumn)
            {
                masking = true;
                palette = m_VDPRegisters[0x7] & 15;
                useSpritePalette = true;
            }

            // a tile can only have a high priority if it isnt palette 0
            // if this doesnt work try chaning the if statement to if(palette == (m_VDPRegisters[0x7] & 15))
            if (palette == 0)
            {
                hiPriority = false;
            }

            if (useSpritePalette)
            {
                palette += 16;
            }

            BYTE colour = m_CRAM[palette];
                  
            BYTE red = colour & 0x3;
            colour >>=2;
            BYTE green = colour & 0x3;
            colour >>=2;
            BYTE blue = colour & 0x3;      

            // a sprite is drawn here so lets not overwrite it :)
            if (!masking && !hiPriority && (GetScreenPixelColour(xpos, m_VCounter, 0) != SCREENBLANKCOLOUR))
            {
                continue;
            }

            if (xpos >= NUM_RES_HORIZONTAL)
            {
                continue;
            }

            WriteToScreen(xpos,vCounter,GetColourShade(red),GetColourShade(green),GetColourShade(blue));
        }
        hStartingCol = (hStartingCol + 1) % 32;
    }
}

/////////////////////////////////////////////////////////////////////////

void TMS9918A::ResetScreen()
{
    if (m_UseGFXOpt)
    {
        m_FrameToggle = !m_FrameToggle;
    }
    else
    {
        m_FrameToggle = true;
    }

    if (m_Height == NUM_RES_VERTICAL)
    {
        std::memset(m_ScreenStandard, SCREENBLANKCOLOUR, sizeof(m_ScreenStandard));
    }
    else if (m_Height == NUM_RES_VERT_MED)
    {
        std::memset(m_ScreenMed, SCREENBLANKCOLOUR, sizeof(m_ScreenMed));
    }
    else if (m_Height == NUM_RES_VERT_HIGH)
    {
        std::memset(m_ScreenHigh, SCREENBLANKCOLOUR, sizeof(m_ScreenHigh));
    }
}

/////////////////////////////////////////////////////////////////////////

// returns the base address of the sprite attribute table
WORD TMS9918A::GetSATBase() const
{
    BYTE reg5 = m_VDPRegisters[0x5];

    // bits 7 and 0 are ignored
    reg5 = BitReset(reg5, 7);
    reg5 = BitReset(reg5, 0);

    WORD res = reg5 << 7;
    return res;
}

/////////////////////////////////////////////////////////////////////////

// returns the base address of the name table
WORD TMS9918A::GetNameBase() const
{
    BYTE reg2 = m_VDPRegisters[0x2];

    // bit 0 is ignored so is top nibble
    reg2 &= 0xF;
    reg2 = BitReset(reg2,0);

    if (m_Height != NUM_RES_VERTICAL)
    {
        // only bits 2 and 3 set name base
        reg2 >>= 2;
        reg2 &= 0x3;
  
        switch (reg2)
        {
            case 0: return 0x700; break;
            case 1: return 0x1700;break;
            case 2: return 0x2700;break;
            case 3: return 0x3700;break;
            default: assert(false); break;
        }
    }   

    WORD res = reg2 << 10;
    return res;
}

/////////////////////////////////////////////////////////////////////////

BYTE TMS9918A::GetColourShade(BYTE val) const
{
    switch (val)
    {
        case 0: return 0; break;
        case 3: return 255; break;
        case 1: return 85; break;
        case 2: return 170; break;
        default : assert(false); return 0; break;
    }
}

/////////////////////////////////////////////////////////////////////////

BYTE TMS9918A::GetVDPMode() const
{
    BYTE res = 0;
    res |= BitGetVal(m_VDPRegisters[0x0],2) << 3;
    res |= BitGetVal(m_VDPRegisters[0x1],3) << 2;
    res |= BitGetVal(m_VDPRegisters[0x0],1) << 1;
    res |= BitGetVal(m_VDPRegisters[0x1],4);
    return res;
}

/////////////////////////////////////////////////////////////////////////

void TMS9918A::WriteToScreen(BYTE x, BYTE y,BYTE red, BYTE green, BYTE blue)
{
    if (m_Height == NUM_RES_VERTICAL)
    {
        m_ScreenStandard[y][x][0] = red;
        m_ScreenStandard[y][x][1] = green;
        m_ScreenStandard[y][x][2] = blue;
    }
    else if (m_Height == NUM_RES_VERT_MED)
    {
        m_ScreenMed[y][x][0] = red;
        m_ScreenMed[y][x][1] = green;
        m_ScreenMed[y][x][2] = blue;
    }
    else if (m_Height == NUM_RES_VERT_HIGH)
    {
        m_ScreenHigh[y][x][0] = red;
        m_ScreenHigh[y][x][1] = green;
        m_ScreenHigh[y][x][2] = blue;
    }
}

/////////////////////////////////////////////////////////////////////////

BYTE TMS9918A::GetScreenPixelColour(BYTE x, BYTE y, int index) const
{
    if (m_Height == NUM_RES_VERTICAL)
    {
        return m_ScreenStandard[y][x][index] ;
    }
    else if (m_Height == NUM_RES_VERT_MED)
    {
        return m_ScreenMed[y][x][index] ;
    }
    else
    {
        return m_ScreenHigh[y][x][index] ;
    }
}

/////////////////////////////////////////////////////////////////////////

BYTE TMS9918A::GetVJump() const
{
    if (m_IsPAL)
    {
        if (m_Height == NUM_RES_VERTICAL)
        {
            return 0xF2;
        }
        else if (m_Height == NUM_RES_VERT_MED)
        {
            return 0xFF;
        }
        else
        {
            return 0xFF;
        }
    }
    else
    {
        if (m_Height == NUM_RES_VERTICAL)
        {
            return 0xDA;
        }
        else if (m_Height == NUM_RES_VERT_MED)
        {
            return 0xEA;
        }
        else
        {
            assert(false); // should never be used
            return 0xFF;
        }
    }
}

/////////////////////////////////////////////////////////////////////////

BYTE TMS9918A::GetVJumpTo() const
{
    if (m_IsPAL)
    {
        if (m_Height == NUM_RES_VERTICAL)
        {
            return 0xBA;
        }
        else if (m_Height == NUM_RES_VERT_MED)
        {
            return 0xC7;
        }
        else
        {
            return 0xC1;
        }
    }
    else
    {
        if (m_Height == NUM_RES_VERTICAL)
        {
            return 0xD5;
        }
        else if (m_Height == NUM_RES_VERT_MED)
        {
            return 0xE5;
        }
        else
        {
            assert(false); // should never be used
            return 0xFF;
        }
    }
}

/////////////////////////////////////////////////////////////////////////

void TMS9918A::DumpVRAM()
{
    std::ofstream outputFile("c:/output.txt");
    for (int i = 0; i <= 0x3FFF; i++)
    {
        char buffer[10];
        std::memset(buffer,'\0', sizeof(buffer));
        BYTE val = m_VRAM[i];
        if (val < 0x10)
        {
            sprintf(buffer, "0%x", val);
        }
        else
        {
            sprintf(buffer, "%x", val);
        }
        outputFile << buffer << " ";
        if ((i != 0) && ((i % 16) == 15))
        {
            outputFile << std::endl;
        }
    }
    outputFile.close();
}

/////////////////////////////////////////////////////////////////////////

bool TMS9918A::GetRefresh()
{
    if (m_Refresh)
    {
        m_Refresh = false;
        return true;
    }
    return false;
}

/////////////////////////////////////////////////////////////////////////

BYTE TMS9918A::GetHCounter() const
{
    // only uses 9 bits
    WORD mod = m_HCounter & 511;
    //mod >> 1; //this must be a typo, right? M
    mod >>= 1;
    BYTE res = mod & 0xFFFF;
    return res;
}

/////////////////////////////////////////////////////////////////////////

void TMS9918A::DumpClockInfo()
{
    char buffer[255];
    std::memset(buffer,0,sizeof(buffer));
    sprintf(buffer, "Graphics Chip Clock Cycles Per Second: %u There has beed %d frames", m_ClockInfo, m_RefreshRatePerSecond);
    LogMessage::GetSingleton()->DoLogMessage(buffer, true);

    m_ClockInfo = 0;
    m_RefreshRatePerSecond = 0;
}
/////////////////////////////////////////////////////////////////////////