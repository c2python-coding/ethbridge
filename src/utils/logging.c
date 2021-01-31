#include "utils.h"
#include <stdio.h>

//TODO improve this
void simple_log(char *log_string)
{
    fprintf(stderr,"=>");
    fprintf(stderr, "%s\n", log_string);
}

