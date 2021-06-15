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

#include "GameSettings.h"
#include "LogMessages.h"
#include "useful_utils.h"

#include <cassert>
#include <fstream>

///////////////////////////////////////////////////////////////////////////////////////////

std::unique_ptr<GameSettings> GameSettings::m_Instance;

GameSettings* GameSettings::CreateInstance()
{
    if (m_Instance == nullptr)
    {
        m_Instance = std::make_unique<GameSettings>();
    }
    return m_Instance.get();
}

///////////////////////////////////////////////////////////////////////////////////////////

GameSettings* GameSettings::GetSingleton()
{
    if (m_Instance == nullptr)
    {
        LogMessage::GetSingleton()->DoLogMessage("Attempting to get the singleton of GameSettings when m_Instance is 0", true);
        assert(false);
    }
    return m_Instance.get();
}

///////////////////////////////////////////////////////////////////////////////////////////

GameSettings::GameSettings()
{
    m_Settings.clear();
}

///////////////////////////////////////////////////////////////////////////////////////////

bool GameSettings::LoadSettings()
{
    m_Settings.clear();

    std::fstream file;
    file.open("settings.ini", std::ios::in);

    // test if the file exists
    if (false == file.is_open())
    {
        // if in visual studio then it will be in the parent directory
        file.open("../settings.ini", std::ios::in);
        if (false == file.is_open())
        {
            LogMessage::GetSingleton()->DoLogMessage("Cannot find file settings.ini",true);
            return false;
        }
    }

    char buffer[256];
    std::string settingname;
    std::string settingvalue;

    // load in all the settings a line at a time
    while (false == file.eof())
    {
        std::memset(buffer, 0, sizeof(buffer));
        file.getline(buffer,256);
        
        char ch = buffer[0];

        // ignore blank lines, comments etc
        if ('\0' == ch || '*' == ch || '\r' == ch || '\n' == ch)
        {
            continue;
        }

        char* tokens = nullptr;
        tokens = std::strtok(buffer, ":");
        
        settingname = tokens;
        
        tokens = std::strtok(nullptr, "*");
        settingvalue = tokens;

        if (settingname.empty() || settingvalue.empty())
        {
            LogMessage::GetSingleton()->DoLogMessage("Settings.ini appears to be malformed", true);
            file.close();
            return false;
        }

        m_Settings.insert(std::make_pair(settingname,settingvalue));
    }

    file.close();

    return true;

}

///////////////////////////////////////////////////////////////////////////////////////////

std::string GameSettings::GetSetting(const std::string& setting) const
{
    std::string res = "";
    GAMESETTINGS::const_iterator it = m_Settings.find(setting);

    if (m_Settings.end() == it)
    {
        std::string message = "Could Not Find Setting ";
        message += setting;
        LogMessage::GetSingleton()->DoLogMessage(message.c_str(), true);
        return "";
    }

    return (*it).second;
}

///////////////////////////////////////////////////////////////////////////////////////////