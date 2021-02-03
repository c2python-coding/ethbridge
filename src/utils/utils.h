#ifndef ERRORHANDLER_H
#define ERRORHANDLER_H

#include <sys/types.h>
#include <net/ethernet.h>

#define CONDITION_FAIL 0
#define CONDITION_SUCCEED 1



typedef struct ether_header *ether_header_ptr;

void error_wrapper(int condition, const char *message, void* user);

void register_error_handler(void(*error_function)(char*, void*));

void prepare_response_buffer(u_char* buffer, u_char* source_mac, u_char* dest_mac, u_short protocol);

void simple_log(char *log_string);

void simple_log(char *log_string);

void timer_start();

void timer_stop();

void timer_wait(long milliseconds);


#endif