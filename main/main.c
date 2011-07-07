#include <pspkernel.h>
#include <malloc.h>
#include <stdio.h>
#include <string.h>
#include <psputility.h>
#include "main.h"
#define sw(n) (((n>>24)&0xff)|((n<<8)&0xff0000)|((n>>8)&0xff00)|((n<<24)&0xff000000))

PSP_MODULE_INFO("OpenTube", PSP_MODULE_USER,0,0);
PSP_HEAP_SIZE_KB(9*1024);

CoreSys sys;
Display lcd={1,0x44000000,0x44000000,512,3,0};//PSP_DISPLAY_PIXEL_FORMAT_8888
MeCtx me;
OpenTube ot={&sys,&lcd,NULL,NULL,NULL,&me};//OpenTube core context
int modload(const char* path){
	int ret,modid=sceKernelLoadModule(path, 0,0);
	OpenTube*tmp=&ot;
	if(modid>=0)sceKernelStartModule(modid, sizeof(&ot), &tmp, &ret, NULL);
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
void dlSync(int fd,unsigned offset){
//	if(ot.dl<0)return;
//	while(offset>ot.dl)
//		sceKernelDelayThread(1000*250);
//	puts("sync");
}
/*
int pad(){
	SceCtrlData pad;
	sceCtrlPeekBufferPositive(&pad, 1);
	return(pad.Buttons);
}
*/
void* my_malloc(unsigned s){//64
//	printf("malloc:\n");
	if(s&0x3f)s+=64-(s&0x3f);
	void* p=memalign(64, s);
	if(p)memset(p,0,s);
	return p;//malloc(size);
}
void* my_realloc(void*p,unsigned size){
//	printf("realloc %p,%i\n",p,size);
	return realloc(p,size);
}
void my_free(void*p){
//	printf("free\n");
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
	if((fd=sceIoDopen("/"))<0)return "bad codec dir";
	SceIoDirent ent;
	const char*mime=name?name:id2str(id);
	printf("searching codec for ");puts(mime);
	while(uid<=0 && sceIoDread(fd,&ent)>0)
		if(strstr(ent.d_name,mime))
			uid=modload(ent.d_name);
	return uid>0?NULL:"no codec found";
}
char* findDemux(const char* file,const char* dir){//load a demuxer from 'dir' for 'file'
	if(!file)return "bad argument";
	if(!dir)dir="/";
	int codec,fd;
	if((fd=sceIoOpen(file,PSP_O_RDONLY,0777))<0)return "file not found";
	if(sceIoRead(fd,&codec,sizeof(codec))!=sizeof(codec))return "bad file size";
	sceIoClose(fd);
	char*mime=id2str(codec);
	printf("searching demux for ");puts(mime);
	if((fd=sceIoDopen(dir))<0)return "bad demux dir";
	SceIoDirent ent;
	while(!ot.dmx&&sceIoDread(fd,&ent)>0)
		if(strstr(ent.d_name,mime))modload(ent.d_name);
	sceIoDclose(fd);
	return (ot.dmx)?NULL:"no demuxer found";
}
char* Play(const char* file){
	char*err=NULL;
	if((err=findDemux(file,NULL)))return err;
	if(!(ot.sys->onPlay&&ot.sys->onLoad&&ot.sys->onSeek&&ot.sys->onStop))return "missing function";
	puts("ready !");
	if(!(err=ot.sys->onLoad(file)))
		err=ot.sys->onPlay();//blocking
	ot.sys->onStop();
	return err;
}
int start(SceSize args,void*argp){
	sceIoChdir(ot.sys->cwd);
	if(!(ot.me->pool=memalign(0x400000,0x400000)))return -1;//first malloc
	ot.me->bootNid=0x051C1601;
	sceUtilityLoadAvModule(0);
	//sudo(SUDO_STARTME,0x051C1601,5);
	//sudo(SUDO_SETFREQ,0x9DB844C6,44100);
	if((ot.sys->err=Play("360p.flv")))puts(ot.sys->err);
//	if((ot.sys->err=Play("test.mp4")))puts(ot.sys->err);
	puts("core unload");
	sceUtilityUnloadAvModule(0);
	__psp_free_heap();
	if(ot.sys->cwd[0]=='m')sceKernelExitGame();//ms0
	sceKernelSelfStopUnloadModule(1,0,NULL);//host0
	return 0;
}
int module_start(SceSize args,char*argp){
	if(!args)return 1;
	char *argv[16+1];int argc=0,i=0;
	while((i+=strlen(argv[argc++]=&((char*)argp)[i])+1)<args);
	for(i=strlen(argv[0])-1;i;i--)if(argv[0][i]=='/'){argv[0][i+1]=0;break;};
	memcpy(ot.sys->cwd,argv[0],256);
	ot.sys->malloc=my_malloc;
	ot.sys->realloc=my_realloc;
	ot.sys->free=my_free;
	ot.sys->sudo=sudo;
	ot.sys->findCodec=findCodec;
	ot.sys->modload=modload;
	ot.sys->modstun=modstun;
	return sceKernelStartThread(sceKernelCreateThread("main",start,0x11,0x10000,0,0),args,argp);
}
int module_stop(SceSize args, char *argp){
	return 0;
//	sceKernelSelfStopUnloadModule(1,0,NULL);
}