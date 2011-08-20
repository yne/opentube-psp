#include <pspkernel.h>
#include <pspjpeg.h>
#include <psputility.h>
#include <stdio.h>
#include <string.h>
#include "jpg.h"
#include "core.h"
#define MIN(A,B) (A<B?A:B)

void*jpg2buf(char*path,int*w,int*h){
	int fd=0,size,i=0,_w=1;*w=0,*h=0;char*res=NULL;u8*f=NULL;
	if(sceJpegInitMJpeg())return "jpgInit";
	if((fd=sceIoOpen(path,PSP_O_RDONLY,0777))<0){res="fileErr";goto err;}
	if(!(f=Malloc((size=sceIoLseek32(fd,0,SEEK_END))))){res="malloc";goto err;}
	sceIoLseek32(fd,0,SEEK_SET);
	sceIoRead(fd,f,size);
	if(!(f[i++]==0xFF&&f[i++]==0xD8&&f[i++]==0xFF&&f[i++]==0xE0)){res="!SOI";goto err;}
	if(!(f[i+2]=='J' &&f[i+3]=='F' &&f[i+4]=='I' &&f[i+5]=='F' )){res="!JFIF";goto err;}
	for(int len=(f[i]<<8)+f[i+1];i+=len;){//seek to 
		if(i>=size){res="!size";goto err;}//Check to protect against segmentation faults
		if(f[i]!=0xFF){res="!start";goto err;}//Check that we are truly at the start of another block
		if(f[i+1]==0xC0){//0xFFC0 is the "Start of frame" marker which contains the file size
		//The structure of the 0xFFC0 block is quite simple [0xFFC0][ushort length][uchar precision][ushort x][ushort y]
			*h=(f[i+5]<<8)+f[i+6];
			*w=(f[i+7]<<8)+f[i+8];
			break;
		}else{
			i+=2;//Skip the block marker
			len=(f[i]<<8)+f[i+1];//Go to the next block
		}
	}
	while((_w<<=1)<*w);
	if(sceJpegCreateMJpeg(*w,*h)){*w=0;res="creaMJp";goto err;}
	if(!(res=Memalign(64,(_w)*(*h)*4))){*w=0;res="!memal";goto err;}
	if(sceJpegDecodeMJpeg(f,size,res,0)<0){Free(res);*w=0;res="!decMJp";goto err;}
	sceJpegDeleteMJpeg();
err:
	if(f)Free(f);
	if(fd)sceIoClose(fd);
	sceJpegFinishMJpeg();
	return res;
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