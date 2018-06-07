/*
 * param.h
 *
 *  Created on: Nov 23, 2015
 *      Author: user
 */

#ifndef LANADA_APP_PARAM_H_
#define LANADA_APP_PARAM_H_

#define MICAZ_L 	2
#define Z1MOTE_L 	0
#define COOJA_L 	1

/* platform */
#define PLATFORM_L 			COOJA_L		/* MICAZ_L, Z1MOTE_L, COOJA_L */
#define EXPERIMENT			0
#define DATA_ON				1
#define PARAM_L 			0
#define BRIDGE_L			0

/* time parameter */
#define ARRIVAL_RATE		50
#define PERIOD 					((long)60)			//(sec)

#define CLOCK_RATE_L		(0.89)
#define PERIOD_MINUTE			(PERIOD/60)				//(min)
#define SHORT_TEST				(0)						// 1: short period test, 0: real_test
#define R_DRIFT 				(25.0/1000000.0)
#define MAX_CLOCK_DRIFT			(unsigned long)((CLOCK_SECOND)*(PERIOD)*(R_DRIFT))
#define TIME_SLOT				((MAX_CLOCK_DRIFT) > (CLOCK_SECOND/10) ? (MAX_CLOCK_DRIFT) : (CLOCK_SECOND/10))
#define TIME_SLOT_NUM			(4)
#define INIT_DURATION			(60) 				//(sec)
#define DRIFT_NORMAL			1	/*1:truncated normal, 0:uniform random */
#define SIGMA					(PERIOD*0.000003528)
#if PLATFORM_L ==  COOJA_L
	#define CARRIER_SENSING_TIME	1*CLOCK_SECOND/128 /2   //7ms
	#define WAIT_ACK_TIME			3*CLOCK_SECOND/128  //3*CLOCK_SECOND/128
	#define DWELL_TIME				3*CLOCK_SECOND/128  // after parent finishes SYNC wait for additional packet for BS
	#define WAIT_PULSE_TIME			3*CLOCK_SECOND/128	// wait time after sending BEACON_P
	#define NEXT_WAKEUP_TIME		(15*CLOCK_SECOND)
	#define MSG_TIME_SLOT			(2 * RTIMER_ARCH_SECOND / 128)
	#define RETRS					7
	#define INIT_CWND				(0.002)
	#define MAX_CWND				(0.010)
	#define FB_TIME					(0.010)
	#define DATA_TIME				(10*CLOCK_SECOND)
	#define DATA_SLOT				4*CLOCK_SECOND/128
	#define DATA_BACKOFF			(0.010)
#else
	#define CARRIER_SENSING_TIME	1*CLOCK_SECOND/128  //7ms
	#define WAIT_ACK_TIME			3*CLOCK_SECOND/128  //3*CLOCK_SECOND/128
	#define DWELL_TIME				3*CLOCK_SECOND/128  // after parent finishes SYNC wait for additional packet for BS
	#define WAIT_PULSE_TIME			7*CLOCK_SECOND/128	// wait time after sending BEACON_P
	#define NEXT_WAKEUP_TIME		(60*CLOCK_SECOND)
	#define MSG_TIME_SLOT			(RTIMER_ARCH_SECOND/128*3)
	#define RETRS					7
	#define INIT_CWND				(0.001)
	#define MAX_CWND				(0.010)
	#define FB_TIME					(0.010)
	#define DATA_TIME				(20*CLOCK_SECOND)
	#define DATA_SLOT				5*CLOCK_SECOND/128
	#define DATA_BACKOFF			(0.010)
#endif

#define BIRD_QUEUE_SIZE 1000

/* Setting */
#define TOPOLOGY				(333)
//#define TOPOLOGY				(3)
#define RANDOM_SEED				(0)
#define BEACON_SUPPRESS			(0)
#define MAX_DATA_SLOT			30
#define PARENT_PRIORITY			1

#define CHECK_RATE		28
#define PC_ON_TIME_R 	(RTIMER_ARCH_SECOND / 160)
#define PC_OFF_TIME_R 	(RTIMER_ARCH_SECOND / CHECK_RATE - PC_ON_TIME_R)
#define PC_ON_TIME 		((CLOCK_SECOND/ 160) > 1 ? (CLOCK_SECOND/ 160):1)
#define PC_OFF_TIME 	(CLOCK_SECOND / CHECK_RATE - PC_ON_TIME)

#endif /* LANADA_APP_PARAM_H_ */
