//Description:
//	Main Server Program.
//
//Sources:
//	http://www.inb.uni-luebeck.de/~boehme/using_libavcodec.html
//	http://www.inb.uni-luebeck.de/~boehme/libavcodec_update.html
//	http://www.ibm.com/developerworks/aix/library/au-unix-getopt.html
//
//Copyright 2011:
//	The UNV Project
//	Department of Electronic & Electrical Engineering
//	University College London
//
//	http://www.ee.ucl.ac.uk/
//		
//Project Authors:
//	George Smart		g.smart@ee.ucl.ac.uk (lead developer)
//	Obada Sawalha		o.sawalha@ee.ucl.ac.uk
//	Grigorios Stathis	uceegrs@ee.ucl.ac.uk
//	Lorenzo Levrini		l.levrini@ee.ucl.ac.uk
//	Stelios Vitorakis	s.vitorakis@ee.ucl.ac.uk
//	Hans Balgobin		h.balgobin@ee.ucl.ac.uk
//	Yiannis Andreopoulos	i.andreop@ee.ucl.ac.uk (supervisor)
//			
//Licence:
//	See Licence.txt
//

#define	VERSIONNUM "2.1.3"
#define	VERSIONTXT	"TCP/UDP Networking with H264/MJPEG Codecs"
#define USE_TCP   0
#define USE_UDP   1
#define USE_H264  2
#define USE_MJPEG 3

//  Get Project Headers.
#include "Server.h"
#include <stdio.h>
#include <stdint.h>
#include <stdlib.h>
#include <sys/time.h>
#include <time.h>
#include <unistd.h>
#include "rtspServer.h" // requires bundled header.

uint32_t	stampstart();
uint32_t	stampstop(uint32_t start);
uint32_t	start, stop;
float 		sum;
int 		counter;
FILE 		*test;
bool		StopExecution = false;

uint32_t stampstart() {
	struct timeval  tv;
	struct timezone tz;
	struct tm      *tm;
	uint32_t         start;

	gettimeofday(&tv, &tz);
	tm = localtime(&tv.tv_sec);

	/*printf("TIMESTAMP-START\t  %d:%02d:%02d:%d (~%d ms)\n", tm->tm_hour,
		  tm->tm_min, tm->tm_sec, tv.tv_usec,
		  tm->tm_hour * 3600 * 1000 + tm->tm_min * 60 * 1000 +
		  tm->tm_sec * 1000 + tv.tv_usec / 1000);
	*/
	
	start = tm->tm_hour * 3600 * 1000 + tm->tm_min * 60 * 1000 +
		tm->tm_sec * 1000 + tv.tv_usec / 1000;

	return (start);

}

uint32_t stampstop(uint32_t start) {

	struct timeval  tv;
	struct timezone tz;
	struct tm      *tm;
	uint32_t         stop;

	gettimeofday(&tv, &tz);
	tm = localtime(&tv.tv_sec);

	stop = tm->tm_hour * 3600 * 1000 + tm->tm_min * 60 * 1000 +
		tm->tm_sec * 1000 + tv.tv_usec / 1000;

	/*printf("TIMESTAMP-END\t  %d:%02d:%02d:%d (~%d ms) \n", tm->tm_hour,
		  tm->tm_min, tm->tm_sec, tv.tv_usec,
		  tm->tm_hour * 3600 * 1000 + tm->tm_min * 60 * 1000 +
		  tm->tm_sec * 1000 + tv.tv_usec / 1000);

	printf("ELAPSED\t  %d ms\n", stop - start);*/

	return (stop);
}

//void runAudioLoop(void);
void runVideoLoop(void);

