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
// TextureMngr.cpp : Defines the entry point for the DLL application.
//

#ifdef _WIN32
  #define WIN32_LEAN_AND_MEAN		// Exclude rarely-used stuff from Windows headers
  #include <windows.h>
  // windows and AIX use stricmp, POSIX uses strcasecmp.
  #define strcasecmp stricmp
#else
  #include <sys/types.h>
  #include <sys/stat.h>
  #include <unistd.h>
  #include <ctype.h>
  #include <stddef.h>
#endif

#ifdef _WIN32
#pragma warning( disable : 4786 )  // Disable warning message
#pragma warning( disable : 4181 )  // Disable warning message
#endif

#include <string>
#include <vector>
#include <map>
#include <set>

#include <stdio.h>

#include "textureManager.h"
#include "OGFTimer.h"
#include "openGL.h"


#ifdef _USE_LIB_IL_
// include the devIL headers including the GL utilitys
#include <IL/il.h>
#include <IL/ilu.h>
#include <IL/ilut.h>
#endif


inline  bool operator < (const std::string &s1,const std::string &s2) { return (s1.compare(s2)<0);}
typedef std::map<std::string,int>  tmTextureNameMap;
typedef std::vector<trTextureInfo>  tvTextureList;
typedef std::set<trTextureInfo*>  tvTextureRefList;

#define _NO_TEXTURE_ID 0xffffffff

void GetTextureName ( char *szData )
{
#ifdef	_WIN32
	strupr(szData);
#else
	while(*szData++)
		*szData = toupper(*szData);
#endif
}

template <>
CTextureManager* Singleton<CTextureManager>::_instance = (CTextureManager*)0;


struct CTextureManager::trTextureManagerInfo
{
	tmTextureNameMap			m_mTextureMap;
	tvTextureList				m_vTextureList;
	tvTextureRefList			m_vBoundList;

	unsigned int				m_uiLastBindID;
	int									m_iTextureMemUsed;
	CLGETime						m_oTime;
};

CTextureManager::CTextureManager()
{
	info = new trTextureManagerInfo;

	info->m_uiLastBindID = _NO_TEXTURE_ID;
	info->m_iTextureMemUsed = 0;

	m_fTextureTimeout = 10.0f;
	info->m_oTime.Init();

#ifdef _USE_LIB_IL_
	ILuint Error;

	ilInit();
	Error = ilGetError();

	// tell devIL to use GL as it's thingy
	ilutRenderer(ILUT_OPENGL);
#endif
}

CTextureManager::~CTextureManager()
{
	Flush(-1);

	if (!info)
		return;

	info->m_mTextureMap.clear();
	info->m_vTextureList.clear();

	delete(info);
	info = NULL;
}

bool isValidExtension ( const char *extension )
{
	if (strcasecmp(extension,"BMP")==0)
		return true;
	else if (strcasecmp(extension,"TGA")==0 )
		return true;
	else if (strcasecmp(extension,"PNG")==0 )
		return true;
	else if (strcasecmp(extension,"JPG")==0 )
		return true;
	else if (strcasecmp(extension,"JPEG")==0 )
		return true;
	else if (strcasecmp(extension,"SGI")==0 )
		return true;
	else if (strcasecmp(extension,"LBM")==0 )
		return true;
	else if (strcasecmp(extension,"PCX")==0 )
		return true;
	else if (strcasecmp(extension,"PSD")==0 )
		return true;
	else if (strcasecmp(extension,"RAW")==0 )
		return true;
	else if (strcasecmp(extension,"TIF")==0 )
		return true;
	else if (strcasecmp(extension,"PIC")==0 )
		return true;

	return false;
}

void CTextureManager::SaveScreenCapture ( const char *szFileName )
{
	if (!szFileName)
		return;

	std::string file = szFileName;
	std::string ext = ".png";
	file += ext;

	COSFile	theFile(file.c_str());

#ifdef _USE_LIB_IL_

	ILuint id;
	ilGenImages(1, &id);
	ilBindImage(id);

	ilutGLScreen();
	ilSaveImage((ILstring)theFile.GetOSName());  // Loads into the current bound image

	ilDeleteImages(1, &id);

#else
	printf("can't save a screenshot with out libIL");
#endif
}

void CTextureManager::GetScreenCapture ( void *data )
{
	if (!data)
		return;

#ifdef _USE_LIB_IL_

	ILuint id;
	ilGenImages(1, &id);
	ilBindImage(id);

	ilutGLScreen();

	int bytesPerPix = ilGetInteger(IL_IMAGE_BYTES_PER_PIXEL);
	int x = ilGetInteger(IL_IMAGE_WIDTH);
	int y = ilGetInteger(IL_IMAGE_HEIGHT);

	int size = bytesPerPix * x * y;
	memcpy(data,ilGetData(),size);
	ilDeleteImages(1, &id);

#else
	printf("can't get a screenshot with out libIL");
#endif
}

