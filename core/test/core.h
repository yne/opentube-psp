#ifndef MAIN_H
#define MAIN_H
#include <pspmpeg.h>
#include <audiocodec.h>
#include <mpegbase.h>
#include <audiocodec.h>

#define TEST_SERVER "192.168.0.100"
#define $(A) sceIoWrite(2,A,sizeof(A))

#define HTTP_SAVE_RAM  0x10000//return pointer to the whole file content
#define HTTP_SAVE_FILE 0x20000//return saved file path

#define Memalign ot->sys->memalign
#define Realloc ot->sys->realloc
#define Malloc ot->sys->malloc
#define Free ot->sys->free

#define Modload ot->sys->modload
#define Modstun ot->sys->modstun
#define FindCodec ot->sys->findCodec
#define Play ot->sys->play
#define SUDO_STARTME 1
#define SUDO_SETFREQ 2
#define Sudo ot->sys->sudo
#define CWD ot->sys->cwd
#define Debug(msg) ot->sys->print(msg,3)
#define Alert(msg) ot->sys->print(msg,2)
#define Error(msg) ot->sys->print(msg,1)
#define Fatal(msg) ot->sys->print(msg,0)
#define Exit ot->sys->stop
#define Mode ot->sys->mode
#define Print(A) sceIoWrite(2,A,sizeof(A))

#define Open ot->io->open
#define Seek ot->io->seek
#define Read ot->io->read
#define ReadX ot->io->readx
#define Write ot->io->write
#define Close ot->io->close
#define Str ot->io->str

#define Draw ot->gui->draw
#define Edit ot->gui->edit

typedef struct{//MP4
	u32 scLen,*scFst,*scSpc,*scSid;//SampleToChunk
	u32 szLen, szCnt,*sz;//SampleSize
	u32 coLen,*co;//ChunkOffset
}St;
typedef struct{//st
	int Acodec,ch,hz,bps;
	int Vcodec,profile;
	St a,v;
}MP4;
typedef struct{//vcodec
	char*(*load)();
	char*(*play)();
	char*(*seek)(int t,int o);
	char*(*stop)();
}Vcodec;
typedef struct{//acodec
	char*(*load)();
	char*(*play)();
	char*(*seek)(int t,int o);
	char*(*stop)();
	int chan;
	AuCodec au;
}Acodec;
typedef struct{//demuxer
	char*(*load)();
	char*(*play)();
	char*(*seek)(int t,int o);
	char*(*stop)();
	void*(*getASample)(u32 s,u32*len);
	void*(*getVSample)(u32 s,u32*len);
	int Alen,hz;
	int Vlen,fps,width,height,profil;	
	Acodec* a;
	Vcodec* v;
//	MP4* f;
//	int fd;
}Demuxer;
typedef struct{//Graphic
	char*(*init)();
	char*(*stop)();
	char*(*draw)(int type);
	char*(*edit)(int what,void*arg);
}Graphic;
typedef struct{//FileSys
	int(*open)(char* path,int mode,int flag);
	int(*seek)(int fd,int type,int len);
	int(*read)(int fd,void*p,int len);
	int(*readx)(void**p,int pos,int len);
	int(*write)(int fd,void*p,int len);
	int(*close)(int fd);
	int(*unload)();
	struct {int fd,th,curr,start,end,run,size;void*buf,*tmp;}str;
}FileSys;
typedef struct{//sudoArg
	int mode;
	u32 nid;
	int param;
}sudoArg;
typedef struct{//Nal
	char*sps;
	int  spsLen;
	char*pps;
	int  ppsLen;
	int  nalPreLen;
	void*nal;
	u32  nalLen;
	int  mode;
}Nal;
typedef struct{//Mp4AvcInfoStruct
	int unknown0;
	int unknown1;
	int width;
	int height;
	int unknown4;
	int unknown5;
	int unknown6;
	int unknown7;
	int unknown8;
	int unknown9;
} Mp4AvcInfoStruct;
typedef struct{//Mp4AvcYuvStruct
	void* b0;
	void* b1;
	void* b2;
	void* b3;
	void* b4;
	void* b5;
	void* b6;
	void* b7;
	int unknown0;
	int unknown1;
	int unknown2;
} Mp4AvcYuvStruct;
typedef struct{//SceMpegAvcDetail2
	int unknown0;
	int unknown1;
	int unknown2;
	int unknown3;
	Mp4AvcInfoStruct* info;
	int unknown5;
	int unknown6;
	int unknown7;
	int unknown8;
	int unknown9;
	int unknown10;
	Mp4AvcYuvStruct* yuv;
	int unknown12;
	int unknown13;
	int unknown14;
	int unknown15;
	int unknown16;
	int unknown17;
	int unknown18;
	int unknown19;
	int unknown20;
	int unknown21;
	int unknown22;
	int unknown23;
} SceMpegAvcDetail2;
typedef struct{//Mp4AvcCscStruct
	int height;
	int width;
	int mode0;
	int mode1;
	void* b0;
	void* b1;
	void* b2;
	void* b3;
	void* b4;
	void* b5;
	void* b6;
	void* b7;
} Mp4AvcCscStruct;
typedef struct{//CoreSys
	char*err;
//	void*mePool;
	char cwd[256];
	int mode;//1:menu,2:playback
	int  (*stop)();//stop playback,unload core+dependency
	int  (*modstun)(u32 modid);
	int  (*modload)(char* path);
	int  (*print)(char*str,int lv);
	void (*free)(void*p);
	void*(*realloc)(void*p,unsigned s);//don't use it on memaligned *p
	void*(*memalign)(unsigned boundary,unsigned s);
	void*(*malloc)(unsigned s);
	char*(*sudo)(int fun,u32 nid,int value);
	char*(*findCodec)(char* name,unsigned id);
	char*(*play)(char* url);
//	char*(*onLoad)(char*file);//called after demux load
//	char*(*onSeek)(int type,int off);//fastforward and time control stuff
//	char*(*onPlay)();//called to switch between play/pause
//	char*(*onStop)();//stop playback and self unload (including dependencies)
	int  _pad;
	int  pad;
}CoreSys;
typedef struct{//Display
	u32 curr;//disp/draw
	u32 draw;
	u32 disp;
	u32 size;//512/720
	int type;//16b/32b
	int vram;//offset
}Display;
typedef struct{//MeCtx
	int   chan;
	int   mode;
	int   pics;
	void* pool;
	int   buffLen;
	void* buff;
	void* buffAu;
	u32   bootNid;
	u32   freqNid;
	SceMpeg mpeg;
	SceMpegAu* mpegAu;
	SceMpegAvcDetail2*d;
	SceMpegRingbuffer ring;
	AuCodec aucd;
	Nal nal;
}MeCtx;
typedef struct{//OpenTube
	CoreSys*sys;
	Display*lcd;
	Demuxer*dmx;
	Graphic*gui;
	FileSys*io;
	MeCtx*me;
}OpenTube;
OpenTube*otGetCtx();
OpenTube*ot;
#endif