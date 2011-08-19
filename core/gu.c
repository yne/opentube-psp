#include <pspkernel.h>
#include <pspgu.h>

static unsigned int __attribute__((aligned(16))) list[262144];

void sceGuStartList(int mode){
	return sceGuStart(mode,list);
}
