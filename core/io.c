#include <pspsdk.h>
#include <pspgu.h>
#include <pspkernel.h>
#include <pspdisplay.h>
#include <string.h>
#include <stdio.h>
#include <malloc.h>
#include <psputility.h>
#include <psputility_netmodules.h>
#include <psputility_htmlviewer.h>
#include <pspnet.h>
#include <pspnet_inet.h>
#include <pspnet_apctl.h>
#include <pspnet_resolver.h>
#include <psphttp.h>
#include <pspssl.h>
#define FD2TPL(fd) (((fd&0x0000FF)>>0 )|0x00000000)
#define FD2CNX(fd) (((fd&0x00FF00)>>8 )|0x01000000)
#define FD2REQ(fd) (((fd&0xFF0000)>>16)|0x02000000)
#include "core.h"
#define SIZE 0x10000//64ko segment
#define NUM 0x20 //20 segment = 1Mo cache
char*UserAgent="OpenTube/2.0 (PSP-0000 0.00)";
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
	if(!ot->gui)guInit();
	for(int done=0;!done;){
		if(!ot->gui)guDraw();else Draw(1|2);
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
		}else Draw(4|8);
	}
	if(!ot->gui)guTerm();
	if(data.base.result){
		Alert("still not connected");
		return netStop(2);
	}
	sceUtilityLoadNetModule(PSP_NET_MODULE_PARSEURI);
	sceUtilityLoadNetModule(PSP_NET_MODULE_PARSEHTTP);
	sceUtilityLoadNetModule(PSP_NET_MODULE_HTTP);
	if((err=sceHttpInit(0x25800))<0)return netStop(3);
	int model=0,fw=sceKernelDevkitVersion();
