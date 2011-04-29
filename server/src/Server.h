//      gs.h	Header File
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
//		Libraries
//			avformat 
//			avcodec
//			avutil 
//			avdevice 
//			swscale
//

#ifndef INT64_C
#define INT64_C(c) (c ## LL)
#define UINT64_C(c) (c ## ULL)
#endif

// Get Standard Libraries

#include <iostream>

#include <cstdio>
#include <cstring>
#include <cstdlib>
#include <cmath>
#include <csignal>      // for SIGQUIT, etc. (CTRL C)

// Need to investigate how the new form of libraries are

#include <stdbool.h>
#include <unistd.h>
#include <getopt.h>

// Get FFMPEG stuff

extern "C" {	// required for compiling in C++ - Thanks Obada! :D
#include <libavcodec/avcodec.h>
#include <libavformat/avformat.h>
#include <libavutil/avutil.h>
#include <libavdevice/avdevice.h> 
#include <libswscale/swscale.h>
}

// Needed for cout, cin, cerr streams

using namespace std;

// Function Definitions

static void 	SaveFrame		(AVFrame *pFrame, int width, int height, int iFrame);
//void 			fail			(char *method_name, char *error_message, int terminate);
void 			fail			(const char* method_name, const char* error_message, int terminate);
void			usage			(char prog[]);
void			show_format		(AVInputFormat * InFmt);
void 			show_version	(void);
int				get_options		(int c, char ** v);
void			signal_handler	(int sig);

#define			FALSE		0
#define			TRUE		!FALSE

/* Structure that contains command line options
 * 	Worked by get_options();
 * 
 * Structure is Globally Accessable, so any function can see what was asked of the program
 *	optString is contains the short options
 * 	longOpts contains the longer options which are re-mapped to the shorter optString.
 */

struct cliOpts_t 
{
	int 		verbose;		// if ! false, show verbose messages
	int 		version;		// if ! false, show version message
	int 		formats;		// if ! false, show supported formats
	int			saveframes;		// if ! false, save every <val>th frame to pmm
	int			width;			// width of image, if non 0
	int 		height;			// height of image, if non 0
	int 		maxframes;		// only process <val> frames
	int			CRASHOUT;		// DO NOT USE! DEVELOPMENT ONLY
    int         saveaudio;      // TRUE if audiopath is defined (used to decide if to open file handles, etc)
	int			savevideo;		// TRUE if videopath is defined (used to decide if to open file handles, etc)
	int			networkport;	// Port for Network
	char*		devicepath;	    // path to file/device
	char*		mode;			// mode for program
	char*		videopath;		// Path for output video
    char*       audiopath;      // Path for output audio
} cliOpts;

static const char *optString = "vfs:z:r:m:d:o:a:p:h?";

static const struct option longOpts[] = {
	{ "verbose",	no_argument, 		NULL, 'v' },
	{ "formats",	no_argument, 		NULL, 'f' },
	{ "save",		required_argument, 	NULL, 's' },
	{ "maxframes",	required_argument, 	NULL, 'z' },
	{ "resolution",	required_argument, 	NULL, 'r' },
	{ "mode",		required_argument, 	NULL, 'm' },
	{ "device",		required_argument, 	NULL, 'd' },
	{ "video",		required_argument, 	NULL, 'o' },
    { "audio",      required_argument,  NULL, 'a' },
	{ "port",		required_argument,	NULL, 'p' },
	{ "help",		no_argument, 		NULL, 'h' },
	{ "version",	no_argument,		NULL, 0 },
	{ "DEFAULT",	no_argument,		NULL, 0 },
	{ NULL,			no_argument, 		NULL, 0 }
};

// Lookup AV Error Codes
void AVERROR_LOOKUP(int EN) {
	//fprintf(stderr, "*** ");
	switch (EN) {
		case AVERROR_IO: /**< I/O error */
			fprintf(stderr, "AVERROR_IO (%d): I/O Error\n", EN);
			break;
		case AVERROR_NUMEXPECTED: /**< Number syntax expected in filename. */
			fprintf(stderr, "AVERROR_NUMEXPECTED (%d): Number Syntax Expected in Filename\n", EN);
			break;
		case AVERROR_INVALIDDATA: /**< invalid data found */
			fprintf(stderr, "AVERROR_INVALIDDATA (%d): Invalid Data Found\n", EN);
			break;
		case AVERROR_NOMEM: /**< not enough memory */
			fprintf(stderr, "AVERROR_NOMEM (%d): Not Enough Memory\n", EN);
			break;
		case AVERROR_NOFMT: /**< unknown format */
			fprintf(stderr, "AVERROR_NOFMT (%d): Unknown Format\n", EN);
			break;
		case AVERROR_NOTSUPP: /**< Operation not supported. */
			fprintf(stderr, "AVERROR_NOTSUPP (%d): Operation Not Supported\n", EN);
			break;
		case AVERROR_NOENT: /**< No such file or directory. */
			fprintf(stderr, "AVERROR_NOENT (%d): No Such File or Directory\n", EN);
			break;
		default:
			fprintf(stderr, "Unknown Error (%d): Not Recognised Error Number\n", EN);
			break;
	}
}

