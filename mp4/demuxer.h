#ifndef DEMUXER_H
typedef struct{
	u32 time,nalPreLen;
	u32 spsLen,ppsLen;
	u8* sps, * pps;
	
	int Ath,Afd,Acodec,ch,hz,bps;
	u32 AstscLen,*AstscFst,*AstscSpc,*AstscSid;//SampleToChunk
	u32 AstszLen, AstszCnt,*Astsz;//SampleSize
	u32 AstcoLen,*Astco;//ChunkOffset

	int Vth,Vfd,Vcodec,fps,delay,profile,width,height;
	u32 VstscLen,*VstscFst,*VstscSpc,*VstscSid;//SampleToChunk
	u32 VstszLen, VstszCnt,*Vstsz;//SampleSize
	u32 VstcoLen,*Vstco;//ChunkOffset
}MP4;
#ifndef VCODEC_H
typedef void Vcodec;
#endif
#ifndef ACODEC_H
typedef void Acodec;
#endif
typedef struct{
	void*(*getASample)(int s);
	void*(*getVSample)(int s);
	int Alen,Ahz;
	int Vlen,Vfps;	
	Acodec* a;
	Vcodec* v;
	char*(*Aload)();
	char*(*Vload)();
	char*(*Aplay)();
	char*(*Vplay)();
	char*(*Aseek)(int t,int o);
	char*(*Vseek)(int t,int o);
	char*(*Astop)();
	char*(*Vstop)();
	MP4* f;
//	int fd;
}Demuxer;
extern void*getVSample(int s);
extern void*getASample(int s);
#define DEMUXER_H
#endif