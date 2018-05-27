
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

// ��������� begin
typedef enum
{
	AVErrSuccess			= 0,			// �ɹ�
	AVErrUnInit				= -1,			// δ��ʼ��
	AVErrParam				= -2,			// ��������
	AVErrGetPort			= -3,			// ��ȡͨ��ʧ��
	AVErrPort				= -4,			// ͨ��δ������û�п���ͨ��
	AVErrEncodeAACInit		= -5,			// ����AAC��ʼ��ʧ�ܻ�δ��ʼ��
	AVErrEncodeAAC			= -6,			// AAC����ʧ��
	AVErrGetSpsPps			= -7,			// ��ȡSPS��PPSʧ��
	AVErrOpenRecFile		= -8,			// ��¼���ļ�ʧ��
	AVErrWriteRecFileHead	= -9,			// д��¼���ļ�ͷʧ��
	AVErrPlay				= -10,			// ��Ƶδ����
	AVErrResolution			= -11,			// ��Ƶ�ֱ��ʴ���
	AVErrCapture			= -12,			// ץ��ʧ��
	AVErrCreateCaptureFile	= -13,			// ����ץ���ļ�ʧ��
	AVErrPlayRecFile_Open	= -14,			// ����¼���ļ�ʱ ���ļ���ʧ��
	AVErrPlayRecFile_GET	= -15,			// ����¼���ļ�ʱ ��ffmpeg��ȡ�ļ���Ϣʧ��
	AVErrPlayRecFile_GET_	= -16,			// ��ȡ֡�ʡ�ʱ��ʧ��
	AVErrReadH264			= -17,			// ���ڶ�H264�ļ�
	AVErrGetAudioType		= -18,			// û�л�ȡ����Ƶ֡����
	AVErrAddH264File		= -19,			// ��Ӳ���H264ʧ��
	AVErrOutPutBuff			= -20,			// ������
}AVErr;
// ��������� end

typedef enum
{
	VideoYUV420				= 0,			// YUV420
	VideoRGB32				= 1,			// 32λ��RGB
	VideoRGB24				= 2,			// 24λ��RGB
	VideoRGB565				= 3,			// 16λ��RGB(565)
	AudioPCM				= 4,			// ��ƵPCM
	RecCaptureSuccess		= 5,			// ��ʱ����ͼ���
	RecCutSuccess			= 6,			// ��ʱ���������
	RecPlaySuccess			= 7,			// ��ʱ���������
	CacheFree				= 8,			// �������
	RecStartPlay			= 9,			// ��ʼ������ʱ��
	RecLoading				= 10,			// ��ʱ��������
	RecLoadSuccess			= 11,			// ��ʱ���������
}AVDecType;

// ����ص��������ò��� begin
typedef struct stDecFrameParam
{
	int				nPort;					// ����ͨ����
	int				nDecType;				// ��ӦAVDecType
	unsigned char*	lpBuf;					// ����������
	int				lSize;					// ��������ݳ�
	int				lWidth;					// ��Ƶ��	
	int				lHeight;				// ��Ƶ��
	int				nSampleRate;			// ��Ƶ������ 
	int				nAudioChannels;			// ��Ƶͨ����
	int				nFrameNo;				// ��Ƶ֡��

}* PSTDecFrameParam;
// ����ص��������ò��� end


// ¼��ص��¼����� begin
typedef enum
{
	AVRecOpenSuccess		= 0,			// ¼��ʱ����¼��ɹ�
	AVRecOpenErr,							// ¼��ʱ����¼��ʧ��
	AVRecRetTime,							// ¼��ʱ������¼��ʱ��
	AVRecTimeEnd,							// ¼��ʱ������ʱ������£�¼��¼���¼�
	AVRetPlayRecTotalTime,					// ����¼��ʱ�� ����¼���ļ���ʱ��
	AVRetPlayRecTime,						// ����¼��ʱ�� ���ص�ǰ¼�񲥷�ʱ��
	AVRetPlayRecFinish,						// ����¼��ʱ�� ¼�񲥷����
	AVRetPlayRecSeekCapture,				// ����¼��ʱ�� ��ʱץ�����
	AVRetPlayRecRecordFinish,				// ��¼���ļ�ʱ�� ¼��MP4���
}AVRecEvent;
// ¼��ص��¼����� end


