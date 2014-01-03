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
// Texture Manager.h

#include "openGL.h"

#include "singleton.h"

#define _USE_LIB_IL_

#include "OSFile.h"
#include "3dTypes.h"

#ifndef _Texture_Manager_H_
#define _Texture_Manager_H_

typedef struct 
{
	unsigned short	x,y;
	bool			alpha;
	GLuint	glID;
	int				manID;
	COSFile			file;
	char			name[64];
	int				type;
	bool			skin;
	int				size;
	float			lastUse;
}trTextureInfo;

class CTextureManager : public Singleton<CTextureManager>
{
protected:
	friend class Singleton<CTextureManager>;
	CTextureManager();
	~CTextureManager();

public:

	// these don't actualy load the texture they just get all the file info so it can be bound
	int	LoadOne ( const char * szPath, bool	bSkin = false );
	int	LoadAll ( const char * szPath, bool	bSkin = false );

	int	GetID ( const char * szName );
	void Preload ( unsigned int iID );

	bool Bind ( unsigned int iID );
	bool Flush ( int iID = -1 );

	void UnloadAll ( void );
	
	bool GetInfo ( unsigned int iID, trTextureInfo** pInfo );
	bool GetInfo ( char * szName, trTextureInfo** pInfo );

	int GetImageWidth ( unsigned int iID );
	int GetImageWidth ( char * szName );
	int GetImageHeight ( unsigned int iID );
	int GetImageHeight ( char * szName );

	int	GetNumTexures ( void );
	int	GetNumTexturesLoaded ( void );
	int	GetTextureMemUsed ( void );

	bool GetImageData ( unsigned int iID, unsigned char *spData );

	int	GetIDNoLoad ( const char * szName );

	void SetLastBoundGLID ( unsigned int iID );

	void SetTextureExpire ( bool bSet ) {m_bUseTimer = bSet;}

	void PollTextures ( void );

	void SaveScreenCapture ( const char *szFileName );

	void GetScreenCapture ( void *data );
	
	// drawing utilitys
	void drawImage ( int iID, C3DVertex &pos, bool center = true, float uScale = 1.0f, float vScale = 1.0f, float alpha = 1.0f, CColor color = CColor(1,1,1) );
	void drawImage ( char * szName, C3DVertex &pos, bool center = true, float uScale = 1.0f, float vScale = 1.0f, float alpha = 1.0f, CColor color = CColor(1,1,1) );
private:

	int	Load ( COSFile &rFile, const char *szName, bool bSkin = false  );
	
	int	LoadBMP ( trTextureInfo	*pTexInfo, bool	bSkin = false, unsigned char *pData = NULL );
	int	LoadTGA ( trTextureInfo	*pTexInfo, bool	bSkin = false, unsigned char *pData = NULL );
	int	LoadPNG ( trTextureInfo	*pTexInfo, bool	bSkin = false, unsigned char *pData = NULL );

	int LoadFile ( trTextureInfo &rTexInfo );

	struct trTextureManagerInfo;

	trTextureManagerInfo	*info;

	bool			m_bUseTimer;
	float			m_fTextureTimeout;
};

#endif //_Texture_Manager_H_
