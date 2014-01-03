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
// clouds.cpp

#include "utils.h"
#include "openGL.h"
#include "clouds.h"
#include "textureManager.h"
#include "OGFTimer.h"
#include "3dView.h"

std::string defaultCloud = "data/sky/cloud/default";

CCloudRenderer::CCloudRenderer()
{
	cloudList.clear();
	cloudGLList = -1;
	cloudInfo.density = 0;
}

CCloudRenderer::~CCloudRenderer()
{
	DisplayListManager::instance().releaseList(cloudGLList);
}

void CCloudRenderer::Init ( trCloudInfo &info )
{ 
	cloudInfo = info;
	cloudList.clear();
	cloudTextures.clear();

	if (cloudInfo.textureList.size() == 0 )
		cloudInfo.textureList.push_back(defaultCloud);

	//load up the cloud texture ids, and the map for the clouds
	tvStringList::iterator itr = cloudInfo.textureList.begin();
	while (itr != cloudInfo.textureList.end())
	{
		int id = CTextureManager::instance().GetID((itr++)->c_str());
		cloudTextures.push_back(id);
		tvCloudList	list;
		cloudList[id] = list;
	}

	if (cloudInfo.density <= 0)
		return;

	cloudSpeed = get_mag(cloudInfo.vector);
	set_mag(cloudInfo.vector,1.0f);

	cloudHeightVariance = (cloudInfo.maxZ-cloudInfo.minZ)*0.5f;
	cloudHeight = cloudInfo.minZ + cloudHeightVariance;

	cloudSizeVariance = (cloudInfo.maxSize-cloudInfo.minSize)*0.5f;
	cloudSize = cloudInfo.minSize + cloudSizeVariance;

	trCloudStruct	rCloud;

	for (int i = 0; i < cloudInfo.density;i++)
	{
		rCloud.id = i;
		SpawnCloud(rCloud,true);
		cloudList[rCloud.texture].push_back(rCloud);
	}

	cloudGLList = DisplayListManager::instance().newList(this);
	// run some iterations on the things so they get broken up

	int initalIterations = 1000;

	for ( int i = 0; i < initalIterations; i++)
	{
		tvCloudTextureMap::iterator textureItr = cloudList.begin();

		while (textureItr != cloudList.end())
		{
			if ( textureItr->second.size())
			{
				tvCloudList::iterator itr = textureItr->second.begin();
				while (itr != textureItr->second.end())
					MoveCloud(*itr++,5.0f);
			}
			textureItr++;
		}
	}
}

void CCloudRenderer::build ( int list )
{
	float size = 1.0f;

	glBegin(GL_QUADS);
		glNormal3f(0,0,-1);
		glTexCoord2f(0,0);
		glVertex3f(-size,-size,0);

		glTexCoord2f(0,1);
		glVertex3f(-size,size,0);

		glTexCoord2f(1,1);
		glVertex3f(size,size,0);

		glTexCoord2f(1,0);
		glVertex3f(size,-size,0);
	glEnd();
}

void CCloudRenderer::Update ( void )
{
	if (cloudInfo.density <= 0)
		return;

	float time = (float)CLGETime::instance().GetFrameTime();
	tvCloudTextureMap::iterator textureItr = cloudList.begin();

	while (textureItr != cloudList.end())
	{
		if ( textureItr->second.size())
		{
			tvCloudList::iterator itr = textureItr->second.begin();
			while (itr != textureItr->second.end())
				MoveCloud(*itr++,time);
		}
		textureItr++;
	}
}

void CCloudRenderer::Draw ( void )
{
	if (cloudInfo.density <= 0)
		return;

	CViewFrustum &frustum = C3dView::instance().GetFrustum();

	glDepthMask(0);
	tvCloudTextureMap::iterator	textureItr = cloudList.begin();
	while (textureItr != cloudList.end())
	{
		if ( textureItr->second.size())
		{
			tvCloudList::iterator itr = textureItr->second.begin();
			CTextureManager::instance().Bind(textureItr->first);
			while (itr != textureItr->second.end())
			{
				if (frustum.SphereIn(itr->pos.x,itr->pos.y,itr->pos.z,itr->size))
					DrawCloud(*itr);
				itr++;
			}
		}
		textureItr++;
	}
	glDepthMask(1);
	glColor4f(1,1,1,1);
}

void CCloudRenderer::SpawnCloud ( trCloudStruct	&rCloud, bool bAnywhere )
{
	// get us a random texture
	rCloud.texture = cloudTextures[rand()%cloudTextures.size()];

	// get us a random angle
	float	fAngle = ParamRand(false) * 360.0f;

	// make that angle a vector
	trVertex3D	rPosVec;
	rPosVec.x = sin(fAngle * rad_con);
	rPosVec.y = cos(fAngle * rad_con);
	rPosVec.z = 0;
	set_mag(rPosVec,1.0f);

	// if we are anywhere, pick a distance
	// else allways put us at the edge.
	float		fPosRad = 0;
	if (bAnywhere)
		fPosRad = ParamRand(true) * cloudInfo.spreadSize*2.0f;	
	else
		fPosRad = cloudInfo.spreadSize*2.0f;
	
	// now make all that data into a cloud
	rCloud.pos = rPosVec * fPosRad;
	rCloud.pos.z = cloudHeight + ParamRand(true) * cloudHeightVariance;

	rCloud.vec = cloudInfo.vector * (cloudSpeed + ParamRand(true) * cloudInfo.speedVariance);

	rCloud.rot = ParamRand(false) * 360.0f;

	rCloud.size = cloudSize + ParamRand(true) * cloudSizeVariance;
}

void CCloudRenderer::DrawCloud ( trCloudStruct	&rCloud )
{
	float	fMag = rCloud.pos.x * rCloud.pos.x + rCloud.pos.y * rCloud.pos.y;

	float fAlpha = 1.0f;

	float	m_fCloudRadSquared = cloudInfo.spreadSize*cloudInfo.spreadSize;

	if ( fMag > m_fCloudRadSquared )
	{
		if (fMag > (m_fCloudRadSquared*2))
			fAlpha = 0;
		else
		{
			fMag -= m_fCloudRadSquared;

			if (fMag > 0.0001f)
				fAlpha = 1.0f - (fMag/m_fCloudRadSquared);
		}
	}

	cloudInfo.color.glColor(fAlpha);

	glPushMatrix();
	glTranslatef(rCloud.pos.x,rCloud.pos.y,rCloud.pos.z);
	glRotatef(rCloud.rot,0,0,1);

	if (cloudGLList != -1)
	{
		glScalef(rCloud.size,rCloud.size,1);
		DisplayListManager::instance().callList(cloudGLList);
	}
	else
	{
		glBegin(GL_QUADS);
			glNormal3f(0,0,-1);
			glTexCoord2f(0,0);
			glVertex3f(-rCloud.size,-rCloud.size,0);

			glTexCoord2f(0,1);
			glVertex3f(-rCloud.size,rCloud.size,0);

			glTexCoord2f(1,1);
			glVertex3f(rCloud.size,rCloud.size,0);

			glTexCoord2f(1,0);
			glVertex3f(rCloud.size,-rCloud.size,0);

		glEnd();
	}
	glPopMatrix();
}


void CCloudRenderer::MoveCloud ( trCloudStruct	&rCloud, float time )
{
	rCloud.pos += rCloud.vec * time;

	float	fMag = sqrt(rCloud.pos.x * rCloud.pos.x + rCloud.pos.y * rCloud.pos.y);

	if (fMag > (cloudInfo.spreadSize*2) )
		SpawnCloud(rCloud,false);
}
