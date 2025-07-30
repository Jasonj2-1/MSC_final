This project aims to complete an DE1-SoC embedded music player that is interactive with SD card, WM8731 codec, DDR3 SDRAM, Switches, keys, and an LCD.

The function is controlled by interfaces:

## Controls
```
KEY[0]  -  Stop/Resume
KEY[1]  -  Replay
KEY[2]  -  Increase volume
KEY[3]  -  Decrease volume

SW[0]  -  Music Ready Flag
SW[1]  -  Music Swicthing
```
Initially, SW1 should be switched off before music 2 is loaded.
LCD displays the music wavform, volume bar, loading progress and Text.

# Overview

<img width="512" height="316" alt="image" src="https://github.com/user-attachments/assets/fcfa0de6-babe-4766-bcba-1d689a1c27c4" />

# Hardware block diagram

<img width="514" height="288" alt="image" src="https://github.com/user-attachments/assets/a0d47f36-3cc5-4121-9280-b6e78dec726f" />

# LCD Layout
<img width="269" height="290" alt="image" src="https://github.com/user-attachments/assets/13b52e9c-76cd-4a6a-aa3e-68786f5997f4" />
<img width="243" height="318" alt="image" src="https://github.com/user-attachments/assets/bfd60582-2bdd-4ae5-8a09-14ae97fe6a55" />

The detailed information is illustrated in the report.

## Software 

The code writing is referenced with LEEDS API, such as HPS_PrivateTimer.c.

```
main.c - written independently, including music output, Key/Switch control, codec volume control, and LCD control. 
LCD_display.c - waveform, text, volume/progressing bar control.

```

The timer trigger and byte read/sent logic are compared in this project.\n
Different sampling rates are compared.\n
The results are analysed with reasons for the data flow/read speed.

