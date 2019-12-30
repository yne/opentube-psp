#include <pspkernel.h>
#include <stdio.h>
#include <string.h>
#include <psputility.h>
#include <pspctrl.h>
#include "core.h"
#include "io.h"
#include "gui.h"
//#include "main.private.h"
#define sw(n) (((n>>24)&0xff)|((n<<8)&0xff0000)|((n>>8)&0xff00)|((n<<24)&0xff000000))

PSP_MODULE_INFO("OpenTube", 0x6,0,0);
PSP_HEAP_SIZE_KB(9*1024);

OpenTube*ot;//OpenTube core context
extern void __psp_free_heap();
char _id2str[8+1];
char* id2str(u32 id){
	id=sw(id);
	for(int i=0;i<8;i++)
		_id2str[7-i]="0123456789ABCDEF"[(id>>(4*i))&0xF];
	return _id2str;
}
int Strlen(char*str){
	int i;for(i=0;str[i]!=0 && str[i]!=255;i++);
	return i;
}
int modload(char* path){
	int is_local=strncmp("http://",path,7);
	if(!is_local){
		if((is_local=Open(path,HTTP_SAVE_FILE,0777)))return is_local;
		char*fname=path;
		for(int i=0;path[i];i++)if(path[i]=='/')fname=path+i+1;
		path=fname;
	}
	int ret,modid=sceKernelLoadModule(path,0,0);
	if(modid>=0)sceKernelStartModule(modid,0,NULL, &ret, NULL);
	if(!is_local)sceIoRemove(path);//removed once unloaded ?!
	if(modid<0)Alert(id2str(modid));
	return modid;
}
char* sudo(int mode,u32 nid,int param){
	int ret,mod=sceKernelLoadModule("kernel.prx",0,0);
	sudoArg arg={mode,nid,param};
	if(mod>0)sceKernelStartModule(mod,3*4,&arg,&ret,NULL);
	return NULL;
}
int modstun(u32 modid){
	if(modid<0)return modid;
	int status=sceKernelStopModule(modid, 0, 0, &status, NULL);
	return sceKernelUnloadModule(modid);
}
int outfd=0;
int print(char*str,int lv){//0:fatal,1:error,2:alert,3:debug
	if(!str)return sceIoWrite(2,"(null)",6);
	char out[256];
	char*pre=NULL,*su=NULL;
	switch(lv){//todo mix all space content \n in a char tmp[]
		case 0 :pre=""; su="\n";break;
		case 1 :pre=" ";su="\n";break;
		case 2 :pre="  "; su="";break;
		case 3 :pre="   ";su="";break;
		default:pre="";su="";
	}
	strcat(strncat(strcpy(out,pre),str,250),su);
	sceIoWrite(outfd,out,Strlen(out));
	return 0;
}
void* my_malloc(unsigned s){//64
	void* p=malloc(s);
//	printf("malloc(%i):%p\n",s,p);
	return p;
}
void* my_memalign(unsigned bit,unsigned s){//64
//	if(s&(bit-1))s+=bit-(s&(bit-1));//round up
	void* p=memalign(bit,s);
//	printf("memalign%i:%p\n",bit,p);
	return p;
}
void* my_realloc(void*p,unsigned size){
//	printf("realloc %p,%i\n",p,size);
	return realloc(p,size);
}
void my_free(void*p){
//	printf("free:%p\n",p);
	return free(p);
}
extern char*mp4Init();
extern char*avc1Init();
extern char*mp4aInit();
char* findCodec(char* name,unsigned id){//load a codec from name or id
	Alert("findcodec:");
	if(id&&!(id&0xFFFFFF))return mp4Init();
	if(id==0x61766331)return avc1Init();
	if(id==0x6D703461)return mp4aInit();
	return "no codec found";
/*
	int fd,uid=0;char*err=NULL;
	if((fd=sceIoDopen(CWD))<0)return "bad codec dir";
	SceIoDirent ent;char abs_ent[256]="";
	char*mime=name?name:id2str(id);
	Alert("searching codec for");Alert(mime);$("\n");
	while(uid<=0 && sceIoDread(fd,&ent)>0)
		if(strstr(ent.d_name,mime))
			uid=modload(strcat(strcat(abs_ent,CWD),ent.d_name));
	sceIoDclose(fd);
	if(uid<=0){err="no codec found";Error(err);}
	return err;*/
}
char* play(char* file){
	char*err=NULL;int mime;
	if(ot->dmx)return "prev dmx still alive";
	int fd=Open(file,PSP_O_NOWAIT|PSP_O_RDONLY,0777);
	Read(fd,&mime,4);
	if((err=findCodec(NULL,mime)))return(err);
	if(!(err=ot->dmx->load(file)))
		err=ot->dmx->play();//blocking
	ot->dmx->stop();
	Close(fd);
	return err;
}
int padTh;
int pad(SceSize args,void*argp){
	SceCtrlData pad,_pad;
	sceCtrlSetSamplingCycle(0);
	sceCtrlSetSamplingMode(1);
	sceCtrlReadBufferPositive(&_pad,1);
	while(1){
		sceCtrlReadBufferPositive(&pad,1);
		ot->sys->pad=pad.Buttons;
		ot->sys->_pad=_pad.Buttons;
		_pad.Buttons=pad.Buttons;
		if(pad.Buttons!=_pad.Buttons && pad.Buttons&PSP_CTRL_CROSS && ot->dmx->play)
			ot->dmx->play();//pause
		if(pad.Buttons&PSP_CTRL_START
		&& pad.Buttons&PSP_CTRL_SELECT
		&& pad.Buttons&PSP_CTRL_LTRIGGER
		&& pad.Buttons&PSP_CTRL_RTRIGGER)
			Exit();
	}
	return 0;
}
int module_stop(SceSize args, char *argp){
	__psp_free_heap();
	$("core unload\n");
	return 0;
}
int stop(){
	if(ot->dmx)ot->dmx->stop();//stop current playing
	if(ot->gui)ot->gui->stop();//if(modGui>0)modstun(modGui);
	ot->io->unload();
	sceKernelTerminateDeleteThread(padTh);
	sceIoClose(outfd);
	if(CWD[0]=='h')sceKernelSelfStopUnloadModule(1,0,NULL);//host0
	sceKernelExitGame();//ms0
	return 0;
}
int start(SceSize args,void*argp){
	if(!(ot->me->pool=Memalign(0x400000,0x200000)))return -1;//first malloc
	sceIoChdir(CWD);
	outfd=CWD[0]=='h'?2:sceIoOpen("out",PSP_O_WRONLY|PSP_O_CREAT|PSP_O_TRUNC,0777);
	sceKernelStartThread((padTh=sceKernelCreateThread("OpenTube.ctrl",pad,0x20,0x10000,0,0)),0,NULL);
	Alert("openTube<"__DATE__" "__TIME__">\n");
	ioInit();
//	modload("gui.prx");
/*
	char url[1024];
	int fd =sceIoOpen("playme",PSP_O_RDONLY,0777);
	if(fd>0)url[sceIoRead(fd,url,1023)]=0;
	sceIoClose(fd);
*/
	char*err=Play("res/480.270.bb.mp4");
	
//	if(err)Alert(err);
	
//	Exit();
//	sceKernelStartThread(sceKernelCreateThread("OpenTube.gui",gui,0x20,0x10000,0,0),args,argp);
	sceKernelExitDeleteThread(0);
	return 0;
}
MeCtx me;
CoreSys sys={NULL,"",0,stop,modstun,modload,print,my_free,my_realloc,my_memalign,my_malloc,sudo,findCodec,play};
Display lcd={1,0x44000000,0x44088000,512,PSP_DISPLAY_PIXEL_FORMAT_565,0};//PSP_DISPLAY_PIXEL_FORMAT_8888
OpenTube Ot={&sys,&lcd,NULL,NULL,NULL,&me};//OpenTube core context
OpenTube*otGetCtx(){return &Ot;}
int module_start(SceSize args,char*argp){
	if(!args)return 1;
	ot=&Ot;
	char *argv[16+1];int argc=0;
	for(int i=0;(i+=Strlen(argv[argc++]=&((char*)argp)[i])+1)<args;);
	for(int i=Strlen(argv[0])-1;i;i--)if(argv[0][i]=='/'){argv[0][i+1]=0;break;};
	memcpy(CWD,argv[0],256);
	return sceKernelStartThread(sceKernelCreateThread("OpenTube.bootstrap",start,0x20,0x10000,0,0),args,argp);
}