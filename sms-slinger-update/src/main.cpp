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

#include "MasterSystem.h"

int main (int argsc, char** argsv)
{
    /*LogMessage* lm = LogMessage::CreateInstance( );
    GameSettings* gs = GameSettings::CreateInstance( );
    gs->LoadSettings( );*/

    /*delete gs ;
    delete lm ;*/

    MasterSystem* sms = MasterSystem::CreateInstance();
    sms->CreateSDLWindow();
    sms->StartRom(/*gs->GetSetting("ROMName").c_str()*/"roms/lander2_v0C.sms");
    sms->BeginGame(/*atoi(gs->GetSetting("TargetFPS").c_str())*/50, false);
    
    return 0;
}
