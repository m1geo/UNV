//
//      (C) 30/Apr/2011 - George Smart, M1GEO <george.smart@ucl.ac.uk>
//		The UNV Project, Electronic Enginering, University College London
//

#include <libavcodec/avcodec.h>
#include <libavformat/avformat.h>
#include <libavdevice/avdevice.h> 
#include <libswscale/swscale.h>

#include <getopt.h>

#define FALSE 0
#define TRUE !FALSE

const char *filename = "test.mkv";

/* audio output */
float t, tincr, tincr2;
int16_t *samples;
uint8_t *audio_outbuf;
int audio_outbuf_size;
int audio_input_frame_size;

/* video output */
AVFrame *picture, *tmp_picture;
uint8_t *video_outbuf;
int frame_count, video_outbuf_size;

/* Encoder stuffs */
int							iYUVFrameSize;
int							iOutBufferSize;
uint8_t 					*pYUVBufferVid;
static struct SwsContext	*swsC_YUV;
int 						swsR_YUV;
int 						iPktSize;
struct cliOpts_t 			cliOpts;





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

AVFrame *alloc_picture(enum PixelFormat pix_fmt, int width, int height)
{
    AVFrame *picture;
    uint8_t *picture_buf;
    int size;

    picture = avcodec_alloc_frame();
    if (!picture)
        return NULL;
    size = avpicture_get_size(pix_fmt, width, height);
    picture_buf = (uint8_t*)av_malloc(size);
    if (!picture_buf) {
        av_free(picture);
        return NULL;
    }
    avpicture_fill((AVPicture *)picture, picture_buf, pix_fmt, width, height);
    return picture;
}

// Lookup AV Error Codes
char * AVERROR_LOOKUP(int EN) {
	switch (EN) {
		case AVERROR_IO: /**< I/O error */
			return("AVERROR_IO: I/O Error\n");
			
		case AVERROR_NUMEXPECTED: /**< Number syntax expected in filename. */
			return("AVERROR_NUMEXPECTED: Number Syntax Expected in Filename\n");
			
		case AVERROR_INVALIDDATA: /**< invalid data found */
			return("AVERROR_INVALIDDATA: Invalid Data Found\n");
			
		case AVERROR_NOMEM: /**< not enough memory */
			return("AVERROR_NOMEM: Not Enough Memory\n");
			
		case AVERROR_NOFMT: /**< unknown format */
			return("AVERROR_NOFMT: Unknown Format\n");
			
		case AVERROR_NOTSUPP: /**< Operation not supported. */
			return("AVERROR_NOTSUPP: Operation Not Supported\n");
			
		case AVERROR_NOENT: /**< No such file or directory. */
			return("AVERROR_NOENT: No Such File or Directory\n");
			
		default:
			return("Unknown Error: Not Recognised Error Number\n");
			
	}
}

