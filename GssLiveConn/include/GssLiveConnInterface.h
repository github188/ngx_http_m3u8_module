#ifndef _GSSLIVECONNINTERFACE_HH_
#define _GSSLIVECONNINTERFACE_HH_

#ifdef __cplusplus
extern "C" {
#endif

	enum {
		GSS_LIVE_CONN_AUDIO_TYPE_UNKNOWN = 0,
		GSS_LIVE_CONN_AUDIO_TYPE_AAC,
		GSS_LIVE_CONN_AUDIO_TYPE_G711,
	};//audioType

	enum {
		GSS_LIVE_CONN_ERROR_UNKNOWN = -10000,
		GSS_LIVE_CONN_ERROR_NEW_FAILED, //创建实例失败
		GSS_LIVE_CONN_ERROR_NO_INVALID_INDEX, //无有效INDEX可用
		GSS_LIVE_CONN_ERROR_INVALID_INDEX, //无效index
		GSS_LIVE_CONN_ERROR_START_FAILED, //LIVE CONN START FAILED
		GSS_LIVE_CONN_ERROR_CONNECTED_FAILED, //连接失败,具体错误原因，可以参见日志
		GSS_LIVE_CONN_ERROR_CURRENT_NO_FRAME, //当前队列没有可用帧

		GSS_LIVE_CONN_ERROR_OK = 0,
	}; //errcode

/*
PARAM :
			pserver -> dispatch server ip, ex : "120.23.23.33:6001" or "cnp2p.ulifecam.com:6001"
			plogpath -> local log path, if plogpath = null , no log will be generated. ex : plogpath = "/var/log/live555"
			loglvl -> local log level, (1->error,2->warn,4->info,8->debug), ex : loglvl = 1+2+4+8,将输出debug，info，warn，error等级的日志
return :
			0 -> success, other -> failed.
*/
int GssLiveConnInterfaceInit(const char* pserver, const char* plogpath, int loglvl,
			const char* sqlHost, int sqlPort, //数据的HOST,PORT
			const char* sqlUser, const char* sqlPasswd, const char* dbName, //数据库登录用户名和密码,数据库名称，
			int maxCounts, //连接池中数据库连接的最大数,假设有n个业务线程使用该连接池，建议:maxCounts=n,假设n>20, 建议maxCounts=20
			int maxPlayTime, //最大播放时长(单位分钟)
			int type);
void GssLiveConnInterfaceUnInit();


/*
PARAM :
			server -> tcp turn server ip
			port -> tcp turn server port
			uid -> device id
			bDispath -> 0/1   [ 0 ->直接连接转发服务器；1->会用到分派服务器返回的server，port，此时不关心server，port，可写为server=NULL，port=0 ]
return :
			>= 0, 返回的是连接标识glcIndex， 用于后面的接口
			< 0,  创建失败， 也算错误码，具体参见errcode
*/
int GssLiveConnInterfaceCreate(const char* server, unsigned short port, char* uid, int bDispath);
int GssLiveConnInterfaceDestroy( int glcIndex);

int GssLiveConnInterfaceTimeOut(int glcIndex);

int GssLiveConnInterfaceGetVideoFrame(int glcIndex, unsigned char** pData, int *datalen, unsigned int *pts);
void GssLiveConnInterfaceFreeVideoFrame(int glcIndex);

int GssLiveConnInterfaceGetAudioFrame(int glcIndex, unsigned char** pData, int *datalen, unsigned int *pts);
void GssLiveConnInterfaceFreeAudioFrame(int glcIndex);

int GssLiveConnInterfaceGetAudioType(int glcIndex);

#ifdef __cplusplus
}
#endif

#endif