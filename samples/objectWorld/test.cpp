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
// test.cpp : Defines the entry point for the application.
//
#include <stdlib.h>
#include <stdio.h>
#include <memory.h>
#include <string.h>
#include <time.h>

#include "test.h"
#include "OGF.h"

CLGETime						&oTime = CLGETime::instance();
C3dView						&oEngine = C3dView::instance();
CTextureManager			&oTextureManager = CTextureManager::instance();
CFontManager				&oFontManager = CFontManager::instance();
CInputMananger			&input = CInputMananger::instance();

#define _CUR_PREFS_VERSION 1
     
#ifdef _WIN32
int APIENTRY WinMain(HINSTANCE hInstance,HINSTANCE hPrevInstance,LPSTR lpCmdLine,int nCmdShow)
#else
int main ( void )
#endif // WIN32
{
	// init the random number generator
#ifdef _WIN32
	srand(GetTickCount());
#else
	srand(time((time_t *)NULL));
#endif

	// get the loop handaler
	CGameLoop oGameLoop;

	if (oGameLoop.PreInitOSWin()!= 0)
		return -1;

	if (oGameLoop.OnInitGame ()!= 0)
		return -1;

	while (!oGameLoop.ProcessEvents());

	return oGameLoop.OnKillGame();
}

// our game loop
CGameLoop::CGameLoop()
{
}

CGameLoop::~CGameLoop()
{
}

int	CGameLoop::PreInitOSWin ( void )
{	
	//Run in a window
	checkPrefs();

	m_bFullScreen =  prefs.GetItemI("display:fullscreen") != 0;

	// put the window at 100,100
	m_iXOSWindowPos = 100;
	m_iYOSWindowPos = 100;

	// 640x480
	m_iXOSWindowSize = prefs.GetItemI("display:res:x");
	m_iYOSWindowSize = prefs.GetItemI("display:res:y");

	// if your data dir is not relitive to the app path then put the path here.
	SetOSFileBaseDir("");
	
	return 0;
}

void CGameLoop::checkPrefs ( void )
{
	// read in the prefs
	prefs.Init("ogf/objectWorld/config");

	int version = prefs.GetItemI("version");

	switch (version)
	{
		case 0:
			prefs.SetItem("display:res:x",800);
			prefs.SetItem("display:res:y",600);
			prefs.SetItem("display:fullscreen",0);
			prefs.SetItem("visual:camera:firstperson_mode",1);
			prefs.SetItem("input:mouse:sensitivity",0.1f);
		case 1:
			break;
	}

	prefs.SetItem("version",_CUR_PREFS_VERSION);

	// set the data
	bFirstPerson = prefs.GetItemI("visual:camera:firstperson_mode") != 0;
	mouseSenstivity = prefs.GetItemF("input:mouse:sensitivity");
	if (mouseSenstivity <= 0.0f)
		mouseSenstivity = 0.1f;
}

int CGameLoop::bindAction ( char *action, char *defaultDevice, char* defaultItem, bool poll )
{
	if ( !action || !defaultDevice || !defaultItem )
		return -1;

	std::string realDevice = defaultDevice;
	std::string realItem = defaultItem;

	std::string configItemKey = "binding:input:";
	configItemKey += action;
	if (prefs.ItemExists(configItemKey.c_str()))
	{
		std::string configLine = prefs.GetItemS(configItemKey.c_str());
		std::vector<std::string> configParams = string_util::tokenize(configLine,std::string(","));
		if (configParams.size() >=2)
		{
			realDevice = configParams[0];
			realItem = configParams[1];
		}
	}

	int command = input.Bind(action,realDevice.c_str(),realItem.c_str());

	if (poll)
		input.SetPollCommand(command);

	return command;
}

void CGameLoop::bindInputs ( void )
{
	// make with the binding
	inputBindings.quit = bindAction("ESCAPE",_KEYBOARD,"ESCAPE",false);
	inputBindings.viewChange = bindAction("VIEW_TOGGLE",_KEYBOARD,"F2",false);
	inputBindings.screenshot = bindAction("SCREENSHOT",_KEYBOARD,"F5",false);

	inputBindings.moveForward = bindAction("FORWARD",_KEYBOARD,"W");
	inputBindings.moveBackwards = bindAction("BACKWARD",_KEYBOARD,"S");
	inputBindings.moveLeft = bindAction("LEFT",_KEYBOARD,"A");
	inputBindings.moveRight = bindAction("RIGHT",_KEYBOARD,"D");
	inputBindings.moveUp = bindAction("UP",_KEYBOARD,"PAGEUP");
	inputBindings.moveDown = bindAction("DOWN",_KEYBOARD,"PAGEUP");

	inputBindings.lookUp = bindAction("LOOK_UP",_KEYBOARD,"UP");
	inputBindings.lookDown = bindAction("LOOK_DOWN",_KEYBOARD,"DOWN");
	inputBindings.lookLeft = bindAction("LOOK_LEFT",_KEYBOARD,"LEFT");
	inputBindings.lookRight = bindAction("LOOK_RIGHT",_KEYBOARD,"RIGHT");

	inputBindings.jump = bindAction("JUMP",_KEYBOARD,"TAB");

	inputBindings.move3rdPerson = bindAction("MOVE_THIRD_PERSON",_MOUSE,"WHEEL");
}

