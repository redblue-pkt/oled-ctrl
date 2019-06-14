/*
 *   Copyright (C) redblue 2019
 *
 *   This program is free software; you can redistribute it and/or modify
 *   it under the terms of the GNU General Public License as published by
 *   the Free Software Foundation; either version 2 of the License, or
 *   (at your option) any later version.
 *
 *   This program is distributed in the hope that it will be useful,
 *   but WITHOUT ANY WARRANTY; without even the implied warranty of
 *   MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.  See the
 *   GNU General Public License for more details.
 *
 *   You should have received a copy of the GNU General Public License
 *   along with this program; if not, write to the Free Software
 *   Foundation, Inc., 675 Mass Ave, Cambridge, MA 02139, USA.
 *
 */

#ifndef _OLED_JPEG_H_
#define _OLED_JPEG_H_

#include <setjmp.h>
#include <jpeglib.h>

#define MIN(a,b) ((a)>(b)?(b):(a))

struct r_jpeg_error_mgr
{
	struct jpeg_error_mgr pub;
	jmp_buf envbuffer;
};

int fh_jpeg_id(const char *name);
int fh_jpeg_load(const char *name, unsigned char **buffer, int* xp, int* yp);
int fh_jpeg_getsize(const char *name, int *x, int *y);

#endif
