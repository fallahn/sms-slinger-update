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
#include "MasterSystem.hpp"
#include "Emulator.hpp"
#include "TMS9918A.hpp"

#include <SDL.h>
#include <SDL_OpenGL.h>

#include <iostream>
#include <cassert>

namespace
{
    constexpr int WINDOWWIDTH = 256;
    constexpr int WINDOWHEIGHT = 192;
    constexpr int SCREENSCALE = 1;

    SDL_Window* window = nullptr;
    SDL_GLContext ctx = nullptr;
    SDL_AudioDeviceID audioDevice = 0;

    class HiResTimer final
    {
    public:
        HiResTimer()
        {
            m_start = m_current = SDL_GetPerformanceCounter();
            m_frequency = SDL_GetPerformanceFrequency();
        }

        float restart()
        {
            m_start = m_current;
            m_current = SDL_GetPerformanceCounter();
            return static_cast<float>(m_current - m_start) / static_cast<float>(m_frequency);
        }

    private:
        Uint64 m_start = 0;
        Uint64 m_current = 0;
        Uint64 m_frequency = 0;
    };
}

std::unique_ptr<MasterSystem> MasterSystem::m_instance;

MasterSystem::MasterSystem()
    : m_emulator(nullptr),
    m_width     (0),
    m_height    (0),
    m_useGFXOpt (false)
{
    m_emulator = Emulator::createInstance();
}

//public
MasterSystem* MasterSystem::createInstance()
{
    if (m_instance == nullptr)
    {
        m_instance = std::make_unique<MasterSystem>();
    }
    return m_instance.get();
}

bool MasterSystem::createSDLWindow()
{
    m_width = WINDOWWIDTH * SCREENSCALE;
    m_height = WINDOWHEIGHT * SCREENSCALE;

    if (SDL_Init(SDL_INIT_EVERYTHING) < 0)
    {
        std::cout << SDL_GetError() << std::endl;
        return false;
    }

    window = SDL_CreateWindow("Sega Master System", 
        SDL_WINDOWPOS_CENTERED, SDL_WINDOWPOS_CENTERED, 
        m_width, m_height, SDL_WINDOW_SHOWN | SDL_WINDOW_OPENGL | SDL_WINDOW_RESIZABLE);
    
    if (window == nullptr)
    {
        std::cout << SDL_GetError() << std::endl;
        SDL_Quit();
        return false;
    }

    ctx = SDL_GL_CreateContext(window);
    if (ctx == nullptr)
    {
        std::cout << SDL_GetError() << std::endl;
        SDL_Quit();
        return false;
    }


    initGL();
    initAudio();
    return true;
}

void MasterSystem::startRom(const char* path)
{
    SDL_PauseAudioDevice(audioDevice, 1);

    m_emulator->reset();
    m_emulator->insertCartridge(path);

    SDL_PauseAudioDevice(audioDevice, 0);
}

void MasterSystem::beginGame(int fps, bool useGFXOpt)
{
    m_useGFXOpt = useGFXOpt;
    m_emulator->setGFXOpt(useGFXOpt);
    
    fps > 0 ? romLoopFixedStep(fps) : romLoopFree();
}

//private
void MasterSystem::initGL()
{
    glViewport(0, 0, m_width, m_height);

    glClearColor(0.f, 0.f, 1.f, 1.f);
    glClear(GL_COLOR_BUFFER_BIT);
    glMatrixMode(GL_MODELVIEW);
    glLoadIdentity();
    glOrtho(0, m_width, m_height, 0, -1.0, 1.0);

    glEnable(GL_TEXTURE_2D);
    glEnable(GL_CULL_FACE);
    glDisable(GL_DEPTH_TEST);
    glDisable(GL_BLEND);
}

void MasterSystem::initAudio()
{
    SDL_AudioSpec as;
    as.freq = SN79489::FREQUENCY;
    as.format = AUDIO_S16SYS;
    as.channels = 1;
    as.silence = 0;
    as.samples = SN79489::BUFFERSIZE;
    as.size = 0;
    //as.callback = audioCallback;
    //as.userdata = &m_emulator->getSoundChip();

    audioDevice = SDL_OpenAudioDevice(nullptr, 0, &as, nullptr, 0);
    SDL_PauseAudioDevice(audioDevice, 1);
}

void MasterSystem::romLoopFixedStep(int fps)
{
    assert(fps > 0);
    bool quit = false;

    HiResTimer timer;
    const float FrameTime = 1.f / fps;
    float accumulator = 0.f;

    while (!quit)
    {
        SDL_Event event;
        while(SDL_PollEvent(&event)) 
        {
            if (event.type == SDL_QUIT
                || handleInput(event))
            {
                quit = true;
            }
        }

        accumulator += timer.restart();

        while (accumulator > FrameTime)
        {
            accumulator -= FrameTime;

            m_emulator->update();
            renderGame();
        }

        auto [data, size] = m_emulator->getSoundChip().getSamples();
        SDL_QueueAudio(audioDevice, data, size);
    }

    SDL_GL_DeleteContext(ctx);
    SDL_Quit();
}

