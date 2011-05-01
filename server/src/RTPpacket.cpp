//
//      (C) 01/May/2011 - Obada
//		The UNV Project, Electronic Enginering, University College London
//
//		Based on
//			http://tldp.org/LDP/LG/issue74/tougher.html#4
//			http://www.ibm.com/developerworks/linux/tutorials/l-sock2/section4.html
//			http://www.justsoftwaresolutions.co.uk/threading/implementing-a-thread-safe-queue-using-condition-variables.html
//

#include <string>
#include <iostream>
#include <string.h>
#include "RTPpacket.h"

//size of the RTP header:
static int HEADER_SIZE = 12;

//Fields that compose the RTP header
int Version;
int Padding;
int Extension;
int CC;
int Marker;
int PayloadType;
int SequenceNumber;
int TimeStamp;
int Ssrc;

char * header;  //Bitstream of the RTP header
char * payload;  //Bitstream of the RTP payload
int payload_size;  //size of the RTP payload

//getpayload : return the payload bistream of the RTPpacket and its size
int RTPpacket::getpayload(char* data) {
	for (int i=0; i < payload_size; i++) {
		data[i] = payload[i];
	}
	return(payload_size);
}

//getpayload_length : return the length of the payload
int RTPpacket::getpayload_length() {
	return(payload_size);
}

//getlength : return the total length of the RTP packet
int RTPpacket::getlength() {
	return(payload_size + HEADER_SIZE);
}

//getpacket : returns the packet bitstream and its length
int RTPpacket::getpacket(char * packet) {
	//packet = new char [payload_size + HEADER_SIZE];
	//construct the packet = header + payload
	for (int i=0; i < HEADER_SIZE; i++)
		packet[i] = header[i];

	for (int i=0; i < payload_size; i++)
		packet[i+HEADER_SIZE] = payload[i];

	//return total size of the packet
	return(payload_size + HEADER_SIZE);
}

//gettimestamp
int RTPpacket::gettimestamp() {
	return(TimeStamp);
}

//getsequencenumber
int RTPpacket::getsequencenumber() {
	return(SequenceNumber);
}

//getpayloadtype
int RTPpacket::getpayloadtype() {
	return(PayloadType);
}

//return the unsigned value of 8-bit integer nb
int RTPpacket::unsigned_int(int nb) {
	if (nb >= 0)
		return(nb);
	else
		return(256+nb);
}

RTPpacket::RTPpacket(int PType, int Framenb, int Time, const char* arrIn, int size) {

	//fill in the default header fields:
	Version = 2;
	Padding = 0;
	Extension = 0;
	CC = 0;
	Marker = 1;
	Ssrc = 305419896;

	//fill changing header fields:
	SequenceNumber = Framenb;
	TimeStamp = Time;
	PayloadType = PType;

	//build the header bistream:
	header = new char[HEADER_SIZE];

	//fill the header array of byte with RTP header fields
	header[0] = ((Version<<6)|(Padding<<5)|(Extension<<4)|CC);
	header[1] = ((Marker<<7)|PayloadType);
	header[2] = (SequenceNumber>>8);
	header[3] = (SequenceNumber);

	for (int i=0; i < 4; i++)
		header[7-i] = (TimeStamp>>(8*i));

	for (int i=0; i < 4; i++)
		header[11-i] = (Ssrc>>(8*i));

	//fill the payload bitstream:
	payload_size = size;
	payload = new char[size];

	//fill payload array of byte from data (given in parameter of the constructor)
	for (int i=0; i < size; i++)
		payload[i] = arrIn[i];
}

RTPpacket::RTPpacket(char* packet, int packet_size)
{
	//fill default fields:
	Version = 2;
	Padding = 0;
	Extension = 0;
	CC = 0;
	Marker = 0;
	Ssrc = 0;

	//check if total packet size is lower than the header size
	if (packet_size >= HEADER_SIZE) {
		//get the header bitsream:
		header = new char [HEADER_SIZE];
		for (int i=0; i < HEADER_SIZE; i++)
			header[i] = packet[i];
	
		//get the payload bitstream:
		payload_size = packet_size - HEADER_SIZE;
		payload = new char [payload_size];
		for (int i=HEADER_SIZE; i < packet_size; i++)
			payload[i-HEADER_SIZE] = packet[i];

		//interpret the changing fields of the header :
		PayloadType = header[1] & 127;
		SequenceNumber = unsigned_int(header[3]) + 256*unsigned_int(header[2]);
		TimeStamp = unsigned_int(header[7]) + 256*unsigned_int(header[6]) + 65536*unsigned_int(header[5]) + 16777216*unsigned_int(header[4]);
	}
}

RTPpacket::~RTPpacket() {
	// TODO Auto-generated destructor stub
}
