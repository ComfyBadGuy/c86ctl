

#include "stdafx.h"
#include "if_BtSynth.h"

#include <setupapi.h>
#include <initguid.h>
#include <algorithm>
#include "chip/chip.h"


//デバッグ用一時
//後で消す
#include <tchar.h>
#include <StrSafe.h>


using namespace c86ctl;

/*----------------------------------------------------------------------------
追加ライブラリ
----------------------------------------------------------------------------*/
#pragma comment(lib, "setupapi.lib")


/*----------------------------------------------------------------------------
定義いろいろ
----------------------------------------------------------------------------*/
#define PIPE_BUFFER_SIZE 2048



/*----------------------------------------------------------------------------
コンストラクタ
----------------------------------------------------------------------------*/
BtSynthBase::BtSynthBase()
	: hDev(0), cps(0), cal(0), calcount(0), nmodules(0)
{
	rbuff.alloc(PIPE_BUFFER_SIZE);
	::InitializeCriticalSection(&csection);

	::QueryPerformanceFrequency(&freq);
	freq.QuadPart /= 1000; // 1ms

	memset(modules, 0, sizeof(modules));
}

/*----------------------------------------------------------------------------
デストラクタ
----------------------------------------------------------------------------*/
BtSynthBase::~BtSynthBase(void)
{
	::DeleteCriticalSection(&csection);
	if(hDev != NULL)	CloseHandle(hDev);
	hDev = NULL;

	for (int i = 0; i<nmodules; i++)
		if (modules[i])
			delete modules[i];

}

/*----------------------------------------------------------------------------
Device Open
----------------------------------------------------------------------------*/
bool BtSynthBase::OpenDevice(std::basic_string<TCHAR> devpath)
{
	OutputDebugString(_T("-->BtSynthBase::OpenDevice()\n"));

	HANDLE	hNewDev = CreateFile(
		devpath.c_str(),
		GENERIC_READ | GENERIC_WRITE,
		0 , //FILE_SHARE_READ|FILE_SHARE_WRITE
		NULL,
		OPEN_EXISTING,
		FILE_FLAG_OVERLAPPED, //FILE_FLAG_NO_BUFFERING,
		NULL);

	if (hNewDev == INVALID_HANDLE_VALUE){
		OutputDebugString(_T("BtSynth::OpenDevice() デバイスが開けない\n"));
		return false;
	}

	// ここでハンドル更新
	hDev = hNewDev;
	devPath = devpath;


	// Module情報取得 -----------
//	DWORD	dwBytes;
	ULONG	chipcount = 0, chiptype = 0, i = 0, j = 0, n = 0;
	BOARD_INFO binfo;

	/*
	DeviceIoControl(
		hDev,
		IOCTL_BTSYNTH_QUERY_CHIP_COUNT,
		NULL,
		0,
		&chipcount,
		sizeof(ULONG),
		&dwBytes,
		NULL);
	if(NMAXCHIP <= chipcount)	goto	MODULE_CHANGED;

	for(i = 0; i < chipcount;i++){
		DeviceIoControl(
			hDev,
			IOCTL_BTSYNTH_QUERY_CHIP_TYPE,
			&i,
			sizeof(ULONG),
			&chiptype,
			sizeof(ULONG),
			&dwBytes,
			NULL);
		ChipType newtype = static_cast<ChipType>(chiptype);
		if (modules[n] == 0){
			BtSynthBase::BtSynthModule *module = new BtSynthModule(this, i, newtype);
			if (module)
				modules[n++] = module;
		}
		else if (modules[n++]->getChipType() != newtype){
			goto MODULE_CHANGED;
		}
	}*/


	TCHAR	szDebug[_MAX_PATH];


	for (i = 0; i < NMAXBOARDS; i++) {
		OutputDebugString(_T(".\n"));
		int ret = getBoardInfo(i, &binfo);
		if (ret < 0)	continue;

		StringCchPrintf(szDebug, _MAX_PATH, _T("binfo.nchips = %u\n "), binfo.nchips);
		OutputDebugString(szDebug);

		for (j = 0; j < binfo.nchips; j++) {	//NMAXCHIPS
			OutputDebugString(_T("x\n"));
			ChipType newtype = static_cast<ChipType>(binfo.chiptype[j]);
			if (modules[n] == 0) {
				BtSynthBase::BtSynthModule* module = new BtSynthModule(this, i, j, newtype);
				if (module)
					modules[n++] = module;
			}
			else if (modules[n++]->getChipType() != newtype) {
				goto MODULE_CHANGED;
			}
		}
	}

	nmodules = n;
	OutputDebugString(_T("<--BtSynthBase::OpenDevice()\n"));

	return true;

MODULE_CHANGED:
	if((hDev != NULL) && (hDev != INVALID_HANDLE_VALUE))	CloseHandle(hDev);
	hDev = NULL;

	return false;
}

