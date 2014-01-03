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
// objects.cpp : Defines the entry point for the application.
//

#include "objects.h"

#include "OGF.h"

// stuff the objects need
#include "skybox.h"

// global factorys
InstanceFactory instanceFactory;
GroupFactory groupFactory;
EnvironmentFactory environmentFactory;
SkyboxFactory skyboxFactory;
GroundFactory groundFactory;
BoxFactory boxFactory;

//--------------------- group---------------------------------------------

class GroupObject : public MapBaseObject
{
public:
	GroupObject(){factory = NULL;world = NULL;}

	virtual void load ( trObjectParserBlock &parserBlock );
};

void GroupObject::load ( trObjectParserBlock &parserBlock )
{
	if (!world)
		return;

	tvStringPairList::iterator itr = parserBlock.elements.begin();

	while (  itr != parserBlock.elements.end() )
		basicElement( *itr++ );

	world->storeGroupBlock(name,parserBlock);
}

MapBaseObject* GroupFactory::newObject( void )
{
	return new GroupObject;
} 

//--------------------- instance---------------------------------------------

class InstanceObject : public MapBaseObject
{
public:
	InstanceObject(){factory = NULL;world = NULL;}

	virtual void load ( trObjectParserBlock &parserBlock );
};

void InstanceObject::load ( trObjectParserBlock &parserBlock )
{
	if (!world)
		return;

	// scan elements
	tvStringPairList::iterator itr = parserBlock.elements.begin();

	std::string		group;
	while (  itr != parserBlock.elements.end() )
	{
		if ( !basicElement( *itr ) )
		{
			std::string tag = string_util::toupper(itr->first);

			if ( tag == "GROUP" )
				group = itr->second;
		}
		itr++;
	}

	// get the group and add it's blocks and sub blocks in as well
	// we will be the parent tho so it will pick up OUR transforms
	trObjectParserBlock *groupBlock = world->retreveGroupBlock(group);
	if (!groupBlock)
		return;

	std::vector<trObjectParserBlock>::iterator subitr = groupBlock->subBlocks.begin();

	while ( subitr != groupBlock->subBlocks.end() )
	{
		world->AddObject(subitr->name,*subitr,this);
		subitr++;
	}

	// last but not least, we call the default to see if we have any blocks on us
	MapBaseObject::load(parserBlock);
}

MapBaseObject* InstanceFactory::newObject( void )
{
	return new InstanceObject;
}
//--------------------- environment---------------------------------------------
class EnvironmentObject : public MapBaseObject
{
public:
	EnvironmentObject(){factory = NULL;world = NULL;}

	virtual void load ( trObjectParserBlock &parserBlock );
};

void EnvironmentObject::load ( trObjectParserBlock &parserBlock )
{
// scan elements
	tvStringPairList::iterator itr = parserBlock.elements.begin();

	float gravity = -5;
	float ground = 0;
	float terminalV = -500;

	while (  itr != parserBlock.elements.end() )
	{
		std::string tag = string_util::toupper(itr->first);

		if ( tag == "GRAVITY" )
			gravity =(float)atof(itr->second.c_str());
		else if ( tag == "GROUND" )
			ground = (float)atof(itr->second.c_str());
		else if ( tag == "TERMINALV" )
			terminalV = (float)atof(itr->second.c_str());
		itr++;
	}

	// set the worlds data
	world->getPhisicals().gravity = gravity;
	world->getPhisicals().groundLimit = ground;
	world->getPhisicals().maxFallVel = terminalV;

	MapBaseObject::load(parserBlock);
}

MapBaseObject* EnvironmentFactory::newObject( void )
{
	return new EnvironmentObject;
}

//--------------------- skybox---------------------------------------------

class SkyboxObject: public MapBaseObject, CBaseDrawItem 
{
public:
	SkyboxObject(){factory = NULL;world = NULL;}
	virtual ~SkyboxObject(){return;}

	virtual void load ( trObjectParserBlock &parserBlock );
	virtual void draw ( void );

	virtual void DrawItem ( int item, int param = 0 );

protected:
	CSkybox		skybox;
};

