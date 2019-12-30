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
char*UserAgent="Mozilla/5.0 (Windows NT 5.1; rv:6.0) Gecko/20100101 Firefox/6.0";//"OpenTube/2.0 (PSP-0000 0.00)";
int err=0,netInited=1;
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
	int tpl=0,cnx=0,req=0,ret=0,status=0;
	if((tpl=sceHttpCreateTemplate(UserAgent, 1, 0))<0){ret=tpl;goto errTpl;}
//to look exactly like FF6
//	sceHttpAddExtraHeader(tpl,"accept-charset","ISO-8859-1,utf-8;q=0.7,*;q=0.7",0);
//	sceHttpAddExtraHeader(tpl,"accept-encoding","gzip, deflate",0);
//	sceHttpAddExtraHeader(tpl,"accept-language","en-us,en;q=0.5",0);
//	sceHttpAddExtraHeader(tpl,"accept","text/html,application/xhtml+xml,application/xml;q=0.9,*/*;q=0.8",0);
	
	if((ret=sceHttpEnableKeepAlive(tpl))<0){ret=cnx;goto errCnx;}
	if((ret=sceHttpSetResolveRetry(tpl,2))<0){ret=cnx;goto errCnx;}
	if((ret=sceHttpSetResolveTimeOut(tpl,7*1000000))<0){ret=cnx;goto errCnx;}
	if((ret=sceHttpSetSendTimeOut   (tpl,7*1000000))<0){ret=cnx;goto errCnx;}
	if((ret=sceHttpSetRecvTimeOut   (tpl,7*1000000))<0){ret=cnx;goto errCnx;}
	if((cnx=sceHttpCreateConnectionWithURL(tpl, url, 1))<0){ret=cnx;goto errCnx;}
	if((req=sceHttpCreateRequestWithURL(cnx,PSP_HTTP_METHOD_GET,url,0))<0){ret=req;goto errReq;}
	if((ret=sceHttpSendRequest(req,data,0))<0)goto errReq;
	if((ret=sceHttpGetStatusCode(req,&status))<0)goto errReq;
	int fd=(req<<16)|(cnx<<8)|(tpl);
//	printf("status:%i\n",status);
	if(status>=500&&status<600){Alert("5XX Server Error\n");Close(fd);}
	if(status>=400&&status<500){Alert("4XX Client Error\n");Close(fd);}
	if(status>=300&&status<400){Alert("3XX Redirecting.\n");}
	if(status>=200&&status<300){Alert("2XX Successfully\n");}
	if(status>=100&&status<200){Alert("1XX Informations\n");}
//	Printf("%llu\n",size);
//	sceHttpAddExtraHeader(req,"Content-Range","bytes 0-5",0);
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
	if(mode==HTTP_SAVE_RAM && param){
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
		if(Str.end-(Str.curr-(8)*SIZE)<SIZE*(NUM)){
			if(Str.end && Str.size==Str.end){sceKernelDelayThread(1000*50);continue;}
			int len=Read(Str.fd,Str.buf+(Str.end%(SIZE*NUM)),SIZE);
			Str.end+=len;
			if(len<SIZE){//end of buffer reached
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
		sceIoOpen(      path,(mode^PSP_O_NOWAIT)|PSP_O_RDONLY|PSP_O_WRONLY,flag):
		httpOpen((char*)path,(mode^PSP_O_NOWAIT),flag);
	if((mode&PSP_O_NOWAIT)&&(!Str.th)&&(ret>0)){
		Str.fd=ret;
		Str.th=ret=sceKernelCreateThread("OpenTube.io.buffer",StreamThread,0x11,0x10000,0,0);
		if(ret>0)sceKernelStartThread(Str.th,0,NULL);
		sceKernelDelayThread(1000);
	}
	return ret;
}
int myReadX(void**p,int pos,int len){//read from buffer
	if(!Str.th||(Str.size&&pos>Str.size))return 0;
	while(!(Str.size && Str.size==Str.end) && pos+len>Str.end){
		$("buffering\n");
		sceKernelDelayThread(1000*50);
	}
	int tmp_len=0;
	if((pos%(SIZE*NUM))+len > SIZE*NUM){//len overpass the ring buffer
//		Alert("1st cp\n");
		tmp_len = len-((pos+len)%(SIZE*NUM));//size until EOB
		if(*p>0)memcpy(*p,Str.buf+(pos%(SIZE*NUM)),tmp_len);
	}
	if(*p>0)memcpy(*p+tmp_len,Str.buf+((pos+tmp_len)%(SIZE*NUM)),len-tmp_len);
	Str.curr=pos+len+tmp_len;
	return len+tmp_len;
}
int myRead(int fd,void*p,int len){
	if(fd==Str.th)return myReadX(&p,Str.curr,len);
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