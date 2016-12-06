//Description:
//	Shares IO
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

#include <iostream>
#include <string>
#include <cstring>
#include <fstream>
#include <sstream>
#include <cstdio>
#include <cstdlib>
#include <unistd.h>

using namespace std;

void sharedwrite(string filestr, int msg)
{
	fstream File;
	File.close();
	
	filestr.append(".txt");
	char *filename = (char*)filestr.c_str();

	File.open(filename, ios::out);
	File << msg << endl;
}

string sharedread(string filestr)
{
	filestr.append(".txt");
	char *filename = (char*)filestr.c_str();
	ifstream file (filename);
	string msg;
    	if (file.is_open() && file.good())
    	{		
		getline (file,msg);
    	}
    	file.close();
	return msg;
}

void deleteshared(string filestr)
{
	filestr.append(".txt");
	char *filename = (char*)filestr.c_str();
	remove((char *)filename);
}

