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
// FontManager.cpp

#include "fontManager.h"
#include "textureManager.h"
#include "utils.h"
#include "openGL.h"

#ifdef _WIN32
  #define strcasecmp stricmp
  #define strncasecmp strnicmp
#endif


void GetTypeFaceName ( char *szData )
{
#ifdef	_WIN32
	strupr(szData);
#else
	while(*szData++)
		*szData = toupper(*szData);
#endif
}

CTextureFont::CTextureFont()
{
	for (int i = 0; i < 128; i++)
	{
		m_aListIDs[i] = -1;
		m_aFontMetrics[i].iCharWidth = -1;
	}

	m_iSize = -1;
	m_iTextureID = -1;

	m_iTextureXSize = -1;
	m_iTextureYSize = -1;
	m_iTextureZStep = -1;
	m_iNumberOfCharacters = -1;
}

CTextureFont::~CTextureFont()
{
	for (int i = 0; i < 128; i++)
	{
		if (m_aListIDs[i] != -1)
			DisplayListManager::instance().releaseList(m_aListIDs[i]);
	}
}

int CTextureFont::GetSize( void )
{
	return m_iSize;
}

const char* CTextureFont::GetFaceName ( void )
{
	return m_szFaceName.c_str();
}

/* read values in Key: Value form from font metrics (.fmt) files */
bool CTextureFont::fmtRead(COSFile &file, std::string expectedLeft, std::string &retval)
{
	static std::string workingFile;
	static int line = 0;

	// reset line number if we've switched files
	if (workingFile != file.GetFileTitle()) 
	{
		workingFile = file.GetFileTitle();
		line = 0;
	}

	std::string tmpBuf;

	// allow for blank lines with native or foreign linebreaks, comment lines
	while (tmpBuf.size() == 0 || tmpBuf[0] == '#' || tmpBuf[0] == 10 || tmpBuf[0] == 13)
	{
		tmpBuf = file.ReadLine();
		// keep a line counter
		line++;
	}

	if (tmpBuf.substr(0, tmpBuf.find(":")) == expectedLeft)
	{
		retval = tmpBuf.substr(tmpBuf.find(":") + 1, tmpBuf.size());
		return true;
	}
	else	
		return false;
}

bool CTextureFont::Load(COSFile &file)
{
	const char *extension = file.GetExtension();

	if (!extension)
		return false;

	if (!file.Open("rb"))
		return false;

	std::string tmpBuf;

	if (!fmtRead(file, "NumChars", tmpBuf))
		return false;

	sscanf(tmpBuf.c_str(), " %d", &m_iNumberOfCharacters);
	if (!fmtRead(file, "TextureWidth", tmpBuf))
		return false;

	sscanf(tmpBuf.c_str(), " %d", &m_iTextureXSize);
	if (!fmtRead(file, "TextureHeight", tmpBuf))
		return false;

	sscanf(tmpBuf.c_str(), " %d", &m_iTextureYSize);
	if (!fmtRead(file, "TextZStep", tmpBuf))
		return false;

	sscanf(tmpBuf.c_str(), " %d", &m_iTextureZStep);

	int i;
	for (i = 0; i < m_iNumberOfCharacters; i++) 
	{
		// check character
		if (!fmtRead(file, "Char", tmpBuf)) return false;
		if ((tmpBuf.size() < 3) ||(tmpBuf[1] != '\"' || tmpBuf[2] != (i + 32) || tmpBuf[3] != '\"')) 
				return false;
			// read metrics
			if (!fmtRead(file, "InitialDist", tmpBuf)) return false;
			sscanf(tmpBuf.c_str(), " %d", &m_aFontMetrics[i].iInitalDist);
			if (!fmtRead(file, "Width", tmpBuf)) return false;
			sscanf(tmpBuf.c_str(), " %d", &m_aFontMetrics[i].iCharWidth);
			if (!fmtRead(file, "Whitespace", tmpBuf)) return false;
			sscanf(tmpBuf.c_str(), " %d", &m_aFontMetrics[i].iWhiteSpaceDist);
			if (!fmtRead(file, "StartX", tmpBuf)) return false;
			sscanf(tmpBuf.c_str(), " %d", &m_aFontMetrics[i].iStartX);
			if (!fmtRead(file, "EndX", tmpBuf)) return false;
			sscanf(tmpBuf.c_str(), " %d", &m_aFontMetrics[i].iEndX);
			if (!fmtRead(file, "StartY", tmpBuf)) return false;
			sscanf(tmpBuf.c_str(), " %d", &m_aFontMetrics[i].iStartY);
			if (!fmtRead(file, "EndY", tmpBuf)) return false;
			sscanf(tmpBuf.c_str(), " %d", &m_aFontMetrics[i].iEndY);
	}

	file.Close();

	// now compute the names
	std::string fullName = file.GetStdName();
	char *temp;

	// get just the file part
	temp = strrchr((char*)fullName.c_str(), '/');
	if (temp)
		m_szFaceName = temp + 1;
	else
		m_szFaceName = fullName;

	// now get the texture name
	m_szTexture = file.GetStdName();

	// now wack off the extension;
	if (extension)
		m_szFaceName.erase(m_szFaceName.size() - (strlen(extension) + 1), m_szFaceName.size());

	temp = strrchr((char*)m_szFaceName.c_str(), '_');

	if (temp) {
		m_iSize = atoi(temp+1);
		m_szFaceName.resize(temp - m_szFaceName.c_str());
	}

	// faceName.erase(faceName.size()-strlen(temp),faceName.size());

	if (extension)
		m_szTexture.erase(m_szTexture.size() - (strlen(extension) + 1), m_szTexture.size());

	return (m_iNumberOfCharacters > 0);
}

