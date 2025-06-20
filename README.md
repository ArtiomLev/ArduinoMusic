# Play music by `tone` on arduino and same

## Commands

* `help` - help message
* `convert` - midi to project format convert guide
* `ls [path]` - list files
* `cd <path>` - change directory
* `play <filename>` - play music

## Convert guide

1) **Open** converter site: https://sshdl-7.extramaster.net/tools/midiToArduino/
2) **Select** your MIDI file
3) Click **Submit**
4) Select channel
5) Select Raw output
6) **Copy output**
7) **Make text file** with output
8) **Put text file** on SD card
9) **Insert SD** card
10) **Restart** Arduino
11) **Connect** to serial by USB, **115200 baud**
12) Enjoy!

## Tips

For better experience use MIDI with 1 channel.
If MIDI has many channels (4, 5, ...) you can select one of it but this will sound bad in most cases.

You can connect a speaker instead of a buzzer for better sound quality.

## Project scheme

![](/pic/scheme.png "Connection scheme")

## Components

* Arduino or same
* SD card module
* SD or microSD card
* Buzzer or speaker

## Firmware

### Load

You can upload precompiled firmware file by some utility. (For example CLI avrdude, but this is for experts.)

Precompiled .hex files stored in `precompiled` folder in subfolders.

### Compile

#### PlatformIO
1) Open project directory as PlatformIO project
2) Select env
3) Compile / upload

#### Arduino IDE
1) Move `src/main.cpp` to some empty folder
2) Rename `main.cpp` to `[folder_name].ino` or just rename file, ArduinoIDE will make folder
3) Open .ino file in ArduinoIDE
4) Select board
5) Upload

