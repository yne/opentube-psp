TARGET = ../modplay.mod.s3m.xm
OBJS = demuxer.o mixer.o modplay.o effects.o envelope.o modplay_core.o mod.o s3m.o xm.o
OBJS+= ../lib/OpenTubeCtx.o
INCDIR = ../lib ../core
CFLAGS = -G0 -Wall -Os -std=c99
LDFLAGS = -mno-crt0 -nostartfiles

BUILD_PRX = 1

PRX_EXPORTS = ../lib/export.exp
LIBS    = -lpspaudio

USE_PSPSDK_LIBC = 1

PSPSDK=$(shell psp-config --pspsdk-path)
include $(PSPSDK)/lib/build.mak

all:
	rm -f *.o $(TARGET) $(TARGET).elf PARAM.SFO
	psp-packer.exe $(TARGET).prx
	mv $(TARGET).prx $(TARGET)
