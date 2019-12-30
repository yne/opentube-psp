#include <pspkernel.h>
#include <systemctrl.h>
#include <psppower.h>
#include <pspctrl.h>
#include <pspsdk.h>
#include <pspimpose_driver.h>
#include "core.h"
#define alert(str) sceIoWrite(2,str,sizeof(str))
PSP_MODULE_INFO("bridge",0x1000,0,0);
/* killable thread list :
thterm @ScePowerBattery
thdel @ScePowerBattery
thterm @SceImpose
thdel @SceImpose
thterm @ScePowerMain
thdel @ScePowerMain
thterm @SceUmdManMount
thdel @SceUmdManMount
thterm @SceUmdManTask
thdel @SceUmdManTask
thdel @SceUmdManTaskSPKStop
thterm @SceHpRemote
thdel @SceHpRemote
thterm @SceMScmMain
thdel @SceMScmMain
thterm @SceMScmMedia
thdel @SceMScmMedia
*/
int meBootStart(u32 nid,u32 mode){
	int (*sceMeBootStart)(int)=(void*)sctrlHENFindFunction("sceMeCodecWrapper","sceMeCore_driver",nid);//0x051C1601
 	int ret=-1,k1=pspSdkSetK1(0);
	if(sceMeBootStart)ret=sceMeBootStart(mode);
	pspSdkSetK1(k1);
	return ret;
}
int setFrequency(u32 nid,u32 freq){
	int (*sceAudioSetFrequency)(int)=(void*)sctrlHENFindFunction("sceAudio_driver","sceAudio_driver",nid);//0x9DB844C6
 	int ret=-1,k1=pspSdkSetK1(0);
	if(sceAudioSetFrequency)ret=sceAudioSetFrequency(freq);
	pspSdkSetK1(k1);
	return ret;
}
int getThCWD(u32 th,u32 cwd){
 	int k1=pspSdkSetK1(0);
	sceIoGetThreadCwd(th,(char*)cwd,256);
	pspSdkSetK1(k1);
	return 0;
}
int getModel(int res){
 	int k1=pspSdkSetK1(0);
	extern int sceKernelGetModel();
	((int*)res)[0]=sceKernelGetModel();
	pspSdkSetK1(k1);
	return 0;
}
int setBackL(int time){
	alert("setBL\n");
	return sceImposeSetBacklightOffTime(time);
}
/*meBootStart_re(int mode){//1-4
  if(mode>5)return 0x80000102;
	if(sceSysregGetTachyonVersion()>0x004FFFFF)mode = (mode^2)?3:mode;
	if(LastMeMode==arg1)return 0;//already setup
	if(sub_01704()<0)return;
	int fd = sceIoOpen ("flash0:/kd/resource/meimg.img", 0x04000001, 0);
	if(fd<0){close();return fd;}
	size = sceIoLseek (fd,0,end);
	sceIoLseek (fd,0,start);
	if(size<0x0005FFFF){sceIoClose (fd);close ();return 0x80000022;}
	sceSysreg_driver_20C9E2C4 ();
	void*bin = 0x883FF000-((size&0xFFFFFFC0));
	int len = sceIoRead (fd, bin, size);
	sceIoClose (fd);
	if(len != size){close ();return (len < 0)? len:0x80000022;}
	initMe ();//sceWmd_driver_7A0E484C
	loadMeBin (bin,1);//1=blocking (while 0xBFC00700==-3)
	LastMeMode = arg1;
	close ();
	return sceMeCore_driver_F702E73A (0x0185, ((0 < (arg1^2))), (arg1^2));
}
 	int k1=pspSdkSetK1(0);
	int out=sceIoOpen("host0:/Medump",PSP_O_CREAT|PSP_O_WRONLY,0777);
	0xBFC00600=0x00000002
	0xBFC00608=0x00198A40
	0xBFC0060C=0x08C10000
	0xBFC00610=0x00000580
	0xBFC00614=0x088552C0
	0xBFC00618=0x08855300
	0xBFC0061C=0x088553C0
	0xBFC00620=0xC0000000
	0xBFC00624=0x0885526C
	sceIoClose(out);
	pspSdkSetK1(k1);
*/

int module_start(SceSize args,int*argp){
	SceModule*mod;
	if(args==3*sizeof(int)){
		switch(argp[0]){
			case 1:alert("startME");meBootStart (argp[1],argp[2])<0?alert(":fail\n"):alert(":done\n");break;
			case 2:alert("setFreq");setFrequency(argp[1],argp[2])<0?alert(":fail\n"):alert(":done\n");break;
			case 3:getThCWD(argp[1],argp[2]);break;
			case 4:getModel(argp[1]);break;
			case 5:setBackL(argp[1]);break;
			case 6:if((mod=(SceModule*)sceKernelFindModuleByName("sceMpegVsh_library")))*(((int**)argp)[1])=mod->text_addr;break;
			default:break;
		}
	}else{
		char*err="unk arg mode:?\n";
		err[13]=argp[0]+'0';
		alert(err);
	}
	return 1;
//	PspIoDrvFuncs func={httpInit,httpExit,httpOpen,httpClose,httpRead,httpWrite,httpLseek,httpIoCtl,0,0,0,0,0,0,0,0,0,0,0,0,httpDevCtl,0};
//	PspIoDrv driver={"http",0x04,0x800,"HTTP",&func};
}
int module_stop(void){
	alert("kernel unload\n");
	return 0;//sceIoDelDrv("HTTP");	
}