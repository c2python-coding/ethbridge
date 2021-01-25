#ifndef ERRORHANDLER_H
#define ERRORHANDLER_H

#define CONDITION_FAIL 0
#define CONDITION_SUCCEED 1

void error_wrapper(int condition, const char *message, void* user);

void register_error_handler(void(*error_function)(char*, void*));

#endif