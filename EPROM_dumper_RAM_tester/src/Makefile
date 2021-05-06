# MPLAB IDE generated this makefile for use with GNU make.
# Project: eprom_dumper_ram_tester.mcp
# Date: Thu May 06 23:58:20 2021

AS = MPASMWIN.exe
CC = mcc18.exe
LD = mplink.exe
AR = mplib.exe
RM = rm

eprom_dumper_ram_tester.cof : utils.o crc16.o clearblock.o scmpp2r_b.o serial.o print.o pico_libc.o eprom_dumper_ram_tester.o init_io.o
	$(LD) /p18F46K22 /l"g:\MCC18\lib" "eprom_dumper_ram_tester.lkr" "utils.o" "crc16.o" "clearblock.o" "scmpp2r_b.o" "serial.o" "print.o" "pico_libc.o" "eprom_dumper_ram_tester.o" "init_io.o" /u_CRUNTIME /u_DEBUG /z__MPLAB_BUILD=1 /z__MPLAB_DEBUG=1 /m"eprom_dumper_ram_tester.map" /w /o"eprom_dumper_ram_tester.cof"

utils.o : utils.c utils.h utils.c hardware.h C:/Program\ Files/Microchip/mplabc18/v3.46/h/p18f46k22.h
	$(CC) -p=18F46K22 /i".\pico_libc" "utils.c" -fo="utils.o" -D__DEBUG -O-

crc16.o : crc16.asm C:/Program\ Files/Microchip/mplabc18/v3.46/mpasm/P18CXXX.INC C:/Program\ Files/Microchip/mplabc18/v3.46/mpasm/p18F46K22.INC C18_MACRO.INC ASM_VAR.INC
	"C:\Program Files\Microchip\mplabc18\v3.46\mpasm\mpasmwin.exe" /q /p18F46K22 "crc16.asm" /l"crc16.lst" /e"crc16.err" /o"crc16.o" /d__DEBUG=1

clearblock.o : clearblock.asm C:/Program\ Files/Microchip/mplabc18/v3.46/mpasm/P18CXXX.INC C:/Program\ Files/Microchip/mplabc18/v3.46/mpasm/p18F46K22.INC C18_MACRO.INC
	"C:\Program Files\Microchip\mplabc18\v3.46\mpasm\mpasmwin.exe" /q /p18F46K22 "clearblock.asm" /l"clearblock.lst" /e"clearblock.err" /o"clearblock.o" /d__DEBUG=1

scmpp2r_b.o : scmpp2r_b.asm C:/Program\ Files/Microchip/mplabc18/v3.46/mpasm/P18CXXX.INC C:/Program\ Files/Microchip/mplabc18/v3.46/mpasm/p18F46K22.INC C:/Program\ Files/Microchip/mplabc18/v3.46/mpasm/P18MACRO.INC
	"C:\Program Files\Microchip\mplabc18\v3.46\mpasm\mpasmwin.exe" /q /p18F46K22 "scmpp2r_b.asm" /l"scmpp2r_b.lst" /e"scmpp2r_b.err" /o"scmpp2r_b.o" /d__DEBUG=1

serial.o : serial.c serial.h serial.c C:/Program\ Files/Microchip/mplabc18/v3.46/h/p18lf46k22.h
	$(CC) -p=18F46K22 /i".\pico_libc" "serial.c" -fo="serial.o" -D__DEBUG -O-

print.o : print.c pico_libc/pico_libc.h print.h print.c C:/Program\ Files/Microchip/mplabc18/v3.46/h/p18lf46k22.h
	$(CC) -p=18F46K22 /i".\pico_libc" "print.c" -fo="print.o" -D__DEBUG -O-

pico_libc.o : pico_libc/pico_libc.c
	$(CC) -p=18F46K22 /i".\pico_libc" "pico_libc\pico_libc.c" -fo="pico_libc.o" -D__DEBUG -O-

eprom_dumper_ram_tester.o : eprom_dumper_ram_tester.c functionasm.h eprom_dumper_ram_tester.h utils.h serial.h pico_libc/pico_libc.h print.h eprom_dumper_ram_tester.c C:/Program\ Files/Microchip/mplabc18/v3.46/h/p18f46k22.h hardware.h init_io.h
	$(CC) -p=18F46K22 /i".\pico_libc" "eprom_dumper_ram_tester.c" -fo="eprom_dumper_ram_tester.o" -D__DEBUG -O-

init_io.o : init_io.c C:/Program\ Files/Microchip/mplabc18/v3.46/h/p18f46k22.h init_io.h
	$(CC) -p=18F46K22 /i".\pico_libc" "init_io.c" -fo="init_io.o" -D__DEBUG -O-

clean : 
	$(RM) "utils.o" "crc16.o" "crc16.err" "crc16.lst" "clearblock.o" "clearblock.err" "clearblock.lst" "scmpp2r_b.o" "scmpp2r_b.err" "scmpp2r_b.lst" "serial.o" "print.o" "pico_libc.o" "eprom_dumper_ram_tester.o" "init_io.o" "eprom_dumper_ram_tester.cof" "eprom_dumper_ram_tester.hex" "eprom_dumper_ram_tester.map"

