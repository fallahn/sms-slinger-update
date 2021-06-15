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
#include "MasterSystem.h"
#include "Emulator.h"
#include "TMS9918A.h"

static const int WINDOWWIDTH = 256;
static const int WINDOWHEIGHT = 192;
static const int SCREENSCALE = 1;


/////////////////////////////////////////////////////////////////////////

MasterSystem* MasterSystem::m_Instance = nullptr;

MasterSystem* MasterSystem::CreateInstance()
{
    if (m_Instance == nullptr)
    {
        m_Instance = new MasterSystem();
    }
    return m_Instance;
}

/////////////////////////////////////////////////////////////////////////

MasterSystem::MasterSystem(void) :
    m_Emulator(NULL)
    ,m_UseGFXOpt(false),
    m_Width(0),
    m_Height(0)
{
    m_Emulator = Emulator::CreateInstance();
}

/////////////////////////////////////////////////////////////////////////

MasterSystem::~MasterSystem(void)
{
    delete m_Emulator;
}

/////////////////////////////////////////////////////////////////////////

bool MasterSystem::CreateSDLWindow()
{
    m_Width = WINDOWWIDTH*SCREENSCALE;
    m_Height = WINDOWHEIGHT*SCREENSCALE;
    if(SDL_Init(SDL_INIT_EVERYTHING) < 0)
    {
        return false;
    }
    if(SDL_SetVideoMode(m_Width, m_Height, 8, SDL_OPENGL) == NULL)
    {
        return false;
    }

    InitGL();

    SDL_WM_SetCaption("Sega Master System", NULL);
    return true;
}

/////////////////////////////////////////////////////////////////////////

void MasterSystem::InitGL()
{
    glViewport(0, 0, m_Width, m_Height);
    glMatrixMode(GL_MODELVIEW);
    glLoadIdentity();
    glOrtho(0, m_Width, m_Height, 0, -1.0, 1.0);
    glClearColor(0, 0, 0, 1.0);
    glClear(GL_COLOR_BUFFER_BIT|GL_DEPTH_BUFFER_BIT);
    glShadeModel(GL_FLAT);

    glEnable(GL_TEXTURE_2D);
    glDisable(GL_DEPTH_TEST);
    glDisable(GL_CULL_FACE);
    glDisable(GL_DITHER);
    glDisable(GL_BLEND);
}

/////////////////////////////////////////////////////////////////////////

void MasterSystem::StartRom(const char* path)
{
    m_Emulator->Reset();
    m_Emulator->InsertCartridge(path);
}

/////////////////////////////////////////////////////////////////////////

void MasterSystem::BeginGame(int fps, bool useGFXOpt)
{
    m_UseGFXOpt = useGFXOpt;
    m_Emulator->SetGFXOpt(useGFXOpt);
    RomLoop(fps);
}

/////////////////////////////////////////////////////////////////////////

static unsigned int fpstime = 0;
static int count = 0;
static bool first = true;

bool LogFrameRate()
{
    bool res = false;
    if (first)
    {
        first = false;
        fpstime = SDL_GetTicks();
    }
    unsigned int fpscurrent = SDL_GetTicks();
    if((fpstime + 1000) < fpscurrent)
    {
        fpstime = fpscurrent;
        char buffer[255];
        sprintf(buffer, "FPS %d", count);
        count = 0;
        SDL_WM_SetCaption(buffer, NULL);
        res = true;
    }
    count++;
    return res;
}

/////////////////////////////////////////////////////////////////////////

void MasterSystem::RomLoop(int fps)
{
    bool quit = false;
    SDL_Event event;
    bool sync=true;

    if (fps == -1)
        sync = false;

    unsigned int time2 = SDL_GetTicks();

    while (!quit)
    {
        SDL_PollEvent(&event);
        
        HandleInput(event);

        if(event.type == SDL_QUIT)
        {
            quit = true;
        }

        unsigned int current = SDL_GetTicks();


        if (sync && (time2+(1000/fps))<current)
        {
            m_Emulator->Update();
            RenderGame();
            time2=current;
            if (LogFrameRate())
            {
                m_Emulator->DumpClockInfo();
            }
        }
        else if(!sync)
        {
            m_Emulator->Update();
            RenderGame();
            LogFrameRate();
        }
    }
    SDL_Quit();
}


/////////////////////////////////////////////////////////////////////////