// ����ص�
typedef long (__stdcall* DECCallBack)(PSTDecFrameParam stDecParam, long lUserParam);
// ¼��ص� 
typedef long (__stdcall* RECCallBack)(long nPort, AVRecEvent eventRec, long lData, long lUserParam);
// ����ص�
typedef long (__stdcall* ENCCallBack)(unsigned char* lpBuf, long lSize, long lUserParam);

// ��ʼ�� nEnableLog 0-������־�� 1-������־  pLogPath �磺"D:\\Log\\",Ϊ����Ϊ��ǰ·��  
AVPLAYER_API long AV_Init(int nEnableLog, const char* pLogPath);

// ����ʼ��
AVPLAYER_API long AV_UnInit();

// ��ȡ����ͨ��
AVPLAYER_API long AV_GetPort();

// �ͷŽ���ͨ��
AVPLAYER_API long AV_FreePort(long nPort);

/*	 
��������Ƶ��
	nPort			����ͨ��, ��AV_GetPort���ء�
	pBuf			����Ƶ���ݣ��豸����һ֡���ݲ�������������ֱ�Ӷ�������ӿڡ�
	nSize			���ݳ��ȡ�
	nDecFlag		�Ƿ���루����ֻ¼�񲻽��룩 1 ����  0 ������
*/
AVPLAYER_API long AV_PutFrame(long nPort, unsigned char *pBuf, int nSize, int nDecFlag);


AVPLAYER_API long AV_SetVolume(long nPort, int nEnable, int nValue);

/*
���ý�����������ͣ������øýӿ�����������ΪYUV420
	nPort			����ͨ��, ��AV_GetPort���ء�
	nDecType		0 - YUV420, 1 - 32λ��RGB, 2 - 24λ��RGB, 3 - 16λ��RGB(565)
*/
AVPLAYER_API long AV_SetDecType(long nPort, int nDecType);

/* 
���û����С , �����øýӿ��򻺴����Ĭ��Ϊ60�������СĬ��Ϊ200K
	nType			0 ʵʱ������, 1 ¼�񻺴�
	nBuffCount		�������  70����Ĭ��Ϊʵʱ���ȣ� 70����Ĭ��Ϊ��������, Ĭ��Ϊ60
	nBuffSize		�����С
*/
AVPLAYER_API long AV_SetBuffSize(long nPort, int nType, int nBuffCount, int nBuffSize);

/*	 
��ʼ������ʾ(ʵʱ��/¼���ļ�)
	nPort			����ͨ��,��AV_GetPort���ء�
	hShowWnd		windows��ʾ���ھ��, Ϊ�ղ���ʾ��
	decodeCB		����ص������ؽ�������ݼ������Ϣ��
	pUserParam		�û��Զ������������ʲô�ص�����ʲô��
*/
AVPLAYER_API long AV_Play(long nPort, long lPlayWnd, /*DECCallBack*/void * decodeCB, long lUserParam);

// ֹͣ����
AVPLAYER_API long AV_Stop(long nPort);

// ���ñ��ش洢¼���ļ����ֻ���ͼƬ�ļ����֣� nType == 0 ͼƬ����, nType == 1 ¼���ļ�����, nType == 2 ���벥��¼��ص�������¼��ʱ��
AVPLAYER_API long AV_SetFileName(long nPort, int nType, const char *pFileName, void* recCB, long lUserParam);

// ץ��
AVPLAYER_API long AV_Capture(long nPort, const char *pFileName);

/*
��ʼ������ʾ
	nPort			����ͨ��,��AV_GetPort���ء�
	lpszPath		¼���ļ�·��
*/

AVPLAYER_API long AV_SetRecParam(long nPort, int nWidth, int nHeight, int nFrameRate, int nAACChannel);


// nAudioType 0.AAC 1.G711A
AVPLAYER_API long AV_StartRec(long nPort, const char *pFileName, /*RECCallBack*/void* recCB, long lUserParam);

// ֹͣ¼��
AVPLAYER_API long AV_StopRec(long nPort);

// ��ȡ¼��ʱ��
AVPLAYER_API DWORD AV_GetRecTime(long nPort);

