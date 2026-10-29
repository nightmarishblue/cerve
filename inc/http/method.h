#ifndef _HTTP_METHOD_H
#define _HTTP_METHOD_H

#define MAX_METHOD_LEN                                                         \
    8 // array size needed to represent the name of all methods

// request methods
enum method
{
    GET,
    HEAD,
    POST,
    PUT,
    DELETE,
    CONNECT,
    OPTIONS,
    TRACE,
    PATCH,
};

enum method
methodfromstr(const char* mthdstr);

const char*
strfrommethod(enum method mthd);

#endif