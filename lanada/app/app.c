/*
 * Copyright (c) 2007, Swedish Institute of Computer Science.
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
 *         Best-effort single-hop unicast example
 * \author
 *         Adam Dunkels <adam@sics.se>
 */

#include "param.h"
#include "contiki.h"
#include "net/rime.h"
#include "lib/random.h"

#include "dev/button-sensor.h"

#include "dev/leds.h"

#include <stdio.h>
#include <stdlib.h>

#define TRAFFIC 2
/*---------------------------------------------------------------------------*/
PROCESS(example_unicast_process, "Example unicast");
AUTOSTART_PROCESSES(&example_unicast_process);
/*---------------------------------------------------------------------------*/
static void
recv_uc(struct unicast_conn *c, const rimeaddr_t *from)
{
  printf("unicast message received from %d.%d\n",
	 from->u8[0], from->u8[1]);
}
static const struct unicast_callbacks unicast_callbacks = {recv_uc};
static struct unicast_conn uc;
static double rd = 0.0;
static int rd_clock = 0;
double
app_randn (double mu, double sigma)
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
double app_randomness (double mu, double sigma)
{
	double ret=0;

	while(1)
	{
		ret = app_randn (mu, sigma);
		if(ret <= (PERIOD)/3 && ret >= -1*(PERIOD)/3)
			return ret;
	}
	return -100;
}

/*---------------------------------------------------------------------------*/
PROCESS_THREAD(example_unicast_process, ev, data)
{
  PROCESS_EXITHANDLER(unicast_close(&uc);)
  PROCESS_BEGIN();

  static clock_time_t poisson_int;
  float rand_num;
  static int seq = 0;
  static struct etimer et;
  static struct etimer et_randomness;

  unicast_open(&uc, 146, &unicast_callbacks);

  etimer_set(&et, (38-WAIT_TIME)*CLOCK_SECOND);
  PROCESS_WAIT_EVENT_UNTIL(etimer_expired(&et));


  while(1) {

    rimeaddr_t addr;
    seq++;

# if TRAFFIC == 1
    rand_num=random_rand()/(float)RANDOM_RAND_MAX;
    poisson_int = (-ARRIVAL_RATE) * logf(rand_num) * CLOCK_SECOND;
    if(poisson_int == 0)
    	poisson_int = 1;

    etimer_set(&et, ARRIVAL_RATE*CLOCK_SECOND);
    PROCESS_WAIT_EVENT_UNTIL(etimer_expired(&et));

    packetbuf_copyfrom(&seq, sizeof(int));
    addr.u8[0] = 1;
    addr.u8[1] = 0;

    if(!rimeaddr_cmp(&addr, &rimeaddr_node_addr)) {
    	printf("app: DATA id:%04d from:%03d\n", seq, rimeaddr_node_addr.u8[0]);
    	unicast_send(&uc, &addr);
    }

# elif TRAFFIC == 2
    etimer_set(&et, PERIOD*CLOCK_SECOND);
    rd = app_randomness(0.0, VARIANCE);
    rd_clock = (int)(rd * CLOCK_SECOND);

    etimer_set(&et_randomness, PERIOD/2*CLOCK_SECOND + rd_clock);
    PROCESS_WAIT_EVENT_UNTIL(etimer_expired(&et_randomness));
	leds_on(LEDS_BLUE);

    packetbuf_copyfrom(&seq, sizeof(int));
    addr.u8[0] = 1;
    addr.u8[1] = 0;

    if(!rimeaddr_cmp(&addr, &rimeaddr_node_addr)) {
    	printf("app: DATA id:%04d from:%03d\n", seq, rimeaddr_node_addr.u8[0]);
    	unicast_send(&uc, &addr);
    }

    PROCESS_WAIT_EVENT_UNTIL(etimer_expired(&et));
# endif
  }

  PROCESS_END();
}
/*---------------------------------------------------------------------------*/
