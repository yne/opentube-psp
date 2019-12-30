#include <pspkernel.h>
#include <pspwlan.h>
#include <pspnet.h>
#include <pspnet_inet.h>
#include <pspnet_apctl.h>
#include <pspnet_resolver.h>
#include <psphttp.h>
#include <psputility.h>
#include <string.h>
#include "gui.h"
#include "graphic.h"
#include "youtube.h"
#include "core.h"

#define ISDIR(stat)(stat.d_stat.st_mode==11C0)
#define MIN(A,B) (A<B?A:B)
#define MAX(A,B) (A>B?A:B)
//#define EXT(name,len,A,B,C) ((name[len-4]=='.')&&(name[len-3]==A)&&(name[len-2]==B)&&(name[len-1]==C))
PSP_MODULE_INFO("OpenTube.gui",0,0,0);
Graphic gui={init,stop,draw,edit};
/*api*/
int  isExt(char* name,int len,char A,char B,char C){
	return ((name[len-4]=='.')&&(name[len-3]==A)&&(name[len-2]==B)&&(name[len-1]==C));
}
void msgbox(char*header,char*message,char*footer){
	guiSetBackColor(0x222222);
	guiSetTextColor(0xDDDDDD);
	guiSetXY(0,33);
	guiPuts("$");
	for(int i=0;i<66;i++)guiPuts(" ");
	guiSetXY(1,33);
	if(header)
		guiPuts(header),
		guiPuts(":");
	if(message)
		guiPuts(message);
	if(footer)
		guiSetXY(67-strlen(footer),33),
		guiPuts(footer);
}
int  info(char*msg){
	msgbox(0,msg,0);
	return 0;
}
int  alert(char*msg){
	msgbox(0,msg,"O,X=continue");
	while(1){
		sceDisplayWaitVblankStart();
		if(_Pad==Pad||_Pad>Pad)continue;
		if(Pad&PSP_CTRL_CIRCLE||Pad&PSP_CTRL_CROSS)break;
	}
	msgbox(0,0,0);
	return 0;
}
int  confirm(char*msg){
	msgbox(msg,"?","\2=yes \1=no");
	while(1){
		sceDisplayWaitVblankStart();
		if(_Pad==Pad||_Pad>Pad)continue;
		if(Pad&PSP_CTRL_CIRCLE||Pad&PSP_CTRL_CROSS)break;
	}
	msgbox(0,0,0);
	return Pad&PSP_CTRL_CROSS;
}
char*oskmap[]={
	",acb.dfe!gih-jlk \x8\x2\x1qop.trs.wuv.yxz.",//default
	"ABCDEFGHIJKLOPMN \x8\x2\x1QOP.TRS.WUV.YXZ.",//L
	"0123456789+-*/()[]{}~&|[]|=+-*/????+",//R
	"???????????????????????????????????+"};//L+R
