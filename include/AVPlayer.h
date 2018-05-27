
#ifndef _AVPLAYER_H_
#define _AVPLAYER_H_

#if (defined _WIN32) || (defined _WIN64)
#ifdef AVPLAYER_EXPORTS
#define AVPLAYER_API __declspec(dllexport)
#else
#define AVPLAYER_API __declspec(dllimport)
#endif
#elif (defined __APPLE_CPP__) || (defined __APPLE_CC__)
#if defined(__arm__) //debug++
typedef unsigned long       DWORD;
#elif defined(__arm64__)
typedef unsigned int        DWORD;
#endif
#ifdef AVPLAYER_EXPORTS
#define AVPLAYER_API extern "C"
#else
#define AVPLAYER_API
#endif
#else
typedef unsigned long       DWORD; //debug++
#define AVPLAYER_API //extern "C"
#endif	

#if (defined _WIN32) || (defined _WIN64)
#include "StdAfx.h"
#else
//typedef unsigned long       DWORD;
#ifndef	__stdcall
#define __stdcall
#endif
#endif

#define AVPLAYER_VERSION	"AVPlayer--V1.0.2.6-20180523"

// 定义错误码 begin
typedef enum
{
	AVErrSuccess			= 0,			// 成功
	AVErrUnInit				= -1,			// 未初始化
	AVErrParam				= -2,			// 参数错误
	AVErrGetPort			= -3,			// 获取通道失败
	AVErrPort				= -4,			// 通道未创建或没有空闲通道
	AVErrEncodeAACInit		= -5,			// 编码AAC初始化失败或未初始化
	AVErrEncodeAAC			= -6,			// AAC编码失败
	AVErrGetSpsPps			= -7,			// 获取SPS、PPS失败
	AVErrOpenRecFile		= -8,			// 打开录像文件失败
	AVErrWriteRecFileHead	= -9,			// 写入录像文件头失败
	AVErrPlay				= -10,			// 视频未播放
	AVErrResolution			= -11,			// 视频分辨率错误
	AVErrCapture			= -12,			// 抓拍失败
	AVErrCreateCaptureFile	= -13,			// 创建抓拍文件失败
	AVErrPlayRecFile_Open	= -14,			// 播放录像文件时 ，文件打开失败
	AVErrPlayRecFile_GET	= -15,			// 播放录像文件时 ，ffmpeg获取文件信息失败
	AVErrPlayRecFile_GET_	= -16,			// 获取帧率、时长失败
	AVErrReadH264			= -17,			// 正在读H264文件
	AVErrGetAudioType		= -18,			// 没有获取到音频帧类型
	AVErrAddH264File		= -19,			// 添加播放H264失败
	AVErrOutPutBuff			= -20,			// 缓存满
}AVErr;
// 定义错误码 end

typedef enum
{
	VideoYUV420				= 0,			// YUV420
	VideoRGB32				= 1,			// 32位的RGB
	VideoRGB24				= 2,			// 24位的RGB
	VideoRGB565				= 3,			// 16位的RGB(565)
	AudioPCM				= 4,			// 音频PCM
	RecCaptureSuccess		= 5,			// 历时流截图完成
	RecCutSuccess			= 6,			// 历时流剪切完成
	RecPlaySuccess			= 7,			// 历时流播放完成
	CacheFree				= 8,			// 缓存空闲
	RecStartPlay			= 9,			// 开始播放历时流
	RecLoading				= 10,			// 历时流加载中
	RecLoadSuccess			= 11,			// 历时流加载完成
}AVDecType;

// 解码回调函数所用参数 begin
typedef struct stDecFrameParam
{
	int				nPort;					// 解码通道号
	int				nDecType;				// 对应AVDecType
	unsigned char*	lpBuf;					// 解码后的数据
	int				lSize;					// 解码后数据长
	int				lWidth;					// 视频宽	
	int				lHeight;				// 视频高
	int				nSampleRate;			// 音频采样率 
	int				nAudioChannels;			// 音频通道数
	int				nFrameNo;				// 视频帧号

}* PSTDecFrameParam;
// 解码回调函数所用参数 end


