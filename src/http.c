#include "http.h"
#include "http/request.h"
#include "http/response.h"
#include "main.h"
#include "sock.h"

#include <stdarg.h>
#include <stdio.h>
#include <stdlib.h>

#include <string.h>

#include <errno.h>
#include <fcntl.h>
#include <sys/socket.h> // for a send or 2
#include <sys/stat.h>
#include <unistd.h>

const char* verstrs[] = { "1.0", "1.1" };
const size_t verc = size(verstrs);
// from a string like HTTP/X.X, deduce version
enum version
versionfromstr(const char verstr[MAX_VERSION_LEN])
{
    if (strncmp(verstr, "HTTP/", 5) != 0)
        return -1;
    verstr += 5; // shift up to the number
    for (int i = 0; i < verc; i++) {
        if (strncmp(verstr, verstrs[i], 4) ==
            0) // version string can be no larger than 4 chars
            return (enum version)i;
    }
    return -1;
}

const char*
strfromversion(enum version ver)
{
    if (ver < (enum version)0 || ver > V1_1)
        return NULL;
    return verstrs[ver];
}

// TODO rename
enum code
errorcode(int errorno)
{
    switch (errno) {
        // file errors
        case ENOENT:
            return NOT_FOUND;
        case EACCES:
            return FORBIDDEN;
        case ENAMETOOLONG:
            return URI_TOO_LONG;

        default:
            return INTERNAL_SERVER_ERROR;
    }
}

bool
serve(SBUFF* sock)
{
    struct request req;
    auto code = parserequest(sock, &req);
    if (code != OK) {
        if (errno == EINTR)
            return false;

        struct response res = { .version = DEFAULT_HTTP_VERSION,
                                .code = code,
                                .contentlen = -1,
                                .type = EMPTY };

        sendres(&res, sock->desc);
        return false; // do we really need to break
                      // the connection here?
    }

    printf("%s %s HTTP/%s\n",
           strfrommethod(req.method),
           req.identifier,
           strfromversion(req.version));

    struct response res;
    if (!respond(&req, &res)) {
        res.code = errorcode(errno);
        res.type = EMPTY;
        res.contentlen = -1;
        res.version = req.version;
    }
    sendres(&res, sock->desc);

    return true;
}