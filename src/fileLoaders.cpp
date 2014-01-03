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

#ifdef _WIN32
#include <windows.h>
#else
#  include "bmp.h"
#endif

#include "textureManager.h"
#include "openGL.h"

// include the devIL headers including the GL utilitys
#include <IL/ilut.h>

#include <stdio.h>

bool useSimpleIL = false;

// this file contains all the various file loaders for the Texture mananger class

int CTextureManager::LoadFile ( trTextureInfo &rTexInfo )
{
	if (rTexInfo.glID != 0xffffffff)
		Flush(rTexInfo.manID);

	rTexInfo.glID = 0xffffffff;

//	return  ilutGLLoadImage((ILstringpTexInfo.file.GetOSName());
#ifdef _USE_LIB_IL_
	if (useSimpleIL)
	{
		rTexInfo.glID = ilutGLLoadImage((ILstring)rTexInfo.file.GetOSName());
	}
	else
	{
		ILuint id;
		ilGenImages(1, &id);
		ilBindImage(id);
		ilLoadImage((ILstring)rTexInfo.file.GetOSName());  // Loads into the current bound image
		int Error = ilGetError();
		if (Error)
			return 0;

		rTexInfo.glID  = 0;

		glGenTextures(1,&rTexInfo.glID );
		// and in the darkness bind it
		glBindTexture(GL_TEXTURE_2D,rTexInfo.glID );

	//	rTexInfo.glID  = ilutGLBindMipmaps();

		Error = ilGetError();
		if (Error)
			return 0;

		int	iEdgeType = 0;

		if (rTexInfo.skin)
			iEdgeType = GL_CLAMP;
		else
			iEdgeType = GL_REPEAT;

		glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_WRAP_S, iEdgeType);
		glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_WRAP_T, iEdgeType);
		glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_MAG_FILTER, GL_LINEAR_MIPMAP_LINEAR);
		glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_MIN_FILTER, GL_LINEAR_MIPMAP_LINEAR);
		glHint(GL_PERSPECTIVE_CORRECTION_HINT, GL_NICEST); 


		GLenum	eFormat = 0;

		int bytesPerPix = ilGetInteger(IL_IMAGE_BYTES_PER_PIXEL);

		rTexInfo.alpha = bytesPerPix > 3;
		rTexInfo.x = ilGetInteger(IL_IMAGE_WIDTH);
		rTexInfo.y = ilGetInteger(IL_IMAGE_HEIGHT);

	if (bytesPerPix == 1)
			eFormat = GL_ALPHA;
		else if (bytesPerPix == 3)
		{
			if (rTexInfo.type == 1)	// it's a TGA and is backwards in color
			{
#ifdef GL_VERSION_1_2
				eFormat = GL_BGR;
#else
				eFormat = GL_BGR_EXT;
#endif
			}
			else
			{
				eFormat = GL_RGB;
			}
		}
		else if (bytesPerPix == 4)
		{
			if (rTexInfo.type == 1)	// it's a TGA and is backwards in color
			{
#ifdef GL_VERSION_1_2
				eFormat = GL_BGRA;
#else
				eFormat = GL_BGRA_EXT;
#endif
			}
			else
				eFormat = GL_RGBA;
		}

		if (rTexInfo.type == 2)
			iluFlipImage();

		gluBuild2DMipmaps(GL_TEXTURE_2D,bytesPerPix,rTexInfo.x,rTexInfo.y,eFormat,GL_UNSIGNED_BYTE,ilGetData());


		ilDeleteImages(1, &id);
	}
#else
	switch(rTexInfo.type)
	{
		case 0:
			rTexInfo.glID = LoadBMP(&rTexInfo,rTexInfo.skin);
			break;

		case 1:
			rTexInfo.glID = LoadTGA(&rTexInfo,rTexInfo.skin);
			break;

		case 2:
			break;

		default:
			return -1; 
	}
#endif //_USE_LIB_IL_
	return 1;
}

bool CTextureManager::GetImageData ( unsigned int iID, unsigned char *spData )
{
	trTextureInfo	*pInfo;

	if (!GetInfo(iID,&pInfo) || !spData )
		return false;
	
	trTextureInfo	rTempInfo;

	memcpy(&rTempInfo,pInfo,sizeof(trTextureInfo));

	switch(rTempInfo.type)
	{
		case 0:
			LoadBMP(&rTempInfo,false,spData);
			break;

		case 1:
			LoadTGA(&rTempInfo,false,spData);
			break;

		default:
			return false;
	}

	if (pInfo->glID == 0xffffffff)
	{
		pInfo->x = rTempInfo.x;
		pInfo->y = rTempInfo.y;
		pInfo->alpha = rTempInfo.alpha;
	}
	return true;
}


