#ifndef FILESYS_H
#include <pspsdk.h>
#include <pspgu.h>
#include <pspkernel.h>
#include <pspdisplay.h>
#include <string.h>
#include <stdio.h>
#include <malloc.h>
#include <psputility.h>
#include <psputility_netmodules.h>
#include <psputility_htmlviewer.h>
#include <pspnet.h>
#include <pspnet_inet.h>
#include <pspnet_apctl.h>
#include <pspnet_resolver.h>
#include <psphttp.h>
#include <pspssl.h>
#define print(A) sceIoWrite(2,A,sizeof(A))
#define FD2TPL(fd) (  fd&0x0000FF)
#define FD2CNX(fd) (((fd&0x00FF00)>>8 )|0x01000000)
#define FD2REQ(fd) (((fd&0xFF0000)>>16)|0x02000000)
//0x00RRCCTT

#define FILESYS_H
int err=0;
int netInited=0;

#endif