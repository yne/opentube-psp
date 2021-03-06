#include <stdio.h>
#include <psptypes.h>
#include <pspkernel.h>
#include <pspdisplay.h>
#include <pspsysclib.h>
#include <pspge.h>
#include <stdarg.h>
#include <pspdebug.h>
#include <string.h>
#define PSP_SCREEN_WIDTH 480
#define PSP_SCREEN_HEIGHT 272
#define PSP_LINE_SIZE 512

void  guiClearLine(int Y);
static int X=0, Y=0;
static int MX=68, MY=34;
static u32 bg_col=0, fg_col=0xFFFFFFFF;
static int bg_enable=1;
static void* g_vram_base=(u32 *) 0x04000000;
static int g_vram_offset=0;
static int g_vram_mode=PSP_DISPLAY_PIXEL_FORMAT_8888;
static int init=0;
static int clearline_en=1;
/* 3*4
"\x40\xa0\xe0\xa0\x00\x00\x00\x00"//a
"\x80\xc0\xa0\xc0\x00\x00\x00\x00"
"\x60\x80\x80\x60\x00\x00\x00\x00"
"\x20\x60\xa0\x60\x00\x00\x00\x00"
"\x40\xe0\x80\x60\x00\x00\x00\x00"
"\xe0\x80\xc0\x80\x00\x00\x00\x00"
"\x60\x80\xa0\x60\x00\x00\x00\x00"
"\xa0\xe0\xe0\xa0\x00\x00\x00\x00"
"\x40\x00\x40\x40\x00\x00\x00\x00"
"\x40\x00\x40\xc0\x00\x00\x00\x00"
"\xa0\xc0\xa0\xa0\x00\x00\x00\x00"
"\x80\x80\x80\xe0\x00\x00\x00\x00"
"\xa0\xe0\xa0\xa0\x00\x00\x00\x00"
"\xa0\xe0\xe0\xa0\x00\x00\x00\x00"
"\x40\xa0\xa0\x40\x00\x00\x00\x00"
"\xc0\xa0\xc0\x80\x00\x00\x00\x00"
"\x60\xa0\x60\x20\x00\x00\x00\x00"
"\xc0\xa0\xc0\xa0\x00\x00\x00\x00"
"\x60\xc0\x20\xc0\x00\x00\x00\x00"
"\xe0\x40\x40\x40\x00\x00\x00\x00"
"\xa0\xa0\xa0\xe0\x00\x00\x00\x00"
"\xa0\xa0\xa0\x40\x00\x00\x00\x00"
"\xa0\xa0\xe0\xa0\x00\x00\x00\x00"
"\xa0\x40\x40\xa0\x00\x00\x00\x00"
"\xa0\xa0\x40\x40\x00\x00\x00\x00"
"\xe0\x20\xc0\xe0\x00\x00\x00\x00"

"\xe0\xa0\xa0\xe0\x00\x00\x00\x00" //0
"\xc0\x40\x40\x40\x00\x00\x00\x00"
"\xc0\x20\x40\xe0\x00\x00\x00\x00"
"\xc0\x60\x20\xe0\x00\x00\x00\x00"
"\xa0\xe0\x20\x20\x00\x00\x00\x00"
"\xe0\x80\x20\xc0\x00\x00\x00\x00"
"\xe0\x80\xe0\xe0\x00\x00\x00\x00"
"\xe0\x20\x40\x40\x00\x00\x00\x00"
"\xe0\xe0\xa0\xe0\x00\x00\x00\x00"
"\xe0\xe0\x20\xe0\x00\x00\x00\x00"
*/
static u8 raw[]=//{
"\x00\x00\x00\x00\x00\x00\x00\x00"
"\x02\x02\x04\x04\x48\x28\x10\x00"//validate (v)
"\x84\xcc\x78\x30\x78\xcc\x84\x00"//cancel (x)
"\x38\x44\xaa\x92\xaa\x44\x38\x00"// (x)
"\x38\x44\x92\xaa\x92\x44\x38\x00"// (o)
"\x38\x44\x92\xaa\xba\x44\x38\x00"// (/\)
"\x38\x44\xba\xaa\xba\x44\x38\x00"// ([])
"\xc0\xf0\xfc\xfe\xfc\xf0\xc0\x00"// eye
"\x1c\x3c\x54\xec\x54\x3c\x1c\x00"// backspace
"\x18\xdb\xff\x7e\x3c\x7e\xe7\xc3"// tab
"\x00\x00\x00\x30\x30\x00\x00\x00"// 
"\x0f\x03\x05\x79\x88\x88\x88\x70"// ^
"\x38\x44\x44\x44\x38\x10\x7c\x10"// v
"\x30\x28\x24\x24\x28\x20\xe0\xc0"// <- //CR
"\x3c\x24\x3c\x24\x24\xe4\xdc\x18"// ->
"\x3e\x42\xa2\xa2\xba\x82\xfc\x00"// L
"\x78\x84\xb2\xb2\xaa\x82\x7e\x00"// R
"\x00\x1a\x26\x4e\x40\x24\x18\x00"// refresh
"\x3c\x42\x89\x89\xb9\x81\x42\x3c"// clock
"\x3c\x42\xa5\xed\x81\xbd\x5a\x3c"// ^_^
"\x00\x44\xee\xfe\x7c\x38\x10\x00"// favorite
"\x08\x12\xa4\x48\x10\x25\x42\x05"// yes/no
"\xff\xff\x00\x00\x00\x00\x00\x00"// border n8
"\xff\xff\x03\x03\x03\x03\x03\x03"// border n9
"\x00\x00\x00\x00\x00\x00\xff\xff"// border n6
"\x03\x03\x03\x03\x03\x03\xff\xff"// border n3
"\x00\x00\x00\x00\x00\x00\xff\xff"// border n2
"\xc0\xc0\xc0\xc0\xc0\xc0\xff\xff"// border n1
"\xc0\xc0\xc0\xc0\xc0\xc0\xc0\xc0"// border n4
"\xff\xff\xc0\xc0\xc0\xc0\xc0\xc0"// border n7
"\x80\x40\x20\x10\x08\x04\x02\x01"// date
"\x00\x44\xee\xfe\x7c\x38\x10\x00"// favorite
"\x00\x00\x00\x00\x00\x00\x00\x00"// spaaaaace
"\x20\x20\x20\x20\x00\x00\x20\x00"// !
"\x50\x50\x50\x00\x00\x00\x00\x00"// "
"\x50\x50\xf8\x50\xf8\x50\x50\x00"// #
"\x20\x78\xa0\x70\x28\xf0\x20\x00"// $
"\xc0\xc8\x10\x20\x40\x98\x18\x00"// %
"\x40\xa0\x40\xa8\x90\x98\x60\x00"// &
"\x10\x20\x40\x00\x00\x00\x00\x00"// '
"\x10\x20\x40\x40\x40\x20\x10\x00"// (
"\x40\x20\x10\x10\x10\x20\x40\x00"// )
"\x20\xa8\x70\x20\x70\xa8\x20\x00"// *
"\x00\x20\x20\xf8\x20\x20\x00\x00"// +
"\x00\x00\x00\x00\x00\x20\x20\x40"// ,
"\x00\x00\x00\x78\x00\x00\x00\x00"// _
"\x00\x00\x00\x00\x00\x60\x60\x00"// .
"\x00\x00\x08\x10\x20\x40\x80\x00"// /
"\x00\xfc\x8c\x94\xa4\xc4\xfc\x00"// 0
"\x00\x10\x10\x10\x10\x10\x10\x00"
"\x00\xfc\x04\xfc\x80\x80\xfc\x00"
"\x00\xfc\x04\xfc\x04\x04\xfc\x00"
"\x00\x84\x84\xfc\x04\x04\x04\x00"
"\x00\xfc\x80\xfc\x04\x04\xfc\x00"
"\x00\xfc\x80\xfc\x84\x84\xfc\x00"
"\x00\xfc\x04\x04\x04\x04\x04\x00"
"\x00\xfc\x84\xfc\x84\x84\xfc\x00"
"\x00\xfc\x84\xfc\x04\x04\xfc\x00"
"\x00\x00\x20\x00\x00\x20\x00\x00"// :
"\x00\x00\x20\x00\x00\x20\x20\x40"// ;
"\x18\x30\x60\xc0\x60\x30\x18\x00"// <
"\x00\x00\xf8\x00\xf8\x00\x00\x00"// =
"\xc0\x60\x30\x18\x30\x60\xc0\x00"// >
"\x70\x88\x08\x10\x20\x00\x20\x00"// ?
"\x70\x88\x08\x68\xa8\xa8\x70\x00"// @
"\x00\xfc\x84\xfc\x84\x84\x84\x00"// A
"\x00\xfc\x84\xf8\x84\x84\xfc\x00"
"\x00\xfc\x80\x80\x80\x80\xfc\x00"
"\x00\xf8\x84\x84\x84\x84\xf8\x00"
"\x00\xf8\x80\xf8\x80\x80\xf8\x00"
"\x00\xf8\x80\xf8\x80\x80\x80\x00"
"\x00\xfc\x80\x80\x84\x84\xfc\x00"
"\x00\x88\x88\xf8\x88\x88\x88\x00"
"\x00\x20\x20\x20\x20\x20\x20\x00"
"\x00\x04\x04\x04\x84\x84\xfc\x00"
"\x00\x84\x84\xf8\x84\x84\x84\x00"
"\x00\x80\x80\x80\x80\x80\xfc\x00"
"\x00\xfc\x94\x94\x84\x84\x84\x00"
"\x00\xf8\x84\x84\x84\x84\x84\x00"//N
"\x00\xfc\x84\x84\x84\x84\xfc\x00"
"\x00\xfc\x84\xfc\x80\x80\x80\x00"
"\x00\xfc\x84\x84\x94\x94\xfc\x00"
"\x00\xfc\x84\xf8\x84\x84\x84\x00"
"\x00\xfc\x80\xfc\x04\x04\xfc\x00"
"\x00\xfc\x10\x10\x10\x10\x10\x00"
"\x00\x84\x84\x84\x84\x84\xfc\x00"
"\x00\x84\x84\x84\x84\x84\x78\x00"
"\x00\x84\x84\x84\x94\x94\x78\x00"
"\x00\x84\x84\x78\x84\x84\x84\x00"
"\x00\x84\x84\xfc\x10\x10\x10\x00"
"\x00\xfc\x04\xfc\x80\x80\xfc\x00"
"\x70\x40\x40\x40\x40\x40\x70\x00"// [
"\x00\x00\x80\x40\x20\x10\x08\x00"// \.
"\x70\x10\x10\x10\x10\x10\x70\x00"// ]
"\x20\x50\x88\x00\x00\x00\x00\x00"// ^
"\x00\x00\x00\x00\x00\x00\xf8\x00"// _
"\x40\x20\x10\x00\x00\x00\x00\x00"// '
"\x00\xfc\x84\xfc\x84\x84\x84\x00"// a
"\x00\xfc\x84\xf8\x84\x84\xfc\x00"
"\x00\xfc\x80\x80\x80\x80\xfc\x00"
"\x00\xf8\x84\x84\x84\x84\xf8\x00"
"\x00\xf8\x80\xf8\x80\x80\xf8\x00"
"\x00\xf8\x80\xf8\x80\x80\x80\x00"
"\x00\xfc\x80\x80\x84\x84\xfc\x00"
"\x00\x88\x88\xf8\x88\x88\x88\x00"
"\x00\x20\x20\x20\x20\x20\x20\x00"
"\x00\x04\x04\x04\x84\x84\xfc\x00"
"\x00\x84\x84\xf8\x84\x84\x84\x00"
"\x00\x80\x80\x80\x80\x80\xfc\x00"
"\x00\xfc\x94\x94\x84\x84\x84\x00"
"\x00\xf8\x84\x84\x84\x84\x84\x00"//n
"\x00\xfc\x84\x84\x84\x84\xfc\x00"
"\x00\xfc\x84\xfc\x80\x80\x80\x00"
"\x00\xfc\x84\x84\x94\x94\xfc\x00"
"\x00\xfc\x84\xf8\x84\x84\x84\x00"
"\x00\xfc\x80\xfc\x04\x04\xfc\x00"
"\x00\xfc\x10\x10\x10\x10\x10\x00"
"\x00\x84\x84\x84\x84\x84\xfc\x00"
"\x00\x84\x84\x84\x84\x84\x78\x00"
"\x00\x84\x84\x84\x94\x94\x78\x00"
"\x00\x84\x84\x78\x84\x84\x84\x00"
"\x00\x84\x84\xfc\x10\x10\x10\x00"
"\x00\xfc\x04\xfc\x80\x80\xfc\x00"
"\x18\x20\x20\x40\x20\x20\x18\x00"// {
"\x20\x20\x20\x00\x20\x20\x20\x00"// |
"\xc0\x20\x20\x10\x20\x20\xc0\x00"// }
"\x40\xa8\x10\x00\x00\x00\x00\x00"// ~
"\x00\x00\x20\x50\xf8\x00\x00\x00"// del
;//}

