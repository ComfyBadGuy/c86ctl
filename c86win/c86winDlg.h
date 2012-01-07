
// c86winDlg.h : �w�b�_�[ �t�@�C��
//

#pragma once
#include "CS98Data.h"
#include "afxwin.h"

// C86winDlg �_�C�A���O
class C86winDlg : public CDialogEx
{
// �R���X�g���N�V����
public:
	C86winDlg(CWnd* pParent = NULL);	// �W���R���X�g���N�^�[

// �_�C�A���O �f�[�^
	enum { IDD = IDD_C86WIN_DIALOG };

	protected:
	virtual void DoDataExchange(CDataExchange* pDX);	// DDX/DDV �T�|�[�g


// ����
protected:
	CS98Data s98data;

protected:
	HICON m_hIcon;

	// �������ꂽ�A���b�Z�[�W���蓖�Ċ֐�
	virtual BOOL OnInitDialog();
	afx_msg void OnSysCommand(UINT nID, LPARAM lParam);
	afx_msg void OnPaint();
	afx_msg HCURSOR OnQueryDragIcon();
	DECLARE_MESSAGE_MAP()
public:
	afx_msg void OnBnClickedOk();
	afx_msg void OnBnClickedCancel();
	afx_msg void OnBnClickedButtonPlay();
	afx_msg void OnBnClickedButtonStop();
	afx_msg void OnBnClickedButtonOpen();

public:
	static unsigned int WINAPI PlayerThread(LPVOID param);
	static bool terminateFlag;
	UINT threadID;
	HANDLE hThread;

	UINT m_tick;

	CEdit m_editFilePath;
	CStatic m_staticTick;
	CEdit m_editMessage;
	afx_msg void OnTimer(UINT_PTR nIDEvent);
	afx_msg void OnBnClickedButtonInitialize();
	afx_msg void OnBnClickedButtonDeinitialize();
	afx_msg void OnBnClickedButtonSetSSGVol();
	afx_msg void OnBnClickedButtonSetPllClock();
	UINT m_ssgVol;
	UINT m_pllClock;
	afx_msg void OnBnClickedButtonMbinfo();
	afx_msg void OnBnClickedButtonModuleinfo();
	afx_msg void OnBnClickedButtonGetSsgvol();
	afx_msg void OnBnClickedButtonGetPllclock();
};