void MasterSystem::RenderGame()
{   

    if (TMS9918A::m_FrameToggle && !TMS9918A::m_ScreenDisabled)
    {
        int width = SCREENSCALE * m_Emulator->GetGraphicChip().GetWidth();
        int height = SCREENSCALE * m_Emulator->GetGraphicChip().GetHeight();

        if (width != m_Width || height != m_Height)
        {
            m_Width = width;
            m_Height = height;
            SDL_SetVideoMode(m_Width, m_Height, 8, SDL_OPENGL);
        }

        glClear(GL_COLOR_BUFFER_BIT | GL_DEPTH_BUFFER_BIT);
        glLoadIdentity();
        glRasterPos2i(-1, 1);
        glPixelZoom(1, -1);
        if (height == SCREENSCALE*TMS9918A::NUM_RES_VERTICAL)
            glDrawPixels(m_Width, m_Height, GL_RGB, GL_UNSIGNED_BYTE, m_Emulator->GetGraphicChip().m_ScreenStandard);
        else if (height == SCREENSCALE*TMS9918A::NUM_RES_VERT_MED)
            glDrawPixels(m_Width, m_Height, GL_RGB, GL_UNSIGNED_BYTE, m_Emulator->GetGraphicChip().m_ScreenMed);
        else if (height == SCREENSCALE*TMS9918A::NUM_RES_VERT_HIGH)
            glDrawPixels(m_Width, m_Height, GL_RGB, GL_UNSIGNED_BYTE, m_Emulator->GetGraphicChip().m_ScreenHigh);
        
        SDL_GL_SwapBuffers();
    }

}

/////////////////////////////////////////////////////////////////////////

unsigned char MasterSystem::GetMemoryByte(int i)
{
    return m_Emulator->ReadMemory(i);
}

/////////////////////////////////////////////////////////////////////////

void MasterSystem::HandleInput(const SDL_Event& event)
{
    if(event.type == SDL_KEYDOWN)
    {
        int key = -1;
        int player = 1;
        switch(event.key.keysym.sym)
        {
            case SDLK_a : key = 4; break;
            case SDLK_s : key = 5; break;
            case SDLK_RIGHT : key = 3; break;
            case SDLK_LEFT : key = 2; break;
            case SDLK_UP : key = 0; break;
            case SDLK_DOWN : key = 1; break;
            case SDLK_BACKSPACE : m_Emulator->ResetButton(); break;

            case SDLK_KP4 : player = 2; key = 0; break; // left
            case SDLK_KP6 : player = 2; key = 1; break; // right
            case SDLK_KP7 : player = 2; key = 2; break; // fire a
            case SDLK_KP9 : player = 2; key = 3; break; // fire b
            case SDLK_KP8 : player = 1; key = 6; break; // up (although marked as player 1 it is player 2 but using overlapped ports)
            case SDLK_KP2 : player = 1; key = 7; break; // down (although marked as player 1 it is player 2 but using overlapped ports)
            default: break;
        }
        if (key != -1)
        {
            m_Emulator->SetKeyPressed(player,key);
        }
    }
    //If a key was released
    else if(event.type == SDL_KEYUP)
    {
        int key = -1;
        int player = 1;
        switch(event.key.keysym.sym)
        {
            case SDLK_a : key = 4; break;
            case SDLK_s : key = 5; break;
            case SDLK_RIGHT : key = 3; break;
            case SDLK_LEFT : key = 2; break;
            case SDLK_UP : key = 0; break;
            case SDLK_DOWN : key = 1; break;
            case SDLK_BACKSPACE : key = 4; player = 2;break;
            case SDLK_KP4 : player = 2; key = 0; break; // left
            case SDLK_KP6 : player = 2; key = 1; break; // right
            case SDLK_KP7 : player = 2; key = 2; break; // fire a
            case SDLK_KP9 : player = 2; key = 3; break; // fire b
            case SDLK_KP8 : player = 1; key = 6; break; // up (although marked as player 1 it is player 2 but using overlapped ports)
            case SDLK_KP2 : player = 1; key = 7; break; // down (although marked as player 1 it is player 2 but using overlapped ports)
            default: break;
        }
        if (key != -1)
        {
            m_Emulator->SetKeyReleased(player,key);
        }
    }

}

/////////////////////////////////////////////////////////////////////////