static u16 convert_8888_to_565(u32 color){
	return ((color>>3)&0x1F)|(((color>>10)&0x3F)<<5)|(((color>>19)&0x1F)<<11);
}
static u16 convert_8888_to_5551(u32 c){
	return ((c>>24)?0x8000:0)|((c>>3)&0x1F)|(((c>>11)&0x1F)<<5)|(((c>>19)&0x1F)<<10);
}
static u16 convert_8888_to_4444(u32 c){
	return (((c>>28)&0xF)<<12)|((c>>4)&0xF)|(((c>>12)&0xF)<<4)|(((c>>20)&0xF)<<8);
}
static void clear_screen_16(u16 color){
	u16 *vram=g_vram_base+(g_vram_offset >> 1);
	for(int x=0; x < (PSP_LINE_SIZE * PSP_SCREEN_HEIGHT); x++)
		*vram++=color; 
}
static void clear_screen_32(u32 color){
	u32 *vram=g_vram_base+(g_vram_offset>>2);
	for(int x=0; x < (PSP_LINE_SIZE * PSP_SCREEN_HEIGHT); x++)
		*vram++=color;
}
static void clear_screen(u32 color){
	if(g_vram_mode == PSP_DISPLAY_PIXEL_FORMAT_8888)
		clear_screen_32(color);
	else{
		u16 c=0;
		switch(g_vram_mode){
			case PSP_DISPLAY_PIXEL_FORMAT_565: c=convert_8888_to_565(color);break;
			case PSP_DISPLAY_PIXEL_FORMAT_5551: c=convert_8888_to_5551(color);break;
			case PSP_DISPLAY_PIXEL_FORMAT_4444: c=convert_8888_to_4444(color);break;
		}
		clear_screen_16(c);
	}
}
void guiInitEx(void *vram_base, int mode, int setup){
	switch(mode){
		case PSP_DISPLAY_PIXEL_FORMAT_565:
		case PSP_DISPLAY_PIXEL_FORMAT_5551:
		case PSP_DISPLAY_PIXEL_FORMAT_4444:
		case PSP_DISPLAY_PIXEL_FORMAT_8888:
			break;
		default: mode=PSP_DISPLAY_PIXEL_FORMAT_8888;
	};
	X=Y=0;
	g_vram_base=vram_base?vram_base:(void*) (0x40000000 | (u32) sceGeEdramGetAddr());
	g_vram_offset=0;
	g_vram_mode=mode;
	if(setup){
		sceDisplaySetMode(0, PSP_SCREEN_WIDTH, PSP_SCREEN_HEIGHT);
		sceDisplaySetFrameBuf((void *) g_vram_base, PSP_LINE_SIZE, mode, 1);
	}
	clear_screen(bg_col);
	init=1;
}
void guiInit(){
	X=Y=0;
	guiInitEx(NULL, PSP_DISPLAY_PIXEL_FORMAT_8888, 1);
}
void*guiGetFont(){
	return raw;
}
void guiEnableBackColor(int enable) {
	bg_enable=enable;
}
void guiSetBackColor(u32 colour){
	bg_col=colour;
}
void guiSetTextColor(u32 colour){
	fg_col=colour;
}
void guiSetColorMode(int mode){
	switch(mode){
		case PSP_DISPLAY_PIXEL_FORMAT_565:
		case PSP_DISPLAY_PIXEL_FORMAT_5551:
		case PSP_DISPLAY_PIXEL_FORMAT_4444:
		case PSP_DISPLAY_PIXEL_FORMAT_8888:
			break;
		default: mode=PSP_DISPLAY_PIXEL_FORMAT_8888;
	};
	g_vram_mode=mode;
}
void guiSetMaxX(unsigned int maxx){
	MX=maxx;
}
void guiSetMaxY(unsigned int maxy){
	MY=maxy;
}
int  guiGetX(){
	return X;
}
int  guiGetY(){
	return Y;
}
void guiSetXY(int x, int y){
	if(x<MX && x>=0) X=x;
	if(y<MY && y>=0) Y=y;
}
void guiClear(){
	if(!init)return;
	for(int y=0;y<MY;y++)
		guiClearLine(y);
	guiSetXY(0,0);
	clear_screen(bg_col);
}
void guiSetOffset(int offset){
	g_vram_offset=offset;
}
void guiSetBase(u32* base){
	g_vram_base=base;
}
static void debug_put_char_32(int x, int y, u32 color, u32 bgc, u8 ch){
	int 	i,j, l;
	u8	*font;
	u32 *vram_ptr;
	if(!init) return;
	u32 *vram=g_vram_base;
	vram += (g_vram_offset >> 2) + x;
	vram += (y * PSP_LINE_SIZE);
	
	font=&raw[ (int)ch * 8];
	for (i=l=0; i < 8; i++, l+= 8, font++){
		vram_ptr =vram;
		for (j=0; j < 8; j++){
				if ((*font & (128 >> j)))
				 *vram_ptr=color; 
				else if(bg_enable)
				 *vram_ptr=bgc; 
			 vram_ptr++;
			}
		vram += PSP_LINE_SIZE;
	}
}
static void debug_put_char_16(int x, int y, u16 color, u16 bgc, u8 ch){
   int 	i,j, l;
   u8	*font;
   u16 *vram_ptr;
   u16 *vram;

   if(!init)return;
   vram = g_vram_base;
   vram += (g_vram_offset >> 1) + x;
   vram += (y * PSP_LINE_SIZE);
   
   font = &raw[ (int)ch * 8];
   for (i=l=0; i < 8; i++, l+= 8, font++){
      vram_ptr  = vram;
      for (j=0; j < 8; j++)
	{
          if ((*font & (128 >> j)))
			  *vram_ptr = color; 
          else if(bg_enable)
			  *vram_ptr = bgc; 

		  vram_ptr++;
	}
      vram += PSP_LINE_SIZE;
   }
}
void guiPutc(int x, int y, u32 color, u8 ch){
	if(g_vram_mode == PSP_DISPLAY_PIXEL_FORMAT_8888)
		debug_put_char_32(x, y, color, bg_col, ch);
	else{
		u16 c=0;
		u16 b=0;
		switch(g_vram_mode){
			case PSP_DISPLAY_PIXEL_FORMAT_565:
				c=convert_8888_to_565(color);
				b=convert_8888_to_565(bg_col);
				break;
			case PSP_DISPLAY_PIXEL_FORMAT_5551:
				c=convert_8888_to_5551(color);
				b=convert_8888_to_5551(bg_col);
				break;
			case PSP_DISPLAY_PIXEL_FORMAT_4444:
				c=convert_8888_to_4444(color);
				b=convert_8888_to_4444(bg_col);
				break;
		};
		debug_put_char_16(x, y, c, b, ch);
	}
}
void guiClearLine(int Y){
	if(clearline_en&&bg_enable)
		for (int i=0; i < MX; i++)
			guiPutc(i*7 , Y * 8, bg_col, 219);
}
void guiSetClearLine(int n){
	clearline_en=n;
}
int  guiPrintData(const char *buff, int size){
	int i;
	int j;
	char c;
	if(!init)return 0;
	for (i=0; i < size; i++){
		c=buff[i];
		switch (c){
			case '\r':X=0;break;
			case '\n':
						while(X<=MX)
							guiPutc(X++*7 , Y * 8, fg_col, ' ');
						X=0;
//						guiClearLine(Y);
						if (++Y == MY)Y=0;
						break;
			case '\t':
						for (j=0; j < 5; j++,X++) 
							guiPutc(X*7 , Y * 8, fg_col, ' ');
						break;
			default:
						guiPutc(X*7 , Y * 8, fg_col, c);
						if (++X != MX)break;
						X=0;
						if (++Y == MY)guiClearLine((Y=0));
		}
	}
	return i;
}
int  guiPuts(const char *str){
	return guiPrintData(str, strlen(str));
}