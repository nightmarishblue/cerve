#ifndef _HTTP_H
#define _HTTP_H

#include "sock.h"

#include <stdbool.h>
#include <sys/types.h>

#define CRLF "\r\n"

#define MAX_VERSION_LEN 9 // array size needed for "HTTP/X.X"
enum version
{
    // _0_9, // no one will get mad if we don't support 0.9
    V1_0,
    V1_1,
};

#define DEFAULT_HTTP_VERSION                                                   \
    V1_1 // the default version to assume when we can't determine the client's

// with a string of the form "HTTP/x.x" (9 chars), return its version
// or -1 on invalid
enum version
versionfromstr(const char verstr[MAX_VERSION_LEN]);

const char*
strfromversion(enum version ver);

// read from a given socket and answer 1 HTTP request
// return false if a HTTP error was encountered
bool
serve(SBUFF* sock);
#endif