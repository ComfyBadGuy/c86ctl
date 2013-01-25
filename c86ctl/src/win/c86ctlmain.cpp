/***
	c86ctl
	
	Copyright (c) 2009-2012, honet. All rights reserved.
	This software is licensed under the BSD license.

	honet.kk(at)gmail.com
 */
#include "stdafx.h"

#include <stdlib.h>
#include <stdio.h>
#include <tchar.h>
#include <assert.h>
#include <process.h>
#include <mmsystem.h>
#include <WinUser.h>
#include <Dbt.h>
#include <ShellAPI.h>

#include <string>
#include <list>
#include <vector>

extern "C" {
#include "hidsdi.h"
}

#include "resource.h"
#include "c86ctl.h"
#include "c86ctlmain.h"
#include "c86ctlmainwnd.h"

#include "config.h"
#include "vis/vis_c86main.h"
#include "ringbuff.h"
#include "interface/if.h"
#include "interface/if_gimic_hid.h"
#include "interface/if_gimic_midi.h"


#pragma comment(lib,"hidclass.lib")

#ifdef _DEBUG
#define new new(_NORMAL_BLOCK,__FILE__,__LINE__)
#endif

using namespace c86ctl;
using namespace c86ctl::vis;


// ------------------------------------------------------------------
// �O���[�o���ϐ�
C86CtlMain theC86CtlMain;


HINSTANCE C86CtlMain::hInstance = 0;
ULONG_PTR C86CtlMain::gdiToken = 0;
Gdiplus::GdiplusStartupInput C86CtlMain::gdiInput;

// ------------------------------------------------------------------
// imprement

C86CtlMain* c86ctl::GetC86CtlMain(void)
{
	return &theC86CtlMain;
}

INT C86CtlMain::init(HINSTANCE h)
{
	hInstance = h;
	return 0;
}
INT C86CtlMain::deinit(void)
{
	hInstance = NULL;
	return 0;
}

HINSTANCE C86CtlMain::getInstanceHandle()
{
	return hInstance;
}

withlock< std::vector< std::shared_ptr<GimicIF> > > &C86CtlMain::getGimics()
{
	return gGIMIC;
}

// ---------------------------------------------------------
// UI���b�Z�[�W�����X���b�h
unsigned int WINAPI C86CtlMain::threadMain(LPVOID param)
{
	C86CtlMain *pThis = reinterpret_cast<C86CtlMain*>(param);
	MSG msg;
	BOOL b;

	ZeroMemory(&msg, sizeof(msg));
	::PeekMessage(&msg, NULL, 0, 0, PM_NOREMOVE);

	try{
		if( Gdiplus::Ok != Gdiplus::GdiplusStartup(&gdiToken, &gdiInput, NULL) )
			throw "failed to initialize GDI+";

		C86CtlMainWnd *pwnd = C86CtlMainWnd::getInstance();

		pwnd->createMainWnd(param);
		pThis->mainThreadReady = true;

		// ���b�Z�[�W���[�v
		while( (b = ::GetMessage(&msg, NULL, 0, 0)) ){
			if( b==-1 ) break;
			switch( msg.message ){
			case WM_THREADEXIT:
				pwnd->destroyMainWnd(param);
				break;

			case WM_MYDEVCHANGE:
				::OutputDebugString(L"YEAH!\r\n");
				GimicHID::UpdateInstances(pThis->gGIMIC); // NOTE: �ǉ��������Ȃ��B
				pwnd->deviceUpdate();
				break;
			}


			::TranslateMessage(&msg);
			::DispatchMessage(&msg);
		}

		C86CtlMainWnd::shutdown();
		Gdiplus::GdiplusShutdown(gdiToken);

		pThis->mainThreadReady = false;
	}
	catch(...){
		::OutputDebugString(_T("ERROR\r\n"));
	}
	
	return (DWORD)msg.wParam;
}



// ---------------------------------------------------------
// ���t�����X���b�h
// mm-timer�ɂ��(��������)1ms�P�ʏ���
// note: timeSetEvent()���Ɠ]���������^�C�}�������x���Ƃ���
//       �ē������̂��|�������̂Ŏ��O���[�v�ɂ���
unsigned int WINAPI C86CtlMain::threadSender(LPVOID param)
{
	try{
		const UINT period = 1;
		UINT now = ::timeGetTime();
		UINT next = now + period;
		UINT nextSec10 = now + 50;
		C86CtlMain *pThis = reinterpret_cast<C86CtlMain*>(param);

		pThis->senderThreadReady = true;

		while(1){
			if( pThis->terminateFlag )
				break;
		
			now = ::timeGetTime();
			if(now < next){
				if( pThis->terminateFlag ) break;
				Sleep(1);
				continue;
			}
			next += period;

			// �����Ń��[�v���T�C�Y�m��B
			// �ʃX���b�h�ŃT�C�Y�g������鎖������̂Œ��ӁB
			size_t sz = pThis->gGIMIC.size(); 

			// update
			for( size_t i=0; i<sz; i++ ){ pThis->gGIMIC[i]->tick(); };
			
			if( nextSec10 < now ){
				nextSec10 += 50;
				for( size_t i=0; i<sz; i++ ){ pThis->gGIMIC[i]->update(); };
			}
		}

		pThis->senderThreadReady = false;
	}catch(...){
	}
	
	return 0;
}