int	CTextureManager::LoadBMP ( trTextureInfo *pTexInfo, bool bSkin, unsigned char *pData )
{
	unsigned int				iImageH, iImageW,offset;
	BITMAPFILEHEADER	FileHeader;
	BITMAPINFOHEADER	InfoHeader;

	GLuint		iglID = 0;
	
	unsigned char		*pImageData;

	COSFile	&pFile = pTexInfo->file;

	if (!pFile.Open("rb"))
		return -1;

	// first parts of a BMP
	pFile.Read(&FileHeader,sizeof(BITMAPFILEHEADER));
	pFile.Read(&InfoHeader,sizeof(BITMAPINFOHEADER));

	if (InfoHeader.biBitCount < 8)
		return -1;

	iImageH = InfoHeader.biHeight;
	iImageW = InfoHeader.biWidth;

	// read in h*w*3 24 bit only
	pImageData = (unsigned char*)malloc(iImageH*iImageW*3);
	offset = FileHeader.bfOffBits;
	pFile.Seek(eFileStart,offset);

	pFile.Read(pImageData,iImageH*iImageW*3);
	pFile.Close();

	int	iEdgeType = 0;

	if (!pData)
	{
		// get us a new Texture ID
		glGenTextures(1,&iglID);
		// and in the darkness bind it
		glBindTexture(GL_TEXTURE_2D,iglID);

		// set the params

		if (bSkin)
			iEdgeType = GL_CLAMP;
		else
			iEdgeType = GL_REPEAT;

		glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_WRAP_S, iEdgeType);
		glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_WRAP_T, iEdgeType);
		glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_MAG_FILTER, GL_LINEAR_MIPMAP_LINEAR);
		glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_MIN_FILTER, GL_LINEAR_MIPMAP_LINEAR);
		glTexEnvf(GL_TEXTURE_ENV, GL_TEXTURE_ENV_MODE, GL_MODULATE);
		glHint(GL_PERSPECTIVE_CORRECTION_HINT, GL_NICEST); 

		// make it a GL texture object and mip map it
		GLenum	eFormat = 0;

		if (InfoHeader.biBitCount == 8)
			eFormat = GL_ALPHA;
		else if (InfoHeader.biBitCount == 24)
		{
#ifdef GL_VERSION_1_2
		eFormat = GL_BGR;
#else
		eFormat = GL_BGR_EXT;
#endif
		}
		else if (InfoHeader.biBitCount == 32)
		{
#ifdef GL_VERSION_1_2
		eFormat = GL_BGRA;
#else
		eFormat = GL_BGRA_EXT;
#endif
		}

		gluBuild2DMipmaps(GL_TEXTURE_2D,InfoHeader.biBitCount/8,iImageW,iImageH,eFormat,GL_UNSIGNED_BYTE,pImageData);
	}
	else
		memcpy(pData,pImageData,InfoHeader.biBitCount/8*iImageW*iImageH);

	free(pImageData);

	pTexInfo->x = iImageW;
	pTexInfo->y = iImageH;
	pTexInfo->alpha = false;

	pTexInfo->size = iImageW * iImageH * 3;


	return (int)iglID;
}

bool checkSize(int x) 
{
  if(x == 2 || x == 4 || x == 8 || x == 16 || x == 32 || x == 64 || x == 128 || x == 256 || x == 512 || x == 1024)
    return true;
  
  return false;
}

unsigned char *getRGBA(COSFile &oFile, int size,GLenum &texFormat)
{
	unsigned char *rgba;
	unsigned char temp;
	int bread;

	rgba = (unsigned char *) malloc(size * 4);
	if(rgba == NULL)
		return NULL;

	bread = oFile.Read(rgba, sizeof(unsigned char), size * 4);
	if(bread != size * 4)
	{
		free(rgba);
		return NULL;
	}

	for(int i = 0; i < size * 4; i += 4)
	{
		temp = rgba[i];
		rgba[i] = rgba[i + 2];
		rgba[i + 2] = temp;
	} 
	
	texFormat = GL_RGBA;
	return rgba;
}

