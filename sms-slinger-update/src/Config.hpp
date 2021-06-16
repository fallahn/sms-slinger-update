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

typedef unsigned char BYTE;
typedef signed char SIGNED_BYTE;
typedef unsigned short int WORD;
typedef signed short int SIGNED_WORD;

template <typename T>
inline bool testBit(T inData, int inBitPosition)
{
    T lMsk = 1 << inBitPosition;
    return (inData & lMsk) ? true : false;
}

template <typename T>
inline T bitGet(T inData, int inBitPosition)
{
    T lMsk = 1 << inBitPosition;
    return lMsk;
}

template <typename T>
inline T bitGetVal(T inData, int inBitPosition)
{
    T lMsk = 1 << inBitPosition;
    return (inData & lMsk) ? 1 : 0;
}

template <typename T>
inline T bitSet(T inData, int inBitPosition)
{
    T lMsk = 1 << inBitPosition;
    inData |= lMsk;
    return inData;
}

template <typename T>
inline T bitReset(T inData, int inBitPosition)
{
    T lMsk = 1 << inBitPosition;
    inData &= ~lMsk;
    return inData;
}

template <typename T>
int bitCount(T inData, int totalBits)
{
    int res = 0;
    for (int i = 0; i < totalBits; i++)
    {
        if (testBit(inData, i))
        {
            res++;
        }
    }
    return res;
}
