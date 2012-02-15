
// c86winDlg.cpp : �����t�@�C��
//

#include "stdafx.h"
#include "c86win.h"
#include "c86winDlg.h"
#include "afxdialogex.h"
#include "c86ctl.h"
#include <MMSystem.h>


#pragma comment(lib, "winmm.lib")

#ifdef _DEBUG
#define new DEBUG_NEW
#endif


// �A�v���P�[�V�����̃o�[�W�������Ɏg���� CAboutDlg �_�C�A���O

class CAboutDlg : public CDialogEx
{
public:
	CAboutDlg();

// �_�C�A���O �f�[�^
	enum { IDD = IDD_ABOUTBOX };

	protected:
	virtual void DoDataExchange(CDataExchange* pDX);    // DDX/DDV �T�|�[�g

// ����
protected:
	DECLARE_MESSAGE_MAP()
};

CAboutDlg::CAboutDlg() : CDialogEx(CAboutDlg::IDD)
{
}

void CAboutDlg::DoDataExchange(CDataExchange* pDX)
{
	CDialogEx::DoDataExchange(pDX);
}

BEGIN_MESSAGE_MAP(CAboutDlg, CDialogEx)
END_MESSAGE_MAP()


// C86winDlg �_�C�A���O




C86winDlg::C86winDlg(CWnd* pParent /*=NULL*/)
	: CDialogEx(C86winDlg::IDD, pParent)
	, m_ssgVol(255)
	, m_pllClock(8000000)
{
	m_hIcon = AfxGetApp()->LoadIcon(IDR_MAINFRAME);
	hThread = NULL;
	threadID = 0;
	terminateFlag = 0;
}

void C86winDlg::DoDataExchange(CDataExchange* pDX)
{
	CDialogEx::DoDataExchange(pDX);
	DDX_Control(pDX, IDC_EDIT_FILENAME, m_editFilePath);
	DDX_Control(pDX, IDC_STATIC_TICK, m_staticTick);
	DDX_Control(pDX, IDC_EDIT_MESSAGE, m_editMessage);
	DDX_Text(pDX, IDC_EDIT_SSGVOL, m_ssgVol);
	DDX_Text(pDX, IDC_EDIT_PLLCLOCK, m_pllClock);
}

BEGIN_MESSAGE_MAP(C86winDlg, CDialogEx)
	ON_WM_SYSCOMMAND()
	ON_WM_PAINT()
	ON_WM_QUERYDRAGICON()
	ON_BN_CLICKED(IDOK, &C86winDlg::OnBnClickedOk)
	ON_BN_CLICKED(IDCANCEL, &C86winDlg::OnBnClickedCancel)
	ON_BN_CLICKED(IDC_BUTTON_PLAY, &C86winDlg::OnBnClickedButtonPlay)
	ON_BN_CLICKED(IDC_BUTTON_STOP, &C86winDlg::OnBnClickedButtonStop)
	ON_BN_CLICKED(IDC_BUTTON_OPEN, &C86winDlg::OnBnClickedButtonOpen)
	ON_WM_TIMER()
	ON_BN_CLICKED(IDC_BUTTON_INITIALIZE, &C86winDlg::OnBnClickedButtonInitialize)
	ON_BN_CLICKED(IDC_BUTTON_DEINITIALIZE, &C86winDlg::OnBnClickedButtonDeinitialize)
	ON_BN_CLICKED(IDC_BUTTON_SSGVOL, &C86winDlg::OnBnClickedButtonSetSSGVol)
	ON_BN_CLICKED(IDC_BUTTON_PLLCLOCK, &C86winDlg::OnBnClickedButtonSetPllClock)
	ON_BN_CLICKED(IDC_BUTTON_MBINFO, &C86winDlg::OnBnClickedButtonMbinfo)
	ON_BN_CLICKED(IDC_BUTTON_MODULEINFO, &C86winDlg::OnBnClickedButtonModuleinfo)
	ON_BN_CLICKED(IDC_BUTTON_GET_SSGVOL, &C86winDlg::OnBnClickedButtonGetSsgvol)
	ON_BN_CLICKED(IDC_BUTTON_GET_PLLCLOCK, &C86winDlg::OnBnClickedButtonGetPllclock)
	ON_WM_DESTROY()
	ON_BN_CLICKED(IDC_BUTTON_GET_FWVER, &C86winDlg::OnBnClickedButtonGetFwver)
	ON_BN_CLICKED(IDC_BUTTON_TEST1, &C86winDlg::OnBnClickedButtonTest1)
	ON_BN_CLICKED(IDC_BUTTON_ADPCM_ZERORESET, &C86winDlg::OnBnClickedButtonAdpcmZeroreset)
