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

// LGE.cpp : Defines the entry point for the DLL application.
//

#include "3dView.h"
#include <math.h>

#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <vector>

#include "SDL.h"

// initialize the singleton
template <>
C3dView* Singleton<C3dView>::_instance = (C3dView*)0;

// make this a var?
#define		_SEL_BUFFER_SIZE	20000

void C3dView::SetDrawBack ( void (*call)(void*), void *cookie)
{
	m_DrawBack = call;
	m_DrawCookie = cookie;
}

void C3dView::CallDrawBack ( void )
{
	if (m_DrawBack)
		(*m_DrawBack)(m_DrawCookie);
}

struct C3dView::trLGEInfo
{
	std::vector<CScreenRes>	modes;
	unsigned int				curentMode;
};

C3dView::C3dView( void )
{	
	// init everything to null and/or some defluat
	fullScreenWindow = false;
	resizeableWindow = false;
	showWindowCaption = true;

	info = new trLGEInfo;
	info->curentMode = 0;
	m_iWinX = -1;
	m_iWinY = -1;
	m_fov = 40.0;

	m_iViewportX = 4;
	m_iViewportY = 3;
	m_fAspect = 1.333333333333333f;

	// white
	m_BackGroundColor.r = 1.0f;
	m_BackGroundColor.g = 1.0f;
	m_BackGroundColor.b = 1.0f;

	m_bDrawing = false;
	m_bOverlayMode = false;

	m_DrawCookie = NULL;
	m_DrawBack = NULL;

	m_aViewPos[0] = 0.0f;
	m_aViewPos[1] = 0.0f;
	m_aViewPos[2] = 0.0f;
	m_aViewRot[0] = 0.0f;
	m_aViewRot[1] = 0.0f;
	m_aViewRot[2] = 0.0f;

	m_fPullBack = 10.0f;

	m_eViewMode = eIdenityXY;

	m_uiSelectBuf = (GLuint*)malloc(sizeof(GLuint)*_SEL_BUFFER_SIZE);

	m_iDepthBuffer = 32;
	m_iDepthBuffer = 32;
	m_iStencelBuffer = 0;
	m_iAlphaBuffer = 0;
	m_iAuxBuffer = 0;

	m_xNearZ = _NearClip;
	m_xFarZ = _FarClip;

	m_bInited = false;

	for ( int i = 0; i < 8; i++ )
		glLights[i].enabled = false;

	ResetFullScreenModes();
}

C3dView::~C3dView( void )
{
	if (m_uiSelectBuf)
		free(m_uiSelectBuf);
	Kill();
	if (info)
		delete(info);
}

bool C3dView::GetNextFullScreenMode ( CScreenRes &res )
{
	if (!info->modes.size())
	{
		/// build a list of modes;
		SDL_Rect	**modes = SDL_ListModes(NULL,SDL_OPENGL|SDL_FULLSCREEN);

		if (!modes || (modes == (SDL_Rect **)-1))
			return false;

		CScreenRes	res;

		for(int i=0;modes[i];++i)
		{
			res.h = modes[i]->w;
			res.v = modes[i]->h;
			info->modes.push_back(res);
		}
	}

	if (info->curentMode >= info->modes.size())
	{
		info->curentMode = 0;
		return false;
	}

	res = info->modes[info->curentMode++];
	return true;
}

void C3dView::ResetFullScreenModes ( void )
{
	info->curentMode = 0;
	info->modes.clear();
}

void C3dView::setWindowTitle( const char* title )
{
	if (title)
		windowTitle = title;
	else
		windowTitle = "";

	if (m_bInited && showWindowCaption && windowTitle.size())
		SDL_WM_SetCaption(windowTitle.c_str(),NULL);
}

// creates the MSGL window and it's OpenGL contexts
bool C3dView::Create ( int x, int y, bool fullScreen, bool resizeable, bool caption )
{
	if (!SDL_WasInit(SDL_INIT_VIDEO))
	{
		if ( SDL_Init(SDL_INIT_VIDEO ) < 0 ) //|SDL_INIT_JOYSTICK
			return false;
	}
	m_bInited = true;

	m_iWinX = x;
	m_iWinY = y;
	if (!SetVideoMode(x,y,fullScreen,resizeable,caption))
		return false;

	InitGL(x,y);

	return true;
}

int C3dView::GetScreenCenter ( teViewAxis eAxis )
{
	if ( eAxis == eZAxis )
		return 0;

	return m_iScreenCenter[eAxis];
}


