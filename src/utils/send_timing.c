#include "utils.h"
#include <sys/time.h>
#include <stdlib.h>

static struct timeval last_time;
volatile static int running = 0;

static long convert_to_microseconds(struct timeval *input)
{
    return ((long)input->tv_sec *1000000l + (long)input->tv_usec);
}

static long  ms_difference(struct timeval *end_time, struct timeval *start_time)
{
    static struct timeval diff_time;
    long microsecond_diff;
    timersub(end_time,start_time,&diff_time);
    microsecond_diff = convert_to_microseconds(&diff_time);
    return microsecond_diff;
}

void timer_start()
{
    gettimeofday(&last_time, NULL);
    running = 1;
}

void timer_stop()
{
    running = 0;
}

void timer_wait(long milliseconds)
{
    long wait_microsec = milliseconds*1000;
    if (!running)
    {
        return;
    }
    struct timeval c_time;
    do {
        gettimeofday(&c_time, NULL);
    }while (ms_difference(&c_time, &last_time) < wait_microsec);
}

int timer_check(long milliseconds)
{
    long wait_microsec = milliseconds*1000;
    if (!running)
    {
        return 1;
    }
    struct timeval c_time;
    gettimeofday(&c_time, NULL);
    if(ms_difference(&c_time, &last_time) < wait_microsec)
    {
        return 0;
    }
    return 1;
}
