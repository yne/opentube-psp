#ifndef GRAPHIC_H
#define GRAPHIC_H
#include <pspkernel.h>
#include <pspdisplay.h>
#include <pspdebug.h>
#include <stdlib.h>
#include <stdio.h>

char*init();
char*stop();
char*draw(int mode);
char*edit(int what,void*arg);

typedef struct{char title[128],desc[256],date[16],size[4],*image,**keywords,uploader[32],view[16],favorite[16],rate[4],id[32];}Entry;

#endif