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
// Test.h

#ifndef _TEST_H_
#define  _TEST_H_

#include "OGF.h"
#include "world.h"

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

	// player state
	typedef struct 
	{	
		C3DVertex	linear;
		C3DVertex	rotation;
		bool		jump;
		bool		shoot;
	}tvPlayerInput;

	tvPlayerInput	playerInput;

	bool initPlayer ( void );
	void updatePlayerPos ( void );

	// input
	bool getPlayerInput ( tvPlayerInput &inputDef );

	bool UpdateInput ( void );

	void bindInputs ( void );

	// check the config to see if the action has a binding, or use the default
	int bindAction ( char *action, char *defaultDevice, char* defaultItem, bool poll = true );

	// drawing
	void setCameraToPlayer ( void );
	void Draw ( void );
	void DrawWorld ( void );
	void DrawHud ( void );

	void checkPrefs ( void );

	CModelManager		modelManager;
	int					escKey;

	SimpleGameWorld	world;
	int groundTexture;

	bool				bFirstPerson;

	int					viewToggle;

	typedef struct 
	{
		float rotSpeed;
		float linSpeed;
	}trPhisicalConstants;

	trPhisicalConstants	phisicalConstants;

	typedef enum
	{
		eOnGround,
		eFloating,
		eLanding,
		eFalling,
		eJumping
	}tePlayerStates;

	typedef struct 
	{
		C3DVertex		pos;
		C3DVertex		vel;
		C3DVertex		rot;
		float			eyeHeight;
		bool			thridPerson;
		float			pullback;
		tePlayerStates	state;		
	}trPlayerData;

	trPlayerData	player;

	typedef	struct 
	{
		// player actions
		int			moveLeft;
		int			moveRight;
		int			moveForward;
		int			moveBackwards;
		int			moveUp;
		int			moveDown;
		int			lookLeft;
		int			lookRight;
		int			lookUp;
		int			lookDown;
		int			jump;
		int			shoot;

		// inteface actions;
		int			quit;
		int			screenshot;
		int			viewChange;
		int			move3rdPerson;
	}trInputBindings;
	
	trInputBindings inputBindings;

	// config
	CPrefsManager	prefs;
	float			mouseSenstivity;

	//screenshot stuff
	int				lastScreenShot;
};

#endif // _TEST_H_
