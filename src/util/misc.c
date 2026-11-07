#include <errno.h>
#include <stdarg.h>
#include <stdio.h>
#include <string.h>
void
eprintf(const char* format, ...)
{
    // maybe quit if silent
    va_list args;
    va_start(args, format);
    fputs("Error: ", stderr); // would end up writing this 10 times otherwise
    vfprintf(stderr, format, args);
    va_end(args);
    fprintf(stderr, ": %s\n", strerror(errno));
}