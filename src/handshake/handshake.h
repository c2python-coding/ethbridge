#ifndef HANDSHAKE_H
#define HANDSHAKE_H

#include "../capture/capture.h"
#include "../forward/forward.h"

#define HANDSHAKE_ETHER_PROTOCOL 0xfeed

#define HANDSHAKE_MS_WAIT 500

void establish_handshake(CaptureSpec *capture_interface, ForwardFileDescriptors *forward_fds);

#endif