void CTextureFont::build ( int list )
{
	std::map<int,int>::iterator itr = listToCharMap.find(list);
	if (itr == listToCharMap.end())
		return;

	int i = itr->second;

	glTranslatef((float)m_aFontMetrics[i].iInitalDist,0,0);

	float fFontY = (float)m_aFontMetrics[i].iEndY-m_aFontMetrics[i].iStartY;
	float fFontX = (float)m_aFontMetrics[i].iEndX-m_aFontMetrics[i].iStartX;

	glBegin(GL_QUADS);
	glNormal3f(0,0,1);
	glTexCoord2f((float)m_aFontMetrics[i].iStartX/(float)m_iTextureXSize,1.0f-(float)m_aFontMetrics[i].iStartY/(float)m_iTextureYSize);
	glVertex3f(0,fFontY,0);

	glTexCoord2f((float)m_aFontMetrics[i].iStartX/(float)m_iTextureXSize,1.0f-(float)m_aFontMetrics[i].iEndY/(float)m_iTextureYSize);
	glVertex3f(0,0,0);

	glTexCoord2f((float)m_aFontMetrics[i].iEndX/(float)m_iTextureXSize,1.0f-(float)m_aFontMetrics[i].iEndY/(float)m_iTextureYSize);
	glVertex3f(fFontX,0,0);

	glTexCoord2f((float)m_aFontMetrics[i].iEndX/(float)m_iTextureXSize,1.0f-(float)m_aFontMetrics[i].iStartY/(float)m_iTextureYSize);
	glVertex3f(fFontX,fFontY,0);
	glEnd();

	glTranslatef(fFontX,0,0);
}


void CTextureFont::PreLoadLists ( void )
{
	if (m_szTexture.size()<1)
		return;

	// load up the texture
	m_iTextureID = CTextureManager::instance().GetID(m_szTexture.c_str());
	
	if (m_iTextureID == -1)
		return;

	for ( int i = 0; i < m_iNumberOfCharacters; i++)
	{
		m_aListIDs[i] = DisplayListManager::instance().newList(this,true);
		listToCharMap[m_aListIDs[i]] = i;
		DisplayListManager::instance().rebuildList(m_aListIDs[i]);
	}
}


float CTextureFont::GetStrLength ( float fScale, const char *szString )
{
	int	iLen = (int)strlen(szString);
	int iCharToUse = 0;
	int iLastCharacter = 0;

	float fTotalLen = 0;
	float fThisPassLen = 0;

	for (int i = 0; i< iLen; i++)
	{

		if (szString[i] == '\n')	// newline, get back to the intial X and push down
		{
			fThisPassLen = 0;
		}
		else
		{
			iLastCharacter = iCharToUse;
			if ( (szString[i] < 32) || (szString[i] < 9))
				iCharToUse = 32;
			else if (szString[i] > m_iNumberOfCharacters+32)
				iCharToUse = 32;
			else
				iCharToUse = szString[i];

			iCharToUse -= 32;

			if (iCharToUse== 0)
			{
				if (i == 0)
					fThisPassLen += m_aFontMetrics[iCharToUse].iInitalDist + m_aFontMetrics[iCharToUse].iCharWidth+m_aFontMetrics[iCharToUse].iWhiteSpaceDist; 
				else
					fThisPassLen += m_aFontMetrics[iLastCharacter].iWhiteSpaceDist+m_aFontMetrics[iCharToUse].iWhiteSpaceDist+m_aFontMetrics[iCharToUse].iInitalDist + m_aFontMetrics[iCharToUse].iCharWidth; 
			}
			else
			{
				float fFontX = (float)m_aFontMetrics[iCharToUse].iEndX-m_aFontMetrics[iCharToUse].iStartX;
				fThisPassLen += fFontX+(float)m_aFontMetrics[iCharToUse].iInitalDist;
			}
		}
		if (fThisPassLen >fTotalLen)
			fTotalLen = fThisPassLen;
	}

	return fTotalLen * fScale;
}

