/*
 * WebCam Video to MKV File
 *
 * Copyright (c) 2003 Fabrice Bellard, and
 * Copyright (c) 2011 George Smart for the UNV Project
 *
 * Permission is hereby granted, free of charge, to any person obtaining a copy
 * of this software and associated documentation files (the "Software"), to deal
 * in the Software without restriction, including without limitation the rights
 * to use, copy, modify, merge, publish, distribute, sublicense, and/or sell
 * copies of the Software, and to permit persons to whom the Software is
 * furnished to do so, subject to the following conditions:
 *
 * The above copyright notice and this permission notice shall be included in
 * all copies or substantial portions of the Software.
 *
 * THE SOFTWARE IS PROVIDED "AS IS", WITHOUT WARRANTY OF ANY KIND, EXPRESS OR
 * IMPLIED, INCLUDING BUT NOT LIMITED TO THE WARRANTIES OF MERCHANTABILITY,
 * FITNESS FOR A PARTICULAR PURPOSE AND NONINFRINGEMENT. IN NO EVENT SHALL
 * THE AUTHORS OR COPYRIGHT HOLDERS BE LIABLE FOR ANY CLAIM, DAMAGES OR OTHER
 * LIABILITY, WHETHER IN AN ACTION OF CONTRACT, TORT OR OTHERWISE, ARISING FROM,
 * OUT OF OR IN CONNECTION WITH THE SOFTWARE OR THE USE OR OTHER DEALINGS IN
 * THE SOFTWARE.
 */
#ifndef UNV_UTIL
#define UNV_UTIL 
#include <stdlib.h>
#include <stdio.h>
#include <string.h>
#include <math.h>

#ifndef INT64_C
#define INT64_C(c) (c ## LL)
#define UINT64_C(c) (c ## ULL)
#endif

//extern "C" {	// required for compiling in C++ - Thanks Obada! :D
#include <libavcodec/avcodec.h>
#include <libavformat/avformat.h>
#include <libavdevice/avdevice.h> 
#include <libswscale/swscale.h>
//#include "video.h"
//#include "audio.h"
//}
//#include "pix_fmt_lookup.h"

#undef exit

/* 5 seconds stream duration */
#define STREAM_DURATION   100.0
#define STREAM_FRAME_RATE 25 /* 25 images/s */
#define STREAM_NB_FRAMES  ((int)(STREAM_DURATION * STREAM_FRAME_RATE))
#define STREAM_PIX_FMT PIX_FMT_YUV420P /* default pix_fmt */

#define FALSE	0
#define TRUE	!FALSE

static int sws_flags = SWS_BICUBIC;



/**************************************************************/
/* audio output */

extern float t, tincr, tincr2;
extern int16_t *samples;
extern uint8_t *audio_outbuf;
extern int audio_outbuf_size;
extern int audio_input_frame_size;

/* video output */

extern AVFrame *picture, *tmp_picture;
extern uint8_t *video_outbuf;
extern int frame_count, video_outbuf_size;


extern int							iYUVFrameSize;
extern int							iOutBufferSize;
extern uint8_t 					*pYUVBufferVid;
extern struct SwsContext	*swsC_YUV;
extern int 						swsR_YUV;
extern int iPktSize;

extern char *filename;




/**************************************************************/

AVFrame *alloc_picture(enum PixelFormat , int, int);

char * AVERROR_LOOKUP(int );
char * PIX_FMT_LOOKUP(int );

#endif
