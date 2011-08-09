TARGET = ../main
OBJS = main.o
#OBJS+= lib/bridge.o lib/mpegbase.o lib/sceMpeg.o
INCDIR = ../lib ../core
CFLAGS = -G0 -Wall -O3 -std=c99
LDFLAGS = -mno-crt0 -nostartfiles

BUILD_PRX = 1

EXTRA_TARGETS = EBOOT.PBP
PSP_EBOOT_TITLE = OpenTube
PSP_EBOOT_ICON = ../res/ICON0.PNG
USE_PSPSDK_LIBC = 1
#LIBS = -lpspaudio -lpspaudiocodec -lpspgu -lpsphttp -lpspssl -lpspjpeg
PSPSDK=$(shell psp-config --pspsdk-path)
include $(PSPSDK)/lib/build.mak

all:
	rm -f *.o $(TARGET).elf ../EBOOT.PBP PARAM.SFO
	mv EBOOT.PBP ..
#	pspsh ../lib/pspsh.cp
#	psp-packer.exe $(TARGET).prx
	