#include <pspkernel.h>
#include <pspdisplay.h>
#include <stdio.h>
#include <string.h>
#include <pspctrl.h>
#include <psputility.h>
#include "demuxer.h"
#include "core.h"
#define try(A) do{A}while(0);
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
MP4 f;
void*video,*audio;
int sm,ath,vth;
u32  read32(){
	u32 i;
	Read(fd,&i,4);
	return sw(i);
}
int  get(){
	return (c=Realloc(c,size))?Read(fd,c,size):Alert("Malloc error\n");
}
void skip(){
	Seek(fd,size,SEEK_CUR);
}
char*parse(){
	size=read32()-8;
	type=read32();
//	printf("size:%08X type:%08X\n",size,type);
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
					Alert("unkAfmt\n");
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
					Alert("unkVfmt\n");
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
char*rebuildVSpsc(){
	u32*spc=Malloc(f.VstszLen*sizeof(u32));//sample per chunk
	u32*fst=Malloc(f.VstszLen*sizeof(u32));//first sample (absolute)
	if(!spc||!fst)return("malloc error !\n");
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
	if(!sid)return("malloc2 error\n");
	for(int i=0;i<f.VstcoLen;i++){
		int size=0;
		for(int j=f.VstscFst[i];j<f.VstscFst[i]+f.VstscSpc[i];j++)size+=f.Vstsz[j];
		sid[i]=size;
	}
	Free(f.VstscSid);
	f.VstscSid=sid;
	return NULL;
}
char*rebuildASpsc(){
	u32*spc=Malloc(f.AstszLen*sizeof(u32));//sample per chunk
	u32*fst=Malloc(f.AstszLen*sizeof(u32));//first sample (absolute)
	if(!spc||!fst)return("malloc error !\n");
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
	if(!sid)return("malloc2 error\n");
	for(int i=0;i<f.AstcoLen;i++){
		int size=0;
		for(int j=f.AstscFst[i];j<f.AstscFst[i]+f.AstscSpc[i];j++)size+=f.Astsz[j];
		sid[i]=size;
	}
	Free(f.AstscSid);
	f.AstscSid=sid;
	return NULL;
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
	Alert("done B\n");
	Free(f.VstscSid);
	f.VstscSid=sid;	
}
void*getVSample(int s){
	int t,pos=0;
	for(t=0;(t<f.VstcoLen)&&(f.VstscFst[t]<=s);t++);//seek to trunk+1
	for(int i=f.VstscFst[t-1];i<s;i++)pos+=f.Vstsz[i];//seek to sample in trunk
	video=Realloc(video,f.Vstsz[s]);
//	printf("%i %08X %08X\n",s,f.Vstco[t-1]+pos,f.Vstsz[s]);
	if(fd>64){
		ReadX(video,f.Vstco[t-1]+pos,f.Vstsz[s]);//read from ring buffer
	}else{
		Seek(f.Vfd,f.Vstco[t-1]+pos,SEEK_SET);
		Read(f.Vfd,video,f.Vstsz[s]);
	}
	return video;
}
void*getASample(int s){
	int t,pos=0;
	for(t=0;(t<f.AstcoLen)&&(f.AstscFst[t]<=s);t++);//seek to trunk+1
	for(int i=f.AstscFst[t-1];i<s;i++)pos+=f.Astsz[i];//seek to sample in trunk
	audio=Realloc(audio,f.Astsz[s]);
	if(fd>64){
		ReadX(audio,f.Astco[t-1]+pos,f.Astsz[s]);//read from ring buffer
	}else{
		Seek(f.Afd,f.Astco[t-1]+pos,SEEK_SET);
		Read(f.Afd,audio,f.Astsz[s]);	
	}
	return audio;
}
char*onLoad(char* file){
	Alert("onLoad\n");
	if(Str.th){fd=Str.th;Str.curr=0;}//stream thread is runing, use it !
	else if((fd=Open(file,/*PSP_O_NOWAIT|*/PSP_O_RDONLY,0777))<0)return "fileErr";
	char*err;
	if((err=parse()))return err;
	sceUtilityLoadAvModule(0);//implying sceMeBootStart(2)
	do{
		if((err=FindCodec(NULL,f.Acodec)))break;//return err;
		if((f.Afd=fd>64?fd:Open(file,PSP_O_RDONLY,0777))<0)return "fileErr";
		if((err=rebuildASpsc()))break;//rebuild(f.AstszLen,f.Astsz,f.AstcoLen,f.AstscSpc,f.AstscFst);
		if((err=ot->dmx->a->load()))break;
	}while(0);
	do{
		if((err=FindCodec(NULL,f.Vcodec)))break;//return err;
		if((f.Vfd=fd>64?fd:Open(file,PSP_O_RDONLY,0777))<0)return "fileErr";
		if((err=rebuildVSpsc()))return err;//rebuild(f.VstszLen,f.Vstsz,f.VstcoLen,f.VstscSpc,f.VstscFst);
		if((err=ot->dmx->v->load()))return err;
	}while(0);
//	ot->lcd->type=PSP_DISPLAY_PIXEL_FORMAT_8888;
//	ot->lcd->size=f.width>480?720:512;
	return err;
}
int  Video(unsigned args,void*argp){
	sm++;//sceKernelSignalSema(sm,1);
	if(ot->dmx->v->play)ot->dmx->v->play();else Alert("noVplay");
	sm--;//sceKernelSignalSema(sm,-1);
	Alert("v ok\n");
	return 0;
}
int  Audio(unsigned args,void*argp){
	sm++;//sceKernelSignalSema(sm,1);
	if(ot->dmx->a->play)ot->dmx->a->play();else Alert("noAplay");
	sm--;//sceKernelSignalSema(sm,-1);
	Alert("a ok\n");
	return 0;
}
int  ctrlOk(){
	if(ot->sys->pad!=ot->sys->_pad)
		if(ot->sys->pad&PSP_CTRL_CIRCLE)return 0;
	return 1;
}
char*onPlay(){
	Alert("start playback\n");
	//for(int i=0;i<100;i++)printf("%i %08X\n",i,f.Vstsz[i]);
	sm=0;//sm=sceKernelCreateSema("test",0x0,0,4,NULL);
	sceKernelStartThread(ath=sceKernelCreateThread("OpenTube.out.video",Video,0x20,0x10000,0,0),0,NULL);
	sceKernelStartThread(vth=sceKernelCreateThread("OpenTube.out.audio",Audio,0x20,0x10000,0,0),0,NULL);
	Alert("wait\n");
	do sceDisplayWaitVblankStart();//sceKernelWaitSema(sm,0,0);
	while(sm);//sceKernelDeleteSema(sm);
	sceKernelTerminateDeleteThread(ath);//self exit & terminate both lead to a crash
	sceKernelTerminateDeleteThread(vth);//so i kill them for here
	Alert("end\n");
	return ot->sys->err;
}
char*onSeek(int w,int o){
	Alert("seek...\n");
	ot->dmx->a->seek(w,o);
	ot->dmx->v->seek(w,o);
	return 0;
}
int  unload(SceSize args,void*argp){
	do sceKernelDelayThread(1000);
	while(sceKernelSelfStopUnloadModule(1,0,NULL)<0);
	return 0;
}
char*onStop(){
	Alert("unloading demuxer\n");
	Realloc(video,0);Realloc(audio,0);
//free atom list
	Free(f.sps);Free(f.pps);
	Free(f.VstscFst);Free(f.AstscFst);
	Free(f.VstscSpc);Free(f.AstscSpc);
	Free(f.VstscSid);Free(f.AstscSid);
	Free(f.Vstsz);   Free(f.Astsz);
	Free(f.Vstco);   Free(f.Astco);
	if(ot->dmx->a->stop)ot->dmx->a->stop();
	if(ot->dmx->v->stop)ot->dmx->v->stop();
	sceUtilityUnloadAvModule(0);//implying sceMeBootStart(4)
	if(fd<64){
		if(f.Afd>0)Close(f.Afd);
		if(f.Vfd>0)Close(f.Vfd);
	}else Close(fd);
	ot->dmx=NULL;
	sceKernelStartThread(sceKernelCreateThread("arakiri",unload,0x11,0x10000,0,0),0,NULL);//to be able to return
	return NULL;
}
Demuxer dmx={onLoad,onPlay,onSeek,onStop,getASample,getVSample};
int  module_start(int args,void*argp){
	if((ot=otGetCtx())<0)return 1;
	Alert("mp4 demux loaded\n");
	dmx.f=&f;
	ot->dmx=&dmx;
	return 0;
}