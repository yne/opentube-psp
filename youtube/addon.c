#include <pspkernel.h>
#include <pspdisplay.h>
#include <stdio.h>
#include <string.h>
#include <pspwlan.h>
#include <pspsdk.h>
#include <pspnet.h>
#include <pspnet_inet.h>
#include <pspnet_apctl.h>
#include <pspnet_resolver.h>
#include <psphttp.h>
#include <psputility.h>
#include <arpa/inet.h>
#define $(A) sceIoWrite(1,A,sizeof(A))

PSP_MODULE_INFO("OpenTube.addon.youtube",0,0,0);

char c[100*1000],*p=0;
char* ext(char*from,char*to){
	char*a=strstr(p,from);if(!a)return NULL;
	a+=strlen(from);
	char*b=strstr(a,to);if(!b)return NULL;
	b[0]=0;
	p=b+1;
	return a;
}
int  module_stop(){
	$("stopped!\n");
	return 0;
}
int init(){
	int tpl=0,cnx=0,req=0,res=0,status=0;
	char*url="http://gdata.youtube.com/feeds/api/videos?q=Djmax%20BGA&start-index=1&max-results=20&v=1";
	sceUtilityLoadNetModule(PSP_NET_MODULE_PARSEURI);
	sceUtilityLoadNetModule(PSP_NET_MODULE_PARSEHTTP);
	sceUtilityLoadNetModule(PSP_NET_MODULE_HTTP);
	if((res=sceHttpInit(0x25800)) < 0)return res;
	if((res=sceHttpLoadSystemCookie()) < 0)return res;
//real stuff
	if((res=tpl=sceHttpCreateTemplate("OpenTube/2.0 (PSP-0000 0.00)", 1, 0))<0)return res;
	if((res=sceHttpEnableKeepAlive(tpl))<0)return res;
	if((res=sceHttpSetResolveRetry(tpl,2))<0)return res;
	if((res=sceHttpSetResolveTimeOut(tpl,7*1000000))<0)return res;
	if((res=sceHttpSetSendTimeOut   (tpl,7*1000000))<0)return res;
	if((res=sceHttpSetRecvTimeOut   (tpl,7*1000000))<0)return res;
	if((res=cnx=sceHttpCreateConnectionWithURL(tpl,url,1))<0)return res;
	if((res=req=sceHttpCreateRequestWithURL(cnx,PSP_HTTP_METHOD_GET,url,0))<0)return res;
	if((res=sceHttpSendRequest(req,NULL,0))<0)return res;
	if((res=sceHttpGetStatusCode(req,&status))<0)return res;
	printf("status:%i\n",status);
	sceHttpReadData(req,c,sizeof(c));p=c;
	char* id,*desc,*title,*time,*rate,*fav,*view,*total=ext("totalResults>","<");
	printf("total:%s\n",total?total:"?");
	while(
		(id   =ext("videos/","<"))&&
		(desc =ext("<media:description type='plain'>","<"))&&
		(title=ext("<media:title type='plain'>","<"))&&
		(time =ext("<yt:duration seconds='","'"))&&
		(rate =ext("rating average='","'"))&&
		(fav  =ext("favoriteCount='","'"))&&
		(view =ext("viewCount='","'"))){
		printf("%s %s %s %s %s %s %s\n\n",id?id:"?",desc?desc:"?",title?title:"?",time?time:"?",rate?rate:"?",fav?fav:"?",view?view:"?");
	}
	return 0;
}
int loop(unsigned args,void*argp){
	init();

	sceHttpEnd();
	sceUtilityUnloadNetModule(PSP_NET_MODULE_HTTP);
	sceUtilityUnloadNetModule(PSP_NET_MODULE_PARSEHTTP);
	sceUtilityUnloadNetModule(PSP_NET_MODULE_PARSEURI);

	sceKernelSelfStopUnloadModule(1,0,0);
	return 0;
}
int  module_start(int args,void*argp){
	sceKernelStartThread(sceKernelCreateThread("netinit",loop,0x11,0x10000,0,0),0,NULL);
//	if((ot=otGetCtx())<0)return 1;
//	Alert("youtube addon loaded\n");
//	Mode=2;
	return 0;
}