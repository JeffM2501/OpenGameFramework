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

#ifndef _UTILS_H_
#define _UTILS_H_

#ifdef _WIN32
#include <windows.h>
#endif

float ParamRand ( bool bPlusToMinus = false );
void OSError ( const char *message );
void OSURLError ( const char *message, const char* url );

#endif //_UTILS_H_

