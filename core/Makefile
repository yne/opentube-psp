TARGET = ../main
OBJS = core.o io.o bridge.o
# aac.o avc.o mp4.o gui.o
#OBJS+=  ../lib/mpegbase.o ../lib/sceMpeg.o ../lib/sceVideocodec.o
INCDIR = ../lib ../core
CFLAGS = -G0 -Wall -O3 -std=c99
LDFLAGS = -mno-crt0 -nostartfiles
PRX_EXPORTS = ../lib/core.exp
$(shell psp-build-exports -s ${PRX_EXPORTS})
BUILD_PRX = 1
#EXTRA_TARGETS = EBOOT.PBP
#PSP_EBOOT_TITLE = OpenTube
#PSP_EBOOT_ICON = ../res/ICON0.PNG
#ENCRYPT = 1
USE_PSPSDK_LIBC = 1
LIBS = -lpspgu -lpsphttp -lpspssl -lpspaudio -lpspaudiocodec
PSPSDK=$(shell psp-config --pspsdk-path)
include $(PSPSDK)/lib/build.mak

all:
	rm -f *.o $(TARGET).elf ../lib/OpenTube.S PARAM.SFO
	mv OpenTube.S ../lib
#	pspsh ../lib/pspsh.cp
#	psp-packer.exe $(TARGET).prx
	