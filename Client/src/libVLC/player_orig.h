//Description:
//	libVLC Player Header (original)
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
#include <fstream>
#include <sstream>
#include <cstdio>
#include <cstdlib>
#include <vlc/vlc.h>
#include <unistd.h>
#include <vlc/libvlc_media.h>
#include <sys/time.h>
#include "flowcontrol.h"
#include "sv.h"
#include "sharedfile.h"

using namespace std;
int playoutPoint;

int media_player(char* mediafile, int x, double speed)
{
    int t_sampling = 150;
    int m_state;

    struct timeval start, sp, end;
    long tr, tp, seconds = 0, useconds = 0, s1 = 0, u1 = 0, seconds_p = 0, useconds_p = 0;

    fstream File;

    bool offline = x;

    libvlc_instance_t * inst;
    libvlc_media_player_t *mp;
    libvlc_media_t *m;
    
    gettimeofday(&start, NULL);

	
    
    /* Load the VLC engine */
    inst = libvlc_new (0, NULL);
 
    /* Create a new item */
    m = libvlc_media_new_path (inst, mediafile);
       
    /* Create a media player playing environement */
    mp = libvlc_media_player_new_from_media (m);
    
    /* No need to keep the media now */
    libvlc_media_release (m);

    /* play the media_player */
    libvlc_media_player_play (mp);
   
    //m_state = libvlc_media_player_get_state (mp);

    do 
    {
        gettimeofday(&sp, NULL);
        m_state = libvlc_media_player_get_state (mp);
    } while (m_state != 3);

    /* Finding playout point and writing to file while playing */
    //int lm = libvlc_media_player_get_length (mp); /* Finding the length of the video */
    playoutPoint = libvlc_media_player_get_time (mp);

    while(offline && sharedread("Data_Client_Signals").compare("3") != 0)
    {
        if (kbhit() && (getchar() == 'p'))
        {
            if(m_state == 3)
            {
                libvlc_media_player_set_pause (mp,1);
            }
            else if(m_state == 4)
	        {
                libvlc_media_player_play (mp);
                gettimeofday(&start, NULL);
	            s1 = seconds;
	            u1 = useconds;
	        }
        }
	
	    usleep (t_sampling * 1000); /* Let it play a bit */
	    gettimeofday(&end, NULL);

	    if(m_state == 3)
	    {
	        seconds  = end.tv_sec  - start.tv_sec + s1;
	        useconds = end.tv_usec - start.tv_usec + u1;

	        tr = ((seconds) * 1000 + useconds/1000.0) + 0.5;
	        flowcontroller(mp, tr, playoutPoint, t_sampling);
	    }
	    cout<< "At time "<< tr << " ms, video played "<<playoutPoint <<"ms, state ="<< m_state << endl;
	
	    sharedwrite("Data_playout", playoutPoint);

	    playoutPoint = libvlc_media_player_get_time (mp);
            m_state = libvlc_media_player_get_state (mp);         
	
	    if(m_state == 6)
	    {
	        break;
	    }	
    }

    while(!offline && sharedread("Data_Client_Signals").compare("3") != 0 && sharedread("Data_Client_Signals").compare("0") != 0)
    {
       m_state = libvlc_media_player_get_state (mp);
	libvlc_media_player_set_rate (mp, speed);
        
        if(m_state == 3)
        {
            usleep (t_sampling * 1000);
	    gettimeofday(&end, NULL);
            
            seconds  = end.tv_sec  - start.tv_sec + s1;
            useconds = end.tv_usec - start.tv_usec + u1;

            tr = ((seconds) * 1000 + useconds/1000.0) + 0.5;

            seconds_p  = end.tv_sec  - sp.tv_sec;
            useconds_p = end.tv_usec - sp.tv_usec;
	
	    tp = ((seconds_p) * 1000 + useconds_p/1000.0) + 0.5;
	    playoutPoint = (int)tp;
	    cout << "At time "<< tr << " ms, video played "<< playoutPoint <<"ms, state ="<< m_state << endl;       
        }
	sharedwrite("Data_playout", playoutPoint);
	
	if(m_state == 6)
    	{
	        break;
    	}
    }
    
    File.close();
    
    /* Stop playing */
    libvlc_media_player_stop (mp);
 
    /* Free the media_player */
    libvlc_media_player_release (mp);
 
    libvlc_release (inst);
 
    return playoutPoint;
}