// Lookup PixelFormats
void PIX_FMT_LOOKUP(int N) {
	//fprintf(stderr, "PixelFormat = ");
	switch (N) {
		case PIX_FMT_NONE:
			fprintf(stderr, "PIX_FMT_NONE (%d)", N);
			break;
		case PIX_FMT_YUV420P:
			fprintf(stderr, "PIX_FMT_YUV420P (%d)", N);
			break;
		case PIX_FMT_YUYV422:
			fprintf(stderr, "PIX_FMT_YUYV422 (%d)", N);
			break;
		case PIX_FMT_RGB24:
			fprintf(stderr, "PIX_FMT_RGB24 (%d)", N);
			break;
		case PIX_FMT_BGR24:
			fprintf(stderr, "PIX_FMT_BGR24 (%d)", N);
			break;
		case PIX_FMT_YUV422P:
			fprintf(stderr, "PIX_FMT_YUV422P (%d)", N);
			break;
		case PIX_FMT_YUV444P:
			fprintf(stderr, "PIX_FMT_YUV444P (%d)", N);
			break;
		case PIX_FMT_YUV410P:
			fprintf(stderr, "PIX_FMT_YUV410P (%d)", N);
			break;
		case PIX_FMT_YUV411P:
			fprintf(stderr, "PIX_FMT_YUV411P (%d)", N);
			break;
		case PIX_FMT_GRAY8:
			fprintf(stderr, "PIX_FMT_GRAY8 (%d)", N);
			break;
		case PIX_FMT_MONOWHITE:
			fprintf(stderr, "PIX_FMT_MONOWHITE (%d)", N);
			break;
		case PIX_FMT_MONOBLACK:
			fprintf(stderr, "PIX_FMT_MONOBLACK (%d)", N);
			break;
		case PIX_FMT_RGB32:
			fprintf(stderr, "PIX_FMT_RGB32 (%d)", N);
			break;
		case PIX_FMT_XVMC_MPEG2_MC:
			fprintf(stderr, "PIX_FMT_XVMC_MPEG2_MC (%d)", N);
			break;
		case PIX_FMT_XVMC_MPEG2_IDCT:
			fprintf(stderr, "PIX_FMT_XVMC_MPEG2_IDCT (%d)", N);
			break;
		case PIX_FMT_UYVY422:
			fprintf(stderr, "PIX_FMT_UYVY422 (%d)", N);
			break;
		case PIX_FMT_UYYVYY411:
			fprintf(stderr, "PIX_FMT_UYYVYY411 (%d)", N);
			break;
		case PIX_FMT_BGR8:
			fprintf(stderr, "PIX_FMT_BGR8 (%d)", N);
			break;
		case PIX_FMT_BGR4:
			fprintf(stderr, "PIX_FMT_BGR4 (%d)", N);
			break;
		case PIX_FMT_BGR4_BYTE:
			fprintf(stderr, "PIX_FMT_BGR4_BYTE (%d)", N);
			break;
		case PIX_FMT_RGB8:
			fprintf(stderr, "PIX_FMT_RGB8 (%d)", N);
			break;
		case PIX_FMT_RGB4:
			fprintf(stderr, "PIX_FMT_RGB4 (%d)", N);
			break;
		case PIX_FMT_RGB4_BYTE:
			fprintf(stderr, "PIX_FMT_RGB4_BYTE (%d)", N);
			break;
		case PIX_FMT_NV12:
			fprintf(stderr, "PIX_FMT_NV12 (%d)", N);
			break;
		case PIX_FMT_NV21:
			fprintf(stderr, "PIX_FMT_NV21 (%d)", N);
			break;
		case PIX_FMT_ARGB:
			fprintf(stderr, "PIX_FMT_ARGB (%d)", N);
			break;
		case PIX_FMT_RGBA:
			fprintf(stderr, "PIX_FMT_RGBA (%d)", N);
			break;
		case PIX_FMT_ABGR:
			fprintf(stderr, "PIX_FMT_ABGR (%d)", N);
			break;
		case PIX_FMT_GRAY16BE:
			fprintf(stderr, "PIX_FMT_GRAY16BE (%d)", N);
			break;
		case PIX_FMT_GRAY16LE:
			fprintf(stderr, "PIX_FMT_GRAY16LE (%d)", N);
			break;
		case PIX_FMT_YUV440P:
			fprintf(stderr, "PIX_FMT_YUV440P (%d)", N);
			break;
		case PIX_FMT_YUVA420P:
			fprintf(stderr, "PIX_FMT_YUVA420P (%d)", N);
			break;
		case PIX_FMT_VDPAU_H264:
			fprintf(stderr, "PIX_FMT_VDPAU_H264 (%d)", N);
			break;
		case PIX_FMT_VDPAU_MPEG1:
			fprintf(stderr, "PIX_FMT_VDPAU_MPEG1 (%d)", N);
			break;
		case PIX_FMT_VDPAU_MPEG2:
			fprintf(stderr, "PIX_FMT_VDPAU_MPEG2 (%d)", N);
			break;
		case PIX_FMT_VDPAU_WMV3:
			fprintf(stderr, "PIX_FMT_VDPAU_WMV3 (%d)", N);
			break;
		case PIX_FMT_VDPAU_VC1:
			fprintf(stderr, "PIX_FMT_VDPAU_VC1 (%d)", N);
			break;
		case PIX_FMT_RGB48BE:
			fprintf(stderr, "PIX_FMT_RGB48BE (%d)", N);
			break;
		case PIX_FMT_RGB48LE:
			fprintf(stderr, "PIX_FMT_RGB48LE (%d)", N);
			break;
		case PIX_FMT_RGB565BE:
			fprintf(stderr, "PIX_FMT_RGB565BE (%d)", N);
			break;
		case PIX_FMT_RGB565LE:
			fprintf(stderr, "PIX_FMT_RGB565LE (%d)", N);
			break;
		case PIX_FMT_RGB555BE:
			fprintf(stderr, "PIX_FMT_RGB555BE (%d)", N);
			break;
		case PIX_FMT_RGB555LE:
			fprintf(stderr, "PIX_FMT_RGB555LE (%d)", N);
			break;
		case PIX_FMT_BGR565BE:
			fprintf(stderr, "PIX_FMT_BGR565BE (%d)", N);
			break;
		case PIX_FMT_BGR565LE:
			fprintf(stderr, "PIX_FMT_BGR565LE (%d)", N);
			break;
		case PIX_FMT_BGR555BE:
			fprintf(stderr, "PIX_FMT_BGR555BE (%d)", N);
			break;
		case PIX_FMT_BGR555LE:
			fprintf(stderr, "PIX_FMT_BGR555LE (%d)", N);
			break;
		case PIX_FMT_VAAPI_MOCO:
			fprintf(stderr, "PIX_FMT_VAAPI_MOCO (%d)", N);
			break;
		case PIX_FMT_VAAPI_IDCT:
			fprintf(stderr, "PIX_FMT_VAAPI_IDCT (%d)", N);
			break;
		case PIX_FMT_VAAPI_VLD:
			fprintf(stderr, "PIX_FMT_VAAPI_VLD (%d)", N);
			break;
		case PIX_FMT_YUV420P16LE:
			fprintf(stderr, "PIX_FMT_YUV420P16LE (%d)", N);
			break;
		case PIX_FMT_YUV420P16BE:
			fprintf(stderr, "PIX_FMT_YUV420P16BE (%d)", N);
			break;
		case PIX_FMT_YUV422P16LE:
			fprintf(stderr, "PIX_FMT_YUV422P16LE (%d)", N);
			break;
		case PIX_FMT_YUV422P16BE:
			fprintf(stderr, "PIX_FMT_YUV422P16BE (%d)", N);
			break;
		case PIX_FMT_YUV444P16LE:
			fprintf(stderr, "PIX_FMT_YUV444P16LE (%d)", N);
			break;
		case PIX_FMT_YUV444P16BE:
			fprintf(stderr, "PIX_FMT_YUV444P16BE (%d)", N);
			break;
		case PIX_FMT_VDPAU_MPEG4:
			fprintf(stderr, "PIX_FMT_VDPAU_MPEG4 (%d)", N);
			break;
		case PIX_FMT_DXVA2_VLD:
			fprintf(stderr, "PIX_FMT_DXVA2_VLD (%d)", N);
			break;
		case PIX_FMT_RGB444BE:
			fprintf(stderr, "PIX_FMT_RGB444BE (%d)", N);
			break;
		case PIX_FMT_RGB444LE:
			fprintf(stderr, "PIX_FMT_RGB444LE (%d)", N);
			break;
		case PIX_FMT_BGR444BE:
			fprintf(stderr, "PIX_FMT_BGR444BE (%d)", N);
			break;
		case PIX_FMT_BGR444LE:
			fprintf(stderr, "PIX_FMT_BGR444LE (%d)", N);
			break;
		case PIX_FMT_Y400A:
			fprintf(stderr, "PIX_FMT_Y400A (%d)", N);
			break;
		case PIX_FMT_NB:
			fprintf(stderr, "PIX_FMT_NB (%d)", N);
			break;
		default:
			fprintf(stderr, "<not recognised> (%d)", N);
			break;
		}
		//fprintf(stderr, "\n");
}
