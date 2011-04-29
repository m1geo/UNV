//============================================================================
// Name        : netClient.cpp
// Author      : Obada Sawalha
// Version     : 0.01
// Copyright   : 
// Description :
// Sources 	   : TCP Libraries: Rob Tougher (http://tldp.org/LDP/LG/issue74/tougher.html#4)
//			   : UDP Functions based loosely on tutorial: (http://www.ibm.com/developerworks/linux/tutorials/l-sock2/section4.html)
//			   : Need to include the Boost Library for multi-threading
//============================================================================

//Include required libraries
#include <boost/thread.hpp>
#include "TCPLib/ClientSocket.h"
#include "TCPLib/SocketException.h"
#include <iostream>
#include <string>
#include <sys/socket.h>
#include <arpa/inet.h>
#include <cstdio>
#include <cstdlib>
#include <cstring>
#include <unistd.h>
#include <netinet/in.h>
#include <fstream>
#include "RTPpacket.h"

//Use standard name space
using namespace std;

//Include the queue class and declare two queues one for the video frame data
//the other for its corresponding size
#include "queue.cpp"
concurrent_queue<const char*> qBuffQueue;
concurrent_queue<int> qBuffQueueSize;

//Declare byte array (pointer) and integer to store the details of the last I-Frame
char * pLastIFrame;
int iLastIFrameSize;

//Define constants
#define BUFFSIZE 100000
#define UDP_PORT 3015
#define TCP_PORT 30015
#define SERVER_ADDRESS "127.0.0.1"
#define FILE_NAME "out2.mkv"

//Initialise TCP client socket
ClientSocket objTCP_Socket ( SERVER_ADDRESS, TCP_PORT );

//Define UDP Socket Global Variables
int iUDP_Sock;
struct sockaddr_in structUDP_Server;

//Include Hans VLC functions
#include "libVLC/player.h"

//Function to display error on screen and exit
void Die(string errMsg) 
{
	perror(errMsg.c_str());
	exit(1);
}// END Die function

void signalToServer(void);

//Function to run VLC with saved file
void runVLC()
{
	media_player((char*)stdout,1);
}//END runVLC function

//Function to add video frame to the queue
void addFrame(char* pFrameIn, int iFrameSize)
{

	//Declare new byte array (pointer) and initialise it with required size
	char * pFrameQ;
	pFrameQ = new char [iFrameSize];

	//Copy the incoming byte array and set to the new pointer
	memcpy(pFrameQ, pFrameIn, iFrameSize);

	//place video frame and its size in their respective arrays
	qBuffQueue.push(pFrameQ);
	qBuffQueueSize.push(iFrameSize);

}//END addFrame function

//Function to write frames in queue to file
void writeToFile()
{
	//Open output file stream for write
	ofstream fFile (FILE_NAME, ios::out | ios::binary);

	//Declare Byte Array pointer and integer to hold frame data
    const char * pWriteFrame;
    int pWriteFrameSize;

    //Declare iterators to be used in loop
	int iTimer=0;
	int iNumFrameWritten = 0;
	int iIFrameRepeated = 0;

	//Loop to pop queue and write frame to file
	while(1)
    	{
		//Do not sleep during first 20 frames writing as these arrive instantly
		if(iNumFrameWritten<1 || iNumFrameWritten > 20)
			usleep(1000);

		//If the frame queue is not empty pop it to write frame to file
		if(!qBuffQueue.empty()){
			//Keep count of first 25 frames - the iterator is used elsewhere
			if(iNumFrameWritten < 25)
				iNumFrameWritten++;

			//Pop the two queues - the frame byte data and frame size
			qBuffQueue.try_pop(pWriteFrame);
			qBuffQueueSize.try_pop(pWriteFrameSize);

		 	//Write frame to file and flush to insure it is written directly to file not memory
			fFile.write(pWriteFrame, pWriteFrameSize);
			fFile.flush();

			//Reset the timer and I-Frame repeat counter
			iTimer = 0;
			iIFrameRepeated = 0;

		//Uncomment to send I-Frame after 50ms timeout of no frame recieved
		/*} else if ( iNumFrameWritten > 20) {
			//Increment timer
			iTimer++;
			//If 50 ms go by without a new packet repeat an I-Frame
			//If 5 I-Frames repeated in succession - dont repeat more
			if(iTimer>49 && iIFrameRepeated < 5){
				//Write frame to file and flush to insure it is written directly to file not memory
				fFile.write(pLastIFrame, iLastIFrameSize);
				fFile.flush();

				//Delcare that I-Frame is being repeated
				cerr << "Repeating I Frame" << endl;

				//Reset timer and iterate I-Frame repeat counter
				iTimer=0;
				iIFrameRepeated++;
			}*/
		}
	}
}//END writeToFile function

