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