int main(int argc, char * argv[]) 
{
	cout << "The UNV Project - Server" << endl;
	cout << "Department of Electronic & Electrical Engineering" << endl;
	cout << "University College London" << endl << endl;
	
	// Read CLI Options, and error if not vaild
	if ( (argc<=1)||(get_options(argc, argv)) ) 
	{
		fail("parsing CLI options", "couldn't understand options. Run with --help.", FALSE);
		exit(EXIT_SUCCESS);
	}
	
	// check that mandatory command line args are there...
	if ( (cliOpts.mode==NULL)||(cliOpts.devicepath==NULL)||(cliOpts.networkport==0)||(cliOpts.transport==4)||(cliOpts.vcodec==4) ) 
	{
		printf("You must specify mode (-m), device (-d), network port (-p), network transport (-t) and a video codec (-c)\n");
		fail("parsing CLI options", "Mandatory options missing! Run with --help.", FALSE);
		exit(EXIT_FAILURE);
	}

	// register all codecs, drivers, devics, etc so we know what we have.
	if (cliOpts.verbose) { cout << "Registering Codecs and Device drivers" << endl; } // Verbose

	// Init libav stuff
	avcodec_init();
	av_register_all();
	avdevice_register_all();
	av_init_packet(&CurrentPacket);

	// Show version information
	if (cliOpts.version) {
		show_version();
		exit(EXIT_SUCCESS);
	}

	// Show formats supported if asked.
	if (cliOpts.formats) {
		show_format(pInputFmtVidDec);
		exit(EXIT_SUCCESS);
	}
	
	// Before we get onto the deep stuff, check if we know what we're about...
	if ((cliOpts.devicepath == NULL) || (cliOpts.mode == NULL)) {
		fail("intialising", "device or mode not set.", TRUE);
	}
	
	// Program actually starts here - before was just checking stuffs //
	
	// Allocate codec context
	if (cliOpts.verbose) { cout << "Allocating memory for codec contexts" << endl; }

	pCodecCtxVidEnc = avcodec_alloc_context();
	pCodecCtxVidDec = avcodec_alloc_context();

	pCodecCtxAudDec = avcodec_alloc_context();
	pCodecCtxAudEnc = avcodec_alloc_context();
	
	if ( (pCodecCtxVidEnc == NULL) || (pCodecCtxVidDec == NULL) ) {
		fail("allocating frames", "couldn't allocate either pFrame of pFrameRGB", TRUE);
	}

	// Allocate Frames
	if (cliOpts.verbose)
	{
		cout << "Allocating memory for frames " << "\n";
	}

	pFrameDec = avcodec_alloc_frame();
	pFrameEnc = avcodec_alloc_frame();
	pFrameRGB = avcodec_alloc_frame();

	if ( (pFrameDec == NULL) || (pFrameEnc == NULL) || (pFrameRGB == NULL) ) {
		fail("allocating frames", "couldn't allocate either pFrame of pFrameRGB", TRUE);
	}

	// Get options for input type
	if (strcmp(cliOpts.mode, "file") == 0 || strcmp(cliOpts.mode, "file") == 0) {
	  int temp;
		// File
		if (cliOpts.verbose) {
			cout << "Opening '" << cliOpts.devicepath << "' as input file" << "\n";
		}
			// Verbose

		temp = av_open_input_file(&pFormatCtxVidDec, cliOpts.devicepath, NULL, 0, NULL);
		iFileOpen = av_open_input_file(&pFormatCtxAudDec,cliOpts.devicepath, NULL, 0, NULL);
		
		if(temp != 0) {
			AVERROR_LOOKUP(temp);
			fail("opening video", "couldn't open input file", TRUE);
		}

		if(iFileOpen != 0) {
			AVERROR_LOOKUP(iFileOpen);
			fail("opening audio", "couldn't open audio device", TRUE);
		}
	} else {
	  int temp;
		// Video4Linux2
		if (cliOpts.verbose) { cout << "Opening " << cliOpts.devicepath << " as Video4Linux2 device" << "\n";  } // Verbose
		FormatParamVidDec.channel = 0;
		FormatParamVidDec.standard = "pal"; // pal (or ntsc)
		FormatParamVidDec.width = cliOpts.width;    // read from CLI
		FormatParamVidDec.height = cliOpts.height;   // read from CLI
		FormatParamVidDec.time_base.num = 1;
		FormatParamVidDec.time_base.den = 15;
		pInputFmtVidDec = av_find_input_format("video4linux2");
		/*FIXME*/ //temp = av_open_input_file(&pFormatCtxVidDec, cliOpts.devicepath, pInputFmtVidDec, 0, &FormatParamVidDec); // should take the FormatParamVidDec option, but seg faults. /////////////////////////////////////////
		temp = av_open_input_file(&pFormatCtxVidDec, cliOpts.devicepath, pInputFmtVidDec, 0, NULL);
		if(temp !=0) {
			AVERROR_LOOKUP(temp);
			fail("opening video", "couldn't open input file", TRUE);
		}
	}

	// Set parameters

	FormatParamAudDec.sample_rate = SAMPLE_RATE;
	FormatParamAudDec.channels = CHANNELS;
	FormatParamAudDec.time_base = (AVRational){1, SAMPLE_RATE};
	// FormatParamAudDec.audio_codec_id = CODEC_TYPE_AUDIO;

	// Find input format
	pInputFmtAudDec = av_find_input_format("alsa");

	// Open device
	if (cliOpts.verbose) {
		cout << "Opening '" << "plughw:0,0" << "'  as ALSA Audio device" << "\n"; 
	}
	
	iFileOpen = av_open_input_file(&pFormatCtxAudDec, "plughw:0,0", pInputFmtAudDec, 0, &FormatParamAudDec);
	if (cliOpts.verbose) { cout << "av_open_input_file: " << iFileOpen << "\n"; }

	if(iFileOpen<0) {
		AVERROR_LOOKUP(iFileOpen);
		fail("opening video", "couldn't open audio device", TRUE);
	}

	if(iFileOpen==0 && cliOpts.verbose) {
		cout << "Opened successfully!" << "\n";
	}

	// Retrieve stream information
	if (cliOpts.verbose) { cout << "Analysing video stream(s)" << "\n"; } {
	  int temp;
	  temp = av_find_stream_info(pFormatCtxVidDec);
		if(temp<0) {
			AVERROR_LOOKUP(temp);
			fail("finding stream", "couldn't find stream infomation", TRUE);
		}
	}
	
	// dump what we've found to stderr.
	if (cliOpts.verbose) {
		cerr << "Format Dump" << "\n";
		dump_format(pFormatCtxVidDec, 0, cliOpts.devicepath, FALSE);
	}

	// Find the first video stream
	if (cliOpts.verbose) {
		cout << "Selecting first video stream" << "\n";
	}

	iVideoStream=-1;

	for(int i=0; i<(int)pFormatCtxVidDec->nb_streams; i++) {
		if(pFormatCtxVidDec->streams[i]->codec->codec_type==CODEC_TYPE_VIDEO)
		{ //modified codec.codec... to union in structure - GS
			iVideoStream=i;
			break;
		}
	}

	if(iVideoStream == -1) {
		fail("selecting stream", "couldn't select first stream", TRUE);
	}

	iAudioStream = -1;

	if (cliOpts.verbose) { // Look for audio stream
		cout << "Looking for audio stream..." << "\n";
	}

	for (int i = 0; i < (int)pFormatCtxAudDec->nb_streams; i++){ // Loop through streams
		if (pFormatCtxAudDec->streams[i]->codec->codec_type==CODEC_TYPE_AUDIO) {
			// If audio stream is found
			iAudioStream = i;
			break;
		}
	}

	if (cliOpts.verbose) {
		cout << "iAudioStream = " << iAudioStream << "\n";
	}

	if (iAudioStream < 0 ) {
		fail("selecting stream", "couldn't select audio stream", TRUE);
	}

	// Get a pointer to the codec context for the video stream
	pCodecCtxVidDec=pFormatCtxVidDec->streams[iVideoStream]->codec; // removed address operator - GS

	// Find the decoder for the video stream
	pCodecVidDec=avcodec_find_decoder(pCodecCtxVidDec->codec_id);

	if(pCodecVidDec==NULL) { fail("finding decoder", "couldn't find required codec", TRUE); }

	// Populate AVCodecContext with info from stream
	pCodecCtxAudDec = pFormatCtxAudDec->streams[iAudioStream]->codec;

	if (pCodecCtxAudDec == NULL) {
		cerr << "Error: pCodecCtxAudDec is null!" << "\n";
		return 1;
	}

	// Find decoder
	pCodecAudDec = avcodec_find_decoder(pCodecCtxAudDec->codec_id);

	if(pCodecAudDec==NULL) {
		fail("finding decoder", "couldn't find required codec", TRUE);
	}

	// Find the codec for our required format
	if (cliOpts.vcodec == USE_MJPEG) {pCodecVidEnc = avcodec_find_encoder(CODEC_ID_MJPEG);}
	if (cliOpts.vcodec == USE_H264) {pCodecVidEnc = avcodec_find_encoder(CODEC_ID_H264);}
	
	if (pCodecVidEnc == NULL) {
		fail("finding encoder", "couldn't find required codec", TRUE);
	}

	// X.264 Parameters
	if (cliOpts.vcodec == USE_H264) {
		pCodecCtxVidEnc->thread_count = 2;
		avcodec_thread_init(pCodecCtxVidEnc, pCodecCtxVidEnc->thread_count);
		pCodecCtxVidEnc->flags |= CODEC_FLAG_LOOP_FILTER;
		pCodecCtxVidEnc->me_cmp |= 1;
		pCodecCtxVidEnc->partitions |= X264_PART_I8X8+X264_PART_I4X4+X264_PART_P8X8+X264_PART_B8X8;
		pCodecCtxVidEnc->me_method = ME_HEX;
		pCodecCtxVidEnc->me_subpel_quality = 2;
		//c->me_range = 16;
		pCodecCtxVidEnc->keyint_min = 25;
		//c->scenechange_threshold=40;
		//c->qcompress = 0.6;
		pCodecCtxVidEnc->qmin = 10;
		pCodecCtxVidEnc->qmax = 51;
		pCodecCtxVidEnc->max_qdiff = 4;
		pCodecCtxVidEnc->max_b_frames = 0;
		pCodecCtxVidEnc->refs = 1;
		//c->directpred = 1;
		//c->trellis = 1;
		pCodecCtxVidEnc->flags2 |= CODEC_FLAG2_MIXED_REFS+CODEC_FLAG2_WPRED+CODEC_FLAG2_8X8DCT+CODEC_FLAG2_FASTPSKIP;//CODEC_FLAG2_BPYRAMID+
		//c->weighted_p_pred = 2;
		pCodecCtxVidEnc->crf = 32;
		pCodecCtxVidEnc->rc_lookahead = 0;
		pCodecCtxVidEnc->gop_size = 10;
		//pCodecCtxVidEnc->time_base.den * 2; //250;// emit one intra frame every twelve frames at most
		//pCodecCtxVidEnc->pix_fmt = STREAM_PIX_FMT;
		pCodecCtxVidEnc->width = 640;
		pCodecCtxVidEnc->height = 480;
		pCodecCtxVidEnc->time_base=(AVRational){1,5};
		// some formats want stream headers to be separate
		if(pCodecCtxVidEnc->flags & AVFMT_GLOBALHEADER)
		pCodecCtxVidEnc->flags |= CODEC_FLAG_GLOBAL_HEADER;
		if(cliOpts.transport==USE_TCP) {pCodecCtxVidEnc->bit_rate = 1000000; printf("Capping BR for TCP/H264\n");}
		if (cliOpts.verbose) { cout << "x264 configured" << "\n"; }
	}
	
	// MJPEG Parameters
	if (cliOpts.vcodec == USE_MJPEG) {
		pCodecCtxVidEnc->codec_type = CODEC_TYPE_VIDEO;
		pCodecCtxVidEnc->codec_id = CODEC_ID_MJPEG;
		pCodecCtxVidEnc->bit_rate = BITRATE;
		pCodecCtxVidEnc->width = pCodecCtxVidDec->width;
		pCodecCtxVidEnc->height = pCodecCtxVidDec->height;
		pCodecCtxVidEnc->time_base=(AVRational){1,25};
		pCodecCtxVidEnc->pix_fmt = PIX_FMT_YUVJ420P;//YUVJ422P
		pCodecCtxVidEnc->color_range=AVCOL_RANGE_JPEG;
		pCodecCtxVidEnc->qmin= 90;
		pCodecCtxVidEnc->qmax= 90;
		if (cliOpts.verbose) { cout << "MJPEG configured" << "\n"; }
	}
	
	// Find encoder
	pCodecAudEnc = avcodec_find_encoder(CODEC_ID_MP3);
	if (pCodecAudEnc == NULL) {
		fail("Error:", "codec not found!", TRUE);
		return 1;
	}

	//Set encoder parameters
	pCodecCtxAudEnc->bit_rate = BITRATE;
	pCodecCtxAudEnc->sample_fmt = SAMPLE_FMT_S16;
	pCodecCtxAudEnc->sample_rate = SAMPLE_RATE;
	pCodecCtxAudEnc->channels = CHANNELS;
	//pCodecCtxAudEnc->profile = 1; //FF_PROFILE_AAC_MAIN;
	pCodecCtxAudEnc->time_base = (AVRational){1, SAMPLE_RATE};
	pCodecCtxAudEnc->codec_type = CODEC_TYPE_AUDIO;
	
	// Open decoder
	if (cliOpts.verbose) { cout << "Opening codec (decoder)" << "\n"; } {
		int temp;
		temp = avcodec_open(pCodecCtxVidDec, pCodecVidDec);

		if(temp<0) {
			AVERROR_LOOKUP(temp);
			fail("opening codec", "couldn't open decoder codec", TRUE);
		}
	}
	
	// Open the encoder
	if (cliOpts.verbose) { cout << "Opening codec (encoder)" << "\n"; } {
		int temp;
		temp = avcodec_open(pCodecCtxVidEnc, pCodecVidEnc);
		if(temp < 0) {
			AVERROR_LOOKUP(temp);
			fail("opening codec", "couldn't open encoder codec", TRUE);
		}
	}
	
	// Open decoder
	iCodecOpen = avcodec_open(pCodecCtxAudDec, pCodecAudDec);
	if (cliOpts.verbose) { cout << "avcodec_open (decoder): " << iCodecOpen << "\n"; }
	if (iCodecOpen < 0 ) {
		AVERROR_LOOKUP(iCodecOpen);
		fail("opening codec", "couldn't open decoder codec", TRUE);
	}

	// Open encoder
	iOpenEncoder = avcodec_open(pCodecCtxAudEnc, pCodecAudEnc);
	if (cliOpts.verbose) { cout << "avcodec_open (encoder): " << iOpenEncoder << "\n"; }
	if(iOpenEncoder < 0) {
		AVERROR_LOOKUP(iOpenEncoder);
		fail("opening codec", "couldn't open encoder codec", TRUE);
	}

	// Allocate YUV and RGB memory.
	if (cliOpts.verbose) { cout << "Allocating format conversion colourspace" << "\n"; }
	iRGBFrameSize=avpicture_get_size(PIX_FMT_RGB24, pCodecCtxVidDec->width, pCodecCtxVidDec->height);
	iYUVFrameSize = avpicture_get_size(PIX_FMT_YUV420P, pCodecCtxVidEnc->width, pCodecCtxVidEnc->height);
	iOutBufferSize = 100000;

	if (cliOpts.verbose) {
		cout << "  RGB: " << iRGBFrameSize << " Bytes\n  YUV: " << iYUVFrameSize << " Bytes\n  OUT: " << iOutBufferSize << " Bytes\n";
	}

	pYUVBufferVid = (uint8_t *) malloc((iYUVFrameSize));
	pRGBBufferVid = (uint8_t *) malloc(iRGBFrameSize);
	
	// check we got the buffer we wanted.
	if ( (pRGBBufferVid == NULL) || (pYUVBufferVid == NULL) ) {   
		fail("malloc buffers", "couldn't allocate memory for pFrameRGB or pFrameEnc buffers", TRUE);
	}

	if (cliOpts.verbose) { cout << "Assigning RGB and YUV buffers to AVFrames.\n"; }

	// Assign appropriate parts of buffer to image planes in pFrameRGB and pFrameYUV
	avpicture_fill((AVPicture *)pFrameRGB, pRGBBufferVid, PIX_FMT_RGB24, pCodecCtxVidDec->width, pCodecCtxVidDec->height);  // RGB size
	avpicture_fill((AVPicture *)pFrameEnc, pYUVBufferVid, PIX_FMT_YUV420P, pCodecCtxVidDec->width, pCodecCtxVidDec->height);  // YUV size
	pOutBufferVid = (unsigned char *) av_malloc(iOutBufferSize * sizeof(uint8_t));  // alloc output buffer for encoded datastream
	
	// Allocate Encoding and Decoding Buffers AUDIO
	// Decode output buffer
	iOutbufDecSize = AVCODEC_MAX_AUDIO_FRAME_SIZE;
	outbufDec = (uint8_t *)av_malloc(iOutbufDecSize + FF_INPUT_BUFFER_PADDING_SIZE);

	// Encode output buffer
	iOutbufEncSize = 10000;
	outbufEnc = (uint8_t *)av_malloc(iOutbufEncSize);
	
	// if we want to save video
	if (cliOpts.savevideo) {
		// Open output file handle

		if (cliOpts.verbose) {
			cout << "Opening output video file" << "\n";
		}

		pOutputVideo = fopen(cliOpts.videopath, "wb"); // Write Binary
		test=fopen("test.264","wb");

		if ((pOutputVideo == NULL) && (test == NULL)) {
			cerr << "Could not open " << cliOpts.videopath << "\n";
			fail("opening file handle", "couldn't open file", TRUE);
		}
	}

	// if we want to save the audio
	if (cliOpts.saveaudio) {
		// Open output file handle
		if (cliOpts.verbose) {
			cout << "Opening output audio file" << "\n";
		}
		pOutputAudio = fopen(cliOpts.audiopath, "wb"); // Write Binary
		if (!pOutputAudio) {
			cerr << "Could not open " << cliOpts.audiopath << "\n";
			fail("opening file handle", "couldn't open file", TRUE);
		}
	}

	// Stats file
	pStatisticsFile = fopen("stats.csv", "w");
	
	if(cliOpts.networkport > 0) {	// if the port is greater than 0, we requested it.
		startServerRTSP(20,cliOpts.networkport, (cliOpts.networkport + 5), cliOpts.transport);
	}

	if (cliOpts.verbose) {
		cout << "Entering main processing loop" << "\n";
		cout << "If you would like to have this debugged, run with two verbose options.\nThis will slow the program a lot!\n";
	}

	//boost::thread AudioThread(runAudioLoop);
	//timer start
	cout << "Spawning Server Child Thread" << endl;
	start = stampstart();
	boost::thread VideoThread(runVideoLoop);
	
	// Setup Signal Catcher callback...
	// This lets us exit nicely once the program loop is running.
	cout << "Setting Signal Handler" <<endl;
	signal(SIGINT, signal_handler);		// on signal, we call signal_handler with the sig type as an integer arg

	// fall through this when the stop flag is set.
	while(StopExecution == false) {
		usleep(10*1000*1000);
	}
	cout << "Parent Thread Stop Execution Requested" << endl;
	
	// report back to the user how many frames were processed.
	cout << (long) iFramesDecoded << " frames parsed" << "\n";
	
	if (cliOpts.verbose) {
		cout << "Freeing memory " << "\n";
	}

	// Free frame memory
	av_free(pFrameDec);
	av_free(pFrameEnc);
	av_free(pFrameRGB);
	free(pOutBufferVid);
	free(pRGBBufferVid);
	free(pYUVBufferVid);
	free(outbufDec);
	free(outbufEnc);

	if (cliOpts.verbose) {
		cout << "Freeing codecs " << "\n";
	}

	// Close the codecs
	avcodec_close(pCodecCtxVidDec);
	avcodec_close(pCodecCtxVidEnc);
	
	avcodec_close(pCodecCtxAudDec);
	avcodec_close(pCodecCtxAudEnc);

	if (cliOpts.verbose) {
		cout << "Freeing I/O " << "\n";
	}

	// Close video file
	av_close_input_file(pFormatCtxVidDec);

	// Close the video output file handle IF it was opened
	if (cliOpts.savevideo) {
		if (cliOpts.verbose) {
			cout << "Close Video Dump" << "\n";
		}
		fclose(pOutputVideo);
	}

	// Close the audio output file handle IF it was opened

	if (cliOpts.saveaudio) {
		if (cliOpts.verbose) {
			cout << "Close Audio Dump" << "\n";
		}
		fclose(pOutputAudio);
	}
	
	// close stats file
	fclose(pStatisticsFile);
	
	if (cliOpts.verbose) {
		cout <<"Exiting..." << "\n";
	}

	exit(EXIT_SUCCESS);
}