bool C3dView::SetVideoMode ( int x, int y, bool isFullScreen, bool resizeable, bool caption )
{
	Uint32 flags;

	if (!m_bInited)
		return false;

	if (!SDL_WasInit(SDL_INIT_VIDEO))
		SDL_InitSubSystem(SDL_INIT_VIDEO);

	flags = SDL_OPENGL;

	if (isFullScreen)
		flags |= SDL_FULLSCREEN;
	else
	{
		if (!caption && !resizeable)	// if it's not captioned or resizeable, then we have no frame
			flags |= SDL_NOFRAME;
		else
		{
			if (resizeable)
				flags |= SDL_RESIZABLE;
		}
	}

	if ( SDL_SetVideoMode(x, y, 0, flags) == NULL )
		return false;

	if (caption && windowTitle.size())
		SDL_WM_SetCaption(windowTitle.c_str(),NULL);

	fullScreenWindow = isFullScreen;

	resizeableWindow = resizeable;
	showWindowCaption = caption || resizeable;

	SizeGlWindow(0,x,y);

	return true;
}

// kills the window, it's GL and it's Assembly
void C3dView::Kill ( void )
{
	SDL_QuitSubSystem(SDL_INIT_VIDEO);
	SDL_Quit();

	m_iWinX = -1;
	m_iWinY = -1;
}

bool C3dView::OSQuit ( void )
{
	bool	bQuit = false;

	SDL_Event event;
	while (SDL_PollEvent(&event)) 
	{
		if (event.type == SDL_QUIT)
			bQuit = true;
	}
	return bQuit;
}

// resize the openGL context
void C3dView::SizeGlWindow(unsigned int nType, int cx, int cy) 
{
	m_fAspect = 1.33f;

	m_iViewportX = cx;
	m_iViewportY = cy;

	if (m_iViewportY != 0)
	m_fAspect = (float) m_iViewportX / (float) m_iViewportY;

	glMatrixMode (GL_PROJECTION);
	glLoadIdentity ();
	gluPerspective (m_fov,        // Field-of-view angle (def = 30)
					m_fAspect,      // Aspect ratio of viewing volume
					m_xNearZ,            // Distance to near clipping plane
					m_xFarZ);         // Distance to far clipping plane

	glViewport (0, 0, m_iViewportX, m_iViewportY);

	m_iScreenCenter[0] = m_iWinX/2;
	m_iScreenCenter[1] = m_iWinY/2;
}

void C3dView::SetFOV ( float fFOV )
{
	if (fFOV <= 0.0f)
		return;

	m_fov  = fFOV;

	glMatrixMode (GL_PROJECTION);
	glLoadIdentity ();
	gluPerspective (m_fov,        // Field-of-view angle (def = 30)
					m_fAspect,      // Aspect ratio of viewing volume
					m_xNearZ,            // Distance to near clipping plane
					m_xFarZ);         // Distance to far clipping plane

	glViewport (0, 0, m_iViewportX, m_iViewportY);
}

// initalise the OpenGL context
void C3dView::InitGL ( long iX,long iY)
{
	m_iViewportX = iX;
	m_iViewportY = iY;

	m_fAspect = 1.33f;

	if (m_iViewportY != 0)
		m_fAspect = (float) m_iViewportX / (float) m_iViewportY;

	// set the projection matrix and viewport info
	SetDepthRange(m_xNearZ,m_xFarZ);

	// set the BG color
	glClearColor ((float)m_BackGroundColor.r, (float)m_BackGroundColor.g, (float)m_BackGroundColor.b, 1.0);
	glClearDepth(1.0f);									// Depth Buffer Setup
	// make everything look it's best not fastest.
	glHint(GL_PERSPECTIVE_CORRECTION_HINT ,GL_NICEST);

	// we want a z buffer
	glEnable (GL_DEPTH_TEST);
	glDepthFunc(GL_LEQUAL);								// The Type Of Depth Testing To Do

	// we want back face culling
    glEnable (GL_CULL_FACE);
	glCullFace(GL_BACK);
	
	glFrontFace(GL_CCW);

	// we want smooth filled polies
	glShadeModel (GL_SMOOTH);
	glPolygonMode (GL_FRONT, GL_FILL);

	glEnable (GL_LIGHTING);

	EnabletLight(0,true);

	float	f[4] = {1.0f};
	f[0] = f[1] = f[2] = 0.25f;

	SetLightInfo(0,eLightAmb,f);

	f[0] = f[1] = f[2] = 0.60f;
	SetLightInfo(0,eLightDif,f);

	f[0] = f[1] = f[2] = 0.60f;
	SetLightInfo(0,eLightSpec,f);

	f[0] = 0;f[1] = 0;f[2] = 10.0f;
	SetLightInfo(0,eLightPos,f);

	UpdateLights();
	// but only enable one ?!?
//	glEnable (GL_LIGHT0); 

	// we want alpha based transperancy
	glEnable(GL_BLEND);
	glBlendFunc(GL_SRC_ALPHA,GL_ONE_MINUS_SRC_ALPHA);

	glEnable (GL_COLOR_MATERIAL);

	// clear out the model view matrix
	glMatrixMode(GL_MODELVIEW);
	glLoadIdentity ();
}

