#ifndef _UTILS_H_
#define _UTILS_H_
#include <string.h>

// 解析主函数参数
int ProgramParameterAnalysis(int argc, char **argv, int *port, char *directory);

// 获取文件类型
void ContentType(char *path, char *content_type);

// 目录路径处理，如果后面没有/就加上，再去掉前面的.
void DirectoryPath(char *path);

#endif