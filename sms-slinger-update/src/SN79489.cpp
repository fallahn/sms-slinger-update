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
#include "SN79489.hpp"
#include "LogMessages.hpp"
#include "Emulator.hpp"

#include <cmath>
#include <cstring>
#include <iostream>

////////////////////////////////////////////////////////////////////

SN79489::SN79489()
    : m_Buffer          (BUFFERSIZE),
    m_LatchedChannel    (CHANNEL::CHANNEL_ZERO),
    m_IsToneLatched     (false),
    m_CurrentBufferPos  (0),
    m_Cycles            (0.f),
    m_LFSR              (0),
    m_ClockInfo         (0),
    m_BufferUpdateCount (0.f),
    m_UpdateBufferLimit (0.f)
{
    constexpr int maxVolume = 8000; // there are 4 channels and the output is a signed 16 bit num giving a range of -32,000 + 32,000 so 4 * 8000 fits into the range
    constexpr double TwodBScalingFactor = 0.79432823; // each volume setting gets lower by 2 decibels

    double vol = maxVolume;

    for (int i = 0; i < 15; ++i)
    {
        m_VolumeTable[i] = static_cast<int>(vol);
        vol *= TwodBScalingFactor;
    }

    // volume 15 is silent
    m_VolumeTable[15] = 0;

    Reset();
    SDL_PauseAudio(1);

    // strange calculation works out how many sound clock cycles is needed before we need to
    // add a new element to the playback buffer.

    // the amount of times that sdl will request the buffer to be filled up in a second
    constexpr float sdlCallbakFreq = (FREQUENCY / BUFFERSIZE) + 1;

    // the clockSpeed of the sound chip is 3.3Mhz / 16
    constexpr float clockSpeed = 220000.f;


    float updateBufferLimit = clockSpeed / sdlCallbakFreq;
    updateBufferLimit /= BUFFERSIZE; 
    m_UpdateBufferLimit = updateBufferLimit;
}

////////////////////////////////////////////////////////////////////

void SN79489::Reset()
{
    m_BufferUpdateCount = 0;
    std::fill(m_Buffer.begin(), m_Buffer.end(), 0);
    //std::memset(m_Buffer, 0, sizeof(m_Buffer));
    std::memset(m_Tones, 0, sizeof(m_Tones));
    std::memset(m_Counters, 0, sizeof(m_Counters));
    m_ClockInfo = 0;
    
    for (int i = 0; i < 4; i++)
    {
        m_Volume[i] = 0xF;
        m_Polarity[i] = 1;
    }
    m_LatchedChannel = CHANNEL_ZERO;
    m_IsToneLatched = true;
    m_CurrentBufferPos = 0;
    m_Cycles = 0;
    m_LFSR = 0x8000;
    SDL_CloseAudio();
    OpenSDLAudioDevice();
}

////////////////////////////////////////////////////////////////////

void SN79489::OpenSDLAudioDevice()
{
    SDL_AudioSpec as;
    as.freq = FREQUENCY;
    as.format = AUDIO_S16SYS;
    as.channels = 1;
    as.silence = 0;
    as.samples = BUFFERSIZE / 4;
    as.size = 0;
    as.callback = HandleSDLCallback;
    as.userdata = this;
    SDL_OpenAudio(&as, 0);
    SDL_PauseAudio(0);
}

////////////////////////////////////////////////////////////////////

void SN79489::WriteData(unsigned long int cycles, BYTE data)
{
    bool updateLatch = false;

    // if bit 7 is set the it updates the latch
    if (TestBit(data, 7))
    {
        updateLatch = true;

        // the channel is indentified by the 6 and 5 bits
        int channel = data;
        channel >>= 5;

        // turn off top bit
        channel &= 0x3;
        m_LatchedChannel = static_cast<CHANNEL>(channel);

        // if bit 4 is set then the channel we're latching volume, otherwise tone
        m_IsToneLatched = TestBit(data, 4) ? false : true;
        
        // bottom 4 bits are the data to be updated
        BYTE channelData = data & 0xF;

        if (m_IsToneLatched)
        {
            if (m_LatchedChannel == TONES_NOISE)
            {
                // noise register is only 4 bits, so you dont need to keep the top nibble (as there isnt one)
                m_Tones[TONES_NOISE] = channelData;
                m_LFSR = 0x8000;
            }
            else
            {
                WORD currentValue = m_Tones[m_LatchedChannel];

                // we want to keep the top 12 bits (technically 10 bits) the same value, but replace the bottom 4
                currentValue &= 0xFFF0;

                // update bottom 4 bits
                currentValue |= channelData;

                m_Tones[m_LatchedChannel] = currentValue;
            }
        }
        else
        {
            BYTE currentValue = m_Volume[m_LatchedChannel];

            // we want to keep the top nibble the same
            currentValue &= 0xF0;

            // update bottom nibble
            currentValue |= channelData;

            m_Volume[m_LatchedChannel] = currentValue;
        }

    }

    // we're updating the currently latched register
    else
    {
        WORD channelData = 0;

        // the data to update with is the bottom 6 bits of the data being passed in
        channelData = (data & 0x3F);

        if (m_IsToneLatched)
        {
            if (m_LatchedChannel == TONES_NOISE)
            {
                m_Tones[TONES_NOISE] = (data & 0xF);
                m_LFSR = 0x8000;
            }
            else
            {
                WORD currentValue = m_Tones[m_LatchedChannel];
                BYTE currentLowNibble = currentValue & 0xF;

                // update the top 6 bits (10 bit register) of the channel with the low 6 bits of the data
                channelData <<= 4;
                
                // we dont want to modify the low 4 bits of what was previously there
                channelData |= currentLowNibble;
                m_Tones[m_LatchedChannel] = channelData;
            }
        }
        else
        {       
            m_Volume[m_LatchedChannel] = data & 0xF;
        }
    }
}

