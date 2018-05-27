#ifndef M3U8_FACTORY
#define M3U8_FACTORY
#include <pthread.h>
#include "cmap.h"
#include "cqueue.h"
#include "lock_utils.h"

typedef struct m3u8_factory_T
{
	struct m3u8_factory_T* factory;
	cmap			hls_map;
	CMtx			mtx_hls_map;
	pthread_t		th_liveness;
	int				stop_liveness;
}m3u8_factory_t;

typedef struct m3u8_node
{
	m3u8_factory_t* factory;
	char			uid[64];			//对应的数据源的uid
	int				liveness;		//是否存活，否则将会被m3u8_factory_hls_liveness_proc清除
	char			m3u8[64];		//m3u8文件
	cqueue			ts_queue;		//当前m3u8文件描述的ts文件队列
	CMtx			mtx_ts;
	int				m3u8_index;		//当前直播的m3u8序列号
	pthread_t		ht_ts_build;
	int				stop_ts_build;

	/////////////////////////////////
	int				glc_index;
	/////////////////////////////////
	int				av_port;
	int				recflush;
	int				fileindex;		//所写文件从fileindex=1开始，为0时表示准备开始
}m3u8_node_t;

//public
m3u8_factory_t* m3u8_factory_create();
void m3u8_factory_destory(m3u8_factory_t* h);
int m3u8_factory_hls_open(m3u8_factory_t* h, char* uid);
int m3u8_factory_hls_close(m3u8_factory_t* h, char* uid);
m3u8_factory_t* m3u8_factory_get();

//private
int m3u8_factory_hls_liveness_set(m3u8_factory_t* h, char* uid);
void* m3u8_factory_hls_liveness_proc(void* args);


typedef struct ts_info{
	int inf;
	char path[64];
}ts_info_t;

typedef struct _ggos_frame_head
{
	unsigned int	nFrameNo;			// 帧号
	unsigned int	nFrameType;			// 帧类型	gos_frame_type_t
	unsigned int	nCodeType;			// 编码类型 gos_codec_type_t
	unsigned int	nFrameRate;			// 视频帧率，音频采样率
	unsigned int	nTimestamp;			// 时间戳
	unsigned short	sWidth;				// 视频宽
	unsigned short	sHeight;			// 视频高
	unsigned int	reserved;			// 预留
	unsigned int	nDataSize;			// data数据长度
}GosFrameHead;

//public
m3u8_node_t* m3u8_node_create();
void m3u8_node_destory(m3u8_node_t* node);
int m3u8_node_gss_open(m3u8_node_t* node);
int m3u8_node_gss_close(m3u8_node_t* node);

//private
void* m3u8_node_ts_buid_proc(void* args);
#endif