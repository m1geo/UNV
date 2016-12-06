//Description:
//	Network Client Application.
//
//Sources:
//	http://tldp.org/LDP/LG/issue74/tougher.html#4
//	http://www.ibm.com/developerworks/linux/tutorials/l-sock2/section4.html
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

//Include required libraries
#include <iostream>
#include <fstream>
#include <string>
#include <cstdio>
#include <cstring>
#include <unistd.h>
#include <sys/socket.h>
#include <arpa/inet.h>
#include <netinet/in.h>
#include <boost/thread.hpp>
#include <sys/time.h>
#include <time.h>
#include "SetTimer.h"

// Include Obada's Header Files

#include "RTPpacket.h"
#include "TCPLib/ClientSocket.h"
#include "TCPLib/SocketException.h"

// Include Hans' and Stelios' Header Files

#include "libVLC/player.h"

//Use standard name space
using namespace std;

#define USE_TCP   0
#define USE_UDP   1
#define USE_H264  2
#define USE_MJPEG 3

// Variables for bundling.  Populated from command line arguments.
int TLAYER = 4; //4 is an invalid state
int VCODEC = 4; //4 is an invalid state

//Include the queue class and declare two queues one for the video frame data
//the other for its corresponding size
#include "queue.cpp"
concurrent_queue<const char*> qBuffQueue;
concurrent_queue<int> qBuffQueueSize;

//Declare byte array (pointer) and integer to store the details of the last I-Frame
bool flag=true;

//Declare packet number counter
int iPacketNoTotal = 0;
int iPacketNo[100];

//RTSP variables
//rtsp states
int INIT = 0;
int READY = 1;
int PLAYING = 2;
//rtsp message types
int SETUP = 3;
int PLAY = 4;
int PAUSE = 5;
int TEARDOWN = 6;

//Other RTSP Variables
int iRTSP_State; //RTSP Server state == INIT or READY or PLAY
int RTSP_ID = 123456; //ID of the RTSP session
int RTSPSeqNb = 0; //Sequence number of RTSP messages within the session

char cVidHeader[50000];
int iVidHeaderSize;

//Declare string to hold TCP reply
string sReply;
//Declare string to hold RTSP message
string sRTSPmessage;

//Open output file stream for write
ofstream audioFile ("audio.mp3", ios::out | ios::binary);

//Define constants
#define BUFFSIZE 100000
#define FILE_NAME "out2.mkv"
#define TESTING_MODE
#define TESTING_PERIOD 50000

long int log_buffer[100000][2];

// Global Variables

int tcp_port;
int udp_port;
char *server_address;

//Timer
uint32_t start,stop;

//Initialise TCP client socket
ClientSocket objTCP_Socket; //( SERVER_ADDRESS, TCP_PORT );

//ClientSocket objTCP_Socket(server_address,tcp_port);

//Define UDP Socket Global Variables
int iUDP_Sock[100];
struct sockaddr_in structUDP_Server[100];
int iNumUDP_Ports;
struct sockaddr_in structUDP_VLC_Client;
int iUDP_VLC_Sock;

void writelogfile(long int a[100000][2],int packetnumber)
{
  FILE *client_log;
 
   client_log=fopen("client_log.csv","wt");
    if(client_log==NULL)
    {
      perror("Couldn't open log file\n");
      exit(1);
    }
  cerr<<"Dumping File..."<<endl;

  for(int i=0; i<packetnumber; i++)
  {
    fprintf(client_log,"%ld, %ld\n",a[i][0],a[i][1]);
  }
  fclose(client_log);
  exit(1);
}

//Function to display error on screen and exit
void Die(string errMsg) 
{
	perror(errMsg.c_str());
	exit(1);
}// END Die function

string prepare_RTSP_request(string request_type) 
{
      string RTSP_request;
      stringstream ss;

      //write the request line:
      ss << request_type << " RTSP/1.0" << "\r\n";
      
      //write the CSeq line: 
      ss << "CSeq: " << ++RTSPSeqNb << "\r\n";

      //check if request_type is equal to "SETUP" and in this case write the Transport: line advertising to the server the port used to receive the RTP packets RTP_RCV_PORT
      if (request_type.compare("SETUP") == 0)
	ss << "Transport: RTP/UDP; client_port= "<< udp_port << "\r\n";
      else
	ss << "Session: " << RTSP_ID << "\n";
      
      RTSP_request = ss.str();
      return RTSP_request;
}

vector<string> &split(const string &s, char delim, vector<string> &elems) {
    stringstream ss(s);
    string item;
    while(getline(ss, item, delim)) {
        elems.push_back(item);
    }
    return elems;
}

