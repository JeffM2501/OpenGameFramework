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
#include "animMesh.h"
#include "OSFile.h"
// UT types

#ifdef _WIN32
  #define strcasecmp stricmp
  #define strncasecmp strnicmp
#endif

#define DEBUG_NORMALS	0

typedef struct
{
	unsigned long	iVert;
}trU3DVertexI;

typedef struct
{
	char	i,j,k;
}trShortVec;
#pragma  pack(1) 
typedef struct 
{
	unsigned short	iNumPolys;
	unsigned short	iNumVerts;

	unsigned short	iUnusedRot;
	unsigned short	iUnusedFrame;

	unsigned long	iUnusedNormX;
	unsigned long	iUnusedNormY;
	unsigned long	iUnusedNormZ;

	unsigned long	iScale;

	unsigned long	iUnused[3];

	unsigned char	name[12];

}trU3DDataHeader;

typedef enum
{
	eNormal1Side = 0,
	eNormal2Side ,
	eTransparent2Side,
	eMasked2Side,
	eModBlend2Side,
	eTagTri = 8
}teUTJamesPolyType;

typedef struct
{
	unsigned short		aVerts[3];
	char				cType;
	char				cColor;
	unsigned char		aRawUVs[3][2];
	char				cTextureID;
	char				cFlags;
}trU3DTriangle;
#pragma  pack() 

typedef struct
{
	trU3DVertexI	*pVerts;
	trU3DVertexI	*pShortNorms;
	trFaceVertNorm	*pVertNorms;
}trU3DFrame;

// name uitls
void GetModelName ( char *szData )
{
#ifdef	_WIN32
	strupr(szData);
#else
	while(*szData++)
		*szData = toupper(*szData);
#endif
}

// unreal utils
void SetShortVec (trShortVec &outVec,trVertex3D&inVec)
{
	outVec.i = (char)(inVec.x * 127.0f);
	outVec.j = (char)(inVec.y * 127.0f);
	outVec.j = (char)(inVec.z * 127.0f);
}

void GetShortVec (trVertex3D &outVec,trShortVec &inVec)
{
	outVec.x = (float)(inVec.i * 0.0078740157480315f);
	outVec.y = (float)(inVec.j * 0.0078740157480315f);
	outVec.z = (float)(inVec.k * 0.0078740157480315f);
}

void NormalizeShortVec (trShortVec	&vec)
{	
	float	len = (float)sqrt((double)(vec.i*vec.i+vec.j*vec.j+vec.k*vec.k));

	vec.i = (char)((float)vec.i/(len * 127.0f));
	vec.j = (char)((float)vec.j/(len * 127.0f));
	vec.k = (char)((float)vec.k/(len * 127.0f));
}

void GetIntVert ( unsigned int inVert, trVertex3D &outVert)
{
	outVert.x = ((float)(inVert & 0x7ff)*0.125f);
	
	if (outVert.x > 128.0f)
		outVert.x -= 256.0f;

	outVert.y = ((float)((inVert>>11) & 0x7ff)*0.125f);

	if (outVert.y > 128.0f)
		outVert.y -= 256.0f;

	outVert.z = ((float)((inVert>>22) & 0x3ff)*0.25f);

	if (outVert.z > 128.0f)
		outVert.z -= 256.0f;
}

unsigned int MakeIntVert ( trVertex3D &inVert )
{
	return ((int)(inVert.x * 8.0)&0x7ff)|(((int)(inVert.y*8.0)&0x7ff)<<11)|(((int)(inVert.z*4.0)&0x3ff)<<22);
}

bool Is2Sided ( unsigned char cFlag )
{
	if ( ((teUTJamesPolyType)cFlag ==eNormal2Side) ||
		((teUTJamesPolyType)cFlag ==eTransparent2Side) ||
		((teUTJamesPolyType)cFlag ==eMasked2Side) ||
		((teUTJamesPolyType)cFlag ==eModBlend2Side) )
		return true;

	return false;
}

// MD3 structs

typedef struct
{
	char				name[64];
	trVertex3D			pos;
	float				rawMatrix[9];
	int					id;
}tMD3Tag;

typedef struct
{
	char	str[68];
}tMD3String;

typedef struct
{
	char	id[4];
	tMD3String	name;
	long	numMeshFrames;
	long	numSkins;
	long	numVerts;
	long	numTris;
	long	triStart;
	long	headerSize;
	long	TXvertStart;
	long	vertStart;
	long	meshSize;
}tMD3MeshHeader;

typedef struct
{
	long		a,b,c;
	int			id;
}tMD3IndexedTri;

typedef struct
{
	trVertex3D	na,nb,nc;
}trMD3VertNormals;

typedef struct
{
	float u,v;
}tMD3uv;

typedef struct
{
	signed short	x,y,z;
	unsigned char	u,v;
}tMD3iVertex;

typedef struct
{
	char	id[4];
	long	vers;
	char	fileName[68];
	long	numBoneFrames;
	long	numTags;
	long	numMeshes;
	long	numSkins;
	long	headerLen;
	long	tagStart;
	long	tagEnd;
	long	fileSize;
}tMD3FileHeader;

typedef struct
{
	trVertex3D	mins;
	trVertex3D	maxs;
	trVertex3D	pos;
	float		scale;
	char		creator[16];
}tMD3BoneFrame;
// common utils

// vertex utils
void GetInterpVert ( trVertex3D &v1, trVertex3D &v2, trVertex3D &outVert, float param )
{
	outVert = v1 + ((v2-v1)*param);
}

void GetInterpVec ( trVertex3D &v1, trVertex3D &v2, trVertex3D &outVec, float param )
{
	outVec = v1 + ((v2-v1)*param);
	set_mag(outVec,1.0f);
}

// text utitls
bool GetLine ( char *pData, COSFile	&oFile )
{
	char	*pPtr = pData;
	char	thisChar;

	*pPtr = '\0';

	if (oFile.Read(&thisChar,1) == 0)
		return true;

	while ((thisChar != 13))
	{
		*pPtr = thisChar;
		pPtr++;

		if (oFile.Read(&thisChar,1) == 0)
		{
			*pPtr = '\0';
			return true;
		}
	}
	*pPtr = '\0';

	oFile.Read(&thisChar,1); // get the 10

	return false;
}

bool GetLine2( char *pData, COSFile	&oFile )
{
	char	*pPtr = pData;
	char	thisChar;

	*pPtr = '\0';

	if (oFile.Read(&thisChar,1) == 0)
		return true;

	while ((thisChar != '\n'))
	{
		*pPtr = thisChar;
		pPtr++;

		if (oFile.Read(&thisChar,1) == 0)
		{
			*pPtr = '\0';
			return true;
		}
	}
	*pPtr = '\0';

	return false;
}
CMesh::CMesh()
{
	m_rScale.x = m_rScale.y = m_rScale.z = 1.0f;
	m_pModel = NULL;
}

CMesh::~CMesh()
{

}

trVertex3D	CMesh::getNormal ( int frame, int face, int vert )
{
	trVertex3D	temp;
	temp.x = 0;
	temp.y = 0;
	temp.z = 0;

	int frames = (int)m_vFaceNormalFrames.size();
	if ( frame >= frames )
		return temp;

	int faces = (int)m_vFaceNormalFrames[frame].size();
	if ( face >= faces )
		return temp;
	if ( vert >= 3 )
		return temp;

	 return m_vFaceNormalFrames[frame][face].norms[vert];
}


void CMesh::Draw ( int iThisFrame, int iNextFrame, float fParam, bool bGenTextures, bool useFaceNormal )
{
	if (!m_pModel)
		return;

	if (m_pModel->GetNumFrames() < 1)
		return;

	trVertex3D	rThisV,rNextV,rInterpV,rNormal,TempScale;

	trAnimFrame	*pThisFrame = m_pModel->GetFrame(iThisFrame);
	trAnimFrame	*pNextFrame = m_pModel->GetFrame(iNextFrame);

	tvIndexedFaceList::iterator		iFaceItr = m_vFaces.begin();

	int iFace = 0;
	glBegin(GL_TRIANGLES);

	TempScale = m_rScale;
	TempScale = m_pModel->GetScale();

	while (iFaceItr != m_vFaces.end())
	{
		for ( int i = 0; i < 3; i++ )
		{
				if ( !bGenTextures )
					glTexCoord2f(iFaceItr->aVerts[i].tc.s,iFaceItr->aVerts[i].tc.t);

			//	if (!useFaceNormal)
			//	{
					rThisV = getNormal(iThisFrame,iFace,i);
					rNextV =  getNormal(iNextFrame,iFace,i);
			//	}
			//	else
			//	{
			//		rThisV = m_vFaceNormalFrames[iThisFrame][iFace].faceNorm;
			//		rNextV =  m_vFaceNormalFrames[iNextFrame][iFace].faceNorm;
			//	}
				GetInterpVec(rThisV,rNextV,rNormal,fParam);

				glNormal3f(rNormal.x,rNormal.y,rNormal.z);

				rThisV = pThisFrame->vVerts[iFaceItr->aVerts[i].index];
				rNextV = pNextFrame->vVerts[iFaceItr->aVerts[i].index];
				GetInterpVert(rThisV,rNextV,rInterpV,fParam);

				rInterpV *= TempScale;
				
				glVertex3f(rInterpV.x,rInterpV.y,rInterpV.z);
		}
		iFace++;
		iFaceItr++;
	}
	glEnd();
}

void CMesh::Index ( void )
{

}

