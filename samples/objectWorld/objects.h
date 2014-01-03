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
// objects.h

#ifndef _OBJECTS_H_
#define _OBJECTS_H_

#include "world.h"

class GroupFactory : public BaseOjbectFactory
{
public:
	virtual MapBaseObject *newObject( void );
};
extern GroupFactory groupFactory;

class InstanceFactory : public BaseOjbectFactory
{
public:
	virtual MapBaseObject *newObject( void );
};
extern InstanceFactory instanceFactory;

class EnvironmentFactory : public BaseOjbectFactory
{
public:
	virtual MapBaseObject *newObject( void );
};
extern EnvironmentFactory environmentFactory;

class SkyboxFactory : public BaseOjbectFactory
{
public:
	virtual MapBaseObject *newObject( void );
};
extern SkyboxFactory skyboxFactory;


class GroundFactory : public BaseOjbectFactory
{
public:
	virtual MapBaseObject *newObject( void );
};
extern GroundFactory groundFactory;

class BoxFactory : public BaseOjbectFactory
{
public:
	virtual MapBaseObject *newObject( void );
};
extern BoxFactory boxFactory;


#endif //_OBJECTS_H_
