#ifndef FORWARD_H
#define FORWARD_H
#include <sys/types.h>

typedef enum
{
    NA = 0,
    LISTEN = 1,
    CONNECT = 2
} InitType;

typedef enum
{
    NONE = 0,
    STDIO = 1,
    TCP = 2,
    UDP = 3
} LocalCommsType;

typedef struct
{
    LocalCommsType comms_type;
    InitType init_type;
    u_short port;
} LocalCommsConfig;

typedef struct
{
    int write_fd;
    int read_fd;
    LocalCommsConfig comms_config;
} ForwardFileDescriptors;

void connect_sockets(ForwardFileDescriptors *forward_info);


void get_forwarding_spec(ForwardFileDescriptors *forward_info, char* capture_spec_string);


#endif