void CMesh::Reverse ( void )
{
	trIndecedVert	rTemp;

	tvIndexedFaceList::iterator	faceItr = m_vFaces.begin();

	while (faceItr != m_vFaces.end())
	{
		rTemp = faceItr->aVerts[0];
		faceItr->aVerts[0] = faceItr->aVerts[2];
		faceItr->aVerts[2] = rTemp;

		faceItr++;
	}

	tvFaceNormFrameList::iterator normFrameItr = m_vFaceNormalFrames.begin();

	while (normFrameItr != m_vFaceNormalFrames.end())
	{
		tvFaceNormList::iterator normItr = normFrameItr->begin();
		while (normItr != normFrameItr->end())
		{
			normItr->faceNorm *= -1;
			normItr++;
		}
		normFrameItr++;
	}
}


typedef std::vector<int>	tvIntList;
typedef std::map<int,tvIntList> tmIntListIntMap;

void CMesh::Smooth ( float fAngle )
{
	float fLimit = cos(fAngle);

	tmIntListIntMap	mvFaceMap;
	tvIntList				vFaceList;
	int	iFace = 0;

	//trAnimFrame	*pFrame = m_pModel->GetFrame(0);

	tvIndexedFaceList::iterator	faceItr = m_vFaces.begin();
	tvIndexedFaceList::iterator otherFaceItr;

	while (faceItr != m_vFaces.end())
	{
		for (int i = 0; i < 3; i++)
		{
			vFaceList.clear();
			unsigned int iVertIndex = faceItr->aVerts[i].index;

			int iOtherFace = 0;
			otherFaceItr = m_vFaces.begin();

			while (otherFaceItr != m_vFaces.end())
			{
				if ( (otherFaceItr->aVerts[0].index == iVertIndex) || (otherFaceItr->aVerts[1].index == iVertIndex) || (otherFaceItr->aVerts[2].index == iVertIndex) )
					vFaceList.push_back(iOtherFace);

				iOtherFace++;
				otherFaceItr++;
			}
			mvFaceMap[iVertIndex] = vFaceList;
		}
		iFace++;
		faceItr++;
	}

	tvFaceNormFrameList::iterator	rFrameNormIr = m_vFaceNormalFrames.begin();
	tvIntList::iterator		faceListItr;

	trVertex3D		fThisNorm;
	trVertex3D		fAcumNorm;
	int				iTotalNorms;

	while (rFrameNormIr != m_vFaceNormalFrames.end())
	{
		faceItr = m_vFaces.begin();
		iFace = 0;

		while (faceItr != m_vFaces.end())
		{
			fThisNorm = (*rFrameNormIr)[iFace].faceNorm;
			for (int i = 0; i < 3; i++)
			{
				int iVertIndex = faceItr->aVerts[i].index;
				
				faceListItr = mvFaceMap[iVertIndex].begin();

				fAcumNorm = fThisNorm;
				iTotalNorms = 1;

				while (faceListItr != mvFaceMap[iVertIndex].end() )
				{
					if ( (*faceListItr) != iFace )
					{
						if ( dot(fThisNorm,(*rFrameNormIr)[(*faceListItr)].faceNorm)>fLimit )
						{
							fAcumNorm += (*rFrameNormIr)[(*faceListItr)].faceNorm;
							iTotalNorms ++;

						}
					}
					faceListItr++;
				}

				if (iTotalNorms > 1)
				{
					fAcumNorm /= (float)iTotalNorms;
					set_mag(fAcumNorm,1.0f);

				}
				(*rFrameNormIr)[iFace].norms[i] = fAcumNorm;
			}
			faceItr++;
			iFace++;
		}
		rFrameNormIr++;
	}
}

//**********************************SkinClasses*********************************

//**********************************CMeshSkin***********************************

CMeshSkin::CMeshSkin()
{
	m_vTexutrePasses.clear();//	tvSkinTextureItemList
	m_bHiden = false;
}

CMeshSkin::~CMeshSkin()
{
	m_vTexutrePasses.clear();
}

int CMeshSkin::GetNumPasses ( void )
{
	return (int)m_vTexutrePasses.size();
}

trSkinTexureItem* CMeshSkin::GetPass ( int iPass )
{
	if ( (iPass < 0) || (iPass >= (int)m_vTexutrePasses.size()) )
		return NULL;

	return &(m_vTexutrePasses[iPass]);
}

void CMeshSkin::NewSimplePass ( const char *szTexture )
{
	int iPass = NewPass();
	m_vTexutrePasses[iPass].sTexture = szTexture;
}

int CMeshSkin::NewPass ( void )
{
	trSkinTexureItem	rNewItem;

	rNewItem.bBlend = true;
	rNewItem.bDepthWrites = true;
	rNewItem.eCullMode = eCullBack;
	rNewItem.eTexMode = GL_MODULATE;
	rNewItem.eDepthFunc = 0xffffffff;
	rNewItem.eDestMode = 0xffffffff;
	rNewItem.eSrcMode = 0xffffffff;
	rNewItem.eTCGenMode = eNone;
	rNewItem.iMeshID = 0xffffffff;
	rNewItem.iTextureID = 0xffffffff;
	rNewItem.sTexture.empty();
	rNewItem.bUseFaceNorms = false;

	m_vTexutrePasses.push_back(rNewItem);
	return (int)m_vTexutrePasses.size()-1;
}

void CMeshSkin::RemovePass ( int iPassID )
{
	if ( (iPassID < 0) || (iPassID >= (int)m_vTexutrePasses.size()) )
		return;

	m_vTexutrePasses.erase(m_vTexutrePasses.begin() + iPassID);
}

void CMeshSkin::ReSetPass ( int iPassID )
{
	trSkinTexureItem	*pNewItem = GetPass(iPassID);

	if (!pNewItem)
		return;

	if (pNewItem->iTextureID != -1)
	{	
		if (!pNewItem->bBlend)
			glEnable(GL_BLEND);
		else
		{
			if ( (pNewItem->eDestMode != 0xffffffff) && (pNewItem->eSrcMode != 0xffffffff) )
				glBlendFunc(GL_SRC_ALPHA,GL_ONE_MINUS_SRC_ALPHA);
		}

		if (pNewItem->eTexMode != GL_MODULATE)
			glTexEnvf(GL_TEXTURE_ENV, GL_TEXTURE_ENV_MODE, GL_MODULATE);

		if (pNewItem->eTCGenMode != eNone)
		{			
			glMatrixMode(GL_TEXTURE);
			glPopMatrix();
			glMatrixMode(GL_MODELVIEW);

			glDisable(GL_TEXTURE_GEN_S);
			glDisable(GL_TEXTURE_GEN_T);
		}
	}
	else
		glEnable(GL_TEXTURE_2D);

	if (!pNewItem->bDepthWrites)
		glDepthMask(1);

	if (pNewItem->eDepthFunc != 0xffffffff)
		glDepthFunc(GL_LEQUAL);

	if (pNewItem->eCullMode != eCullBack )
	{
		glEnable(GL_CULL_FACE);
		glCullFace(GL_BACK);
	}
}

void CMeshSkin::SetPass ( int iPassID )
{
	trSkinTexureItem	*pNewItem = GetPass(iPassID);

	if (!pNewItem)
		return;

	if ((pNewItem->iTextureID == -1) && pNewItem->sTexture.size())
		pNewItem->iTextureID = CTextureManager::instance().GetID((char*)pNewItem->sTexture.c_str());

	if (pNewItem->iTextureID != -1)
	{	
		if (!pNewItem->bBlend)
			glDisable(GL_BLEND);
		else
		{
			if ( (pNewItem->eDestMode != 0xffffffff) && (pNewItem->eSrcMode != 0xffffffff) )
				glBlendFunc(pNewItem->eSrcMode, pNewItem->eDestMode );
		}

		if (pNewItem->eTexMode != GL_MODULATE)
			glTexEnvi(GL_TEXTURE_ENV, GL_TEXTURE_ENV_MODE, pNewItem->eTexMode);

		if (pNewItem->eTCGenMode != eNone)
		{
			GLint	iMode;

			switch (pNewItem->eTCGenMode)
			{
				case eEnviron:
					iMode = GL_SPHERE_MAP;
				break;

				case eLiniar:
					iMode = GL_LINEAR;
				break;

				case eEyeLinar:
					iMode = GL_EYE_LINEAR;
				default:
				break;
			}
			
			glMatrixMode(GL_TEXTURE);
			glPushMatrix();
			glScalef(5.0f,5,1);
			glMatrixMode(GL_MODELVIEW);

			glTexGeniv(GL_S, GL_TEXTURE_GEN_MODE, &iMode);
			glTexGeniv(GL_T, GL_TEXTURE_GEN_MODE, &iMode);
			glEnable(GL_TEXTURE_GEN_S);
			glEnable(GL_TEXTURE_GEN_T);
		}

		CTextureManager::instance().Bind(pNewItem->iTextureID);
	}
	else
		glDisable(GL_TEXTURE_2D);

	if (!pNewItem->bDepthWrites)
		glDepthMask(0);

	if (pNewItem->eDepthFunc != 0xffffffff)
		glDepthFunc(pNewItem->eDepthFunc);

	if (pNewItem->eCullMode != eCullBack )
	{
		switch(pNewItem->eCullMode)
		{
			case eNoCull:
				glDisable(GL_CULL_FACE);
				break;
			case eCullFront:
				glCullFace(GL_FRONT);
				break;
			case eDoubleSided:
				glCullFace(GL_FRONT_AND_BACK);
			default:
				break;
		}
	}
}

bool CMeshSkin::PassUsesTCs ( int iPassID )
{
	trSkinTexureItem	*pNewItem = GetPass(iPassID);

	if (!pNewItem)
		return false;

	return pNewItem->eTCGenMode == eNone;
}

