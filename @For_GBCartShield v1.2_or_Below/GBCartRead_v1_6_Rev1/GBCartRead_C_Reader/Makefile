# Command-line client
CMDLINE = GBCartRead_v1_6_C_Reader.exe

# By default, build the firmware and command-line client
all: $(CMDLINE)

# One-liner to compile the command-line client
$(CMDLINE): GBCartRead_v1_6_C_Reader.c
	gcc -O -Wall GBCartRead_v1_6_C_Reader.c rs232.c -o GBCartRead_v1_6_C_Reader.exe

# Housekeeping if you want it
clean:
	$(RM) *.exe
