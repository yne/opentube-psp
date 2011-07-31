#include <pspkernel.h>
#include <psputility.h>
#include <pspaudio.h>
#include <pspctrl.h>
#include <audiocodec.h>
#include <string.h>
#include <stdio.h>
#include "vcodec.h"
#include "demuxer.h"
#include "main.h"

PSP_MODULE_INFO("OpenTube.codec.h264",0,0,0);
OpenTube*ot;
Vcodec v;
void* swapBuffer(){
	while((sceKernelGetSystemTimeWide())<v.next)
		sceDisplayWaitVblankStart();
	v.next+=v.delay;
//	return (void*)lcd.disp;//+lcd.vram
	if(++ot->lcd->curr%2){
		sceDisplaySetFrameBuf((void*)ot->lcd->draw,ot->lcd->size,ot->lcd->type,0);
		sceDisplaySetFrameBuf((void*)ot->lcd->disp,ot->lcd->size,ot->lcd->type,1);
		return (void*)ot->lcd->disp+ot->lcd->vram;
	}
	sceDisplaySetFrameBuf((void*)ot->lcd->disp,ot->lcd->size,ot->lcd->type,0);
	sceDisplaySetFrameBuf((void*)ot->lcd->draw,ot->lcd->size,ot->lcd->type,1);
	return (void*)ot->lcd->draw+ot->lcd->vram;
//	return (void*)0x44000000;
}
char* load(){
	Alert("vload\n");
	sceUtilityLoadAvModule(0);//u32 modavc=modload("flash0:/kd/avcodec.prx");
	if(	/*ot->dmx->f->profile==PROFILE_MAIN&&*/(ot->dmx->f->width>480||ot->dmx->f->height>272)){
		Alert("HQ mode\n");
		ot->me->mode=5;
		ot->lcd->size=768;
		ot->sys->sudo(SUDO_STARTME,ot->me->bootNid,1);
	}else if(ot->dmx->f->profile==PROFILE_MAIN||ot->dmx->f->profile==PROFILE_BASELINE){
		Alert("LD mode\n");
		ot->me->mode=4;
		ot->lcd->size=512;
		ot->sys->sudo(SUDO_STARTME,ot->me->bootNid,ot->dmx->f->profile==PROFILE_MAIN?3:4);
	}else return "badFmt";
	v.modmpg=ot->sys->modload("flash0:/kd/mpeg_vsh.prx");
	if(!ot->me->pool)return("noPool");
	if(sceMpegInit())return("MpegIni");
	if( (ot->me->buffLen=sceMpegQueryMemSize(ot->me->mode))<0)return("noMemSz");
	if(!(ot->me->buffLen&0xF))ot->me->buffLen=(ot->me->buffLen&0xFFFFFFF0)+16;
	if(!(ot->me->buff=Malloc(ot->me->buffLen)))return("noMpgBf");
	if(sceMpegCreate(&ot->me->mpeg,ot->me->buff,ot->me->buffLen,&ot->me->ring,512,ot->me->mode,(int)ot->me->pool))return("MpgCrea");//fail if ME not started
	if(!(ot->me->mpegAu=(SceMpegAu*)Malloc(64)))return("noMpgAu");
	if(sceMpegInitAu(&ot->me->mpeg,(ot->me->buffAu=ot->me->pool+0x10000),ot->me->mpegAu))return("MpgInit");
	return NULL;
}
void blitErr(){
	void* vram=swapBuffer();
//	memset(vram+4*0*ot->lcd->size,0,46*4);
	for(int y=0;y<4;y++)
		memcpy(vram+4*y*ot->lcd->size,vdecErr+48*y,48*4);
//	memset(vram+4*5*ot->lcd->size,0,46*4);
}
char* play(){
	Alert("vplay\n");
	v.delay=1000000000/ot->dmx->f->fps;
	v.next=sceKernelGetSystemTimeWide();
	for(int s=0;(s<ot->dmx->f->VstszLen);s++){
		Nal nal={ot->dmx->f->sps,ot->dmx->f->spsLen,ot->dmx->f->pps,ot->dmx->f->ppsLen,ot->dmx->f->nalPreLen,ot->dmx->getVSample(s),ot->dmx->f->Vstsz[s],s?0:3};
		sceMpegGetAvcNalAu(&ot->me->mpeg,&nal,ot->me->mpegAu);
		SceMpegAvcMode mode={-1,ot->lcd->type};
		sceMpegAvcDecodeMode(&ot->me->mpeg,&mode);
		if(sceMpegAvcDecode(&ot->me->mpeg,ot->me->mpegAu,ot->lcd->size,0,&ot->me->pics)<0)/*return*/blitErr();//("VdecErr");
		sceMpegAvcDecodeDetail2(&ot->me->mpeg,&ot->me->d);
		Mp4AvcCscStruct csc={(ot->me->d->info->height+15)>>4,(ot->me->d->info->width+15)>>4,0,0,ot->me->d->yuv->b0,ot->me->d->yuv->b1,ot->me->d->yuv->b2,ot->me->d->yuv->b3,ot->me->d->yuv->b4,ot->me->d->yuv->b5,ot->me->d->yuv->b6,ot->me->d->yuv->b7};
		for(int i=0;i<ot->me->pics;i++)sceMpegBaseCscAvc(swapBuffer(),0,ot->lcd->size, &csc);//color space conversion (TODO:display each buffer)
//		if(s>60)return NULL;
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
	puts("vstop");
	sceMpegDelete(&ot->me->mpeg);
	sceMpegFinish();
	Free(ot->me->buff);
	Free(ot->me->mpegAu);
	ot->dmx->Vload=NULL;
	ot->dmx->Vplay=NULL;
	ot->dmx->Vseek=NULL;
	ot->dmx->Vstop=NULL;
	ot->sys->modstun(v.modmpg);
	sceKernelStartThread(sceKernelCreateThread("arakiri",unload,0x11,0x1000,0,0),0,NULL);//to be unable to return
	return NULL;
}
int module_stop(int args,void*argp){
	return 0;
}
int module_start(int args,void*argp){
	puts("AVC codec loaded");
	if(args!=4)return 0;//return if no context
	ot=(OpenTube*)((u32*)argp)[0];
	ot->dmx->Vload=load;
	ot->dmx->Vplay=play;
	ot->dmx->Vseek=seek;
	ot->dmx->Vstop=stop;
//	deflateImg();
	return 0;
}