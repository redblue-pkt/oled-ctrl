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

#ifndef _OLED_GIF_H_
#define _OLED_GIF_H_

#include <setjmp.h>
#include <gif_lib.h>

#if GIFLIB_MAJOR >= 5 && GIFLIB_MINOR >= 1
#define DGIFCLOSEFILE(x) { int _error; DGifCloseFile(x, &_error); }
#else
#define DGIFCLOSEFILE(x) DGifCloseFile(x)
#endif

#include <signal.h>
#define min(a,b) ((a) < (b) ? (a) : (b))

#define gflush return(FH_ERROR_FILE);
#define grflush { DGIFCLOSEFILE(gft); return(FH_ERROR_FORMAT); }
#define mgrflush { free(lb); free(slb); DGIFCLOSEFILE(gft); return(FH_ERROR_FORMAT); }

int fh_gif_id(const char *name);
int fh_gif_load(const char *name, unsigned char **buffer, int* xp, int* yp);
int fh_gif_getsize(const char *name, int *x, int *y);

#endif