//Parse Server Response
int parse_server_response(string server_response) 
{
    int reply_code = 0;
    vector<string> lines;
    split(server_response, '\n', lines);

    //parse status line and extract the reply_code:
    string StatusLine = lines[0];
    cerr << StatusLine << endl;
    
    vector<string> tokens;
    split(StatusLine, ' ', tokens);

      reply_code = atoi(tokens[1].c_str());
      
      //if reply code is OK get and print the 2 other lines
    if (reply_code == 200)
	{
	    string SeqNumLine = lines[1];
	    cerr << SeqNumLine << endl;

	    vector<string> seq_tokens;
        split(SeqNumLine, ' ', seq_tokens);
        RTSPSeqNb = atoi(tokens[1].c_str());
	  
	    string SessionLine = lines[2];
	    cerr << SessionLine << endl;
	
	    //if state == INIT gets the Session Id from the SessionLine
	    vector<string> session_tokens;
    	split(SessionLine, ' ', session_tokens);
	    RTSP_ID = atoi(session_tokens[1].c_str());
	}
    
    return reply_code;
}

void runRTSP_Player()
{
    sRTSPmessage = prepare_RTSP_request("PLAY");
    
    //send keyboard input via TCP to server
    objTCP_Socket << sRTSPmessage;
	
    //get server response
    objTCP_Socket >> sReply;
	      
    parse_server_response(sReply);
	      
    iRTSP_State = PLAYING;
}

//Start receiving RTP/UDP packets and pushing them to queue
void startClientUDP(int iPortNum)
{
	
	//Declare UDP socket address and integer to hold its size
	struct sockaddr_in structUDP_Client[iNumUDP_Ports];
	unsigned int iClientSize;

	//Declare byte arrays to handle incoming RTP frame and its payload
	char cRTP_Packet[BUFFSIZE];
	char cRTP_Payload[BUFFSIZE];
	char cRTP_Payload_local[BUFFSIZE];

	//Declare integer to hold RTP payload size
	int iRTP_PacketSize,iRTP_PacketSize_local=0;

	//Declare integer to hold size of received UDP packet
	int iReceived = 0;
	
#ifdef TESTING_MODE
	start=get_timestamp();
#endif

	//loop to listen to the UDP port retrieve the RTP payload and add it to queue
	while(1)
	{
		//iterate packet number counter
		iPacketNoTotal++;
		iPacketNo[iPortNum]++;

		//get size of UDP socket address
		iClientSize = sizeof(structUDP_Client[iPortNum]);
		//start = stampstart();
		//Try to run code otherwise deal with exception
		try 
		{
			//listen to UDP port and pass the size of the packet when it arrives
			iReceived = recv(iUDP_Sock[iPortNum], cRTP_Packet, BUFFSIZE, 0);
			
			
			
#ifdef TESTING_MODE
	stop = get_timestamp();
	log_buffer[iPacketNoTotal][0] = stop-start;
	log_buffer[iPacketNoTotal][1] = iReceived;
	if(stop-start>=TESTING_PERIOD)
	{
	  writelogfile(log_buffer,iPacketNoTotal);
	  //counter=0;
	  //exit(1);
	}
#endif
			// if this is true, expect fragmented MJPEG packets via UDP
			if((TLAYER==USE_UDP) && (VCODEC==USE_MJPEG)) {
				// Fragmented
				cerr<<"FRAGMENTED DATA"<<endl;
				if(iPortNum != 1)
				{
					if(flag)
					{
						cerr<<"Header size = "<<iReceived<<endl;
						for(int i = 1; i<iReceived; i++)
						{
							cRTP_Payload_local[i-1+iRTP_PacketSize_local]=cRTP_Packet[i];//Push from right to left to fill the cRTP_Payload_local
						}
						iRTP_PacketSize_local+=iReceived-1;//passes the size which is 1B less
						if (cRTP_Packet[0]==127) // independent packet
						{
							for(int i = 0; i<iRTP_PacketSize_local; i++)
							{
								cout << cRTP_Payload_local[i];
							}
							iRTP_PacketSize_local=0;
						}
						flag=false;
					} else {
						for(int i = 1; i<iReceived; i++)
						{
							cRTP_Payload_local[i-1+iRTP_PacketSize_local]=cRTP_Packet[i];//Push from right to left to fill the cRTP_Payload_local
						}
						iRTP_PacketSize_local+=iReceived-1;//passes the size which is 1B less
						if (cRTP_Packet[0]==127) // independent packet
						{
							for(int i = 0; i<iRTP_PacketSize_local; i++)
							{
								cout << cRTP_Payload_local[i];
							}
							iRTP_PacketSize_local=0;
						}
					}	  
				}
			} else { // End of FRAGMENTED Handler
				// Non Fragmented
				cerr<<"WHOLE PACKET DATA"<<endl;
				if(flag)
				{
					cerr<<"Header size = "<<iReceived<<endl;
					for(int i = 0; i<iReceived; i++)
					{
						cout << cRTP_Packet[i]; // write video to stdout
					}
					flag=false;
				} else {
					for(int i = 0; i<iReceived; i++)
					{
					cout << cRTP_Packet[i];//write video to stdout
					}
				}
			} // end of NON FRAGMENTED Handler
		} // end of TRY
		//catch any exceptions
		catch(...){ cerr << "exception caught";}
	}

    // Check that client and server are using same socket
    if (structUDP_Server[iPortNum].sin_addr.s_addr != structUDP_Client[iPortNum].sin_addr.s_addr) 
    {
        Die("Received a packet from an unexpected server");
    }

    //Close the USP socket
    close(iUDP_Sock[iPortNum]);
    exit(0);
}//END startClientUDP function