// ����¼��ʱ�� (��)
AVPLAYER_API long AV_SetRecTime(long nPort, DWORD dwRecTime);


AVPLAYER_API long AV_SetRecTime(long nPort, DWORD dwRecTime);
// ���ô�¼���ļ���¼��MP4������ 
//	nIsRec �Ƿ�¼��, nStartTime¼��ʼʱ�䣬  nTotalTime¼��ʱ�� 
AVPLAYER_API long AV_SetH264FileRecParam(long nPort, int nIsRec, const char *pMp4FileName, int nStartTime, int nTotalTime);

// nIsRand �Ƿ������ĳ��I֡����
AVPLAYER_API long AV_StartDecH264File(long nPort, const char *pFileName, int nIsRand, void* playRecCB, long lUserParam);
AVPLAYER_API long AV_StopDecH264File(long nPort);
AVPLAYER_API long AV_AddH264File(long nPort, const char *pFileName, int nFileNameLen);
// ��¼���ļ�(MP4)
// dwDuration ����¼���ļ�ʱ���� dwFrameRate ����¼���ļ�֡��, playRecCB �ص�����ǰ����¼���ʱ�䣬 pUserParam �û��Զ������
AVPLAYER_API long AV_OpenRecFile(long nPort, const char* pFileName, DWORD* dwDuration, DWORD* dwFrameRate, void* playRecCB, long lUserParam);
// �ر�¼���ļ�
AVPLAYER_API long AV_CloseRecFile(long nPort);

// nPause = 1 ��ͣ�� nPause = 0 �ָ�����
AVPLAYER_API long AV_RecPause(long nPort, int nPause);

// ���ò����ٶ� nSpeed -4 �� + 4 ֮�� ����16���� �� ��16����, 0 ���������ٶ�
AVPLAYER_API long AV_RecSetSpeed(long nPort, long nSpeed);

// ��ȡ�����ٶ�
AVPLAYER_API long AV_RecGetSpeed(long nPort);

// �ƶ���ָ��ʱ�䲥�� ���ʱ��(����ڿ�ʼλ��0��)
AVPLAYER_API long AV_RecSeek(long nPort, DWORD dwTime, const char* pFileName);

/*
unsigned char* pRgbData= new unsigned char[nWidth*nHeight*3];

AV_YUV2RGB(yuvData, nWidth, nHeight, pRgbData, &nRgbLen);
*/
AVPLAYER_API long AV_YUV420_2_RGB24(unsigned char* pYUV, int nWidth, int nHeight, unsigned char* pRGB);
/*
���ܣ�PCM����AAC 
������
	nSample		������Ƶ(PCM)������
	nChannel	������Ƶ������
	pInData		������Ƶ(PCM)����
	nInLen		������Ƶ���ݳ���
	pOutData	�����Ƶ(AAC)����
	nOutLen		������Ƶ���ݳ���
����ֵ��
	AVPErr �����붨��
*/
AVPLAYER_API long AV_EncodeAACStart(DWORD nSample, int nChannel, ENCCallBack encCB, long lUserParam);
AVPLAYER_API long AV_EncodeAACPutBuf(unsigned  char *pInData, int nInLen);
AVPLAYER_API long AV_EncodeAACStop();



AVPLAYER_API long AV_EncodePCM2G711A(DWORD nSample, int nChannel, unsigned  char *pInData, int nInLen, unsigned  char **pOutData, int *nOutLen);

AVPLAYER_API long AV_DeleteData(char *pData);
// windows����Ƶ�ɼ������� begin
#if (defined _WIN32) || (defined _WIN64)

// ������Ƶ PLAY֮�����
AVPLAYER_API long AV_EnableAudio(long nPort, int nEnable, int nSample);

// ��Ƶ�ɼ��ص�
typedef long (__stdcall* PickAudioCallBack)(unsigned char* pBuf, DWORD dwSize, long lUserParam);

// ��ʼ��Ƶ�ɼ�
AVPLAYER_API long AV_StartPickAudio(DWORD nSamples, PickAudioCallBack fcb, long lUserParam);

// ������Ƶ�ɼ�
AVPLAYER_API long AV_StopPickAudio();
#endif
// windows����Ƶ�ɼ������� end

#endif