#include <stdio.h>
#include <sys/time.h>
#include <sys/types.h>
#include <unistd.h>
#include <getopt.h>
#include <stdlib.h>
#include <termios.h>
#include <string.h>
#include <signal.h>

#include "oled_bmp.h"
#include "oled_gif.h"
#include "oled_jpeg.h"
#include "oled_png.h"
#include "oled_picture.h"
#include "oled_driver.h"
#include "oled_main.h"

static int width()
{
	return lcd_get_xres();
}

static int height()
{
	return lcd_get_yres();
}

void set_screen(const uint32_t * data, int wid, int hgt)
{
	int x, y;

	if (wid > width())
		wid = width();
	if (hgt > height())
		hgt = height();

	if (data)
	{
		for (y = 0; y < hgt; y++)
		{
			for (x = 0; x < wid; x++)
			{
				lcd_setpixel(x, y, data[y * wid + x]);
			}
		}
	}
}

bool process_alpha = false;

uint32_t align_alpha(uint32_t col)
{
	switch (col) {
		case 0x00FFFFFF:
		case 0x00000000:
			return col;
		default:
			return (col & 0xFF000000) ? col : (col | 0xFF000000);
	}
}

void draw_pixel(int x, int y, uint32_t color)
{
	if (x < 0 || x > width() - 1)
		return;
	if (y < 0 || y > height() - 1)
		return;

	if (color != 0x00FFFFFF) {
		uint32_t col = align_alpha(color);
		if (process_alpha) {
			uint32_t bg = bitmap[x + (width() * y)];
			uint32_t afg = (col & 0xFF000000) >> 24;
			uint32_t rfg = (col & 0x00FF0000) >> 16;
			uint32_t gfg = (col & 0x0000FF00) >> 8;
			uint32_t bfg = (col & 0x000000FF);

			uint32_t rbg = (bg & 0x00FF0000) >> 16;
			uint32_t gbg = (bg & 0x0000FF00) >> 8;
			uint32_t bbg = (bg & 0x000000FF);

			// calculate colour channels of new colour depending on alpha channel and background colour
			rfg = (rfg * afg + rbg * (255 - afg)) / 255;
			gfg = (gfg * afg + gbg * (255 - afg)) / 255;
			bfg = (bfg * afg + bbg * (255 - afg)) / 255;

			// as we draw from bottom to top, the new colour will always have alpha level == 0xFF
			// (it will serve as background colour for future objects that will be drawn onto the current object)
			col = 0xFF000000 | (rfg << 16) | (gfg << 8) | bfg;
		}
		bitmap[x + (width() * y)] = col;
	}
}

int get_bounding_box(uint32_t *buffer, int width, int height, int *bb_x, int *bb_y, int *bb_w, int *bb_h)
{
	int y_min, y_max, x_min, x_max;
	if (!width || !height) {
		*bb_x = *bb_y = *bb_w = *bb_h = 0;
		return -1;
	}

	y_min = height;
	uint32_t *b = buffer;
	for (int y = 0; y < height; y++) {
		for (int x = 0; x < width; x++, b++)
			if (*b) {
				y_min = y;
				goto out1;
			}
	}
out1:
	y_max = y_min;
	b = buffer + height * width - 1;
	for (int y = height - 1; y_min < y; y--) {
		for (int x = 0; x < width; x++, b--)
			if (*b) {
				y_max = y;
				goto out2;
			}
	}
out2:
	x_min = width;
	for (int x = 0; x < width; x++) {
		b = buffer + x + y_min * width;
		for (int y = y_min; y < y_max; y++, b += width)
			if (*b) {
				x_min = x;
				goto out3;
			}
	}
out3:
	x_max = x_min;
	for (int x = width - 1; x_min < x; x--) {
		b = buffer + x + y_min * width;
		for (int y = y_min; y < y_max; y++, b += width)
			if (*b) {
				x_max = x;
				goto out4;
			}
	}
out4:
	*bb_x = x_min;
	*bb_y = y_min;
	*bb_w = 1 + x_max - x_min;
	*bb_h = 1 + y_max - y_min;

	if (*bb_x < 0)
		*bb_x = 0;
	if (*bb_y < 0)
		*bb_y = 0;

	return 0;
}

