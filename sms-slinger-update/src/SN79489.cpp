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

namespace
{
    int parity(WORD val)
    {
        val ^= val >> 8;
        val ^= val >> 4;
        val ^= val >> 2;
        val ^= val >> 1;
        return val;
    }
}

SN79489::SN79489()
    : m_buffer          (BUFFERSIZE),
    m_latchedChannel    (Channel::Zero),
    m_isToneLatched     (false),
    m_currentBufferPos  (0),
    m_LFSR              (0),
    m_clockInfo         (0),
    m_incomingCycleCount(0),
    m_sampler           (223500.0, 44100.0, 840) //measuring the timing says the actual speed is slightly less than 224000
{
    constexpr float MaxVolume = 1.f / Channel::Count; //so that when all tones are playing we're never more than 1
    constexpr float TwodBScalingFactor = 0.79432823f; //each volume setting gets lower by 2 decibels

    float vol = MaxVolume;

    for (int i = 0; i < 15; ++i)
    {
        vol *= TwodBScalingFactor;
        m_volumeTable[i] = vol;
    }

    // volume 15 is silent
    m_volumeTable[15] = 0.f;
    std::fill(m_mixerVolumes.begin(), m_mixerVolumes.end(), 0.5f);

    reset();
}

//public
void SN79489::writeData(BYTE data)
{
    // if bit 7 is set the it updates the latch
    if (testBit(data, 7))
    {
        // the channel is indentified by the 6 and 5 bits
        int channel = data;
        channel >>= 5;

        // turn off top bit
        channel &= 0x3;
        m_latchedChannel = channel;

        // if bit 4 is set then the channel we're latching volume, otherwise tone
        m_isToneLatched = testBit(data, 4) ? false : true;
        
        // bottom 4 bits are the data to be updated
        BYTE channelData = data & 0xF;

        if (m_isToneLatched)
        {
            if (m_latchedChannel == Tones::Noise)
            {
                // noise register is only 4 bits, so you dont need to keep the top nibble (as there isnt one)
                m_tones[Tones::Noise] = channelData;
                m_LFSR = 0x8000;
            }
            else
            {
                WORD currentValue = m_tones[m_latchedChannel];

                // we want to keep the top 12 bits (technically 10 bits) the same value, but replace the bottom 4
                currentValue &= 0xFFF0;

                // update bottom 4 bits
                currentValue |= channelData;

                m_tones[m_latchedChannel] = currentValue;
            }
        }
        else
        {
            BYTE currentValue = m_volume[m_latchedChannel];

            // we want to keep the top nibble the same
            currentValue &= 0xF0;

            // update bottom nibble
            currentValue |= channelData;

            m_volume[m_latchedChannel] = currentValue;
        }

    }

    // we're updating the currently latched register
    else
    {
        WORD channelData = 0;

        // the data to update with is the bottom 6 bits of the data being passed in
        channelData = (data & 0x3F);

        if (m_isToneLatched)
        {
            if (m_latchedChannel == Tones::Noise)
            {
                m_tones[Tones::Noise] = (data & 0xF);
                m_LFSR = 0x8000;
            }
            else
            {
                WORD currentValue = m_tones[m_latchedChannel];
                BYTE currentLowNibble = currentValue & 0xF;

                // update the top 6 bits (10 bit register) of the channel with the low 6 bits of the data
                channelData <<= 4;
                
                // we dont want to modify the low 4 bits of what was previously there
                channelData |= currentLowNibble;
                m_tones[m_latchedChannel] = channelData;
            }
        }
        else
        {       
            m_volume[m_latchedChannel] = data & 0xF;
        }
    }
}

void SN79489::reset()
{
    std::fill(m_buffer.begin(), m_buffer.end(), 0.f);
    std::fill(m_tones.begin(), m_tones.end(), 0);
    std::fill(m_counters.begin(), m_counters.end(), 0);
    std::fill(m_volume.begin(), m_volume.end(), 0x0F);
    std::fill(m_polarity.begin(), m_polarity.end(), 1);

    m_clockInfo = 0;
    m_latchedChannel = Channel::Zero;
    m_isToneLatched = true;
    m_currentBufferPos = 0;
    m_incomingCycleCount = 0;

    m_LFSR = 0x8000;
}

