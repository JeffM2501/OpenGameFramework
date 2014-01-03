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

// 3dutils.cpp

#include "3dTypes.h"
#include <vector>
#include <string>
#include "textUtils.h"

#include "openGL.h"

void Matrix2GLMatrix( const trMatrix *pMaxtix, float *pGLMatrix )
{
	pGLMatrix[0] = pMaxtix->el[0][0];
	pGLMatrix[1] = pMaxtix->el[1][0];
	pGLMatrix[2] = pMaxtix->el[2][0];
	pGLMatrix[3] = 0.0f;

	pGLMatrix[4] = pMaxtix->el[0][1];
	pGLMatrix[5] = pMaxtix->el[1][1];
	pGLMatrix[6] = pMaxtix->el[2][1];
	pGLMatrix[7] = 0.0f;

	pGLMatrix[8] = pMaxtix->el[0][2];
	pGLMatrix[9] = pMaxtix->el[1][2];
	pGLMatrix[10] = pMaxtix->el[2][2];
	pGLMatrix[11] = 0.0f;

	pGLMatrix[12] = pMaxtix->el[0][3];
	pGLMatrix[13] = pMaxtix->el[1][3];
	pGLMatrix[14] = pMaxtix->el[2][3];
	pGLMatrix[15] = 1.0f;
}

void GLMatrix2Matrix( trMatrix *pMaxtix, const float *pGLMatrix )
{
	pMaxtix->el[0][0] = pGLMatrix[0];
	pMaxtix->el[1][0] = pGLMatrix[1];
	pMaxtix->el[2][0] = pGLMatrix[2];
	pMaxtix->el[3][0] = pGLMatrix[2];

	pMaxtix->el[0][1] = pGLMatrix[4];
	pMaxtix->el[1][1] = pGLMatrix[5];
	pMaxtix->el[2][1] = pGLMatrix[6];
	pMaxtix->el[3][1] = pGLMatrix[6];

	pMaxtix->el[0][2] = pGLMatrix[8];
	pMaxtix->el[1][2] = pGLMatrix[9];
	pMaxtix->el[2][2] = pGLMatrix[10];
	pMaxtix->el[3][2] = pGLMatrix[10];

	pMaxtix->el[0][3] = pGLMatrix[12];
	pMaxtix->el[1][3] = pGLMatrix[13];
	pMaxtix->el[2][3] = pGLMatrix[14];
	pMaxtix->el[3][3] = pGLMatrix[14];
}

void CGLMatrix::LoadFromGL( void )
{
	float	afGLMatrix[16];

	glGetFloatv(GL_MODELVIEW_MATRIX,afGLMatrix);

	GLMatrix2Matrix(&matrix,afGLMatrix);
}

void CGLMatrix::SetToGL( void )
{
	float	afGLMatrix[16];
	Matrix2GLMatrix(&matrix,afGLMatrix);
	glMultMatrixf(afGLMatrix);
}

// color class
CColor::CColor()
{
	Set(1,1,1,1);
}

CColor::CColor(trColor &rColor)
{
	Set(rColor);
	m_fAlpha = 1.0f;
}

CColor::CColor(float r,float g, float b, float a)
{
	Set(r,g,b,a);
}

CColor::~CColor()
{

}

void CColor::Set (trColor &rColor)
{
	m_fRed = rColor.r;
	m_fGreen =rColor.g;
	m_fBlue = rColor.b;
}

void CColor::Set ( float r,float g, float b, float a )
{
	m_fRed = r;
	m_fGreen = g;
	m_fBlue = b;
	m_fAlpha = a;
}

float CColor::Red (void)
{
	return m_fRed;
}

float CColor::Green (void)
{
	return m_fGreen;
}

float CColor::Blue (void)
{
	return m_fBlue;
}

float CColor::Alpha (void)
{
	return m_fAlpha;
}

void CColor::glColor ( void )
{
	glColor4f(m_fRed,m_fGreen,m_fBlue,m_fAlpha);
}

void CColor::glColor ( float alpha )
{
	glColor4f(m_fRed,m_fGreen,m_fBlue,alpha);
}