bool CMeshSkin::PassUsesFaceNorms ( int iPassID )
{
	trSkinTexureItem	*pNewItem = GetPass(iPassID);

	if (!pNewItem)
		return false;

	return pNewItem->bUseFaceNorms;
}
//#######################CSkin###################################

CSkin::CSkin()
{
	m_sName.empty();
	m_vMeshSkins.clear();
}

CSkin::~CSkin()
{
	m_sName.empty();
	m_vMeshSkins.clear();
}

int CSkin::GetNumMeshes ( void )
{
	return (int)m_vMeshSkins.size();
}

CMeshSkin* CSkin::GetMesh ( int iMeshID )
{
	if ( (iMeshID < 0) || (iMeshID >= (int) m_vMeshSkins.size()) )
		return NULL;

	return &(m_vMeshSkins[iMeshID]);
}

int CSkin::NewMesh( void )
{
	CMeshSkin	rNewMesh;

	m_vMeshSkins.push_back(rNewMesh);

	return (int)m_vMeshSkins.size()-1;
}

void CSkin::RemoveMesh ( int iMeshID )
{
	if ( (iMeshID < 0) || (iMeshID >= (int) m_vMeshSkins.size()) )
		return;

	m_vMeshSkins.erase(m_vMeshSkins.begin() + iMeshID);
}

void CSkin::Init( int iNumMeshes )
{
	for (int i = 0; i < iNumMeshes; i ++)
		NewMesh();
}


//*********************************CAnimModel***************************
CAnimModel::CAnimModel()
{
	m_iEnvironTexture  = -1;
	m_rScale.x = m_rScale.y = m_rScale.z = 1.0f;
	m_rOffset.x = m_rOffset.y = m_rOffset.z = 0.0f;
	m_rRotation.x = m_rRotation.y = m_rRotation.z = 0.0f;
	m_vSkins.clear();

	m_rAddScale.x = 1;
	m_rAddScale.y = 1;
	m_rAddScale.z = 1;
}

CAnimModel::~CAnimModel()
{

}
//*********************FILE IO*****************

bool CAnimModel::LoadA3D ( const char * szFile )
{
	COSFile	oFile (szFile);

	if (!oFile.Open("rb"))
		return false;

	char	szTag[4] = {'A','3','D','1'};
	int		iValidate = *((int*)(szTag));
	int		iFileTag = 0;
	oFile.Read(&iFileTag,4);

	if (iFileTag != iValidate)
	{
		oFile.Close();
		return false;
	}

	// global object data
	oFile.Read(&m_rOffset,sizeof(trVertex3D));
	oFile.Read(&m_rRotation,sizeof(trVertex3D));
	oFile.Read(&m_rScale,sizeof(trVertex3D));

	char	szTemp[256];

	// seq data
	m_vSequences.clear();

	int iSeqs;// = m_vSequences.size();
	oFile.Read(&iSeqs,sizeof(int));

	trSequence	rSeq;

	for ( int iSeq = 0; iSeq < iSeqs; iSeq++)
	{
		oFile.Read(&rSeq.id,sizeof(unsigned int));
		
		oFile.Read(szTemp,256);
		rSeq.name = szTemp;

		oFile.Read(szTemp,256);
		rSeq.group = szTemp;

		oFile.Read(&(rSeq.start),sizeof(unsigned int));
		oFile.Read(&(rSeq.len),sizeof(unsigned int));
		oFile.Read(&(rSeq.speed),sizeof(float));
		
		m_vSequences.push_back(rSeq);
	}

	// events data
	m_vEvents.clear();

	int iEvents;// = m_vEvents.size();
	oFile.Read(&iEvents,sizeof(int));

	trEvent	rEvent;
	int		iEventSeq;

	for ( int iEvent = 0; iEvent < iEvents; iEvent++)
	{
		oFile.Read(szTemp,256);
		rEvent.name = szTemp;

		oFile.Read(&iEventSeq,sizeof(unsigned int));
		rEvent.seq = &(m_vSequences[iEventSeq]);

		oFile.Read(&(rEvent.time),sizeof(float));
		
		m_vEvents.push_back(rEvent);
	}

	// tag data
	m_vTags.clear();

	int iTags;// = m_vTags.size();
	oFile.Read(&iTags,sizeof(int));

	trTag	rTag;

	for ( int iTag = 0; iTag < iTags; iTag++)
	{
		oFile.Read(szTemp,256);
		rTag.name = szTemp;

		oFile.Read(&(rTag.tagID),sizeof(int));
		oFile.Read(&(rTag.tagmateID),sizeof(int));
		
		m_vTags.push_back(rTag);
	}

	// frame data
	m_vFrames.clear();

	int iFrames;// = m_vFrames.size();
	oFile.Read(&iFrames,sizeof(int));

	int iVerts;// = m_vFrames.size();
	oFile.Read(&iVerts,sizeof(int));
	
	trAnimFrame				rFrame;
	trVertex3D				rVertex;
	trTagCS					rTagCS;

	for ( int iFrame = 0; iFrame < iFrames; iFrame++)
	{
		rFrame.vVerts.clear();
		rFrame.vTagCSs.clear();

		for ( int iVert = 0; iVert < iVerts; iVert++)
		{
			oFile.Read(&rVertex,sizeof(trVertex3D));
			rFrame.vVerts.push_back(rVertex);
		}

		// then the TagCSList
		for ( int iTag = 0; iTag < iTags; iTag++)
		{
			oFile.Read(&rTagCS,sizeof(trTagCS));
			rFrame.vTagCSs.push_back(rTagCS);
		}
		m_vFrames.push_back(rFrame);
	}

	// mesh data
	m_vMeshes.clear();

	int	iMeshes;// = m_vMeshes.size();
	oFile.Read(&iMeshes,sizeof(int));

	CMesh				oMesh;
	trIndexedFace		rFace;
	tvFaceNormList		vNormList;
	int					iFaces;
	trFaceVertNorm		vFaceNorm;

	oMesh.m_pModel = this;

	for ( int iMesh = 0; iMesh < iMeshes; iMesh++)
	{
		oMesh.m_vFaces.clear();
		oMesh.m_vFaceNormalFrames.clear();
		oMesh.m_rScale = m_rScale;

		oFile.Read(szTemp,256);
		oMesh.m_sName = szTemp;

		iFaces = 0;
		oFile.Read(&iFaces,sizeof(int));

		for ( int iFace = 0; iFace < iFaces; iFace++)
		{
			oFile.Read(&(rFace.aVerts),sizeof(trIndecedVert)*3);
			oMesh.m_vFaces.push_back(rFace);
		}

		for ( int iFrame = 0; iFrame < iFrames; iFrame++)
		{
			vNormList.clear();
			
			for ( int iFace = 0; iFace < iFaces; iFace++)	// one per face
			{
				oFile.Read(&(vFaceNorm),sizeof(trFaceVertNorm));
				vNormList.push_back(vFaceNorm);
			}
			oMesh.m_vFaceNormalFrames.push_back(vNormList);
		}
		m_vMeshes.push_back(oMesh);
	}
	oFile.Close();

	return true;
}

