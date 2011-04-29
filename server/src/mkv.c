#include "util.h"


	AVCodecContext  			*pCodecCtxVidEncMKV;
	AVCodec						*pCodecVidEncMKV;
	AVFrame						*pFrameEncMKV;

    AVOutputFormat *MKVfmt;
    AVFormatContext *MKVoc;
    AVStream *MKV_audio_st, *MKV_video_st;
    double MKV_audio_pts, MKV_video_pts;
    int i;
    
    long int	iFrame = 0;
    
void mkv_all_init(int width, int height, int pix_fmt) {

    MKVfmt = av_guess_format("mkv", NULL, NULL);
    if (!MKVfmt) {
		fprintf(stderr, "Couldn;t setup format\n");
		exit(EXIT_FAILURE);
	}

    /* allocate the output media context */
    MKVoc = avformat_alloc_context();
    if (!MKVoc) {
        fprintf(stderr, "Memory error\n");
        exit(1);
    }
    MKVoc->oformat = MKVfmt;
    snprintf(MKVoc->filename, sizeof(MKVoc->filename), "%s", filename);

    /* add the audio and video streams using the default format codecs
       and initialize the codecs */
    MKV_video_st = NULL;
    MKV_audio_st = NULL;
    if (MKVfmt->video_codec != CODEC_ID_NONE) {
        MKV_video_st = add_video_stream(MKVoc, MKVfmt->video_codec);
    }
    if (fmt->audio_codec != CODEC_ID_NONE) {
        MKV_audio_st = add_audio_stream(MKVoc, MKVfmt->audio_codec);
    }

    /* set the output parameters (must be done even if no
       parameters). */
    if (av_set_parameters(MKVoc, NULL) < 0) {
        fprintf(stderr, "Invalid output format parameters\n");
        exit(1);
    }

   // dump_format(MKVoc, 0, filename, 1);

    /* now that all the parameters are set, we can open the audio and
       video codecs and allocate the necessary encode buffers */
    
        
    if (video_st)
        open_video(MKVoc, MKV_video_st);
    if (audio_st)
        open_audio(MKVoc, MKV_audio_st);

	
    /* open the output file, if needed */
    if (!(fmt->flags & AVFMT_NOFILE)) {
        if (url_fopen(&MKVoc->pb, filename, URL_WRONLY) < 0) {
            fprintf(stderr, "Could not open '%s'\n", filename);
            exit(1);
        }
    }

    /* write the stream header, if any */
    av_write_header(MKVoc);
    
}

uint8_t *encode_frame_to_mkv(AVFrame * pFrameRaw)
        /* compute current audio and video time */
        if (MKV_audio_st)
            MKV_audio_pts = (double)MKV_audio_st->pts.val * MKV_audio_st->time_base.num / MKV_audio_st->time_base.den;
        else
            MKV_audio_pts = 0.0;

        if (MKV_video_st)
            MKV_video_pts = (double)MKV_video_st->pts.val * MKV_video_st->time_base.num / MKV_video_st->time_base.den;
        else
            MKV_video_pts = 0.0;

        if ((!MKV_audio_st || MKV_audio_pts >= STREAM_DURATION) &&
            (!MKV_video_st || MKV_video_pts >= STREAM_DURATION))
            break;

        /* write interleaved audio and video frames */
        if (!MKV_video_st || (MKV_video_st && MKV_audio_st && MKV_audio_pts < MKV_video_pts)) {
            write_audio_frame(MKVoc, MKV_audio_st);
        } else {
            write_video_frame(MKVoc, MKV_video_st);
        }
        MKVoc
}	
   
void close_mpeg_all() {
	 /* write the trailer, if any.  the trailer must be written
     * before you close the CodecContexts open when you wrote the
     * header; otherwise write_trailer may try to use memory that
     * was freed on av_codec_close() */
    //av_write_trailer(oc);
    /* close each codec */
    if (MKV_video_st)
        close_video(MKVoc, MKV_video_st);
    if (MKV_audio_st)
        close_audio(MKVoc, MKV_audio_st);
        
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
    av_free(MKV_oc);

    //return 0;
    dontDie();
}
////////////////////////////////////////// GEORGES MKV PART END ///////////////////////////////////////////////////////////////////////////
///////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////
