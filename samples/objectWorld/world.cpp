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
// world.cpp : Defines the entry point for the application.
//

#include "world.h"
#include "OGF.h"

// some standard objects
#include "objects.h"


#define _NAME			"NAME"
#define _POSITION	"POSITION"
#define _ROTATION	"ROTATION"
#define _COLOR		"COLOR"
#define _UVSCALE	"UVSCALE"
#define _SIZE			"SIZE"
#define _TEXTURE	"TEXTURE"


// load function for the base object class
// all it realy does is check for sub blocks and calls add object on them
// is such common code that it should go here
// the important thing is, that it passes THIS as the parent, this makes nesting work
void MapBaseObject::load ( trObjectParserBlock &parserBlock )
{
	if (!world)
		return;

	std::vector<trObjectParserBlock>::iterator itr = parserBlock.subBlocks.begin();

	while ( itr != parserBlock.subBlocks.end() )
	{
		world->AddObject(itr->name,*itr,this);
		itr++;
	}
}

bool MapBaseObject::glTransform ( void )
{
	if (parent)
		parent->glTransform();

	glTranslatef(position.X(),position.Y(),position.Z());
	glRotatef(rotation.Z(),0,0,1);
	glRotatef(rotation.Y(),0,1,0);
	glRotatef(rotation.X(),1,0,0);
	return true;
}

bool MapBaseObject::getTransformMatrix ( CMatrix &matrix )
{
	CMatrix parentMatrix;

	parentMatrix.GetIdentity();

	if (parent)
		parent->getTransformMatrix(parentMatrix);

	matrix.GetIdentity();
	matrix.Concat(parentMatrix);

	matrix.Translate(position.X(),position.Y(),position.Z());

	matrix.Rotate2d(rotation.Z(),0,0,1);
	matrix.Rotate2d(rotation.Y(),0,1,0);
	matrix.Rotate2d(rotation.X(),1,0,1);
	return true;
}

bool MapBaseObject::basicElement ( tpStringPair &element )
{
	std::string tag = string_util::toupper(element.first);

	if ( tag == _NAME )
		name = element.second;
	else if ( tag == _SIZE )
		size.parseTriplet(element.second);
	else if ( tag == _POSITION )
		position.parseTriplet(element.second);
	else if ( tag == _ROTATION )
		rotation.parseTriplet(element.second);
	else if ( tag == _UVSCALE )
	{
		C3DVertex	temp;
		temp.parseTriplet(element.second);
		temp.inverse();
		uvScales.push_back(temp);
	}
	else if ( tag == _TEXTURE )
		textures.push_back(CTextureManager::instance().GetID(element.second.c_str()));
	else
		return false;
	return true;
}


//-----------------game world---------------------------------

SimpleGameWorld::SimpleGameWorld()
{

}

SimpleGameWorld::~SimpleGameWorld()
{

}

bool SimpleGameWorld::init ( const char* mapFile )
{
	// maybe precompute some stuff here on launch?
	// maybe load some object plugins?
	// dono something?
	// curl for resources?

	// for now just load the sucker
	return load(mapFile);
}

