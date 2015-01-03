/*
 GBCartRead
 Version: 1.5
 Author: Alex from insideGadgets (http://www.insidegadgets.com)
 Created: 18/03/2011
 Last Modified: 15/11/2014
 
 Read ROM, Read RAM or Write RAM from/to a Gameboy Cartridge.
 Works for Arduino Duemilanove and Uno. Will work for 5V Arduinos but requires pin changes.
 
 Speed increase thanks to Frode vdM. (fvdm1992@yahoo.no)
*/

// Edit these in pindelcarations.h too
int latchPin = 10;
int dataPin = 11;
int clockPin = 12;
int rdPin = A5;
int wrPin = A3;
int mreqPin = A4;

#include "pindeclarations.h"

void setup() {
  Serial.begin(57600);
  pinMode(latchPin, OUTPUT);
  pinMode(clockPin, OUTPUT);
  pinMode(dataPin, OUTPUT);
  pinMode(rdPin, OUTPUT);
  pinMode(wrPin, OUTPUT);
  pinMode(mreqPin, OUTPUT);
  
  // Set pins as inputs
  DDRB &= ~((1<<PB0) | (1<<PB1)); // D8 & D9
  DDRD &= ~((1<<PD2) | (1<<PD3) | (1<<PD4) | (1<<PD5) | (1<<PD6) | (1<<PD7)); // D2 to D7
}

void loop() {
  // Wait for serial input
  while (Serial.available() <= 0) {
    delay(200);
  }

  // Decode input
  char readInput[10];
  int readCount = 0;
  while (Serial.available() > 0) {
    char c = Serial.read();
    readInput[readCount] = c;
    readCount++;
  }
  readInput[readCount] = '\0';
  
  // Turn everything off
  rdPin_high; // RD off
  wrPin_high; // WR off
  mreqPin_high; // MREQ off

  // Read Cartridge Header
  char gameTitle[17];
  for (int addr = 0x0134; addr <= 0x143; addr++) {
    gameTitle[(addr-0x0134)] = (char) readByte(addr);
  }
  gameTitle[16] = '\0';
  int cartridgeType = readByte(0x0147);
  int romSize = readByte(0x0148);
  int ramSize = readByte(0x0149);
  int romBanks = 2; // Default 32K
  if (romSize == 1) { romBanks = 4; } 
  if (romSize == 2) { romBanks = 8; } 
  if (romSize == 3) { romBanks = 16; } 
  if (romSize == 4) { romBanks = 32; } 
  if (romSize == 5 && (cartridgeType == 1 || cartridgeType == 2 || cartridgeType == 3)) { romBanks = 63; } 
  else if (romSize == 5) { romBanks = 64; } 
  if (romSize == 6 && (cartridgeType == 1 || cartridgeType == 2 || cartridgeType == 3)) { romBanks = 125; } 
  else if (romSize == 6) { romBanks = 128; }
  if (romSize == 7) { romBanks = 256; }
  if (romSize == 82) { romBanks = 72; }
  if (romSize == 83) { romBanks = 80; }
  if (romSize == 84) { romBanks = 96; }
  int ramBanks = 1; // Default 8K RAM
  if (ramSize == 3) { ramBanks = 4; }
  if (ramSize == 4){ ramBanks = 16; } // GB Camera
   
  // Cartridge Header
  else if (strstr(readInput, "HEADER")) {
    Serial.println(gameTitle);
    Serial.println(cartridgeType);
    Serial.println(romSize);
    Serial.println(ramSize);
  }

  // Dump ROM
  else if (strstr(readInput, "READROM")) {
    unsigned int addr = 0;
    
    // Read x number of banks
    for (int y = 1; y < romBanks; y++) {
      writeByte(0x2100, y); // Set ROM bank
      if (y > 1) {addr = 0x4000;}
      for (; addr <= 0x7FFF; addr = addr+64) {
        uint8_t readData[64];
        for(int i = 0; i < 64; i++){
          readData[i] = readByte(addr+i);
        }
        Serial.write(readData, 64);
      }
    }
  }
  
  // Read RAM
  else if (strstr(readInput, "READRAM")) {
    // MBC2 Fix (unknown why this fixes it, maybe has to read ROM before RAM?)
    readByte(0x0134);

    unsigned int addr = 0;
    unsigned int endaddr = 0;
    if (cartridgeType == 6 && ramSize == 0) { endaddr = 0xA1FF; } // MBC2 512bytes (nibbles)
    if (ramSize == 1) { endaddr = 0xA7FF; } // 2K RAM
    if (ramSize > 1) { endaddr = 0xBFFF; } // 8K RAM
    
    // Does cartridge have RAM
    if (endaddr > 0) {
      // Initialise MBC
      writeByte(0x0000, 0x0A);

      // Switch RAM banks
      for (int bank = 0; bank < ramBanks; bank++) {
        writeByte(0x4000, bank);

        // Read RAM
        for (addr = 0xA000; addr <= endaddr; addr = addr+64) {  
          uint8_t readData[64];
          for(int i = 0; i < 64; i++){
            readData[i] = readByte(addr+i);
          }
          Serial.write(readData, 64);
        }
      }
      
      // Disable RAM
      writeByte(0x0000, 0x00);
    }
  }
  
  // Write RAM
  else if (strstr(readInput, "WRITERAM")) {
    Serial.println("START");
    
    // MBC2 Fix (unknown why this fixes it, maybe has to read ROM before RAM?)
    readByte(0x0134);
    unsigned int addr = 0;
    unsigned int endaddr = 0;
    if (cartridgeType == 6 && ramSize == 0) { endaddr = 0xA1FF; } // MBC2 512bytes (nibbles)
    if (ramSize == 1) { endaddr = 0xA7FF; } // 2K RAM
    if (ramSize > 1) { endaddr = 0xBFFF; } // 8K RAM
    
    // Does cartridge have RAM
    if (endaddr > 0) {
      // Initialise MBC
      writeByte(0x0000, 0x0A);
      
      // Switch RAM banks
      for (int bank = 0; bank < ramBanks; bank++) {
        writeByte(0x4000, bank);
        
        // Write RAM
        for (addr = 0xA000; addr <= endaddr; addr=addr+64) {  
          Serial.println("NEXT"); // Tell Python script to send next 64bytes
          
          // Wait for serial input
          for (int i = 0; i < 64; i++) {
            while (Serial.available() <= 0) {
              delay(1);
            }
            
            // Read input
            uint8_t bval = 0;
            if (Serial.available() > 0) {
              char c = Serial.read();
              bval = (int) c;
            }
            
            // Write to RAM
            mreqPin_low;
       	    writeByte(addr+i, bval);
            asm volatile("nop");
            asm volatile("nop");
            asm volatile("nop");
            mreqPin_high;
          }
        }
      }
      
      // Disable RAM
      writeByte(0x0000, 0x00);
      Serial.flush(); // Flush any serial data that wasn't processed
    }
    Serial.println("END");
  }
}

