
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

	// TODO: �������������ɒǉ����܂��B

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
	c86ctl_initialize();

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
//		str.Format( _T("%01d, INDEX:%5d, TICK:%5d, NOW:%05d, NEXT:%05d"), delay, idx, tick, now/1000, next/1000 );
//		pThis->m_staticTick.SetWindowText(str);
//		pThis->m_staticTick.UpdateWindow();

		auto prow = &pThis->s98data.row;
		if( idx < prow->size() ){
			auto pr = &prow->at(idx);
			while( pr->gtick <= tick ){
				if( pr->getDeviceNo() == 0 && 
					pr->len == 3 ){
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
						c86ctl_out(addr, data);
						last_is_adpcm = ( addr == 0x108 );
				}
				if( ++idx >= prow->size() )
					break;
				pr = &prow->at(idx);
			}
		}

	
#if 0
		for( std::list< std::shared_ptr<CGimicIF> >::iterator it = gGIMIC.begin(); it != gGIMIC.end(); it++ ){
			(*it)->Tick();
		}
#else
//		// �������E�E�E
//		std::for_each( gGIMIC.begin(), gGIMIC.end(), [](std::shared_ptr<GimicIF> x){ x->Tick(); } );
#endif

	}
	c86ctl_reset();
	c86ctl_deinitialize();
	

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
		terminateFlag = 0;
		hThread = (HANDLE)_beginthreadex( NULL, 0, C86winDlg::PlayerThread, this, 0, &threadID );
		SetThreadPriority(hThread, THREAD_PRIORITY_ABOVE_NORMAL);
	}
}


void C86winDlg::OnBnClickedButtonStop()
{
	if( hThread ){
		terminateFlag = 1;
		WaitForSingleObject( hThread, INFINITE );
		hThread = 0;
		threadID = 0;
	}
}


void C86winDlg::OnBnClickedButtonOpen()
{
	CFileDialog dlg( TRUE, _T("s98"), NULL, NULL, _T("s98 files (*.s98)|*.s98||"), this );

	if( dlg.DoModal() == IDOK ){
		m_editFilePath.SetWindowText( dlg.GetPathName() );
		s98data.loadFile(dlg.GetPathName());
	}
}
