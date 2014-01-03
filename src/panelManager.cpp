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
//PanelManager.cpp

#include "panelManager.h"
#include "utils.h"
#include "input.h"
#include "fontManager.h"
#include "3dView.h"
#include "OGFTimer.h"
#include "textureManager.h"

#include "SDL.h"

#define _DEFAULT_Z	-0.5f

CBasePanelItem::CBasePanelItem()
{
	m_iFontID = -1;
	m_iFontSize = -1;
	m_bFocus = false;
	m_bVisible = true;
	m_pPanelManager = NULL;
	m_pPanel = NULL;

	m_bCanFocus = false;
	m_bCanClick = false;
	m_bCanKey = false;
	m_bCanText = false;
	InitData();
}

CBasePanelItem::CBasePanelItem( const C3DVertex &oPos, const char *szWindowText )
{
	CBasePanelItem();
	Create ( oPos,szWindowText);
}

CBasePanelItem::CBasePanelItem ( const C3DVertex &oMax, const C3DVertex &oMin, const char *szWindowText)
{
	CBasePanelItem();
	Create ( oMax,oMin,szWindowText);
}

CBasePanelItem::CBasePanelItem ( float fLeft, float fTop, float fRight, float fBottom, const char *szWindowText)
{
	CBasePanelItem();
	Create ( fLeft, fTop, fRight, fBottom, szWindowText);
} 

CBasePanelItem::~CBasePanelItem()
{

}

void CBasePanelItem::InitData ( void )
{

}

void CBasePanelItem::Create ( const C3DVertex &oPos, const char *szWindowText )
{
	InitData();
	if (m_pPanelManager)
		m_oColor = m_pPanelManager->GetColor();

	m_oPos = oPos;
	SetWinText(szWindowText);
	if (m_pPanel)
	{
		m_iFontID = m_pPanel->GetDefaultFont();
		m_iFontSize = m_pPanel->GetDefaultFontSize();
	}
	else if (m_pPanelManager)
	{
		m_iFontID = m_pPanelManager->GetDefaultFont();
		m_iFontSize = m_pPanelManager->GetDefaultFontSize();
	}
	else
	{
		m_iFontID = CFontManager::instance().GetFaceID("Arial");
		m_iFontSize = 12;
	}
}

void CBasePanelItem::Create ( const C3DVertex &oMax, const C3DVertex &oMin, const char *szWindowText)
{
	C3DVertex p = ((oMax-oMin)/2)+oMin;
	Create( p, szWindowText );
	Size(oMin.X(),oMax.Y(),oMax.Y(),oMin.Y());
}

void CBasePanelItem::Create ( float fLeft, float fTop, float fRight, float fBottom, const char *szWindowText )
{
	C3DVertex oCenter( ((fRight-fLeft)/2)+fLeft,((fTop-fBottom)/2)+fBottom,_DEFAULT_Z);

	Create(oCenter,szWindowText);
	Size(fLeft,fTop,fRight,fBottom);
}

void CBasePanelItem::Size ( float fLeft, float fTop, float fRight, float fBottom, bool bIncremental )
{
	if (bIncremental)
	{
		m_afBoundsBox[0] = fLeft; 
		m_afBoundsBox[1] = fTop; 
		m_afBoundsBox[2] = fRight; 
		m_afBoundsBox[3] = fBottom; 
	}
	else
	{
		m_afBoundsBox[0] = fLeft-m_oPos.X(); 
		m_afBoundsBox[3] = fTop-m_oPos.Y(); 
		m_afBoundsBox[2] = fRight-m_oPos.X(); 
		m_afBoundsBox[1] = fBottom-m_oPos.Y(); 
	}
}

void CBasePanelItem::SetWinText ( const char *szWindowText )
{
	if (szWindowText)
		m_szWindowText = szWindowText;
}

void CBasePanelItem::SetColor ( CColor &oColor )
{
	m_oColor = oColor;
}

void CBasePanelItem::SetFont ( const char *szFaceName, int iSize )
{
	m_iFontID = CFontManager::instance().GetFaceID(szFaceName);
	m_iFontSize = iSize;
}

bool CBasePanelItem::PointIn ( const C3DVertex &rPos )
{
	C3DVertex	rRelPos = m_oPos-rPos;

	if (rRelPos.X() > m_afBoundsBox[2])
		return false;

	if (rRelPos.X() < m_afBoundsBox[0])
		return false;

	if (rRelPos.Y() > m_afBoundsBox[3])
		return false;

	if (rRelPos.Y() < m_afBoundsBox[1])
		return false;

	return true;
}