void SkyboxObject::load ( trObjectParserBlock &parserBlock )
{

	float skyboxSize = 500;
	skybox.SetTexturePath("data/skyboxes");

	// scan elements
	tvStringPairList::iterator itr = parserBlock.elements.begin();

	while (  itr != parserBlock.elements.end() )
	{
		if ( !basicElement( *itr ) )
		{
			std::string tag = string_util::toupper(itr->first);

			if ( tag == "BOUNDS" )
				skyboxSize = (float)atof(itr->second.c_str());
			else if ( tag == "ZSHIFT" )
				skybox.SetZShift((float)atof(itr->second.c_str()));
		}
		itr++;
	}

	skybox.Create(skyboxSize,name.c_str());
	MapBaseObject::load(parserBlock);
}

void  SkyboxObject::DrawItem ( int item, int param )
{
	glDisable(GL_LIGHTING);
	skybox.Draw();
	glEnable(GL_LIGHTING);
}

void SkyboxObject::draw ( void )
{
	C3dView::instance().GetDrawList().AddDrawItem(this,-1,eFirst);
}

MapBaseObject* SkyboxFactory::newObject( void )
{
	return new SkyboxObject;
}


//--------------------- ground---------------------------------------------

class GroundObject: public MapBaseObject, CBaseDrawItem , DisplayListBuilder
{
public:
	GroundObject(){factory = NULL;world = NULL;glList = -1;}
	virtual ~GroundObject();

	virtual void load ( trObjectParserBlock &parserBlock );
	virtual void draw ( void );

	virtual void DrawItem ( int item, int param = 0 );
	virtual void build ( int list );

protected:
	int									glList;
	float								bounds;
};

GroundObject::~GroundObject()
{
	DisplayListManager::instance().releaseList(glList);
}

void GroundObject::load ( trObjectParserBlock &parserBlock )
{
	textures.clear();
	uvScales.clear();
	bounds = 500;

	// scan elements
	tvStringPairList::iterator itr = parserBlock.elements.begin();

	while (  itr != parserBlock.elements.end() )
	{
		if ( !basicElement(*itr) )
		{
			std::string tag = string_util::toupper(itr->first);
			if ( tag == "BOUNDS" )
				bounds = (float)atof(itr->second.c_str());
		}
		itr++;
	}

	world->getPhisicals().size = bounds;
	glList = DisplayListManager::instance().newList(this);

	MapBaseObject::load(parserBlock);
}

void  GroundObject::build ( int list )
{
	glBegin(GL_QUADS);

		glNormal3f(0,0,1);
		float zDepth = 0;
		float uvScale = 1.0f;
		glTexCoord2f(0,uvScale*bounds);
		glVertex3f(-bounds,bounds,zDepth);

		// lower left corner
		glTexCoord2f(0,0);
		glVertex3f(-bounds,-bounds,zDepth);

		// lower right
		glTexCoord2f(uvScale*bounds,0);
		glVertex3f(bounds,-bounds,zDepth);

		// upper right
		glTexCoord2f(uvScale*bounds,uvScale*bounds);
		glVertex3f(bounds,bounds,zDepth);

	glEnd();
}

void  GroundObject::DrawItem ( int item, int param )
{
	C3DVertex uvScale(1.0f,1.0f,1.0f);
	int pass = param;
	int texture = item;

	if ( texture != -1)
	{
		if (pass < (int)uvScales.size())
			uvScale = uvScales[pass];
		else if ( uvScales.size() )
			uvScale = uvScales[uvScales.size()-1];

		glMatrixMode(GL_TEXTURE);
		glLoadIdentity();
		glScalef(uvScale.X(),uvScale.Y(),uvScale.Z());
		glMatrixMode(GL_MODELVIEW); 

		glColor4f(1,1,1,1);
		if ( pass > 1 )
			glDepthMask(0);

		DisplayListManager::instance().callList(glList);
	}

	glMatrixMode(GL_TEXTURE);
	glLoadIdentity();
	glMatrixMode(GL_MODELVIEW);
	glDepthMask(1);
}

void GroundObject::draw ( void )
{
	int pass = 0;

	std::vector<int>::iterator itr = textures.begin();
	while ( itr != textures.end())
	{
		teRenderPriority  priority = ePass1;
		if ( pass > 1 )
			priority = eTrans;

		C3dView::instance().GetDrawList().AddDrawItem(this,*itr,priority,pass);
		pass++;
		itr++;
	}
}

