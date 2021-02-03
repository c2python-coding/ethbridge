#include "utils.h"
#include <string.h>
#include <arpa/inet.h>


void prepare_response_buffer(u_char* buffer, u_char* source_mac, u_char* dest_mac, u_short protocol)
{
    ether_header_ptr packet_header = (ether_header_ptr)buffer;
    memcpy(packet_header->ether_dhost, dest_mac, 6);
    memcpy(packet_header->ether_shost, source_mac, 6);
    packet_header->ether_type = htons((unsigned short)protocol);
}