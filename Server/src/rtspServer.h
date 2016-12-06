//Description:
//	Main Networking Stuff.
//
//Sources:
//	http://tldp.org/LDP/LG/issue74/tougher.html#4
//	http://www.ibm.com/developerworks/linux/tutorials/l-sock2/section4.html
//	http://www.justsoftwaresolutions.co.uk/threading/implementing-a-thread-safe-queue-using-condition-variables.html
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
//============================================================================
// Name        : rtspServer.h
// Author      : Obada Sawalha, George Smart and Grigorios Stathis
// Version     : 0.01 TCP/H264
// 
// Bundled     : George Smart, M1GEO.
// Date        : Mon 3 Oct 2011
//============================================================================

//Include required libraries
#include <boost/thread.hpp>
#include "TCPLib/ServerSocket.h"
#include "TCPLib/SocketException.h"
#include <sys/socket.h>
#include <arpa/inet.h>
#include <unistd.h>
#include <netinet/in.h>
#include "RTPpacket.h"

#define USE_TCP   0
#define USE_UDP   1
#define USE_H264  2
#define USE_MJPEG 3

// For timer

#include <sys/time.h>

// Call Concurrent Queue Class and initialise a variable
#include "queue.cpp"
concurrent_queue<const char*> qBuffQueue;
concurrent_queue<int> qBuffQueueSize;
char * pVidHeader;
int iVidHeaderSize;

ServerSocket objTCP_Socket;

//rtsp states
int INIT = 0;
int READY = 1;
int PLAYING = 2;
//rtsp message types
int SETUP = 3;
int PLAY = 4;
int PAUSE = 5;
int TEARDOWN = 6;

//Open output file stream for write
//ofstream fFile ("file.mkv", ios::out | ios::binary);
char pFrameQ[200000];
int framesize;
bool slp=true;
bool flag=true;
//Declare integer to hold RTP payload size
int iPacketNo = 0;
//Declare integer to track the time in ms
int iTime = 0;
//Declare byte array (pointer) to handle RTP packet and its size
char* pRTP_Packet;
int iRTP_PacketSize;

//Other RTSP Variables
int iRTSP_State; //RTSP Server state == INIT or READY or PLAY
int RTSP_ID = 123456; //ID of the RTSP session
int RTSPSeqNb = 0; //Sequence number of RTSP messages within the session

//Define UDP Socket Global Variables
int iUDP_Sock[100];
int acc;
struct sockaddr_in structUDP_Client[100];
int iNumUDP_Ports;

//Function to display error on screen and exit
void Die(string errMsg) {
	perror(errMsg.c_str());
	exit(1);
}// END Die function

// not used.
/*
//Function to write frames in queue to file
void writeToFile(const char * pWriteFrame, int pWriteFrameSize){

 	//Write frame to file and flush to insure it is written directly to file not memory
	fFile.write(pWriteFrame, pWriteFrameSize);
	fFile.flush();

}//END writeToFile function
*/

//Funstion to reset (queue) to its minimum size
void resetBuffer(int iQueueMinSize)
{
    if(1==0)
    {
	    //Declare and initialise counter to current size of queue
	    int iSize;
	    iSize = qBuffQueue.size();
	
	    if(iRTSP_State != PLAYING && iSize>iQueueMinSize)
        {
		    //Declare pointer to a disposable array and a corresponding integer for its size
		    const char* pBinArray;
		    int iBinArraySize;
		    //While Queue is bigger than the minimum - reset to minimum
		    while(iSize>iQueueMinSize)
            {
			    //pop the queue and assign the array to the disposable pointer
			    qBuffQueue.try_pop(pBinArray);
			    qBuffQueueSize.try_pop(iBinArraySize);
			    //decrement counter
			    iSize--;
		    }
		    //Delete disposable pointer
		    delete pBinArray;
	    }
    }
}//END resetBuffer function

void sendHeader(char *pVidHeader, int size)
{
	//Integer to hold size of sent packet
	if(size > 0){
		objTCP_Socket.send(pVidHeader, size);
		//Give the screen an update
		cout << "Sending header of size " << size << " bytes" << endl;
	}else{
		objTCP_Socket.send((char *)"no-header", 10);
	}

}//END sendHeader function


