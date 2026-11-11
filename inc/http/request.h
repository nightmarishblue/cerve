#ifndef _HTTP_REQUEST_H
#define _HTTP_REQUEST_H

#include "http.h"
#include "http/method.h"
#include "str.h"

#define INDEX "index.html"

#define MAX_REQ_PATH_LEN 256

struct request
{
    enum method method;   // what method this request is
    enum version version; // the request's version
    STRING identifier;    // id of the requested resource
};

// read the first line out of a socket and figure out if it's a valid request
// populate the request object with its version and file identifier
// return the code to respond with (will be 4xx if bad)
// HACK must be casted to enum code - but this should be returning a separate
// error type anyway
int
parserequest(SBUFF* sock, struct request* request);

void
requestfree(struct request* r);

#endif