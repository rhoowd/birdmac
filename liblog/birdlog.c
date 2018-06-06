/*
 * birdlog.c
 *
 *  Created on: Nov 28, 2015
 *      Author: user
 */

#include "birdlog.h"
char birdLogEnergy(char onoff, clock_time_t time,struct BirdLog * log)
{
	if(onoff == 1)
	{
		log->tempTime = (int)time;
	}
	else
	{
		log->onTime += (int)time - log->tempTime;
//		printf("[log] off : %d\n",log->onTime);
	}
	return 1;
}
char birdLogCom(int type,struct BirdLog * log)
{
	if(type == BIRD_LOG_MSG)
	{
		log->msg++;
	}
	else if (type == BIRD_LOG_BEACON)
	{
		log->beacon++;
	}
	else if (type == BIRD_LOG_RECOVERY)
	{
		log->recovery = 1;
	}
	else if (type == BIRD_LOG_FB)
	{
		log->fb = 1;
	}
	return 1;
}
char birdLogGet(struct BirdLog * log)
{
	return 1;
}
char birdLogClear(struct BirdLog * log)
{
	log->beacon = 0;
	log->fb = 0;
	log->msg = 0;
	log->onTime = 0;
	log->recovery = 0;
	log->tempTime = 0;
	log->clockdrift = 0;
	return 1;
}
char birdLogPrint(struct BirdLog * log)
{
	/*printf("Energy: %d\n",log->onTime);
	printf("Beacon Retr: %d\n",log->beacon);
	printf("Msg Retr: %d\n",log->msg);
	printf("Final Beacon: %d\n",log->fb);
	printf("Recovery: %d\n",log->recovery);
	printf("clockdrift: %d\n",log->clockdrift);*/
	printf("  %6ld  %2d  %2d  ",log->onTime,log->beacon,log->msg);
	printf("%1d  %1d  %d  ",log->fb,log->recovery,log->clockdrift);
	return 1;
}


