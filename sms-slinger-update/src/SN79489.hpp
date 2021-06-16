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
#include <array>
#include <cstdint>

class SN79489 final
{
public:
    SN79489();

    void writeData(unsigned long int cycles, BYTE data);
    void reset();
    void update(float cycles);
    void dumpClockInfo();

private:

    struct Channel final
    {
        enum
        {
            Zero,
            One,
            Two,
            Three,

            Count
        };
    } ;

    struct Tones final
    {
        enum
        {
            Zero,
            One,
            Two,
            Noise,

            Count
        };
    };

    struct Volume final
    {
        enum
        {
            Zero,
            One,
            Two,
            Three,

            Count
        };
    };

    std::vector<std::int16_t> m_buffer;

    std::array<WORD, Tones::Count> m_tones = {};
    std::array<BYTE, Volume::Count> m_volume = {};
    std::array<int, Channel::Count> m_counters = {};
    std::array<int, Channel::Count> m_polarity = {};
    std::array<int, 16> m_volumeTable = {};

    int m_latchedChannel;
    bool m_isToneLatched;
    int m_currentBufferPos;
    float m_cycles;
    WORD m_LFSR;
    unsigned long int m_clockInfo;
    float m_bufferUpdateCount;
    float m_updateBufferLimit;

    static void handleSDLCallback(void* userData, Uint8* buffer, int len);
    void handleSDLCallback(Uint8* buffer, int len);
    void openSDLAudioDevice();
};
