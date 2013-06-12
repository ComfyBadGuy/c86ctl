﻿/***
	c86ctl
	
	Copyright (c) 2009-2012, honet. All rights reserved.
	This software is licensed under the BSD license.

	honet.kk(at)gmail.com
 */
#include "stdafx.h"
#include <stdlib.h>
#include <stdio.h>
#include <tchar.h>
#include "config.h"
#include <shlobj.h>
#include <Shlwapi.h>

#ifdef _DEBUG
#define new new(_NORMAL_BLOCK,__FILE__,__LINE__)
#endif

using namespace c86ctl;

class c86ctl::CC86CtlConfig gConfig;

BOOL CC86CtlConfig::writeInt( LPCTSTR section, LPCTSTR key, INT val )
{
	TCHAR cfg[128];
	_stprintf_s( cfg, TEXT("%d"), val );
	return ::WritePrivateProfileString( section, key, cfg, inipath );
}


void CC86CtlConfig::init(HMODULE hModule)
{
	// TODO: UAC対応する --- ポリシー実験中
#if 0
	//---- iniファイル名設定 ---------

	// Roamingパスをまず調べる
	TCHAR szPath[_MAX_PATH];
	::SHGetFolderPath(NULL, CSIDL_APPDATA, NULL, 0, szPath);
	_tcsncat_s(szPath, _countof(szPath), _T("\\c86ctl\\c86ctl.ini"), _TRUNCATE);

	// Roamingパスに無ければside-by-sideパスに。
	if ( ::PathFileExists(szPath) ){
		_tcscpy( inipath, szPath );
	}else{
		TCHAR modulePath[_MAX_PATH];
		TCHAR drv[_MAX_PATH], dir[_MAX_PATH], fname[_MAX_PATH], ext[_MAX_PATH];

		::GetModuleFileName( hModule, modulePath, _MAX_PATH );
		_tsplitpath( modulePath, drv, dir, fname, ext );
		_tcsncat( inipath, drv, _MAX_PATH );
		_tcsncat( inipath, dir, _MAX_PATH );
		_tcsncat( inipath, TEXT("c86ctl.ini"), _MAX_PATH );
	}
#else
	TCHAR modulePath[_MAX_PATH];
	TCHAR drv[_MAX_PATH], dir[_MAX_PATH], fname[_MAX_PATH], ext[_MAX_PATH];

	::GetModuleFileName( hModule, modulePath, _MAX_PATH );
	_tsplitpath( modulePath, drv, dir, fname, ext );
	_tcsncat( inipath, drv, _MAX_PATH );
	_tcsncat( inipath, dir, _MAX_PATH );
	_tcsncat( inipath, TEXT("c86ctl.ini"), _MAX_PATH );
#endif
};