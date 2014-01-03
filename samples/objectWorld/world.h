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
// World.h

#ifndef _WORLD_H_
#define  _WORLD_H_

#include <string>
#include <map>
#include <vector>
#include "OGF.h"

typedef std::pair<std::string,std::string> tpStringPair;
typedef std::vector<tpStringPair> tvStringPairList;

typedef struct trObjectParserBlock
{
	std::string name;
	tvStringPairList	elements;
	std::vector<trObjectParserBlock>	subBlocks;
}trObjectParserBlock;

class BaseOjbectFactory;
class SimpleGameWorld;

class MapBaseObject
{
public:
	MapBaseObject(){factory = NULL;world = NULL;parent = NULL;}
	MapBaseObject(BaseOjbectFactory* theFactory, SimpleGameWorld* theWorld){setFactory(theFactory);setWorld(theWorld);parent = NULL;}

	virtual ~MapBaseObject(){return;}

	virtual void init ( void ){return;}
	virtual void load ( trObjectParserBlock &parserBlock );

	virtual void think ( float frameTime, float curTime ){return;}
	virtual void draw ( void ){return;}

	virtual bool visible ( CViewFrustum& frustum ){return true;}

	virtual bool glTransform ( void );
	virtual bool getTransformMatrix ( CMatrix &matrix );

	virtual bool inSphere ( C3DVertex &center, float radius ){return true;}
	virtual bool inCylinder( C3DVertex &center, float radius, float height ){return true;}

	// non overide utils for standard behavor

	// returns true if this is one of the standard elements
	bool basicElement ( tpStringPair &element );

	// get/set methods
	BaseOjbectFactory* getFactory ( void ) { return factory;}
	void setFactory ( BaseOjbectFactory* theFactory ) {factory = theFactory;}

	void setWorld ( SimpleGameWorld* theWorld ) {world = theWorld;}
	void setParent ( MapBaseObject* theParent ) {parent = theParent;}

protected:
	BaseOjbectFactory	*factory;
	SimpleGameWorld		*world;
	MapBaseObject			*parent;

	// common data
	std::string					name;
	C3DVertex						position;
	C3DVertex						rotation;
	C3DVertex						size;
	std::vector<int>		textures;
	std::vector<C3DVertex>	uvScales;
};

class BaseOjbectFactory
{
public:
	virtual ~BaseOjbectFactory(){return;}
	virtual MapBaseObject *newObject( void ) = 0;
	virtual void deleteObject (MapBaseObject *object ) {::delete(object);}
};

class SimpleGameWorld
{
public:
	SimpleGameWorld();
	~SimpleGameWorld();

	bool init ( const char* mapFile );
	bool load ( const char* mapFile );

	bool think ( float frameTime, float curTime );
	void vis ( CViewFrustum &frustum );
	void draw ( void );

	void registerObjectFactory ( BaseOjbectFactory *factory, const char* name );
	void removeObjectFactory ( const char* name );

	typedef std::vector<MapBaseObject*>	tvObjectList;

	int getNear ( tvObjectList &list, C3DVertex &center, float radius, bool cylinder, float height );

	MapBaseObject* AddObject ( std::string name, trObjectParserBlock &parserBlock, MapBaseObject* parent = NULL );

	// phystics stuff
	typedef struct 
	{
		float gravity;
		float groundLimit;
		float maxFallVel;
		float size;
	}trWorldPhisicalInfo;

	trWorldPhisicalInfo& getPhisicals ( void ) { return phisicals;}

	void storeGroupBlock ( std::string &name, trObjectParserBlock& block );
	trObjectParserBlock* retreveGroupBlock (std::string &name ); 
protected:
	bool parseBlock ( trObjectParserBlock &block, std::vector<std::string>::iterator &itr, std::vector<std::string> &lines );
	trObjectParserBlock* findBlock ( std::string name, trObjectParserBlock& rootBlock );

	typedef struct 
	{
		std::string name;
		std::string value;
		bool		blockStart;
		bool		blockEnd;
	}trBlockElement;
	
	bool parseLine ( std::string &line, trBlockElement &element );

	typedef std::map<std::string,BaseOjbectFactory*>	tmFactoryList;
	tmFactoryList	factories;
	tvObjectList	objects;			// list of objects in world
	tvObjectList	visibleList;		// list of objects visible here

	trWorldPhisicalInfo	phisicals;

	std::map<std::string,trObjectParserBlock>	groups;
};

void registerDefaultFactories (SimpleGameWorld &world );
#endif //_WORLD_H_
