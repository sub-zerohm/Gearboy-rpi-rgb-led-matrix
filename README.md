Gearboy - for led matrix displays
=======

Gearboy is a Nintendo Game Boy / GameBoy Color emulator written in C++ that runs on iOS, Raspberry Pi, Mac, Windows, Linux and RetroArch.
This forks addds an additional display mode the emulator, that uses the [rpi-rgb-led-matrix](https://github.com/hzeller/rpi-rgb-led-matrix)-library to display the image directly on LED Matrix Panels.

See Original Git for Instructions for other platforms / use cases: https://github.com/drhelius/Gearboy 

<p align="center">
  <img src="https://www.korgel.net/wp-content/uploads/2020/01/IMG_20200102_181205.jpg" width="350" title="Screenshot of the RGB Panel running Tetris]"><br />
  <strong>Video: https://www.youtube.com/watch?v=phSSKYKx0Ys</strong>
</p>



----------

Features
--------
- Supports most rgb matrix panels, which are supported by  [rpi-rgb-led-matrix](https://github.com/hzeller/rpi-rgb-led-matrix)
- Scale to match LED Matrix resolution by doublicating pixels or merging pixels if resolution is smaller than original gameboy resolution  (e.g. 128 * 128)

Build Instructions
----------------------

### Raspberry 4
- Install and configure [SDL 2](http://www.libsdl.org/download-2.0.php) for development:
``` shell
sudo apt-get update
sudo apt-get upgrade
sudo apt-get install build-essential libfreeimage-dev libopenal-dev libpango1.0-dev libsndfile-dev libudev-dev libasound2-dev libjpeg-dev libtiff5-dev libwebp-dev automake
cd ~
wget https://www.libsdl.org/release/SDL2-2.0.10.tar.gz
tar zxvf SDL2-2.0.10.tar.gz
cd SDL2-2.0.10 && mkdir build && cd build
../configure --enable-video-kmsdrm --disable-video-rpi
make -j 4
sudo make install
```

Clone this repo with submodules:
```
git clone --recursive https://github.com/Dak0r/Gearboy-rpi-rgb-led-matrix.git  
```

- Install libconfig and boost filesystem library dependencies for development: <code>sudo apt-get install libconfig++-dev</code> and <code>sudo apt-get install libboost-filesystem-dev</code>
- Use <code>make -j 4</code> in the <code>platforms/raspberrypi4-rpi-rgb-led-matrix</code> folder to build the project.
- Use <code>export SDL_AUDIODRIVER=ALSA</code> before running the emulator for the best performance.
- Gearboy generates a <code>gearboy.cfg</code> configuration file where you can customize keyboard and gamepads. Key codes are from [SDL](https://wiki.libsdl.org/SDL_Keycode).


- Exit Emulator with Gamepad: Start + Select
- Button 5 (Shoulder Button R): Switch between Normal, Demo and Attraction Mode.
Demo: If no button is pressed for 10 Seconds, Sromb will randomly pick a game and start it with the audio muted. The Game will be exited automatically after 75s. And a new random game will start after 10 seconds. It's possible to cancel the automatic termination of a game, by pressing a button on the gamepad. Audio will then be automatically enabled.
If at any time after the canceltion no Button is pressed for 120 seconds, the termination of the game will be resceduled in 75s and audio will be muted.
Attraction Mode: Same as Demo mode, but the game won't be muted.

##### Simple Rom Browser (Sromb)

This project also comes with a a simple browser, that allows browsing and starting roms from a given directory on the matrix panel. I have this programm in my startup routine, so I can just turn on the pi and then use a gamepad to start any game.
When emulator exists the rom, the rom browser is automatically restarted.


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
