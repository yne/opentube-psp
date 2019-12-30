#include <pspkernel.h>
#include <pspdisplay.h>
#include <pspgu.h>
#include <string.h>
#include <stdio.h>
#include <psputility.h>
#include <psputility_netmodules.h>
#include <psputility_htmlviewer.h>
#include <pspnet.h>
#include <pspnet_inet.h>
#include <pspnet_apctl.h>
#include <pspnet_resolver.h>
#include <psphttp.h>
#include <pspssl.h>

#include "core.h"
#include "bridge.h"

#define BROWSER_MEMORY (4*1024*1024)
static char list[0x100] __attribute__((aligned(64)));

void setupGu(void){
#define BUF_WIDTH (512)
#define SCR_WIDTH (480)
#define SCR_HEIGHT (272)
	sceGuInit();
	sceGuStart(GU_DIRECT, list);
	sceGuDrawBuffer(GU_PSM_5650, 0, BUF_WIDTH);
	sceGuDispBuffer(SCR_WIDTH, SCR_HEIGHT, (void*)0x88000, BUF_WIDTH);//0x88000
	sceGuDisable(GU_DEPTH_TEST);
	sceGuOffset(0,0);
	sceGuScissor(0,0,SCR_WIDTH,SCR_HEIGHT);
	sceGuEnable(GU_SCISSOR_TEST);
	sceGuFinish();
	sceGuSync(0, 0);
	sceDisplayWaitVblankStart();
	sceGuDisplay(GU_TRUE);
}
void draw(){
	sceGuStart(GU_DIRECT, list);
	sceGuFinish();
	sceGuSync(0, 0);	
}
void netTerm(){
	sceHttpSaveSystemCookie();
	sceHttpsEnd();
	sceHttpEnd();
	sceSslEnd();
	sceNetApctlTerm();
	sceNetInetTerm();
	sceNetTerm();
	sceUtilityUnloadNetModule(PSP_NET_MODULE_SSL);
	sceUtilityUnloadNetModule(PSP_NET_MODULE_HTTP);
	sceUtilityUnloadNetModule(PSP_NET_MODULE_PARSEHTTP);
	sceUtilityUnloadNetModule(PSP_NET_MODULE_PARSEURI);
	sceUtilityUnloadNetModule(PSP_NET_MODULE_INET);
	sceUtilityUnloadNetModule(PSP_NET_MODULE_COMMON);
}
int netInit(){
	int res;
	sceUtilityLoadNetModule(PSP_NET_MODULE_COMMON);
	sceUtilityLoadNetModule(PSP_NET_MODULE_INET);
	sceUtilityLoadNetModule(PSP_NET_MODULE_PARSEURI);
	sceUtilityLoadNetModule(PSP_NET_MODULE_PARSEHTTP);
	sceUtilityLoadNetModule(PSP_NET_MODULE_HTTP);
	sceUtilityLoadNetModule(PSP_NET_MODULE_SSL);
	if((res=sceNetInit(0x20000, 0x2A, 0, 0x2A, 0)) < 0){return res;}
	if((res=sceNetInetInit()) < 0){netTerm();return res;}
	if((res=sceNetResolverInit()) < 0){netTerm();return res;}
	if((res=sceNetApctlInit(0x1800, 0x30)) < 0){netTerm();return res;}
	if((res=sceSslInit(0x28000)) < 0){netTerm();return res;}
	if((res=sceHttpInit(0x25800)) < 0){netTerm();return res;}
	if((res=sceHttpsInit(0, 0, 0, 0)) < 0){netTerm();return res;}
	if((res=sceHttpsLoadDefaultCert(0, 0)) < 0){netTerm();return res;}
	if((res=sceHttpLoadSystemCookie()) < 0){netTerm();return res;}
	return 0;
}
int htmlViewerInit(void* vplp,char*url){
	int res;
	$("html initing");
	pspUtilityHtmlViewerParam params;
	memset(&params, 0, sizeof(pspUtilityHtmlViewerParam));
	params.base.size = sizeof(pspUtilityHtmlViewerParam);
	sceUtilityGetSystemParamInt(PSP_SYSTEMPARAM_ID_INT_LANGUAGE, &params.base.language);
	sceUtilityGetSystemParamInt(PSP_SYSTEMPARAM_ID_INT_UNKNOWN, &params.base.buttonSwap);
	params.base.graphicsThread = 17;
	params.base.accessThread = 19;
	params.base.fontThread = 18;
	params.base.soundThread = 16;
	params.memaddr = vplp;
	params.memsize = BROWSER_MEMORY;
	
	params.initialurl = url;
	params.numtabs = 1;
	params.cookiemode = PSP_UTILITY_HTMLVIEWER_COOKIEMODE_DEFAULT;
//	params.homeurl = params.initialurl;
	params.textsize = PSP_UTILITY_HTMLVIEWER_TEXTSIZE_NORMAL;
	params.displaymode = PSP_UTILITY_HTMLVIEWER_DISPLAYMODE_SMART_FIT;
	params.options = 
		PSP_UTILITY_HTMLVIEWER_DISABLE_CURSOR|
		PSP_UTILITY_HTMLVIEWER_DISABLE_LRTRIGGER|
//		PSP_UTILITY_HTMLVIEWER_DISABLE_EXIT_DIALOG|
		PSP_UTILITY_HTMLVIEWER_DISABLE_TAB_DISPLAY|
		PSP_UTILITY_HTMLVIEWER_DISABLE_DOWNLOAD_START_DIALOG|
		PSP_UTILITY_HTMLVIEWER_DISABLE_DOWNLOAD_DESTINATION_DIALOG|
		PSP_UTILITY_HTMLVIEWER_DISABLE_DOWNLOAD_COMPLETE_DIALOG|
		PSP_UTILITY_HTMLVIEWER_DISABLE_STARTUP_LIMITS|//?
		0;
	params.interfacemode = PSP_UTILITY_HTMLVIEWER_INTERFACEMODE_NONE;
	params.connectmode = PSP_UTILITY_HTMLVIEWER_CONNECTMODE_LAST;

	char* mime="application/x-psp-extplugin";
	params.unknown4[0]=(int)"WipEoutPulse";
	params.unknown4[6]=9;//number of function
	jsMeth funList[]={
		{mime,"open",JsOpen,&params,0},
		{mime,"play",JsPlay,&params,0},
		{mime,"dread",Delay,&params,0},
		{mime,"write",Delay,&params,0},
		{mime,"read",Delay,&params,0},
		{mime,"wget",Delay,&params,0},
		{mime,"setDlDir",Delay,&params,0},
		{mime,"getFW",Delay,&params,0},
		{mime,"delay",Delay,&params,0},
		{0,0,0,0,0}
	};
	params.unknown4[7]=(int)funList;
	params.unknown4[9]=1;
//	params.dldirname = "/VIDEO";
	if ((res = sceUtilityHtmlViewerInitStart(&params)) < 0)return res;
	return 0;
}
void loop(){
	for(Mode=1;;){
		if(Mode!=1){
			sceDisplayWaitVblankStart();
			continue;
		}
		switch (sceUtilityHtmlViewerGetStatus()){
			case PSP_UTILITY_DIALOG_VISIBLE:
				draw();
				sceUtilityHtmlViewerUpdate(1);
				break;
			case PSP_UTILITY_DIALOG_QUIT:sceUtilityHtmlViewerShutdownStart();break;
			case PSP_UTILITY_DIALOG_FINISHED:return;
			case PSP_UTILITY_DIALOG_NONE:return;
			default:break;
		}
		sceDisplayWaitVblankStart();
		sceGuSwapBuffers();
	}
}
int gui(SceSize args, void *argp){
	setupGu();
	void* vplp;
	SceUID vpl=sceKernelCreateVpl("BrowserVpl", PSP_MEMORY_PARTITION_USER, 0, BROWSER_MEMORY + 256, NULL);
	if((sceKernelAllocateVpl(vpl, BROWSER_MEMORY, &vplp, NULL))<0)Alert("vpl error");
	if(netInit())Alert("netError");
	if(htmlViewerInit(vplp,"file:/test.html"))Alert("htmlErr");
	else loop();
	netTerm();
	sceGuTerm();
	sceKernelFreeVpl(vpl, vplp);
	sceKernelDeleteVpl(vpl);
	Exit();
	return 0;
}
