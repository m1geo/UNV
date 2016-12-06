//Description:
//	Implementation of the ClientSocket class
//
//Source Documents:
//	None.
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

#include "ClientSocket.h"
#include "SocketException.h"


int ClientSocket::start( std::string host, int port )
{
  if ( ! Socket::create() )
    {
      throw SocketException ( "Could not create client socket." );
    }
  if ( ! Socket::connect ( host, port ) )
    {
      throw SocketException ( "Could not bind to port." );
    }

    return 1;
}

const ClientSocket& ClientSocket::operator << ( const std::string& s ) const
{
  if ( ! Socket::send ( s ) )
    {
      throw SocketException ( "Could not write to socket." );
    }

  return *this;

}


const ClientSocket& ClientSocket::operator >> ( std::string& s ) const
{
  if ( ! Socket::recv ( s ) )
    {
      throw SocketException ( "Could not read from socket." );
    }

  return *this;
}

bool ClientSocket::send ( char s[], int bSize )
{
  bool ret = Socket::sendBytes ( s, bSize );
  if ( ! ret )
    {
      throw SocketException ( "Could not write to socket." );
    }

  return ret;

}

int ClientSocket::recv ( char s[] )
{
  int ret = Socket::recvBytes ( s );
  if ( ! ret )
    {
      throw SocketException ( "Could not write to socket." );
    }

  return ret;

}