void runVideoLoop()
{
	int iFramesDecoded = 0, pOutBufferVid_size=0;

	//int cnt=1;
	uint8_t *packetBufferVid;
	packetBufferVid = (uint8_t *) av_malloc(100000 * sizeof(uint8_t));  // alloc output buffer for encoded datastream
	
	int frame_id=0, framerate_drop=1; // these are used to drop camera frames if needed, set to 1 for no drops
	while(av_read_frame(pFormatCtxVidDec, &packetVidDec)>=0) {
		if( StopExecution == true ) {
			cout << "Child Thread Stop Execution Requested; terminating..." << endl;
			break;
		}
		start=stampstart();
		
		// Only drop frames with TCP/H264
		if ( (cliOpts.transport==USE_TCP) && (cliOpts.vcodec==USE_H264) ) {
			frame_id++;
			if (frame_id>204800000)
				frame_id=0;
		
			if ((frame_id % framerate_drop) != 0)
				packetVidDec.stream_index = -1;
		}
		
		// Is this a packetVidDec from the video stream?
		if(packetVidDec.stream_index == iVideoStream) {
			
			  start = stampstart();
			
			// Decode video frame
			if (cliOpts.verbose >= 2) {
				cout << "Decode new packetVidDec" << "\n";
			}
			avcodec_decode_video2(pCodecCtxVidDec, pFrameDec, &iFrameFinished, &packetVidDec);
			
			// Did we get a video frame?
			if(iFrameFinished) {
				if (cliOpts.verbose >= 2) {
					cout << "Got a new video frame: " << iFramesDecoded << "\n";
				}
				
				// Convert the image from its native format to RGB to save as ppm
				if (swsC_RGB == NULL) 
				{
					if (cliOpts.verbose >= 2) 
					{
						cerr << "Setting up conversion context for RGB:\n ";
						cerr << " Input type: " << pCodecCtxVidDec->width << "x" << pCodecCtxVidDec->height << " in "; 
						PIX_FMT_LOOKUP(pCodecCtxVidDec->pix_fmt); 
						cerr << " format\n";
						cerr << "  Output type: " << pCodecCtxVidEnc->width << "x" << pCodecCtxVidEnc->height << " in "; 
						PIX_FMT_LOOKUP(PIX_FMT_RGB24); 
						cerr << " format\n";
					}
					
					// Setup image converter...  Convert from what we get to RGB24 to save as image
					swsC_RGB = sws_getContext(	pCodecCtxVidDec->width,	//	Source width
												pCodecCtxVidDec->height,	//	Source height
												pCodecCtxVidDec->pix_fmt, //	Source format
												pCodecCtxVidDec->width,	//	Dest width
												pCodecCtxVidDec->height,	//	Dest height
												PIX_FMT_RGB24,		//	Dest format
												SWS_FAST_BILINEAR,	//	Transform Type (choose from SWS FLAGS)
												NULL, NULL, NULL);	//	Filters & Pointer Setup (NULL chooses defaults for last 3 options)
				}
				
				if (swsC_RGB == NULL) {	fail("sws_getContext RGB", "Couldn't setup a format conversion.", TRUE); }
				if (cliOpts.verbose >= 2) { cout << "Calling sws_scale for colourspace conversion RGB\n"; }
				
				swsR_RGB = sws_scale( swsC_RGB,									//	SwsContext - Setup for scaling
									(const uint8_t* const*)	pFrameDec->data,	//	Source frame data (cast to required type)
									pFrameDec->linesize,						//	Source frame stride
									0,										//	Source frame slice width (raster, so no width slicing)
									pCodecCtxVidDec->height,						//	Source frame slice height
									pFrameRGB->data,						//	Dest frame data
									pFrameRGB->linesize);					//	Dest frame stride

				if (swsR_RGB <= 0) { fail("sws_scale RGB", "Couldn't determine output slice size.", TRUE); }
				
				// Convert the image from its native format to YUV420P for encoder
				if (swsC_YUV == NULL) {
					if (cliOpts.verbose >= 2) {
						cerr << "Setting up conversion context for YUV:\n "; 
						cerr << " Input type: " << pCodecCtxVidDec->width << "x" << pCodecCtxVidDec->height << " in ";
						PIX_FMT_LOOKUP(pCodecCtxVidDec->pix_fmt); 
						cerr << " format\n";
						cerr << "  Output type: " << pCodecCtxVidEnc->width << "x" << pCodecCtxVidEnc->height << " in "; 
						PIX_FMT_LOOKUP(pCodecCtxVidEnc->pix_fmt); 
						cerr << " format\n";
					}
					
					// Setup image converter...  Convert from what we get to YUV420P for Encoder
					swsC_YUV = sws_getContext(	pCodecCtxVidDec->width,	//	Source width
												pCodecCtxVidDec->height,	//	Source height
												pCodecCtxVidDec->pix_fmt, //	Source format
												pCodecCtxVidDec->width,	//	Dest width
												pCodecCtxVidDec->height,	//	Dest height
												PIX_FMT_YUV420P,		//	Dest format
												SWS_FAST_BILINEAR,	//	Transform Type (choose from SWS FLAGS)
												NULL, NULL, NULL);	//	Filters & Pointer Setup (NULL chooses defaults for last 3 options)
				}
				
				if (swsC_YUV == NULL) {	fail("sws_getContext YUV", "Couldn't setup a format conversion.", TRUE); }
				if (cliOpts.verbose >= 2) { cout << "Calling sws_scale for colourspace conversion YUV" << "\n"; }
				
				swsR_YUV = sws_scale( swsC_YUV,									//	SwsContext - Setup for scaling
									(const uint8_t* const*)	pFrameDec->data,	//	Source frame data (cast to required type)
									pFrameDec->linesize,						//	Source frame stride
									0,											//	Source frame slice width (raster, so no width slicing)
									pCodecCtxVidDec->height,					//	Source frame slice height
									pFrameEnc->data,							//	Dest frame data
									pFrameEnc->linesize);						//	Dest frame stride

				if (swsR_YUV <= 0) {
					fail("sws_scale YUV", "Couldn't determine output slice size.", TRUE);
				}
				
				// Save frame to disk PPM ( if the user wants it, and if it's the nth frame )
				if ((cliOpts.saveframes > 0) && (iFramesDecoded%cliOpts.saveframes == 0)) {
					if (cliOpts.verbose >= 2) { cout << "Saving this frame, as requested." << "\n"; }
					SaveFrame(pFrameRGB, pCodecCtxVidDec->width, pCodecCtxVidDec->height, iFramesDecoded);
				}
				
				// Encode the video
				if (cliOpts.verbose >= 2) {
					cout << "Calling avcodec_encode_video(): ";
				}
				
				iEncodedBytes = avcodec_encode_video(pCodecCtxVidEnc, packetBufferVid, iOutBufferSize, pFrameEnc);
				
				if (iEncodedBytes > 0) {
					
					if(cliOpts.networkport > 0) {	
						stop = stampstop(start);
						double dt=stop-start;
						sum=sum+dt;
						if(counter==10) {
							cout<<"The average delay is(/10 frames): "<<sum/10<<" ms"<<endl;
							sum=0;
							counter=0;
						}
						counter++;
						
						while(slp) {
							sleep(1);
						}
						
						if ( (cliOpts.transport==USE_UDP) && (cliOpts.vcodec==USE_MJPEG) ) {
							// Fragmentation code: This is used to separate packets into chunks of predefined size MAX_FLUSH_BUFF_SIZE
							// Currently commented out for H.264 as it is still in testing phase and it is not needed under normal circumstances
							// (i.e. the codec produces small enough packets)
						  
							for (int i=pOutBufferVid_size; i<pOutBufferVid_size+iEncodedBytes; i++) {
								if (pOutBufferVid_size==0) {
									pOutBufferVid[i+1]=packetBufferVid[i];
								} else {
									pOutBufferVid[i]=packetBufferVid[i-pOutBufferVid_size];
								}
							}  // FOR
							
							if (pOutBufferVid_size==0) {
								pOutBufferVid_size+=iEncodedBytes+1; // this includes the 1st byte that is the dependencies byte
							} else {
								pOutBufferVid_size+=iEncodedBytes;
							} //IF/ELSE
							
							if (pOutBufferVid_size>MIN_FLUSH_BUFF_SIZE) {
								cout<<"\n Going to deal with a packet of size: "<<pOutBufferVid_size<<endl;
								if (pOutBufferVid_size>MAX_FLUSH_BUFF_SIZE) {
									// first chunk here
									pOutBufferVid[0]=0; // chunk num
									// if we requested networking, do it.
									if (!cliOpts.savevideo) {
										addFrame((char *)(pOutBufferVid), MAX_FLUSH_BUFF_SIZE, cliOpts.transport);
									} else {
										unsigned char *tmp;
										tmp=(pOutBufferVid+1);
										fwrite(tmp, sizeof(uint8_t), MAX_FLUSH_BUFF_SIZE-1, pOutputVideo);
									}
							
									for (int i=1; i<pOutBufferVid_size/MAX_FLUSH_BUFF_SIZE; i++) { // all intermediate chunks
										pOutBufferVid[i*MAX_FLUSH_BUFF_SIZE-1]=i; // chunk num
								
										if (!cliOpts.savevideo) {
											addFrame((char *)(pOutBufferVid+i*MAX_FLUSH_BUFF_SIZE-1), MAX_FLUSH_BUFF_SIZE+1, cliOpts.transport);
										} else {
											unsigned char *tmp;
											tmp=(pOutBufferVid+i*MAX_FLUSH_BUFF_SIZE);
											fwrite(tmp, sizeof(uint8_t), MAX_FLUSH_BUFF_SIZE, pOutputVideo);
								  
										}
									}
									if (pOutBufferVid_size%MAX_FLUSH_BUFF_SIZE>0) { // last chunk
										pOutBufferVid[(pOutBufferVid_size/MAX_FLUSH_BUFF_SIZE)*MAX_FLUSH_BUFF_SIZE-1]=127; // no dependencies
										// if we requested networking, do it.
										if (!cliOpts.savevideo) {
											addFrame((char *)(pOutBufferVid+(pOutBufferVid_size/MAX_FLUSH_BUFF_SIZE)*MAX_FLUSH_BUFF_SIZE-1), (pOutBufferVid_size%MAX_FLUSH_BUFF_SIZE)+1, cliOpts.transport);
										} else {
											unsigned char *tmp;
											tmp=(pOutBufferVid+(pOutBufferVid_size/MAX_FLUSH_BUFF_SIZE)*MAX_FLUSH_BUFF_SIZE);
											fwrite(tmp, sizeof(uint8_t), (pOutBufferVid_size%MAX_FLUSH_BUFF_SIZE), pOutputVideo);
										}								
										pOutBufferVid_size=0;
									}
								} else {
									pOutBufferVid[0]=127; // no dependencies
									if (!cliOpts.savevideo) {
										addFrame((char *)pOutBufferVid, pOutBufferVid_size, cliOpts.transport);
									} else {
										fwrite(packetBufferVid, sizeof(uint8_t), iEncodedBytes, pOutputVideo);
									}
									pOutBufferVid_size=0;
								}
							}
						
						//Sleep so that file is not read all to memory instantly
						if ((strcmp(cliOpts.mode, "FILE")==0)||(strcmp(cliOpts.mode, "FIL")==0)) {
							usleep(33000); //Sleep for 33 miliseconds
						}
						
						if (cliOpts.verbose >= 2) {
							cout << "Network ";
						}

						} else {
							// don't use fragmentation code if it's not MJPEG/UDP.
							addFrame((char *)(packetBufferVid), iEncodedBytes, cliOpts.transport);
						}
					}
					if (cliOpts.savevideo) {
						// if we want video saved to file
						// Write first iEncodedBytes of pOutBufferVid (buffer) to pOutputVideo (file handle)
			
						fwrite(packetBufferVid, sizeof(uint8_t), iEncodedBytes, test);
					 
						if (cliOpts.verbose >= 2) {
							cout << "File ";
						}
					} else {
						if (cliOpts.verbose >= 2) {
						cout << "Buffer ";
					}
				}
				  
				if (cliOpts.verbose >= 2) {
					printf("%06d bytes\n", iEncodedBytes);
				}
				
				fprintf(pStatisticsFile, "%d,%d\n", iFramesDecoded, iEncodedBytes);
			} else {
				if (cliOpts.verbose >= 2) { cout << "Codec buffered frame" << "\n"; }
			}

			if (iEncodedBytes < 0) {
				AVERROR_LOOKUP(iEncodedBytes);
				fail("avcodec_encode_video()", "Encoder returned fault", FALSE);
			}
			
			if (cliOpts.verbose >= 2) {
				fflush(stdout);
			}  // flush stdout if we're pritning to it
			
			// Stop after cliOpts.maxframes frames
			if ((cliOpts.maxframes > 0) && (iFramesDecoded >= cliOpts.maxframes)) {
				break;
			}
			
			iFramesDecoded++;
		}
	}
	// Free the packetVidDec that was allocated by av_read_frame
	av_free_packet(&packetVidDec);
	}
	free(packetBufferVid);
	cout << "Child Thread Execution Haulted!" << endl;
}