MapBaseObject* GroundFactory::newObject( void )
{
	return new GroundObject;
}

//--------------------- box---------------------------------------------

class BoxObject: public MapBaseObject, CBaseDrawItem , DisplayListBuilder
{
public:
	BoxObject(){factory = NULL;world = NULL;depthWrites = true;glList=-1;noTop = false;glList = -1;}
	virtual ~BoxObject();

	virtual void load ( trObjectParserBlock &parserBlock );
	virtual void draw ( void );

	virtual void DrawItem ( int item, int param = 0 );
	virtual void build ( int list );

protected:
	int				glList;
	CColor			color;
	bool			depthWrites;
	bool			noTop;
};

BoxObject::~BoxObject()
{
	DisplayListManager::instance().releaseList(glList);
}

void BoxObject::load ( trObjectParserBlock &parserBlock )
{
	depthWrites = true;
	// scan elements
	tvStringPairList::iterator itr = parserBlock.elements.begin();

	std::string grimeTexture;
	float				grimeZ = 1;
	while (  itr != parserBlock.elements.end() )
	{
		if ( !basicElement(*itr) )
		{
			std::string tag = string_util::toupper(itr->first);

			if ( tag == "COLOR" )
				color.parseColor(itr->second);
			if ( tag == "DEPTHWRITE" )
				depthWrites = atoi(itr->second.c_str()) != 0;
			if ( tag == "NOTOP" )
				noTop = atoi(itr->second.c_str()) != 0;
			if ( tag == "GRIMETEXTURE" )
				grimeTexture = itr->second.c_str();
			if ( tag == "GRIMEZ" )
				grimeZ = (float)atof(itr->second.c_str());
		}
		itr++;
	}

	glList = DisplayListManager::instance().newList(this);

	if ( grimeTexture.size() )
	{
		// we need to make a child grim object
		// we copy the elements so it gets all the same stuff
		// but we DON'T do the sub blocks
		trObjectParserBlock	miniBlock;
		miniBlock.name = parserBlock.name;
		miniBlock.elements.push_back(tpStringPair(std::string("SIZE"),string_util::format("%f,%f,%f",size.X(),size.Y(),grimeZ*0.5f)));

		float localCenter = -(size.Z()-grimeZ/2.0f);
		miniBlock.elements.push_back(tpStringPair(std::string("POSITION"),string_util::format("%f,%f,%f",0.0f,0.0f,localCenter)));
		miniBlock.elements.push_back(tpStringPair(std::string("DEPTHWRITE"),std::string("0")));
		miniBlock.elements.push_back(tpStringPair(std::string("NOTOP"),std::string("1")));
		miniBlock.elements.push_back(tpStringPair(std::string("TEXTURE"),grimeTexture));
	
		C3DVertex uvScale(1,1,1);
		if ((int)uvScales.size())
			uvScale = uvScales[(int)uvScales.size()-1];
		miniBlock.elements.push_back(tpStringPair(std::string("UVSCALE"),string_util::format("%f,%f,%f",uvScale.X(),uvScale.Y(),uvScale.Z()*(grimeZ/size.Z()))));

		world->AddObject("BOX",miniBlock,this);
	}
	MapBaseObject::load(parserBlock);
}