//Start receiving RTP/UDP packets and pushing them to queue
void startClientUDP()
{
	//Declare UDP socket address and integer to hold its size
	struct sockaddr_in structUDP_Client;
	unsigned int iClientSize;

	//Declare byte arrays to handle incoming RTP frame and its payload
	char cRTP_Packet[BUFFSIZE];
	char cRTP_Payload[BUFFSIZE];

	//Declare integer to hold RTP payload size
	int iRTP_PacketSize;

	//Declare integer to hold size of received UDP packet
	int iReceived = 0;

	//Start the writeToFile function in a new thread
	//boost::thread writeFile(writeToFile);

	//Declare packet number counter
	int iPacketNo = 0;

	//loop to listen to the UDP port retrieve the RTP payload and add it to queue
	while(1)
        {
		//iterate packet number counter
		iPacketNo++;

		//get size of UDP socket address
		iClientSize = sizeof(structUDP_Client);

		//Try to run code otherwise deal with exception
		try 
                {
			int i;
			//listen to UDP port and pass the size of the packet when it arrives
			iReceived = recvfrom(iUDP_Sock, cRTP_Packet, BUFFSIZE+12, 0, (struct sockaddr *) &structUDP_Client, &iClientSize);

			//Build an RTPpacket object containing the video frame
			RTPpacket objRTP_Packet(cRTP_Packet, iReceived);

			//set RTP payload size
			iRTP_PacketSize = objRTP_Packet.getpayload_length();

			//place the retrieved RTP Payload in its byte array
			objRTP_Packet.getpayload(cRTP_Payload);

			//Send information about the RTP packet to screen
			cerr << "Got RTP packet with SeqNum # " << objRTP_Packet.getsequencenumber() << " TimeStamp " << objRTP_Packet.gettimestamp() << " ms, of type " << objRTP_Packet.getpayloadtype() << " and size " << iRTP_PacketSize << " bytes" << endl;

			//If I-Frame set it to lastIFrame pointer
			if(iRTP_PacketSize > 7500)
            		{
				//copy the Iframe so we can get to it when needed
				pLastIFrame = new char [iRTP_PacketSize];
				memcpy(pLastIFrame, cRTP_Payload, iRTP_PacketSize);
				//keep a not of its size
				iLastIFrameSize = iRTP_PacketSize;
			}
            
			// Bypassing file dependency 
			    
			for(i = 0; i<iRTP_PacketSize; i++)
			{
				cout << cRTP_Payload[i];
		   	}
			    
			//Add the video frame to queue
			addFrame(cRTP_Payload, iRTP_PacketSize);

			//Once 20 packets have been received start VLC
			
			int p = 5;	    	
			if((iPacketNo % p) == (p - 1))
			{
				//VLC Status Vector
			        boost::thread statusvector(signalToServer);
			}
         

			//reset RTP payload byte array contents
			memset (cRTP_Payload,'0',sizeof(cRTP_Payload));
		}
		//catch any exceptions
		catch(...){ cerr << "exception caught";}
	}

    // Check that client and server are using same socket
    if (structUDP_Server.sin_addr.s_addr != structUDP_Client.sin_addr.s_addr) 
    {
      Die("Received a packet from an unexpected server");
    }

    //Close the USP socket
    close(iUDP_Sock);
    exit(0);
}//END startClientUDP function

