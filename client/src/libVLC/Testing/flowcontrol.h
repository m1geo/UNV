#include <stdio.h>
#include <stdlib.h>
#include <vlc/vlc.h>
#include <unistd.h>
#include <vlc/libvlc_media.h>
#include <iostream>
#include <string>
#include <fstream>
#include <sys/time.h>
#include <math.h>
#include <signal.h>
 
using namespace std;
 
void flowcontroller(libvlc_media_player_t *mp, long tr, int playoutPoint, int t_sampling)
{
	float dmax = 75; /* Setting maximum time difference allowable*/
	float time_benchmark = 2000; /* Setting the time_benchmark for the time difference to be compared against */
	float r;
	r = libvlc_media_player_get_rate(mp); /* Getting rate of play */
	float f;
	f = libvlc_media_player_get_fps(mp); /* Getting frame rate */
	float ml;
	ml = libvlc_media_player_get_length(mp); /* Getting movie length */
	
	float d = tr - playoutPoint; /* Finding time difference between playing time and playout point */
	
	if(abs((int)d) > time_benchmark)
	{
		time_benchmark = 1.05*abs((int)d);
	}
	
	
	cout<< "Rate = "<< r << " d = "<< d << endl;
	
	//Optional: Wait till playout-point goes beyond dmax in deviation w.r.t. real time, tr
	if(abs((int)d) > dmax)
	{
		
		//Algorithm 1: Smooth rate correction with both acceleration and deceleration of playback
		r = 1 / sqrt(1 - d/time_benchmark);

		//Algorithm 2: Smooth rate correction with both acceleration and deceleration of playback + Adaptative dmax
		//dmax = abs(d);
		//r = 1 / sqrt(1 - d/time_benchmark);

		//Algorithm 3: Snap playout point correction
		//libvlc_media_player_set_time (mp, tr);

		//Algorithm 4: Rectifier-type operation for Algorithm 1 (Only acceleration or unity rate)
		//r = 1 / sqrt(1 - d/time_benchmark);
		//if(r < 1)
		//{
		//	r = 1;
		//}

		//Algorithm 5: Preventing playout-point from going over movie-length (Important for time-changing video files)
		if(r * t_sampling + playoutPoint > tr + 2*t_sampling)
		{
			//Algorithm 5.1: Pause video when risk of going over present
			//libvlc_media_player_set_pause(mp); /* Pause movie */

			//Algorithm 5.2: Find and set suitable rate to prevent risk of going over
			r = 1; 
		}
		
	}
	else
	{
		r = 1;
	}

	//Algorithm 6: Rate to correct within sampling time
	/*	
	if((1 + 0.25*(d/t_sampling)) > 0.75)
	{
		r = 1 + 0.25*(d/t_sampling);
	}
	else
	{
		r = 0.75;
	}
	*/
	libvlc_media_player_set_rate (mp, r); /* Setting new play rate */
	
}