// ---------------------------------------------------------
HRESULT C86CtlMain::QueryInterface( REFIID riid, LPVOID *ppvObj )
{
	if( ::IsEqualGUID( riid, IID_IRealChipBase ) ){
		*ppvObj = (LPVOID)this;
		return NOERROR;
	}
	*ppvObj = 0;
	return E_NOINTERFACE;
}

ULONG C86CtlMain::AddRef(VOID)
{
	return ++refCount;
}

ULONG C86CtlMain::Release(VOID)
{
	return --refCount;
}


int C86CtlMain::initialize(void)
{
	if( isInitialized )
		return C86CTL_ERR_UNKNOWN;
	
	// �C���X�^���X����
	int type = gConfig.getInt(INISC_MAIN, INIKEY_GIMICIFTYPE, 0);
	if( type==0 ){
		//gGIMIC = GimicHID::CreateInstances();
		GimicHID::UpdateInstances(gGIMIC);
	}else if( type==1 ){
		//gGIMIC = GimicMIDI::CreateInstances(); // TODO!!
	}
	
	// �^�C�}����\�ݒ�
	TIMECAPS timeCaps;
	if( ::timeGetDevCaps(&timeCaps, sizeof(timeCaps)) == TIMERR_NOERROR ){
		::timeBeginPeriod(timeCaps.wPeriodMin);
		timerPeriod = timeCaps.wPeriodMin;
	}

	// �`��/UI�X���b�h�J�n
	hMainThread = (HANDLE)_beginthreadex( NULL, 0, &threadMain, this, 0, &mainThreadID );
	if( !hMainThread )
		return C86CTL_ERR_UNKNOWN;

	while(!mainThreadReady);

	// ���t�X���b�h�J�n
	hSenderThread = (HANDLE)_beginthreadex( NULL, 0, &threadSender, this, 0, &senderThreadID );
	if( !hSenderThread ){
		::PostThreadMessage( mainThreadID, WM_THREADEXIT, 0, 0 );
		::WaitForSingleObject( hMainThread, INFINITE );
		return C86CTL_ERR_UNKNOWN;
	}
	while(!senderThreadReady);

	SetThreadPriority( hSenderThread, THREAD_PRIORITY_ABOVE_NORMAL );

	isInitialized = true;
	return C86CTL_ERR_NONE;
}


int C86CtlMain::deinitialize(void)
{
	if( !isInitialized )
		return C86CTL_ERR_UNKNOWN;

	reset();

	// �e��X���b�h�I��

	if( hMainThread ){
		::PostThreadMessage( mainThreadID, WM_THREADEXIT, 0, 0 );
		::WaitForSingleObject( hMainThread, INFINITE );
		hMainThread = NULL;
		mainThreadID = 0;
	}

	if( hSenderThread ){
		terminateFlag = true;
		::WaitForSingleObject( hSenderThread, INFINITE );
		hSenderThread = NULL;
		senderThreadID = 0;
	}
	terminateFlag = false;

	// �C���X�^���X�폜
	// note: ���̃^�C�~���O�ŏI���������s����B
	//       gGIMIC���Q�Ƃ��鉉�t�E�`��X���b�h�͏I�����Ă��Ȃ���΂Ȃ�Ȃ��B
	gGIMIC.clear();

	// �^�C�}����\�ݒ����
	::timeEndPeriod(timerPeriod);
	isInitialized = false;
	
	mainThreadReady = false;
	senderThreadReady = false;

	return C86CTL_ERR_NONE;
}

int C86CtlMain::reset(void)
{
	gGIMIC.lock();
	std::for_each( gGIMIC.begin(), gGIMIC.end(), [](std::shared_ptr<GimicIF> x){ x->reset(); } );
	gGIMIC.unlock();

	return 0;
}

int C86CtlMain::getNumberOfChip(void)
{
	return static_cast<int>(gGIMIC.size());
}

HRESULT C86CtlMain::getChipInterface( int id, REFIID riid, void** ppi )
{
	if( id < gGIMIC.size() ){
		return gGIMIC[id]->QueryInterface( riid, ppi );
	}
	return E_NOINTERFACE;
}

void C86CtlMain::out( UINT addr, UCHAR data )
{
	if( gGIMIC.size() ){
		gGIMIC.front()->out(addr,data);
	}
}

UCHAR C86CtlMain::in( UINT addr )
{
	if( gGIMIC.size() ){
		return gGIMIC.front()->in(addr);
	}else
		return 0;
}