int	CGameLoop::OnInitGame ( void )
{
	// set up the time system
	oTime.Init();
	input.Init();

	bindInputs();

	// engine init
	// set the  view depth from 1 to 2500 units
	oEngine.SetDepthRange(1,2500.0f);

	// make our engine.
	oEngine.setWindowTitle("OGF::Sample::ObjectWorld");
	oEngine.Create(m_iXOSWindowSize,m_iYOSWindowSize,m_bFullScreen);

	// black is the coolest of colors
	oEngine.SetBGColor(0.125f,0.125f,0.125f);
	
	// set up a preset camera mode
	oEngine.SetViewType(eFirstPerson);
	oEngine.RotateView(0,0,0);
	oEngine.PanView(0,-20,5);
//	m_oEngine.MovePullBack(20);

	// Load the basic texures
	oTextureManager.LoadAll("data/textures");

	// load the textures for the fonts
	oTextureManager.LoadAll("data/fonts");

	// load the textures for the models
	oTextureManager.LoadAll("data/models",true);

	// load the textures for the skyboxes
	oTextureManager.LoadAll("data/skyboxes",false);

	// tell the draw list to init now that all posible textures are known
	oEngine.GetDrawList().Init();

	// get the ID for the ground
	groundTexture = oTextureManager.GetID("data/textures/ground");

	// load all the fonts
	oFontManager.LoadAll("data/fonts");

	// have the model manager scan the data dir for all models
	modelManager.LoadAll("data/models");

	registerDefaultFactories(world);
	world.load("data/maps/simple.omap");

	input.HideOSCursor(true);
	input.SetMouseRelitiveMode(true);

	// setup a light
	oEngine.SetLightInfo(0,eLightPos,250,300,500,1);
	initPlayer();

	lastScreenShot = 0;
	return 0;
}

int CGameLoop::OnKillGame ( void )
{
	// to be called when we die
	input.HideOSCursor(false);
	oEngine.Kill();
	oTextureManager.Flush();
	return 0;
}

int	CGameLoop::OnActivate ( void )
{
	// when we get foucs
	return 0;
}

int CGameLoop::OnDeactivate ( void )
{
	// when we lose focus
	return 0;
}

bool CGameLoop::OnLoadGame ( void )
{
	return false;
}

bool CGameLoop::initPlayer ( void )
{
	player.eyeHeight = 2.0f;
	player.thridPerson = false;

	// give us a bit of a boost
	player.pos.SetZ(10.0f);

	// set the world constants
	phisicalConstants.rotSpeed = 90.0f;
	phisicalConstants.linSpeed = 50.0f;
	return true;
}

