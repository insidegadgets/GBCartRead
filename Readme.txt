GBCartRead v1.8 Rev 1 - Gameboy Cart Reader Readme
By insideGadgets - www.insidegadgets.com / support@insidegadgets.com

GBCartRead is an Arduino based Gameboy Cartridge Reader which uses a C program or python script to interface with 
the Arduino. GBCartRead allows you to dump your ROM, save the RAM and write to the RAM.

Works with Arduino Duemilanove and Uno. Will work for 5V Arduinos but requires wiring changes.
Speed increase thanks to Frode vdM. (fvdm1992@yahoo.no) and David R


REQUIREMENTS
=============
Option 1 - Ready to use
- Arduino
- Gameboy Cart Shield - http://www.insidegadgets.com/projects/gameboy-cart-shield/


Option 2 - DIY (recommend GB Cart Read v1.2 as it contains less parts)
- Breadboard or similar
- Arduino
- 74HC595 Shift Registers (2)
- 470 Ohm Resistors – optional, the resistors would be for safety purposes only to prevent accidental shorts.
- Gameboy Cartridge Adapter (or similar) - http://dx.com/p/repair-parts-replacement-gba-game-cart-slot-for-nds-lite-37787


HOW TO USE
=================================
1. Open the \GBCartRead_v(xxx)_Arduino\GBCartRead_v(xxx).ino file and program it to your Arduino. 
   Note down the COM port number in use; you can find this out from the Arduino software by going to Tools -> Serial port.
2. There are two choices to communicate with the Arduino – using the C program or the Python program.

  ---Python Program---
  a. Download and install Python 3.2 (http://www.python.org/download/) and pySerial (http://pypi.python.org/pypi/pyserial)
  b. Open up the “GBCartRead_v(xxx)_Python_Interface.py” script by right clicking it and selecting “Edit with IDLE”.
  c. Change the “COM2” serial port to the serial port that your Arduino is connected on, save the file and press F5 
     to run it.
  d. A new window will appear, after 2-3 seconds you’ll have some options available. 

  ---C Program---
  a. Open config.ini and change the COM port number to the serial port that your Arduino is connected on and save 
     the file.
  b. Run “GBCartRead_v(xxx)_C_Interface.exe” in Windows or re-compile the source for your OS 

3. At this stage you should insert your Gameboy cartridge and press the power button, the power LED should light up.
4. Press 0 to read the header and verify that it looks correct. If it doesn’t look correct, press the power button 
   to power off the Gameboy cartridge, remove and re-insert it and power it up again.
5. Press 1 to Dump the ROM, 2 to Backup your RAM or 3 to Load your RAM file. Hashes (#) will start printing every 
   few seconds and a file called <gametitle>.gb or .sav will be created if you chose option 1 or 2. If you choose 
   option 3, it will load the save from <gametitle>.sav.

We recommended verifying your Gameboy ROM using BGB (a Gameboy emulator) or "xgbfix.exe -v -d <your_rom.rom>" 
found in the project called "ASMotor" (we've included it). It’s a good idea to verify your save files too by 
running the ROM when the save file is present in BGB.


CuSTOM CARTRIDGE FILES
=================================
There are some cartridges which don't quite conform to cartridge header standards or require something out of the ordinary.
Below is the listing of custom cartridge folders, if you wish to access these cartridges, you will need to use GBCartRead 
files found on those folders.

Wisdom Tree - /Custom_Cartridge_Files/Wisdom_Tree/ - Provided by Benjamin E


REVISION HISTORY
=================================
v1.8 Rev 1 (28 May 2016)
- Re-add checking for non-printable ASCII characters code that was removed when cleaning up code
- Added custom cartridge files for Wisdom Tree games (thanks Benjamin E)

v1.8 (21 March 2016)
- Added Nintendo Logo check and compare so you can tell if the cartridge is being read correctly (sometimes the title
  was read ok but the ROM dump wasn't correct)
- Added missing cartridge RAM support
- Cleaned up code

v1.7 Rev 2 (10 February 2016)
- Added checking for non-printable ASCII characters in Gameboy header and replaced them with underscores

v1.7 Rev 1 (17 February 2015)
- Fixed clockPin to change correct port - PB5 (thanks Anna)

v1.7 (7 February 2015) *Only compatible with Gameboy Cart Shield v1.3*
- Added interrupt for button on A1 to switch P mosfet on A0 that powers the cartridge
- Changed so at startup it doesn't display the Game Cartridge header

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
(c) 2011-2016 by insideGadgets
http://www.insidegadgets.com

This work is licensed under a Creative Commons Attribution-NonCommercial 3.0 Unported License.
http://creativecommons.org/licenses/by-nc/3.0/