void CTextureFont::Free ( void )
{
	m_iTextureID = -1;
}

void CTextureFont::DrawString ( float fScale, CColor &oColor, const char *szString )
{
	if (!szString)
		return;

	if (m_iTextureID == -1)
		PreLoadLists();

	if (m_iTextureID == -1)
		return;

	CTextureManager::instance().Bind(m_iTextureID);

	oColor.glColor();

	glPushMatrix();
	glScalef(fScale,fScale,1);
		
	glPushMatrix();
	int	iLen = (int)strlen(szString);
	int iCharToUse = 0;
	int iLastCharacter = 0;
	for (int i = 0; i< iLen; i++)
	{
		if (szString[i] == '\n')	// newline, get back to the intial X and push down
		{
			glPopMatrix();
			glTranslatef(0,-(float)m_iTextureZStep,0);
			glPushMatrix();
		}
		else
		{
			iLastCharacter = iCharToUse;
			if ( (szString[i] < 32) || (szString[i] < 9))
				iCharToUse = 32;
			else if (szString[i] > m_iNumberOfCharacters+32)
				iCharToUse = 32;
			else
				iCharToUse = szString[i];

			iCharToUse -= 32;

			if (iCharToUse== 0)
			{
				if (i == 0)
					glTranslatef((float)m_aFontMetrics[iCharToUse].iInitalDist + (float)m_aFontMetrics[iCharToUse].iCharWidth+(float)m_aFontMetrics[iCharToUse].iWhiteSpaceDist,0,0); 
				else
					glTranslatef((float)m_aFontMetrics[iLastCharacter].iWhiteSpaceDist+(float)m_aFontMetrics[iCharToUse].iWhiteSpaceDist+m_aFontMetrics[iCharToUse].iInitalDist + (float)m_aFontMetrics[iCharToUse].iCharWidth,0,0); 
			}
			else
				DisplayListManager::instance().callList(m_aListIDs[iCharToUse]);
		}
	}
	glPopMatrix();
	glColor4f(1,1,1,1);
	glPopMatrix();
}

/*
typedef std::map<int,CTextureFont*> tmFontSizeMap;
typedef std::vector<tmFontSizeMap>	tmFontFaceList;
typedef std::map<std::string,int> tmFontFaceMap;
*/

template <>
CFontManager* Singleton<CFontManager>::_instance = (CFontManager*)0;

CFontManager::CFontManager()
{
	m_mFaceNames.clear();
	m_vFontFaces.clear();
}

CFontManager::~CFontManager()
{
	m_mFaceNames.clear();

	tmFontFaceList::iterator faceItr = m_vFontFaces.begin();

	while (faceItr != m_vFontFaces.end())
	{
		tmFontSizeMap::iterator itr = faceItr->begin();

		while (itr != faceItr->end())
		{
			delete(itr->second);
			itr++;
		}
		faceItr++;
	}
}

void CFontManager::LoadAll ( const char* szDir )
{
	if (!szDir)
		return;

	COSFile	oFile;

	COSDir	oDir(szDir);

	while (oDir.GetNextFile(oFile,true))
	{
		const char *szExt = oFile.GetExtension();

		if (szExt)
		{
			if (strcasecmp(szExt,"fmt") == 0)
			{
				CTextureFont	*pFont = new CTextureFont;
				if (pFont)
				{
					if (pFont->Load(oFile))
					{
						std::string		str = pFont->GetFaceName();
						GetTypeFaceName((char*)str.c_str());

						tmFontFaceMap::iterator faceItr = m_mFaceNames.find(str);
						
						int faceID = 0;
						if (faceItr == m_mFaceNames.end())
						{
							// its new
							tmFontSizeMap	faceList;
							m_vFontFaces.push_back(faceList);
							faceID = (int)m_vFontFaces.size()-1;
							m_mFaceNames[str] = faceID;
						}
						else
							faceID = faceItr->second;

						m_vFontFaces[faceID][pFont->GetSize()] = pFont;
					}
					else
					{
						delete(pFont);
					}
				}
			}
		}
	}
}

