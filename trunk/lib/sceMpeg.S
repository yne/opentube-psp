	.set noreorder

#include "pspstub.s"

	STUB_START "sceMpeg",0x40090000,0x00270005
	STUB_FUNC 0x21FF80E4,sceMpegQueryStreamOffset
	STUB_FUNC 0x611E9E11,sceMpegQueryStreamSize
	STUB_FUNC 0x682A619B,sceMpegInit
	STUB_FUNC 0x874624D6,sceMpegFinish
#	STUB_FUNC 0xC132E22F,sceMpegQueryMemSize #ori
	STUB_FUNC 0x42C679F6,sceMpegQueryMemSize #new
#	STUB_FUNC 0xD8C5F121,sceMpegCreate #ori
	STUB_FUNC 0x75E21135,sceMpegCreate #new
	STUB_FUNC 0x606A4649,sceMpegDelete
	STUB_FUNC 0x42560F23,sceMpegRegistStream
	STUB_FUNC 0x591A4AA2,sceMpegUnRegistStream
	STUB_FUNC 0xA780CF7E,sceMpegMallocAvcEsBuf
	STUB_FUNC 0xCEB870B1,sceMpegFreeAvcEsBuf
	STUB_FUNC 0xF8DCB679,sceMpegQueryAtracEsSize
	STUB_FUNC 0xC02CF6B5,sceMpegQueryPcmEsSize
	STUB_FUNC 0x167AFD9E,sceMpegInitAu
	STUB_FUNC 0x234586AE,sceMpegChangeGetAvcAuMode
	STUB_FUNC 0x9DCFB7EA,sceMpegChangeGetAuMode
	STUB_FUNC 0xFE246728,sceMpegGetAvcAu
	STUB_FUNC 0x8C1E027D,sceMpegGetPcmAu
	STUB_FUNC 0xE1CE83A7,sceMpegGetAtracAu
	STUB_FUNC 0x11F95CF1,sceMpegGetAvcNalAu
	STUB_FUNC 0x500F0429,sceMpegFlushStream
	STUB_FUNC 0x707B7629,sceMpegFlushAllStream
	STUB_FUNC 0x0E3C2E9D,sceMpegAvcDecode
	STUB_FUNC 0x0F6C18D7,sceMpegAvcDecodeDetail
	STUB_FUNC 0xCF3547A2,sceMpegAvcDecodeDetail2
	STUB_FUNC 0xA11C7026,sceMpegAvcDecodeMode
	STUB_FUNC 0x740FCCD1,sceMpegAvcDecodeStop
	STUB_FUNC 0x800C44DF,sceMpegAtracDecode
	STUB_FUNC 0xD7A29F46,sceMpegRingbufferQueryMemSize
	STUB_FUNC 0x37295ED8,sceMpegRingbufferConstruct
	STUB_FUNC 0x13407F13,sceMpegRingbufferDestruct
	STUB_FUNC 0xB240A59E,sceMpegRingbufferPut
	STUB_FUNC 0xB5F6DC87,sceMpegRingbufferAvailableSize
	STUB_FUNC 0x11CAB459,sceMpeg_11CAB459
	STUB_FUNC 0x3C37A7A6,sceMpeg_3C37A7A6
	STUB_FUNC 0xB27711A8,sceMpeg_B27711A8
	STUB_FUNC 0xD4DD6E75,sceMpeg_D4DD6E75
	STUB_FUNC 0xC345DED2,sceMpeg_C345DED2
	STUB_FUNC 0x988E9E12,sceMpeg_988E9E12
	STUB_END
