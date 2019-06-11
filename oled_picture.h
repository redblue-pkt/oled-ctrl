#ifndef _OLED_PICTURE_H_
#define _OLED_PICTURE_H_

#include "oled_driver.h"

#define fb_pixel_t uint32_t

static fb_pixel_t * bitmap = NULL;
static fb_pixel_t *fbbuff = NULL;

#ifndef min
#define min(a,b) ((a) < (b) ? (a) : (b))
#endif
#ifndef max
#define max(a,b) ((a) > (b) ? (a) : (b))
#endif

void set_screen(const uint32_t * data, int wid, int hgt);
uint32_t align_alpha(uint32_t col);
void _draw_pixel(int x, int y, uint32_t color);
int get_bounding_box(uint32_t *buffer, int width, int height, int *bb_x, int *bb_y, int *bb_w, int *bb_h);
int draw_image_f2(fb_pixel_t *s, uint32_t sw, uint32_t sh, uint32_t dx, uint32_t dy, uint32_t dw, uint32_t dh, bool transp, bool maximize);
unsigned char * _resize(unsigned char *orgin, int ox, int oy, int dx, int dy, bool simple, unsigned char * dst, bool alpha);
unsigned char * simple_resize(unsigned char *orgin, int ox, int oy, int dx, int dy);
unsigned char * advance_resize(unsigned char *orgin, int ox, int oy, int dx, int dy);
void * _convert_rgb2fb(unsigned char *rgbbuff, unsigned long count, int bpp, bool alpha);
void * convert_rgb2fb(unsigned char *rgbbuff, unsigned long count, int bpp);
void * convert_rgb2fb_alpha(unsigned char *rgbbuff, unsigned long count, int bpp);
int get_size_identified(const char *filename, int *w, int *h, int *type);
fb_pixel_t * get_image(const char * filename, int width, int height);
int draw_image_f1(const char * filename, uint32_t sw, uint32_t sh, uint32_t dw, uint32_t dh, bool transp, bool maximize);
int draw_image(const char * filename, uint32_t dw, uint32_t dh, bool transp, bool maximize);
int show_image(char *filename);

#endif
