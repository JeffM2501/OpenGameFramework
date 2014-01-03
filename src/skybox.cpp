// skybox.cpp

#include "openGL.h"
#include "skybox.h"
#include "gameLoop.h"
#include "3dView.h"

CSkybox::CSkybox()
{
	m_szPath[0] = '\0';
	m_szName[0] = '\0';

	m_aiTextureIDs[0] = m_aiTextureIDs[1] = m_aiTextureIDs[2] = m_aiTextureIDs[3] = m_aiTextureIDs[4] = m_aiTextureIDs[5] = -1;
	m_aiLists[0] = m_aiLists[1] = m_aiLists[2] = m_aiLists[3] = m_aiLists[4] = m_aiLists[5] = -1;

	m_fSize = -1;
	m_fShift = 0;

	fogInfo.active = false;
}

CSkybox::~CSkybox()
{
	for ( int i =eTop; i <= eYNeg; i++)
		DisplayListManager::instance().releaseList(m_aiLists[i]);
}

void CSkybox::SetTexturePath ( const char *szTexturePath )
{
	if (szTexturePath)
		strcpy(m_szPath,szTexturePath);
	else
		m_szPath[0] = '\0'; 
}

void CSkybox::SetZShift ( float fShift )
{
	m_fShift = fShift;
}

bool CSkybox::Create ( float fSize, const char *szName, trFogInfo *fog )
{
	// clear out any old display lists
	for ( int i =eTop; i <= eYNeg; i++)
		DisplayListManager::instance().releaseList(m_aiLists[i]);

	SetFog(fog);
	if (!szName)
		return false;

	strcpy(m_szName,szName);
	m_fSize = fSize;

	// get the base folder name
	char szTextureBaseName[512];
	char szTextureName[512];

	strcpy(szTextureBaseName,m_szPath);
	strcat(szTextureBaseName,"/");
	strcat(szTextureBaseName,szName);
	strcat(szTextureBaseName,"/");
	strcat(szTextureBaseName,szName);

	// load up the textures
	// top 
	strcpy(szTextureName,szTextureBaseName);
	strcat(szTextureName,"_up");
	m_aiTextureIDs[eTop] = CTextureManager::instance().GetID(szTextureName);

	// bottom 
	strcpy(szTextureName,szTextureBaseName);
	strcat(szTextureName,"_dn");
	m_aiTextureIDs[eBottom] = CTextureManager::instance().GetID(szTextureName);

	// eXPlus 
	strcpy(szTextureName,szTextureBaseName);
	strcat(szTextureName,"_rt");
	m_aiTextureIDs[eXPlus] = CTextureManager::instance().GetID(szTextureName);

	// eXNeg
	strcpy(szTextureName,szTextureBaseName);
	strcat(szTextureName,"_lf");
	m_aiTextureIDs[eXNeg] = CTextureManager::instance().GetID(szTextureName);

	// eYPlus 
	strcpy(szTextureName,szTextureBaseName);
	strcat(szTextureName,"_ft");
	m_aiTextureIDs[eYPlus] = CTextureManager::instance().GetID(szTextureName);

	// eYNeg 
	strcpy(szTextureName,szTextureBaseName);
	strcat(szTextureName,"_bk");
	m_aiTextureIDs[eYNeg] = CTextureManager::instance().GetID(szTextureName);

	Rebuild();
	return true;
}

