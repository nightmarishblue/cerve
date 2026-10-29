#include "http/method.h"
#include "main.h"
#include <string.h>

const char* mthdstrs[] = { "GET",     "HEAD",    "POST",  "PUT",  "DELETE",
                           "CONNECT", "OPTIONS", "TRACE", "PATCH" };
const size_t mthdc = size(mthdstrs);

enum method
methodfromstr(const char* mthdstr)
{
    for (int i = 0; i < mthdc; i++) {
        if (strncmp(mthdstr, mthdstrs[i], 8) == 0) // biggest is CONNECT
            return (enum method)i;
    }
    return -1;
}

const char*
strfrommethod(enum method mthd)
{
    if (mthd < (enum method)0 || mthd > PATCH)
        return NULL;
    return mthdstrs[mthd];
}