END_MESSAGE_MAP()


// C86winDlg ���b�Z�[�W �n���h���[

BOOL C86winDlg::OnInitDialog()
{
	CDialogEx::OnInitDialog();

	// "�o�[�W�������..." ���j���[���V�X�e�� ���j���[�ɒǉ����܂��B

	// IDM_ABOUTBOX �́A�V�X�e�� �R�}���h�͈͓̔��ɂȂ���΂Ȃ�܂���B
	ASSERT((IDM_ABOUTBOX & 0xFFF0) == IDM_ABOUTBOX);
	ASSERT(IDM_ABOUTBOX < 0xF000);

	CMenu* pSysMenu = GetSystemMenu(FALSE);
	if (pSysMenu != NULL)
	{
		BOOL bNameValid;
		CString strAboutMenu;
		bNameValid = strAboutMenu.LoadString(IDS_ABOUTBOX);
		ASSERT(bNameValid);
		if (!strAboutMenu.IsEmpty())
		{
			pSysMenu->AppendMenu(MF_SEPARATOR);
			pSysMenu->AppendMenu(MF_STRING, IDM_ABOUTBOX, strAboutMenu);
		}
	}

	// ���̃_�C�A���O�̃A�C�R����ݒ肵�܂��B�A�v���P�[�V�����̃��C�� �E�B���h�E���_�C�A���O�łȂ��ꍇ�A
	//  Framework �́A���̐ݒ�������I�ɍs���܂��B
	SetIcon(m_hIcon, TRUE);			// �傫���A�C�R���̐ݒ�
	SetIcon(m_hIcon, FALSE);		// �������A�C�R���̐ݒ�


	// -----------------------------------------------------------------
	C86winApp *pApp = (C86winApp*)AfxGetApp();
	pApp->pChipBase->initialize();
	//pApp->pChipBase->deinitialize();
	// -----------------------------------------------------------------
	UpdateData(FALSE);


	return TRUE;  // �t�H�[�J�X���R���g���[���ɐݒ肵���ꍇ�������ATRUE ��Ԃ��܂��B
}

void C86winDlg::OnSysCommand(UINT nID, LPARAM lParam)
{
	if ((nID & 0xFFF0) == IDM_ABOUTBOX)
	{
		CAboutDlg dlgAbout;
		dlgAbout.DoModal();
	}
	else
	{
		CDialogEx::OnSysCommand(nID, lParam);
	}
}

// �_�C�A���O�ɍŏ����{�^����ǉ�����ꍇ�A�A�C�R����`�悷�邽�߂�
//  ���̃R�[�h���K�v�ł��B�h�L�������g/�r���[ ���f�����g�� MFC �A�v���P�[�V�����̏ꍇ�A
//  ����́AFramework �ɂ���Ď����I�ɐݒ肳��܂��B

void C86winDlg::OnPaint()
{
	if (IsIconic())
	{
		CPaintDC dc(this); // �`��̃f�o�C�X �R���e�L�X�g

		SendMessage(WM_ICONERASEBKGND, reinterpret_cast<WPARAM>(dc.GetSafeHdc()), 0);

		// �N���C�A���g�̎l�p�`�̈���̒���
		int cxIcon = GetSystemMetrics(SM_CXICON);
		int cyIcon = GetSystemMetrics(SM_CYICON);
		CRect rect;
		GetClientRect(&rect);
		int x = (rect.Width() - cxIcon + 1) / 2;
		int y = (rect.Height() - cyIcon + 1) / 2;

		// �A�C�R���̕`��
		dc.DrawIcon(x, y, m_hIcon);
	}
	else
	{
		CDialogEx::OnPaint();
	}
}

