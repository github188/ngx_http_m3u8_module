#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <errno.h>
#include <unistd.h>

#include "m3u8_factory.h"
#include "utils_log.h"
#include "comm_helper.h"

#include "GssLiveConnInterface.h"
#include "AVPlayer.h"

#define HLS_FRAGMENT 		3
#define HLS_KEEPLIVE_SEC 	20

#define M3U8_HEADER "\
#EXTM3U\r\n\
#EXT-X-TARGETDURATION:%d\r\n\
#EXT-X-VERSION:6\r\n\
#EXT-X-MEDIA-SEQUENCE:%d\r\n\
"
#define M3U8_INFO "\
#EXT-X-DISCONTINUITY\r\n\
#EXTINF:%d,\r\n\
%s\r\n\
"

#define P2P_DISPATCH_ADDR	"cnp2p.ulifecam.com:6001" //cnp2p.ulifecam.com:6001
static m3u8_factory_t* s_m3u8_factory = NULL;

//public
m3u8_factory_t* m3u8_factory_create()
{
	if(s_m3u8_factory == NULL){
		////////////////////////////////////////
		GssLiveConnInterfaceInit(P2P_DISPATCH_ADDR, ".", 1);
		AV_Init(0, ".");
		
		LOGI_print("start connect p2p server:%s", P2P_DISPATCH_ADDR);

		s_m3u8_factory = (m3u8_factory_t*)malloc(sizeof(m3u8_factory_t));
		s_m3u8_factory->stop_liveness = 0;
		s_m3u8_factory->th_liveness = 0;
		s_m3u8_factory->factory = s_m3u8_factory;
		cmap_init(&s_m3u8_factory->hls_map);
		m3u8_get_current_path(s_m3u8_factory->cur_path, sizeof(s_m3u8_factory->cur_path));
		LOGI_print("cur_path:%s", s_m3u8_factory->cur_path);

		int ret = pthread_create(&s_m3u8_factory->th_liveness, NULL, m3u8_factory_hls_liveness_proc, (void*)s_m3u8_factory);
		if(ret != 0 ){
			LOGE_print(" create thread_turn_dispatch error:%s", strerror(ret));
			exit(-1);
		}
	}

	return s_m3u8_factory;
}

void m3u8_factory_destory(m3u8_factory_t* h)
{
	if(h == NULL)
		return;
	if(s_m3u8_factory->th_liveness != 0)
	{
		s_m3u8_factory->stop_liveness = 1;
		pthread_join(s_m3u8_factory->th_liveness, NULL);
		s_m3u8_factory->th_liveness = 0;
	}
	
	int i;
	int size = cmap_size(&h->hls_map);
	for(i=0; i<size; i++){
		cmapnode* node = cmap_index_get(&h->hls_map, i);
		if(node){
			m3u8_node_t* clnt = (m3u8_node_t*)(node->data);
			if(clnt) 
			{
				m3u8_node_destory(clnt);
				node->data = NULL;
			}
		}
	}
	cmap_clear(&h->hls_map);
	cmap_destory(&h->hls_map);

	free(h);
	s_m3u8_factory = NULL;
	//////////////////////////////////
	GssLiveConnInterfaceUnInit();
	AV_UnInit();
}

int m3u8_factory_hls_open(m3u8_factory_t* h, char* uid)
{
	if(uid == NULL)
		return -1;
	if(h == NULL)
		h = m3u8_factory_create();
	LOGI_print("h:%p uid:%s", h, uid);

	m3u8_node_t* client = (m3u8_node_t*)cmap_find(&h->hls_map, uid);
	if(client == NULL){
		LOGW_print("cmap_find %s error", uid);
		m3u8_node_t* node = m3u8_node_create(h, uid);
		if(node != NULL)
		{
			int ret = cmap_insert(&h->hls_map, uid, node);
			if(ret != 0)
			{
				LOGW_print("hls_map cmap_insert error %s", uid);
			}
			else
			{
				LOGW_print("hls_map cmap_insert %s", uid);
				m3u8_node_gss_open(node);
			}
		}
	}
	else{
		LOGE_print("uid:%s", uid);
	}
	LOGI_print("hls_map size:%d", cmap_size(&h->hls_map));
	m3u8_factory_hls_liveness_set(h, uid);
	
	return 0;
}

int m3u8_factory_hls_close(m3u8_factory_t* h, char* uid)
{
	if(uid == NULL)
		return -1;

	return 0;
}

