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
#include "OGFTimer.h"
#include "SDL.h"

template <>
CLGETime* Singleton<CLGETime>::_instance = (CLGETime*)0;

CLGETime::CLGETime()
{
	m_fThisTime = 0.0;
	m_fLastTime = 0.0;
	m_fTimeDelta = 0.0;

	m_liCurrentTime = 0;
	m_liFirstTime = 0;

	if(!(SDL_WasInit(SDL_INIT_EVERYTHING) & SDL_INIT_TIMER))
		SDL_InitSubSystem(SDL_INIT_TIMER);
}

CLGETime::~CLGETime()
{
}

void CLGETime::Init()
{
	if (m_liCurrentTime == 0)
		m_liCurrentTime = m_liFirstTime = SDL_GetTicks();
}

void CLGETime::Update()
{
	m_fLastTime = m_fThisTime;
	m_liCurrentTime = SDL_GetTicks();
	m_fThisTime = static_cast<double>(m_liCurrentTime - m_liFirstTime) / 1000.0;
	m_fTimeDelta = m_fThisTime - m_fLastTime;
}

