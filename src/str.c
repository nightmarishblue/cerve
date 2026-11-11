#include "str.h"

#include <stdarg.h>
#include <stdio.h>
#include <stdlib.h>

#define DEFAULT_STR_CAP 8

const char* empty = "";

// true if s is at capacity and can't be appended to
bool
strfull(STRING* s)
{
    // accounts for the null
    return s->len + 1 >= s->cap;
}

// public functions

STRING
emptystr()
{
    return (STRING){ .buf = (char*)empty };
}

bool
strpush(STRING* s, char c)
{
    if (strfull(s) && !strreserve(s, s->cap ? s->cap * 2 : DEFAULT_STR_CAP))
        return false;
    s->buf[s->len++] = c;
    s->buf[s->len] = 0;
    return true;
}

bool
strreserve(STRING* s, size_t cap)
{
    bool new = s->cap == 0;
    void* arr = (new) ? malloc(cap) : realloc(s->buf, cap);
    if (arr == nullptr)
        return false;

    s->buf = arr;
    s->cap = cap;
    if (new) // null terminate the new string
        s->buf[0] = 0;
    return true;
}

void
strfree(STRING* s)
{
    if (s->cap)
        free(s->buf);
}

char*
msprintf(const char* fmt, ...)
{
    va_list args;
    va_start(args, fmt);
// BUG this isn't being set in our makefile
#ifdef _GNU_SOURCE
    char* str;
    int len = vasprintf(&str, fmt, args);
    va_end(args);
    if (len == -1)
        return NULL;
    return str;
#else
    int size = vsnprintf(NULL, 0, fmt, args);
    va_end(args);
    if (size == -1)
        return NULL;

    char* str = malloc((size + 1) * sizeof(char));
    if (!str)
        return NULL;

    va_start(args, fmt);
    size = vsnprintf(str, size + 1, fmt, args);
    va_end(args);
    if (size == -1) {
        free(str);
        return NULL;
    }
    return str;
#endif
}