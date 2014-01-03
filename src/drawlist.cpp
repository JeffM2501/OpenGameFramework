// renderer.cpp

#include "openGL.h"

#include "drawlist.h"
#include "3dView.h"
#include "materials.h"
#include "textureManager.h"

#define _LIST_DEF_SIZE 512

// draw list class
// like a stack that grows in chunks when needed
CSubDrawList::CSubDrawList()
{
	iEnd = 0;
	iSize = 0;
	pData = NULL;
}

CSubDrawList::~CSubDrawList()
{
	iEnd = 0;
	iSize = 0;
	if (pData)
		free(pData);
}

void CSubDrawList::Clear ( void )
{
	iEnd = 0;
}

void CSubDrawList::Push (  trDrawItem &rItem )
{
	if (iSize == 0)
	{
		iSize = _LIST_DEF_SIZE;
		pData = (trDrawItem*)malloc(sizeof(trDrawItem)*iSize);
	}

	pData[iEnd++] = rItem;

	if (iEnd >= iSize)
	{
		trDrawItem*hNewAray = (trDrawItem*)malloc(sizeof(trDrawItem)*iSize*2);

		memcpy(hNewAray,pData,iSize*(sizeof(trDrawItem)));

		iSize *=2;
		free(pData);
		pData = hNewAray;
	}
}

trDrawItem* CSubDrawList::Pop ( void )
{
	if (iEnd == 0)
		return NULL;

	return &(pData[--iEnd]);
}

trDrawItem* CSubDrawList::Peek ( void )
{
	if (iEnd == 0)
		return NULL;

	return &(pData[iEnd]);
}

// textured list class

CTexturedDrawList::CTexturedDrawList()
{
	iMaxTextures = 0;
}

CTexturedDrawList::~CTexturedDrawList()
{
	tvDrawListList::iterator itr = vList.begin();

	while (itr != vList.end())
		delete(*(itr++));
}

void CTexturedDrawList::Init ( int iMaxTextires )
{
	iMaxTextures = iMaxTextires;

	for (unsigned int i = 0; i < iMaxTextures; i++)
		vList.push_back(new CSubDrawList());
}

CSubDrawList* CTexturedDrawList::GetList ( int iTexture )
{
	return vList[iTexture];
}

void CTexturedDrawList::Push ( trDrawItem &rItem, int iTexture )
{
	if (iTexture >= (int)vList.size())
		return;

	vList[iTexture]->Push(rItem);
}

void CTexturedDrawList::ClearAll ( void )
{
	tvDrawListList::iterator itr = vList.begin();

	while (itr != vList.end())
		(*(itr++))->Clear();
}

// base render class

CDrawingList::CDrawingList()
{
//	m_iMaxTextures = 0;
//	m_pDrawList = NULL;
}

CDrawingList::~CDrawingList()
{
}

void CDrawingList::AddDrawItem ( CBaseDrawItem *pObject, int iTexture, teRenderPriority ePriority, int param )
{
	trDrawItem rItem;
	
	rItem.ePriority = ePriority;
	rItem.pItem = pObject;
	rItem.param = param;
	afLists[ePriority].Push(rItem,iTexture+1);
}

void CDrawingList::Init( void )
{
	// set up a default world light
	InitDrawList();
}


void CDrawingList::InitDrawList ( void )
{
	iMaxTextures = CTextureManager::instance().GetNumTexures();

        for ( int i = 0; i < (int)eLast;i++)
	  afLists[i].Init(iMaxTextures+1);
}

void CDrawingList::PreDraw ( void )
{
	ClearDrawList();
}

void CDrawingList::Draw ( void )
{
	glEnable(GL_TEXTURE_2D);
	glEnable(GL_LIGHTING);
	C3dView::instance().UpdateLights();

	DrawVisList();
}

void CDrawingList::DrawVisPrioritysList ( teRenderPriority ePriority )
{
	CTextureManager	&tm = CTextureManager::instance();

	trDrawItem * pItem;

	CSubDrawList*		pList;

	for (unsigned int i = 1; i <= iMaxTextures; i++)
	{		
		pList = afLists[ePriority].GetList(i);

		pItem = pList->Pop();
		if (pItem)
		{
			tm.Bind(i-1);
			
			while (pItem)
			{
				pItem->pItem->DrawItem(i-1,pItem->param);

				// really should not have to do this
				glColor4f( 1,1,1,1 );
				pItem = pList->Pop();
			}
		}
	}
	
	pList = afLists[ePriority].GetList(0);

	pItem = pList->Pop();
	if (pItem)
	{
		glDisable(GL_TEXTURE_2D);
		while (pItem)
		{
			pItem->pItem->DrawItem(-1);
			pItem = pList->Pop();
		}
		glEnable(GL_TEXTURE_2D);
	}
}

void CDrawingList::DrawVisList ( void )
{
  for ( int i = 0; i < (int)eLast;i++)
  {
    if (i == eTrans)
      glDepthMask(0);
    DrawVisPrioritysList((teRenderPriority)i);

    if (i == eTrans)
      glDepthMask(1);
  }
}

void CDrawingList::ClearDrawList ( void )
{
  for ( int i = 0; i < (int)eLast;i++)
    afLists[i].ClearAll();
}
