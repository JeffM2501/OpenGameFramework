/* 
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
// Clouds.h

#ifndef _CLOUDS_H_
#define _CLOUDS_H_

#include "mathUtils.h"
#include "3dTypes.h"
#include "displayListManager.h"

#include <vector>
#include <map>

typedef std::vector<std::string>	tvStringList;

extern std::string defaultCloud;

typedef struct
{
	tvStringList	textureList;
	CColor			color;
	float			density;
	trVertex3D		vector;
	float			maxZ;
	float			minZ;
	float			maxSize;
	float			minSize;
	float			speedVariance;
	float			spreadSize;
}trCloudInfo;

typedef struct
{
	trVertex3D	pos;
	trVertex3D	vec;
	int			id;
	float		rot;
	float		size;
	int			texture;
}trCloudStruct;

typedef std::vector<trCloudStruct>	tvCloudList;
typedef std::map<int,tvCloudList>	tvCloudTextureMap;

class CCloudRenderer : public DisplayListBuilder
{
public:
	CCloudRenderer();
	~CCloudRenderer();

	void Init ( trCloudInfo &info);
	void Update ( void );
	void Draw ( void );
	
	bool	useLists;

	virtual void build ( int list );

protected:
	void SpawnCloud ( trCloudStruct	&rCloud, bool bAnywhere );
	void DrawCloud ( trCloudStruct	&rCloud );
	void MoveCloud ( trCloudStruct	&rCloud, float time );

	tvCloudTextureMap		cloudList;
	trCloudInfo				cloudInfo;

	float					cloudSpeed;
	float					cloudHeight;
	float					cloudHeightVariance;
	float					cloudSize;
	float					cloudSizeVariance;
	float					cloudTexPos[2];
	std::vector<int>		cloudTextures;

	int						cloudGLList;
};

#endif//_CLOUDS_H_
