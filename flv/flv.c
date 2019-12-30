#include <stdio.h>
#include <fcntl.h>
#include "flv.h"
#define BE(x) ((x>>24)|((x<<8)&0x00FF0000)|((x>>8)&0x0000FF00)|(x<<24))
#define BE16(x) (((x<<8)&0xFF00)|((x>>8)&0xFF))
//#define printf(...); ;
int fd;
//unsigned BE(unsigned*x){return (x[0]>>24) | ((x[0]<<8) & 0x00FF0000) | ((x[0]>>8) & 0x0000FF00) | (x[0]<<24);}
unsigned*Astsz,*Astco,*Vstsz,*Vstco;
unsigned Alen=0,Vlen=0;
parseA(int size){
	ATag au;
	int end=lseek(fd,0,SEEK_CUR)+size;
	read(fd,&au,sizeof(au));size-=sizeof(au);
	if(au.fmt==10){
		char type;
		read(fd,&type,sizeof(char));size-=sizeof(char);
		//printf("%s ",type?"data":"info",size);
		if(!type)return lseek(fd,end,SEEK_SET);//AAC header
	}
	Alen++;
	Astsz=(unsigned*)realloc(Astsz,Alen*sizeof(unsigned));
	Astco=(unsigned*)realloc(Astco,Alen*sizeof(unsigned));
	Astsz[Alen-1]=size;
	Astco[Alen-1]=end-size;
	//printf("[%i] %i %s %s",au.fmt,au.khz,au.bps?"16b":"8b",au.ch?"stereo":"mono");	
	return lseek(fd,end,SEEK_SET);
}
parseV(int size){
	VTag vi;
	int end=lseek(fd,0,SEEK_CUR)+size;
	read(fd,&vi,sizeof(VTag));         size-=sizeof(VTag);
//	printf("video [%i]",vi.codec);
	if(vi.codec==7){
		unsigned char pkgType,compoTime[3];
		read(fd,&pkgType,sizeof(char));
//		printf(" %s ",pkgType?"NALU":"header");
		read(fd,&compoTime,sizeof(compoTime));
		size-=sizeof(compoTime)+sizeof(char);
//		if(pkgType==1)printf(" %03X",compoTime);
		if(pkgType==0){//header
			DCR cfg;
			read(fd,&cfg,sizeof(DCR));//len+1=avc_nal_prefix_size
//			printf("ver:%i ",cfg.ver);
//			printf("profile:%02X ",cfg.AVCprofile);
//			printf("compatibility:%02X ",cfg.AVCprofileCompatibility);
//			printf("level:%02X ",cfg.AVClevel);
			unsigned short sps_size,pps_size;unsigned char num;
			read(fd,&sps_size,sizeof(sps_size));
			unsigned char*sps=(unsigned char*)malloc((sps_size=BE16(sps_size)));
			read(fd,sps,sizeof(sps_size));
			read(fd,&num,sizeof(num));//numOfPictureParameterSets
			read(fd,&pps_size,sizeof(sps_size));
			unsigned char*pps=(unsigned char*)malloc((pps_size=BE16(pps_size)));
			read(fd,pps,sizeof(pps_size));
			return lseek(fd,end,SEEK_SET);
		}
	}
	Vlen++;
	Vstsz=(unsigned*)realloc(Vstsz,Vlen*sizeof(unsigned));
	Vstco=(unsigned*)realloc(Vstco,Vlen*sizeof(unsigned));
	Vstsz[Vlen-1]=size;
	Vstco[Vlen-1]=end-size;
	return lseek(fd,end,SEEK_SET);
}
parseS(int size){
//	printf("%08X\n",lseek(fd,0,SEEK_CUR));
	return lseek(fd,size,SEEK_CUR);
	lseek(fd,size,SEEK_CUR);
//	printf("script\n");
}
//#undef printf(...);
parseR(int size){
//	printf("unk tagType %08X %i\n",size,type);
	return lseek(fd,size,SEEK_CUR);//	exit(0);
}
printInfo(){
	printf("--- A U D I O ---  --- V I D E O ---\n");
	printf(" offset    size     offset    size  \n");
	int max=Alen>Vlen?Alen:Vlen;
	int i;for(i=0;i<max;i++){
		if(i<Alen)printf("%08X %08X  ",Astco[i],Astsz[i]);else printf("                   ");
		if(i<Vlen)printf("%08X %08X\n",Vstco[i],Vstsz[i]);else printf("                 \n");
	}
	printf("   total %8i     total %8i\n",Alen,Vlen);
}
int main(int argc,char* argv[]){
	if(argc<2)return printf("Usage :flv-demuxer video.flv");
	fd=open("240.flv",O_RDONLY,0777);
	int len=lseek(fd,0,SEEK_END);
	lseek(fd,9,SEEK_SET);
	Tag t;int i=0;
	do{
		read(fd,&i,sizeof(i));//prev tag size
		read(fd,&t,11);//real sizeof(Tag)
		i=BE((unsigned)t);
		(&t)[0]=(Tag)i;
//		printf("\n%2i %08X:",t.type,lseek(fd,0,SEEK_CUR));
//		if(size>0xAA0D)puts("strange");
		switch(t.type){//7=sizeof(Tag)-sizeof(Tag.length)
			case  8:parseA(t.length);break;
			case  9:parseV(t.length);break;
			case 18:parseS(t.length);break;
			default:printf("unk:%i %08X (%i)\n",t.type,lseek(fd,0,SEEK_CUR),t.length);parseR(t.length);break;
		}
	}while(lseek(fd,0,SEEK_CUR)<=len);
//	printInfo();
	return 0;
}