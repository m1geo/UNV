//Description:
//	Definition of the ServerSocket class
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

#ifndef ServerSocket_class
#define ServerSocket_class

#include "Socket.h"


class ServerSocket : private Socket
{
 public:

  int start ( int port );
  ServerSocket (){};
  virtual ~ServerSocket();

  const ServerSocket& operator << ( const std::string& ) const;
  const ServerSocket& operator >> ( std::string& ) const;

  bool send ( char s[], int bSize );
  int recv ( char s[] );

  void accept ( ServerSocket& );

};


#endif
