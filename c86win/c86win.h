
// c86win.h : PROJECT_NAME �A�v���P�[�V�����̃��C�� �w�b�_�[ �t�@�C���ł��B
//

#pragma once

#include "c86ctl.h"

#ifndef __AFXWIN_H__
	#error "PCH �ɑ΂��Ă��̃t�@�C�����C���N���[�h����O�� 'stdafx.h' ���C���N���[�h���Ă�������"
#endif

#include "resource.h"		// ���C�� �V���{��


// C86winApp:
// ���̃N���X�̎����ɂ��ẮAc86win.cpp ���Q�Ƃ��Ă��������B
//

class C86winApp : public CWinApp
{
public:
	C86winApp();

// �I�[�o�[���C�h
public:
	virtual BOOL InitInstance();

// ����
public:
	HMODULE hC86DLL;
	IRealChipBase *pChipBase;
	
	DECLARE_MESSAGE_MAP()
};

extern C86winApp theApp;

