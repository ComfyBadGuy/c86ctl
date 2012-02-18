/***
	c86ctl
	
	Copyright (c) 2009-2012, honet. All rights reserved.
	This software is licensed under the BSD license.

	honet.kk(at)gmail.com
 */

#include "stdafx.h"
#include "opna.h"
#include "vis_c86fm.h"
#include "vis_c86sub.h"

#ifdef _DEBUG
#define new new(_NORMAL_BLOCK,__FILE__,__LINE__)
#endif

#define WINDOW_WIDTH  (316+4)
#define WINDOW_HEIGHT (75*5+4+15)


bool CVisC86OPNAFm::create( HWND parent )
{
	int left = INT_MIN;
	int top = INT_MIN;
	
	if( !CVisWnd::create(
		left, top, windowWidth, windowHeight,
		WS_EX_TOOLWINDOW, (WS_POPUP | WS_CLIPCHILDREN), parent ) )
		return false;

	COPNAFmCh *pFmCh = pOPNA->fm->ch[this->ch];
	// AMS
	knobAMS = CVisKnobPtr( new CVisKnob(this, 134-11, 25-10));
	knobAMS->setRange(0,3);
	knobAMS->getter = [pFmCh]() -> int{ return pFmCh->getAMS(); };
	//knobAMS->setter = [pOPNAFm](int ams) { pOPNAFm->setAMS(ams); };
	widgets.push_back(knobAMS);

	// PMS
	knobPMS = CVisKnobPtr( new CVisKnob(this, 158-11, 25-10));
	knobPMS->setRange(0,7);
	knobPMS->getter = [pFmCh]() -> int{ return pFmCh->getPMS(); };
	widgets.push_back(knobPMS);

	// FB
	knobFB = CVisKnobPtr( new CVisKnob(this, 134-11, 59-10));
	knobFB->setRange(0,7);
	knobFB->getter = [pFmCh]() -> int{ return pFmCh->getFeedback(); };
	widgets.push_back(knobFB);

	// PAN
	knobPAN = CVisKnobPtr( new CVisKnob(this, 158-11, 59-10));
	knobPAN->setRange(-1,1);
	knobPAN->getter = [pFmCh]() -> int{ return pFmCh->getPan(); };
	widgets.push_back(knobPAN);

	int x=0, y=75;
	for(int i=0; i<4; i++){
		COPNAFmSlot *slot = pFmCh->slot[i];
		// AR
		knobAR[i] = CVisKnobPtr( new CVisKnob(this, x+165-11, y+27-10));
		knobAR[i]->setRange(0,31);
		knobAR[i]->getter = [slot, i]() -> int{ return slot->getAttackRate(); };
		widgets.push_back(knobAR[i]);
		
		// DR1
		knobDR1[i] = CVisKnobPtr( new CVisKnob(this, x+190-11, y+27-10));
		knobDR1[i]->setRange(0,31);
		knobDR1[i]->getter = [slot, i]() -> int{ return slot->getDecayRate(); };
		widgets.push_back(knobDR1[i]);

		// DR2 - OPNAでは使えない。
		knobDR2[i] = CVisKnobPtr( new CVisKnob(this, x+215-11, y+27-10));
		knobDR2[i]->setRange(0,31);
		widgets.push_back(knobDR2[i]);

		// SR
		knobSR[i] = CVisKnobPtr( new CVisKnob(this, x+240-11, y+27-10));
		knobSR[i]->setRange(0,31);
		knobSR[i]->getter = [slot, i]() -> int{ return slot->getSustainRate(); };
		widgets.push_back(knobSR[i]);

		// RR
		knobRR[i] = CVisKnobPtr( new CVisKnob(this, x+265-11, y+27-10));
		knobRR[i]->setRange(0,31);
		knobRR[i]->getter = [slot, i]() -> int{ return slot->getReleaseRate(); };
		widgets.push_back(knobRR[i]);

		// SL
		knobSL[i] = CVisKnobPtr( new CVisKnob(this, x+290-11, y+27-10));
		knobSL[i]->setRange(0,15);
		knobSL[i]->getter = [slot, i]() -> int{ return slot->getSustainLevel(); };
		widgets.push_back(knobSL[i]);

		// TL
		knobTL[i] = CVisKnobPtr( new CVisKnob(this, x+315-11, y+27-10));
		knobTL[i]->setRange(0,127);
		knobTL[i]->getter = [slot, i]() -> int{ return slot->getTotalLevel(); };
		widgets.push_back(knobTL[i]);
		
		// MUL
		knobMUL[i] = CVisKnobPtr( new CVisKnob(this, x+165-11, y+60-10));
		knobMUL[i]->setRange(0,15);
		knobMUL[i]->getter = [slot, i]() -> int{ return slot->getMultiple(); };
		widgets.push_back(knobMUL[i]);

		// DET
		knobDET[i] = CVisKnobPtr( new CVisKnob(this, x+190-11, y+60-10));
		knobDET[i]->setRange(0,7);
		knobDET[i]->getter = [slot, i]() -> int{ return slot->getDetune(); };
		widgets.push_back(knobDET[i]);

		// AM
		dipswAM[i] = CVisDipSwPtr( new CVisDipSw(this, x+216, y+50));
		dipswAM[i]->getter = [slot, i]() -> int{ return slot->isAM(); };
		widgets.push_back(dipswAM[i]);
		y+=75;
	}
	
	::ShowWindow( hWnd, SW_SHOWNOACTIVATE );
	
	return true;
}

