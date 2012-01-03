/***
	c86ctl
	gimic �R���g���[�� HID��(�����R�[�h)
	
	Copyright (c) 2009-2010, honet. All rights reserved.
	This software is licensed under the BSD license.

	honet.kk(at)gmail.com
	Thanks to Nagai "Guu" Osamu 2011/12/08 for his advice.
 */
#pragma once

#include "if.h"

#ifdef SUPPORT_HID

#include <mmsystem.h>
#include <vector>
#include "ringbuff.h"


class GimicHID : public GimicIF
{
private:
	GimicHID(HANDLE h);

public:
	~GimicHID(void);
	
public:
	// IGimicModule
	virtual int __stdcall setSSGVolume(UCHAR vol);
	virtual int __stdcall setPLLClock(UINT clock);

public:
	// IRealChip
	virtual int __stdcall reset(void);
	virtual void __stdcall out(UINT addr, UCHAR data);
	virtual void __stdcall tick(void);

private:
	HANDLE hHandle;
	CRingBuff<UCHAR> rbuff;
	
public:
	static std::vector< std::shared_ptr<GimicIF> > CreateInstances(void);
};

typedef std::shared_ptr<GimicHID> GimicHIDPtr;

#endif

