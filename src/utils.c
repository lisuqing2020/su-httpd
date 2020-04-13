#include "utils.h"
#include <getopt.h>
#include <stdlib.h>
#include <string.h>
#include <stdio.h>
#include <errno.h>

void ErrorHandler(int ret, int error_code, char* description) {
    if(ret == error_code) {
        perror(description);
        exit(0);
    }
}

void ProgramParameterAnalysis(int argc, char **argv, int *port, char *directory) {

    const char* const short_opts = "p:d:vh";
    const struct option long_opts[] = {
        {"port", 1, NULL, 'p'},
        {"directory", 1, NULL, 'd'},
        {"version", 0, NULL, 'v'},
        {"help", 0, NULL, 'h'}
    };

    int opt = getopt_long(argc, argv, short_opts, long_opts, NULL);
    while(opt != -1) {
        switch(opt) {
            case 'p':
                *port = atoi(optarg);
                break;
            case 'd':
                strcpy(directory, optarg);
                break;
            case 'v':
                printf("su-httpd 1.0.0 20200403.\n");
                break;
            case 'h':
                printf("You can start with su -pport -dpath.\n");
                break;
        }
        opt = getopt_long(argc, argv, short_opts, long_opts, NULL);
    }
}