#include <pspkernel.h>
#include <systemctrl.h>
#include <pspsdk.h>
#define alert(str) sceIoWrite(2,str,sizeof(str))
PSP_MODULE_INFO("bridge",0x1006,0,0);

int meBootStart(u32 nid,u32 mode){
	int (*sceMeBootStart)(int)=(void*)sctrlHENFindFunction("sceMeCodecWrapper","sceMeCore_driver",nid);//0x051C1601
 	int ret=-1,k1=pspSdkSetK1(0);
//	sceIoWrite(2,sceMeBootStart?"MeBoot\n":"noFunc\n",7);
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
int module_start(SceSize args,int*argp){
	if(args==3*4){
		switch(argp[0]){
			case 1:
				alert("startME");
				meBootStart(argp[1],argp[2])<0?alert(":fail\n"):alert(":done\n");
			break;
			case 2:
				alert("setFreq");
				setFrequency(argp[1],argp[2])<0?alert(":fail\n"):alert(":done\n");
			break;
			default:break;
		}
	}else alert("badArgs\n");
	return 1;
//	PspIoDrvFuncs func={httpInit,httpExit,httpOpen,httpClose,httpRead,httpWrite,httpLseek,httpIoCtl,0,0,0,0,0,0,0,0,0,0,0,0,httpDevCtl,0};
//	PspIoDrv driver={"http",0x04,0x800,"HTTP",&func};
}
int module_stop(void){
	return 0;//sceIoDelDrv("HTTP");	
}