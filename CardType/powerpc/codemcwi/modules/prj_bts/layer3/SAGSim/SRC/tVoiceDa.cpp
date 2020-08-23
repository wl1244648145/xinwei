#include "tVoiceData.h"
#include "tVoiceSignal.h"
#include "stdio.h"

CTask_VoiceData* CTask_VoiceData::s_ptaskTVOICEDATA = NULL;

CTask_VoiceData* CTask_VoiceData::GetInstance()
{
	if(NULL==s_ptaskTVOICEDATA)
		s_ptaskTVOICEDATA = new CTask_VoiceData;
	return s_ptaskTVOICEDATA;
}

bool CTask_VoiceData::Initialize()
{
	return true;
}

void CTask_VoiceData::MainLoop()
{
	CTask_VoiceSignal::GetInstance()->initMediaServer();
	while(1)
	{
		CTask_VoiceSignal::GetInstance()->ProcessMedia();
		//printf("aaa");
	}

}

