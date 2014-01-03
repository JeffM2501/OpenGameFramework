// skybox.h

#ifndef _SKYBOX_H_
#define _SKYBOX_H_

#include "textureManager.h"
#include "viewCull.h"
#include "displayListManager.h"

typedef enum
{
	eTop,
	eBottom,
	eXPlus,
	eXNeg,
	eYPlus,
	eYNeg
}teSkyboxFace;

typedef struct
{
	bool		active;
	CColor		color;
	float		nearD;
	float		farD;
}trFogInfo;

class CSkybox : public DisplayListBuilder
{
public:
	CSkybox();
	virtual ~CSkybox();

	virtual bool Create ( float fSize, const char *szName, trFogInfo *fog = NULL );
	virtual void Draw ( void );

	virtual void Rebuild ( void );

	void SetTexturePath ( const char *szTexturePath );
	void SetZShift ( float fShift );
	void SetFog ( trFogInfo *fog );

	virtual void build ( int list );

protected:
	trFogInfo			fogInfo;
	
	char				m_szPath[256];
	char				m_szName[256];
	int					m_aiTextureIDs[6];
	int					m_aiLists[6];
	float				m_fSize;
	float				m_fShift;

	float				m_afVerts[8][3];
};

#endif //_SKYBOX_H_
