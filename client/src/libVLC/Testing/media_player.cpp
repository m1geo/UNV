#include <iostream>
#include <cstdio>
#include <cstdlib>
#include "../player.h"

using namespace std;

int main(int argc,char *argv[])
{
    if(argc != 3)
    {
        cerr << "You must pass two arguments! " << endl;
        cerr << "The first argument is the filename and the second one is the mode: \n" << endl;
        cerr << "0 for offline mode and 1 for offline mode" << endl;
        exit(1);
    }

	media_player(argv[1],atoi(argv[2]));	
	return 0;
}
