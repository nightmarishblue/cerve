#include "inet.h"
#include "main.h"

#include <asm-generic/errno-base.h>
#include <errno.h>
#include <stdarg.h>
#include <stdbool.h>
#include <stdio.h>
#include <string.h>

#include <unistd.h>

#include <arpa/inet.h>  // get human-readable ip names
#include <sys/socket.h> // socket API
#include <sys/time.h>

fd
crtsock()
{
    fd sock = socket(AF_INET,
                     SOCK_STREAM,
                     0); // protocol 0 selects the default for the domain
    if (sock == -1)
        eprintf("could not create socket for host ");
    return sock;
}

bool
bindport(fd sock, struct sockaddr_in* addr, uint16_t port)
{
    addr->sin_family = AF_INET;   // the domain we are using
    addr->sin_port = htons(port); // the port we will listen on
    addr->sin_addr.s_addr =
      INADDR_ANY; // the addresses we will allow to connect

    bool ret = bind(sock, (struct sockaddr*)addr, sizeof(*addr)) != -1;
    if (!ret)
        eprintf("could not bind socket to port %d", port);
    return ret;
}

bool
socklisten(fd sock)
{
    bool ret = listen(sock, QUEUE_LEN) != -1;
    if (!ret)
        eprintf("could not mark socket passive");
    return ret;
}

bool
closesock(fd sock)
{
    bool ret = close(sock) == 0;
    if (!ret)
        eprintf("could not close file descriptor");
    return ret;
}

bool
closepeer(fd sock)
{
    // TODO swallowing this error is bad, even if we can't act much on it
    if (shutdown(sock, SHUT_RDWR) == -1) {
        eprintf("could not shutdown socket");
    }
    return closesock(sock);
}

fd
acceptconn(fd sock, struct sockaddr* clientaddr, socklen_t* addrlen)
{
    fd conn = accept(sock, clientaddr, addrlen);
    if (conn == -1 && errno != EINTR) // an interruption may be expected
        eprintf("could not create socket for client");
    return conn;
}

bool
ipstr(const struct in_addr* iaddr, size_t len, char str[len])
{
    const char* ptr = inet_ntop(AF_INET, iaddr, str, len);
    if (ptr == NULL) // this throws an error if len isn't enough
        eprintf("could not get IP string");
    return ptr != NULL;
}

bool
addrstr(const struct sockaddr_in* iaddr, size_t len, char str[])
{
    if (!ipstr(&iaddr->sin_addr, len, str))
        return false;
    size_t used = strnlen(str, len), size = len - used;
    uint16_t portnum = ntohs(iaddr->sin_port);
    int written =
      snprintf(str + used, len - used, ":%u", (unsigned int)portnum);
    return !(written >= size || written < 0); // an error has happened
}