// 录像回调事件类型 begin
typedef enum
{
	AVRecOpenSuccess		= 0,			// 录像时，打开录像成功
	AVRecOpenErr,							// 录像时，打开录像失败
	AVRecRetTime,							// 录像时，返回录像时间
	AVRecTimeEnd,							// 录像时，设置时长情况下，录像录完事件
	AVRetPlayRecTotalTime,					// 播放录像时， 返回录像文件总时长
	AVRetPlayRecTime,						// 播放录像时， 返回当前录像播放时长
	AVRetPlayRecFinish,						// 播放录像时， 录像播放完成
	AVRetPlayRecSeekCapture,				// 播放录像时， 定时抓拍完成
	AVRetPlayRecRecordFinish,				// 读录像文件时， 录制MP4完成
}AVRecEvent;
// 录像回调事件类型 end


// 解码回调
typedef long (__stdcall* DECCallBack)(PSTDecFrameParam stDecParam, long lUserParam);
// 录像回调 
typedef long (__stdcall* RECCallBack)(long nPort, AVRecEvent eventRec, long lData, long lUserParam);
// 编码回调
typedef long (__stdcall* ENCCallBack)(unsigned char* lpBuf, long lSize, long lUserParam);

// 初始化 nEnableLog 0-禁用日志， 1-启用日志  pLogPath 如："D:\\Log\\",为空则为当前路径  
AVPLAYER_API long AV_Init(int nEnableLog, const char* pLogPath);

// 反初始化
AVPLAYER_API long AV_UnInit();

// 获取解码通道
AVPLAYER_API long AV_GetPort();

// 释放解码通道
AVPLAYER_API long AV_FreePort(long nPort);

/*	 
接收音视频流
	nPort			解码通道, 由AV_GetPort返回。
	pBuf			音视频数据，设备返回一帧数据不用做其他处理直接丢到这个接口。
	nSize			数据长度。
	nDecFlag		是否解码（可以只录像不解码） 1 解码  0 不解码
*/
AVPLAYER_API long AV_PutFrame(long nPort, unsigned char *pBuf, int nSize, int nDecFlag);


AVPLAYER_API long AV_SetVolume(long nPort, int nEnable, int nValue);

/*
设置解码后数据类型，不调用该接口则解码后数据为YUV420
	nPort			解码通道, 由AV_GetPort返回。
	nDecType		0 - YUV420, 1 - 32位的RGB, 2 - 24位的RGB, 3 - 16位的RGB(565)
*/
AVPLAYER_API long AV_SetDecType(long nPort, int nDecType);

/* 
设置缓存大小 , 不调用该接口则缓存个数默认为60，缓存大小默认为200K
	nType			0 实时流缓存, 1 录像缓存
	nBuffCount		缓存个数  70以下默认为实时优先， 70以上默认为流畅优先, 默认为60
	nBuffSize		缓存大小
*/
AVPLAYER_API long AV_SetBuffSize(long nPort, int nType, int nBuffCount, int nBuffSize);

/*	 
开始解码显示(实时流/录像文件)
	nPort			解码通道,由AV_GetPort返回。
	hShowWnd		windows显示窗口句柄, 为空不显示。
	decodeCB		解码回调，返回解码后数据及相关信息。
	pUserParam		用户自定义参数，传入什么回调返回什么。
*/
AVPLAYER_API long AV_Play(long nPort, long lPlayWnd, /*DECCallBack*/void * decodeCB, long lUserParam);

// 停止解码
AVPLAYER_API long AV_Stop(long nPort);

// 设置本地存储录像文件名字或者图片文件名字， nType == 0 图片名字, nType == 1 录像文件名字, nType == 2 传入播放录像回调，返回录像时长
AVPLAYER_API long AV_SetFileName(long nPort, int nType, const char *pFileName, void* recCB, long lUserParam);

// 抓拍
AVPLAYER_API long AV_Capture(long nPort, const char *pFileName);

/*
开始解码显示
	nPort			解码通道,由AV_GetPort返回。
	lpszPath		录像文件路径
*/

AVPLAYER_API long AV_SetRecParam(long nPort, int nWidth, int nHeight, int nFrameRate, int nAACChannel);


