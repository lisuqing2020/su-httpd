#include "utils.h"
#include <getopt.h>
#include <stdlib.h>
#include <string.h>
#include <stdio.h>
#include <errno.h>
#include <ctype.h>

#define NAME "su-httpd"
#define VERSION "1.1.0"
#define DATE "20200418"

int IsPhp(char *path) {
    int index = -1;
    for(int i = 0; path[i] != '\0'; ++i) {
        if(path[i] == '.') {
            index = i;
        }
    }
    if(strcmp(path+index, ".php") == 0) {
        return 1;
    }
    return 0;
}

void UrlHandler(char *path) {
    // 先删url参数
    char *token = strtok(path, "?");
    strcpy(path, token);
}

int hexit(char c) {
	if (c >= '0' && c <= '9')
		return c - '0';
	if (c >= 'a' && c <= 'f')
		return c - 'a' + 10;
	if (c >= 'A' && c <= 'F')
		return c - 'A' + 10;
	return 0;
}

void decode16(char *dst, char *src) {
	for (; *src != '\0'; ++dst, ++src) {
		if (src[0] == '%' && isxdigit(src[1]) && isxdigit(src[2])) {
			*dst = hexit(src[1]) * 16 + hexit(src[2]);
			src += 2;
		} else {
			*dst = *src;
		}
	}
	*dst = '\0';
}

void encode16(char *dst, int dst_size, const char *src) {
	int i;
	for (i = 0; *src != '\0' && i + 4 < dst_size; ++src) {
		if (isalnum(*src) || strchr("/_.-~", *src) != NULL) {
			*dst = *src;
			++dst;
			++dst_size;
		} else {
			sprintf(dst, "%%%02x", (int)*src & 0xff);
			dst += 3;
			dst_size += 3;
		}
	}
	*dst = '\0';
}

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
    if(strcmp(path+index, ".html") == 0 || strcmp(path+index, ".php") == 0) {
        strcpy(content_type, "text/html; charset=utf-8");
    } else if(strcmp(path+index, ".xml") == 0) {
        strcpy(content_type, "text/xml");
    } else if(strcmp(path+index, ".gif") == 0) {
        strcpy(content_type, "image/gif");
    } else if(strcmp(path+index, ".jpeg") == 0 || strcmp(path+index, ".jpg") == 0) {
        strcpy(content_type, "image/jpeg");
    } else if(strcmp(path+index, ".png") == 0) {
        strcpy(content_type, "image/png");
    } else if(strcmp(path+index, ".css") == 0) {
        strcpy(content_type, "text/css");
    } else {
        strcpy(content_type, "text/plain; charset=utf-8");
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