bool CAnimModel::LoadU3D ( const char *szPath, const char * szFile )
{
	COSDir oDir (szPath);

	if (!szFile)
		return false;

	char	szAName[255];
	char	szDName[255];
	char	szFileToOpen[255];
	char	szFileName[255];
	COSFile	oFile;

	short 				iFrames;
	unsigned short		iFrameSize;

	strcpy (szFileName,oDir.GetStdName());
	strcat (szFileName,"/");
	strcat (szFileName,szFile);

	strcpy (szFileToOpen,szFileName);
	strcat (szFileToOpen,".uc");

	// load UC file
	LoadUCFile(szFileToOpen,szAName,szDName);

	strcpy (szFileToOpen,oDir.GetStdName());
	strcat(szFileToOpen,"/");
	strcat(szFileToOpen,szDName);

	oFile.StdName(szFileToOpen);

	int iMeshes = -1;

	bool		bGotWepTag = false;
	trTag		rTagInfo;
	trTagCS		rTagCS;
	int			aTagIndecies[3];

	trU3DDataHeader	rDataHeader;
	trU3DTriangle	*pTris = NULL;
	// read in the tris
	if (oFile.Open("rb"))
	{
		oFile.Read(&rDataHeader,sizeof(trU3DDataHeader));

		pTris = (trU3DTriangle*)malloc(sizeof(trU3DTriangle)*rDataHeader.iNumPolys);

		for (int p = 0; p < rDataHeader.iNumPolys; p++)
		{
			oFile.Read(&pTris[p],sizeof(trU3DTriangle));

			if (pTris[p].cTextureID > iMeshes)
				iMeshes = pTris[p].cTextureID;
		}
		oFile.Close();
	}
	else
		return false;

	// IDs are 0 based, this gives us a count
	iMeshes = iMeshes+1;

	// make the real triangles
	// we make a mesh for each texture

	char	szTemp[512];
	CMesh	oMesh;

	oMesh.m_pModel = this;

	trIndexedFace	rFace;
	trIndecedVert	rIndexedVert;

	for ( int iMesh = 0; iMesh < iMeshes; iMesh++)
	{
		sprintf(szTemp,"UTMesh-%d",iMesh);
		oMesh.m_sName = szTemp;
		oMesh.m_vFaces.clear();
		oMesh.m_vFaceNormalFrames.clear();

		for ( int iFaces = 0; iFaces < rDataHeader.iNumPolys; iFaces++)
		{
			if (pTris[iFaces].cFlags != eTagTri)
			{
				if (pTris[iFaces].cTextureID == iMesh)
				{
					for ( int i = 0; i < 3; i++)
					{
						rIndexedVert.index = pTris[iFaces].aVerts[i];

						rIndexedVert.tc.s = pTris[iFaces].aRawUVs[i][0] * 0.00390625f;
						rIndexedVert.tc.t = 1.0f - pTris[iFaces].aRawUVs[i][1] * 0.00390625f;

						rFace.aVerts[i] = rIndexedVert;
					}

					if ( pTris[iFaces].cType == eTagTri )
					{
						if (!bGotWepTag)
						{
							aTagIndecies[0] = pTris[iFaces].aVerts[0];
							aTagIndecies[1] = pTris[iFaces].aVerts[1];
							aTagIndecies[2] = pTris[iFaces].aVerts[2];

							rTagInfo.name = "UTWepTag";
							rTagInfo.tagID = 0;
							rTagInfo.tagmateID = -1;
							bGotWepTag = true;
						}
					}
					else
					{
						if ( Is2Sided(pTris[iFaces].cType) )
						{
							oMesh.m_vFaces.push_back(rFace);

							rIndexedVert = rFace.aVerts[0];
							rFace.aVerts[0] = rFace.aVerts[2];
							rFace.aVerts[2] = rIndexedVert;
						}
					}

					oMesh.m_vFaces.push_back(rFace);
				}
			}
		}
		oMesh.m_rScale = m_rScale;
		m_vMeshes.push_back(oMesh);
	}

	free(pTris);

	strcpy (szFileToOpen,oDir.GetStdName());
	strcat(szFileToOpen,"/");
	strcat(szFileToOpen,szAName);

	oFile.StdName(szFileToOpen);

	unsigned int iIntVert;
	trVertex3D	rVert;
	trAnimFrame	rFrame;

	if (oFile.Open("rb"))
	{
		oFile.Read(&iFrames,2,1);
		oFile.Read(&iFrameSize,2,1);

		for ( int f = 0;f< iFrames; f++)
		{
			rFrame.vTagCSs.clear();
			rFrame.vVerts.clear();

			for (int v = 0;v< rDataHeader.iNumVerts; v++)
			{
				oFile.Read(&iIntVert,4);

				GetIntVert(iIntVert,rVert);
				rFrame.vVerts.push_back(rVert);
			}
			m_vFrames.push_back(rFrame);
		}
		oFile.Close();
	}
	else
		return false;

	// now for the post pass

	trVertex3D	v1,v2,norm,p0,p1,p2;
	tvFaceNormList	vNormList;
	trFaceVertNorm	vFaceNorm;

	tvMeshList::iterator	iMeshItr = m_vMeshes.begin();
	tvIndexedFaceList::iterator	iFaceItr;

	tvFrameList::iterator	iFrameItr;

	while (iMeshItr != m_vMeshes.end())
	{
		iMeshItr->m_vFaceNormalFrames.clear();

		iFrameItr = m_vFrames.begin();
		
		while (iFrameItr != m_vFrames.end())
		{
			iFrameItr->vTagCSs.clear();

			vNormList.clear();
			iFaceItr = iMeshItr->m_vFaces.begin();

			while (iFaceItr != iMeshItr->m_vFaces.end())
			{
				p0 = iFrameItr->vVerts[iFaceItr->aVerts[0].index];
				p1 = iFrameItr->vVerts[iFaceItr->aVerts[1].index];
				p2 = iFrameItr->vVerts[iFaceItr->aVerts[2].index];

				v1 = p1-p0;
				set_mag(v1,1.0f);

				v2 = p2-p0;
				set_mag(v2,1.0f);

				cross(v1,v2,norm);

				norm.x *= -1;
				set_mag(norm,1.0f);

				vFaceNorm.faceNorm = norm;
				vFaceNorm.norms[0] = norm;
				vFaceNorm.norms[1] = norm;
				vFaceNorm.norms[2] = norm;

				vNormList.push_back(vFaceNorm);
				iFaceItr++;
			}
			iMeshItr->m_vFaceNormalFrames.push_back(vNormList);

			// now get the tag info for the frame
			if (bGotWepTag)
			{
				p0 = iFrameItr->vVerts[aTagIndecies[0]];
				p1 = iFrameItr->vVerts[aTagIndecies[1]];
				p2 = iFrameItr->vVerts[aTagIndecies[2]];
				p0 *= m_rScale;

				rTagCS.o = p0;

				p1 *= m_rScale;
				rTagCS.v = p1 - p0;
				set_mag(rTagCS.v,1.0f);

				p2 *= m_rScale;
				rTagCS.d = p2 - p0;
				set_mag(rTagCS.d,1.0f);

				cross(rTagCS.v,rTagCS.d,rTagCS.h);
				set_mag(rTagCS.h,1.0f);

				cross(rTagCS.h,rTagCS.v,rTagCS.d);
				set_mag(rTagCS.d,1.0f);

				iFrameItr->vTagCSs.push_back(rTagCS);
			
			}
			iFrameItr++;
		}
		iMeshItr++;
	}
	return true;
}

