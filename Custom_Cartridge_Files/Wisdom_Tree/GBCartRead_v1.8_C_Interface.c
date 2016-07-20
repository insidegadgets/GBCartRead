/*
 GBCartRead - C Interface
 Version: 1.8
 Author: Alex from insideGadgets (www.insidegadgets.com)
 Created: 21/07/2013
 Last Modified: 21/03/2016

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
bdrate = 57600; // 57600 baud (default)
int firstStart = 1;

int headercounter = 0;
char gametitle[80];
char filename[30];
int cartridgeType = 255;
int romSize = 255;
int ramSize = 255;
int logoCheck = 0;

// Read the config.ini file for the COM port to use
void read_config(void) {
	FILE* configfile = fopen ( "config.ini" , "rb" );
	char* buffer;
	if (configfile != NULL) {
		// Allocate memory 
		buffer = (char*) malloc (sizeof(char) * 8);
		
		// Copy the first line into the buffer for COM port
		fread (buffer, 1, 2, configfile);
		cport_nr = atoi(buffer);
		
		// Copy the second line into the buffer for baud rate
		fread (buffer, 1, 8, configfile);
		bdrate = atol(buffer);
		
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
void write_to_file(char* filename, char* cmd, int blocksize) {
	// Create a new file
	FILE *pFile = fopen(filename, "wb");
	RS232_cputs(cport_nr, cmd);
	
	int Kbytesread = 0;
	int uptoKbytes = 1;
	unsigned char buf[4096];
	int n = 0;
	int timeout = 0;
	while(1) {
		n = RS232_PollComport(cport_nr, buf, 4095);
		
		if (n > 0) {
			buf[n] = 0;
			fwrite((char *) buf, 1, n, pFile);
			printf("#");
			Kbytesread += n;
			
			if (blocksize == 32) {
				if (Kbytesread / 32768 == uptoKbytes) {
					printf("%iK", (Kbytesread/32768) * 32);
					uptoKbytes++;
				}
			}
			else {
				if (Kbytesread / 1024 >= uptoKbytes) {
					printf("%iK", (Kbytesread/1024));
					uptoKbytes++;
				}
			}
			
			fflush(stdout);
			timeout = 0;
		}
		else {
			timeout++;
			if (timeout >= 50) {
				break;
			}
			
			#ifdef _WIN32
			Sleep(50);
			#else
			usleep(50000); // Sleep for 50 milliseconds
			#endif
		}
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
	unsigned char readbuf[100];
	while(1) {
		if (!(fread((char *) readbuf, 1, 64, pFile))) {
			break;
		}
		readbuf[64] = 0;
		
		// Send 64 bytes at a time
		RS232_SendBuf(cport_nr, readbuf, 64);
		
		printf("#");
		Kbytesread += 64;
		if (Kbytesread / 1024 == uptoKbytes) {
			printf("%iK", (Kbytesread/1024));
			uptoKbytes++;
		}
		fflush(stdout);
		
		#ifdef _WIN32
		Sleep(5);
		#else
		usleep(5000); // Sleep for 200 milliseconds
		#endif
	}
	
	fclose(pFile);
}

int main() {
	read_config();
	
	printf("GBCartRead v1.8 by insideGadgets\n");
	printf("################################\n\n");
	
	printf("Opening COM PORT %d at %d baud...\n\n", cport_nr+1, bdrate);
	
	// Open COM port
	if(RS232_OpenComport(cport_nr, bdrate)) {
		printf("Can not open comport\n");
		return(0);
	}
	
	#ifdef _WIN32
	Sleep(2000);
	#else
	usleep(2000000); // Sleep for 2 seconds
	#endif
	
	char userInput = '0';
	while (1) {
		printf ("\nSelect an option below\n0. Read Header\n1. Dump ROM\n2. Save RAM\n3. Write RAM\n4. Exit\n");
		printf (">");
		userInput = read_one_letter();
		
		if (userInput == '0') {
			headercounter = 0;
			RS232_cputs(cport_nr, "HEADER\n");
			
			unsigned char buffer[4096];
			int n = 0;
			int waitingforheader = 0;
			while (1) {
				n = RS232_PollComport(cport_nr, buffer, 4095);
				if (n > 0) {
					buffer[n] = 0;
					fflush(stdout);
					break; // Got the data, exit
				}
				else {
					waitingforheader++;
					if (waitingforheader >= 50) {
						break;
					}
					
					#ifdef _WIN32
					Sleep(50);
					#else
					usleep(50000); // Sleep for 100 milliseconds
					#endif
				}
			}
			
			char* tokstr = strtok ((char *) buffer, "\r\n");
			while (tokstr != NULL) {
				if (headercounter == 0) {
					printf ("\nGame title... ");
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
						case 99: printf ("WISDOM TREE MAPPER\n"); break;
						case 252: printf("Gameboy Camera\n"); break;
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
						case 4: printf ("128 KBytes (16 banks of 8Kbytes)\n"); break;
						default: printf ("Not found\n");
					}
				}
				else if (headercounter == 4) {
					printf ("Logo check... ");
					logoCheck = atoi(tokstr);
					
					if (logoCheck == 1) {
						printf ("OK\n");
					}
					else {
						printf ("Failed\n");
					}
				}
				
				tokstr = strtok (NULL, "\r\n");
				headercounter++;
			}
			fflush(stdout);
		}
		else if (userInput == '1') {    
			printf ("\nDumping ROM to %s.gb... ", gametitle);
			strncpy(filename, gametitle, 20);
			strcat(filename, ".gb");
			write_to_file(filename, "READROM\n", 32);
			printf ("\nFinished\n");
		}
		else if (userInput == '2') {    
			printf ("\nDumping RAM to %s.sav... ", gametitle);
			strncpy(filename, gametitle, 20);
			strcat(filename, ".sav");
			write_to_file(filename, "READRAM\n", 1);
			printf ("\nFinished\n");
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
				printf ("\nFinished\n");
			}
			else {
				printf ("\nAborted\n");
			}
		}
		else if (userInput == '4') {
			RS232_CloseComport(cport_nr);
			return(0);
		}
		else {  
			printf ("\nOption not recognised, please try again.\n");
		}
	}
	
	return(0);
}