////////////////////////////////////////////////////////////////////

void SN79489::HandleSDLCallback(void* userData, Uint8* buffer, int len)
{
    SN79489* data = static_cast<SN79489*>(userData);
    data->HandleSDLCallback(buffer, len);
}

////////////////////////////////////////////////////////////////////

void SN79489::HandleSDLCallback(Uint8* buffer, int len)
{
    static int lastLen = 0;

    if (len > lastLen)
    {
        lastLen = len;
        std::cout << "buffer requested " << len << std::endl;
    }

    std::memcpy(buffer, m_Buffer.data(), len);
    m_CurrentBufferPos = 0;
}

////////////////////////////////////////////////////////////////////

int parity (BYTE data)
{
    int bitCount = BitCount(data, 4);
    if ((bitCount % 2) == 0)
    {
        return 0;
    }
    return 1;
}

////////////////////////////////////////////////////////////////////

void SN79489::Update(float cyclesMac)
{
    constexpr int sampleRate = 16;
    cyclesMac /= sampleRate;

    m_Cycles += cyclesMac;

    float floor = std::floor(m_Cycles);
    m_ClockInfo += static_cast<unsigned long>(floor);

    m_Cycles -= floor; 

    m_BufferUpdateCount += floor;
    
    // tone channels
    signed short int tone = 0;
    
    for (int i = 0; i < CHANNEL_THREE; i++)
    {
        if (m_Tones[i] == 0)
        {
            continue;
        }

        m_Counters[i]-= static_cast<int>(floor);

        if (m_Counters[i] <= 0)
        {
            m_Counters[i] = m_Tones[i]; 
            m_Polarity[i] *= -1;
        }

        tone += m_VolumeTable[m_Volume[i]] * m_Polarity[i];
    }

    // emulate noise
    if (m_Tones[TONES_NOISE] != 0)
    {
        m_Counters[TONES_NOISE] -= static_cast<int>(floor);
        
        if (m_Counters[TONES_NOISE] <= 0)
        {
            WORD freq = m_Tones[TONES_NOISE];
            freq &= 0x3;

            int count = 0;
            switch(freq)
            {
                case 0: count = 0x10; break;
                case 1: count = 0x20; break;
                case 2: count = 0x40; break;
                case 3: count = m_Tones[CHANNEL_TWO]; break;
            }

            m_Counters[TONES_NOISE] = count;
            m_Polarity[TONES_NOISE] *= -1;

            // if the polarity changed from -1 to 1 then shift the random number
            if (m_Polarity[TONES_NOISE] == 1)
            {
                bool isWhiteNoise = TestBit(m_Tones[TONES_NOISE], 2);
                BYTE tappedBits = BitGetVal(m_Tones[TONES_NOISE], 0);
                tappedBits |= (BitGetVal(m_Tones[TONES_NOISE], 3) << 3);
                
                m_LFSR = (m_LFSR>>1) | ((isWhiteNoise ? parity(m_LFSR & tappedBits) : (m_LFSR & 1)) << 15);
            }
        }

        tone += m_VolumeTable[m_Volume[TONES_NOISE]] * (m_LFSR & 1);
    }

    if (m_BufferUpdateCount >= m_UpdateBufferLimit)
    {
        if (m_CurrentBufferPos < BUFFERSIZE)
        {
            m_Buffer[m_CurrentBufferPos] = tone;
        }
            
        m_CurrentBufferPos++;
        m_BufferUpdateCount = m_UpdateBufferLimit - m_BufferUpdateCount;       
    }       
}

////////////////////////////////////////////////////////////////////

void SN79489::DumpClockInfo()
{
    char buffer[255];
    std::memset(buffer, 0, sizeof(buffer));
    sprintf(buffer, "Sound Chip Clock Cycles Per Second: %u", m_ClockInfo);
    LogMessage::GetSingleton()->DoLogMessage(buffer, true);

    m_ClockInfo = 0;
}

////////////////////////////////////////////////////////////////////
