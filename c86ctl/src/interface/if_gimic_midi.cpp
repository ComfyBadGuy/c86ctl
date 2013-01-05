/***
	c86ctl
	gimic �R���g���[�� MIDI��(�����R�[�h)
	
	Copyright (c) 2009-2012, honet. All rights reserved.
	This software is licensed under the BSD license.

	honet.kk(at)gmail.com
	Thanks to Nagai "Guu" Osamu 2011/12/08 for his advice.

	note: honet/k.kotajima
	MIDI�ł̃��W�X�^�_���v�]���͂�����p�~�������Ǝv���Ă��邪
	���̂Ƃ���HID�ł��܂����肵�Ă��Ȃ��̂Ŏc���Ă���B

	�]���d�l�F sysex��3byte�Ƃ��Ƀp�b�N���đ���B
	 idx  data   mean
	   0  0xF0: Start of SysEx
	   1  0x7D:   device id
	3n+2  (d0):   {000, exaddr[1:0], addr[7:6]}
	3n+3  (d1):   {addr[5:0], data[7:7]}
	3n+4  (d2):   data[6:0]
	...
	3N+5  0xF7: End of SysEx

	�V�X�e�����b�Z�[�W�F
	SW Reset     : f0, 7d, 40, 01, 00, 7e, f7
	HW Reset     : f0, 7d, 40, 01, 00, 7f, f7
	PLL Set      : f0, 7d, 40, 01, 00, 70, v0, v1, v2, v3, f7
	GM System ON : f0, 7e, 7f, 09, 01, f7
	SSG Vol      : f0, 7f, 04, vol[6:0], vol[13:7], f7
 */

#include <stdafx.h>
#include <algorithm>
#include "if_gimic_midi.h"
#include "config.h"
#include "chip/chip.h"
#include "chip/opm.h"
#include "chip/opna.h"
#include "chip/opn3l.h"


#ifdef _DEBUG
#define new new(_NORMAL_BLOCK,__FILE__,__LINE__)
#endif

using namespace c86ctl;

/*----------------------------------------------------------------------------
	�R���X�g���N�^
----------------------------------------------------------------------------*/
GimicMIDI::GimicMIDI( HMIDIOUT h ) : hHandle(h), chip(0), chiptype(CHIP_OPNA)
{
	rbuff.alloc(128);
}

/*----------------------------------------------------------------------------
	�f�X�g���N�^
----------------------------------------------------------------------------*/
GimicMIDI::~GimicMIDI(void)
{
	if(hHandle) {
		midiOutClose(hHandle);
		hHandle = NULL;
	}
	if( chip )
		delete chip;
}


/*----------------------------------------------------------------------------
	MIDI-IF factory
----------------------------------------------------------------------------*/
std::vector< std::shared_ptr<GimicIF> > GimicMIDI::CreateInstances(void)
{
	std::vector< std::shared_ptr<GimicIF> > instances;

	// MIDIOUTCAPS��gimic���ǂ����I�ʂł��Ȃ����Ǝv�������ǃ_���������B
//	UINT n = midiOutGetNumDevs();
//	for( UINT i=0; i<n; i++ ){
//		MIDIOUTCAPS midiOutCaps;
//		midiOutGetDevCaps( i, &midiOutCaps, sizeof(midiOutCaps) );
//	}

	int DeviceID = gConfig.getInt(INISC_MAIN, INIKEY_MIDIDEVICE, -1);
	HMIDIOUT hmidi = NULL;
	UINT res = midiOutOpen(&hmidi, DeviceID, 0, 0, CALLBACK_NULL);
	if(res == MMSYSERR_NOERROR){
		instances.push_back( GimicMIDIPtr(new GimicMIDI(hmidi)) );
	}

	std::for_each( instances.begin(), instances.end(), [](std::shared_ptr<GimicIF> x){ x->init(); } );
	return instances;
}

void GimicMIDI::sendSysEx( uint8_t *data, uint32_t sz )
{
	MIDIHDR head;
	ZeroMemory(&head, sizeof(MIDIHDR));
	head.lpData = reinterpret_cast<LPSTR>(data);
	head.dwBufferLength = sz;

	midiOutPrepareHeader(hHandle, &head, sizeof(MIDIHDR));
	midiOutLongMsg(hHandle, &head, sizeof(MIDIHDR));
	// �����ꖳ���Ă������ł���ˁH(by Guu)
	//	while((head.dwFlags & MHDR_DONE) == 0) ;
	midiOutUnprepareHeader(hHandle, &head, sizeof(MIDIHDR));
}

