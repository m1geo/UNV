//
//      (C) 01/May/2011 - Obada
//		The UNV Project, Electronic Enginering, University College London
//
//		Based on
//			http://tldp.org/LDP/LG/issue74/tougher.html#4
//			http://www.ibm.com/developerworks/linux/tutorials/l-sock2/section4.html
//			http://www.justsoftwaresolutions.co.uk/threading/implementing-a-thread-safe-queue-using-condition-variables.html
//

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

#endif
