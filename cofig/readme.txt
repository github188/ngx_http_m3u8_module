1. nginx代码编译
	版本：nginx-1.13.12
	额外需要icon和openssl
	config参数如下
./configure --prefix=./_install/ --add-module=./extends/ngx_http_m3u8_module --with-http_ssl_module --with-openssl=/usr/local --with-http_v2_module --with-http_realip_module --with-http_gzip_static_module --with-http_stub_status_module

2. nginx服务配置
	1）复制本目录下的证书cert.key和cert.pem到nginx安装目录的conf目录下
	2）复制本目录下的nginx.conf文件替换nginx安装目录的conf目录下nginx.conf
	3）复制本目录下的gss_globle.conf到nginx安装目录
	4）nginx安装目录的html目录下创建hls目录
	5）复制本目录下的loading_01.ts到nginx安装目录的html目录下的hls目录
	6）复制本目录下的loading.m3u8和crossdomain.xml到nginx安装目录的html目录

3. 数据库配置
	1）安装mysql服务端
	2）执行本目录下的database.sh脚本创建数据库（注：请自行根据自己的数据库用户名和密码修改脚本）

4. 运行nginx服务
	nginx运行时一定要添加参数 -p 指定nginx安装目录（绝对路径）
	例如nginx安装目录是/data/nginx_hls，则运行指令如下：
	/data/nginx_hls/sbin/nginx -p /data/nginx_hls/

5. 使用vlc看图
	在VLC的网络播放URL中输入地址（假设nginx运行服务器的IP为192.168.1.100,想观看的设备uid为A99762101001001），则地址如下：
	https://192.168.1.100:4433/hls/A99762101001001.m3u8

6. 修改nginx服务配置
	请自行百度修改nginx.conf

7. 修改我们服务的参数
	请参照注释修改gss_globle.conf内容