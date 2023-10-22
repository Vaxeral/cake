#ifndef INCLUDED_CAKE_H
#define INCLUDED_CAKE_H

#include <ctype.h>
#include <errno.h>
#include <math.h>
#include <stdbool.h>
#include <stdlib.h>
#include <string.h>
#include <stdio.h>

typedef long double number_t;

typedef struct vector {
	number_t x;
	number_t y;
} Vector;

#include <SDL2/SDL.h>
#include <SDL2/SDL_ttf.h>

#include "window.h"
#include "tokenizer.h"

#endif
