TARGET = ../mp4.0000001C.m4a.00000020
OBJS = demuxer.o
#OBJS+= lib/bridge.o lib/mpegbase.o lib/sceMpeg.o

INCDIR = ../lib ../main
CFLAGS = -G0 -Wall -O3 -std=c99
LDFLAGS =  -mno-crt0 -nostartfiles
ASFLAGS = $(CFLAGS)

BUILD_PRX = 1

USE_PSPSDK_LIBC = 1

PSPSDK=$(shell psp-config --pspsdk-path)
include $(PSPSDK)/lib/build.mak

all:
	rm -f *.o $(TARGET).elf
#	psp-packer.exe $(TARGET).prx
