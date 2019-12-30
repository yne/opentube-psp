#include <pspkernel.h>
#include <psputility.h>
#include <pspaudio.h>
#include <pspctrl.h>
#include <pspdisplay.h>
#include <string.h>
#include <stdio.h>
#include "core.h"

#define PROFILE_BASELINE 0x42
#define PROFILE_MAIN 0x4D
#define Mpeg &ot->me->mpeg
#define ME ot->me->

#define Width ot->dmx->width
#define Height ot->dmx->height

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
Vcodec v;
extern int sceMpegGetAvcNalAu(SceMpeg*,Nal*,SceMpegAu*);
extern int sceMpegAvcDecodeDetail2(SceMpeg*,SceMpegAvcDetail2**);
extern int sceMpegBaseCscAvc(void*,unsigned,unsigned,Mp4AvcCscStruct*);
int delay,modmpg;long long next;
Display _lcd;//backup for after-playback restor
int dispLst[]={0x44000000,0x44044000,0x44088000,0x440CC000};
u16*curr,*disp;int offset;
void* swapBuffer(int i){
	while((sceKernelGetSystemTimeWide())<next)
		if(ot->gui)Draw(4);else sceDisplayWaitVblankStart();
	next+=delay;
	disp=dispLst[i]-(offset<0?offset:0);
	sceDisplaySetFrameBuf(disp,ot->lcd->size,ot->lcd->type,1);
	sceDisplaySetFrameBuf(disp,ot->lcd->size,ot->lcd->type,0);
	return (void*)(curr=dispLst[i]+(offset>0?offset:0));
}
char* avc1load(){
	Alert("vload\n");
//	_lcd=*ot->lcd;//save lcd context
//	ot->lcd->size=256;//multipl 2^6 512/576/640/704/768
	ot->me->mode=(Width>480||Height>272)?5:4;
	if(Width>272)ot->lcd->type=0;//PSP_DISPLAY_PIXEL_FORMAT_565;
	if(Width>480)return "tooBig";//still playable, but too slow.
	offset=(480-Width + (272-Height)*512);
	if(!ot->me->pool)return("noPool");
//	if((modmpg=ot->sys->modload("flash0:/kd/mpeg_vsh.prx"))<0)
		sceUtilityLoadAvModule(PSP_AV_MODULE_MPEGBASE);
	if(sceMpegInit())return("MpegIni");
	if( (ot->me->buffLen=(sceMpegQueryMemSize(ot->me->mode)&0xFFFFFFF0)+16)<0)return("noMemSz");
	if(!(ot->me->buff=Memalign(64,ot->me->buffLen)))return("noMpgBf");
	if(sceMpeg_75E21135(Mpeg,ot->me->buff,ot->me->buffLen,&ot->me->ring,/*cscInit*/512,ot->me->mode,(int)ot->me->pool))return("MpgCrea");//fail if ME not started
	if(!(ot->me->mpegAu=(SceMpegAu*)Memalign(64,64)))return("noMpgAu");
	if(sceMpegInitAu(Mpeg,(ot->me->buffAu=ot->me->pool+0x10000),ot->me->mpegAu))return("MpgInit");
	return NULL;
}
void blitErr(){
	swapBuffer(0);
	$("decErr\n");
	for(int y=0;y<4;y++)
		memcpy(curr+4*y*ot->lcd->size,vdecErr+48*y,48*4);
}
void drawBar(){
	if(curr){//draw overlay stuff
		int total=1+(Str.size?Str.size:Str.end)/480;
		for(int i=Str.start/total;i<Str.end/total;i++)((u16*)disp+512*271)[i]=0x00FF;
		((u16*)disp+512*271)[Str.curr/total]=0xFFFF;
	}
}
void decode(){// OFW
	if(sceMpegAvcDecode(Mpeg,ot->me->mpegAu,ot->lcd->size,dispLst,&ot->me->pics)<0){blitErr();return;}//("VdecErr");
	for(int i=0;i<ot->me->pics;i++){
		swapBuffer(i);
		drawBar();
	}
}
void decodeVsh(){// CFW
	if(sceMpegAvcDecode(Mpeg,ot->me->mpegAu,ot->lcd->size,0,&ot->me->pics)<0){blitErr();return;}//("VdecErr");
	sceMpegAvcDecodeDetail2(Mpeg,&ot->me->d);//  ot->me->d = *Mpeg[414]<7?*Mpeg[424]+0x2C:*Mpeg[419];
	for(int i=0;i<ot->me->pics;i++){
		Mp4AvcYuvStruct*yuv=ot->me->d->yuv+i;
		Mp4AvcCscStruct csc={(ot->me->d->info->height+15)>>4,(ot->me->d->info->width+15)>>4,0,0,yuv->b0,yuv->b1,yuv->b2,yuv->b3,yuv->b4,yuv->b5,yuv->b6,yuv->b7};
		sceMpegBaseCscAvc(swapBuffer(0),0,ot->lcd->size, &csc);
		drawBar();
	}
}
char* avc1play(){
	Alert("vplay\n");
	delay=1000000000/ot->dmx->fps;
	next=sceKernelGetSystemTimeWide();
	for(u32 s=0;(s<ot->dmx->Vlen);s++){
		if(ot->sys->pad&PSP_CTRL_CIRCLE)break;
		ot->me->nal.nal=ot->dmx->getVSample(s,&ot->me->nal.nalLen);
		ot->me->nal.mode=s?0:3;
		SceMpegAvcMode mode={-1,ot->lcd->type};
		sceMpegAvcDecodeMode(&ot->me->mpeg,&mode);
		sceMpegGetAvcNalAu(Mpeg,&ot->me->nal,ot->me->mpegAu);
		if(modmpg>0)decodeVsh();else decode();
		drawBar();
	}
	return NULL;
}
char* avc1seek(int t,int o){
	Alert("vseek\n");
	return NULL;
}
char* avc1stop(){
	Alert("vstop\n");	
	sceMpegDelete(Mpeg);
	sceMpegFinish();
	Free(ot->me->buff);
	Free(ot->me->mpegAu);
	sceUtilityUnloadAvModule(PSP_AV_MODULE_MPEGBASE);
	if(modmpg>0)ot->sys->modstun(modmpg);
//	*ot->lcd=_lcd;
	ot->dmx->v=NULL;
	return NULL;
}
char*avc1Init(){
	Alert("AVC codec loaded\n");
	Vcodec _v={avc1load,avc1play,avc1seek,avc1stop};v=_v;
	ot->dmx->v=&v;
	return 0;
}