int draw_image_f2(fb_pixel_t *s, uint32_t sw, uint32_t sh, uint32_t dx, uint32_t dy, uint32_t dw, uint32_t dh, bool transp, bool maximize)
{
	uint32_t bb_x, bb_y, bb_w, bb_h;

	if (get_bounding_box(s, sw, sh, &bb_x, &bb_y, &bb_w, &bb_h) == FH_ERROR_OK && bb_w && bb_h)
	{
		if (!maximize)
		{
			if (bb_h * dw > bb_w * dh)
			{
				uint32_t dw_new = dh * bb_w / bb_h;
				dx += (dw - dw_new) >> 1;
				dw = dw_new;
			} else {
				uint32_t dh_new = dw * bb_h / bb_w;
				dy += (dh - dh_new) >> 1;
				dh = dh_new;
			}
		}
		for (u_int y = 0; y < dh; y++)
		{
			for (u_int x = 0; x < dw; x++)
			{
				uint32_t pix = *(s + (y * bb_h / dh + bb_y) * sw + x * bb_w / dw + bb_x);
				if (!transp || pix)
				{
					draw_pixel(x + dx, y + dy, pix);
				}
			}
		}
		return 0;
	}
	return -1;
}

unsigned char * _resize(unsigned char *orgin, int ox, int oy, int dx, int dy, bool simple, unsigned char * dst, bool alpha)
{
	unsigned char * cr;
	if (dst == NULL)
	{
		size_t bufsize = dx * dy * ((alpha) ? 4 : 3);
		cr = (unsigned char*) malloc(bufsize);
		if(cr == NULL)
		{
			printf("[OLED] [%s] resize error: malloc\n", __FUNCTION__);
			return orgin;
		}
	} else
		cr = dst;

	if (simple)
	{
		unsigned char *p, *l;
		int i, j, k, ip;
		l = cr;

		for (j = 0; j < dy; j++, l += dx * 3)
		{
			p = orgin + (j * oy / dy * ox * 3);
			for (i = 0, k = 0; i < dx; i++, k += 3)
			{
				ip = i * ox / dx * 3;
				memmove(l + k, p + ip, 3);
			}
		}
	} else {
		unsigned char *p, *q;
		int i, j, k, l, ya, yb;
		int sq, r, g, b, a;

		p = cr;

		int xa_v[dx];
		for (i=0; i < dx; i++)
			xa_v[i] = i * ox / dx;
		int xb_v[dx + 1];
		for (i = 0; i < dx; i++)
		{
			xb_v[i] = (i + 1) * ox / dx;
			if (xb_v[i] >= ox)
				xb_v[i] = ox - 1;
		}

		if (alpha)
		{
			for (j = 0; j < dy; j++)
			{
				ya = j * oy / dy;
				yb = (j + 1) * oy / dy;
				if (yb >= oy)
					yb = oy - 1;
				for (i = 0; i < dx; i++, p += 4)
				{
					for (l = ya, r = 0, g = 0, b = 0, a = 0, sq = 0; l <= yb; l++)
					{
						q = orgin + ((l * ox + xa_v[i]) * 4);
						for (k = xa_v[i]; k <= xb_v[i]; k++, q += 4, sq++)
						{
							r += q[0]; g += q[1]; b += q[2]; a += q[3];
						}
					}
					int sq_tmp = sq ? sq : 1;//avoid division by zero
					p[0] = (uint8_t)(r / sq_tmp);
					p[1] = (uint8_t)(g / sq_tmp);
					p[2] = (uint8_t)(b / sq_tmp);
					p[3] = (uint8_t)(a / sq_tmp);
				}
			}
		} else {
			for (j = 0; j < dy; j++)
			{
				ya = j * oy / dy;
				yb = (j + 1) * oy / dy;
				if (yb >= oy)
					yb = oy - 1;
				for (i = 0; i < dx; i++, p += 3)
				{
					for (l = ya, r = 0, g = 0, b = 0, sq = 0; l <= yb; l++)
					{
						q = orgin + ((l * ox + xa_v[i]) * 3);
						for (k = xa_v[i];k <= xb_v[i]; k++, q += 3, sq++)
						{
							r += q[0]; g += q[1]; b += q[2];
						}
					}
					int sq_tmp = sq ? sq : 1;//avoid division by zero
					p[0] = (uint8_t)(r / sq_tmp);
					p[1] = (uint8_t)(g / sq_tmp);
					p[2] = (uint8_t)(b / sq_tmp);
				}
			}
		}
	}
	free(orgin);
	orgin = NULL;
	return(cr);
}

