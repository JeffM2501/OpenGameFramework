/* Open Game Framework
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
#ifndef _3DVIEW_H_
#define _3DVIEW_H_

#ifdef _WIN32
	#define WIN32_LEAN_AND_MEAN		// Exclude rarely-used stuff from Windows headers
	#include <windows.h>
#endif//Win32

#include "singleton.h"

#include "openGL.h"

#include <string>

#include "3dTypes.h"
#include "viewCull.h"

#include "drawlist.h"

#define		_NearClip	1.0
#define		_FarClip	1000.0

typedef enum
{	
	eManual,		// this will load up whatever is in the currnet matrix
	eIdenity,		// just loads the idenity
	eIdenityXY,		// loads the identy + a -90 Rot in X to make Z go "UP"
	eFirstPerson,	// eIdenityXY, then position camera to XYZ pos with ZXY rot
	eThirdPerson,	// eIdenityXY, then position camera to XYZ pos, ZX rot, then pullback by pullback amount.
	eLastType
}teViewType;

typedef enum
{
	eLightPos,
	eLightSpec,
	eLightDif,
	eLightAmb,
	eLightOther,
	eLastLight
}teLightOption;

typedef struct 
{
	float	pos[4];
	float spec[4];
	float dif[4];
	float amb[4];
	float other;
	bool enabled;
}trOpenGLLight;

typedef enum
{
	eXAxis = 0,
	eYAxis,
	eZAxis
}teViewAxis;

typedef void (*tfDrawBackCB)(void*);

class CScreenRes
{
public:
	CScreenRes(){h=-1;v=-1;}
	~CScreenRes(){return;}
	inline bool operator == ( const CScreenRes & r ){return ( h==r.h && v==r.v);}
	int h;
	int v;
};

class C3dView : public Singleton<C3dView>
{
protected:
	friend class Singleton<C3dView>;

	C3dView ( void );
	~C3dView ( void );

public:
	// size enumeration
	bool GetNextFullScreenMode ( CScreenRes &res );
	void ResetFullScreenModes ( void );

	// context creation
	void setWindowTitle ( const char* title );
	bool Create ( int x, int y, bool fullScreen = false, bool resizeable = false, bool caption = true );
	void Kill ( void );
	bool SetVideoMode ( int x, int y, bool isFullScreen = false, bool resizeable = false, bool caption = true);

	// OSTimeFunction
	bool OSQuit ( void );

	// Window functions
	int GetWinH ( void ) {return m_iViewportX;}
	int GetWinV ( void ) {return m_iViewportY;}
	bool GetFullScreen ( void ){return fullScreenWindow;}

	int GetScreenCenter ( teViewAxis eAxis );

	void SetDepthRange ( float xFarZ );
	void SetDepthRange ( float fNearZ, float xFarZ );

	void SetBGColor ( trColor clr );
	void SetBGColor ( float r, float g, float b);

	void SetFOV ( float fFOV );
	float GetFOV ( void ) {return m_fov;}

	float GetAspect ( void ) {return m_fAspect;}

	float getNearZ ( void ){return m_xNearZ;}
	float getFarZ ( void ){return m_xFarZ;}

	// view controls
	void SetViewType ( teViewType type ) { m_eViewMode = type;}
	teViewType GetViewType ( void ) { return m_eViewMode;}

	void GetCurrentViewRotation ( float *px, float *py, float *pz );
	void GetCurrentViewPosition ( float *px, float *py, float *pz );

	float GetCurrentPullBack ( void ){ return m_fPullBack;}

	float GetCurrentViewPosition ( teViewAxis eAxis );
	float GetCurrentViewRotation ( teViewAxis eAxis );

	void RotateView ( float px, float py, float pz );
	void RotateViewInc ( float px, float py, float pz );

	void PanView ( float px, float py, float pz );
	void PanViewInc ( float px, float py, float pz );

	void MovePullBack ( float fPull ) { m_fPullBack = fPull;}
	void MovePullBackInc  ( float fPull ) { m_fPullBack += fPull;}

	void SetDrawBack ( void (*call)(void*), void *cookie);
	void CallDrawBack ( void );

	void MakeCurrent ( void );

	void GetViewMatrix ( float *pfMatrix );
	void SetViewMatrix ( float *pfMatrix );

	// entry and exit for drawing
	bool BeginDraw ( void );
	void EndDraw ( void );
	
	bool BeginSelect ( int x, int y );
	unsigned int EndSelect ( void );

	bool BeginOverlay ( void );
	void EndOverlay ( void );

	// lighting
	void EnabletLight ( int light, bool enable );
	void SetLightInfo ( int light, int iItem, float *pParams );
	void SetLightInfo ( int light, int iItem, float i, float j, float k, float o );
	void UpdateLights ( void );

	CViewFrustum &GetFrustum ( void ) { return frustum;}
	CDrawingList &GetDrawList( void ) { return drawList;}

private:
	struct trLGEInfo;
	trLGEInfo	*info;
	int			m_iWinX,m_iWinY;
	bool		fullScreenWindow,resizeableWindow, showWindowCaption;

	// Drawing flags
	bool		m_bDrawing;
	bool		m_bOverlayMode;
	bool		m_bForcedDrawing;

	void		*m_DrawCookie;
	tfDrawBackCB	m_DrawBack;
	
	float		m_aViewMatrix[16];

	long		m_iViewportX;
	long		m_iViewportY;
	float		m_fov;
	float		m_fAspect;

	float		m_aViewPos[3];
	float		m_aViewRot[3];
	float		m_fPullBack;

	float		m_xFarZ;
	float		m_xNearZ;

	trColor		m_BackGroundColor;

	teViewType	m_eViewMode;

	bool				m_bInited;
	int					m_iScreenCenter[2];
public:
	// gl view functions
	void		InitGL ( long iX,long iY );

	void		SizeGlWindow(unsigned int nType, int cx, int cy);
private:	bool		PaletteValid (void);
	
	void		SetInitalMatrix();

	GLuint		*m_uiSelectBuf;

/*	float		m_fLight1Pos[3];
	float		m_fLight1AmbColor[3];
	float		m_fLight1DifuseColor[3];
	float		m_fLight1SpecularColor[3]; */

	// buffer info
	int		m_iFrameBuffer;
	int		m_iDepthBuffer;
	int		m_iStencelBuffer;
	int		m_iAlphaBuffer;
	int		m_iAuxBuffer;
	int		m_iBufferMode;
	std::string windowTitle;

	trOpenGLLight		glLights[8];

	CViewFrustum		frustum;
	CDrawingList		drawList;
};

#endif // 3dView