bool CColor::parseColor ( std::string &text, std::string delim )
{
	if (!text.size())
		return false;

	std::vector<std::string> parts = string_util::tokenize(text,delim);
	if (parts.size() == 1)
	{
		// it's a word
		std::string colorText = string_util::toupper(parts[0]);

		if (colorText[0] == '#')
		{
			// it's a web color, parse it somehow
		}
		else
		{
			if (colorText == "WHITE")
				Set(1,1,1,1);
			else if (colorText == "BLACK")
				Set(0,0,0,1);
			else if (colorText == "RED")
				Set(1,0,0,1);
			else if (colorText == "BLUE")
				Set(0,0,1,1);
			else if (colorText == "GREEN")
				Set(0,1,0,1);
			else if (colorText == "YELLOW")
				Set(1,1,0,1);
			else if (colorText == "PURPLE")
				Set(1,0,1,1);
			else if (colorText == "CYAN")
				Set(0,1,1,1);
			else if (colorText == "ORANGE")
				Set(1,0.5f,0,1);
			else if (colorText == "BROWN")
				Set(0.75f,0.5f,0,1);
			else if (colorText == "GRAY")
				Set(0.5f,0.5f,0.5f,1);
			else if (colorText == "LTGRAY")
				Set(0.75f,0.75f,0.75f,1);
			else if (colorText == "DKGRAY")
				Set(0.25f,0.25f,0.25f,1);
			else
				return false;
		}
	}
	else 
		m_fRed = (float)atof(parts[0].c_str());

	if ( parts.size() > 1)
		m_fGreen = (float)atof(parts[1].c_str());

	if ( parts.size() > 2)
		m_fBlue = (float)atof(parts[2].c_str());

	m_fAlpha = 1.0f;
	if ( parts.size() > 3)
		m_fAlpha = (float)atof(parts[3].c_str());

	if  ( m_fRed > 1 || m_fGreen > 1 || m_fBlue > 1  || m_fAlpha > 1 )	//is it 256 RGB format format
	{	
		m_fRed = m_fRed/255.0f;
		m_fGreen = m_fGreen/255.0f;
		m_fBlue = m_fBlue/255.0f;
		m_fAlpha = m_fAlpha/255.0f;
	}
	return true;
}

CColor::operator trColor& (void)
{
	m_rColor.r = m_fRed;
	m_rColor.g = m_fGreen;
	m_rColor.b = m_fBlue;
	return m_rColor;
}

CColor::operator const trColor& (void)
{
	m_rColor.r = m_fRed;
	m_rColor.g = m_fGreen;
	m_rColor.b = m_fBlue;
	return m_rColor;
}

bool CColor::operator == ( const CColor& r )
{
	if (m_fAlpha != r.m_fAlpha)
		return false;

	if (m_fRed != r.m_fRed)
		return false;

	if (m_fGreen != r.m_fGreen)
		return false;

	if (m_fBlue != r.m_fBlue)
		return false;

	return true;
}

bool CColor::operator != ( const CColor& r )
{
	return !(*this == r);
}


// CGLPoint Class

CGLPoint::CGLPoint()
{
	C3DVertex::C3DVertex();
}

CGLPoint::CGLPoint( const C3DVertex& r)
{
	m_rVertex = r.m_rVertex;
}

CGLPoint::CGLPoint( const CGLPoint& r)
{
	m_rVertex = r.m_rVertex;
}

CGLPoint::CGLPoint(float x , float y, float z)
{
	C3DVertex::C3DVertex(x,y,z);
}

//CGLPoint::CGLPoint(float *pos)
//{
//	C3DVertex::C3DVertex(pos);
//}

CGLPoint::~CGLPoint()
{
	//C3DVertex::~C3DVertex();
}

// operators
CGLPoint& CGLPoint::operator = ( const C3DVertex& r)
{
	m_rVertex = r.m_rVertex;
	return *this;
}

CGLPoint& CGLPoint::operator = (const trVertex3D &p0)
{
	m_rVertex = p0;
	return *this;
}

CGLPoint CGLPoint::operator - (const CGLPoint &p0) const
{
	return CGLPoint( this->m_rVertex.x -p0.m_rVertex.x,this->m_rVertex.y -p0.m_rVertex.y,this->m_rVertex.z -p0.m_rVertex.z);
}

CGLPoint CGLPoint::operator + (const CGLPoint &p0) const
{
	return CGLPoint( this->m_rVertex.x +p0.m_rVertex.x,this->m_rVertex.y + p0.m_rVertex.y,this->m_rVertex.z + p0.m_rVertex.z);
}


void CGLPoint::operator -= (const CGLPoint &p0)
{
	m_rVertex -= p0;
}

void CGLPoint::operator += (const CGLPoint &p0)
{
	m_rVertex += p0;
}

void CGLPoint::operator *= (const CGLPoint &p0)
{
	m_rVertex *= p0;
}

void CGLPoint::operator /= (const CGLPoint &p0)
{
	m_rVertex /= p0;
}

// gl funcs


void CGLPoint::glNormal (void) const
{
	glNormal3f(m_rVertex.x,m_rVertex.y,m_rVertex.z);
}

void CGLPoint::glVertex (void) const
{
	glVertex3f(m_rVertex.x,m_rVertex.y,m_rVertex.z);
}

void CGLPoint::glTranslate (void) const
{
	glTranslatef(m_rVertex.x,m_rVertex.y,m_rVertex.z);
}
