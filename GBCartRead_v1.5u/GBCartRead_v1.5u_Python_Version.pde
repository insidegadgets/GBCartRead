/*
 GBCartRead
 Version: 1.5u
 Author: Alex from insideGadgets (http://www.insidegadgets.com)
 Edit: Frode vdM. (fvdm1992@yahoo.no)
 Created: 18/03/2011
 Last Modified: 08/11/2014
 
 Read ROM, Read RAM or Write RAM from/to a Gameboy Cartridge.

 Build from N64 Transfer Pack header, wich includes a nifty cartridge-sense switch.
 Connect switch to +5V on one side and to digital pin 13 by a pulldown on the other.
 Also connect CLK on the GB header to GROUND for support with the GB Camera.

 You MUST use a transistor to switch the power going to the shift registers and GB header
 (except for +5V going to the sense switch). Output A2 switches the power ON when LOW,
 and OFF when HIGH. Here is a diagram that also includes a LED indicator:

                     ,--------------------- +5V
               ,,--./
              /  |<´ \
 A2 ---[10K]-----|   | PNP Transistor
              \  |-. /
               ``--´\
                     `-+------------------- Power going to 74LS595's and GB Header
                       |
                       |           ^^
                       +--[ ? ]--->|---. (optional LED)
                       |               |
                       `-----[10K]-----+--- GND

 You should also put 10K resistors in series on pin A3, A4, A5, 10, 11 and 12 to prevent
 too much current from leaking through. This and the 10K pulldown in parallel with the
 optional LED is sufficient for getting the line to 0V when the transistor is blocking.

 */


int latchPin = 10;
int dataPin = 11;
int clockPin = 12;
int detectPin = 13;
int rdPin = A5;
int wrPin = A3;
int mreqPin = A4;
int powerPin = A2;

void setup() {
  Serial.begin(57600);
  pinMode(latchPin, OUTPUT);
  pinMode(clockPin, OUTPUT);
  pinMode(dataPin, OUTPUT);
  pinMode(rdPin, OUTPUT);
  pinMode(wrPin, OUTPUT);
  pinMode(mreqPin, OUTPUT);
  pinMode(detectPin, INPUT);
  pinMode(powerPin, OUTPUT);
  powerOff();
  inputs();
}