bool CAnimModel::LoadMD3 ( const char * szFile )
{	
	if (!szFile)
		return false;
	
	COSFile	oFile (szFile);

	if (!oFile.Open("rb"))
		return false;
	
	m_rScale.x = m_rScale.y = m_rScale.z = 0.01f;
	m_rOffset.x = m_rOffset.y = m_rOffset.z = 0;
	m_rRotation.x = m_rRotation.y = m_rRotation.z = 0;

	int	iThisItem = 0;

	// file header
	tMD3FileHeader	rFileHeader;

	oFile.Read(&rFileHeader.id,4);
	oFile.Read(&rFileHeader.vers,4);
	oFile.Read(rFileHeader.fileName,68);
	oFile.Read(&rFileHeader.numBoneFrames,4);
	oFile.Read(&rFileHeader.numTags,4);
	oFile.Read(&rFileHeader.numMeshes,4);
	oFile.Read(&rFileHeader.numSkins,4);
	oFile.Read(&rFileHeader.headerLen,4);
	oFile.Read(&rFileHeader.tagStart,4);
	oFile.Read(&rFileHeader.tagEnd,4);
	oFile.Read(&rFileHeader.fileSize,4);

	// bone frames
	tMD3BoneFrame		rBoneFrame;
	
	std::vector<tMD3BoneFrame>	vBoneList;

	for (iThisItem = 0; iThisItem < rFileHeader.numBoneFrames; iThisItem++)
	{
		oFile.Read(&rBoneFrame.mins,4*3);
		oFile.Read(&rBoneFrame.maxs,4*3);
		oFile.Read(&rBoneFrame.pos,4*3);
		oFile.Read(&rBoneFrame.scale,4);
		oFile.Read(rBoneFrame.creator,16);
		vBoneList.push_back(rBoneFrame);
	}
		
	m_vFrames.clear();
	m_vTags.clear();

	trAnimFrame	rFrame;
	// tags
	oFile.Seek(eFileStart,rFileHeader.tagStart);

	trTag			rTag;
	trTagCS			rTagCS;

	tMD3Tag			rMD3Tag;
	int				iTag = 0;
	for (iThisItem = 0; iThisItem < rFileHeader.numBoneFrames; iThisItem++)
	{
		rFrame.vTagCSs.clear();
		rFrame.vVerts.clear();
		for (iTag = 0; iTag < rFileHeader.numTags; iTag++)
		{
			oFile.Read(&rMD3Tag.name,64);
			oFile.Read(&rMD3Tag.pos,3*4);
			oFile.Read(&rMD3Tag.rawMatrix,9*4);
			rMD3Tag.id = iTag;

			if (iThisItem == 0)
			{
				rTag.name = rMD3Tag.name;
				rTag.tagID = iTag;
				rTag.tagmateID = false;

				m_vTags.push_back(rTag);
			}

			rTagCS.o= rMD3Tag.pos;

			rTagCS.h.x = rMD3Tag.rawMatrix[0];
			rTagCS.h.y = rMD3Tag.rawMatrix[1];
			rTagCS.h.z = rMD3Tag.rawMatrix[2];

			rTagCS.v.x = rMD3Tag.rawMatrix[3];
			rTagCS.v.y = rMD3Tag.rawMatrix[4];
			rTagCS.v.z = rMD3Tag.rawMatrix[5];

			rTagCS.d.x = rMD3Tag.rawMatrix[6];
			rTagCS.d.y = rMD3Tag.rawMatrix[7];
			rTagCS.d.z = rMD3Tag.rawMatrix[8];

			rFrame.vTagCSs.push_back(rTagCS);
		}
		m_vFrames.push_back(rFrame);
	}
	oFile.Seek(eFileStart,rFileHeader.tagEnd);

	int				iThisMeshMark = rFileHeader.tagEnd;

	tMD3MeshHeader	rMeshHeader;
	int				iFrames;
	int				iTemp;
	tMD3String		rString;
	tMD3IndexedTri	rTri;
	tMD3uv			rUV;
	tMD3iVertex		rVert;

	trVertex3D		rVertex;

	CMesh			oMesh;

	trIndexedFace				rFace;
	std::vector<std::string>	vSkinList;
	std::string					str;

	std::vector<tMD3IndexedTri>	vTempTris;
	std::vector<tMD3uv>			vTempUVs;
	m_vMeshes.clear();

	oMesh.m_pModel = this;
	oMesh.m_rScale = m_rScale;

	int		iThisVert = 0;

	for (iThisItem = 0; iThisItem < rFileHeader.numMeshes; iThisItem++)
	{
		oFile.Read(&rMeshHeader.id,4);
		oFile.Read(&rMeshHeader.name,68);
		oFile.Read(&rMeshHeader.numMeshFrames,4);
		oFile.Read(&rMeshHeader.numSkins,4);
		oFile.Read(&rMeshHeader.numVerts,4);
		oFile.Read(&rMeshHeader.numTris,4);
		oFile.Read(&rMeshHeader.triStart,4);
		oFile.Read(&rMeshHeader.headerSize,4);
		oFile.Read(&rMeshHeader.TXvertStart,4);
		oFile.Read(&rMeshHeader.vertStart,4);
		oFile.Read(&rMeshHeader.meshSize,4);
		
		oMesh.m_sName = rMeshHeader.name.str;
		oMesh.m_vFaceNormalFrames.clear();
		oMesh.m_vFaces.clear();

		vSkinList.clear();
		// skins
		oFile.Seek(eFileStart,iThisMeshMark+rMeshHeader.headerSize);
		for (iTemp = 0; iTemp < rMeshHeader.numSkins; iTemp++)
		{
			oFile.Read(&rString.str,68);
			str = rString.str;
			vSkinList.push_back(str);
		}
		// tris
		vTempTris.clear();

		oFile.Seek(eFileStart,iThisMeshMark+rMeshHeader.triStart);
		for (iTemp = 0; iTemp<rMeshHeader.numTris; iTemp++)
		{
			oFile.Read(&rTri,4*3);
			vTempTris.push_back(rTri);
		}

		// UVs
		vTempUVs.clear();

		oFile.Seek(eFileStart,iThisMeshMark+rMeshHeader.TXvertStart);
		for (iTemp = 0; iTemp <rMeshHeader.numVerts; iTemp++)
		{
			oFile.Read(&rUV,4*2);
			vTempUVs.push_back(rUV);
		}

		oMesh.m_vFaceNormalFrames.clear();
		oMesh.m_vFaces.clear();

		// make the real indexed 
		for (iTemp = 0; iTemp<rMeshHeader.numTris; iTemp++)
		{	
			rFace.aVerts[0].index = vTempTris[iTemp].c+iThisVert;
			rFace.aVerts[0].tc.s = vTempUVs[vTempTris[iTemp].c].u;
			rFace.aVerts[0].tc.t = 1.0f-vTempUVs[vTempTris[iTemp].c].v;

			rFace.aVerts[1].index = vTempTris[iTemp].b+iThisVert;
			rFace.aVerts[1].tc.s = vTempUVs[vTempTris[iTemp].b].u;
			rFace.aVerts[1].tc.t = 1.0f-vTempUVs[vTempTris[iTemp].b].v;

			rFace.aVerts[2].index = vTempTris[iTemp].a+iThisVert;
			rFace.aVerts[2].tc.s = vTempUVs[vTempTris[iTemp].a].u;
			rFace.aVerts[2].tc.t = 1.0f-vTempUVs[vTempTris[iTemp].a].v;

			oMesh.m_vFaces.push_back(rFace);
		}	
		
		// move pointer to Verts
		oFile.Seek(eFileStart,iThisMeshMark+rMeshHeader.vertStart);

		for (iFrames = 0; iFrames <rMeshHeader.numMeshFrames; iFrames++)
		{
			for (iTemp = 0; iTemp < rMeshHeader.numVerts; iTemp++)
			{
				oFile.Read(&rVert.x,2);
				oFile.Read(&rVert.y,2);
				oFile.Read(&rVert.z,2);

				oFile.Read(&rVert.u,1);
				oFile.Read(&rVert.v,1);
				
				rVertex.x = rVert.x;
				rVertex.y = rVert.y;
				rVertex.z = rVert.z;

				m_vFrames[iFrames].vVerts.push_back(rVertex);
			}
		}
		
		m_vMeshes.push_back(oMesh);

		iThisVert += rMeshHeader.numVerts;

		if (iThisItem < rFileHeader.numMeshes )
		{
			oFile.Seek(eFileStart,iThisMeshMark+rMeshHeader.meshSize);
			iThisMeshMark += rMeshHeader.meshSize;
		}
	}
	oFile.Close();

	tvIndexedFaceList::iterator	iFaceItr;
	tvMeshList::iterator		iMeshItr = m_vMeshes.begin();
	tvFrameList::iterator		iFrameItr = m_vFrames.begin();
	tvFaceNormList				vNormList;
	trFaceVertNorm				vNorm;

	trVertex3D					p0,p1,p2,v1,v2,norm;

	while (iFrameItr != m_vFrames.end())
	{
		iMeshItr = m_vMeshes.begin();
		while ( iMeshItr != m_vMeshes.end() )
		{
			vNormList.clear();

			iFaceItr = iMeshItr->m_vFaces.begin();
			while (iFaceItr != iMeshItr->m_vFaces.end())
			{
				p0 = iFrameItr->vVerts[iFaceItr->aVerts[0].index];
				p1 = iFrameItr->vVerts[iFaceItr->aVerts[1].index];
				p2 = iFrameItr->vVerts[iFaceItr->aVerts[2].index];

				v1 = p1-p0;
				v2 = p2-p0;
				
				cross(v1,v2,norm);
				norm.x *= -1;
				set_mag(norm,1.0f);

				vNorm.faceNorm = norm;
				vNorm.norms[0] = vNorm.norms[1] = vNorm.norms[2] = vNorm.faceNorm;
				
				vNormList.push_back(vNorm);
				iFaceItr++;
			}
			iMeshItr->m_vFaceNormalFrames.push_back(vNormList);
			iMeshItr++;
		}
		iFrameItr++;
	}
	return true;
}

bool CAnimModel::LoadOBJ ( const char * szFile )
{
	if (!szFile)
		return false;
	
	COSFile	oFile (szFile);

	if (!oFile.Open("rt"))
		return false;

	m_vMeshes.clear();
	m_vTags.clear();
	m_vFrames.clear();

	m_rScale.x = m_rScale.y = m_rScale.z = 1.0f;
	m_rOffset.x = m_rOffset.y = m_rOffset.z = 0;
	m_rRotation.x = m_rRotation.y = m_rRotation.z = 0;

	char	szLine[1024];

	trVertex3D	rVert,p0,p1,p2,norm,v1,v2;
	trVertex2D	rTC;
	trVertex3D	rNorm;

	int			aTCIndecies[3];
	int			aNormIndecies[3];

	std::vector<trVertex3D>	vNormList;
	std::vector<trVertex2D>	vTCList;

	trFaceVertNorm	rFaceNorm;
	trIndexedFace	rFace;

	tvFaceNormList	rNormList;

	trAnimFrame	rFrame;
	CMesh		oMesh;
	char		szTemp[128];

	oMesh.m_pModel = this;
	oMesh.m_rScale = m_rScale;

	szLine[0] = '\0';
	bool bDone = GetLine2(szLine,oFile);
	
	while (!bDone)
	{
		switch(szLine[0])
		{
			case 'v':
			case 'V':
				switch(szLine[1])
				{
					case 't':
					case 'T':
						sscanf(szLine,"%s %f %f",szTemp,&rTC.s,&rTC.t);
						rTC.t = 1.0f - rTC.t;
						vTCList.push_back(rTC);
					break;

					case 'n':
					case 'N':
						rNorm.x = rNorm.y = rNorm.z = 0;
						sscanf(szLine,"%s %f %f %f",szTemp,&rNorm.x,&rNorm.y,&rNorm.z);
						//rnorm *= -1;
						set_mag(rNorm,1.0f);
						vNormList.push_back(rNorm);
					break;
					
					default:
						sscanf(szLine,"%s %f %f %f",szTemp,&rVert.x,&rVert.y,&rVert.z);
						rFrame.vVerts.push_back(rVert);
					break;
				}
			break;

			case 'g':
			case 'G':
				if (oMesh.m_vFaces.size() > 0)
				{
					oMesh.m_vFaceNormalFrames.push_back(rNormList);
					m_vMeshes.push_back(oMesh);

					oMesh.m_vFaces.clear();
					rNormList.clear();
				}

				sscanf(szLine,"%s %s",szTemp,szTemp);
				oMesh.m_sName = szTemp;
			break;

			case 'f':
			case 'F':
				rFace.aVerts[0].index = rFace.aVerts[1].index = rFace.aVerts[2].index = 0;
				aTCIndecies[0] = aTCIndecies[1] = aTCIndecies[2] = 0;
				aNormIndecies[0] = aNormIndecies[1] = aNormIndecies[2] = 0;

				sscanf(szLine,"%s %d/%d/%d %d/%d/%d %d/%d/%d",szTemp,
					&rFace.aVerts[0].index,&aTCIndecies[0],&aNormIndecies[0],
					&rFace.aVerts[1].index,&aTCIndecies[1],&aNormIndecies[1],
					&rFace.aVerts[2].index,&aTCIndecies[2],&aNormIndecies[2]);

				rFace.aVerts[0].index--;
				rFace.aVerts[1].index--;
				rFace.aVerts[2].index--;

				rFace.aVerts[0].tc = vTCList[aTCIndecies[0]-1];
				rFace.aVerts[1].tc = vTCList[aTCIndecies[1]-1];
				rFace.aVerts[2].tc = vTCList[aTCIndecies[2]-1];

				rFaceNorm.norms[0] = vNormList[aNormIndecies[0]-1];
				rFaceNorm.norms[1] = vNormList[aNormIndecies[1]-1];
				rFaceNorm.norms[2] = vNormList[aNormIndecies[2]-1];


				p0 = rFrame.vVerts[rFace.aVerts[0].index];
				p1 = rFrame.vVerts[rFace.aVerts[1].index];
				p2 = rFrame.vVerts[rFace.aVerts[2].index];

				v1 = p1-p0;
				set_mag(v1,1.0f);

				v2 = p2-p0;
				set_mag(v2,1.0f);

				cross(v1,v2,norm);

				norm.x *= -1;
				set_mag(norm,1.0f);

				rFaceNorm.faceNorm = norm;

				rNormList.push_back(rFaceNorm);
				oMesh.m_vFaces.push_back(rFace);
			break;
		}
		szLine[0] = '\0';
		bDone = GetLine2(szLine,oFile);
	}

	m_vFrames.push_back(rFrame);
	oMesh.m_vFaceNormalFrames.push_back(rNormList);
	m_vMeshes.push_back(oMesh);

	oFile.Close();

	return true;
}

