	.set noreorder

#include "pspstub.s"

	STUB_START "sceLibFont_HV",0x00090000,0x001C0005
	STUB_FUNC  0x67F17ED7,sceFontNewLib
	STUB_FUNC  0x574B6FBC,sceFontDoneLib
	STUB_FUNC  0x48293280,sceFontSetResolution
	STUB_FUNC  0x27F6E642,sceFontGetNumFontList
	STUB_FUNC  0xBC75D85B,sceFontGetFontList
	STUB_FUNC  0x099EF33C,sceFontFindOptimumFont
	STUB_FUNC  0x681E61A7,sceFontFindFont
	STUB_FUNC  0x2F67356A,sceFontCalcMemorySize
	STUB_FUNC  0x5333322D,sceFontGetFontInfoByIndexNumber
	STUB_FUNC  0xA834319D,sceFontOpen
	STUB_FUNC  0x57FCB733,sceFontOpenUserFile
	STUB_FUNC  0xBB8E7FE6,sceFontOpenUserMemory
	STUB_FUNC  0x3AEA8CB6,sceFontClose
	STUB_FUNC  0x0DA7535E,sceFontGetFontInfo
	STUB_FUNC  0xDCC80C2F,sceFontGetCharInfo
	STUB_FUNC  0x5C3E4A9E,sceFontGetCharImageRect
	STUB_FUNC  0x980F4895,sceFontGetCharGlyphImage
	STUB_FUNC  0xCA1E6945,sceFontGetCharGlyphImage_Clip
	STUB_FUNC  0x74B21701,sceFontPixelToPointH
	STUB_FUNC  0xF8F0752E,sceFontPixelToPointV
	STUB_FUNC  0x472694CD,sceFontPointToPixelH
	STUB_FUNC  0x3C4B7E82,sceFontPointToPixelV
	STUB_FUNC  0xEE232411,sceFontSetAltCharacterCode
	STUB_FUNC  0xAA3DE7B5,sceFontGetShadowInfo
	STUB_FUNC  0x48B06520,sceFontGetShadowImageRect
	STUB_FUNC  0x568BE516,sceFontGetShadowGlyphImage
	STUB_FUNC  0x5DCF6858,sceFontGetShadowGlyphImage_Clip
	STUB_FUNC  0x02D7F94B,sceFontFlush
	STUB_END
