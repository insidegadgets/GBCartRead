/*
 GBCartRead
 Version: 1.8
 Author: Alex from insideGadgets (http://www.insidegadgets.com)
 Created: 18/03/2011
 Last Modified: 21/03/2016
 
 GBCartRead is an Arduino based Gameboy Cartridge Reader which uses a C program or python script to interface with 
 the Arduino. GBCartRead allows you to dump your ROM, save the RAM and write to the RAM.

 Works with Arduino Duemilanove and Uno. Will work for other 5V Arduinos but requires wiring changes.

 Speed increase thanks to Frode vdM. (fvdm1992@yahoo.no) and David R
*/

// Edit these in pindelcarations.h too
uint8_t latchPin = 10;
uint8_t dataPin = 11;
uint8_t clockPin = 13;
uint8_t rdPin = A5;
uint8_t wrPin = A3;
uint8_t mreqPin = A4;
uint8_t mosfetControlPin = A0;

#include "pindeclarations.h"
#include <SPI.h>
#include <util/delay.h>
#include <string.h>

uint8_t nintendoLogo[] = {0xCE, 0xED, 0x66, 0x66, 0xCC, 0x0D, 0x00, 0x0B, 0x03, 0x73, 0x00, 0x83, 0x00, 0x0C, 0x00, 0x0D,
                  0x00, 0x08, 0x11, 0x1F, 0x88, 0x89, 0x00, 0x0E, 0xDC, 0xCC, 0x6E, 0xE6, 0xDD, 0xDD, 0xD9, 0x99,
                  0xBB, 0xBB, 0x67, 0x63, 0x6E, 0x0E, 0xEC, 0xCC, 0xDD, 0xDC, 0x99, 0x9F, 0xBB, 0xB9, 0x33, 0x3E};
char gameTitle[17];
uint16_t cartridgeType = 0;
uint16_t romSize = 0;
uint16_t romBanks = 0;
uint16_t ramSize = 0;
uint16_t ramBanks = 0;
uint16_t ramEndAddress = 0;

void setup() {
  pinMode(latchPin, OUTPUT);
  pinMode(clockPin, OUTPUT);
  pinMode(dataPin, OUTPUT);
  pinMode(rdPin, OUTPUT);
  pinMode(wrPin, OUTPUT);
  pinMode(mreqPin, OUTPUT);
  pinMode(mosfetControlPin, OUTPUT);
  PORTC |= (1<<PC0); // Turn mosfet off
  
  // Set pins as inputs
  DDRB &= ~((1<<PB0) | (1<<PB1)); // D8 & D9
  DDRD &= ~((1<<PD2) | (1<<PD3) | (1<<PD4) | (1<<PD5) | (1<<PD6) | (1<<PD7)); // D2 to D7
  rd_wr_mreq_off();
  
  // Setup pin interrupt for button
  PCMSK1 = (1<<PCINT9);
  PCICR = (1<<PCIE1);
  
  Serial.begin(250000);
}

