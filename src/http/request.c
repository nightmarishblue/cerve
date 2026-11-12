#include "http/request.h"
#include "http/method.h"
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

// Parse a method from the next bytes out of sock, consuming them (including the
// ' ').
//
// On failure, return false; method is unchanged, and the state of sock is not
// defined
bool
parsemethod(SBUFF* sock, enum method* method)
{
    char mthdstr[MAX_METHOD_STR_SIZE];
    // would like to check if there was a transport error
    readuntilchar(sock, MAX_METHOD_STR_SIZE, mthdstr, ' ');
    auto m = methodfromstr(mthdstr);
    if (m == -1)
        return false;
    *method = m;
    return true;
}

// Parse the request's identifier from the next bytes of sock, consuming them
// (including the ' ') and storing the result in identifier.
//
// On success, return OK. Else, return an error code; identifier is unmodified.
enum code
parseid(SBUFF* sock, STRING* identifier)
{
    // there are 4 kinds of identifier
    // /xyz - origin form - supported
    // http://foo.com/xyz - absolute form
    // foo.com:443 - authority form
    // * - asterisk form
    // TODO recognise the other forms (at least avoid returning 400)

    int c = sbuffpeek(sock);
    if (c == -1)
        return INTERNAL_SERVER_ERROR;
    if (c != '/')
        return BAD_REQUEST;

    // extract the path
    // read up to ' ', up to limit
    enum code error = URI_TOO_LONG;
    STRING id = { 0 };
    for (int i = 0; i < MAX_REQ_PATH_LEN; i++) {
        c = sbuffgetc(sock);
        if (c == -1)
            BAIL(INTERNAL_SERVER_ERROR);

        if (c == ' ') {
            *identifier = id;
            return OK;
        }

        strpush(&id, c);
    }

reterr:
    strfree(&id);
    return error;
}

// Parse the HTTP request version from the next bytes of sock, consuming them
// (including the '\r\n') and storing the result in version.
//
// On success, return OK. Else, return error code; version is unmodified.
enum code
parseversion(SBUFF* sock, enum version* version)
{
    char verstr[MAX_VERSION_STR_SIZE];
    readuntilchar(sock, MAX_VERSION_STR_SIZE, verstr, '\r');
    enum version v = versionfromstr(verstr);
    if (v == -1)
        return BAD_REQUEST;

    // if the next character isn't a \n, KILL
    auto c = sbuffgetc(sock);
    if (c == -1)
        return INTERNAL_SERVER_ERROR;
    if (c != '\n')
        return BAD_REQUEST;

    *version = v;
    return OK;
}

// Read and discard the remaining HTTP headers from sock, up to '\r\n\r\n'. Stop
// after MAX_REQUEST_LEN characters.
//
// Return OK if no errors encountered.
enum code
consumeheaders(SBUFF* sock)
// TODO actually parse - at least the important ones like Content-Length
{
    const char* end = "\r\n\r\n";
    size_t j = 0;
    for (int i = 0; i < MAX_REQUEST_LEN; i++) {
        int c = sbuffgetc(sock);
        // BUG doesn't take EINTR into account
        if (c == -1) {
            eprintf("Error reading from socket");
            return INTERNAL_SERVER_ERROR;
        }
        if (c == end[j]) {
            if (end[++j] == '\0')
                return OK;
        } else
            j = 0;
    }
    return REQUEST_HEADERS_TOO_LARGE;
}

// public

// TODO document the pointer invariants
// FIXME there are actually 3 possible outcomes - successful, error and
// interrupted
// BUG requests with \n instead of \r\n hang indefinitely - they
// should either just work or return 400
int
parserequest(SBUFF* sock, struct request* request)
{
    int error, status;

    // read first few bytes - ensure they are GET /
    enum method method;
    if (!parsemethod(sock, &method))
        return INTERNAL_SERVER_ERROR;
    if (method > HEAD) {
        fprintf(
          stderr, "oops... we don't have '%s' @u@\n", strfrommethod(method));
        return IM_A_TEAPOT;
    }

    STRING id;
    if ((status = parseid(sock, &id)) != OK)
        return status;

    enum version version;
    if ((status = parseversion(sock, &version)) != OK)
        BAIL(status);

    // consume the remaining headers
    if ((status = consumeheaders(sock)) != OK)
        BAIL(status);

    // TODO parse the request's body

    request->method = method;
    request->identifier = id;
    request->version = version;

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
