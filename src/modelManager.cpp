/* Open Gaming Framework
* Copyright (c) 2002 - 2005 Jeffrey Myers
*
* This package is free software;  you can redistribute it and/or
* modify it under the terms of the license found in the file
* named license.txt that should have accompanied this file.
*
* THIS PACKAGE IS PROVIDED ``AS IS'' AND WITHOUT ANY EXPRESS OR
* IMPLIED WARRANTIES, INCLUDING, WITHOUT LIMITATION, THE IMPLIED
* WARRANTIES OF MERCHANTIBILITY AND FITNESS FOR A PARTICULAR PURPOSE.
*/
#include "modelManager.h"

CMeshObjectInstance::CMeshObjectInstance()
{
	transformCB = NULL;
	param = NULL;
	object = NULL;
	skinID = -1;
	scaleFactor.Set(1,1,1);;
}

CMeshObjectInstance::CMeshObjectInstance(CBaseModel* meshObject)
{
	transformCB = NULL;
	param = NULL;
	object = meshObject;
	skinID = -1;
	scaleFactor.Set(1,1,1);;
}

void CMeshObjectInstance::draw ( CDrawingList *dl )
{
	CDrawingList &dl = CDrawingList::inst
	for ( int i = 0; i < object->getNumSubMeshes();i++ )
	{
		int materialID = 0;
		// get the material ID for the sub mesh here

		if (dl)
			dl->AddDrawItem(this,materialID,ePass1,i);
		else
			DrawItem(materialID,i);
	}
}

void CMeshObjectInstance:DrawItem ( int item, int param )
{
	if (object)
		object->drawsubMesh(param,item);
}

void CMeshObjectInstance::translate ( C3DVertex _pos, bool relitive )
{
	if (relitive)
		pos += _pos;
	else
		pos = _pos;
}

void CMeshObjectInstance::rotate ( C3DVertex _rot, bool relitive )
{
	if (relitive)
		rot += _rot;
	else
		rot = _rot;
}

void CMeshObjectInstance::scale ( C3DVertex factor, bool relitive )
{
	if (relitive)
		scale += factor;
	else
		scale = factor;
}

C3DVertex CMeshObjectInstance::position ( void )
{
	return pos;
}

C3DVertex CMeshObjectInstance::rotation ( void )
{
	return rot;
}

C3DVertex CMeshObjectInstance::scaling ( void )
{
	return scale;
}

void CMeshObjectInstance::transform ( void )
{
	if (transformCB)
		transformCB->transform(*this,param);
	else
	{
		glTranslatef(pos.X(),pos.Y(),pos.Z());
		glRotatef(rot.Z(),0,0,1);
		glRotatef(rot.X(),1,0,0);
		glRotatef(rot.Y(),0,1,0);

		glScalef(scaleFactor.X(),scaleFactor.Y(),scaleFactor.Z());
	}
}

CMeshObjectSkin::CMeshObjectSkin()
{
}
CMeshObjectSkin::~CMeshObjectSkin()
{
}

bool CMeshObjectSkin::load ( trParserBlock &block )
{
	return false;
}
int CMeshObjectSkin::pushSubSkin (tvSkinPasses subSkin)
{
	return 0;
}

int CMeshObjectSkin::getNumSubSkins ( void )
{
	return 0;
}
int CMeshObjectSkin::getNumSubSkinPasses ( int subSkin )
{
	return 0;
}
void CMeshObjectSkin::setSubSkinPass ( int subSkin, int pass )
{
}
void CMeshObjectSkin::resetPass ( void )
{
}


//-----------------------CMeshObjectManager-------------------------------
CMeshObjectManager::CMeshObjectManager()
{
}

CMeshObjectManager::~CMeshObjectManager()
{
	flush();
}

// scans dirs for known models and skin formats
int CMeshObjectManager::loadAll ( const char *dir, bool recursive );
{
	return loadAll(std::string(dir),recursive);
}

int CMeshObjectManager::loadAll ( std::string dir, bool recursive )
{
	COSDir osDIR(dir);

	COSFile file;

	int count = 0;
	while ( osDIR.GetNextFile(file,NULL,recursive) )
		count += load(file);

	return count;
}

// loads one file
int CMeshObjectManager::load ( COSFile &file )
{
	std::string ext = file.GetExtension();
	
	if (objectFactorys.find(toupper(ext)) == objectFactorys.end())
		return 0;

	CBaseModelFactory* factory = objectFactorys[toupper(ext)];
	if (!factory)
		return 0;

	CBaseModel	*object = factory->newModel();
	if (!object)
		return 0;

	object->manager = this;

	if (!object.load(file))
	{
		factory->deleteModel(object);
		return 0;
	}

	objectList[toupper(file.GetFileTitle())] = object;
	return 1;
}

void CMeshObjectManager::flush ( void )
{
	std::map<std::string,CBaseModel*>::iterator itr = objectList.begin();

	while (itr != objectList.end())
	{
		CBaseModel	*object = itr->second;

		CBaseModelFactory* factory = objectFactorys[object->getExtenstion()];
		if (factory)
			factory->deleteModel(object);
		itr++;
	}

	objectList.clear();
	objectFactorys.clear();
}

int CMeshObjectManager::getSkinCount ( void )
{
	return 0;
}

std::string CMeshObjectManager::getSkinName ( int skinID )
{
	return std::string("");
}

int CMeshObjectManager::getSkinID ( std::string skinName )
{
	return 0;
}

std::vector<int> CMeshObjectManager::getMeshTagedSkins ( std::string meshName )
{
	std::vector<int>	v;
	return v;
}

bool CMeshObjectManager::newInstance ( CMeshObjectInstance &inst, std::string meshName )
{
	std::map<std::string,CBaseModel*>::iterator itr = objectList.find(toupper(meshName));

	if (itr == objectList.end())
		return false;

	inst.setObjet(itr->second);
	return true;
}

void CMeshObjectManager::registerModelHandaler ( std::string extenstion, CBaseModelFactory* factory )
{
	objectFactorys[toupper(extenstion)] = factory;
}
