#include <pspkernel.h>
#include <pspaudio.h>
#include <pspctrl.h>
//#include <stdio.h>
#include "modplay.h"
#include "core.h"

PSP_MODULE_INFO("OpenTube.demux.mod",0,0,0);

unsigned out[2][4096*2] __attribute__((aligned(64)));

//int playerStart(char *path,pMalloc extern_malloc,pFree extern_free,	int(*callback)(int)){
int ret,ch;MODFILE mod;
char*onLoad(char* file){
	Alert("onLoad\n");
	if((ch=sceAudioChReserve(PSP_AUDIO_NEXT_CHANNEL,PSP_AUDIO_SAMPLE_ALIGN(768),PSP_AUDIO_FORMAT_STEREO))<0)return "ChanErr";
	MODFILE_Init(&mod);
	if((ret=MODFILE_Load(file, &mod))<0)return sceAudioChRelease(ch),"LoadErr";
	mod.musicvolume = 255;
	mod.sfxvolume = 255;
	mod.callback = NULL;//callback;
	return 0;
}
char*onPlay(){
	MODFILE_Start(&mod);
	MODFILE_SetFormat(&mod, 44100, 2,16,1/*unsigned*/);
	sceCtrlSetSamplingCycle(0);
	sceCtrlSetSamplingMode(1);
	for(int i=0;;i++){
		SceCtrlData pad;
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
char*onSeek(int w,int o){
	Alert("seek...\n");
	return 0;
}
int  unload(SceSize args,void*argp){
	do sceKernelDelayThread(1000);
	while(sceKernelSelfStopUnloadModule(1,0,NULL)<0);
	return 0;
}
char*onStop(){
	Alert("unloading demuxer\n");
	MODFILE_Stop(&mod);
	MODFILE_Free(&mod);
	sceAudioChRelease(ch);
	ot->dmx=NULL;
	sceKernelStartThread(sceKernelCreateThread("arakiri",unload,0x11,0x10000,0,0),0,NULL);//to be able to return
	return NULL;
}
Demuxer dmx={onLoad,onPlay,onSeek,onStop,NULL,NULL};
int  module_start(int args,void*argp){
	if((ot=otGetCtx())<0)return 1;
	Alert("mod demux loaded\n");
	ot->dmx=&dmx;
	return 0;
}
int module_stop(){
	return 0;
}