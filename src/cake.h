#ifndef INCLUDED_CAKE_H
#define INCLUDED_CAKE_H

#include <ctype.h>
#include <errno.h>
#include <math.h>
#include <stdbool.h>
#include <stdlib.h>
#include <string.h>
#include <stdio.h>
#include <wchar.h>
#include <wctype.h>

#define ARRLEN(a) (sizeof(a)/sizeof*(a))
#define MAX(a, b) ({ \
	__auto_type _a = (a); \
	__auto_type _b = (b); \
	_a > _b ? _a : _b; \
})
#define MIN(a, b) ({ \
	__auto_type _a = (a); \
	__auto_type _b = (b); \
	_a < _b ? _a : _b; \
})

typedef long double number_t;

typedef struct vector {
	number_t x;
	number_t y;
} Vector;

#include <SDL2/SDL.h>
#include <SDL2/SDL_ttf.h>

#include "math.h"
#include "window.h"

#endif
