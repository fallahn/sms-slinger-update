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

#include <SDL2/SDL.h>

#include <vector>
#include <cstdint>

class SN79489
{
public:
                                SN79489();

            void                WriteData(unsigned long int cycles, BYTE data);
            void                Reset();
            void                Update(float cycles);
            void                DumpClockInfo();
private:

    enum CHANNEL
    {
        CHANNEL_ZERO,
        CHANNEL_ONE,
        CHANNEL_TWO,
        CHANNEL_THREE,
        CHANNEL_NUM
    } ;

    enum TONES
    {
        TONES_ZERO,
        TONES_ONE,
        TONES_TWO,
        TONES_NOISE,
        TONES_NUM
    };

    enum VOLUME
    {
        VOLUME_ZERO,
        VOLUME_ONE,
        VOLUME_TWO,
        VOLUME_THREE,
        VOLUME_NUM
    };
    
    static  void                HandleSDLCallback(void* userData, Uint8* buffer, int len);
            void                HandleSDLCallback(Uint8* buffer, int len);
            void                OpenSDLAudioDevice();

            static  constexpr unsigned int  BUFFERSIZE = 8096;
    static  constexpr int           FREQUENCY = 44100;
            std::vector<std::int16_t>    m_Buffer;
            WORD                m_Tones[TONES_NUM];
            BYTE                m_Volume[VOLUME_NUM];
            int                 m_Counters[CHANNEL_NUM];
            int                 m_Polarity[CHANNEL_NUM];
            CHANNEL             m_LatchedChannel;
            bool                m_IsToneLatched;
            int                 m_VolumeTable[16];
            int                 m_CurrentBufferPos;
            float               m_Cycles;
            WORD                m_LFSR;
            unsigned long int   m_ClockInfo;
            float               m_BufferUpdateCount;
            float               m_UpdateBufferLimit;
};
