#include <libavcodec/avcodec.h>
#include <libavformat/avformat.h>
#include <libavdevice/avdevice.h> 
#include <libswscale/swscale.h>
#include "util.h"

#define SAMPLE_RATE 44100
#define BITRATE 64000
#define CHANNELS 2



AVFormatContext 	*pMicFormatContext;
AVCodecContext		*pMicCodecContext;
AVCodec				*pMicCodec;
AVFrame				*pMicDec;		
AVPacket			pMicPacket;
int 				iAudioStream;

int16_t 			*AudBuffer;
int16_t 			*pAudBufCurr;
int16_t			*pAudBufFirst;
long int			iAudBufSize;
int test=0;





/*
 * add an audio output stream
 */
AVStream *add_audio_stream(AVFormatContext *oc, enum CodecID codec_id)
{
    AVCodecContext *c;
    AVStream *st;

    st = av_new_stream(oc, 1);
    if (!st) {
        fprintf(stderr, "Could not alloc stream\n");
        exit(1);
    }

    c = st->codec;
    c->codec_id = codec_id;
    c->codec_type = AVMEDIA_TYPE_AUDIO;

    /* put sample parameters */
   // c->sample_fmt = AV_SAMPLE_FMT_S16;
    c->bit_rate = 64000;
    c->sample_rate = 44100;
    c->channels = 2;

    // some formats want stream headers to be separate
    if(oc->oformat->flags & AVFMT_GLOBALHEADER)
        c->flags |= CODEC_FLAG_GLOBAL_HEADER;

    return st;
}
void get_audio_frame(int16_t *samples, int frame_size, int nb_channels)
{
    int j, i, v;
    int16_t *q;

    q = samples;
    for(j=0;j<frame_size;j++) {
        v = (int)(sin(t) * 100000);
        for(i = 0; i < nb_channels; i++)
            *q++ = v;
        t += tincr;
        tincr += tincr2;
    }
}

void open_microphone() {
	int temp = -1;
	AVInputFormat		*pMicInputFormat;
	AVFormatParameters	FormatParamAudDec;

	FormatParamAudDec.sample_rate = SAMPLE_RATE;
    FormatParamAudDec.channels = CHANNELS;
    FormatParamAudDec.time_base = (AVRational){1, SAMPLE_RATE};
    FormatParamAudDec.audio_codec_id = CODEC_TYPE_AUDIO;
    
	pMicInputFormat = av_find_input_format("alsa");

	temp = av_open_input_file(&pMicFormatContext, "plughw:0", pMicInputFormat, 0, &FormatParamAudDec);
	if(temp !=0) {
		printf("Couldn't open Microphone. Error code %d - %s\n", temp, AVERROR_LOOKUP(temp));
		exit(EXIT_FAILURE);
	}
	
	temp = av_find_stream_info(pMicFormatContext);
	if(temp<0) {
		printf("couldn't find stream infomation\n");
		exit(EXIT_FAILURE);
	}

	iAudioStream=-1;
	for(temp=0; temp<(int)pMicFormatContext->nb_streams; temp++) {
		if(pMicFormatContext->streams[temp]->codec->codec_type==CODEC_TYPE_AUDIO) {
			iAudioStream=temp;
			break;
		}
	}
	if(iAudioStream == -1) {
		printf("couldn't select Audio stream\n");
		exit(EXIT_FAILURE);
	}
	
	pMicCodecContext=pMicFormatContext->streams[iAudioStream]->codec; 
	pMicCodec=avcodec_find_decoder(pMicCodecContext->codec_id);
	if(pMicCodec==NULL) {
		fprintf(stderr, "Audio: couldn't find required codec\n");
	}
	
	temp = avcodec_open(pMicCodecContext, pMicCodec);
	if(temp<0) {
		printf("couldn't open decoder codec\n");
	}
	
	printf("Audio Init Complete\n");
}



 void open_audio(AVFormatContext *oc, AVStream *st)
{

    AVCodecContext *c;
    AVCodec *codec;

    c = st->codec;

    /* find the audio encoder */
    codec = avcodec_find_encoder(c->codec_id);
    if (!codec) {
        fprintf(stderr, "codec not found\n");
        exit(1);
    }

    /* open it */
    if (avcodec_open(c, codec) < 0) {
        fprintf(stderr, "could not open codec\n");
        exit(1);
    }

    /* init signal generator */
    t = 0;
    tincr = 2 * M_PI * 110.0 / c->sample_rate;
    /* increment frequency by 110 Hz per second */
    tincr2 = 2 * M_PI * 110.0 / c->sample_rate / c->sample_rate;

    audio_outbuf_size = AVCODEC_MAX_AUDIO_FRAME_SIZE;
    audio_outbuf = (uint8_t*)av_malloc(audio_outbuf_size);

    /* ugly hack for PCM codecs (will be removed ASAP with new PCM
       support to compute the input frame size in samples */
    if (c->frame_size <= 1) {
        audio_input_frame_size = audio_outbuf_size / c->channels;
        switch(st->codec->codec_id) {
        case CODEC_ID_PCM_S16LE:
        case CODEC_ID_PCM_S16BE:
        case CODEC_ID_PCM_U16LE:
        case CODEC_ID_PCM_U16BE:
            audio_input_frame_size >>= 1;
            break;
        default:
            break;
        }
    } else {
        audio_input_frame_size = c->frame_size;
    }
    audio_input_frame_size =AVCODEC_MAX_AUDIO_FRAME_SIZE+FF_INPUT_BUFFER_PADDING_SIZE; // good Measure
    iAudBufSize = audio_input_frame_size*5*c->channels;
    AudBuffer = (int16_t*)av_malloc(iAudBufSize * sizeof(uint16_t));
    pAudBufCurr = AudBuffer;
    pAudBufFirst = AudBuffer;
    samples = (int16_t*)av_malloc(audio_input_frame_size * 2 * c->channels);
}

