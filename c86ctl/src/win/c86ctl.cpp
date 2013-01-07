/***
	c86ctl
	
	Copyright (c) 2009-2012, honet. All rights reserved.
	This software is licensed under the BSD license.

	honet.kk(at)gmail.com
 */
#include "stdafx.h"
#include "c86ctlmain.h"
#include "config.h"

#ifdef _DEBUG
#define new new(_NORMAL_BLOCK,__FILE__,__LINE__)
#endif

#ifdef _MANAGED
#pragma managed(push, off)
#endif

// ------------------------------------------------------------------
// DLL Main
BOOL APIENTRY DllMain(
	HMODULE hModule,
	DWORD  ul_reason_for_call,
	LPVOID lpReserved
	)
{
	switch (ul_reason_for_call){
	case DLL_PROCESS_ATTACH:
		c86ctl::GetC86CtlMain()->init(hModule);
//		gModule = hModule;
		gConfig.init(hModule);
		break;
	case DLL_PROCESS_DETACH:
		//gModule = NULL;
		c86ctl::GetC86CtlMain()->deinit();
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


// ----------------------------------------------------------------------
// �O���C���^�[�t�F�C�X
HRESULT WINAPI CreateInstance( REFIID riid, void** ppi )
{
	// C86Ctl���P��C���X�^���X�Ȃ̂Ŏ蔲�������B
	return c86ctl::GetC86CtlMain()->QueryInterface(riid,ppi);
}

// ---------------------------------------------------
// �ȉ��͌���݊��̂��߂�native I/F
int WINAPI c86ctl_initialize(void)
{
	return c86ctl::GetC86CtlMain()->initialize();
}

int WINAPI c86ctl_deinitialize(void)
{
	return c86ctl::GetC86CtlMain()->deinitialize();
}

int WINAPI c86ctl_reset(void)
{
	return c86ctl::GetC86CtlMain()->reset();
}

void WINAPI c86ctl_out( UINT addr, UCHAR data )
{
	c86ctl::GetC86CtlMain()->out( addr, data );
}

UCHAR WINAPI c86ctl_in( UINT addr )
{
	return c86ctl::GetC86CtlMain()->in( addr );
}

