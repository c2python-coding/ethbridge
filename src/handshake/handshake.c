#include "handshake.h"
#include "../forward/forward.h"
#include "../utils/utils.h"
#include "assert.h"
#include "net/ethernet.h"
#include "pcap.h"
#include "unistd.h"
#include <arpa/inet.h>
#include <string.h>

#define HANDSHAKE_SEND 1
#define HANDSHAKE_WAIT 2

typedef enum
{
    ERROR = 0,
    PINGING = 1,
    FORWARD_READY = 2,
    CONFIRMATION = 3,
    DONE = 4

} HandshakeState;

static volatile HandshakeState own_state;

static CaptureSpec *capture = NULL;

static u_char packet_buffer[ETHER_HDR_LEN + 1];
static u_char *data = packet_buffer + ETHER_HDR_LEN;

static volatile int remote_match = 0;

void send_state(HandshakeState state)
{
    assert(capture != NULL);
    *data = (u_int)state;
    error_wrapper(pcap_inject(capture->capture_handle, packet_buffer, ETHER_HDR_LEN + 1) > 1, "couldnt send handshake packet", NULL);
}

static void handshake_callback(u_char *user, const struct pcap_pkthdr *h, const u_char *bytes)
{
    ether_header_ptr packet_header = (ether_header_ptr)bytes;
    HandshakeState recieved_state;
    HandshakeState look_for = (HandshakeState)(*user);
    if (ntohs(packet_header->ether_type) == HANDSHAKE_ETHER_PROTOCOL)
    {
        recieved_state = (HandshakeState)(*(bytes + ETHER_HDR_LEN));
        if (recieved_state == look_for)
        {
            remote_match = 1;
            send_state(look_for);
        }
    }
}

void establish_handshake(CaptureSpec *capture_interface, ForwardFileDescriptors *forward_fds)
{
    assert(capture_interface->capture_handle != NULL);
    capture = capture_interface;
    prepare_response_buffer(packet_buffer, capture_interface->own_mac, capture_interface->dest_mac, HANDSHAKE_ETHER_PROTOCOL);
    char message[30];
    memset(message, 0, 30);

    own_state = PINGING;
    strcpy(message, "Sending Ping");
    remote_match = 0;
    do
    {
        if (timer_check(HANDSHAKE_MS_WAIT))
        {
            simple_log(message);
            send_state(own_state);
            timer_start();
        }
        pcap_dispatch(capture->capture_handle, 0, &handshake_callback, (u_char *)(&own_state));
        if (remote_match)
        {
            switch (own_state)
            {
            case PINGING:
                own_state = FORWARD_READY;
                simple_log("Establishing Local Forwarding");
                connect_sockets(forward_fds);
                strcpy(message, "Sending Forward Ready");
                break;
            case FORWARD_READY:
                own_state = CONFIRMATION;
                strcpy(message, "Sending Final Confirm");
            case CONFIRMATION:
                own_state = DONE;
            case DONE:
            default:
                break;
            }
            remote_match = 0;
        }
    }while (own_state != DONE);
    simple_log("Handshake Complete");
}