int	CTextureManager::Load ( COSFile &rFile, const char *szName, bool bSkin )
{
	trTextureInfo	rTexInfo;// = (trTextureInfo*)malloc(sizeof(trTextureInfo));

	rTexInfo.alpha = 0;
	rTexInfo.size = 0;
	rTexInfo.x = 0;
	rTexInfo.y = 0;
	rTexInfo.manID = 0;

	rTexInfo.lastUse = -1;
	
		// get a name for it
	char	extension[32] = {0};

	rFile.GetExtension();
	
	if (!rFile.GetExtension())
		return-1;

	strcpy(extension,rFile.GetExtension());
	if (!extension[0])
		return -1;

#ifdef _USE_LIB_IL_
	if (!isValidExtension(extension))
		return -1;

	rTexInfo.type = 0;

	// pngs are "special', we have to flip em
	if (strcasecmp(extension,"PNG")==0 )
		rTexInfo.type = 2;
	else if (strcasecmp(extension,"TGA")==0 )
		rTexInfo.type = 1;

#else
	if (strcasecmp(extension,"BMP")==0)
		rTexInfo.type = 0;
	else if (strcasecmp(extension,"TGA")==0 )
		rTexInfo.type = 1;
	else if (strcasecmp(extension,"PNG")==0 )
		rTexInfo.type = 2;
	else
		return -1; 
#endif //_USE_LIB_IL_

	rTexInfo.skin = bSkin;
	rTexInfo.glID = 0xffffffff;
	rTexInfo.manID = (int)info->m_vTextureList.size();
	rTexInfo.file = rFile;

	char	szRealName[1024];

	strcpy(szRealName,szName);

	GetTextureName(szRealName);

	strcpy(rTexInfo.name,szRealName);
	std::string	str = szRealName;
	info->m_mTextureMap[str] = rTexInfo.manID;
	info->m_vTextureList.push_back(rTexInfo);

	return rTexInfo.manID;
}

int CTextureManager::LoadOne ( const char * szPath, bool bSkin )
{
	if (!szPath)
		return -1;

	COSFile	oFile(szPath);

	char	szName[512];

	strcpy(szName,oFile.GetStdName());

	if (strrchr(szName,'.'))
		*(strrchr(szName,'.')) = '\0';

	return Load(oFile,szName,bSkin);
}

int CTextureManager::LoadAll ( const char * szPath, bool bSkin )
{	
	COSDir	oDir(szPath);
	COSFile	oFile;
	char	szName[512];

	while (oDir.GetNextFile(oFile,true))
	{
		strcpy(szName,oFile.GetStdName());

		if (strrchr(szName,'.'))
			*(strrchr(szName,'.')) = '\0';

		Load(oFile,szName,bSkin);
	}

	return 1;
}

int CTextureManager::GetNumTexures ( void )
{
	return (int)info->m_vTextureList.size();
}

int CTextureManager::GetID ( const char * szName )
{
	int id = GetIDNoLoad(szName);

	if ( (&id >=0 ) && (info->m_vTextureList[id].glID == _NO_TEXTURE_ID) )
	{
		LoadFile(info->m_vTextureList[id]);
		info->m_iTextureMemUsed += info->m_vTextureList[id].size;
	}
	return id;
}

int	CTextureManager::GetIDNoLoad ( const char * szName )
{
	std::string	str = szName;

	GetTextureName((char*)str.c_str());

	tmTextureNameMap::iterator	itr = info->m_mTextureMap.find(str);
	
	if (itr == info->m_mTextureMap.end())
		return -1;
	else
		return itr->second;
}

void CTextureManager::Preload ( unsigned int iID )
{
	if ( iID > info->m_vTextureList.size())
		return;

	if (info->m_vTextureList[iID].glID != _NO_TEXTURE_ID)
		return;

	LoadFile(info->m_vTextureList[iID]);
	info->m_iTextureMemUsed += info->m_vTextureList[iID].size;
}

bool CTextureManager::GetInfo ( unsigned int iID, trTextureInfo** pInfo )
{
	if ( (iID>=info->m_vTextureList.size()) || !pInfo)
		return false;

	*pInfo = &info->m_vTextureList[iID];
	return true;
}

int CTextureManager::GetImageWidth ( unsigned int iID )
{
	if ( (iID>=info->m_vTextureList.size()))
		return -1;

	return	info->m_vTextureList[iID].x;
}

int CTextureManager::GetImageHeight ( unsigned int iID )
{
	if ( (iID>=info->m_vTextureList.size()))
		return -1;

	return	info->m_vTextureList[iID].y;
}

int CTextureManager::GetImageWidth ( char * szName )
{
	return GetImageWidth(GetID(szName));
}

