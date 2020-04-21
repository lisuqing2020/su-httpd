#ifndef _UTILS_H_
#define _UTILS_H_
#include <string.h>

// 解析主函数参数
int ProgramParameterAnalysis(int argc, char **argv, int *port, char *directory);

// 获取文件类型
void ContentType(char *path, char *content_type);

// 目录路径处理，如果后面没有/就加上，再去掉前面的.
void DirectoryPath(char *path);

// 16进制解码
void decode16(char *dst, char *src);

// 路径16进制编码，http的头是不能有中文的
void encode16(char *dst, int dst_size, const char *src);

// 处理路径参数
int UrlHandler(char *path, char **parameter);

// 判断是不是php文件
int IsPhp(char *path);

#endif