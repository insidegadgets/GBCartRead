GBCartRead v1.6 - Gameboy Cart Reader Readme
By insideGadgets - http://www.insidegadgets.com / support@insidegadgets.com

GBCartRead is an Arduino based Gameboy Cartridge Reader which uses a C program or python script to interface with 
the Arduino. GBCartRead allows you to dump your ROM, save the RAM and write to the RAM.

Works with Arduino Duemilanove and Uno. Will work for 5V Arduinos but requires wiring changes.
Speed increase thanks to Frode vdM. (fvdm1992@yahoo.no) and David R


REQUIREMENTS
=============
- Arduino
- 74HC595D SMD SO16 (2)
- Gameboy Cart Shield - http://www.insidegadgets.com/projects/gameboy-cart-shield/
- GBA Game Cart Slot for NDS Lite SMD (1) - http://dx.com/p/repair-parts-replacement-gba-game-cart-slot-for-nds-lite-37787
- Arduino 8 Pin .100" Stackable Header or Female headers (2)
- Arduino 6 Pin .100" Stackable Header or Female headers (2)

OR

- Breadboard or similar
- Arduino
- 74HC595 Shift Registers (2)
- 470 Ohm Resistors – optional, the resistors would be for safety purposes only to prevent accidental shorts.
- Gameboy Cartridge Adapter e.g: http://www.insidegadgets.com/projects/gameboy-cart-adapter/


HOW TO USE
=================================
Note: It is important to insert your Gameboy Cartridge before you plug in your Arduino as you may experience
RAM data loss if you insert the cartridge when the Arduino is on.

--- OPTION 1 - PYTHON PROGRAM ---
1. Upload "GBCartRead_v1_6.ino" to the Arduino.
2. Download and install Python 3.2 (http://www.python.org/download/) and pySerial (http://pypi.python.org/pypi/pyserial)
3. Open up the "GBCartRead_v1_6_Python_Reader.py" script by right clicking it and selecting “Edit with IDLE”.
4. Change the 'COM2' serial port to the serial port that your Arduino is connected on. You can find this out 
   from the Arduino software by going to Tools -> Serial port when the Arduino is plugged in. You may also have to 
   lower the baud rate from 400000 if you find CRC checksum errors when reading your ROM and running it on an emulator.
5. Press F5 to run the script.
6. A new window will appear, press 0 to Read the header, 1 to Dump the ROM, 2 to Backup your RAM or 
   3 to Load your RAM file. Hashes (#) will start printing every few seconds and a file called <gametitle>.gb or .sav 
   will be created if you chose option 1 or 2. If you choose option 3, it will load the save from <gametitle>.sav.

--- OPTION 2 - C PROGRAM ---
1. Upload "GBCartRead_v1_6.ino" to the Arduino.
2. Open config.ini and change the COM serial port number to the serial port that your Arduino is connected on.
   You may also have to lower the baud rate from 400000 if you find CRC checksum errors when reading your ROM and 
   running it on an emulator. You can find this out from the Arduino software by going to Tools -> Serial port when the Arduino is plugged in.
3. Run "GBCartRead_v1_6_C_Reader" in Windows or re-compile the source for your OS
4. A new window will appear, press 0 to Read the header, 1 to Dump the ROM, 2 to Backup your RAM or 
   3 to Load your RAM file. Hashes (#) will start printing every few seconds and a file called <gametitle>.gb or .sav 
   will be created if you chose option 1 or 2. If you choose option 3, it will load the save from <gametitle>.sav.

We recommended verifying your Gameboy ROM using "xgbfix.exe -v -d <your_rom.rom>" found in the project 
called "ASMotor" (we've included it). We also recommend verifying your save files too.


REVISION HISTORY
=================================
v1.6 Rev 1 (17 February 2015)
- Fixed clockPin to change correct port - PB5 (thanks Anna)

v1.6 (3 January 2015)
- Speed increase by using a higher baud rate (400Kbit from 57.6Kbit) and using hardware SPI (thanks David R)
- Changed SCK from pin 12 to pin 13 for SPI
- RAM writing speed increase by continually sending 64bytes data from PC and pausing for 5 ms in-between
(Compatible with Gameboy Cart Shield v1.2 if you connect pins 12 and 13 together)

v1.5 (15 November 2014)
- Speed increases by using direct PORT writing/reading and sending 64bytes at a time (thanks Frode vdM. fvdm1992@yahoo.no)
- Compacting functions (thanks Frode vdM. fvdm1992@yahoo.no)
- Delays when pulsing WR/RD/MREQ pins reduced to a few NOP cycles

v1.4.2 (14 November 2014)
- Fixed gameTitle array length issue (thanks Frode vdM and David R)
- Added Gameboy Camera support (thanks Frode vdM)

v1.4.1 (3 January 2014)
- Changed wrPin from pin 13 to A3 to suit Gameboy Cart Shield v1.2

v1.4 (30 August 2013)
- Added gbcartread.exe program for Windows (with source) which can be used instead of using the Python script
  and is a bit faster

v1.3.1 (2 May 2011)
- Updated schematic as R19, R20 and R31 were actually acting as voltage dividers

v1.3 (9 April 2011)
- Added RAM writing ability
- Fixed MBC2 reading RAM overflow relating to the first 4 bits not being ignored properly
- Cleaned up bank select code to use BitRead instead of checking each bit ourselves
- Fixed selecting wrong RAM size when reading
- Fixed incorrect check if RAM was present before reading
- Removed unnecessary changing of data pins to inputs after MBC was initialised when reading RAM

v1.2 (2 April 2011)
- Added RAM saving ability.
- Now prints the amount of data read in 64K increments (i.e ###64K###128K###192K)
- Changed <gametitle>.rom to <gametitle>.gb

v1.1 (21 March 2011)
- ROM reading is now fully automated.
- The Game Title, MBC type, ROM size and RAM size are presented in the Python script.

v1.0 (19 March 2011)
- Reading ROM only, user to change the maximum ROM bank to read


----------------------------------------------------------------------------
(c) 2011-2015 by insideGadgets
http://www.insidegadgets.com

This work is licensed under a Creative Commons Attribution-NonCommercial 3.0 Unported License.
http://creativecommons.org/licenses/by-nc/3.0/