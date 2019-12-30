#include <pspkernel.h>
#include "core.h"
#include "bridge.h"

JS_FUN(Delay){
	if(!argc)return -1;
	$("hellooooo");
	RETURN_I(0);
}
char jsbuf[128*1024];
JS_FUN(JsOpen){
	if(!argc)return -1;
	int fd=Open(vp[0].s,0,0);
	if(fd<0)return -1;
	int len=Read(fd,jsbuf,sizeof(jsbuf)-1);
	Close(fd);
	jsbuf[len]=0;
	if(len>=0)RETURN_S(jsbuf);
	return -1;
}
JS_FUN(JsPlay){
	if(!argc)return -1;
	Alert(vp[0].s);
	Mode=0;
	char*err=Play(vp[0].s);
	Mode=1;
	if(err)RETURN_S(err);
	return 0;
}