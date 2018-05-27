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
	char			uid[64];			//��Ӧ������Դ��uid
	int				liveness;		//�Ƿ�����򽫻ᱻm3u8_factory_hls_liveness_proc���
	char			m3u8[64];		//m3u8�ļ�
	cqueue			ts_queue;		//��ǰm3u8�ļ�������ts�ļ�����
	CMtx			mtx_ts;
	int				m3u8_index;		//��ǰֱ����m3u8���к�
	pthread_t		ht_ts_build;
	int				stop_ts_build;

	/////////////////////////////////
	int				glc_index;
	/////////////////////////////////
	int				av_port;
	int				recflush;
	int				fileindex;		//��д�ļ���fileindex=1��ʼ��Ϊ0ʱ��ʾ׼����ʼ
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
	unsigned int	nFrameNo;			// ֡��
	unsigned int	nFrameType;			// ֡����	gos_frame_type_t
	unsigned int	nCodeType;			// �������� gos_codec_type_t
	unsigned int	nFrameRate;			// ��Ƶ֡�ʣ���Ƶ������
	unsigned int	nTimestamp;			// ʱ���
	unsigned short	sWidth;				// ��Ƶ��
	unsigned short	sHeight;			// ��Ƶ��
	unsigned int	reserved;			// Ԥ��
	unsigned int	nDataSize;			// data���ݳ���
}GosFrameHead;

//public
m3u8_node_t* m3u8_node_create();
void m3u8_node_destory(m3u8_node_t* node);
int m3u8_node_gss_open(m3u8_node_t* node);
int m3u8_node_gss_close(m3u8_node_t* node);

//private
void* m3u8_node_ts_buid_proc(void* args);
#endif