//Setup UDP socket and send welcome message to Server
void setupClientUDP()
{
 	//Integer to hold size of sent packet
	int iSent = 0;

	//Set the welcome message and declare integer to hold its size
	char cWelcomeMessage[] = "Client says: Hi";
	unsigned int iWelcomeMessageSize = strlen(cWelcomeMessage);

	// Create the UDP socket
	if ((iUDP_Sock = socket(PF_INET, SOCK_DGRAM, IPPROTO_UDP)) < 0) 
        {
		Die("Failed to create socket");
	}

   // Construct the server socket address structure
   memset(&structUDP_Server, 0, sizeof(structUDP_Server));       	// Clear structure
   structUDP_Server.sin_family = AF_INET;                  			// Set Internet/IP
   structUDP_Server.sin_addr.s_addr = inet_addr(SERVER_ADDRESS);  	// Set address
   structUDP_Server.sin_port = htons(UDP_PORT);		     			// Set server port

   // Send the welcome message to the server
   iSent = sendto(iUDP_Sock, cWelcomeMessage, iWelcomeMessageSize, 0, (struct sockaddr *) &structUDP_Server, sizeof(structUDP_Server));

   //run the startClientUDP function in a new thread
   boost::thread workerThread(startClientUDP);

}//END setupClientUDP function

//Setup RTSP/TCP session and loop keyboard input and send/receive to control session
void startClientRTSP()
{
  try
    {
      //Declare string to hold TCP reply
      string sReply;

      //Declare string to hold Keyboard input
      string sKeyboardInput;

      //If keyboard input is not "QUIT" then continue loop
      while(sKeyboardInput.compare("QUIT") != 0)
      {
    	  //Try to run code else catch exception
          try
          {
        	  //Request and wait for keyboard input
              cerr << "Enter Command: ";
              getline(cin, sKeyboardInput);

              //send keyboard input via TCP to server
              objTCP_Socket << sKeyboardInput;

        	  //get server response
              objTCP_Socket >> sReply;
          }
          //catch exception
          catch ( SocketException& ) {}

          //If keyboard input was "QUIT"
          if(sKeyboardInput.compare("QUIT") == 0)
          {
        	  //Tell the screen that we are quitting
        	  cerr << "Quitting";

          //If keyboard input was "SETUP"
          }
          else if(sKeyboardInput.compare("SETUP") == 0)
          {
        	  //Sleep for 5 ms to allow server to setup its UDP session first
        	  usleep(5*1000);
        	  //Display server response
        	  cerr << "We received this response from the server:\n\"" << sReply << endl;
        	  //Start setupClientUDP function in new thread
              boost::thread workerThread(setupClientUDP);
              //Wait for the thread to join - meaning function completed
              workerThread.join();

          //If keyboard input was "PLAY"
          }
          else if(sKeyboardInput.compare("PLAY") == 0)
          {
        	  cerr << "We received this response from the server:\n\"" << sReply << endl;
              //boost::thread workerThread(startClientUDP);
              //workerThread.join();

          //If keyboard input is not recognised, display what server has to say
          }else
          {
        	  cerr << "We received this response from the server:\n\"" << sReply << endl;
          }
      }//End keyboard and TCP loop

    }
	//Catch exception
	catch ( SocketException& e ) { cerr << "Exception was caught:" << e.description() << "\n"; }

}//END startClientRTSP function

void signalToServer()
{
    //Declare string to hold TCP reply
    string sReply;

    //Stelios catch signal stuff here
    string playoutPoint;
    
    ifstream file ("Data.txt");
    if (file.is_open() && file.good())
    {
    	getline (file,playoutPoint);
	
	//send signal via TCP to server
    	objTCP_Socket << playoutPoint;
    }
    file.close();

    //get server response
    objTCP_Socket >> sReply;
    cerr << "We received this response from the server:\n\"" << sReply << endl;
}

//main function - where it all starts
int main() 
{    	
	remove("Data.txt");
	//run the startClientRTSP function
	startClientRTSP();

	return 0;
}//END main function
