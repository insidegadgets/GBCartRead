import serial
import string
import sys
import time

ser = serial.Serial('COM2', 57600) # Change COM2 to the port the Arduino is on
time.sleep(2) # Arduino gets reset when serial is started so we wait a bit
ser.write('HEADER'.encode())

sys.stdout.write('GBCartRead v1.3 by InsideGadgets\n')
sys.stdout.write('################################\n')
sys.stdout.write('Game title... ')
gameTitle = ascii(ser.readline())
gameTitle = gameTitle[2:(len(gameTitle)-5)]
print (gameTitle)

sys.stdout.write('MBC type... ')
cartridgeType = ascii(ser.readline())
cartridgeType = int(cartridgeType[2:(len(cartridgeType)-5)])
if (cartridgeType == 0):
    print ('ROM ONLY')
elif (cartridgeType == 1):
    print ('MBC1')
elif (cartridgeType == 2):
    print ('MBC1+RAM')
elif (cartridgeType == 3):
    print ('MBC1+RAM+BATTERY')
elif (cartridgeType == 5):
    print ('MBC2')
elif (cartridgeType == 6):
    print ('MBC2+BATTERY')
elif (cartridgeType == 8):
    print ('ROM+RAM')
elif (cartridgeType == 9):
    print ('ROM+RAM+BATTERY')
elif (cartridgeType == 11):
    print ('MMM01')
elif (cartridgeType == 12):
    print ('MMM01+RAM')
elif (cartridgeType == 13):
    print ('MMM01+RAM+BATTERY')
elif (cartridgeType == 15):
    print ('MBC3+TIMER+BATTERY')
elif (cartridgeType == 16):
    print ('MBC3+TIMER+RAM+BATTERY')
elif (cartridgeType == 17):
    print ('MBC3')
elif (cartridgeType == 18):
    print ('MBC3+RAM')
elif (cartridgeType == 19):
    print ('MBC3+RAM+BATTERY')
elif (cartridgeType == 21):
    print ('MBC4')
elif (cartridgeType == 22):
    print ('MBC4+RAM')
elif (cartridgeType == 23):
    print ('MBC4+RAM+BATTERY')
elif (cartridgeType == 25):
    print ('MBC5')
elif (cartridgeType == 26):
    print ('MBC5+RAM')
elif (cartridgeType == 27):
    print ('MBC5+RAM+BATTERY')
elif (cartridgeType == 28):
    print ('MBC5+RUMBLE')
elif (cartridgeType == 29):
    print ('MBC5+RUMBLE+RAM')
elif (cartridgeType == 30):
    print ('MBC5+RUMBLE+RAM+BATTERY')
else:
    print ('Not found')
    
sys.stdout.write('ROM size... ')
romSize = ascii(ser.readline())
romSize = int(romSize[2:(len(romSize)-5)])
if (romSize == 0):
    print ('32KByte (no ROM banking)')
elif (romSize == 1):
    print ('64KByte (4 banks)')
elif (romSize == 2):
    print ('128KByte (8 banks)')
elif (romSize == 3):
    print ('256KByte (16 banks)')
elif (romSize == 4):
    print ('512KByte (32 banks)')
elif (romSize == 5 and (cartridgeType == 1 or cartridgeType == 2 or cartridgeType == 3)):
    print ('1MByte (63 banks)')
elif (romSize == 5):
    print ('1MByte (64 banks)')
elif (romSize == 6 and (cartridgeType == 1 or cartridgeType == 2 or cartridgeType == 3)):
    print ('2MByte (125 banks)')
elif (romSize == 6):
    print ('2MByte (128 banks)')
elif (romSize == 7):
    print ('4MByte (256 banks)')
elif (romSize == 82):
    print ('1.1MByte (72 banks)')
elif (romSize == 83):
    print ('1.2MByte (80 banks)')
elif (romSize == 84):
    print ('1.5MByte (96 banks)')
else:
    print('Not found')

sys.stdout.write('RAM size... ')
ramSize = ascii(ser.readline())
ramSize = int(ramSize[2:(len(ramSize)-5)])
if (ramSize == 0 and cartridgeType == 6):
    print ('512 bytes (nibbles)\n')
elif (ramSize == 0):
    print ('None\n')
elif (ramSize == 1):
    print ('2 KBytes\n')
elif (ramSize == 2):
    print ('8 KBytes\n')
