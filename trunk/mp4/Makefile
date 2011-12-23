TARGET = ../ftyp.0000001C.00000020.00000018.mp4
OBJS = demuxer.o
OBJS+= ../lib/OpenTubeCtx.o
INCDIR = ../lib ../core
CFLAGS = -G0 -Wall -O3 -std=c99
LDFLAGS = -mno-crt0 -nostartfiles

BUILD_PRX = 1

PRX_EXPORTS = ../lib/export.exp


USE_PSPSDK_LIBC = 1

PSPSDK=$(shell psp-config --pspsdk-path)
include $(PSPSDK)/lib/build.mak

all:
	rm -f *.o $(TARGET) $(TARGET).elf PARAM.SFO
	psp-packer.exe $(TARGET).prx
	mv $(TARGET).prx $(TARGET)
