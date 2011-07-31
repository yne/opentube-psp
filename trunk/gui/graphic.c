#include <pspkernel.h>
#include "jpg.h"
#include "graphic.h"
#include "main.h"
#include "font.h"

PSP_MODULE_INFO("OpenTube.gui",0,0,0);

static unsigned int  __attribute__((aligned(16))) list[0x40000];int n=0;
int panelVisible=0,panelPos=0,topBarVisible=1,topBarPos=0,scrollBarVisible=1,scrollBarPos=0;

void queueSprite(short sx,short sy,short sw,short sh,short tx,short ty,short tw,short th,Vertex*v){
	if(!wallTex){sx=sy=0;sw=sh=32;}
	v[n].u=sx;v[n+1].u=v[n].u+sw;
	v[n].v=sy;v[n+1].v=v[n].v+sh;
	v[n].x=tx;v[n+1].x=v[n].x+tw;
	v[n].y=ty;v[n+1].y=v[n].y+th;
	n+=2;
}
void*getVramDrawBuffer(){
	return (void*)(dispBufferNumber?0x44088000:0x44000000);
}
void blitWall(void*p){
//	void* vram;int fmt,tbw;
//	sceDisplayGetFrameBuf(&vram,&tbw,&fmt,1);
	//sceKernelDcacheWritebackInvalidateAll();
	//sceGuStart(GU_DIRECT,list);
	if(wallTex)sceGuCopyImage(GU_PSM_8888,32,0,480,272,512,p,0,0,512,getVramDrawBuffer());
	else sceGuClear(GU_COLOR_BUFFER_BIT|GU_DEPTH_BUFFER_BIT);
	//sceGuFinish();
	//sceGuSync(0,0);
}
void drawVideoShadow(Vertex*v,int w,int h){
	int x=(480-w)/2-8,y=(272-h)/2-8;
	v[0].u=112;v[1].u=120;v[0].v=0;v[1].v=8;
	v[0].x=x;v[1].x=x+8;v[0].y=y;v[1].y=y+8;
	v[2].u=120;v[3].u=136;v[2].v=0;v[3].v=8;
	v[2].x=x+8;v[3].x=x+8+w;v[2].y=y;v[3].y=y+8;
	v[4].u=136;v[5].u=144;v[4].v=0;v[5].v=8;
	v[4].x=x+8+w;v[5].x=x+8+w+8;v[4].y=y;v[5].y=y+8;

	v[6].u=112;v[7].u=120;v[6].v=8;v[7].v=24;
	v[6].x=x;v[7].x=x+8;v[6].y=y+8;v[7].y=y+8+h;
//	v[8].u=120;v[9].u=136;v[8].v=8;v[9].v=24;
//	v[8].x=x+8;v[9].x=x+8+w;v[8].y=y;v[9].y=y+8+h;
	v[10].u=136;v[11].u=144;v[10].v=8;v[11].v=24;
	v[10].x=x+8+w;v[11].x=x+8+w+8;v[10].y=y+8;v[11].y=y+8+h;	

	v[12].u=112;v[13].u=120;v[12].v=24;v[13].v=32;
	v[12].x=x;v[13].x=x+8;v[12].y=y+8+h;v[13].y=y+8+h+8;
	v[14].u=120;v[15].u=136;v[14].v=24;v[15].v=32;
	v[14].x=x+8;v[15].x=x+8+w;v[14].y=y+8+h;v[15].y=y+8+h+8;
	v[16].u=136;v[17].u=144;v[16].v=24;v[17].v=32;
	v[16].x=x+8+w;v[17].x=x+8+w+8;v[16].y=y+8+h;v[17].y=y+8+h+8;	
}
void fallBackTex(){
	wallTex_=Malloc(32*32*4);
	for(int y=0;y<32;y++)
		for(int x=0;x<32;x++)
			((int*)wallTex_)[x+32*y]=(x==0||x==31||y==0||y==31||x==y||x==31-y)?0xFF0000FF:0x0;
}
char*demo="no licence on this lib ?!";
int fw=0,fh=0,ft=1;
void*fontTest;
char*init(){
	sceGuInit();
	dispBufferNumber=0;
	sceGuStart(GU_DIRECT,list);
	sceGuDrawBuffer(GU_PSM_8888,(void*)0x00000000,BUF_WIDTH);
	sceGuDispBuffer(SCR_WIDTH,SCR_HEIGHT,(void*)0x00088000,BUF_WIDTH);
	sceGuDepthBuffer((void*)0x00110000,BUF_WIDTH);
	sceGuOffset(2048 - (SCR_WIDTH/2),2048 - (SCR_HEIGHT/2));
	sceGuViewport(2048,2048,SCR_WIDTH,SCR_HEIGHT);
	sceGuDepthRange(0xc350,0x2710);
	sceGuScissor(0,0,SCR_WIDTH,SCR_HEIGHT);sceGuEnable(GU_SCISSOR_TEST);
	sceGuAlphaFunc(GU_GREATER,0,0xff);sceGuEnable(GU_ALPHA_TEST);
	sceGuDepthFunc(GU_GEQUAL);sceGuEnable(GU_DEPTH_TEST);
	sceGuFrontFace(GU_CW);sceGuEnable(GU_CULL_FACE);
	sceGuShadeModel(GU_SMOOTH);
	sceGuEnable(GU_CLIP_PLANES);
	sceGuBlendFunc(GU_ADD, GU_SRC_ALPHA, GU_ONE_MINUS_SRC_ALPHA, 0, 0);sceGuEnable(GU_BLEND);
	sceGuTexFunc(GU_TFX_REPLACE, GU_TCC_RGBA);
	int bw=0,bh=0;
	wallTex=jpgOpen("res/wall.jpg","res/wall_.jpg",&bw,&bh);
	if(!wallTex)fallBackTex();
	sceGuTexMode(GU_PSM_8888,0,0,0);
	if(fontInit()<0)puts("fonterr");
	sceGuEnable(GU_TEXTURE_2D);
	sceGuClearColor(0xff37352D);
	sceGuClearDepth(0);
	sceGuTexFunc(GU_TFX_REPLACE,GU_TCC_RGBA); // NOTE: this enables reads of the alpha-component from the texture, otherwise blend/test won't work
	sceGuTexFilter(GU_NEAREST,wallTex?GU_LINEAR:GU_NEAREST);
	sceGuTexWrap(GU_CLAMP,GU_CLAMP);
	sceGuFinish();
	sceGuSync(0,0);
	
	getStrRes(demo,&fw,&fh,&ft);
	fontTest=Malloc(ft*fh*4);
	fontPrint(demo,fontTest,&ft,&fh);
	sceDisplayWaitVblankStart();
	sceGuDisplay(GU_TRUE);
	ready=1;
	return 0;
}
void clean(){
	panelVisible<<=1;
	topBarVisible<<=1;
	scrollBarVisible<<=1;
}
void restor(){
	panelVisible>>=1;
	topBarVisible>>=1;
	scrollBarVisible>>=1;
}
void setPanel(){
	if(panelVisible==1){
		if(panelPos<160)panelPos+=(160-panelPos)/4;
		if(panelPos>160)panelPos=160;
	}else{
		if(panelPos>0)panelPos-=1+(panelPos)/2;
		if(panelPos<0)panelPos=0;
	}
}
void setTopBar(){
	if(topBarVisible==1){
		if(topBarPos<22)topBarPos++;
		if(topBarPos>22)topBarPos=22;
	}else{
		if(topBarPos>0)topBarPos--;
		if(topBarPos<0)topBarPos=0;
	}
}
void setScrollBar(){
	if(scrollBarVisible==1){
		if(scrollBarPos<16)scrollBarPos++;
		if(scrollBarPos>16)scrollBarPos=16;
	}else{
		if(scrollBarPos>0)scrollBarPos--;
		if(scrollBarPos<0)scrollBarPos=0;
	}
}
char*draw(int mode){//1:draw,2:sync,3:both
	if(!ready)init();
	if(mode&1){
		setPanel();
		setTopBar();
		setScrollBar();
		
		sceGuStart(GU_DIRECT, list);
		if(wallTex)sceGuTexImage(0,512,272,512,wallTex);else sceGuTexImage(0,32,32,32,wallTex_);
		blitWall(wallTex);
		//sceGuClear(GU_COLOR_BUFFER_BIT|GU_DEPTH_BUFFER_BIT);
		Vertex*v = sceGuGetMemory(6 * 2 * sizeof(Vertex));n=0;
		queueSprite( 0, 0,24,22, 0,0,480,topBarPos,v);//topBar
		queueSprite( 0,56,32, 8, 480-panelPos,22,160,250,v);
		queueSprite( 0,64,16,16, scrollBarPos-16,22,16,16,v);
		queueSprite( 0,80,16,16, scrollBarPos-16,128,16,16,v);
		queueSprite(16,64,16,16, scrollBarPos-16,256,16,16,v);
		sceGuDrawArray(GU_SPRITES,GU_TEXTURE_16BIT|GU_VERTEX_16BIT|GU_TRANSFORM_2D,n,0,v);
		sceGuFinish();
		sceGuSync(0,0);

		sceGuStart(GU_DIRECT, list);
		sceGuTexImage(0,fw,fh,ft,fontTest);
		Vertex*t = sceGuGetMemory(1 * 2 * sizeof(Vertex));n=0;
		queueSprite( 0,0,16,16, 240,136,16,16,t);
		sceGuDrawArray(GU_SPRITES,GU_TEXTURE_16BIT|GU_VERTEX_16BIT|GU_TRANSFORM_2D,n,0,t);
		sceGuFinish();
		sceGuSync(0,0);
	}
	if(mode&2){
		sceDisplayWaitVblankStart();
		sceGuSwapBuffers();
		dispBufferNumber^=1;
	}
	return 0;
}
int loop(SceSize args,void*argp){//thread
	sceIoChdir(CWD);
	while(1){
		if(Mode!=1){//no draw while playback
			sceDisplayWaitVblankStart();
			continue;
		}
		if(ot->sys->pad!=ot->sys->_pad){
			if(ot->sys->pad&PSP_CTRL_START)break;
			if(ot->sys->pad&PSP_CTRL_CROSS)Play("360p.mp4");
			if(ot->sys->pad&PSP_CTRL_TRIANGLE)panelVisible=!panelVisible;
			if(ot->sys->pad&PSP_CTRL_CIRCLE)Play("cache.mp4");
			if(ot->sys->pad&PSP_CTRL_SQUARE){
				clean();
				Open("http://google.com",0,0777);
				restor();
			}
			if(ot->sys->pad&PSP_CTRL_UP);
			if(ot->sys->pad&PSP_CTRL_DOWN);
			if(ot->sys->pad&PSP_CTRL_LEFT);
			if(ot->sys->pad&PSP_CTRL_RIGHT);
			if(ot->sys->pad&PSP_CTRL_LTRIGGER)clean();
			if(ot->sys->pad&PSP_CTRL_RTRIGGER)restor();
		}
		draw(3);
	}
	Exit();
	return 0;
}
char*stop(){
	sceGuStart(GU_DIRECT, list);
	sceGuClearColor(0xff000000);
	sceGuClear(GU_COLOR_BUFFER_BIT|GU_DEPTH_BUFFER_BIT);
	sceGuFinish();
	sceGuSync(0,0);
	sceDisplayWaitVblankStart();
	sceGuSwapBuffers();
	Free(wallTex);
//	if(<0)puts("fonterr");
	fontStop();
	sceGuTerm();	
	return NULL;
}
int module_stop(SceSize args,char*argp){
	stop();
	puts("gui unload");
	return 0;
}
Graphic gui={init,stop,draw};
int module_start(SceSize args,char*argp){
	if(args!=sizeof(OpenTube*))return 1;//return if no context
	puts("gui loaded");
	ot=(OpenTube*)((u32*)argp)[0];
	ot->gui=&gui;
	Mode=1;//menu mode
	sceKernelStartThread(sceKernelCreateThread("OpenTube.gui",loop,0x11,0x10000,0,0),0,NULL);
	return 0;
}