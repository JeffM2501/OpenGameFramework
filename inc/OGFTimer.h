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

#ifndef _SGLTIMER_H_
#define _SGLTIMER_H_

#ifdef _WIN32
#include <windows.h>
#endif

#include "singleton.h"


class CLGETime : public Singleton<CLGETime>
{
private:
	unsigned int	m_liCurrentTime;
	unsigned int	m_liFirstTime;

	double		m_fThisTime;
	double		m_fLastTime;
	double		m_fTimeDelta;

public:
	CLGETime();
	~CLGETime();


	void		Init();
	void		Update();
	double		GetTime() { return m_fThisTime; }
	double		GetFrameTime() { return m_fTimeDelta; }

};

#endif
