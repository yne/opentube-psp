TARGET = ../mp4a.6134706D
OBJS = acodec.o

INCDIR = ../lib ../core ../mp4
CFLAGS = -G0 -Wall -O3 -std=c99
LDFLAGS = -mno-crt0 -nostartfiles

BUILD_PRX = 1

PRX_EXPORTS = ../lib/export.exp


USE_PSPSDK_LIBC = 1
LIBS = -lpspaudio -lpspaudiocodec
PSPSDK=$(shell psp-config --pspsdk-path)
include $(PSPSDK)/lib/build.mak

all:
	rm -f *.o $(TARGET).elf PARAM.SFO
#	psp-packer.exe $(TARGET).prx