void loop() {
  // Wait for serial input
  int timeout = 50;
  while (Serial.available() <= 0) {
    delay(200);
    if(timeout >= 0)
      timeout--;
    if((digitalRead(detectPin) == LOW && digitalRead(powerPin) == LOW) || timeout == 0) {
      powerOff();
    }
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

  char gameTitle[17];  
  int cartridgeType = -1;
  int romSize = -1;
  int ramSize = -1;
  int romBanks = -1;
  int ramBanks = 1;

  if(digitalRead(powerPin) == LOW) {
    digitalWrite(latchPin, HIGH);
    digitalWrite(rdPin, HIGH); // RD off
    digitalWrite(wrPin, HIGH); // WR off
    digitalWrite(mreqPin, HIGH); // MREQ off

    // Read Cartridge Header
    for (int addr = 0x0134; addr <= 0x143; addr++) {
      gameTitle[(addr-0x0134)] = (char) readByte(addr);
    }
    gameTitle[16] = '\0';
    if (gameTitle[15] == (char)0x80)
      gameTitle[15] = '\0';
    else if (gameTitle[15] == (char)0xC0)
      gameTitle[11] = '\0';
    cartridgeType = readByte(0x0147);
    romSize = readByte(0x0148);
    ramSize = readByte(0x0149);


    romBanks = 2; // Default 32K
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
    ramBanks = 1; // Default 8K RAM
    if (ramSize == 3) { ramBanks = 4; }
    if (ramSize == 4){ ramBanks = 16; } // GB Camera
  }


  if (strstr(readInput, "POWERON")){
    powerOn();
    Serial.println("END");
  }

  else if (strstr(readInput, "POWEROFF")) {
    powerOff();
    Serial.println("END");
  }

  // Cartridge Header
  else if (strstr(readInput, "HEADER")) {
     if(digitalRead(detectPin) == HIGH && digitalRead(powerPin) == LOW) {
       Serial.println(gameTitle);
       Serial.println(cartridgeType);
       Serial.println(romSize);
       Serial.println(ramSize);
       Serial.println(digitalRead(powerPin) == LOW);
    }
    else {
      if(digitalRead(detectPin) == LOW){
        Serial.println("NOCART");
      }
      else {
        Serial.println("NOPOWER");
      }
      Serial.println(-1);
      Serial.println(-1);
      Serial.println(-1);
      Serial.println(digitalRead(powerPin) == LOW);
    }
  }

  // Dump ROM
  else if (strstr(readInput, "READROM")) {
    Serial.write(0x5A);
    Serial.write(0x55);
    if(digitalRead(detectPin) == HIGH && digitalRead(powerPin) == LOW) {
    unsigned int addr = 0;
    
    // Read x number of banks
    for (int y = 1; y < romBanks; y++) {
      writeByte(y, 0x2100); // Set ROM bank
      if (y > 1) {addr = 0x4000;}
      for (; addr <= 0x7FFF; addr = addr+64) {
        Serial.write(0xA5);
        Serial.write(0xA5);
        byte dat[64];
        for(int i=0; i<64; i++){
          dat[i] = readByte(addr+i);
        }
        Serial.write(dat, 64);
      }
    }
    }
    Serial.write(0x5A);
    Serial.write(0xAA);
  }
  
  // Read RAM
  else if (strstr(readInput, "READRAM")) {
 
    
    Serial.write(0x5A);
    Serial.write(0x55);
    if(digitalRead(detectPin) == HIGH && digitalRead(powerPin) == LOW) {
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
      writeByte(0x0A, 0x0000);
      
      // Switch RAM banks
      for (int bank = 0; bank < ramBanks; bank++) {
        writeByte(bank, 0x4000);
        
        // Read RAM
        for (addr = 0xA000; addr <= endaddr; addr=addr+64) {  
          Serial.write(0xA5);
          Serial.write(0xA5);
          byte dat[64];
          for(int i=0; i<64; i++){
            dat[i] = readByte(addr+i);
          }
          Serial.write(dat, 64);
        }
      }
      
      // Disable RAM
      writeByte(0x00, 0x0000);
   
    }
    }
    Serial.write(0x5A);
    Serial.write(0xAA);
  }
  
  // Write RAM
  else if (strstr(readInput, "WRITERAM")) {

    
    Serial.println("START");
    if(digitalRead(detectPin) == HIGH && digitalRead(powerPin) == LOW) {
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
      writeByte(0x0A, 0x0000);
      
      // Switch RAM banks
      for (int bank = 0; bank < ramBanks; bank++) {
        
        writeByte(bank, 0x4000);
        
        // Write RAM
        for (addr = 0xA000; addr <= endaddr; addr=addr+64) {  
          // Wait for serial input
          Serial.println("NEXT"); // Tell Python script to send next 64bytes
          // Decode input

          for(int i=0; i<64; i++) {
          while (Serial.available() <= 0) {
            delay(1);
          }
          byte bval = 0;
          if (Serial.available() > 0) {
            char c = Serial.read();
            bval = (int) c;
          }

          digitalWrite(mreqPin, LOW);
       	  writeByte(bval, addr+i);
          digitalWrite(mreqPin, HIGH);
          }
        }
      }
      // Disable RAM
      writeByte(0x00, 0x0000);
      
      Serial.flush(); // Flush any serial data that wasn't processed
    }
    }
    Serial.println("END");
  }
}



