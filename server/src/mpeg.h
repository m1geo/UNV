/* 
 * April 29th 2011 - George Smart, M1GEO 
 * The UNV Project, University College London
 * 
 * mpeg.h	mpeg.c header file
 */
#ifndef UNV_MPEG_H

	extern AVCodecContext  				*pCodecCtxVidEncMPEG;
	extern AVCodec						*pCodecVidEncMPEG;
	extern AVFrame						*pFrameEncMPEG;
	extern int							iYUVFrameSize;
	extern int							iOutBufferSize;
	extern uint8_t 						*pYUVBufferVid;
	extern static struct SwsContext		*swsC_YUV;
	extern int 							swsR_YUV;




void allocate_variables()
void init_mpeg_codec (int width, int height, int pixel_format)
void allocate_memory ()
void mpeg_init_all (int w, int h, int pix)
uint8_t * encode_frame_to_mpeg(AVFrame * pRawFrame)

#define UNV_MPEG_H 1
#endif
