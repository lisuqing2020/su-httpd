# super httpd

## 使用方法
1. 下载/builds/httpd，添加到环境变量。或下载源码，使用make && make install.
2. 程序参数：
    - -p/--port：指定端口，默认80
    - -d/--directory：指定资源根目录，默认当前目录
    - -h/--help：帮助文档
    - -v/--version：获取版本信息

## 依赖
- php-fpm
    - 确保安装php-fpm
    - 修改配置文件：/usr/local/php/etc/php-fpm.conf
        - listen = /tmp/php-cgi.sock 替换为 listen = listen = 127.0.0.1:9000
    - 重启php-fpm
        - sudo service php-fpm restart

## 版本更新
- v1.0.0：支持GET请求，目录/文件
    - v1.0.1：修复了中文路径问题
    - v1.0.2：make install添加到/usr/bin
    - v1.0.3：解决了访问目录资源连接不断开的bug，在requesthandler中服务器主动close connectfd
    - v1.0.4：解决了EOT传输结束符的问题，响应正文的buf不能卡的刚好
    - v1.0.5：解决了外部css无法连接问题，需要支持css的content-type
    - v1.0.6：解决了读取图片不成功的问题，fopen换成了open
    - v1.1.0：修复了一个大BUG，响应时写缓冲区满的问题
        - 在循环读文件，响应文件内容时，如果文件过大会发生EAGAIN错误，即写缓冲区满，因为connect_fd是非阻塞的，所以会进入下一次循环，这样就无法响应大文件
        - 解决办法是在回复客户端时，将connect_fd临时设置为阻塞
        - 顺便把响应目录的部分也将connect_fd设置为阻塞了
        - 临时设置后不需要再设置回非阻塞，因为函数走完这个connect_fd就被关闭了
    - v1.1.1：url后可以接参数了，不过还没什么用
    - v1.2.0：支持php，但是php文件无法接收表单数据
    - v1.2.1：事实上功能没变，就是加了几行代码用来获取url的参数部分
        - 在这里sf了无数遍，心态炸了，有眼瞎，其次是while strtok的时候，token有可能为NULL，然后我没判断就strcpy了
        - 这里要注意的是我传参是字符串指针数组，需要循环malloc，感觉这里应该有更好的处理方式
- v2.0.0：支持php
    - 令人激动的进步，支持php获取get参数了，关于fastcgi发送参数一直找不到解决办法，全仰仗这位大哥的[博客](https://blog.csdn.net/liushengxi_root/article/details/84976353)
    - 支持post正在向我招手👋
    - v2.0.1 对php大文件的bug做了改动，但现在是一次性读取到buf，但如果更大的文件就还是有错，应该是循环读循环写，但是用的是别人的代码就很难改...
    - v2.0.2 php大文件的问题解决了，用了malloc和realloc动态分配内存，✌
- v3.0.0：支持post
    - 版本迭代的就很快= =已经支持post了，php也可拿到post的数据然后返回
    - 但是这个post数据时灵时不灵，有时候后台能打印出post数据，有时候打不出来...玄学
    - 有可能是拿body的方式不太对，"\r\n\r\n"有时候找不到
    - v3.0.1：post数据的问题，用练手的一些php测试，post数据提交的成功率还挺高，但是拿自己之前写过的一个大项目，提交表单，数据末尾会有乱码。结论就是数据量大的时候会乱码，但是因为get只需要第一行，所以还能用，但是post就乱套了。bug很严重啊。
        - 同样也找到了之前没注意到的一个错误，因为之前数据量没那么大没发现
        - 然后发现了还不支持php获取get参数还没有提供支持

## 已知BUG
- 程序启动传参那里还有点问题
- 数据传输的时候有乱码问题

## TODO
- 添加到service
- 添加日志文件
- 添加配置文件，解决自定义根目录，404页面问题
- 程序结构看看能不能再优化一下，看着有点乱
- 学习fastcgi，不过找不到学习资源...
- 提供php获取get参数的支持

## magic code
1. net.c:51 用fopen即便是2进制读，图片也依旧无法显示，换成open就没问题了，但是open效率低