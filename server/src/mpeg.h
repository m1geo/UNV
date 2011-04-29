/* 
 * April 29th 2011 - George Smart, M1GEO 
 * The UNV Project, University College London
 * 
 * mpeg.h	mpeg.c header file
 */
#ifndef UNV_MPEG_H

	AVCodecContext  			*pCodecCtxVidEncMPEG;
	AVCodec						*pCodecVidEncMPEG;
	AVFrame						*pFrameEncMPEG;
	int							iYUVFrameSize;
	int							iOutBufferSize;
	uint8_t 					*pYUVBufferVid;
	static struct SwsContext	*swsC_YUV;
	int 						swsR_YUV;

#define UNV_MPEG_H 1
#endif
