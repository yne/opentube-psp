#ifndef GRAPHIC_H
#define GRAPHIC_H
#include <pspkernel.h>
#include <pspdisplay.h>
#include <stdlib.h>
#include <stdio.h>
#include <pspgu.h>
#include <pspctrl.h>

#define BUF_WIDTH (512)
#define SCR_WIDTH (480)
#define SCR_HEIGHT (272)

void* wallTex_=NULL,*wallTex=NULL;
int ready=0;//gu inited
int dispBufferNumber=0;

typedef struct {
	short unsigned u, v;
	short x,y,z;
}Vertex;

#endif