/* 3dScreamers
 * Copyright (c) 2002 - 2003 Jeffrey Myers
 *
 * This package is free software;  you can redistribute it and/or
 * modify it under the terms of the license found in the file
 * named license.txt that should have accompanied this file.
 *
 * THIS PACKAGE IS PROVIDED ``AS IS'' AND WITHOUT ANY EXPRESS OR
 * IMPLIED WARRANTIES, INCLUDING, WITHOUT LIMITATION, THE IMPLIED
 * WARRANTIES OF MERCHANTIBILITY AND FITNESS FOR A PARTICULAR PURPOSE.
 */
// game loop.cpp
// ***** TODO
//
// Add other OS stuff
//
#include "gameLoop.h"
#include "OGFEvents.h"
#include "3dView.h"
#include "displayListManager.h"
// C RunTime Header Files
#include <stdlib.h>
#include <stdio.h>
#include "SDL.h"

CBaseGameLoop::CBaseGameLoop()
{
	m_iXOSWindowPos = -1;
	m_iYOSWindowPos = -1;

	m_iXOSWindowSize = -1;
	m_iYOSWindowSize = -1;

	m_bFullScreen = false;
}

CBaseGameLoop::~CBaseGameLoop()
{
}

bool CBaseGameLoop::ProcessEvents ( void )
{
	SDL_Event event;

	bool bDone = false;

	while (SDL_PollEvent(&event)) 
	{
		switch(event.type)
		{
			case SDL_QUIT:
				bDone = true;
				break;

			case SDL_ACTIVEEVENT:

				if (event.active.gain == 1)
					OnActivate();
				else
					OnDeactivate();
				break;

			case SDL_VIDEORESIZE:
				{
					SDL_ResizeEvent	*resize = (SDL_ResizeEvent*)&event;
					OnSizeOSWindow ( resize->w, resize->h);
				}
				break;
		}
		
		CEventManager::instance().callEventHandaler(event.type,&event);

		if (!bDone)
			bDone = GameLoop();	
	}
	if (!bDone)
		bDone = GameLoop();

	return bDone;
}

bool CBaseGameLoop::DoOneEventLoop ( void )
{
	SDL_Event event;

	SDL_PollEvent(&event);

	switch(event.type)
	{
	case SDL_QUIT:
		return true;

	case SDL_ACTIVEEVENT:

		if (event.active.gain == 1)
			OnActivate();
		else
			OnDeactivate();
		break;

	case SDL_VIDEORESIZE:
		{
			SDL_ResizeEvent	*resize = (SDL_ResizeEvent*)&event;
			OnSizeOSWindow ( resize->w, resize->h);
		}
		break;
	}

	CEventManager::instance().callEventHandaler(event.type,&event);

	return false;
}

// provide your own version of these
int	CBaseGameLoop::PreInitOSWin ( void )
{
	return 0;
}

int	CBaseGameLoop::OnInitGame ( void )
{
	return 0;
}

int CBaseGameLoop::OnKillGame ( void )
{
	return 0;
}

bool CBaseGameLoop::OnLoadGame ( void )
{
	return false;
}

int	CBaseGameLoop::OnActivate ( void )
{
	DisplayListManager::instance().rebuildAll();
	return 0;
}

int CBaseGameLoop::OnDeactivate ( void )
{
	DisplayListManager::instance().releaseAll();
	return 0;
}

bool CBaseGameLoop::GameLoop ( void )
{
	return 0;
}

int CBaseGameLoop::OnSizeOSWindow ( int cx, int cy )
{
	C3dView::instance().SizeGlWindow(0,cx,cy);
	return 0;
}