// ���[�U�[���ŏ��������E�B���h�E���h���b�O���Ă���Ƃ��ɕ\������J�[�\�����擾���邽�߂ɁA
//  �V�X�e�������̊֐����Ăяo���܂��B
HCURSOR C86winDlg::OnQueryDragIcon()
{
	return static_cast<HCURSOR>(m_hIcon);
}


bool C86winDlg::terminateFlag;

// ���t�����X���b�h
unsigned int WINAPI C86winDlg::PlayerThread(LPVOID param)
{
	C86winDlg *pThis = reinterpret_cast<C86winDlg*>(param);
	DWORD next;
	next = ::timeGetTime()*1000;
	UINT tick = 0;
	UINT idx = 0;
	UINT delay = 0;
	CString str;
	UINT tpus; // tick per micro second
	UINT last_is_adpcm=0;

	IRealChip *pRC = NULL;

	// -----------------------------------------------------------------
	// IRealChip�擾
	C86winApp *pApp = (C86winApp*)AfxGetApp();
	int nchip = pApp->pChipBase->getNumberOfChip();
	if( 0<nchip ){
		pApp->pChipBase->getChipInterface( 0, IID_IRealChip, (void**)&pRC );
	}
	pRC->reset();
	// -----------------------------------------------------------------

	tpus = (INT)(pThis->s98data.getTimerPrec() * 1000.0);
	if(tpus==0) tpus = 1;
	while(1){
		if( terminateFlag )
			break;
		
		DWORD now = ::timeGetTime()*1000;
		if(now < next){
			Sleep(1);
			continue;
		}
		if( next+(tpus*10) < now ){ // �]�����x���ĊԂɍ���Ȃ��ꍇ�̃X�L�b�v����
			next = now + tpus;
			delay = 1;
		}else{
			next += tpus;
			delay = 0;
		}

		// update
		tick++;
		pThis->m_tick = tick;
		// ������CEdit�M��ƂȂ񂩃X���b�h�I�����Ɍł܂�B�B�B�B�������_���Ȃ񂾂����H
//		str.Format( _T("%01d, INDEX:%5d, TICK:%5d, NOW:%05d, NEXT:%05d"), delay, idx, tick, now/1000, next/1000 );
//		pThis->m_staticTick.SetWindowText(str);
//		pThis->m_staticTick.UpdateWindow();

		auto prow = &pThis->s98data.row;
		if( idx < prow->size() ){
			auto pr = &prow->at(idx);
			while( pr->gtick <= tick ){
				if( pr->getDeviceNo() == 0 && pr->len == 3 ){
					UINT addr = pr->data[1];
					if( pr->isExtDevice() ) addr += 0x100;
					UCHAR data = pr->data[2];

#if 0
					// ���O�̓]����adpcm�f�[�^�ŁA����̃f�[�^��adpcm�f�[�^�ł�timer-A/B�Z�b�g�ł������ꍇ��
					// tick�����Z�b�g�B���������΍�ł���Ă݂����ǂ���܂���ʂ����������̂ł�߁B
					if( last_is_adpcm && addr != 0x108 && !(0x24<=addr && addr<=0x27) ){
						next = now+(tpus*1000);
						last_is_adpcm = 0;
						break;
					}
#endif
					//c86ctl_out(addr, data);
					pRC->out( addr, data );
					last_is_adpcm = ( addr == 0x108 );
				}
				if( ++idx >= prow->size() )
					break;
				pr = &prow->at(idx);
			}
		}

	}

	pRC->reset();

	return 0;
}


void C86winDlg::OnBnClickedOk()
{
	// TODO: �����ɃR���g���[���ʒm�n���h���[ �R�[�h��ǉ����܂��B
	CDialogEx::OnOK();
}


void C86winDlg::OnBnClickedCancel()
{
	// TODO: �����ɃR���g���[���ʒm�n���h���[ �R�[�h��ǉ����܂��B
	CDialogEx::OnCancel();
}


