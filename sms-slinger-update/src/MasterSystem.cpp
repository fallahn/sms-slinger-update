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

#include "glad.hpp"

#include <SDL.h>

#include <iostream>
#include <cassert>

namespace
{
    const std::string Vertex = 
        R"(
        #version 330 core

        layout (location = 0) in vec2 a_position;
        layout (location = 1) in vec2 a_texCoord;

        out vec2 v_texCoord;

        void main()
        {
            v_texCoord = a_texCoord;
            gl_Position = vec4(a_position.x, a_position.y, 0.0, 1.0);
        })";

    const std::string Fragment = 
        R"(
        #version 330 core

        uniform sampler2D u_texture;

        in vec2 v_texCoord;

        out vec4 FragColour;

        void main()
        {
            FragColour = texture(u_texture, v_texCoord);
        })";

    constexpr int WINDOWWIDTH = 256;
    constexpr int WINDOWHEIGHT = 192;

    constexpr int WINDOWSCALE = 2; //TODO make this a variable

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

MasterSystem::MasterSystem()
    : m_emulator(nullptr),
    m_width     (0),
    m_height    (0),
    m_useGFXOpt (false),
    m_shader    (0),
    m_texture   (0),
    m_vao       (0),
    m_vbo       (0)
{
    m_emulator = Emulator::createInstance();
}

MasterSystem::~MasterSystem()
{
    if (m_vao)
    {
        glDeleteVertexArrays(1, &m_vao);
    }

    if (m_vbo)
    {
        glDeleteBuffers(1, &m_vbo);
    }

    if (m_shader)
    {
        glDeleteProgram(m_shader);
    }

    if (m_texture)
    {
        glDeleteTextures(1, &m_texture);
    }
}

//public
bool MasterSystem::createSDLWindow()
{
    m_width = WINDOWWIDTH;
    m_height = WINDOWHEIGHT;

    if (SDL_Init(SDL_INIT_EVERYTHING) < 0)
    {
        std::cout << SDL_GetError() << std::endl;
        return false;
    }

    window = SDL_CreateWindow("Sega Master System", 
        SDL_WINDOWPOS_CENTERED, SDL_WINDOWPOS_CENTERED, 
        m_width * WINDOWSCALE, m_height * WINDOWSCALE, SDL_WINDOW_SHOWN | SDL_WINDOW_OPENGL/* | SDL_WINDOW_RESIZABLE*/);
    
    if (window == nullptr)
    {
        std::cout << SDL_GetError() << std::endl;
        SDL_Quit();
        return false;
    }


    //for some reason this has to be done first on macOS
#ifdef __APPLE__
    SDL_GL_SetAttribute(SDL_GL_CONTEXT_PROFILE_MASK, SDL_GL_CONTEXT_PROFILE_CORE);
    SDL_GL_SetAttribute(SDL_GL_CONTEXT_MAJOR_VERSION, 3);
    SDL_GL_SetAttribute(SDL_GL_CONTEXT_MINOR_VERSION, 3);
    SDL_GL_SetAttribute(SDL_GL_DOUBLEBUFFER, 1);
#endif

    ctx = SDL_GL_CreateContext(window);
    if (ctx == nullptr)
    {
        std::cout << SDL_GetError() << std::endl;
        SDL_Quit();
        return false;
    }

#ifndef __APPLE__
    SDL_GL_SetAttribute(SDL_GL_CONTEXT_PROFILE_MASK, SDL_GL_CONTEXT_PROFILE_CORE);
    SDL_GL_SetAttribute(SDL_GL_CONTEXT_MAJOR_VERSION, 3);
    SDL_GL_SetAttribute(SDL_GL_CONTEXT_MINOR_VERSION, 3);
    SDL_GL_SetAttribute(SDL_GL_DOUBLEBUFFER, 1);
#endif

    if (!gladLoadGLLoader(SDL_GL_GetProcAddress))
    {
        std::cout << "Failed loading OpenGL" << std::endl;
        SDL_GL_DeleteContext(ctx);
        SDL_Quit();
        return false;
    }

    if (!initGL())
    {
        SDL_GL_DeleteContext(ctx);
        SDL_Quit();
        return false;
    }
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
bool MasterSystem::initGL()
{
    if (!loadShader())
    {
        std::cout << "Failed to create Shader!" << std::endl;
        return false;
    }

    if (!loadTexture())
    {
        std::cout << "Failed to create Texture!" << std::endl;
        return false;
    }

    if (!loadQuad())
    {
        std::cout << "Failed to createQuad!" << std::endl;
        return false;
    }


    glViewport(0, 0, m_width * WINDOWSCALE, m_height * WINDOWSCALE);

    glClearColor(0.f, 0.f, 1.f, 1.f);

    glEnable(GL_TEXTURE_2D);
    glEnable(GL_CULL_FACE);
    glDisable(GL_DEPTH_TEST);
    glDisable(GL_BLEND);

    return true;
}

bool MasterSystem::loadShader()
{
    //vert shader
    const char* vertSrc = Vertex.c_str();
    
    auto vertShader = glCreateShader(GL_VERTEX_SHADER);
    glShaderSource(vertShader, 1, &vertSrc, nullptr);
    glCompileShader(vertShader);

    std::int32_t success = -1;
    glGetShaderiv(vertShader, GL_COMPILE_STATUS, &success);
    if (!success)
    {
        std::cout << "Failed to compile Vertex Shader" << std::endl;
        return false;
    }

    //frag shader
    const char* fragSrc = Fragment.c_str();

    auto fragShader = glCreateShader(GL_FRAGMENT_SHADER);
    glShaderSource(fragShader, 1, &fragSrc, nullptr);
    glCompileShader(fragShader);

    glGetShaderiv(fragShader, GL_COMPILE_STATUS, &success);
    if (!success)
    {
        std::cout << "Failed to compile Fragment Shader" << std::endl;
        return false;
    }

    
    //link to program
    m_shader = glCreateProgram();
    glAttachShader(m_shader, vertShader);
    glAttachShader(m_shader, fragShader);
    glLinkProgram(m_shader);

    glGetProgramiv(m_shader, GL_LINK_STATUS, &success);
    if (!success)
    {
        std::cout << "Failed to link shader" << std::endl;
        glDeleteShader(fragShader);
        glDeleteShader(vertShader);

        return false;
    }

    glDeleteShader(fragShader);
    glDeleteShader(vertShader);

    return true;
}

bool MasterSystem::loadTexture()
{
    //this is the only texture we have so just set it once
    //here, instead of every frame.
    glActiveTexture(GL_TEXTURE0);


    glGenTextures(1, &m_texture);
    glBindTexture(GL_TEXTURE_2D, m_texture);

    glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_MIN_FILTER, GL_NEAREST);
    glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_MAG_FILTER, GL_NEAREST);
    glTexImage2D(GL_TEXTURE_2D, 0, GL_RGB, m_width, m_height, 0, GL_RGB, GL_UNSIGNED_BYTE, nullptr);


    glUseProgram(m_shader);
    glUniform1i(0, 0); //assume this is the only uniform, therefore 0. Norty.

    return true;
}

