#ifndef _HTTP_RESPONSE_H
#define _HTTP_RESPONSE_H

#include "file.h"
#include "http.h"
#include "http/request.h"

// codes for the statuses, by name
enum code
{
    OK = 200,
    BAD_REQUEST = 400,
    FORBIDDEN = 403,
    NOT_FOUND = 404,
    LENGTH_REQUIRED = 411, // the client didn't supply Content-Length
    PAYLOAD_TOO_LARGE = 413,
    URI_TOO_LONG = 414,
    IM_A_TEAPOT = 418,           // things we should comply with but don't
    INTERNAL_SERVER_ERROR = 500, // ¯\_(ツ)_/¯
    NOT_IMPLEMENTED = 501, // things we don't have to comply with (and don't)
};

enum bodytype
{
    EMPTY,
    FILE_ // fuck c
};

struct response
{
    enum code code;
    enum version version;

    ssize_t
      contentlen; // value of the Content-Length header, or -1 if not included

    enum bodytype type;

    fd file; // handle of the file to serve if type == FILE
};

// destroy the response and free any resources
void
freeresponse(struct response* response);

// construct the response for a valid request
// return false if any error occurred (errno will be set)
bool
respond(const struct request* request, struct response* response);

// write the valid response into the given descriptor
void
sendres(const struct response* response, fd sock);

// struct representing the response type, like 200 OK
// FIXME delete
struct status
{
    const char* desc;
    enum code code;
};

const struct status*
statusfromcode(enum code code);

#endif