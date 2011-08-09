#include "io.h"
#include "main.h"

PSP_MODULE_INFO("OpenTube.io",0,0,0);
static unsigned int  __attribute__((aligned(16))) list[0x40000];
char*UserAgent="OpenTube/2.0 (PSP-1000 5.00)";
int err=0,netInited=0;
//extern int sceKernelGetModel();
int netStop(int lv){
//Printf("stopLv:%i\n",lv);
	if(lv==-1)lv=3;
	if(lv>=3){//Print("stopHTTP\n");
		sceHttpEnd();
		sceUtilityUnloadNetModule(PSP_NET_MODULE_HTTP);
		sceUtilityUnloadNetModule(PSP_NET_MODULE_PARSEHTTP);
		sceUtilityUnloadNetModule(PSP_NET_MODULE_PARSEURI);
	}if(lv>=2){//Print("stopApctl\n");
		sceNetApctlTerm();
		sceNetInetTerm();
	}if(lv>=1){//Print("stopNet\n");
		sceNetTerm();
		sceUtilityUnloadNetModule(PSP_NET_MODULE_INET);
		sceUtilityUnloadNetModule(PSP_NET_MODULE_COMMON);
	}
	netInited=0;
	return err;
}
int netInit(){
	sceUtilityLoadNetModule(PSP_NET_MODULE_COMMON);
	if((err=sceNetInit(0x20000, 42,0x1000, 42,0x1000))<0)return netStop(1);

	sceUtilityLoadNetModule(PSP_NET_MODULE_INET);
	if((err=sceNetInetInit())<0)return netStop(2);
	//if((err=sceNetResolverInit())<0)goto endApctl;
	if((err=sceNetApctlInit(0x8000, 0x30))<0)return netStop(2);
	pspUtilityNetconfData data={{sizeof(pspUtilityNetconfData),-1,1,17,19,18,16,0},PSP_NETCONF_ACTION_CONNECTAP+3,NULL,0,0,0};//3
	if((err=sceUtilityNetconfInitStart(&data))<0)netStop(2);
	if(!ot->gui){
		sceGuInit();
		sceGuStart(GU_DIRECT,list);
		sceGuDrawBuffer(GU_PSM_8888,(void*)0x00000000,512);
		sceGuDispBuffer(480,272,(void*)0x00088000,512);
		sceGuDepthBuffer((void*)0x00110000,512);
		sceGuScissor(0,0,480,272);sceGuEnable(GU_SCISSOR_TEST);
		sceGuClearColor(0xff37352D);
		sceGuFinish();
		sceGuDisplay(GU_TRUE);
	}
	for(int done=0;!done;){
		if(!ot->gui){
			sceGuStart(GU_DIRECT,list);
			sceGuClear(5);
			sceGuFinish();
			sceGuSync(0,0);
		}else Draw(1);
		switch(sceUtilityNetconfGetStatus()){
			case PSP_UTILITY_DIALOG_NONE:break;
			case PSP_UTILITY_DIALOG_VISIBLE:sceUtilityNetconfUpdate(1);break;
			case PSP_UTILITY_DIALOG_QUIT:sceUtilityNetconfShutdownStart();break;
			case PSP_UTILITY_DIALOG_FINISHED:done=1;break;
			default:break;
		}
		if(!ot->gui){
			sceDisplayWaitVblankStart();
			sceGuSwapBuffers();
		}else Draw(2);
	}
	if(data.base.result){
		puts("still not connected");
		return netStop(2);
	}
	sceUtilityLoadNetModule(PSP_NET_MODULE_PARSEURI);
	sceUtilityLoadNetModule(PSP_NET_MODULE_PARSEHTTP);
	sceUtilityLoadNetModule(PSP_NET_MODULE_HTTP);
	if((err=sceHttpInit(0x25800))<0)return netStop(3);
	int model=0,fw=sceKernelDevkitVersion();
	Sudo(4,(int)&model,0);
	UserAgent[18]=model+'1';
	UserAgent[23]=((fw&0x0F000000)>>24)+'0';
	UserAgent[25]=((fw&0x000F0000)>>16)+'0';
	UserAgent[26]=((fw&0x00000F00)>> 8)+'0';
	Print("userAgent:");puts(UserAgent);
	netInited=1;
	return 0;
}
int httpClose(int fd){
	if(fd<0x010101)return fd;
	sceHttpDeleteRequest(FD2REQ(fd));
	sceHttpDeleteConnection(FD2CNX(fd));
	sceHttpDeleteTemplate(FD2TPL(fd));
	Print("httpClose\n");
	return 0;
}
int httpRead(int fd,void*buf,int len){
	if(fd<0x010101)return fd;
	int ret=sceHttpReadData(FD2REQ(fd),buf,len);
	return ret;//sceIoWrite(2,buf,ret);
}
int httpOpen(char* url,int mode,int param){
	char*data=NULL;//(mode>0x8800000)?NULL:(char*)mode;
	if(!netInited)netInit();
	int tpl=0,cnx=0,req=0,ret=0;
	if((tpl=sceHttpCreateTemplate(UserAgent, 1, 1))<0){ret=tpl;goto errTpl;}
	if((ret=sceHttpSetResolveRetry(tpl,2))<0){ret=cnx;goto errCnx;}
	if((ret=sceHttpSetResolveTimeOut(tpl,7*1000000))<0){ret=cnx;goto errCnx;}
	if((ret=sceHttpSetSendTimeOut   (tpl,7*1000000))<0){ret=cnx;goto errCnx;}
	if((ret=sceHttpSetRecvTimeOut   (tpl,7*1000000))<0){ret=cnx;goto errCnx;}
	if((cnx=sceHttpCreateConnectionWithURL(tpl, url, 1))<0){ret=cnx;goto errCnx;}
	if((req=sceHttpCreateRequestWithURL(cnx,data?PSP_HTTP_METHOD_POST:PSP_HTTP_METHOD_GET,url,data?strlen(data):0))<0){ret=req;goto errReq;}
	if((ret=sceHttpSendRequest(req,data,data?strlen(data):0))<0)goto errReq;
//	unsigned long long size;
//	sceHttpGetContentLength(req,&size);
//	Printf("%llu\n",size);
//	sceHttpAddExtraHeader(req,"Content-Range","bytes 0-5",0);
	int fd=(req<<16)|(cnx<<8)|(tpl);
	if(mode==HTTP_SAVE_FILE){
		char*fname=url;
		for(int i=0;url[i];i++)if(url[i]=='/')fname=url+i+1;
		int out=sceIoOpen(fname,PSP_O_CREAT|PSP_O_WRONLY,0777);
		char buf[1024];
		while((ret=sceHttpReadData(req,buf,sizeof(buf)))>0)
			sceIoWrite(out,buf,ret);
		sceIoClose(out);
		httpClose(fd);
		return 0;
	}
	if(mode==HTTP_SAVE_RAM){
		int length=0;
		void*result=Malloc(1024);
		while((ret=sceHttpReadData(req,result+length,1024))>0){
			length+=ret;
			result=Realloc(result,length+1024);
		}
		httpClose(fd);
		*((char**)param)=result;
		//param=(int)result;
		return length;
	}
	
	return fd;
errReq:
	sceHttpDeleteRequest(req);
errCnx:
	sceHttpDeleteConnection(cnx);
errTpl:
	sceHttpDeleteTemplate(tpl);
	return ret;
}
int httpSeek(int fd,int type,int len){
	return 0;
}
int httpBuffer(int fd){
/*
sceKernelCreateVpl ("streamBuffer",PSP_MEMORY_PARTITION_USER,0, unsigned int size, struct SceKernelVplOptParam *opt);void* data;
sceKernelAllocateVpl (SceUID uid, unsigned int size,&data,0);
sceKernelFreeVpl (SceUID uid, void *data);
sceKernelDeleteVpl (SceUID uid);
*/
#define MTU 1024
#define BUFFERMAXLEN 1024*MTU//1Mo
	void*buf=NULL;unsigned bufLen=0;
	while(1){
		if((bufLen+MTU)<BUFFERMAXLEN){//je verifie que je oversize pas le new buffer
			buf=Realloc(buf,(bufLen+=MTU));//je resize le buffer
			httpRead(fd,buf+bufLen-MTU,MTU);//je read @buf+len-MTU
		}else{
			Print("we're run out of capsule up there...\n");
			bufLen=0;
			break;
		}
	}	
	return 0;
}
int myOpen(const char*path,int mode,int flag){
	if(!memcmp(path,"http://",7))return httpOpen((char*)path,mode,flag);
	return sceIoOpen(path,mode,flag);
}
int myRead(int fd,void*p,int flag){
	if(fd<0x010101)httpRead(fd,p,flag);
	return sceIoRead(fd,p,flag);
}
int mySeek(int fd,int type,int len){
	if(fd<0x010101)httpSeek(fd,type,len);
	return sceIoLseek(fd,type,len);
}
int myClose(int fd){
	if(fd>=0x010101)return httpClose(fd);
	return sceIoClose(fd);
}
int module_stop(){
	return 0;
}
int unload(SceSize args,void*argp){
	do sceKernelDelayThread(1000);
	while(sceKernelSelfStopUnloadModule(1,0,NULL)<0);
	return 0;
}
int myUnload(){
	netStop(-1);
	ot->io=NULL;
	Print("IO unload\n");
	sceKernelStartThread(sceKernelCreateThread("arakiri",unload,0x11,0x1000,0,0),0,NULL);//to be unable to return
	return 0;
}
FileSys io={myOpen,myRead,mySeek,myClose,myUnload};
int module_start(SceSize args,char*argp){
	Print("IO loaded\n");
	if(args!=sizeof(OpenTube*))return 1;//return if no context
	ot=(OpenTube*)((u32*)argp)[0];
	ot->io=&io;
	return 0;
}