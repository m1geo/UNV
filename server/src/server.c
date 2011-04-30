#include "video.h"
#include "audio.h"
#include "util.h"
#include <libavcodec/avcodec.h>
#include <libavformat/avformat.h>
#include <libavdevice/avdevice.h> 
#include <libswscale/swscale.h>

#include "rtspServer.h"

#include <stdlib.h>
#include <stdio.h>


#define STREAM_DURATION 100.0
#define STREAM_FRAME_RATE 25
#define STREAM_NB_FRAMES ((int) (STREAM_DURATION*STREAM_FRAME_RATE))
#define STREAM_PIX_FMT PIX_FMT_YUV420P

#define min(a,b) ((a<=b)? a:b)


int main(int argc, char *argv[]) {

	AVOutputFormat *fmt;
    AVFormatContext *oc;
    AVStream *audio_st, *video_st;
    double audio_pts, video_pts;
    int i;
    
    long int iFrame = 0;
	setup_ffmpeg();
	printf("This program attempts to make %f seconds of video/audio from webcam\n", STREAM_DURATION);

    fmt = av_guess_format(NULL, filename, NULL);
    
    if (!fmt) {
        fprintf(stderr, "Could not find suitable output format\n");
        exit(1);
    }

    /* allocate the output media context */
    oc = avformat_alloc_context();
    if (!oc) {
        fprintf(stderr, "Memory error\n");
        exit(1);
    }
    oc->oformat = fmt;
    snprintf(oc->filename, sizeof(oc->filename), "%s", filename);

    /* add the audio and video streams using the default format codecs
and initialize the codecs */
    video_st = NULL;
    audio_st = NULL;
    if (fmt->video_codec != CODEC_ID_NONE) {
        video_st = add_video_stream(oc, fmt->video_codec);
    }
    if (fmt->audio_codec != CODEC_ID_NONE) {
        audio_st = add_audio_stream(oc, fmt->audio_codec);
    }

    /* set the output parameters (must be done even if no
parameters). */
    if (av_set_parameters(oc, NULL) < 0) {
        fprintf(stderr, "Invalid output format parameters\n");
        exit(1);
    }

    dump_format(oc, 0, filename, 1);

    /* now that all the parameters are set, we can open the audio and
video codecs and allocate the necessary encode buffers */
       
    open_webcam();
    
    if (video_st)
        open_video(oc, video_st);
    if (audio_st)
        open_audio(oc, audio_st);


    /* open the output file, if needed */
    if (!(fmt->flags & AVFMT_NOFILE)) {
        if (url_fopen(&oc->pb, filename, URL_WRONLY) < 0) {
            fprintf(stderr, "Could not open '%s'\n", filename);
            exit(1);
        }
    }
    startServerRTSP(20,30015,3015);
    printf("WAITING\n");
	sleep(10);
    /* write the stream header, if any */
    av_write_header(oc);
        //addFrameByFile(filename,"header");
    //addHeader(pHead) //OABDA - How? what is oc?
    
	addFrame((char *) oc->pb->buf_ptr, 552);
	iPktSize = 552;
	iFrame =0;
	//exit(1);
    for(;;) {
		if(iPktSize !=0 && iFrame > 0) addFrame((char *) oc->pb->buffer, min(iPktSize, oc->pb->buf_ptr - oc->pb->buffer));
		printf("1\rFrame %5ld: iPktsize: %d , Diff: %d\n", iFrame, iPktSize, oc->pb->buf_end - oc->pb->buf_ptr);
        /* compute current audio and video time */
        if (audio_st)
            audio_pts = (double)audio_st->pts.val * audio_st->time_base.num / audio_st->time_base.den;
        else
            audio_pts = 0.0;

        if (video_st)
            video_pts = (double)video_st->pts.val * video_st->time_base.num / video_st->time_base.den;
        else
            video_pts = 0.0;

        if ((!audio_st || audio_pts >= STREAM_DURATION) &&
            (!video_st || video_pts >= STREAM_DURATION))
            break;

        /* write interleaved audio and video frames */
        if (!video_st || (video_st && audio_st && audio_pts < video_pts)) {
            write_audio_frame(oc, audio_st);
        } else {
            write_video_frame(oc, video_st);
        }
        
       // addFrame((char*) oc->pb->buf_ptr, oc->pb->buffer_size );
        
        iFrame++;
        if (iFrame>10) iFrame=10;
    }
printf("\n");

    /* write the trailer, if any. the trailer must be written
* before you close the CodecContexts open when you wrote the
* header; otherwise write_trailer may try to use memory that
* was freed on av_codec_close() */
    //av_write_trailer(oc);

    /* close each codec */
    if (video_st)
        close_video(oc, video_st);
    if (audio_st)
        close_audio(oc, audio_st);
        
    close_webcam();

    /* free the streams */
   
for(i = 0; i < (int)oc->nb_streams; i++) {
        av_freep(&oc->streams[i]->codec);
        av_freep(&oc->streams[i]);
    }
   if (!(fmt->flags & AVFMT_NOFILE)) {
        /* close the output file */
        url_fclose(oc->pb);
    }

    /* free the stream */
    av_free(oc);

    //return 0;
   // dontDie();
    

}