// Not currently implimented.
/*void runAudioLoop()
{
	iAudioFramesDecoded = 0;
	
	// Decode until EOF

	CurrentPacket.data = inbufDec;
	
	while(av_read_frame(pFormatCtxAudDec, &CurrentPacket) >= 0)
	{
		if(CurrentPacket.stream_index == iAudioStream)
		{
			if (cliOpts.verbose >= 2)
			{
				cout << "Decoding audio frame..." << "\n";
			}

			iOutbufDecSize = AVCODEC_MAX_AUDIO_FRAME_SIZE;

			// Decode

			iDecodeAudio = avcodec_decode_audio3(pCodecCtxAudDec, (short *)outbufDec, &iOutbufDecSize, &CurrentPacket);

			if(iOutbufDecSize > 0)
			{
				if (cliOpts.verbose >= 2)
				{
					cout<< "Decode return: " << iDecodeAudio << "\n";
					cout << "outbufDec: " << (int)outbufDec[iAudioFramesDecoded] << "\n"; // How do we do this properly with cout?
					cout << "Encoding frame..." << "\n";
				}

				// Encode

				audio_encode_example(outbufDec, iOutbufDecSize);

				if (cliOpts.verbose >= 2)
				{
					cout << "Encode return: " << iEncodeAudio << "\n";
					cout << "outbufEnc: " << (int)outbufEnc[iAudioFramesDecoded] << "\n";
				}

				if (cliOpts.verbose >= 2)
				{
					if(iEncodeAudio>0)
					{
						if(cliOpts.networkport > 0)
						{   // if we requested networking, do it.
							cout << "Network\n";
						}

						if (cliOpts.saveaudio)
						{
							// if we want video saved to file
							// Write first iEncodedBytes of pOutBufferVid (buffer) to pOutputVideo (file handle)
							cout << "writing\n";
						}
						else
						{
							cout << "not writing\n";
						}
					}
				}
			}
		}
	
		CurrentPacket.size -= iDecodeAudio;
		CurrentPacket.data += iDecodeAudio;

		if (CurrentPacket.size < AUDIO_REFILL_THRESH)
		{
			memmove(inbufDec, CurrentPacket.data, CurrentPacket.size);
			CurrentPacket.data = inbufDec;
			// iDecodeAudio = fread(CurrentPacket.data + CurrentPacket.size, 1,AUDIO_INBUF_SIZE - CurrentPacket.size, f);
			if (iDecodeAudio > 0)
			{
				CurrentPacket.size += iDecodeAudio;
			}
		}
		iAudioFramesDecoded++;
	}
}*/