void SN79489::update(int cyclesMac)
{
    constexpr int sampleRate = 16; //sound chip runs 1/16 the update speed

    m_incomingCycleCount += cyclesMac;
    auto updateCount = m_incomingCycleCount / sampleRate;
    m_incomingCycleCount %= sampleRate;

#ifdef SMS_DEBUG
    m_clockInfo += updateCount;

    //static float accum = 0.f;
    //accum += m_timer.restart();
    //while (accum > 1.f)
    //{
    //    accum -= 1.f;
    //    std::cout << m_clockInfo << std::endl;
    //    m_clockInfo = 0;
    //}
#endif

    for (auto l = 0; l < updateCount; ++l)
    {
        //tone channels
        float tone = 0.f;

        for (int i = 0; i < Channel::Three; i++)
        {
            if (m_tones[i] == 0)
            {
                continue;
            }

            m_counters[i]--;

            if (m_counters[i] <= 0)
            {
                m_counters[i] = m_tones[i];
                m_polarity[i] *= -1;
            }

            tone += m_volumeTable[m_volume[i]] * m_polarity[i] * m_mixerVolumes[i];
        }

        //emulate noise
        if (m_tones[Tones::Noise] != 0)
        {
            m_counters[Tones::Noise]--;

            if (m_counters[Tones::Noise] <= 0)
            {
                WORD freq = m_tones[Tones::Noise];
                freq &= 0x3;

                int count = 0;
                switch (freq)
                {
                case 0: count = 0x10; break;
                case 1: count = 0x20; break;
                case 2: count = 0x40; break;
                case 3: count = m_tones[Channel::Two]; break;
                default: break;
                }

                m_counters[Tones::Noise] = count;
                m_polarity[Tones::Noise] *= -1;

                //if the polarity changed from -1 to 1 then shift the random number
                if (m_polarity[Tones::Noise] == 1)
                {
                    bool isWhiteNoise = testBit(m_tones[Tones::Noise], 2);

                    //not sure where the tapped bits value is coming from here:
                    //according to https://www.smspower.org/uploads/Development/SN76489-20030421.txt
                    //the master system is fixed at 0x0009, which in this instance
                    //gives an audibly more pleasing sound - M

                    /*WORD tappedBits = bitGetVal(m_tones[Tones::Noise], 0);
                    tappedBits |= (bitGetVal(m_tones[Tones::Noise], 3) << 3);*/

                    static constexpr WORD tappedBits = 0x0009;

                    m_LFSR = (m_LFSR >> 1) | ((isWhiteNoise ? parity(m_LFSR & tappedBits) : (m_LFSR & 1)) << 15);
                }
            }

            tone += m_volumeTable[m_volume[Tones::Noise]] * (m_LFSR & 1) * m_mixerVolumes[MixerChannel::Noise];
        }

        m_sampler.push(tone);
    }

    if (m_sampler.pending())
    {
        m_buffer[m_currentBufferPos] = static_cast<float>(m_sampler.pop()) * m_mixerVolumes[MixerChannel::Master];   
        m_currentBufferPos = (m_currentBufferPos + 1) % BUFFERSIZE;      
    }       
}

void SN79489::audioCallback(std::uint8_t* buffer, std::int32_t len)
{
    std::memcpy(buffer, m_buffer.data(), len);
    m_currentBufferPos = 0;
}

SN79489::SampleBuffer SN79489::getSamples() const
{
    SampleBuffer buf;
    buf.data = m_buffer.data();
    buf.size = m_currentBufferPos * sizeof(float);

    //this assumes all the data has been consumed by the caller
    //so we start writing from the beginning again
    m_currentBufferPos = 0;

    return buf;
}

void SN79489::setVolume(SN79489::MixerChannel::Label channel, float vol)
{
    m_mixerVolumes[channel] = std::max(0.f, std::min(1.f, vol));
}

//private