bool SimpleGameWorld::parseLine ( std::string &line, trBlockElement &element )
{
	if (!line.size())
		return false;

	std::string::iterator itr = line.begin();
	
	element.blockEnd = false;
	element.blockStart = false;
	element.name = "";
	element.value = "";

	bool gotTagStart = false;
	bool gotName = false;
	bool hasValue = false;
	bool inQuotes = false;
	bool hitEndSlash = false;
	bool endProper = false;

	while ( itr != line.end() )
	{
		if ( !gotTagStart )
		{
			if (*itr == '<')
				gotTagStart = true;
		}
		else
		{
			if (!gotName)
			{
				if ( *itr =='/')
					element.blockEnd = true;
				else if ( string_util::isWhitespace(*itr) || *itr == '>')
					gotName = true;
				else
					element.name += *itr;

				if (gotName && *itr == '>')
					element.blockStart = true;
			}
			else	// we have the tagname find a value
			{
				if (!hasValue)
				{
					if ( *itr == '=' )
						hasValue = true;
				}
				else
				{
					if (!string_util::isWhitespace(*itr) && !element.value.size())	// start of the thing
					{
						if (*itr == '\"')
							inQuotes = true;
						else
							element.value += *itr;
					}
					else if (element.value.size())
					{
						if (inQuotes)
						{
							if (*itr == '\"')
								inQuotes = false;
							else
								element.value += *itr;
						}
						else if (*itr == '/')
						{
							element.blockStart = false;
							element.blockEnd = false;
							hitEndSlash = true;
							endProper = true;
							itr != line.end();
						}
						else if (*itr == '>')
						{
							if (!hitEndSlash)
								element.blockStart = true;
						}
						else if (!string_util::isWhitespace(*itr))
							element.value += *itr;
					}
				}
			}
		}
		if ( itr != line.end() )
		{
			if (*itr == '>')	// end of the tag, don't care about the rest
			{
				endProper = true;
				itr = line.end();
			}
			else
				itr++;
		}
	}
	return endProper;
}

bool SimpleGameWorld::parseBlock ( trObjectParserBlock &block, std::vector<std::string>::iterator &itr, std::vector<std::string> &lines )
{
	trBlockElement	element;
	bool inBlock = false;

	// find the first block start, thats our block
	parseLine(*(itr++),element);
	while ( !element.blockStart && itr != lines.end() )
		parseLine(*(itr++),element);
	
	inBlock = true;

	block.name = element.name;

	while ( inBlock && itr != lines.end() )
	{
		bool err = !parseLine(*(itr),element);

		if (err)
			return false;

		if (element.blockEnd)
		{
			inBlock = false;
			itr++;
		}
		else
		{
			if (element.blockStart)// start of a new block, let the recursion begin
			{
			//	itr++;
				trObjectParserBlock	subBlock;
				err = !parseBlock(subBlock,itr,lines);
				if (err)
					return false;
				else
					block.subBlocks.push_back(subBlock);
			}
			else	// just a regular element
			{
				itr++;
				tpStringPair item;
				item.first = element.name;
				item.second = element.value;
				block.elements.push_back(item);
			}
		}
	}

	return true;
}

trObjectParserBlock* SimpleGameWorld::findBlock ( std::string name, trObjectParserBlock& rootBlock )
{
	if (string_util::compare_nocase(rootBlock.name,name) ==0)
		return &rootBlock;
	
	std::vector<trObjectParserBlock>::iterator itr = rootBlock.subBlocks.begin();

	while ( itr != rootBlock.subBlocks.end() )
	{
		trObjectParserBlock* ret = findBlock(name, *itr);
		if (ret)
			return ret;
		itr++;
	}
	return NULL;
}

bool SimpleGameWorld::load ( const char* mapFile )
{
	// out with the old
	visibleList.clear();
	objects.clear();

	// in with the new
	COSFile	file(mapFile,"rb");

	size_t fileSize = file.Size();

	void *tempData = malloc(fileSize+1);
	memset(tempData,0,fileSize);

	file.Read(tempData,(int)fileSize);
	file.Close();

	std::string mapData = (char*)tempData;
	free(tempData);

	// iterate the data and remove any windows linefeeds (ascii 10 )
	// this makes sure all files are in unix format for parsing
	std::string::iterator strItr = mapData.begin();
	while (strItr != mapData.end())
	{
		if (*strItr == 13)
			strItr = mapData.erase(strItr);
		else
			strItr ++;
	}

	// now break up the map file into a vector with each element being one line
	std::vector<std::string> mapLines = string_util::tokenize(mapData,std::string("\n"));

	// scan the lines and remove blanks or comments
	std::vector<std::string>::iterator lineItr = mapLines.begin();

	while ( lineItr != mapLines.end() )
	{
		std::string theLine = *lineItr;
		*lineItr = string_util::trim_whitespace(*lineItr);
		theLine = *lineItr;
		if ( !theLine.size() || theLine[0] != '<')	// all lines MUST have <> in them
			lineItr = mapLines.erase(lineItr);
		else
			lineItr++;
	}

	lineItr = mapLines.begin();

	// ok lets fill this sucker in

	trObjectParserBlock rootBlock;

	if (!parseBlock(rootBlock,lineItr,mapLines))
		return false;	// the map was not complete

	trObjectParserBlock	*blockPtr = findBlock(std::string("STML"),rootBlock);
	if (!blockPtr)	// the file is NOT a STML file
		return false;

	blockPtr = findBlock(std::string("world"),*blockPtr);
	if (!blockPtr)
		return false; // the file does NOT contain a world

	// iterate thru the objects in the map at this level
	// only pull in the top level items sub objects will be handled via their parent object
	std::vector<trObjectParserBlock>::iterator itr = blockPtr->subBlocks.begin();

	while ( itr != blockPtr->subBlocks.end() )
	{
		AddObject(itr->name,*itr);
		itr++;
	}
	return true;
}

