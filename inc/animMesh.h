// CAnimMesh.h
#ifndef _ANIME_MESH_H_
#define _ANIME_MESH_H_

#ifdef _WIN32
	#pragma warning( disable : 4786 )  // Disable warning message
#else
	#define stricmp strcasecmp
#endif

#ifdef _WIN32
#include <windows.h>
#endif// W32

#include "openGL.h"
#include "utils.h"
#include "3dTypes.h"
#include "textureManager.h"

#include <vector>
#include <string>
#include <map>

typedef struct
{
	unsigned int	index;
	trVertex2D		tc;
}trIndecedVert;

typedef struct
{
	trIndecedVert	aVerts[3];
}trIndexedFace;

typedef std::vector<trIndexedFace>		tvIndexedFaceList;

typedef struct
{
	trVertex3D	faceNorm;
	trVertex3D	norms[3];
}trFaceVertNorm;

typedef std::vector<trVertex3D>		tvVertexList;
typedef std::vector<trFaceVertNorm>	tvFaceNormList;
typedef std::vector<tvFaceNormList>	tvFaceNormFrameList;

typedef struct
{
	trVertex3D	h,v,d,o;
}trTagCS;

typedef std::vector<trTagCS>	tvTagCSList;

typedef struct
{
	tvVertexList	vVerts;
	tvTagCSList		vTagCSs;
}trAnimFrame;

typedef std::vector<trAnimFrame>	tvFrameList;

typedef struct
{
	std::string	name;
	int			tagID;
	int			tagmateID;
}trTag;

typedef std::vector<trTag>	tvTagList;
typedef std::vector<int>	tvSkinIDList;

class CAnimModel;

class CMesh
{
public:
	CMesh();
	~CMesh();

	void Draw ( int iThisFrame, int iNextFrame, float fParam, bool bGenTextures, bool useFaceNormals = false );

	void Smooth ( float fAngle );
	void Index ( void );

	void Reverse ( void );

	tvIndexedFaceList		m_vFaces;
	tvFaceNormFrameList		m_vFaceNormalFrames;
	std::string				m_sName;

	CTextureManager			*m_pTextureManager;

	trVertex3D				m_rScale;
	CAnimModel				*m_pModel;

protected:
	trVertex3D	getNormal ( int frame, int face, int vert );
};

typedef std::vector<CMesh>	tvMeshList;

typedef struct
{
	int				id;
	std::string		name;
	std::string		group;
	unsigned int	start;
	unsigned int	len;
	float			speed;
}trSequence;

typedef std::vector<trSequence>	tvSequenceListList;

typedef struct
{
	std::string		name;
	trSequence		*seq;
	float			time;
}trEvent;

typedef std::vector<trEvent>	tvEventListList;

typedef	enum
{
	eNone,
	eEnviron,
	eLiniar,
	eEyeLinar,
	eCubic
}teTexGenMode;

typedef enum
{
	eNoCull,
	eCullBack,
	eCullFront,
	eDoubleSided
}teCullMode;

typedef struct
{
	int				iMeshID;
	std::string		sTexture;
	int				iTextureID;
	GLenum			eTexMode;
	teTexGenMode	eTCGenMode;
	bool			bBlend;
	GLenum			eSrcMode;
	GLenum			eDestMode;
	teCullMode		eCullMode;
	bool			bDepthWrites;
	GLenum			eDepthFunc;
	bool				bUseFaceNorms;
}trSkinTexureItem;

typedef std::vector<trSkinTexureItem>	tvSkinTextureItemList;

class CMeshSkin
{
public:
	CMeshSkin();
	~CMeshSkin();

	int GetNumPasses ( void );
	trSkinTexureItem* GetPass ( int iPass );

	void NewSimplePass ( const char *szTexture );
	
	int NewPass ( void );
	void RemovePass ( int iPassID );

	void SetPass ( int iPassID );
	void ReSetPass ( int iPassID );

	bool Hidden ( void ) { return m_bHiden; };
	void Show ( bool bShow ) { m_bHiden = !bShow; }

	bool PassUsesTCs ( int iPassID );
	bool PassUsesFaceNorms ( int iPassID );

protected:
	int LoadOne ( COSFile &oFile);

	tvSkinTextureItemList	m_vTexutrePasses;

