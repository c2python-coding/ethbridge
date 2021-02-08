#include "utils.h"
#include "easyprint.h"
#include <pcap.h>
#include <string.h>
#include <stdlib.h>
#include <assert.h>

#define ETHBRIDGE_ERROR_BUFFER_LEN PCAP_ERRBUF_SIZE

static char ETHBRIDGE_ERROR_BUFFER[ETHBRIDGE_ERROR_BUFFER_LEN];

static void (*error_function)(char*, void*) = NULL;

void error_wrapper(int condition, const char *message, void *user)
{
    assert(error_function != NULL);
    if (!condition)
    {
        strncpy(ETHBRIDGE_ERROR_BUFFER, message, ETHBRIDGE_ERROR_BUFFER_LEN);
        ETHBRIDGE_ERROR_BUFFER[ETHBRIDGE_ERROR_BUFFER_LEN - 1] = '\x00';
        (*error_function)(ETHBRIDGE_ERROR_BUFFER, user);
    }
}

void register_error_handler(void (*custom_function)(char*, void*))
{
    if (!error_function)
    {
        error_function = custom_function;
    }
    else
    {
        print("ERROR: REDEFINITION OF ERROR HANDLER");
        exit(1);
    }
}
