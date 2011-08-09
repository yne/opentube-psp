TARGET = ../gui
OBJS = jpg.o libccc.o intrafont.o graphic.o
#OBJS+= font.o sceLibFont_HV.o
INCDIR = ../lib ../core
CFLAGS = -G0 -Wall -Os -std=c99
LDFLAGS = -mno-crt0 -nostartfiles

BUILD_PRX = 1

PRX_EXPORTS = ../lib/export.exp


USE_PSPSDK_LIBC = 1
LIBS = -lpspgu -lpspjpeg -lpspumd
PSPSDK=$(shell psp-config --pspsdk-path)
include $(PSPSDK)/lib/build.mak

all:
	rm *.o $(TARGET).elf
	psp-packer.exe $(TARGET).prx

