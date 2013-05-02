/***
	c86ctl
	gimic �R���g���[�� WinUSB��(�����R�[�h)
	
	Copyright (c) 2009-2012, honet. All rights reserved.
	This software is licensed under the BSD license.

	honet.kk(at)gmail.com
 */

#pragma once

#include "if.h"

#ifdef SUPPORT_WINUSB

#include <mmsystem.h>
#include <winusb.h>
#include <vector>
#include "ringbuff.h"
#include "withlock.h"
#include "chip/chip.h"

namespace c86ctl{

class GimicWinUSB : public GimicIF
{
// �t�@�N�g�� -------------------------------------------------------
public:
	static int UpdateInstances( withlock< std::vector< std::shared_ptr<GimicIF> > > &gimics);

// ���J�C���^�t�F�C�X -----------------------------------------------
public:
	// IGimic
	virtual int __stdcall setSSGVolume(UCHAR vol);
	virtual int __stdcall getSSGVolume(UCHAR *vol);
	virtual int __stdcall setPLLClock(UINT clock);
	virtual int __stdcall getPLLClock(UINT *clock);
	virtual int __stdcall getFWVer( UINT *major, UINT *minor, UINT *rev, UINT *build );
	virtual int __stdcall getMBInfo(struct Devinfo *info);
	virtual int __stdcall getModuleInfo(struct Devinfo *info);
	
public:
	// IGimic2
	virtual int __stdcall getModuleType(enum ChipType *type);

public:
	// IRealChip
	virtual int __stdcall reset(void);
	virtual void __stdcall out(UINT addr, UCHAR data);
	virtual UCHAR __stdcall in(UINT addr);

public:
	// IRealChip2
	virtual int __stdcall getChipStatus(UINT addr, UCHAR *status);
	virtual void __stdcall directOut(UINT addr, UCHAR data);

// ������ -----------------------------------------------
public:
	virtual int __stdcall setDelay(int delay);
	virtual int __stdcall getDelay(int *delay);
	virtual int __stdcall isValid(void);


// C86CTL�������p ---------------------------------------------------
private:
	GimicWinUSB(HANDLE dev, HANDLE winUsb);

public:
	~GimicWinUSB(void);

public:
	// ����J
	virtual int init(void);
	virtual void tick(void);
	virtual void update(void);
	virtual Chip* getChip(void){ return chip; };
	virtual const GimicParam* getParam(){ return &gimicParam; };

	virtual UINT getCPS(void){ return cps; };
	virtual void checkConnection(void);

// �v���C�x�[�g -----------------------------------------------------
private:
	struct MSG{
		// �Ȃ�ƂȂ����v2-DWORD�ɂȂ�悤�ɂ��Ă݂��B
		UCHAR len;
		UCHAR dat[7];	// �ő僁�b�Z�[�W���͍��̂Ƃ���6byte.
	};
	
	struct REQ{
		UINT t;
		USHORT addr;
		UCHAR dat;
		UCHAR dummy;
	};

private:
	int sendMsg( MSG *data );
	int transaction( MSG *txdata, uint8_t *rxdata, uint32_t rxsz );
	void out2buf(UINT addr, UCHAR data);
	
	int devWrite( LPCVOID data );
	int devRead( LPVOID data );
	
private:
	HANDLE hDev;
	WINUSB_INTERFACE_HANDLE hWinUsb;
	std::basic_string<TCHAR> devPath;
	UCHAR inPipeId;
	USHORT inPipeMaxPktSize;
	UCHAR outPipeId;
	USHORT outPipeMaxPktSize;

	CRITICAL_SECTION csection;
	CRingBuff<MSG> rbuff;
	UINT cps, cal, calcount;

	int delay;
	CRingBuff<REQ> dqueue;
	
	Chip *chip;
	ChipType chiptype;
	GimicParam gimicParam;

	LARGE_INTEGER freq;
	
};

typedef std::shared_ptr<GimicWinUSB> GimicWinUSBPtr;
};

#endif