// nAudioType 0.AAC 1.G711A
AVPLAYER_API long AV_StartRec(long nPort, const char *pFileName, /*RECCallBack*/void* recCB, long lUserParam);

// 停止录像
AVPLAYER_API long AV_StopRec(long nPort);

// 获取录像时间
AVPLAYER_API DWORD AV_GetRecTime(long nPort);

// 设置录像时长 (秒)
AVPLAYER_API long AV_SetRecTime(long nPort, DWORD dwRecTime);


AVPLAYER_API long AV_SetRecTime(long nPort, DWORD dwRecTime);
// 设置从录像文件中录制MP4参数， 
//	nIsRec 是否录像, nStartTime录像开始时间，  nTotalTime录像时长 
AVPLAYER_API long AV_SetH264FileRecParam(long nPort, int nIsRec, const char *pMp4FileName, int nStartTime, int nTotalTime);

// nIsRand 是否随机从某个I帧播放
AVPLAYER_API long AV_StartDecH264File(long nPort, const char *pFileName, int nIsRand, void* playRecCB, long lUserParam);
AVPLAYER_API long AV_StopDecH264File(long nPort);
AVPLAYER_API long AV_AddH264File(long nPort, const char *pFileName, int nFileNameLen);
// 打开录像文件(MP4)
// dwDuration 返回录像文件时长， dwFrameRate 返回录像文件帧率, playRecCB 回调出当前播放录像的时间， pUserParam 用户自定义参数
AVPLAYER_API long AV_OpenRecFile(long nPort, const char* pFileName, DWORD* dwDuration, DWORD* dwFrameRate, void* playRecCB, long lUserParam);
// 关闭录像文件
AVPLAYER_API long AV_CloseRecFile(long nPort);

// nPause = 1 暂停， nPause = 0 恢复播放
AVPLAYER_API long AV_RecPause(long nPort, int nPause);

// 设置播放速度 nSpeed -4 到 + 4 之间 即慢16倍速 到 快16倍速, 0 正常播放速度
AVPLAYER_API long AV_RecSetSpeed(long nPort, long nSpeed);

// 获取播放速度
AVPLAYER_API long AV_RecGetSpeed(long nPort);

// 移动到指定时间播放 相对时间(相对于开始位置0秒)
AVPLAYER_API long AV_RecSeek(long nPort, DWORD dwTime, const char* pFileName);

/*
unsigned char* pRgbData= new unsigned char[nWidth*nHeight*3];

AV_YUV2RGB(yuvData, nWidth, nHeight, pRgbData, &nRgbLen);
*/
AVPLAYER_API long AV_YUV420_2_RGB24(unsigned char* pYUV, int nWidth, int nHeight, unsigned char* pRGB);
/*
功能：PCM编码AAC 
参数：
	nSample		输入音频(PCM)采样率
	nChannel	输入音频声道数
	pInData		输入音频(PCM)数据
	nInLen		输入音频数据长度
	pOutData	输出音频(AAC)数据
	nOutLen		输入音频数据长度
返回值：
	AVPErr 错误码定义
*/
AVPLAYER_API long AV_EncodeAACStart(DWORD nSample, int nChannel, ENCCallBack encCB, long lUserParam);
AVPLAYER_API long AV_EncodeAACPutBuf(unsigned  char *pInData, int nInLen);
AVPLAYER_API long AV_EncodeAACStop();



AVPLAYER_API long AV_EncodePCM2G711A(DWORD nSample, int nChannel, unsigned  char *pInData, int nInLen, unsigned  char **pOutData, int *nOutLen);

AVPLAYER_API long AV_DeleteData(char *pData);
// windows下音频采集及播放 begin
#if (defined _WIN32) || (defined _WIN64)

// 开启音频 PLAY之后调用
AVPLAYER_API long AV_EnableAudio(long nPort, int nEnable, int nSample);

// 音频采集回调
typedef long (__stdcall* PickAudioCallBack)(unsigned char* pBuf, DWORD dwSize, long lUserParam);

// 开始音频采集
AVPLAYER_API long AV_StartPickAudio(DWORD nSamples, PickAudioCallBack fcb, long lUserParam);

// 结束音频采集
AVPLAYER_API long AV_StopPickAudio();
#endif
// windows下音频采集及播放 end

#endif