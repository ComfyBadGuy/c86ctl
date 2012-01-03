/***
	c86ctl
	
	Copyright (c) 2009-2010, honet. All rights reserved.
	This software is licensed under the BSD license.

	honet.kk(at)gmail.com


	note: honet.kk
	ini�t�@�C���x�[�X�̎d�g�݂Ŏ������Ă���̂�
	�P�Ƀe�L�X�g�G�f�B�^�ŕҏW�o����t�@�C���ɐݒ��
	�ۑ��������Ƃ�����҂̎�Ȃ̂ő債���Ӗ��͂���܂���B
 */
#pragma once

#include <tchar.h>

//#define INIFILE			TEXT(".\\c86ctl.ini")
#define INISC_MAIN		TEXT("c86ctl")
#define INISC_KEY		TEXT("c86key")
#define INISC_REG		TEXT("c86reg")
#define INISC_FMx		TEXT("c86fm%d")

#define INIKEY_WNDLEFT		TEXT("wndleft")
#define INIKEY_WNDTOP		TEXT("wndtop")
#define INIKEY_WNDVISIBLE	TEXT("wndvisible")
#define INIKEY_MIDIDEVICE	TEXT("mididevice")
#define INIKEY_HIDDEVICE	TEXT("hiddevice")
#define INIKEY_GIMICIFTYPE	TEXT("gimic_iftype")


class CC86CtlConfig
{
protected:
	TCHAR inipath[_MAX_PATH];
	
public:
	void init(HMODULE hModule){
		TCHAR modulePath[_MAX_PATH];
		TCHAR drv[_MAX_PATH], dir[_MAX_PATH], fname[_MAX_PATH], ext[_MAX_PATH];

		// ini�t�@�C�����擾
		::GetModuleFileName( hModule, modulePath, _MAX_PATH );
		_tsplitpath( modulePath, drv, dir, fname, ext );
		_tcsncat( inipath, drv, _MAX_PATH );
		_tcsncat( inipath, dir, _MAX_PATH );
		_tcsncat( inipath, TEXT("c86ctl.ini"), _MAX_PATH );
	};
	
	UINT get( LPCTSTR section, LPCTSTR key, LPCTSTR defstr, LPTSTR val, UINT sz_val ){
		return GetPrivateProfileString( section, key, defstr, val, sz_val, inipath );
	};
	
	BOOL write( LPCTSTR section, LPCTSTR key, LPCTSTR val ){
		return WritePrivateProfileString( section, key, val, inipath );
	};
	
	int getInt( LPCTSTR section, LPCTSTR key, int defval ){
		return GetPrivateProfileInt( section, key, defval, inipath );
	};
	
	BOOL writeInt( LPCTSTR section, LPCTSTR key, INT val );
};


extern class CC86CtlConfig gConfig;

