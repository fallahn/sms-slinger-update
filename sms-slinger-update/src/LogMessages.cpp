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

#include "Config.h"
#include "LogMessages.h"

#include <cassert>
#include <cstdio>
#include <ctime>

////////////////////////////////////////////////////////////////////////////////////////

LogMessage* LogMessage::m_Instance = nullptr;

LogMessage* LogMessage::CreateInstance()
{
    if (m_Instance == nullptr)
    {
        m_Instance = new LogMessage();
    }
    return m_Instance;
}

////////////////////////////////////////////////////////////////////////////////////////

LogMessage* LogMessage::GetSingleton()
{
    assert(m_Instance != 0);
    return m_Instance;
}

////////////////////////////////////////////////////////////////////////////////////////

LogMessage::LogMessage()
    : m_LogFile (nullptr),
    m_FileNum   (1)
{
    Open();
}

////////////////////////////////////////////////////////////////////////////////////////

LogMessage::~LogMessage()
{
    if (m_LogFile != nullptr)
    {
        fclose((FILE*)m_LogFile);
    }
}

////////////////////////////////////////////////////////////////////////////////////////

void LogMessage::Open()
{
    char buffer[100];
    sprintf(buffer, "emulator%d.log", m_FileNum);
    m_FileNum++;
    m_LogFile = (_iobuf*)fopen(buffer, "w");
}

////////////////////////////////////////////////////////////////////////////////////////

void LogMessage::DoLogMessage(const char* message, bool logToConsole)
{
    if (m_LogFile != nullptr)
    {
        long long size = ftell(m_LogFile);
        if (size >= (30 * 1024 * 1024))
        {
            fclose(m_LogFile);
            Open();
        }
        fputs(message, (FILE*)m_LogFile);
        fputs("\r\n", (FILE*)m_LogFile);
    }

    if (!logToConsole)
    {
        return;
    }

    printf(message);
    printf("\n");
}

////////////////////////////////////////////////////////////////////////////////////////
