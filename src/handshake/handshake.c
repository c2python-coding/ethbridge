#include "handshake.h"
#include "../utils/utils.h"
#include "../forward/forward.h"
#include "assert.h"
#include "net/ethernet.h"
#include "pcap.h"
#include "unistd.h"
#include <arpa/inet.h>

typedef enum
{
    INIT = 0,
    PINGING = 1,
    HANDSHAKE_READY = 2,
    LOCAL_COMMS_READY = 2,
    CONFIRMATION = 3,
    DONE = 4

} HandshakeState;

static volatile HandshakeState own_state;
static pcap_t *handle = NULL;
static ForwardFileDescriptors * local_fds;

static u_char packet_buffer[ETHER_HDR_LEN+1];
static u_char *data = packet_buffer+ETHER_HDR_LEN;

void send_state(HandshakeState state, char* message)
{   
    timer_wait(500);
    *data = (u_int)state;
    simple_log(message);
    error_wrapper(pcap_inject(handle, packet_buffer, ETHER_HDR_LEN + 1)>1,"couldnt send handshake packet",NULL);
    timer_start();
}

void handle_remote_state(HandshakeState recieved_state)
{
    switch (recieved_state)
    {
        case PING:
            simple_log("Recieved PING");
            if (own_state == PING) own_state = LOCAL_COMMS_READY;
            timer_stop();
            break;
        case LOCAL_COMMS_READY:
            simple_log("Recieved CONNECTIONS READY");
            if (own_state == LOCAL_COMMS_READY)  own_state = CONFIRMATION;
            timer_stop();
            break;
        case CONFIRMATION:
            simple_log("Recieved CONFIRM");
            if (own_state == CONFIRMATION) own_state = DONE;
            timer_stop();
            break;
        case INIT:
        case DONE:
        default:
            error_wrapper(CONDITION_FAIL,"Recieved an erroneous state from remote",NULL);
            break;
    }
}

void handle_own_state()
{
    switch (own_state)
    {
        case INIT:
            own_state = PING;
            break;
        case PING:
            send_state(PING,"Sending PING");
            break;
        case LOCAL_COMMS_READY:
            if (local_fds->read_fd <0 || local_fds->write_fd<0)
            {
                simple_log("Setting up local connections");
                connect_sockets(local_fds);
            }
            send_state(LOCAL_COMMS_READY,"Sending CONNECTIONS READY");
            break;
        case CONFIRMATION:
            send_state(CONFIRMATION,"Sending CONFIRM");
            break;
        case DONE:
            break;
    }
}

static void handshake_callback(u_char *user, const struct pcap_pkthdr *h, const u_char *bytes)
{
    ether_header_ptr packet_header = (ether_header_ptr)bytes;
    const u_char *data = bytes + ETHER_HDR_LEN;
    HandshakeState recieved_state = (HandshakeState)(*data);
    if (ntohs(packet_header->ether_type) == HANDSHAKE_ETHER_PROTOCOL)
    {
        handle_remote_state(recieved_state);
    }
}


void establish_handshake(CaptureSpec *capture_interface, ForwardFileDescriptors *forward_fds)
{
    simple_log("Handshake Starting");
    assert(capture_interface->capture_handle != NULL);
    handle  = capture_interface->capture_handle;
    local_fds = forward_fds;
    prepare_response_buffer(packet_buffer,capture_interface->own_mac,capture_interface->dest_mac,HANDSHAKE_ETHER_PROTOCOL);
    own_state = INIT;
    while (own_state != DONE)
    {
        pcap_dispatch(capture_interface->capture_handle, 0, &handshake_callback, NULL);
        handle_own_state();
    }
    simple_log("Handshake Complete");
}