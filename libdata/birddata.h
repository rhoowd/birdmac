/*
 * birddata.h
 *
 *  Created on: Nov 29, 2015
 *      Author: user
 */

#ifndef LIBDATA_BIRDDATA_H_
#define LIBDATA_BIRDDATA_H_

#include "param.h"
#include "birdlog.h"
#include "stdio.h"

#define MAX_DATA_NUM	10
#define DATA_END		(-1)

static char dup_table[15][5000];

typedef struct
{
	int id;
	int seq;

} BirdData;

typedef struct
{
	int head;
	int tail;
	int max;
	BirdData* queue;
} Birdqueue;


char birddata_enqueue(BirdData data,Birdqueue* pQueue);
char birddata_dequeue(Birdqueue* pQueue);
char birddata_from_data(int id, int seq, Birdqueue* pQueue);
int birddata_make_packet(char* ptr,Birdqueue* pQueue);
char birddata_get_packet(char* ptr,Birdqueue* pQueue);
int birddata_queue_get_next(Birdqueue* pQueue);
char birddata_queue_clear(Birdqueue* pQueue);
int birddata_queue_size(Birdqueue* pQueue);
int birddata_queue_init(BirdData* queue, int max, Birdqueue* pQueue);
int birddata_recv_packet(char* packet, Birdqueue* pQueue);
void birddata_print_one_data(BirdData* data);
void birddata_queue_print(Birdqueue* queue);
char birddata_queue_free(int num, Birdqueue* queue);
char birddata_dup_check(int id, int seq);
char birddata_dup_set(int id, int seq);

#endif /* LIBDATA_BIRDDATA_H_ */
