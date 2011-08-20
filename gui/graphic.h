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

//void* wallTex_=NULL,*wallTex=NULL;
int ready=0;//gu inited
char*init();
char*draw(int mode);
char*edit(int what,void*arg);
char*stop();

typedef struct{short visible,pos;char title[256];}TopBar;
typedef struct{short visible,pos;char title[256];}StatBar;
typedef struct{short visible,pos,y,h;}ScrollBar;
typedef struct{short visible,pos,curr,len;char**p;}Panel;
typedef struct{short visible,pos,x,y,curr,len,size;char**p;unsigned color,shadow;}List;
typedef struct{short visible,x,y,_x,_y;}Cursor;
typedef struct{int tbw,h;void*p;}Wall;
typedef struct{short u,v,x,y,z;}Vertex;

#endif