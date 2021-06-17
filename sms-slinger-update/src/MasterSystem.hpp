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

#pragma once

#include <SDL_events.h>

#include <memory>

class Emulator;
class MasterSystem final
{
public:
    MasterSystem();
    ~MasterSystem();

    MasterSystem(MasterSystem&&) = delete;
    MasterSystem(const MasterSystem&) = delete;

    MasterSystem& operator = (MasterSystem&&) = delete;
    MasterSystem& operator = (const MasterSystem&) = delete;

    bool createSDLWindow();
    void startRom(const char* path);
    //value < 1 for no sync, else attempts to sync to given frame rate
    void run(int fps = -1, bool useGfxOpt = false);

private:

    Emulator* m_emulator;
    int m_width;
    int m_height;
    bool m_useGFXOpt;

    std::uint32_t m_shader;
    std::uint32_t m_texture;
    std::uint32_t m_vao;
    std::uint32_t m_vbo;


    bool initGL();
    bool loadShader();
    bool loadTexture();
    bool loadQuad();
    void initAudio();

    void romLoopFixedStep(int fps);
    void romLoopFree();

    bool handleEvent(const SDL_Event& event);
    void render();
};