//      gs.c	Source File
//      
//      Copyright 2010 George Smart, M1GEO <george.smart@ucl.ac.uk>
//
//		Written for the UNV Project, E&EE, University College London.
//      
//      This program is free software; you can redistribute it and/or modify
//      it under the terms of the GNU General Public License as published by
//      the Free Software Foundation; either version 2 of the License, or
//      (at your option) any later version.
//      
//      This program is distributed in the hope that it will be useful,
//      but WITHOUT ANY WARRANTY; without even the implied warranty of
//      MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.  See the
//      GNU General Public License for more details.
//      
//      You should have received a copy of the GNU General Public License
//      along with this program; if not, write to the Free Software
//      Foundation, Inc., 51 Franklin Street, Fifth Floor, Boston,
//      MA 02110-1301, USA.
//
//		Based on
//			http://www.inb.uni-luebeck.de/~boehme/using_libavcodec.html
//			http://www.inb.uni-luebeck.de/~boehme/libavcodec_update.html
//			http://www.ibm.com/developerworks/aix/library/au-unix-getopt.html
//
//		Libraries (all source files)
//			avformat 
//			avcodec
//			avutil 
//			avdevice 
//			swscale
//

#define	VERSIONNUM "0.0.5"
#define	VERSIONTXT	"Second Attempt + TCP Networking"
#include <fstream>

//  Get Project Headers.
#include "Server.h"

const char *filename = "test.mkv";

//Include the Networking libraries and functions - Include last
#include "rtspServer.h"
#include "gcs.h"

// Networking stuffs :)
//#include "gs_network.c"

