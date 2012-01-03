/***
	c86ctl
	
	Copyright (c) 2009-2010, honet. All rights reserved.
	This software is licensed under the BSD license.

	honet.kk(at)gmail.com
 */
#include "stdafx.h"

#include <windows.h>
#include <stdlib.h>
#include <stdio.h>
#include <tchar.h>
#include <assert.h>
#include <process.h>
#include <mmsystem.h>

#include <string>
#include <list>
#include <algorithm>



#include "c86ctl.h"
#include "module.h"
#include "config.h"
#include "vis_c86main.h"
#include "ringbuff.h"
#include "if.h"
#include "if_gimic_hid.h"
#include "if_gimic_midi.h"

#ifdef _MANAGED
#pragma managed(push, off)
#endif

// ------------------------------------------------------------------
static HANDLE gMainThread = 0;
static HANDLE gSenderThread = 0;
static UINT mainThreadID = 0;
static UINT senderThreadID = 0;
DWORD timerPeriod = 0;

static bool terminateFlag = false;
std::list< std::shared_ptr<GimicIF> > gGIMIC;


// ------------------------------------------------------------------
BOOL APIENTRY DllMain(
	HMODULE hModule,
	DWORD  ul_reason_for_call,
	LPVOID lpReserved
	)
{
	switch (ul_reason_for_call){
	case DLL_PROCESS_ATTACH:
		initializeWndManager();
		gModule = hModule;
		gConfig.init(hModule);
		break;
	case DLL_PROCESS_DETACH:
		uninitializeWndManager();
		gModule = NULL;
		break;
	case DLL_THREAD_ATTACH:
		break;
	case DLL_THREAD_DETACH:
		break;
	}
    return TRUE;
}

#ifdef _MANAGED
#pragma managed(pop)
#endif


// �`�揈���X���b�h
// mm-timer�ɂ��60fps����
unsigned int WINAPI threadMain(LPVOID param)
{
	MSG msg;
	CVisC86Main mainWnd;
	DWORD next;
	next = ::timeGetTime()*6 + 100;

	mainWnd.create();
	while(1){
		if( terminateFlag )
			break;
		
		if( ::PeekMessage(&msg , NULL , 0 , 0, PM_REMOVE )) {
			::TranslateMessage(&msg);
			::DispatchMessage(&msg);
		}

		DWORD now = ::timeGetTime() * 6;
		if(now < next){
			if( terminateFlag ) break;
			Sleep(1);
			continue;
		}
		next += 100;

		//update();
	}
	
	mainWnd.close();

	return (DWORD)msg.wParam;
}


// ���t�����X���b�h
// mm-timer�ɂ��1ms�P�ʏ���
// note: timeSetEvent()���Ɠ]���������^�C�}�������x���Ƃ���
//       �ē������̂��|�������̂Ŏ��O���[�v�ɂ���
unsigned int WINAPI threadSender(LPVOID param)
{
	const UINT period = 1;
	UINT next = ::timeGetTime() + period;

	while(1){
		if( terminateFlag )
			break;
		
		DWORD now = ::timeGetTime();
		if(now < next){
			if( terminateFlag ) break;
			Sleep(1);
			continue;
		}
		next += period;

		// update
		//for( std::list< std::shared_ptr<CGimicIF> >::iterator it = gGIMIC.begin(); it != gGIMIC.end(); it++ )
		//	(*it)->Tick();
		std::for_each( gGIMIC.begin(), gGIMIC.end(), [](std::shared_ptr<GimicIF> x){ x->Tick(); } );
	}
	
	return 0;
}



// ----------------------------------------------------------------------
// �O���C���^�[�t�F�C�X

int WINAPI c86ctl_initialize(void)
{
	// �C���X�^���X����
	//gGIMIC = GimicHID::CreateInstances();
	gGIMIC = GimicMIDI::CreateInstances();
	
	// �^�C�}����\�ݒ�
	TIMECAPS timeCaps;
	if( ::timeGetDevCaps(&timeCaps, sizeof(timeCaps)) == TIMERR_NOERROR ){
		::timeBeginPeriod(timeCaps.wPeriodMin);
		timerPeriod = timeCaps.wPeriodMin;
	}

	// �`��X���b�h�J�n
	if( gConfig.getInt(INISC_MAIN, _T("GUI"), 1) ){
		gMainThread = (HANDLE)_beginthreadex( NULL, 0, &threadMain, NULL, 0, &mainThreadID );
		if( !gMainThread )
			return C86CTL_ERR_UNKNOWN;
	}

	// ���t�X���b�h�J�n
	gSenderThread = (HANDLE)_beginthreadex( NULL, 0, &threadSender, NULL, 0, &senderThreadID );
	if( !gSenderThread ){
		SetThreadPriority( gSenderThread, THREAD_PRIORITY_ABOVE_NORMAL );
		terminateFlag = true;
		::WaitForSingleObject( gMainThread, INFINITE );
		return C86CTL_ERR_UNKNOWN;
	}
	
	return C86CTL_ERR_NONE;
}


int WINAPI c86ctl_deinitialize(void)
{
	c86ctl_reset();

	// �e��X���b�h�I��
	terminateFlag = true;
	if( gMainThread ){
		::WaitForSingleObject( gMainThread, INFINITE );
		gMainThread = NULL;
		mainThreadID = 0;
	}
	if( gSenderThread ){
		::WaitForSingleObject( gSenderThread, INFINITE );
		gSenderThread = NULL;
		senderThreadID = 0;
	}
	terminateFlag = false;

	// �C���X�^���X�폜
	// note: ���̃^�C�~���O�ŏI���������s����B
	gGIMIC.clear();

	// �^�C�}����\�ݒ����
	::timeEndPeriod(timerPeriod);
	
	return C86CTL_ERR_NONE;
}

int WINAPI c86ctl_reset(void)
{
	gOPNA[0].reset();
	gOPNA[1].reset();

	std::for_each( gGIMIC.begin(), gGIMIC.end(), [](std::shared_ptr<GimicIF> x){ x->Reset(); } );
	return 0;
}

void WINAPI c86ctl_out( UINT addr, UCHAR data )
{
	UINT id = 0;//addr >> 10;
	//addr &= 0x3ff;
	if( id < gGIMIC.size() ){
		gOPNA[0].setReg(addr,data);
		gGIMIC.front()->Out((uint16_t)addr,data);
	}

}

UCHAR WINAPI c86ctl_in( UINT addr )
{
	return gOPNA[0].getReg(addr);
}

// ---------------------------------------------------------------------------
#if 0
C86CTL_API INT c86ctl_get_version(UINT *ver)
{
	return C86CTL_ERR_NOT_IMPLEMENTED;
}

C86CTL_API INT c86ctl_out2(UINT module, UINT addr, UCHAR adata )
{
	return C86CTL_ERR_NOT_IMPLEMENTED;
}

C86CTL_API INT c86ctl_in2(UINT module, UINT addr, UCHAR *data )
{
	return C86CTL_ERR_NOT_IMPLEMENTED;
}

C86CTL_API INT c86ctl_set_pll_clock(UINT module, UINT clock )
{
	return C86CTL_ERR_NOT_IMPLEMENTED;
}

C86CTL_API INT c86ctl_set_volume(UINT module, UINT ch, UINT vol )
{
	return C86CTL_ERR_NOT_IMPLEMENTED;
}
#endif