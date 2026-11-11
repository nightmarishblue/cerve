// string manipulation
#ifndef _STR_H
#define _STR_H

#include <stddef.h>
#include <stdlib.h>
#include <string.h>

typedef struct
{
    char* buf;  // holds a null-terminated string
    size_t len; // length of the string, excluding the null
    size_t cap; // capacity of the buffer, 0 if it isn't allocated
} STRING;

// return an empty (but valid) non-owning string
STRING
emptystr();

// Expand s's buffer to hold the given capacity.
// On failure, return false; s is unmodified.
bool
strreserve(STRING* s, size_t cap);

// Append the character c to s, reallocating its buffer if needed.
// On failure, return false; s is unmodified.
bool
strpush(STRING* s, char c);

// Free s's buffer if allocated.
void
strfree(STRING* s);

// magic sprintf - return a heap-allocated string according to fmt
// on error, return NULL
char*
msprintf(const char* fmt, ...);
#endif