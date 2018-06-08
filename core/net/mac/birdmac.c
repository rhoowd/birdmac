/*
 * Copyright (c) 2010, Swedish Institute of Computer Science.
 * All rights reserved.
 *
 * Redistribution and use in source and binary forms, with or without
 * modification, are permitted provided that the following conditions
 * are met:
 * 1. Redistributions of source code must retain the above copyright
 *    notice, this list of conditions and the following disclaimer.
 * 2. Redistributions in binary form must reproduce the above copyright
 *    notice, this list of conditions and the following disclaimer in the
 *    documentation and/or other materials provided with the distribution.
 * 3. Neither the name of the Institute nor the names of its contributors
 *    may be used to endorse or promote products derived from this software
 *    without specific prior written permission.
 *
 * THIS SOFTWARE IS PROVIDED BY THE INSTITUTE AND CONTRIBUTORS ``AS IS'' AND
 * ANY EXPRESS OR IMPLIED WARRANTIES, INCLUDING, BUT NOT LIMITED TO, THE
 * IMPLIED WARRANTIES OF MERCHANTABILITY AND FITNESS FOR A PARTICULAR PURPOSE
 * ARE DISCLAIMED.  IN NO EVENT SHALL THE INSTITUTE OR CONTRIBUTORS BE LIABLE
 * FOR ANY DIRECT, INDIRECT, INCIDENTAL, SPECIAL, EXEMPLARY, OR CONSEQUENTIAL
 * DAMAGES (INCLUDING, BUT NOT LIMITED TO, PROCUREMENT OF SUBSTITUTE GOODS
 * OR SERVICES; LOSS OF USE, DATA, OR PROFITS; OR BUSINESS INTERRUPTION)
 * HOWEVER CAUSED AND ON ANY THEORY OF LIABILITY, WHETHER IN CONTRACT, STRICT
 * LIABILITY, OR TORT (INCLUDING NEGLIGENCE OR OTHERWISE) ARISING IN ANY WAY
 * OUT OF THE USE OF THIS SOFTWARE, EVEN IF ADVISED OF THE POSSIBILITY OF
 * SUCH DAMAGE.
 *
 * This file is part of the Contiki operating system.
 *
 */

/**
 * \file
 *         A null RDC implementation that uses framer for headers.
 * \author
 *         Adam Dunkels <adam@sics.se>
 *         Niclas Finne <nfi@sics.se>
 */

#include "birdmac.h"

#include "net/mac/nullrdc.h"
#include "net/packetbuf.h"
#include "net/queuebuf.h"
#include "net/netstack.h"
#include "net/rime/rimestats.h"
#include "dev/leds.h"
#include "topology.h"
#include "lib/random.h"

#include <string.h>

#if CONTIKI_TARGET_COOJA
#include "lib/simEnvChange.h"
#endif /* CONTIKI_TARGET_COOJA */

/*--------- Time Constants --------------------------------------------------*/
//#define PC_CLOCK_MAX_TIME (3 * NETSTACK_RDC_CHANNEL_CHECK_RATE)
//#define INTER_PACKET_INTERVAL              RTIMER_ARCH_SECOND / 5000
//#define AFTER_ACK_DETECTECT_WAIT_TIME      RTIMER_ARCH_SECOND / 1000
#define DEFAULT_STROBE_WAIT_TIME (7 * PC_ON_TIME_R / 8)
struct bird_config bird_config = {
		PC_ON_TIME_R,
		PC_OFF_TIME_R,
		4 * PC_ON_TIME_R + PC_OFF_TIME_R,
		DEFAULT_STROBE_WAIT_TIME
};
#ifndef INFINITE_TIME
#define INFINITE_TIME		(1000)*CLOCK_SECOND
#endif /* INFINITE_TIME */

/*--------- PACKET_TYPE constants -------------------------------------------*/
#define DISPATCH                  	0
#define CLEAR	                	0x82
#define TYPE_BEACON             	0x01
#define TYPE_BEACON_ACK         	0x02
#define TYPE_BEACON_P             	0x04
#define TYPE_BEACON_END            	0x88
#define TYPE_SYNC_PULSE            	0x11
#define TYPE_SYNC_ACK            	0x10
#define TYPE_SYNC_FINAL         	0x12
#define TYPE_INIT                	0x84
#define TYPE_BEACON_SYNC_PULSE     	0x08
#define TYPE_DATA 			    	0x72
#define TYPE_DATA_ACK            	0x73

/*---------------------------------------------------------------------------*/
#define MAX_PACKET_SIZE 			100
/*---------------------------------------------------------------------------*/
static volatile unsigned char is_radio_on = 0;
static volatile unsigned char i_bird = 0;
static volatile unsigned char j_bird = 0;
static struct Time_stamp bird_time_stamp;
static struct Bird_wait_info
{
	volatile uint8_t bird_interrupt;
	volatile uint8_t bird_input;
	volatile uint8_t bird_mine;
	volatile uint8_t bird_sibling; // added by jjh for beacon suppression 1227
	volatile uint8_t bird_interference;
	volatile uint8_t bird_wait_packet;
	volatile uint8_t bird_from;

} bird_wait_info = {0,0,0,0,0,0,0}; // jjh 1227

static volatile unsigned char recovery_flag = 0;
static volatile unsigned char FB_flag = 0;
static clock_time_t cycle_timeout = 0;
static volatile unsigned char beacon_backoff_cnt = 0;
static double beacon_backoff_cwnd = 0;
static volatile unsigned char msg_backoff_cnt = 0;
static volatile unsigned char prev_stage = 0;
static volatile unsigned char beacon_supp_cnt = 0; // jjh 0103
static clock_time_t beacon_supp_timeout = 0; // jjh 0105

static clock_time_t dwell_time = 0;
static volatile unsigned char dwell_cnt = 0;
static clock_time_t wait_pulse_time = 0;
static volatile unsigned char wait_pulse_cnt = 0;
static rimeaddr_t temp_addr;
static uint8_t recv_table[MAX_CHILD];
static sleep_min=0;

static volatile unsigned char strobe_cnt = 0;

static int data_cnt = 0;
static int data_num = 0;
static int bird_cycle_cnt = 0;

#if PARAM_L == 1
static int param_l = 0;
#endif

