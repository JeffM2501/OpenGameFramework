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

// FontManager.h

#ifndef _FONT_MANAGER_H_
#define _FONT_MANAGER_H_

#ifdef _WIN32
	#pragma warning( disable : 4786 )  // Disable warning message
#else
	#define stricmp strcasecmp
#endif
#include "singleton.h"

#include "OSFile.h"
#include "3dTypes.h"
#include "displayListManager.h"

#include <map>
#include <string>
#include <vector>

typedef struct
{
	int iInitalDist;
	int	iCharWidth;
	int iWhiteSpaceDist;
	int iStartX;
	int iEndX;
	int iStartY;
	int iEndY;
}trFontMetrics;

class CTextureFont : public DisplayListBuilder
{
public:
	CTextureFont();
	~CTextureFont();

	int GetSize( void );
	const char* GetFaceName ( void );

	bool Load ( COSFile &oFile );

	void DrawString ( float fScale, CColor &oColor, const char *szString );

	float GetStrLength ( float fScale, const char *szString );

	void Free ( void );

	virtual void build ( int list );

protected:
	void PreLoadLists ( void);
	bool fmtRead(COSFile &file, std::string expectedLeft, std::string &retval);
	int	m_aListIDs[128];
	std::map<int,int>	listToCharMap;
	trFontMetrics	m_aFontMetrics[128];

	std::string		m_szFaceName;
	std::string		m_szTexture;
	int				m_iSize;
	int				m_iTextureID;
	int				m_iTextureXSize;
	int				m_iTextureYSize;
	int				m_iTextureZStep;
	int				m_iNumberOfCharacters;
};

typedef std::map<int,CTextureFont*> tmFontSizeMap;
typedef std::vector<tmFontSizeMap>	tmFontFaceList;
typedef std::map<std::string,int> tmFontFaceMap;

class CFontManager: public Singleton<CFontManager>
{
protected:
	friend class Singleton<CFontManager>;
	CFontManager();
	~CFontManager();

public:

	void LoadAll ( const char* szDir );

	int GetFaceID ( const char *szFaceName );

	int GetNumFaces ( void );
	const char* GetFaceName ( int iFaceID );

	void DrawString ( CGLPoint &oPos, CColor &oColor, int iFaceID, int iSize, const char *szText );
	void DrawString ( CGLPoint &oPos, CColor &oColor, const char *szFace, int iSize, const char *szText );

	void DrawString ( float x, float y, float z, CColor &oColor, int iFaceID, int iSize, const char *szText );
	void DrawString ( float x, float y, float z, CColor &oColor, const char *szFace, int iSize, const char *szText );

	float GetStrLength ( int iFaceID, int iSize, const char *szText);
	float GetStrLength ( const char *szFace, int iSize, const char *szText);

	float GetStrHeight ( int iFaceID, int iSize, const char *szText);
	float GetStrHeight ( const char *szFace, int iSize, const char *szText);

	void UnloadAll ( void );

protected:
	CTextureFont*	GetClosestSize ( int iFaceID, int iSize );
	tmFontFaceMap	m_mFaceNames;
	tmFontFaceList	m_vFontFaces;
};

#endif//_FONT_MANAGER_H_
