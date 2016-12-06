//Description:
//	Thread to provide timing functions.
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

#include <stdio.h>
#include <stdint.h>
#include <stdlib.h>
#include <sys/time.h>
#include <time.h>
#include <iostream>
#include <fstream>
#include <string>
#include <cstdio>
#include <cstring>
#include <unistd.h>

using namespace std;

uint32_t
get_timestamp()
{
	struct timeval  tv;
	struct timezone tz;
	struct tm      *tm;
	uint32_t         start;
 
	gettimeofday(&tv, &tz);
	tm = localtime(&tv.tv_sec);
 
	/*printf("TIMESTAMP-START\t  %d:%02d:%02d:%d (~%d ms)\n", tm->tm_hour,
	       tm->tm_min, tm->tm_sec, tv.tv_usec,
	       tm->tm_hour * 3600 * 1000 + tm->tm_min * 60 * 1000 +
	       tm->tm_sec * 1000 + tv.tv_usec / 1000);*/
	
	/*cerr<<"TIMESTAMP-START\t "<<tm->tm_hour<<":"<<tm->tm_min<<":"<<tm->tm_sec<<":"<<tv.tv_usec<<"("<<tm->tm_hour * 3600 * 1000 + tm->tm_min * 60 * 1000 +
	       tm->tm_sec * 1000 + tv.tv_usec / 1000<<"ms)"<<endl;*/
 
	start = tm->tm_hour * 3600 * 1000 + tm->tm_min * 60 * 1000 +
		tm->tm_sec * 1000 + tv.tv_usec / 1000;
 
	return (start);
	
}

