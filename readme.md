SMS-Slinger
-----------

http://www.codeslinger.co.uk/pages/projects/mastersystem.html

Copyright (c) 2008 copyright holders (sic)

This software is provided 'as-is', without any express or implied
warranty. In no event will the authors be held liable for any damages
arising from the use of this software.

Permission is granted to anyone to use this software for any purpose,
including commercial applications, and to alter it and redistribute it
freely, subject to the following restrictions:

1. The origin of this software must not be misrepresented; you must not
   claim that you wrote the original software. If you use this software
   in a product, an acknowledgment in the product documentation would be
   appreciated but is not required.
2. Altered source versions must be plainly marked as such, and must not be
   misrepresented as being the original software.
3. This notice may not be removed or altered from any source distribution.


###### Changes

This repository contains the original source updated to SDL 2.0 from
SDL 1.2, as well as some cross platform enhancements (removed MFC/ATL
dependency on Windows, use dearImGui on all platforms, built in shader
editor for post-process effects). The sound emulation has also been
updated to fix a bug in the noise channel, as well as tighten the cycle
timing relative to the CPU emulation, which vastly improves (imo) the
quality of the audio sync with the video.


Ported to SDL2 2021 Matt Marchant