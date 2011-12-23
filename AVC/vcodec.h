#ifndef VCODEC_H
#define VCODEC_H
#include <pspdisplay.h>
#include <pspmpeg.h>
#include <audiocodec.h>
#include <mpegbase.h>
#include "core.h"

extern char* load();
extern char* play();
extern char* seek();
extern char* stop();

//extern void sceMpegGetAvcNalAu(SceMpeg*,Nal*,SceMpegAu*);
//extern void sceMpegAvcDecodeDetail2(SceMpeg*,SceMpegAvcDetail2**);
//extern int sceMpegBaseCscAvc(void*,unsigned,unsigned,Mp4AvcCscStruct*);
#endif