//	Sudo(4,(int)&model,0);
	UserAgent[18]=model+'1';
	UserAgent[23]=((fw&0x0F000000)>>24)+'0';
	UserAgent[25]=((fw&0x000F0000)>>16)+'0';
	UserAgent[26]=((fw&0x00000F00)>> 8)+'0';
	netInited=1;
	return 0;
}
int httpClose(int fd){
	if(fd<0x010101)return fd;
	sceHttpDeleteRequest(FD2REQ(fd));
	sceHttpDeleteConnection(FD2CNX(fd));
	sceHttpDeleteTemplate(FD2TPL(fd));
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
	int tpl=0,cnx=0,req=0,ret=0,status=0;
	if((tpl=sceHttpCreateTemplate(UserAgent, 1, 1))<0){ret=tpl;goto errTpl;}
	if((ret=sceHttpSetResolveRetry(tpl,2))<0){ret=cnx;goto errCnx;}
	if((ret=sceHttpSetResolveTimeOut(tpl,7*1000000))<0){ret=cnx;goto errCnx;}
	if((ret=sceHttpSetSendTimeOut   (tpl,7*1000000))<0){ret=cnx;goto errCnx;}
	if((ret=sceHttpSetRecvTimeOut   (tpl,7*1000000))<0){ret=cnx;goto errCnx;}
	if((cnx=sceHttpCreateConnectionWithURL(tpl, url, 1))<0){ret=cnx;goto errCnx;}
	if((req=sceHttpCreateRequestWithURL(cnx,PSP_HTTP_METHOD_GET,url,0))<0){ret=req;goto errReq;}
	if((ret=sceHttpSendRequest(req,data,0))<0)goto errReq;
	if((ret=sceHttpGetStatusCode(req,&status))<0)goto errReq;
	if(status!=200){Alert("badReqStatus\n");}
//	Printf("%llu\n",size);
//	sceHttpAddExtraHeader(req,"Content-Range","bytes 0-5",0);
	int fd=(req<<16)|(cnx<<8)|(tpl);
	return fd;
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
	$("done\n");
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
int StreamThread(SceSize args,void*argp){
	Str.buf=Malloc(SIZE*NUM);
	if(Str.fd<0x010101){
		Str.size=sceIoLseek(Str.fd,0,SEEK_END);
		sceIoLseek(Str.fd,0,SEEK_SET);		
	}else{
		unsigned long long size;
		if(!sceHttpGetContentLength(FD2REQ(Str.fd),&size))Str.size=size;
	}
	for(Str.run=1;Str.run&&Str.buf;){
		if(Str.end-(Str.curr-3*SIZE)<SIZE*(NUM)){
			if(Str.end && Str.size==Str.end){
//			printf("<%08X %08X %08X> full\n",Str.start,Str.curr,Str.end);
			sceKernelDelayThread(1000*50);continue;}
			int len=Read(Str.fd,Str.buf+(Str.end%(SIZE*NUM)),SIZE);
			Str.end+=len;
			if(len<SIZE){//end of buffer reached
//				Alert("\nEOB\n");
				Str.size=Str.end;
				continue;
			}
			Str.start=Str.end-SIZE*NUM;
			if(Str.start<0)Str.start=0;
//			printf("<%08X %08X %08X> load\n",Str.start,Str.curr,Str.end);
		}else sceKernelDelayThread(1000*50);
	}
	if(Str.buf)Free(Str.buf);Str.buf=NULL;
	if(Str.fd)Close(Str.fd);Str.fd=0;
	memset(&Str,0,sizeof(Str));
	return sceKernelExitDeleteThread(0);
}
int myOpen(char*path,int mode,int flag){
	int ret=(memcmp(path,"http://",7))?
		sceIoOpen(path,(mode^PSP_O_NOWAIT)|PSP_O_RDONLY,flag):
		httpOpen((char*)path,mode,flag);
	if((mode&PSP_O_NOWAIT)&&(!Str.th)&&(ret>0)){
		Str.fd=ret;
		Str.th=ret=sceKernelCreateThread("OpenTube.io.buffer",StreamThread,0x11,0x10000,0,0);
		if(ret>0)sceKernelStartThread(Str.th,0,NULL);
		sceKernelDelayThread(1000);
	}
	return ret;
}
int myReadX(void*p,int pos,int len){//read from buffer
	if(!Str.th)return 0;
	if(Str.size && pos>Str.size)return 0;
	while(!(Str.size && Str.size==Str.end) && pos+len>Str.end){
		$(".");
//		printf("\nno enought data : (%i-%i>%i)\n",Str.curr,len,Str.end);
		sceKernelDelayThread(1000*50);
	}
//	printf("READX>%08X %08X = %08X\n",pos,len,((u32*)Str.buf)[pos/4]);
	int tmp_len=0;
	if((pos%(SIZE*NUM))+len > SIZE*NUM){//len overpass the ring buffer
		Alert("1st cp\n");
		tmp_len = len-((pos+len)%(SIZE*NUM));//size until EOB
		if(p)memcpy(p,Str.buf+(pos%(SIZE*NUM)),tmp_len);
	}
//	printf("memcpy(%i,%i,%i)\n",tmp_len,((pos+tmp_len)%(SIZE*NUM)),len-tmp_len);
	if(p)memcpy(p+tmp_len,Str.buf+((pos+tmp_len)%(SIZE*NUM)),len-tmp_len);
	Str.curr=pos+len+tmp_len;
//	printf("F>%i %i %i %i\n",Str.start,Str.curr,Str.end,Str.size);
	return len+tmp_len;
}
int myRead(int fd,void*p,int len){
	if(fd==Str.th)return myReadX(p,Str.curr,len);
	/*{
		Alert("th read\n");
		if(Str.curr<Str.start){
			Alert("!rdBef\n");
			return 0;
		}
		if((Str.size && Str.size==Str.end) && Str.curr+len>Str.end){//buffer filled but asking too much
			len=Str.end-Str.curr;
		}
		while((Str.size && Str.size==Str.end) && Str.curr+len>Str.end){
			//printf("\nno enought data : (%i-%i>%i)\n",Str.curr,len,Str.end);
			Alert("!enDat\n");
			sceKernelDelayThread(1000*50);
		}
		int tmp_len=0;//used if len overpass the ring buffer
		if((Str.curr%(SIZE*NUM))+len > SIZE*NUM){
			tmp_len = len-((Str.curr+len)%(SIZE*NUM));//size until EOB
			if(p)memcpy(p,Str.buf+(Str.curr%(SIZE*NUM)),tmp_len);
			Str.curr+=tmp_len;
		}
		if(p)memcpy(p,Str.buf+(Str.curr%(SIZE*NUM)),len-tmp_len);
		Str.curr+=len-tmp_len;
		return len;
	}*/
//	printf("R %i %08X %08X\n",fd,sceIoLseek32(fd,0,SEEK_CUR),len);
	if(fd<0x010101)return sceIoRead(fd,p,len);
	return httpRead(fd,p,len);
}
int mySeek(int fd,int len,int type){
	if(fd==Str.th){
		if(type==SEEK_CUR)Str.curr+=len;
		if(type==SEEK_SET)Str.curr=len;
		if(type==SEEK_END)Alert("cant seek from end\n");
	}
	if(fd<0x010101)httpSeek(fd,len,type);
	return sceIoLseek(fd,len,type);
}
int myWrite(int fd,void*p,int len){
	if(fd<0x010101)return sceIoWrite(fd,p,len);
	return 0;
}
int myClose(int fd){
	if(fd==Str.th){
		for(Str.run=0;Str.th;sceKernelDelayThread(1000*100));
		return 0;
	}
	if(fd<0x010101)return sceIoClose(fd);
	return httpClose(fd);
}
int ioUnload(){
	if(Str.th)myClose(Str.th);
	netStop(-1);
	ot->io=NULL;
	$("IO unload\n");
	return 0;
}
FileSys io={myOpen,mySeek,myRead,myReadX,myWrite,myClose,ioUnload};
int ioInit(){
	$("IO loaded\n");
	ot->io=&io;
	return 0;
}