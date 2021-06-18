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
#include "ConfigFile.hpp"

#include "glad.hpp"
#include "imgui/imgui.h"
#include "imgui/imgui_impl_sdl.h"
#include "imgui/imgui_impl_opengl3.h"
#include "imgui/imgui_style.h"
#include "imgui/tinyfiledialogs.h"

#include <SDL.h>

#include <iostream>
#include <fstream>
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
    vec4 colour = texture(u_texture, v_texCoord);

    //process the colour fragment here...

    FragColour = colour;
})";

    std::string shaderError;

    enum DialogResult
    {
        Cancel, Yes, No
    };

    constexpr int WINDOW_WIDTH = TMS9918A::NUM_RES_HORIZONTAL;
    constexpr int WINDOW_HEIGHT = TMS9918A::NUM_RES_VERTICAL;
    bool fullScreen = false; //kludge to allow settings file to set this before window created.

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
    : m_emulator    (nullptr),
    m_width         (0),
    m_height        (0),
    m_windowScale   (1),
    m_useGFXOpt     (false),
    m_shader        (0),
    m_texture       (0),
    m_vao           (0),
    m_vbo           (0),
    m_showUI        (true),
    m_showOptions   (false),
    m_showEditor    (false),
    m_running       (false)
{
    m_emulator = Emulator::createInstance();

    loadSettings();
}