// Audio Encoder

void audio_encode_example(uint8_t *EncInpBuf, int iEncInpBufSize)
{
	// encode the samples

	int frameBytes = pCodecCtxAudEnc->frame_size * pCodecCtxAudEnc->channels * 2;

	while(iEncInpBufSize >= frameBytes) {
		iEncodeAudio = avcodec_encode_audio(pCodecCtxAudEnc, outbufEnc, iOutbufEncSize, (short *)EncInpBuf);
		
		if(iEncodeAudio > 0) {
			if(cliOpts.networkport > 0) {
				// if we requested networking, do it.
				addAudioFrame((char *)outbufEnc, iEncodeAudio, cliOpts.transport);
			}
			
			if(cliOpts.saveaudio ) {
				fwrite(outbufEnc, sizeof(uint8_t), iEncodeAudio, pOutputAudio);
			}
		}
		EncInpBuf += frameBytes;
		iEncInpBufSize -= frameBytes;
	}
}

// Save Frame to file
static void SaveFrame(AVFrame *aFrame, int width, int height, int iFrame)
{
	FILE *pFile;
	char szFilename[32];
	int  y;

	// Open file
	sprintf(szFilename, "frames/frame%04dB.ppm", iFrame);
	pFile=fopen(szFilename, "wb");
	if(pFile==NULL)
		fail("SaveFrame", "Couldn't open file to save frame.", FALSE);

	fprintf( pFile, "P6\n" );									//	PNM Type (Portable Pixel Map, Binary File)
	fprintf( pFile, "# George Smart, UNV Project, UCL 2010\n");	//	Comment
	fprintf( pFile, "%d %d\n", width, height);					//	Image size
	fprintf( pFile, "255\n" );	

	// Write pixel data
	for(y=0; y<height; y++) {
		fwrite(aFrame->data[0]+y*aFrame->linesize[0], 1, width*3, pFile);
	}
	
	// Close file
	fclose(pFile);
}

