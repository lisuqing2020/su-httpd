#include "utils.h"
#include <getopt.h>
#include <stdlib.h>
#include <string.h>
#include <stdio.h>
#include <errno.h>

#define NAME "su-httpd"
#define VERSION "1.0.0"
#define DATE "20200405"

void DirectoryPath(char *path) {
    int i = 0;
    while(++i) {
        if(path[i] == '\0') {
            if(path[i-1] == '/') {
                return;
            } else {
                strcat(path, "/");
                return;
            }
        }
    }
}

void ContentType(char* path, char *content_type) {
    int index = -1;
    for(int i = 0; path[i] != '\0'; ++i) {
        if(path[i] == '.') {
            index = i;
        }
    }
    if(strcmp(path+index, ".html") == 0) {
        strcpy(content_type, "text/html");
    } else if(strcmp(path+index, ".xml") == 0) {
        strcpy(content_type, "text/xml");
    } else if(strcmp(path+index, ".gif") == 0) {
        strcpy(content_type, "image/gif");
    } else if(strcmp(path+index, ".jpeg") == 0 || strcmp(path+index, ".jpg") == 0) {
        strcpy(content_type, "image/jpeg");
    } else if(strcmp(path+index, ".png") == 0) {
        strcpy(content_type, "image/png");
    } else {
        strcpy(content_type, "text/plain");
    }
}

int ProgramParameterAnalysis(int argc, char **argv, int *port, char *directory) {

    const char* const short_opts = "p:d:vh";
    const struct option long_opts[] = {
        {"port", 1, NULL, 'p'},
        {"directory", 1, NULL, 'd'},
        {"version", 0, NULL, 'v'},
        {"help", 0, NULL, 'h'}
    };

    int ret = 1;
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
                printf("%s %s %s\n", NAME, VERSION, DATE);
                ret = 0;
                break;
            case 'h':
                printf("You can start with su -pport -dpath.\n");
                ret = 0;
                break;
        }
        opt = getopt_long(argc, argv, short_opts, long_opts, NULL);
    }
    return ret;
}