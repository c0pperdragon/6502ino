# 6502ino

A minimal circuit to form the environment necessary to run a 65c02 CPU.

Normally getting any form of 6502 processor to run and communicate with the outside
world is very cumbersome. In addition to the CPU and RAM you need very specific 
IO controller parts that can be hooked up to the system address and data bus.
You need a ROM for the initial boot code and some form of MMU to drive the various
selection lines according to the memory address (all IO is memory mapped).
All this leads to a pretty large and costly board even for the most basic 
program to execute.

This project tries to reduce the system size as much as possible by
using a microcontroller - in the form of an Arduino Uno - to do all the
tricky stuff. Together with only a bit of 7400 series logic and 32KB RAM,
this is  enough to get the 65c02 working with 16Mhz on a breadboard.


### Setting the fuses to generate 16Mhz on pin 8

For the setup to work, you need to configure your arduino to permanently
ouput the full system clock on a dedicated pin. This feature can not be set up
from inside a program, but needs to be done by configuring the "fuse bits". 

Setting the fuse bits is a bit complicated, but you only need to do it once. 
To access these bits, you need a dedicated programming device, called the 
AVR ISP (in-circuit-programmer). Luckily any arduino can be programmed to become 
such a device with the "ArduinoISP" example sketch that is included in the 
Arduino IDE. So you basically need a spare arduino for this process.

Wiring up the programmer arduino to the arduino being programmed can be seen in
countless internet resources. But there it is mainly done to programm a bootloader.
You need to do a different thing, for which you need the command-line tool "avrdude.exe"
which is installed along with the Arduino IDE somewhere in the depth of the "hardware/tools"
folder. 

Once everything is hooked up, open a system console and execute the following command (replace com10 
with whatever port your programmer arduino is registered in the system):
```
avrdude -p atmega328p -b 19200 -C ../etc/avrdude.conf -c avrisp -P com10
```

If everything works correctly, this will show you some status and the current settings of the
fuse bits. We are particularly interested in the value of the 'L' fuse bits, which on my
factory default Arduino Uno shows as L:FF. To set the permanent clock output, you need
to modify this value (an 8-bit value) to have the bit 6 set to 0. In my case this means

to set the 'L' fuses to BF:
```
avrdude -p atmega328p -b 19200 -C ../etc/avrdude.conf -c avrisp -P com10 -U lfuse:w:0xBF:m
```

After this settings are done, the Arduino will constantly send a 16Mhz signal on pin 8, 
no matter what, even during reset.

