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