void CGameLoop::updatePlayerPos ( void )
{
	float frameTime = (float)oTime.GetFrameTime();
	SimpleGameWorld::trWorldPhisicalInfo	&phisicals =world.getPhisicals();
	float airControl = 0.5f;	// how much we can do when jumping

	//ok lets see what we want to do
	trPlayerData	newPlayerState = player;

	// take us to our new rotation
	newPlayerState.rot += playerInput.rotation * frameTime;

	bool noThrust = ( playerInput.linear.X() == 0.0f) && ( playerInput.linear.Y() == 0.0f);	// see if we arn't moving at all;

	//found out what direction in worldspace we want to go.
	float dx = 0,dy = 0;
	if (!noThrust)
	{
		// compute vector from forward/backward facing
		dx = -sin(newPlayerState.rot.Z()*fastRad_con) * playerInput.linear.X();
		dy = cos(newPlayerState.rot.Z()*fastRad_con) * playerInput.linear.X();

		// just do a swap here instead of this crap
		// when you can think like a terran
		dx += -sin((newPlayerState.rot.Z()+90)*fastRad_con) * playerInput.linear.Y();
		dy += cos((newPlayerState.rot.Z()+90)*fastRad_con) * playerInput.linear.Y();
	}

	if ( player.state == eOnGround)	// ok we are on the ground, so we just set our vector to the input vector, even if it's nothing ( 100% friction )
	{
		newPlayerState.vel.SetX(dx);
		newPlayerState.vel.SetY(dy);
	}
	else	// we are flying high so dampen the input and merge it in, don't just set it
	{
		newPlayerState.vel.SetX( ((dx*airControl*frameTime)+newPlayerState.vel.X()));
		newPlayerState.vel.SetY( ((dy*airControl*frameTime)+newPlayerState.vel.Y()));
		if (newPlayerState.vel.GetMag2D() > phisicalConstants.linSpeed)	// make sure we just don't keep goint TOO fast
			newPlayerState.vel.SetMag2D(phisicalConstants.linSpeed);
	}

	// compute our new intended XY state
	newPlayerState.pos.IncrementX(newPlayerState.vel.X()*frameTime);
	newPlayerState.pos.IncrementY(newPlayerState.vel.Y()*frameTime);

	// compute our Z velocity
	if ( player.state == eOnGround)	// ok we are on the ground, so this is the only time it's cool to jump
	{
		float jumpTol = 0.1f;
		float	jumpVel = 25.0f;

		// see if we need to jump
		if ( playerInput.jump && (newPlayerState.pos.Z() < (phisicals.groundLimit + jumpTol)) )
			newPlayerState.vel.SetZ(jumpVel);
	}
	else // falling, jumping, landing, something.....
	{	
		// apply the gravity
		newPlayerState.vel.IncrementZ(phisicals.gravity * frameTime);
		if ( newPlayerState.vel.Z() < phisicals.maxFallVel )
			newPlayerState.vel.SetZ(phisicals.maxFallVel);
	}

	newPlayerState.pos.IncrementZ(newPlayerState.vel.Z()*frameTime);

	// ok so we know where we want to be
	// time to make sure that's a VALID state

	// clamp the look up to +- 45, since we can't make it TOO loopy
	float lookLimit = 45.0f;
	newPlayerState.rot.LimitX(-lookLimit,lookLimit);

	// simple limit to keep us inside the map in X and Y
	newPlayerState.pos.LimitX(-phisicals.size,phisicals.size);
	newPlayerState.pos.LimitY(-phisicals.size,phisicals.size);

	if ( newPlayerState.pos.Z() < phisicals.groundLimit) // see if we've gone under the ground
	{
		newPlayerState.pos.SetZ(phisicals.groundLimit);
		newPlayerState.vel.SetZ(0);
		newPlayerState.state = eOnGround;
	}

	switch (newPlayerState.state)
	{
		default:
		case eOnGround:
			if ( newPlayerState.vel.Z() >0 )
				newPlayerState.state = eJumping;
			else if ( newPlayerState.vel.Z() < 0)
				newPlayerState.vel.SetZ(0);
			break;

		case eJumping:	// see if we are not going UP any more
			if ( newPlayerState.vel.Z() <= 0 ) 
				newPlayerState.state = eFalling;
			break;

		case eFalling:
			if ( newPlayerState.vel.Z() > 0 ) 
				newPlayerState.state = eJumping;
	}

	player = newPlayerState;
}

bool CGameLoop::getPlayerInput ( tvPlayerInput &inputDef )
{
	// get the mouse pos
	// todo, fix this for a poll event, for some reason they don't work
	int intMouse[2];
	input.GetMousePos(intMouse[0],intMouse[1],false);

	inputDef.jump = false;
	inputDef.shoot = false;
	inputDef.linear.Set(0,0,0);
	inputDef.rotation.Set(0,0,0);

	float rotModifyer[2] = {0,0};

	float togleLimit = 0.0125f;

	// left/right
	if (input.PollState(inputBindings.lookLeft) > togleLimit)
		rotModifyer[0] = phisicalConstants.rotSpeed*input.PollState(inputBindings.lookLeft);
	else if (input.PollState(inputBindings.lookRight) > togleLimit)
		rotModifyer[0] = -phisicalConstants.rotSpeed*input.PollState(inputBindings.lookRight);
	else
		rotModifyer[0] = (float)-intMouse[0]*mouseSenstivity*phisicalConstants.rotSpeed;

	// up/down
	if (input.PollState(inputBindings.lookUp) > togleLimit)
		rotModifyer[1] = phisicalConstants.rotSpeed*input.PollState(inputBindings.lookUp);
	else if (input.PollState(inputBindings.lookDown) > togleLimit)
		rotModifyer[1] = -phisicalConstants.rotSpeed*input.PollState(inputBindings.lookDown);
	else
		rotModifyer[1] = (float)-intMouse[1]*mouseSenstivity*phisicalConstants.rotSpeed;

	inputDef.rotation.Set(rotModifyer[1],0,rotModifyer[0]);

	float linModifyer[3] = {0,0,0};

	if (input.PollState(inputBindings.moveForward) > togleLimit)
		linModifyer[0] = phisicalConstants.linSpeed*input.PollState(inputBindings.moveForward);
	else if (input.PollState(inputBindings.moveBackwards) > togleLimit)
		linModifyer[0] = -phisicalConstants.linSpeed*input.PollState(inputBindings.moveBackwards);

	if (input.PollState(inputBindings.moveLeft) > togleLimit)
		linModifyer[1] = phisicalConstants.linSpeed*input.PollState(inputBindings.moveLeft);
	else if (input.PollState(inputBindings.moveRight) > togleLimit)
		linModifyer[1] = -phisicalConstants.linSpeed*input.PollState(inputBindings.moveRight);

	if (input.PollState(inputBindings.moveUp) > togleLimit)
		linModifyer[2] = phisicalConstants.linSpeed*input.PollState(inputBindings.moveUp);
	else if (input.PollState(inputBindings.moveDown) > togleLimit)
		linModifyer[2] = -phisicalConstants.linSpeed*input.PollState(inputBindings.moveDown);

	inputDef.linear.Set(linModifyer[0],linModifyer[1],linModifyer[2]);

	inputDef.jump = input.PollState(inputBindings.jump) > togleLimit;

	return true;
}

