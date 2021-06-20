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

#include "Sampler.hpp"

Sampler::Sampler(double inFreq, double outFreq, std::size_t buffSize)
{
    reset(inFreq, outFreq, buffSize);
}

void Sampler::push(double sample)
{
    m_history[0] = m_history[1];
    m_history[1] = m_history[2];
    m_history[2] = m_history[3];
    m_history[3] = sample;

    while (m_fraction <= 1.0)
    {
        auto a = m_history[3] - m_history[2] - m_history[0] + m_history[1];
        auto b = m_history[0] - m_history[1] - a;
        auto c = m_history[2] - m_history[0];
        auto d = m_history[1];

        m_buffer.push(a * m_fraction * m_fraction * m_fraction + b * m_fraction * m_fraction + c * m_fraction + d);
        m_fraction += m_ratio;
    }
    m_fraction -= 1.0;
}

void Sampler::reset(double inputFreq, double outputFreq, std::size_t size)
{
    m_inputFreq = inputFreq;
    m_outputFreq = outputFreq;

    if (size == 0)
    {
        size = static_cast<std::size_t>(outputFreq * 0.02);
    }

    m_ratio = m_inputFreq / m_outputFreq;
    m_fraction = 0.0;
    m_history = {};
    m_buffer.resize(size, 0.0);
}