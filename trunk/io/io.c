#include "io.h"
#include "main.h"

PSP_MODULE_INFO("OpenTube.io",0,0,0);

int netStop(int lv){
//printf("stopLv:%i\n",lv);
	if(lv==-1)lv=3;
	if(lv>=3){//print("stopHTTP\n");
		sceHttpEnd();
		sceUtilityUnloadNetModule(PSP_NET_MODULE_HTTP);
		sceUtilityUnloadNetModule(PSP_NET_MODULE_PARSEHTTP);
		sceUtilityUnloadNetModule(PSP_NET_MODULE_PARSEURI);
	}if(lv>=2){//print("stopApctl\n");
		sceNetApctlTerm();
		sceNetInetTerm();
	}if(lv>=1){//print("stopNet\n");
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
	
	pspUtilityNetconfData data={{sizeof(pspUtilityNetconfData),-1,1,17,19,18,16,0},PSP_NETCONF_ACTION_CONNECTAP,NULL,0,0,0};//3
	if((err=sceUtilityNetconfInitStart(&data))<0)netStop(2);
	for(int done=0;!done;){
		Draw(1);
		switch(sceUtilityNetconfGetStatus()){
			case PSP_UTILITY_DIALOG_NONE:break;
			case PSP_UTILITY_DIALOG_VISIBLE:sceUtilityNetconfUpdate(1);break;
			case PSP_UTILITY_DIALOG_QUIT:sceUtilityNetconfShutdownStart();break;
			case PSP_UTILITY_DIALOG_FINISHED:done=1;break;
			default:break;
		}
		Draw(2);
	}
	if(data.base.result){
		puts("still not connected");
		return netStop(2);
	}
	sceUtilityLoadNetModule(PSP_NET_MODULE_PARSEURI);
	sceUtilityLoadNetModule(PSP_NET_MODULE_PARSEHTTP);
	sceUtilityLoadNetModule(PSP_NET_MODULE_HTTP);
	if((err=sceHttpInit(0x25800))<0)return netStop(3);
	netInited=1;
	return 0;
}
int httpClose(int fd){
	if(fd<0x010101)return fd;
	sceHttpDeleteRequest(FD2REQ(fd));
	sceHttpDeleteConnection(FD2CNX(fd));
	sceHttpDeleteTemplate(FD2TPL(fd));
	print("httpClose\n");
	return 0;
}
int httpRead(int fd,void*buf,int len){
	if(fd<0x010101)return fd;
	int ret=sceHttpReadData(FD2REQ(fd),buf,len);
	return sceIoWrite(2,buf,ret);
}
int httpOpen(char* url,char*data,int param){
	if(!netInited)netInit();
	int tpl=0,cnx=0,req=0,ret=0;
	if((tpl=sceHttpCreateTemplate("PSP-InternetRadio/1.0", 1, 1))<0){ret=tpl;goto errTpl;}
	if((ret=sceHttpSetResolveRetry(tpl,0))<0){ret=cnx;goto errCnx;}
	if((ret=sceHttpSetResolveTimeOut(tpl,1000000*((param>>6)&7)))<0){ret=cnx;goto errCnx;}
	if((ret=sceHttpSetSendTimeOut(tpl,1000000*((param>>3)&7)))<0){ret=cnx;goto errCnx;}
	if((ret=sceHttpSetRecvTimeOut(tpl,1000000*((param>>0)&7)))<0){ret=cnx;goto errCnx;}
	if((cnx=sceHttpCreateConnectionWithURL(tpl, url, 1))<0){ret=cnx;goto errCnx;}
	if((req=sceHttpCreateRequestWithURL(cnx,data?PSP_HTTP_METHOD_POST:PSP_HTTP_METHOD_GET,url,strlen(data)))<0){ret=req;goto errReq;}
	if((ret=sceHttpSendRequest(req,data,data?strlen(data):0))<0)goto errReq;
	unsigned long long size;
	sceHttpGetContentLength(req,&size);
//	printf("%llu\n",size);
//	sceHttpAddExtraHeader(req,"Content-Range","bytes 0-5",0);
	return (req<<16)|(cnx<<8)|(tpl);
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
			print("we're run out of capsule up there...\n");
			bufLen=0;
			break;
		}
	}	
	return 0;
}
int myOpen(const char*path,int mode,int flag){
	if(!memcmp(path,"http://",7))return httpOpen((char*)path,NULL,flag);
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
	print("IO unload\n");
	return 0;
}
FileSys io={myOpen,myRead,mySeek,myClose};
int module_start(SceSize args,char*argp){
	print("IO loaded\n");
	if(args!=sizeof(OpenTube*))return 1;//return if no context
	ot=(OpenTube*)((u32*)argp)[0];
	ot->io=&io;
	return 0;
}