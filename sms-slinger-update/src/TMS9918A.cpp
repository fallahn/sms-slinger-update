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
#include "TMS9918A.hpp"
#include "LogMessages.hpp"

#include <cassert>
#include <cmath>
#include <cstring>
#include <fstream>

namespace
{
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
}

bool TMS9918A::screenDisabled = true;
bool TMS9918A::frameToggle = true;

TMS9918A::TMS9918A()
    : m_runningCycles       (0.f),
    m_clockInfo             (0),
    m_isPAL                 (true),
    m_numScanlines          (NUM_NTSC_VERTICAL),
    m_isVBlank              (false),
    m_status                (0),
    m_controlWord           (0),
    m_tempWord              (0),
    m_isSecondControlWrite  (false),
    m_requestInterrupt      (false),
    m_useGFXOpt             (false),
    m_VCounter              (0),
    m_HCounter              (0),
    m_VCounterFirst         (true),
    m_lineInterrupt         (0xFF),
    m_VScroll               (0),
    m_readBuffer            (0),
    m_width                 (NUM_RES_HORIZONTAL),
    m_height                (NUM_RES_VERTICAL),
    m_refresh               (false),
    m_refreshRatePerSecond  (0)
{
    reset(false);
}

//public
void TMS9918A::update(float nextCycle)
{
    m_requestInterrupt = testBit(m_status, 7) && isRegBitSet(1, 5);

    WORD hcount = m_HCounter;
    bool nextline = false;
    m_isVBlank = false;
    m_refresh = false;

    m_runningCycles += nextCycle;

    int clockInfo = static_cast<int>(std::floor(m_runningCycles));

    m_clockInfo += clockInfo;

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
            m_refreshRatePerSecond++;
            m_VCounter = 0;
            m_VCounterFirst = true;
            
            if (!m_useGFXOpt)
            {
                render();
            }
            m_refresh = true;
        }
        else if ((m_VCounter == getVJump()) && m_VCounterFirst) 
        {
            m_VCounterFirst = false;
            m_VCounter = getVJumpTo();
        }

        // are we just about to enter vertical refresh?
        else if (m_VCounter == m_height)
        {
            if (m_useGFXOpt)
            {
                renderOpt();
            }
            m_isVBlank = true;
            m_status = bitSet(m_status, 7);
        }

        if (m_VCounter >= m_height)
        {
            // do not reload the line interupt until we are past the FIRST line of the none active display period
            if (m_VCounter != m_height)
            {
                m_lineInterrupt = m_VDPRegisters[0xA];
            }

            m_VScroll = m_VDPRegisters[0x9];
            BYTE mode = getVDPMode();
            if (mode == 11)
            {
                m_height = NUM_RES_VERT_MED;
                //m_currentBuffer = m_screenMed.data();
            }
            else if (mode == 14)
            {
                m_height = NUM_RES_VERT_HIGH;
                //m_currentBuffer = m_screenHigh.data();
            }
            else
            {
                m_height = NUM_RES_VERTICAL;
                //m_currentBuffer = m_screenStandard.data();
            }
        }

        // else if we are still drawing the screen then draw next scanline
        if (m_VCounter < m_height)
        {
            screenDisabled = !isRegBitSet(1, 6);
            
            if (!m_useGFXOpt)
            {
                render();
            }
        }

        // decrement the line interupt counter during the active display period
        // including the first line of the none active display period
        if (m_VCounter <= m_height)
        {
            bool underflow = false;
            if (m_lineInterrupt == 0)
            {
                underflow = true;
            }
            m_lineInterrupt--;

            // it is going to underflow
            if (underflow)
            {
                m_lineInterrupt = m_VDPRegisters[0xA];
                if (isRegBitSet(0, 4))
                {
                    m_requestInterrupt = true;
                }
            }
        }
    }
    if (testBit(m_status, 7) && isRegBitSet(1, 5))
    {
        m_requestInterrupt = true;
    }

    m_runningCycles -= clockInfo;
}