void stoupper(std::string& s) 
{
    string::iterator i = s.begin();
    string::iterator end = s.end();

    while (i != end) 
    {
        *i = std::toupper((unsigned char)*i);
        ++i;
    }
}

//Setup UDP socket and send welcome message to Server
void setupClientUDP(int iPortNum, int iUDP_Port)
{
 	//Integer to hold size of sent packet
	int iSent = 0;
	//Set the welcome message and declare integer to hold its size
	char cWelcomeMessage[] = "Client says: Hi";
	//cWelcomeMessage += iPortNum;
	unsigned int iWelcomeMessageSize = strlen(cWelcomeMessage);
	
	// Create the sockets
	if (TLAYER == USE_UDP) {
		if ((iUDP_Sock[iPortNum] = socket(PF_INET, SOCK_DGRAM, 0)) < 0) { // UDP
			Die("Failed to create UDP socket");
		}
	} else {
		if ((iUDP_Sock[iPortNum] = socket(AF_INET, SOCK_STREAM, 0)) < 0) { //TCP
			Die("Failed to create TCP socket");
		}
	}
	
    // Construct the server socket address structure
    memset(&structUDP_Server[iPortNum], 0, sizeof(structUDP_Server[iPortNum]));       	// Clear structure
    structUDP_Server[iPortNum].sin_family = AF_INET;                  					// Set Internet/IP
    structUDP_Server[iPortNum].sin_addr.s_addr = inet_addr(server_address);  			// Set address
    structUDP_Server[iPortNum].sin_port = htons(iUDP_Port);		     					// Set server port
	
	// If we're using UDP
	if (TLAYER == USE_UDP) {
		// send a welcome message back from Client
		iSent = sendto(iUDP_Sock[iPortNum], cWelcomeMessage, iWelcomeMessageSize, 0, (struct sockaddr *) &structUDP_Server[iPortNum], sizeof(structUDP_Server[iPortNum])); //UH
	} else { // by elimination, we're "USE_TCP".
		// Else, safely assume TCP, and so connect.
		int conn = connect(iUDP_Sock[iPortNum], (struct sockaddr *) &structUDP_Server[iPortNum], sizeof(structUDP_Server[iPortNum]));
		if (conn < 0) { //check we connected okay, and continue or die.
			Die("Failed to connect to server socket");
		}	
		// Send the welcome message to the server.  Assuming we connected, send welcome message.
		if(send(iUDP_Sock[iPortNum], cWelcomeMessage, iWelcomeMessageSize, 0)==-1){
			Die("Failed to send the welcome message");
		}
	}
	
    //run the startClientUDP function in a new thread
    boost::thread workerThread(startClientUDP, iPortNum);

}//END setupClientUDP function