void C3dView::SetLightInfo ( int light, int iItem, float *pParams )
{
	switch(iItem)
	{
		case eLightPos:
			memcpy(glLights[light].pos,pParams,sizeof(float)*4);
			break;
		case eLightSpec:
			memcpy(glLights[light].spec,pParams,sizeof(float)*4);
			break;
		case eLightDif:
			memcpy(glLights[light].dif,pParams,sizeof(float)*4);
			break;
		case eLightAmb:
			memcpy(glLights[light].amb,pParams,sizeof(float)*4);
			break;
	}
}

void C3dView::SetLightInfo ( int light, int iItem, float i, float j, float k, float o )
{
	float a[4] = {i,j,k,o};
	SetLightInfo(light,iItem,a);
}

void C3dView::EnabletLight ( int light, bool enable )
{
	glLights[light].enabled = enable;
}


void C3dView::UpdateLights ( void )
{
	for ( int i = 0; i < 8; i++)
	{
		if (glLights[i].enabled)
		{
			glEnable (GL_LIGHT0+i); 
			glLightfv (GL_LIGHT0+i, GL_AMBIENT,glLights[i].amb);
			glLightfv (GL_LIGHT0+i, GL_DIFFUSE,glLights[i].dif);
			glLightfv (GL_LIGHT0+i, GL_SPECULAR,glLights[i].spec);
			glLightfv (GL_LIGHT0+i, GL_POSITION,glLights[i].pos);
		}
		else
			glDisable (GL_LIGHT0+i); 
	}
}

void C3dView::SetBGColor ( float r, float g, float b)
{
	trColor clr = {r,g,b};
	SetBGColor(clr);
}

void C3dView::SetBGColor ( trColor clr )
{
	m_BackGroundColor = clr; 
	glClearColor ((float)m_BackGroundColor.r, (float)m_BackGroundColor.g, (float)m_BackGroundColor.b, 1.0);
}

// sets the range for the view frustum ( and z buffer range )
void  C3dView::SetDepthRange ( float xFarZ )
{
	SetDepthRange(m_xNearZ,xFarZ);
}

void C3dView::SetDepthRange ( float fNearZ, float xFarZ )
{
	m_xFarZ = xFarZ;
	m_xNearZ = fNearZ;

	// this stuff don't work in SDL
	glViewport (0, 0, m_iViewportX, m_iViewportY); 
	glMatrixMode (GL_PROJECTION);
	glLoadIdentity ();
	gluPerspective (m_fov,				// Field-of-view angle (def = 30)
					m_fAspect,			// Aspect ratio of viewing volume
					m_xNearZ,			// Distance to near clipping plane
					m_xFarZ);			// Distance to far clipping plane

	glMatrixMode(GL_MODELVIEW);
	glLoadIdentity ();
}


// gets and sets the current view pos and rot
void C3dView::GetCurrentViewRotation ( float *px, float *py, float *pz )
{
	if (!px || !py || !pz)
		return;

	*px = m_aViewRot[0];
	*py = m_aViewRot[1];
	*pz = m_aViewRot[2];
}

void C3dView::GetCurrentViewPosition ( float *px, float *py, float *pz )
{
	if (!px || !py || !pz)
		return;

	*px = m_aViewPos[0];
	*py = m_aViewPos[1];
	*pz = m_aViewPos[2];
}

float C3dView::GetCurrentViewPosition ( teViewAxis eAxis )
{
	return m_aViewPos[eAxis];
}

float C3dView::GetCurrentViewRotation ( teViewAxis eAxis )
{
	return m_aViewRot[eAxis];	
}

