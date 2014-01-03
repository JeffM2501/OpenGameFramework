/* 
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
// drawlist.h

#ifndef _DRAWING_LIST_H_
#define _DRAWING_LIST_H_

#include <vector>
#include <string>
#include <list>
#include <map>

typedef enum
{
	eFirst = 0,
	ePass0,
	ePass1,
	ePass2,
	eTrans,
	eLast
}teRenderPriority;

class CBaseDrawItem
{
public:	
	virtual ~CBaseDrawItem(){return;}
	virtual void DrawItem ( int item, int param = 0 ) = 0;
};

typedef struct
{
	CBaseDrawItem           *pItem;
	int						param;
	teRenderPriority		ePriority;
}trDrawItem;

class CSubDrawList
{
public:
	CSubDrawList();
	~CSubDrawList();

	void Push ( trDrawItem	&rItem );
	trDrawItem* Pop ( void );
	trDrawItem* Peek ( void );

	void Clear ( void );

protected:
	int			iEnd;
	int			iSize;

	trDrawItem *pData;
};

typedef std::vector<CSubDrawList*> tvDrawListList;

class CTexturedDrawList
{
public:
	CTexturedDrawList();
	~CTexturedDrawList();

	void Init ( int iMaxTextires );
	CSubDrawList* GetList ( int materials );

	void Push ( trDrawItem &rItem, int iTexture  );

	void ClearAll ( void );
protected:
	tvDrawListList	vList;
	unsigned int	iMaxTextures;
};

class CDrawingList
{
public:
	CDrawingList();
	virtual ~CDrawingList();

	virtual void Init ( void );
	
	virtual void PreDraw ( void );
	virtual void Draw ( void );

	virtual void AddDrawItem ( CBaseDrawItem *pObject, int iTexture, teRenderPriority ePriority = ePass1, int param = 0 );
	
protected:
	void InitDrawList ( void );
	void DrawVisList ( void );
	void DrawVisPrioritysList ( teRenderPriority ePriority );

	void ClearDrawList ( void );

	CTexturedDrawList	afLists[eLast];
	unsigned int		iMaxTextures;
};

#endif//_DRAWING_LIST_H_
