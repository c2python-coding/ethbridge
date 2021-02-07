#include "processing.h"
#include "../handshake/handshake.h"
#include "../utils/utils.h"
#include <assert.h>
#include <fcntl.h>
#include <net/ethernet.h>
#include <pcap.h>
#include <string.h>
#include <unistd.h>


#ifdef OSX
#define ETH_DATA_LEN 1500
#endif

volatile int PROCESS_LOOP_CONTINUE = 1;



static u_char packet_buffer[ETHER_HDR_LEN + ETH_DATA_LEN];
static u_char *const data_ptr = packet_buffer + ETHER_HDR_LEN;
static ForwardFileDescriptors *fds;

void abort_process_loop()
{
    PROCESS_LOOP_CONTINUE = 0;
}

static void packet_callback(u_char *user, const struct pcap_pkthdr *h, const u_char *bytes)
{
    ether_header_ptr packet_header = (ether_header_ptr)bytes;
    const u_char *data;
    int data_len;
    if (packet_header->ether_type == CUSTOM_ETHER_PROTOCOL)
    {
        data = bytes + ETHER_HDR_LEN;
        data_len = (h->caplen) - ETHER_HDR_LEN;
        write(fds->write_fd, (void *)data, data_len);
    }
}

void start_process_loop(CaptureSpec *capture_interface, ForwardFileDescriptors *forward_fds)
{
    int num_bytes_in;
    fds = forward_fds;
    prepare_response_buffer(packet_buffer, capture_interface->own_mac, capture_interface->dest_mac, CUSTOM_ETHER_PROTOCOL);
    assert(fds->read_fd >= 0);
    assert(fds->write_fd >= 0);
    assert(capture_interface->capture_handle != NULL);
    while (PROCESS_LOOP_CONTINUE)
    {
        num_bytes_in = read(fds->read_fd, data_ptr, ETH_DATA_LEN);
        if (num_bytes_in > 0)
        {
            pcap_inject(capture_interface->capture_handle, packet_buffer, ETHER_HDR_LEN + num_bytes_in);
        }
        pcap_dispatch(capture_interface->capture_handle, 0, &packet_callback, NULL);
    }
}
