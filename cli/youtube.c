#include <pspkernel.h>
#include <string.h>
#include "graphic.h"
#include "core.h"

int h2i(char c){//hex to int
	if((c>='0')&&(c<='9'))return c-'0';
	if((c>='a')&&(c<='f'))return c-'a'+10;
	if((c>='A')&&(c<='F'))return c-'A'+10;
	return 0;
}
char*p=0;//current ext() position
char* ext(char*from,char*to){
	char*a=strstr(p,from);if(!a)return NULL;
	a+=strlen(from);
	char*b=strstr(a,to);if(!b)return NULL;
	b[0]=0;
	p=b+1;
	return a;
}
char* unescape(char*in){
	if(!in)return 0;
	int i=0,j=0,len=strlen(in);
	for(;i<len;j++,i++){//i <     j >
		if(in[j]=='%'){
			in[i]=(h2i(in[j+1])<<4)|h2i(in[j+2]);//h2i(in[++j]<<8)&h2i(in[++j]);
			j+=2;
		}else
			in[i]=in[j];
	}
	return in;
}
char*Strncpy(char*dst,char*src,int max){
	if(!dst)return dst;
	if(!src){dst[0]=0;return dst;}
	int i=0;
	while((i<max)&&src[i])
		dst[i]=src[i],i++;
	dst[i]=0;
	return dst;
}
Entry*youtube_search(char*kw){
	if(!kw)return NULL;
	Entry*r=NULL;
	char url[128]="http://gdata.youtube.com/feeds/api/videos?start-index=1&max-results=20&v=1&q=";
	strcpy(url+77,kw);
	char*c=(char*)Open(url,HTTP_SAVE_RAM,0777);p=c;
	char*totalResults=ext("totalResults>","<");
	totalResults=0;
	for(int i=0;;i++){
		r=Realloc(r,(i+1)*sizeof(Entry));
		memset(&r[i],0,sizeof(Entry));
		Strncpy(r[i].id,ext("api/videos/","<"),63);
		if(r[i].id[0]==0)break;
		Strncpy(r[i].date,ext("<published>","<"),10);
		Strncpy(r[i].title,ext("<title type='text'>","<"),127);
		Strncpy(r[i].desc,ext("<content type='text'>","<"),255);
		Strncpy(r[i].uploader,ext("<name>","<"),32);
		Strncpy(r[i].size,ext("seconds='","'"),3);
		Strncpy(r[i].rate,ext("average='","'"),3);
		Strncpy(r[i].favorite,ext("favoriteCount='","'"),15);
		Strncpy(r[i].view,ext("viewCount='","'"),15);
	}
	Free(c);
	return r;
}
void youtube_play(char*id){
	char url[128]="http://www.youtube.com/get_video_info?asv=3&el=embedded&hl=en_US&video_id=";
	strcpy(url+74,id);
	char*c=(char*)Open(url,HTTP_SAVE_RAM,0777);p=c;
	$("ok je vien de dl la page");
//	char* fmtmap=unescape(ext("&fmt_list=","&"));p=c;
//	char* urlmap=unescape(ext("&url_encoded_fmt_stream_map=","&"));p=urlmap;
	printf(c);
//	while(1){
//		char*url=/*unescape*/(ext("url=","&"));
//		if(!url)break;
//		printf("\n ");
//		printf(url);
//	}
	Free(c);
}