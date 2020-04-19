#ifndef FCGI_H
#define FCGI_H

typedef struct
{
    unsigned char version;          //版本
    unsigned char type;             //操作类型
    unsigned char requestIdB1;      //请求id
    unsigned char requestIdB0;  
    unsigned char contentLengthB1;  //内容长度
    unsigned char contentLengthB0;
    unsigned char paddingLength;    //填充字节长度
    unsigned char reserved;         //保留字节
}FCGI_Header;   //消息头

//允许传送的最大数据 65536
#define FCGI_MAX_LENGTH     0xffff

//上述 FCGI_Header 长度
#define FCGI_HEADER_LEN     8

//FCGI的版本
#define FCGI_VERSION_1      1

// FCGI_Header 中 type 的具体值
#define FCGI_BEGIN_REQUEST      1   //开始请求
#define FCGI_ABORT_REQUEST      2   //异常终止请求
#define FCGI_END_REQUEST        3   //正常终止请求
#define FCGI_PARAMS             4   //传递参数
#define FCGI_STDIN              5   //POST 内容传递
#define FCGI_STDOUT             6   //正常响应内容
#define FCGI_STDERR             7   //错误输出
#define FCGI_DATA               8
#define FCGI_GET_VALUES         9
#define FCGI_GET_VALUES_RESULT  10
#define FCGI_UNKNOWN_TYPE       11  //通知 webserver 所请求 type 非正常类型
#define FCGI_MAXTYPE (FCGI_UNKNOWN_TYPE)    


//空的请求 ID
#define FCGI_NULL_REQUEST_ID    0

/*******************************************/

typedef struct
{
    unsigned char roleB1;           //webserver 所期望php-fpm 扮演的角色，具体取值下面有
    unsigned char roleB0; 
    unsigned char flags;            //确定 php-fpm 处理万一次请求之后是否关闭
    unsigned char reserved[5];      //保留字段
}FCGI_BeginRequestBody;     //开始请求体


typedef struct
{
    FCGI_Header header;         //消息头
    FCGI_BeginRequestBody body; //开始请求体
}FCGI_BeginRequestRecord; //完整消息--开始


//webserver 期望 php-fpm 扮演的角色(想让php-fpm做什么)
#define FCGI_KEEP_CONN      1   //如果为0则处理玩请求应用就关闭，否则不关闭
#define FCGI_RESPONDER      1   //接受http关联的所有信息，并产生http响应，接受来自webserver的PARAMS环境变量
#define FCGI_AUTHORIZER     2   //对于认证的会关联其http请求，未认证的则关闭请求
#define FCGI_FILTER         3   //过滤web server 中的额外数据流，并产生过滤后的http响应


/*******************************************/

typedef struct 
{
    unsigned char appStatusB3;      //结束状态，0为正常
    unsigned char appStatusB2;  
    unsigned char appStatusB1;
    unsigned char appStatusB0;
    unsigned char protocolStatus;   //协议状态
    unsigned char reserved[3];
}FCGI_EndRequestBody;   //结束消息体

typedef struct
{
    FCGI_Header header;         //结束头
    FCGI_EndRequestBody body;   //结束体
}FCGI_EndRequestRecord; //完整结束消息


//几种结束状态
#define FCGI_REQUEST_COMPLETE   0   //正常结束
#define FCGI_CANT_MPX_XONN      1   //拒绝新请求，单线程
#define FCGI_OVERLOADED         2   //拒绝新请求，应用负载了
#define FCGI_UNKNOWN_ROLE       3   //webserver 指定了一个应用不能识别的角色



#define FCGI_MAX_CONNS  "FCGI_MAX_CONNS"    //可接受的并发传输线路的最大值

#define FCGI_MAX_REQS   "FCGI_MAX_REQS"     //可接受并发请求的最大值

#define FCGI_MPXS_CONNS "FCGI_MPXS_CONNS"   //是否多路复用，其状态值也不同

/*******************************************/

typedef struct 
{
    unsigned char type;
    unsigned char reserved[7];
}FCGI_UnknownTypeBody;

typedef struct
{
    FCGI_Header header;
    FCGI_UnknownTypeBody body;
}FCGI_UnKnownTypeRecord;

typedef struct 
{
    int sockfd_;        //与php-fpm 建立的 sockfd
    int requestId_;     //record 里的请求ID
    int flag_;          //用来标志当前读取内容是否为html内容


}FastCgi_t;

void FastCgi_init(FastCgi_t *c);

void FastCgi_finit(FastCgi_t *c);

//设置请求Id
void setRequestId(FastCgi_t *c, int requestId);

//生成头部
FCGI_Header makeHeader(int type, int request, 
                    int contentLength, int paddingLength);

//生成发起请求的请求体
FCGI_BeginRequestBody makeBeginRequestBody(int role, int keepConnection);

//生成 PARAMS 的 name-value body
int makeNameValueBody(char *name, int nameLen,
                        char *value, int valueLen,
                        unsigned char *bodyBuffPtr, int *bodyLen);

//获取express_help.conf 配置文件中的 ip 地址
char *getIpFromConf(void);

//连接php-fpm，如果成功则返回对应的套接字描述符
void startConnect(FastCgi_t *c);

//发送开始请求记录
int sendStartRequestRecord(FastCgi_t *c);

//向php-fpm发送name-value参数对
int sendParams(FastCgi_t *c, char *name, char *value);

//发送结束请求消息
int sendEndRequestRecord(FastCgi_t *c);

//只读php-fpm 返回内容，读到的内容处理后期再添加
int readFromPhp(FastCgi_t *c, int connect_fd);

#endif
