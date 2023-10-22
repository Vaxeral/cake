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
		fprintf(stderr, "Window could not be created: %s\n",
				SDL_GetError());
		goto err;
	}
	window->renderer = SDL_CreateRenderer(window->sdl, -1,
			SDL_RENDERER_ACCELERATED | SDL_RENDERER_PRESENTVSYNC);
	if (window->renderer == NULL) {
		fprintf(stderr, "Renderer could not be created: %s\n",
				SDL_GetError());
		goto err;
	}

	window->font = TTF_OpenFont("font.ttf", 16);
	if (window->font == NULL) {
		fprintf(stderr, "'font.ttf' could not be opened: %s\n",
				TTF_GetError());
		goto err;
	}
	window->keys = SDL_GetKeyboardState(NULL);

	window->text.lines = malloc(8 * sizeof(*window->text.lines));
	if (window->text.lines == NULL) {
		fprintf(stderr, "Failed allocating lines: %s\n",
				strerror(errno));
		goto err;
	}
	memset(&window->text.lines[0], 0, sizeof(*window->text.lines));
	window->text.count = 1;

	window->plot = SDL_CreateRGBSurface(0, 640, 480, 32, 0, 0, 0, 0);
	if (window->plot == NULL) {
		fprintf(stderr, "Failed creating plot surface: %s\n",
				SDL_GetError());
		goto err;
	}

	window->zoom = 1;
	return 0;
err:
	SDL_DestroyRenderer(window->renderer);
	SDL_DestroyWindow(window->sdl);
	SDL_FreeSurface(window->plot);
	free(window->text.lines);
	return -1;
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

static void window_render(Window *window)
{
	SDL_Renderer *renderer;
	struct text *text;
	SDL_Surface *surface;
	SDL_Texture *texture;
	SDL_Color textColor;
	SDL_Rect rect;
	SDL_Surface *plot;
	Uint32 dark, light;
	Uint32 *pixels;
	number_t invZoom;
	Sint32 tx, ty;
	Sint32 cellSize;

	renderer = window->renderer;

	/* draw lines on the left */
	text = &window->text;
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
	plot = window->plot;
	SDL_LockSurface(plot);
	dark = SDL_MapRGB(plot->format, 0, 60, 255);
	light = SDL_MapRGB(plot->format, 0, 60, 155);
	pixels = plot->pixels;
	for (size_t i = 0; i < (size_t) plot->w * (size_t) plot->h; i++)
		pixels[i] = SDL_MapRGB(plot->format, 14, 10, 25);
	invZoom = 1 / window->zoom;

	cellSize = 96 * window->zoom;
	if (cellSize < 64 || cellSize >= 128)
		cellSize = 64 + cellSize % 64;

	tx = (Sint32) (window->zoom * window->translation.x) % cellSize;
	ty = (Sint32) (window->zoom * window->translation.y) % cellSize;
	if (window->translation.x > 0)
		tx -= cellSize;
	if (window->translation.y > 0)
		ty -= cellSize;
	for (Sint32 i = 0; i < plot->w / cellSize; i++) {
		const Sint32 x = i * cellSize - tx;
		for (Sint32 j = 0; j < plot->h; j++) {
			pixels[x + j * plot->w] = dark;
			for (Sint32 n = 1; n < 4; n++) {
				const Sint32 nx = x + n * cellSize / 4;
				if (nx < 0 || nx >= plot->w)
					continue;
				pixels[nx + j * plot->w] = light;
			}
		}
	}
	for (Sint32 i = 0; i < plot->h / cellSize; i++) {
		const Sint32 y = i * cellSize - ty;
		for (Sint32 j = 0; j < plot->w; j++) {
			pixels[j + y * plot->w] = dark;
			for (Sint32 n = 1; n < 4; n++) {
				const Sint32 ny = y + n * cellSize / 4;
				if (ny < 0 || ny >= plot->h)
					continue;
				pixels[j + ny * plot->w] = light;
			}
		}
	}

	number_t f(number_t x, number_t y)
	{
		return x * x - y;
	}
	for (Sint32 i = 0; i < plot->w; i++)
		for (Sint32 j = 0; j < plot->h; j++) {
			number_t x, y;
			Sint32 config;

			x = i * invZoom + window->translation.x;
			y = j * invZoom + window->translation.y;
			const number_t cells[4] = {
				f(x, y),
				f(x + invZoom, y),
				f(x + invZoom, y + invZoom),
				f(x, y + invZoom),
			};
			config = 0;
			for (Sint32 i = 0; i < 4; i++)
				if (cells[i] <= 0)
					config |= 1 << i;
			if (config == 0 || config == 15)
				continue;
			pixels[i + j * plot->w] =
				SDL_MapRGB(plot->format, 0, 255, 0);
		}
	SDL_UnlockSurface(plot);
	rect = (SDL_Rect) { 0, 0, plot->w, plot->h };
	texture = SDL_CreateTextureFromSurface(renderer, plot);
	SDL_RenderCopy(renderer, texture, NULL, &rect);
	SDL_DestroyTexture(texture);
}

int window_show(Window *window)
{
	Uint64 start, end, ticks;
	SDL_Event event;
	const number_t zoomIncrement = 0.4;
	const number_t zoomFactor = 1.1;
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
				window->zoom += window->zoom * zoomIncrement *
					event.wheel.y;
				window->zoom *= event.wheel.y > 0 ?
					zoomFactor : 1 / zoomFactor;
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