bool CBasePanelItem::OnHover ( const C3DVertex &rPos )
{
	return false;
}

bool CBasePanelItem::OnClick ( const C3DVertex &rPos, bool bMouseDown )
{
	return m_bCanClick;
}

bool CBasePanelItem::OnKeyEvent ( int iKey )
{
	return false;
}

bool CBasePanelItem::OnTextEvent ( const char *text )
{
	return false;
}

int  CBasePanelItem::Think ( float fTransitionParam )
{
	return 0;
}

void CBasePanelItem::Draw ( void )
{

}

bool CBasePanelItem::SetItemData ( const char* szName, const char* szData )
{
	return false;
}

bool CBasePanelItem::GetItemData ( const char* szName, char* szData )
{
	return false;
}

// base panel class
CBasePanel::CBasePanel()
{
	m_vItemList.clear();
	m_pPanelManager = false;
	m_pItemWithFocus = NULL;

	m_szPanelName = "BasePanel";
}

CBasePanel::~CBasePanel()
{
	FlushItems();
}

void CBasePanel::FlushItems ( void )
{
	tvItemList::iterator itr = m_vItemList.begin();

	while ( itr != m_vItemList.end())
	{
		if (*itr)
			delete(*itr);

		itr++;
	}
	
	m_vItemList.clear();
}

void CBasePanel::SetItems ( void )
{
	// load up some stuff here
}

void CBasePanel::Create ( void )
{
	if (m_pPanelManager)
	{
		m_iDefaultFont = m_pPanelManager->GetDefaultFont();
		m_iDefaultFontSize = m_pPanelManager->GetDefaultFontSize();
	}
	else
	{
		m_iDefaultFont = CFontManager::instance().GetFaceID("Arial");
		m_iDefaultFontSize = 12;
	}
	SetItems();
}

void CBasePanel::Resume ( void )
{

}

CBasePanelItem*	CBasePanel::AddItem ( CBasePanelItem* pItem )
{
	pItem->m_pPanelManager =m_pPanelManager;
	pItem->m_pPanel = this;
	m_vItemList.push_back(pItem);
	return pItem;
}

CBasePanelItem*	CBasePanel::GetItemByName ( const char *szName )
{
	if (!szName)
		return NULL;

	std::string sName = szName;
	tvItemList::iterator itr = m_vItemList.begin();

	while ( itr != m_vItemList.end())
	{
		if ((*itr)->GetItemName() == sName)
			return *itr;
		itr++;
	}
	return NULL;
}

CBasePanelItem* CBasePanel::GetItemFromPoint ( const C3DVertex &rPos )
{
	tvItemList::iterator itr = m_vItemList.begin();

	while ( itr != m_vItemList.end())
	{
		if ((*itr)->PointIn(rPos))
			return *itr;

		itr++;
	}
	return NULL;
}

bool CBasePanel::OnClick ( const C3DVertex &rPos, bool bMouseDown )
{
	CBasePanelItem*  pClicked = GetItemFromPoint(rPos);
	if (!pClicked || !pClicked->Visible() || !pClicked->CanClick())
		return false;

	// no need to refocus that what has focus
	if (m_pItemWithFocus != pClicked)
	{
		if (m_pItemWithFocus)
			m_pItemWithFocus->SetFocus(false);

		if (pClicked->CanFocus() && bMouseDown)
		{
			pClicked->SetFocus(true);
			m_pItemWithFocus = pClicked;
		}
	}

	bool bRet = pClicked->OnClick(rPos,bMouseDown);
	if (bRet)
		OnItemActivate(pClicked);

	return bRet;
}

void CBasePanel::OnItemActivate ( CBasePanelItem* pItem )
{

}

bool CBasePanel::OnKeyEvent ( int iKey )
{
	// tab order anyone?
	if ( (iKey != SDLK_TAB) || (iKey != SDLK_UP) || (iKey != SDLK_DOWN) )
	{
		if (m_pItemWithFocus && m_pItemWithFocus->CanKey())
			return m_pItemWithFocus->OnKeyEvent(iKey);
	}
	else if (!m_pItemWithFocus)
	{
		// it's a tab type thing and we don't have a current guy so try and find the first one that is vis and focusable
		tvItemList::iterator itr = m_vItemList.begin();

		while ( itr != m_vItemList.end())
		{
			if ((*itr)->Visible() && (*itr)->CanFocus())
			{
				m_pItemWithFocus = (*itr);
				m_pItemWithFocus ->SetFocus(true);
				return true;
			}
			itr++;
		}
	}
	else
	{
		if ( (iKey != SDLK_TAB) || (iKey != SDLK_UP) || (iKey != SDLK_DOWN) )
			return false;

		tvItemList::iterator itr;
		tvItemList::reverse_iterator ritr;
		switch ( iKey )
		{
			case SDLK_TAB:
				itr = m_vItemList.begin();

				while ( (*itr) != m_pItemWithFocus)
					itr++;

				CBasePanelItem*	pItem = NULL;
				while ( (itr !=m_vItemList.end()) && !pItem )
				{

				}
			break;
		}
	}

	return false;
}

