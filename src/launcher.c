#include "forward/forward.h"
#include "capture/capture.h"
#include "processing/processing.h"
#include "utils/easyprint.h"
#include "utils/utils.h"
#include <net/if.h>
#include <pcap.h>
#include <stdlib.h>
#include <string.h>
#include <sys/socket.h>
#include <unistd.h>
#include <errno.h>
#include <signal.h>



static char MAC_FLAG = 1;
static char INTERFACE_FLAG = 1 << 1;
static char OUTPUT_FLAG = 1 << 2;
static const char REQUIRED_FLAGS = 7;

static const int MAC_FILTER_SIZE = 18;

static const int FORWARD_SPEC_SIZE = 12;

static const char *USAGE_STR = "Usage: ethbridge -i interface, -m mac_address, -o output_spec\n\
  interface: regex of the interface to use\n\
  mac_address: mac address of the other end of the bridge\n\
  output_spec: output specification [STDIO,TCP,UDP]:[C,L]:[PORT]\n";

CaptureSpec capture_interface;
ForwardFileDescriptors forward_fds;

static void ctrl_c_handler(int sitnalnum) { 
    abort_process_loop();
    }

//TODO move this to respective modules and register the cleanup functions
static void clean_connections()
{
    if (capture_interface.capture_handle != NULL)
    {
        pcap_close(capture_interface.capture_handle);
    }
    if (forward_fds.read_fd > 2)
    {
        shutdown(forward_fds.read_fd,SHUT_RDWR);
        close(forward_fds.read_fd);
    }
    if (forward_fds.write_fd > 2)
    {
        shutdown(forward_fds.write_fd,SHUT_RDWR);
        close(forward_fds.write_fd);
    }
}

void error_handler(char *message, void *user)
{
    print(message);
    if (errno>0)
    {
        print("Error number (could be irrelevant): ", errno);
    }
    clean_connections();
    exit(1);
}


void launch_bridge()
{
    struct sigaction ctrlc_action;
    memset(&ctrlc_action, 0, sizeof(struct sigaction));
    ctrlc_action.sa_handler = &ctrl_c_handler;
    error_wrapper(sigaction(SIGINT, &ctrlc_action, NULL) == 0, "couldn't register ctrl-c handler", NULL);
    
}

int main(int argc, char **argv)
{

    register_error_handler(&error_handler);
    memset(&capture_interface, 0, sizeof(CaptureSpec));
    forward_fds.read_fd = -1;
    forward_fds.read_fd = -1;

    int opt;

    char mac_string[MAC_FILTER_SIZE];
    char interface_regex[MAC_FILTER_SIZE];
    char forward_spec[FORWARD_SPEC_SIZE];
    memset(mac_string, 0, MAC_FILTER_SIZE);
    memset(interface_regex, 0, IFNAMSIZ);
    memset(forward_spec, 0, FORWARD_SPEC_SIZE);

    char specifed_flags = 0;
    while ((opt = getopt(argc, argv, "i:m:o:")) != -1)
    {
        switch (opt)
        {
        case 'i':
            strncpy(interface_regex, optarg, MAC_FILTER_SIZE - 1);
            specifed_flags = (specifed_flags | INTERFACE_FLAG);
            break;
        case 'm':
            strncpy(mac_string, optarg, MAC_FILTER_SIZE - 1);
            specifed_flags = (specifed_flags | MAC_FLAG);
            break;
        case 'o':
            strncpy(forward_spec, optarg, FORWARD_SPEC_SIZE - 1);
            specifed_flags = (specifed_flags | OUTPUT_FLAG);
            break;
        default:
            print(USAGE_STR);
            exit(1);
        }
    }
    error_wrapper(specifed_flags == REQUIRED_FLAGS, USAGE_STR, NULL);
    get_capture_spec(&capture_interface, interface_regex, mac_string);
    get_forwarding_spec(&forward_fds, forward_spec);
    start_process_loop(&capture_interface,&forward_fds);
    clean_connections();
    simple_log("Done");
    return 0;
}