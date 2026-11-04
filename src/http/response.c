#include "http/response.h"
#include "main.h"
#include "opts.h"
#include <fcntl.h>
#include <unistd.h>

bool
sendstatus(fd sock, enum version version, enum code code)
{
    const struct status* status = statusfromcode(code);
    const char* verstr = strfromversion(version);
    bool success = status && verstr;
    if (!verstr)
        verstr = strfromversion(DEFAULT_HTTP_VERSION);
    if (!status)
        status = statusfromcode(INTERNAL_SERVER_ERROR);
    if ((success = status && verstr))
        success = sockprintf(sock,
                             "HTTP/%s %d %s\r\n",
                             verstr,
                             (int)status->code,
                             status->desc) > 0;
    return success; // this is a lot of error checking I know
}

const struct status statuses[] = {
    { "OK", OK },
    { "Bad Request", BAD_REQUEST },
    { "Forbidden", FORBIDDEN },
    { "Not Found", NOT_FOUND },
    { "Length Required", LENGTH_REQUIRED },
    // things for our implementation limits
    { "Payload Too Large", PAYLOAD_TOO_LARGE },
    { "URI Too Long", URI_TOO_LONG },
    { "I'm a teapot", IM_A_TEAPOT }, // things we don't support
    { "Internal Server Error", INTERNAL_SERVER_ERROR },
    { "Not Implemented", NOT_IMPLEMENTED },
};
const size_t statusc = size(statuses);

const struct status*
statusfromcode(enum code code)
{
    for (int i = 0; i < statusc; i++) {
        const struct status* curr = &statuses[i];
        if (curr->code == code)
            return curr;
    }
    return NULL;
}

bool
respond(const struct request* request, struct response* response)
{
    switch (request->method) {
        case GET:
        case HEAD: {
            /*
                open the file
                check if that workeded:
                - return a 400/500
                - return that file
            */
            const char* filepath = request->identifier;
            // TODO probably throw errors if the file isn't "regular"
            fd file = openunder(options->srvdir, filepath, O_RDONLY);
            if (file == -1)
                return false;

            struct stat finfo;
            if (fstat(file, &finfo) == -1) {
                close(file);
                return false;
            }

            response->code = OK; // what if this fails later?
            response->version = request->version;
            response->contentlen = finfo.st_size;

            if (request->method == GET) {
                response->type = FILE_;
                response->file = file;
            } else {
                response->type = EMPTY;
                close(file);
            }

            return true;
        };
        default: {
            fprintf(stderr,
                    "Illegal state: unsupported method '%s'\n",
                    strfrommethod(request->method));
            return false;
        }
    }
}

void
sendres(const struct response* response, fd sock)
{
    // 1. status
    sendstatus(sock, response->version, response->code);
    // 2. headers
    if (response->contentlen != -1)
        sockprintf(sock, "Content-Length: %ld" CRLF, response->contentlen);
    sockprintf(sock, "Connection: close" CRLF);
    // 3. blank line
    write(sock, CRLF, len(CRLF));
    // 4. body (if there)
    switch (response->type) {
        case EMPTY:
            break;
        case FILE_: {
            transmitfile(sock, response->file, response->contentlen);
            break;
        }
        default: {
            eprintf("illegal state - unknown response body type");
            break;
        }
    }
}

void
freeresponse(struct response* res)
{
    // free the response/close its file
    if (res->type == FILE_)
        close(res->file);
}
