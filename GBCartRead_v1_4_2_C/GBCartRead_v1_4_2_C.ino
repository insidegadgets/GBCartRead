/*
 GBCartRead
 Version: 1.4.2
 Author: Alex from insideGadgets (http://www.insidegadgets.com)
 Created: 18/03/2011
 Last Modified: 14/11/2014
 
 Read ROM, Read RAM or Write RAM from/to a Gameboy Cartridge.
 To be used with the gbcartread.exe C program.
 
 */
 
int latchPin = 10;
int dataPin = 11;
int clockPin = 12;
int rdPin = A5;
int wrPin = A3;
int mreqPin = A4;

void setup() {
  Serial.begin(57600);
  pinMode(latchPin, OUTPUT);
  pinMode(clockPin, OUTPUT);
  pinMode(dataPin, OUTPUT);
  pinMode(rdPin, OUTPUT);
  pinMode(wrPin, OUTPUT);
  pinMode(mreqPin, OUTPUT);
  for (int i = 2; i <= 9; i++) {
    pinMode(i, INPUT);
  }
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
  digitalWrite(rdPin, HIGH); // RD off
  digitalWrite(wrPin, HIGH); // WR off
  digitalWrite(mreqPin, HIGH); // MREQ off
  
  // Read Cartridge Header
  char gameTitle[17];
  for (int addr = 0x0134; addr <= 0x143; addr++) {
    gameTitle[(addr-0x0134)] = (char) readbank0Address(addr);
  }
  gameTitle[16] = '\0';
  int cartridgeType = readbank0Address(0x0147);
  int romSize = readbank0Address(0x0148);
  int ramSize = readbank0Address(0x0149);
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
  
  // Cartridge Header
  if (strstr(readInput, "HEADER")) {
    Serial.println(gameTitle);
    Serial.println(cartridgeType);
    Serial.println(romSize);
    Serial.println(ramSize);
  }
  
  // Dump ROM
  else if (strstr(readInput, "READROM")) {
    //Serial.println("START");
    unsigned int addr = 0;
    
    // Read x number of banks
    for (int y = 1; y < romBanks; y++) {
      selectROMbank(y);

      if (y > 1) {
        addr = 0x4000;
      }
    
      for (; addr <= 0x7FFF; addr++) {  
        shiftoutAddress(addr); // Shift out
        digitalWrite(rdPin, LOW); // RD on
        byte bval = readdataPins(); // Read data
        digitalWrite(rdPin, HIGH); // RD off 
        Serial.write(bval);
      }
    }
    //Serial.println("END");
  }
  
  // Read RAM
  else if (strstr(readInput, "READRAM")) {
  
    // MBC2 Fix (unknown why this fixes it, maybe has to read ROM before RAM?)
    shiftoutAddress(0x0134);
    
    //Serial.println("START");
    unsigned int addr = 0;
    unsigned int endaddr = 0;
    if (cartridgeType == 6 && ramSize == 0) { endaddr = 0xA1FF; } // MBC2 512bytes (nibbles)
    if (ramSize == 1) { endaddr = 0xA7FF; } // 2K RAM
    if (ramSize == 2 || ramSize == 3) { endaddr = 0xBFFF; } // 8K RAM
    
    // Does cartridge have RAM
    if (endaddr > 0) {
    
      // Initialise MBC
      shiftoutAddress(0x0000);
      bankSelect(0x0A);
      digitalWrite(wrPin, LOW); // WR on
      digitalWrite(wrPin, HIGH); // WR off
      
      // Switch RAM banks
      for (int bank = 0; bank < ramBanks; bank++) {
        shiftoutAddress(0x4000);// Shift out
        bankSelect(bank); // Select bank
        digitalWrite(wrPin, LOW); // WR on
        digitalWrite(wrPin, HIGH); // WR off
        
        // Turn outputs off and change back to inputs
        for (int l = 2; l <= 9; l++) {
          digitalWrite(l, LOW);
          pinMode(l, INPUT);
        }
        
        // Read RAM
        for (addr = 0xA000; addr <= endaddr; addr++) {  
          shiftoutAddress(addr); // Shift out
          digitalWrite(mreqPin, LOW); // MREQ on
          digitalWrite(rdPin, LOW); // RD on
          byte bval = readdataPins(); // Read data pins
          if (ramSize == 0) { bval |= 0x0F<<4; } // For MBC2
          digitalWrite(mreqPin, HIGH); // MREQ off
          digitalWrite(rdPin, HIGH); // RD off
          Serial.write(bval);
        }
      }
      
      // Disable RAM
      shiftoutAddress(0x0000);
      bankSelect(0x00);
      digitalWrite(wrPin, LOW); // WR on
      digitalWrite(wrPin, HIGH); // WR off
    
      // Set pins back to inputs
      for (int l = 2; l <= 9; l++) {
        pinMode(l, INPUT);
      }
    }
    //Serial.println("END");
  }
  
  // Write RAM
  else if (strstr(readInput, "WRITERAM")) {
  
    // MBC2 Fix (unknown why this fixes it, maybe has to read ROM before RAM?)
    shiftoutAddress(0x0134);
    
    //Serial.println("START");
    unsigned int addr = 0;
    unsigned int endaddr = 0;
    if (cartridgeType == 6 && ramSize == 0) { endaddr = 0xA1FF; } // MBC2 512bytes (nibbles)
    if (ramSize == 1) { endaddr = 0xA7FF; } // 2K RAM
    if (ramSize == 2 || ramSize == 3) { endaddr = 0xBFFF; } // 8K RAM
    
    // Does cartridge have RAM
    if (endaddr > 0) {
    
      // Initialise MBC
      shiftoutAddress(0x0000);
      bankSelect(0x0A);
      digitalWrite(wrPin, LOW); // WR on
      digitalWrite(wrPin, HIGH); // WR off
      
      // Switch RAM banks
      for (int bank = 0; bank < ramBanks; bank++) {
        shiftoutAddress(0x4000);// Shift out
        bankSelect(bank); // Select bank
        digitalWrite(wrPin, LOW); // WR on
        digitalWrite(wrPin, HIGH); // WR off
        
        // Write RAM
        for (addr = 0xA000; addr <= endaddr; addr++) {  
          shiftoutAddress(addr); // Shift out
          digitalWrite(mreqPin, LOW); // MREQ on
          digitalWrite(wrPin, LOW); // WR on
          
          // Wait for serial input
          int waiting = 0;
          while (Serial.available() <= 0) {
            delay(1);
            if (waiting == 0) {  
              Serial.print("N"); // Tell Python script to send next 64bytes
              waiting = 1;
            }
          }
    
          // Decode input
          byte bval = 0;
          if (Serial.available() > 0) {
            char c = Serial.read();
            bval = (int) c;
          }
          
          // Read the bits in the received character and turn on the 
          // corresponding D0-D7 pins
          for (int z = 9; z >= 2; z--) {
            if (bitRead(bval, z-2) == HIGH) {
              digitalWrite(z, HIGH);
            }
            else {
              digitalWrite(z, LOW);
            }
          }
          
          digitalWrite(mreqPin, HIGH); // MREQ off
          digitalWrite(wrPin, HIGH); // WR off
        }
      }
      
      // Disable RAM
      shiftoutAddress(0x0000);
      bankSelect(0x00);
      digitalWrite(wrPin, LOW); // WR on
      digitalWrite(wrPin, HIGH); // WR off
      
      // Set pins back to inputs
      for (int l = 2; l <= 9; l++) {
        pinMode(l, INPUT);
      }
      
      Serial.flush(); // Flush any serial data that wasn't processed
    }
    Serial.println("END");
  }
}

