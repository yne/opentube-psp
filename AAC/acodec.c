#include <pspkernel.h>
#include <pspaudio.h>
#include <psputility.h>
#include <stdio.h>
#include "acodec.h"
#include "demuxer.h"
#include "main.h"

PSP_MODULE_INFO("OpenTube.codec.aac",0,0,0);
OpenTube*ot;
Acodec a;

char*load(){
	puts("aload");
	a.au.freq=ot->dmx->Ahz;
	a.chan=sceAudioChReserve(PSP_AUDIO_NEXT_CHANNEL,PSP_AUDIO_SAMPLE_ALIGN(1024),PSP_AUDIO_FORMAT_STEREO);
	if(sceAudiocodecCheckNeedMem(&a.au, PSP_CODEC_AAC)<0)return "chekmem";
	if(sceAudiocodecGetEDRAM(&a.au, PSP_CODEC_AAC)<0)return "getEDRAM";
	if(sceAudiocodecInit(&a.au, PSP_CODEC_AAC))return "cdcInit";
	return NULL;
}
unsigned out[2][1024*2] __attribute__((aligned(64)));
char*play(){
	puts("aplay");
	for(int s=0;s<ot->dmx->f->AstszLen;s++){
		a.au.src=ot->dmx->getASample(s);
		a.au.srcLen=ot->dmx->f->Astsz[s];
		a.au.dst=out[s%2];
		sceAudiocodecDecode(&a.au,PSP_CODEC_AAC);
		sceAudioOutputBlocking(a.chan, PSP_AUDIO_VOLUME_MAX,out[s%2]);
//		if(s>60)return NULL;
	}
	return NULL;
}
char*seek(int t,int o){
	puts("aplay");
	return NULL;
}
int unload(SceSize args,void*argp){
	do sceKernelDelayThread(1000);
	while(sceKernelSelfStopUnloadModule(1,0,NULL)<0);
	return 0;
}
char*stop(){
	puts("astop");
	if(a.au.EDRAM)sceAudiocodecReleaseEDRAM(&a.au);
	if(a.chan)sceAudioChRelease(a.chan);
	ot->dmx->Aload=NULL;
	ot->dmx->Aplay=NULL;
	ot->dmx->Aseek=NULL;
	ot->dmx->Astop=NULL;
	sceKernelStartThread(sceKernelCreateThread("arakiri",unload,0x11,0x1000,0,0),0,NULL);//to be unable to return
	return NULL;
}
int module_stop(int args,void*argp){
	return 0;
}
int module_start(int args,void*argp){
	puts("AAC codec loaded");
	if(args!=4)return 0;//return if no context
	ot=(OpenTube*)((u32*)argp)[0];
	ot->dmx->Aload=load;
	ot->dmx->Aplay=play;
	ot->dmx->Aseek=seek;
	ot->dmx->Astop=stop;
	return 0;
}