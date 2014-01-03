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
// that takes care of managing simple materials
// it needs more then just color later, but for now this will do
#include "materials.h"
#include "openGL.h"
#include "textUtils.h"
#include "textureManager.h"

template <>
MaterialsManager* Singleton<MaterialsManager>::_instance = (MaterialsManager*)0;

bool MaterialDef::operator == ( const MaterialDef& r )
{
	return color == r.color && r.texture == texture && r.textureID == textureID;
}

MaterialsManager::MaterialsManager()
{
	lastMat = -1;
}

MaterialsManager::~MaterialsManager()
{
}

int MaterialsManager::add(MaterialDef mat, std::string name)
{
	int ID = find(mat);

	if ( ID == -1 )	// gotta add it
	{
		ID = (int)materalList.size();
		materalList.push_back(mat);
	}
	if(name.size())
		materalNameMap[string_util::toupper(name)] = ID;

	return ID;
}

int MaterialsManager::find ( std::string name )
{
	if (!name.size())
		return -1;

	if (materalNameMap.find(string_util::toupper(name)) == materalNameMap.end())
		return -1;

	return materalNameMap[string_util::toupper(name)];
}

int MaterialsManager::find ( MaterialDef mat )
{
	for ( unsigned int i = 0; i < materalList.size(); i++ )
	{
		if ( mat == materalList[i] )
			return (int)i;
	}
	return -1;
}

void MaterialsManager::bind( int mat )
{
	if ( mat < 0 || mat > (int)materalList.size())
		return;

	if ( lastMat != -1 )
	{
		if (materalList[lastMat].textureID == -1 && materalList[mat].textureID != -1)	// textures were off now on
			glEnable(GL_TEXTURE_2D);
		else if (materalList[lastMat].textureID != -1 && materalList[mat].textureID == -1)	// textures were on now off
			glDisable(GL_TEXTURE_2D);

		if (materalList[lastMat].color != materalList[mat].color)
			materalList[mat].color.glColor();
	}
	else
		materalList[mat].color.glColor();

	if (materalList[mat].textureID != -1)
		CTextureManager::instance().Bind(materalList[mat].textureID);
}