// Select the ROM bank by writing the bank number on the data pins
void selectROMbank(int bank) {
  shiftoutAddress(0x2100); // Shift out
  bankSelect(bank); // Select the bank
  digitalWrite(wrPin, LOW); // WR on
  digitalWrite(wrPin, HIGH); // WR off
   
  // Reset outputs to LOW and change back to inputs
  for (int i = 2; i <= 9; i++) {
    digitalWrite(i, LOW);
    pinMode(i, INPUT);
  }
}

// Read bank 0 address
int readbank0Address(unsigned int address) {
  shiftoutAddress(address); // Shift out address
  digitalWrite(rdPin, LOW); // RD on
  byte bval = readdataPins(); // Read data
  digitalWrite(rdPin, HIGH); // RD off 
  return bval;
}

// Use the shift registers to shift out the address
void shiftoutAddress(unsigned int shiftAddress) {
  digitalWrite(latchPin, LOW);
  shiftOut(dataPin, clockPin, MSBFIRST, (shiftAddress >> 8));
  shiftOut(dataPin, clockPin, MSBFIRST, (shiftAddress & 0xFF));
  digitalWrite(latchPin, HIGH);
  delayMicroseconds(50);
}

// Turn on the data lines corresponding to the bank number
void bankSelect(int bank) {
  
  // Change to outputs
  for (int i = 2; i <= 9; i++) {
    pinMode(i, OUTPUT);
  }
  
  // Read bits in bank variable
  for (int z = 9; z >= 2; z--) {
    if (bitRead(bank, z-2) == HIGH) {
      digitalWrite(z, HIGH);
    }
    else {
      digitalWrite(z, LOW);
    }
  }
}

// Read data pins
byte readdataPins() {
  
  // Change to inputs
  for (int i = 2; i <= 9; i++) {
    pinMode(i, INPUT);
  }
  
  // Read data pins
  byte bval = 0;
  for (int z = 9; z >= 2; z--) {
    if (digitalRead(z) == HIGH) {
      bitWrite(bval, (z-2), HIGH);
    }
  }
  
  return bval;
}
