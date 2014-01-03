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

#include "simple.h"

CLGETime						&oTime = CLGETime::instance();
C3dView						&oEngine = C3dView::instance();
CTextureManager			&oTextureManager = CTextureManager::instance();
CFontManager				&oFontManager = CFontManager::instance();
CInputMananger			&input = CInputMananger::instance();

     
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
	m_bFullScreen = false;

	// put the window at 100,100
	m_iXOSWindowPos = 100;
	m_iYOSWindowPos = 100;

	// 640x480
	m_iXOSWindowSize = 640;
	m_iYOSWindowSize =480;

	// if your data dir is not relitive to the app path then put the path here.
	SetOSFileBaseDir("");
	
	return 0;
}

int	CGameLoop::OnInitGame ( void )
{
	// set up the time system
	oTime.Init();

	input.Init();

	// make with the binding
	escKey = input.Bind("ESCAPE","KEYBOARD","ESCAPE");
	input.Bind("SCREENSHOT","KEYBOARD","F5");

	input.SetPollCommand(input.Bind("XPOS","KEYBOARD","A"));
	input.SetPollCommand(input.Bind("XNEG","KEYBOARD","D"));

	input.SetPollCommand(input.Bind("FORWARD","KEYBOARD","W"));
	input.SetPollCommand(input.Bind("BACKWARD","KEYBOARD","S"));
	input.SetPollCommand(input.Bind("LEFT","KEYBOARD","A"));
	input.SetPollCommand(input.Bind("RIGHT","KEYBOARD","D"));

	input.SetPollCommand(input.Bind("LOOK_UP","KEYBOARD","UP"));
	input.SetPollCommand(input.Bind("LOOK_DOWN","KEYBOARD","DOWN"));
	input.SetPollCommand(input.Bind("LOOK_LEFT","KEYBOARD","LEFT"));
	input.SetPollCommand(input.Bind("LOOK_RIGHT","KEYBOARD","RIGHT"));

	input.SetPollCommand(input.Bind("WHEEL","MOUSE","WHEEL"));
	// engine init
	// set the  view depth from 1 to 2500 units
	oEngine.SetDepthRange(1,2500.0f);
	// make our engine.
	oEngine.setWindowTitle("SGL super cool test app");
	oEngine.Create(m_iXOSWindowSize,m_iYOSWindowSize,m_bFullScreen);

	// black is the coolest of colors
	oEngine.SetBGColor(0.0f,0.0f,0.25f);
	
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

	// get the ID for the ground
	groundTexture = oTextureManager.GetID("data/textures/ground");

	// load all the fonts
	oFontManager.LoadAll("data/fonts");

	// have the model manager scan the data dir for all models
	modelManager.LoadAll("data/models");

	// load up the shotgun
	shotgun = modelManager.GetModel("data/models/shotgun/shotgun");

	// it's REALY big so lets scale it down for our test
	trVertex3D	scale;
	scale.x =scale.y = scale.z = 0.005f;
	shotgun->SetScale(scale);

	// build up a list of textures for a base "skin"
	// this model has 2 meshes, so add in both names in order.
	std::vector<std::string>	textures;
	textures.push_back(std::string("data/models/shotgun/shotgun1"));
	textures.push_back(std::string("data/models/shotgun/shotgun2"));

	// we don't need any fancy multipass skins so just make a simple skin from the texture name list
	skinID = shotgun->NewSimpleSkin(textures);

	m_fSpinAngle = 0;
	m_iSpinDir = 1;
	m_fTiltAngle = 0;

	ballZPos = 0;
	ballZVel = 10;

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

			if (id ==  escKey)	// bail
				return true;

			if (id == input.GetCommandID("SCREENSHOT"))
				oTextureManager.SaveScreenCapture("screenshot");
		}
	}

	// get the mouse pos
	// todo, fix this for a poll event, for some reason they don't work
	int intMouse[2];
	input.GetMousePos(intMouse[0],intMouse[1],false);
	mouse[0] = (float)intMouse[0];
	mouse[1] = (float)intMouse[1];

	// compute some speeds
	oEngine.RotateView(0,0,0);
	oEngine.PanView(0,-20,5);

	if (input.PollState("XPOS") > 0.25f)
		m_iSpinDir = 1;
	else if (input.PollState("XNEG") > 0.25f)
		m_iSpinDir = -1; 

	m_fTiltAngle = mouse[1]*0.125f;

	float wheel = input.PollState("WHEEL");

	if (wheel != 0)
		m_iSpinDir = (int)wheel;
	

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

	float frameTime = (float)oTime.GetFrameTime();
	float gravity = 5.0f;

	// do some simple physics
	// bounce the ball
	ballZPos += frameTime * ballZVel;
	ballZVel -= frameTime * gravity;

	if (ballZPos < 0)	// boing
	{
		ballZPos = 0;
		ballZVel = 10.0f;
	}

	// now increment the spin angle based on how long it's been since the last frame;
	m_fSpinAngle += m_iSpinDir * (float)oTime.GetFrameTime() *45.0f; // we want 45 degs a sec.

	Draw();
	// do some stuff

	return false;
}

