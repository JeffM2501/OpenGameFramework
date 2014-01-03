/* Open Game Framework
* Copyright (c) 2004 - 2005 Jeffrey Myers
*
* This package is free software;  you can redistribute it and/or
* modify it under the terms of the license found in the file
* named license.txt that should have accompanied this file.
*
* THIS PACKAGE IS PROVIDED ``AS IS'' AND WITHOUT ANY EXPRESS OR
* IMPLIED WARRANTIES, INCLUDING, WITHOUT LIMITATION, THE IMPLIED
* WARRANTIES OF MERCHANTIBILITY AND FITNESS FOR A PARTICULAR PURPOSE.
*/
#ifndef _OGF_H_
#define _OGF_H_

#ifndef OGF_LITE
	#include "openGL.h"
	#include "3dTypes.h"
	#include "3dView.h"
	#include "textureManager.h"
	#include "animMesh.h"
	#include "clouds.h"
	#include "displayListManager.h"
	#include "drawlist.h"
	#include "fontManager.h"
	#include "input.h"
	#include "panelManager.h"
	#include "viewCull.h"
	#include "skybox.h"
	#include "soundmanager.h"
#endif

#include "gameLoop.h"
#include "commandArgs.h"
#include "OGFEvents.h"
#include "OSFile.h"
#include "prefs.h"
#include "textUtils.h"
#include "OGFTimer.h"
#include "utils.h"
#include "mathUtils.h"
#include "parser.h"

#endif // _OGF_H_
