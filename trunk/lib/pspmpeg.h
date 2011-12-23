/*
 * pspmpeg.h - Prototypes for the sceMpeg library.
 *
 * Copyright (c) 2006 Sorin P. C. <magik@hypermagik.com>
 */

#ifndef __SCELIBMPEG_H__
#define __SCELIBMPEG_H__

#include <psptypes.h>

#ifdef __cplusplus
extern "C" {
#endif

/** points to "LIBMPEG" */
typedef void* SceMpeg;

/** some structure */
typedef SceVoid  SceMpegStream;

/** Ringbuffer callback */
typedef int (*sceMpegRingbufferCB)(void* pData, int iNumPackets, void* pParam);

typedef struct SceMpegRingbuffer
{
    /** packets */
	int iPackets;

    /** unknown */
	int iUnk0;
    /** unknown */
	int iUnk1;
    /** unknown */
	int iUnk2;
    /** unknown */
	int iUnk3;

    /** pointer to data */
	void* pData;

    /** ringbuffer callback */
	sceMpegRingbufferCB	Callback;
    /** callback param */
	void* pCBparam;

    /** unknown */
	int iUnk4;
    /** unknown */
	int iUnk5;
    /** mpeg id */
	SceMpeg 	pSceMpeg;

} SceMpegRingbuffer;

typedef struct SceMpegAu
{
    /** unknown */
	int iUnk0;
    /** presentation timestamp? */
	int  iTimestamp;
    /** unknown */
	int iUnk1;
    /** unknown */
	int iUnk2;
    /** Es buffer handle */
	void* iEsBuffer;
    /** Au size */
	int iAuSize;

} SceMpegAu;

typedef struct SceMpegAvcMode
{
	/** unknown, set to -1 */
	int iUnk0;
	/** unknonw, set to 3 */
	int iUnk1;

} SceMpegAvcMode;

/**
 * sceMpegInit
 *
 * @returns 0 if success.
 */
int sceMpegInit();

/**
 * sceMpegFinish
 */
SceVoid sceMpegFinish();

/**
 * sceMpegRingbufferQueryMemSize
 *
 * @param iPackets - number of packets in the ringbuffer
 *
 * @returns < 0 if error else ringbuffer data size.
 */
int sceMpegRingbufferQueryMemSize(int iPackets);

/**
 * sceMpegRingbufferConstruct
 *
 * @param Ringbuffer - pointer to a sceMpegRingbuffer struct
 * @param iPackets - number of packets in the ringbuffer
 * @param pData - pointer to allocated memory
 * @param iSize - size of allocated memory, shoud be sceMpegRingbufferQueryMemSize(iPackets)
 * @param Callback - ringbuffer callback
 * @param pCBparam - param passed to callback
 *
 * @returns 0 if success.
 */
int sceMpegRingbufferConstruct(SceMpegRingbuffer* Ringbuffer, int iPackets, void* pData, int iSize, sceMpegRingbufferCB Callback, void* pCBparam);

/**
 * sceMpegRingbufferDestruct
 *
 * @param Ringbuffer - pointer to a sceMpegRingbuffer struct
 */
SceVoid sceMpegRingbufferDestruct(SceMpegRingbuffer* Ringbuffer);

/**
 * sceMpegQueryMemSize 
 *
 * @param Ringbuffer - pointer to a sceMpegRingbuffer struct
 *
 * @returns < 0 if error else number of free packets in the ringbuffer.
 */
int sceMpegRingbufferAvailableSize(SceMpegRingbuffer* Ringbuffer);

/**
 * sceMpegRingbufferPut
 *
 * @param Ringbuffer - pointer to a sceMpegRingbuffer struct
 * @param iNumPackets - num packets to put into the ringbuffer
 * @param iAvailable - free packets in the ringbuffer, should be sceMpegRingbufferAvailableSize()
 *
 * @returns < 0 if error else number of packets.
 */
int sceMpegRingbufferPut(SceMpegRingbuffer* Ringbuffer, int iNumPackets, int iAvailable);

/**
 * sceMpegQueryMemSize
 *
 * @param iUnk - Unknown, set to 0
 *
 * @returns < 0 if error else decoder data size.
 */
int sceMpegQueryMemSize(int iUnk);

/**
 * sceMpegCreate
 *
 * @param Mpeg - will be filled
 * @param pData - pointer to allocated memory of size = sceMpegQueryMemSize()
 * @param iSize - size of data, should be = sceMpegQueryMemSize()
 * @param Ringbuffer - a ringbuffer
 * @param iFrameWidth - display buffer width, set to 512 if writing to framebuffer
 * @param iUnk1 - unknown, set to 0
 * @param iUnk2 - unknown, set to 0
 *
 * @returns 0 if success.
 */
int sceMpegCreate(SceMpeg* Mpeg, void* pData, int iSize, SceMpegRingbuffer* Ringbuffer, int iFrameWidth, int iUnk1, int iUnk2);
int sceMpeg_75E21135(SceMpeg* Mpeg, void* pData, int iSize, SceMpegRingbuffer* Ringbuffer, int iFrameWidth, int iUnk1, int iUnk2);

/**
 * sceMpegDelete
 *
 * @param Mpeg - SceMpeg handle
 */
SceVoid sceMpegDelete(SceMpeg* Mpeg);

/**
 * sceMpegQueryStreamOffset
 *
 * @param Mpeg - SceMpeg handle
 * @param pBuffer - pointer to file header
 * @param iOffset - will contain stream offset in bytes, usually 2048
 *
 * @returns 0 if success.
 */
int sceMpegQueryStreamOffset(SceMpeg* Mpeg, void* pBuffer, int* iOffset);

/**
 * sceMpegQueryStreamSize
 *
 * @param pBuffer - pointer to file header
 * @param iSize - will contain stream size in bytes
 *
 * @returns 0 if success.
 */
int sceMpegQueryStreamSize(void* pBuffer, int* iSize);

/**
 * sceMpegRegistStream
 *
 * @param Mpeg - SceMpeg handle
 * @param iStreamID - stream id, 0 for video, 1 for audio
 * @param iUnk - unknown, set to 0
 *
 * @returns 0 if error.
 */
SceMpegStream* sceMpegRegistStream(SceMpeg* Mpeg, int iStreamID, int iUnk);

/**
 * sceMpegUnRegistStream
 *
 * @param Mpeg - SceMpeg handle
 * @param pStream - pointer to stream
 */
SceVoid sceMpegUnRegistStream(SceMpeg Mpeg, SceMpegStream* pStream);

/**
 * sceMpegFlushAllStreams
 *
 * @returns 0 if success.
 */
int sceMpegFlushAllStream(SceMpeg* Mpeg);

/**
 * sceMpegMallocAvcEsBuf
 *
 * @returns 0 if error else pointer to buffer.
 */
void* sceMpegMallocAvcEsBuf(SceMpeg* Mpeg);

/**
 * sceMpegFreeAvcEsBuf
 *
 */
SceVoid sceMpegFreeAvcEsBuf(SceMpeg* Mpeg, void* pBuf);

/**
 * sceMpegQueryAtracEsSize
 *
 * @param Mpeg - SceMpeg handle
 * @param iEsSize - will contain size of Es
 * @param iOutSize - will contain size of decoded data
 *
 * @returns 0 if success.
 */
int sceMpegQueryAtracEsSize(SceMpeg* Mpeg, int* iEsSize, int* iOutSize);

/**
 * sceMpegInitAu
 *
 * @param Mpeg - SceMpeg handle
 * @param pEsBuffer - prevously allocated Es buffer
 * @param pAu - will contain pointer to Au
 *
 * @returns 0 if success.
 */
int sceMpegInitAu(SceMpeg* Mpeg, void* pEsBuffer, SceMpegAu* pAu);

/**
 * sceMpegGetAvcAu
 *
 * @param Mpeg - SceMpeg handle
 * @param pStream - associated stream
 * @param pAu - will contain pointer to Au
 * @param iUnk - unknown
 *
 * @returns 0 if success.
 */
int sceMpegGetAvcAu(SceMpeg* Mpeg, SceMpegStream* pStream, SceMpegAu* pAu, int* iUnk);

/**
 * sceMpegAvcDecodeMode
 *
 * @returns 0 if success.
 */
int sceMpegAvcDecodeMode(SceMpeg* Mpeg, SceMpegAvcMode* pMode);

/**
 * sceMpegAvcDecode
 *
 * @param Mpeg - SceMpeg handle
 * @param pAu - video Au
 * @param iFrameWidth - output buffer width, set to 512 if writing to framebuffer
 * @param pBuffer - buffer that will contain the decoded frame
 * @param iInit - will be set to 0 on first call, then 1
 *
 * @returns 0 if success.
 */
int sceMpegAvcDecode(SceMpeg* Mpeg, SceMpegAu* pAu, int iFrameWidth, void* pBuffer, int* iInit);

/**
 * sceMpegAvcDecodeStop
 *
 * @param Mpeg - SceMpeg handle
 * @param iFrameWidth - output buffer width, set to 512 if writing to framebuffer
 * @param pBuffer - buffer that will contain the decoded frame
 * @param iFrameNum - frame number
 *
 * @returns 0 if success.
 */
int sceMpegAvcDecodeStop(SceMpeg* Mpeg, int iFrameWidth, void* pBuffer, int* iStatus);

/**
 * sceMpegGetAtracAu
 *
 * @param Mpeg - SceMpeg handle
 * @param pStream - associated stream
 * @param pAu - will contain pointer to Au
 * @param pUnk - unknown
 *
 * @returns 0 if success.
 */
int sceMpegGetAtracAu(SceMpeg* Mpeg, SceMpegStream* pStream, SceMpegAu* pAu, void* pUnk);

/**
 * sceMpegAtracDecode
 *
 * @param Mpeg - SceMpeg handle
 * @param pAu - video Au
 * @param pBuffer - buffer that will contain the decoded frame
 * @param iInit - set this to 1 on first call
 *
 * @returns 0 if success.
 */
int sceMpegAtracDecode(SceMpeg* Mpeg, SceMpegAu* pAu, void* pBuffer, int iInit);

#ifdef __cplusplus
}
#endif

#endif
