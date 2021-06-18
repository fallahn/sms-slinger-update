/*-----------------------------------------------------------------------

Matt Marchant 2021
http://trederia.blogspot.com

This class is part of crogine: https://github.com/fallahn/crogine

Zlib license.

This software is provided 'as-is', without any express or
implied warranty.In no event will the authors be held
liable for any damages arising from the use of this software.

Permission is granted to anyone to use this software for any purpose,
including commercial applications, and to alter it and redistribute
it freely, subject to the following restrictions :

1. The origin of this software must not be misrepresented;
you must not claim that you wrote the original software.
If you use this software in a product, an acknowledgment
in the product documentation would be appreciated but
is not required.

2. Altered source versions must be plainly marked as such,
and must not be misrepresented as being the original software.

3. This notice may not be removed or altered from any
source distribution.

-----------------------------------------------------------------------*/

template <>
inline std::string ConfigProperty::getValue<std::string>() const
{
    return m_value;
}

template <>
inline std::int32_t ConfigProperty::getValue<std::int32_t>() const
{
    std::int32_t retVal;
    std::istringstream is(m_value);
    if (is >> retVal) return retVal;
    return 0;
}

template <>
inline float ConfigProperty::getValue<float>() const
{
    float retVal;
    std::istringstream is(m_value);
    if (is >> retVal) return retVal;
    return 0.f;
}

template <>
inline bool ConfigProperty::getValue<bool>() const
{
    return (m_value == "true");
}