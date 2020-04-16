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

## 已知BUG
- 写了图片格式，但是无法显示图片

## TODO
- 添加到service
- 添加日志文件
- 处理get传参
- 添加配置文件，解决自定义根目录，404页面问题