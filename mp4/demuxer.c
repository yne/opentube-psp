#include <pspkernel.h>
#include <pspdisplay.h>
#include <stdio.h>
#include <string.h>
#include <pspctrl.h>
#include "demuxer.h"
#include "main.h"

#define OR :case
#define sw(n) (((n>>24)&0xff)|((n<<8)&0xff0000)|((n>>8)&0xff00)|((n<<24)&0xff000000))
#define sw64(n) (((n<<56)&0xFF)|((n<<48)&0xFF00)|((n<<40)&0xFF0000)|((n<<32)&0xFF000000)|((n<<24)&0xFF00000000)|((n<<16)&0xFF0000000000)|((n<<8)&0xFF000000000000)|((n<<0)&0xFF00000000000000))
/*
mp4
	header(moov)
	stream(mdat)
		chunk(stco=chunk offset)
			\/ (sample per chunk)
			sample(stsz=sample size)
*/
char dummy_audio[]={0x21,0x00,0x49,0x90,0x02,0x19,0x00,0x23,0x80};
char dummy_video[]={0x00,0x08,0x41,0x9a,0x04,0x0e,0x1f,0x80,0x7f,0xe0};

PSP_MODULE_INFO("OpenTube.demux.mp4",0,0,0);
int fd;unsigned size,type,tmp;//atom parsing
char curType,*c=NULL;//atom content buffer
OpenTube*ot;
MP4 f;
Demuxer dmx={getASample,getVSample};
u32 read32(){
	u32 i;
//	dlSync(fd,Seek(fd,0,SEEK_CUR)+4);
	Read(fd,&i,4);
	return sw(i);
}
int get(){
	return (c=Realloc(c,size))?Read(fd,c,size):puts("Malloc error");
}
void skip(){
//	dlSync(fd,Seek(fd,0,SEEK_CUR)+size);
	Seek(fd,size,SEEK_CUR);
}
char* parse(){
	size=read32()-8;
	type=read32();
//	puts("parse...");
	if((type>0x7FFFFFFF)||(type<0x60000000))return "badAtom";//bad atom
	switch(type){//parent atom must be in cased (otherwith they are skipped)
		case 0x6d6f6f76 OR 0x7472616b OR 0x6d646961 OR 0x6d696e66 OR 0x7374626c:break;//moov|trak|mdia|minf|stbl=parent:dont skip them
		case 0x6d646864:{get();//mdhd:timeScale+Duration
			tmp=sw(((int*)c)[3]);
		}break;
		case 0x68646c72:{get();//hdlr
			curType=c[8];
		}break;
		case 0x73747364:{get();//stsd
			if(curType=='s'){
				f.Acodec=sw(((int*)c)[3]);
				if(c[12]=='m'&&c[13]=='p'&&c[14]=='4'&&c[15]=='a'){
					f.ch=c[15+18];
					f.bps=c[15+20];
					f.hz=((c[15+25]<<8)+(c[15+26]))&0xFFFF;
					ot->dmx->Ahz=f.hz;
				}else{
					puts("unkAfmt");
				}
			}else	if(curType=='v'){
				f.Vcodec=sw(((int*)c)[3]);
				f.fps=tmp;
				f.width=sw(((int*)c)[10])>>16;
				f.height=sw(((int*)c)[10])&0xFFFF;
				if((c[98]=='a')&&(c[99]=='v')&&(c[100]=='c')&&(c[101]=='C')){
					f.profile=*(c+103);
					f.nalPreLen=1+((*(c+106))&0x03);
					f.spsLen=(*(c+108)<<8)+*(c+109);
					f.sps=Malloc(f.spsLen);
					for(tmp=0;tmp<f.spsLen;tmp++)
						f.sps[tmp]=*(c+110+tmp);
					f.ppsLen=(*(c+111+tmp)<<8)+*(c+112+tmp);
					f.pps=Malloc(f.ppsLen);
					for(int i=0;i<f.ppsLen;i++)
						f.pps[i]=*(c+113+tmp+i);
				}else{
					puts("unkVfmt");
				}
			}
		}break;
		case 0x73747363:{get();//stsc
			if(curType=='s'){
				f.AstscLen=sw(((int*)c)[1]);
				f.AstscFst=Malloc(f.AstscLen*sizeof(u32));
				f.AstscSpc=Malloc(f.AstscLen*sizeof(u32));
				f.AstscSid=Malloc(f.AstscLen*sizeof(u32));
				for(int i=0;i<f.AstscLen;i++){
					f.AstscFst[i] = sw(((int*)c)[2+i*3]);
					f.AstscSpc[i] = sw(((int*)c)[3+i*3]);
					f.AstscSid[i] = sw(((int*)c)[4+i*3]);
				}
			}else if(curType=='v'){
				f.VstscLen=sw(((int*)c)[1]);
				f.VstscFst=Malloc(f.VstscLen*sizeof(u32));
				f.VstscSpc=Malloc(f.VstscLen*sizeof(u32));
				f.VstscSid=Malloc(f.VstscLen*sizeof(u32));
				for(int i=0;i<f.VstscLen;i++){
					f.VstscFst[i] = sw(((int*)c)[2+i*3]);
					f.VstscSpc[i] = sw(((int*)c)[3+i*3]);
					f.VstscSid[i] = sw(((int*)c)[4+i*3]);
				}
			}
		}break;
		case 0x7374737a:{get();//stsz
			if(curType=='s'){
				f.AstszCnt=sw(((int*)c)[1]);//0 if const size
				f.AstszLen=sw(((int*)c)[2]);
				f.Astsz=Malloc(f.AstszLen*sizeof(u32));
				for(int i=0;i<f.AstszLen;i++)
					f.Astsz[i] = sw(((int*)c)[3+i]);
			}else if(curType=='v'){
				f.VstszCnt=sw(((int*)c)[1]);//0 on const size
				f.VstszLen=sw(((int*)c)[2]);
				f.Vstsz=Malloc(f.VstszLen*sizeof(u32));
				for(int i=0;i<f.VstszLen;i++)
					f.Vstsz[i] = sw(((int*)c)[3+i]);
			}
		}break;
		case 0x7374636f:{get();//stco
			if(curType=='s'){
				f.AstcoLen=sw(((int*)c)[1]);
				f.Astco=Malloc(f.AstcoLen*sizeof(u32));
				for(int i=0;i<f.AstcoLen;i++)
					f.Astco[i] = sw(((int*)c)[2+i]);
			}else if(curType=='v'){
				f.VstcoLen=sw(((int*)c)[1]);
				f.Vstco=Malloc(f.VstcoLen*sizeof(u32));
				for(int i=0;i<f.VstcoLen;i++)
					f.Vstco[i] = sw(((int*)c)[2+i]);
			}
		}break;
		case 0x6d646174:return NULL;//mdat
		default:skip();break;
	}
	return parse();
}
void rebuildVSpsc(){
	u32*spc=Malloc(f.VstszLen*sizeof(u32));//sample per chunk
	u32*fst=Malloc(f.VstszLen*sizeof(u32));//first sample (absolute)
	for(int i=0,t=0,j=0,s=0;i<f.VstcoLen;i++){
		spc[i]=f.VstscSpc[j];
		fst[i]=s;
		if(i==f.VstscFst[j]-1)t=f.VstscSpc[j++];
		s+=t;
	}
	Free(f.VstscSpc);
	f.VstscSpc=spc;
	Free(f.VstscFst);
	f.VstscFst=fst;
	u32*sid=Malloc(f.VstszLen*sizeof(u32));//trunk size
	for(int i=0;i<f.VstcoLen;i++){
		int size=0;
		for(int j=f.VstscFst[i];j<f.VstscFst[i]+f.VstscSpc[i];j++)size+=f.Vstsz[j];
		sid[i]=size;
	}
	Free(f.VstscSid);
	f.VstscSid=sid;
}
void rebuildASpsc(){
	u32*spc=Malloc(f.AstszLen*sizeof(u32));//sample per chunk
	u32*fst=Malloc(f.AstszLen*sizeof(u32));//first sample (absolute)
	for(int i=0,t=0,j=0,s=0;i<f.AstcoLen;i++){
		spc[i]=f.AstscSpc[j];
		fst[i]=s;
		if(i==f.AstscFst[j]-1)t=f.AstscSpc[j++];
		s+=t;
	}
	Free(f.AstscSpc);
	f.AstscSpc=spc;
	Free(f.AstscFst);
	f.AstscFst=fst;
	u32*sid=Malloc(f.AstszLen*sizeof(u32));//trunk size
	for(int i=0;i<f.AstcoLen;i++){
		int size=0;
		for(int j=f.AstscFst[i];j<f.AstscFst[i]+f.AstscSpc[i];j++)size+=f.Astsz[j];
		sid[i]=size;
	}
	Free(f.AstscSid);
	f.AstscSid=sid;
}
void rebuild(u32 szLen,u32*sz,u32 coLen,u32*scSpc,u32*scFst){//f.VstszLen f.Vstsz f.VstcoLen f.VstscSpc f.VstscFst
	u32*spc=Malloc(szLen*sizeof(u32));//sample per chunk
	u32*fst=Malloc(szLen*sizeof(u32));//first sample (absolute)
	for(int i=0,t=0,j=0,s=0;i<coLen;i++){
		spc[i]=scSpc[j];
		fst[i]=s;
		if(i==scFst[j]-1)t=scSpc[j++];
		s+=t;
	}
	Free(scSpc);
	scSpc=spc;
	Free(scFst);
	scFst=fst;
	u32*sid=Malloc(szLen*sizeof(u32));//trunk size
	for(int i=0;i<coLen;i++){
//		printf("%i~%08X\n",scFst[i],f.VstscSpc[i]);
		int size=0;
		for(int j=scFst[i];j<scFst[i]+scSpc[i];j++)
			size+=sz[j];
		sid[i]=size;
//	printf("%i/%i %08X\n",i,coLen,sid[i]);
	}
	puts("done B");
	Free(f.VstscSid);
	f.VstscSid=sid;	
}
void*video;
void* getVSample(int s){
	int t,pos=0;
	for(t=0;(t<f.VstcoLen)&&(f.VstscFst[t]<=s);t++);//seek to trunk+1
	for(int i=f.VstscFst[t-1];i<s;i++)pos+=f.Vstsz[i];//seek to sample in trunk
	video=Realloc(video,f.Vstsz[s]);
	Seek(f.Vfd,f.Vstco[t-1]+pos,SEEK_SET);
	Read(f.Vfd,video,f.Vstsz[s]);
//	printf("%p = %08X %i\n",video,offset[s],f.Vstsz[s]);
	return video;
}
void*audio;
void* getASample(int s){
	int t,pos=0;
	for(t=0;(t<f.AstcoLen)&&(f.AstscFst[t]<=s);t++);//seek to trunk+1
	for(int i=f.AstscFst[t-1];i<s;i++)pos+=f.Astsz[i];//seek to sample in trunk
	audio=Realloc(audio,f.Astsz[s]);
	Seek(f.Afd,f.Astco[t-1]+pos,SEEK_SET);
	Read(f.Afd,audio,f.Astsz[s]);
	return audio;
}
char* onLoad(const char* file){
	puts("opening");
	if((fd=Open(file,PSP_O_RDONLY,0777))<0)return "fileErr";
	char*err=parse();
	Close(fd);
	if(err){puts(err);return err;}

	if((err=ot->sys->findCodec(NULL,f.Acodec)))return err;
	if(!(ot->dmx->Aload&&ot->dmx->Aplay&&ot->dmx->Aseek&&ot->dmx->Astop))return "badAApi";
	if((f.Afd=Open(file,PSP_O_RDONLY,0777))<0)return "fileErr";
	rebuildASpsc();
	//rebuild(f.AstszLen,f.Astsz,f.AstcoLen,f.AstscSpc,f.AstscFst);
	if((err=ot->dmx->Aload()))return err;
	
	if((err=ot->sys->findCodec(NULL,f.Vcodec)))return err;
	if(!(ot->dmx->Vload&&ot->dmx->Vplay&&ot->dmx->Vseek&&ot->dmx->Vstop))return "badVApi";
	if((f.Vfd=Open(file,PSP_O_RDONLY,0777))<0)return "fileErr";
	rebuildVSpsc();
	//rebuild(f.VstszLen,f.Vstsz,f.VstcoLen,f.VstscSpc,f.VstscFst);
	if((err=ot->dmx->Vload()))return err;

	//int tmp=Open("audio",PSP_O_CREAT|PSP_O_WRONLY,0777);
	//for(int i=0;i<f.AstcoLen;i++)
	//	sceIoWrite(tmp,getASample(i),f.Astsz[i]);
	//Close(tmp);
	
	//int dummy;sceDisplayGetMode(&ot->lcd->type,dummy,dummy);
	ot->lcd->type=PSP_DISPLAY_PIXEL_FORMAT_8888;
	ot->lcd->size=f.width>480?720:512;
	//printf("%08X\n",f.Vcodec);
	return 0;
}
int Video(unsigned args,void*argp){
	ot->dmx->Vplay();
	((int*)argp)[0]=0;
	puts("v ok");
	return sceKernelExitDeleteThread(0);
}
int Audio(unsigned args,void*argp){
	ot->dmx->Aplay();
	((int*)argp)[0]=0;
	puts("a ok");
	return sceKernelExitDeleteThread(0);
}
int ctrlOk(){
	if(ot->sys->pad!=ot->sys->_pad)if(ot->sys->pad&PSP_CTRL_CIRCLE)return 0;
	return 1;
}
char* onPlay(){
	puts("start playback");
	int vTh=sceKernelCreateThread("video",Video,0x11,0x10000,0,0);
	int aTh=sceKernelCreateThread("audio",Audio,0x11,0x10000,0,0);
	sceKernelStartThread(vTh,4,&vTh);
	sceKernelStartThread(aTh,4,&aTh);
	while(ctrlOk()&&(vTh&&aTh))sceDisplayWaitVblankStart();
	if(vTh)sceKernelTerminateDeleteThread(vTh);
	if(aTh)sceKernelTerminateDeleteThread(aTh);
	puts("end");
	return ot->sys->err;
}
char* onSeek(int w,int o){
	puts("seek...");
	ot->dmx->Aseek(w,o);
	ot->dmx->Vseek(w,o);
	return 0;
}
int unload(SceSize args,void*argp){
	do sceKernelDelayThread(1000);
	while(sceKernelSelfStopUnloadModule(1,0,NULL)<0);
	return 0;
}
char* onStop(){
	puts("unloading demuxer");
	Realloc(video,0);Realloc(audio,0);
//free atom list
	Free(f.sps);Free(f.pps);
	Free(f.VstscFst);Free(f.AstscFst);
	Free(f.VstscSpc);Free(f.AstscSpc);
	Free(f.VstscSid);Free(f.AstscSid);
	Free(f.Vstsz);   Free(f.Astsz);
	Free(f.Vstco);   Free(f.Astco);
	ot->dmx->Astop();
	ot->dmx->Vstop();
	if(f.Afd>0)Close(f.Afd);
	if(f.Vfd>0)Close(f.Vfd);
	ot->sys->onLoad=NULL;
	ot->sys->onPlay=NULL;
	ot->sys->onSeek=NULL;
	ot->sys->onStop=NULL;
	ot->dmx=NULL;
	sceKernelStartThread(sceKernelCreateThread("arakiri",unload,0x11,0x1000,0,0),0,NULL);//to be able to return
	return NULL;
}
int module_start(int args,void*argp){
	if(args!=sizeof(OpenTube*))return 1;//return if no context
	dmx.f=&f;
	ot=(OpenTube*)((u32*)argp)[0];
	ot->sys->onLoad=onLoad;
	ot->sys->onPlay=onPlay;
	ot->sys->onSeek=onSeek;
	ot->sys->onStop=onStop;
	ot->dmx=&dmx;
	puts("mp4 demux loaded");
	return 0;
}