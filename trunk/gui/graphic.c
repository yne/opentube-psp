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
int n,run;
char dir[256];
intraFont*ltn;
Graphic gui={init,stop,draw,edit};
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
	if(bg.tbw !=512){sx=sy=0;sw=sh=bg.tbw;}
	v[n].u=sx;v[n+1].u=v[n].u+sw;
	v[n].v=sy;v[n+1].v=v[n].v+sh;
	v[n].x=tx;v[n+1].x=v[n].x+tw;
	v[n].y=ty;v[n+1].y=v[n].y+th;
	v[n].z=v[n+1].z=0;
	n+=2;
}
void*getVramDrawBuffer(){
	return (void*)(ot->lcd->curr?ot->lcd->disp:ot->lcd->draw);
}
int  fileExist(char*path){
	int ret=sceIoOpen(path,PSP_O_RDONLY,0);
	sceIoClose(ret);
	return ret>0;
}
char*argcat(char* dst,char* src){
	int n = strlen(src)+1;
	memcpy(dst,src,n);
	return dst+n;
}
void fallBackTex(){
	Wall tmp={32,32,Memalign(64,32*32*4)};//mem leak
	bg=tmp;
	for(int y=0;y<32;y++)
		for(int x=0;x<32;x++)
			((int*)bg.p)[x+32*y]=0x80FFFFFF;
//			((int*)bg.p)[x+32*y]=(!x||x==31||!y||y==31||x==y||x==31-y)?0xFF0000FF:0x0;
}
char*init(){
	ot->lcd->curr=0;
	sceGuInit();
	sceGuStartList(GU_DIRECT);
	sceGuDrawBuffer(GU_PSM_8888,(void*)(ot->lcd->draw^0x44000000),ot->lcd->size);
	sceGuDispBuffer(SCR_WIDTH,SCR_HEIGHT,(void*)(ot->lcd->disp^0x44000000),ot->lcd->size);

	sceGuScissor(0,0,SCR_WIDTH,SCR_HEIGHT);
	sceGuEnable(GU_SCISSOR_TEST);
	sceGuClearColor(0xFF000000);
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
	ltn=intraFontLoad("flash0:/font/ltn8.pgf",INTRAFONT_CACHE_ASCII);
	bg.p=jpgOpen("res/wall.jpg","res/wall_.jpg",&bg.tbw,&bg.h);
	if(!bg.tbw){bg.p=NULL;fallBackTex();}
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
	scr.visible<<=1;
	sta.visible<<=1;
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
	lst.x=16;
	lst.y=35;
	if(lst.visible==1){
		lst.pos=255;
//		if((lst.color >>24)<0xF0)lst.color  +=0x10000000;
//		if((lst.shadow>>24)<0xF0)lst.shadow +=0x10000000;
	}else{
		lst.pos=0;
//		if((lst.color >>24)>0x10)lst.color  -=0x10000000;
//		if((lst.shadow>>24)>0x10)lst.shadow -=0x10000000;
	}
}
void setCur(){
	if(pan.visible==1){
		cur.x=478 - pan.pos;
		cur.y=108 + 12*pan.curr;
	}else{
		cur.x=lst.x - 16;
		cur.y=(lst.curr*12)+ 22;
	}
	if(cur._x<cur.x)cur._x +=((cur.x-cur._x)+ 3)/4;
	if(cur._x>cur.x)cur._x +=((cur.x-cur._x)- 3)/4;
	if(cur._y<cur.y)cur._y +=((cur.y-cur._y)+ 3)/4;
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
		if(top.pos<32)top.pos++;
		if(top.pos>32)top.pos=32;
	}else{
		if(top.pos>0)top.pos--;
		if(top.pos<0)top.pos=0;
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
		if(scr.pos<0)scr.pos++;
		if(scr.pos>0)scr.pos=0;
	}else{
		if(scr.pos>-15)scr.pos--;
		if(scr.pos<-15)scr.pos=-15;
	}
}
void strnccpy(char*dst,char*src,char c,int n){
	for(int i=0;i<n;i++){
		if(src[i]==c){
			dst[i]=0;
			break;
		}else{
			dst[i]=src[i];
		}
	}
}
int  strclen(char*p,char c){
	int i;
	for(i=0;p[i]!=c&&p[i];i++);
	return i;
}
void cd(char* dir){
/*	if(!memcmp("disc0:/",dir,6)){
		puts("activating umd");
		if(!sceUmdCheckMedium())return (void)(dir[0]=0);
		sceUmdActivate(1,"disc0:");
		sceUmdWaitDriveStat(PSP_UMD_READY);
	}*/
	sceIoChdir(dir);
	strncpy(top.title,dir,255);
}
void ls(char* dir){
	while(lst.len)Free(lst.p[lst.len--]);
	int fd=sceIoDopen(dir);
	if(fd<0){$("failed to open dir\n");return;}
	for(SceIoDirent ent;sceIoDread(fd,memset(&ent,0,sizeof(ent)))>0;){
		if(ent.d_name[0]=='.'&&ent.d_name[1]==0)continue;
		if(ent.d_stat.st_mode&0x1000)strcat(ent.d_name,"/");
		lst.p=Realloc(lst.p,(++lst.len)*sizeof(char*));
		int size=strlen(ent.d_name)+ 1;
		memcpy((lst.p[lst.len-1]=Malloc(size)),ent.d_name,size);
	}
	sceIoDclose(fd);
//	if(!lst.len){lst.p=badcd;lst.len=1;}
}
void updir(char*dir){
	int i=strlen(dir)- 2;
	for(;i;i--)
		if(dir[i]=='/')return (void)(dir[i+1]=0);
}
void runJs(char*path){
	char arg[256]={0};
	strcpy(arg,CWD);//libjs folder
	strnccpy(path,dir,'?',255);
	char*_arg=argcat(argcat(argcat(arg,strcat(arg,"libjs")),path),"1024");
	int ret,mod=sceKernelLoadModule(arg,0,0);
	sceKernelStartModule(mod,_arg-arg,&arg,&ret,NULL);
}
void select(){
	char*curr=lst.p[lst.curr];
	$(">");Alert(dir);$(">>");Alert(curr);$("\n");
	if(!lst.visible)return;
	if(strstr(dir ,".js")||strstr(dir ,".js")){//file is a script
		char tmp[256];
		strnccpy(tmp,dir,'?',255);//param free adress
		strnccpy(dir,dir,'?',255);//dir-=param
		strcat(dir,"?");//add curr param to dir
		strcat(dir,curr);
		runJs(tmp);
		return;
	}
	//we are in file browser
	if(curr[strlen(curr)-1]=='/'){//is a dir
		if(((int*)curr)[0]==0x002F2E2E)
			updir(dir);
		else
			strcat(dir,curr);
		cd(dir);
		ls(dir);
		lst.curr=0;
	}else{
		int len=strlen(curr);
		if(strstr(curr ,".js")||strstr(curr ,".JS")){//script
			strcat(dir,curr);
			strcat(dir,"?boot");
			runJs(dir);
		}else if(curr[len-4]=='.'&&curr[len-3]=='p'&&curr[len-2]=='g'&&curr[len-1]=='f'){//pgf font
			intraFont*ltn_tmp=intraFontLoad(curr,INTRAFONT_CACHE_ASCII);
			if(ltn_tmp){
				intraFontUnload(ltn);
				ltn=ltn_tmp;
			}
		}else	if(curr[len-5]=='_'&&curr[len-4]=='.'&&curr[len-3]=='j'&&curr[len-2]=='p'&&curr[len-1]=='g'){//alpha wall
			char wall[256];
			strcpy(wall,curr);//replace(/_\.jpg$/,".jpg") ... so easy
			wall[len-5]='.';wall[len-4]='j';wall[len-3]='p';wall[len-2]='g';wall[len-1]=0;
			char*tmp=jpgOpen(wall,curr,&bg.tbw,&bg.h);
			if(bg.tbw){
				free(bg.p);
				bg.p=tmp;
			}else{
				Alert(tmp);
			}
		}else{
			clean();
			char*err=Play(curr);
			restor();
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
void left(){
//	clean();
//	Open("http://gdata.youtube.com/feeds/api/videos?q=djmax&start-index=1&max-results=20&v=1",PSP_O_RDONLY,0777);
//	Play("http://192.168.0.100/480p.mp4");
//	restor();
}
void right(){
}
char*edit(int what,void*arg){
	if(!what){// ?
		Alert(arg);
	}else if(what==1){//set list
		while(lst.len)Free(lst.p[lst.len--]);//free previous content
		char*list=arg;
		for(int i=strlen(list);i;i--)if(list[i]=='\n')lst.len++;//count separator
		lst.p=Realloc(lst.p,(++lst.len)*sizeof(char*));
		for(int l=0,i=0;i<lst.len;i++){
			int t=strclen(list+l,'\n');
			lst.p[i]=memcpy((char*)Malloc(t+1),list+l,t);
			lst.p[i][t]=0;
			l+=t+1;
		}
	}else if(what==2){//set title
		strncpy(top.title,arg,255);
	}else if(what==3){//set menu
		while(pan.len)Free(pan.p[pan.len--]);//free previous content
		char*list=arg;
		for(int i=strlen(list);i;i--)if(list[i]=='\n')pan.len++;//count separator
		pan.p=Realloc(pan.p,(++pan.len)*sizeof(char*));
		for(int l=0,i=0;i<pan.len;i++){
			int t=strclen(list+l,'\n');
			pan.p[i]=memcpy((char*)Malloc(t+1),list+l,t);
			pan.p[i][t]=0;
			l+=t+1;
		}
	}else if(what==-1){//get curr list elem
	}else if(what==-2){//get curr dir
		return dir;
	}else if(what==-3){//get curr menu
	}
	return NULL;
}
void drawBuffer(Vertex*v){
	if(Str.th){
		int total=1+(Str.size?Str.size:Str.end)/480;
		queueSprite( 8,32, 8, 8,Str.start/total,264,(Str.end-Str.start)/total, 8,v);//buffered zone
		queueSprite( 0,32, 8, 8,0,264,480, 8,v);//buffer bar
		queueSprite(16,32, 8, 8,(Str.curr/total)-4,264,8, 8,v);//current position
	}
}
char*draw(int mode){
	if(!ready)init();
	if(mode&1){//wall
		sceGuStartList(GU_DIRECT);
		sceGuTexImage(0,bg.tbw,bg.h,bg.tbw,bg.p);
		sceGuTexSync();
		if(bg.tbw<512)
			sceGuClear(GU_COLOR_BUFFER_BIT);
		else
			sceGuCopyImage(GU_PSM_8888,32,0,480,272,512,bg.p,0,0,ot->lcd->size,getVramDrawBuffer());
		sceGuFinish();
		sceGuSync(GU_SYNC_FINISH,GU_SYNC_WHAT_DONE);
	}
	if(mode&2){//interface
		sceGuStartList(GU_DIRECT);
		if(ot->dmx){//just draw the buffer bar while on playback
			sceGuTexImage(0,bg.tbw,bg.h,bg.tbw,bg.p);
			sceGuTexMode(GU_PSM_8888,0,0,0);
			Vertex*v=sceGuGetMemory(2*3*sizeof(Vertex));n=0;//shadow
			drawBuffer(v);
			sceGuTexSync();
			sceGuDrawArray(GU_SPRITES,GU_TEXTURE_16BIT|GU_VERTEX_16BIT|GU_TRANSFORM_2D,n,0,v);
		}else{
			setCur();
			setSta();
			setList();
			setPanel();
			setTopBar();
			setScrollBar();
			intraFontSetStyle(ltn,1.0f,lst.color|(lst.pos<<24),lst.shadow|(lst.pos<<24),0);
				for(int i=0,y=lst.y;(i<lst.len)&&(y<480);i++,y+=12)
					intraFontPrint(ltn,lst.x,y,lst.p[i]);//result/file list
			sceGuTexImage(0,bg.tbw,bg.h,bg.tbw,bg.p);
			sceGuTexMode(GU_PSM_8888,0,0,0);
			sceGuTexSync();
			Vertex*v=sceGuGetMemory(2*16*sizeof(Vertex));n=0;//shadow
			queueSprite( 0, 0, 1, 1,  0, 0,480,top.pos-13,v);//top
			queueSprite( 0, 0, 1, 1,  0, top.pos-13,scr.pos,272+13-top.pos-sta.pos,v);// left
			queueSprite( 0, 0, 1, 1,495-pan.pos, top.pos-13,pan.pos,272+13-top.pos-sta.pos,v);// right
			queueSprite( 0, 0, 1, 1,  0,272-sta.pos,480,sta.pos,v);// bottom
			queueSprite( 0, 0,15,13,  0+scr.pos,-13+top.pos,15,13,v);// top left
			queueSprite(15, 0, 2,13, 15+scr.pos,-13+top.pos,465-pan.pos-scr.pos,13,v);//title bar bg
			queueSprite(17, 0,15,13,480-pan.pos,-13+top.pos,15,13,v);// top right
			queueSprite( 0,14,15, 2,  0+scr.pos,  0+top.pos,15,242+13-top.pos-sta.pos,v);//scroll bg
			queueSprite(17,14,15, 2,480-pan.pos,  0+top.pos,15,242+13-top.pos-sta.pos,v);//menu bg
			queueSprite( 0,15,15,17,  0+scr.pos,255-sta.pos,15,17,v);// bottom left
			queueSprite(15,15, 2,17, 15+scr.pos,255-sta.pos,465-pan.pos-scr.pos,17,v);//??? bg
			queueSprite(17,15,15,17,480-pan.pos,255-sta.pos,15,17,v);// bottom right
			if(cur.visible==1)queueSprite( 0,80,16,16,cur._x,cur._y,16,16,v);//cursor
			drawBuffer(v);
			sceGuDrawArray(GU_SPRITES,GU_TEXTURE_16BIT|GU_VERTEX_16BIT|GU_TRANSFORM_2D,n,0,v);

			intraFontSetStyle(ltn,1.0f,0xFFFFFFFF,0x00000000,0);
			intraFontPrint(ltn,15,top.pos-16,top.title);//title
			if(pan.visible)
				for(int i=0,y=(272-pan.len*20)/2;i<pan.len;i++,y+=20)
					intraFontPrint(ltn,500-pan.pos,y,pan.p[i]);//result/file list
		}
		sceGuFinish();
		sceGuSync(GU_SYNC_FINISH,GU_SYNC_WHAT_DONE);
	}
	if(mode&4)sceDisplayWaitVblankStart();
	if(mode&8){//swap
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
	pan.visible=0;
	scr.visible=0;
	lst.visible=1;
	sta.visible=0;//!!ot->sys->err;
	strncpy(dir,CWD,256);
	for(run=1;run;){
		if(Mode!=1){//no draw while playback
			sceDisplayWaitVblankStart();
			continue;
		}
		if(ot->sys->pad !=ot->sys->_pad){
//			if(ot->sys->pad & PSP_CTRL_START)break;
			if(ot->sys->pad & PSP_CTRL_CROSS)select();
			if(ot->sys->pad & PSP_CTRL_CIRCLE){
				updir(dir);
				strcpy(top.title,dir);
				ls(dir);
			}
			if(ot->sys->pad & PSP_CTRL_TRIANGLE){
				pan.visible=!pan.visible;
				if(!pan.p)pan.visible=0;
			}
			if(ot->sys->pad & PSP_CTRL_SQUARE);
			if(ot->sys->pad & PSP_CTRL_SELECT){
//				clean();
//				Open("http://google.com",0,0777);
//				restor();
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
//	Exit();
	return sceKernelExitDeleteThread(0);
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
	run=0;
	sceDisplayWaitVblankStart();
	Free(bg.p);
	sceGuTerm();
	$("gui unload\n");
	sceKernelStartThread(sceKernelCreateThread("arakiri",unload,0x11,0x1000,0,0),0,NULL);//to be unable to return
	return NULL;
}
int  module_start(SceSize args,char*argp){
	if((ot=otGetCtx())<0)return 1;
	$("gui loaded\n");
	ot->gui=&gui;
	Mode=1;
	sceKernelStartThread(sceKernelCreateThread("OpenTube.gui",loop,0x11,0x10000,0,0),0,NULL);
	return 0;
}
//}