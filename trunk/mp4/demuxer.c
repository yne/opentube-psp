#include <pspkernel.h>
#include <pspdisplay.h>
#include <stdio.h>
#include <string.h>
#include <pspctrl.h>
#include <psputility.h>
#include "core.h"
#define try(A) do{A}while(0);
#define OR :case
#define sw(n) (((n>>24)&0xff)|((n<<8)&0xff0000)|((n>>8)&0xff00)|((n<<24)&0xff000000))
#define sw64(n) (((n<<56)&0xFF)|((n<<48)&0xFF00)|((n<<40)&0xFF0000)|((n<<32)&0xFF000000)|((n<<24)&0xFF00000000)|((n<<16)&0xFF0000000000)|((n<<8)&0xFF000000000000)|((n<<0)&0xFF00000000000000))

char dummy_audio[]={0x21,0x00,0x49,0x90,0x02,0x19,0x00,0x23,0x80};
char dummy_video[]={0x00,0x08,0x41,0x9a,0x04,0x0e,0x1f,0x80,0x7f,0xe0};

PSP_MODULE_INFO("OpenTube.demux.mp4",0,0,0);
int fd;unsigned size,type,tmp;//atom parsing
char curType,*c=NULL;//atom content buffer
MP4 f;St*st;
void*video,*audio;u32*Voff,*Aoff;
int sm;
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
			st=NULL;
			if(c[8]=='s')st=&f.a;
			if(c[8]=='v')st=&f.v;
		}break;
		case 0x73747364:{get();//stsd
			if(curType=='s'){
				f.Acodec=sw(((int*)c)[3]);
				if(c[12]=='m'&&c[13]=='p'&&c[14]=='4'&&c[15]=='a'){
					f.ch=c[15+18];
					f.bps=c[15+20];
					ot->dmx->hz=((c[15+25]<<8)+(c[15+26]))&0xFFFF;
				}else{
					Alert("unkAfmt\n");
				}
			}else	if(curType=='v'){
				f.Vcodec=sw(((int*)c)[3]);
				ot->dmx->fps=tmp;
				ot->dmx->width=sw(((int*)c)[10])>>16;
				ot->dmx->height=sw(((int*)c)[10])&0xFFFF;
				if((c[98]=='a')&&(c[99]=='v')&&(c[100]=='c')&&(c[101]=='C')){
					f.profile=*(c+103);
					ot->me->nal.nalPreLen=1+((*(c+106))&0x03);
					ot->me->nal.sps=Malloc((ot->me->nal.spsLen=(*(c+108)<<8)+*(c+109)));
					for(tmp=0;tmp<ot->me->nal.spsLen;tmp++)
						ot->me->nal.sps[tmp]=*(c+110+tmp);
					ot->me->nal.pps=Malloc((ot->me->nal.ppsLen=(*(c+111+tmp)<<8)+*(c+112+tmp)));
					for(int i=0;i<ot->me->nal.ppsLen;i++)
						ot->me->nal.pps[i]=*(c+113+tmp+i);
				}else{
					Alert("unkVfmt\n");
				}
			}
		}break;
		case 0x73747363:{get();//stsc
			if(!st)break;
			st->scLen=sw(((int*)c)[1]);
			st->scFst=Malloc((st->scLen+1)*sizeof(u32));st->scFst[st->scLen]=0;
			st->scSpc=Malloc(st->scLen*sizeof(u32));
			st->scSid=Malloc(st->scLen*sizeof(u32));
			for(int i=0;i<st->scLen;i++){
				st->scFst[i] = sw(((int*)c)[2+i*3]);
				st->scSpc[i] = sw(((int*)c)[3+i*3]);
				st->scSid[i] = sw(((int*)c)[4+i*3]);
			}
		}break;
		case 0x7374737a:{get();//stsz
			if(!st)break;
			st->szCnt=sw(((int*)c)[1]);//0 on const size
			st->szLen=sw(((int*)c)[2]);
			st->sz=Malloc(st->szLen*sizeof(u32));
			for(int i=0;i<st->szLen;i++)
				st->sz[i] = sw(((int*)c)[3+i]);
		}break;
		case 0x7374636f:{get();//stco
			if(!st)break;
			st->coLen=sw(((int*)c)[1]);
			st->co=Malloc(st->coLen*sizeof(u32));
			for(int i=0;i<st->coLen;i++)
				st->co[i] = sw(((int*)c)[2+i]);
		}break;
		case 0x6d646174:return NULL;//mdat
		default:skip();break;
	}
	return parse();
}
u32 *rebuildSpc(St*st){
	u32*table=Malloc(st->szLen*sizeof(u32));
	for(int c=0,p=0,s=0;c<st->coLen;c++){//p=position in the sample2chunk table
		if(st->scFst[p+1]==c+1)p++;
		for(int i=0,off=0;i<st->scSpc[p];i++){
			table[s]=st->co[c]+off;
			off+=st->sz[s++];
		}
	}
	return table;
}
void*getVSample(u32 s,u32*len){
	video=Realloc(video,(*len=f.v.sz[s]));
	ReadX(&video,Voff[s],f.v.sz[s]);//read from ring buffer
	return video;
}
void*getASample(u32 s,u32*len){
	audio=Realloc(audio,(*len=f.a.sz[s]));
	ReadX(&audio,Aoff[s],f.a.sz[s]);//read from ring buffer
	return audio;
}
char*onLoad(char* file){
	Alert("onLoad ");
	if((fd=Open(file,PSP_O_NOWAIT|PSP_O_RDONLY,0777))<0)return "fileErr";
	char*err;
	if((err=parse()))return err;
	Alert("parsed!\n");
	sceUtilityLoadAvModule(0);//implying sceMeBootStart(2)
	do{
		if((err=FindCodec(NULL,f.Acodec)))break;//return err;
		if((err=ot->dmx->a->load()))break;
		Aoff=rebuildSpc(&f.a);
		ot->dmx->Alen=f.a.szLen;
	}while(0);
	do{
		if((err=FindCodec(NULL,f.Vcodec)))break;//return err;
		if((err=ot->dmx->v->load()))return err;
		Voff=rebuildSpc(&f.v);
		ot->dmx->Vlen=f.v.szLen;
	}while(0);
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
	int ath,vth;sm=0;//sm=sceKernelCreateSema("test",0x0,0,4,NULL);
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
	Free(ot->me->nal.sps);Free(ot->me->nal.pps);
	Free(f.v.scFst);Free(f.a.scFst);
	Free(f.v.scSpc);Free(f.a.scSpc);
	Free(f.v.scSid);Free(f.a.scSid);
	Free(f.v.sz);   Free(f.a.sz);
	Free(f.v.co);   Free(f.a.co);
	if(ot->dmx->a&&ot->dmx->a->stop)ot->dmx->a->stop();
	if(ot->dmx->v&&ot->dmx->v->stop)ot->dmx->v->stop();
	sceUtilityUnloadAvModule(0);//implying sceMeBootStart(4)
	Close(fd);
	ot->dmx=NULL;
	sceKernelStartThread(sceKernelCreateThread("arakiri",unload,0x11,0x10000,0,0),0,NULL);//to be able to return
	return NULL;
}
Demuxer dmx={onLoad,onPlay,onSeek,onStop,getASample,getVSample};
int  module_stop(){return 0;}
int  module_start(int args,void*argp){
	if((ot=otGetCtx())<0)return 1;
	Alert("mp4 demux loaded\n");
	ot->dmx=&dmx;
	return 0;
}