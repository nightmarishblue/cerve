#include "sock.h"

#include <asm-generic/errno.h>
#include <errno.h>
#include <stdarg.h>
#include <stddef.h>
#include <stdio.h>
#include <stdlib.h>

#include <sys/types.h>
#include <unistd.h>

#include <sys/socket.h>

#include <sys/sendfile.h>

// get this's head (the next byte of data in its buffer)
char*
sockhead(SBUFF* this)
{
    return &this->buff[this->used];
}

// return true if there are no bytes left in s
bool
sockempty(const SBUFF* s)
{
    return s->used >= s->len;
}

// try to fill s if it's empty
// return ENODATA if the socket is at EOF
bool
ensuredata(SBUFF* s)
{
    if (sockempty(s)) {
        switch (sbuffrecv(s, 0)) {
            case 0:
                errno = ENODATA;
            case -1:
                return false;
            default:
        }
    }
    return true;
}

// public

int
sbuffgetc(SBUFF* this)
{
    if (!ensuredata(this))
        return -1;
    return this->buff[this->used++];
}

SBUFF
mksbuff(fd desc)
{
    SBUFF out;
    out.desc = desc;
    out.len = 0;
    out.used = 0;
    return out;
}

ssize_t
sbuffrecv(SBUFF* this, int flags)
{
    // this syscall is b-b-blocking
    // FIXME implement a timeout
    ssize_t received = recv(this->desc, this->buff, SBUFF_SIZE, flags);
    if (received != -1) {
        this->used = 0;
        this->len = received;
    }
    return received;
}

int
sbuffpeek(SBUFF* this)
{
    if (!ensuredata(this))
        return -1;
    return this->buff[this->used];
}

int
sgetc(fd sock, int flags)
{
    unsigned char c[1];
    int i = recv(sock, c, 1, flags);
    if (i != 1)
        return i;
    return *c;
}

size_t
readuntilchar(SBUFF* sock, const size_t len, char buf[len], char illegal)
{
    if (len == 0)
        return 0; // safer to simply exit if len is 0
    // consume len - 1 chars
    for (size_t i = 0; i < len - 1; i++) {
        int c = sbuffgetc(sock);
        if (c == illegal) {
            buf[i] = '\0';
            return i;
        }
        if (c <= 0) // error or socket close
        {
            buf[i] = '\0';
            return len;
        }
        buf[i] = c;
    }
    buf[len - 1] = '\0'; // terminate the string
    // peek at the next char and consume it if it's the breakchar
    if (sbuffpeek(sock) == illegal) {
        sock->used++;
        return len - 1; // also return the number of bytes consumed
    }
    return len; // return len on bad
}

int
sockprintf(fd sock, const char* fmt, ...)
{
    va_list args;
    va_start(args, fmt);
    int i = vdprintf(sock, fmt, args); // possibly ifdef this
    va_end(args);
    return i;
}

ssize_t
transmitfile(fd sock, fd file, size_t n)
{
    size_t written = 0;
    while (written < n) {
        ssize_t ret = sendfile(sock, file, NULL, n - written);
        if (ret == 0)
            break;
        if (ret == -1)
            return -1;
        written += ret;
    }
    return written;
}