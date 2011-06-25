TARGET = ../kernel
OBJS = main.o

CFLAGS = -G0 -Wall -O2 -std=c99
LDFLAGS = -mno-crt0 -nostartfiles
ASFLAGS = $(CFLAGS)

#PRX_EXPORTS = exports.exp
#$(shell psp-build-exports -s ${PRX_EXPORTS})

LIBS = -lpspsystemctrl_kernel

BUILD_PRX = 1
USE_PSPSDK_LIBC = 1

PSPSDK=$(shell psp-config --pspsdk-path)
include $(PSPSDK)/lib/build.mak

all:
	rm *.o $(TARGET).elf
#	psp-packer.exe $(TARGET).prx