void TMS9918A::reset(bool isPAL)
{
    m_refreshRatePerSecond = 0;
    m_clockInfo = 0;

    std::fill(m_VRAM.begin(), m_VRAM.end(), 0);
    std::fill(m_CRAM.begin(), m_CRAM.end(), 0);
    std::fill(m_VDPRegisters.begin(), m_VDPRegisters.end(), 0);

    m_VDPRegisters[0x2] = 0xFF; // will deafualt name table to 0x3800
    m_VDPRegisters[0x3] = 0xFF; // must set all bits
    m_VDPRegisters[0x4] = 0x07; // bits 2-0 should be set
    m_VDPRegisters[0x5] = 0xFF; // will default sprite table to 0x3F00;
    m_VDPRegisters[0xA] = 0xFF; // no line interupts to start with please
    m_requestInterrupt = false;
    m_controlWord = 0;
    m_tempWord = 0;
    m_HCounter = 0;
    m_VCounter = 0;
    m_lineInterrupt = 0xFF;
    m_VScroll = 0;
    screenDisabled = true;

    // the rest of the vdp registers are unused

    m_isPAL = isPAL;
    m_VCounterFirst = true;

    m_numScanlines = m_isPAL ? NUM_PAL_VERTICAL : NUM_NTSC_VERTICAL;

    m_height = NUM_RES_VERTICAL;
    m_runningCycles = 0;

    resetScreen();
}

BYTE TMS9918A::readMemory(BYTE address)
{
    return m_VRAM[address];
}

void TMS9918A::writeMemory(BYTE address, BYTE data)
{
    m_VRAM[address] = data;
}

void TMS9918A::writeVDPAddress(BYTE data)
{
    //  char buffer[200];
    //  sprintf(buffer, "Value before write is %x", GetAddressRegister());
    //  LogMessage::GetSingleton()->DoLogMessage(buffer,false);
    if (m_isSecondControlWrite)
    {
        m_controlWord &= 0xFF;
        m_controlWord |= data << 8;
        m_tempWord &= 0xFF;
        m_tempWord |= data << 8;
        //  m_controlWord = m_tempWord;
        m_isSecondControlWrite = false;

        switch (getCodeRegister())
        {
        case 0: m_readBuffer = m_VRAM[getAddressRegister()]; incrementAddress(); break;
        case 1: break;
        case 2: setRegData(); break;
        case 3: break;
        }
    }

    else
    {
        m_tempWord &= 0xFF00;
        m_tempWord |= data;
        m_isSecondControlWrite = true;
        m_controlWord &= 0xFF00;
        m_controlWord |= data;
    }
}

BYTE TMS9918A::readDataPort()
{
    m_isSecondControlWrite = false;

    BYTE res = m_readBuffer;

    auto code = getCodeRegister();
    switch (code)
    {
        case 0:
        case 1:
        case 2:
        case 3:
            m_readBuffer = m_VRAM[getAddressRegister()]; break;
            //undefined - return no value ?
            //case 3:
            //res = 0xff;
            break;
        default: assert(false); break;
    }

    incrementAddress();

    return res;
}

void TMS9918A::writeDataPort(BYTE data)
{

    m_isSecondControlWrite = false;
    BYTE code = getCodeRegister();

    switch (code)
    {
        case 0: m_VRAM[getAddressRegister()] = data; break; // not sure about this one
        case 1: m_VRAM[getAddressRegister()] = data; break;
        case 2: m_VRAM[getAddressRegister()] = data; break;
        case 3: m_CRAM[getAddressRegister() & 31] = data; break; // write to CRAM
        default: assert(false); break;
    }

    m_readBuffer = data;

    incrementAddress();
}

BYTE TMS9918A::getStatus()
{
    BYTE res = m_status;
    if (getVDPMode() == 2)
    {
        m_status &= 0x2F; // turn off bits 7 and 5
    }
    else
    {
        m_status &= 0x1F; // turn off top 3 bits
    }
    m_isSecondControlWrite = false;
    m_requestInterrupt = false;

    return res;
}

void TMS9918A::resetScreen()
{
    if (m_useGFXOpt)
    {
        frameToggle = !frameToggle;
    }
    else
    {
        frameToggle = true;
    }

    std::fill(m_buffer.begin(), m_buffer.end(), SCREENBLANKCOLOUR);
}

BYTE TMS9918A::getHCounter() const
{
    // only uses 9 bits
    WORD mod = m_HCounter & 511;
    //mod >> 1; //this must be a typo, right? - M
    mod >>= 1;
    BYTE res = mod & 0xFFFF;
    return res;
}

bool TMS9918A::getRefresh()
{
    if (m_refresh)
    {
        m_refresh = false;
        return true;
    }
    return false;
}