int main(int argc, char * argv[]) {
	
	AVFormatContext 			*pFormatCtxVidDec;
	AVFormatParameters			FormatParamVidDec;
	AVInputFormat				*pInputFmtVidDec = NULL;
	AVCodecContext  			*pCodecCtxVidDec;
	AVCodecContext  			*pCodecCtxVidEnc;
	static AVPacket 			packetVidDec;
	AVFrame						*pFrameDec;		// YUYV422
	AVFrame						*pFrameEnc;		// YUV420P
	AVFrame						*pFrameRGB;		// RGB24
	AVCodec						*pCodecVidDec;
	AVCodec						*pCodecVidEnc;

	static struct SwsContext	*swsC_RGB;
	static struct SwsContext	*swsC_YUV;
	
	FILE						*pOutputVideo;
	FILE						*pStatisticsFile;
	
	uint8_t						*pRGBBufferVid;
	uint8_t						*pOutBufferVid;
	uint8_t 					*pYUVBufferVid;
	
	long int					iFramesDecoded;
	int							iRGBFrameSize;
	int							iYUVFrameSize;
	int							iFrameFinished;
	int 						iVideoStream;
	int							iOutBufferSize;
	int							iEncodedBytes;
	
	int 						swsR_RGB;
	int 						swsR_YUV;
	int 						temp;
	int             			i;
	
	// Setup Signal Catcher callback...
	signal(SIGINT, signal_handler);		// on signal, we call signal_handler with the sig type as an integer arg
	
	printf("*** FFMPEG Experiment Testbed ***\n");
	printf("UNV Project, E&EE, UCL.\n");
	printf("Written by George Smart (http://www.george-smart.co.uk/)\n\n");
	
	// Read CLI Options, and error if not vaild
	if ( (argc<=1)||(get_options(argc, argv)) ) {
		fail("parsing CLI options", "couldn't understand options. Run with --help.", FALSE);
		exit(EXIT_SUCCESS);
	}
	
	// register all codecs, drivers, devics, etc so we know what we have.
	if (cliOpts.verbose) {printf("Registering Codecs and Device drivers\n");} // Verbose
	av_register_all();
	avdevice_register_all();

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
	
	// Program actually starts here - before was just checking stuffs /////////////////////////////////////////////////////////////////////////////////////
	
	// Allocate codec context
	if (cliOpts.verbose) {printf("Allocating memory for codec contexts\n");}
	pCodecCtxVidEnc = avcodec_alloc_context();
	pCodecCtxVidDec = avcodec_alloc_context();
	if ( (pCodecCtxVidEnc == NULL) || (pCodecCtxVidDec == NULL) ) {
		fail("allocating frames", "couldn't allocate either pFrame of pFrameRGB", TRUE);
	}

	// Allocate Frames
	if (cliOpts.verbose) {printf("Allocating memory for frames\n");}
	pFrameDec = avcodec_alloc_frame();
	pFrameEnc = avcodec_alloc_frame();
	pFrameRGB = avcodec_alloc_frame();
	if ( (pFrameDec == NULL) || (pFrameEnc == NULL) || (pFrameRGB == NULL) ) {
		fail("allocating frames", "couldn't allocate either pFrame of pFrameRGB", TRUE);
	}

	// Get options for input type
	if ((strcmp(cliOpts.mode, "FILE")==0)||(strcmp(cliOpts.mode, "FIL")==0)) {
		// File
		if (cliOpts.verbose) {printf("Opening '%s' as video file\n", cliOpts.devicepath);}	// Verbose
		temp = av_open_input_file(&pFormatCtxVidDec, cliOpts.devicepath, NULL, 0, NULL);
		if(temp !=0) {
			AVERROR_LOOKUP(temp);
			fail("opening video", "couldn't open input file", TRUE);
		}
	} else if ((strcmp(cliOpts.mode, "V4L2")==0)||(strcmp(cliOpts.mode, "V4L")==0)) {
		// Video4Linux2
		if (cliOpts.verbose) {printf("Opening '%s' as Video4Linux2 device\n", cliOpts.devicepath);} // Verbose
		FormatParamVidDec.channel = 0;
		FormatParamVidDec.standard = "pal"; // pal (or ntsc)
		FormatParamVidDec.width = cliOpts.width;	// read from CLI
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
	} else if ((strcmp(cliOpts.mode, "MKV")==0)||(strcmp(cliOpts.mode, "MK")==0)) {
/*
 * 
 *  PLACE HOLDER FOR MKV STUFFS
 * 
 * 
 * 
 */
	} else {
		fprintf(stderr, "Did not recognise mode '%s'\n", cliOpts.mode);
		fail("selecting mode", "couldn't understand user's mode input.", TRUE);
	}

	// Retrieve stream information
	if (cliOpts.verbose) {printf("Analysing video stream(s)\n");}
	temp = av_find_stream_info(pFormatCtxVidDec);
	if(temp<0) {
		AVERROR_LOOKUP(temp);
		fail("finding stream", "couldn't find stream infomation", TRUE);
	}
	
	// dump what we've found to stderr.
	if (cliOpts.verbose) {fprintf(stderr, "Format Dump\n");
		dump_format(pFormatCtxVidDec, 0, cliOpts.devicepath, FALSE);
	}
	
	// Find the first video stream
	if (cliOpts.verbose) {printf("Selecting first video stream\n");}
	iVideoStream=-1;
	for(i=0; i<(int)pFormatCtxVidDec->nb_streams; i++) {
		if(pFormatCtxVidDec->streams[i]->codec->codec_type==CODEC_TYPE_VIDEO) { //modified codec.codec... to union in structure - GS
			iVideoStream=i;
			break;
		}
	}
	
	if(iVideoStream == -1) {
		fail("selecting stream", "couldn't select first stream", TRUE);
	}
	
	// Get a pointer to the codec context for the video stream
	pCodecCtxVidDec=pFormatCtxVidDec->streams[iVideoStream]->codec; // removed address operator - GS

	// Find the decoder for the video stream
	pCodecVidDec=avcodec_find_decoder(pCodecCtxVidDec->codec_id);
	if(pCodecVidDec==NULL) {
		fail("finding decoder", "couldn't find required codec", TRUE);
	}

	// Find the codec for our required format
    //pCodecVidEnc = avcodec_find_encoder(CODEC_ID_H264);
    //pCodecVidEnc = avcodec_find_encoder(CODEC_ID_MPEG4);
    pCodecVidEnc = avcodec_find_encoder(CODEC_ID_MPEG2VIDEO);
    if (pCodecVidEnc == NULL) {
        fail("finding encoder", "couldn't find required codec", TRUE);
    }
	
	// *** MAY NEED A AVFormatContext HERE SOMEWHERE ***
	
	// Default from some random internet code, but well generic params.
	
	// Set codec context (these are for MPEG1/2)
	avcodec_get_context_defaults(pCodecCtxVidEnc);
	pCodecCtxVidEnc->codec_type = CODEC_TYPE_VIDEO;
	pCodecCtxVidEnc->codec_id = CODEC_ID_MPEG2VIDEO;
	
    pCodecCtxVidEnc->bit_rate = 400000;
    pCodecCtxVidEnc->width = pCodecCtxVidDec->width;
    pCodecCtxVidEnc->height = pCodecCtxVidDec->height;
    pCodecCtxVidEnc->time_base= (AVRational){1,25};
    pCodecCtxVidEnc->gop_size = pCodecCtxVidEnc->time_base.den * 1; // every two seconds
    pCodecCtxVidEnc->max_b_frames=0;
    pCodecCtxVidEnc->pix_fmt = PIX_FMT_YUV420P;
    
    /*
    // These are for H.264 (libx264) - need optimising by Lorenzo.
	float bitRate = 164000.f;
	avcodec_get_context_defaults(pCodecCtxVidEnc);
	pCodecCtxVidEnc->pix_fmt = PIX_FMT_YUV420P;
	pCodecCtxVidEnc->width = pCodecCtxVidDec->width;
	pCodecCtxVidEnc->height = pCodecCtxVidDec->height;
	pCodecCtxVidEnc->time_base = (AVRational){1,15};
	
	pCodecCtxVidEnc->rc_lookahead = 0;
	pCodecCtxVidEnc->refs = 2;
	pCodecCtxVidEnc->scenechange_threshold = 0;
	pCodecCtxVidEnc->me_subpel_quality = 0;
	pCodecCtxVidEnc->partitions = X264_PART_I4X4 | X264_PART_I8X8 | X264_PART_P8X8 | X264_PART_B8X8;
	pCodecCtxVidEnc->me_method = ME_EPZS;
	pCodecCtxVidEnc->trellis = 0;
	
	pCodecCtxVidEnc->me_range = 16;
	pCodecCtxVidEnc->max_qdiff = 4;
	//pCodecCtxVidEnc->mb_qmin = 10;
	pCodecCtxVidEnc->qmin = 10;
	//pCodecCtxVidEnc->mb_qmax = 51;
	pCodecCtxVidEnc->qmax = 51;
	pCodecCtxVidEnc->qcompress = 0.6f;
	pCodecCtxVidEnc->mb_decision = FF_MB_DECISION_SIMPLE;
	pCodecCtxVidEnc->flags2 |= CODEC_FLAG2_FASTPSKIP;
	pCodecCtxVidEnc->flags |= CODEC_FLAG_LOOP_FILTER;
	pCodecCtxVidEnc->flags |= CODEC_FLAG_GLOBAL_HEADER;
	pCodecCtxVidEnc->max_b_frames = 0;
	pCodecCtxVidEnc->b_frame_strategy = 1;
	pCodecCtxVidEnc->chromaoffset = 0;
	
	pCodecCtxVidEnc->thread_count = 1;
	//pCodecCtxVidEnc->rtp_payload_size = H264_RTP_PAYLOAD_SIZE;
	//pCodecCtxVidEnc->opaque = tsk_null;
	pCodecCtxVidEnc->bit_rate = (int) (bitRate * 0.80f);
	pCodecCtxVidEnc->bit_rate_tolerance = (int) (bitRate * 0.20f);
	pCodecCtxVidEnc->gop_size = pCodecCtxVidEnc->time_base.den * 2; // Each 2 seconds
	*/
	
	// Open codec
	if (cliOpts.verbose) {printf("Opening codec (decoder)\n");}
	temp = avcodec_open(pCodecCtxVidDec, pCodecVidDec);
	if(temp<0) {
		AVERROR_LOOKUP(temp);
		fail("opening codec", "couldn't open decoder codec", TRUE);
	}

	// Open the codec
	if (cliOpts.verbose) {printf("Opening codec (encoder)\n");}
    temp = avcodec_open(pCodecCtxVidEnc, pCodecVidEnc);
	if(temp < 0) {
		AVERROR_LOOKUP(temp);
		fail("opening codec", "couldn't open encoder codec", TRUE);
	}

	// Allocate YUV and RGB memory.
	if (cliOpts.verbose) {printf("Allocating format conversion colourspace\n");}
	iRGBFrameSize=avpicture_get_size(PIX_FMT_RGB24, pCodecCtxVidDec->width, pCodecCtxVidDec->height);
    iYUVFrameSize = avpicture_get_size(PIX_FMT_YUV420P, pCodecCtxVidEnc->width, pCodecCtxVidEnc->height);
    iOutBufferSize = 100000;
    if (cliOpts.verbose) {printf("  RGB: %d Bytes\n  YUV: %d Bytes\n  OUT: %d Bytes\n", iRGBFrameSize, iYUVFrameSize, iOutBufferSize);}
	pYUVBufferVid = (uint8_t *) malloc((iYUVFrameSize));
	pRGBBufferVid = (uint8_t *) malloc(iRGBFrameSize);
    if ( (pRGBBufferVid == NULL) || (pYUVBufferVid == NULL) ) {	// check we got the buffer we wanted.
		fail("malloc buffers", "couldn't allocate memory for pFrameRGB or pFrameEnc buffers", TRUE);
	}
	
	if (cliOpts.verbose) {printf("Assigning RGB and YUV buffers to AVFrames.\n");}
    // Assign appropriate parts of buffer to image planes in pFrameRGB and pFrameYUV
    avpicture_fill((AVPicture *)pFrameRGB, pRGBBufferVid, PIX_FMT_RGB24, pCodecCtxVidDec->width, pCodecCtxVidDec->height);  // RGB size
    avpicture_fill((AVPicture *)pFrameEnc, pYUVBufferVid, PIX_FMT_YUV420P, pCodecCtxVidDec->width, pCodecCtxVidDec->height);  // YUV size
	pOutBufferVid = (uint8_t *) av_malloc(iOutBufferSize * sizeof(uint8_t));  // alloc output buffer for encoded datastream
    
    // if we want to save video
    if (cliOpts.savevideo) {
		// Open output file handle
		if (cliOpts.verbose) {printf("Opening output video file\n");}
		pOutputVideo = fopen(cliOpts.videopath, "wb"); // Write Binary
		if (pOutputVideo == NULL) {
			fprintf(stderr, "Could not open %s\n", cliOpts.videopath);
			fail("opening file handle", "couldn't open file", TRUE);
		}    
    }
    
    // Stats file
    pStatisticsFile = fopen("stats.csv", "w");
    
    if(cliOpts.networkport > 0) {	// if the port is greater than 0, we requested it.
		//gs_network_init(cliOpts.networkport); // open the network with the right port.
		//This function initiates and calls the network components
		//Usage: (Queue size when reset, time between frames in miliseconds, TCP Port, UDP Port)
		startServerRTSP(20, 30015, 3015);
	}

	if (cliOpts.verbose) {printf("Entering main processing loop\nIf you would like to have this debugged, run with two verbose options.\nThis will slow the program a lot!\n");}
	iFramesDecoded = 0;
	
	while(av_read_frame(pFormatCtxVidDec, &packetVidDec)>=0) {
		// Is this a packetVidDec from the video stream?
		if(packetVidDec.stream_index==iVideoStream) {
			// Decode video frame
			if (cliOpts.verbose >= 2) {printf("Decode new packetVidDec\n");}
			avcodec_decode_video2(pCodecCtxVidDec, pFrameDec, &iFrameFinished, &packetVidDec);
			// Did we get a video frame?
			if(iFrameFinished) {
				if (cliOpts.verbose >= 2) {printf("Got a new video frame: %ld\n", iFramesDecoded);}
				
				// Convert the image from its native format to RGB to save as ppm
				if (swsC_RGB == NULL) {
					if (cliOpts.verbose >= 2) {
						fprintf(stderr, "Setting up conversion context for RGB:\n  Input type:  %dx%d in ", pCodecCtxVidDec->width, pCodecCtxVidDec->height); PIX_FMT_LOOKUP(pCodecCtxVidDec->pix_fmt); fprintf(stderr, " format\n");
						fprintf(stderr, "  Output type: %dx%d in ", pCodecCtxVidEnc->width, pCodecCtxVidEnc->height); PIX_FMT_LOOKUP(PIX_FMT_RGB24); fprintf(stderr, " format\n");
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
				
				if (swsC_RGB == NULL) {
					fail("sws_getContext RGB", "Couldn't setup a format conversion.", TRUE);
				}
				if (cliOpts.verbose >= 2) {printf("Calling sws_scale for colourspace conversion RGB\n");}
				swsR_RGB = sws_scale( swsC_RGB,									//	SwsContext - Setup for scaling
									(const uint8_t* const*)	pFrameDec->data,	//	Source frame data (cast to required type)
									pFrameDec->linesize,						//	Source frame stride
									0,										//	Source frame slice width (raster, so no width slicing)
									pCodecCtxVidDec->height,						//	Source frame slice height
									pFrameRGB->data,						//	Dest frame data
									pFrameRGB->linesize);					//	Dest frame stride

				if (swsR_RGB <= 0) {
					fail("sws_scale RGB", "Couldn't determine output slice size.", TRUE);
				}
				
				
				// Convert the image from its native format to YUV420P for encoder
				if (swsC_YUV == NULL) {
					if (cliOpts.verbose >= 2) {
						fprintf(stderr, "Setting up conversion context for YUV:\n  Input type:  %dx%d in ", pCodecCtxVidDec->width, pCodecCtxVidDec->height); PIX_FMT_LOOKUP(pCodecCtxVidDec->pix_fmt); fprintf(stderr, " format\n");
						fprintf(stderr, "  Output type: %dx%d in ", pCodecCtxVidEnc->width, pCodecCtxVidEnc->height); PIX_FMT_LOOKUP(pCodecCtxVidEnc->pix_fmt); fprintf(stderr, " format\n");
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
				
				if (swsC_YUV == NULL) {
					fail("sws_getContext YUV", "Couldn't setup a format conversion.", TRUE);
				}
				if (cliOpts.verbose >= 2) {printf("Calling sws_scale for colourspace conversion YUV\n");}
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
					if (cliOpts.verbose >= 2) {printf("Saving this frame, as requested.\n");}
					SaveFrame(pFrameRGB, pCodecCtxVidDec->width, pCodecCtxVidDec->height, iFramesDecoded);
				}
				
				// Encode the video
				if (cliOpts.verbose >= 2) {printf("Calling avcodec_encode_video(): ");}
				iEncodedBytes = avcodec_encode_video(pCodecCtxVidEnc, pOutBufferVid, iOutBufferSize, pFrameEnc);
				
				if (iEncodedBytes > 0) {
					if(cliOpts.networkport > 0) {	// if we requested networking, do it.
						addFrame((char *)pOutBufferVid, iEncodedBytes); //OBADA ADD
						//Sleep so that file is not read all to memory instantly
						if ((strcmp(cliOpts.mode, "FILE")==0)||(strcmp(cliOpts.mode, "FIL")==0)) {
							usleep(33000); //Sleep for 33 miliseconds
						}
						if (cliOpts.verbose >= 2) {printf("Network ");}
					}	
					if (cliOpts.savevideo) { // if we want video saved to file
						// Write first iEncodedBytes of pOutBufferVid (buffer) to pOutputVideo (file handle)
						fwrite(pOutBufferVid, sizeof(uint8_t), iEncodedBytes, pOutputVideo);
						if (cliOpts.verbose >= 2) {printf("File ");}
					} else {
						if (cliOpts.verbose >= 2) {printf("Buffer ");}
					}
					if (cliOpts.verbose >= 2) {printf("%06d bytes\n", iEncodedBytes);}
					fprintf(pStatisticsFile, "%ld,%d\n", iFramesDecoded, iEncodedBytes);
				} else {
					if (cliOpts.verbose >= 2) {printf("Codec buffered frame\n");}
				}
				if (iEncodedBytes < 0) {
					AVERROR_LOOKUP(iEncodedBytes);
					fail("avcodec_encode_video()", "Encoder returned fault", FALSE);
				}
				
				if (cliOpts.verbose >= 2) {fflush(stdout);}  // flush stdout if we're pritning to it
				
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
	
	// don't bother flushing the codec - we dont care for frames after the user quits...
	
	//  THIS IS FOR MPEG 1 STREAMS ONLY BE CAREFULL!
	// If we're writing to a file, then end it nicely
	if (cliOpts.savevideo) {
		// add sequence end code to have a real mpeg file
		pOutBufferVid[0] = 0x00;
		pOutBufferVid[1] = 0x00;
		pOutBufferVid[2] = 0x01;
		pOutBufferVid[3] = 0xb7;
		fwrite(pOutBufferVid, 1, 4, pOutputVideo);
	}
	
	
	// report back to the user how many frames were processed.
	printf("%ld frames parsed\n", iFramesDecoded);
	
dontDie(); // OBADA ADD

	if(cliOpts.networkport > 0) {  // if networking was opened
		//gs_network_close(); // close the network
	}
	
	if (cliOpts.verbose) {printf("Freeing memory\n");}
	// Free frame memory
	av_free(pFrameDec);
	av_free(pFrameEnc);
	av_free(pFrameRGB);
	free(pOutBufferVid);
	free(pRGBBufferVid);
	free(pYUVBufferVid);
    
	// Close the codecs
	avcodec_close(pCodecCtxVidDec);
	avcodec_close(pCodecCtxVidEnc);
	
	// Close video file
	av_close_input_file(pFormatCtxVidDec);
	
	// Close the video output file handle IF it was opened
	if (cliOpts.savevideo) {
		fclose(pOutputVideo);
	}
	
	// close stats file
	fclose(pStatisticsFile);
	
	if (cliOpts.verbose) {printf("Exiting...\n");}
	exit(EXIT_SUCCESS);
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
void fail(const char* method_name, const char* error_message, int terminate) {
	// print error message to stderr
	fprintf(stderr, "Error in %s: %s\n", method_name, error_message);
	// kill if user requests
	if (terminate == TRUE) {
		fprintf(stderr, "Terminating with EXIT_FAILURE\n");
		exit(EXIT_FAILURE);
	}
}

int get_options (int c, char ** v) {
	int i = 0;
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
	cliOpts.networkport = 0;	// not valid

	/* Process the arguments with getopt_long(), then 
	 * populate cliOpts. 
	 */
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
				for (i=0; i<(int)strlen(optarg)-1; i++) {
					cliOpts.mode[i] = toupper(optarg[i]);
				}
				cliOpts.mode[i] = '\0';
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
				
				if( strcmp( "DEFAULT", longOpts[longIndex].name ) == 0 ) {
					printf("***DEVELOPMENT: SETTING TEST DEFAULT OPTIONS***\n");
					cliOpts.verbose = 2;
					cliOpts.saveframes = 10;
					cliOpts.maxframes = 100;
					cliOpts.mode = (char*)"V4L2";
					cliOpts.devicepath = (char*)"/dev/video0";
					cliOpts.videopath = (char*)"test_out.mpg";
					cliOpts.savevideo = TRUE;
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
		printf("  Network Port:   %d\n", cliOpts.networkport);
		printf("  Mode:           %s\n", cliOpts.mode);
		printf("\n");
	}
	return error;
}

void usage (char prog[]) {
	printf("Usage:\t%s [options]\n", prog);
	printf("Options:\n");
	printf("\t-h\t--help\n\t\tPrints this message\n");
	printf("\t-v\t--verbose\n\t\tEnable verbose output\n");
	printf("\t(none)\t--version\n\t\tPrint version information\n");
	printf("\t-f\t--formats\n\t\tShow supported codecs and drivers\n");
	printf("\t-s <n>\t--save <n>\n\t\tSave every <n>-th frame to disk\n");
	printf("\t-z <n>\t--maxframes <n>\n\t\tStop after <n> frames\n");
	printf("\t-r <widthxheight>\t--resolution <widthxheight>\n\t\tResolution of Webcam (V4L2 mode only)\n");
	printf("\t-m <mode>\t--mode <mode>\n\t\tWhere <mode> is either:\n\t\t  FILE\tRead file specified with -d option as video source\n\t\t  V4L2\tOpen capture device specified by -d option (eg Webcam)\n");
	printf("\t-d <input path>\t--device <input path>\n\t\tPath to device (V4L2 mode) or file (FILE mode)\n");
	printf("\t-o <video path>\t--video <input path>\n\t\tPath of video to be output\n");
	printf("\t-p <port>\t--port <port>\n\t\tWhere <port> is TCP port number\n");
	
	exit(EXIT_FAILURE);
}

void show_format (AVInputFormat * InFmt) {
		printf("Supported Formats:\n");
	    while ((InFmt = av_iformat_next(InFmt))) {
			printf("  %s\t\t%s:\n", InFmt->name, InFmt->long_name); 
		}
}

void show_version (void) {
		printf("Version %s (%s)\n", VERSIONNUM, VERSIONTXT);
}

void signal_handler (int sig) {
	fprintf(stderr, "\nCaught %d in signal_handler:\nExiting horribly because I haven't designed anything better\n", sig);
	signal(SIGINT, SIG_DFL);
	exit(EXIT_SUCCESS);
}

