#include <iostream>
#include <cstdio>
#include <cstdlib>
#include <string>

//Use standard name space
using namespace std;

int main(int argc, char *argv[])
{
    string argument;
    for(int i=1;i<argc;i++){
        if(argc>1){
            argument+=argv[i];
            if(i<argc-1)
                argument+= ' ';
        }
    }

    string sysCall = "./exec/Client ";
    sysCall += argument;
    sysCall += " | ./exec/player -";

    system(sysCall.c_str());
    //system("./Client | ./player -");
}
