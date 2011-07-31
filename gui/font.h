#ifndef FONT_H
#define FONT_H

#include <pspsdk.h>
#include <psputility.h>
#include <malloc.h>
#include <string.h>

#define PSP_FONT_8BIT 2
#define PSP_FONT_24BIT 3
#define PSP_FONT_32BIT 4

#define FILE 0
#define MEMORY 1

#define FONT(f) (f+1)
#define REGULAR 0
#define SERIF 1//on regular only
#define ITALIC 2
#define BOLD 4

typedef struct{//Font
    float unk0;
    float unk4;
    float unk8;
    float unk12;
    float unk16;
    unsigned short unk20;
    unsigned short unk22;
    unsigned short unk24;
    unsigned short unk26;
    unsigned short unk28;
    unsigned short unk30;
    char font[64];//unk32
    char file[64];//unk96
    unsigned int unk160;
    unsigned int unk164;
}Font;
typedef struct{ 
    int maxIGlyphMetrics; /* maximum metric values (Fixed-point representation) */ 
    float maxFGlyphMetrics; /* maximum metric values (Floating point representation) */ 
    unsigned short maxGlyphBitmapWidth; /* a maximum width of the bitmap */ 
    unsigned short maxGlyphBitmapHeight; /* a maximum height of the bitmap */ 
    unsigned numChars; /* the number of character types that are included */ 
    unsigned numSubChars; /* shape of the number of substrings that are included (such as data for the shadow) */ 
    Font style;
    char pixelDepth;
    char padding[3];
}FontInfo; 
typedef struct{//GlyphInfo
    unsigned width;//unk0
    unsigned height;//unk2
    int unk4;
    int unk8;
    int unk12;
    int unk16;
    int unk20;
    int unk24;
    int unk28;
    int unk32;
}GlyphInfo;
typedef struct{//CharInfo
    unsigned width;
    unsigned height;
    unsigned left;
    unsigned top;
    GlyphInfo glyph;
    char padding[2];
    unsigned short unk14;
} CharInfo;
typedef struct{//Rect
    unsigned short width;
    unsigned short height;
}Rect;
typedef struct{//Image
    unsigned int psm;
    int x;
    int y;
    Rect rect; /* buffer size */ 
    unsigned short tbw;
    unsigned short unk;
    unsigned char * buffer;
}Image; 
typedef struct{//InitParam
    void* userData;
    unsigned maxNumFonts;
    void * cache;
    void* (* malloc)(void*,unsigned); 
    void (* free)(void*,void*); 
    void * (* open)(void*,void*,int *);
    int (* close)(void*,void*); 
    unsigned (* read)(void*,void*,void*,unsigned,unsigned,int *);
    int (* seek)(void*,void*,unsigned);
    int (* errCB)(void*, int);
    int (* finalize)(void*,int);
} InitParam; 
extern int fontInit();
extern void*fontPrint(char*str,void*p,int*w,int*h);
extern int fontStop();
extern void* sceFontNewLib(InitParam *,int * error);
extern unsigned sceFontDoneLib(void* lib);
extern void* sceFontOpen(void* lib,long,unsigned AccessMode,int * error);
extern unsigned sceFontClose(void* id );
extern unsigned sceFontGetCharInfo(void* id, short unsigned, CharInfo * );
extern unsigned sceFontGetCharGlyphImage(void* id, short unsigned, Image * );
extern unsigned sceFontGetFontInfo(void* id,FontInfo*);
#endif