bool CAnimModel::LoadBZG ( const char * szFile )
{
	return false;
}

bool CAnimModel::SaveA3D ( const char * szFile )
{
	COSFile	oFile (szFile);

	if (!oFile.Open("wb"))
		return false;

	char	szTag[4] = {'A','3','D','1'};
	oFile.Write(szTag,4);

	// global object data
	oFile.Write(&m_rOffset,sizeof(trVertex3D));
	oFile.Write(&m_rRotation,sizeof(trVertex3D));
	oFile.Write(&m_rScale,sizeof(trVertex3D));

	// seq data
	int iSeqs = (int)m_vSequences.size();
	oFile.Write(&iSeqs,sizeof(int));

	tvSequenceListList::iterator seqItr = m_vSequences.begin();

	char	szTemp[256];

	while (seqItr != m_vSequences.end())
	{
		oFile.Write(&(seqItr->id),sizeof(unsigned int));
		
		strcpy(szTemp,seqItr->name.c_str());
		oFile.Write(szTemp,256);

		strcpy(szTemp,seqItr->group.c_str());
		oFile.Write(szTemp,256);

		oFile.Write(&(seqItr->start),sizeof(unsigned int));
		oFile.Write(&(seqItr->len),sizeof(unsigned int));
		oFile.Write(&(seqItr->speed),sizeof(float));
		seqItr++;
	}

	// events data
	int iEvents = (int)m_vEvents.size();
	oFile.Write(&iEvents,sizeof(int));

	tvEventListList::iterator eventItr = m_vEvents.begin();

	while (eventItr != m_vEvents.end())
	{
		strcpy(szTemp,eventItr->name.c_str());
		oFile.Write(szTemp,256);

		oFile.Write(&(eventItr->seq->id),sizeof(unsigned int));
		oFile.Write(&(eventItr->time),sizeof(float));
		eventItr++;
	}

	// tag data
	int iTags = (int)m_vTags.size();
	oFile.Write(&iTags,sizeof(int));

	tvTagList::iterator tagItr = m_vTags.begin();

	while (tagItr != m_vTags.end())
	{
		strcpy(szTemp,tagItr->name.c_str());
		oFile.Write(szTemp,256);

		oFile.Write(&(tagItr->tagID),sizeof(int));
		oFile.Write(&(tagItr->tagmateID),sizeof(int));
		tagItr++;
	}

	// frame data
	int iFrames = (int)m_vFrames.size();
	oFile.Write(&iFrames,sizeof(int));

	int iVerts = (int)m_vFrames[0].vVerts.size();
	oFile.Write(&iVerts,sizeof(int));

	tvFrameList::iterator	frameItr = m_vFrames.begin();
	tvVertexList::iterator	vertItr;
	tvTagCSList::iterator	tagCSItr;

	while (frameItr != m_vFrames.end())
	{
		vertItr = frameItr->vVerts.begin();
		while (vertItr != frameItr->vVerts.end())
		{
			oFile.Write(&(*vertItr),sizeof(trVertex3D));
			vertItr++;
		}
		// then the TagCSList

		tagCSItr = frameItr->vTagCSs.begin();

		while (tagCSItr != frameItr->vTagCSs.end())
		{
			oFile.Write(&(*tagCSItr),sizeof(trTagCS));
			tagCSItr++;
		}
		frameItr++;
	}

	int	iMeshes = (int)m_vMeshes.size();
	oFile.Write(&iMeshes,sizeof(int));

	int	iFaces;

	tvMeshList::iterator				meshItr = m_vMeshes.begin();
	tvIndexedFaceList::iterator			faceItr;
	tvFaceNormFrameList::iterator		faceNormalItr;
	tvFaceNormList::iterator			normalItr;
	while (meshItr != m_vMeshes.end())
	{
		strcpy(szTemp,meshItr->m_sName.c_str());
		oFile.Write(szTemp,256);

		iFaces = (int)meshItr->m_vFaces.size();
		oFile.Write(&iFaces,sizeof(int));

		faceItr = meshItr->m_vFaces.begin();
		while ( faceItr != meshItr->m_vFaces.end() )
		{
			oFile.Write(&(faceItr->aVerts),sizeof(trIndecedVert)*3);
			faceItr++;
		}

		faceNormalItr = meshItr->m_vFaceNormalFrames.begin();
		
		while (faceNormalItr != meshItr->m_vFaceNormalFrames.end() )	// one per frame
		{
			normalItr = faceNormalItr->begin();
			
			while ( normalItr != faceNormalItr->end() )	// one per face
			{
				oFile.Write(&(*normalItr),sizeof(trFaceVertNorm));
				normalItr++;
			}
			faceNormalItr++;
		}

		meshItr++;
	}
	oFile.Close();

	return true;
}

bool CAnimModel::SaveBZG ( const char * szFile )
{
	return false;
}

int CAnimModel::GetMeshCount ( void )
{
	return (int)m_vMeshes.size();
}

int CAnimModel::GetMeshID ( const char *szName )
{
	if (!szName || (m_vMeshes.size()<1) )
		return -1;

	std::string	str = szName;
	tvMeshList::iterator	itr = m_vMeshes.begin();

	int i = 0;
	while (itr != m_vMeshes.end())
	{
		if (szName == itr->m_sName)
			return i;
		i++;
		itr++;
	}
	return -1;
}

CMesh* CAnimModel::GetMesh ( int iMeshID )
{
	if ( (iMeshID < 0) || (iMeshID >=(int) m_vMeshes.size()) )
		return NULL;

	return &(m_vMeshes[iMeshID]);
}

int CAnimModel::GetNumFrames ( void )
{
	return (int)m_vFrames.size();
}

trAnimFrame* CAnimModel::GetFrame ( int iFrame )
{
	if ( (iFrame < 0) || (iFrame >=(int) m_vFrames.size()) )
		return NULL;

	return &(m_vFrames[iFrame]);
}

int CAnimModel::GetNumSequences ( void )
{
	return 	(int)m_vSequences.size();
}

int CAnimModel::GetSequenceID ( const char *szName )
{
	if (!szName || (m_vSequences.size()<1) )
		return -1;

	std::string	str = szName;
	tvSequenceListList::iterator	itr = m_vSequences.begin();

	int i = 0;
	while (itr != m_vSequences.end())
	{
		if (szName == itr->name)
			return i;

		i++;
		itr++;
	}
	return -1;
}

trSequence* CAnimModel::GetSequence ( int iSeqID )
{
	if ( (iSeqID < 0) || (iSeqID >=(int) m_vSequences.size()) )
		return NULL;

	return &(m_vSequences[iSeqID]);
}

int CAnimModel::GetNumEvents ( void )
{
	return 	(int)m_vEvents.size();
}

int CAnimModel::GetEventID ( const char *szName )
{
	if (!szName || (m_vEvents.size()<1) )
		return -1;

	std::string	str = szName;
	tvEventListList::iterator	itr = m_vEvents.begin();

	int i = 0;
	while (itr != m_vEvents.end())
	{
		if (szName == itr->name)
			return i;

		i++;
		itr++;
	}
	return -1;
}

trEvent* CAnimModel::GetEvent ( int iEventID )
{
	if ( (iEventID < 0) || (iEventID >=(int) m_vEvents.size()) )
		return NULL;

	return &(m_vEvents[iEventID]);
}

int CAnimModel::GetNumSkins ( void )
{
	return 	(int)m_vSkins.size();
}

int CAnimModel::GetSkinID ( const char *szName )
{
	if (!szName || (m_vSkins.size()<1) )
		return -1;

	std::string	str = szName;
	tvSkinList::iterator	itr = m_vSkins.begin();

	int i = 0;
	while (itr != m_vSkins.end())
	{
		if (szName == itr->GetName())
			return i;

		i++;
		itr++;
	}
	return -1;
}

CSkin* CAnimModel::GetSkin ( int iSkinID )
{
	if ( (iSkinID < 0) || (iSkinID >=(int) m_vSkins.size()) )
		return NULL;

	return &(m_vSkins[iSkinID]);
}

int CAnimModel::NewSimpleSkin ( const char *szTextureName ) // makes a skin for all meshes that has the set textures
{
	int iSkin = NewSkin();

	int iMeshes = m_vSkins[iSkin].GetNumMeshes();

	for (int i = 0; i < iMeshes; i++)
		m_vSkins[iSkin].GetMesh(i)->NewSimplePass(szTextureName);

	return iSkin;
}

int CAnimModel::NewSimpleSkin ( std::vector<std::string> &textures  )
{
	int iSkin = NewSkin();

	int iMeshes = m_vSkins[iSkin].GetNumMeshes();

	for (int i = 0; i < iMeshes; i++)
		m_vSkins[iSkin].GetMesh(i)->NewSimplePass(textures[i].c_str());

	return iSkin;
}

int CAnimModel::NewSkin ( void )
{
	CSkin	rSkin;

	rSkin.Init(GetMeshCount());

	return AddSkin(rSkin);
}

