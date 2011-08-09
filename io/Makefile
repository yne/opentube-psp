TARGET = ../io
OBJS = io.o

INCDIR = ../lib ../core
CFLAGS = -G0 -Wall -Os -std=c99
LDFLAGS = -mno-crt0 -nostartfiles

BUILD_PRX = 1

PRX_EXPORTS = ../lib/export.exp


USE_PSPSDK_LIBC = 1
LIBS = -lpsphttp -lpspssl -lpspgu
PSPSDK=$(shell psp-config --pspsdk-path)
include $(PSPSDK)/lib/build.mak

all:
	rm -f *.o $(TARGET).elf PARAM.SFO
#	psp-packer.exe $(TARGET).prx
