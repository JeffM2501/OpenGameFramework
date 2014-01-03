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

#ifndef	_OBJ_FILE_H_
#define	_OBJ_FILE_H_

#include "modelManager.h"

static float shineFactor = 1.0f;
static const float maxShineExponent = 128.0f; // OpenGL minimum shininess


typedef std::vector<int> tvIndexList;

typedef enum
{
	eXAxis,
	eYAxis,
	eZAxis
}teModelAxis;

class CTexCoord
{
public:
	CTexCoord(){u = v = 0;}
	~CTexCoord(){};
	float u,v;

	bool same ( const CTexCoord &c );

	void glTexCoord2f( void ){::glTexCoord2f(u,v);}
};

typedef std::vector<CTexCoord> tvTexCoordList;

class CVertex
{
public:
	CVertex(){x = y = z = 0;}
	~CVertex(){};
	float x,y,z;

	float get ( teModelAxis axis );
	void translate ( float val, teModelAxis axis );
	bool same ( const CVertex &v );

	void glNormal3f( void ){::glNormal3f(x,y,z);}
	void glVertex3f( void ){::glVertex3f(x,y,z);}
};

typedef std::vector<CVertex> tvVertList;

class CFace
{
public:
	CFace(){};
	~CFace(){};

	tvIndexList verts;
	tvIndexList	normals;
	tvIndexList	texCoords;
};
typedef std::vector<CFace> tvFaceList;


class CMaterial
{
public:
	CMaterial(){clear();}
	~CMaterial(){};

	std::string texture;
	float		ambient[4];
	float		diffuse[4];
	float		specular[4];
	float		emission[4];
	float		shine;

	void clear ( void );
};

typedef std::map<std::string,CMaterial> tmMaterialMap;


class CMesh
{
public:
	CMesh(){};
	~CMesh(){};

	std::string material;

	tvVertList		verts;
	tvVertList		normals;
	tvTexCoordList	texCoords;

	std::string name;
	tvFaceList	faces;

	float getMaxAxisValue ( teModelAxis axis );
	float getMinAxisValue ( teModelAxis axis );
	void translate ( float value, teModelAxis axis );
	bool valid ( void );
	void clear ( void );
	void reindex ( void );

	void draw ( void );
};

typedef std::vector<CMesh> tvMeshList;

class CObjModel
{
public:
	CObjModel(){};
	~CObjModel(){};

	tmMaterialMap	materials;
	tvMeshList		meshes;

	void clear ( void );
};

class CObjFile : public CBaseModel
{
public:
	virtual ~CObjFile();

	virtual bool load ( COSFile &file );
	virtual std::string getExtenstion ( void );
	virtual int getNumSubMeshes ( void );
	virtual void drawsubMesh ( int submesh, int material, int UVset = 0  );

protected:
	CObjModel	objModel;
};

class CObjModelFactory : public CBaseModelFactory
{
public:
	virtual CBaseModel* newModel ( void ){ return new CObjFile;}
	virtual void deleteModel  ( CBaseModel *model ){ delete (model);}
};

extern CObjModelFactory objFactory;

#endif //_OBJ_FILE_H_
