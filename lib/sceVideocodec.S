	.set noreorder

#include "pspstub.s"

	STUB_START "sceVideocodec",0x40090000,0x000A0005
	STUB_FUNC 0x2D31F5B1,sceVideocodecGetEDRAM
	STUB_FUNC 0x4F160BF4,sceVideocodecReleaseEDRAM
	STUB_FUNC 0xC01EC829,sceVideocodecOpen
	STUB_FUNC 0x17099F0A,sceVideocodecInit
	STUB_FUNC 0xDBA273FA,sceVideocodecDecode
	STUB_FUNC 0xA2F0564E,sceVideocodecStop
	STUB_FUNC 0x307E6E1C,sceVideocodecDelete
	STUB_FUNC 0x745A7B7A,sceVideocodecSetMemory
	STUB_FUNC 0x2F385E7F,sceVideocodecScanHeader
	STUB_FUNC 0x26927D19,sceVideocodecGetVersion
	STUB_END