/*----------------------------------------------------------------------------
	����
----------------------------------------------------------------------------*/
int GimicMIDI::init(void)
{
	// MIDI-IF�̏ꍇ��OPNA���߂���(module����if������ĂȂ�����)
	//chiptype = CHIP_OPNA;
	//chip = new COPNA();
	chiptype=CHIP_OPM;
	chip = new COPM(static_cast<IRealChip2*>(this));
	return C86CTL_ERR_NONE;
}

int GimicMIDI::reset( void )
{
	// �]�������҂�
	while(rbuff.length()){
		Sleep(10);
	}

	// GM System ON
	UCHAR d[] = { 0xf0, 0x7e, 0x7f, 0x9, 0x1, 0xf7 };
	sendSysEx( &d[0], 6 );
	return C86CTL_ERR_NONE;
}

void GimicMIDI::out(UINT addr, UCHAR data)
{
	bool flag = true;
	if( chip )
		flag = chip->setReg(addr, data );
	if( flag ){
		// data packing.
		UCHAR d[3] = { (addr>>6)&0x0f, (addr&0x3f)<<1 | (data>>7), (data&0x7f) };
		rbuff.push(d,3);
	}
}

UCHAR GimicMIDI::in(UINT addr)
{
	if( chip )
		return chip->getReg(addr);

	return 0;
}

int GimicMIDI::setSSGVolume(UCHAR vol)
{
	// master volume set.
	UCHAR d[] = { 0xf0, 0x7f, 0x04, (vol<<6)&0x7f, (vol>>1)&0x7f, 0xf7 };
	sendSysEx( &d[0], 6 );
	return C86CTL_ERR_NONE;
}

int GimicMIDI::getSSGVolume(UCHAR *vol)
{
	return C86CTL_ERR_NOT_IMPLEMENTED;
}

int GimicMIDI::setPLLClock(UINT clock)
{
	UCHAR d[] = { 0xf0, 0x7d, 0x40, 0x01, 0x00, 0x70,
		clock&0x7f, (clock>>7)&0x7f, (clock>>14)&0x7f, 0xf7 };
	sendSysEx( &d[0], 10 );
	return C86CTL_ERR_NONE;
}
int GimicMIDI::getPLLClock(UINT *clock)
{
	return C86CTL_ERR_NOT_IMPLEMENTED;
}

int GimicMIDI::getMBInfo( struct Devinfo *info )
{
	return C86CTL_ERR_NOT_IMPLEMENTED;
}

int GimicMIDI::getModuleInfo( struct Devinfo *info )
{
	return C86CTL_ERR_NOT_IMPLEMENTED;
}

int GimicMIDI::getFWVer( UINT *major, UINT *minor, UINT *rev, UINT *build )
{
	return C86CTL_ERR_NOT_IMPLEMENTED;
}

int GimicMIDI::getChipStatus( UINT addr, UCHAR *status )
{
	return C86CTL_ERR_NOT_IMPLEMENTED;
}
/*
int GimicMIDI::adpcmZeroClear(void)
{
	return C86CTL_ERR_NOT_IMPLEMENTED;
}

int GimicMIDI::adpcmWrite( UINT startAddr, UINT size, UCHAR *data )
{
	return C86CTL_ERR_NOT_IMPLEMENTED;
}

int GimicMIDI::adpcmRead( UINT startAddr, UINT size, UCHAR *data )
{
	return C86CTL_ERR_NOT_IMPLEMENTED;
}
*/
void GimicMIDI::tick(void)
{
	UCHAR buff[150];
	
	if( !hHandle )
		return;

	// �]���z�񏀔�
	buff[0] = 0xf0;		// start of sysex
	buff[1] = 0x7d;		// device id
	UINT msz = rbuff.length();
	UINT sz = msz / 3;
	if( 0 < sz ){
		rbuff.pop( &buff[2], MIN(sz*3,144) );
		buff[sz*3+2] = 0xf7;		// end of sysex
		sendSysEx( &buff[0], sz*3+3 );
	}
	
	return;
}


