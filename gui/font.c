#include "font.h"
#include "main.h"

#define MYIMAGE_WIDTH (512)
#define MYIMAGE_HEIGHT (20)
#define MYIMAGE_BPP (4)
#define MYIMAGE_SIZE (MYIMAGE_WIDTH*MYIMAGE_HEIGHT)*MYIMAGE_BPP

//PSP_MODULE_INFO("libFont", 0, 1, 1);
//PSP_HEAP_SIZE_KB(1024);

static void* myMalloc(void* pMyData,unsigned int size){
	void*p = Malloc(size);
//	printf("malloc:%i %p\n",size,p);
	return p;
}
static void myFree(void* pMyData,void* p){
//	printf("free  : %p\n",p);
	free(p);
}
void RGBA2ARGB(void* p,u32 pixel){
	while(pixel--)
		((u32*)p)[pixel] = (((u32*)p)[pixel]>>8) | (((u32*)p)[pixel]<<24);
}
void RGBA2GREY(void* p,u32 pixel){
	while(pixel--){
		register u32 c = (((u32*)p)[pixel])&0xFF;
		((u32*)p)[pixel] = c<<24 | c<<16 | c<<8 | c<<0;
	}
}
void* libID,*fontID;
int getStrRes(char*str,int*w,int*h,int*t){
	for(int i=0;str[i];i++){
		CharInfo charInfo;
		if(str[i]==' '){*w+=4*3;continue;}//dosent exist
		if(sceFontGetCharInfo(fontID, str[i], &charInfo)) return  -__LINE__;
//	printf("%c : %02i %02i %02i %02i\n",str[i],charInfo.width,charInfo.height,charInfo.left,charInfo.top);
		if(charInfo.height>*h)*h=charInfo.height;
		*w+=charInfo.width+charInfo.left;
	}
	while((*t<<=1)<*w);
	return 0;
}
void*fontPrint(/*void* fontID,*/char*str,void*p,int*w,int*h){
//	if(sceFontGetCharInfo(fontID,'@', &charInfo))return p;
//	printf("%c : %02i %02i %02i %02i\n",'@',charInfo.width,charInfo.height,charInfo.left,charInfo.top);
	for(int i=0,x=0;str[i];i++){
//		if(str[i]==' '){x+=3*4;continue;}//dosent exist
//		Image myImage={PSP_FONT_32BIT,0,0,{*w,*h},(*w+x)*4,0,p};//memset(&myImage,0,sizeof(myImage));
//		if(sceFontGetCharGlyphImage(fontID, str[i], &myImage)) return NULL;
		CharInfo charInfo;
		sceFontGetCharInfo(fontID, str[i], &charInfo);
		x+=charInfo.width+charInfo.left;
//		printf("%i\n",x);
	}
	RGBA2ARGB(p,(*w)*(*h));
	return p;
}
int modid,stat;
int fontInit(){
	int errorCode;
	//if((modid=Modload("flash0:/vsh/module/libfont_hv.prx"))<0)return modid;
	InitParam initParams = {NULL,4,NULL,myMalloc,myFree,NULL,NULL,NULL,NULL,NULL,NULL};
	libID = sceFontNewLib(&initParams, &errorCode);if(errorCode)return errorCode;
	fontID = sceFontOpen(libID, FONT(REGULAR), FILE, &errorCode);if(errorCode)return errorCode;
//	FontInfo info;
//	sceFontGetFontInfo(fontID,&info);
//	printf("<<%i\n",info.maxIGlyphMetrics);
//	printf("<<%f\n",info.maxFGlyphMetrics);
//	memset((void*)0x44000000,0,4*512*32);//
	return 0;
}
int fontStop(){
	if(sceFontClose(fontID)) return  -__LINE__;
	if(sceFontDoneLib(libID)) return  -__LINE__;
	return 0;
	//return Modstun(modid);
//	sceKernelSelfStopUnloadModule(1,0,NULL);
}
/*
int module_start(int args, void* argp){
	return sceKernelStartThread(sceKernelCreateThread("load", load, 0x18, 0x10000, PSP_THREAD_ATTR_USER, NULL), args, argp);
}*/

