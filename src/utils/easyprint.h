#ifndef LIBPRINT_H
#define LIBPRINT_H

#include <stdio.h>
#include <stdarg.h>

typedef unsigned int _ep_uint;
typedef unsigned short _ep_ushort;
typedef unsigned char _ep_uchar;
typedef unsigned long _ep_ulong;

typedef enum
{
    _easyprint_TYPE_CHAR,
    _easyprint_TYPE_UCHAR,
    _easyprint_TYPE_LONG,
    _easyprint_TYPE_ULONG,
    _easyprint_TYPE_DOUBLE,
    _easyprint_TYPE_STRING,
    _easyprint_TYPE_POINTER,
    _easyprint_TYPE_DEFAULT,
    _easyprint_TYPE_TERMINATOR
} _easyprint_ARGTYPE;

#define _easyprint_typename(x)                                                   \
    _Generic((x),                     \
                                        int                      \
                                      : _easyprint_TYPE_LONG,    \
                                        short                  \
                                      : _easyprint_TYPE_LONG,    \
                                        _ep_uint                   \
                                      : _easyprint_TYPE_ULONG,    \
                                        _ep_ushort                 \
                                      : _easyprint_TYPE_ULONG,    \
                                        char                   \
                                      : _easyprint_TYPE_CHAR,    \
                                        _ep_uchar                  \
                                      : _easyprint_TYPE_UCHAR,    \
                                        long                   \
                                      : _easyprint_TYPE_LONG,    \
                                        _ep_ulong                  \
                                      : _easyprint_TYPE_ULONG,    \
                                        char *                 \
                                      : _easyprint_TYPE_STRING,  \
                                        const char *                 \
                                      : _easyprint_TYPE_STRING,  \
                                        _ep_uchar *                \
                                      : _easyprint_TYPE_POINTER, \
                                        const _ep_uchar *                \
                                      : _easyprint_TYPE_POINTER, \
                                        int *                  \
                                      : _easyprint_TYPE_POINTER, \
                                        const int *                  \
                                      : _easyprint_TYPE_POINTER, \
                                        short *                \
                                      : _easyprint_TYPE_POINTER, \
                                       const short *                \
                                      : _easyprint_TYPE_POINTER, \
                                        _ep_uint *                 \
                                      : _easyprint_TYPE_POINTER, \
                                       const _ep_uint *                 \
                                      : _easyprint_TYPE_POINTER, \
                                        _ep_ushort *               \
                                      : _easyprint_TYPE_POINTER, \
                                      const _ep_ushort *               \
                                      : _easyprint_TYPE_POINTER, \
                                        long *                 \
                                      : _easyprint_TYPE_POINTER, \
                                        const long *                 \
                                      : _easyprint_TYPE_POINTER, \
                                        _ep_ulong *                \
                                      : _easyprint_TYPE_POINTER, \
                                        const _ep_ulong *                \
                                      : _easyprint_TYPE_POINTER, \
                                        float                  \
                                      : _easyprint_TYPE_DOUBLE,  \
                                        double                 \
                                      : _easyprint_TYPE_DOUBLE,  \
                                        float *                \
                                      : _easyprint_TYPE_POINTER, \
                                        const float *                \
                                      : _easyprint_TYPE_POINTER, \
                                        double *               \
                                      : _easyprint_TYPE_POINTER, \
                                       const double *               \
                                      : _easyprint_TYPE_POINTER, \
                                        void *                 \
                                      : _easyprint_TYPE_POINTER, \
                                        const void *          \
                                      : _easyprint_TYPE_TERMINATOR, \
                                        default                \
                                      : _easyprint_TYPE_DEFAULT)

char _easyprint_charvar;
_ep_uchar _easyprint_ucharvar;
long _easyprint_longvar;
_ep_ulong _easyprint_ulongvar;
double _easyprint_doublevar;
char *_easyprint_stringvar;
void *_easyprint_pointervar;
_ep_ulong _easyprint_objval;

