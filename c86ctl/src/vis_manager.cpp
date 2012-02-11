/***
	c86ctl
	
	Copyright (c) 2009-2012, honet. All rights reserved.
	This software is licensed under the BSD license.

	honet.kk(at)gmail.com
 */
#include "StdAfx.h"

#include "vis_manager.h"
#include "vis_c86wnd.h"

#pragma comment(lib, "d2d1.lib")
//#include <mmsystem.h>
//#pragma comment(lib, "winmm.lib")

#ifdef _DEBUG
#define new new(_NORMAL_BLOCK,__FILE__,__LINE__)
#endif


void CVisManager::add( CVisWnd *wnd )
{
	clients.push_back(wnd);
	wnd->setManager(this);
}

void CVisManager::del( CVisWnd *wnd )
{
	auto ei = std::remove(clients.begin(), clients.end(), wnd);
	clients.erase(ei, clients.end());
}

void CVisManager::draw(void)
{
	std::for_each( clients.begin(), clients.end(),
			  [](CVisWnd* x){ ::InvalidateRect(x->getHWND(), NULL, FALSE); } );
//		[](CVisWnd* x){ ::RedrawWindow(x->getHWND(), NULL, NULL, RDW_INVALIDATE|RDW_INTERNALPAINT); } );
//		[](CVisWnd* x){ x->onPaint(); } );
	fps = counter.getFPS();
}

