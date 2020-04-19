# super httpd

## 使用方法
1. 下载/builds/httpd，添加到环境变量。或下载源码，使用make && make install.
2. 程序参数：
    - -p/--port：指定端口，默认80
    - -d/--directory：指定资源根目录，默认当前目录
    - -h/--help：帮助文档
    - -v/--version：获取版本信息

## 版本更新
- v1.0.0：支持GET请求，目录/文件
    - v1.0.1：修复了中文路径问题
    - v1.0.2：make install添加到/usr/bin
    - v1.0.3：解决了访问目录资源连接不断开的buf，在requesthandler中服务器主动close connectfd
    - v1.0.4：解决了EOT传输结束符的问题，响应正文的buf不能卡的刚好
    - v1.0.5：解决了外部css无法连接问题，需要支持css的content-type
    - v1.0.6：解决了读取图片不成功的问题，fopen换成了open
    - v1.1.0：修复了一个大BUG，响应时写缓冲区满的问题
        - 在循环读文件，响应文件内容时，如果文件过大会发生EAGAIN错误，即写缓冲区满，因为connect_fd是非阻塞的，所以会进入下一次循环，这样就无法响应大文件
        - 解决办法是在回复客户端时，将connect_fd临时设置为阻塞
        - 顺便把响应目录的部分也将connect_fd设置为阻塞了
        - 临时设置后不需要再设置回非阻塞，因为函数走完这个connect_fd就被关闭了

## 已知BUG
- 程序启动传参那里还有点问题

## TODO
- 添加到service
- 添加日志文件
- 处理get传参
- 添加配置文件，解决自定义根目录，404页面问题
- 程序结构看看能不能再优化一下，看着有点乱
- 学习fastcgi，不过找不到学习资源...

## magic code
1. net.c:51 用fopen即便是2进制读，图片也依旧无法显示，换成open就没问题了，但是open效率低