void CGameLoop::Draw ( void )
{
	// now draw some stuff
	// init the engine for drawing
	// and load up the current camera positions.
	// in first and thrid person modes, LGE goes into a normal XYZ mode
	// where Z is "UP"
	if (oEngine.BeginDraw())
	{
		DrawWorld();

		oEngine.BeginOverlay();	//swap to ortho mode to do some hud type drawing
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
	
	// draw a bouncing ball

	float ballRad = 1.0f;

	glPushMatrix();
	glTranslatef(0,0,ballZPos+ballRad);

	glDisable(GL_TEXTURE_2D);
	glEnable(GL_LIGHTING);

	glColor4f(1,0,0,1);
	gluSphere(gluNewQuadric(),ballRad,10,10);
	glColor4f(1,1,1,1);

	glPopMatrix();


	// make some spin in the object.
	// save off the world matrix so we can get back to it.
	// this pushes the matrix onto a stack
	glPushMatrix();

	// load up a texture
	// textures are accessed by unix style path names
	// with out an extension.
	if (groundTexture != -1)	// make sure it's a good one
	{
		// turn on textureing and lighting
		glEnable(GL_TEXTURE_2D);
		glEnable(GL_LIGHTING);

		// have OpenGL set the texture as the current active texture
		oTextureManager.Bind(groundTexture);

		// draw the ground
		glBegin(GL_QUADS); // quads have 4 sides;

		// set the normal for the quad
		// since the quad is flat we only need one normal for the quad
		// normals should be of unit lenght
		glNormal3f(0,0,1);

		// now set our 4 verts for the quad,
		// the quad is "standing up" so it will face the camera so it will have X and Z coordinates.
		// each vertex has an XYZ postion and a UV texture coordinate to tell how to
		// map the texture image to the face of the quad. UVs are from 0 to 1.0 regardless of the textures size.

		// Open GL is a state machine so you have to define all the data for a vertex before the call to glVertex

		// the order of the verts is used to determine what side of the polygon is shown, or the "front"
		// the default mode is CCW.
		// you will notice that you can only see one side of the quad

		// upper left corner

		float size = 100.0f;
		float uvScale = 0.25f;

		float zDepth = -1;

		glTexCoord2f(0,uvScale*size);
		glVertex3f(-size,size,zDepth);

		// lower left corner
		glTexCoord2f(0,0);
		glVertex3f(-size,-size,zDepth);

		// lower right
		glTexCoord2f(uvScale*size,0);
		glVertex3f(size,-size,zDepth);

		// upper right
		glTexCoord2f(uvScale*size,uvScale*size);
		glVertex3f(size,size,zDepth);

		glEnd();
	}

	glTranslatef(6,5,0);
	// now do some rotation
	glRotatef(m_fSpinAngle,0,0,1); // rotate around the Z axis
	glRotatef(m_fTiltAngle,1,0,0); // rotate around the Z axis
	// load up a texture
	// textures are accessed by unix style path names
	// with out an extension.
	int iTexture = oTextureManager.GetID("data/models/Test");

	if (iTexture != -1)	// make sure it's a good one
	{
		// turn on textureing and lighting
		glEnable(GL_TEXTURE_2D);
		glEnable(GL_LIGHTING);

		// have OpenGL set the texture as the current active texture
		oTextureManager.Bind(iTexture);
	}

	// set the current color, this will be applied to all verts till it's changed
	// white with 100% alpha
	glColor4f(1,1,1,1);
	// now draw a spining quad
	glBegin(GL_QUADS); // quads have 4 sides;

	// set the normal for the quad
	// since the quad is flat we only need one normal for the quad
	// normals should be of unit lenght
	glNormal3f(0,-1,0);

	// now set our 4 verts for the quad,
	// the quad is "standing up" so it will face the camera so it will have X and Z coordinates.
	// each vertex has an XYZ postion and a UV texture coordinate to tell how to
	// map the texture image to the face of the quad. UVs are from 0 to 1.0 regardless of the textures size.

	// Open GL is a state machine so you have to define all the data for a vertex before the call to glVertex

	// the order of the verts is used to determine what side of the polygon is shown, or the "front"
	// the default mode is CCW.
	// you will notice that you can only see one side of the quad

	// upper left corner
	glTexCoord2f(0,1);
	glVertex3f(-5,0,10);

	// lower left corner
	glTexCoord2f(0,0);
	glVertex3f(-5,0,0);

	// lower right
	glTexCoord2f(1,0);
	glVertex3f(5,0,0);

	// upper right
	glTexCoord2f(1,1);
	glVertex3f(5,0,10);


	//Same thing but backwards
	glNormal3f(0,1,0);

	// upper right
	glTexCoord2f(0,1);
	glVertex3f(5,0,10);

	// lower right
	glTexCoord2f(0,0);
	glVertex3f(5,0,0);

	// lower left corner
	glTexCoord2f(1,0);
	glVertex3f(-5,0,0);

	// upper left corner
	glTexCoord2f(1,1);
	glVertex3f(-5,0,10);

	// end the quad.
	// for every 4 verts gl will make a quad
	glEnd();

	// put the matrix back
	glPopMatrix();

	// new pop for model
	glPushMatrix();

	// move it back and over
	glTranslatef(-2,0,5);
	glRotatef(-m_fSpinAngle,0,0,1); // rotate around the Z axis
	glRotatef(-m_fTiltAngle,0,1,0); // rotate around the Z axis

	//draw the shotgun, with the skin, from frame0 to frame 0 at param 0
	shotgun->Draw(skinID,0,0,0);
	glPopMatrix();
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
	sprintf(temp,"mouse: x %f, y %f\npos: x %f, y %f",mouse[0],mouse[1],oEngine.GetCurrentViewPosition(eXAxis),oEngine.GetCurrentViewPosition(eYAxis));
	CColor	white(1,1,1,1);
	oFontManager.DrawString(10.0f,30.0f,-0.5f,white,"LuxiMono",16,temp);
}


int CGameLoop::OnSizeOSWindow ( int cx, int cy )
{
	// just call the base class
	return CBaseGameLoop::OnSizeOSWindow(cx,cy);
}