int readByte(int address) {
  shiftoutAddress(address); // Shift out address
  digitalWrite(mreqPin, LOW);
  digitalWrite(rdPin, LOW);
  delayMicroseconds(1);
  byte bval = ((PINB<<6)|(PIND>>2))&0xFF; // Read data
  digitalWrite(rdPin, HIGH);
  digitalWrite(mreqPin, HIGH);
  return bval;
}

void writeByte(int data, int address) {
  outputs();
  shiftoutAddress(address);
  setData(data);
  digitalWrite(wrPin, LOW);
  delayMicroseconds(1);
  digitalWrite(wrPin, HIGH);
  inputs();
}



// Use the shift registers to shift out the address
void shiftoutAddress(unsigned int shiftAddress) {

  PORTB &= B11100111;
  PORTB |= (shiftAddress >> 12)&B00001000;
  PORTB |= B00010000;
  PORTB &= B11100111;
  PORTB |= (shiftAddress >> 11)&B00001000;
  PORTB |= B00010000;
  PORTB &= B11100111;
  PORTB |= (shiftAddress >> 10)&B00001000;
  PORTB |= B00010000;
  PORTB &= B11100111;
  PORTB |= (shiftAddress >> 9)&B00001000;
  PORTB |= B00010000;
  PORTB &= B11100111;
  PORTB |= (shiftAddress >> 8)&B00001000;
  PORTB |= B00010000;
  PORTB &= B11100111;
  PORTB |= (shiftAddress >> 7)&B00001000;
  PORTB |= B00010000;
  PORTB &= B11100111;
  PORTB |= (shiftAddress >> 6)&B00001000;
  PORTB |= B00010000;
  PORTB &= B11100111;
  PORTB |= (shiftAddress >> 5)&B00001000;
  PORTB |= B00010000;
  PORTB &= B11100111;
  PORTB |= (shiftAddress >> 4)&B00001000;
  PORTB |= B00010000;
  PORTB &= B11100111;
  PORTB |= (shiftAddress >> 3)&B00001000;
  PORTB |= B00010000;
  PORTB &= B11100111;
  PORTB |= (shiftAddress >> 2)&B00001000;
  PORTB |= B00010000;
  PORTB &= B11100111;
  PORTB |= (shiftAddress >> 1)&B00001000;
  PORTB |= B00010000;
  PORTB &= B11100111;
  PORTB |= (shiftAddress)&B00001000;
  PORTB |= B00010000;
  PORTB &= B11100111;
  PORTB |= (shiftAddress << 1)&B00001000;
  PORTB |= B00010000;
  PORTB &= B11100111;
  PORTB |= (shiftAddress << 2)&B00001000;
  PORTB |= B00010000;
  PORTB &= B11100111;
  PORTB |= (shiftAddress << 3)&B00001000;
  PORTB |= B00010000;

  digitalWrite(latchPin, LOW);
  digitalWrite(latchPin, HIGH);
}



// Turn the data lines
void setData(int data) {
  data &= 0xFF;
  PORTD &= B00000011;
  PORTB &= B11111100;
  PORTD |= (data << 2);
  PORTB |= (data >> 6);
}



void inputs() {
  DDRB &= B11111100;
  DDRD &= B00000011;
}

void outputs() {
  DDRB |= B00000011;
  DDRD |= B11111100;
}



void powerOn() {
  if(digitalRead(detectPin) == HIGH) {
    digitalWrite(latchPin, HIGH);
    digitalWrite(rdPin, HIGH); // RD off
    digitalWrite(wrPin, HIGH); // WR off
    digitalWrite(mreqPin, HIGH); // MREQ off
    delay(100);
    digitalWrite(powerPin, LOW);
  }
}

void powerOff() {
  digitalWrite(powerPin, HIGH);
  delay(100);
  digitalWrite(latchPin, LOW);
  digitalWrite(dataPin, LOW);
  digitalWrite(rdPin, LOW); // RD off
  digitalWrite(wrPin, LOW); // WR off
  digitalWrite(mreqPin, LOW); // MREQ off
}