bool MasterSystem::loadQuad()
{
    std::array verts =
    {
        -1.f, 1.f,
        0.f, 0.f,

        -1.f, -1.f,
        0.f, 1.f,

        1.f, 1.f,
        1.f, 0.f,

        1.f, -1.f,
        1.f, 1.f
    };

    glGenVertexArrays(1, &m_vao);
    glGenBuffers(1, &m_vbo);

    glBindVertexArray(m_vao);

    glBindBuffer(GL_ARRAY_BUFFER, m_vbo);
    glBufferData(GL_ARRAY_BUFFER, verts.size() * sizeof(float), verts.data(), GL_STATIC_DRAW);

    //pos
    glVertexAttribPointer(0, 2, GL_FLOAT, GL_FALSE, 4 * sizeof(float), (void*)0);
    glEnableVertexAttribArray(0);

    //uv
    glVertexAttribPointer(1, 2, GL_FLOAT, GL_FALSE, 4 * sizeof(float), (void*)(2*sizeof(float)));
    glEnableVertexAttribArray(1);

    glBindVertexArray(0);
    glBindBuffer(GL_ARRAY_BUFFER, 0);

    return true;
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
    as.callback = nullptr; //we're pulling from the soundcard
    
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
        case SDLK_BACKSPACE : key = 4; player = 1;break;
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
            glViewport(0, 0, w, h);
        }
    }

    return false;
}

void MasterSystem::renderGame()
{
    if (TMS9918A::frameToggle && !TMS9918A::screenDisabled)
    {
        int width = m_emulator->getGraphicChip().getWidth();
        int height = m_emulator->getGraphicChip().getHeight();

        if (width != m_width || height != m_height)
        {
            m_width = width;
            m_height = height;
            SDL_SetWindowSize(window, m_width * WINDOWSCALE, m_height * WINDOWSCALE);

            //resize texture
            glTexImage2D(GL_TEXTURE_2D, 0, GL_RGB, m_width, m_height, 0, GL_RGB, GL_UNSIGNED_BYTE, nullptr);
        }

        //again, assuming we only have one texture that is always bound
        glTexSubImage2D(GL_TEXTURE_2D, 0, 0, 0, m_width, m_height, GL_RGB, GL_UNSIGNED_BYTE, m_emulator->getGraphicChip().getPixelBuffer());

        glClear(GL_COLOR_BUFFER_BIT);

        glUseProgram(m_shader);
        glBindVertexArray(m_vao);
        glDrawArrays(GL_TRIANGLE_STRIP, 0, 4);

        SDL_GL_SwapWindow(window);
    }
}