#ifndef _UTILS_H_
#define _UTILS_H_

// 解析主函数参数
void ProgramParameterAnalysis(int argc, char **argv, int *port, char *directory);

// 打印errno
void ErrorHandler(int ret, int error_code, char* description);

#endif