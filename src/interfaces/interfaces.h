#ifndef INTERFACES_H
#define INTERFACES_H
#include <pcap.h>

typedef struct
{
    u_char own_mac[6];
    u_char dest_mac[6];
    pcap_t* capture_handle;
} CaptureSpec;


void setup_capture_spec(CaptureSpec * target, char* interface_regex, char* bind_mac);

#endif



