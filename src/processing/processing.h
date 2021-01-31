#ifndef PROCESSING_H
#define PROCESSING_H

#include "../capture/capture.h"
#include "../forward/forward.h"

#define CUSTOM_ETHER_PROTOCOL 0xffff

void start_process_loop(CaptureSpec *capture_interface, ForwardFileDescriptors* forward_fds);
void abort_process_loop();

#endif