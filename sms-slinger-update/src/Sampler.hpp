/*
    2017 Matt Marchant https://github.com/fallahn

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

/*
    Interpolated sampling of audio output.
    See https://stackoverflow.com/q/1125666/6740859
*/

#include "RingBuffer.hpp"

#include <array>

class Sampler final
{
public:

    Sampler(double inSampleRate, double outSampleRate, std::size_t bufferSize);
    double pop() { return m_buffer.pop(); }
    void push(double); //values should be normalised.
    bool pending() const { return m_buffer.pending(); }
    void reset(double, double, std::size_t);

private:
    double m_inputFreq = 0.0;
    double m_outputFreq = 0.0;
    double m_ratio = 0.0;
    double m_fraction = 0.0;
    std::array<double, 4> m_history = {};
    RingBuffer<double> m_buffer;
};