/*---------------------------------------------------------------------------*/
static char bird_send_type(rimeaddr_t *dst,uint8_t type);
static char bird_send_strobe(rimeaddr_t *dst,uint8_t type);
static char bird_send_data(rimeaddr_t *dst);
static char bird_init(void *ptr);
static char bird_data(void *ptr);
static char bird_schedule(void *ptr);
static char cpowercycle(void *ptr);
static void bird_wait(clock_time_t t, uint8_t interrupt, uint8_t wait_type,	struct ctimer *c, void (*f)(void *));
static char bird_wait_r(double sec, char random);
static char bird_send_sync_ack(rimeaddr_t *dst, struct Time_stamp time_stamp);
static char bird_check_recv(char recv);
static void radio_on();
static void radio_off();
static clock_time_t bird_time(clock_time_t time);
/*---------------------------------------------------------------------------*/
static char
bird_init(void *ptr)
{
	PT_BEGIN(&pt_init);

	bird_wait(bird_time(CLOCK_SECOND),0,0,&init_ctimer,bird_init);
	PT_YIELD(&pt_init);

	PRINTF("[bird] init\n");
	radio_on();

	pc_mode = PC_INIT;

	if(topo_info.tree_state == SINK)
		//if(rimeaddr_node_addr.u8[0] == 11) // To test node 11, 12 init phase
	{
		pc_state = INIT_PARENT;
		PRINTF_I("[init] INIT_PARENT\n");
		bird_wait(bird_time(6*CLOCK_SECOND),0,0,&init_ctimer,bird_init);
		PT_YIELD(&pt_init);
		next_wakeup_time = bird_time(INIT_DURATION*CLOCK_SECOND) + dclock_get_time(dc);

		//!! need trigger
	}
	else
	{
		pc_state = INIT_WAIT;
		PRINTF_I("[init] INIT_WAIT\n");
	}

	while(1)
	{
		if(pc_state == INIT_WAIT)
		{
			// wait for INIT packet
			bird_wait(CLOCK_SECOND,1,TYPE_INIT,&init_ctimer,bird_init);
			PT_YIELD(&pt_init);
			if(bird_wait_info.bird_input == TYPE_INIT)
			{
				pc_state = INIT_CHILD;
				PRINTF_I("[init] INIT_WAIT -> INIT_CHILD\n");
			}

		}
		else if(pc_state == INIT_CHILD)
		{
			// send sync_pulse
			bird_send_type(&topo_info.parent_addr,TYPE_SYNC_PULSE);
			// wait for sync ACK
			bird_wait(WAIT_ACK_TIME,1,TYPE_SYNC_ACK,&init_ctimer,bird_init);
			PT_YIELD(&pt_init);
			if(bird_wait_info.bird_input == TYPE_SYNC_ACK)
			{
				// send FINAL for sync ACK
				if(bird_send_type(&topo_info.parent_addr,TYPE_SYNC_FINAL)==0)
				{
					// change state to parent
					pc_state = INIT_PARENT;
					PRINTF_I("[init] INIT_WAIT -> INIT_PARENT\n");
				}
			}
			bird_wait_r(0.3,1); // random wait
		}
		else if(pc_state == INIT_PARENT)
		{
			leds_on(LEDS_BLUE);
			// repeat for all child nodes
			for(i_bird = 0; i_bird < topo_info.num_child; i_bird++)
			{
				PRINTF_I("i%d\n",i_bird);
				j_bird = 0;
				while(j_bird < RETRS)
				{
					// wait for random time + carrier sensing
					bird_wait_r(1,1); // random wait
					bird_wait(CARRIER_SENSING_TIME,0,0,&init_ctimer,bird_init); // carrier sensing
					PT_YIELD(&pt_init);
					if (bird_wait_info.bird_interference == 1) // channel busy
					{
						continue;
					}
					// channel clear -> send TYPE_INIT packet
					temp_addr.u8[1] = 0; temp_addr.u8[0] = topo_info.child_addr[i_bird];
					bird_send_type(&temp_addr,TYPE_INIT);
					// wait for sync pulse
					bird_wait(WAIT_ACK_TIME,1,TYPE_SYNC_PULSE,&init_ctimer,bird_init);
					PT_YIELD(&pt_init);
					if(bird_wait_info.bird_input == TYPE_SYNC_PULSE)
					{
						PRINTF_I("[init] get TYPE_SYNC_PULSE\n");
						// send sync ack
						temp_addr.u8[1] = 0; temp_addr.u8[0] = topo_info.child_addr[i_bird];
						bird_send_sync_ack(&temp_addr,bird_time_stamp);
						// wait for ack
						bird_wait(WAIT_ACK_TIME,1,TYPE_SYNC_FINAL,&init_ctimer,bird_init);
						PT_YIELD(&pt_init);
						if(bird_wait_info.bird_input == TYPE_SYNC_FINAL)
						{
							// if success -> sync with next child
							PRINTF_I("[init] init done with %d\n",topo_info.child_addr[i_bird]);
							break;
						}
					}
					// else repeat for 7 times -> skip this child
					j_bird++;
#if DEBUG_INIT
					if(j_bird==RETRS)
						PRINTF_I("[init] init fail with %d (skip)\n",topo_info.child_addr[i_bird]);
					else
						PRINTF_I("[init] init retry (%d) times\n",j_bird);
#endif
				}
				//bird_wait_r(1,0); // random wait
				bird_wait(bird_time(128),0,0,&init_ctimer,bird_init); // carrier sensing
				PT_YIELD(&pt_init);

			}
			// done with all child -> change state to INIT_DONE
			pc_state = INIT_DONE;
			radio_off();
			leds_off(7);
			PRINTF_I("[init] INIT_PARENT -> INIT_DONE\n");
		}
		else if(pc_state == INIT_DONE) // it is for covering skipped child nodes.
		{
			// send INIT_PACKET repeatly with random interval
			// react to sync_pulse
			// do this until end of init phase just in case
#if PLATFORM_L==Z1MOTE_L
			birdtrace_log("i");
#endif
			bird_wait(next_wakeup_time-dclock_get_time(dc),0,0,&init_ctimer,bird_init);
			PT_YIELD(&pt_init);
			break;
		}
	}
	// init done
	pc_mode = PC_SLEEP;
	ctimer_set(&sche_ctimer, 1,(void (*)(void *))bird_schedule, NULL);
	PT_END(&pt_init);
	return 1;
}
/*---------------------------------------------------------------------------*/
static void
bird_wait(clock_time_t t, uint8_t interrupt, uint8_t wait_type,
		struct ctimer *c, void (*f)(void *))
{
	if (t == 0) // interrupt
	{
		ctimer_set(c, 0, (void (*)(void *))f, NULL);
	}
	else
	{
		bird_wait_info.bird_interrupt=interrupt;
		bird_wait_info.bird_input = CLEAR;
		bird_wait_info.bird_interference = 0;
		bird_wait_info.bird_mine = 0;
		bird_wait_info.bird_sibling = 0;
		bird_wait_info.bird_wait_packet = wait_type;
		bird_wait_info.bird_from = 0; // bird_from init jjh 1227
		ctimer_set(c, t, (void (*)(void *))f, NULL);
	}

}
/*---------------------------------------------------------------------------*/
static char
bird_wait_r(double sec,char random)
{
	rtimer_clock_t t;
	if(random==1)
	{
		t = random_rand()%(rtimer_clock_t)(RTIMER_ARCH_SECOND*sec) + RTIMER_NOW();
	}
	else
	{
		t = (rtimer_clock_t)(RTIMER_ARCH_SECOND*sec) + RTIMER_NOW();
	}
	while(RTIMER_CLOCK_LT(RTIMER_NOW(), t))    {}
	return 1;
}
/*---------------------------------------------------------------------------*/
static char
cpowercycle(void *ptr)
{
	PT_BEGIN(&pt);

	char pc_ret;


	while(pc_mode != PC_START) // wait until init done.
	{
		bird_wait(INFINITE_TIME,0,0,&pc_ctimer,cpowercycle);
		PT_YIELD(&pt);
	}

	while(1) {

		while(pc_mode != PC_PARENT && pc_mode != PC_CHILD) // wait until being scheduled.
		{
			bird_wait(INFINITE_TIME,0,0,&pc_ctimer,cpowercycle);
			PT_YIELD(&pt);
		}

		birdLogClear(&birdLog); // jjh 0103 for print log

		PRINTF("[bird] Start cycle as %d\n",pc_mode);
		pc_state=(pc_mode==PC_CHILD)?CHILD_WAKEUP:PARENT_WAKEUP;

		// init variables
		beacon_backoff_cnt = 0;
		msg_backoff_cnt = 0;
		dwell_cnt = 0;
		wait_pulse_cnt = 0;
		strobe_cnt = 0;
		bird_check_recv(-1); // init
		FB_flag = 0;
		recovery_flag = 0;
		beacon_supp_cnt = 0; // 0103 jjh
#if PARAM_L == 1
		if(topo_info.tree_state != SINK)
			if(pc_mode==PC_CHILD)
				param_l = 0;
#endif
		cycle_timeout = dclock_get_time(dc);
		while(1)
		{
			// time out here
			// now this is for test
			if(FB_flag == 0)
			{
				if((int)(cycle_timeout+2*TIME_SLOT + PC_OFF_TIME*3 -dclock_get_time(dc)) < 0)
				{
					if(pc_state == CHILD_DONE || pc_state == PARENT_DONE)
					{
						radio_off();
						break;
					}
					else if(pc_state == CHILD_NODDING || pc_state == PARENT_NODDING)
					{
						PRINTF_D("[cycle] FB on %d \n",pc_state);
						printf("[FB] on %d \n",pc_state);
						FB_flag = 1;
//#if DATA_ON && PLATFORM_L != COOJA_L
//						birdLogCom(BIRD_LOG_FB,&birdLog);
//#endif
					}
				}
			}
			if(pc_mode==PC_CHILD)
			{
				if(FB_flag > 0)
				{
					// cover final beacon
					if(FB_flag==1)
					{
						FB_flag++;
						bird_wait_r(FB_TIME,1);
						pc_state = CHILD_WAKEUP;
						PRINTF_S("[state] FINAL_BEACON -> CHILD_WAKEUP\n");
						dwell_cnt = 0; // in FB, init dwell_cnt
					}
					else
					{
						if(pc_state == CHILD_NODDING)
						{
							PRINTF_D("[cycle] timeout!!! %d \n",pc_state);
							printf("FAIL\n");
							recovery_flag = 1;
//#if DATA_ON && PLATFORM_L != COOJA_L
							birdLogCom(BIRD_LOG_RECOVERY,&birdLog);
//#endif
							radio_off();
							break;
						}
					}
				}

				if(pc_state == CHILD_WAKEUP)
				{
					PRINTF("CHILD_WAKEUP DEBUG\n");

					radio_on();
					// wait for 7ms
					//					bird_wait(CARRIER_SENSING_TIME,0,0,&pc_ctimer,cpowercycle);
					bird_wait(CARRIER_SENSING_TIME,1,TYPE_BEACON_P,&pc_ctimer,cpowercycle);
					PT_YIELD(&pt);
					// if clean -> CHILD_TX_BEACON
					if(bird_wait_info.bird_interference==0)
					{
						PRINTF_D("[cycle] Channel clear\n");
						PRINTF_S("[state] CHILD_WAKEUP -> CHILD_TX_BEACON\n");
						pc_state = CHILD_TX_BEACON;
					}
					// busy -> defer tx
					else
					{

						if(bird_wait_info.bird_input == TYPE_BEACON_P
								&& bird_wait_info.bird_from == topo_info.parent_addr.u8[0])
						{
							PRINTF_S("[state] CHILD_WAKEUP -> CHILD_RX_BEACON_P\n");
							pc_state = CHILD_RX_BEACON_P;
							continue;
						}
//						else if(bird_wait_info.bird_input == TYPE_BEACON_ACK
//								&& bird_wait_info.bird_from == topo_info.parent_addr.u8[0])  // kkk: fake
//						{
//							PRINTF_S("[state] CHILD_WAKEUP -> CHILD_RX_BEACON_P\n");
//							pc_state = CHILD_RX_BEACON_P;
//							continue;
//						}
#if BEACON_SUPPRESS == 0
						PRINTF_D("[cycle] Channel busy\n");
						PRINTF_S("[state] CHILD_WAKEUP (stay)\n");
						continue;
#else
						else if(bird_wait_info.bird_from == topo_info.parent_addr.u8[0]
							 || bird_wait_info.bird_sibling) // BS by jjh 1227
						{
//							printf("sibling %d bird_from %d\n",bird_wait_info.bird_sibling,bird_wait_info.bird_from); 1227 for debug
							beacon_supp_cnt = 1;
							PRINTF_S("[state] CHILD_WAKEUP -> CHILD_BEACON_SUPPRESS\n");
							pc_state = CHILD_BEACON_SUPPRESS;
							beacon_supp_timeout = dclock_get_time(dc);
							continue;
						}
						// BS ON -> goto CHILD_BEACON_SUPPRESS
#endif
					}
				}
				else if(pc_state == CHILD_TX_BEACON)
				{
					// send strobe
//					bird_wait(10,1,TYPE_BEACON_P,&pc_ctimer,cpowercycle);
//					PT_YIELD(&pt);

					pc_ret = bird_send_strobe(&topo_info.parent_addr,TYPE_BEACON);
					PRINTF_D("[cycle] strobe return %d\n",pc_ret);

					if(pc_ret == 3) // collision
					{
						PRINTF_S("[state] CHILD_TX_BEACON -> CHILD_BACK_OFF\n");
						pc_state = CHILD_BACK_OFF;
						continue;
					}

					radio_on();
					bird_wait(PC_ON_TIME,1,TYPE_BEACON_ACK,&pc_ctimer,cpowercycle);
					PT_YIELD(&pt);

					if(bird_wait_info.bird_input == CLEAR) // NO_ACK
					{
						PRINTF_S("[state] CHILD_TX_BEACON -> CHILD_NODDING\n");
						pc_state = CHILD_NODDING;
					}
					else if(bird_wait_info.bird_input == TYPE_BEACON_ACK) // got ack
					{
						PRINTF_S("[state] CHILD_TX_BEACON -> CHILD_SYNC\n");
						pc_state = CHILD_SYNC;
					}
					else if(bird_wait_info.bird_input == TYPE_BEACON_P
							&& bird_wait_info.bird_from == topo_info.parent_addr.u8[0])
					{
						PRINTF_S("[state] CHILD_WAKEUP -> CHILD_RX_BEACON_P\n");
						pc_state = CHILD_RX_BEACON_P;
						continue;
					}



//					// NO_ACK
//					if(pc_ret == 0)
//					{
//						PRINTF_S("[state] CHILD_TX_BEACON -> CHILD_NODDING\n");
//						pc_state = CHILD_NODDING;
//					}
//					else if(pc_ret == 1) // got ack
//					{
//						PRINTF_S("[state] CHILD_TX_BEACON -> CHILD_SYNC\n");
//						pc_state = CHILD_SYNC;
//					}
//					else if(pc_ret == 3) // collision
//					{
//						PRINTF_S("[state] CHILD_TX_BEACON -> CHILD_BACK_OFF\n");
//						pc_state = CHILD_BACK_OFF;
//					}
//					else
//					{
//						PRINTF_S("[state] CHILD_TX_BEACON -> CHILD_DONE\n");
//						pc_state = CHILD_DONE;
//					}


				}
				else if(pc_state == CHILD_NODDING)
				{
					// repeat on and off // kkk: turn off child's nodding
//					radio_off();
//					bird_wait(PC_OFF_TIME,0,0,&pc_ctimer,cpowercycle);
//					PT_YIELD(&pt);

					radio_on();
					bird_wait(PC_ON_TIME,1,TYPE_BEACON_P,&pc_ctimer,cpowercycle);
					PT_YIELD(&pt);

					// recv beacon_p from parent
					if(bird_wait_info.bird_input == TYPE_BEACON_P) // 1226 Should check this packet is from my parent
					{
						PRINTF_S("[state] CHILD_NODDING -> CHILD_RX_BEACON_P\n");
						pc_state = CHILD_RX_BEACON_P;
					}
					else if(bird_wait_info.bird_input == TYPE_BEACON_ACK) // 1226 Should check this packet is from my parent
					{
						PRINTF_S("[state] CHILD_NODDING -> CHILD_RX_BEACON_P\n");
						pc_state = CHILD_RX_BEACON_P;
					}


					// time out

				}
				else if(pc_state == CHILD_SYNC)
				{
					i_bird = 0;
					msg_backoff_cnt=0;
					beacon_backoff_cwnd = INIT_CWND;
					while(i_bird<RETRS)
					{
//#if DATA_ON && PLATFORM_L != COOJA_L
						if(i_bird != 0)
							birdLogCom(BIRD_LOG_MSG,&birdLog);
//#endif
						// send sync_pulse
						bird_send_type(&topo_info.parent_addr,TYPE_SYNC_PULSE);
						// wait for sync ACK
						bird_wait(WAIT_ACK_TIME,1,TYPE_SYNC_ACK,&pc_ctimer,cpowercycle);
						PT_YIELD(&pt);
						if(bird_wait_info.bird_input == TYPE_SYNC_ACK && bird_wait_info.bird_mine == 1) // kkk: for me? or for others?
						{
							// send FINAL for sync ACK
							if(bird_send_type(&topo_info.parent_addr,TYPE_SYNC_FINAL)==0)
							{

								// change state to parent
								break;
							}
						}
						else //no ack -> collision -> back off
						{
							//back off
							bird_wait_r(beacon_backoff_cwnd,1);
							beacon_backoff_cwnd = (2*beacon_backoff_cwnd > MAX_CWND)?MAX_CWND:2*beacon_backoff_cwnd;
							beacon_backoff_cnt++;
							PRINTF_D("[cycle] msg collision back off (%d)\n",beacon_backoff_cnt);
						}
						PRINTF_D("[cycle] retrs sync (%d)\n",i_bird);
						if(prev_stage == CHILD_RX_BEACON_P) // Meaningless? or not? jjh
						{
							// wait for some time
							prev_stage = 0;
						}
						i_bird++; // 1224 CHILD single fail
					}
					if(i_bird<RETRS) // success
					{
						PRINTF_S("[state] CHILD_SYNC -> CHILD_DONE\n");
						pc_state = CHILD_DONE;
					}
					else // fail 7 times -> go back to nodding for FB
					{
						PRINTF_S("[state] CHILD_SYNC -> CHILD_NODDING\n");
						pc_state = CHILD_NODDING;
					}
				}
				else if(pc_state == CHILD_RX_BEACON_P)
				{

					radio_off();
					rtimer_clock_t t;
					rtimer_clock_t t_wait = (topo_info.slot_num-1) * MSG_TIME_SLOT; // + (RTIMER_ARCH_SECOND/128); // kkk: Do we need Delay?

					t = RTIMER_NOW() + t_wait;
					while(RTIMER_CLOCK_LT(RTIMER_NOW(), t))    {}
					radio_on();
					prev_stage = CHILD_RX_BEACON_P;
					// wait for certain time
					PRINTF_S("[state] CHILD_RX_BEACON_P -> CHILD_SYNC\n");
					pc_state = CHILD_SYNC;
				}
/*				else if(pc_state == CHILD_FINAL_BEACON)
				{

				}
				else if(pc_state == CHILD_KEEP_ON)
				{

				}*/
				else if(pc_state == CHILD_BACK_OFF)
				{
					if(beacon_backoff_cnt >= RETRS)
					{
						PRINTF("fail to send beacon due to collision\n");
						PRINTF_S("[state] CHILD_BACK_OFF -> CHILD_NODDING (ret %d)\n",beacon_backoff_cnt);
						pc_state = CHILD_NODDING;
						beacon_backoff_cnt = 0;
						continue;
					}
					// update cwnd
					if(beacon_backoff_cnt == 0)
					{
						beacon_backoff_cwnd = INIT_CWND;
					}
					else
					{
						beacon_backoff_cwnd = (2*beacon_backoff_cwnd > MAX_CWND)?MAX_CWND:2*beacon_backoff_cwnd;
					}
					// increase counter
					beacon_backoff_cnt++;
					bird_wait_r(beacon_backoff_cwnd,1);
					PRINTF_S("[state] CHILD_BACK_OFF -> CHILD_WAKEUP (ret %d)\n",beacon_backoff_cnt);
					pc_state = CHILD_WAKEUP;
//#if DATA_ON && PLATFORM_L != COOJA_L
					birdLogCom(BIRD_LOG_BEACON,&birdLog);
//#endif
				}
				else if(pc_state == CHILD_BEACON_SUPPRESS)
				{
					if((int)(beacon_supp_timeout + 3*PC_OFF_TIME_R - dclock_get_time(dc)) < 0) // jjh for avoid loop 0105
					{
						PRINTF_S("[state] CHILD_BEACON_SUPPRESS -> CHILD_NODDIG\n");
						PRINTF("BEACON SUPP TIME OUT!\n");
						pc_state = CHILD_NODDING;
						continue;
					}
					if(bird_wait_info.bird_input == TYPE_SYNC_FINAL)
					{
						PRINTF_S("[state] CHILD_BEACON_SUPPRESS -> CHILD_SYNC\n");
						pc_state = CHILD_SYNC;
						continue;
						// Move to SYNC pulse tx state
					}
					else if(bird_wait_info.bird_input == TYPE_BEACON_END) // 1227 jjh
					{
						PRINTF_S("[state] CHILD_BEACON_SUPPRESS -> CHILD_NODDING\n");
						pc_state = CHILD_NODDING;
						continue;
						// Move to CHILD_NODDIG
					}
					else if(bird_wait_info.bird_input == TYPE_BEACON_P)
					{
						PRINTF_S("[state] CHILD_BEACON_SUPPRESS -> CHILD_RX_BEACON_P\n");
						pc_state = CHILD_RX_BEACON_P;
						continue;
					}
					bird_wait(PC_ON_TIME,1,TYPE_SYNC_FINAL,&pc_ctimer,cpowercycle);
					PT_YIELD(&pt);
					// Wait until previous communication is finished
				}
				else if(pc_state == CHILD_DONE)
				{
					radio_off();
					break;
				}

			}
			else if(pc_mode==PC_PARENT)
			{

				if(FB_flag > 0)
				{
					// cover final beacon
					if(FB_flag==1)
					{
						FB_flag++;
						bird_wait_r(FB_TIME,1);
						pc_state = PARENT_WAKEUP;
						dwell_cnt=0;
					}
					else
					{
						if(pc_state == PARENT_NODDING)
						{
							PRINTF_D("[cycle] timeout!!! %d \n",pc_state);
							printf("FAIL\n");
							radio_off();
//#if DATA_ON && PLATFORM_L != COOJA_L
							birdLogCom(BIRD_LOG_RECOVERY,&birdLog);
//#endif
							break;
						}
					}
				}

				if(pc_state == PARENT_WAKEUP)
				{
					radio_on();
					// wait for 7ms
					bird_wait(CARRIER_SENSING_TIME,0,0,&pc_ctimer,cpowercycle);
					PT_YIELD(&pt);
					// if clean -> PARENT_TX_BEACON_P
					if(bird_wait_info.bird_interference==0)
					{
						PRINTF_D("[cycle] Channel clear\n");
						PRINTF_S("[state] PARENT_WAKEUP -> PARENT_TX_BEACON_P\n");
						pc_state = PARENT_TX_BEACON_P;
						beacon_supp_cnt = 0;
					}
					// busy -> defer tx
					else
					{
#if BEACON_SUPPRESS == 0
						PRINTF_D("[cycle] Channel busy\n");
						PRINTF_S("[state] PARENT_WAKEUP (stay)\n");
						continue;
#else
						if(bird_wait_info.bird_input == TYPE_BEACON)
						{
							rimeaddr_t addr;
							addr.u8[1] = 0;
							addr.u8[0] = bird_wait_info.bird_from;
							if(bird_send_type(&addr,TYPE_BEACON_ACK)==0)
							{
								PRINTF_S("[state] PARENT_WAKEUP -> PARENT_RX_BEACON\n");
								pc_state = PARENT_RX_BEACON;
								prev_stage = PARENT_WAKEUP;
							}
							beacon_supp_cnt = 1; // jjh 0103

						}
						else if(bird_wait_info.bird_input == TYPE_SYNC_PULSE) // BEACON SUPP for SYNC PULSE 1228
						{
							PRINTF_S("[state] PARENT_WAKEUP -> PARENT_WAIT_SYNC_PULSE\n");
							pc_state = PARENT_WAIT_SYNC_PULSE;
							prev_stage = PARENT_WAKEUP;
							beacon_supp_cnt = 1; // jjh 0103
						}
						// BS ON -> goto PARENT_BEACON_SUPPRESS
#endif
					}
				}
				else if(pc_state == PARENT_TX_BEACON_P)
				{
					// send strobe
					pc_ret = bird_send_strobe(&rimeaddr_null,TYPE_BEACON_P);
					PRINTF_D("[cycle] strobe return %d\n",pc_ret);

					// NO_ACK
					if(pc_ret == 0)
					{
						PRINTF_S("[state] PARENT_TX_BEACON_P -> PARENT_WAIT_SYNC_PULSE\n");
						pc_state = PARENT_WAIT_SYNC_PULSE;
					}
					else if(pc_ret == 4)
					{
						//During tx beacon, it recv child's SYNC pulse 1229 jjh
						PRINTF_S("[state] PARENT_TX_BEACON_P -> PARENT_WAIT_SYNC_PULSE\n");
						prev_stage = PARENT_TX_BEACON_P; // To tx beacon again
						pc_state = PARENT_WAIT_SYNC_PULSE;
					}
					else
					{
						PRINTF_S("[state] PARENT_TX_BEACON_P -> PARENT_BACK_OFF\n");
						pc_state = PARENT_BACK_OFF;
					}
				}
				else if(pc_state == PARENT_WAIT_SYNC_PULSE)
				{
					if(wait_pulse_cnt == 0)
						wait_pulse_time = dclock_get_time(dc);

					if(dwell_cnt == 1) // check channel for dwell time if wait time out
						bird_wait(DWELL_TIME,1,TYPE_BEACON_SYNC_PULSE,&pc_ctimer,cpowercycle);
					else
						bird_wait(WAIT_ACK_TIME,1,TYPE_BEACON_SYNC_PULSE,&pc_ctimer,cpowercycle); // jjh 1227
					PT_YIELD(&pt);
					if(bird_wait_info.bird_input == TYPE_SYNC_PULSE)
					{
						PRINTF_D("[cycle] get TYPE_SYNC_PULSE\n");
						// send sync ack
						temp_addr.u8[1] = 0;
						temp_addr.u8[0] = bird_wait_info.bird_from;
						PRINTF_D("[cycle] send TYPE_SYNC_ACK to %d\n",temp_addr.u8[0]);
						bird_send_sync_ack(&temp_addr,bird_time_stamp);
						// wait for ack
						bird_wait(WAIT_ACK_TIME,1,TYPE_SYNC_FINAL,&pc_ctimer,cpowercycle);
						PT_YIELD(&pt);
						if(bird_wait_info.bird_input == TYPE_SYNC_FINAL)
						{
							// if success -> sync with next child
							// update table
							if(bird_check_recv(0))
							{
								PRINTF_S("[state] PARENT_WAIT_SYNC_PULSE -> PARENT_DONE\n");
								pc_state = PARENT_DONE;
							}
							dwell_cnt = 0; // Change order for dwell time can be applied jjh 1228
						}
					}
					else if(bird_wait_info.bird_input == TYPE_BEACON)
					{
						temp_addr.u8[1] = 0;
						temp_addr.u8[0] = bird_wait_info.bird_from;
						bird_send_type(&temp_addr,TYPE_BEACON_ACK);
						dwell_cnt = 0; // Give the node one more chance if it gets BEACON jjh 1228
						continue; // added for handling #4 case jjh 1228
					}
					if(dwell_cnt == 1 && bird_wait_info.bird_interference == 0)
					{
						wait_pulse_cnt = 0;
						if(bird_check_recv(0))
						{
							PRINTF_S("[state] PARENT_WAIT_SYNC_PULSE -> PARENT_DONE\n");
							pc_state = PARENT_DONE;
							dwell_cnt = 0;
							continue;
						}
						else if(prev_stage == PARENT_WAKEUP || prev_stage == PARENT_TX_BEACON_P)
						{
							prev_stage = 0;
							pc_state = PARENT_WAKEUP; // Since Parent does not transmit beacon go back to parent_wakeup
							continue;
						}
						PRINTF_S("[state] PARENT_WAIT_SYNC_PULSE -> PARENT_NODDING\n");
						pc_state = PARENT_NODDING;
					}
					else if((int)(wait_pulse_time + WAIT_PULSE_TIME * topo_info.num_child + (CLOCK_SECOND/128) - dclock_get_time(dc))<0)
					{
						dwell_cnt = 1;
					}
					wait_pulse_cnt++;

				}
				else if(pc_state == PARENT_NODDING)
				{
					// repeat on and off // kkk: Turn off parent's nodding
//					radio_off();
//					bird_wait(PC_OFF_TIME,0,0,&pc_ctimer,cpowercycle);
//					PT_YIELD(&pt);

					radio_on();
					bird_wait(PC_ON_TIME,1,TYPE_BEACON,&pc_ctimer,cpowercycle);
					PT_YIELD(&pt);

					// recv beacon from child
					if(bird_wait_info.bird_input == TYPE_BEACON)
					{
						rimeaddr_t addr;
						addr.u8[1] = 0;
						addr.u8[0] = bird_wait_info.bird_from;
						if(bird_send_type(&addr,TYPE_BEACON_ACK)==0)
						{
							PRINTF_S("[state] PARENT_NODDING -> PARENT_RX_BEACON\n");
							pc_state = PARENT_RX_BEACON;
						}
					}
					// time out

				}
				else if(pc_state == PARENT_RX_BEACON)
				{
					if(dwell_cnt == 0)
					{
						dwell_time = dclock_get_time(dc);
					}
					bird_wait(WAIT_ACK_TIME,1,TYPE_SYNC_PULSE,&pc_ctimer,cpowercycle);
					PT_YIELD(&pt);
					if(bird_wait_info.bird_input == TYPE_SYNC_PULSE)
					{
						PRINTF_D("[cycle] get TYPE_SYNC_PULSE\n");
						// send sync ack
						temp_addr.u8[1] = 0;
						temp_addr.u8[0] = bird_wait_info.bird_from;
						PRINTF_D("[cycle] send TYPE_SYNC_ACK to %d\n",temp_addr.u8[0]);
						bird_send_sync_ack(&temp_addr,bird_time_stamp);
						// wait for ack
						bird_wait(WAIT_ACK_TIME,1,TYPE_SYNC_FINAL,&pc_ctimer,cpowercycle);
						PT_YIELD(&pt);
						if(bird_wait_info.bird_input == TYPE_SYNC_FINAL)
						{
							// if success -> sync with next child
							// update table
							if(bird_check_recv(0))
							{
								PRINTF_S("[state] PARENT_RX_BEACON -> PARENT_DONE\n");
								pc_state = PARENT_DONE;

							}
							dwell_cnt = 0; // 1227 jjh dwell time
							continue;
						}
					}
					else if(bird_wait_info.bird_input == TYPE_BEACON) // jjh 0106 FIX!
					{
						rimeaddr_t addr;
						addr.u8[1] = 0;
						addr.u8[0] = bird_wait_info.bird_from;
						if(bird_send_type(&addr,TYPE_BEACON_ACK)==0)
						{
							PRINTF_S("[state] PARENT_RX_BEACON -> PARENT_RX_BEACON\n");
							continue;
						}
					}
					if((int)(dwell_time + DWELL_TIME - dclock_get_time(dc))<0)
					{
						PRINTF_S("[state] PARENT_RX_BEACON -> PARENT_NODDING\n");
						dwell_cnt = 0;
						if(prev_stage == PARENT_WAKEUP)
						{
							prev_stage = 0;
							pc_state = PARENT_WAKEUP; // Since Parent does not transmit beacon go back to parent_wakeup
							continue;
						}
						pc_state = PARENT_NODDING; // Change order
						continue; // Add continue
					}
					dwell_cnt++;
				}
				else if(pc_state == PARENT_KEEP_ON)
				{

				}
				else if(pc_state == PARENT_BACK_OFF)
				{
					if(beacon_backoff_cnt >= RETRS)
					{
						PRINTF("fail to send beacon due to collision\n");
						PRINTF_S("[state] PARENT_BACK_OFF -> PARENT_NODDING (%d)\n",beacon_backoff_cnt);
						pc_state = PARENT_NODDING;
						beacon_backoff_cnt = 0;
						continue;
					}
					// update cwnd
					if(beacon_backoff_cnt == 0)
					{
						beacon_backoff_cwnd = INIT_CWND;
					}
					else
					{
						beacon_backoff_cwnd = (2*beacon_backoff_cwnd > MAX_CWND)?MAX_CWND:2*beacon_backoff_cwnd;
					}
					// increase counter
					beacon_backoff_cnt++;
					bird_wait_r(beacon_backoff_cwnd,1);
					PRINTF_S("[state] PARENT_BACK_OFF -> PARENT_WAKEUP (%d)\n",beacon_backoff_cnt);
					pc_state = PARENT_WAKEUP;
//#if DATA_ON && PLATFORM_L != COOJA_L
					birdLogCom(BIRD_LOG_BEACON,&birdLog);
//#endif
				}
				else if(pc_state == PARENT_DONE)
				{
					radio_off();
					break;
				}
			}
			else if(pc_mode == PC_P_TIMEOUT || pc_mode == PC_C_TIMEOUT) // For cpowercycle timeout
			{
				printf("FAIL2\n");
				recovery_flag = 1;
				//#if DATA_ON && PLATFORM_L != COOJA_L
				birdLogCom(BIRD_LOG_RECOVERY,&birdLog);
				radio_off();
				break;
			}
		}

		PRINTF("[bird] End cycle as %d\n",pc_mode);
#if PARAM_L == 1
		printf("[param] %d\n",param_l);
#endif

//#if PLATFORM_L ==  COOJA_L
//		printf("[data] %3d  %4d  ",rimeaddr_node_addr.u8[0],bird_cycle_cnt);
//		printf("%s",(pc_mode==1 || pc_mode==8)?"PARENT":"CHILD ");
//		birdLogPrint(&birdLog); // jjh 0103 for print
//		printf("%d\n",beacon_supp_cnt);
//#endif
//
		pc_mode = PC_SLEEP;

	}
	PT_END(&pt);
}
/*---------------------------------------------------------------------------*/
static char
bird_data(void *ptr)
{

	PT_BEGIN(&pt_data);

	PRINTF_DATA("[data] start\n");
	while(1)
	{

		while(pc_mode != PC_DATA) // wait until being scheduled.
		{
			bird_wait(INFINITE_TIME,0,0,&data_ctimer,bird_data);
			PT_YIELD(&pt_data);
		}
		PRINTF_DATA("[data] data scheduled\n");

		data_cnt = 0;

		radio_on();
		bird_wait((MAX_LEVEL-topo_info.level)*DATA_SLOT,0,0,&data_ctimer,bird_data);
		PT_YIELD(&pt_data);

		while(1)
		{
			data_cnt++;

			leds_off(7);
			leds_on(LEDS_GREEN);
			radio_on();
			bird_wait(DATA_SLOT,0,0,&data_ctimer,bird_data);
			PT_YIELD(&pt_data);
			radio_off();
			if(topo_info.tree_state == SINK /*|| topo_info.bridge == 1*/)
			{
				birddata_queue_print(&birdQueue);
				birddata_queue_clear(&birdQueue);
			}
			leds_off(7);
			leds_on(LEDS_BLUE);
			bird_wait(DATA_SLOT,0,0,&data_ctimer,bird_data);

			if(topo_info.tree_state != SINK /*&& topo_info.bridge == 0*/)
			{
				if(topo_info.slot_num == (data_cnt%topo_info.num_contention+1))
				{
					if (birddata_queue_size(&birdQueue) > 0)
					{
						radio_on();
						bird_wait_r(DATA_BACKOFF,1);
						data_num = bird_send_data(&topo_info.parent_addr);
					}
				}
				bird_wait(WAIT_ACK_TIME,1,TYPE_DATA_ACK,&data_ack_ctimer,bird_data);
				PT_YIELD(&pt_data);
				if(bird_wait_info.bird_interference == 1
						&& bird_wait_info.bird_input == TYPE_DATA_ACK
						&& bird_wait_info.bird_mine == 1
						&& data_num > 0)
				{
					PRINTF_DATA("[data] get data ACK\n");
					birddata_queue_free(data_num, &birdQueue);
				}
				radio_off();
			}
			PT_YIELD(&pt_data);

			leds_off(7);
			radio_off();
			bird_wait(DATA_SLOT,0,0,&data_ctimer,bird_data);
			PT_YIELD(&pt_data);

			if(data_cnt >MAX_DATA_SLOT)
				break;
		}
		pc_mode = PC_SLEEP;
		radio_off();

		if(topo_info.tree_state == SINK /*|| topo_info.bridge == 1*/)
		{
			birddata_queue_print(&birdQueue);
			birddata_queue_clear(&birdQueue);
		}

	}


	PT_END(&pt_data);
}
/*---------------------------------------------------------------------------*/