void MasterSystem::romLoopFree()
{
    bool quit = false;

    while (!quit)
    {
        SDL_Event event;
        while (SDL_PollEvent(&event))
        {
            if (event.type == SDL_QUIT
                || handleInput(event))
            {
                quit = true;
            }
        }

        m_emulator->update();
        renderGame();

        auto [data, size] = m_emulator->getSoundChip().getSamples();
        SDL_QueueAudio(audioDevice, data, size);
    }

    SDL_GL_DeleteContext(ctx);
    SDL_Quit();
}

bool MasterSystem::handleInput(const SDL_Event& evt)
{
    if(evt.type == SDL_KEYDOWN)
    {
        int key = -1;
        int player = 0;
        switch (evt.key.keysym.sym)
        {
        case SDLK_ESCAPE: return true;

        case SDLK_a: key = 4; break;
        case SDLK_s: key = 5; break;
        case SDLK_RIGHT: key = 3; break;
        case SDLK_LEFT: key = 2; break;
        case SDLK_UP: key = 0; break;
        case SDLK_DOWN: key = 1; break;
        case SDLK_BACKSPACE: m_emulator->resetButton(); break;

        case SDLK_KP_4: player = 1; key = 0; break; // left
        case SDLK_KP_6: player = 1; key = 1; break; // right
        case SDLK_KP_7: player = 1; key = 2; break; // fire a
        case SDLK_KP_9: player = 1; key = 3; break; // fire b
        case SDLK_KP_8: player = 0; key = 6; break; // up (although marked as player 1 it is player 2 but using overlapped ports)
        case SDLK_KP_2: player = 0; key = 7; break; // down (although marked as player 1 it is player 2 but using overlapped ports)
        default: break;
        }
        if (key != -1)
        {
            m_emulator->setKeyPressed(player, key);
        }
    }
    //If a key was released
    else if(evt.type == SDL_KEYUP)
    {
        int key = -1;
        int player = 0;
        switch(evt.key.keysym.sym)
        {
        case SDLK_a: key = 4; break;
        case SDLK_s: key = 5; break;
        case SDLK_RIGHT: key = 3; break;
        case SDLK_LEFT: key = 2; break;
        case SDLK_UP: key = 0; break;
        case SDLK_DOWN: key = 1; break;
        case SDLK_BACKSPACE : key = 4; player = 2;break;
        case SDLK_KP_4: player = 1; key = 0; break; // left
        case SDLK_KP_6: player = 1; key = 1; break; // right
        case SDLK_KP_7: player = 1; key = 2; break; // fire a
        case SDLK_KP_9: player = 1; key = 3; break; // fire b
        case SDLK_KP_8: player = 0; key = 6; break; // up (although marked as player 1 it is player 2 but using overlapped ports)
        case SDLK_KP_2: player = 0; key = 7; break; // down (although marked as player 1 it is player 2 but using overlapped ports)
        default: break;
        }

        if (key != -1)
        {
            m_emulator->setKeyReleased(player, key);
        }
    }
    else if (evt.type == SDL_WINDOWEVENT)
    {
        if (evt.window.event == SDL_WINDOWEVENT_RESIZED)
        {
            auto w = evt.window.data1;
            auto h = evt.window.data2;

        }
    }

    return false;
}

void MasterSystem::renderGame()
{
    if (TMS9918A::frameToggle && !TMS9918A::screenDisabled)
    {
        int width = SCREENSCALE * m_emulator->getGraphicChip().getWidth();
        int height = SCREENSCALE * m_emulator->getGraphicChip().getHeight();

        if (width != m_width || height != m_height)
        {
            m_width = width;
            m_height = height;
            SDL_SetWindowSize(window, m_width, m_height);
        }

        glClear(GL_COLOR_BUFFER_BIT);
        glLoadIdentity();
        glRasterPos2i(-1, 1);
        glPixelZoom(1, -1);
        if (height == SCREENSCALE * TMS9918A::NUM_RES_VERTICAL)
        {
            glDrawPixels(m_width, m_height, GL_RGB, GL_UNSIGNED_BYTE, m_emulator->getGraphicChip().screenStandard);
        }
        else if (height == SCREENSCALE * TMS9918A::NUM_RES_VERT_MED)
        {
            glDrawPixels(m_width, m_height, GL_RGB, GL_UNSIGNED_BYTE, m_emulator->getGraphicChip().screenMed);
        }
        else if (height == SCREENSCALE * TMS9918A::NUM_RES_VERT_HIGH)
        {
            glDrawPixels(m_width, m_height, GL_RGB, GL_UNSIGNED_BYTE, m_emulator->getGraphicChip().screenHigh);
        }

        SDL_GL_SwapWindow(window);
    }
}