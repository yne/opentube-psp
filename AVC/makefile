TARGET = ../avc1.31637661
OBJS = vcodec.o
# mp4.o play.o utils.o gui.o jsfunctions.o
OBJS+= ../lib/bridge.o ../lib/mpegbase.o ../lib/sceMpeg.o

INCDIR = ../lib ../main ../mp4
CFLAGS = -G0 -Wall -O3 -std=c99
LDFLAGS =  -mno-crt0 -nostartfiles
ASFLAGS = $(CFLAGS)

BUILD_PRX = 1

USE_PSPSDK_LIBC = 1
#LIBS = -lpspaudio -lpspaudiocodec -lpspgu -lpsphttp -lpspssl -lpspjpeg
PSPSDK=$(shell psp-config --pspsdk-path)
include $(PSPSDK)/lib/build.mak

all:
	rm -f *.o $(TARGET).elf PARAM.SFO
#	psp-packer.exe $(TARGET).prx
