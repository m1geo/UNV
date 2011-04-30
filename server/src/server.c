//
//      (C) 30/Apr/2011 - George Smart, M1GEO <george.smart@ucl.ac.uk>
//		The UNV Project, Electronic Enginering, University College London
//
//		Based on
//			http://www.inb.uni-luebeck.de/~boehme/using_libavcodec.html
//			http://www.inb.uni-luebeck.de/~boehme/libavcodec_update.html
//			http://www.ibm.com/developerworks/aix/library/au-unix-getopt.html
//
//		Libraries
//			avformat avcodec avutil avdevice swscale
//

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
    	
	FILE * db = fopen("debug.mkv","wb");
    startServerRTSP(20,30015,3015);
    printf("WAITING\n");
	sleep(10);
    /* write the stream header, if any */
    av_write_header(oc);
	addFrame((char *) oc->pb->buffer, 552);	// send the header...
	fwrite (oc->pb->buffer, 1 , 552 , db );
	iFrame =0;

	
    for(;;) {
		printf("\rFrame %5ld", iFrame);
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
        
        // Write it to the network.
        long unsigned int diff = (oc->pb->buf_ptr - oc->pb->buffer);
		int minim = min(iPktSize, diff);
		//if (minim > 12) minim -= 12;
		if(iPktSize !=0 && iFrame > 0) {
			addFrame((char *) oc->pb->buffer, diff);
			fwrite (oc->pb->buffer, 1 , diff , db );
		}
        iFrame++;
    }
	printf("\n");
	fclose(db); /*done!*/ 
	
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
