#include <pspkernel.h>
#include <pspaudio.h>
#include <pspctrl.h>
#include <stdio.h>
#include "acodec.h"
#include "core.h"

PSP_MODULE_INFO("OpenTube.codec.aac",0,0,0);
OpenTube*ot;
Acodec a={load,play,seek,stop};

char*load(){
	Alert("aload\n");
	a.au.freq=ot->dmx->hz;
	a.chan=sceAudioChReserve(PSP_AUDIO_NEXT_CHANNEL,PSP_AUDIO_SAMPLE_ALIGN(1024),PSP_AUDIO_FORMAT_STEREO);
	if(sceAudiocodecCheckNeedMem(&a.au, PSP_CODEC_AAC)<0)return "chekmem";
	if(sceAudiocodecGetEDRAM(&a.au, PSP_CODEC_AAC)<0)return "getEDRAM";
	if(sceAudiocodecInit(&a.au, PSP_CODEC_AAC))return "cdcInit";
	return NULL;
}
unsigned out[2][1024*2] __attribute__((aligned(64)));
char*play(){
	Alert("aplay\n");
	for(int s=0;s<ot->dmx->Alen;s++){
		Alert("audio ...\r");
		if(/*ot->sys->pad!=ot->sys->_pad&&*/ot->sys->pad&PSP_CTRL_CIRCLE)break;
		a.au.src=ot->dmx->getASample(s,&a.au.srcLen);
		a.au.dst=out[s%2];
		if(sceAudiocodecDecode(&a.au,PSP_CODEC_AAC))$("a!\n");
		sceAudioOutputBlocking(a.chan, PSP_AUDIO_VOLUME_MAX,out[s%2]);
//		if(s>60)return NULL;
	}
	Alert("aplay:ok\n");
	return NULL;
}
char*seek(int t,int o){
	Alert("aseek\n");
	return NULL;
}
int unload(SceSize args,void*argp){
	do sceKernelDelayThread(1000);
	while(sceKernelSelfStopUnloadModule(1,0,NULL)<0);
	return 0;
}
char*stop(){
	Alert("astop");
	if(a.au.EDRAM)sceAudiocodecReleaseEDRAM(&a.au);
	if(a.chan)sceAudioChRelease(a.chan);
	ot->dmx->a=NULL;
	sceKernelStartThread(sceKernelCreateThread("arakiri",unload,0x11,0x10000,0,0),0,NULL);//to be unable to return
	return 0;
}
int module_stop(int args,void*argp){
	return 0;
}
int module_start(int args,void*argp){
	if((ot=otGetCtx())<0)return 1;
	Alert("AAC codec loaded\n");
	ot->dmx->a=&a;
	return 0;
}