/*----------------------------------------------------------------------------
factory
----------------------------------------------------------------------------*/
int BtSynthBase::UpdateInstances(withlock< std::vector< std::shared_ptr<BaseSoundDevice> > > &devices)
{
	devices.lock();
	std::for_each(devices.begin(), devices.end(), [](std::shared_ptr<BaseSoundDevice> x){ x->checkConnection(); });

	BOOL bResult = TRUE;

	HDEVINFO devinf = INVALID_HANDLE_VALUE;
	SP_DEVICE_INTERFACE_DATA spid;


	devinf = SetupDiGetClassDevs(
		(LPGUID)&GUID_DEVINTERFACE_BtSynth,
		NULL,
		0,
		DIGCF_PRESENT | DIGCF_DEVICEINTERFACE);

	if (devinf){
		for (int i = 0;; i++){
			ZeroMemory(&spid, sizeof(spid));
			spid.cbSize = sizeof(SP_DEVICE_INTERFACE_DATA);
			if (!SetupDiEnumDeviceInterfaces(devinf, NULL,
				(LPGUID)&GUID_DEVINTERFACE_BtSynth, i, &spid)){
				break;
			}

			unsigned long sz;
			std::basic_string<TCHAR> devpath;

			// 必要なバッファサイズ取得
			bResult = SetupDiGetDeviceInterfaceDetail(devinf, &spid, NULL, 0, &sz, NULL);
			PSP_INTERFACE_DEVICE_DETAIL_DATA dev_det = (PSP_INTERFACE_DEVICE_DETAIL_DATA)(malloc(sz));
			dev_det->cbSize = sizeof(SP_INTERFACE_DEVICE_DETAIL_DATA);

			// デバイスノード取得
			if (!SetupDiGetDeviceInterfaceDetail(devinf, &spid, dev_det, sz, &sz, NULL)){
				free(dev_det);
				break;
			}

			devpath = dev_det->DevicePath;
			free(dev_det);
			dev_det = NULL;

			// 既にインスタンスがあるかどうか検索
			auto it = std::find_if(devices.begin(), devices.end(),
				[devpath](std::shared_ptr<BaseSoundDevice> x) -> bool {
				BtSynthBase *gdev = dynamic_cast<BtSynthBase*>(x.get());
				if (!gdev) return false;
				if (gdev->devPath != devpath) return false;
				return true;
			}
			);

			if (it == devices.end()){
				BtSynthBase *dev = new BtSynthBase();
				if (dev){
					if (dev->OpenDevice(devpath)){
						devices.push_back(BaseSoundDevicePtr(dev));
					}
					else{
						delete dev;
					}
				}
			}
			else if (!(*it)->isValid()){
				BtSynthBase *dev = dynamic_cast<BtSynthBase*>(it->get());
				if (!dev->OpenDevice(devpath)){
					// OpenDeviceが失敗した場合は音源モジュールが前回接続時と
					// 異なっているため、別インスタンスを生成する
					BtSynthBase *newdev = new BtSynthBase();
					if (newdev){
						if (newdev->OpenDevice(devpath)){
							devices.push_back(BaseSoundDevicePtr(newdev));
						}
						else{
							delete newdev;
						}
					}
				}
			}
		}

		SetupDiDestroyDeviceInfoList(devinf);
	}

	devices.unlock();
	return 0;
}


/*----------------------------------------------------------------------------
internal.
----------------------------------------------------------------------------*/