void C3dView::RotateView ( float px, float py, float pz )
{	
	m_aViewRot[0] = px;
	m_aViewRot[1] = py;
	m_aViewRot[2] = pz;
}

void C3dView::RotateViewInc ( float px, float py, float pz )
{
	m_aViewRot[0] += px;
	m_aViewRot[1] += py;
	m_aViewRot[2] += pz;
}

void C3dView::PanView ( float px, float py, float pz )
{
	m_aViewPos[0] = px;
	m_aViewPos[1] = py;
	m_aViewPos[2] = pz;
}

void C3dView::PanViewInc ( float px, float py, float pz )
{
	if ( (m_eViewMode == eThirdPerson) || (m_eViewMode == eFirstPerson) )
	{
		// do the vertical part
		m_aViewPos[0]+= (float)sin(m_aViewRot[2]*DEG_RAD) * py;
		m_aViewPos[1]-= (float)cos(m_aViewRot[2]*DEG_RAD) * py;

		// do the hirozonatl part
		m_aViewPos[0]+= (float)sin((m_aViewRot[2]-90.0f)*DEG_RAD) * px;
		m_aViewPos[1]-= (float)cos((m_aViewRot[2]-90.0f)*DEG_RAD) * px;
	}
	else
	{
		m_aViewPos[0] += px;
		m_aViewPos[1] += py;
		m_aViewPos[2] += pz;
	}
	m_aViewPos[2] += pz;
}

void C3dView::SetInitalMatrix ( void )
{
	switch (m_eViewMode)
	{
		case eManual:
			glLoadIdentity ();
			glMultMatrixf(m_aViewMatrix);
			break;

		case eIdenity:
			glLoadIdentity();
			break;

		case eIdenityXY:
			glLoadIdentity();
			glRotatef(-90, 1.0f, 0.0f, 0.0f);
			break;

		case eFirstPerson:
			glLoadIdentity ();
			glRotatef(m_aViewRot[1], 0.0f, 0.0f, 1.0f);
			glRotatef(-m_aViewRot[0], 1.0f, 0.0f, 0.0f);
			glRotatef(-m_aViewRot[2], 0.0f, 1.0f, 0.0f);
			glTranslatef(-m_aViewPos[0],-m_aViewPos[2],m_aViewPos[1]);
			glRotatef(-90, 1.0f, 0.0f, 0.0f);
			break;
		
		case eThirdPerson:
			glLoadIdentity ();
			glTranslatef(0,0,-m_fPullBack);								// pull back on allong the zoom vector
			glRotatef(m_aViewRot[0], 1.0f, 0.0f, 0.0f);					// pops us to the tilt
			glRotatef(-m_aViewRot[2], 0.0f, 1.0f, 0.0f);					// gets us on our rot
			glTranslatef(-m_aViewPos[0],-m_aViewPos[2],m_aViewPos[1]);	// take us to the pos
			glRotatef(-90, 1.0f, 0.0f, 0.0f);							// gets us into XY
			break;

		default:
			break;
	}
}

void C3dView::GetViewMatrix ( float *pfMatrix )
{
	memcpy(pfMatrix,m_aViewMatrix,sizeof(float)*16);
}

void C3dView::SetViewMatrix ( float *pfMatrix )
{
	memcpy(m_aViewMatrix,pfMatrix,sizeof(float)*16);
}

// draw the grid and tell the asembly to draw
bool C3dView::BeginDraw ( void )
{
	if (m_bDrawing )
		return false;

	m_bDrawing = true;
	m_bOverlayMode = false;
	m_bForcedDrawing = false;

	// rest GL
	glClear(GL_COLOR_BUFFER_BIT | GL_DEPTH_BUFFER_BIT | GL_STENCIL_BUFFER_BIT);
	glMatrixMode(GL_MODELVIEW);
	glPushMatrix();
	SetInitalMatrix();
	// save the matrix

	// set the frustum based on the view
	frustum.Get();

	glColor4f(1,1,1,1);

	return m_bDrawing;
}

void C3dView::EndDraw ( void )
{
	if (!m_bDrawing)
		return;

	if (m_bOverlayMode)
		EndOverlay();

	// put the matrix back
	glPopMatrix();
	//	flip it
	SDL_GL_SwapBuffers();
	glFlush();

	m_bDrawing = false;
	m_bOverlayMode = false;
	m_bForcedDrawing = false;
}

