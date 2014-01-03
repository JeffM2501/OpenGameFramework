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
#ifndef _MATERIALS_MANAGER_H_
#define _MATERIALS_MANAGER_H_

// this class is a singleton manager
// that takes care of the creation and removal of open GL display lists
// it abstracts the list ID away from GL
// and manages a set of callback classes that are used
// to rembuild the lists when ever they are needed.
// this needs to happen after a context is invalidated, after
// a deactivate/activate pair ( aka alt+tab )

#include "singleton.h"
#include <string>
#include <map>
#include <vector>
#include "3dTypes.h"

class MaterialDef
{
public:
	MaterialDef(){textureID = -1;}
	bool operator == ( const MaterialDef& r );

	std::string texture;
	int					textureID;
	CColor			color;
};

class MaterialsManager : public Singleton<MaterialsManager>
{
protected:
	friend class Singleton<MaterialsManager>;

	MaterialsManager();
	~MaterialsManager();

public:

	int find ( std::string name );
	int add(MaterialDef mat, std::string name = std::string(""));
	void clear ( void );
	void bind( int mat );

	int count ( void ) { return (int)materalList.size();}
protected:

	int find ( MaterialDef mat );

	std::vector<MaterialDef> materalList;
	std::map<std::string, int> materalNameMap;

	int lastMat;
};	

#endif//_MATERIALS_MANAGER_H_
