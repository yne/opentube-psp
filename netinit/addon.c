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
#include <psputility.h>
#define $(A) sceIoWrite(2,A,sizeof(A))

PSP_MODULE_INFO("OpenTube.addon.youtube",0,0,0);

int  module_stop(){
	$("stopped!\n");
	return 0;
}
char* apco(){
	if(!sceWlanDevIsPowerOn())return "noWlan";
	sceUtilityLoadNetModule(PSP_NET_MODULE_COMMON);
	sceUtilityLoadNetModule(PSP_NET_MODULE_INET);
	int err,_state=-1;
	if((err=sceNetInit(0x20000, 0x20, 0x1000, 0x20, 0x1000)))return("netInit");
	if((err=sceNetInetInit()))return("inetIni");
	if((err=sceNetResolverInit()))return("resInit");
	if((err=sceNetApctlInit(0x1400, 0x42)))return("ApctlIn");
	for(int i=1;;i++){
		if(sceUtilityCheckNetParam(i))return("noApctl");
		char name[100];
		sceUtilityGetNetParam(i, 0, (netData*)name);
		$(name);$("\n");
		if((err=sceNetApctlConnect(i)))return("ApctlCo");	
		while(1){
			int state;
			sceDisplayWaitVblankStart();
			if((err=sceNetApctlGetState(&state)))return("getStat");
			if(state==_state)continue;
			_state=state;
			printf("state %i\n",state);
			if(state==4)return NULL;//connected
			if(!state)break;//try next one
		}
	}
}
int loop(unsigned args,void*argp){
	char*ret=apco();
	if(ret)sceIoWrite(1,ret,7);
	sceKernelSelfStopUnloadModule(1,0,0);
	return 0;
}
int  module_start(int args,void*argp){
	return sceKernelStartThread(sceKernelCreateThread("netinit",loop,0x11,0x10000,0,0),0,NULL);
}