// Error handler...
//void fail(char *method_name, char *error_message, int terminate) {
void fail(const char* method_name, const char* error_message, int terminate)
{
	// print error message to stderr
	cerr << "Error in " << method_name << ": " << error_message << "\n";
	// kill if user requests
	if (terminate == TRUE)
	{
		cerr << "Terminating with EXIT_FAILURE " << "\n";
		exit(EXIT_FAILURE);
	}
}

int get_options (int c, char ** v) {
	int opt = 0;
	int error = FALSE;
	int longIndex = 0;
	
	/* Initialize cliOpts before we get to work. */
	cliOpts.verbose		= FALSE;
	cliOpts.formats		= FALSE;
	cliOpts.saveframes	= 0;
	cliOpts.maxframes	= 0;
	cliOpts.width		= 640;	// default
	cliOpts.height		= 480;	// default
	cliOpts.devicepath	= NULL;
	cliOpts.mode		= NULL;
	cliOpts.transport   = 4;	// not valid
	cliOpts.vcodec      = 4;	// not valid
	cliOpts.networkport = 0;	// not valid

	/* Process the arguments with getopt_long(), then populate cliOpts. */

	opt = getopt_long( c, v, optString, longOpts, &longIndex );
	
	while( opt != -1 ) {
		switch( opt ) {
			case 'v':	// verbose
				cliOpts.verbose++;
				break;
				
			case 's':	// save frames
				cliOpts.saveframes = atoi(optarg);
				break;
				
			case 'z':	// maximum  frames
				cliOpts.maxframes = atoi(optarg);
				break;
				
			case 'f':	// formats
				cliOpts.formats = TRUE;
				break;
				
			case 'r':	// resoution
				if ( strlen(optarg) <= 0 ) {error=TRUE; break;}
				cliOpts.width = atoi(strtok (optarg, "x"));
				cliOpts.height = atoi(strtok (NULL, "x"));
				break;
				
			case 'm':	// mode
				if ( strlen(optarg) <= 0 ) {error=TRUE; break;}
				cliOpts.mode = (char *) malloc(sizeof(char)*strlen(optarg));
				for (int i=0; i<(int)strlen(optarg)-1; i++) { cliOpts.mode[i] = toupper(optarg[i]); }
				break;
				
			case 't':	// transport layer (UDP/TCP)
				if ( strlen(optarg) <= 0 ) {error=TRUE; break;}
				if( (strcmp( "UDP",  optarg) == 0) || (strcmp( "udp",  optarg) == 0) ) {
					cliOpts.transport = USE_UDP;
				} else {
					cliOpts.transport = USE_TCP;
				}
				break;
				
			case 'c':	// cvideo codec (MJPEG/H264)
				if ( strlen(optarg) <= 0 ) {error=TRUE; break;}
				if( (strcmp( "MJPEG",  optarg) == 0) || (strcmp( "mjpeg",  optarg) == 0) ) {
					cliOpts.vcodec = USE_MJPEG;
				} else {
					cliOpts.vcodec = USE_H264;
				}
				break;
				
			case 'p':	// Network Port
				if ( strlen(optarg) <= 0 ) {error=TRUE; break;}
				cliOpts.networkport = atoi(optarg);
				break;
			
			case 'd':	// device
				if ( strlen(optarg) <= 0 ) {error=TRUE; break;}
				cliOpts.devicepath = optarg;
				break;
			
			case 'o':	// output video path
				if ( strlen(optarg) <= 0 ) {error=TRUE; break;}
				cliOpts.videopath = optarg;
				cliOpts.savevideo = TRUE;
				break;
				
			case 'a':   // output audio path
				if ( strlen(optarg) <= 0 ) {error=TRUE; break;}
				cliOpts.audiopath = optarg;
				cliOpts.saveaudio = TRUE;
				break;		
							
			case 'h':	// help, with fallthrough.
			case '?':
				usage(v[0]);
				break;
				
			case 0:		// long option without a short arg
				// version?
				if( strcmp( "version", longOpts[longIndex].name ) == 0 ) {
					cliOpts.version = TRUE;
					break;
				}
				
				if( strcmp( "DEFAULT", longOpts[longIndex].name ) == 0 ) 
				{
					cout << "***DEVELOPMENT: SETTING TEST DEFAULT OPTIONS***" << endl;
					cliOpts.verbose = 2;
					cliOpts.saveframes = 10;
					cliOpts.maxframes = 100;
					cliOpts.mode = (char*) "V4L";
					cliOpts.devicepath = (char*) "/dev/video0";
					cliOpts.videopath = (char*) "test_out.mpg";
					cliOpts.audiopath = (char*) "audio.mp3" ;
					cliOpts.savevideo = TRUE;
					cliOpts.saveaudio = TRUE;
					cliOpts.width = 640;
					cliOpts.height = 480;
					cliOpts.networkport = 5000;
					break;
				}
				
			default:
				// Never happens, but just to be sure...
				fail("get_options", "hit impossible DEFAULT case ??", TRUE);
				break;
		}
		opt = getopt_long( c, v, optString, longOpts, &longIndex );
	}
	
	// If verbose mode, tell the user what we've deduced from their input.
	if (cliOpts.verbose) {
		printf("Command Line Options: (1=TRUE, 0=FALSE)\n");
		printf("  Be Verbose:     %d\n", cliOpts.verbose);
		printf("  Show Version:   %d\n", cliOpts.version);
		printf("  Show Formats:   %d\n", cliOpts.formats);
		printf("  SaveFrames:     %d\n", cliOpts.saveframes);
		printf("  MaxFrames:      %d\n", cliOpts.maxframes);
		printf("  Width:          %d\n", cliOpts.width);
		printf("  Height:         %d\n", cliOpts.height);
		printf("  Device:         %s\n", cliOpts.devicepath);
		printf("  Video Output:   %d (%s)\n", cliOpts.savevideo, cliOpts.videopath);
		printf("  Audio Output:   %d (%s)\n", cliOpts.saveaudio, cliOpts.audiopath);
		printf("  Network Port:   %d\n", cliOpts.networkport);
		printf("  Transport:      %d (0=TCP, 1=UDP)\n", cliOpts.transport);
		printf("  Video Codec:    %d (2=H264, 3=MJPEG)\n", cliOpts.vcodec);
		printf("  Mode:           %s\n", cliOpts.mode);
		printf("\n");
	}
	return error;
}

