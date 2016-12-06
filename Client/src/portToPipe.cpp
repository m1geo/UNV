//Description:
//	Moves data from port to unix pipe.
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
#include <cstdlib>
#include <cstring>
#include <unistd.h>
#include <sys/socket.h>
#include <arpa/inet.h>
#include <netinet/in.h>

//Use standard name space
using namespace std;

#define BUFFSIZE 100000

//main function - where it all starts
int main(int argc, char *argv[]) 
{

	int iUDP_VLC_Sock;
	struct sockaddr_in structUDP_VLC_Server;

	char cPayload[BUFFSIZE];

	//Declare UDP socket address and integer to hold its size
	struct sockaddr_in structUDP_VLC_Client;
	unsigned int iClientSize;

	//Integer to hold size of sent packet
	int iSent = 0;
	int iReceived =0;

	//Set the welcome message and declare integer to hold its size
	char cWelcomeMessage[] = "VLC Client says: Hi";
	//cWelcomeMessage += iPortNum;
	unsigned int iWelcomeMessageSize = strlen(cWelcomeMessage);

	// Create the UDP socket
	if ((iUDP_VLC_Sock = socket(PF_INET, SOCK_DGRAM, IPPROTO_UDP)) < 0) 
    {
		cerr << ("Failed to create socket");
		exit(1);
	}

    // Construct the server socket address structure
    memset(&structUDP_VLC_Server, 0, sizeof(structUDP_VLC_Server));       	// Clear structure
    structUDP_VLC_Server.sin_family = AF_INET;                  			// Set Internet/IP
    structUDP_VLC_Server.sin_addr.s_addr = inet_addr("127.0.0.1");  	// Set address
    structUDP_VLC_Server.sin_port = htons(atoi(argv[1]));		     			// Set server port

    // Send the welcome message to the server
    iSent = sendto(iUDP_VLC_Sock, cWelcomeMessage, iWelcomeMessageSize, 0, (struct sockaddr *) &structUDP_VLC_Server, sizeof(structUDP_VLC_Server));

	iClientSize = sizeof(structUDP_VLC_Client);
	while(1)
    {
	    try 
        {
			int i;
			//listen to UDP port and pass the size of the packet when it arrives
			iReceived = recvfrom(iUDP_VLC_Sock, cPayload, BUFFSIZE, 0, (struct sockaddr *) &structUDP_VLC_Client, &iClientSize);
            		cerr<<iReceived;

			// send to pipe   
			for(i = 0; i<iReceived; i++)
			{
				cout << cPayload[i];
			}
		}
		//catch any exceptions
		catch(...){ cerr << "exception caught";}
	}

	return 0;
}//END main function
