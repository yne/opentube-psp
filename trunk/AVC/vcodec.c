#include <pspkernel.h>
#include <psputility.h>
#include <pspaudio.h>
#include <pspctrl.h>
#include <pspdisplay.h>
#include <string.h>
#include <stdio.h>
#include "vcodec.h"
#include "core.h"

#define PROFILE_BASELINE 0x42
#define PROFILE_MAIN 0x4D
#define Mpeg &ot->me->mpeg
#define ME ot->me->

PSP_MODULE_INFO("OpenTube.codec.h264",0,0,0);

#define W 0xFF
#define _ 0x00
u32 vdecErr[]={
	W,W,_,_,_,W,_,_,_,W,W,_,_,W,_,_,W,W,_,_,_,W,_,_,_,_,_,_,_,W,_,_,W,W,_,_,W,W,_,_,_,W,_,_,W,W,_,_,
	W,_,W,_,W,W,W,_,W,_,_,_,W,_,W,_,W,_,W,_,W,W,W,_,_,_,_,_,W,W,W,_,W,_,W,_,W,_,W,_,W,_,W,_,W,_,W,_,
	W,_,W,_,W,_,_,_,W,_,_,_,W,_,W,_,W,_,W,_,W,_,_,_,_,_,_,_,W,_,_,_,W,W,_,_,W,W,_,_,W,_,W,_,W,W,_,_,
	W,W,_,_,_,W,W,_,_,W,W,_,_,W,_,_,W,W,_,_,_,W,W,_,_,_,_,_,_,W,W,_,W,_,W,_,W,_,W,_,_,W,_,_,W,_,W,_,
};
#undef W
#undef _
Vcodec v={load,play,seek,stop};
extern int sceMpegGetAvcNalAu(SceMpeg*,Nal*,SceMpegAu*);
extern int sceMpegAvcDecodeDetail2(SceMpeg*,SceMpegAvcDetail2**);
extern int sceMpegBaseCscAvc(void*,unsigned,unsigned,Mp4AvcCscStruct*);
int delay,modmpg;long long next;
Display _lcd;//backup for after-playback restor

void* swapBuffer(){
	while((sceKernelGetSystemTimeWide())<next)
		if(ot->gui)Draw(4);else sceDisplayWaitVblankStart();
	next+=delay;
/*	if(ot->gui){
		Draw(8);
		return (void*)(ot->lcd->curr?ot->lcd->disp:ot->lcd->draw);//+lcd.vram
	}else{*/
		if(ot->lcd->curr^=1){
			sceDisplaySetFrameBuf((void*)ot->lcd->draw,ot->lcd->size,ot->lcd->type,0);
			sceDisplaySetFrameBuf((void*)ot->lcd->disp,ot->lcd->size,ot->lcd->type,1);
			return (void*)ot->lcd->disp+ot->lcd->vram;
		}
		sceDisplaySetFrameBuf((void*)ot->lcd->disp,ot->lcd->size,ot->lcd->type,0);
		sceDisplaySetFrameBuf((void*)ot->lcd->draw,ot->lcd->size,ot->lcd->type,1);
		return (void*)ot->lcd->draw+ot->lcd->vram;
//	}
/*
//	return (void*)0x44000000;
*/
}
char* load(){
	Alert("vload\n");
//	_lcd=*ot->lcd;//save lcd context
//	ot->lcd->size=256;//multipl 2^6 512/576/640/704/768
	ot->me->mode=(ot->dmx->width>480||ot->dmx->height>272)?5:4;
	if(ot->dmx->width>272)ot->lcd->type=0;//PSP_DISPLAY_PIXEL_FORMAT_565;
	if(ot->dmx->width>480)return "tooBig";//still playable, but too slow.
	modmpg=ot->sys->modload("flash0:/kd/mpeg_vsh.prx");
	if(!ot->me->pool)return("noPool");
	if(sceMpegInit())return("MpegIni");
	if( (ot->me->buffLen=(sceMpegQueryMemSize(ot->me->mode)&0xFFFFFFF0)+16)<0)return("noMemSz");
	if(!(ot->me->buff=Memalign(64,ot->me->buffLen)))return("noMpgBf");
	if(sceMpegCreate(Mpeg,ot->me->buff,ot->me->buffLen,&ot->me->ring,/*cscInit*/512,ot->me->mode,(int)ot->me->pool))return("MpgCrea");//fail if ME not started
	if(!(ot->me->mpegAu=(SceMpegAu*)Memalign(64,64)))return("noMpgAu");
	if(sceMpegInitAu(Mpeg,(ot->me->buffAu=ot->me->pool+0x10000),ot->me->mpegAu))return("MpgInit");
	return NULL;
}
void blitErr(){
	void* vram=swapBuffer();
	for(int y=0;y<4;y++)
		memcpy(vram+4*y*ot->lcd->size,vdecErr+48*y,48*4);
}
char* play(){
	Alert("vplay\n");
	delay=1000000000/ot->dmx->fps;
	next=sceKernelGetSystemTimeWide();
	for(u32 s=0;(s<ot->dmx->Vlen);s++){
		if(ot->sys->pad&PSP_CTRL_CIRCLE)break;
		ot->me->nal.nal=ot->dmx->getVSample(s,&ot->me->nal.nalLen);
		ot->me->nal.mode=s?0:3;
		sceMpegGetAvcNalAu(Mpeg,&ot->me->nal,ot->me->mpegAu);
		if(sceMpegAvcDecode(Mpeg,ot->me->mpegAu,ot->lcd->size,0,&ot->me->pics)<0){blitErr();continue;}//("VdecErr");
		sceMpegAvcDecodeDetail2(Mpeg,&ot->me->d);//  ot->me->d = *Mpeg[414]<7?*Mpeg[424]+0x2C:*Mpeg[419];
		SceMpegAvcMode mode={-1,ot->lcd->type};
		sceMpegAvcDecodeMode(&ot->me->mpeg,&mode);
		for(int i=0;i<ot->me->pics;i++){
			Mp4AvcYuvStruct*yuv=ot->me->d->yuv+i;
			Mp4AvcCscStruct csc={(ot->me->d->info->height+15)>>4,(ot->me->d->info->width+15)>>4,0,0,yuv->b0,yuv->b1,yuv->b2,yuv->b3,yuv->b4,yuv->b5,yuv->b6,yuv->b7};
			sceMpegBaseCscAvc(swapBuffer(),0,ot->lcd->size, &csc);
			if(ot->gui)Draw(2);//draw some overlay stuff
		}
	}
	return NULL;
}
char* seek(int t,int o){
	Alert("vseek\n");
	return NULL;
}
int unload(SceSize args,void*argp){
	do sceKernelDelayThread(1000);
	while(sceKernelSelfStopUnloadModule(1,0,NULL)<0);
	return 0;
}
char* stop(){
	Alert("vstop\n");	
	sceMpegDelete(Mpeg);
	sceMpegFinish();
	Free(ot->me->buff);
	Free(ot->me->mpegAu);
	ot->sys->modstun(modmpg);
//	*ot->lcd=_lcd;
	ot->dmx->v=NULL;
	sceKernelStartThread(sceKernelCreateThread("arakiri",unload,0x11,0x10000,0,0),0,NULL);//to be unable to return
	return NULL;
}
int module_stop(int args,void*argp){
	return 0;
}
int module_start(int args,void*argp){
	if((ot=otGetCtx())<0)return 1;
	Alert("AVC codec loaded\n");
	ot->dmx->v=&v;
	return 0;
}