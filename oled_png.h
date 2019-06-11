/*
 *   Copyright (C) redblue 2018
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

#ifndef _OLED_PNG_H_
#define _OLED_PNG_H_

#define PNG_BYTES_TO_CHECK 4
#ifndef min
#define min(x,y) ((x) < (y) ? (x) : (y))
#endif

int fh_png_id(const char *name);
int fh_png_load(const char *name, unsigned char **buffer, int* xp, int* yp);
int fh_png_getsize(const char *name, int *x, int *y);

#endif
