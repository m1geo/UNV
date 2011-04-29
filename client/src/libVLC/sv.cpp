/* sv.cpp
 *
 * This file is written by Stelios Vitorakis. It contains functions 
 * needed for libVLC player to handle the play stop and pause buttons 
 * pressed from keyboard. 
 *
 * For Windows compilers these funcions are contained in the header file
 * conio.h written by Borland for Turbo C++
 */

#include <cstdio>
#include <cstdlib>

// Needed for the kbhit function in Linux

#include <sys/select.h>

// Needed for getch and getche function

#include <termios.h>

// clrsrc() function

void clrscr(void)
{
    system("clear");
} // end of clrsrc() function

// kbhit function

int kbhit(void)
{
    struct timeval tv;
    fd_set read_fd;

    tv.tv_sec=0;
    tv.tv_usec=0;
    FD_ZERO(&read_fd);
    FD_SET(0,&read_fd);

    if(select(1, &read_fd, NULL, NULL, &tv) == -1)
        return 0;

    if(FD_ISSET(0,&read_fd))
        return 1;

    return 0;
} // end of kbhit function

// getch function

int getch(void)
{
   static int ch = -1, fd = 0;
   struct termios neu, alt;
   
   fd = fileno(stdin);
   tcgetattr(fd, &alt);
   neu = alt;
   neu.c_lflag &= ~(ICANON|ECHO);
   tcsetattr(fd, TCSANOW, &neu);
   ch = getchar();
   tcsetattr(fd, TCSANOW, &alt);
   return ch;
} // end of getch function

/* reads from keypress, echoes */

// getche() function

int getche(void)
{
    struct termios oldattr, newattr;
    int ch;
    tcgetattr( STDIN_FILENO, &oldattr );
    newattr = oldattr;
    newattr.c_lflag &= ~( ICANON );
    tcsetattr( STDIN_FILENO, TCSANOW, &newattr );
    ch = getchar();
    tcsetattr( STDIN_FILENO, TCSANOW, &oldattr );
    return ch;
} // end of getche() function
