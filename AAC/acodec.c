#include <pspkernel.h>
#include <stdio.h>
#include "acodec.h"
#include "demuxer.h"
#include "main.h"

PSP_MODULE_INFO("OpenTube.codec.aac",0,0,0);
OpenTube*ot;

char*load(){
	puts("aload");
	/*
	me.aucd.freq=f.hz;
	me.chan=sceAudioChReserve(PSP_AUDIO_NEXT_CHANNEL,PSP_AUDIO_SAMPLE_ALIGN(1024),PSP_AUDIO_FORMAT_STEREO);
	if(sceAudiocodecCheckNeedMem(&me.aucd, PSP_CODEC_AAC)<0)puts("chekmem");
	if(sceAudiocodecGetEDRAM(&me.aucd, PSP_CODEC_AAC)<0)puts("getEDRAM");
	if(sceAudiocodecInit(&me.aucd, PSP_CODEC_AAC)<0)puts("ACodIni");
	*/
	return NULL;
}
char*play(){
	puts("aplay");
/*
	unsigned out[2][1024*2] __attribute__((aligned(64)));
	for(int s=0;(s<f.AstszLen)&&(!(pad()));s++){
		me.aucd.src=getASample(s);
		me.aucd.srcLen=f.Astsz[s];
		me.aucd.dst=out[s%2];
		sceAudiocodecDecode(&me.aucd,PSP_CODEC_AAC);
		sceAudioOutputBlocking(me.chan, PSP_AUDIO_VOLUME_MAX,out[s%2]);
	}
*/
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
//	if(me.aucd.EDRAM)sceAudiocodecReleaseEDRAM(&me.aucd);
//	if(me.chan)sceAudioChRelease(me.chan);
	ot->dmx->Aload=NULL;
	ot->dmx->Aplay=NULL;
	ot->dmx->Aseek=NULL;
	ot->dmx->Astop=NULL;
	sceKernelStartThread(sceKernelCreateThread("arakiri",unload,0x11,0x10000,0,0),0,NULL);//to be unable to return
	return NULL;
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