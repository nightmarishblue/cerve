// string manipulation
#ifndef _STR_H
#define _STR_H

// magic sprintf - return a heap-allocated string according to fmt
// on error, return NULL
char*
msprintf(const char* fmt, ...);
#endif