// opengl.h

#ifndef _OPEN_GL_H_
#define _OPEN_GL_H_

#ifdef __APPLE__
#  include <OpenGL/gl.h>
#  include <OpenGL/glu.h>
#else
#ifdef _WIN32
#include <windows.h>
#endif
#  include <GL/gl.h>
#  include <GL/glu.h>
#endif

#define GL_INVALID_ID 0xFFFFFFFF
#endif //_OPEN_GL_H_
