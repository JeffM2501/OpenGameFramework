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
// Game Loop.h

#ifndef _GAME_LOOP_H_
#define _GAME_LOOP_H_

// main stuff
// the app needs to provide a derived CBaseGameLoop
// as the base on will just quit out

class CBaseGameLoop
{
public:
	CBaseGameLoop();
	virtual ~CBaseGameLoop();

	bool ProcessEvents ( void );

	virtual int	PreInitOSWin ( void );
	virtual int	OnInitGame ( void );
	virtual int OnKillGame ( void );

	virtual bool OnLoadGame ( void ); // after init and after menus

	virtual int	OnActivate ( void );
	virtual int OnDeactivate ( void );

	virtual bool GameLoop ( void );

	virtual int OnSizeOSWindow ( int cx, int cy );

	// just run thru one event loop, return true if we have to bail
	// this is mostly used when your in the middle of some sort of
	// blocking type call, and you just want to keep the messages flowing
	// like when doing a web update;
	bool DoOneEventLoop ( void ); 
	
	int	m_iXOSWindowPos;
	int	m_iYOSWindowPos;

	int	m_iXOSWindowSize;
	int	m_iYOSWindowSize;

	bool m_bFullScreen;
};

#endif //_GAME_LOOP_H_

