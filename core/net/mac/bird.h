/*
 * bird.h
 *
 *  Created on: Nov 23, 2015
 *      Author: user
 */

#ifndef CORE_NET_MAC_BIRD_H_
#define CORE_NET_MAC_BIRD_H_

#include "net/rime.h"
#include "param.h"
#include "topology.h"
#include "dclock.h"
#include "birddata.h"

#if DRIFT_NORMAL
#include "lib/random.h"
#include <stdlib.h>
#endif /* DRIFT_NORMAL */

//#if PLATFORM_L != COOJA_L
#define BRID_LOG 	1
#include "birdlog.h"
struct BirdLog birdLog = {0,0,0,0,0,0};
//#endif

enum pc_mode_list {
	PC_NULL, PC_PARENT, PC_CHILD, PC_INIT, PC_END, PC_DATA, PC_SLEEP, PC_START, PC_P_TIMEOUT, PC_C_TIMEOUT
}; // jjh add PC_P&C_TIMEOUT for cpowercycle timeout 0105
enum pc_state_list {
	INIT_WAIT, INIT_CHILD, INIT_PARENT, INIT_DONE,
	CHILD_WAKEUP, CHILD_TX_BEACON, CHILD_NODDING, CHILD_SYNC, CHILD_RX_BEACON_P, CHILD_FINAL_BEACON, CHILD_KEEP_ON, CHILD_BACK_OFF, CHILD_BEACON_SUPPRESS,CHILD_DONE,
	PARENT_WAKEUP, PARENT_TX_BEACON_P,PARENT_WAIT_SYNC_PULSE, PARENT_NODDING, PARENT_RX_BEACON,PARENT_FINAL_BEACON, PARENT_KEEP_ON, PARENT_BACK_OFF,PARENT_DONE,
	TIMEOUT,POWERCYCLE_END
};
enum pc_mode_list pc_mode;
enum pc_state_list pc_state;

/* Dclock */
struct Dclock *dc;


/* DATA */
#if PLATFORM_L != COOJA_L
Birdqueue birdQueue;
BirdData birdData[2*NODE_NUM];
#endif


/* Time synchronization */
clock_time_t next_wakeup_time;

/* topology setting */
//enum tree {SINK, MIDDLE, LEAF};
struct Topo_info
{
	enum tree tree_state;
	uint8_t level;
	uint8_t descent;
	uint8_t max_level;
	uint8_t slot_num;
	uint8_t time_seq;
	rimeaddr_t parent_addr;
	char num_child;
	char num_contention;
	uint8_t child_addr[MAX_CHILD];
	char bridge;
} topo_info;
/* Declare Threads */
struct pt pt;
struct pt pt_init;
struct pt pt_sche;
struct pt pt_data;

/* Timers */
struct ctimer pc_ctimer;
struct ctimer init_ctimer;
struct ctimer data_ctimer;
struct ctimer data_ack_ctimer;
struct ctimer sche_ctimer;
struct ctimer sche_cycle_ctimer;

#if DRIFT_NORMAL // For normal random 1228
double
randn (double mu, double sigma)
{
  double U1, U2, W, mult;
  static double X1, X2;
  static int call = 0;

  if (call == 1)
    {
      call = !call;
      return (mu + sigma * (double) X2);
    }

  do
    {
      U1 = -1 + ((double) rand () / RAND_MAX) * 2;
      U2 = -1 + ((double) rand () / RAND_MAX) * 2;
      W = pow (U1, 2) + pow (U2, 2);
    }
  while (W >= 1 || W == 0);

  mult = sqrt ((-2 * log (W)) / W);
  X1 = U1 * mult;
  X2 = U2 * mult;

  call = !call;

  return (mu + sigma * (double) X1);
}
double randtn (double mu, double sigma)
{
	double ret=0;

	while(1)
	{
		ret = randn (mu, sigma);
		if(ret <= (PERIOD)*(R_DRIFT) && ret >= -1*(PERIOD)*(R_DRIFT))
			return ret;
	}
	return -100;
}
#endif /* DRIFT_NORMAL */

#endif /* CORE_NET_MAC_BIRD_H_ */
