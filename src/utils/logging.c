#include "utils.h"
#include <stdio.h>

//TODO improve this
static int enable_log = 0;

void simple_log(char *log_string)
{
    if (enable_log)
    {
        fprintf(stderr, "=>%s\n", log_string);
    }
}

void enable_logging()
{
    enable_log = 1;
}