MapBaseObject* SimpleGameWorld::AddObject ( std::string name, trObjectParserBlock &parserBlock, MapBaseObject* parent )
{
	tmFactoryList::iterator itr = factories.find(string_util::toupper(name));

	if (itr == factories.end())
		return NULL;

	MapBaseObject	*object = itr->second->newObject();
	if (!object)
		return NULL;

	objects.push_back(object);
	object->setFactory(itr->second);
	object->setWorld(this);
	object->setParent(parent);

	object->init();
	object->load(parserBlock);
	return object;
}

bool SimpleGameWorld::think ( float frameTime, float curTime )
{
	return false;
}

void SimpleGameWorld::vis ( CViewFrustum &frustum )
{
	visibleList.clear();

	tvObjectList::iterator itr = objects.begin();
	
	while ( itr != objects.end() )
	{
		if ( (*itr)->visible(frustum))
			visibleList.push_back(*itr);
		itr++;
	}
}

void SimpleGameWorld::draw ( void )
{
	tvObjectList::iterator itr = visibleList.begin();

	while ( itr != visibleList.end() )
		(*itr++)->draw();

#ifdef _DEBUG	// just put some markers so we know what way is what
	glPushMatrix();
	glDisable(GL_TEXTURE_2D);

	glColor4f(1,0,0,1);
	glTranslatef(20,0,0);
	gluSphere(gluNewQuadric(),2,16,16);
	glPopMatrix();

	glPushMatrix();
	glTranslatef(0,25,5);
	glColor4f(0,1,0,1);
	gluSphere(gluNewQuadric(),1,3,3);


	glEnable(GL_TEXTURE_2D);
	glPopMatrix();
#endif//_DEBUG
}

void SimpleGameWorld::registerObjectFactory ( BaseOjbectFactory *factory, const char* name )
{
	if (!name || !factory)
		return;

	factories[string_util::toupper(std::string(name))] = factory;
}

void SimpleGameWorld::removeObjectFactory ( const char* name )
{
	if (!name)
		return;

	tmFactoryList::iterator itr = factories.find(string_util::toupper(std::string(name)));

	if (itr != factories.end())
		factories.erase(itr);
}

int SimpleGameWorld::getNear ( tvObjectList &list, C3DVertex &center, float radius, bool cylinder, float height )
{
	return -1;
}

void SimpleGameWorld::storeGroupBlock ( std::string &name, trObjectParserBlock& block )
{
	groups[string_util::toupper(name)] = block;
}

trObjectParserBlock* SimpleGameWorld::retreveGroupBlock (std::string &name )
{
	std::map<std::string,trObjectParserBlock>::iterator itr = groups.find(string_util::toupper(name));
	if (itr == groups.end())
		return NULL;
	
	return &(itr->second);
}



void registerDefaultFactories (SimpleGameWorld &world )
{
	world.registerObjectFactory(&instanceFactory,"INSTANCE");
	world.registerObjectFactory(&groupFactory,"GROUP");
	world.registerObjectFactory(&environmentFactory,"ENVIRONMENT");
	world.registerObjectFactory(&skyboxFactory,"SKYBOX");
	world.registerObjectFactory(&groundFactory,"GROUND");
	world.registerObjectFactory(&boxFactory,"BOX");
}

