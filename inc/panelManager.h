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

// PanelManager.h

#ifndef _PANEL_MANAGER_H_
#define _PANEL_MANAGER_H_

#ifdef _WIN32
	#pragma warning( disable : 4786 )  // Disable warning message
#include <windows.h>
#else
	#define stricmp strcasecmp
#endif

#include "mathUtils.h"
#include "3dTypes.h"

#include <string>
#include <vector>
#include <map>

typedef enum
{
	ePRContinue,
	ePRExit,
	ePRBegin,
	ePRResume,
	ePRNext,
	ePRReload	// only called on a res switch, the pannel manager will handle this it'self
}tePanelReturn;

class CBasePanelManager;
class CBasePanel;

class CBasePanelItem
{
public:
	CBasePanelItem();
	CBasePanelItem ( const C3DVertex &oPos, const char *szWindowText = NULL );
	CBasePanelItem ( const C3DVertex &oMax, const C3DVertex &oMin, const char *szWindowText = NULL );
	CBasePanelItem ( float fLeft, float fTop, float fRight, float fBottom, const char *szWindowText = NULL );

	virtual ~CBasePanelItem();

	virtual void InitData ( void );

	virtual void Create ( const C3DVertex &oPos, const char *szWindowText = NULL );	// this is the only real one you have to implemt since everyone calls it and size;
	virtual void Create ( const C3DVertex &oMax, const C3DVertex &oMin, const char *szWindowText = NULL );
	virtual void Create ( float fLeft, float fTop, float fRight, float fBottom, const char *szWindowText = NULL );

	virtual void Size ( float fLeft, float fTop, float fRight, float fBottom, bool bIncremental = false );

	void SetWinText (  const char *szWindowText = NULL );
	void SetColor ( CColor &oColor );
	void SetFont ( const char *szFaceName, int iSize );

	const char* GetWinText ( void ) { return m_szWindowText.c_str();}

	virtual bool PointIn ( const C3DVertex &rPos );
	virtual bool OnHover ( const C3DVertex &rPos );
	virtual bool OnClick ( const C3DVertex &rPos, bool bMouseDown = true  );
	virtual bool OnKeyEvent ( int iKey );
	virtual bool OnTextEvent ( const char *text );

	virtual int  Think ( float fTransitionParam );

	virtual void Draw ( void );

	virtual bool SetItemData ( const char* szName, const char* szData );
	virtual bool GetItemData ( const char* szName, char* szData );

	bool HasFocus ( void ) {return m_bFocus;}
	void SetFocus ( bool bSet ) {m_bFocus = bSet;}

	bool Visible ( void ) {return m_bVisible;}
	void Show ( bool bSet ) {m_bVisible = bSet;}

	CBasePanelManager	*m_pPanelManager;
	CBasePanel			*m_pPanel;

	// flags
	bool CanFocus ( void ) {return m_bCanFocus;}
	bool CanClick ( void ) {return m_bCanClick;}
	bool CanKey( void ) {return m_bCanKey;}
	bool CanText( void ) {return m_bCanText;}

	const char* GetItemName ( void ){return m_ItemName.c_str();}
	void SetItemName ( const char *szName ){m_ItemName = szName;}

protected:
	CColor				m_oColor;
	C3DVertex			m_oPos;
	float				m_afBoundsBox[4];
	std::string			m_szWindowText;
	int					m_iFontID;
	int					m_iFontSize;
	bool				m_bFocus;
	bool				m_bVisible;

	bool				m_bCanFocus;
	bool				m_bCanClick;
	bool				m_bCanKey;
	bool				m_bCanText;

	std::string			m_ItemName;
};		

typedef std::vector<CBasePanelItem*>	tvItemList;			

class CBasePanel
{
public:
	CBasePanel();
	virtual ~CBasePanel();

	virtual void Create ( void );
	virtual	void SetItems ( void );

	virtual	void Resume ( void );

	void FlushItems ( void );

	virtual CBasePanelItem* GetItemFromPoint ( const C3DVertex &rPos );

	virtual bool OnClick ( const C3DVertex &rPos, bool bMouseDown = true );
	virtual bool OnKeyEvent ( int iKey );
	virtual bool OnTextEvent ( const char *text );

	virtual void OnItemActivate ( CBasePanelItem *Iptem );

	virtual tePanelReturn  Think ( float fTransitionParam );

	virtual void Draw ( void );

	virtual const char *GetPanelName ( void ) { return m_szPanelName.c_str();}
	virtual const char *GetNextPanel ( void ) { return m_szNextPanel.c_str();}

	int GetDefaultFont ( void ){return m_iDefaultFont;}
	int GetDefaultFontSize ( void ){return m_iDefaultFontSize;}

	CBasePanelItem*	AddItem ( CBasePanelItem* pItem );
	CBasePanelItem*	GetItemByName ( const char *szName );

	CBasePanelManager	*m_pPanelManager;
	
protected:
	tvItemList				m_vItemList;
	int						m_iDefaultFont;
	int						m_iDefaultFontSize;
	CBasePanelItem			*m_pItemWithFocus;
	std::string				m_szPanelName;
	std::string				m_szNextPanel;
};

typedef std::map<std::string,CBasePanel*> tvPanelList;

class CBasePanelManager
{
public:
	CBasePanelManager();
	virtual ~CBasePanelManager();

	void Create ( void );
	virtual void LoadPanels ( void );

	CBasePanel*	AddPanel ( CBasePanel* pPanel, const char *szName );
	void Resume ( void );

	tePanelReturn Think ( void );
	void Draw ( void );

	virtual void LoadBackground ( void );
	virtual void DrawBackground ( void );

	int GetDefaultFont ( void ){return m_iDefaultFont;}
	int GetDefaultFontSize ( void ){return m_iDefaultFontSize;}

	virtual const char* GetWorldString ( void ) { return m_szWorldName.c_str();}
	virtual void SetWorldString ( const char* name ) { m_szWorldName = name;}

	const CColor& GetColor ( void ){return m_oColor;}
	void SetColor ( const CColor& color ){m_oColor = color;}

	virtual void EnterText ( const char *text );

	void ReloadPanels ( void );

protected:
	CBasePanel* GetPanelByName ( const char* szName );

	tvPanelList	m_vPanelList;
	CBasePanel	*m_pThisPanel;
	CBasePanel	*m_pNextPanel;

	float		m_fTransitionSpeed;
	float		m_fTranstionParam;
	int			m_iDefaultFont;
	int			m_iDefaultFontSize;

	bool		m_bLastMouseState;

	bool		m_abLastKeyStates[256];

	CColor		m_oColor;
	std::string	m_szWorldName;
	std::string	m_szStartPanel;
	std::string	currentPanelName;
};

#endif//_PANEL_MANAGER_H_
