#ifndef MAIN_H
#define MAIN_H
#include <pspmpeg.h>
#include <audiocodec.h>
#include <mpegbase.h>

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
#define Alert ot->sys->print
#define Exit ot->sys->stop
#define Mode ot->sys->mode

#define Open ot->io->open
#define Read ot->io->read
#define Seek ot->io->seek
#define Close ot->io->close

#define Draw ot->gui->draw

#ifndef DEMUXER_H
typedef void Demuxer;
#endif
typedef struct{
	char*(*init)();
	char*(*stop)();
	char*(*draw)(int type);
}Graphic;
typedef struct{
	int(*open)(const char* path,int mode,int flag);
	int(*read)(int fd,void*p,int len);
	int(*seek)(int fd,int type,int len);
	int(*close)(int fd);
}FileSys;
typedef struct{
	int mode;
	u32 nid;
	int param;
}sudoArg;
typedef struct{
	void*sps;
	int  spsLen;
	void*pps;
	int  ppsLen;
	int  nalPreLen;
	void*nal;
	int  nalLen;
	int  mode;
}Nal;
typedef struct{
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
typedef struct{
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
typedef struct{
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
typedef struct{
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
typedef struct{
	char*err;
//	void*mePool;
	char cwd[256];
	int mode;//1:menu,2:playback
	int  (*stop)();//stop playback,unload core+dependency
	int  (*modstun)(u32 modid);
	int  (*modload)(const char* path);
	int  (*print)(char*str);
	void (*free)(void*p);
	void*(*realloc)(void*p,unsigned s);
	void*(*malloc)(unsigned s);
	char*(*sudo)(int fun,u32 nid,int value);
	char*(*findCodec)(const char* name,unsigned id);
	char*(*play)(const char* url);
	char*(*onLoad)(const char*file);//called after demux load
	char*(*onSeek)(int type,int off);//fastforward and time control stuff
	char*(*onPlay)();//called to switch between play/pause
	char*(*onStop)();//stop playback and self unload (including dependencies)
	int  _pad;
	int  pad;
}CoreSys;
typedef struct{
	u32 curr;//disp/draw
	u32 disp;
	u32 draw;
	u32 size;//512/720
	int type;//16b/32b
	int vram;//offset
}Display;
typedef struct{
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
}MeCtx;
typedef struct{
	CoreSys*sys;
	Display*lcd;
	Demuxer*dmx;
	Graphic*gui;
	FileSys*io;
	MeCtx*me;
}OpenTube;
OpenTube*ot;
#endif