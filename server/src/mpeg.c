/* 
 * April 29th 2011 - George Smart, M1GEO 
 * The UNV Project, University College London
 * 
 * mpeg.c	setup stuff to write mpeg data
 */

#include "util.h"

AVCodecContext  			*pCodecCtxVidEncMPEG;
AVCodec						*pCodecVidEncMPEG;
AVFrame						*pFrameEncMPEG;






void allocate_variables() {
	// Allocate contexts
	pCodecCtxVidEnc = avcodec_alloc_context();
	if ( (pCodecCtxVidEnc == NULL) ) {
		fprintf(stderr, "cannot allocate context in mpeg\n");
		exit(EXIT_FAILURE);
	}
	// Allocate frames
	pRawFrame = avcodec_alloc_frame();
	pMPEGFrame = avcodec_alloc_frame();
	if ( (pRawFrame == NULL) || (pMPEGFrame == NULL) ) {
		fprintf(stderr, "cannot allocate frame for mpeg\n");
		exit(EXIT_FAILURE);
	}
}

void init_mpeg_codec (int width, int height, int pixel_format) {

    pCodecVidEncMPEG = avcodec_find_encoder(CODEC_ID_MPEG2VIDEO);
    if (pCodecVidEncMPEG == NULL) {
        fprintf(stderr, "cannot find mpeg encoder\n");
        exit(EXIT_FAILURE);
	}
	avcodec_get_context_defaults(pCodecCtxVidEncMPEG);
	pCodecCtxVidEncMPEG->codec_type = CODEC_TYPE_VIDEO;
	pCodecCtxVidEncMPEG->codec_id = CODEC_ID_MPEG2VIDEO;
	
    pCodecCtxVidEncMPEG->bit_rate = 400000;
    pCodecCtxVidEncMPEG->width = width;
    pCodecCtxVidEncMPEG->height = height;
    pCodecCtxVidEncMPEG->time_base= (AVRational){1,25};
    pCodecCtxVidEncMPEG->gop_size = pCodecCtxVidEncMPEG->time_base.den * 1; // every one second
    pCodecCtxVidEncMPEG->max_b_frames=0;
    pCodecCtxVidEncMPEG->pix_fmt = PIX_FMT_YUV420P;
    
    // all we want to do is convert from pixel_format supplied as an argument to PIX_FMT_YUV420P for the encoder.
    // DEBUG //
    fprintf(stderr, "Just so you know, we're setting the converter to go between %s -> %s\n", PIX_FMT_LOOKUP(pixel_format), PIX_FMT_LOOKUP(pCodecCtxVidEncMPEG->pix_fmt));
	swsC_YUV = sws_getContext(	width,							//	Source width
								height,							//	Source height
								pixel_format,					//	Source format
								width,							//	Dest width
								height,							//	Dest height
								pCodecCtxVidEncMPEG->pix_fmt,	//	Dest format
								SWS_FAST_BILINEAR,				//	Transform Type (choose from SWS FLAGS)
								NULL, NULL, NULL);				//	Filters & Pointer Setup (NULL chooses defaults for last 3 options)
    
    if (swsC_YUV == NULL) {
		fprintf(stderr, "failed to setup software scaler\n");
		exit(EXIT_FAILURE);
	}
	
    // Open the mpeg encoder
    temp = avcodec_open(pCodecCtxVidEncMPEG, pCodecVidEncMPEG);
	if(temp < 0) {
		fprintf(stderr, "cannot open mpeg encoder : %s\n", AVERROR_LOOKUP(temp));
		exit(EXIT_FAILURE);
	}
}

void allocate_memory () {
    iYUVFrameSizeMPEG = avpicture_get_size(PIX_FMT_YUV420P, pCodecCtxVidEncMPEG->width, pCodecCtxVidEncMPEG->height);
    iOutBufferSizeMPEG = 100000;
	pYUVBufferVidMPEG = (uint8_t *) malloc((iYUVFrameSize));
    if ( (pYUVBufferVidMPEG == NULL) ) {
		fprintf(stderr, "cannot allocate mpeg encoder frame buffer memory");
		exit(EXIT_FAILURE);
	}
	
	pOutBufferVidMPEG = (uint8_t *) av_malloc(iOutBufferSize * sizeof(uint8_t));  // alloc output buffer for encoded datastream
    if ( (pOutBufferVidMPEG == NULL) ) {
		fprintf(stderr, "cannot allocate mpeg encoder output buffer memory");
		exit(EXIT_FAILURE);
	}
}

void mpeg_init_all (int w, int h, int pix) {

	allocate_variables();
	init_mpeg_codec(w, h, pix);
	allocate_memory();
	
}

uint8_t * encode_frame_to_mpeg(AVFrame * pRawFrame) {
	
	swsR_YUV = sws_scale( swsC_YUV,									//	SwsContext - Setup for scaling
						(const uint8_t* const*)	pRawFrame->data,	//	Source frame data (cast to required type)
						pRawFrame->linesize,						//	Source frame stride
						0,											//	Source frame slice width (raster, so no width slicing)
						pCodecCtxVidEncMPEG->height,				//	Source frame slice height
						pMPEGFrame->data,							//	Dest frame data
						pMPEGFrame->linesize);						//	Dest frame stride

	if (swsR_YUV <= 0) {
		fprintf(stderr, "something went wrong with the software scaler\n");
		exit(EXIT_FAILURE);
	}
	
	iEncodedBytes = avcodec_encode_video(pCodecCtxVidEncMPEG, pOutBufferVidMPEG, iOutBufferSize, pFrameEncMPEG);
	if (iEncodedBytes < 0) {
		fprintf(stderr, "something went wrong with the mpeg encoder : %s\n", AVERROR_LOOKUP(iEncodedBytes));
		exit(EXIT_FAILURE);
	}
	
	return (pOutBufferVidMPEG);
}

