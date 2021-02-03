#include "utils.h"
#include <sys/time.h>
#include <stdlib.h>

static struct timeval last_time;
volatile static int running = 0;


static long convert_to_microseconds(struct timeval *input)
{
    return ((long)input->tv_sec *1000000l + (long)input->tv_usec);
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
    static struct timeval c_time;
    static struct timeval diff_time;
    long microsecond_diff;
    do {
        gettimeofday(&c_time, NULL);
        timersub(&c_time,&last_time,&diff_time);
        microsecond_diff = convert_to_microseconds(&diff_time);

    }while (microsecond_diff < wait_microsec);
}