bool CBasePanel::OnTextEvent ( const char *text )
{
	if (m_pItemWithFocus && m_pItemWithFocus->CanText())
		return m_pItemWithFocus->OnTextEvent(text);

	return false;
}


tePanelReturn CBasePanel::Think ( float fTransitionParam )
{
	tvItemList::iterator itr = m_vItemList.begin();

	while ( itr != m_vItemList.end())
	{
		(*itr)->Think(fTransitionParam);
		itr++;
	}
	return ePRContinue;
}

void CBasePanel::Draw ( void )
{
	tvItemList::iterator itr = m_vItemList.begin();

	while ( itr != m_vItemList.end())
	{
		if ((*itr)->Visible())
		{
			(*itr)->Draw();
		}
		itr++;
	}
}

//typedef std:map<std::string,CBasePanel*> tvPanelList;

CBasePanelManager::CBasePanelManager()
{
	m_pThisPanel = NULL;
	m_pNextPanel = NULL;
	m_bLastMouseState = false;
}

CBasePanelManager::~CBasePanelManager()
{
	tvPanelList::iterator itr = m_vPanelList.begin();

	while(itr != m_vPanelList.end())
		delete((itr++)->second);
}

void CBasePanelManager::Create ( void )
{
	// some inits
	m_iDefaultFont = CFontManager::instance().GetFaceID("Arial");
	m_iDefaultFontSize = 12;
	m_fTransitionSpeed = 1.0f;
	m_fTranstionParam = 0;

	// set the panels
	// and set the staring panel
	LoadPanels();

	// make sure all the panels know we are there daddy
	// and call there create methods
	tvPanelList::iterator itr = m_vPanelList.begin();

	while (itr != m_vPanelList.end())
	{
		if (itr->second)
			itr->second->Create();
		itr++;
	}

	LoadBackground();

	unsigned char *pKeys;
	CInputMananger::instance().GetKeysState(&pKeys);

	for ( int i = 0; i< 256; i++)
		m_abLastKeyStates[i] = pKeys[i] != 0;

	// get the start panel
	m_pThisPanel = GetPanelByName(m_szStartPanel.c_str());
}

void CBasePanelManager::Resume ( void )
{
	LoadBackground();

	unsigned char *pKeys;
	CInputMananger::instance().GetKeysState(&pKeys);

	for ( int i = 0; i< 256; i++)
		m_abLastKeyStates[i] = pKeys[i] != 0;

	tvPanelList::iterator itr = m_vPanelList.begin();

	while (itr != m_vPanelList.end())
	{
		if (itr->second)
			itr->second->Resume();
		itr++;
	}

	// get the start panel
	m_pThisPanel = GetPanelByName(m_szStartPanel.c_str());
}


CBasePanel* CBasePanelManager::GetPanelByName ( const char* szName )
{
	std::string	str = szName;
	tvPanelList::iterator itr = m_vPanelList.find(str);
	if (itr == m_vPanelList.end())
		return NULL;
	return itr->second;
}

void CBasePanelManager::ReloadPanels ( void )
{
	// blow out all the old panels
	tvPanelList::iterator itr = m_vPanelList.begin();

	// tell mr texture manager and the font manager to invalidate all textures cus they are JACKED
	CTextureManager::instance().UnloadAll();
	CFontManager::instance().UnloadAll();

	while(itr != m_vPanelList.end())
		delete((itr++)->second);

	// make the panels againg
	Create();

	// now put the last panel back
	if (currentPanelName.size() == 0)
		currentPanelName = m_szStartPanel;
	m_pThisPanel = GetPanelByName(currentPanelName.c_str());
}


void CBasePanelManager::LoadPanels ( void )
{
	// do something here
	// please for the love of god
	// do something here
}

CBasePanel*	CBasePanelManager::AddPanel ( CBasePanel* pPanel, const char *szName )
{
	if (!pPanel || !szName)
		return NULL;

	std::string str = szName;

	pPanel->m_pPanelManager = this;
	m_vPanelList[str] = pPanel;

	return pPanel;
}

void CBasePanelManager::EnterText ( const char *text )
{
	if (m_pThisPanel)
		m_pThisPanel->OnTextEvent(text);
}