//public
bool MasterSystem::createSDLWindow()
{
    m_width = WINDOW_WIDTH;
    m_height = WINDOW_HEIGHT;

    if (SDL_Init(SDL_INIT_EVERYTHING) < 0)
    {
        std::cout << SDL_GetError() << std::endl;
        return false;
    }

    window = SDL_CreateWindow("Sega Master System", 
        SDL_WINDOWPOS_CENTERED, SDL_WINDOWPOS_CENTERED, 
        m_width * m_windowScale, m_height * m_windowScale, SDL_WINDOW_SHOWN | SDL_WINDOW_OPENGL);
    
    if (window == nullptr)
    {
        std::cout << SDL_GetError() << std::endl;
        SDL_Quit();
        return false;
    }
    if (fullScreen)
    {
        SDL_SetWindowFullscreen(window, SDL_WINDOW_FULLSCREEN_DESKTOP);
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

void MasterSystem::startRom(const std::string& path)
{
    SDL_PauseAudioDevice(audioDevice, 1);

    m_emulator->reset();
    m_emulator->insertCartridge(path.c_str());

    m_currentRom = path;

    SDL_PauseAudioDevice(audioDevice, 0);
    SDL_ClearQueuedAudio(audioDevice);
}

void MasterSystem::run(int fps, bool useGFXOpt)
{
    m_useGFXOpt = useGFXOpt;
    m_emulator->setGFXOpt(useGFXOpt);
    
    fps > 0 ? romLoopFixedStep(fps) : romLoopFree();
}

//private
bool MasterSystem::initGL()
{
    if (!m_currentShaderPath.empty())
    {
        std::ifstream file(m_currentShaderPath);
        if (file.is_open() && file.good())
        {
            std::string str((std::istreambuf_iterator<char>(file)), std::istreambuf_iterator<char>());
        
            if (!loadShader(str))
            {
                if (!loadShader(Fragment))
                {
                    std::cout << "Failed to create Shader!" << std::endl;
                    file.close();
                    return false;
                }
            }
        }
        file.close();
    }
    else if (!loadShader(Fragment))
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


    glViewport(0, 0, m_width * m_windowScale, m_height * m_windowScale);

    glClearColor(0.f, 0.f, 1.f, 1.f);

    glEnable(GL_TEXTURE_2D);
    glEnable(GL_CULL_FACE);
    glDisable(GL_DEPTH_TEST);
    glDisable(GL_BLEND);


    //set up imgui here while we're at it...
    ImGui::CreateContext();
    setImguiStyle(&ImGui::GetStyle());
    ImGui_ImplSDL2_InitForOpenGL(window, ctx);
    ImGui_ImplOpenGL3_Init("#version 330");

    m_textEditor.SetLanguageDefinition(TextEditor::LanguageDefinition::GLSL());
    m_textEditor.SetText(Fragment);

    return true;
}

bool MasterSystem::loadShader(const std::string& fragString)
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
    const char* fragSrc = fragString.c_str();

    auto fragShader = glCreateShader(GL_FRAGMENT_SHADER);
    glShaderSource(fragShader, 1, &fragSrc, nullptr);
    glCompileShader(fragShader);

    shaderError.clear();
    glGetShaderiv(fragShader, GL_COMPILE_STATUS, &success);
    if (!success)
    {
        char errStr[256];
        glGetShaderInfoLog(fragShader, 256, nullptr, errStr);
        shaderError = errStr;

        std::cout << "Failed to compile Fragment Shader" << std::endl;
        std::cout << shaderError << std::endl;
        return false;
    }

    
    //link to program
    auto shader = glCreateProgram();
    glAttachShader(shader, vertShader);
    glAttachShader(shader, fragShader);
    glLinkProgram(shader);

    glGetProgramiv(shader, GL_LINK_STATUS, &success);
    if (!success)
    {
        std::cout << "Failed to link shader" << std::endl;
        glDeleteShader(fragShader);
        glDeleteShader(vertShader);

        return false;
    }

    //remove the old one if it exists
    if (m_shader)
    {
        glDeleteProgram(m_shader);
    }

    m_shader = shader;
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
    glTexImage2D(GL_TEXTURE_2D, 0, GL_RGB, m_width, m_height, 0, GL_RGB, GL_UNSIGNED_BYTE, m_emulator->getGraphicChip().getPixelBuffer());


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
    m_running = true;

    HiResTimer timer;
    const float FrameTime = 1.f / fps;
    float accumulator = 0.f;

    while (m_running)
    {
        SDL_Event evt;
        while(SDL_PollEvent(&evt)) 
        {
            m_running = !(evt.type == SDL_QUIT || handleEvent(evt));
        }

        accumulator += timer.restart();

        while (accumulator > FrameTime)
        {
            accumulator -= FrameTime;
            m_emulator->update();
        }
        render();

        auto [data, size] = m_emulator->getSoundChip().getSamples();
        SDL_QueueAudio(audioDevice, data, size);
    }

    shutdown();
}

void MasterSystem::romLoopFree()
{
    m_running = true;

    while (m_running)
    {
        SDL_Event evt;
        while (SDL_PollEvent(&evt))
        {
            m_running = !(evt.type == SDL_QUIT || handleEvent(evt));
        }

        m_emulator->update();
        render();

        auto [data, size] = m_emulator->getSoundChip().getSamples();
        SDL_QueueAudio(audioDevice, data, size);
    }

    shutdown();
}

bool MasterSystem::handleEvent(const SDL_Event& evt)
{
    ImGui_ImplSDL2_ProcessEvent(&evt);

    if(evt.type == SDL_KEYDOWN)
    {
        if (evt.key.keysym.mod & KMOD_ALT)
        {
            switch (evt.key.keysym.sym)
            {
            default: break;
            case SDLK_o:
                m_showOptions = true;
                break;
            case SDLK_F4:
                return true; //returning true from this func quits
            }
        }
        else if (evt.key.keysym.mod & KMOD_CTRL)
        {
            switch (evt.key.keysym.sym)
            {
            default: break;
            case SDLK_o:
                browseRom();
                break;
            case SDLK_r:
                if (!m_currentRom.empty())
                {
                    startRom(m_currentRom);
                }
                break;
            }
        }

        int key = -1;
        int player = 0;
        switch (evt.key.keysym.sym)
        {
        case SDLK_INSERT:
            m_showUI = !m_showUI;
            break;
        case SDLK_ESCAPE:
#ifdef SMS_DEBUG
            return true;
#else
            m_showUI = !m_showUI;
            break;
#endif //debug

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

void MasterSystem::render()
{
    doImGui();

    ImGui::Render();
    glClear(GL_COLOR_BUFFER_BIT);

    if (TMS9918A::frameToggle && !TMS9918A::screenDisabled)
    {
        int width = m_emulator->getGraphicChip().getWidth();
        int height = m_emulator->getGraphicChip().getHeight();

        if (width != m_width || height != m_height)
        {
            m_width = width;
            m_height = height;

            auto w = m_width * m_windowScale;
            auto h = m_height * m_windowScale;
            SDL_SetWindowSize(window, w, h);
            glViewport(0, 0, w, h);

            //resize texture
            glTexImage2D(GL_TEXTURE_2D, 0, GL_RGB, m_width, m_height, 0, GL_RGB, GL_UNSIGNED_BYTE, nullptr);
        }

        //again, assuming we only have one texture that is always bound
        glTexSubImage2D(GL_TEXTURE_2D, 0, 0, 0, m_width, m_height, GL_RGB, GL_UNSIGNED_BYTE, m_emulator->getGraphicChip().getPixelBuffer());
    }

    glUseProgram(m_shader);
    glBindVertexArray(m_vao);
    glDrawArrays(GL_TRIANGLE_STRIP, 0, 4);

    ImGui_ImplOpenGL3_RenderDrawData(ImGui::GetDrawData());
    SDL_GL_SwapWindow(window);
}

void MasterSystem::doImGui()
{
    ImGui_ImplOpenGL3_NewFrame();
    ImGui_ImplSDL2_NewFrame(window);
    ImGui::NewFrame();

    if (m_showUI)
    {
        if (ImGui::BeginMainMenuBar())
        {
            //file menu
            if (ImGui::BeginMenu("File"))
            {
                if (ImGui::MenuItem("Open ROM", "CTRL+O", nullptr))
                {
                    browseRom();
                }

                if (ImGui::MenuItem("Reset Machine", "CTRL+R", nullptr))
                {
                    if (!m_currentRom.empty())
                    {
                        startRom(m_currentRom);
                    }
                }

                if (ImGui::MenuItem("Quit", "ALT+F4", nullptr))
                {
                    m_running = false;
                }
                ImGui::EndMenu();
            }

            //view menu
            if (ImGui::BeginMenu("View"))
            {
                if (ImGui::MenuItem("Options", "ALT+O", &m_showOptions))
                {
                    if (!m_showOptions)
                    {
                        //TODO window was closed, so save options file
                    }
                }
                
                if (ImGui::MenuItem("Hide UI", "Esc, Ins", nullptr))
                {
                    m_showUI = false;
                }

                ImGui::EndMenu();
            }

            ImGui::EndMainMenuBar();
        }
    }

    if (m_showOptions)
    {
        ImGui::SetNextWindowSize({ 340.f, 210.f });
        ImGui::Begin("Options", &m_showOptions, ImGuiWindowFlags_NoResize | ImGuiWindowFlags_NoCollapse);
        ImGui::BeginTabBar("##0");

        if (ImGui::BeginTabItem("Video"))
        {
            ImGui::Text("Application average %.3f ms/frame (%.1f FPS)", 1000.0f / ImGui::GetIO().Framerate, ImGui::GetIO().Framerate);
            ImGui::NewLine();

            if (ImGui::InputInt("Screen Size", &m_windowScale))
            {
                m_windowScale = std::max(1, std::min(8, m_windowScale));

                auto w = m_width * m_windowScale;
                auto h = m_height * m_windowScale;
                SDL_SetWindowSize(window, w, h);
                glViewport(0, 0, w, h);
            }

            if (ImGui::Checkbox("Full Screen", &fullScreen))
            {
                if (fullScreen)
                {
                    SDL_SetWindowFullscreen(window, SDL_WINDOW_FULLSCREEN_DESKTOP);
                    //TODO letterbox this properly
                }
                else
                {
                    SDL_SetWindowFullscreen(window, 0);
                    glViewport(0, 0, m_width * m_windowScale, m_height * m_windowScale);
                }
            }

            bool vsync = SDL_GL_GetSwapInterval() == 1;
            if (ImGui::Checkbox("V-Sync", &vsync))
            {
                SDL_GL_SetSwapInterval(vsync);
            }
            
            ImGui::NewLine();
            if (ImGui::Button("Load Shader"))
            {
                static const char* filters[] = { "*.glsl", "*.txt", ".fs" };
                auto path = tinyfd_openFileDialog("Open File", nullptr, 3, filters, "Fragment Shader Source", 0);
                if (path)
                {
                    std::ifstream file(path);
                    if (file.is_open() && file.good())
                    {
                        std::string str((std::istreambuf_iterator<char>(file)), std::istreambuf_iterator<char>());
                        m_textEditor.SetText(str);

                        if (!loadShader(str))
                        {
                            tinyfd_messageBox("Error", shaderError.c_str(), "ok", "error", DialogResult::Yes);
                        }
                    }
                    file.close();

                    m_currentShaderPath = path;
                }
            }            
            ImGui::SameLine();
            if (ImGui::Button("Reset To Default"))
            {
                loadShader(Fragment);
                m_currentShaderPath.clear();
            }
            ImGui::SameLine();
            if (ImGui::Button("Shader Editor"))
            {
                m_showEditor = true;
            }

            ImGui::EndTabItem();
        }

        if (ImGui::BeginTabItem("Audio"))
        {
            static bool tone1 = true;
            if (ImGui::Checkbox("Tone 1", &tone1))
            {
                std::cout << "implement me!" << std::endl;
            }

            static bool tone2 = true;
            if (ImGui::Checkbox("Tone 2", &tone2))
            {
                std::cout << "implement me!" << std::endl;
            }

            static bool tone3 = true;
            if (ImGui::Checkbox("Tone 3", &tone3))
            {
                std::cout << "implement me!" << std::endl;
            }
            
            static float vol = 1.f;
            if (ImGui::SliderFloat("Volume", &vol, 0.f, 1.f))
            {
                std::cout << "implement me!" << std::endl;
            }

            ImGui::EndTabItem();
        }

        if (ImGui::BeginTabItem("Input"))
        {
            ImGui::Text("TODO: Key binds");
            ImGui::Text("TODO: Controller input");

            ImGui::EndTabItem();
        }

        ImGui::EndTabBar();
        
        ImGui::End();

        if (!m_showOptions)
        {
            //window was closed - save options
            saveSettings();
        }
    }

    if (m_showEditor)
    {
        shaderEditor();
    }
}

void MasterSystem::browseRom()
{
    static const char* filters[] = { "*.sms", "*.sg" };

    auto path = tinyfd_openFileDialog("Open ROM", nullptr, 1, filters, "Master System ROMs", 0);
    if (path)
    {
        //TODO assert file exists
        //TODO handle opening failures
        startRom(path);
        m_showUI = false;
    }
}

void MasterSystem::shaderEditor()
{
    const auto saveFile = [&]()
    {
        static const char* filters[] = { "*.glsl" };
        auto path = tinyfd_saveFileDialog("Save As...", nullptr, 1, filters, "GLSL file");
        if (path)
        {
            std::ofstream file(path);
            if (file.is_open() && file.good())
            {
                auto str = m_textEditor.GetText();
                file.write(str.c_str(), str.size());
            }
            file.close();

            m_currentShaderPath = path;
        }
    };

    const auto confirmationBox = [&saveFile]()->bool
    {
        auto ret = tinyfd_messageBox("Confirm", "Save Current File?", "yesnocancel", "question", DialogResult::Yes);
        if (ret != DialogResult::Cancel)
        {
            if (ret == DialogResult::Yes)
            {
                //save the file
                saveFile();
            }
            return true;
        }
        return false;
    };

    ImGui::SetNextWindowSize({ 600.f, 400.f });
    ImGui::Begin("Shader Editor", &m_showEditor, 
        ImGuiWindowFlags_NoCollapse | ImGuiWindowFlags_NoResize |
    ImGuiWindowFlags_MenuBar | ImGuiWindowFlags_HorizontalScrollbar);

    if (ImGui::BeginMenuBar())
    {
        if (ImGui::BeginMenu("File##000"))
        {
            if (ImGui::MenuItem("New..."))
            {
                if (confirmationBox())
                {
                    m_textEditor.SetText(Fragment);
                    m_currentShaderPath.clear();
                }
            }

            if (ImGui::MenuItem("Open##000"))
            {
                if (confirmationBox())
                {
                    static const char* filters[] = { "*.glsl", "*.txt", ".fs" };
                    auto path = tinyfd_openFileDialog("Open File", nullptr, 3, filters, "Fragment Shader Source", 0);
                    if (path)
                    {
                        std::ifstream file(path);
                        if (file.is_open() && file.good())
                        {
                            std::string str((std::istreambuf_iterator<char>(file)), std::istreambuf_iterator<char>());
                            m_textEditor.SetText(str);
                        }
                        file.close();

                        m_currentShaderPath = path;
                    }
                }
            }

            if (ImGui::MenuItem("Save##000"))
            {
                if (m_currentShaderPath.empty())
                {
                    saveFile();
                }
                else
                {
                    std::ofstream file(m_currentShaderPath);
                    if (file.is_open() && file.good())
                    {
                    auto str = m_textEditor.GetText();
                    file.write(str.c_str(), str.size());
                    }
                    file.close();
                }
            }

            if (ImGui::MenuItem("Save As...##000"))
            {
                saveFile();
            }
            ImGui::Separator();
            if (ImGui::MenuItem("Exit##000"))
            {
                if (confirmationBox())
                {
                    m_showEditor = false;
                }
            }
            ImGui::EndMenu();

            if (ImGui::BeginMenu("Edit"))
            {
                if (ImGui::MenuItem("Undo", "ALT-Backspace", nullptr, m_textEditor.CanUndo()))
                {
                    m_textEditor.Undo();
                }
                if (ImGui::MenuItem("Redo", "Ctrl-Y", nullptr, m_textEditor.CanRedo()))
                {
                    m_textEditor.Redo();
                }

                ImGui::Separator();

                if (ImGui::MenuItem("Copy", "Ctrl-C", nullptr, m_textEditor.HasSelection()))
                {
                    m_textEditor.Copy();
                }
                if (ImGui::MenuItem("Cut", "Ctrl-X", nullptr, m_textEditor.HasSelection()))
                {
                    m_textEditor.Cut();
                }
                if (ImGui::MenuItem("Delete", "Del", nullptr, m_textEditor.HasSelection()))
                {
                    m_textEditor.Delete();
                }
                if (ImGui::MenuItem("Paste", "Ctrl-V", nullptr, ImGui::GetClipboardText() != nullptr))
                {
                    m_textEditor.Paste();
                }

                ImGui::Separator();

                if (ImGui::MenuItem("Select all", nullptr, nullptr))
                {
                    m_textEditor.SetSelection(TextEditor::Coordinates(), TextEditor::Coordinates(m_textEditor.GetTotalLines(), 0));
                }
                ImGui::EndMenu();
            }
        }

        ImGui::EndMenuBar();
    }


    if (ImGui::Button("Build"))
    {
        loadShader(m_textEditor.GetText());
    }
    ImGui::SameLine();
    if (shaderError.empty())
    {
        ImGui::Text("No Errors.");
    }
    else
    {
        ImGui::PushStyleColor(0, { 1.f, 0.f, 0.f, 1.f });
        ImGui::Text("%s", shaderError.c_str());
        ImGui::PopStyleColor();
    }

    auto cursorPos = m_textEditor.GetCursorPosition();
    ImGui::Text("%6d/%-6d %6d lines  | %s | %s | %s", cursorPos.mLine + 1, cursorPos.mColumn + 1, m_textEditor.GetTotalLines(),
        m_textEditor.IsOverwrite() ? "Ovr" : "Ins",
        m_textEditor.CanUndo() ? "*" : " ",
        m_textEditor.GetLanguageDefinition().mName.c_str());

    m_textEditor.Render("TextEditor"); //we have to do this last as apparently there is no way to set its size

    ImGui::End();
}

void MasterSystem::loadSettings()
{
    ConfigFile cfg;
    if (cfg.loadFromFile("settings.cfg"))
    {
        const auto properties = cfg.getProperties();
        for( const auto& prop : properties)
        {
            const auto& name = prop.getName();
            if (name == "window_scale")
            {
                m_windowScale = std::max(1, std::min(8, prop.getValue<std::int32_t>()));
                //this ought to be loaded before window creation so no need to apply
            }
            else if (name == "full_screen")
            {
                fullScreen = prop.getValue<bool>();
            }
            else if (name == "active_shader")
            {
                m_currentShaderPath = prop.getValue<std::string>();
            }

            //TODO audio settings
            //TODO keybinds
        }
    }
}

void MasterSystem::saveSettings()
{
    ConfigFile cfg("settings");
    cfg.addProperty("window_scale").setValue(m_windowScale);
    cfg.addProperty("full_screen").setValue(fullScreen);
    if (!m_currentShaderPath.empty())
    {
        cfg.addProperty("active_shader").setValue(m_currentShaderPath);
    }

    //TODO audio settings
    //TODO keybinds

    cfg.save("settings.cfg");
}

void MasterSystem::shutdown()
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

    ImGui_ImplOpenGL3_Shutdown();
    ImGui_ImplSDL2_Shutdown();
    ImGui::DestroyContext();

    SDL_GL_DeleteContext(ctx);
    SDL_Quit();
}