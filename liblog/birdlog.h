/*
 * birdlog.h
 *
 *  Created on: Nov 28, 2015
 *      Author: user
 */

#ifndef LIBLOG_BIRDLOG_H_
#define LIBLOG_BIRDLOG_H_

#include "param.h"
#include "contiki.h"
#include "sys/rtimer.h"
#include "sys/etimer.h"
#include "sys/pt.h"

#define BIRD_LOG_MSG		1
#define BIRD_LOG_BEACON		2
#define BIRD_LOG_RECOVERY	3
#define BIRD_LOG_FB			4

struct BirdLog{
  char msg;
  char beacon;
  char fb;
  char recovery;
  long int onTime;
  long int tempTime;
  int clockdrift;
};

char birdLogEnergy(char onoff, clock_time_t time,struct BirdLog * log);
char birdLogCom(int type,struct BirdLog * log);
char birdLogGet(struct BirdLog * log);
char birdLogClear(struct BirdLog * log);
char birdLogPrint(struct BirdLog * log);

#endif /* LIBLOG_BIRDLOG_H_ */