int CAnimModel::AddSkin ( CSkin &rSkin )
{
	m_vSkins.push_back(rSkin);
	return (int)m_vSkins.size()-1;
}

bool CAnimModel::RemoveSkin ( int iID )
{
	return false;
}

bool CAnimModel::LoadSkin ( const char *szFile )
{
	return false;
}

void WriteSkinPassInfo ( FILE *fp, trSkinTexureItem *pItem )
{
/*	if (!pItem->bBlend)
		fprintf(fp,"#Blend: 0\n");

	if (!pItem->bDepthWrites)
		fprintf(fp,"#DepthWrites: 0\n");

	if (pItem->eDepthFunc != 0xffffffff)
	{
		fprintf(fp,"#DepthFunc:	");
		switch (pItem->eDepthFunc)
		{
			case GL_NEVER:
				fprintf(fp,"GL_NEVER");
				break;

			case GL_LESS: 
				fprintf(fp,"GL_LESS");
				break;

			case GL_LEQUAL:
				fprintf(fp,"GL_LEQUAL");
				break;

			case GL_EQUAL: 
				fprintf(fp,"GL_EQUAL");
				break;

			case GL_GREATER:
				fprintf(fp,"GL_GREATER");
				break;

			case GL_NOTEQUAL: 
				fprintf(fp,"GL_NOTEQUAL");
				break;

			case GL_GEQUAL: 
				fprintf(fp,"GL_GEQUAL");
				break;

			case GL_ALWAYS:
				fprintf(fp,"GL_ALWAYS");
			break;
		}
		fprintf(fp,"\n");

	}
	
	if (pItem->eCullMode != eNoCull)
	{
		fprintf(fp,"#CullMode:	");
		switch (pItem->eCullMode)
		{
			case eNoCull:
				fprintf(fp,"NO_CULL");
				break;

			case eCullBack:
				fprintf(fp,"CULL_BACK");
				break;

			case eCullFront:
				fprintf(fp,"CULL_FRONT");
				break;

			case eDoubleSided:
				fprintf(fp,"DOUBLE_SIDED");
				break;
		}
		fprintf(fp,"\n");
	}

	if ( (pItem->eDestMode != 0xffffffff) &&  (pItem->eSrcMode != 0xffffffff) )
	{
		fprintf(fp,"#BlendFunc:	");
		switch (pItem->eDestMode )
		{
			case GL_ZERO:
				fprintf(fp,"GL_ZERO");
				break;

			case GL_ONE:
				fprintf(fp,"GL_ONE");
				break;

			case GL_SRC_COLOR:
				fprintf(fp,"GL_SRC_COLOR");
				break;

			case GL_ONE_MINUS_SRC_COLOR:
				fprintf(fp,"GL_ONE_MINUS_SRC_COLOR");
				break;

			case GL_DST_COLOR:
				fprintf(fp,"GL_DST_COLOR");
				break;

			case GL_SRC_ALPHA:
				fprintf(fp,"GL_SRC_ALPHA");
				break;

			case GL_ONE_MINUS_SRC_ALPHA:
				fprintf(fp,"GL_ONE_MINUS_SRC_ALPHA");
				break;

			case GL_DST_ALPHA:
				fprintf(fp,"GL_DST_ALPHA");
				break;

			case GL_ONE_MINUS_DST_ALPHA:
				fprintf(fp,"GL_ONE_MINUS_DST_ALPHA");
				break;

			case GL_SRC_ALPHA_SATURATE:
				fprintf(fp,"GL_SRC_ALPHA_SATURATE");
				break;
		}
		fprintf(fp," ");
		switch (pItem->eSrcMode )
		{
			case GL_ZERO:
				fprintf(fp,"GL_ZERO");
				break;

			case GL_ONE:
				fprintf(fp,"GL_ONE");
				break;

			case GL_SRC_COLOR:
				fprintf(fp,"GL_SRC_COLOR");
				break;

			case GL_ONE_MINUS_SRC_COLOR:
				fprintf(fp,"GL_ONE_MINUS_SRC_COLOR");
				break;

			case GL_DST_COLOR:
				fprintf(fp,"GL_DST_COLOR");
				break;

			case GL_SRC_ALPHA:
				fprintf(fp,"GL_SRC_ALPHA");
				break;

			case GL_ONE_MINUS_SRC_ALPHA:
				fprintf(fp,"GL_ONE_MINUS_SRC_ALPHA");
				break;

			case GL_DST_ALPHA:
				fprintf(fp,"GL_DST_ALPHA");
				break;

			case GL_ONE_MINUS_DST_ALPHA:
				fprintf(fp,"GL_ONE_MINUS_DST_ALPHA");
				break;

			case GL_SRC_ALPHA_SATURATE:
				fprintf(fp,"GL_SRC_ALPHA_SATURATE");
				break;
		}
		fprintf(fp,"\n");
	}

	if (pItem->eTCGenMode != eNone)
	{
		fprintf(fp,"#TexGenMode:	");
		switch (pItem->eCullMode)
		{
			case eNone:
				fprintf(fp,"NONE");
				break;

			case eEnviron:
				fprintf(fp,"ENVIRON");
				break;

			case eLiniar:
				fprintf(fp,"LINEAR");
				break;

			case eEyeLinar:
				fprintf(fp,"EYE_LINEAR");
				break;
		}
		fprintf(fp,"\n");
	}

	if (pItem->eTexMode != 0xffffffff)
	{
		fprintf(fp,"#TexMode:	");
		switch (pItem->eTexMode)
		{
			case GL_MODULATE:
				fprintf(fp,"GL_MODULATE");
				break;

			case GL_DECAL:
				fprintf(fp,"GL_DECAL");
				break;
		}
		fprintf(fp,"\n");
	}

	if (pItem->sTexture.size()>0)
		fprintf(fp,"#Texture:	%s\n",pItem->sTexture.c_str()); */
}

bool CAnimModel::SaveSkin ( int iSkinID, const char *szFile )
{
/*	if ( !szFile )
		return false;

	CSkin	*pSkin = GetSkin(iSkinID);

	if (!pSkin)
		return false;

	FILE *fp = fopen (szFile, "wt");

	if (!fp)
		return false;

	fprintf(fp,"// Skin file for CAnimMeshClass/n#Name: %s\n#Meshes: %d\n",pSkin->GetName(),GetMeshCount());

	for ( int iMesh = 0; iMesh < GetMeshCount(); iMesh++)
	{
		CMesh	*pMesh = GetMesh(iMesh);
		CMeshSkin *pMeshSkin = pSkin->GetMesh(iMesh);
		trSkinTexureItem	*pItem;

		fprintf(fp,"#Mesh\n#ID: %d\n#Name: %s\n",iMesh,pMesh->m_sName.c_str());
		fprintf(fp,"#Passes: %d\n",pMeshSkin->GetNumPasses());
		for ( int iPass = 0; iPass < pMeshSkin->GetNumPasses(); iPass++)
		{
			fprintf(fp,"#Pass\n");
			//fprintf(fp,"#Pass\n",iPass);
			pItem = pMeshSkin->GetPass(iPass);
			if (pItem)
				WriteSkinPassInfo(fp,pItem);
			fprintf(fp,"#End\n");
		}
		fprintf(fp,"#End\n");
	}

	fclose(fp);*/

	return true;
}

void CAnimModel::Draw ( int iSkinID, int iThisFrame, int iNextFrame, float fParam )
{
	if ( (iSkinID<0) || (iSkinID>=(int)m_vSkins.size()) || (m_vFrames.size() < 1) )
		return;

	CSkin	*rSkin = GetSkin(iSkinID);

	if (!rSkin)
		return;

	glPushMatrix();

	glRotatef(-m_rRotation.x,1,0,0);
	glRotatef(m_rRotation.y,0,1,0);
	glRotatef(m_rRotation.z,0,0,1);

	trVertex3D		tempV = m_rOffset;

	tempV *= m_rScale;
	tempV *= -1.0f;

	glTranslatef(tempV.x,tempV.y,tempV.z);

	CMeshSkin	*pMeshSkin;

	glEnable(GL_TEXTURE_2D);
//	glEnable(GL_LIGHTING);
	
	for ( int iMesh = 0; iMesh < GetMeshCount(); iMesh ++)
	{
		pMeshSkin = rSkin->GetMesh(iMesh);
		if (pMeshSkin)
		{
			if (!pMeshSkin->Hidden())
			{
				if (pMeshSkin->GetNumPasses()>0)
				{
					for (int iPass = 0; iPass < pMeshSkin->GetNumPasses();iPass++)
					{
						pMeshSkin->SetPass(iPass);
						m_vMeshes[iMesh].Draw(iThisFrame,iNextFrame,fParam,!pMeshSkin->PassUsesTCs(iPass),pMeshSkin->PassUsesFaceNorms(iPass));
						pMeshSkin->ReSetPass(iPass);
					}
				}
				else
				{
					glDisable(GL_TEXTURE_2D);
					m_vMeshes[iMesh].Draw(iThisFrame,iNextFrame,fParam,false);
					glEnable(GL_TEXTURE_2D);
				}
			}
		}
	}
	glPopMatrix();
}