static char
bird_schedule(void *ptr)
{
	PT_BEGIN(&pt_sche);

	// wait for init done //
	while(1)
	{
		ctimer_set(&sche_ctimer, INFINITE_TIME,
				(void (*)(void *))bird_schedule, NULL);
		PT_YIELD(&pt_sche);
		if(pc_mode == PC_SLEEP)
			break;
	}
	PRINTF("[bird] schedule start (init_done)\n");

	// trigger power cycle
	pc_mode = PC_START;
	bird_wait(0,0,0,&pc_ctimer,cpowercycle);
	ctimer_set(&sche_ctimer,bird_time(CLOCK_SECOND),(void (*)(void *))bird_schedule, NULL);
	PT_YIELD(&pt_sche);

	//schedule start
	while(1) {
		bird_cycle_cnt++;
#if EXPERIMENT == 0
		// random clock drift simulate
#if DRIFT_NORMAL
		clock_time_t random_time=(randtn(0.0,SIGMA)*CLOCK_SECOND) + MAX_CLOCK_DRIFT; // added for normal dist jjh 1228
#else
		clock_time_t random_time=random_rand()%(2*MAX_CLOCK_DRIFT);
#endif
		ctimer_set(&sche_cycle_ctimer,random_time,
				(void (*)(void *))bird_schedule, NULL);
		PT_YIELD(&pt_sche);
#endif

		// sleep until next schedule
		ctimer_set(&sche_ctimer,(MAX_LEVEL+2)*TIME_SLOT*TIME_SLOT_NUM,(void (*)(void *))bird_schedule, NULL);

		if(topo_info.tree_state == SINK)
		{
#if PARAM_L == 1
			// set param here
			param_l = 10;
#endif
			leds_on(LEDS_BLUE);
			next_wakeup_time = dclock_get_time(dc) + bird_time(NEXT_WAKEUP_TIME);
			ctimer_set(&sche_cycle_ctimer, TIME_SLOT_NUM*TIME_SLOT,
					(void (*)(void *))bird_schedule, NULL);
			pc_mode = PC_PARENT;
			bird_wait(0,0,0,&pc_ctimer,cpowercycle);
			PT_YIELD(&pt_sche);
			if(pc_state != PARENT_DONE)
			{
				PRINTF("PARENT: cpowercycle timeout!\n");
				pc_mode = PC_P_TIMEOUT;
				ctimer_set(&sche_cycle_ctimer, PC_OFF_TIME,
									(void (*)(void *))bird_schedule, NULL);
				bird_wait(0,0,0,&pc_ctimer,cpowercycle);
				PT_YIELD(&pt_sche);
			}
			leds_off(LEDS_BLUE);
		}
		else
		{
			int par = 0;
			if(topo_info.parent_addr.u8[0]==2)
				par = 0;
			else if(topo_info.parent_addr.u8[0]==3)
				par = 1;
			else if(topo_info.parent_addr.u8[0]==4)
				par = 2;

			next_wakeup_time = dclock_get_time(dc) + bird_time(NEXT_WAKEUP_TIME); // for recovery
			// level scheduling
			ctimer_set(&sche_cycle_ctimer,(topo_info.level-1 + par)*TIME_SLOT_NUM*TIME_SLOT+2,
					(void (*)(void *))bird_schedule, NULL);
			PT_YIELD(&pt_sche);

			// scheduled as child
			leds_on(LEDS_GREEN);
			ctimer_set(&sche_cycle_ctimer, TIME_SLOT_NUM*TIME_SLOT-4,
					(void (*)(void *))bird_schedule, NULL);
			pc_mode = PC_CHILD;
			bird_wait(0,0,0,&pc_ctimer,cpowercycle);
			PT_YIELD(&pt_sche);
			if(pc_state != CHILD_DONE)
			{
				PRINTF("CHILD: cpowercycle timeout!\n");
				pc_mode = PC_C_TIMEOUT;
				ctimer_set(&sche_cycle_ctimer, PC_OFF_TIME,
									(void (*)(void *))bird_schedule, NULL);
				bird_wait(0,0,0,&pc_ctimer,cpowercycle);
				PT_YIELD(&pt_sche);
			}
			leds_off(LEDS_GREEN);


			if (recovery_flag == 1)
			{
				PRINTF("[bird] recovery on\n");
				recovery_flag = 0;
			}


			if(topo_info.tree_state == MIDDLE)
			{
				ctimer_set(&sche_cycle_ctimer,(rimeaddr_node_addr.u8[0]-2)*TIME_SLOT_NUM*TIME_SLOT+2,
						(void (*)(void *))bird_schedule, NULL);
				PT_YIELD(&pt_sche);
				leds_on(LEDS_BLUE);
				ctimer_set(&sche_cycle_ctimer, TIME_SLOT_NUM*TIME_SLOT+2,
						(void (*)(void *))bird_schedule, NULL);
				pc_mode = PC_PARENT;
				bird_wait(0,0,0,&pc_ctimer,cpowercycle);
				PT_YIELD(&pt_sche);
				if(pc_state != PARENT_DONE)
				{
					PRINTF("PARENT: cpowercycle timeout!\n");
					pc_mode = PC_P_TIMEOUT;
					ctimer_set(&sche_cycle_ctimer, PC_OFF_TIME,
										(void (*)(void *))bird_schedule, NULL);
					bird_wait(0,0,0,&pc_ctimer,cpowercycle);
					PT_YIELD(&pt_sche);
				}
				leds_off(LEDS_BLUE);
#if PLATFORM_L==Z1MOTE_L
				birdtrace_log("p");
#endif
			}


		}
		PT_YIELD(&pt_sche);
		// SYNC PHASE done

		//sleep until next update time
		//		printf("sleep intv: %d\n",next_wakeup_time-dclock_get_time(dc));
		//		printf("sleep next: %d\n",next_wakeup_time);
		//		printf("sleep now: %d\n",dclock_get_time(dc));

		// next wake up time define later..

		ctimer_set(&sche_ctimer,next_wakeup_time-dclock_get_time(dc),(
				void (*)(void *))bird_schedule, NULL);
		PT_YIELD(&pt_sche);

#if DATA_ON
		leds_on(7);
		pc_mode = PC_DATA;
		bird_wait(0,0,0,&data_ctimer,bird_data);
		ctimer_set(&sche_ctimer,bird_time(DATA_TIME),
				(void (*)(void *))bird_schedule, NULL);
		PT_YIELD(&pt_sche);
		leds_off(7);
#endif

#if SHORT_TEST == 0
		if(bird_cycle_cnt>0)
		{
			while(sleep_min<PERIOD_MINUTE)
			{
				sleep_min++;
				ctimer_set(&sche_ctimer,bird_time(CLOCK_SECOND*60),(
						void (*)(void *))bird_schedule, NULL);
				PT_YIELD(&pt_sche);
			}
			sleep_min = 0;
		}
#endif

	}

	PT_END(&pt_sche);
	return 1;
}
/*---------------------------------------------------------------------------*/
static char bird_send_strobe(rimeaddr_t *dst,uint8_t type)
{
	int strobes;
	struct bird_hdr *hdr;
	uint8_t strobe[MAX_PACKET_SIZE];
	int strobe_len, len;
	int strobe_ret=0;
	int collision_len = 0;

	// set packet attribute //
	packetbuf_clear();
	packetbuf_set_addr(PACKETBUF_ADDR_SENDER,&rimeaddr_node_addr);
	packetbuf_set_addr(PACKETBUF_ADDR_RECEIVER,dst);
	len = NETSTACK_FRAMER.create();

	// strobe_len: 1 for slot num for num of beacon //
	strobe_len = len + sizeof(struct bird_hdr)+1;
	if(len < 0 || strobe_len > (int)sizeof(strobe))
	{
		PRINTF("[bird] send strobe failed, too large header\n");
		return MAC_TX_ERR_FATAL;
	}

	// make strobe pacekt //
	memcpy(strobe, packetbuf_hdrptr(), len);
	strobe[len] = DISPATCH; /* dispatch */
	strobe[len + 1] = type; /* type */

	strobes = 0;
	radio_on();
	collision_len = NETSTACK_RADIO.read(packetbuf_dataptr(), PACKETBUF_SIZE); // kdw
	if (collision_len > 0)
	{
		return 3;
	}
	/* Send the strobe packet. */
	if(type==TYPE_BEACON_P)
	{
		strobe[strobe_len-1] = (char)strobes;
	}
	strobe_ret=NETSTACK_RADIO.send(strobe, strobe_len);
	if(strobe_ret != RADIO_TX_OK) // FAIL to TX strobe
	{
		return 3;
	}

	return 0; // no packet at all


}
static char bird_send_strobe1(rimeaddr_t *dst,uint8_t type)
{
	rtimer_clock_t t0;
	rtimer_clock_t t;
	int strobes;
	struct bird_hdr *hdr;
	int got_strobe_ack = 0;
	uint8_t strobe[MAX_PACKET_SIZE];
	int strobe_len, len;
	int strobe_ret=0;
	int is_dispatch, is_strobe_ack;

	//    int suppressed=0;

	uint8_t collisions;

	// set packet attribute //
	packetbuf_clear();
	packetbuf_set_addr(PACKETBUF_ADDR_SENDER,&rimeaddr_node_addr);
	packetbuf_set_addr(PACKETBUF_ADDR_RECEIVER,dst);
	len = NETSTACK_FRAMER.create();

	// strobe_len: 1 for slot num for num of beacon //
	strobe_len = len + sizeof(struct bird_hdr)+1;
	if(len < 0 || strobe_len > (int)sizeof(strobe))
	{
		PRINTF("[bird] send strobe failed, too large header\n");
		return MAC_TX_ERR_FATAL;
	}

	// make strobe pacekt //
	memcpy(strobe, packetbuf_hdrptr(), len);
	strobe[len] = DISPATCH; /* dispatch */
	strobe[len + 1] = type; /* type */
	//    strobe[len + 2] = slot_num; /* slot num */

	/* By setting we_are_sending to one, we ensure that the rtimer
         powercycle interrupt do not interfere with us sending the packet. */
	t0 = RTIMER_NOW();
	strobes = 0;
	/* Turn on the radio to listen for the strobe ACK. */
	radio_on();
	collisions = 0;
	got_strobe_ack = 0;

	t = RTIMER_NOW();
	NETSTACK_RADIO.read(packetbuf_dataptr(), PACKETBUF_SIZE); // kdw

	for(strobes = 0, collisions = 0;
			got_strobe_ack == 0 && collisions == 0 &&
					RTIMER_CLOCK_LT(RTIMER_NOW(), t0 + bird_config.strobe_time);
			strobes++)
	{

		while(got_strobe_ack == 0 &&
				RTIMER_CLOCK_LT(RTIMER_NOW(), t + bird_config.strobe_wait_time))
		{
			/* See if we got an ACK */
			packetbuf_clear();
			len = NETSTACK_RADIO.read(packetbuf_dataptr(), PACKETBUF_SIZE);
			if(len > 0)
			{
				packetbuf_set_datalen(len);
				if(NETSTACK_FRAMER.parse() >= 0)
				{
					hdr = packetbuf_dataptr();
					is_dispatch = hdr->dispatch == DISPATCH;
					is_strobe_ack = hdr->type == type+1;
					if(is_dispatch)
					{
						if(is_strobe_ack)
						{
							if(rimeaddr_cmp(packetbuf_addr(PACKETBUF_ADDR_RECEIVER),&rimeaddr_node_addr))
							{
								PRINTF("got ACK!!!\n");
								return 1;
							}
						}
						else
						{
							if(rimeaddr_cmp(packetbuf_addr(PACKETBUF_ADDR_RECEIVER),&rimeaddr_node_addr) && hdr->type == TYPE_SYNC_PULSE)
							{
								return 4; // SYNC pulse for parent during tx beacon, it should react 1229 jjh
							}
#if PARENT_PRIORITY == 1
		if(hdr->type == TYPE_BEACON && type==TYPE_BEACON_P)
		{
			if(!rimeaddr_cmp(packetbuf_addr(PACKETBUF_ADDR_RECEIVER),&rimeaddr_node_addr))
			{
				return 3;
			}
		}
		else
		{
			return 3;
		}
#else
	return 3;
#endif

						}
					}
					else
					{
						return 3;
					}
				}
				else
				{
					// fail to parse interference //
					PRINTF("[bird] send failed to parse %u\n", len);
					return 3;
				}
			}
		}
		t = RTIMER_NOW();

		/* Send the strobe packet. */
		if(got_strobe_ack == 0 && collisions == 0)
		{
			if(type==TYPE_BEACON_P)
			{
				strobe[strobe_len-1] = (char)strobes;
			}
			radio_on();
			strobe_ret=NETSTACK_RADIO.send(strobe, strobe_len);
			if(strobe_ret != RADIO_TX_OK) // FAIL to TX strobe
			{
				return 3;
			}
		}
	}

	    if(type==TYPE_BEACON)
	    {
	    	if(got_strobe_ack == 0 & collisions == 0 /*& suppressed == 0*/)
	        {
	            while(RTIMER_CLOCK_LT(RTIMER_NOW(), t+bird_config.strobe_wait_time))    {} //To lengthen time interval between last BEACON and BEACON_END JJH 0424
	//            bird_send_type(&rimeaddr_null,TYPE_BEACON_END); //Send end signal of beacon
	            bird_send_type(&topo_info.parent_addr,TYPE_BEACON_END); //Send end signal of beacon jjh 1227 re-config
	            PRINTF("[sss] send beacon end\n");
	        }
	    }
	PRINTF("[bird] send_strobe (# of strobes=%u, len=%u, %s)\n", strobes-1,
			packetbuf_totlen(), got_strobe_ack ? "ack" : "no ack");

	return 0; // no packet at all
}
/*---------------------------------------------------------------------------*/
static char bird_send_data(rimeaddr_t *dst)
{
	PRINTF_D("[bird] bird_send_data to addr:%d\n",dst->u8[0]);
	uint8_t data[MAX_PACKET_SIZE];
	int data_len, len, bird_data_len;
	uint8_t ret;
	packetbuf_clear();
	packetbuf_set_addr(PACKETBUF_ADDR_RECEIVER,dst);
	packetbuf_set_addr(PACKETBUF_ADDR_SENDER,&rimeaddr_node_addr);
	len = NETSTACK_FRAMER.create();
	data_len = len + sizeof(struct bird_hdr);
	if(len < 0 || data_len > (int)sizeof(data))
	{
		/* Failed to send */
		PRINTF("[bird] send failed, too large header\n");
		return MAC_TX_ERR_FATAL;
	}

	memcpy(data, packetbuf_hdrptr(), len);
	data[len] = DISPATCH; /* dispatch */
	data[len + 1] = TYPE_DATA; /* type */

	bird_data_len = birddata_make_packet(data+len+2,&birdQueue);

	if(bird_data_len == 0)
		return -1;
	else
		data_len += bird_data_len;

	radio_on();
	ret = NETSTACK_RADIO.send(data, data_len);

	PRINTF_D("[bird] send_data ret %d \n",ret);
	if(ret != RADIO_TX_OK)
		return -2;
	return bird_data_len/sizeof(BirdData);
}
/*---------------------------------------------------------------------------*/
static char bird_send_type(rimeaddr_t *dst,uint8_t type)
{
	PRINTF_D("[bird] bird_send_type (%x) to addr:%d\n",type,dst->u8[0]);
	uint8_t beacon[MAX_PACKET_SIZE];
	int beacon_len, len;
	uint8_t ret;
	packetbuf_clear();
	packetbuf_set_addr(PACKETBUF_ADDR_RECEIVER,dst);
	packetbuf_set_addr(PACKETBUF_ADDR_SENDER,&rimeaddr_node_addr);
	len = NETSTACK_FRAMER.create();
	beacon_len = len + sizeof(struct bird_hdr);
	if(len < 0 || beacon_len > (int)sizeof(beacon))
	{
		/* Failed to send */
		PRINTF("[bird] send failed, too large header\n");
		return MAC_TX_ERR_FATAL;
	}

	memcpy(beacon, packetbuf_hdrptr(), len);
	beacon[len] = DISPATCH; /* dispatch */
	beacon[len + 1] = type; /* type */

	if(type == TYPE_SYNC_PULSE)
	{
		clock_time_t current_time = dclock_get_time(dc);
		memcpy(beacon+len+2,&current_time,sizeof(clock_time_t));
		beacon_len = len + sizeof(struct bird_hdr)+sizeof(clock_time_t);
		PRINTF_T("[time] t1: %d\n",current_time);
	}
	radio_on();
	ret = NETSTACK_RADIO.send(beacon, beacon_len);
	PRINTF_D("[bird] send_beacon ret %d type %d\n",ret,type);
	return ret;
}
/*----------------------------- ----------------------------------------------*/
static char bird_send_sync_ack(rimeaddr_t *dst, struct Time_stamp time_stamp)
{
	int len,sync_ack_len;
	uint8_t sync_ack[MAX_PACKET_SIZE];
	char ret;
	/* make header */
	packetbuf_clear();
	packetbuf_set_addr(PACKETBUF_ADDR_RECEIVER,dst);
	packetbuf_set_addr(PACKETBUF_ADDR_SENDER, &rimeaddr_node_addr);
	packetbuf_compact();
	len = NETSTACK_FRAMER.create();
	sync_ack_len = len + sizeof(struct bird_hdr) + 4 * sizeof(clock_time_t);

	/* header check */
	if(len < 0 || sync_ack_len > (int)sizeof(sync_ack))
	{
		PRINTF("[bird] send failed, too large header\n");
		return MAC_TX_ERR_FATAL;    /* Failed to send */
	}

	/* set packet data part */
	memcpy(sync_ack, packetbuf_hdrptr(), len);
	sync_ack[len] = DISPATCH;                 /* dispatch */
	sync_ack[len + 1] = TYPE_SYNC_ACK;         /* type */
	time_stamp.t3 = dclock_get_time(dc);     /* set t3 */

	PRINTF_T("[time] send_sync_ack t1 %d\n",time_stamp.t1);
	PRINTF_T("[time] send_sync_ack t2 %d\n",time_stamp.t2);
	PRINTF_T("[time] send_sync_ack t3 %d\n",time_stamp.t3);

	memcpy(sync_ack + len + 2, &time_stamp,
			3*sizeof(clock_time_t));         /* make packet with t1,t2,t3 */
	memcpy(sync_ack + len + 2 + 3*sizeof(clock_time_t), &next_wakeup_time,
			sizeof(clock_time_t));           /* set next wake up time */

#if PARAM_L == 1

	memcpy(sync_ack + len + 2 + 4*sizeof(clock_time_t), &param_l,
			sizeof(int));           /* set next wake up time */
	sync_ack_len += sizeof(int);
#endif
	ret = NETSTACK_RADIO.send(sync_ack, sync_ack_len);
	return ret;
}
/*---------------------------------------------------------------------------*/
static char bird_check_recv(char recv)
{
	PRINTF_TABLE("[table] bird_check_recv %d\n",recv);
	int num_recv=0;
	int i;
	if(recv==-1) // init!!
	{
		for(i=0;i<topo_info.num_child;i++)
			recv_table[i]=0;
		return -1;
	}
	// updat table
	for(i=0;i<topo_info.num_child;i++)
	{
		if(recv > 0)
			if(recv == topo_info.child_addr[i])
				recv_table[i]=1;
		// if recv == 0 just check recv done or not
		num_recv +=recv_table[i];
		if(recv_table[i] == 1)
		{
			PRINTF_TABLE("[table] child id: %d\n",topo_info.child_addr[i] );
		}
	}
	PRINTF_TABLE("[table] recv child : %d\n",num_recv );
#if DEBUG
	if(recv != 0 && pc_mode == PC_PARENT && pc_state != PARENT_DONE)
	{
		PRINTF("[cycle] done with %d\n",recv);
	}
#endif
	if (num_recv == topo_info.num_child)
		return 1;
	else
		return 0;
}
/*---------------------------------------------------------------------------*/
static clock_time_t bird_time(clock_time_t time)
{
#if PLATFORM_L==MICAZ_L
	time = (clock_time_t)((double)time * CLOCK_RATE_L);
#endif
	return time;
}
/*---------------------------------------------------------------------------*/
static void radio_on() {
	if (is_radio_on == 0) {
		leds_on(LEDS_RED);
		NETSTACK_RADIO.on();
		is_radio_on = 1;
//#if PLATFORM_L != COOJA_L
		//		birdLogEnergy(is_radio_on,dclock_get_time(dc),&birdLog);
		birdLogEnergy(is_radio_on,clock_time(),&birdLog);

//#endif
	}
}
static void radio_off() {
	if (is_radio_on == 1) {
		leds_off(LEDS_RED);
		NETSTACK_RADIO.off();
		is_radio_on = 0;
//#if PLATFORM_L != COOJA_L
		//		birdLogEnergy(is_radio_on,dclock_get_time(dc),&birdLog);
		birdLogEnergy(is_radio_on,clock_time(),&birdLog);
//#endif
	}
}
/*---------------------------------------------------------------------------*/
static int
send_one_packet(mac_callback_t sent, void *ptr)
{
	//	rimeaddr_t addr;
	//	addr.u8[0] = 1;
	//	addr.u8[1] = 0;
	//	bird_send_type(&addr,TYPE_BEACON);
	//	bird_send_strobe(&addr,TYPE_BEACON);

	int ret;
	int last_sent_ok = 0;
	//
	//  packetbuf_set_addr(PACKETBUF_ADDR_SENDER, &rimeaddr_node_addr);
	//  if(NETSTACK_FRAMER.create() < 0) {
	//    /* Failed to allocate space for headers */
	//    PRINTF("nullrdc: send failed, too large header\n");
	//    ret = MAC_TX_ERR_FATAL;
	//  } else {
	//    switch(NETSTACK_RADIO.send(packetbuf_hdrptr(), packetbuf_totlen())) {
	//    case RADIO_TX_OK:
	//      ret = MAC_TX_OK;
	//      break;
	//    case RADIO_TX_COLLISION:
	//      ret = MAC_TX_COLLISION;
	//      break;
	//    case RADIO_TX_NOACK:
	//      ret = MAC_TX_NOACK;
	//      break;
	//    default:
	//      ret = MAC_TX_ERR;
	//      break;
	//    }
	//  }
	//  if(ret == MAC_TX_OK) {
	//    last_sent_ok = 1;
	//  }
	//  mac_call_sent_callback(sent, ptr, ret, 1);
	return last_sent_ok;
}
/*---------------------------------------------------------------------------*/
static void
send_packet(mac_callback_t sent, void *ptr)
{
	int seq = 0;
	memcpy(&seq, packetbuf_dataptr(), sizeof(int));

	birddata_from_data(rimeaddr_node_addr.u8[0], seq, &birdQueue);
 	// birddata_queue_print(&birdQueue);
	// send_one_packet(sent, ptr);
}
/*---------------------------------------------------------------------------*/
static void
send_list(mac_callback_t sent, void *ptr, struct rdc_buf_list *buf_list)
{
	printf("list\n");
	while(buf_list != NULL) {
		/* We backup the next pointer, as it may be nullified by
		 * mac_call_sent_callback() */
		struct rdc_buf_list *next = buf_list->next;
		int last_sent_ok;

		queuebuf_to_packetbuf(buf_list->buf);
		last_sent_ok = send_one_packet(sent, ptr);

		/* If packet transmission was not successful, we should back off and let
		 * upper layers retransmit, rather than potentially sending out-of-order
		 * packet fragments. */
		if(!last_sent_ok) {
			return;
		}
		buf_list = next;
	}
}
/*---------------------------------------------------------------------------*/
static void
packet_input(void)
{

	int original_datalen;
	uint8_t *original_dataptr;

	bird_wait_info.bird_interference=1; // there are some packets

	original_datalen = packetbuf_datalen();
	original_dataptr = packetbuf_dataptr();

	if(NETSTACK_FRAMER.parse() < 0)
	{
		PRINTF("nullrdc: failed to parse %u\n", packetbuf_datalen());
		return;
	}

	struct bird_hdr *hdr;
	hdr = packetbuf_dataptr();
	rimeaddr_t temp_addr;

	if(hdr->dispatch==0)
	{
		if(pc_mode == PC_INIT)
		{
			if(!rimeaddr_cmp(packetbuf_addr(PACKETBUF_ADDR_RECEIVER),&rimeaddr_node_addr)) // General packet
			{
				return;
			}
			bird_wait_info.bird_input = hdr->type;
			if(hdr->type == TYPE_SYNC_ACK)
			{
				bird_time_stamp.t4 = dclock_get_time(dc); //set t4
#if BRIDGE_L == 1
				long t[4]={0,};
				clock_time_t nt,ct;
				memcpy(&t,packetbuf_dataptr()+2,
						4*2 * sizeof(clock_time_t));    // get t from packet
				bird_time_stamp.t1 = (clock_time_t)t[0];
				bird_time_stamp.t2 = (clock_time_t)t[1];
				bird_time_stamp.t3 = (clock_time_t)t[2];
				nt = (clock_time_t)t[3];
				ct = dclock_get_time(dc);
				next_wakeup_time = (clock_time_t)(bird_time(nt-ct)+ct);
#else
				memcpy(&bird_time_stamp,packetbuf_dataptr()+2,
						3 * sizeof(clock_time_t));    // get t from packet
				memcpy(&next_wakeup_time,packetbuf_dataptr()+ 2 + 3*sizeof(clock_time_t),
						sizeof(clock_time_t));        // get next wake up time

#endif
#if PLATFORM_L != COOJA_L
				birdLog.clockdrift = dclock_synchronize(dc,bird_time_stamp);
				PRINTF_T("drift : %d\n",birdLog.clockdrift);
#endif
				PRINTF_T("[time] t1 : %d\n",bird_time_stamp.t1);
				PRINTF_T("[time] t2 : %d\n",bird_time_stamp.t2);
				PRINTF_T("[time] t3 : %d\n",bird_time_stamp.t3);
				PRINTF_T("[time] t4 : %d\n",bird_time_stamp.t4);
				PRINTF_T("[time] nt : %d\n",next_wakeup_time);

			}
			else if(hdr->type == TYPE_SYNC_PULSE)
			{
				bird_time_stamp.t2 = dclock_get_time(dc); //set t2
				memcpy(&bird_time_stamp,packetbuf_dataptr()+2,sizeof(clock_time_t));
				PRINTF_T("[time] TYPE_SYNC_PULSE get t1 %d\n",bird_time_stamp.t1);
				PRINTF_T("[time] TYPE_SYNC_PULSE set t2 %d\n",bird_time_stamp.t2);
			}
			if(bird_wait_info.bird_input == bird_wait_info.bird_wait_packet)
			{
				PRINTF_D("recv target packet %x\n",bird_wait_info.bird_input);
				bird_wait(0,0,0,&init_ctimer,bird_init);
			}

		}
		else if(pc_mode==PC_DATA)
		{
			if(!rimeaddr_cmp(packetbuf_addr(PACKETBUF_ADDR_RECEIVER),&rimeaddr_node_addr)) // General packet
				return;
			bird_wait_info.bird_mine = 1;
			bird_wait_info.bird_input = hdr->type;
			bird_wait_info.bird_from = packetbuf_addr(PACKETBUF_ADDR_SENDER)->u8[0];

			if(bird_wait_info.bird_input == TYPE_DATA)
			{
				char* recv_data_ptr = (char*)hdr+sizeof(struct bird_hdr);

				birddata_get_packet(recv_data_ptr,&birdQueue);

				temp_addr.u8[1] = 0;
				temp_addr.u8[0] = bird_wait_info.bird_from;
				bird_send_type(&temp_addr,TYPE_DATA_ACK);
#if PLATFORM_L != COOJA_L
				PRINTF_DATA("[data] get size : %d\n",birddata_queue_size(&birdQueue));
#endif

			}
			else if(hdr->type == TYPE_DATA_ACK)
			{
				if(bird_wait_info.bird_interrupt == 1)
				{
					if(bird_wait_info.bird_wait_packet == bird_wait_info.bird_input)
					{
						PRINTF_D("recv target packet %x\n",bird_wait_info.bird_input);
						bird_wait(0,0,0,&data_ack_ctimer,bird_data);

					}
				}
			}


		}
		else
		{

			if(rimeaddr_cmp(packetbuf_addr(PACKETBUF_ADDR_RECEIVER),&rimeaddr_node_addr)) // General packet
			{
				bird_wait_info.bird_mine = 1;
			}
			else if(rimeaddr_cmp(packetbuf_addr(PACKETBUF_ADDR_SENDER),&topo_info.parent_addr)) // For broadcast
			{

			}
			else if(rimeaddr_cmp(packetbuf_addr(PACKETBUF_ADDR_RECEIVER),&topo_info.parent_addr))
			{
				bird_wait_info.bird_sibling = 1; // jjh 1227
				if(hdr->type == TYPE_SYNC_FINAL || hdr->type == TYPE_BEACON_END)
					bird_wait_info.bird_input = hdr->type; // To check SYNC_FINAL or BEACON_END jjh 1227
				// input? type setting?
				return;
			}
			else
			{
				return;
			}

//			printf("type input: %d\n", hdr->type);
			bird_wait_info.bird_input = hdr->type;
			bird_wait_info.bird_from = packetbuf_addr(PACKETBUF_ADDR_SENDER)->u8[0];

			if(hdr->type == TYPE_SYNC_ACK)
			{
				bird_time_stamp.t4 = dclock_get_time(dc); //set t4
#if BRIDGE_L == 1
				long t[4]={0,};
				clock_time_t nt,ct;
				memcpy(&t,packetbuf_dataptr()+2,
						4*2 * sizeof(clock_time_t));    // get t from packet
				bird_time_stamp.t1 = (clock_time_t)t[0];
				bird_time_stamp.t2 = (clock_time_t)t[1];
				bird_time_stamp.t3 = (clock_time_t)t[2];
				birdLog.clockdrift = dclock_synchronize(dc,bird_time_stamp);
				nt = (clock_time_t)t[3];
				ct = dclock_get_time(dc);
				next_wakeup_time = (clock_time_t)(bird_time(nt-ct)+ct);
				//				printf("dddddddd\n");
				//				printf("%ul %d %d\n",t[3],ct,next_wakeup_time);
				//				printf("%d\n",birdLog.clockdrift);
#if PARAM_L == 1
				memcpy(&param_l,packetbuf_dataptr()+ 2 + 2*4*sizeof(clock_time_t),
						sizeof(int));        // get next wake up time
#endif
#else
				memcpy(&bird_time_stamp,packetbuf_dataptr()+2,
						3 * sizeof(clock_time_t));    // get t from packet
				memcpy(&next_wakeup_time,packetbuf_dataptr()+ 2 + 3*sizeof(clock_time_t),
						sizeof(clock_time_t));        // get next wake up time
#if PLATFORM_L != COOJA_L
				birdLog.clockdrift = dclock_synchronize(dc,bird_time_stamp);
#endif
#if PARAM_L == 1
				memcpy(&param_l,packetbuf_dataptr()+ 2 + 4*sizeof(clock_time_t),
						sizeof(int));        // get next wake up time
#endif
#endif
#if PLATFORM_L != COOJA_L
				PRINTF_T("drift : %d\n",birdLog.clockdrift);
#endif
				PRINTF_T("[time] t1 : %d\n",bird_time_stamp.t1);
				PRINTF_T("[time] t2 : %d\n",bird_time_stamp.t2);
				PRINTF_T("[time] t3 : %d\n",bird_time_stamp.t3);
				PRINTF_T("[time] t4 : %d\n",bird_time_stamp.t4);
				PRINTF_T("[time] nt : %d\n",next_wakeup_time);
			}
			else if(hdr->type == TYPE_SYNC_PULSE)
			{
				bird_time_stamp.t2 = dclock_get_time(dc); //set t2
				memcpy(&bird_time_stamp,packetbuf_dataptr()+2,sizeof(clock_time_t));
				PRINTF_T("[time] TYPE_SYNC_PULSE get t1 %d\n",bird_time_stamp.t1);
				PRINTF_T("[time] TYPE_SYNC_PULSE set t2 %d\n",bird_time_stamp.t2);
			}
			else if(hdr->type == TYPE_BEACON_P)
			{
				strobe_cnt=*(char*)(packetbuf_dataptr()+2);
				PRINTF_D("strobe_cnt : %d\n",strobe_cnt);

			}
			else if(hdr->type == TYPE_SYNC_FINAL)
			{
				bird_check_recv(bird_wait_info.bird_from);
			}


			if(bird_wait_info.bird_interrupt == 1)
			{
				if(bird_wait_info.bird_wait_packet == bird_wait_info.bird_input)
				{
					PRINTF_D("recv target packet %x\n",bird_wait_info.bird_input);
					bird_wait(0,0,0,&pc_ctimer,cpowercycle);
				}
				else if (bird_wait_info.bird_wait_packet == TYPE_BEACON_P && bird_wait_info.bird_input == TYPE_BEACON_ACK)
				{
					bird_wait(0,0,0,&pc_ctimer,cpowercycle);
				}
				else if (bird_wait_info.bird_wait_packet == TYPE_BEACON_SYNC_PULSE)
				{
					if(bird_wait_info.bird_input == TYPE_BEACON ||
							bird_wait_info.bird_input == TYPE_SYNC_PULSE)
					{
						PRINTF_D("recv target packet %x\n",bird_wait_info.bird_input);
						bird_wait(0,0,0,&pc_ctimer,cpowercycle);
					}
				}

			}
		}
	}
	else
	{
		NETSTACK_MAC.input();
	}
}
/*---------------------------------------------------------------------------*/
static void bird_set_address()
{
	int i=0,j=0;
	//#if EXPERIMENT
	for(i=0; i<NODE_NUM; i++)
		if(topology[i][8]==rimeaddr_node_addr.u8[0])
			break;
	topo_info.tree_state = topology[i][0];
	topo_info.level = topology[i][1];
	topo_info.num_child = topology[i][2];
	topo_info.parent_addr.u8[0]=topology[i][3];
	topo_info.parent_addr.u8[1]=topology[i][4];
	topo_info.num_contention = topology[i][5];
	topo_info.descent=topology[i][6];
	topo_info.slot_num=topology[i][7];
	topo_info.max_level=MAX_LEVEL;
	/*#else
    topo_info.tree_state = topology[rimeaddr_node_addr.u8[0]-1][0];
    topo_info.level = topology[rimeaddr_node_addr.u8[0]-1][1];
    topo_info.num_child = topology[rimeaddr_node_addr.u8[0]-1][2];
    topo_info.parent_addr.u8[0]=topology[rimeaddr_node_addr.u8[0]-1][3];
    topo_info.parent_addr.u8[1]=topology[rimeaddr_node_addr.u8[0]-1][4];
    topo_info.num_contention = topology[rimeaddr_node_addr.u8[0]-1][5];
    topo_info.descent=topology[rimeaddr_node_addr.u8[0]-1][6];
    topo_info.slot_num=topology[rimeaddr_node_addr.u8[0]-1][7];
    topo_info.max_level=MAX_LEVEL;
#endif*/
	topo_info.bridge = BRIDGE_L;
	for(i=0,j=0; i<NODE_NUM; i++)
	{
		if(topology[i][3]==rimeaddr_node_addr.u8[0])
		{
			topo_info.child_addr[j] = topology[i][8];
			printf("child : %d\n",topo_info.child_addr[j]);
			j++;
		}
	}
	if(j!=topo_info.num_child)
	{
		printf("[Error] Topology num child is not matched\n");
	}

}
/*---------------------------------------------------------------------------*/
static int
on(void)
{
	bird_wait(0,0,0,&pc_ctimer,cpowercycle);
	ctimer_set(&init_ctimer, 1,(void (*)(void *))bird_init, NULL);
#if DATA_ON
	ctimer_set(&data_ctimer, 1,(void (*)(void *))bird_data, NULL);
#endif
	ctimer_set(&sche_ctimer, 1,(void (*)(void *))bird_schedule, NULL);


	return NETSTACK_RADIO.on();
}
/*---------------------------------------------------------------------------*/
static int
off(int keep_radio_on)
{
	if(keep_radio_on) {
		return NETSTACK_RADIO.on();
	} else {
		return NETSTACK_RADIO.off();
	}
}
/*---------------------------------------------------------------------------*/
static unsigned short
channel_check_interval(void)
{
	return 0;
}
/*---------------------------------------------------------------------------*/
static void
init(void)
{
	bird_set_address();
	random_init(rimeaddr_node_addr.u8[0]+RANDOM_SEED);
	dclock_init(&dc);

	PT_INIT(&pt);
	PT_INIT(&pt_init);
#if DATA_ON
	PT_INIT(&pt_data);
	birddata_queue_init(birdData, BIRD_QUEUE_SIZE, &birdQueue);
#endif
	PT_INIT(&pt_sche);
	on();
}
/*---------------------------------------------------------------------------*/
const struct rdc_driver birdmac_driver = {
		"birdmac",
		init,
		send_packet,
		send_list,
		packet_input,
		on,
		off,
		channel_check_interval,
};
/*---------------------------------------------------------------------------*/