void loop() {
  // Wait for serial input
  while (Serial.available() <= 0) {
    delay(200);
  }

  // Decode input
  char readInput[10];
  uint8_t readCount = 0;
  while (Serial.available() > 0) {
    char c = Serial.read();
    readInput[readCount] = c;
    readCount++;
  }
  readInput[readCount] = '\0';

  // Cartridge Header
  if (strstr(readInput, "HEADER")) {
    rd_wr_mreq_reset();

    // Read Cartridge Header
    // All Wisdom Tree games have 0x00's from 0x134 through 0x14C
    int count = 0;
    for (uint16_t romAddress = 0x0134; romAddress <= 0x14C; romAddress++) {
      if (romAddress <= 0x143) {
        gameTitle[(romAddress-0x0134)] = (char) read_byte(romAddress);
      }
      count += read_byte(romAddress);     // Running total of byte values
    }
    gameTitle[16] = '\0';
    
    // Nintendo Logo Check
    uint8_t logoCheck = 1;
    uint8_t x = 0;
    for (uint16_t romAddress = 0x0104; romAddress <= 0x133; romAddress++) {
      if (nintendoLogo[x] != read_byte(romAddress)) {
        logoCheck = 0;
        break;
      }
      x++;
    }

    cartridgeType = read_byte(0x0147);
    romSize = read_byte(0x0148);
    ramSize = read_byte(0x0149);

    //Wisdom Tree game
    if (count == 0) {  // Count is 0 if all byte values from 0x134 to 0x14C are 0x00
      uint8_t unique = read_byte(0x600); //This address contains a unique byte for each Wisdom Tree game
      String title = "";
      switch(unique) {
        case 0x79:
          title = "SPIRITUALWARFARE";
          romSize = 3;
          break;
        case 0x78:
          title = "NIVBIBLE";
          romSize = 6;
          break;
        case 0x00:
          title = "KJBIBLE";
          romSize = 5;
          break;
        case 0x05:
          title = "JOSHUA";
          romSize = 2;
          break;
        case 0x7C:
          title = "EXODUS";
          romSize = 2;
          break;
      }
      title.toCharArray(gameTitle,17);  // Gives each Wisdom Tree game a unique title, otherwise would be null
      cartridgeType = 99;  // Fake cartridge type used to identify Wisdom Tree game
      ramSize = 0; //No Wisdom Tree carts have RAM
    }
    
    // ROM banks
    romBanks = 2; // Default 32K
    if (romSize >= 1) { // Calculate rom size
      romBanks = 2 << romSize;
    }
    // Wisdom Tree mapper uses 32K banks, so half the number of banks
    if (cartridgeType == 99) {
      romBanks = romBanks / 2;
    }
    
    // RAM banks
    ramBanks = 0; // Default 0K RAM
    if (cartridgeType == 6) { ramBanks = 1; }
    if (ramSize == 2) { ramBanks = 1; }
    if (ramSize == 3) { ramBanks = 4; }
    if (ramSize == 4) { ramBanks = 16; }
    if (ramSize == 5) { ramBanks = 8; }
    
    // RAM end address
    if (cartridgeType == 6) { ramEndAddress = 0xA1FF; } // MBC2 512bytes (nibbles)
    if (ramSize == 1) { ramEndAddress = 0xA7FF; } // 2K RAM
    if (ramSize > 1) { ramEndAddress = 0xBFFF; } // 8K RAM

    Serial.println(gameTitle);
    Serial.println(cartridgeType);
    Serial.println(romSize);
    Serial.println(ramSize);
    Serial.println(logoCheck);
  }

  // Dump ROM
  else if (strstr(readInput, "READROM")) {
    rd_wr_mreq_reset();
    uint16_t romAddress = 0;
        
    // Read number of banks and switch banks
    for (uint16_t bank = 1; bank < romBanks; bank++) {
      if (cartridgeType >= 5 && cartridgeType != 99) { // MBC2 and above
        write_byte(0x2100, bank); // Set ROM bank
      }
      else if (cartridgeType == 99) { // Wisdom Tree mapper
        write_byte(bank - 1, 1);  // Writes to address select bank, not value
      }
      else { // MBC1
        write_byte(0x6000, 0); // Set ROM Mode 
        write_byte(0x4000, bank >> 5); // Set bits 5 & 6 (01100000) of ROM bank
        write_byte(0x2000, bank & 0x1F); // Set bits 0 & 4 (00011111) of ROM bank
      }
      if (bank > 1) { romAddress = 0x4000; }
      if (cartridgeType == 99) { romAddress = 0x0000; }  //Wisdom Tree banks are 32K, so read entire range
      
      // Read up to 7FFF per bank
      while (romAddress <= 0x7FFF) {
        uint8_t readData[64];
        for (uint8_t i = 0; i < 64; i++) {
          readData[i] = read_byte(romAddress+i);
        }
        
        Serial.write(readData, 64); // Send the 64 byte chunk
        romAddress += 64;
      }
    }
    if (cartridgeType == 99) { // Read last bank of Wisdom Tree game
      romAddress = 0x0000;
      write_byte(romBanks - 1, 1);
      while (romAddress <= 0x7FFF) {
        uint8_t readData[64];
        for (uint8_t i = 0; i < 64; i++) {
          readData[i] = read_byte(romAddress+i);
        }
        Serial.write(readData, 64); // Send the 64 byte chunk
        romAddress += 64;
      }
    }
  }

  // Read RAM
  else if (strstr(readInput, "READRAM")) {
    rd_wr_mreq_reset();
    
    // MBC2 Fix (unknown why this fixes reading the ram, maybe has to read ROM before RAM?)
    read_byte(0x0134);
    
    // Does cartridge have RAM
    if (ramEndAddress > 0) {
      if (cartridgeType <= 4) { // MBC1
        write_byte(0x6000, 1); // Set RAM Mode
      }
      
      // Initialise MBC
      write_byte(0x0000, 0x0A);
      
      // Switch RAM banks
      for (uint8_t bank = 0; bank < ramBanks; bank++) {
        write_byte(0x4000, bank);
        
        // Read RAM
        for (uint16_t ramAddress = 0xA000; ramAddress <= ramEndAddress; ramAddress += 64) {
          uint8_t readData[64];
          for (uint8_t i = 0; i < 64; i++) {
            readData[i] = read_byte(ramAddress+i);
          }
          
          Serial.write(readData, 64); // Send the 64 byte chunk
        }
      }
      
      // Disable RAM
      write_byte(0x0000, 0x00);
    }
  }
  
  // Write RAM
  else if (strstr(readInput, "WRITERAM")) {
    rd_wr_mreq_reset();
    
    // MBC2 Fix (unknown why this fixes it, maybe has to read ROM before RAM?)
    read_byte(0x0134);
    
    // Does cartridge have RAM
    if (ramEndAddress > 0) {
      if (cartridgeType <= 4) { // MBC1
        write_byte(0x6000, 1); // Set RAM Mode
      }
      
      // Initialise MBC
      write_byte(0x0000, 0x0A);
      
      // Switch RAM banks
      for (uint8_t bank = 0; bank < ramBanks; bank++) {
        write_byte(0x4000, bank);
        
        // Write RAM
        for (uint16_t ramAddress = 0xA000; ramAddress <= ramEndAddress; ramAddress++) {
          // Wait for serial input
          while (Serial.available() <= 0);
          
          // Read input
          uint8_t readValue = (uint8_t) Serial.read();
          
          // Write to RAM
          mreqPin_low;
          write_byte(ramAddress, readValue);
          asm volatile("nop");
          asm volatile("nop");
          asm volatile("nop");
          mreqPin_high; 
        }
      }
      
      // Disable RAM
      write_byte(0x0000, 0x00);
      Serial.flush(); // Flush any serial data that wasn't processed
    }
  }

  rd_wr_mreq_off();
}

