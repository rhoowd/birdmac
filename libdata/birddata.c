/*
 * birddata.c
 *
 *  Created on: Nov 29, 2015
 *      Author: user
 */
#include "birddata.h"

char birddata_enqueue(BirdData data,Birdqueue* pQueue)
{
    if ((pQueue->tail + 1) % pQueue->max == pQueue->head){
        printf("[error] Queue overflow.\n");
        return 0;
    }
    pQueue->queue[pQueue->tail] = data;
    pQueue->tail = ++pQueue->tail % pQueue->max;
    return 1;
}

char birddata_dequeue(Birdqueue* pQueue)
{
	if (pQueue->head == pQueue->tail){
		printf("[error] Queue underflow.\n");
		return 0;
	}
	pQueue->head = ++pQueue->head % pQueue->max;
	return 1;
}

char birddata_from_data(int id, int seq, Birdqueue* pQueue)
{
	pQueue->queue[pQueue->tail].id = id;
	pQueue->queue[pQueue->tail].seq = seq;
	pQueue->tail++;
	return 1;
}

int birddata_make_packet(char* ptr,Birdqueue* pQueue)
{
	char* p;
	int i=0;
	while(i<MAX_DATA_NUM)
	{
		if(pQueue->head +i != pQueue->tail)
		{
			memcpy(ptr+i*sizeof(BirdData),&(pQueue->queue[(pQueue->head+i)%pQueue->max]),
				sizeof(BirdData));
			i++;
		}
		else
		{
			break;
		}
	}
	ptr[i*sizeof(BirdData)]=DATA_END;
	if (i==0)
		return 0;
	else
		return (i*sizeof(BirdData)+1);
}

char birddata_get_packet(char* ptr,Birdqueue* pQueue)
{
	char* p;
	int i=0;
	BirdData tempdata;
	while(ptr[i*sizeof(BirdData)] != DATA_END)
	{
		memcpy(&tempdata,ptr+i*sizeof(BirdData),sizeof(BirdData));
		if(birddata_enqueue(tempdata,pQueue) == 0)
			return 0;
		i++;
	}
	return 1;
}
int birddata_queue_get_next(Birdqueue* pQueue)
{
	return pQueue->tail;
}
char birddata_queue_clear(Birdqueue* pQueue)
{
	pQueue->head = pQueue->tail = 0;
	return 1;
}
int birddata_queue_size(Birdqueue* pQueue)
{
	int ret = pQueue->tail - pQueue->head;
	if (ret>=0)
		return ret;
	else
		return ret + pQueue->max;
}

int birddata_queue_init(BirdData* queue, int max, Birdqueue* pQueue)
{
	printf("BirdData size: %d\n",sizeof(BirdData));
	pQueue->queue = queue;
	pQueue->max = max;
	birddata_queue_clear(pQueue);
	return 1;
}
int birddata_recv_packet(char* packet, Birdqueue* pQueue)
{

	return 1;
}
void birddata_print_one_data(BirdData* data)
{
	//	printf("[queue] %3d  %4d \n",data->id, data->seq);
	printf("DATA recv 'DATA id:%04d from:%03d'\n", data->seq, data->id);
}
void birddata_queue_print(Birdqueue* pQueue)
{

	int i=0;
	for(i=pQueue->head; i%pQueue->max != pQueue->tail; i++)
		birddata_print_one_data(&(pQueue->queue[i]));
}
char birddata_queue_free(int num, Birdqueue* queue)
{
	int i=0;
	for(i=0;i<num;i++)
	{
		if(birddata_dequeue(queue) == 0)
			return 0;
	}
	return 1;
}
