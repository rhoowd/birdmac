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

#ifndef __BIRDMAC_H__
#define __BIRDMAC_H__

#include "net/mac/rdc.h"
#include "bird.h"
#include "birdtrace.h"
struct bird_config {
  rtimer_clock_t on_time;
  rtimer_clock_t off_time;
  rtimer_clock_t strobe_time;
  rtimer_clock_t strobe_wait_time;
};

struct bird_hdr {
  uint8_t dispatch;
  uint8_t type;
};

#include <stdio.h>
//1:DEBUG_ALL, 2:SELECTED, 0:Nothing //
#define DEBUG_ALL 0
#if DEBUG_ALL==2
	#define DEBUG 0
	#define DEBUG_DETAIL 0
	#define DEBUG_TIME 0
	#define DEBUG_INIT 1
	#define DEBUG_STATE 0
	#define DEBUG_TABLE 0
	#define DEBUG_DATA 0
#elif DEBUG_ALL==1
	#define DEBUG 1
	#define DEBUG_DETAIL 1
	#define DEBUG_TIME 1
	#define DEBUG_INIT 1
	#define DEBUG_STATE 1
	#define DEBUG_TABLE 1
	#define DEBUG_DATA 1
#elif DEBUG_ALL==0
	#define DEBUG 0
	#define DEBUG_DETAIL 0
	#define DEBUG_TIME 0
	#define DEBUG_INIT 0
	#define DEBUG_STATE 0
	#define DEBUG_TABLE 0
	#define DEBUG_DATA 0
#endif

#if DEBUG
#define PRINTF(...) printf(__VA_ARGS__)
#else
#define PRINTF(...)
#endif

#if DEBUG_DETAIL
#define PRINTF_D(...) printf(__VA_ARGS__)
#else
#define PRINTF_D(...)
#endif

#if DEBUG_TIME
#define PRINTF_T(...) printf(__VA_ARGS__)
#else
#define PRINTF_T(...)
#endif

#if DEBUG_INIT
#define PRINTF_I(...) printf(__VA_ARGS__)
#else
#define PRINTF_I(...)
#endif

#if DEBUG_STATE
#define PRINTF_S(...) printf(__VA_ARGS__)
#else
#define PRINTF_S(...)
#endif

#if DEBUG_TABLE
#define PRINTF_TABLE(...) printf(__VA_ARGS__)
#else
#define PRINTF_TABLE(...)
#endif

#if DEBUG_DATA
#define PRINTF_DATA(...) printf(__VA_ARGS__)
#else
#define PRINTF_DATA(...)
#endif

extern const struct rdc_driver birdmac_driver;

#endif /* __NULLRDC_H__ */