void BoxObject::build ( int list )
{
	float uvScale = 1.0f;

	glPushMatrix();
	glTransform();

	glBegin(GL_QUADS);

		if (!noTop)
		{
			// top
			glNormal3f(0,0,1);
			glTexCoord2f(0,uvScale*size.Y());
			glVertex3f(-size.X(),size.Y(),size.Z());

			glTexCoord2f(0,0);
			glVertex3f(-size.X(),-size.Y(),size.Z());

			glTexCoord2f(uvScale*size.X(),0);
			glVertex3f(size.X(),-size.Y(),size.Z());

			glTexCoord2f(uvScale*size.X(),uvScale*size.Y());
			glVertex3f(size.X(),size.Y(),size.Z());

			// bottom
			glNormal3f(0,0,-1);

			glTexCoord2f(0,uvScale*size.Y());
			glVertex3f(size.X(),size.Y(),-size.Z());

			glTexCoord2f(0,0);
			glVertex3f(size.X(),-size.Y(),-size.Z());

			glTexCoord2f(uvScale*size.X(),0);
			glVertex3f(-size.X(),-size.Y(),-size.Z());

			glTexCoord2f(uvScale*size.X(),uvScale*size.Y());
			glVertex3f(-size.X(),size.Y(),-size.Z());
		}
		// X+ side
		glNormal3f(1,0,0);
		glTexCoord2f(uvScale*size.Y(),uvScale*size.Z());
		glVertex3f(size.X(),size.Y(),size.Z());

		glTexCoord2f(0,uvScale*size.Z());
		glVertex3f(size.X(),-size.Y(),size.Z());

		glTexCoord2f(0,0);
		glVertex3f(size.X(),-size.Y(),-size.Z());

		glTexCoord2f(uvScale*size.Y(),0);
		glVertex3f(size.X(),size.Y(),-size.Z());

		// X- side
		glNormal3f(-1,0,0);
		glTexCoord2f(0,0);
		glVertex3f(-size.X(),size.Y(),-size.Z());

		glTexCoord2f(uvScale*size.Y(),0);
		glVertex3f(-size.X(),-size.Y(),-size.Z());

		glTexCoord2f(uvScale*size.Y(),uvScale*size.Z());
		glVertex3f(-size.X(),-size.Y(),size.Z());

		glTexCoord2f(0,uvScale*size.Z());
		glVertex3f(-size.X(),size.Y(),size.Z());

		// y+ side
		glNormal3f(0,1,0);
		glTexCoord2f(0,0);
		glVertex3f(size.X(),size.Y(),-size.Z());

		glTexCoord2f(uvScale*size.X(),0);
		glVertex3f(-size.X(),size.Y(),-size.Z());

		glTexCoord2f(uvScale*size.X(),uvScale*size.Z());
		glVertex3f(-size.X(),size.Y(),size.Z());

		glTexCoord2f(0,uvScale*size.Z());
		glVertex3f(size.X(),size.Y(),size.Z());

		// y- side
		glNormal3f(0,-1,0);
		glTexCoord2f(uvScale*size.X(),uvScale*size.Z());	
		glVertex3f(size.X(),-size.Y(),size.Z());

		glTexCoord2f(0,uvScale*size.Z());
		glVertex3f(-size.X(),-size.Y(),size.Z());

		glTexCoord2f(0,0);
		glVertex3f(-size.X(),-size.Y(),-size.Z());

		glTexCoord2f(uvScale*size.X(),0);
		glVertex3f(size.X(),-size.Y(),-size.Z());

	glEnd();
	glPopMatrix();
}

void BoxObject::DrawItem ( int item, int param )
{
	C3DVertex uvScale(1,1,1);
	int pass = param;
	int texture = item;

	color.glColor();
	if (!depthWrites)
		glDepthMask(0);

	if ( texture != -1)
	{
		if (pass < (int)uvScales.size())
			uvScale = uvScales[pass];
		else if ( uvScales.size() )
			uvScale = uvScales[uvScales.size()-1];

		glMatrixMode(GL_TEXTURE);
		glLoadIdentity();
		glScalef(uvScale.X(),uvScale.Y(),uvScale.Z());
		glMatrixMode(GL_MODELVIEW);

		if ( pass > 1 )
			glDepthMask(0);

		glPushMatrix();
		DisplayListManager::instance().callList(glList);
		glPopMatrix();
	}
	else
	{
		glPushMatrix();
		glDisable(GL_TEXTURE_2D);
		DisplayListManager::instance().callList(glList);
		glEnable(GL_TEXTURE_2D);
		glPopMatrix();
	}

	glColor4f(1,1,1,1);
	glEnable(GL_TEXTURE_2D);
	glMatrixMode(GL_TEXTURE);
	glLoadIdentity();
	glMatrixMode(GL_MODELVIEW);
	glDepthMask(1); 
}

void BoxObject::draw ( void )
{
	int pass = 0;

	std::vector<int>::iterator itr = textures.begin();
	while ( itr != textures.end())
	{
		teRenderPriority  priority = ePass1;
		if ( pass > 1 && !depthWrites )
			priority = eTrans;

		C3dView::instance().GetDrawList().AddDrawItem(this,*itr,priority,pass);
		
		pass++;
		itr++;
	}
}

MapBaseObject* BoxFactory::newObject( void )
{
	return new BoxObject;
} 