//ここではチップへのレジスタデータのみ送信
//第二引数はデータサイズではなくメッセージ数
int BtSynthBase::sendMsg(UINT *data, UINT sz)
{
	int ret = C86CTL_ERR_UNKNOWN;
	DWORD	dwBytes;

	::EnterCriticalSection(&csection);
	DeviceIoControl(
		hDev,
		IOCTL_BTSYNTH_WRITEREG_C86CTL,
		data,
		(sz * sizeof(UINT)),
		NULL,
		0,
		&dwBytes,
		NULL);
	::LeaveCriticalSection(&csection);

	ret = C86CTL_ERR_NONE;
	return ret;
}


//リングバッファに格納する。
void BtSynthBase::out(UCHAR idx, UINT addr, UCHAR data)
{
	while (!rbuff.remain());

	uint32_t d = (idx << 24) | (addr & 0xffff) << 8 | data;
	rbuff.push(d);
}

int c86ctl::BtSynthBase::getBoardInfo(int boardIdx, BOARD_INFO* binfo)
{
	BOOL bResult;
	DWORD	dwBytes;

	if (boardIdx < 0 || boardIdx > NMAXBOARDS)
		return C86CTL_ERR_INVALID_PARAM;
	if (!binfo)
		return C86CTL_ERR_INVALID_PARAM;

	::EnterCriticalSection(&csection);
	bResult = DeviceIoControl(
		hDev,
		IOCTL_BTSYNTH_QUERY_BOARD_INFO,
		&boardIdx,
		sizeof(int),
		binfo,
		sizeof(BOARD_INFO),
		&dwBytes,
		NULL);
	::LeaveCriticalSection(&csection);

	return (bResult) ? C86CTL_ERR_NONE : C86CTL_ERR_UNKNOWN;
}

int c86ctl::BtSynthBase::setPLL(UINT idx, UINT clock)
{
	BOOL	retval = FALSE;
	DWORD	dwBytes;
	UINT	buff[2];

	buff[0] = idx;
	buff[1] = clock;

	TCHAR	szDebug[MAX_PATH];

	StringCchPrintf(szDebug, MAX_PATH, _T("setPLL(%02X,%u) \n"), idx, clock);
	OutputDebugString(szDebug);

	// 1MHz～16MHzに
	if (clock < 1000000)		return C86CTL_ERR_INVALID_PARAM;
	if (clock > 16000000)		return C86CTL_ERR_INVALID_PARAM;

	retval = DeviceIoControl(hDev,
		IOCTL_BTSYNTH_SET_MCLK,
		buff,
		(2 * sizeof(UINT)),
		NULL,
		0,
		&dwBytes,
		NULL);


	return 0;
}


/*----------------------------------------------------------------------------
実装
---------------------------------------------------------------------------*/
int BtSynthBase::reset(void)
{
	BOOL retval = FALSE;
	DWORD dwBytes;

	::EnterCriticalSection(&csection);
	retval = DeviceIoControl(
		hDev,
		IOCTL_BTSYNTH_RESET,
		NULL,
		0,
		NULL,
		0,
		&dwBytes,
		NULL);
	::LeaveCriticalSection(&csection);

	return (retval == TRUE) ? C86CTL_ERR_NONE : C86CTL_ERR_UNKNOWN;
}

int BtSynthBase::isValid(void)
{
	return (hDev == 0) ? FALSE : TRUE;
}


void BtSynthBase::tick(void)
{
	int ret = C86CTL_ERR_UNKNOWN;
	DWORD dwBytes;
	BOOL	bResult = FALSE;
	LARGE_INTEGER et;
	::QueryPerformanceCounter(&et);
	et.QuadPart += freq.QuadPart;

	while (!rbuff.isempty()){
		UINT buff[512];
		UINT sz = 0, i = 0;

		for (sz = 0; sz<512;){	
			if (!rbuff.pop(&buff[sz++]))
				break;
			if (rbuff.isempty())
				break;
		}

		// WriteFileがスレッドセーフかどうかよく分からないので
		// 念のため保護しているが、いらない気がする。
		// (directOut()と重なる可能性がある)
		::EnterCriticalSection(&csection);
		bResult = DeviceIoControl(hDev, 
			IOCTL_BTSYNTH_WRITEREG_C86CTL, 
			buff, 
			(sz * sizeof(UINT)), 
			NULL, 
			0, 
			&dwBytes, 
			NULL);
		::LeaveCriticalSection(&csection);

		//送信出来たら送信バイト数をcalに加算
		if (bResult == TRUE)
			cal += sz * sizeof(UINT);

		// 1tickの処理が1msを超えたら一回抜ける
		//::QueryPerformanceCounter(&ct);
		//if( et.QuadPart<ct.QuadPart ){
		//	break;
		//}
	}

	return;
}


