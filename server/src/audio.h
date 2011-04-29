#ifndef UNV_AUDIO

#include <libavcodec/avcodec.h>
#include <libavformat/avformat.h>
#include <libavdevice/avdevice.h> 
#include <libswscale/swscale.h>
#include "util.h"


#define UNV_AUDIO 1

static AVStream *add_audio_stream(AVFormatContext *, enum CodecID );
static void close_audio(AVFormatContext *, AVStream *);
static void get_audio_frame(int16_t *, int , int);
static void open_audio(AVFormatContext *, AVStream *);
static void write_audio_frame(AVFormatContext *, AVStream *);

#endif