void TMS9918A::dumpClockInfo()
{
    char buffer[255];
    std::memset(buffer, 0, sizeof(buffer));
    sprintf(buffer, "Graphics Chip Clock Cycles Per Second: %lu There has been %d frames", m_clockInfo, m_refreshRatePerSecond);
    LogMessage::GetSingleton()->DoLogMessage(buffer, true);

    m_clockInfo = 0;
    m_refreshRatePerSecond = 0;
}

//private
WORD TMS9918A::getAddressRegister() const
{
    return m_controlWord & 0x3FFF;
}

BYTE TMS9918A::getCodeRegister() const
{
    WORD w = m_controlWord >> 14;
    return static_cast<BYTE>(w);
}

void TMS9918A::incrementAddress()
{
    
    if (getAddressRegister() == 0x3FFF)
    {
        m_controlWord &= 0xC000;
    }
    else
    {
        m_controlWord++;
    }
}

void TMS9918A::setRegData()
{
    BYTE data = m_controlWord & 0xFF;
    BYTE reg = m_controlWord >> 8;
    reg &= 0xF;

    if (reg > 11)
    {
        return;
    }

    m_VDPRegisters[reg] = data;

    if (reg == 5)
    {
        if (testBit(m_status, 7) && isRegBitSet(1, 5))
        {
            m_requestInterrupt = true;
        }
    }
}

void TMS9918A::render()
{
    if (!frameToggle)
    {
        return;
    }

    BYTE mode = getVDPMode();
        
    // this may seem strange rendering sprites before background, however
    // it makes it easier to detect sprite collisions and get the background
    // priority working.

    if (mode == 2)
    {
        renderSpritesMode2();
        renderBackgroundMode2();          
    }
    else
    {
        renderSpritesMode4();
        renderBackgroundMode4();
    }
}

void TMS9918A::renderOpt()
{
    if (!frameToggle)
    {
        return;
    }

    BYTE vCounterBackup = m_VCounter;
    BYTE mode = getVDPMode();

    for (int i = 0; i < m_height; i++)
    {
        m_VCounter = i;    
        // this may seem strange rendering sprites before background, however
        // it makes it easier to detect sprite collisions and get the background
        // priority working.

        if (mode == 2)
        {
            renderSpritesMode2();
            renderBackgroundMode2();          
        }
        else
        {
            renderSpritesMode4();
            renderBackgroundMode4();
        }
    }
    m_VCounter = vCounterBackup;
}

void TMS9918A::renderSpritesMode2()
{
    WORD satbase = getSATBase();
    WORD sgtable = m_VDPRegisters[0x6] & 7;
    sgtable <<= 11;
    
    int size = isRegBitSet(1, 1) ? 16 : 8;
    //bool isZoomed = isRegBitSet(1, 0);

    int spriteCount = 0;

    for (int sprite = 0; sprite < 32; sprite++)
    {
        WORD location = satbase + (sprite * 4);
        int y = m_VRAM[location];

        // d0 means dont draw this sprite and all remaining sprites
        if (y == 0xD0)
        {
            // if there is no illegal sprites then store this sprite in the status
            if (!testBit(m_status,6))
            {
                m_status &= 0xE0; // turn off last 5 bits
                m_status |= sprite; // puts sprite into last 5 bits
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
            bool ec = testBit(colour,7);

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
                setMode2IllegalSprites(sprite);
                return; // need to return otherwise outside of for loop will reset last 5 bits of sprite
            }
            else
            {
                m_status = bitReset(m_status, 6); // not an illegal sprite
            }

            int line = m_VCounter - y;
        
            if (size == 8)
            {
                drawMode2Sprite(sgtable + (pattern * 8), x, line, colour);
            }
            else
            {
                WORD address = sgtable + ((pattern & 252) * 8);
                drawMode2Sprite(address, x, line, colour);
                drawMode2Sprite(address, x+8, line+16, colour);
            }
        }
    }

    m_status &= 0xE0; // turn off last 5 bits
    m_status |= 31; // puts last sprite into last 5 bits   
}

