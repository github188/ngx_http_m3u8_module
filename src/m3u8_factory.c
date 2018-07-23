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

#define HLS_FRAGMENT 		4	//只用作最大片时长，这个会影响m3u8请求间隔
#define HLS_TS_REC_LEN 		3	//ts分片时长，不准确，有时间偏差
#define HLS_KEEPLIVE_SEC 	20
#define HLS_M3U8_LIST_SIZE 	3

#define M3U8_HEADER "\
#EXTM3U\r\n\
#EXT-X-TARGETDURATION:%d\r\n\
#EXT-X-VERSION:6\r\n\
#EXT-X-MEDIA-SEQUENCE:%d\r\n\
"
#define M3U8_INFO "\
#EXT-X-DISCONTINUITY\r\n\
#EXTINF:%.2f,\r\n\
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
			cur_path[path - szPath] = '\0';
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
	node->m3u8_index = 0;
	cqueue_init(&node->ts_queue);

	//默认的加载视频缓存
	ts_info_t* info = (ts_info_t*)malloc(sizeof(ts_info_t));
	info->inf = 4.0;
	strcpy(info->path, "loading_01.ts");
	cqueue_enqueue(&node->ts_queue, (void *)info);

	pthread_rwlock_init(&node->rwlock, NULL);
	LOGI_print("uid:%s m3u8:%s",node->uid, node->m3u8);
	return node;
}
void m3u8_node_destory(m3u8_node_t* node)
{
	m3u8_node_gss_close(node);
	
	cqueue_clear(&node->ts_queue);
	cqueue_destory(&node->ts_queue);
    pthread_rwlock_destroy(&node->rwlock);
	
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
	if(size > HLS_M3U8_LIST_SIZE)
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
	info->inf = node->timestamp_ref/1000.0;
	snprintf(cmd, 128, "%s_%d.ts", node->uid, node->fileindex);
	strcpy(info->path, cmd);
	
	//入队新的文件
	cqueue_enqueue(&node->ts_queue, (void *)info);
	LOGI_print("info inf:%.2f path:%s", info->inf, info->path);

	int i;
	float inf = 0.0;
	//最大时长TARGETDURATION
	for(i=0; i<size; i++)
	{
		info = (ts_info_t*)cqueue_get(&node->ts_queue, i);
		if(info->inf > inf)
		{
			inf = info->inf;
		}
	}

	//这里需要做文件读写锁??
	pthread_rwlock_wrlock(&node->rwlock);//请求写锁
	snprintf(cmd, 128, "%shtml/hls/%s", node->factory->cur_path, node->m3u8);
	FILE* m3u8 = fopen(cmd, "wb");

	snprintf(cmd, 128, M3U8_HEADER, abs(inf+0.5), node->m3u8_index);
	fwrite(cmd, strlen(cmd), 1, m3u8);
	
	size = cqueue_size(&node->ts_queue);
	for(i=0; i<size; i++)
	{
		info = (ts_info_t*)cqueue_get(&node->ts_queue, i);
		LOGI_print("index:%d info inf:%.2f path:%s", i, info->inf, info->path);
		snprintf(cmd, 128, M3U8_INFO, info->inf, info->path);
		fwrite(cmd, strlen(cmd), 1, m3u8);
	}
	fflush(m3u8);
	LOGI_print("ts_queue_size:%d", size);
	fclose(m3u8);
	pthread_rwlock_unlock(&node->rwlock);//请求写锁
}

static long m3u8_node_rec_call_back(long nPort, AVRecEvent eventRec, long lData, long lUserParam)
{
	LOGI_print("nPort:%ld, eventRec:%d lData:%ld lUserParam:%ld", nPort, eventRec, lData, lUserParam);

	if(eventRec == 2)
	{
		m3u8_node_t* h = (m3u8_node_t*)lUserParam;
		if(h->m3u8_index == 0 && lData > 1)	//前几个文件尽量小
		{
			h->recflush = 1;
		}
		else if(lData >= HLS_TS_REC_LEN)
		{
			h->recflush = 1;
		}
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
		unsigned int pts;
		ret = GssLiveConnInterfaceGetVideoFrame(h->glc_index, &pData, &datalen, &pts);
		if(ret == 0)
		{
			GosFrameHead head;
			memcpy(&head, pData, sizeof(GosFrameHead));

			if(h->recflush != 0 && head.nFrameType == 1)
			{
				ret = AV_StopRec(h->av_port);
				if(ret == 0 && h->fileindex != 0)
				{
					LOGI_print("pts:%u timestamp_ref:%d", pts, h->timestamp_ref);
					h->timestamp_ref = pts - h->timestamp_ref;
					m3u8_node_update(h);
				}
				
				h->fileindex++;
				char filename[64] = {0};
				snprintf(filename, 64, "%shtml/hls/%s_%d.ts", h->factory->cur_path, h->uid, h->fileindex);
				LOGI_print("h->av_port:%d filename:%s", h->av_port, filename);
				ret = AV_StartRec(h->av_port, filename, (void*)m3u8_node_rec_call_back, (long)h);
				h->timestamp_ref = pts;
				h->recflush = 0;
			}
					
			ret = AV_PutFrame(h->av_port,pData,datalen, 0);
//			LOGI_print("datalen:%d ret:%d sWidth:%d sHeight:%d nFrameType:%d", datalen, ret, head.sWidth, head.sHeight, head.nFrameType);
			GssLiveConnInterfaceFreeVideoFrame(h->glc_index);
		}
		
		while(h->stop_ts_build != 1)
		{
			pData = NULL;
			ret = GssLiveConnInterfaceGetAudioFrame(h->glc_index, &pData, &datalen, &pts);
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

