#include "handshake.h"
#include "../utils/utils.h"
#include "../forward/forward.h"
#include "assert.h"
#include "net/ethernet.h"
#include "pcap.h"


typedef enum
{
    INIT = 0,
    PING = 1,
    LOCAL_COMMS_READY = 2,
    CONFIRMATION = 3,
    DONE = 4

} HandshakeState;

static volatile HandshakeState own_state;
static pcap_t *handle = NULL;
static ForwardFileDescriptors * local_fds;

static u_char packet_buffer[ETHER_HDR_LEN+1];
static u_char *data = packet_buffer+ETHER_HDR_LEN;


void send_state(HandshakeState state)
{
    *data = (u_int)state;
    pcap_inject(handle, packet_buffer, ETHER_HDR_LEN + 1);
}


void handle_remote_state(HandshakeState recieved_state)
{
    switch (recieved_state)
    {
        case INIT:
            error_wrapper(CONDITION_FAIL,"Recieved an erroneous state from remote",NULL);
            break;
        case PING:
            simple_log("Ping Recieved");
            own_state = LOCAL_COMMS_READY;
            break;
        case LOCAL_COMMS_READY:
            simple_log("Remote side forwarding connected");
            own_state = CONFIRMATION;
            break;
        case CONFIRMATION:
            simple_log("Recieved confirmation");
            own_state = DONE;
            break;
        case DONE:
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
            simple_log("Sending Ping");
            send_state(PING);
            break;
        case LOCAL_COMMS_READY:
            simple_log("Setting up local connections");
            connect_sockets(local_fds);
            send_state(LOCAL_COMMS_READY);
            break;
        case CONFIRMATION:
            simple_log("Recieved confirmation");
            send_state(CONFIRMATION);
            own_state = DONE;
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
    if (packet_header->ether_type == HANDSHAKE_ETHER_PROTOCOL)
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