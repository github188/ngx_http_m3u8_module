#include <ngx_config.h>
#include <ngx_core.h>
#include <ngx_http.h>
#include <stdio.h>  
#include <dirent.h>  
#include <stdlib.h>  
#include <string.h>  
#include <sys/types.h>  
#include <sys/stat.h>  
#include <fcntl.h>
#include "m3u8_factory.h"


static ngx_int_t ngx_http_m3u8_handler(ngx_http_request_t *r);
static char* ngx_http_m3u8(ngx_conf_t *cf, ngx_command_t *cmd, void *conf);
static ngx_int_t ngx_http_default_m3u8(ngx_http_request_t *r, ngx_str_t path);
static ngx_int_t ngx_http_p2p_data_ask(ngx_str_t path);

#define DEFAULT_M3U8_PATH	"html/loading.m3u8"
/**
 * 处理nginx.conf中的配置命令解析
 * 例如：
 * location ~*\.(m3u8)$ {
 *  	m3u8
 * }
 * 当用户请求:http://127.0.0.1/.m3u8的时候，请求会跳转到m3u8这个配置上
 * m3u8的命令行解析回调函数：ngx_http_m3u8
 */
static ngx_command_t ngx_http_m3u8_commands[] = {
		{
				ngx_string("m3u8"),
				NGX_HTTP_MAIN_CONF | NGX_HTTP_SRV_CONF | NGX_HTTP_LOC_CONF | NGX_HTTP_LMT_CONF | NGX_CONF_NOARGS,
				ngx_http_m3u8,
				NGX_HTTP_LOC_CONF_OFFSET,
				0,
				NULL
		},
		ngx_null_command
};

/**
 * 模块上下文
 */
static ngx_http_module_t ngx_http_m3u8_module_ctx = { NULL, NULL, NULL, NULL,
		NULL, NULL, NULL, NULL };

/**
 * 模块的定义
 */
ngx_module_t ngx_http_m3u8_module = {
		NGX_MODULE_V1,
		&ngx_http_m3u8_module_ctx,
		ngx_http_m3u8_commands,
		NGX_HTTP_MODULE,
		NULL,
		NULL,
		NULL,
		NULL,
		NULL,
		NULL,
		NULL,
		NGX_MODULE_V1_PADDING
};

ngx_int_t ngx_http_default_m3u8(ngx_http_request_t *r, ngx_str_t path)
{	
	char cur_path[NGX_MAX_PATH] = {0};
	ngx_log_t* log;

	log = r->connection->log;
	ngx_copy_file_t 		  cf;
	ngx_file_info_t 		  fi;
	if(ngx_strstr(path.data, "/hls/") == NULL){
		ngx_log_error(NGX_LOG_DEBUG_HTTP, log, 0,
		"path %s error", path.data);
		return NGX_HTTP_NOT_FOUND;
	}

	m3u8_get_current_path(cur_path, sizeof(NGX_MAX_PATH));
	strcat(cur_path, DEFAULT_M3U8_PATH);
	ngx_log_debug(NGX_LOG_DEBUG_HTTP, log, 0,
		"current path===%s, %s", cur_path, path.data);
	
	if (ngx_link_info(path.data, &fi) == NGX_FILE_ERROR){	//文件不存在
	
		ngx_str_t tmp = ngx_string(cur_path);
		ngx_link_info(tmp.data, &fi);
			
		cf.size = ngx_file_size(&fi);
		cf.buf_size = 0;
		cf.access = NGX_FILE_DEFAULT_ACCESS;
		cf.time = ngx_file_mtime(&fi);
		cf.log = r->connection->log;
				
		if (ngx_copy_file(tmp.data, path.data, &cf) != NGX_OK) {
			return NGX_HTTP_NO_CONTENT;
		}

		ngx_log_debug(NGX_LOG_DEBUG_HTTP, log, 0, "default m3u8 set OK");
	}
	else
	{
		if(ngx_is_dir(&fi)){								//如果是目录
			return NGX_HTTP_CONFLICT;
		}
	}
	
	ngx_http_p2p_data_ask(path);
	return NGX_HTTP_OK;
}

ngx_int_t ngx_http_p2p_data_ask(ngx_str_t path)
{
	u_char* 	p;
	u_char* 	hls;
	ngx_int_t 	pl;
	u_char 		uid[32] = {0};
	ngx_str_t 	dir = ngx_string("/hls/");
	
	p   = (u_char *)ngx_strstr(path.data, ".m3u8");
	hls = (u_char *)ngx_strstr(path.data, dir.data);
	pl  = (ngx_int_t)(p-hls) - ngx_strlen(dir.data);
	
	ngx_snprintf(uid, pl,"%s", hls + ngx_strlen(dir.data));
	m3u8_factory_hls_open(NULL, (char*)uid);
	
	return NGX_HTTP_OK;
}

/**
 * 模块回调函数
 */
