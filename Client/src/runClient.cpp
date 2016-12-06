//Description:
//	Crude way to start VLC via system call, piping video data from client
//	via unix pipe/STDOUT.
//
//Copyright 2011:
//	The UNV Project
//	Department of Electronic & Electrical Engineering
//	University College London
//
//	http://www.ee.ucl.ac.uk/
//		
//Project Authors:
//	George Smart		g.smart@ee.ucl.ac.uk
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

#include <iostream>
#include <cstdlib>
#include <string>
#include <sstream>
#include <cstring>

#define USE_TCP   0
#define USE_UDP   1
#define USE_H264  2
#define USE_MJPEG 3

using namespace std;

int main(int argc, char *argv[])
{    
    stringstream ssTLAYER, ssVCODEC, ssFCACHE;
    string str, vlcParams;
    char * strfinal;
    bool UserSelectCodec = false;
    bool UserSelectTransport = false;
    
    int TLAYER=4, VCODEC=4;
    int FCACHE=10;
    
    if ((argc != 5))
    {
        cerr << "You must supply 4 arguments when calling the client" << endl;
        cerr << string(argv[0]) + " <host> <port> <transport> <codec>" << endl;
        cerr << "\t<host>\t\tThe FQDN or IP to the machine running the server" << endl;
        cerr << "\t<port>\t\tThe base port which the server is running on" << endl;
        cerr << "\t<transport>\tThe transport layer required; either TCP or UDP" << endl;
        cerr << "\t<codec>\t\tThe video codec required; either H264 or MJPEG" << endl;
        exit(1);
    }
    
	if( (strcmp( "MJPEG",  argv[4]) == 0) || (strcmp( "mjpeg",  argv[4]) == 0) ) {
		VCODEC = USE_MJPEG;
		UserSelectCodec = true;
		//cerr << "MJPEG" << endl;
	}
	if( (strcmp( "H264",  argv[4]) == 0) || (strcmp( "h264",  argv[4]) == 0) ) {
		VCODEC = USE_H264;
		UserSelectCodec = true;
		//cerr << "H264" << endl;
	}
	if( (strcmp( "TCP",  argv[3]) == 0) || (strcmp( "tcp",  argv[3]) == 0) ) {
		TLAYER = USE_TCP;
		UserSelectTransport = true;
		//cerr << "TCP" << endl;
	}
	if( (strcmp( "UDP",  argv[3]) == 0) || (strcmp( "udp",  argv[3]) == 0) ) {
		TLAYER = USE_UDP;
		UserSelectTransport = true;
		//cerr << "UDP" << endl;
	}
	
	// If the user hasn't entered both Transport and Codec.
    if (!UserSelectCodec || !UserSelectTransport)
    {
		cerr << "Invalid Transport Layer or Codec requested." << endl;
		cerr << endl;
        cerr << "You must supply 4 arguments when calling the client" << endl;
        cerr << string(argv[0]) + " <host> <port> <transport> <codec>" << endl;
        cerr << "\t<host>\t\tThe FQDN or IP to the machine running the server" << endl;
        cerr << "\t<port>\t\tThe base port which the server is running on" << endl;
        cerr << "\t<transport>\tThe transport layer required; either TCP or UDP" << endl;
        cerr << "\t<codec>\t\tThe video codec required; either H264 or MJPEG" << endl;
        exit(1);
    }
    
    // Set the VLC File Cashing variable
    if ( (TLAYER == USE_TCP) && (VCODEC == USE_H264) ) {
		FCACHE=100;
	} else {
		FCACHE=10;
	}
	
	// Convert Number into C-string.
	ssTLAYER << TLAYER;
    ssVCODEC << VCODEC;
    ssFCACHE << FCACHE;
	
	// If we're using H264, Tell VLC how to demux raw H264 (only demuxes wrapped usually)/
	if ( VCODEC == USE_H264 ) {
		// include the h264 demuxer flag
		vlcParams = string("--file-caching ") + ssFCACHE.str() + string(" --clock-jitter=0") + string(" :demux=h264");
    } else {
		// vlc will work out MJPEG
		vlcParams = string("--file-caching ") + ssFCACHE.str() + string(" --clock-jitter=0");
	}
    
    str = string("./exec/Client ") + argv[1] + string (" ") + argv[2] + string (" ") + ssTLAYER.str() + string(" ") + ssVCODEC.str() + string(" | vlc - ") + vlcParams;
    strfinal = &str[0];
    
    cout << "SystemCall: " + string(strfinal) << endl;
    system(strfinal);
}
