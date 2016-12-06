//Description:
//	RTP Header File.
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