void CSkybox::build ( int list )
{

	float textureSize = (float)CTextureManager::instance().GetImageWidth(m_aiTextureIDs[eTop]);
	float fudge = 1/textureSize;//0.1f;	// just to make it blend 1 pixel in
	//	float geoFudge = 1.0f + (fudge/2.0f);

	float realSize = m_fSize;//*geoFudge;

	float uvOne = 1.0f-fudge;
	float uvZero = fudge;

	// top
	if (m_aiLists[eTop] == list)
	{
		glBegin(GL_QUADS);
		glTexCoord2f(uvOne,uvOne);
		glVertex3f(-realSize,realSize,m_fSize + m_fShift);
		glTexCoord2f(uvOne,uvZero);
		glVertex3f(realSize,realSize,m_fSize + m_fShift);
		glTexCoord2f(uvZero,uvZero);
		glVertex3f(realSize,-realSize,m_fSize + m_fShift);
		glTexCoord2f(uvZero,uvOne);
		glVertex3f(-realSize,-realSize,m_fSize + m_fShift);
		glEnd();
	}

	// bottom 
	if (m_aiLists[eBottom] == list)
	{
		glBegin(GL_QUADS);
			glTexCoord2f(uvZero,uvZero);
			glVertex3f(-realSize,-realSize,-m_fSize + m_fShift);
			glTexCoord2f(uvZero,uvOne);
			glVertex3f(realSize,-realSize,-m_fSize + m_fShift);
			glTexCoord2f(uvOne,uvOne);
			glVertex3f(realSize,realSize,-m_fSize + m_fShift);
			glTexCoord2f(uvOne,uvZero);
			glVertex3f(-realSize,realSize,-m_fSize + m_fShift);
		glEnd();
	}

	// eXPlus 
	if (m_aiLists[eXPlus] == list)
	{
		glBegin(GL_QUADS);
			glTexCoord2f(uvZero,uvOne);
			glVertex3f(m_fSize,-realSize,realSize + m_fShift);
			glTexCoord2f(uvOne,uvOne);
			glVertex3f(m_fSize,realSize,realSize + m_fShift);
			glTexCoord2f(uvOne,uvZero);
			glVertex3f(m_fSize,realSize,-realSize + m_fShift);
			glTexCoord2f(uvZero,uvZero);
			glVertex3f(m_fSize,-realSize,-realSize + m_fShift);
		glEnd();
	}

	// eXNeg
	if (m_aiLists[eXNeg] == list)
	{
		glBegin(GL_QUADS);
			glTexCoord2f(uvOne,uvZero);
			glVertex3f(-m_fSize,-realSize,-realSize + m_fShift);
			glTexCoord2f(uvZero,uvZero);
			glVertex3f(-m_fSize,realSize,-realSize + m_fShift);
			glTexCoord2f(uvZero,uvOne);
			glVertex3f(-m_fSize,realSize,realSize + m_fShift);
			glTexCoord2f(uvOne,uvOne);
			glVertex3f(-m_fSize,-realSize,realSize + m_fShift);
		glEnd();
	}

	// eYPlus 
	if (m_aiLists[eYPlus] == list)
	{
		glBegin(GL_QUADS);
			glTexCoord2f(uvOne,uvZero);
			glVertex3f(-realSize,m_fSize,-realSize + m_fShift);
			glTexCoord2f(uvZero,uvZero);
			glVertex3f(realSize,m_fSize,-realSize + m_fShift);
			glTexCoord2f(uvZero,uvOne);
			glVertex3f(realSize,m_fSize,realSize + m_fShift);
			glTexCoord2f(uvOne,uvOne);
			glVertex3f(-realSize,m_fSize,realSize + m_fShift);
		glEnd();
	}

	// eYNeg 
	if (m_aiLists[eYNeg] == list)
	{	
		glBegin(GL_QUADS);
			glTexCoord2f(uvZero,uvOne);
			glVertex3f(-realSize,-m_fSize,realSize + m_fShift);
			glTexCoord2f(uvOne,uvOne);
			glVertex3f(realSize,-m_fSize,realSize + m_fShift);
			glTexCoord2f(uvOne,uvZero);
			glVertex3f(realSize,-m_fSize,-realSize + m_fShift);
			glTexCoord2f(uvZero,uvZero);
			glVertex3f(-realSize,-m_fSize,-realSize + m_fShift);
		glEnd();
	}
}