void C86winDlg::OnBnClickedButtonPlay()
{
	if( hThread == 0 ){
		C86winApp *pApp = (C86winApp*)AfxGetApp();
		int nchip = pApp->pChipBase->getNumberOfChip();
		if( 0<nchip ){
			terminateFlag = 0;
			hThread = (HANDLE)_beginthreadex( NULL, 0, C86winDlg::PlayerThread, this, 0, &threadID );
			SetThreadPriority(hThread, THREAD_PRIORITY_ABOVE_NORMAL);
		}
	}
	SetTimer(0, 100, NULL);
}


void C86winDlg::OnBnClickedButtonStop()
{
	if( hThread ){
		terminateFlag = 1;
		WaitForSingleObject( hThread, INFINITE );
		hThread = 0;
		threadID = 0;
	}
	KillTimer(0);
}


void C86winDlg::OnBnClickedButtonOpen()
{
	CFileDialog dlg( TRUE, _T("s98"), NULL, NULL, _T("s98 files (*.s98)|*.s98||"), this );

	if( dlg.DoModal() == IDOK ){
		OnBnClickedButtonStop();
		m_editFilePath.SetWindowText( dlg.GetPathName() );
		s98data.loadFile(dlg.GetPathName());
	}
}


void C86winDlg::OnTimer(UINT_PTR nIDEvent)
{
	CString str;
	//str.Format( _T("%01d, INDEX:%5d, TICK:%5d, NOW:%05d, NEXT:%05d"), delay, idx, tick, now/1000, next/1000 );
	str.Format( _T("%d"), m_tick );
	m_staticTick.SetWindowText(str);
	m_staticTick.UpdateWindow();

	CDialogEx::OnTimer(nIDEvent);
}


void C86winDlg::OnBnClickedButtonInitialize()
{
	// TODO: �����ɃR���g���[���ʒm�n���h���[ �R�[�h��ǉ����܂��B
	C86winApp *pApp = (C86winApp*)AfxGetApp();
	pApp->pChipBase->initialize();

}


void C86winDlg::OnBnClickedButtonDeinitialize()
{
	// TODO: �����ɃR���g���[���ʒm�n���h���[ �R�[�h��ǉ����܂��B
	C86winApp *pApp = (C86winApp*)AfxGetApp();
	pApp->pChipBase->deinitialize();
}


void C86winDlg::OnBnClickedButtonSetSSGVol()
{
	// TODO: �����ɃR���g���[���ʒm�n���h���[ �R�[�h��ǉ����܂��B
	UpdateData();
	C86winApp *pApp = (C86winApp*)AfxGetApp();
	IGimic *pGimicModule;
	if( S_OK == pApp->pChipBase->getChipInterface( 0, IID_IGimic, (void**)&pGimicModule ) ){
		pGimicModule->setSSGVolume(m_ssgVol);
		pGimicModule->Release();
	}
}


void C86winDlg::OnBnClickedButtonSetPllClock()
{
	// TODO: �����ɃR���g���[���ʒm�n���h���[ �R�[�h��ǉ����܂��B
	UpdateData();
	C86winApp *pApp = (C86winApp*)AfxGetApp();
	IGimic *pGimicModule;
	if( S_OK == pApp->pChipBase->getChipInterface( 0, IID_IGimic, (void**)&pGimicModule ) ){
		pGimicModule->setPLLClock(m_pllClock);
		pGimicModule->Release();
	}
}


void C86winDlg::OnBnClickedButtonMbinfo()
{
	UpdateData();
	C86winApp *pApp = (C86winApp*)AfxGetApp();
	IGimic *pGimicModule;
	if( S_OK == pApp->pChipBase->getChipInterface( 0, IID_IGimic, (void**)&pGimicModule ) ){
		struct Devinfo info;
		pGimicModule->getMBInfo(&info);

		CString str, devname, rev, serial, dump0, dump1;
		devname = info.Devname;
		rev = info.Rev;
		serial = info.Serial;
		for( int i=0; i<16; i++ ){
			dump1.Format(_T("0x%02x, "), info.Devname[i]);
			dump0 += dump1;
		}

		str = devname + _T("\r\n") + rev + _T("\r\n") + serial + _T("\r\n") + dump0;
		m_editMessage.SetWindowText(str);
		pGimicModule->Release();
	}
}