uint8_t readByte(int address) {
  shiftoutAddress(address); // Shift out address

  mreqPin_low;
  rdPin_low;
  asm volatile("nop"); // Delay a little (minimum is 2 nops, using 3 to be sure)
  asm volatile("nop");
  asm volatile("nop");
  uint8_t bval = ((PINB << 6) | (PIND >> 2)); // Read data
  rdPin_high;
  mreqPin_high;
  
  return bval;
}

void writeByte(int address, uint8_t data) {
  // Set pins as outputs
  DDRB |= ((1<<PB0) | (1<<PB1)); // D8 & D9
  DDRD |= ((1<<PD2) | (1<<PD3) | (1<<PD4) | (1<<PD5) | (1<<PD6) | (1<<PD7)); // D2 to D7
  
  shiftoutAddress(address); // Shift out address
  
  // Clear outputs and set them to the data variable
  PORTB &= ~((1<<PB0) | (1<<PB1)); // D8 & D9
  PORTD &= ~((1<<PD2) | (1<<PD3) | (1<<PD4) | (1<<PD5) | (1<<PD6) | (1<<PD7)); // D2 to D7
  PORTD |= (data << 2);
  PORTB |= (data >> 6);
  
  // Pulse WR
  wrPin_low;
  asm volatile("nop");
  wrPin_high;
  
  // Set pins as inputs
  DDRB &= ~((1<<PB0) | (1<<PB1)); // D8 & D9
  DDRD &= ~((1<<PD2) | (1<<PD3) | (1<<PD4) | (1<<PD5) | (1<<PD6) | (1<<PD7)); // D2 to D7
}

// Use the shift registers to shift out the address
void shiftoutAddress(unsigned int shiftAddress) {
  for (int8_t i = 15; i >= 0; i--) {
    if (shiftAddress & (1<<i)) {
      dataPin_high;
    } 
    else {
      dataPin_low;
    }
    clockPin_high;
    clockPin_low;
  }

  latchPin_low;
  asm volatile("nop");
  latchPin_high;
}