string prepare_RTSP_response() {
      string RTSP_request;
      stringstream ss;
      ss << "RTSP/1.0 200 OK\r\n";
      ss << "CSeq: " << ++RTSPSeqNb << "\r\n";
      ss << "Session: " << RTSP_ID << "\r\n";
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
int parse_RTSP_request(string server_response) {

    int request_type = -1;
    vector<string> lines;
    split(server_response, '\n', lines);

    //parse request line and extract the request_type:
    string RequestLine = lines[0];
    cerr << RequestLine << endl;

    vector<string> tokens;
    split(RequestLine, ' ', tokens);

    string request_type_string = tokens[0];

      //convert to request_type structure:
      if (request_type_string.compare("SETUP") == 0)
	request_type = SETUP;
      else if (request_type_string.compare("PLAY") == 0)
	request_type = PLAY;
      else if (request_type_string.compare("PAUSE") == 0)
	request_type = PAUSE;
      else if (request_type_string.compare("TEARDOWN") == 0)
	request_type = TEARDOWN;
      else
	return -1;

      //parse the SeqNumLine and extract CSeq field
      string SeqNumLine = lines[1];
      cout << SeqNumLine << endl;

      vector<string> seq_tokens;
      split(SeqNumLine, ' ', seq_tokens);

      RTSPSeqNb = atoi(tokens[1].c_str());
	
      //get LastLine
      string LastLine = lines[2];
      cout << LastLine << endl;

      if (request_type == SETUP)
	{
	  //extract RTP_dest_port from LastLine
          //vector<string> seq_tokens;
	  //split(RequestLine, ' ', seq_tokens);
	  //RTP_dest_port = atoi(tokens[3].c_str());
	}
      //else LastLine will be the SessionId line ... do not check for now.
    
    return request_type;
}

void sendRTP_PacketTCP(int iPortNum, const char * pVideoFrame, int iVideoFrameSize)
{
	//Integer to hold size of sent packet
	int iSent = 0;
	//iterate packet number counter
	iPacketNo++;
	cout<<"TCP Frame Size = "<<iVideoFrameSize<<endl;
	iSent = send(acc, pVideoFrame, iVideoFrameSize, 0);
}

void sendRTP_PacketUDP(int iPortNum, const char * pVideoFrame, int iVideoFrameSize)
{
	//Integer to hold size of sent packet
	int iSent = 0;

	//iterate packet number counter
	iPacketNo++;

	cout<<"UDP Frame Size = "<<iVideoFrameSize<<endl;
	
	iSent = sendto(iUDP_Sock[iPortNum], pVideoFrame, iVideoFrameSize, 0, (struct sockaddr *) &structUDP_Client[iPortNum], sizeof(structUDP_Client[iPortNum]));
	
	if(iSent<0)
	{
	  Die("Packet couldn't send");
	}
}


void startUDP(int iPortNum)
{
	//Declare byte array (pointer) to handle incoming video frame and its size
    const char* pVideoFrame;
    int iVideoFrameSize = 0;

	//loop to pop queue and send byte arrays over UDP after wrapping in RTP
	while (1) 
    {
		//sleep for 1ms - (prevents from using 100% cpu and allows us to timekeep
		usleep(1000);

		//if the queue is not empty
		if(!qBuffQueue.empty())
        {	
			//Retrieve a video frame and its respective size from the queues
			qBuffQueue.try_pop(pVideoFrame);
			qBuffQueueSize.try_pop(iVideoFrameSize);

			//send RTP Packet
			//sendRTP_Packet(iPortNum, pVideoFrame,iVideoFrameSize);
		}
	}

}//END startUDP function

void setupUDPTCP(int iPortNum, int iUDP_Port)
{	
	//Declare UDP socket address
    struct sockaddr_in sockUDP_Server[iNumUDP_Ports];	

    //Set the byte array to store the client welcome message
    char cUDP_RcvWelcomeBuffer[150];
	
	//TCP Socket
	if ((iUDP_Sock[iPortNum] = socket(AF_INET, SOCK_STREAM, 0)) < 0) {
		Die("Failed to create socket");
	}

	int on=1;
	
	setsockopt( iUDP_Sock[iPortNum], SOL_SOCKET, SO_REUSEADDR, &on, sizeof(on) );

	// Construct the server sockaddr_in structure
	memset(&sockUDP_Server[iPortNum], 0, sizeof(sockUDP_Server[iPortNum]));       	// Clear struct
	sockUDP_Server[iPortNum].sin_family = AF_INET;                  		// Set Internet/IP
	sockUDP_Server[iPortNum].sin_addr.s_addr = htonl(INADDR_ANY);   		// Set Any IP address
	sockUDP_Server[iPortNum].sin_port = htons(iUDP_Port);       			// Set server port

	// Bind the socket
	if (bind(iUDP_Sock[iPortNum], (struct sockaddr *) &sockUDP_Server[iPortNum], sizeof(sockUDP_Server[iPortNum])) < 0) {
		Die("Failed to bind server socket");
	}
	//Listen the socket
	if (listen(iUDP_Sock[iPortNum], 5) < 0) {
        Die("Failed to listen the socket");
    }
    
    int size=sizeof(sockUDP_Server[iPortNum]);
	
	acc=accept(iUDP_Sock[iPortNum], (struct sockaddr *) &sockUDP_Server[iPortNum],(socklen_t *) &size);
	
	
	  if (acc < 0) 
	  {
		Die("Failed to accept the connection");
	  }
	  
	
	int receive=recv(acc, cUDP_RcvWelcomeBuffer, sizeof(cUDP_RcvWelcomeBuffer), 0);
	  
	if(receive<0)
	{
	  Die("Failed to read welcome message");
	}

	//Send client IP address and message to screen
	if(iPortNum == 0) cout << "Client connected: " << inet_ntoa(sockUDP_Server[iPortNum].sin_addr) << endl;
	cout << "Client: " << cUDP_RcvWelcomeBuffer << endl;

}//END setupUDP function

void setupUDPUDP(int iPortNum, int iUDP_Port)
{	
	//Declare UDP socket address
    struct sockaddr_in sockUDP_Server[iNumUDP_Ports];	

	//Declare integer to hold UDP client socket address size
    unsigned int iClientSize = sizeof(structUDP_Client[iPortNum]);
    
    //Set the byte array to store the client welcome message
    char cUDP_RcvWelcomeBuffer[150];

	//TCP Socket
	if ((iUDP_Sock[iPortNum] = socket(PF_INET, SOCK_DGRAM, 0)) < 0) {
		Die("Failed to create socket");
	}

	// Construct the server sockaddr_in structure
	memset(&sockUDP_Server[iPortNum], 0, sizeof(sockUDP_Server[iPortNum]));       	// Clear struct
	sockUDP_Server[iPortNum].sin_family = AF_INET;                  		// Set Internet/IP
	sockUDP_Server[iPortNum].sin_addr.s_addr = htonl(INADDR_ANY);   		// Set Any IP address
	sockUDP_Server[iPortNum].sin_port = htons(iUDP_Port);       			// Set server port

	// Bind the socket
	if (bind(iUDP_Sock[iPortNum], (struct sockaddr *) &sockUDP_Server[iPortNum], sizeof(sockUDP_Server[iPortNum])) < 0) {
		Die("Failed to bind server socket");
	}
	
	int receive=recvfrom(iUDP_Sock[iPortNum], cUDP_RcvWelcomeBuffer, sizeof(cUDP_RcvWelcomeBuffer), 0, (struct sockaddr *) &structUDP_Client[iPortNum], &iClientSize);
	  
	if(receive<0)
	{
	  Die("Failed to read welcome message");
	}

	//Send client IP address and message to screen
	if(iPortNum == 0) cout << "Client connected: " << inet_ntoa(structUDP_Client[iPortNum].sin_addr) << endl;
	cout << "Client: " << cUDP_RcvWelcomeBuffer << endl;

}//END setupUDP function


void timeKeep() {
   
	struct timeval start, end;
	long seconds = 0, useconds = 0;

	gettimeofday(&start, NULL);
	while(1){
		usleep(1000);
		gettimeofday(&end, NULL);

		seconds  = end.tv_sec  - start.tv_sec;
	        useconds = end.tv_usec - start.tv_usec;

	        iTime = (int) (((seconds) * 1000 + useconds/1000.0) + 0.5);
	}
}

//Function to prevent a thread dying
void dontDie()
{
	//sleep for 10 seconds - and loop
	while(1)
		usleep(10*1000*1000);
}//END dintDie function

void threadStartServerRTSP(int iQueueMinSize, int iTCP_Port,int iSetUDP, int TLAYER)
{	
	cout << "RTSP Server running on port... \n";

	//Try to run code else catch exception
	try 
    {
		// Create the initial socket with the TCP port number
		ServerSocket objInitTCP_Socket;
		objInitTCP_Socket.start ( iTCP_Port );

		//loop to wait for socket accept
		while(true) 
        {
			// Create connection socket on accept and set RTSP state to INIT
			objInitTCP_Socket.accept ( objTCP_Socket );
			iRTSP_State = INIT;

			//Try to run code else catch exception
			try 
            {
				//Declare string to store incoming data
				string sIncomingData;
				int iNewState;
				
				//loop to recieve data from TCP port and act on it
				while(true) 
                {	
					//recieve data
					objTCP_Socket >> sIncomingData;
					iNewState = parse_RTSP_request(sIncomingData);
					
					//If command was "SETUP"
					if (iNewState == SETUP){
						//Update client
						objTCP_Socket << prepare_RTSP_response();
						//Tell screen UDP session starting
						cout << "Starting UDP" << endl;
						//Run setupUDP function in new thread and wait for it to join
						for (int i=0; i<iNumUDP_Ports; i++){
							if (TLAYER == USE_TCP) {
								boost::thread workerThread(setupUDPTCP, i, iSetUDP+i);
							} else if (TLAYER == USE_UDP) {
								boost::thread workerThread(setupUDPUDP, i, iSetUDP+i);
							} else {
								printf("rtspServer.h couldn't find TLAYER %d\n", TLAYER);
								exit(EXIT_FAILURE);
							}
							//workerThread.join();
						}
						//Set RTSP state to READY
						iRTSP_State = READY;
						//run resetBuffer function
						resetBuffer(iQueueMinSize);
						//Send video header
						//sendHeader(pFrameQ,framesize);
						
					//If command was "PLAY"
					}else if(iNewState == PLAY){
						//Update client
						objTCP_Socket << prepare_RTSP_response();
						//run resetBuffer function
						resetBuffer(iQueueMinSize);
						//Set RTSP state to PLAYING
						iRTSP_State = PLAYING;
						//Run startUDP function in new thread
						slp=false;

					//If command was "TERMINATE"
					}else if(iNewState == TEARDOWN){
						//Update client
						objTCP_Socket << prepare_RTSP_response();

					//If command was not recognised it is a signal
					}else{
						cout << "\nClient signal: " << sIncomingData << endl;
						//Tell client
						objTCP_Socket << "Signal Recieved";
					}//END if/ifelse statement
				}//END while loop 2 - (TCP port recieve data)
			}
			//catch exception
			catch ( SocketException& ) {}

		}//END while loop 1 - (socket accept loop)
	}
	//catch exception
	catch ( SocketException& e ) { cout << "Exception was caught:" << e.description() << "\nExiting.\n"; }
}//END threadStartServerRTSP function

//Role of this function is to simply start function threadStartServerRTSP in a new thread
void startServerRTSP(int iSetQueueSize, int iSetTCP, int iSetUDP, int TLAYER)
{
	bool unwrappedAudio = true;
	iNumUDP_Ports = 1;

	boost::thread serverRTSP(&threadStartServerRTSP, iSetQueueSize, iSetTCP, iSetUDP, TLAYER);
	boost::thread timeKeepThread(timeKeep);

	if(unwrappedAudio)
		iNumUDP_Ports++;

}//END startServerRTSP function

//Function to store video header
void addHeader(char* pFrameIn, int iFrameSize)
{
	 //Set pointer to new byte array of requires size
	 pVidHeader = new char [iFrameSize];
	 
	 //Copy the incoming byte array and set to the new pointer
	 memcpy(pVidHeader, pFrameIn, iFrameSize);
	 
	 //Save the frame size to our global variable
	 iVidHeaderSize = iFrameSize;
}//END addHeader function

//Function to add video frame to the queue
void addFrame(char* pFrameIn, int iFrameSize, int TLAYER)
{
	//Declare new byte array (pointer) and initialise it with required size
	

	//Copy the incoming byte array and set to the new pointer
	memcpy(pFrameQ, pFrameIn, iFrameSize);
	
	framesize=iFrameSize;
	//send RTP Packet
	if(iRTSP_State == PLAYING)
    {
		if (TLAYER == USE_TCP) {
			sendRTP_PacketTCP(0, pFrameQ,iFrameSize);
		} else if (TLAYER == USE_UDP) {
			sendRTP_PacketUDP(0, pFrameQ,iFrameSize);
		} else {
			printf("rtspServer.h couldn't find TLAYER %d\n", TLAYER);
			exit(EXIT_FAILURE);
		}
    }
}//END addFrame function

//Function to add video frame to the queue
void addAudioFrame(char* pFrameIn, int iFrameSize, int TLAYER)
{
    //Declare new byte array (pointer) and initialise it with required size
    char * pFrameQ;
    pFrameQ = new char [iFrameSize];
	
    //Copy the incoming byte array and set to the new pointer
    memcpy(pFrameQ, pFrameIn, iFrameSize);
	
    //send RTP Packet
    if(iRTSP_State == PLAYING)
    {
		if (TLAYER == USE_TCP) {
			sendRTP_PacketTCP(0, pFrameQ,iFrameSize);
		} else if (TLAYER == USE_UDP) {
			sendRTP_PacketUDP(0, pFrameQ,iFrameSize);
		} else {
			printf("rtspServer.h couldn't find TLAYER %d\n", TLAYER);
			exit(EXIT_FAILURE);
		}
    }
}//END addFrame function
