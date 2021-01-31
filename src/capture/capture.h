#ifndef CAPTURE_H
#define CAPTURE_H
#include <pcap.h>

#define CUSTOM_ETHER_PROTOCOL 0xffff

typedef struct __CaptureSpec
{
    u_char own_mac[6];
    u_char dest_mac[6];
    pcap_t* capture_handle;
} CaptureSpec;

void get_capture_spec(CaptureSpec * target, char* interface_regex, char* bind_mac);

#endif



