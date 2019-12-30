#include <pspkernel.h>
#include <pspaudio.h>
#include <pspctrl.h>
#include <stdio.h>
#include "core.h"
Acodec a;
char*mp4aload(){
	Alert("aload\n");
	a.au.freq=ot->dmx->hz;
	a.chan=sceAudioChReserve(PSP_AUDIO_NEXT_CHANNEL,PSP_AUDIO_SAMPLE_ALIGN(1024),PSP_AUDIO_FORMAT_STEREO);
	if(sceAudiocodecCheckNeedMem(&a.au, PSP_CODEC_AAC)<0)return "chekmem";
	if(sceAudiocodecGetEDRAM(&a.au, PSP_CODEC_AAC)<0)return "getEDRAM";
	if(sceAudiocodecInit(&a.au, PSP_CODEC_AAC))return "cdcInit";
	return NULL;
}
unsigned out[2][1024*2] __attribute__((aligned(64)));
char*mp4aplay(){
	Alert("aplay\n");
	for(int s=0;s<ot->dmx->Alen;s++){
//		Alert("audio ...\r");
		if(/*ot->sys->pad!=ot->sys->_pad&&*/ot->sys->pad&PSP_CTRL_CIRCLE)break;
		a.au.src=ot->dmx->getASample(s,&a.au.srcLen);
		a.au.dst=out[s%2];
		sceAudiocodecDecode(&a.au,PSP_CODEC_AAC);
		sceAudioOutputBlocking(a.chan, PSP_AUDIO_VOLUME_MAX,out[s%2]);
//		if(s>60)return NULL;
	}
	return NULL;
}
char*mp4aseek(int t,int o){
	Alert("aplay\n");
	return NULL;
}
char*mp4astop(){
	Alert("astop");
	if(a.au.EDRAM)sceAudiocodecReleaseEDRAM(&a.au);
	if(a.chan)sceAudioChRelease(a.chan);
	ot->dmx->a=NULL;
	return 0;
}
int mp4aInit(){
	if((ot=otGetCtx())<0)return 1;
	Alert("AAC codec loaded\n");
	Acodec _a={mp4aload,mp4aplay,mp4aseek,mp4astop};a=_a;
	ot->dmx->a=&a;
	return 0;
}