static ngx_int_t ngx_http_m3u8_handler(ngx_http_request_t *r) {
	u_char					  *last;
	off_t					   start, len;
	size_t					   root;
	ngx_int_t				   rc;
	ngx_uint_t				   level, i;
	ngx_str_t				   path;
	ngx_log_t				  *log;
	ngx_buf_t				  *b;
	ngx_chain_t 			   out[2];
	ngx_open_file_info_t	   of;
	ngx_http_core_loc_conf_t  *clcf;


	if (!(r->method & (NGX_HTTP_GET|NGX_HTTP_HEAD))) {
		return NGX_HTTP_NOT_ALLOWED;
	}

	if (r->uri.data[r->uri.len - 1] == '/') {
		return NGX_DECLINED;
	}

	rc = ngx_http_discard_request_body(r);

	if (rc != NGX_OK) {
		return rc;
	}

	last = ngx_http_map_uri_to_path(r, &path, &root, 0);
	if (last == NULL) {
		return NGX_HTTP_INTERNAL_SERVER_ERROR;
	}

	log = r->connection->log;

	path.len = last - path.data;

	ngx_log_debug1(NGX_LOG_DEBUG_HTTP, log, 0,
				   "http m3u8 filename: \"%V\"", &path);
	
	rc = ngx_http_default_m3u8(r, path);
	if(rc != NGX_HTTP_OK){
		return NGX_HTTP_INTERNAL_SERVER_ERROR;
	}
	
	clcf = ngx_http_get_module_loc_conf(r, ngx_http_core_module);

	ngx_memzero(&of, sizeof(ngx_open_file_info_t));

	of.read_ahead = clcf->read_ahead;
	of.directio = clcf->directio;
	of.valid = clcf->open_file_cache_valid;
	of.min_uses = clcf->open_file_cache_min_uses;
	of.errors = clcf->open_file_cache_errors;
	of.events = clcf->open_file_cache_events;

	if (ngx_http_set_disable_symlinks(r, clcf, &path, &of) != NGX_OK) {
		return NGX_HTTP_INTERNAL_SERVER_ERROR;
	}

	if (ngx_open_cached_file(clcf->open_file_cache, &path, &of, r->pool)
		!= NGX_OK)
	{
		switch (of.err) {

		case 0:
			return NGX_HTTP_INTERNAL_SERVER_ERROR;

		case NGX_ENOENT:
		case NGX_ENOTDIR:
		case NGX_ENAMETOOLONG:

			level = NGX_LOG_ERR;
			rc = NGX_HTTP_NOT_FOUND;
			break;

		case NGX_EACCES:
#if (NGX_HAVE_OPENAT)
		case NGX_EMLINK:
		case NGX_ELOOP:
#endif

			level = NGX_LOG_ERR;
			rc = NGX_HTTP_FORBIDDEN;
			break;

		default:

			level = NGX_LOG_CRIT;
			rc = NGX_HTTP_INTERNAL_SERVER_ERROR;
			break;
		}

		if (rc != NGX_HTTP_NOT_FOUND || clcf->log_not_found) {
			ngx_log_error(level, log, of.err,
						  "%s \"%s\" failed", of.failed, path.data);
		}

		return rc;
	}

	if (!of.is_file) {

		if (ngx_close_file(of.fd) == NGX_FILE_ERROR) {
			ngx_log_error(NGX_LOG_ALERT, log, ngx_errno,
						  ngx_close_file_n " \"%s\" failed", path.data);
		}

		return NGX_DECLINED;
	}

	r->root_tested = !r->error_page;

	start = 0;
	len = of.size;
	i = 1;

	log->action = "sending m3u8 to client";

	r->headers_out.status = NGX_HTTP_OK;
	r->headers_out.content_length_n = len;
	r->headers_out.last_modified_time = of.mtime;

	if (ngx_http_set_etag(r) != NGX_OK) {
		return NGX_HTTP_INTERNAL_SERVER_ERROR;
	}

	if (ngx_http_set_content_type(r) != NGX_OK) {
		return NGX_HTTP_INTERNAL_SERVER_ERROR;
	}

	b = ngx_calloc_buf(r->pool);
	if (b == NULL) {
		return NGX_HTTP_INTERNAL_SERVER_ERROR;
	}

	b->file = ngx_pcalloc(r->pool, sizeof(ngx_file_t));
	if (b->file == NULL) {
		return NGX_HTTP_INTERNAL_SERVER_ERROR;
	}

	r->allow_ranges = 1;

	rc = ngx_http_send_header(r);

	if (rc == NGX_ERROR || rc > NGX_OK || r->header_only) {
		return rc;
	}

	b->file_pos = start;
	b->file_last = of.size;

	b->in_file = b->file_last ? 1: 0;
	b->last_buf = (r == r->main) ? 1 : 0;
	b->last_in_chain = 1;

	b->file->fd = of.fd;
	b->file->name = path;
	b->file->log = log;
	b->file->directio = of.is_directio;

	out[1].buf = b;
	out[1].next = NULL;

	return ngx_http_output_filter(r, &out[i]);
}

/**
 * 命令解析的回调函数
 * 该函数中，主要获取loc的配置，并且设置location中的回调函数handler
 */
static char *
ngx_http_m3u8(ngx_conf_t *cf, ngx_command_t *cmd, void *conf) {
	ngx_http_core_loc_conf_t *clcf;

	clcf = ngx_http_conf_get_module_loc_conf(cf, ngx_http_core_module);
	/* 设置回调函数。当请求http://127.0.0.1/hello的时候，会调用此回调函数 */
	clcf->handler = ngx_http_m3u8_handler;

	return NGX_CONF_OK;
}

