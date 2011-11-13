TARGET = ../mod
OBJS = main.o mixer.o effects.o modplay.o envelope.o modplay_core.o mod.o s3m.o xm.o

BUILD_PRX = 1

CFLAGS  = -std=c99 -O3 -Os -Wall
ASFLAGS = $(CFLAGS)
LDFLAGS = -mno-crt0 -nostartfiles
LIBS    = -lpspaudio -nodefaultlibs -nostdlib

PRX_EXPORTS = export.exp
$(shell psp-build-exports -s ${PRX_EXPORTS})

USE_PSPSDK_LIBC = 1
USE_PSPSDK_LIBS = 1


PSPSDK=$(shell psp-config --pspsdk-path)
include $(PSPSDK)/lib/build.mak

all:
	rm *.o $(TARGET).elf
	psp-packer $(TARGET).prx
