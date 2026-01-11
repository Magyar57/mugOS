#ifndef __FONT_H__
#define __FONT_H__

#include <stdint.h>

// Bitmaps
#define BITMAP_CHAR_HEIGHT	16
#define BITMAP_CHAR_WIDTH	8
#define BITMAP_N_CHAR		256 // ASCII characters

extern const uint8_t DEFAULT_FONT[BITMAP_N_CHAR][BITMAP_CHAR_HEIGHT];

#endif