void CSkybox::Rebuild ( void )
{
	// set up all the lists

	for ( int i =eTop; i <= eYNeg; i++)
	{
		m_aiLists[i] = DisplayListManager::instance().newList(this,true);
		DisplayListManager::instance().rebuildList(m_aiLists[i]);
	}

	// set the verts
	float realSize = m_fSize;

	m_afVerts[0][0] = realSize;
	m_afVerts[0][1] = realSize;
	m_afVerts[0][2] = realSize+m_fShift;

	m_afVerts[1][0] = realSize;
	m_afVerts[1][1] = -realSize;
	m_afVerts[1][2] = realSize+m_fShift;

	m_afVerts[2][0] = -realSize;
	m_afVerts[2][1] = -realSize;
	m_afVerts[2][2] = realSize+m_fShift;

	m_afVerts[3][0] = -realSize;
	m_afVerts[3][1] = realSize;
	m_afVerts[3][2] = realSize+m_fShift;

	m_afVerts[4][0] = realSize;
	m_afVerts[4][1] = realSize;
	m_afVerts[4][2] = -realSize+m_fShift;

	m_afVerts[5][0] = realSize;
	m_afVerts[5][1] = -realSize;
	m_afVerts[5][2] = -realSize+m_fShift;

	m_afVerts[6][0] = -realSize;
	m_afVerts[6][1] = -realSize;
	m_afVerts[6][2] = -realSize+m_fShift;

	m_afVerts[7][0] = -realSize;
	m_afVerts[7][1] = realSize;
	m_afVerts[7][2] = -realSize+m_fShift;
}

void CSkybox::SetFog ( trFogInfo *fog )
{
	// save off the data
	if (fog)
	{
		fogInfo.color = fog->color;
		fogInfo.farD = fog->farD;
		fogInfo.nearD = fog->nearD;

		if ( (fogInfo.farD != 0.0 ) && (fogInfo.farD != fogInfo.nearD) )
			fogInfo.active = true;
	}
	else
		fogInfo.active = false;

	// now GL it
	if (!fogInfo.active)
	{
		glDisable(GL_FOG);
		return;
	}
	glEnable(GL_FOG);
	glFogi(GL_FOG_MODE,GL_LINEAR);

	float	afParam[4];

	glFogf(GL_FOG_START,fogInfo.nearD);
	glFogf(GL_FOG_END,fogInfo.farD);

	afParam[0] = fogInfo.color.Red();
	afParam[1] = fogInfo.color.Green();
	afParam[2] = fogInfo.color.Blue();
	afParam[3] = 0.25f;

	glFogfv(GL_FOG_COLOR,afParam);
}

void CSkybox::Draw ( void )
{
	if (!m_szName[0] || (m_fSize < 1) )
		return;

	glEnable(GL_TEXTURE_2D);
	glDisable(GL_LIGHTING);

	glDepthMask(0);
	glColor4f(1,1,1,1);

	CViewFrustum &frustum = C3dView::instance().GetFrustum();

	// top
	if (!frustum.QuadOut(m_afVerts[0],m_afVerts[1],m_afVerts[2],m_afVerts[3]))
	{
		CTextureManager::instance().Bind(m_aiTextureIDs[eTop]);
		DisplayListManager::instance().callList(m_aiLists[eTop]);
	}

	// bottom
	if (!frustum.QuadOut(m_afVerts[4],m_afVerts[5],m_afVerts[6],m_afVerts[7]))
	{
		CTextureManager::instance().Bind(m_aiTextureIDs[eBottom]);
		DisplayListManager::instance().callList(m_aiLists[eBottom]);
	}

	// XPlus
	if (!frustum.QuadOut(m_afVerts[0],m_afVerts[1],m_afVerts[5],m_afVerts[4]))
	{
		CTextureManager::instance().Bind(m_aiTextureIDs[eXPlus]);
		DisplayListManager::instance().callList(m_aiLists[eXPlus]);
	}

	// XNeg
	if (!frustum.QuadOut(m_afVerts[2],m_afVerts[3],m_afVerts[6],m_afVerts[7]))
	{
		CTextureManager::instance().Bind(m_aiTextureIDs[eXNeg]);
		DisplayListManager::instance().callList(m_aiLists[eXNeg]);
	}

	// YPlus
	if (!frustum.QuadOut(m_afVerts[0],m_afVerts[3],m_afVerts[5],m_afVerts[7]))
	{
		CTextureManager::instance().Bind(m_aiTextureIDs[eYPlus]);
		DisplayListManager::instance().callList(m_aiLists[eYPlus]);
	}

	// YNeg
	if (!frustum.QuadOut(m_afVerts[1],m_afVerts[2],m_afVerts[4],m_afVerts[6]))
	{
		CTextureManager::instance().Bind(m_aiTextureIDs[eYNeg]);
		DisplayListManager::instance().callList(m_aiLists[eYNeg]);
	}

	glEnable(GL_LIGHTING);
	glDepthMask(1);
}
