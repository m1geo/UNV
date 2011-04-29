/*
 * RTPpacket.h
 *
 *  Created on: 27 Nov 2010
 *      Author: obada
 */

#ifndef RTPPACKET_H_
#define RTPPACKET_H_

class RTPpacket {
public:
	int getpayload(char* data);
	int getpayload_length();
	int getlength();
	int getpacket(char * packet);
	int gettimestamp();
	int getsequencenumber();
	int getpayloadtype();
	int unsigned_int(int nb);
	RTPpacket(int PType, int Framenb, int Time, const char* arrIn, int size);
	RTPpacket(char* packet, int packet_size);
	virtual ~RTPpacket();
};

#endif /* RTPPACKET_H_ */
