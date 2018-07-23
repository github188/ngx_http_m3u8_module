#include "GssLiveConnInterface.h"
#include "GssLiveConn.h"
#include "MyClock.h"

#define MAX_GSSLIVECONNS_SIZE 1024
GssLiveConn* g_liveConns[MAX_GSSLIVECONNS_SIZE] = {0};
MyClock *g_lock = NULL;

int GssLiveConnInterfaceInit(const char* pserver, const char* plogpath, int loglvl)
{
	if(g_lock == NULL)
		g_lock = new MyClock;
	return GssLiveConn::GlobalInit(pserver,plogpath,loglvl);
}

void GssLiveConnInterfaceUnInit()
{
	GssLiveConn::GlobalUnInit();
	if(g_lock)
		delete g_lock;
}

int GetGssLiveConnValidIndex()
{
	int nIndex = -1;
	g_lock->Lock();
	for (int i = 0; i < MAX_GSSLIVECONNS_SIZE; i++)
	{
		if(g_liveConns[i] == NULL)
		{
			nIndex = i;
			break;
		}
	}
	g_lock->Unlock();
	return nIndex;
}

int GssLiveConnInterfaceCreate(const char* server, unsigned short port, char* uid, int bDispath)
{
	int nIndex = GSS_LIVE_CONN_ERROR_UNKNOWN;

	do 
	{
		nIndex = GetGssLiveConnValidIndex();
		if(nIndex == -1)
		{
			nIndex = GSS_LIVE_CONN_ERROR_NO_INVALID_INDEX;
			break;
		}

		char* pTmpServer = (char*)server;
		unsigned short tmpPort = port;
		if (bDispath)
		{
			pTmpServer = GssLiveConn::m_sGlobalInfos.domainDispath;
			tmpPort = GssLiveConn::m_sGlobalInfos.port;
		}
		
		GssLiveConn* pLiveConn = new GssLiveConn(pTmpServer, tmpPort, uid, (bool)bDispath);
		if(pLiveConn == NULL)
		{
			nIndex = GSS_LIVE_CONN_ERROR_NEW_FAILED;
			break;
		}

		if(!pLiveConn->Start())
		{
			nIndex = GSS_LIVE_CONN_ERROR_START_FAILED;
		}

		if (!pLiveConn->IsConnected())
		{
			nIndex = GSS_LIVE_CONN_ERROR_CONNECTED_FAILED;
		}

		if (nIndex < 0)
		{
			delete pLiveConn;
		}
		else
		{
			g_liveConns[nIndex] = pLiveConn;
		}
	} while (0);

	return nIndex;
}

int GssLiveConnInterfaceDestroy( int glcIndex)
{
	if (glcIndex >= 0 && glcIndex < MAX_GSSLIVECONNS_SIZE)
	{
		g_lock->Lock();
		if (g_liveConns[glcIndex])
		{
			delete g_liveConns[glcIndex];
			g_liveConns[glcIndex] = NULL;
		}
		g_lock->Unlock();
		return GSS_LIVE_CONN_ERROR_OK;
	}
	else
	{
		return GSS_LIVE_CONN_ERROR_INVALID_INDEX;
	}
}

int GssLiveConnInterfaceGetVideoFrame(int glcIndex, unsigned char** pData, int *datalen, unsigned int *pts)
{
	if (glcIndex >= 0 && glcIndex < MAX_GSSLIVECONNS_SIZE)
	{
		int len = 0;
		unsigned int timestamp = 0;
		if( g_liveConns[glcIndex]->GetVideoFrame(pData,len,timestamp) )
		{
			*datalen = len;
			*pts = timestamp;
			return GSS_LIVE_CONN_ERROR_OK;
		}
		else
		{
			return GSS_LIVE_CONN_ERROR_CURRENT_NO_FRAME;
		}
	}
	else
	{
		return GSS_LIVE_CONN_ERROR_INVALID_INDEX;
	}
}
void GssLiveConnInterfaceFreeVideoFrame(int glcIndex)
{
	if (glcIndex >= 0 && glcIndex < MAX_GSSLIVECONNS_SIZE)
	{
		g_liveConns[glcIndex]->FreeVideoFrame();
	}
}

int GssLiveConnInterfaceGetAudioFrame(int glcIndex, unsigned char** pData, int *datalen, unsigned int *pts)
{
	if (glcIndex >= 0 && glcIndex < MAX_GSSLIVECONNS_SIZE)
	{
		int len = 0;
		unsigned int timestamp = 0;
		if( g_liveConns[glcIndex]->GetAudioFrame(pData,len,timestamp) )
		{
			*datalen = len;
			*pts = timestamp;
			return GSS_LIVE_CONN_ERROR_OK;
		}
		else
		{
			return GSS_LIVE_CONN_ERROR_CURRENT_NO_FRAME;
		}
	}
	else
	{
		return GSS_LIVE_CONN_ERROR_INVALID_INDEX;
	}
}
void GssLiveConnInterfaceFreeAudioFrame(int glcIndex)
{
	if (glcIndex >= 0 && glcIndex < MAX_GSSLIVECONNS_SIZE)
	{
		g_liveConns[glcIndex]->FreeAudioFrame();
	}
}

int GssLiveConnInterfaceGetAudioType(int glcIndex)
{
	if (glcIndex >= 0 && glcIndex < MAX_GSSLIVECONNS_SIZE)
	{
		if (g_liveConns[glcIndex]->IsKnownAudioType())
		{
			if (g_liveConns[glcIndex]->IsAudioG711AType())
			{
				return GSS_LIVE_CONN_AUDIO_TYPE_G711;
			}
			if (g_liveConns[glcIndex]->IsAudioAacType())
			{
				return GSS_LIVE_CONN_AUDIO_TYPE_AAC;
			}
			return GSS_LIVE_CONN_AUDIO_TYPE_UNKNOWN;
		}
		else
		{
			return GSS_LIVE_CONN_AUDIO_TYPE_UNKNOWN;
		}
	}
	else
	{
		return GSS_LIVE_CONN_ERROR_INVALID_INDEX;
	}
}