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

// utils.cpp

#include "utils.h"
#include <stdio.h>
#include <stdlib.h>

#ifdef _WIN32
#include <windows.h>
#include <process.h>
#endif

float ParamRand ( bool bPlusToMinus )
{
	float fRand = (float)rand()/(float)RAND_MAX;

	if (bPlusToMinus)
		fRand = fRand*2.0f -1.0f;

	return fRand;
}

void OSError ( const char *message )
{
	if (!message)
		return;

#ifdef _WIN32
	MessageBox(NULL,message,"Error",MB_OK);
#else
	printf("Error: %s\n",message);
#endif
}

void OSURLError ( const char *message, const char* url )
{
	if (!message || !url)
		return;

#ifdef _WIN32
	char	realMessage[1024];
	sprintf(realMessage,"%s\n%s\nGo to this URL now?",message,url);
	if (MessageBox(NULL,realMessage,"Alert",MB_OKCANCEL)== IDOK)
		ShellExecute(NULL, "open", "IExplore", url, ".", SW_SHOW);
#else
	printf("Error: %s\n",message);
#endif

}