tePanelReturn CBasePanelManager::Think ( void )
{
	// this would be bad
	if (!m_pThisPanel)
		return ePRExit;

	tePanelReturn eReturn = ePRContinue;

	// m_fTranstionParam = 0 = not transitioning, we are stable
	// m_fTranstionParam = 1 = we are the one transitioning out 1 = all the way gone
	// m_fTranstionParam = -1 = we are the one transitioning in, -1 = not int yet 0 = all the way in
	if (m_fTranstionParam == 0)	// we are not transitioning
	{
		int iX,iY;
		CInputMananger::instance().GetMousePos(iX,iY,false);

		C3DVertex	oPoint((float)iX,(float)C3dView::instance().GetWinV()-(float)iY);

		CBasePanelItem	*pItem = m_pThisPanel->GetItemFromPoint(oPoint);

		if (pItem)
		{
			bool abButtons[3] = {false,false,false};
			CInputMananger::instance().GetMouseButtons(abButtons[0],abButtons[1],abButtons[2]);

			if (abButtons[0] != m_bLastMouseState)
			{
				m_bLastMouseState = abButtons[0];
				m_pThisPanel->OnClick(oPoint,abButtons[0]);
			}
			else if (!abButtons[0])
				pItem->OnHover(oPoint);
		}

		unsigned char *pKeys;
		CInputMananger::instance().GetKeysState(&pKeys);

		for ( int i = 0; i< 256; i++)
		{
			if (m_abLastKeyStates[i] != (pKeys[i]!=0))
			{
				if (pKeys[i]!= 0)
					m_pThisPanel->OnKeyEvent(i);

				m_abLastKeyStates[i] = pKeys[i]!=0;
			}
		}
		
		eReturn = m_pThisPanel->Think(m_fTranstionParam);

		// are we changing resolutions, or doing somethign that effects all panels? like a font reload or something
		if ( eReturn == ePRReload )
		{
			ReloadPanels();
			return ePRContinue;
		}

		// we don't have to do anything
		if ((eReturn != ePRNext))
			return eReturn;

		// we are transitioning
		m_fTranstionParam += (float)(CLGETime::instance().GetFrameTime()/m_fTransitionSpeed);

		// this should not hapen
		// but if it does this should let it get done
		if (m_pNextPanel)
			m_pNextPanel->Think(1);

		currentPanelName = m_pThisPanel->GetNextPanel();
		m_pNextPanel = GetPanelByName(currentPanelName.c_str());

		// if we can't find the next panel, then well hell we are screwed
		if (!m_pNextPanel)
			m_fTranstionParam = 0;
		else
		{
			m_pNextPanel->Resume();
			m_pNextPanel->Think(-1);	// give it a -1 to let it know it's goin for the gusto
		}

		return ePRContinue;
	}
	else
	{
		m_bLastMouseState = 0;
		if (m_fTranstionParam >= 1)
		{
			// we are done with the transition
			// let the old one get it's last update
			m_pThisPanel->Think(1);

			// the future is now
			m_pNextPanel->Resume();
			m_pThisPanel = m_pNextPanel;
			m_pNextPanel = NULL;

			// this's first full up message
			eReturn = m_pThisPanel->Think(0);

			// no more transition
			m_fTranstionParam = 0;
			
			// she's not allowed to actualy get a message on his first frame
			return ePRContinue;
		}
		else
		{
			m_pThisPanel->Think(m_fTranstionParam);
			
			// realy there has to be one of these
			// but JUST in case
			if (m_pNextPanel)
				m_pNextPanel->Think(-(1.0f-m_fTranstionParam));
			
			m_fTranstionParam += (float)CLGETime::instance().GetFrameTime()/m_fTransitionSpeed;

			return ePRContinue;
		}
	}
	return ePRContinue;
}

void CBasePanelManager::Draw ( void )
{
	if (C3dView::instance().BeginDraw())
	{
		glEnable(GL_TEXTURE_2D);
		DrawBackground();

		if (m_fTranstionParam != 0)
			glDepthMask(0);

		if (C3dView::instance().BeginOverlay())
		{
			glPushMatrix();
			glTranslatef(0,0,-0.25f);
			if (m_pThisPanel)
				m_pThisPanel->Draw();
			glPopMatrix();

			if (m_pNextPanel)
				m_pNextPanel->Draw();

		
			C3dView::instance().EndOverlay();
		} 
		glDepthMask(1);
		C3dView::instance().EndDraw();
	}
}

void CBasePanelManager::LoadBackground ( void )
{

}

void CBasePanelManager::DrawBackground ( void )
{

}
