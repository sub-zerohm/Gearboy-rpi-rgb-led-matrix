Gearboy
=======
<b>Original Copyright: Gearboy &copy; 2012 by Ignacio Sanchez</b>

----------
Gearboy is a Nintendo Game Boy / GameBoy Color emulator written in C++ that runs on iOS, Raspberry Pi, Mac, Windows, Linux and RetroArch.
This forks addds an additional display mode the emulator, that uses the [rpi-rgb-led-matrix](https://github.com/hzeller/rpi-rgb-led-matrix)-library to display the image directly onto an LED Matrix Panel.

See Original Git for Instructions for other platforms / use cases: https://github.com/drhelius/Gearboy 

----------

Features
--------
- Highly accurate CPU emulation, passes cpu_instrs.gb from blargg's tests.
- Accurate instruction and memory timing, passes instr_timing.gb and mem_timing.gb from blargg's tests.
- Memory Bank Controllers (MBC1, MBC2, MBC3 with RTC, MBC5), ROM + RAM and multicart cartridges.
- Accurate LCD controller emulation. Background, window and sprites, with correct timings and priorities including mid-scanline timing.
- Mix frames: Mimics the LCD ghosting effect seen in the original Game Boy.
- Sound emulation using SDL Audio and [Gb_Snd_Emu library](http://blargg.8bitalley.com/libs/audio.html#Gb_Snd_Emu).
- Game Boy Color support.
- Integrated disassembler. It can dump the full disassembled memory to a text file or access it in real time.
- Saves battery powered RAM cartridges to file.
- Save states.
- Compressed rom support (ZIP deflate).
- Game Genie and GameShark cheat support.
- Multi platform. Runs on Windows, Linux, Mac OS X, Raspberry Pi, iOS and as a libretro core (RetroArch).

Build Instructions
----------------------

### Raspberry Pi 2 & 3 - Raspbian
- Install and configure [SDL 2](http://www.libsdl.org/download-2.0.php) for development:
``` shell
sudo apt-get update
sudo apt-get upgrade
sudo apt-get install build-essential libfreeimage-dev libopenal-dev libpango1.0-dev libsndfile-dev libudev-dev libasound2-dev libjpeg-dev libtiff5-dev libwebp-dev automake
cd ~
wget https://www.libsdl.org/release/SDL2-2.0.9.tar.gz
tar zxvf SDL2-2.0.9.tar.gz
cd SDL2-2.0.9 && mkdir build && cd build
../configure --disable-pulseaudio --disable-esd --disable-video-mir --disable-video-wayland --disable-video-x11 --disable-video-opengl --host=armv7l-raspberry-linux-gnueabihf
make -j 4
sudo make install
```
- Install libconfig library dependencies for development: <code>sudo apt-get install libconfig++-dev</code>
- Use <code>make -j 4</code> in the <code>platforms/raspberrypi3/x64/</code> folder to build the project.
- Use <code>export SDL_AUDIODRIVER=ALSA</code> before running the emulator for the best performance.
- Gearboy generates a <code>gearboy.cfg</code> configuration file where you can customize keyboard and gamepads. Key codes are from [SDL](https://wiki.libsdl.org/SDL_Keycode).



License
-------

<i>Gearboy - Nintendo Game Boy Emulator</i>

<i>Copyright (C) 2012  Ignacio Sanchez</i>

<i>This program is free software: you can redistribute it and/or modify</i>
<i>it under the terms of the GNU General Public License as published by</i>
<i>the Free Software Foundation, either version 3 of the License, or</i>
<i>any later version.</i>

<i>This program is distributed in the hope that it will be useful,</i>
<i>but WITHOUT ANY WARRANTY; without even the implied warranty of</i>
<i>MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE. See the</i>
<i>GNU General Public License for more details.</i>

<i>You should have received a copy of the GNU General Public License</i>
<i>along with this program.  If not, see http://www.gnu.org/licenses/</i>
