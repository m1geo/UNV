#include <iostream>
#include "libVLC/player.h"

using namespace std;

int main(int argc, char *argv[])
{
    //media_player(argv[1], 1); // Offline Mode
    media_player(argv[1], 0); // Online Mode
    
    return 0;
}
