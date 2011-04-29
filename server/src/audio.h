#ifndef UNV_AUDIO

#include <libavcodec/avcodec.h>
#include <libavformat/avformat.h>
#include <libavdevice/avdevice.h> 
#include <libswscale/swscale.h>
#include "util.h"


#define UNV_AUDIO 

 AVStream *add_audio_stream(AVFormatContext *, enum CodecID );
void close_audio(AVFormatContext *, AVStream *);
void get_audio_frame(int16_t *, int , int);
void open_audio(AVFormatContext *, AVStream *);
void write_audio_frame(AVFormatContext *, AVStream *);

#endif
