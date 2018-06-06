/*
 * bird-pt.c
 *
 *  Created on: Mar 17, 2015
 *      Author: koo
 */


#include "contiki.h"
#include "contiki-lib.h"
#include "sys/compower.h"
#include "net/rime.h"
#include "cfs/cfs-coffee.h"

#include "birdtrace.h"

#include <stdio.h>
#include <string.h>

#define DEBUG_BIRDTRACE 1
#if DEBUG_BIRDTRACE
#define PRINTF(...) printf(__VA_ARGS__)
#else
#define PRINTF(...)
#endif


static int fd;
static uint32_t cpu, lpm, transmit, listen;
static char buffer[MAX_LOG_SIZE];



/*---------------------------------------------------------------------------*/
void
birdtrace_start()
{
	energest_flush();
	fd = cfs_open("bird_log.txt", CFS_WRITE | CFS_APPEND);
}
/*---------------------------------------------------------------------------*/
void
birdtrace_stop(void)
{
	cfs_close(fd);
}
/*---------------------------------------------------------------------------*/
void
birdtrace_log(char* input)
{
	cpu = energest_type_time(ENERGEST_TYPE_CPU);
	lpm = energest_type_time(ENERGEST_TYPE_LPM);
	transmit = energest_type_time(ENERGEST_TYPE_TRANSMIT);
	listen = energest_type_time(ENERGEST_TYPE_LISTEN);


	sprintf(buffer, "+%s-%lu-%lu-%lu-%lu\n", input, cpu, lpm, transmit, listen);
	PRINTF("+%s-%lu-%lu-%lu-%lu\n", input, cpu, lpm, transmit, listen);
	cfs_write(fd, buffer, strlen(buffer));
}

void
birdtrace_log_datapacket(int input)
{
	sprintf(buffer, "+d %d\n", input);
	PRINTF("+d %d\n", input);
	cfs_write(fd, buffer, strlen(buffer));
}

void
birdtrace_log_sink(int input,int input2)
{
	sprintf(buffer, "+d %d %d\n", input,input2);
	PRINTF("+d %d %d\n", input,input2);
	cfs_write(fd, buffer, strlen(buffer));
}


void
birdtrace_log_string(char* input)
{
	sprintf(buffer, "+%s\n", input);
	PRINTF("+%s\n", input);
	cfs_write(fd, buffer, strlen(buffer));
}

void
birdtrace_log_string_int(char* input,int input2)
{
	sprintf(buffer, "+%s %d\n", input,input2);
	PRINTF("+%s %d\n", input,input2);
	cfs_write(fd, buffer, strlen(buffer));
}



