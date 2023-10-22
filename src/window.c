#include "cake.h"

int window_init(Window *window)
{
	memset(window, 0, sizeof(*window));
	if (SDL_Init(SDL_INIT_VIDEO) < 0) {
		fprintf(stderr, "SDL could not initialize: %s\n",
				SDL_GetError());
		return 1;
	}

	if (TTF_Init() == -1) {
		fprintf(stderr, "TTF could not initialize: %s\n",
				TTF_GetError());
		return 1;
	}
	window->sdl = SDL_CreateWindow("My SDL2 Window",
		SDL_WINDOWPOS_UNDEFINED, SDL_WINDOWPOS_UNDEFINED,
		640, 480,
		SDL_WINDOW_SHOWN);
	if(window->sdl == NULL) {
		fprintf(stderr, "SDL window could not be created: %s\n",
				SDL_GetError());
		return 1;
	}
	window->renderer = SDL_CreateRenderer(window->sdl, -1,
			SDL_RENDERER_ACCELERATED | SDL_RENDERER_PRESENTVSYNC);
	if (window->renderer == NULL) {
		SDL_DestroyWindow(window->sdl);
		fprintf(stderr, "SDL renderer could not be created: %s\n",
				SDL_GetError());
		return 1;
	}

	window->font = TTF_OpenFont("font.ttf", 16);
	if (window->font == NULL) {
		SDL_DestroyRenderer(window->renderer);
		SDL_DestroyWindow(window->sdl);
		fprintf(stderr, "Font 'font.ttf' could not be opened: %s\n",
				TTF_GetError());
		return -1;
	}
	window->keys = SDL_GetKeyboardState(NULL);

	window->text.lines = malloc(8 * sizeof(*window->text.lines));
	if (window->text.lines == NULL) {
		fprintf(stderr, "Failed allocating lines: %s\n",
				strerror(errno));
		return -1;
	}
	memset(&window->text.lines[0], 0, sizeof(*window->text.lines));
	window->text.count = 1;

	window->zoom = 1;
	return 0;
}

static void window_handlekeyboard(Window *window, SDL_KeyboardEvent *key)
{
	struct text *text;
	struct line *line;

	text = &window->text;
	line = &text->lines[text->y];
	switch (key->keysym.sym) {
	case SDLK_BACKSPACE:
		if (text->x == 0)
			break;
		line->count--;
		text->x--;
		memmove(&line->data[text->x],
			&line->data[text->x + 1],
			line->count - text->x);
		line->data[line->count] = '\0';
		break;
	}
}

static void window_inputtext(Window *window, const char *utf8)
{
	struct text *const text = &window->text;
	struct line *const line = &text->lines[text->y];
	const size_t len = strlen(utf8);
	char *const newData = realloc(line->data, line->count + len + 1);
	if (newData == NULL)
		return;
	line->data = newData;
	memmove(&line->data[text->x + len],
		&line->data[text->x],
		line->count - text->x);
	memcpy(&line->data[text->x], utf8, len);
	text->x += len;
	line->count += len;
	line->data[line->count] = '\0';
}

static void drawContour(SDL_Renderer *renderer, Sint32 x, Sint32 y, int edge) {
	SDL_SetRenderDrawColor(renderer, 255, 255, 255, 255);
	switch (edge) {
	case 1:
		SDL_RenderDrawLine(renderer, x, y, x + 1, y);
		break;
	case 2:
		SDL_RenderDrawLine(renderer, x, y, x, y + 1);
		break;
	case 3:
		SDL_RenderDrawLine(renderer, x, y, x + 1, y);
		SDL_RenderDrawLine(renderer, x, y, x, y + 1);
		break;
	case 4:
		break;
	}
}

static void window_render(Window *window)
{
	struct text *text;
	SDL_Renderer *renderer;
	SDL_Surface *surface;
	SDL_Texture *texture;
	SDL_Color textColor;
	SDL_Rect rect;

	/* draw lines on the left */
	text = &window->text;
	renderer = window->renderer;
	textColor = (SDL_Color) { 205, 140, 0, 255 };
	rect.x = 0;
	rect.y = 0;
	for (size_t i = 0; i < text->count; i++) {
		struct line *const line = &text->lines[i];
		if (line->count == 0)
			continue;
		surface = TTF_RenderUTF8_Solid(window->font, line->data,
				textColor);
		texture = SDL_CreateTextureFromSurface(renderer, surface);
		rect.w = surface->w;
		rect.h = surface->h;
		SDL_RenderCopy(renderer, texture, NULL, &rect);

		rect.y += surface->h;
		SDL_FreeSurface(surface);
		SDL_DestroyTexture(texture);
	}

	/* draw plot on the right */
	number_t f(number_t x, number_t y)
	{
		return -x * x - y;
	}
	for (Sint32 i = 0; i < 640; i++)
		for (Sint32 j = 0; j < 480; j++) {
			number_t x, y;
			Sint32 config;

			x = i / window->zoom + window->translation.x;
			y = j / window->zoom + window->translation.y;
			const number_t cells[4] = {
				f(x, y),
				f(x + 1, y),
				f(x + 1, y + 1),
				f(x, y + 1),
			};
			config = 0;
			for (Sint32 i = 0; i < 4; i++)
				if (cells[i] <= 0)
					config |= 1 << i;

			switch (config) {
			case 1:
			case 14:
				drawContour(renderer, i, j, 1);
				break;
			case 2:
			case 13:
				drawContour(renderer, i, j, 2);
				break;
			case 3:
			case 12:
				drawContour(renderer, i, j, 3);
				break;
			case 4:
			case 11:
				drawContour(renderer, i, j, 1);
				break;
			case 5:
			case 10:
				drawContour(renderer, i, j, 4);
				break;
			case 6:
			case 9:
				drawContour(renderer, i, j, 2);
				break;
			case 7:
			case 8:
				drawContour(renderer, i, j, 3);
				break;
			}
		}

}

int window_show(Window *window)
{
	Uint64 start, end, ticks;
	SDL_Event event;
	const number_t zoomIncrement = 0.4;
	bool dragging = false;

	SDL_StartTextInput();
	start = SDL_GetTicks64();
	while (1) {
		SDL_SetRenderDrawColor(window->renderer, 0, 0, 0, 0);
		SDL_RenderClear(window->renderer);
		end = SDL_GetTicks64();
		ticks = end - start;
		start = end;
		while (SDL_PollEvent(&event)) {
			switch (event.type) {
			case SDL_QUIT:
				SDL_StopTextInput();
				return 0;
			case SDL_KEYDOWN:
				window_handlekeyboard(window, &event.key);
				break;
			case SDL_MOUSEBUTTONDOWN:
				dragging = true;
				break;
			case SDL_MOUSEBUTTONUP:
				dragging = false;
				break;
			case SDL_MOUSEMOTION:
				if (!dragging)
					break;
				window->translation.x -= event.motion.xrel / window->zoom;
				window->translation.y -= event.motion.yrel / window->zoom;
				break;
			case SDL_MOUSEWHEEL:
				window->translation.x *= window->zoom;
				window->translation.y *= window->zoom;
				window->zoom += zoomIncrement * event.wheel.y;
				window->translation.x /= window->zoom;
				window->translation.y /= window->zoom;
				break;
			case SDL_TEXTINPUT:
				window_inputtext(window, event.text.text);
				break;
			}
		}
		window_render(window);
		SDL_RenderPresent(window->renderer);
	}
}
