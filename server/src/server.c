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
#include "rtspServer.h"

#include <stdlib.h>
#include <stdio.h>

#include <libavcodec/avcodec.h>
#include <libavformat/avformat.h>
#include <libavdevice/avdevice.h> 
#include <libswscale/swscale.h>

FILE * infile;

/* Globals for Live capture - As i'm too lazy to pass them around*/
AVOutputFormat *fmt;
AVFormatContext *oc;
AVStream *audio_st, *video_st;
double audio_pts, video_pts;



int send_packet_from_file(FILE * inf) {
	char * buffer;
	int size;
	int len;
	size = 12 * 1e3;
	
	buffer = malloc(size * sizeof(char));
	len = fread(buffer, 1, size, inf);
	addFrame(buffer, size);
	if (feof(inf)) {
		len = -1;
	}
	return(len);
}

unsigned char * send_live_packet(unsigned char * old_buf_ptr){
	long int diff;
 if (audio_st)
            audio_pts = (double)audio_st->pts.val * audio_st->time_base.num / audio_st->time_base.den;
        else
            audio_pts = 0.0;

        if (video_st)
            video_pts = (double)video_st->pts.val * video_st->time_base.num / video_st->time_base.den;
        else
            video_pts = 0.0;

        if ((!audio_st) &&
            (!video_st))
            return(NULL);

        /* write interleaved audio and video frames */
        if (!video_st || (video_st && audio_st && audio_pts < video_pts)) {
            write_audio_frame(oc, audio_st);
        } else {
            write_video_frame(oc, video_st);
        }
        
        // Write it to the network.
        
        // THIS PIECE OF CODE TOO ABOUT 20 HOURS TO GET RIGHT
        // ANYBODY BREAKS THIS, AND I WILL KILL THEM
        //    GS, 2:26AM, 1ST MAY 2011
		diff = (oc->pb->buf_ptr - old_buf_ptr);
		if (oc->pb->buf_ptr == oc->pb->buffer) { // buffer got flushed
			old_buf_ptr = oc->pb->buf_ptr;
		} else if (diff < 0) { // wrapped buffer
			addFrame((char *) old_buf_ptr,  oc->pb->buf_end - old_buf_ptr);
			addFrame((char *) oc->pb->buffer, oc->pb->buf_ptr - oc->pb->buffer);
			old_buf_ptr = oc->pb->buf_ptr;
		} else if (diff > 0) { // continued buffer
			addFrame((char *) old_buf_ptr, diff);
			old_buf_ptr = oc->pb->buf_ptr;
		}
		return(old_buf_ptr);
}


void setup_file_source() {
	infile = fopen(cliOpts.devicepath, "rb");
	if (infile == NULL) {
		fprintf(stderr, "Couldn't open %s for reading\n", cliOpts.devicepath);
	}
		
}


void setup_webcam_source() {
		setup_ffmpeg();
    
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

    /* set the output parameters (must be done even if no parameters). */
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
    
        
}

void cleanup() {
	int i;
	if (strcmp(cliOpts.mode, "WEBCAM") == 0 ) {
	// write trailer
    av_write_trailer(oc);
	addFrame((char *) oc->pb->buffer, oc->pb->buf_ptr - oc->pb->buffer);	// send the trailer...

    /* close each codec */
    if (video_st)
        close_video(oc, video_st);
    if (audio_st)
        close_audio(oc, audio_st);
        
    close_webcam();

	// free the streams
	for(i = 0; i < (int)oc->nb_streams; i++) {
        av_freep(&oc->streams[i]->codec);
        av_freep(&oc->streams[i]);
    }
	
	if (!(fmt->flags & AVFMT_NOFILE)) {
        url_fclose(oc->pb);
    }
    
    av_free(oc);
	} else if (strcmp(cliOpts.mode, "FILE") == 0) {
		fclose(infile);
	}
	printf("Clean exit, Champagne time\n");
}

int main(int argc, char *argv[]) {
    unsigned char * old_buf_ptr;
    int i;
    long int iFrame = 0;
    

	
	// Read CLI Options, and error if not vaild
	if ( (argc<=1)||(get_options(argc, argv)) ) {
		fprintf(stderr, "error parsing CLI options - couldn't understand options. Run with --help.");
		exit(EXIT_FAILURE);
	}

        
  
    startServerRTSP(20,30015,3015);
    printf("Waiting for 10 seconds for client to connect, before starting.\n");
	sleep(10);	// allow client to connect before the server starts, so that client receives header!
	
	
	if (strcmp(cliOpts.mode, "FILE")==0) {
		setup_file_source();
	} else {
		setup_webcam_source();
		/* write the stream header, if any */
		av_write_header(oc);
		addFrame((char *) oc->pb->buffer, 552);	// send the header...
		iFrame =0;

		old_buf_ptr = oc->pb->buf_ptr;
	}
    for(;;) {
		
		printf("\rFrame %5ld", iFrame);
        if(strcmp(cliOpts.mode, "FILE") == 0) {
			if(send_packet_from_file(infile) == -1) break;
		} else {
			old_buf_ptr = send_live_packet(old_buf_ptr);
			if (old_buf_ptr == NULL)
			break;
		}
		
        iFrame++;
    }
	
	cleanup();
	exit(EXIT_SUCCESS);
}