bool CGameLoop::UpdateInput ( void )
{
	// get the key input
	// return true if it's time to quit.
	input.Update();

	if (input.GetEvents() >0)
	{
		while (input.GetNextEvent())
		{
			int id = input.GetEventID();

			if (id ==  inputBindings.quit)	// bail
				return true;
			else if (id == inputBindings.viewChange)	// see if we need to swap modes
			{
				// do something
			}
			else if (id == inputBindings.screenshot)
				oTextureManager.SaveScreenCapture(string_util::format("screenshot-%d",lastScreenShot++).c_str());
		}
	}

	getPlayerInput(playerInput);
	
	return false;
}

bool CGameLoop::GameLoop ( void )
{
	// game loop called onece for each frame
	// return true if it's time to quit

	// update the time for this frame
	oTime.Update();
	// get out input
	if (UpdateInput())
		return true;

	updatePlayerPos();

	world.think((float)oTime.GetFrameTime(),(float)oTime.GetTime());

	Draw();
	// do some stuff

	return false;
}

void CGameLoop::setCameraToPlayer ( void )
{
	oEngine.PanView(player.pos.X(),player.pos.Y(),player.pos.Z()+player.eyeHeight);
	if ( player.thridPerson != (oEngine.GetViewType() == eThirdPerson) )
	{
		//need to change the camera
		oEngine.SetViewType(player.thridPerson ? eThirdPerson : eFirstPerson );
		oEngine.MovePullBack(player.pullback);
	}
	else if (player.thridPerson )
		oEngine.MovePullBack(player.pullback);

	oEngine.RotateView(player.rot.X(),player.rot.Y(),player.rot.Z());
}

void CGameLoop::Draw ( void )
{
	// set the camera to where our player is
	setCameraToPlayer();

	// clear out the draw list
	oEngine.GetDrawList().PreDraw();

	// now draw some stuff
	// init the engine for drawing
	// and load up the current camera positions.
	// in first and thrid person modes, LGE goes into a normal XYZ mode
	// where Z is "UP"
	if (oEngine.BeginDraw())
	{
		// tell everything that has to draw to register stuff with the draw list
		world.vis(oEngine.GetFrustum());

		DrawWorld();

		// draw everything that is in the draw list
		oEngine.GetDrawList().Draw();

		//swap to ortho mode to do some hud type drawing
		oEngine.BeginOverlay();	
		DrawHud();
		oEngine.EndOverlay();

		// tell the engine to finish drawing and display the image
		oEngine.EndDraw();
	}
}

void CGameLoop::DrawWorld ( void )
{
	// have the engine set up the default light
	oEngine.UpdateLights();

	world.draw();
}

void CGameLoop::DrawHud ( void )
{
	glColor4f(0,0,0,0.5f);
	glBegin(GL_QUADS);
	glVertex3f(0,0,-1);
	glVertex3f((float)oEngine.GetWinH(),0,-1);
	glVertex3f((float)oEngine.GetWinH(),60,-1);
	glVertex3f(0,60,-1);
	glEnd();
	glColor4f(1,1,1,1);

	glEnable(GL_TEXTURE_2D);

	char temp[512];
	sprintf(temp,"pos: x %f, y %f",oEngine.GetCurrentViewPosition(eXAxis),oEngine.GetCurrentViewPosition(eYAxis));
	CColor	white(1,1,1,1);
	oFontManager.DrawString(10.0f,20.0f,-0.5f,white,"LuxiMono",16,temp);
}

int CGameLoop::OnSizeOSWindow ( int cx, int cy )
{
	// just call the base class
	return CBaseGameLoop::OnSizeOSWindow(cx,cy);
}
