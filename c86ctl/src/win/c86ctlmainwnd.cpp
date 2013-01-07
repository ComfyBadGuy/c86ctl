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

//#include "module.h"
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
#define WM_THREADEXIT       (WM_APP+10)
#define WM_TASKTRAY_EVENT   (WM_APP+11)


C86CtlMainWnd *C86CtlMainWnd::pthis = 0;

int C86CtlMainWnd::createMainWnd(LPVOID param)
{
	const TCHAR szAppName[] = _T("msg-receiver");
	WNDCLASSEX  wndclass;
	NOTIFYICONDATA notifyIcon;

	HANDLE hNotifyDevNode;
	HINSTANCE hinst = C86CtlMain::getInstanceHandle();

	// ���b�Z�[�W�����p�E�B���h�E����
	wndclass.cbSize        = sizeof(wndclass);
	wndclass.style         = CS_HREDRAW | CS_VREDRAW;
	wndclass.lpfnWndProc   = wndProc;
	wndclass.cbClsExtra    = 0;
	wndclass.cbWndExtra    = 0;
	wndclass.hInstance     = hinst;
	wndclass.hIcon         = LoadIcon(NULL, MAKEINTRESOURCE(IDI_ICON_C86CTL));
	wndclass.hCursor       = LoadCursor(NULL, IDC_ARROW);
	wndclass.hbrBackground = (HBRUSH)GetStockObject(BLACK_BRUSH);
	wndclass.lpszMenuName  = NULL;
	wndclass.lpszClassName = szAppName;
	wndclass.hIconSm       = LoadIcon (NULL, IDI_APPLICATION);

	if( !RegisterClassEx(&wndclass) )
		return -1;

	hwnd = ::CreateWindowEx(
		0, szAppName, NULL, WS_OVERLAPPEDWINDOW,
		CW_USEDEFAULT, CW_USEDEFAULT,CW_USEDEFAULT, CW_USEDEFAULT,
		HWND_MESSAGE, NULL, hinst, NULL);
	
	if(!hwnd)
		return -1;

	// �^�X�N�g���C�A�C�R���̓o�^
	notifyIcon.cbSize = sizeof(NOTIFYICONDATA);
	notifyIcon.uID = 0;
	notifyIcon.hWnd = hwnd;
	notifyIcon.uFlags = NIF_ICON | NIF_MESSAGE | NIF_TIP;
	notifyIcon.hIcon = ::LoadIcon(hinst, MAKEINTRESOURCE(IDI_ICON_C86CTL));
	notifyIcon.uCallbackMessage = WM_TASKTRAY_EVENT;
	lstrcpy( notifyIcon.szTip, _T("C86CTL") );
	::Shell_NotifyIcon( NIM_ADD, &notifyIcon );

	// �f�o�C�X�}���Ď��o�^
	DEV_BROADCAST_DEVICEINTERFACE *pFilterData = (DEV_BROADCAST_DEVICEINTERFACE*)_alloca(sizeof(DEV_BROADCAST_DEVICEINTERFACE));
	if( pFilterData ){
		ZeroMemory(pFilterData, sizeof(DEV_BROADCAST_DEVICEINTERFACE));

		pFilterData->dbcc_size = sizeof(DEV_BROADCAST_DEVICEINTERFACE);
		pFilterData->dbcc_devicetype = DBT_DEVTYP_DEVICEINTERFACE;
		HidD_GetHidGuid(&pFilterData->dbcc_classguid);

		hNotifyDevNode = RegisterDeviceNotification(hwnd, pFilterData, DEVICE_NOTIFY_WINDOW_HANDLE);
	}

	return 0;
}


int C86CtlMainWnd::destroyMainWnd(LPVOID param)
{
	if(hwnd){
		::DestroyWindow(hwnd);
		::Shell_NotifyIcon( NIM_DELETE, &notifyIcon );
	}
	
	return 0;
}


