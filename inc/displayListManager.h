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
#ifndef _DISPLAY_LIST_MANAGER_H_
#define _DISPLAY_LIST_MANAGER_H_

// this class is a singleton manager
// that takes care of the creation and removal of open GL display lists
// it abstracts the list ID away from GL
// and manages a set of callback classes that are used
// to rembuild the lists when ever they are needed.
// this needs to happen after a context is invalidated, after
// a deactivate/activate pair ( aka alt+tab )

#include "singleton.h"

#include <map>

// class that will provide the openGL calls to build a list
// whenever the manager may need it
class DisplayListBuilder
{
public:
	virtual ~DisplayListBuilder(){return;}
	virtual void build ( int list ) = 0;
};

class DisplayListManager : public Singleton<DisplayListManager>
{
protected:
	friend class Singleton<DisplayListManager>;

	DisplayListManager();
	~DisplayListManager();

public:
	// clear us out for new lists and stuff;
	void init ( void );

	// before the context is killed, release all lists
	// but don't remove them from them manager
	void releaseAll ( void );

	// after the context is restored, rebuild and bind all lists again
	void rebuildAll ( void );

	// give us a new list and a builder to call to get it's geometry
	int newList ( DisplayListBuilder *builder, bool dontBuild = false );

	// call a stored list
	void callList ( int displayList );

	// kill a stored list and remove it from the manager
	void releaseList ( int displayList );

	// remove the old GL binding, and recall the builder to bind to a new GL list
	// this is used in case the contents of a list change.
	void rebuildList ( int displayList );

protected:
	void buildGLList ( int id, unsigned int list, DisplayListBuilder*	builder );
	typedef struct 
	{
		DisplayListBuilder*	builder;
		unsigned int				glList;
	}trDisplayListInfo;

	typedef std::map<int,trDisplayListInfo>	tmDisplayListList;
	tmDisplayListList		displayLists;
	int lastList;
};	

#endif//_DISPLAY_LIST_MANAGER_H_
