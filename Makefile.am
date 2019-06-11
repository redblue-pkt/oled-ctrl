AM_CPPFLAGS = \
	@FREETYPE_CFLAGS@

ACLOCAL_AMFLAGS = -I m4

bin_PROGRAMS = oled

oled_SOURCES = oled_picture.c oled_bmp.c oled_gif.c oled_jpeg.c oled_png.c oled_freetype.c oled_driver.c oled_main.c

oled_LDADD = -lfreetype -lpng -ljpeg -lgif
