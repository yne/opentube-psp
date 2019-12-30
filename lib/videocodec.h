#ifndef VIDEOCODEC_H

#define PSP_CODEC_MPEG4 (0x00000000)
#define PSP_CODEC_AVC   (0x00000001)
#define PSP_CODEC_UNK   (0x00000002)
#define PSP_CODEC_JPEG  (0x00000003)

typedef struct{//size:0x60
/* 0*/	u32  version;// 0x05100601
/* 1*/	int  unk1;//0xD4 (53*(void*) ?)
/* 2*/	void*unk2;//0
/* 3*/	int  EDRAM;//0x198A40 == [5]
/* 4*/	Mp4AvcInfoStruct*info;//size = arg2 ? 0x100:0x28; (contain video info w/h)
/* 5*/	int  EDRAM64;//EDRAM related?(memalign 64)
/* 6*/	int  EDRAMlen;//0x8344 sizeof needed EDRAM
/* 7*/	void*init;//init stuff ?
/* 8*/	int  initLen;//sizeof init sutff
/* 9*/	char*sample;//0x01000000+sample content
/*10*/	int  sampleLen;
/*11*/	Mp4AvcYuvStruct*yuv;//size=entry*0x2C
/*12*/	void*unk12;//size=0x64|0x40
/*13*/	int  cscMode;//0 csc mode
/*14*/	void*unk14;//size=entry*0x148 (0x540)
/*15*/	int  entry;//4 (static)
/*16*/	int  maxWidth;//480/720
/*17*/	int  maxHeight;//272/480/576
/*18*/	int  maxEntry;//3
/*19*/	int  haveSEI;//1 if mode == 3,5,6
/*20*/	void*SEI;//size=0xC0 Supplemental enhancement information (optional)
/*21*/	void*crop;//0x40 frame crop (optional)
/*22*/	int  csc;//manual:1 (csc)
/*23*/	void*ex;//unused (memalign 64)//EDRAM*?
}VCodec;

int sceVideocodecInit (VCodec*v,int mode);
int sceVideocodecGetFrameCrop (VCodec*v,int mode);
int sceVideocodecGetVersion (VCodec*v,int mode);
int sceVideocodecScanHeader (VCodec*v,int mode);
int sceVideocodecDelete (VCodec*v,int mode);
int sceVideocodecGetSEI (VCodec*v,int mode);
int sceVideocodecSetMemory (VCodec*v,int mode);
int sceVideocodec_893B32B1 (VCodec*v,int mode);
int sceVideocodecStop (VCodec*v,int mode);
int sceVideocodecOpen (VCodec*v,int mode);
int sceVideocodecDecode (VCodec*v,int mode);
int sceVideocodecGetEDRAM (VCodec*v,int mode);
int sceVideocodecReleaseEDRAM (VCodec*v);
#define VIDEOCODEC_H
#endif
