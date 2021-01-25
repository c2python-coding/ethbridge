#include "forward.h"
#include "../utils/errorutils.h"
#include <arpa/inet.h>
#include <assert.h>
#include <ctype.h>
#include <fcntl.h>
#include <netinet/in.h>
#include <stdlib.h>
#include <string.h>
#include <sys/socket.h>
#include <unistd.h>
#include <stdio.h>

typedef enum
{
    NONE = 0,
    STDIO = 1,
    TCP = 2,
    UDP = 3
} LocalCommsType;

typedef enum
{
    NA = 0,
    LISTEN = 1,
    CONNECT = 2
} InitType;

typedef struct
{
    LocalCommsType comms_type;
    InitType init_type;
    u_short port;
} LocalCommsConfig;

static const char *generic_error_string = "Output specification must be either STDIO, TCP:[L,H]:[port] or UDP:[port]";

static void validate_config(LocalCommsConfig *config)
{
    error_wrapper(config->comms_type != NONE, generic_error_string, NULL);
    if (config->comms_type == STDIO)
    {
        error_wrapper((config->init_type == NA && config->port == 0), "STDOUT output specifer must be by itself", NULL);
    }
    else if (config->comms_type == UDP)
    {
        error_wrapper(config->init_type == NA, "UDP output specifer cannot have listen or connect", NULL);
        error_wrapper(config->port != 0, "UDP output specifer cannot have listen or connect", NULL);
    }
    else if (config->comms_type == TCP)
    {
        error_wrapper(config->init_type != NA, "TCP output specifer must be contain (L)isten or (C)onnect", NULL);
        error_wrapper(config->port != 0, "TCP output specifer must have a port", NULL);
    }
}

static void split_socket_spec(LocalCommsConfig *config, char *config_str)
{
    memset(config, 0, sizeof(LocalCommsConfig));

    int config_str_len = strlen(config_str);
    char *upperstring = (char *)malloc(config_str_len + 1);
    memset(upperstring, 0, config_str_len + 1);
    strncpy(upperstring, config_str, config_str_len);
    char *it, *end_it;
    for (it = upperstring, end_it = upperstring + config_str_len; it < end_it; ++it)
    {
        *it = toupper(*it);
    }

    it = strtok(upperstring, ":");
    error_wrapper(it != NULL, generic_error_string, NULL);
    int total_tokens = 0;
    int port_num;
    while (it != NULL)
    {
        total_tokens += 1;
        error_wrapper(total_tokens <= 3, generic_error_string, NULL);
        if (strcmp(it, "STDIO") == 0)
        {
            config->comms_type = STDIO;
            config->init_type = NA;
        }

        if (strcmp(it, "TCP") == 0)
            config->comms_type = TCP;
        if (strcmp(it, "UDP") == 0)
            config->comms_type = UDP;
        if (strcmp(it, "L") == 0)
            config->init_type = LISTEN;
        if (strcmp(it, "C") == 0)
            config->init_type = CONNECT;
        port_num = atoi(it);
        if (port_num > 0)
        {
            config->port = port_num;
        }

        it = strtok(NULL, ":");
    }
    free(upperstring);
}


void setup_forward_connection(ForwardFileDescriptors *forward_info, char *capture_spec_string)
{
    LocalCommsConfig output_config;
    forward_info->read_fd = -1;
    forward_info->write_fd = -1;
    struct sockaddr_in address;
    struct sockaddr_in comm_address;
    u_int address_size = sizeof(address);
    int socket_fd =- 1;
    int flags;
    int comm_socket =- 1;
    int option;


    split_socket_spec(&output_config, capture_spec_string);
    validate_config(&output_config);
    if (output_config.comms_type == STDIO)
    {
        forward_info->read_fd = STDIN_FILENO;
        flags = fcntl(forward_info->read_fd, F_GETFL, 0);
        error_wrapper(fcntl(forward_info->read_fd, F_SETFL, flags | O_NONBLOCK) == 0, "Could not set stdin to nonblock", NULL);
        forward_info->write_fd = STDOUT_FILENO;
        return;
    }
    address.sin_family = AF_INET;
    address.sin_port = htons(output_config.port);
    if (output_config.comms_type == TCP)
    {
        error_wrapper(inet_pton(AF_INET, "127.0.0.1", &address.sin_addr)>0,"Could not set socket address",NULL);
        socket_fd = socket(AF_INET, SOCK_STREAM, IPPROTO_TCP);
        error_wrapper(socket_fd >2, "Could not create socket", NULL);
        option = 1;
        error_wrapper(setsockopt(socket_fd, SOL_SOCKET, SO_REUSEADDR, &option, sizeof(option))==0, "Could not set addr to nonblock", NULL);
        if (output_config.init_type == LISTEN)
        {
           
            error_wrapper(bind(socket_fd, (struct sockaddr *)&address, address_size) == 0, "Could not bind tcp socket", NULL);
            error_wrapper(listen(socket_fd, 1) == 0, "Could not start socket in listen mode", NULL);
            fprintf(stderr, "=>Waiting for tcp connection....\n");
            comm_socket = accept(socket_fd, (struct sockaddr *)&comm_address, &address_size);
            error_wrapper(comm_socket > 2, "Could not accept incoming connection", NULL);
            close(socket_fd);
        }
        else if (output_config.init_type == CONNECT)
        {
            comm_socket = socket_fd;
            error_wrapper(connect(comm_socket, (struct sockaddr *)&address, address_size)==0,"Could not connect socket", NULL);
        }
        forward_info->read_fd = comm_socket;
        forward_info->write_fd = comm_socket;
        flags = fcntl(comm_socket, F_GETFL, 0);
        error_wrapper(fcntl(comm_socket, F_SETFL, flags | O_NONBLOCK) >= 0, "Could not set socket to nonblock", NULL);
    }
    else
    {
        assert(output_config.comms_type == UDP);
        error_wrapper(0, "Sorry UDP is not implemented yet", NULL); // TODO
    }
}
