/* Open Game Framework
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

// this class is a singleton manager
// that takes care of the creation and removal of open GL display lists
// it abstracts the list ID away from GL
// and manages a set of callback classes that are used
// to rembuild the lists when ever they are needed.
// this needs to happen after a context is invalidated, after
// a deactivate/activate pair ( aka alt+tab )

#include "displayListManager.h"
#include "openGL.h"


template <>
DisplayListManager* Singleton<DisplayListManager>::_instance = (DisplayListManager*)0;

DisplayListManager::DisplayListManager()
{
	lastList = 0;
}

DisplayListManager::~DisplayListManager()
{
	releaseAll();
}

void DisplayListManager::init ( void )
{
	releaseAll();
	displayLists.clear();
	lastList = 0;
}

void DisplayListManager::releaseAll ( void )
{	
	tmDisplayListList::iterator itr = displayLists.begin();
	while ( itr != displayLists.end() )
	{
		glDeleteLists(itr->second.glList,1);
		itr->second.glList = GL_INVALID_ID;
		itr++;
	}
}

void DisplayListManager::rebuildAll ( void )
{
	tmDisplayListList::iterator itr = displayLists.begin();
	while ( itr != displayLists.end() )
	{
		rebuildList(itr->first);
		itr++;
	}
}

int DisplayListManager::newList ( DisplayListBuilder *builder, bool dontBuild )
{
	if (!builder)
		return -1;

	lastList++;

	trDisplayListInfo info;
	info.builder = builder;
	if (dontBuild)
		info.glList = GL_INVALID_ID;
	else
		info.glList = glGenLists(1);

	displayLists[lastList] = info;
	
	if (!dontBuild)
		buildGLList(lastList, info.glList,builder);
	return lastList;
}

void DisplayListManager::callList ( int displayList )
{
	tmDisplayListList::iterator itr = displayLists.find(displayList);
	if ( itr == displayLists.end() )
		return;

	if (itr->second.glList == GL_INVALID_ID)
	{
		itr->second.glList = glGenLists(1);
		buildGLList(displayList, itr->second.glList,itr->second.builder);
	}
	glCallList(itr->second.glList);
}

void DisplayListManager::releaseList ( int displayList )
{
	tmDisplayListList::iterator itr = displayLists.find(displayList);
	if ( itr == displayLists.end() )
		return;

	if (itr->second.glList != GL_INVALID_ID)
		glDeleteLists(itr->second.glList,1);

	displayLists.erase(itr);
}

void DisplayListManager::rebuildList ( int displayList )
{
	tmDisplayListList::iterator itr = displayLists.find(displayList);
	if ( itr == displayLists.end() )
		return;

	if (itr->second.glList != GL_INVALID_ID)
		glDeleteLists(itr->second.glList,1);

	itr->second.glList = glGenLists(1);
	buildGLList(itr->first, itr->second.glList,itr->second.builder);
}

void DisplayListManager::buildGLList ( int id, unsigned int list, DisplayListBuilder* builder )
{
	glPushMatrix();
		glNewList(list,GL_COMPILE);
		builder->build(id);
		glEndList();
	glPopMatrix();
}