void drawosk(int mode,int pos){
//	guiEnableBackColor(0);
//	for(int i=0;i<255;i++)guiPuts(&i);
	guiSetTextColor(0xFFFFFFFF);
	for(int i=0,_x=414,_y=216,x=0,y=0;i<9;i++,x=3*(i%3),y=2*(i/3)){
		guiSetBackColor((pos==i?0x666666:0x222222));
		//if((i+2)%3==0)y--;else if((i+1)%3==0)x--;else if((i+3)%3==0)x++;//compact
		guiPutc(_x+6*(x+1),_y+8*(y+0),-1,oskmap[mode][4*i+0]);
		guiPutc(_x+6*(x+0),_y+8*(y+1),-1,oskmap[mode][4*i+1]);
		guiPutc(_x+6*(x+1),_y+8*(y+1),-1,oskmap[mode][4*i+2]);
		guiPutc(_x+6*(x+2),_y+8*(y+1),-1,oskmap[mode][4*i+3]);
	}
}
char*prompt(char*msg,char*def){
	drawosk(0,4);
	int len=def?strlen(def):0;
	char*res=Malloc(len+1);
	if(def)memcpy(res,def,len);
	res[len]=0;
	do{
		msgbox(msg,res,"");
		sceDisplayWaitVblankStart();
		if(_Pad==Pad)continue;
		char c=0,mode=!!(Pad&PSP_CTRL_LTRIGGER)+2*!!(Pad&PSP_CTRL_RTRIGGER),pos=4+!!(Pad&PSP_CTRL_RIGHT)-!!(Pad&PSP_CTRL_LEFT)+3*!!(Pad&PSP_CTRL_DOWN)-3*!!(Pad&PSP_CTRL_UP);
		drawosk(mode,pos);
		if(Pad&PSP_CTRL_TRIANGLE&&!(_Pad&PSP_CTRL_TRIANGLE))c=oskmap[mode+0][4*pos+0];
		if(Pad&PSP_CTRL_SQUARE  &&!(_Pad&PSP_CTRL_SQUARE)  )c=oskmap[mode+0][4*pos+1];
		if(Pad&PSP_CTRL_CIRCLE  &&!(_Pad&PSP_CTRL_CIRCLE)  )c=oskmap[mode+0][4*pos+2];
		if(Pad&PSP_CTRL_CROSS   &&!(_Pad&PSP_CTRL_CROSS)   )c=oskmap[mode+0][4*pos+3];
		switch(c){
			case 0:break;//key presed
			case 1:msgbox(0,0,0);return res;//ok
			case 2:msgbox(0,0,0);Free(res);return NULL;//cancel
			case 8:if(--len<0)len=0;break;//backspace
			default:
				res=Realloc(res,++len+1);
				res[len-1]=c;
		}
		res[len]=0;
	}while(1);
}
/*core*/
int  ls(Entry**list){
	SceIoDirent ent;
	int fd=sceIoDopen(".");
	int len=sceIoDread(fd,&ent);
	*list=Realloc(*list,(len+1)*sizeof(Entry));
	memset(*list,0,(len+1)*sizeof(Entry));
	(*list)[len].title[0]=0;
	for(int i=0;i<len;i++)
		strcpy((*list)[i].title,ent.d_name),
		strcpy((*list)[i].desc,ent.d_stat.st_mode&0x1000?"dir.":"file"),
		sceIoDread(fd,&ent);
	sceIoDclose(fd);
	return len;
}
void drawdir(Entry*dirlst,int dirlen,int n){
	guiSetBackColor(0x222222);
	guiSetTextColor(0x888888);
	guiSetXY(0,0);
	guiPuts("File explorer\n");
	if(!dirlst)return;
	for(int i=0;i<32;i++){//draw cwd
		Entry*e=&dirlst[(i+n)%dirlen];
		if(i==16){
			guiSetBackColor(0x333333);
			guiSetTextColor(0xFFFFFF);
			guiPuts(e->title);
			if(e->desc)
				guiSetTextColor(0x888888),
				guiPuts(" - "),
				guiPuts(e->desc);
			if(e->date[0]||e->size[0]||e->uploader[0]||e->view[0]||e->favorite[0]||e->rate[0]){
				guiPuts("\n ");
				if(e->date[0])
					guiPuts(e->date),guiPuts(" ");
				if(e->size[0])
					guiPuts(e->size),guiPuts("b ");
				if(e->uploader[0])
					guiPuts(e->uploader),guiPuts("\x13 ");
				if(e->view[0])
					guiPuts(e->view),guiPuts("\x07 ");
				if(e->favorite[0])
					guiPuts(e->favorite),guiPuts("\x1F ");
				if(e->rate[0])
					guiPuts(e->rate),guiPuts("%");
			}
		}else{
			guiSetBackColor(0x444444);
			guiSetTextColor(e->desc[0]=='d'?0xAAAA00:0xAA00AA);
			guiPuts(e->title);
		}
		guiPuts("\n");
	}
}
int  loop(SceSize args,void*argp){
	Entry*dirlst=NULL;int dirlen=0,n=0;
	sceIoChdir(CWD);
	msgbox(NULL,"OpenTube txtUI build<"__DATE__" "__TIME__">",NULL);
	dirlen=ls(&dirlst);
	drawdir(dirlst,dirlen,n);
	do{
		sceDisplayWaitVblankStart();
		if((Mode=!1)||(_Pad==Pad)||(_Pad>Pad))continue;
		if(Pad&PSP_CTRL_CIRCLE){
			if(confirm("exit")){
				Exit();
				sceKernelExitDeleteThread(0);
			}
		}
		if(Pad&PSP_CTRL_TRIANGLE){
			alert("on va tester tout ca...");
			if(!confirm("tester prompt"))continue;
			char* res=prompt("c'est bon ?","nop");
			if(!res)alert("annulation");
			else if(res[0]=='o')alert("alors c cool");
			else alert("tempis");
			Free(res);
			continue;
		}
		if(Pad&PSP_CTRL_CROSS){
			if(dirlst[(n+16)%dirlen].id[0]){
				youtube_play(dirlst[(n+16)%dirlen].id);
			}else if(dirlst[(n+16)%dirlen].desc[0]=='d'){
				if(!sceIoChdir(dirlst[(n+16)%dirlen].title))
					dirlen=ls(&dirlst);
			}else{
				char*file=dirlst[(n+16)%dirlen].title;
				int len=strlen(file);
				if(isExt(file,len,'f','n','t')){
					int fd=sceIoOpen(file,PSP_O_RDONLY,0777);
					sceIoRead(fd,guiGetFont(),2048);
					sceIoClose(fd);
					continue;
				}else if(isExt(file,len,'a','d','n')){//addon
					
				}else{
					Play(file);
					guiInitEx((void*)ot->lcd->draw,ot->lcd->type, 1);
				}
			}
		}
		if(Pad&PSP_CTRL_START){
			Entry*r=youtube_search(prompt("youtube","test"));
			if(r)Free(dirlst),dirlst=r;
		}
		if(Pad&PSP_CTRL_UP)if(--n<0)n=dirlen;
		if(Pad&PSP_CTRL_DOWN)n++;
		if(Pad&PSP_CTRL_LEFT)n=16;
		drawdir(dirlst,dirlen,n);
	}while(1);
	return 0;
}
extern unsigned char*msx[];
/*net*/
int netInit(){
	while(!sceWlanDevIsPowerOn())
		sceKernelDelayThread(1000*500);
	sceUtilityLoadNetModule(PSP_NET_MODULE_COMMON);
	sceUtilityLoadNetModule(PSP_NET_MODULE_INET);
	int err,_state=-1;
	if((err=sceNetInit(0x20000, 0x20, 0x1000, 0x20, 0x1000)))return err;
	if((err=sceNetInetInit()))return err;
	if((err=sceNetResolverInit()))return err;
	if((err=sceNetApctlInit(0x1400, 0x42)))return err;
	for(int i=1;;i++){
		if(sceUtilityCheckNetParam(i))return err;
		char name[100],state_str[]="state:?";
		sceUtilityGetNetParam(i, 0, (netData*)name);
		info(name);
		if((err=sceNetApctlConnect(i)))return err;	
		while(1){
			int state;
			sceDisplayWaitVblankStart();
			if((err=sceNetApctlGetState(&state)))return err;
			if(state==_state)continue;
			_state=state;
			state_str[6]=state+'0';
			info(state_str);
			if(state==4)return 0;//connected
			if(!state)break;//try next one
		}
	}
}
int netTh(SceSize args,void*argp){
	if(netInit()){
		info("offline");
	}else{
		sceUtilityLoadNetModule(PSP_NET_MODULE_PARSEURI);
		sceUtilityLoadNetModule(PSP_NET_MODULE_PARSEHTTP);
		sceUtilityLoadNetModule(PSP_NET_MODULE_HTTP);
		sceHttpInit(0x25800);
		info("online\1");
	}
	sceKernelExitDeleteThread(0);
	return 0;
}
/*mandatory*/
int  unload(SceSize args,void*argp){
	sceHttpEnd();
	sceUtilityUnloadNetModule(PSP_NET_MODULE_HTTP);
	sceUtilityUnloadNetModule(PSP_NET_MODULE_PARSEHTTP);
	sceUtilityUnloadNetModule(PSP_NET_MODULE_PARSEURI);
	if(sceNetApctlTerm())$("apctl\n");
	if(sceNetInetTerm())$("inet\n");
	if(sceNetTerm())$("net\n");
	sceUtilityUnloadNetModule(PSP_NET_MODULE_INET);
	sceUtilityUnloadNetModule(PSP_NET_MODULE_COMMON);
	do sceKernelDelayThread(1000);
	while(sceKernelSelfStopUnloadModule(1,0,NULL)<0);
	return 0;
}
int  module_stop(SceSize args,char*argp){
	$("gui stop\n");
	return 0;
}
int  module_start(SceSize args,char*argp){
	if((ot=otGetCtx())<0)return 1;
	$("gui loaded\n");
	ot->gui=&gui;
	Mode=1;
	return 0;
}
char*init(){
	guiInitEx((void*)ot->lcd->draw,ot->lcd->type, 1);
	guiSetBackColor(0x444444);
	guiSetTextColor(0xFFFFFF);
	guiSetClearLine(1);
//	guiSetMaxY(32);
	guiClear();
	TH("OpenTube.netInit",netTh);
	return TH("OpenTube.gui",loop)<0?"":0;
}
char*stop(){
	return TH("arakiri",unload)<0?"":0;//to be unable to return
}
char*draw(int mode){
	return NULL;
}
char*edit(int what,void*arg){
	return 0;
}

