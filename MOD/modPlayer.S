	.set noreorder

#include "pspstub.s"

	STUB_START "modPlayer",0x00090000,0x00010005
	STUB_FUNC  0x8A178F0C,playerStart
	STUB_END
