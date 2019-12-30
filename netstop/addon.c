#include <pspkernel.h>
#include <pspdisplay.h>
#include <stdio.h>
#include <string.h>
#include <pspwlan.h>
#include <pspsdk.h>
#include <pspnet.h>
#include <psphttp.h>
#include <pspnet_inet.h>
#include <pspnet_apctl.h>
#include <pspnet_resolver.h>
#include <psputility.h>
#define $(A) sceIoWrite(2,A,sizeof(A))

PSP_MODULE_INFO("netStop",0,0,0);

int  module_stop(){
	$("stopped!\n");
	return 0;
}
int loop(unsigned args,void*argp){
	sceHttpEnd();
	sceUtilityUnloadNetModule(PSP_NET_MODULE_HTTP);
	sceUtilityUnloadNetModule(PSP_NET_MODULE_PARSEHTTP);
	sceUtilityUnloadNetModule(PSP_NET_MODULE_PARSEURI);
	if(sceNetApctlTerm())$("apctl\n");
	if(sceNetInetTerm())$("inet\n");
	if(sceNetTerm())$("net\n");
	sceUtilityUnloadNetModule(PSP_NET_MODULE_INET);
	sceUtilityUnloadNetModule(PSP_NET_MODULE_COMMON);
	sceKernelSelfStopUnloadModule(1,0,0);
	return 0;
}
int  module_start(int args,void*argp){
	return sceKernelStartThread(sceKernelCreateThread("netstop",loop,0x11,0x10000,0,0),0,NULL);
}