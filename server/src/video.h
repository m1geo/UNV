//
//      (C) 01/May/2011 - George Smart, M1GEO <george.smart@ucl.ac.uk>
//		The UNV Project, Electronic Enginering, University College London
//

#include <libavcodec/avcodec.h>
#include <libavformat/avformat.h>
#include <libavdevice/avdevice.h> 
#include <libswscale/swscale.h>
#include "util.h"

#ifndef UNVVIDEO
#define UNVVIDEO

/* General Global Variables */
extern AVFormatContext		*pWebcamFormatContext;
extern AVCodecContext		*pWebcamCodecContext;
extern AVCodec				*pWebcamCodec;
extern AVFrame				*pFrameDec;		// YUYV422
extern AVPacket				pWebcamPacket;
extern int					iVideoStream;


void write_video_frame(AVFormatContext *, AVStream *);
AVStream *add_video_stream(AVFormatContext *, enum CodecID );
void close_video(AVFormatContext *, AVStream *);
void close_webcam();
void open_video(AVFormatContext *, AVStream *);
AVFrame * get_webcam_frame();
void open_webcam();

#endif