uint8_t read_byte(uint16_t address) {
  shiftout_address(address); // Shift out address
  
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

void write_byte(uint16_t address, uint8_t data) {
  // Set pins as outputs
  DDRB |= ((1<<PB0) | (1<<PB1)); // D8 & D9
  DDRD |= ((1<<PD2) | (1<<PD3) | (1<<PD4) | (1<<PD5) | (1<<PD6) | (1<<PD7)); // D2 to D7
  
  shiftout_address(address); // Shift out address
  
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
void shiftout_address(uint16_t shiftAddress) {
  SPI.transfer(shiftAddress >> 8);
  SPI.transfer(shiftAddress & 0xFF);
  
  latchPin_low;
  asm volatile("nop");
  latchPin_high;
}

// Turn RD, WR and MREQ to high so they are deselected (reset state)
void rd_wr_mreq_reset(void) {
  rdPin_high; // RD off
  wrPin_high; // WR off
  mreqPin_high; // MREQ off
}

// Turn RD, WR and MREQ off as no power should be applied to GB Cart
void rd_wr_mreq_off(void) {
  rdPin_low;
  wrPin_low;
  mreqPin_low;
}

ISR (PCINT1_vect) {
  _delay_ms(10);
  if (PINC & (1<<PC1)) { // Invert mosfet control pin
    PORTC ^= (1<<PC0);
  }

  while (PINC & (1<<PC1)); // Wait for button release
  _delay_ms(250);

  // Check if mosfet is on or off (inverted as it's a P mosfet)
  if (PINC & (1<<PC0)) {
    SPI.end(); // End SPI
    rd_wr_mreq_off(); // Set everying low
    latchPin_low;
    dataPin_low;
    clockPin_low;
    _delay_ms(10);
    asm volatile ("jmp 0"); // Jump to address 0 (soft reset)
  }
  else {
    SPI.begin(); // Setup SPI
    SPI.setBitOrder(MSBFIRST);
    SPI.setDataMode(SPI_MODE0);
    SPI.setClockDivider(SPI_CLOCK_DIV2);
  }
} 