unsigned char * simple_resize(unsigned char *orgin, int ox, int oy, int dx, int dy)
{
	return _resize(orgin, ox, oy, dx, dy, true, NULL, true);
}

unsigned char * advance_resize(unsigned char *orgin, int ox, int oy, int dx, int dy)
{
	return _resize(orgin, ox, oy, dx, dy, false, NULL, false);
}

inline static unsigned char make8color(unsigned char r, unsigned char g, unsigned char b)
{
	return ((((r >> 5) & 7) << 5) | (((g >> 5) & 7) << 2) | ((b >> 6) & 3));
}

inline static unsigned short make15color(unsigned char r, unsigned char g, unsigned char b)
{
	return ((((r >> 3) & 31) << 10) | (((g >> 3) & 31) << 5) | ((b >> 3) & 31));
}

inline static unsigned short make16color(unsigned char r, unsigned char g, unsigned char b)
{
	return ((((r >> 3) & 31) << 11) | (((g >> 2) & 63) << 5) | ((b >> 3) & 31));
}

void * _convert_rgb2fb(unsigned char *rgbbuff, unsigned long count, int bpp, bool alpha)
{
	unsigned long i;
	void *fbbuff = NULL;
	u_int8_t  *c_fbbuff;
	u_int16_t *s_fbbuff;
	u_int32_t *i_fbbuff;

	switch(bpp)
	{
		case 8:
			c_fbbuff = (unsigned char *) malloc(count * sizeof(unsigned char));
			for (i = 0; i < count; i++)
				c_fbbuff[i] = make8color(rgbbuff[i * 3], rgbbuff[i * 3 + 1], rgbbuff[i * 3 + 2]);
			fbbuff = (void *) c_fbbuff;
			break;
		case 15:
			s_fbbuff = (unsigned short *) malloc(count * sizeof(unsigned short));
			for (i = 0; i < count ; i++)
				s_fbbuff[i] = make15color(rgbbuff[i * 3], rgbbuff[i * 3 + 1], rgbbuff[i * 3 + 2]);
			fbbuff = (void *) s_fbbuff;
			break;
		case 16:
			s_fbbuff = (unsigned short *) malloc(count * sizeof(unsigned short));
			for (i = 0; i < count ; i++)
				s_fbbuff[i] = make16color(rgbbuff[i * 3], rgbbuff[i * 3 + 1], rgbbuff[i * 3 + 2]);
			fbbuff = (void *) s_fbbuff;
			break;
		case 24:
		case 32:
			i_fbbuff = (unsigned int *) malloc(count * sizeof(unsigned int));
			if (i_fbbuff==NULL)
			{
				printf("Error: malloc\n");
				return NULL;
			}
			if (alpha) {
				for (i = 0; i < count ; i++) {
					i_fbbuff[i] = ((rgbbuff[i * 4 + 3] << 24) & 0xFF000000) |
						    ((rgbbuff[i * 4]   << 16) & 0x00FF0000) |
						    ((rgbbuff[i * 4 + 1] <<  8) & 0x0000FF00) |
						    ((rgbbuff[i * 4 + 2])       & 0x000000FF);
				}
			} else {
				int _transp;
				for (i = 0; i < count ; i++) {
					_transp = 0;
					if(rgbbuff[i * 3] || rgbbuff[i * 3 + 1] || rgbbuff[i * 3 + 2])
					_transp = 0xFF;
					i_fbbuff[i] = (_transp << 24) |
						((rgbbuff[i*3]    << 16) & 0xFF0000) |
						((rgbbuff[i*3+1]  <<  8) & 0xFF00) |
						(rgbbuff[i*3+2]          & 0xFF);
				}
			}
			fbbuff = (void *) i_fbbuff;
			break;
		default:
			printf("Unsupported video mode! You've got: %dbpp\n", bpp);
			exit(1);
	}
	return fbbuff;
}

void * convert_rgb2fb(unsigned char *rgbbuff, unsigned long count, int bpp)
{
	return _convert_rgb2fb(rgbbuff, count, bpp, false);
}

void * convert_rgb2fb_alpha(unsigned char *rgbbuff, unsigned long count, int bpp)
{
	return _convert_rgb2fb(rgbbuff, count, bpp, true);
}

