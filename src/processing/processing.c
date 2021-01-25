#include "processing.h"
#include "../utils/errorutils.h"
#include <string.h>
#include <pcap.h>
#include <net/ethernet.h>
#include <unistd.h>
#include <assert.h>
#include <fcntl.h>
#include <signal.h>


#define CUSTOM_ETHER_PROTOCOL 0xffff

static u_char packet_buffer[ETHER_HDR_LEN+ETH_DATA_LEN];
static u_char * const data_ptr= packet_buffer+ETHER_HDR_LEN;

typedef  struct ether_header* ether_header_t;

static ForwardFileDescriptors * fds;

volatile static int contin = 1;

static void ctrl_c_handler(int sitnalnum)
{
    contin = 0;
}

static void prepare_response_buffer(CaptureSpec *interface)
{
    ether_header_t packet_header = (ether_header_t)packet_buffer;
    memcpy(packet_header->ether_dhost, interface->dest_mac,6);
    memcpy(packet_header->ether_shost,interface->own_mac,6);
    packet_header->ether_type=(unsigned short)CUSTOM_ETHER_PROTOCOL;
}


static void packet_callback(u_char *user, const struct pcap_pkthdr *h,
                     const u_char *bytes)
{
    ether_header_t packet_header = (ether_header_t)bytes;
    const u_char* data = bytes + sizeof(struct ether_header);
    int data_len = (h->caplen)-(sizeof(struct ether_header));
    if (packet_header->ether_type == 0xffff) //TODO move this into BPF
    {
        write(fds->write_fd,(void*) data, data_len);
    }

}

void start_loop(CaptureSpec *capture_interface, ForwardFileDescriptors* forward_fds)
{
    int num_bytes_stdin;
    fds = forward_fds;
    prepare_response_buffer(capture_interface);
    struct sigaction ctrlc_action;
    memset(&ctrlc_action, 0, sizeof(struct sigaction));
    ctrlc_action.sa_handler = &ctrl_c_handler;
    error_wrapper(sigaction(SIGINT,&ctrlc_action,NULL)==0, "couldn't register ctrl-c handler",NULL);
    fprintf(stderr, "=>Started\n");
    while (contin)
    {
        num_bytes_stdin = read(fds->read_fd,data_ptr,ETH_DATA_LEN);
        if (num_bytes_stdin > 0)
        {
            pcap_inject(capture_interface->capture_handle, packet_buffer, ETHER_HDR_LEN+num_bytes_stdin);
        }
        pcap_dispatch(capture_interface->capture_handle, 0, &packet_callback, NULL);
    }
}




