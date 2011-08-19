#include <pspkernel.h>
#include <pspgu.h>
#include <pspdisplay.h>
#include <pspgum.h>

typedef struct{
	float x,y,z;
}Vertex;
static unsigned int __attribute__((aligned(16))) list[262144];
Vertex __attribute__((aligned(16))) vertices[]={
	{0.5-0.72,0.5-0.64,0},//hammer
	{0.5-0.52,0.5-0.47,0},
	{0.5-0.62,0.5-0.35,0},
	{0.5-0.60,0.5-0.33,0},
	{0.5-0.51,0.5-0.37,0},
	{0.5-0.43,0.5-0.35,0},
	{0.5-0.28,0.5-0.52,0},
	{0.5-0.38,0.5-0.62,0},
	{0.5-0.46,0.5-0.53,0},
	{0.5-0.66,0.5-0.70,0},
	{0.5-0.50,0.5-0.76,0},//inner engine
	{0.5-0.36,0.5-0.72,0},
	{0.5-0.26,0.5-0.62,0},
	{0.5-0.23,0.5-0.49,0},
	{0.5-0.26,0.5-0.37,0},
	{0.5-0.34,0.5-0.28,0},
	{0.5-0.50,0.5-0.23,0},
	{0.5-0.66,0.5-0.28,0},
	{0.5-0.74,0.5-0.37,0},
	{0.5-0.77,0.5-0.49,0},
	{0.5-0.72,0.5-0.64,0},
	{0.5-0.91,0.5-0.81,0},//outer engine
	{0.5-0.92,0.5-0.80,0},
	{0.5-0.87,0.5-0.73,0},
	{0.5-0.93,0.5-0.58,0},
	{0.5-1.02,0.5-0.56,0},
	{0.5-1.02,0.5-0.42,0},
	{0.5-0.93,0.5-0.40,0},
	{0.5-0.87,0.5-0.26,0},
	{0.5-0.91,0.5-0.19,0},
	{0.5-0.81,0.5-0.09,0},
	{0.5-0.74,0.5-0.13,0},
	{0.5-0.59,0.5-0.07,0},
	{0.5-0.57,0.5-0.00,0},
	{0.5-0.43,0.5-0.00,0},
	{0.5-0.41,0.5-0.07,0},
	{0.5-0.26,0.5-0.13,0},
	{0.5-0.19,0.5-0.09,0},
	{0.5-0.08,0.5-0.19,0},
	{0.5-0.13,0.5-0.26,0},
	{0.5-0.07,0.5-0.40,0},
	{0.5-0.00,0.5-0.42,0},
	{0.5-0.00,0.5-0.56,0},
	{0.5-0.07,0.5-0.58,0},
	{0.5-0.13,0.5-0.73,0},
	{0.5-0.08,0.5-0.80,0},
	{0.5-0.19,0.5-0.90,0},
	{0.5-0.26,0.5-0.85,0},
	{0.5-0.41,0.5-0.91,0},
	{0.5-0.43,0.5-1.00,0},
	{0.5-0.57,0.5-1.00,0},
	{0.5-0.59,0.5-0.92,0},
	{0.5-0.74,0.5-0.85,0},
	{0.5-0.81,0.5-0.90,0},
	{0.5-0.85,0.5-0.87,0},
	{0.5-0.94,0.5-0.95,0},
	{0.5-1.00,0.5-0.89,0},
	{0.5-0.72,0.5-0.64,0},
};
int guInited=0,val=0;

void sceGuStartList(int mode){
	return sceGuStart(mode,list);
}
void guInit(){
	sceGuInit();
	sceGuStart(GU_DIRECT,list);
	sceGuDrawBuffer(GU_PSM_8888,(void*)0x00000000,512);
	sceGuDispBuffer(480,272,(void*)0x00088000,512);
//	sceGuDepthBuffer((void*)0x00110000,512);
	sceGuOffset(2048 - (480/2),2048 - (272/2));
	sceGuViewport(2048,2048,480,272);
//	sceGuDepthRange(65535,0);
	sceGuScissor(0,0,480,272);
	sceGuEnable(GU_SCISSOR_TEST);
	sceGuDepthFunc(GU_GEQUAL);
	sceGuDisable(GU_DEPTH_TEST);
	sceGuFrontFace(GU_CW);
	sceGuShadeModel(GU_SMOOTH);
	sceGuEnable(GU_CULL_FACE);
	sceGuEnable(GU_CLIP_PLANES);
	sceGuClearColor(0xff0000ff);
	sceGuClearDepth(0);
	sceGuColor(0xff00ffff);
	sceGumMatrixMode(GU_PROJECTION);
	sceGumLoadIdentity();
	sceGumPerspective(75.0f,16.0f/9.0f,0.0f,10.0f);
	sceGumMatrixMode(GU_VIEW);
	sceGumLoadIdentity();
	sceGuFinish();
	sceGuSync(0,0);
	sceDisplayWaitVblankStart();
	sceGuDisplay(GU_TRUE);
	guInited=1;
}
void guDraw(){
	sceGuStartList(GU_DIRECT);
	sceGuClear(GU_COLOR_BUFFER_BIT);
	sceGumMatrixMode(GU_MODEL);
	sceGumLoadIdentity();
	ScePspFVector3 pos = {0,0,-0.75f};
	sceGumTranslate(&pos);
	sceGumRotateY(val*0.01);
	sceGumDrawArray(GU_LINE_STRIP,GU_VERTEX_32BITF|GU_TRANSFORM_3D,sizeof(vertices)/sizeof(Vertex),0,vertices);
//	ScePspFVector3 pos2 = {0,0,-0.1f};
//	sceGumTranslate(&pos2);
//	sceGumDrawArray(GU_LINE_STRIP,GU_VERTEX_32BITF|GU_TRANSFORM_3D,sizeof(vertices)/sizeof(Vertex),0,vertices);
	sceGuFinish();
	sceGuSync(0,0);
	val++;
}
void guTerm(){
	sceGuTerm();
}