int CTextureManager::GetImageHeight ( char * szName )
{
	return GetImageHeight(GetID(szName));
}

bool CTextureManager::Bind ( unsigned int iID )
{
	if ( iID >= info->m_vTextureList.size() )
		return false;

	if (info->m_vTextureList[iID].glID == _NO_TEXTURE_ID)
	{
		LoadFile(info->m_vTextureList[iID]);
		info->m_iTextureMemUsed += info->m_vTextureList[iID].size;

	}

	if (info->m_vTextureList[iID].glID == _NO_TEXTURE_ID)
		return false; // it failed
	
	info->m_vTextureList[iID].lastUse = (float)info->m_oTime.GetTime();
	info->m_vBoundList.insert(&(info->m_vTextureList[iID]));
	glBindTexture(GL_TEXTURE_2D,info->m_vTextureList[iID].glID);
	return true;
}

bool CTextureManager::Flush ( int iID )
{
	if (iID == -1)
	{
		tvTextureList::iterator itr = info->m_vTextureList.begin();

		while (itr !=  info->m_vTextureList.end())
		{
			if ((*itr).glID != _NO_TEXTURE_ID)
			{
				glDeleteTextures(1,(GLuint *)&((*itr).glID));
				info->m_iTextureMemUsed -= itr->size;

				(*itr).glID = _NO_TEXTURE_ID;
			}
			itr++;
		}
		info->m_vTextureList.clear();
		info->m_mTextureMap.clear();
		return true;
	}
	else
	{
		if ( (iID<0) || (iID>=(int)info->m_vTextureList.size()) )
			return false;

		if (info->m_vTextureList[iID].glID != _NO_TEXTURE_ID)
		{
			glDeleteTextures(1,(GLuint *)&info->m_vTextureList[iID].glID);
			info->m_iTextureMemUsed -= info->m_vTextureList[iID].size;
		}

		info->m_vTextureList[iID].glID = _NO_TEXTURE_ID;
		return true;
	}
}

void CTextureManager::UnloadAll ( void )
{
	if (info->m_vTextureList.size() <1)
		return;

	glBindTexture(GL_TEXTURE_2D,0);

	tvTextureList::iterator itr = info->m_vTextureList.begin();

	while (itr !=  info->m_vTextureList.end())
	{
		if ((*itr).glID != _NO_TEXTURE_ID)
		{
			glDeleteTextures(1,(GLuint *)&((*itr).glID));
			info->m_iTextureMemUsed -= itr->size;

			(*itr).glID = _NO_TEXTURE_ID;
		}
		itr++;
	}

	info->m_uiLastBindID = _NO_TEXTURE_ID;
}

void CTextureManager::PollTextures ( void )
{
	if (!m_bUseTimer)
		return;

	float	fThisTime = (float)info->m_oTime.GetTime();

	tvTextureRefList::iterator itr = info->m_vBoundList.begin();

	while (itr != info->m_vBoundList.end())
	{
		if (fThisTime - (*itr)->lastUse > m_fTextureTimeout)
		{
			glDeleteTextures(1,(GLuint *)&(*itr)->glID);
			(*itr)->glID = _NO_TEXTURE_ID;
			info->m_iTextureMemUsed -= (*itr)->size;
			info->m_vBoundList.erase(itr);
		}
		else
			itr++;
	}
}

void CTextureManager::drawImage ( int iID, C3DVertex &pos, bool center, float uScale, float vScale, float alpha, CColor color )
{
	if (iID < 0)
		return ;

	C3DVertex realPos = pos;

	float XOffset = GetImageWidth(iID)/2.0f;
	float YOffset = GetImageHeight(iID)/2.0f;

	if (!center)
	{	
		realPos.IncrementX(-XOffset*uScale);
		realPos.IncrementY(-YOffset*vScale);
	}

	glPushMatrix();
	glTranslatef(realPos.X(),realPos.Y(),realPos.Z());

	color.glColor(alpha);

	Bind(iID);

	glBegin(GL_QUADS);

	float depth = 0;

	glTexCoord2f(0,0);
	glVertex3f(-XOffset*uScale,-YOffset*vScale,depth);

	glTexCoord2f(1,0);
	glVertex3f(XOffset*uScale,-YOffset*vScale,depth);

	glTexCoord2f(1,1);
	glVertex3f(XOffset*uScale,YOffset*vScale,depth);

	glTexCoord2f(0,1);
	glVertex3f(-XOffset*uScale,YOffset*vScale,depth);

	glEnd();
	glColor4f(1,1,1,1);
	glPopMatrix();
}

void CTextureManager::drawImage ( char *szName, C3DVertex &pos, bool center, float uScale, float vScale, float alpha, CColor color )
{
	drawImage(GetID(szName),pos,center,uScale,vScale,alpha,color);
}


