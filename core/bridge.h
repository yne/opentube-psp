#define RETURN_S(s) {*rtype=1;*rval=(int)(s);return 0;}
#define RETURN_I(i) {*rtype=0;*rval=(int)(i);return 0;}
#define JS_FUN(fun) int fun(int arg1,int argc,jsval vp[],int parent,int* rtype,int* rval)

typedef struct{
	char* mime;
	char* name;
	void* function;
	void* param;
	int unk;
}jsMeth;

typedef union{
	int i;
	char*s;
}jsval;

//extern jsMeth funList[];
JS_FUN(Delay);
JS_FUN(JsOpen);
JS_FUN(JsPlay);
JS_FUN(Test);

