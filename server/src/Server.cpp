//      gs.c	Source File
//      
//      Copyright 2010 George Smart, M1GEO <george.smart@ucl.ac.uk>
//		Written for the UNV Project, E&EE, University College London.
//
//		You can do what you like with this software, providing the original
//		copyright message from above remains at all times.
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
#include "util.h"
#include "video.h"
#include "audio.h"
#include "mpeg.h"

// Networking stuffs :)
//#include "gs_network.c"

int main(int argc, char * argv[]) {
	

	
	// Setup Signal Catcher callback...
	signal(SIGINT, signal_handler);		// on signal, we call signal_handler with the sig type as an integer arg
	
	printf("UNV Project, E&EE, UCL.\n");
	printf("Written by George Smart (http://www.george-smart.co.uk/)\n\n");

	av_register_all();
	avdevice_register_all();

	open_webcam();
	mpeg_init_all(pWebcamCodecContext->height, pWebcamCodecContext->width, pWebcamCodecContext->pix_fmt);
	
	for(;;) {
		pOutBufferVid =  encode_frame_to_mpeg(get_webcam_frame());
		addFrame((char *)pOutBufferVid, iEncodedBytes);
	}
	
	printf("Exiting at program end\n");
	exit(EXIT_SUCCESS);
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

