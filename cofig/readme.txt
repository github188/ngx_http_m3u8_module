1. nginx�������
	�汾��nginx-1.13.12
	������Ҫicon��openssl
	config��������
./configure --prefix=./_install/ --add-module=./extends/ngx_http_m3u8_module --with-http_ssl_module --with-openssl=/usr/local --with-http_v2_module --with-http_realip_module --with-http_gzip_static_module --with-http_stub_status_module

2. nginx��������
	1�����Ʊ�Ŀ¼�µ�֤��cert.key��cert.pem��nginx��װĿ¼��confĿ¼��
	2�����Ʊ�Ŀ¼�µ�nginx.conf�ļ��滻nginx��װĿ¼��confĿ¼��nginx.conf
	3�����Ʊ�Ŀ¼�µ�gss_globle.conf��nginx��װĿ¼
	4��nginx��װĿ¼��htmlĿ¼�´���hlsĿ¼
	5�����Ʊ�Ŀ¼�µ�loading_01.ts��nginx��װĿ¼��htmlĿ¼�µ�hlsĿ¼
	6�����Ʊ�Ŀ¼�µ�loading.m3u8��crossdomain.xml��nginx��װĿ¼��htmlĿ¼

3. ���ݿ�����
	1����װmysql�����
	2��ִ�б�Ŀ¼�µ�database.sh�ű��������ݿ⣨ע�������и����Լ������ݿ��û����������޸Ľű���

4. ����nginx����
	nginx����ʱһ��Ҫ��Ӳ��� -p ָ��nginx��װĿ¼������·����
	����nginx��װĿ¼��/data/nginx_hls��������ָ�����£�
	/data/nginx_hls/sbin/nginx -p /data/nginx_hls/

5. ʹ��vlc��ͼ
	��VLC�����粥��URL�������ַ������nginx���з�������IPΪ192.168.1.100,��ۿ����豸uidΪA99762101001001�������ַ���£�
	https://192.168.1.100:4433/hls/A99762101001001.m3u8

6. �޸�nginx��������
	�����аٶ��޸�nginx.conf

7. �޸����Ƿ���Ĳ���
	�����ע���޸�gss_globle.conf����