void BtSynthBase::update(void)
{
	if (1 <= calcount++){
		cps = cal;
		cal = 0;
		calcount = 0;
	}
}

void BtSynthBase::checkConnection(void)
{
};


//public
int BtSynthBase::getFWVer(UINT *major, UINT *minor, UINT *rev, UINT *build)
{
	BOOL	retval = FALSE;
	DWORD	dwBytes;
	ULONG	FwVer[4];

	retval = DeviceIoControl(
		hDev,
		IOCTL_BTSYNTH_QUERY_FW_VERSION,
		NULL,
		0,
		FwVer,
		(sizeof(ULONG) * 4),
		&dwBytes,
		NULL);

	if (retval == TRUE ){
		*major	= (UINT)FwVer[0];
		*minor	= (UINT)FwVer[1];
		*rev	= (UINT)FwVer[2];
		*build	= (UINT)FwVer[3];
		return C86CTL_ERR_NONE;
	}
	else{
		return C86CTL_ERR_UNKNOWN;
	}
}

int BtSynthBase::getBoardName(char *board_name)
{
	BOOL	retval = FALSE;
	DWORD	dwBytes;
	if(board_name == NULL)	return C86CTL_ERR_UNKNOWN;

	retval = DeviceIoControl(
		hDev,
		IOCTL_BTSYNTH_QUERY_BOARD_NAME,
		NULL,
		0,
		board_name,
		256,
		&dwBytes,
		NULL);

	return (retval)? C86CTL_ERR_NONE: C86CTL_ERR_UNKNOWN;
}

bool c86ctl::BtSynthBase::setInternalParameter(ULONG idx, ULONG value)
{
	bool	retval = FALSE;
	DWORD	dwBytes;
	ULONG	param[2];

	param[0] = idx;
	param[1] = value;

	retval = DeviceIoControl(
		hDev,
		IOCTL_BTSYNTH_SET_INTERNAL_PARAM,
		param,
		sizeof(ULONG) * 2,
		NULL,
		0,
		&dwBytes,
		NULL);

	return retval;
}

// -------------------------------------------------------------------------------
/*
//廃止
BtSynthBase::BtSynthModule::BtSynthModule(BtSynthBase *device, int chipidx, ChipType chipType)
	: devif(device), chiptype(chipType), chipidx(chipidx),
	devidx((chipidx & 0x7))
{
	//devidx下位3bitのみ使用
	//実質devidx==chipidx
}
*/

BtSynthBase::BtSynthModule::BtSynthModule(BtSynthBase* device, int slotidx, int chipidx, ChipType chipType)
	: devif(device), chiptype(chipType), slotidx(slotidx), chipidx(chipidx),
	devidx(((slotidx & 0x7) << 2) | (chipidx & 0x3))
{
}

BtSynthBase::BtSynthModule::~BtSynthModule()
{
}

void BtSynthBase::BtSynthModule::byteOut(UINT addr, UCHAR data)
{
	devif->out(devidx, addr, data);
}


void BtSynthBase::BtSynthModule::directOut(UINT addr, UCHAR data)
{
	uint32_t d = (devidx << 24) | (addr & 0xffff) << 8 | data;
	devif->sendMsg(&d, 1);

}


int c86ctl::BtSynthBase::BtSynthModule::setPLLClock(UINT clock)
{
	BOOL	retval = FALSE;

	// 1MHz～16MHzに制限
	if (clock < 1000000)		return C86CTL_ERR_INVALID_PARAM;
	if (clock > 16000000)		return C86CTL_ERR_INVALID_PARAM;
	
	retval = devif->setPLL((this->devidx << 2) | this->chipidx, clock);

	return (retval) ? C86CTL_ERR_NONE : C86CTL_ERR_UNKNOWN;
}