void C86winDlg::OnBnClickedButtonModuleinfo()
{
	UpdateData();
	C86winApp *pApp = (C86winApp*)AfxGetApp();
	IGimic *pGimicModule;
	if( S_OK == pApp->pChipBase->getChipInterface( 0, IID_IGimic, (void**)&pGimicModule ) ){
		struct Devinfo info;
		pGimicModule->getModuleInfo(&info);

		CString str, devname, rev, serial;
		devname = info.Devname;
		rev = info.Rev;
		serial = info.Serial;
		str = devname + _T("\r\n") + rev + _T("\r\n") + serial;
		m_editMessage.SetWindowText(str);
		pGimicModule->Release();
	}
}


void C86winDlg::OnBnClickedButtonGetSsgvol()
{
	C86winApp *pApp = (C86winApp*)AfxGetApp();
	IGimic *pGimicModule;
	if( S_OK == pApp->pChipBase->getChipInterface( 0, IID_IGimic, (void**)&pGimicModule ) ){
		UCHAR vol;
		pGimicModule->getSSGVolume(&vol);
		m_ssgVol = vol;
		UpdateData(FALSE);
		pGimicModule->Release();
	}
}


void C86winDlg::OnBnClickedButtonGetPllclock()
{
	C86winApp *pApp = (C86winApp*)AfxGetApp();
	IGimic *pGimicModule;
	if( S_OK == pApp->pChipBase->getChipInterface( 0, IID_IGimic, (void**)&pGimicModule ) ){
		UINT clock;
		pGimicModule->getPLLClock(&clock);
		m_pllClock = clock;
		UpdateData(FALSE);
		pGimicModule->Release();
	}
}


void C86winDlg::OnDestroy()
{
	CDialogEx::OnDestroy();

	C86winApp *pApp = (C86winApp*)AfxGetApp();
	pApp->pChipBase->deinitialize();

}


void C86winDlg::OnBnClickedButtonGetFwver()
{
	// TODO: �����ɃR���g���[���ʒm�n���h���[ �R�[�h��ǉ����܂��B
	C86winApp *pApp = (C86winApp*)AfxGetApp();
	IGimic *pGimicModule;
	if( S_OK == pApp->pChipBase->getChipInterface( 0, IID_IGimic, (void**)&pGimicModule ) ){
		UINT clock;
		UINT major=0, minor=0, rev=0, build=0;
		pGimicModule->getFWVer( &major, &minor, &rev, &build );
		CString str;
		str.Format( _T("%d. %d. %d. %d\r\n"), major, minor, rev, build );
		m_editMessage.SetWindowText( str );
		pGimicModule->Release();
	}
}


void C86winDlg::OnBnClickedButtonTest1()
{
	C86winApp *pApp = (C86winApp*)AfxGetApp();
	IGimic *pGimicModule;
	if( S_OK == pApp->pChipBase->getChipInterface( 0, IID_IGimic, (void**)&pGimicModule ) ){
		IRealChip *pchip=NULL;
		if( S_OK == pGimicModule->QueryInterface( IID_IRealChip, (void**)&pchip ) ){
			//pchip->out( 0x28, 0 );
			for( int i=0x38; i<=0x3f; i++ ){
				pchip->out( i, 0x73 );
			}

			pchip->Release();
		}
		pGimicModule->Release();
	}
}


void C86winDlg::OnBnClickedButtonAdpcmZeroreset()
{
	// TODO: �����ɃR���g���[���ʒm�n���h���[ �R�[�h��ǉ����܂��B
	C86winApp *pApp = (C86winApp*)AfxGetApp();
	IRealChip2 *pchip;
	if( S_OK == pApp->pChipBase->getChipInterface( 0, IID_IRealChip2, (void**)&pchip ) ){
		pchip->adpcmZeroClear();
		pchip->Release();
	}
}