bool C3dView::BeginSelect ( int x, int y )
{
	if (m_bDrawing )
		return false;

	m_bDrawing = true;
	m_bOverlayMode = false;
	m_bForcedDrawing = false;

	GLint	viewport[4] = {0};
	
	memset(m_uiSelectBuf,0,sizeof(GLuint)*_SEL_BUFFER_SIZE);
	glGetIntegerv(GL_VIEWPORT, viewport);

	glSelectBuffer(_SEL_BUFFER_SIZE, m_uiSelectBuf);
	glRenderMode(GL_SELECT);

	glInitNames();
	glPushName(0xffffffff);

	glMatrixMode(GL_PROJECTION);

	glPushMatrix();
	glLoadIdentity();
	gluPickMatrix((GLdouble) x, (GLdouble) (viewport[3] - y),2.0, 2.0, viewport);
	gluPerspective (m_fov,m_fAspect,m_xNearZ,m_xFarZ);

	glMatrixMode(GL_MODELVIEW);
	glPushMatrix();

	SetInitalMatrix();

	return true;
}

unsigned int C3dView::EndSelect ( void )
{
	if (!m_bDrawing)
		return 0xffffffff;

	GLint	hits = 0;
	long	iTopName = -1;
	float	LastZ = 0.0f;
	bool	bFirstShot = true;
	long	i;
	unsigned int	j;
	GLuint	names = 0, *ptr = NULL;
	float	z1 = 0,z2 = 0;
	long	iName = 0;

	glPopMatrix();

	glMatrixMode(GL_PROJECTION);
	glPopMatrix();
	
	glFlush();
	
	// end render
	hits = glRenderMode(GL_RENDER);

	glMatrixMode(GL_PROJECTION);
	glLoadIdentity();
	gluPerspective (m_fov,m_fAspect,m_xNearZ,m_xFarZ);
	glViewport (0, 0, m_iViewportX, m_iViewportY);
	
	glMatrixMode(GL_MODELVIEW);
	glLoadIdentity ();
	
	char	msg[255];

	if (hits >0)
	{
		ptr = (GLuint *) m_uiSelectBuf;

		for (i = 0; i < hits; i++) 
		{  /* for each hit  */
			names = *ptr;
			ptr++;
			z1 = (float) *ptr/0x7fffffff;
			ptr++;
			z2 = (float) *ptr/0x7fffffff;
			ptr++;
			
			// stupid compiler bug
			sprintf(msg,"names %u f1 %f f2 %f",(unsigned int)names,z1,z2);

			if (bFirstShot)
			{
				LastZ = z1;
				bFirstShot = false;
			}
			for (j = 0; j < names; j++)
			{  /* for each name */
				iName = *ptr;
				ptr++;
			}
			if (z1 <= LastZ)
			{
				LastZ = z1;
				iTopName = iName;
			}
		}

	}
	m_bDrawing = false;
	m_bOverlayMode = false;
	m_bForcedDrawing = false;


	if (iTopName == -1)
		return 0;
	else
		return iTopName;
}

bool C3dView::BeginOverlay ( void )
{
	if (m_bOverlayMode)
		return true;

	if (!m_bDrawing)
	{
		BeginDraw();
		m_bForcedDrawing = true;
	}
	else
		m_bForcedDrawing = false;

	m_bOverlayMode = true;

	glMatrixMode (GL_PROJECTION);
	glLoadIdentity ();
	glOrtho(0.0,(double)m_iViewportX,
			0.0,(double)m_iViewportY,
			0.1,100.0);

	glMatrixMode(GL_MODELVIEW);
	glPushMatrix();
	glLoadIdentity();
	glDisable(GL_LIGHTING);

	return true;
}

void C3dView::EndOverlay ( void )
{
	if (m_bDrawing && m_bOverlayMode)
	{
		glMatrixMode(GL_MODELVIEW);
		glPopMatrix();
		
		glMatrixMode (GL_PROJECTION);
		glLoadIdentity ();
		gluPerspective (m_fov,  // Field-of-view angle (def = 30)
					m_fAspect,	// Aspect ratio of viewing volume
					m_xNearZ,	// Distance to near clipping plane
					m_xFarZ);	// Distance to far clipping plane 
		glViewport (0, 0, m_iViewportX, m_iViewportY);

		glMatrixMode(GL_MODELVIEW);
		glEnable(GL_LIGHTING);
		m_bOverlayMode = false;

		if (m_bForcedDrawing)
			EndDraw();
	}
}


