#include "http.h"
#include "inet.h"
#include "opts.h"
#include "sock.h"

#include <asm-generic/socket.h>
#include <bits/types/siginfo_t.h>
#include <fcntl.h>
#include <netinet/in.h>
#include <signal.h>
#include <stdarg.h>
#include <stdio.h>
#include <string.h>

#include <errno.h>
#include <sys/socket.h>

volatile sig_atomic_t loop = true;

void
cleanup(int _signal)
{
    puts("Terminating loop...");
    loop = false;
}

void
eprintf(const char* format, ...)
{
    // maybe quit if silent
    va_list args;
    va_start(args, format);
    fputs("Error: ", stderr); // would end up writing this 10 times otherwise
    vfprintf(stderr, format, args);
    va_end(args);
    fprintf(stderr, ": %s\n", strerror(errno));
}

int
main(int argc, char* argv[])
{
    configure(argc,
              argv); // will exit if unrecoverable invalid args are given

    struct sockaddr_in myaddr;
    fd sockid = crtsock();
    if (setsockopt(
          sockid, SOL_SOCKET, SO_REUSEADDR, &(int){ 1 }, sizeof(int)) == -1) {
        eprintf("could not set socket options");
    };

    if (sockid == -1)
        return 1;
    if (!(bindport(sockid, &myaddr, options->portn) && socklisten(sockid))) {
        closesock(sockid);
        return 1;
    }

    struct sigaction a = { .sa_handler = &cleanup, .sa_flags = SA_RESETHAND };
    sigaction(SIGINT, &a, NULL);

    while (loop) {
        printf("Waiting for connection...\n");
        struct sockaddr_in clientaddr;
        socklen_t addrsz = sizeof(clientaddr);
        fd clisock = acceptconn(sockid, (struct sockaddr*)&clientaddr, &addrsz);
        if (clisock == -1)
            continue;

        char name[16];
        if (addrstr(&clientaddr, 16, name))
            printf("Received connection from %s\n", name);

        SBUFF socket = mksbuff(clisock);
        puts("Serving request...\n");
        serve(&socket);
        puts("Closing connection...");
        closepeer(clisock);
    }

    puts("Closing socket...");
    closesock(sockid);
    return 0;
}