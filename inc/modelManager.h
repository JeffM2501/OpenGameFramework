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

#ifndef	_MODEL_MANAGER_H_
#define	_MODEL_MANAGER_H_

#include "3dTypes.h"
#include "drawlist.h"
#include <vector>
#include "parser.h"
#include "OSFile.h"
#include "openGL.h"

class CMeshObjectManager;

class CBaseModel
{
public:
	virtual ~CBaseModel(){};

	virtual bool load ( COSFile &file ) = 0;
	virtual std::string getExtenstion ( void ) = 0;

	virtual int getNumSubMeshes ( void ) = 0;
	
	virtual void drawsubMesh ( int submesh, int material, int UVset = 0 ) = 0;

	virtual std::string getDefaultSkin ( void ){return std::string("");}

	CMeshObjectManager	*manager;
};

class CBaseModelFactory
{
public:
	virtual ~CBaseModelFactory(){};
	virtual CBaseModel* newModel ( void ) =0;
	virtual void deleteModel  ( CBaseModel *model ) = 0;
};

class CMeshObjectInstance;

class CBaseObjectTransformCallback
{
public:
	virtual ~CBaseObjectTransformCallback(){};
	virtual bool transform ( CMeshObjectInstance &instance, void *param ) = 0;
};

class CMeshObjectInstance : public CBaseDrawItem
{
public:
	CMeshObjectInstance();
	CMeshObjectInstance(CBaseModel* meshObject);

	void setObjet ( CBaseModel* meshObject ){object = meshObject;}
	void setOSkin ( int skin ){skinID = skin;}
	void setTransformCallBack ( CBaseObjectTransformCallback *cb, void *p ){transformCB = cb;param=p;}

	void draw (	CDrawingList *dl = NULL );

	void translate ( C3DVertex pos, bool relitive );
	void rotate ( C3DVertex rot, bool relitive );
	void scale ( C3DVertex factor, bool relitive );

	C3DVertex position ( void );
	C3DVertex rotation ( void );
	C3DVertex scaling ( void );

	void transform ( void );

	virtual void DrawItem ( int item, int param = 0 );

protected:
	CBaseObjectTransformCallback	*transformCB;
	void							*param;
	CBaseModel						*object;
	int								skinID;

	C3DVertex						pos;
	C3DVertex						rot;
	C3DVertex						scaleFactor;
};

typedef struct 
{
	std::string texture;
	int					textureID;
	CColor			color;
	bool				useLighting;
	bool				bDepthWrites;
	int					uvPass;
}trSkinPassDescriptor;

typedef std::vector<trSkinPassDescriptor>	tvsubSkin;

class CMeshObjectSkin
{
public:
	CMeshObjectSkin();
	~CMeshObjectSkin();

	int pushSubSkin (tvsubSkin subSkin);

	int getNumSubSkins ( void );
	int getNumSubSkinPasses ( int subSkin );

	void setSubSkinPass ( int subSkin, int pass );
	void resetPass ( void );

	std::string getSkinName ( void ){return name;}

	void makeSimple( int textureID, int subSkins );

protected:
	std::string					name;
	std::vector<tvsubSkin>	subSkins;
};

class CMeshObjectManager
{
	CMeshObjectManager();
	~CMeshObjectManager();

	// scans dirs for known models and skin formats
	int loadAll ( std::string dir, bool recursive );
	int loadAll ( const char *dir, bool recursive );

	int load ( COSFile &file );

	void flush ( void );

	int newSkin ( std::string name = std::string("") );

	int getSkinCount ( void );

	std::string getSkinName ( int skinID );
	int getSkinID ( std::string skinName );

	CMeshObjectSkin* getSkin ( int skinID );
	CMeshObjectSkin* getSkin ( std::string skinName );

	void asignSkinToMesh ( int mesh, int skin );

	std::vector<int> getMeshAsignedSkins ( std::string meshName );

	bool newInstance ( CMeshObjectInstance &inst, std::string meshName );

	void registerModelHandaler ( std::string extenstion, CBaseModelFactory* factory );
protected:

	std::map<std::string,CBaseModelFactory*> objectFactorys;
	std::map<std::string,CBaseModel*>		 objectList;

	typedef std::vector<int> tvSkinRefList;
	std::map<std::string,tvSkinRefList>		meshSkins;
};

#endif //_MODEL_MANAGER_H_