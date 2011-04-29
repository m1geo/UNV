#ifndef UNV_VIDEO
#include <libavcodec/avcodec.h>
#include <libavformat/avformat.h>
#include <libavdevice/avdevice.h> 
#include <libswscale/swscale.h>
#include "gcs.h"

#define UNV_VIDEO 1

static void write_video_frame(AVFormatContext *, AVStream *);
static AVStream *add_video_stream(AVFormatContext *, enum CodecID );
static void close_video(AVFormatContext *, AVStream *);
static void close_webcam();
static void open_video(AVFormatContext *, AVStream *);
AVFrame * get_webcam_frame();

#endif
