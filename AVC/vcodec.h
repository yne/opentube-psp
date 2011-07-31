#ifndef VCODEC_H
#define VCODEC_H
typedef struct{
	int delay;
	long long next;
	u32 modmpg;
}Vcodec;

#include <pspdisplay.h>
#include <pspmpeg.h>
#include <audiocodec.h>
#include <mpegbase.h>
#include "demuxer.h"
#include "main.h"
#define PROFILE_BASELINE 0x42
#define PROFILE_MAIN 0x4D

#define W 0xFF0000FF
#define _ 0xFF000000
u32 vdecErr[]={
	W,W,_,_,_,W,_,_,_,W,W,_,_,W,_,_,W,W,_,_,_,W,_,_,_,_,_,_,_,W,_,_,W,W,_,_,W,W,_,_,_,W,_,_,W,W,_,_,
	W,_,W,_,W,W,W,_,W,_,_,_,W,_,W,_,W,_,W,_,W,W,W,_,_,_,_,_,W,W,W,_,W,_,W,_,W,_,W,_,W,_,W,_,W,_,W,_,
	W,_,W,_,W,_,_,_,W,_,_,_,W,_,W,_,W,_,W,_,W,_,_,_,_,_,_,_,W,_,_,_,W,W,_,_,W,W,_,_,W,_,W,_,W,W,_,_,
	W,W,_,_,_,W,W,_,_,W,W,_,_,W,_,_,W,W,_,_,_,W,W,_,_,_,_,_,_,W,W,_,W,_,W,_,W,_,W,_,_,W,_,_,W,_,W,_,
};
#undef W
#undef _

//MeCtx me;
int next;

//extern int loadNplay(SceSize args,void *argp);
//extern char* play();
extern void sceMpegGetAvcNalAu(SceMpeg*,Nal*,SceMpegAu*);
extern void sceMpegAvcDecodeDetail2(SceMpeg*,SceMpegAvcDetail2**);
extern int sceMpegBaseCscAvc(void*,unsigned,unsigned,Mp4AvcCscStruct*);
#endif