int CFontManager::GetFaceID ( const char *szFaceName )
{
	if (!szFaceName)
		return -1;

	std::string	str = szFaceName;

	GetTypeFaceName((char*)str.c_str());
	
	tmFontFaceMap::iterator faceItr = m_mFaceNames.find(str);
	
	int faceID = 0;
	if (faceItr == m_mFaceNames.end())
	{
		// see if there is a default;
		str = "DEFAULT";
		faceItr = m_mFaceNames.find(str);
		if (faceItr == m_mFaceNames.end())
		{
			// see if we have arial
			str = "ARIAL";
			faceItr = m_mFaceNames.find(str);
			if (faceItr == m_mFaceNames.end())
			{
				// hell we are outa luck, you just get the first one
				faceItr = m_mFaceNames.begin();
				if (faceItr == m_mFaceNames.end())
					return -1;	//we must have NOT fonts, so you are screwed no mater what
			}
		}
	}

	return faceID = faceItr->second;
}

int CFontManager::GetNumFaces ( void )
{
	return (int)m_vFontFaces.size();
}

const char* CFontManager::GetFaceName ( int iFaceID )
{
	if ( (iFaceID<0)|| (iFaceID>GetNumFaces()) )
		return NULL;

	return m_vFontFaces[iFaceID].begin()->second->GetFaceName();
}

void CFontManager::DrawString ( CGLPoint &oPos, CColor &oColor, int iFaceID, int iSize, const char *szText )
{
	DrawString(oPos.X(),oPos.Y(),oPos.Z(),oColor,iFaceID,iSize,szText);
}

void CFontManager::DrawString ( CGLPoint &oPos,CColor &oColor, const char *szFace, int iSize, const char *szText )
{
	DrawString(oPos,oColor,GetFaceID(szFace),iSize,szText);
}

void CFontManager::DrawString ( float x, float y, float z, CColor &oColor, int iFaceID, int iSize, const char *szText )
{
	if ( !szText || (iFaceID<0) || (iFaceID>GetNumFaces()) )
		return;

	CTextureFont* pFont = GetClosestSize(iFaceID,iSize);

	if (!pFont)
		return;

	float fScale = (float)iSize/(float)pFont->GetSize();

	glPushMatrix();
	glTranslatef(x,y,z);
	GLboolean  depthMask;
	glGetBooleanv(GL_DEPTH_WRITEMASK,&depthMask);
	glDepthMask(0);
	pFont->DrawString(fScale,oColor,szText);
	glDepthMask(depthMask);
	glPopMatrix();
}

void CFontManager::DrawString ( float x, float y, float z, CColor &oColor, const char *szFace, int iSize, const char *szText )
{
	DrawString(x,y,z,oColor,GetFaceID(szFace),iSize,szText);
}

float CFontManager::GetStrLength ( int iFaceID, int iSize, const char *szText)
{
	if ( !szText || (iFaceID<0) || (iFaceID>GetNumFaces()) )
		return 0;

	CTextureFont* pFont = GetClosestSize(iFaceID,iSize);

	if (!pFont)
		return 0;

	float fScale = (float)iSize/(float)pFont->GetSize();

	return pFont->GetStrLength(fScale,szText);
}

float CFontManager::GetStrLength ( const char *szFace, int iSize, const char *szText)
{
	return GetStrLength(GetFaceID(szFace),iSize,szText);
}

float CFontManager::GetStrHeight ( int iFaceID, int iSize, const char *szText)
{
	int iLines = 1;

	int iLen = (int)strlen(szText);

	for (int i = 0; i < iLen; i++)
	{
		if (szText[i] == '\n')
			iLines++;
	}

	return (float)(iLines*iSize);
}

float CFontManager::GetStrHeight ( const char *szFace, int iSize, const char *szText)
{
	return GetStrHeight(GetFaceID(szFace),iSize,szText);
}

void CFontManager::UnloadAll ( void )
{
	tmFontFaceList::iterator faceItr = m_vFontFaces.begin();

	while (faceItr != m_vFontFaces.end())
	{
		tmFontSizeMap::iterator itr = faceItr->begin();

		while (itr != faceItr->end())
		{
			itr->second->Free();
			itr++;
		}
		faceItr++;
	}
}


CTextureFont* CFontManager::GetClosestSize ( int iFaceID, int iSize )
{
	if (m_vFontFaces[iFaceID].size() == 0)
		return NULL;

	// only have 1 so this is easy
	if (m_vFontFaces[iFaceID].size() == 1)
		return m_vFontFaces[iFaceID].begin()->second;

	// find the first one that is equal or biger
	tmFontSizeMap::iterator itr = m_vFontFaces[iFaceID].begin();

	CTextureFont*	pFont = NULL;

	while (itr != m_vFontFaces[iFaceID].end())
	{
		if (iSize <= itr->second->GetSize())
		{
			pFont = itr->second;
			itr = m_vFontFaces[iFaceID].end();
		}
		else
			itr++;
	}
	// if we don't have one that is larger then take the largest one we have and pray for good scaling
	if (!pFont)	
		pFont = m_vFontFaces[iFaceID].rbegin()->second;

	return pFont;
}