int get_size_identified(const char *filename, int *w, int *h, int *type)
{
	int x, y;
	if (fh_bmp_id(filename))
	{
		if (fh_bmp_getsize(filename, &x, &y) == FH_ERROR_OK)
		{
			*w = x;
			*h = y;
			*type = 0;
			return 0;
		}
	}

	if (fh_gif_id(filename))
 	{
		if (fh_gif_getsize(filename, &x, &y) == FH_ERROR_OK)
		{
			*w = x;
			*h = y;
			*type = 1;
			return 0;
		}
	}

	if (fh_jpeg_id(filename))
	{
		if (fh_jpeg_getsize(filename, &x, &y) == FH_ERROR_OK)
		{
			*w = x;
			*h = y;
			*type = 2;
			return 0;
		}
	}

	if (fh_png_id(filename))
	{
		if (fh_png_getsize(filename, &x, &y) == FH_ERROR_OK)
		{
			*w = x;
			*h = y;
			*type = 3;
			return 0;
		}
	}
	return -1;
}

fb_pixel_t * get_image(const char * filename, int width, int height)
{
	int x = 0, y = 0, type = 0, load_ret = 0, bpp = 0;
	unsigned char * buffer = NULL;
	fb_pixel_t * ret = NULL;
	bool identified = false;

	int (*load)(const char *, unsigned char **, int *, int *);

	if (get_size_identified(filename, &x, &y, &type) == FH_ERROR_OK)
	{
		if (type == 0)
		{
			load = fh_bmp_load;
			identified = true;
		}
		else if (type == 1)
		{
			load = fh_gif_load;
			identified = true;
		}
		else if (type == 2)
		{
			load = fh_jpeg_load;
			identified = true;
		}
		else if (type == 3)
		{
			load = fh_png_load;
			identified = true;
		}
	}

	if (x < 1 || y < 1)
	{
		return NULL;
	}

	size_t bufsize = x * y * 4;

	if (identified)
	{
		buffer = (unsigned char *) malloc(bufsize);//x * y * 4
		if (buffer == NULL)
		{
			return NULL;
		}

		load_ret = load(filename, &buffer, &x, &y);

		if (load_ret == FH_ERROR_OK)
		{
			// image size error
			if (width < 1 || height < 1) {
				free(buffer);
				buffer = NULL;
				return NULL;
			}
			// resize only getImage
			if (x != width || y != height)
			{
				if (bpp == 4)
					buffer = simple_resize(buffer, x, y, width, height);
				else
					buffer = advance_resize(buffer, x, y, width, height);
				x = width;
				y = height;
			}
			if (bpp == 4)
				ret = (fb_pixel_t *) convert_rgb2fb_alpha(buffer, x * y, 32);
			else
				ret = (fb_pixel_t *) convert_rgb2fb(buffer, x * y, 32);
		} else {
			free(buffer);
			buffer = NULL;
			return NULL;
		}
	}
	free(buffer);
	buffer = NULL;
	return ret;
}

int draw_image_f1(const char * filename, uint32_t sw, uint32_t sh, uint32_t dw, uint32_t dh, bool transp, bool maximize)
{
	bool res = false;
	if (!dw || !dh)
		return res;
	fb_pixel_t *s = get_image(filename, sw, sh);
	if (s && sw && sh)
		res = draw_image_f2(s, sw, sh, 0, 0, dw, dh, transp, maximize);
	if (s)
		free(s);
	return res;
}

int draw_image(const char * filename, uint32_t dw, uint32_t dh, bool transp, bool maximize)
{
	bool ret = false;
	int sw, sh, type;
	if (get_size_identified(filename, &sw, &sh, &type) == FH_ERROR_OK)
	{
		if (sw && sh)
		{
			ret = draw_image_f1(filename, sw, sh, dw, dh, transp, maximize);
		}
	}
	return ret;
}

int show_image(char *filename)
{
	if (!(bitmap = (unsigned char*) malloc(width() * height()))) {
		printf("%s: Out of memory.\n", filename);
		return -1;
	}

	if (draw_image(filename, width(), height(), false, false) != FH_ERROR_OK)
	{
		printf("%s: Cannot draw image.\n", filename);
		return -1;
	} else {
		set_screen(bitmap, 480, 320);
		lcd_draw_picture();
	}

	return 0;
}

