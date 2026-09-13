#include "opts.h"
#include "file.h"
#include "main.h"

#include <ctype.h>
#include <stdio.h>
#include <stdlib.h>

#include <fcntl.h>
#include <getopt.h>
#include <unistd.h>

const char* usagemsg = "Usage: cerve [-h] [-p <port number>] <root directory>";
struct cerveconf config = { .help = false, .portn = 80, .srvdirpath = NULL };
const struct cerveconf* options = &config;

// list of all options recognised by cerve
// verbose, version, quiet, max connections, etc.
const struct cerveopt optlist[] = {
    { 'h', "Display this help message and exit", NULL, false },
    { 'p', "Specify the port to serve on", "80", true },
    // {'d', "Specify directory to serve", NULL},
};
const size_t optc = size(optlist);

int
parseopts(struct cerveconf* opts, int argc, char* argv[])
{
    size_t optstrsz = 0;
    for (size_t i = 0; i < optc; i++) {
        optstrsz++;
        if (optlist[i].takesarg)
            optstrsz++;
    }
    char valid[optstrsz + 1];
    for (size_t i = 0; i < optc; i++) {
        valid[i] = optlist[i].symb;
        if (optlist[i].takesarg)
            valid[++i] = ':';
    }
    valid[optstrsz] = '\0';

    int c;
    while ((c = getopt(argc, argv, valid)) != -1) {
        switch (c) {
            case 'h':
                opts->help = true;
                break;
            case 'p': {
                for (size_t i = 0; optarg[i]; i++) {
                    if (!isdigit((unsigned char)optarg[i])) {
                        fputs("Error: given port '",
                              stderr); // 19 chars
                        i += 19;       // we're breaking this loop
                                       // anyway
                        fprintf(stderr, "%s' invalid\n", optarg);
                        for (size_t j = 0; j < i; j++)
                            putc(' ', stderr);
                        fputs("^\n", stderr);
                        return 1;
                    }
                }
                opts->portn = atoi(optarg);
                break;
            }
        }
    }
    return optind;
}

void
printopts(FILE* file)
{
    for (size_t i = 0; i < optc; i++) {
        for (int j = 0; j < 2; j++)
            putc(' ', file);
        putc('-', file);
        putc(optlist[i].symb, file);
        for (int j = 0; j < 4; j++)
            putc(' ', file);
        fputs(optlist[i].help, file);
        putc('\n', file);
        if (optlist[i].deflt) {
            for (int j = 0; j < 8; j++)
                putc(' ', file);
            fprintf(file, "Default: %s\n", optlist[i].deflt);
        }
    }
}

void
configure(int argc, char* argv[])
{
    int argindex = parseopts(&config, argc, argv);

    if (config.help) {
        printf("%s\n", usagemsg);
        printopts(stdout);
        exit(0);
    }

    switch (argc - argindex) {
        case 0:
            fprintf(stderr, "Error: no root directory given\n%s\n", usagemsg);
            exit(2);
        case 1:
            break;
        default:
            fprintf(stderr, "Warning: ignoring extraneous arguments\n");
    }

    config.srvdirpath = argv[argindex];
    config.srvdir = opend(config.srvdirpath);
    if (config.srvdir == -1) {
        eprintf("could not open dir '%s'", config.srvdirpath);
        exit(1);
    }
}