#include "dclock.h"

#define DEBUG_DCLOCK 0
#if DEBUG_DCLOCK
#define PRINTF(...) printf(__VA_ARGS__)
#else
#define PRINTF(...)
#endif


void dclock_init(struct Dclock **dc)
{
	*dc = &dclock;
}

clock_time_t dclock_get_time(struct Dclock *dc)
{

	if (dc->offset < 0)
		return clock_time() - (clock_time_t)(-1*dc->offset);
	else
		return clock_time() + dc->offset;
}

void dclock_get_offset(struct Dclock *dc,long int *i)
{
	*i = dc->offset;
}
void dclock_print(struct Dclock *dc)
{
	PRINTF("[Dclock] offset : %ld\n",dc->offset);
}

void dclock_print_time(struct Dclock *dc)
{
#if DEBUG_DCLOCK
#if PLATFORM_L == MICAZ_L
	int ms,sec,min,hour;
	clock_time_t day;
	clock_time_t temp_time = dclock_get_time(dc);
//	printf("%u ",temp_time % CLOCK_SECOND);
//	printf("%d %d \n",temp_time,CLOCK_SECOND);
	ms = (int)(temp_time%CLOCK_SECOND * 7.8125);
	printf("ms : %d ",ms);
	temp_time = temp_time/CLOCK_SECOND;
	sec = temp_time%60;
	printf("sec : %d \n",sec);
	temp_time = temp_time/60;
	min = temp_time%60;
	temp_time = temp_time/60;
	hour = temp_time % 24;
	day = temp_time/24;

	printf("[Dclock] Day|Hour|Min|Sec|Msec\n");
	printf("[Dclock] %3d|%4d|%3d|%3d|%4d\n",day,hour,min,sec,ms);
#else
	int ms,sec,min,hour;
	clock_time_t day;
	clock_time_t temp_time = dclock_get_time(dc);
	printf("%ld %ld \n",temp_time,CLOCK_SECOND);
	ms = temp_time % CLOCK_SECOND;
	printf("ms : %d ",ms);
	temp_time = temp_time/CLOCK_SECOND;
	sec = temp_time%60;
	printf("sec : %d \n",sec);
	temp_time = temp_time/60;
	min = temp_time%60;
	temp_time = temp_time/60;
	hour = temp_time % 24;
	day = temp_time/24;

	printf("[Dclock] Day|Hour|Min|Sec|Msec\n");
	printf("[Dclock] %3ld|%4d|%3d|%3d|%4d\n",day,hour,min,sec,ms);
#endif /* PLATFORM_L == MICAZ_L */
#endif /* DEBUG_DCLOCK */
}

void dclock_print_time_stamp(struct Time_stamp time_stamp)
{
#if PLATFORM_L == MICAZ_L
	PRINTF("[Dclock] t1:%u\t  t2:%u\t  t3:%u\t  t4:%u\n",
			time_stamp.t1, time_stamp.t2, time_stamp.t3, time_stamp.t4);
#else
	PRINTF("[Dclock] t1:%ld\t  t2:%ld\t  t3:%ld\t  t4:%ld\n",
				time_stamp.t1, time_stamp.t2, time_stamp.t3, time_stamp.t4);
#endif

}

int dclock_synchronize(struct Dclock *dc,struct Time_stamp ts)
{
//	long int temp = (long int)((ts.t2 - ts.t1 - ts.t4 + ts.t3)/2);
//	dc->offset += temp;
#if PLATFORM_L==MICAZ_L
	dc->offset += ((long int)ts.t2 - (long int)ts.t1 - (long int)ts.t4 + (long int)ts.t3)/2;
	PRINTF("[Dclock] offset : %ld current time: %d\n",dc->offset,dclock_get_time(dc));
	PRINTF("[SYNC] %ld\n",((long int)ts.t2 - (long int)ts.t1 - (long int)ts.t4 + (long int)ts.t3)/2);
#else
	dc->offset += (long int)(ts.t2 - ts.t1 - ts.t4 + ts.t3)/2;
	PRINTF("[Dclock] offset : %ld current time: %ld\n",dc->offset,dclock_get_time(dc));
	PRINTF("[SYNC] %ld\n",(long int)(ts.t2 - ts.t1 - ts.t4 + ts.t3)/2);
#endif
	return (int)(((long int)ts.t2 - (long int)ts.t1 - (long int)ts.t4 + (long int)ts.t3)/2);
}