//Setup RTSP/TCP session and loop keyboard input and send/receive to control session
void startClientRTSP()
{
    //Declare string to hold Keyboard input
    string sKeyboardInput;
    try
    {
        //If keyboard input is not "QUIT" then continue loop
        
        while(sKeyboardInput.compare("QUIT") != 0)
        {
    	    //Try to run code else catch exception
           
            try
            {
        	    //Request and wait for keyboard input
                cerr<<endl<<"Enter Command: ";
				getline(cin, sKeyboardInput);
				
		        //Client communicates to Player here
		        if(sKeyboardInput.compare("s") == 0)
		        {
			        sharedwrite("Data_Client_Signals", 0);
		        }
		        else if(sKeyboardInput.compare("p") == 0)
		        {
			        sharedwrite("Data_Client_Signals", 1);
		        }
            }
            
            //catch exception
            catch ( SocketException& ) {/* do nothing */}
	        stoupper(sKeyboardInput);
	        
            //If keyboard input was "QUIT"
            if(sKeyboardInput.compare("QUIT") == 0)
            {
	            sRTSPmessage = prepare_RTSP_request("TEARDOWN");
                //send keyboard input via TCP to server
                objTCP_Socket << sRTSPmessage;
	            //get server response
                objTCP_Socket >> sReply;
	            parse_server_response(sReply);

		        sharedwrite("Data_Client_Signals", 3);
    
                //Tell the screen that we are quitting
        	    cerr << "Quitting";
                //If keyboard input was "SETUP"
            }
            else if(sKeyboardInput.compare("SETUP") == 0)
            {
				
	            sRTSPmessage = prepare_RTSP_request("SETUP");
                //send keyboard input via TCP to server
                objTCP_Socket << sRTSPmessage;
	            //get server response
				objTCP_Socket >> sReply;
				
	            parse_server_response(sReply);
	            iRTSP_State = READY;
				  
                //Sleep for 5 ms to allow server to setup its UDP session first
                usleep(5*1000);
                
       	        //Start setupClientUDP function in new thread
	            for (int i=0; i<iNumUDP_Ports; i++)
                {
	      		    boost::thread workerThread(setupClientUDP, i, udp_port+i);
			        workerThread.join();
			        usleep(1000);
	            }
	            
                cerr << "Opened " << iNumUDP_Ports << " ports." << endl;
				
                //If keyboard input was "PLAY"
            }
            else if(sKeyboardInput.compare("PLAY") == 0)
            {
		        runRTSP_Player();
                //If keyboard input is not recognised, display what server has to say
            } 
            else
            {
        	    cerr << "We received this response from the server:\n\"" << sReply << endl;
            }
		}//End keyboard and TCP loop
    }
	//Catch exception
	catch ( SocketException& e ) { cerr << "Exception was caught:" << e.description() << "\n"; }

}//END startClientRTSP function

void tryStartSocket()
{
	try
    {
	    objTCP_Socket.start( server_address, tcp_port );
	    cerr<<" Client started"<<endl;
	    iRTSP_State = INIT;
    }
    //catch exception
    catch ( SocketException& ) 
    {
	    usleep(1000000);
	    cerr<<".";
	    tryStartSocket();
    }
}
//main function - where it all starts
int main(int argc, char *argv[]) 
{
    cerr << "The UNV Project - Client" << endl;
    cerr << "Department of Electronic & Electrical Engineering" << endl;
    cerr << "University College London" << endl << endl;

    if (argc != 5)
    {
        cerr << "Should be called through runClient" << endl;
        cerr << "./Client <host> <port> <#transport> <#codec>" << endl;
        cerr << "You must supply 4 arguments when calling the client" << endl;
        cerr << "\t<host>\t\tThe FQDN or IP to the machine running the server" << endl;
        cerr << "\t<port>\t\tThe base port which the server is running on" << endl;
        cerr << "\t<#transport>\tThe transport layer required; either 0 or 1" << endl;
        cerr << "\t<#codec>\tThe video codec required; either 2 or 3" << endl;
        exit(1);
    }
	
    server_address = argv[1];
    tcp_port = atoi(argv[2]);
    udp_port = tcp_port + 5;
    
    TLAYER = atoi(argv[3]); // TCP=0   UDP=1
    VCODEC = atoi(argv[4]); // H264=2  MJPEG=3
    
    // Check we understood the codec required.
    if ( !((VCODEC==USE_H264) || (VCODEC==USE_MJPEG)) ) {
		cerr << "<#codec> must be 2(H264) or 3(MJPEG)" << endl;
		exit(1);
	}
    
    // Check we understood the transport layer required.
    if ( !((TLAYER==USE_UDP) || (TLAYER==USE_TCP)) ) {
		cerr << "<#transport> must be 0(TCP) or 1(UDP)" << endl;
		exit(1);
	}
    
    bool unwrappedAudio = false;
    iNumUDP_Ports = 1;

	if (VCODEC==USE_H264) {
		cerr << "Streaming H264 via";
	} else if (VCODEC==USE_MJPEG) {
		cerr << "Streaming MJPEG via";
	} else {
		cerr << "Streaming INVALID CODEC via";
		exit(1);
	}

	if (TLAYER==USE_TCP) {
		cerr << " TCP" << endl;
	} else if (TLAYER==USE_UDP) {
		cerr << " UDP" << endl;
	} else {
		cerr << " INVALID METHOD" << endl;
		exit(1);
	}

	cerr << "Attempting to connect to server at '" << server_address << "' on TCP " << tcp_port << " / UDP " << udp_port << endl;

    cerr<<"-------------------------------------------------------------"<<endl;
    tryStartSocket();

    if(unwrappedAudio)
	iNumUDP_Ports++;
    
    for (int i=0; i<iNumUDP_Ports; i++){ iPacketNo[i] = 0; }

    // Manage the playput point and signals
    
    deleteshared("Data_playout");
	sharedwrite("Data_Client_Signals", 1);

    //run the startClientRTSP function
	boost::thread wk(startClientRTSP);
	wk.join();
	
	return 0;
}//END main function