void TMS9918A::renderSpritesMode4()
{
    int vCounter = m_VCounter;
    int spriteCount = 0;
    WORD satbase = getSATBase();

    bool is8x16 = false;
    int size = 8;      

    bool shiftX = isRegBitSet(0, 3);
    bool useSecondPattern = isRegBitSet(6, 2);

    // is it 8x16 sprite?
    if (isRegBitSet(1, 1))
    {
        is8x16 = true;
        size = 16;
    }

    if (isRegBitSet(1, 0))
    {
        size = 16;
    }

    for (int sprite = 0; sprite < 64; sprite++)
    {
        int y = m_VRAM[satbase+sprite];

        if ((m_height == NUM_RES_VERTICAL) && (y == 0xD0))
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
                setSpriteOverflow();
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
                    tileNumber = bitReset(tileNumber, 0);
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
                if (getScreenPixelColour(x+i, vCounter, 0) != SCREENBLANKCOLOUR)
                {
                    setSpriteCollision();
                    continue;
                }
                BYTE palette = 0;
                BYTE bit = bitGetVal(data4,col);
                palette = (bit << 3);
                bit = bitGetVal(data3,col);
                palette |= (bit << 2);
                bit = bitGetVal(data2,col);
                palette |= (bit << 1);
                bit = bitGetVal(data1, col);
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

                writeToScreen(x+i, vCounter, getColourShade(red), getColourShade(green), getColourShade(blue));
            }
        }
    }
}

void TMS9918A::renderBackgroundMode2()
{
    WORD nameBase = getNameBase();

    // reg 3 contains colour table info
    BYTE reg3 = m_VDPRegisters[0x3];

    // reg 4 contains the pattern table info
    BYTE reg4 = m_VDPRegisters[0x4];

    // if bit 2 is set of reg 4 the pattern table address starts at 0x2000 otherwise 0x0
    WORD pgBase = testBit(reg4, 2) ? 0x2000 : 0x0;

    // if bit 7 is set of reg 3 the colour table address starts at 0x2000 otherwise 0x0
    WORD colBase = testBit(reg3, 7) ? 0x2000 : 0x0;

    // bits 0 to 6 of reg 3 get anded over bits 1-7 of the character number
    BYTE colAnd = reg3 & 127;

    // bit 0-6 get anded over bit 1-7 of character number so shift left 1
    colAnd <<= 1;

    // make sure bit 0 is set to it doesnt affect bit 0 of character number as we only want to affect 1-7
    colAnd = bitSet(colAnd, 0);
    
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
            if (testBit(reg4, 1))
            {
                pgTable = 2;
            }
        }
        // we must be drawing the middle third
        else
        {
            // then use table 2 if bit 0 of reg 4 is set else use table 0
            if (testBit(reg4, 0))
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
            BYTE colNum = testBit(pixelLine, invert)?fore:back;

            if (colNum == 0)
            {
                continue;
            }

            BYTE red = 0;
            BYTE green = 0;
            BYTE blue = 0;

            GetOldStyleColour(colNum, red,green,blue);
            int xpos = (column * 8) + x;

            if (getScreenPixelColour(xpos, m_VCounter, 0) != SCREENBLANKCOLOUR)
            {
                continue;
            }

            if (xpos >= NUM_RES_HORIZONTAL)
            {
                continue;
            }

            writeToScreen(xpos, m_VCounter, red,green,blue);       
        }
    }
}

void TMS9918A::renderBackgroundMode4()
{
    WORD nameBase = getNameBase();
    int vCounter = m_VCounter;
    BYTE vScroll = m_VScroll; // v scrolling only gets updated outside active display
    BYTE hScroll = m_VDPRegisters[0x8];

    BYTE vStartingRow = vScroll >> 3;
    BYTE vFineScroll = vScroll & 0x7;
    BYTE hStartingCol = hScroll >> 3;
    BYTE hFineScroll = hScroll & 0x7;

    bool limitVScroll = isRegBitSet(0, 7);
    bool limitHScroll = isRegBitSet(0, 6);

    int row = vCounter;
    row /= 8;

    bool maskFirstColumn = isRegBitSet(0,5);

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
                xpos = xpos % m_width;
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

                int mod = (m_height == NUM_RES_VERTICAL) ? 28 : 32;
                vrow = vrow % mod;
            }           

            int col = column;
            
            WORD nameBaseOffset = nameBase;
            nameBaseOffset += vrow * 64; //each scanline has 32 tiles (1 tile per column) but 1 tile is 2 bytes in memory
            nameBaseOffset += col * 2; // each tile is two bytes in memory

            WORD tileData = m_VRAM[nameBaseOffset+1] << 8;
            tileData |= m_VRAM[nameBaseOffset];

            bool hiPriority = testBit(tileData,12);
            bool useSpritePalette = testBit(tileData,11);
            bool vertFlip = testBit(tileData,10);
            bool horzFlip = testBit(tileData,9);
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
            BYTE bit = bitGetVal(data4,colourbit);
            palette = (bit << 3);
            bit = bitGetVal(data3,colourbit);
            palette |= (bit << 2);
            bit = bitGetVal(data2,colourbit);
            palette |= (bit << 1);
            bit = bitGetVal(data1, colourbit);
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
            if (!masking && !hiPriority && (getScreenPixelColour(xpos, m_VCounter, 0) != SCREENBLANKCOLOUR))
            {
                continue;
            }

            if (xpos >= NUM_RES_HORIZONTAL)
            {
                continue;
            }

            writeToScreen(xpos,vCounter, getColourShade(red), getColourShade(green), getColourShade(blue));
        }
        hStartingCol = (hStartingCol + 1) % 32;
    }
}

