#include <stdio.h>
#include <stdlib.h>
#include "GssLiveConnInterface.h"
#include "AVPlayer.h"

long record_call_back(long nPort, AVRecEvent eventRec, long lData, long lUserParam)
{
	printf("nPort:%ld, eventRec:%d lData:%d lUserParam:%d", nPort, eventRec, lData, lUserParam);
}

int main()
{
	AV_Init(1, "./");
	long av_port = AV_GetPort();
	
	GssLiveConnInterfaceInit("cnp2p.ulifecam.com:6001", "./", 1);
	int glcIndex = GssLiveConnInterfaceCreate(NULL, 0 , "A99762101001002", 1);

 	AV_StartRec(av_port, "test.ts", record_call_back, NULL);
	while(1)
	{
		unsigned char* pData = NULL;
		int datalen;
		
		int ret = GssLiveConnInterfaceGetVideoFrame(glcIndex, &pData, &datalen);
		if(ret == 0)
		{
			
			AV_PutFrame(av_port,pData,datalen);
			printf("ret:%d datalen:%d \n", ret, datalen);
			GssLiveConnInterfaceFreeVideoFrame(glcIndex);
		}
		
		usleep(10*1000);
	}
	
	int GssLiveConnInterfaceDestroy(glcIndex);
	void GssLiveConnInterfaceUnInit();

	AV_StopRec(av_port);
	AV_FreePort(av_port);
	AV_UnInit();
	return 0;
}
