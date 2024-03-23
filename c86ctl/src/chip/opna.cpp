﻿/***
	c86ctl
	
	Copyright (c) 2009-2012, honet. All rights reserved.
	This software is licensed under the BSD license.

	honet.kk(at)gmail.com
 */
#include "stdafx.h"
#include <stdlib.h>
#include "opna.h"

#ifdef _DEBUG
#define new new(_NORMAL_BLOCK,__FILE__,__LINE__)
#endif

using namespace c86ctl;

bool COPNAAdpcm::setReg(UCHAR adrs, UCHAR data)
{
	bool handled = true;
	UINT	shift, complement;
	static bool isPlayed = false;
	reg[adrs] = data;

	switch (adrs) {
	case 0x00: // control 1
		control1 = data;
		if ((data & 0xa0) == 0xa0) {	// start & mem
			keyOnLevel = level >> 3;
			sw = true;
			isPlayed = true;
		}
		if (data & 0x01) {		// reset
			keyOnLevel = 0;
			sw = false;
		}
		if (!(data & 0x80)) { // !start
			sw = false;
		}
		//(data&0x80) // start
		//(data&0x40) // rec
		//(data&0x20);// mem/data
		//(data&0x10);// repeat
		//(data&0x08);// sp off
		//(data&0x01);// reset
		break;

	case 0x01: // control 2
		control2 = data;
		if (data & 0xc0)
			setLR(((data & 0x80) ? true : false), ((data & 0x40) ? true : false));
		//data&0x80; // L
		//data&0x40; // R
		//data&0x08; // SAMPLE
		//data&0x04; // DA/AD
		//data&0x02; // RAM TYPE
		//data&0x01; // ROM/DRAM
		break;

	case 0x02: /// start addr (L)
	case 0x03: /// start addr (H)
		shift = (reg[0x01] & 0x03)? 5 : 2;
		startAddr = ((reg[0x03] << 8) | reg[0x02]) << shift;
		currentAddr = startAddr;
		break;
	case 0x04: /// stop addr (L)
	case 0x05: /// stop addr (H)
		shift = (reg[0x01] & 0x03) ? 5 : 2;
		complement = (reg[0x01] & 0x03) ? 0x1f : 0x03;
		stopAddr = (((reg[0x05] << 8) | reg[0x04]) << shift) | complement;
		break;
	case 0x06: // prescale(L)
	case 0x07: // prescale(H)
		prescale = (reg[0x07] << 8) | reg[0x06];
		break;

	case 0x08: // adpcm data.
		if (isPlayed) {
			memset(minimap, 0, minimapsize);
			isPlayed = false;
		}
		if (control1 & 0x60) {
			if (0 <= currentAddr && currentAddr < ramsize) { // 念のため
				dram[currentAddr] = data;
				map[currentAddr] |= 0x01;
				minimap[currentAddr >> 9] |= 0x01;
			}
			if (stopAddr > currentAddr) {
				if (limitAddr > currentAddr) {
					currentAddr++;
				}
				else {
					currentAddr = 0;
				}
			}
		}
		break;

	case 0x09: // delta-N(L)
	case 0x0a: // delta-N(H)
		deltaN = (reg[0x0a] << 8) | reg[0x09];
		break;

	case 0x0b: // level
		level = data;
		break;

	case 0x0c: /// limit addr (L)
	case 0x0d: /// limit addr (H)
		shift = (reg[0x01] & 0x03) ? 5 : 2;
		complement = (reg[0x01] & 0x03) ? 0x1f : 0x03;
		limitAddr = (((reg[0x0d] << 8) | reg[0x0c]) << shift) | complement;
		break;

	case 0x0e: // dac data.
	case 0x0f: // pcm data.
		break;

		//case 0x10: // flag control
		//	break;

	default:
		handled = false;
	}

	return handled;
}




void COPNA::byteOut( UINT addr, UCHAR data )
{
	int ch;
	if( 0x200 <= addr ) return;

	switch( addr ){
	case 0x29: // SCH/IRQ ENABLE
		data |= 0x80;
		break;

	//OPNA/OPN mode-mask
	//case 0x60: case 0x61: case 0x62:	// FM -- AMON/DR
	//case 0x64: case 0x65: case 0x66:
	//case 0x68: case 0x69: case 0x6a:
	//case 0x6c: case 0x6d: case 0x6e:
	//	if(!modeOPNA)
	//		*data &= 0x70;
		
	case 0xb4: // FM -- LR, AMS, PMS
	case 0xb5:
	case 0xb6:
		ch = addr - 0xb4;
		if( getMixedMask( ch ) )
			data &= 0x3f;
		//if(!modeOPNA)
		//	*data = 0xc0;
		break;
			
	case 0x1b4: // FM -- LR, AMS, PMS
	case 0x1b5:
	case 0x1b6:
		ch = addr - 0x1b4;
		if( getMixedMask( 6+ch ) )
			data &= 0x3f;
		break;

	case 0x08: // SSG amp level
	case 0x09:
	case 0x0a:
		ch = addr-0x08;
		if( getMixedMask( 9+ch ) )
			data &= 0xf0;
		break;
		
	case 0x10: // Rhythm key on
		if( getMixedMask( 13 ) ){
			return;
		}
		break;
	case 0x11: // Rhythm total level
		if( getMixedMask( 13 ) )
			data = 0;
		break;

	case 0x10b: // ADPCM level control
		if( getMixedMask( 12 ) )
			data = 0;
		break;
	}

	if(setReg(addr,data))
		if(ds) ds->byteOut(addr,data);
}