bool TMS9918A::isRegBitSet(int reg, BYTE bit)
{
    return testBit(m_VDPRegisters[reg], bit);
}

void TMS9918A::setSpriteOverflow()
{
    m_status = bitSet(m_status, 6);
}

void TMS9918A::setSpriteCollision()
{
    m_status = bitSet(m_status, 5);
}

WORD TMS9918A::getSATBase() const
{
    // returns the base address of the sprite attribute table
    BYTE reg5 = m_VDPRegisters[0x5];

    // bits 7 and 0 are ignored
    reg5 = bitReset(reg5, 7);
    reg5 = bitReset(reg5, 0);

    WORD res = reg5 << 7;
    return res;
}

WORD TMS9918A::getNameBase() const
{
    // returns the base address of the name table
    BYTE reg2 = m_VDPRegisters[0x2];

    // bit 0 is ignored so is top nibble
    reg2 &= 0xF;
    reg2 = bitReset(reg2,0);

    if (m_height != NUM_RES_VERTICAL)
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

BYTE TMS9918A::getColourShade(BYTE val) const
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

BYTE TMS9918A::getVDPMode() const
{
    BYTE res = 0;
    res |= bitGetVal(m_VDPRegisters[0x0],2) << 3;
    res |= bitGetVal(m_VDPRegisters[0x1],3) << 2;
    res |= bitGetVal(m_VDPRegisters[0x0],1) << 1;
    res |= bitGetVal(m_VDPRegisters[0x1],4);
    return res;
}

void TMS9918A::writeToScreen(BYTE x, BYTE y,BYTE red, BYTE green, BYTE blue)
{
    auto idx = y * (NUM_RES_HORIZONTAL * BYTES_PER_CHANNEL) + (BYTES_PER_CHANNEL * x);
    m_buffer[idx] = red;
    m_buffer[idx+1] = green;
    m_buffer[idx+2] = blue;
}

BYTE TMS9918A::getScreenPixelColour(BYTE x, BYTE y, int index) const
{
    auto idx = y * (NUM_RES_HORIZONTAL * BYTES_PER_CHANNEL) + (BYTES_PER_CHANNEL * x);
    return m_buffer[idx + index];
}

BYTE TMS9918A::getVJump() const
{
    if (m_isPAL)
    {
        if (m_height == NUM_RES_VERTICAL)
        {
            return 0xF2;
        }
        else if (m_height == NUM_RES_VERT_MED)
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
        if (m_height == NUM_RES_VERTICAL)
        {
            return 0xDA;
        }
        else if (m_height == NUM_RES_VERT_MED)
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

BYTE TMS9918A::getVJumpTo() const
{
    if (m_isPAL)
    {
        if (m_height == NUM_RES_VERTICAL)
        {
            return 0xBA;
        }
        else if (m_height == NUM_RES_VERT_MED)
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
        if (m_height == NUM_RES_VERTICAL)
        {
            return 0xD5;
        }
        else if (m_height == NUM_RES_VERT_MED)
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

void TMS9918A::dumpVRAM()
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

void TMS9918A::drawMode2Sprite(const WORD& address, BYTE x, BYTE line, BYTE colour)
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
        if (getScreenPixelColour(xpos, m_VCounter, 0) != SCREENBLANKCOLOUR)
        {
            m_status = bitSet(m_status,5);
            continue;
        }

        if (!testBit(drawLine, invert))
        {
            continue;
        }

        writeToScreen(xpos, m_VCounter, red,green,blue);
    }
}

void TMS9918A::setMode2IllegalSprites(BYTE sprite)
{
    m_status = bitSet(m_status, 6);
    m_status &= 0xE0; // turn off last 5 bits
    m_status |= sprite; // puts sprite into last 5 bits
}