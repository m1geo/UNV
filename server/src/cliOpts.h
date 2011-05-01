//
//      (C) 02/May/2011 - George Smart, M1GEO <george.smart@ucl.ac.uk>
//		The UNV Project, Electronic Enginering, University College London
//

struct cliOpts_t {	
	int     verbose;      // if ! false, show verbose messages
	int     width;        // width of image, if non 0
	int     height;       // height of image, if non 0
	int     tcpport;      // Port for Network (TCP)
	int		udpport;      // Port for Network (UDP)
	char*   url;          // path to file/device
	char*   mode;         // mode for program
};
