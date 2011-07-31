TARGET = ../gui
OBJS = graphic.o jpg.o font.o sceLibFont_HV.o
#OBJS+= lib/bridge.o lib/mpegbase.o lib/sceMpeg.o
INCDIR = ../lib ../main
CFLAGS = -G0 -Wall -O3 -std=c99
LDFLAGS = -mno-crt0 -nostartfiles

BUILD_PRX = 1

PRX_EXPORTS = ../lib/export.exp


USE_PSPSDK_LIBC = 1
LIBS = -lpspgu -lpspjpeg
PSPSDK=$(shell psp-config --pspsdk-path)
include $(PSPSDK)/lib/build.mak

all:
	rm *.o $(TARGET).elf
#	psp-packer.exe $(TARGET).prx

