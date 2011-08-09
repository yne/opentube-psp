#include <pspkernel.h>
#include <malloc.h>
#include <stdio.h>
#include <string.h>
#include <psputility.h>
#include <pspctrl.h>
#include "main.h"
//#include "main.private.h"
#define sw(n) (((n>>24)&0xff)|((n<<8)&0xff0000)|((n>>8)&0xff00)|((n<<24)&0xff000000))

PSP_MODULE_INFO("OpenTube", 0x6,0,0);
PSP_HEAP_SIZE_KB(9*1024);

OpenTube*ot;//OpenTube core context
extern void __psp_free_heap();
int modload(char* path){
//	if(((u32*)path)[0]==0x70747468 && (((u32*)path)[1]&0xFFFFFF)==0x632F2F3A)
	int is_local=strncmp("http://",path,7);
	if(!is_local){
		if((is_local=Open(path,HTTP_SAVE_FILE,0777)))return is_local;
		char*fname=path;
		for(int i=0;path[i];i++)if(path[i]=='/')fname=path+i+1;
		path=fname;
	}
	int ret,modid=sceKernelLoadModule(path, 0,0);
	if(modid>=0)sceKernelStartModule(modid, sizeof(ot), &ot, &ret, NULL);
	if(!is_local)sceIoRemove(path);//removed once unloaded ?!
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
int alert(char*str/*,int lv*/){//0:fatal,1:error,2:warning,3:info
	return sceIoWrite(2,str?str:"(null)",str?strlen(str):6);
}
void* my_malloc(unsigned s){//64
	if(s&0x3f)s+=64-(s&0x3f);
	void* p=memalign(64,s);
	if(p)memset(p,0,s);
//	printf("malloc:%p\n",p);
	return p;//malloc(size);
}
void* my_realloc(void*p,unsigned size){
//	printf("realloc %p,%i\n",p,size);
	return realloc(p,size);
}
void my_free(void*p){
//	printf("free:%p\n",p);
	return free(p);
}
char _id2str[8+1];
char* id2str(u32 id){
	id=sw(id);
	for(int i=0;i<8;i++)
		_id2str[7-i]="0123456789ABCDEF"[(id>>(4*i))&0xF];
	return _id2str;
}
char* findCodec(const char* name,unsigned id){//load a codec from name or id
	int fd,uid=0;
	if((fd=sceIoDopen(CWD))<0)return "bad codec dir";
	SceIoDirent ent;char abs_ent[256]="";
	const char*mime=name?name:id2str(id);
	printf("searching codec for ");puts(mime);
	while(uid<=0 && sceIoDread(fd,&ent)>0)
		if(strstr(ent.d_name,mime))
			uid=modload(strcat(strcat(abs_ent,CWD),ent.d_name));
	return uid>0?NULL:"no codec found";
}
char* play(const char* file){
	char*err=NULL;int mime;
	int fd=sceIoOpen(file,PSP_O_RDONLY,0777);
	sceIoRead(fd,&mime,4);
	if((err=findCodec(NULL,mime)))return(err);
	sceIoClose(fd);
	if(!(ot->sys->onPlay&&ot->sys->onLoad&&ot->sys->onSeek&&ot->sys->onStop))return "missing function";
	puts("ready !");
	if(!(err=ot->sys->onLoad(file)))
		err=ot->sys->onPlay();//blocking
	ot->sys->onStop();
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
		if(pad.Buttons&PSP_CTRL_START
		&& pad.Buttons&PSP_CTRL_SELECT
		&& pad.Buttons&PSP_CTRL_LTRIGGER
		&& pad.Buttons&PSP_CTRL_RTRIGGER)Exit();
	}
	return 0;
}
int module_stop(SceSize args, char *argp){
	__psp_free_heap();
	puts("core unload");
	return 0;
}
int stop(){
	if(ot->sys->onStop)ot->sys->onStop();//stop current playing
	if(!ot->io)puts("no IO");
	//	if(modGui>0)modstun(modGui);
	if(ot->io&&ot->io->unload)ot->io->unload();else puts("no OI unload CB");
	sceKernelTerminateDeleteThread(padTh);
	sceUtilityUnloadAvModule(0);
	module_stop(0,NULL);
	if(CWD[0]=='m')sceKernelExitGame();//ms0
	sceKernelSelfStopUnloadModule(1,0,NULL);//host0
	return 0;
}
int start(SceSize args,void*argp){
	if(!(ot->me->pool=memalign(0x400000,0x400000)))return -1;//first malloc
	ot->me->bootNid=0x051C1601;
	sceIoChdir(CWD);
	sceUtilityLoadAvModule(0);
	sceKernelStartThread((padTh=sceKernelCreateThread("OpenTube.ctrl",pad,0x11,0x10000,0,0)),0,NULL);
	modload("io.prx");
	char*result=NULL;
	Open("http://gdata.youtube.com/feeds/api/videos?q=Djmax%20BGA&start-index=1&max-results=20&v=1",HTTP_SAVE_RAM,(int)&result);

//	modload("gui.prx");
//	modload("http://opentube-psp.googlecode.com/files/test.prx");
	sceKernelExitDeleteThread(0);
	return 0;
}
MeCtx me;
CoreSys sys={NULL,"",0,stop,modstun,modload,alert,my_free,my_realloc,my_malloc,sudo,findCodec,play};
Display lcd={1,0x44000000,0x44000000,512,3,0};//PSP_DISPLAY_PIXEL_FORMAT_8888
OpenTube Ot={&sys,&lcd,NULL,NULL,NULL,&me};//OpenTube core context
int module_start(SceSize args,char*argp){
	if(!args)return 1;
	ot=&Ot;
	char *argv[16+1];int argc=0;
	for(int i=0;(i+=strlen(argv[argc++]=&((char*)argp)[i])+1)<args;);
	for(int i=strlen(argv[0])-1;i;i--)if(argv[0][i]=='/'){argv[0][i+1]=0;break;};
	memcpy(CWD,argv[0],256);
	return sceKernelStartThread(sceKernelCreateThread("OpenTube.bootstrap",start,0x11,0x10000,0,0),args,argp);
}