m3u8_factory_t* m3u8_factory_get()
{
	return s_m3u8_factory;
}

void m3u8_get_current_path(char* cur_path, int size)
{
	char* path = 0;
	char szPath[M3U8_MAX_PATH] = {0};
	if(readlink("/proc/self/exe", szPath, M3U8_MAX_PATH))
	{
		LOGI_print("szPath:%s", szPath);
		path = strstr(szPath, "sbin");
		if(path)
		{
			strncpy(cur_path, szPath, path - szPath);
		}
	}
}

//private
int m3u8_factory_hls_liveness_set(m3u8_factory_t* h, char* uid){
	LOGI_print("uid:%s", uid);
	m3u8_node_t* client = (m3u8_node_t*)cmap_find(&h->hls_map, uid);
	if(client != NULL)
		client->liveness = 1;
	
	return 0;
}

void* m3u8_factory_hls_liveness_proc(void* args)
{
	int i;
	m3u8_factory_t* h = (m3u8_factory_t*)args;
	while(h->stop_liveness != 1)
	{
		LOGI_print("hls_map size:%d", cmap_size(&h->hls_map));
		int erase = 0;
		int size = cmap_size(&h->hls_map);
		for(i=0; i<size; i++)
		{
			cmapnode* mnode = cmap_index_get(&h->hls_map, i);
			if(mnode == NULL) continue;
			
			m3u8_node_t* client = (m3u8_node_t*)(mnode->data);
			if(client == NULL) continue;

			if(client->liveness == 1)
			{
				LOGI_print("live hls set liveness = 0");
				client->liveness = 0;
			}
			else
			{
				LOGI_print("TODO: close this node");
				m3u8_node_destory(client);
				cmap_erase(&h->hls_map, mnode->key);
				erase = 1;
				break;
			}
		}
		if(erase == 1)
			continue;
		
		usleep(HLS_KEEPLIVE_SEC*1000*1000);
	}
	return NULL;
}

m3u8_node_t* m3u8_node_create(m3u8_factory_t* h, char* uid)
{
	m3u8_node_t* node = (m3u8_node_t*)malloc(sizeof(m3u8_node_t));

	node->stop_ts_build = 0;
	node->ht_ts_build = 0;
	node->factory = h;
	strcpy(node->uid, uid);
	node->liveness = 1;
	strcpy(node->m3u8, uid);
	strcat(node->m3u8, ".m3u8");
	node->m3u8_index = 1;
	cqueue_init(&node->ts_queue);

	//默认的加载视频缓存
	ts_info_t* info = (ts_info_t*)malloc(sizeof(ts_info_t));
	info->inf = 10;
	strcpy(info->path, "loading_01.ts");
	cqueue_enqueue(&node->ts_queue, (void *)info);
	
	LOGI_print("uid:%s m3u8:%s",node->uid, node->m3u8);
	return node;
}
void m3u8_node_destory(m3u8_node_t* node)
{
	m3u8_node_gss_close(node);
	
	cqueue_clear(&node->ts_queue);
	cqueue_destory(&node->ts_queue);

	LOGI_print("delete ts file");
	LOGI_print("delete m3u8 file");
	char cmd[128] = {0};
	snprintf(cmd, 128, "rm -rf %shtml/hls/%s*", node->factory->cur_path, node->uid);
	exec_cmd(cmd);

	free(node);
}

int m3u8_node_gss_open(m3u8_node_t* node)
{
	LOGI_print("m3u8_node_gss_open");
	node->glc_index = GssLiveConnInterfaceCreate(NULL, 0 , node->uid, 1);
	node->av_port = AV_GetPort();
	node->recflush = 1;
	node->fileindex = 0;
	printf("av_port:%d\n", node->av_port);
	
	LOGI_print("gss connect p2p, start recv thread, thread write ts file");
	int ret = pthread_create(&node->ht_ts_build, NULL, m3u8_node_ts_buid_proc, (void*)node);
	if(ret != 0 ){
		LOGE_print(" create thread_turn_dispatch error:%s", strerror(ret));
		exit(-1);
	}
		
	return 0;
}

int m3u8_node_gss_close(m3u8_node_t* node)
{
	LOGI_print("m3u8_node_gss_close");
	if(node->ht_ts_build != 0)
	{
		node->stop_ts_build = 1;
		pthread_join(node->ht_ts_build, NULL);
		node->ht_ts_build = 0;
	}
	GssLiveConnInterfaceDestroy(node->glc_index);
	
	return 0;
}

