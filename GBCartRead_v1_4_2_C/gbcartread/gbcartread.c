/*
 GBCartRead - Gameboy Cart Reader - Arduino Serial Reader
 Version: 1.4
 Author: Alex from insideGadgets (www.insidegadgets.com)
 Created: 21/07/2013
 Last Modified: 30/08/2013

 */

#include <stdlib.h>
#include <stdio.h>
#include <time.h>
#include <string.h>

#ifdef _WIN32
#include <Windows.h>
#else
#include <unistd.h>
#endif

#include "rs232.h"

int cport_nr = 0, // /dev/ttyS0 (COM1 on windows)
bdrate = 57600; // 57600 baud

// Read the config.ini file for the COM port to use
void read_config(void) {
	FILE* configfile = fopen ( "config.ini" , "rb" );
	char* buffer;
	if (configfile != NULL) {
		// Allocate memory 
		buffer = (char*) malloc (sizeof(char) * 2);
		
		// Copy the file into the buffer, we only read 2 characters
		fread (buffer, 1, 2, configfile);
		cport_nr = atoi(buffer);
		
		fclose (configfile);
		free (buffer);
	}
}

// Read one letter from stdin
int read_one_letter(void) {
	int c = getchar();
	while (getchar() != '\n' && getchar() != EOF);
	return c;
}

// Write serial data to file - used for ROM and RAM dumping
void write_to_file(char* filename, char* cmd) {
	// Create a new file
	FILE *pFile = fopen(filename, "wb");
	RS232_cputs(cport_nr, cmd);
	
	// Wait a little bit until we start gettings some data
	#ifdef _WIN32
	Sleep(500);
	#else
	usleep(500000); // Sleep for 500 milliseconds
	#endif
	
	int Kbytesread = 0;
	int uptoKbytes = 1;
	unsigned char buf[4096];
	int n = 0;
	while(1) {
		n = RS232_PollComport(cport_nr, buf, 4095);
		
		if (n > 0) {
			buf[n] = 0;
			fwrite((char *) buf, 1, n, pFile);
			printf("#");
			Kbytesread += n;
			if (Kbytesread / 32768 == uptoKbytes) {
				printf("%iK", (Kbytesread/32768) * 32);
				uptoKbytes++;
			}
			fflush(stdout);
		}
		else {
			break;
		}
		
		#ifdef _WIN32
		Sleep(100);
		#else
		usleep(100000); // Sleep for 100 milliseconds
		#endif
	}
	
	fclose(pFile);
}

// Read from file to serial - used writing to RAM
void read_from_file(char* filename, char* cmd) {
	// Load a new file
	FILE *pFile = fopen(filename, "rb");
	RS232_cputs(cport_nr, cmd);
	
	// Wait a little bit until we start gettings some data
	#ifdef _WIN32
	Sleep(500);
	#else
	usleep(500000); // Sleep for 500 milliseconds
	#endif
	
	int Kbytesread = 0;
	int uptoKbytes = 1;
	unsigned char buf[4096];
	unsigned char readbuf[100];
	int n = 0;
	while(1) {
		n = RS232_PollComport(cport_nr, buf, 4095);
		
		if (n > 0) {
			buf[n] = 0;
			
			// Exit if save is finished
			if (strstr(buf, "END")) {
				break;
			}
			
			fread((char *) readbuf, 1, 64, pFile);
			readbuf[64] = 0;
			
			// Becuase Sendbuf doesn't work properly, so send one byte at a time
			int z = 0;
			for (z = 0; z < 64; z++) {
				RS232_SendByte(cport_nr, readbuf[z]);
				Sleep(1);
			}
			
			printf("#");
			Kbytesread += n;
			if (Kbytesread / 32768 == uptoKbytes) {
				printf("%iK", (Kbytesread/32768) * 32);
				uptoKbytes++;
			}
			fflush(stdout);
		}
		else {
			break;
		}
		
		#ifdef _WIN32
		Sleep(20);
		#else
		usleep(200000); // Sleep for 100 milliseconds
		#endif
	}
	
	fclose(pFile);
}