	bool m_bHiden;
};

typedef std::vector<CMeshSkin>	tvMeshSkinList;

class CSkin
{
public:
	CSkin();
	~CSkin();

	int GetNumMeshes ( void );
	CMeshSkin* GetMesh ( int iMeshID );

	int NewMesh( void );
	void RemoveMesh ( int iPassID );

	void Init( int iNumMeshes );

	bool Load ( const char *szFile );
	bool Save ( const char *szFile );

	void SetName ( const char *szName ) { m_sName = szName; }
	const char* GetName ( void ) { return m_sName.c_str(); }
	
protected:
	std::string		m_sName;
	tvMeshSkinList	m_vMeshSkins;
};

typedef std::vector<CSkin>	tvSkinList;

class CAnimModel
{
public:
	CAnimModel();
	~CAnimModel();

	bool LoadA3D ( const char * szFile );
	bool LoadU3D ( const char *szPath, const char * szFile );
	bool LoadMD3 ( const char * szFile );
	bool LoadOBJ ( const char * szFile );
	bool LoadBZG ( const char * szFile );

	bool SaveA3D ( const char * szFile );
	bool SaveBZG ( const char * szFile );

	int GetMeshCount ( void );
	int GetMeshID ( const char *szName );
	CMesh* GetMesh ( int iMeshID );
	
	int GetNumFrames ( void );
	trAnimFrame* GetFrame ( int iFrame );

	int GetNumSequences ( void );
	int GetSequenceID ( const char *szName );
	trSequence* GetSequence ( int iSeqID );

	int GetNumEvents ( void );
	int GetEventID ( const char *szName );
	trEvent* GetEvent ( int iEventID );

	int GetNumSkins ( void );
	int GetSkinID ( const char *szName );
	CSkin* GetSkin ( int iSkinID );

	int NewSimpleSkin ( const char *szTextureName ); // makes a skin for all meshes that has the set textures
	int NewSimpleSkin ( std::vector<std::string> &textures ); // makes a skin for all meshes using the list of textures
	int NewSkin ( void );
	int AddSkin ( CSkin &rSkin );
	bool RemoveSkin ( int iID );

	bool LoadSkin ( const char *szFile );
	bool SaveSkin ( int iSkinID, const char *szFile );

	void Draw ( int iSkinID, int iThisFrame, int iNextFrame, float fParam );
	void Draw ( int iSkinID, int iThisFrame, int iNextFrame, float fParam, bool bExplode, float fExplodeParam );

	void SetScale ( trVertex3D rScale ) {m_rAddScale = rScale;}
	trVertex3D GetScale ( void ) {return m_rAddScale;}

protected:
	int					m_iEnvironTexture;
	tvMeshList			m_vMeshes;
	tvSkinList			m_vSkins;

	tvSequenceListList	m_vSequences;
	tvEventListList		m_vEvents;

	trVertex3D			m_rOffset;
	trVertex3D			m_rRotation;
	trVertex3D			m_rScale;
	trVertex3D			m_rAddScale;

	tvFrameList			m_vFrames;			
	tvTagList			m_vTags;

	void LoadUCFile ( char *szName, char* szAName, char *szDName );
};


typedef enum
{
	eOBJ,
	eMD3,
	eU3d,
	eA3d
}teModelType;

typedef struct
{
	CAnimModel	*pModel;
	COSFile		oFile;
	teModelType	eType;
	std::string	szName;
}trModelRefrence;

typedef std::vector<trModelRefrence> tvModelList;
typedef std::map<std::string, int> tvModelNameMap;
typedef std::vector<int> tvModelIDList;

class CModelManager
{
public:
	CModelManager();
	~CModelManager();

	void LoadAll ( const char *szDirName );
	void Load ( const char *szName );

	void Flush ( void );

	int GetModelID ( const char *szModelName );

	CAnimModel* GetModel (  const char *szModelName );
	CAnimModel* GetModel ( int iID );

	int GetModelCount ( void );

	int GetSubList ( const char* szKey, tvModelIDList &rList );
	
protected:
	int LoadOne ( COSFile &oFile );

	void LoadModelFile ( trModelRefrence &rModelRef );

	tvModelList			m_vModelList;
	tvModelNameMap		m_mModelNameMap;
};

#endif//_ANIME_MESH_H_
