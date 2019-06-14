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

#ifndef _OLED_BMP_H_
#define _OLED_BMP_H_

#define BMP_TORASTER_OFFSET     10
#define BMP_SIZE_OFFSET         18
#define BMP_BPP_OFFSET          28
#define BMP_RLE_OFFSET          30
#define BMP_COLOR_OFFSET        54

#define fill4B(a)       ( ( 4 - ( (a) % 4 ) ) & 0x03)

struct color {
	unsigned char red;
	unsigned char green;
	unsigned char blue;
};

int fh_bmp_id(const char *name);
int fh_bmp_load(const char *name, unsigned char **buffer, int* xp, int* yp);
int fh_bmp_getsize(const char *name, int *x, int *y);

#endif
