#include <pspkernel.h>
#include <pspumd.h>
#include <string.h>
#include "jpg.h"
#include "graphic.h"
#include "core.h"
#include "intrafont.h"
#define ISDIR(stat)(stat.d_stat.st_mode==11C0)
PSP_MODULE_INFO("OpenTube.gui",0,0,0);

//{.bss
int n;
char dir[256];
intraFont*ltn;
Graphic gui;
Wall bg;
Panel pan;
StatBar sta;
TopBar top;
ScrollBar scr;
List lst;
Cursor cur;
//}
//{.data
char*drives[]={"ms0:/","host0:/","disc0:/","flash0:/","ef0:/"};
char*badcd[]={"../"};
//}
//{.text
void queueSprite(u16 sx,u16 sy,u16 sw,u16 sh,u16 tx,u16 ty,u16 tw,u16 th,Vertex *v){
	if(bg.tbw !=512){sx=sy=0;sw=sh=bg.tbw;	}
	v[n].u=sx;v[n+1].u=v[n].u+sw;
	v[n].v=sy;v[n+1].v=v[n].v+sh;
	v[n].x=tx;v[n+1].x=v[n].x+tw;
	v[n].y=ty;v[n+1].y=v[n].y+th;
	v[n].z=v[n+1].z=0;
	n +=2;
}
void*getVramDrawBuffer(){
	return (void*)(ot->lcd->curr?ot->lcd->disp:ot->lcd->draw);
}
int  fileExist(char*path){
	int ret=sceIoOpen(path,PSP_O_RDONLY,0);
	sceIoClose(ret);
	return ret>0;
}
void blitWall(){
	sceGuTexImage(0,bg.tbw,bg.h,bg.tbw,bg.p);
	sceGuTexSync();
	if(bg.tbw<32)sceGuClear(GU_COLOR_BUFFER_BIT);
	else sceGuCopyImage(GU_PSM_8888,32,0,480,272,512,bg.p,0,0,ot->lcd->size,getVramDrawBuffer());
}
/*
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
*/
void fallBackTex(){
	Wall tmp={32,32,Memalign(64,32*32*4)};
	bg=tmp;
	for(int y=0;y<32;y++)
		for(int x=0;x<32;x++)
			((int*)bg.p)[x+32*y]=(!x||x==31||!y||y==31||x==y||x==31-y)?0xFF0000FF:0x0;
}
char*init(){
	ot->lcd->curr=0;
	sceGuInit();
	sceGuStartList(GU_DIRECT);
	sceGuDrawBuffer(GU_PSM_8888,(void*)(ot->lcd->draw^0x44000000),ot->lcd->size);
	sceGuDispBuffer(SCR_WIDTH,SCR_HEIGHT,(void*)(ot->lcd->disp^0x44000000),ot->lcd->size);

	sceGuScissor( 0,0,SCR_WIDTH,SCR_HEIGHT);
	sceGuEnable( GU_SCISSOR_TEST );
	sceGuClearColor(0x30303030);
	sceGuColor(0xFFFFFFFF);
	
	sceGuTexMode(GU_PSM_8888,0,1,0);
	sceGuTexFunc(GU_TFX_MODULATE,GU_TCC_RGBA);
	sceGuTexWrap(GU_CLAMP,GU_CLAMP);
	sceGuTexFilter(GU_NEAREST,GU_NEAREST);	

	sceGuBlendFunc(GU_ADD,GU_SRC_ALPHA,GU_ONE_MINUS_SRC_ALPHA,0,0);
	sceGuEnable(GU_BLEND);
	sceGuEnable(GU_TEXTURE_2D);

	sceGuFinish();

	sceGuSync(GU_SYNC_FINISH,GU_SYNC_WHAT_DONE);
	sceDisplayWaitVblankStart();

	sceGuDisplay(GU_TRUE);

	intraFontInit();
	ltn=intraFontLoad(fileExist("res/font")?"res/font":"flash0:/font/ltn8.pgf",INTRAFONT_CACHE_ASCII);

	if(!(bg.p=jpgOpen("res/wall.jpg","res/wall_.jpg",&bg.tbw,&bg.h)))
		fallBackTex();

	lst.color=0xFFFFFF;
	lst.shadow=0x000000;
	ready=1;

	return 0;
}
void clean(){
	cur.visible <<=1;
	lst.visible <<=1;
	pan.visible <<=1;
	top.visible <<=1;
	scr.visible <<=1;
	sta.visible <<=1;
}
void restor(){
	cur.visible>>=1;
	lst.visible>>=1;
	pan.visible>>=1;
	top.visible>>=1;
	scr.visible>>=1;
	sta.visible>>=1;
}
void setList(){
	if(lst.visible==1){
		if((lst.color>>24)<0xF0)lst.color  +=0x10000000;
		if((lst.shadow>>24)<0xF0)lst.shadow +=0x10000000;
	}else{
		if((lst.color>>24)>0x10)lst.color  -=0x10000000;
		if((lst.shadow>>24)>0x10)lst.shadow -=0x10000000;
	}
}
void setCur(){
	if(pan.visible==1){
		cur.x=470 - pan.pos;
		cur.y=108 + 12*pan.curr;
	}else{
		cur.x=scr.pos - 16;
		cur.y=(lst.curr*12)+ 22;
	}
	if(cur._x <cur.x)cur._x +=((cur.x-cur._x)+ 3)/4;
	if(cur._x>cur.x)cur._x +=((cur.x-cur._x)- 3)/4;
	if(cur._y <cur.y)cur._y +=((cur.y-cur._y)+ 3)/4;
	if(cur._y>cur.y)cur._y +=((cur.y-cur._y)- 3)/4;
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
		if(top.pos <22)top.pos++;
		if(top.pos>22)top.pos=22;
	}else{
		if(top.pos>0)top.pos--;
		if(top.pos <0)top.pos=0;
	}
}
void setSta(){
	if(sta.visible==1){
		if(sta.pos<22)sta.pos++;
		if(sta.pos>22)sta.pos=22;
	}else{
		if(sta.pos>-15)sta.pos--;
		if(sta.pos<-15)sta.pos=-15;
	}
}
void setScrollBar(){
	if(scr.visible==1){
		if(scr.pos<8)scr.pos++;
		if(scr.pos>8)scr.pos=8;
	}else{
		if(scr.pos>-15)scr.pos--;
		if(scr.pos<-15)scr.pos=-15;
	}
}
void strnccpy(char*src,char*dst,char c,int n){
	for(int i=0;i<n;i++){
		if(src[i]==c){
			dst[i]=0;
			break;
		}else{
			dst[i]=src[i];
		}
	}
}
void cd(char* dir){
/*	if(!memcmp("disc0:/",dir,6)){
		puts("activating umd");
		if(!sceUmdCheckMedium())return (void)(dir[0]=0);
		sceUmdActivate(1,"disc0:");
		sceUmdWaitDriveStat(PSP_UMD_READY);
	}*/
	sceIoChdir(dir);
}
void ls(char* dir){
	while(lst.len)Free(lst.p[lst.len--]);
	int fd=sceIoDopen(dir);
	for(SceIoDirent ent; (fd>0)&& (sceIoDread(fd,&ent)>0);){
		if(ent.d_name[0]=='.'&&ent.d_name[1]==0)continue;
		if(ent.d_stat.st_mode&0x1000)strcat(ent.d_name,"/");
		lst.p=Realloc(lst.p,(++lst.len)*sizeof(char*));
		int size=strlen(ent.d_name)+ 1;
		memcpy((lst.p[lst.len-1]=Malloc(size)),ent.d_name,size);
	}
	sceIoDclose(fd);
	if(!lst.len){lst.p=badcd;lst.len=1;}
}
void updir(char*dir){
	int i=strlen(dir)- 2;
	for(;i;i--)
		if(dir[i]=='/')return (void)(dir[i+1]=0);
}
void select(){
	if(pan.visible)return;
	if(lst.visible){
		if(lst.p[lst.curr][strlen(lst.p[lst.curr])-1]=='/'){
			if(((int*)lst.p[lst.curr])[0]==0x002F2E2E)
				updir(dir);
			else
				strcat(dir,lst.p[lst.curr]);
			cd(dir);
			ls(dir);
			lst.curr=0;
		}else{
			char*err=Play(lst.p[lst.curr]);
			Alert(err?err:"fine");
		}
	}
}
int  up(){
	if(pan.visible)return pan.curr-=2;
	if(lst.visible){
		if(lst.curr)
			lst.curr--;
		else
			lst.curr=lst.len-1;
	}
	return 0;
}
int  down(){
	if(pan.visible)return pan.curr+=2;
	if(lst.visible){
		if(lst.curr<lst.len-1)
			lst.curr++;
		else
			lst.curr=0;
	}
	return 0;
}
//int hfd=0;
void left(){
//	clean();
	Play("http://192.168.0.102/480p.mp4");
//	restor();
}
char tmp[5];
void right(){
//	Read(hfd,tmp,4);
//	ReadX(tmp,4,4);
//	$(tmp);
	Str.curr+=0x10000;
	//printf("rg\n");
}
char*draw(int mode){
//	printf("<%08X %08X %08X>load\n",Str.start,Str.curr,Str.end);
	if(!ready)init();
	
	if(mode&1){
		sceGuStartList(GU_DIRECT);
		blitWall();
		sceGuFinish();
		sceGuSync(GU_SYNC_FINISH,GU_SYNC_WHAT_DONE);
	}
	if(mode&2){
		setCur();
		setSta();
		setList();
		setPanel();
		setTopBar();
		setScrollBar();
		sceGuStartList(GU_DIRECT);
		for(int i=0,y=35; (i<lst.len)&& (y<480); i++,y +=12)intraFontPrint(ltn,16,y,lst.p[i]);//result/file list
		sceGuTexImage(0,bg.tbw,bg.h,bg.tbw,bg.p);
		sceGuTexMode(GU_PSM_8888,0,0,0);
		sceGuTexSync();
		Vertex *v=sceGuGetMemory(2*16*sizeof(Vertex));n=0;//shadow
		queueSprite( 0, 0, 1, 1,  0, 0,480,top.pos,v);// top
		queueSprite( 0, 0, 1, 1,  0, top.pos,scr.pos,272-top.pos-sta.pos,v);// left
		queueSprite( 0, 0, 1, 1,480-pan.pos, top.pos,pan.pos,272-top.pos-sta.pos,v);// right
		queueSprite( 0, 0, 1, 1,  0,272-sta.pos,480,sta.pos,v);// bottom

		queueSprite( 0, 0,15,13,  0+scr.pos,  0+top.pos,15,13,v);// top left
		queueSprite(15, 0, 2,13, 15+scr.pos,  0+top.pos,450-pan.pos-scr.pos,13,v);//title bar bg
		queueSprite(17, 0,15,13,465-pan.pos,  0+top.pos,15,13,v);// top right
		queueSprite( 0,14,15, 2,  0+scr.pos, 13+top.pos,15,242-top.pos-sta.pos,v);//scroll bg
		queueSprite(17,14,15, 2,465-pan.pos, 13+top.pos,15,242-top.pos-sta.pos,v);//menu bg
		queueSprite( 0,15,15,17,  0+scr.pos,255-sta.pos,15,17,v);// bottom left
		queueSprite(15,15, 2,17, 15+scr.pos,255-sta.pos,450-pan.pos-scr.pos,17,v);//??? bg
		queueSprite(17,15,15,17,465-pan.pos,255-sta.pos,15,17,v);// bottom right
		
		queueSprite( 0,80,16,16,cur._x,cur._y,16,16,v);//cursor
		
		int total=1+(Str.size?Str.size:Str.end)/480;
		queueSprite( 8,32, 8, 8,Str.start/total,264,(Str.end-Str.start)/total, 8,v);//buffered zone
		queueSprite( 0,32, 8, 8,0,264,480, 8,v);//buffer bar
		queueSprite(16,32, 8, 8,(Str.curr/total)-4,264,8, 8,v);//current position

		sceGuDrawArray(GU_SPRITES,GU_TEXTURE_16BIT|GU_VERTEX_16BIT|GU_TRANSFORM_2D,n,0,v);

		intraFontSetStyle(ltn,1.0f,0xFFFFFFFF,0x00000000,0);
		intraFontPrint(ltn,15,top.pos-7,dir);//title

		intraFontSetStyle(ltn,1.0f,lst.color,lst.shadow,0);
		if(pan.visible)intraFontPrint(ltn,485-pan.pos,121,"View\n\nSave\n\nInfo");//menu

		sceGuFinish();
		sceGuSync(GU_SYNC_FINISH,GU_SYNC_WHAT_DONE);
	}
	if(mode&4)
		sceDisplayWaitVblankStart();
	if(mode&8){
		sceGuSwapBuffers();
		ot->lcd->curr^=1;
	}
	return 0;
}
int  loop(SceSize args,void*argp){
	cd(CWD);
	ls(CWD);
	cur.visible=1;
	top.visible=1;
	scr.visible=1;
	lst.visible=1;
	sta.visible=1;//!!ot->sys->err;
	strncpy(dir,CWD,256);
	while(1){
		if(Mode!=1){//no draw while playback
			sceDisplayWaitVblankStart();
			continue;
		}
		if(ot->sys->pad !=ot->sys->_pad){
			if(ot->sys->pad & PSP_CTRL_START)break;
			if(ot->sys->pad & PSP_CTRL_CROSS)select();//Play("res/360p.mp4");
			if(ot->sys->pad & PSP_CTRL_TRIANGLE)pan.visible=!pan.visible;
			if(ot->sys->pad & PSP_CTRL_SQUARE);
			if(ot->sys->pad & PSP_CTRL_SELECT){
				clean();
				Open("http://google.com",0,0777);
				restor();
			}
			if(ot->sys->pad & PSP_CTRL_UP)up();
			if(ot->sys->pad & PSP_CTRL_DOWN)down();
			if(ot->sys->pad & PSP_CTRL_LEFT)left();
			if(ot->sys->pad & PSP_CTRL_RIGHT)right();
//			if(ot->sys->pad & PSP_CTRL_LTRIGGER)clean();
//			if(ot->sys->pad & PSP_CTRL_RTRIGGER)restor();
		}
		draw(1|2|4|8);
	}
	Exit();
	return 0;
}
int  unload(SceSize args,void*argp){
	do sceKernelDelayThread(1000);
	while(sceKernelSelfStopUnloadModule(1,0,NULL)<0);
	return 0;
}
int  module_stop(SceSize args,char*argp){
	return 0;
}
char*stop(){
	Free(bg.p);
	sceGuTerm();
	$("gui unload\n");
	sceKernelStartThread(sceKernelCreateThread("arakiri",unload,0x11,0x1000,0,0),0,NULL);//to be unable to return
	return NULL;
}
int  module_start(SceSize args,char*argp){
	if((ot=otGetCtx())<0)return 1;
	$("gui loaded\n");
	Graphic _gui={init,stop,draw};gui=_gui;
	ot->gui=&gui;
	Mode=1;
	sceKernelStartThread(sceKernelCreateThread("OpenTube.gui",loop,0x11,0x10000,0,0),0,NULL);
	return 0;
}
//}