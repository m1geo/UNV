//
//      (C) 01/May/2011 - George Smart, M1GEO <george.smart@ucl.ac.uk>
//		The UNV Project, Electronic Enginering, University College London
//

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

#include <libavcodec/avcodec.h>
#include <libavformat/avformat.h>
#include <libavdevice/avdevice.h> 
#include <libswscale/swscale.h>

// define to return the minimum of two parameters; a,b
#define min(a,b) ((a<=b)? a:b)

#define STREAM_DURATION 	200.0 //
#define STREAM_FRAME_RATE 	25
#define STREAM_NB_FRAMES	((int) (STREAM_DURATION*STREAM_FRAME_RATE))
#define STREAM_PIX_FMT		PIX_FMT_YUV420P

#undef exit

#define FALSE	0
#define TRUE	!FALSE

// software scaler parameters.
static int 					sws_flags = SWS_BICUBIC;

/* audio output */
extern float 				t, tincr, tincr2;
extern int16_t 				*samples;
extern uint8_t 				*audio_outbuf;
extern int					audio_outbuf_size;
extern int					audio_input_frame_size;

/* video output */
extern AVFrame 				*picture, *tmp_picture;
extern uint8_t 				*video_outbuf;
extern int 					frame_count, video_outbuf_size;
extern int					iYUVFrameSize;
extern int					iOutBufferSize;
extern uint8_t 				*pYUVBufferVid;
extern struct SwsContext	*swsC_YUV;
extern int 					swsR_YUV;
extern int					iPktSize;
extern char					*filename;




struct cliOpts_t 
{	
	int     verbose;    // if ! false, show verbose messages
	int     version;    // if ! false, show version message
	int     formats;    // if ! false, show supported formats
	int      saveframes;    // if ! false, save every <val>th frame to pmm
	int      width;      // width of image, if non 0
	int     height;      // height of image, if non 0
	int     maxframes;    // only process <val> frames
	int      CRASHOUT;    // DO NOT USE! DEVELOPMENT ONLY
	int         saveaudio;      // TRUE if audiopath is defined (used to decide if to open file handles, etc)
	int      savevideo;    // TRUE if videopath is defined (used to decide if to open file handles, etc)
	int      networkport;  // Port for Network
	char*    devicepath;      // path to file/device
	char*    mode;      // mode for program
	char*    videopath;    // Path for output video	
	char*       audiopath;      // Path for output audio
};

AVFrame *alloc_picture(enum PixelFormat , int, int);
char * AVERROR_LOOKUP(int );
char * PIX_FMT_LOOKUP(int );
void show_format (AVInputFormat * InFmt);
void usage (char prog[]);
int get_options (int, char **);



extern struct cliOpts_t 	cliOpts;

#endif