void CVisC86OPNAFm::close()
{
	// TODO: delete widgets.
	widgets.clear();
	
	CVisWnd::close();
}


void CVisC86OPNAFm::onPaintClient()
{
	visFillRect( clientCanvas, 0, 0, clientCanvas->getWidth(), clientCanvas->getHeight(), ARGB(255,0,0,0) );

	// slot view
	if( pOPNA ){
		int sx=5, sy=5, cx=6, cy=8;
		drawFMView( clientCanvas, 0, 0, pOPNA->fm->ch[ch] );
		for( int i=0; i<4; i++ )
			drawFMSlotView( clientCanvas, 0, 75*(1+i), pOPNA->fm->ch[ch]->slot[i], i );
	}
}

void CVisC86OPNAFm::drawFMView( IVisBitmap *canvas, int x, int y, COPNAFmCh *pFmCh )
{
	int exmode = pFmCh->getExMode();
	CVisC86Skin *skin = &gVisSkin;
	skin->drawFMSkin( canvas, x, y, exmode );
	
	int algno = pFmCh->getAlgorithm();
	skin->drawFMAlgorithm( canvas, x+20, y+7, algno );
	
	//slot ON/OFF
	const int lx[4] = {243, 258, 273, 288};
	for( int i=0; i<4; i++ ){
		skin->drawFMSlotLight( canvas, x+lx[i], y+56, i, pFmCh->slot[i]->isOn() );
	}

	// freq
	double freq = pFmCh->getFreq();
	char str[128];
	if( !exmode ){
		sprintf( str, "% 11.1fHz", freq );
		skin->drawStr( canvas, 0, 211, 10, str );
	}else{
		const char *modestr[] = { "CSM", "EFF", "???" };
		skin->drawStr( canvas, 0, 183, 59, modestr[exmode-1] );
		
		for( int i=0; i<4; i++ ){
			freq = pFmCh->getFreqEx(i);
			sprintf( str, "% 11.1fHz", freq );
			skin->drawStr( canvas, 0, 211, 6+12*i, str );
		}
	}
}

void CVisC86OPNAFm::drawFMSlotView( IVisBitmap *canvas, int x, int y, COPNAFmSlot *pSlot, int slotidx )
{
	CVisC86Skin *skin = &gVisSkin;
	skin->drawFMSlotSkin( canvas, x, y );
#if 0
	double ar = pSlot->getAttackRate();
	double dr = pSlot->getDecayRate();
	double sr = pSlot->getSustainRate();
	double rr = pSlot->getReleaseRate();
	double sl = pSlot->getSustainLevel();
	double tl = pSlot->getTotalLevel();
	double mul = pSlot->getMultiple();
	double det = pSlot->getDetune();

	double ax, ay, d1x, d1y, d2x, d2y, rx, ry;
	
	if(ar==0 || tl==127){
		// 無音
	}else{
		// attack ----
		ay = tl;
		ax = (127.0-tl) / (5.0*ar);
		
		// decay1 ----
		if( (sl*4)>tl ){
			if( 0<dr ){
				d1y = 4.0*sl;
				d1x = (d1y-ay)/dr + ax;
			}else{
				d1y = ay;
				d1x = 3.0*127.0/4.0;
			}
		}else{
			d1x = ax;
			d1y = ay;
		}

		// decay2 ----
		d2x = 3.0*127/4.0;
		if( 0<dr ){
			d2y = sr * (d2x-d1x) + d1y;
		}else{
			d2y = d1y;
		}
		
		// release ----
		if( 0<rr ){
			rx = (127.0-d2y) / (2.0*rr) + d2x;
			ry = 127.0;
		}else{
			rx = 127.0;
			ry = d2y;
		}

		int sx=x+23, sy=y+6+64;
		visDrawLine( canvas, sx         , sy         , sx+(int)ax , sy-(int)ay , 0xffffffff );
		visDrawLine( canvas, sx+(int)ax , sy-(int)ay , sx+(int)d1x, sy-(int)d1y, 0xffffffff );
		visDrawLine( canvas, sx+(int)d1x, sy-(int)d1y, sx+(int)d2x, sy-(int)d2y, 0xffffffff );
		visDrawLine( canvas, sx+(int)d2x, sy-(int)d2y, sx+(int)rx , sy-(int)ry, 0xffffffff );
	}
#endif
}

// --------------------------------------------------------
CVisC86FmPtr visC86FmViewFactory(Chip *pchip, int id, int ch)
{
	if( typeid(*pchip) == typeid(COPNA) ){
		return CVisC86FmPtr( new CVisC86OPNAFm(dynamic_cast<COPNA*>(pchip), id, ch ) );
//	}else if( typeid(*pchip) == typeid(COPN3L) ){
//		return CVisC86RegPtr( new CVisC86OPN3LKey(dynamic_cast<COPN3L*>(pchip), id) );
//	}else if( typeid(*pchip) == typeid(COPM) ){
//		return CVisC86RegPtr( new CVisC86OPMKey(dynamic_cast<COPM*>(pchip), id) );
	}
	return 0;
}

