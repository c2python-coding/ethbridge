#ifndef FORWARD_H
#define FORWARD_H
#include <sys/types.h>


typedef struct
{
    int write_fd;
    int read_fd;
} ForwardFileDescriptors;

void setup_forward_connection(ForwardFileDescriptors *forward_info, char* capture_spec_string);


#endif