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
#ifndef __BITMAP_H__
#define __BITMAP_H__

#ifndef _WIN32
#define HANDLE_PRAGMA_PACK_PUSH_POP 1
#pragma pack(push, 2)

typedef struct 
{
	unsigned short			bfType;
	unsigned long			bfSize;
	unsigned short			bfReserved1;
	unsigned short			bfReserved2;
	unsigned long			bfOffBits;
} BITMAPFILEHEADER;

typedef struct
{
        unsigned long			biSize;
        long				biWidth;
        long				biHeight;
        unsigned short			biPlanes;
        unsigned short			biBitCount;
        unsigned long			biCompression;
        unsigned long			biSizeImage;
        long				biXPelsPerMeter;
        long				biYPelsPerMeter;
        unsigned long			biClrUsed;
        unsigned long			biClrImportant;
} BITMAPINFOHEADER;

#pragma pack(pop)

#endif // _WIN32

#endif
