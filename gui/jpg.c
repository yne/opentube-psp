#include <pspkernel.h>
#include <pspjpeg.h>
#include <psputility.h>
#include <stdio.h>
#include <string.h>
#include "jpg.h"
#include "core.h"
#define MIN(A,B) (A<B?A:B)

void*jpg2buf(char*path,int*w,int*h){
	int fd,i=0,_w=1;
	if((fd=sceIoOpen(path,PSP_O_RDONLY,0777))<0)return NULL;
	sceJpegInitMJpeg();
	int size=sceIoLseek32(fd,0,SEEK_END);
	u8* data=Malloc(size);
	sceIoLseek32(fd,0,SEEK_SET);
	sceIoRead(fd,data,size);
	if(!(data[i++]==0xFF&&data[i++]==0xD8&&data[i++]==0xFF&&data[i++]==0xE0))goto err;//invalid SOI header
	if(!(data[i+2]=='J' &&data[i+3]=='F' &&data[i+4]=='I' &&data[i+5]=='F' ))goto err;//invalid JFIF
	for(int len=(data[i]<<8)+data[i+1];i+=len;){//seek to 
		if(i>=size)goto err;//Check to protect against segmentation faults
		if(data[i] != 0xFF)goto err;//Check that we are truly at the start of another block
		if(data[i+1] == 0xC0){//0xFFC0 is the "Start of frame" marker which contains the file size
		//The structure of the 0xFFC0 block is quite simple [0xFFC0][ushort length][uchar precision][ushort x][ushort y]
			*h=(data[i+5]<<8)+data[i+6];
			*w=(data[i+7]<<8)+data[i+8];
			break;
		}else{
			i+=2;//Skip the block marker
			len=(data[i]<<8)+data[i+1];//Go to the next block
		}
	}
	while((_w<<=1)<*w);
	sceJpegCreateMJpeg(*w,*h);
	void*raw=Memalign(64,(_w)*(*h)*4);
	sceJpegDecodeMJpeg(data,size,raw,0);
	Free(data);
	sceJpegDeleteMJpeg();
	sceJpegFinishMJpeg();
	sceIoClose(fd);
	return raw;
err:
	sceJpegFinishMJpeg();
	sceIoClose(fd);
	return 0;
}
void*addAlpha(void*buf,int bw,int bh,void*alpha,int aw,int ah){
	if(!buf)return buf;
	if(!alpha)for(int y=0;y<bw*bh;y++)*((int*)buf+y)|=0xFF000000;	
	else
		for(int y=0;y<ah;y++)
			for(int x=0;x<aw;x++)
			 *((int*)buf+x+y*bw)|=*((int*)alpha+x+y*aw)<<24;
	return buf;
}
void*jpgOpen(char*path,char*apath,int*bw,int*bh){
	sceUtilityLoadAvModule(0);//implying sceMeBootStart(2)
	int aw=0,ah=0;
	void*color=jpg2buf(path,bw,bh);
	void*alpha=jpg2buf(apath,&aw,&ah);
	void*muxed=addAlpha(color,*bw,*bh,alpha,aw,ah);
	if(alpha)Free(alpha);
	sceUtilityUnloadAvModule(0);//implying sceMeBootStart(4)
	return muxed;	
}