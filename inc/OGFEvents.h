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
// utils.h

#ifndef _EVENTS_H_
#define _EVENTS_H_


#include <stdio.h>
#include <stdlib.h>
#include <vector>
#include <map>

typedef int (*tEventHandalerCallBack)(unsigned char event, void* message, void *param);

typedef struct
{
	tEventHandalerCallBack	callback;
	void *					param;
}trEventHandaler;

typedef std::vector<trEventHandaler> tvEventHandalerEventList;
typedef std::map<unsigned char,tvEventHandalerEventList> tvEventHandalerEventMap;

#include "singleton.h"

class CEventManager: public Singleton<CEventManager>
{
protected:
	friend class Singleton<CEventManager>;
	CEventManager();
	~CEventManager();

public:
	// registration for handaling of raw SDL Events
	// param is some pointer passed by the handaler that is passed back when the event is called
	void registerEventHandaler ( unsigned char event, tEventHandalerCallBack callback, void* param );
	void removeEventHandaler ( unsigned char event, tEventHandalerCallBack callback, void* param ); 

	// only used by gameloop
	void callEventHandaler ( unsigned char event, void* message );

protected:
	tvEventHandalerEventMap		theEventMap;
};
#endif //_EVENTS_H_