void m3u8_node_update(m3u8_node_t* node)
{
	char cmd[128] = {0};
	ts_info_t* info = NULL;
	
	int size = cqueue_size(&node->ts_queue);
	if(size >= 4)
	{
		info = (ts_info_t*)cqueue_dequeue(&node->ts_queue);
		if(strstr(info->path, "loading") == NULL)
		{
			//删除这个文件
			snprintf(cmd, 128, "rm -rf %shtml/hls/%s", node->factory->cur_path, info->path);
			exec_cmd(cmd);
		}
		node->m3u8_index++;
	}
	else
	{
		info = (ts_info_t*)malloc(sizeof(ts_info_t));
	}
	info->inf = 10;
	snprintf(cmd, 128, "%s_%d.ts", node->uid, node->fileindex);
	strcpy(info->path, cmd);
	
	//入队新的文件
	cqueue_enqueue(&node->ts_queue, (void *)info);
	LOGI_print("info inf:%d path:%s", info->inf, info->path);

	//这里需要做文件读写锁??
	snprintf(cmd, 128, "%shtml/hls/%s", node->factory->cur_path, node->m3u8);
	FILE* m3u8 = fopen(cmd, "wb");
	
	snprintf(cmd, 128, M3U8_HEADER, HLS_FRAGMENT, node->m3u8_index);
	fwrite(cmd, strlen(cmd), 1, m3u8);
	
	int i;
	size = cqueue_size(&node->ts_queue);
	for(i=0; i<size; i++)
	{
		info = (ts_info_t*)cqueue_get(&node->ts_queue, i);
		LOGI_print("index:%d info inf:%d path:%s", i, info->inf, info->path);
		snprintf(cmd, 128, M3U8_INFO, info->inf, info->path);
		fwrite(cmd, strlen(cmd), 1, m3u8);
	}
	LOGI_print("ts_queue_size:%d", size);
	fclose(m3u8);
	
}

static long m3u8_node_rec_call_back(long nPort, AVRecEvent eventRec, long lData, long lUserParam)
{
	LOGI_print("nPort:%ld, eventRec:%d lData:%ld lUserParam:%ld", nPort, eventRec, lData, lUserParam);
	if(eventRec == 2 && lData >= HLS_FRAGMENT)
	{
		m3u8_node_t* h = (m3u8_node_t*)lUserParam;
		h->recflush = 1;
	}

	return 0;
}

void* m3u8_node_ts_buid_proc(void* args)
{
	int ret = 0;
	m3u8_node_t* h = (m3u8_node_t*)args;

	while(h->stop_ts_build != 1)
	{				
		unsigned char* pData = NULL;
		int datalen;
		ret = GssLiveConnInterfaceGetVideoFrame(h->glc_index, &pData, &datalen);
		if(ret == 0)
		{
			GosFrameHead head;
			memcpy(&head, pData, sizeof(GosFrameHead));

			if(h->recflush == 1 && head.nFrameType == 1)
			{
				ret = AV_StopRec(h->av_port);
				if(ret == 0 && h->fileindex != 0)
				{
					m3u8_node_update(h);
				}
				
				h->fileindex++;
				char filename[64] = {0};
				snprintf(filename, 64, "%shtml/hls/%s_%d.ts", h->factory->cur_path, h->uid, h->fileindex);
				LOGI_print("h->av_port:%d filename:%s", h->av_port, filename);
				ret = AV_StartRec(h->av_port, filename, (void*)m3u8_node_rec_call_back, (long)h);
				h->recflush = 0;
			}
					
			ret = AV_PutFrame(h->av_port,pData,datalen, 0);
//			LOGI_print("datalen:%d ret:%d sWidth:%d sHeight:%d nFrameType:%d", datalen, ret, head.sWidth, head.sHeight, head.nFrameType);
			GssLiveConnInterfaceFreeVideoFrame(h->glc_index);
		}
		
		while(h->stop_ts_build != 1)
		{
			pData = NULL;
			ret = GssLiveConnInterfaceGetAudioFrame(h->glc_index, &pData, &datalen);
			if(ret == 0)
			{
				ret = AV_PutFrame(h->av_port,pData,datalen, 0);
				GssLiveConnInterfaceFreeAudioFrame(h->glc_index);
			}
			else
			{
				break;
			}
		}
	}

	AV_StopRec(h->av_port);
	AV_FreePort(h->av_port);
	
	return NULL;
}

