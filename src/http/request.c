#include "http/request.h"
#include "http/response.h"
#include "sock.h"
#include "str.h"
#include "util/misc.h"
#include <stdio.h>
#include <string.h>

#define MAX_REQUEST_LEN 4096 // TODO tune this once header parsing is done

#define BAIL(e)                                                                \
    do {                                                                       \
        error = e;                                                             \
        goto reterr;                                                           \
    } while (false)

// TODO document the pointer invariants
// FIXME there are actually 3 possible outcomes - successful, error and
// interrupted
// BUG requests with \n instead of \r\n hang indefinitely - they
// should either just work or return 400
int
parserequest(SBUFF* sock, struct request* request)
{
    int error;

    // read first few bytes - ensure they are GET /
    char mthdstr[MAX_METHOD_LEN];
    // reject invalid input - smallest method name is GET, 3 chars
    if (readuntilchar(sock, MAX_METHOD_LEN, mthdstr, ' ') < 3 ||
        (request->method = methodfromstr(mthdstr)) == -1)
        return BAD_REQUEST;
    else if (request->method > HEAD) {
        fprintf(stderr,
                "oops... we don't have '%s' @u@\n",
                strfrommethod(request->method));
        return IM_A_TEAPOT;
    }

    if (sbuffpeek(sock) != '/')
        return BAD_REQUEST;

    // extract the path
    // read up to ' ', up to limit
    STRING
    id = { .buf = "" };
    for (int i = 0; i < MAX_REQ_PATH_LEN; i++) {
        int c = sbuffgetc(sock);
        if (c == -1)
            BAIL(INTERNAL_SERVER_ERROR);

        if (c == ' ')
            break;

        strpush(&id, c);
    }
    request->identifier = id;

    // extract the version string
    char verstr[MAX_VERSION_LEN];
    readuntilchar(sock, MAX_VERSION_LEN, verstr, '\r');
    enum version version = versionfromstr(verstr);
    if (version == -1)
        BAIL(BAD_REQUEST);
    request->version = version;
    // if the next character isn't a \n, KILL
    if (sbuffgetc(sock) != '\n')
        BAIL(BAD_REQUEST);

    // consume the remaining headers
    // TODO actually parse - at least the important ones like Content-Length
    const char* end = "\r\n\r\n";
    size_t j = 0;
    for (int i = 0; i < MAX_REQUEST_LEN; i++) {
        int c = sbuffgetc(sock);
        // BUG doesn't take EINTR into account
        if (c == -1) {
            eprintf("Error reading from socket");
            BAIL(INTERNAL_SERVER_ERROR);
        }
        if (c == end[j]) {
            if (end[++j] == '\0')
                break; // FIXME does not consume any request body
        } else
            j = 0;
    }

    return OK;

reterr:
    strfree(&id);
    return error;
}

void
requestfree(struct request* r)
{
    strfree(&r->identifier);
}
