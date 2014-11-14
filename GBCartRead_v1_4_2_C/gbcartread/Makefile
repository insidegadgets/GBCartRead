# Command-line client
CMDLINE = gbcartread.exe

# By default, build the firmware and command-line client
all: $(CMDLINE)

# One-liner to compile the command-line client
$(CMDLINE): gbcartread.c
	gcc -O -Wall gbcartread.c rs232.c -o gbcartread.exe

# Housekeeping if you want it
clean:
	$(RM) *.exe