unsigned char *getRGB(COSFile &oFile, int size,GLenum &texFormat)
{
	unsigned char *rgb;
	unsigned char temp;
	int bread;

	rgb = (unsigned char *) malloc(size * 3);

	if(rgb == NULL)
		return NULL;

	bread = oFile.Read(rgb, sizeof(unsigned char), size * 3);
	if(bread != size * 3)
	{
		free(rgb);
		return NULL;
	}
	for(int i = 0; i < size * 3; i += 3)
	{
		temp = rgb[i];
		rgb[i] = rgb[i + 2];
		rgb[i + 2] = temp;
	}

	texFormat = GL_RGB;
	return rgb;
}

unsigned char *getGrey(COSFile &oFile, int size,GLenum &texFormat)
{
	unsigned char *grey;
	int bread;

	grey = (unsigned char *) malloc(size);
	if(grey == NULL)
		return NULL;

	bread = oFile.Read(grey, sizeof(unsigned char), size);
	if(bread != size)
	{
		free(grey);
		return NULL;
	}

	texFormat = GL_ALPHA;
	return grey;
}

int	CTextureManager::LoadTGA ( trTextureInfo *pTexInfo, bool bSkin, unsigned char *pData )
{
	unsigned char type[4];
	unsigned char info[7];
	unsigned char *imageData = NULL;
	int w, h;
	int bits, size;

	COSFile	&oFile = pTexInfo->file;
		
	if(!oFile.Open("rb"))
		return -1;

	oFile.Read(&type, sizeof(char),3);

	oFile.Seek(eFileStart,12);

	oFile.Read(&info, sizeof(char),6);
	if(type[1] != 0 || (type[2] != 2 && type[2] != 3))
		return -1;

	w = info[0] + info[1] * 256;
	h = info[2] + info[3] * 256;

	bits = info[4];
	size = w * h;

	GLenum texFormat;

	if(!checkSize(w) || !checkSize(h))
		return -1;

	  switch(bits)
	  {
		case 32:
		  imageData = getRGBA(oFile,size,texFormat);
		  break;

		case 24:
		  imageData = getRGB(oFile,size,texFormat);
		  break;

		case 8:
		  imageData = getGrey(oFile,size,texFormat);
			break;

		default:
			 return -1;
	  }

	if(imageData == NULL)
		return -1;

	oFile.Close();

	GLuint iglID = 0;

	if (!pData)
	{
		// get us a new Texture ID
		glGenTextures(1,&iglID);

		glBindTexture(GL_TEXTURE_2D, iglID);
		glPixelStorei(GL_UNPACK_ALIGNMENT, 1);

		int	iEdgeType;

		if (bSkin)
			iEdgeType = GL_CLAMP;
		else
			iEdgeType = GL_REPEAT;

		glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_WRAP_S, iEdgeType);
		glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_WRAP_T, iEdgeType);
		glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_MAG_FILTER, GL_LINEAR_MIPMAP_LINEAR);
		glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_MIN_FILTER, GL_LINEAR_MIPMAP_LINEAR);
		glTexEnvf(GL_TEXTURE_ENV, GL_TEXTURE_ENV_MODE, GL_MODULATE);
		glHint(GL_PERSPECTIVE_CORRECTION_HINT, GL_NICEST);


		glTexImage2D(GL_TEXTURE_2D, 0, texFormat, w, h, 0, texFormat, GL_UNSIGNED_BYTE, imageData);
		gluBuild2DMipmaps(GL_TEXTURE_2D, texFormat, w, h, texFormat, GL_UNSIGNED_BYTE, imageData);
	}
	else
		memcpy(pData,imageData,(bits/8)*w*h);
	
	pTexInfo->alpha = ((texFormat == GL_RGBA) || (texFormat == GL_ALPHA));
	pTexInfo->x = w;
	pTexInfo->y = h;

	pTexInfo->size = w*h*(bits/8);

	free(imageData);
	return iglID;
}

int	CTextureManager::LoadPNG ( trTextureInfo *pTexInfo, bool bSkin, unsigned char *pData )
{
	printf("png support is not enabled, build with libIL");
	return  -1;//ilutGLLoadImage((ILstring)pTexInfo->file.GetOSName());
}