int get_mic_samples() {
	int len;
	int x;
	x = audio_input_frame_size;
	AVPacket pkt;
	av_init_packet(&pkt);
	len = av_read_frame(pMicFormatContext, &pkt);
	
	if (len != 0) {
		fprintf(stderr, "Couldn't get packet to process, returned %d - %s\n",len,AVERROR_LOOKUP(len));
		exit(EXIT_FAILURE);
	}
	do
	{
	len =  avcodec_decode_audio3(pMicCodecContext, pAudBufCurr, &x, &pkt);
	} while (!x);
	
	return(len);
}



void write_audio_frame(AVFormatContext *oc, AVStream *st)
{
    AVCodecContext *c;
    AVPacket pkt;
    av_init_packet(&pkt);
	int len;
    c = st->codec;

	//~ 
    //~ if (pAudBufCurr == pAudBufFirst) {
	//~ //	printf("Out of samples\n");
		//~ while(pAudBufCurr < (AudBuffer + iAudBufSize - audio_input_frame_size)) {
			//~ //Fill Buffer
			//~ /* Run out of samples get some (buffer full) */
			//~ //get_audio_frame(pAudBufCurr, audio_input_frame_size, c->channels);
			//~ len =get_mic_samples();
			//~ //len = audio_input_frame_size;
			//~ if (len <= 0) {
				//~ break;
			//~ }  else {
				//~ pAudBufCurr+=len;
			//~ }
		//~ }
		//~ if (pAudBufCurr >= (AudBuffer + iAudBufSize - audio_input_frame_size)){
			//~ printf("Resetting pointer\n");
			//~ pAudBufCurr= AudBuffer;
			//~ t = 0;
			//~ tincr = 2 * M_PI * 110.0 / c->sample_rate;
			//~ /* increment frequency by 110 Hz per second */
			//~ tincr2 = 2 * M_PI * 110.0 / c->sample_rate / c->sample_rate;
		//~ }
	//~ }
	//~ 

	get_mic_samples();
	//audio_input_frame_size =100000;
	//get_audio_frame(pAudBufCurr, audio_input_frame_size, c->channels);
	
    pkt.size= avcodec_encode_audio(c, audio_outbuf, audio_outbuf_size, pAudBufFirst);
   //~ pAudBufFirst += audio_input_frame_size;
    //~ if (pAudBufFirst >= (AudBuffer + iAudBufSize - audio_input_frame_size)) {
		//~ pAudBufFirst = AudBuffer;
	//~ }
   // iPktSize=0;
	iPktSize= pkt.size;
	
    if (c->coded_frame && c->coded_frame->pts != (int)AV_NOPTS_VALUE) {
        pkt.pts= av_rescale_q(c->coded_frame->pts, c->time_base, st->time_base);

	}
    pkt.flags |= AV_PKT_FLAG_KEY;
    pkt.stream_index= st->index;
    pkt.data= audio_outbuf;

    /* write the compressed frame in the media file */
    if (av_interleaved_write_frame(oc, &pkt) != 0) {
        fprintf(stderr, "Error while writing audio frame\n");
        exit(1);
    }
    
}

void close_audio(AVFormatContext *oc, AVStream *st)
{
    avcodec_close(st->codec);

    av_free(samples);
    av_free(audio_outbuf);
}