int main() {
	read_config();
	
	printf("GBCartRead v1.4 by insideGadgets\n");
	printf("################################\n\n");
	
	printf("Opening COM PORT %d...\n\n", cport_nr+1);
	
	// Open COM port
	if(RS232_OpenComport(cport_nr, bdrate)) {
		printf("Can not open comport\n");
		return(0);
	}
	
	// Read gameboy cart header
	while (1) {
		#ifdef _WIN32
		Sleep(2000);
		#else
		usleep(2000000); // Sleep for 2 seconds
		#endif
		
		RS232_cputs(cport_nr, "HEADER\n");
		
		unsigned char buf[4096];
		unsigned char header[50];
		int headersize = 0;
		int n = 0;
		int waitingforheader = 0;
		while (1) {
			n = RS232_PollComport(cport_nr, buf, 4095);
			if (n > 0) {
				buf[n] = 0;
				if (headersize == 0) {
					strncpy((char *) header, (char *) buf, n);
				}
				else {
					strncat((char *) header, (char *) buf, n);
				}
				headersize += n;
			}
			waitingforheader++;
			if (waitingforheader >= 10) {
				break;
			}
			
			#ifdef _WIN32
			Sleep(100);
			#else
			usleep(100000); // Sleep for 2 seconds
			#endif
		}
		header[headersize] = '\0';
		
		// Split the string
		int headercounter = 0;
		char gametitle[80];
		int cartridgeType = 255;
		int romSize = 255;
		int ramSize = 255;
		
		char* tokstr;
		tokstr = strtok ((char *) header, "\r\n");
		while (tokstr != NULL) {
			if (headercounter == 0) {
				printf ("Game title... ");
				printf ("%s\n", tokstr);
				strncpy(gametitle, tokstr, 30);
			}
			if (headercounter == 1) {
				printf ("MBC type... ");
				cartridgeType = atoi(tokstr);
				
				switch (cartridgeType) {
					case 0: printf ("ROM ONLY\n"); break;
					case 1: printf ("MBC1\n"); break;
					case 2: printf ("MBC1+RAM\n"); break;
					case 3: printf ("MBC1+RAM+BATTERY\n"); break;
					case 5: printf ("MBC2\n"); break;
					case 6: printf ("MBC2+BATTERY\n"); break;
					case 8: printf ("ROM+RAM\n"); break;
					case 9: printf ("ROM ONLY\n"); break;
					case 11: printf ("MMM01\n"); break;
					case 12: printf ("MMM01+RAM\n"); break;
					case 13: printf ("MMM01+RAM+BATTERY\n"); break;
					case 15: printf ("MBC3+TIMER+BATTERY\n"); break;
					case 16: printf ("MBC3+TIMER+RAM+BATTERY\n"); break;
					case 17: printf ("MBC3\n"); break;
					case 18: printf ("MBC3+RAM\n"); break;
					case 19: printf ("MBC3+RAM+BATTERY\n"); break;
					case 21: printf ("MBC4\n"); break;
					case 22: printf ("MBC4+RAM\n"); break;
					case 23: printf ("MBC4+RAM+BATTERY\n"); break;
					case 25: printf ("MBC5\n"); break;
					case 26: printf ("MBC5+RAM\n"); break;
					case 27: printf ("MBC5+RAM+BATTERY\n"); break;
					case 28: printf ("MBC5+RUMBLE\n"); break;
					case 29: printf ("MBC5+RUMBLE+RAM\n"); break;
					case 30: printf ("MBC5+RUMBLE+RAM+BATTERY\n"); break;
					default: printf ("Not found\n");
				}
			}
			else if (headercounter == 2) {
				printf ("ROM size... ");
				romSize = atoi(tokstr);
				
				switch (romSize) {
					case 0: printf ("32KByte (no ROM banking)\n"); break;
					case 1: printf ("64KByte (4 banks)\n"); break;
					case 2: printf ("128KByte (8 banks)\n"); break;
					case 3: printf ("256KByte (16 banks)\n"); break;
					case 4: printf ("512KByte (32 banks)\n"); break;
					case 5: 
						if (cartridgeType == 1 || cartridgeType == 2 || cartridgeType == 3) {
							printf ("1MByte (63 banks)\n");
						}
						else {
							printf ("1MByte (64 banks)\n");
						}
						break;
					case 6: 
						if (cartridgeType == 1 || cartridgeType == 2 || cartridgeType == 3) {
							printf ("2MByte (125 banks)\n");
						}
						else {
							printf ("2MByte (128 banks)\n");
						}
						break;
					case 7: printf ("4MByte (256 banks)\n"); break;
					case 82: printf ("1.1MByte (72 banks)\n"); break;
					case 83: printf ("1.2MByte (80 banks)\n"); break;
					case 84: printf ("1.5MByte (96 banks)\n"); break;
					default: printf ("Not found\n");
				}
			}
			else if (headercounter == 3) {
				printf ("RAM size... ");
				ramSize = atoi(tokstr);
				
				switch (ramSize) {
					case 0: 
						if (cartridgeType == 6) {
							printf ("512 bytes (nibbles)\n");
						}
						else {
							printf ("None\n");
						}
						break;
					case 1: printf ("2 KBytes\n"); break;
					case 2: printf ("8 KBytes\n"); break;
					case 3: printf ("32 KBytes (4 banks of 8Kbytes)\n"); break;
					default: printf ("Not found\n");
				}
			}
			
			tokstr = strtok (NULL, "\r\n");
			headercounter++;
		}
		
		printf ("\nSelect an option below\n1. Dump ROM\n2. Save RAM\n3. Write RAM\n4. Exit\n");
		printf (">");
		
		char userInput = read_one_letter();
		
		char filename[30];
		if (userInput == '1') {    
			printf ("\nDumping ROM to %s.gb... ", gametitle);
			strncpy(filename, gametitle, 20);
			strcat(filename, ".gb");
			write_to_file(filename, "READROM\n");
		}
		else if (userInput == '2') {    
			printf ("\nDumping RAM to %s.sav... ", gametitle);
			strncpy(filename, gametitle, 20);
			strcat(filename, ".sav");
			write_to_file(filename, "READRAM\n");
		}
		else if (userInput == '3') { 
			printf ("\nGoing to write to RAM from %s.sav...", gametitle);
			printf ("\n*** This will erase the save game from your Gameboy Cartridge ***");
			printf ("\nPress y to continue or any other key to abort.");
			printf ("\n>");
			
			char userInputConfirm = read_one_letter();	
			
			if (userInputConfirm == 'y') {
				printf ("\nWriting to RAM from %s.sav... ", gametitle);
				strncpy(filename, gametitle, 20);
				strcat(filename, ".sav");
				read_from_file(filename, "WRITERAM\n");
			}
			else {
				printf ("Aborted.\n");
			}
		}
		else {  
			RS232_CloseComport(cport_nr);
			return(0);
		}
		
		printf("\n\n");
	}
	
	return(0);
}
