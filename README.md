Project Description
===================
Please find the included paper, UNV: Content Centric Live Video Streaming.
This is included as LCS1113.pdf

Supported Features:
	Live Low Delay Video Streaming
	Transport via RTSP/RTP over UDP or TCP selectable
	Selectable video codecs H.264 or MJPEG


Copyright
=========
The UNV Project
Department of Electronic & Electrical Engineering
University College London
http://www.ee.ucl.ac.uk/

Friday, 14th October 2011.

		
Project Authors
===============
George Smart		g.smart@ee.ucl.ac.uk (lead developer)
Grigorios Stathis	uceegrs@ee.ucl.ac.uk
Obada Sawalha		o.sawalha@ee.ucl.ac.uk
Lorenzo Levrini		l.levrini@ee.ucl.ac.uk
Stelios Vitorakis	s.vitorakis@ee.ucl.ac.uk
Hans Balgobin		h.balgobin@ee.ucl.ac.uk
Yiannis Andreopoulos	i.andreop@ee.ucl.ac.uk (supervisor)


Important
=========
This license file, the README.md file, and LCS1113_Paper.pdf must remain intact
in all derivative works produced by this work.


Licence
=======
This program is free software; you can redistribute it and/or modify it under 
the terms of the GNU General Public License as published by the Free Software
Foundation; either version 2 of the License, or (at your option) any later
version.

This program is distributed in the hope that it will be useful, but WITHOUT ANY 
WARRANTY; without even the implied warranty of MERCHANTABILITY or FITNESS FOR A
PARTICULAR PURPOSE.  See the GNU General Public License for more details.

You should have received a copy of the GNU General Public License along with
this program; if not, write to the Free Software Foundation, Inc., 
51 Franklin Street, Fifth Floor, Boston, MA 02110-1301, USA.

This proggram is based on the libraries listed below, each of which is
distributed under it's respective licence.


Dependencies
============
This program depends on the following libraries
	libboost-dev
	libboost-thread-dev
	libavcodec-dev
	libavformat-dev
	libavdevice-dev
	libavutil-dev
	libswscale-extra* (must be extra)
	libavcodec-extra* (must be extra)
	libvlc-dev
	build-essentials
	ffmpeg
	vlc
	g++

Open a terminal window and type:
	sudo apt-get install g++ build-essential libboost-dev \
	  libboost-thread-dev libav{codec,format,device,util}-dev \
	  libav{codec,format,device,util}-extra* libswscale-dev libvlc-dev \
	  vlc ffmpeg


Compiling
=========
This suite of programs comes supplied with make scripts for the client and
server, located in the top level directory of each branch. The make files are
simple and call the compiler to build the objects, and link them into the
applications.  

Once you have the dependencies met, simply open a terminal window and go to 
folder ./Client and type:
	make

Then go to folder ./Server and type:
	make 


Precompiled Binaries
====================
Should you have a problem compiling the server or client applications, 
precompiled 32-bit binaries can be found inside the Server and Client folders.
These were compiled on under Ubuntu 11.04, Natty, with GCC 4.5.2-8ubuntu4.

	Server/Server
	Client/runClient
	Client/exec/Client


Running
=======
The Client and Server programs both take arguments on the command line.  Try
running each without arguments or with the "--help" flag for specific details.

Example: running the streaming session on localhost

1)	Find your webcam's device node, typically /dev/video0

2)	Open a terminal, and run the server by:
	./Server -m v4l -d /dev/video0 -p 9999 -t UDP -c H264

3)	Open another terminal, and run the client by:
	./runClient localhost 9999 UDP H264

	Notice that the client has specific syntax, i.e.:
	./runClient <ipaddress> <port_no> <UDP or TCP> <H264 or MJPEG>

4)	Once both are launched, type: 
	SETUP
   
	in the client window, followed by 
	PLAY

5)	To stop the streaming, just press CTRL+C in the Server window, which is 
	explicitly captured by the server, freeing codecs and memory. 