LRESULT CALLBACK C86CtlMainWnd::wndProc(HWND hwnd, UINT iMsg, WPARAM wParam, LPARAM lParam)
{
	static UINT taskbarRestartMsg=0;
	C86CtlMainWnd *pThis = C86CtlMainWnd::getInstance();

	switch(iMsg){
	case WM_CREATE:
		// �^�X�N�g���C�A�C�R���̗v�ēo�^�ʒm�p
		taskbarRestartMsg = ::RegisterWindowMessage(_T("TaskbarCreated"));

		if( gConfig.getInt(INISC_MAIN, _T("GUI"), 1) )
			pThis->startVis();
		break;

	case WM_DESTROY:
		pThis->stopVis();
		break;
		
	case WM_DEVICECHANGE:
		OutputDebugString(_T("CAHNGE\r\n"));
		break;

	case WM_TASKTRAY_EVENT:
		{
			POINT point;
			GetCursorPos(&point);

			if( lParam == WM_RBUTTONDOWN ){
				::SetForegroundWindow(hwnd);

				HMENU hMenu = ::LoadMenu(C86CtlMain::getInstanceHandle(), MAKEINTRESOURCE(IDR_MENU_SYSPOPUP));
				if( !hMenu )
					break;
				HMENU hSubMenu = ::GetSubMenu(hMenu, 0);
				if( !hSubMenu ){
					::DestroyMenu(hMenu);
					break;
				}
				TrackPopupMenu(hSubMenu, TPM_LEFTALIGN, point.x, point.y, 0, hwnd, NULL);
				::DestroyMenu(hMenu);
			}
		}
		break;

	case WM_COMMAND:
		switch(LOWORD(wParam)){
		case ID_POPUP_CONFIG:
			break;
		case ID_POPUP_SHOWVIS:
			if( pThis->mainVisWnd ){
				pThis->stopVis();
			}else{
				pThis->startVis();
			}
			break;
		}

	default:
		if( iMsg == taskbarRestartMsg ){
			// �^�X�N�g���C�A�C�R���̍ēo�^
			::Shell_NotifyIcon( NIM_ADD, &pThis->notifyIcon );

		}else{
			return DefWindowProc(hwnd, iMsg, wParam, lParam);
		}
	}
	return 0;
}


int C86CtlMainWnd::startVis()
{
	wm = new CVisManager();
	mainVisWnd = new CVisC86Main();

	mainVisWnd->attach( GetC86CtlMain()->getGimics() );
	wm->add( mainVisWnd );
	mainVisWnd->create();

	// �`��X���b�h�J�n
	hVisThread = (HANDLE)_beginthreadex( NULL, 0, &threadVis, wm, 0, &visThreadID );
	if( !hVisThread ){
	}
	return 0;
}

int C86CtlMainWnd::stopVis()
{
	// �`��X���b�h�I��
	if( hVisThread ){
		::PostThreadMessage( visThreadID, WM_THREADEXIT, 0, 0 );
		::WaitForSingleObject( hVisThread, INFINITE );

		hVisThread = NULL;
		visThreadID = 0;
	}
	
	if( mainVisWnd && wm ){
		wm->del( mainVisWnd );
		mainVisWnd->close();

		delete mainVisWnd;
		delete wm;
		mainVisWnd = 0;
		wm = 0;
	}
	return 0;
}


// ---------------------------------------------------------
// �`�揈���X���b�h
// mm-timer�ɂ��60fps����
unsigned int WINAPI C86CtlMainWnd::threadVis(LPVOID param)
{
	MSG msg;

	ZeroMemory(&msg, sizeof(msg));
	::PeekMessage(&msg, NULL, 0, 0, PM_NOREMOVE);

	CVisManager *pwm = reinterpret_cast<CVisManager*>(param);

	DWORD next = ::timeGetTime()*6 + 100;
	while(1){
		// message proc
		if( ::PeekMessage(&msg , NULL , 0 , 0, PM_REMOVE )) {
			if( msg.message == WM_THREADEXIT )
				break;
		}else{
			// fps management
			DWORD now = ::timeGetTime() * 6;
			if(now < next){
				Sleep(1);
				continue;
			}
			next += 100;
			if( next < now ){
				//next = now;
				while(next<now) next += 100;
			}

			//update
			pwm->draw();
		}
	}
	
	return 0;
}