bool COPNA::setReg( UINT addr, UCHAR data ){
	if( 0x200 <= addr ) return false;
	int idx = 0;
	if( 0x100 <= addr ){
		addr -= 0x100;
		idx = 1;
	}
	reg[idx][addr] = data;
	INT c = regATime[idx][addr];
	c+=64;
	regATime[idx][addr] = 255<c ? 255 : c;

// TODO: DEBUG ME!!
// Filter the Timer Control
//if(0x24<=addr && addr<=0x27)
//	return false;

	if( idx == 0 ){
		if( ssg->setReg( addr, data ) )
			return true;
		if( rhythm->setReg( addr, data ) )
			return true;
		if( fmCommonRegHandling( addr, data ) )
			return true;
	}else{
		if( adpcm->setReg( addr, data ) )
			return true;
	}
	if( fm->setReg( idx, addr, data ) )
		return true;

	return false;
};

UCHAR COPNA::getReg( UINT addr ){
	if( 0x200 <= addr ) return 0;
	int idx = 0;
	if( 0x100 <= addr ){
		addr -= 0x100;
		idx = 1;
	}
	return reg[idx][addr];
};


bool COPNA::fmCommonRegHandling( UCHAR adrs, UCHAR data )
{
	bool handled = true;
	
	switch(adrs){
	case 0x10:	// STATUS MASK
		//data&0x80; // IRQ RESET
		//data&0x10; // MASK ZERO (ADPCM)
		//data&0x08; // MASK BRDY (ADPCM)
		//data&0x04; // MASK EOS (ADPCM)
		//data&0x02; // MASK TIMER-B
		//data&0x01; // MASK TIMER-A
		break;
		
	// prescaler
	case 0x2d:
		prescale_fm = 6;
		prescale_ssg = 4;
		break;
	case 0x2e:
		prescale_fm = 3;
		prescale_ssg = 2;
		break;
	case 0x2f:
		prescale_fm = 2;
		prescale_ssg = 1;
		break;

	case 0x24:	// Timer-A Corse
	case 0x25:	// Timer-A Fine
	case 0x26:	// Timer-B
	case 0x27:  // Timer Control
	case 0x29:	// IRQ/SCH
		break;
	//if( addr == 0x29){
		//	// SCH/IRQ ENABLE
		//	modeOPNA = (data&0x80) ? true : false;
		//	return true;
	//}
		

	default:
		handled = false;
		break;
	}

	return handled;
}

void COPNA::applyMask(int ch)
{
	UCHAR data;
	if(!ds) return;

	bool mask = getMixedMask(ch);
	
	if( 0<=ch && ch<=2 ){ // FM 1~3
		int fmNo = ch;
		data = (fm->ch[fmNo]->getAMS()<<4) | fm->ch[fmNo]->getPMS();
		if( !mask ){
			bool l,r;
			fm->ch[fmNo]->getLR( l,r );
			data |= (l?0x80:0) | (r?0x40:0);
		}
		ds->byteOut( 0xb4+ch, data );
		//pIF->directOut( 0xb4+ch, data );
	}
	else if( 6<=ch && ch<=8 ){ // FM 4~6
		int fmNo = ch - 3;
		data = (fm->ch[fmNo]->getAMS()<<4) | fm->ch[fmNo]->getPMS();
		if( !mask ){
			bool l,r;
			fm->ch[fmNo]->getLR( l,r );
			data |= (l?0x80:0) | (r?0x40:0);
		}
		ds->byteOut( 0x1b4+(ch-6), data );
		//pIF->directOut( 0x1b4+(ch-6), data );
	}
	else if( 9<=ch && ch<=11 ){ // SSG 1~3
		int ssgNo = ch-9;
		data = mask ? 0 : ssg->ch[ssgNo]->getLevel();
		data |= ssg->ch[ssgNo]->isUseEnv() ? 0x10 : 0;
		ds->byteOut( 0x08+ssgNo, data );
		//pIF->directOut( 0x08+ssgNo, data );
	}
	else if( ch == 12 ){ // ADPCM
		data = mask ? 0 : adpcm->getLevel();
		ds->byteOut( 0x10b, data );
		//pIF->directOut( 0x10b, data );
	}
	else if( ch == 13 ){ // RHYTHM
		data = mask ? 0 : rhythm->getTotalLevel();
		ds->byteOut( 0x11, data );
		//pIF->directOut( 0x11, data );
	}
}

void COPNA::setPartMask(int ch, bool mask)
{
	if( ch<0 || 14<=ch ) return;

	if( mask ){
		partMask |= 1<<ch;
	}else{
		partMask &= ~(1<<ch);
	}
	applyMask(ch);
}

void COPNA::setPartSolo(int ch, bool mask)
{
	if( ch<0 || 14<=ch ) return;
	
	if( mask )	partSolo |= 1<<ch;
	else		partSolo &= ~(1<<ch);
	
	for( int i=0; i<14; i++ )
		applyMask(i);
}