// Lookup PixelFormats
char * PIX_FMT_LOOKUP(int N) {
	switch (N) {
		case PIX_FMT_NONE:
			return("PIX_FMT_NONE");
			
		case PIX_FMT_YUV420P:
			return("PIX_FMT_YUV420P");
			
		case PIX_FMT_YUYV422:
			return("PIX_FMT_YUYV422");
			
		case PIX_FMT_RGB24:
			return("PIX_FMT_RGB24");
			
		case PIX_FMT_BGR24:
			return("PIX_FMT_BGR24");
			
		case PIX_FMT_YUV422P:
			return("PIX_FMT_YUV422P");
			
		case PIX_FMT_YUV444P:
			return("PIX_FMT_YUV444P");
			
		case PIX_FMT_YUV410P:
			return("PIX_FMT_YUV410P");
			
		case PIX_FMT_YUV411P:
			return("PIX_FMT_YUV411P");
			
		case PIX_FMT_GRAY8:
			return("PIX_FMT_GRAY8");
			
		case PIX_FMT_MONOWHITE:
			return("PIX_FMT_MONOWHITE");
			
		case PIX_FMT_MONOBLACK:
			return("PIX_FMT_MONOBLACK");
			
		case PIX_FMT_RGB32:
			return("PIX_FMT_RGB32");
			
		case PIX_FMT_XVMC_MPEG2_MC:
			return("PIX_FMT_XVMC_MPEG2_MC");
			
		case PIX_FMT_XVMC_MPEG2_IDCT:
			return("PIX_FMT_XVMC_MPEG2_IDCT");
			
		case PIX_FMT_UYVY422:
			return("PIX_FMT_UYVY422");
			
		case PIX_FMT_UYYVYY411:
			return("PIX_FMT_UYYVYY411");
			
		case PIX_FMT_BGR8:
			return("PIX_FMT_BGR8");
			
		case PIX_FMT_BGR4:
			return("PIX_FMT_BGR4");
			
		case PIX_FMT_BGR4_BYTE:
			return("PIX_FMT_BGR4_BYTE");
			
		case PIX_FMT_RGB8:
			return("PIX_FMT_RGB8");
			
		case PIX_FMT_RGB4:
			return("PIX_FMT_RGB4");
			
		case PIX_FMT_RGB4_BYTE:
			return("PIX_FMT_RGB4_BYTE");
			
		case PIX_FMT_NV12:
			return("PIX_FMT_NV12");
			
		case PIX_FMT_NV21:
			return("PIX_FMT_NV21");
			
		case PIX_FMT_ARGB:
			return("PIX_FMT_ARGB");
			
		case PIX_FMT_RGBA:
			return("PIX_FMT_RGBA");
			
		case PIX_FMT_ABGR:
			return("PIX_FMT_ABGR");
			
		case PIX_FMT_GRAY16BE:
			return("PIX_FMT_GRAY16BE");
			
		case PIX_FMT_GRAY16LE:
			return("PIX_FMT_GRAY16LE");
			
		case PIX_FMT_YUV440P:
			return("PIX_FMT_YUV440P");
			
		case PIX_FMT_YUVA420P:
			return("PIX_FMT_YUVA420P");
			
		case PIX_FMT_VDPAU_H264:
			return("PIX_FMT_VDPAU_H264");
			
		case PIX_FMT_VDPAU_MPEG1:
			return("PIX_FMT_VDPAU_MPEG1");
			
		case PIX_FMT_VDPAU_MPEG2:
			return("PIX_FMT_VDPAU_MPEG2");
			
		case PIX_FMT_VDPAU_WMV3:
			return("PIX_FMT_VDPAU_WMV3");
			
		case PIX_FMT_VDPAU_VC1:
			return("PIX_FMT_VDPAU_VC1");
			
		case PIX_FMT_RGB48BE:
			return("PIX_FMT_RGB48BE");
			
		case PIX_FMT_RGB48LE:
			return("PIX_FMT_RGB48LE");
			
		case PIX_FMT_RGB565BE:
			return("PIX_FMT_RGB565BE");
			
		case PIX_FMT_RGB565LE:
			return("PIX_FMT_RGB565LE");
			
		case PIX_FMT_RGB555BE:
			return("PIX_FMT_RGB555BE");
			
		case PIX_FMT_RGB555LE:
			return("PIX_FMT_RGB555LE");
			
		case PIX_FMT_BGR565BE:
			return("PIX_FMT_BGR565BE");
			
		case PIX_FMT_BGR565LE:
			return("PIX_FMT_BGR565LE");
			
		case PIX_FMT_BGR555BE:
			return("PIX_FMT_BGR555BE");
			
		case PIX_FMT_BGR555LE:
			return("PIX_FMT_BGR555LE");
			
		case PIX_FMT_VAAPI_MOCO:
			return("PIX_FMT_VAAPI_MOCO");
			
		case PIX_FMT_VAAPI_IDCT:
			return("PIX_FMT_VAAPI_IDCT");
			
		case PIX_FMT_VAAPI_VLD:
			return("PIX_FMT_VAAPI_VLD");
			
		case PIX_FMT_YUV420P16LE:
			return("PIX_FMT_YUV420P16LE");
			
		case PIX_FMT_YUV420P16BE:
			return("PIX_FMT_YUV420P16BE");
			
		case PIX_FMT_YUV422P16LE:
			return("PIX_FMT_YUV422P16LE");
			
		case PIX_FMT_YUV422P16BE:
			return("PIX_FMT_YUV422P16BE");
			
		case PIX_FMT_YUV444P16LE:
			return("PIX_FMT_YUV444P16LE");
			
		case PIX_FMT_YUV444P16BE:
			return("PIX_FMT_YUV444P16BE");
			
		case PIX_FMT_VDPAU_MPEG4:
			return("PIX_FMT_VDPAU_MPEG4");
			
		case PIX_FMT_DXVA2_VLD:
			return("PIX_FMT_DXVA2_VLD");
			
		case PIX_FMT_RGB444BE:
			return("PIX_FMT_RGB444BE");
			
		case PIX_FMT_RGB444LE:
			return("PIX_FMT_RGB444LE");
			
		case PIX_FMT_BGR444BE:
			return("PIX_FMT_BGR444BE");
			
		case PIX_FMT_BGR444LE:
			return("PIX_FMT_BGR444LE");
			
		case PIX_FMT_Y400A:
			return("PIX_FMT_Y400A");
			
		case PIX_FMT_NB:
			return("PIX_FMT_NB");
			
		default:
			return("<not recognised>");
		}
}

void show_format (AVInputFormat * InFmt) {
		printf("Supported Formats:\n");
	    while ((InFmt = av_iformat_next(InFmt))) {
			printf("  %s\t\t%s:\n", InFmt->name, InFmt->long_name); 
		}
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
					cliOpts.maxframes = 100;
					cliOpts.mode = (char*)"V4L2";
					cliOpts.devicepath = (char*)"/dev/video0";
					cliOpts.videopath = (char*)"test_out.mpg";
					cliOpts.width = 640;
					cliOpts.height = 480;
					cliOpts.networkport = 9999;
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
