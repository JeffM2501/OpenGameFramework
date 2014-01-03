/* 
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
// simple.h

#ifndef _TEST_H_
#define  _TEST_H_

// ??? is this supposed to be here?
#include "OGF.h"

class CGameLoop : public CBaseGameLoop
{
public:
	CGameLoop();
	~CGameLoop();

	virtual int	PreInitOSWin ( void );
	virtual int	OnInitGame ( void );
	virtual int OnKillGame ( void );

	virtual bool OnLoadGame ( void ); // after init and after menus

	virtual int	OnActivate ( void );
	virtual int OnDeactivate ( void );

	virtual bool GameLoop ( void );

	virtual int OnSizeOSWindow ( int cx, int cy );
	
	int	m_iXOSWindowPos;
	int	m_iYOSWindowPos;

	int	m_iXOSWindowSize;
	int	m_iYOSWindowSize;

	bool m_bFullScreen;

protected:
	bool UpdateInput ( void );

	void Draw ( void );
	void DrawWorld ( void );
	void DrawHud ( void );

	float m_fSpinAngle;

	float m_fTiltAngle;

	int		m_iSpinDir;

	CModelManager		modelManager;
	CAnimModel			*shotgun;
	int					escKey;
	int					skinID;

	int groundTexture;

	float				mouse[2];

	// ball physics
	float	ballZPos;
	float	ballZVel;
};

#endif // _TEST_H_



