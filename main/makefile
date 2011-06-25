TARGET = ../main
OBJS = main.o
# mp4.o play.o utils.o gui.o jsfunctions.o
#OBJS+= lib/bridge.o lib/mpegbase.o lib/sceMpeg.o

INCDIR = ../lib ../main
CFLAGS = -G0 -Wall -O3 -std=c99
LDFLAGS =  -mno-crt0 -nostartfiles

BUILD_PRX = 1

#EXTRA_TARGETS = EBOOT.PBP
PSP_EBOOT_TITLE = OpenTube
PSP_EBOOT_ICON = ICON0.PNG

USE_PSPSDK_LIBC = 1
#LIBS = -lpspaudio -lpspaudiocodec -lpspgu -lpsphttp -lpspssl -lpspjpeg
PSPSDK=$(shell psp-config --pspsdk-path)
include $(PSPSDK)/lib/build.mak

all:
	rm -f *.o $(TARGET).elf PARAM.SFO
#	psp-packer.exe $(TARGET).prx
