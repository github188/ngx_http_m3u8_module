crossdomain.xml放在html目录下，设置可访问权限
和nginx.conf中的add_header设置跨域问题

编译命令行
./configure --prefix=./_install/ --add-module=./extends/ngx_http_m3u8_module --with-http_ssl_module --with-openssl=/usr/local --with-http_v2_module --with-http_realip_module --with-http_gzip_static_module --with-http_stub_status_module