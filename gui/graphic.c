#include <pspkernel.h>
#include <pspumd.h>
#include <string.h>
#include "jpg.h"
#include "graphic.h"
#include "main.h"
#include "intrafont.h"
#define ISDIR(stat) (stat.d_stat.st_mode==11C0)
PSP_MODULE_INFO("OpenTube.gui",0,0,0);

static unsigned int  __attribute__((aligned(16))) list[0x40000];int n=0;
Wall bg;Panel pan;TopBar top;ScrollBar bar;List lst;Cursor cur;

void queueSprite(short sx,short sy,short sw,short sh,short tx,short ty,short tw,short th,Vertex*v){
	if(bg.tbw!=512){sx=sy=0;sw=sh=bg.tbw;}
	v[n].u=sx;v[n+1].u=v[n].u+sw;
	v[n].v=sy;v[n+1].v=v[n].v+sh;
	v[n].c=   v[n+1].c=0xFFFF;
	v[n].x=tx;v[n+1].x=v[n].x+tw;
	v[n].y=ty;v[n+1].y=v[n].y+th;
	n+=2;
}
void*getVramDrawBuffer(){
	return (void*)(dispBufferNumber?0x44088000:0x44000000);
}
void blitWall(){
	sceGuTexImage(0,bg.tbw,bg.h,bg.tbw,bg.p);
	if(bg.tbw!=512)return sceGuClear(GU_COLOR_BUFFER_BIT|GU_DEPTH_BUFFER_BIT);
	sceGuCopyImage(GU_PSM_8888,32,0,480,272,512,bg.p,0,0,512,getVramDrawBuffer());
	return sceGuTexSync();
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
	Wall tmp={32,32,Malloc(32*32*4)};bg=tmp;
	for(int y=0;y<32;y++)
		for(int x=0;x<32;x++)
			((int*)bg.p)[x+32*y]=(x==0||x==31||y==0||y==31||x==y||x==31-y)?0xFF0000FF:0x0;
}
intraFont*ltn;
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
	sceGuTexMode(GU_PSM_8888,0,0,0);
	sceGuEnable(GU_TEXTURE_2D);
	sceGuClearColor(0xff37352D);
	sceGuClearDepth(0);
	sceGuTexWrap(GU_CLAMP,GU_CLAMP);
	sceGuFinish();
	sceGuSync(0,0);
	sceDisplayWaitVblankStart();
	sceGuDisplay(GU_TRUE);
	if(!(bg.p=jpgOpen("res/wall.jpg","res/wall_.jpg",&bg.tbw,&bg.h)))fallBackTex();
	intraFontInit();
	ltn=intraFontLoad("flash0:/font/ltn8.pgf",INTRAFONT_CACHE_ASCII);
	lst.color=0xFFFFFF;
	lst.shadow=0x000000;
	ready=1;
	return 0;
}
void clean(){
	cur.visible<<=1;
	lst.visible<<=1;
	pan.visible<<=1;
	top.visible<<=1;
	bar.visible<<=1;
}
void restor(){
	cur.visible>>=1;
	lst.visible>>=1;
	pan.visible>>=1;
	top.visible>>=1;
	bar.visible>>=1;
}
void setList(){
	if(lst.visible==1){
		if((lst.color >>24)<0xF0)lst.color +=0x10000000;//(lst.color+0x10000000)&(lst.color&0xFFFFFF);
		if((lst.shadow>>24)<0xF0)lst.shadow+=0x10000000;//(lst.shadow+0x10000000)&(lst.shadow&0xFFFFFF);
	}else{
		if((lst.color >>24)>0x10)lst.color -=0x10000000;//(lst.color+0x10000000)&(lst.color&0xFFFFFF);
		if((lst.shadow>>24)>0x10)lst.shadow-=0x10000000;//(lst.shadow+0x10000000)&(lst.shadow&0xFFFFFF);
	}
}
void setCur(){
	if(pan.visible==1){
		cur.x=470-pan.pos;
		cur.y=108+12*pan.curr;
	}else{
		cur.x=bar.pos-16;
		cur.y=(lst.curr*12)+22;
	}
	if(cur._x<cur.x)cur._x+=((cur.x-cur._x)+3)/4;
	if(cur._x>cur.x)cur._x+=((cur.x-cur._x)-3)/4;
	if(cur._y<cur.y)cur._y+=((cur.y-cur._y)+3)/4;
	if(cur._y>cur.y)cur._y+=((cur.y-cur._y)-3)/4;
//	printf("%i <%i %i> <%i %i>\n",lst.curr,cur.x,cur.y,cur._x,cur._y);
}
void setPanel(){
	if(pan.visible==1){
		if(pan.pos<160)pan.pos+=(160-pan.pos)/4;
		if(pan.pos>160)pan.pos=160;
	}else{
		if(pan.pos>0)pan.pos-=1+(pan.pos)/2;
		if(pan.pos<0)pan.pos=0;
	}
}
void setTopBar(){
	if(top.visible==1){
		if(top.pos<22)top.pos++;
		if(top.pos>22)top.pos=22;
	}else{
		if(top.pos>0)top.pos--;
		if(top.pos<0)top.pos=0;
	}
}
void setScrollBar(){
	if(bar.visible==1){
		if(bar.pos<16)bar.pos++;
		if(bar.pos>16)bar.pos=16;
	}else{
		if(bar.pos>0)bar.pos--;
		if(bar.pos<0)bar.pos=0;
	}
}
char dir[256];//current browser dir
void strnccpy(char*src,char*dst,char c,int n){//cp src to dst while src!=c (at max n time)
	for(int i=0;i<n;i++)
		if(src[i]==c){dst[i]=0;break;}
		else dst[i]=src[i];
}
char*drives[]={"ms0:/","host0:/","disc0:/","flash0:/","ef0:/"};
char*badcd[]={"../"};
void cd(char* dir){
	if(!memcmp("disc0:/",dir,6)){
		puts("activating umd");
		if(!sceUmdCheckMedium())return (void)(dir[0]=0);
		sceUmdActivate(1,"disc0:");
		sceUmdWaitDriveStat(PSP_UMD_READY);
	}
	sceIoChdir(dir);
}
void ls(char* dir){
	while(lst.len)Free(lst.p[lst.len--]);
//	if(!dir[0]){printf("DIR\n");lst.p=drives;lst.len=sizeof(drives)/sizeof(char*);return;}//drives listing
	int fd=sceIoDopen(dir);
	for(SceIoDirent ent;fd>0&&sceIoDread(fd,&ent)>0;){
		if(ent.d_name[0]=='.'&&ent.d_name[1]==0)continue;//skip "."
		if(ent.d_stat.st_mode&0x1000)strcat(ent.d_name,"/");//suffix '/' if dir
		lst.p=Realloc(lst.p,(++lst.len)*sizeof(char*));//resize the list
		int size=strlen(ent.d_name)+1;
		memcpy((lst.p[lst.len-1]=Malloc(size)),ent.d_name,size);//copy name to malloc'd entry
	}
	sceIoDclose(fd);
	if(!lst.len){lst.p=badcd;lst.len=1;}
}
void updir(char*dir){
	int i=strlen(dir)-2;
//	if(dir[i]==':')return (void)(dir[0]=0);//no dir=device listing
	for(;i;i--)if(dir[i]=='/')return (void)(dir[i+1]=0);
}
void select(){
	if(pan.visible)return;
	if(lst.visible){//file browser mode
		if(lst.p[lst.curr][strlen(lst.p[lst.curr])-1]=='/'){//item is dir
//			printf("cd %s ",lst.p[lst.curr]);
			if(((int*)lst.p[lst.curr])[0]==0x002F2E2E)updir(dir);// "../"
			else strcat(dir,lst.p[lst.curr]);
//			printf("(%s)\n",dir);
			cd(dir);
			ls(dir);
			lst.curr=0;
		}else{//file selected
			char*err=Play(lst.p[lst.curr]);
			puts(err?err:"fine");
		}
	}
}
int  up(){
	if(pan.visible)return pan.curr-=2;
	if(lst.visible){
		if(lst.curr)lst.curr--;
		else lst.curr=lst.len-1;}
	return 0;
}
int  down(){
	if(pan.visible)return pan.curr+=2;
	if(lst.visible){if(lst.curr<lst.len-1)lst.curr++;else lst.curr=0;}
	return 0;
}
void left(){
	printf("lt\n");
}
void right(){
	printf("rg\n");
}
char*draw(int mode){//1:draw,2:sync,3:both
	if(!ready)init();
	if(mode&1){
		setCur();
		setList();
		setPanel();
		setTopBar();
		setScrollBar();
		sceGuStart(GU_DIRECT, list);
		blitWall();
		for(int i=0,y=35;(i<lst.len)&&(y<480);i++,y+=12)
			intraFontPrint(ltn, 16,y,lst.p[i]);//result/file list
		sceGuTexMode(GU_PSM_8888,0,0,0);
//		sceGuFinish();
//		sceGuSync(0,0);
//		sceGuStart(GU_DIRECT, list);
//		sceGuTexFilter(GU_LINEAR,bg.tbw==512?GU_LINEAR:GU_NEAREST);
		sceGuTexImage(0,bg.tbw,bg.h,bg.tbw,bg.p);
		Vertex*v=sceGuGetMemory(3*2*sizeof(Vertex));n=0;
		queueSprite( 0, 0,24,22, 0,0,480,top.pos,v);
		printf("%i\n\n",top.pos);
		queueSprite( 0,57,32, 6, 480-pan.pos,22,160,250,v);
		queueSprite( 0,80,16,16, cur._x,cur._y,16,16,v);
		sceGuDrawArray(GU_SPRITES,GU_TEXTURE_16BIT|GU_COLOR_4444|GU_VERTEX_16BIT|GU_TRANSFORM_2D,n,0,v);
		sceGuTexFilter(GU_NEAREST,GU_NEAREST);
		intraFontSetStyle(ltn,1.0f,0xFFFFFFFF,0xFF000000,0);
		intraFontPrint(ltn, 15, top.pos-7, dir);//title
		intraFontSetStyle(ltn,1.0f,lst.color,lst.shadow,0);
		if(pan.visible)
			intraFontPrint(ltn, 485-pan.pos, 121,"View\n\nSave\n\nInfo");//menu
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
	cd(CWD);
	ls(CWD);
	cur.visible=1;
	top.visible=1;
	bar.visible=1;
	lst.visible=1;
	strncpy(dir,CWD,256);
	while(1){
		if(Mode!=1){//no draw while playback
			sceDisplayWaitVblankStart();
			continue;
		}
		if(ot->sys->pad!=ot->sys->_pad){
			if(ot->sys->pad&PSP_CTRL_START)break;
			if(ot->sys->pad&PSP_CTRL_CROSS)select();//Play("res/360p.mp4");
			if(ot->sys->pad&PSP_CTRL_TRIANGLE)pan.visible=!pan.visible;
			if(ot->sys->pad&PSP_CTRL_CIRCLE)Play("res/cache.mp4");
			if(ot->sys->pad&PSP_CTRL_SQUARE){
				clean();
				Open("http://google.com",0,0777);
				restor();
			}
			if(ot->sys->pad&PSP_CTRL_UP)up();
			if(ot->sys->pad&PSP_CTRL_DOWN)down();
			if(ot->sys->pad&PSP_CTRL_LEFT)left();
			if(ot->sys->pad&PSP_CTRL_RIGHT)right();
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
	Free(bg.p);
//	fontStop();
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