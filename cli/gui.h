#ifndef __GUI_H__
#define __GUI_H__
void guiInitEx(void *vram_base, int mode, int setup);
void guiEnableBackColor(int enable);
void guiPutc(int x, int y, u32 color, u8 ch);
int  guiPuts(const char *str);
void guiClear(void);
void guiClearLine(int);
void*guiGetFont(void);
int  guiGetX(void);
int  guiGetY(void);
void guiSetXY(int x, int y);
void guiSetMaxX(unsigned int maxx);
void guiSetMaxY(unsigned int maxy);
void guiSetBase(u32* base);
void guiSetOffset(int offset);
void guiSetClearLine(int);
void guiSetBackColor(u32 color);
void guiSetTextColor(u32 color);
#endif
