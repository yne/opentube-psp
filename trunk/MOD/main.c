#include <pspkernel.h>
#include <pspaudio.h>
#include <pspctrl.h>
#include "modplay.h"
#include "stdio.h"

PSP_MODULE_INFO("modPlayer",0,0,0);

unsigned out[2][4096*2] __attribute__((aligned(64)));

int playerStart(char *path,pMalloc extern_malloc,pFree extern_free,	int(*callback)(int)){
	int ret,ch;MODFILE mod;
	if(!extern_malloc || !extern_free)return -1;
	if((ch=sceAudioChReserve(PSP_AUDIO_NEXT_CHANNEL,PSP_AUDIO_SAMPLE_ALIGN(768),PSP_AUDIO_FORMAT_STEREO))<0)return ch;
	myMalloc=extern_malloc;
	myFree	=extern_free;
	MODFILE_Init(&mod);
	if((ret=MODFILE_Load(path, &mod))<0)return ret;
	mod.musicvolume = 255;
	mod.sfxvolume = 255;
	mod.callback = NULL;//callback;
	MODFILE_Start(&mod);
	MODFILE_SetFormat(&mod, 44100, 2,16,1/*unsigned*/);
	SceCtrlData pad;
	sceCtrlSetSamplingCycle(0);
	sceCtrlSetSamplingMode(1);
	for(int i=0;;i++){
		sceCtrlReadBufferPositive(&pad,1);
		if(pad.Buttons&PSP_CTRL_START)break;
		mod.mixingbuf =(void*)out[i%2];
		mod.mixingbuflen = 768*2*2;
		MODFILE_Player(&mod);
		sceAudioOutputBlocking(ch, PSP_AUDIO_VOLUME_MAX,out[i%2]);
	}
	MODFILE_Stop(&mod);
	MODFILE_Free(&mod);
	sceAudioChRelease(ch);
	return 0;
}
int module_start(int args,char*argp){
	return 0;
}
int module_stop(){
	return 0;
}