// #define _ELEVENTH_ARGUMENT(a1, a2, a3, a4, a5, a6, a7, a8, a9, a10, a11, ...)
// a11 #define NARGS(...) _ELEVENTH_ARGUMENT(placeholder, ##__VA_ARGS__, 9, 8,
// 7, 6, 5, 4, 3, 2, 1, 0)

static const void *_easyprint_dummy;
static _easyprint_ARGTYPE _easyprint_arg_array[10];

#define _easyprint_BASE_TYPES(a1, a2, a3, a4, a5, a6, a7, a8, a9, a10, ...)      \
    _easyprint_arg_array[0] = _easyprint_typename(a1);                             \
    _easyprint_arg_array[1] = _easyprint_typename(a2);                             \
    _easyprint_arg_array[2] = _easyprint_typename(a3);                             \
    _easyprint_arg_array[3] = _easyprint_typename(a4);                             \
    _easyprint_arg_array[4] = _easyprint_typename(a5);                             \
    _easyprint_arg_array[5] = _easyprint_typename(a6);                             \
    _easyprint_arg_array[6] = _easyprint_typename(a7);                             \
    _easyprint_arg_array[7] = _easyprint_typename(a8);                             \
    _easyprint_arg_array[8] = _easyprint_typename(a9);                             \
    _easyprint_arg_array[9] = _easyprint_typename(a10);

#define _easyprint_BASE_SHIFTER(...)                                             \
    _easyprint_BASE_TYPES(__VA_ARGS__, _easyprint_dummy, _easyprint_dummy,           \
                        _easyprint_dummy, _easyprint_dummy, _easyprint_dummy,        \
                        _easyprint_dummy, _easyprint_dummy, _easyprint_dummy,        \
                        _easyprint_dummy)

static void _easyprint_print_processor(_easyprint_ARGTYPE *argtypes, ...)
{
    va_list valist;
    va_start(valist, argtypes);
    _easyprint_ARGTYPE ttype;

    for (int i = 0; i < 10; ++i)
    {
        ttype = argtypes[i];
        switch (ttype)
        {
        case _easyprint_TYPE_UCHAR:
            _easyprint_ucharvar = (_ep_uchar)va_arg(valist, int);
            printf("0x%02hhx ", _easyprint_ucharvar);
            break;
        case _easyprint_TYPE_CHAR:
            _easyprint_charvar = (char)va_arg(valist, int);
            if (_easyprint_charvar >= 0 && _easyprint_charvar <= 127)
            {
                printf("%c ", _easyprint_charvar);
            }
            else
            {
                printf("0x%02hhx ", _easyprint_charvar);
            }
            break;
        case _easyprint_TYPE_LONG:
            _easyprint_longvar = va_arg(valist, long);
            printf("%ld ", _easyprint_longvar);
            break;
        case _easyprint_TYPE_ULONG:
            _easyprint_ulongvar = va_arg(valist, _ep_ulong);
            printf("%lu ", _easyprint_ulongvar);
            break;
        case _easyprint_TYPE_DOUBLE:
            _easyprint_doublevar = va_arg(valist, double);
            printf("%6.4f ", _easyprint_doublevar);
            break;
        case _easyprint_TYPE_STRING:
            _easyprint_stringvar = va_arg(valist, char *);
            printf("%s ", _easyprint_stringvar);
            break;
        case _easyprint_TYPE_POINTER:
            _easyprint_pointervar = va_arg(valist, void *);
            printf("%p ", _easyprint_pointervar);
            break;
        case _easyprint_TYPE_DEFAULT:
            _easyprint_objval = va_arg(valist, _ep_ulong);
            printf("Obj val 0x%2lx ", _easyprint_objval);
            break;
        case _easyprint_TYPE_TERMINATOR:
            break;
        }
    }
    printf("\n");
    va_end(valist);
}

#define print(...)                                                             \
    _easyprint_BASE_SHIFTER(__VA_ARGS__);                                        \
    _easyprint_print_processor(_easyprint_arg_array, ##__VA_ARGS__)

#endif