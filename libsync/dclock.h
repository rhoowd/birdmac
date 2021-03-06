#ifndef DCLOCK_H
#define DCLOCK_H

#include "param.h"
#include "contiki.h"
#include "sys/rtimer.h"
#include "sys/etimer.h"
#include "sys/pt.h"

struct Dclock {
  long int offset;
};
struct Time_stamp {
  clock_time_t t1;
  clock_time_t t2;
  clock_time_t t3;
  clock_time_t t4;
};

static struct Dclock dclock;

void dclock_init(struct Dclock **dc);

clock_time_t dclock_get_time(struct Dclock *dc);
void dclock_print_time(struct Dclock *dc);
void dclock_print_time_stamp(struct Time_stamp time_stamp);
int dclock_synchronize(struct Dclock *dc,struct Time_stamp time_stamp);
void dclock_get_offset(struct Dclock *dc,long int *i);
void dclock_print(struct Dclock *dc);

#endif /* DCLOCK_H */