//**************************** ut converters
void CAnimModel::LoadUCFile ( char *szName, char* szAName, char *szDName )
{
	szAName[0] = '\0';
	szDName[0] = '\0';
	
	COSFile	oFile (szName);

	if (!oFile.Open("rb"))
		return;

	char	line[512];
	char	label[256],command[256];
	char	*pLinePtr,*pPos;
	char	temp[512];
	char	szTemp[512];

	int		iSeq = 0;

	while (!GetLine(line,oFile))
	{
		if (line[0] && (line[0] !='/'))// not blank or a commennt
		{
			sscanf(line,"%s",label);
			pLinePtr = line + strlen(label)+1;

			if (strcasecmp(label,"class")==0)	// class label
			{
				//class SGirl extends TournamentPlayer;
				// skip it
			}
			else if (strcasecmp(label,"#exec")==0)	// exec command
			{
				sscanf(pLinePtr,"%s",temp);// mesh or meshmap block
				pLinePtr += strlen(temp)+1;

				sscanf(pLinePtr,"%s",command); // command name
				pLinePtr += strlen(command)+1;

				if (strcasecmp(command,"IMPORT")==0)			// IMPORT command
				{
					//MESH=SGirl ANIVFILE=MODELS\SGirl_a.3d DATAFILE=MODELS\SGirl_d.3d
					sscanf(pLinePtr,"MESH=%s ANIVFILE=MODELS\\%s DATAFILE=MODELS\\%s",temp,szAName,szDName);
				}
				else if (strcasecmp(command,"LODPARAMS")==0)	// LODPARAMS command
				{
					//MESH=SGirl HYSTERESIS=0.00 STRENGTH=1.00 MINVERTS=10.00 MORPH=0.30 ZDISP=0.00
					// just skip it
				}
				else if (strcasecmp(command,"ORIGIN")==0)	// ORIGIN command
				{
					//MESH=SGirl X=-30.00 Y=-115.00 Z=20.00 YAW=0.00 ROLL=64.00 PITCH=-64.00
					sscanf(pLinePtr,"MESH=%s X=%f Y=%f Z=%f YAW=%f ROLL=%f PITCH=%f",temp,
											&m_rOffset.x,&m_rOffset.y,&m_rOffset.z,
											&m_rRotation.z,&m_rRotation.y,&m_rRotation.x);

					m_rRotation.x *= 1.40625f;
					m_rRotation.z *= 1.40625f;
					m_rRotation.y *= 1.40625f;
				}
				else if (strcasecmp(command,"SEQUENCE")==0)	// SEQUENCE command
				{
					//MESH=SGirl SEQ=Taunt1    STARTFRAME=5 NUMFRAMES=15 RATE=20.00 GROUP=Gesture

					// new seq 
					trSequence	seq;

					szTemp[0] = '\0';
					pPos = strstr(pLinePtr,"SEQ=");
					if (pPos)
						sscanf(pPos,"SEQ=%s",szTemp);
						
					seq.name = szTemp;

					szTemp[0] = '\0';
					pPos = strstr(pLinePtr,"GROUP=");
					if (pPos)
						sscanf(pPos,"GROUP=%s",szTemp);
					seq.group = szTemp;

					seq.start = 0;
					pPos = strstr(pLinePtr,"STARTFRAME=");
					if (pPos)
						sscanf(pPos,"STARTFRAME=%d",&seq.start);

					seq.len = 1;
					pPos = strstr(pLinePtr,"NUMFRAMES=");
					if (pPos)
						sscanf(pPos,"NUMFRAMES=%d",&seq.len);

					seq.speed = 15.0f;
					pPos = strstr(pLinePtr,"RATE=");
					if (pPos)
						sscanf(pPos,"RATE=%f",&seq.speed);

					seq.id = iSeq ++;
					m_vSequences.push_back(seq);
				}
				else if (strcasecmp(command,"SCALE")==0)		// SCALE command
				{
					//MESHMAP=SGirl X=0.06 Y=0.06 Z=0.13
					sscanf(pLinePtr,"%s X=%f Y=%f Z=%f",temp,&m_rScale.x,&m_rScale.z,&m_rScale.y);

					m_rScale.y = m_rScale.y/2.0f;
				}
				else if (strcasecmp(command,"NOTIFY")==0)	// NOTIFY command
				{
					//MESH=SGirl SEQ=RunLg     TIME=0.25 FUNCTION=PlayFootStep
					trEvent	event;
					
					szTemp[0] = '\0';
					pPos = strstr(pLinePtr,"SEQ=");
					if (pPos)
						sscanf(pPos,"SEQ=%s",szTemp);

					event.name = szTemp;

					szTemp[0] = '\0';
					pPos = strstr(pLinePtr,"FUNCTION=");
					if (pPos)
						sscanf(pPos,"FUNCTION=%s",szTemp);

					event.time = 0.0f;
					pPos = strstr(pLinePtr,"TIME=");
					if (pPos)
						sscanf(pPos,"TIME=%f",&event.time);

					m_vEvents.push_back(event);
				}
			}
		}
	}
	oFile.Close();
}

// model mananger utils

bool IsOBJFile ( const char *szExtenstion )
{
	if (!szExtenstion)
		return false;

	if (strcasecmp(szExtenstion,"OBJ") == 0)
		return true;

	if (strcasecmp(szExtenstion,"3OB") == 0)
		return true;


	return false;
}

bool IsMD3File ( const char *szExtenstion )
{
	if (!szExtenstion)
		return false;

	if (strcasecmp(szExtenstion,"MD3") == 0)
		return true;

	return false;
}

bool IsA3dFile ( const char *szExtenstion )
{
	if (!szExtenstion)
		return false;

	if (strcasecmp(szExtenstion,"A3D") == 0)
		return true;

	return false;
}

bool IsUnrealFile ( const char *szExtenstion )
{
	if (!szExtenstion)
		return false;

	if (strcasecmp(szExtenstion,"UC") == 0)
		return true;

	return false;
}

CModelManager::CModelManager()
{
	m_vModelList.clear();
	m_mModelNameMap.clear();
}

CModelManager::~CModelManager()
{
	Flush();
}

void CModelManager::LoadAll ( const char *szDirName )
{
	COSDir	oDir(szDirName);

	COSFile	oFile;
	while (oDir.GetNextFile(oFile,true))
	{
		LoadOne(oFile);
	}
}

void CModelManager::Load ( const char *szName )
{
	COSFile	oFile(szName);	
	LoadOne(oFile);
}

int CModelManager::LoadOne ( COSFile &oFile )
{
	trModelRefrence	rRefrence;

	if (IsOBJFile(oFile.GetExtension()))
		rRefrence.eType = eOBJ;
	else if (IsMD3File(oFile.GetExtension()))
		rRefrence.eType = eMD3;
	else if (IsUnrealFile(oFile.GetExtension()))
		rRefrence.eType = eU3d;
	else if (IsA3dFile(oFile.GetExtension()))
		rRefrence.eType = eA3d;
	else
		return -1;

	rRefrence.oFile = oFile;

	char szName[512];

	strcpy(szName,oFile.GetStdName());

	if (strrchr(szName,'.'))
		*strrchr(szName,'.') = '\0';

	GetModelName(szName);

	rRefrence.szName = szName;

	rRefrence.pModel = NULL;

	int iID = (int)m_vModelList.size();
	m_vModelList.push_back(rRefrence);

	m_mModelNameMap[rRefrence.szName] = iID;

	return iID;
}

void CModelManager::LoadModelFile ( trModelRefrence &rModelRef )
{
	if (rModelRef.pModel)
		return;
	
	rModelRef.pModel = new CAnimModel;

	switch(rModelRef.eType)
	{
		case eOBJ:
			rModelRef.pModel->LoadOBJ(rModelRef.oFile.GetStdName());	
		break;

		case eMD3:
			rModelRef.pModel->LoadMD3(rModelRef.oFile.GetStdName());	
		break;

		case eU3d:
			char szDir[512];
			char szName[512];

			strcpy(szDir,rModelRef.oFile.GetStdName());
			if (strrchr(szDir,'/'))
			{
				strcpy(szName,strrchr(szDir,'/')+1);
				*strrchr(szDir,'/') = '\0';

				if (strrchr(szName,'.'))
					*strrchr(szName,'.') = '\0';
			}

			rModelRef.pModel->LoadU3D(szDir,szName);	
		break;

		case eA3d:
			rModelRef.pModel->LoadA3D(rModelRef.oFile.GetStdName());
		break;
	}
}

void CModelManager::Flush ( void )
{
	tvModelList::iterator	itr = m_vModelList.begin();

	while (itr != m_vModelList.end())
	{
		delete(itr->pModel);
		itr++;
	}

	m_vModelList.clear();
	m_mModelNameMap.clear();
}

int CModelManager::GetModelID ( const char *szModelName )
{
	if ( !szModelName )
		return -1;
	
	std::string  str = szModelName;

	GetModelName((char*)str.c_str());

	tvModelNameMap::iterator	itr = m_mModelNameMap.find(str);

	if (itr == m_mModelNameMap.end())
		return -1;

	return (itr->second);
}

CAnimModel* CModelManager::GetModel ( const char *szModelName)
{
	return GetModel(GetModelID(szModelName));
}

CAnimModel* CModelManager::GetModel ( int iID )
{
	if ( (iID < 0) || (iID >= GetModelCount()) )
		return NULL;

	if (!m_vModelList[iID].pModel)
		LoadModelFile(m_vModelList[iID]);

	return m_vModelList[iID].pModel;
}

int CModelManager::GetModelCount ( void )
{
	return (int)m_vModelList.size();
}

int CModelManager::GetSubList ( const char* szKey, tvModelIDList &rList )
{
	int iLen = (int)strlen(szKey);

	tvModelList::iterator	itr = m_vModelList.begin();
	
	rList.clear();
	int iCount = 0;
	while (itr != m_vModelList.end())
	{
		if (strncasecmp(szKey,itr->szName.c_str(),iLen) == 0)
			rList.push_back(iCount);

		iCount++;
		itr++;
	}
	return 0;
}