void usage (char prog[]) {
	cout << "Usage:" << "\t" << prog << "[options]" << "\n";
	cout << "Options:" << "\n";
	cout << "\t" << "-h" << "\t" << "--help" << "\n\t\t" << "Prints this message" << "\n";
	cout << "\t" << "-v" << "\t" << "--verbose" << "\n\t\t" << "Enable verbose output" << "\n";
	cout << "\t" << "(none)" << "\t" << "--version" << "\n\t\t" << "Print version information" << "\n";
	cout << "\t" << "-f" << "\t" << "--formats" << "\n\t\t" << "Show supported codecs and drivers" << "\n";
	cout << "\t" << "-s <n>" << "\t" << "--save <n>" << "\n\t\t" << "Save every <n>-th frame to disk" << "\n";
	cout << "\t" << "-z <n>" << "\t" << "--maxframes <n>" << "\n\t\t" << "Stop after <n> frames" << "\n";
	cout << "\t" << "-r <widthxheight>" << "\t" << "--resolution <widthxheight>" << "\n\t\t" << "Resolution of Webcam (V4L2 mode only)" << "\n";
	cout << "\t" << "-m <mode>" << "\t" << "--mode <mode>" << "\n\t\t" << "Where <mode> is either:" << "\n\t\t" << "  FILE" << "\t" << "Read file specified with -d option as video source" << "\n\t\t "<< " V4L2"<< "\t" << "Open capture device specified by -d option (eg Webcam)" << "\n";
	cout << "\t" << "-d <input path>" << "\t" << "--device <input path>" << "\n\t\t" << "Path to device (V4L2 mode) or file (FILE mode)" << "\n";
	cout << "\t" << "-o <video path>" << "\t" << "--video <input path>" << "\n\t\t" << "Path of video to be output" << "\n";
	cout << "\t" << "-a <audio path>" << "\t" << "--audio <input path>" << "\n\t\t" << "Path of audio to be output" << "\n";
	cout << "\t" << "-p <port>" << "\t" << "--port <port>" << "\n\t\t" << "Where <port> is TCP port number" << "\n";

	exit(EXIT_FAILURE);
}

void show_format (AVInputFormat * InFmt) {
	cout << "Supported Formats: " << "\n";
	while ((InFmt = av_iformat_next(InFmt))) {
		cout << "  " << InFmt->name << "\t" << "\t" << InFmt->long_name << ":" << "\n";
	}
}

void show_version (void) {
	cout << "Version " << VERSIONNUM << "(" << VERSIONTXT << ")" << "\n";
}

void signal_handler (int sig) {
	cout << "Signal Handler: Setting termination condition" << endl;
	StopExecution = true;
	signal(SIGINT, SIG_DFL);
}
