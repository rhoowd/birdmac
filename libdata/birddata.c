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
char birddata_from_birdlog(char id, char pc_mode, int cycle_cnt, struct BirdLog* log,Birdqueue* pQueue)
{
	pQueue->queue[pQueue->tail].id = id;
	pQueue->queue[pQueue->tail].pc_mode = pc_mode;
	pQueue->queue[pQueue->tail].cycle_cnt = cycle_cnt;
	pQueue->queue[pQueue->tail].onTime = log->onTime;
	pQueue->queue[pQueue->tail].msg = log->msg;
	pQueue->queue[pQueue->tail].beacon = log->beacon;
	pQueue->queue[pQueue->tail].flag = 2*log->fb + log->recovery;
	pQueue->queue[pQueue->tail].clockdrift = log->clockdrift;
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
	printf("[data] %3d  %4d  ",data->id,data->cycle_cnt);
	printf("%s",(data->pc_mode==1)?"PARENT":"CHILD ");
	printf("  %6ld  %2d  %2d  ",data->onTime,data->beacon,data->msg);
	printf("%1d  %1d  %d\n",data->flag/2%2,data->flag%2,data->clockdrift);
}
void birddata_queue_print(Birdqueue* pQueue)
{
	printf("[data]  id cycle  mode     ontime  rb  rm f  r  drift\n");
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