elif (ramSize == 3):
    print ('32 KBytes (4 banks of 8Kbytes)\n')
else:
    print ('Not Found\n')

waitInput = 1
while (waitInput == 1):
    sys.stdout.write('Select an option below\n1. Dump ROM\n2. Save RAM\n3. Write RAM\n4. Exit\n')
    sys.stdout.write('>')
    userInput = input()

    if (userInput == "1"):      
        sys.stdout.write('\nDumping ROM to ' + gameTitle + '.gb... ')
        readBytes = 0
        inRead = 0
        Kbytesread = 0;
        ser.write('READROM'.encode())
        f = open(gameTitle+'.gb', 'wb')
        while 1:
            line = ser.readline()
            lineascii = ascii(line)
            if not line:
                break
            if lineascii.find('END') >= 0:
                break
            if inRead == 1:
                writeLine = chr(int(line))
                #print (writeLine),
                readBytes += 1
                f.write(writeLine.encode('latin_1'))
            if lineascii.find('START') >= 0:
                inRead = 1
            if readBytes % 1024 == 0 and readBytes != 0:
                sys.stdout.write('#')
            if readBytes % 32768 == 0 and readBytes != 0:
                Kbytesread = Kbytesread + 1
                Kbytesprint = Kbytesread * 32
                sys.stdout.write("%sK" % Kbytesprint)
        sys.stdout.write('\nFinished\n\n')
        f.close()

    elif (userInput == "2"):
        sys.stdout.write('\nDumping RAM to ' + gameTitle + '.sav... ')
        readBytes = 1
        inRead = 0
        Kbytesread = 0
        ser.write('READRAM'.encode())
        f = open(gameTitle+'.sav', 'wb')
        while 1:
            line = ser.readline()
            lineascii = ascii(line)
            if not line:
                break
            if lineascii.find('END') >= 0:
                break
            if inRead == 1:
                writeLine = chr(int(line))
                #print (writeLine),
                readBytes += 1
                f.write(writeLine.encode('latin_1'))
            if lineascii.find('START') >= 0:
                inRead = 1
            if readBytes % 256 == 0 and readBytes != 0:
                sys.stdout.write('#')
            if readBytes % 1024 == 0 and readBytes != 0:
                Kbytesread = Kbytesread + 1
                sys.stdout.write("%sK" % Kbytesread)
        sys.stdout.write('\nFinished\n\n')
        f.close()

    elif (userInput == "3"):
        sys.stdout.write('\nGoing to write to RAM from ' + gameTitle + '.sav...')
        sys.stdout.write('\n*** This will erase the save game from your Gameboy Cartridge ***')
        sys.stdout.write('\nPress y to continue or any other key to abort.')
        sys.stdout.write('\n>')
        userInput2 = input()

        if (userInput2 == "y"):
            sys.stdout.write('\nWriting to RAM from ' + gameTitle + '.sav... ')
            fileExists = 1
            doExit = 0
            printHash = 0
            Kbyteswrite = 0
            try:
                f = open(gameTitle+'.sav', 'rb')
            except IOError:
                sys.stdout.write('No save file found, aborted.\n\n')
                fileExists = 0
            if (fileExists == 1):
                ser.write('WRITERAM'.encode())
                while 1:
                    waitArduino = 1
                    while waitArduino == 1:
                        line = ser.readline()
                        lineascii = ascii(line)
                        if lineascii.find('NEXT') >= 0:
                            waitArduino = 0
                        if lineascii.find('END') >= 0:
                            doExit = 1
                            break
                    if printHash % 4 == 0 and printHash != 0: # 256 / 64 = 4
                        sys.stdout.write('#')
                    if printHash % 16 == 0 and printHash != 0:
                        Kbyteswrite = Kbyteswrite + 1
                        sys.stdout.write("%sK" % Kbyteswrite)
                    printHash += 1
                    
                    if doExit == 1:
                        break

                    line1 = f.read(64) # Read 64bytes of save file
                    if not line1:
                        break
                    ser.write(line1)
                    
                sys.stdout.write('\nFinished\n\n')
            f.close()
        else:
            sys.stdout.write('Aborted.\n\n')

    elif (userInput == "4"):
        waitInput = 0
        
    else:
        sys.stdout.write('\nOption not recognised, please try again.\n\n')
ser.close()
