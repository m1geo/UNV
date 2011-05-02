#include <iostream>
#include <string>
#include <cstring>
#include <cstdio>
#include <cstdlib>
#include "libVLC/player.h"

using namespace std;

int main(int argc, char *argv[])
{
	//media_player(argv[1], 1); // Offline Mode
	while(sharedread("Data_Client_Signals").compare("3") != 0)
	{
	 	media_player(argv[1], 0); // Online Mode
    	}
    return 0;
}
