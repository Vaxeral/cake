#include "cake.h"

static const struct symbols {
	const char *word;
	const char *out;
} math_symbols[] = {
	{ "element_of", "∈" },
	{ "intersection", "∩" },
	{ "union", "∪" },
	{ "real_numbers", "ℝ" },
	{ "complex_numbers", "ℂ" },
	{ "integers", "ℤ" },
	{ "whole_numbers", "ℤ⁺" },
	{ "natural_numbers", "ℕ" },
	{ "maps_to", "↦" },
	{ "subset_of", "⊆" },
	{ "implies", "⇒" },
};

int window_init(Window *window)
{
	char *data = NULL;

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

	data = malloc(8);
	if (data == NULL) {
		fprintf(stderr, "Failed initial line data: %s\n",
				strerror(errno));
		goto err;
	}
	data[0] = '\0';
	window->text.lines = malloc(8 * sizeof(*window->text.lines));
	if (window->text.lines == NULL) {
		fprintf(stderr, "Failed allocating lines: %s\n",
				strerror(errno));
		goto err;
	}
	memset(&window->text.lines[0], 0, sizeof(*window->text.lines));
	window->text.lines[0].data = data;
	window->text.count = 1;

	window->plot = SDL_CreateRGBSurface(0, 640, 480, 32, 0, 0, 0, 0);
	if (window->plot == NULL) {
		fprintf(stderr, "Failed creating plot surface: %s\n",
				SDL_GetError());
		goto err;
	}
	window->zoom = 10;
	window->translation = (Vector) {
		-32, -24
	};
	return 0;
err:
	SDL_DestroyRenderer(window->renderer);
	SDL_DestroyWindow(window->sdl);
	SDL_FreeSurface(window->plot);
	free(data);
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
	case SDLK_TAB: {
		char *start, *end;
		size_t len;
		char ch;
		/* get the word we are on */
		start = &line->data[text->x];
		end = start;
		if (text->x > 0) {
			for (; (ch = *(--start)) == '_' || isalpha(ch); );
			start++;
		}
		for (; (ch = *end) == '_' || isalpha(ch); end++);
		len = end - start;
		if (len == 0)
			break;
		for (size_t i = 0; i < ARRLEN(math_symbols); i++) {
			const char *const word = math_symbols[i].word;
			if (strncmp(word, start, end - start) == 0 &&
					word[len] == '\0') {
				const char *const out = math_symbols[i].out;
				const size_t outLen = strlen(out);
				const size_t s = start - line->data;
				const size_t e = end - line->data;
				memmove(start + outLen, end, line->count - e);
				memmove(start, out, outLen);
				line->count -= len - outLen;
				text->x = s + outLen;
				line->data[line->count] = '\0';
				break;
			}
		}
		break;
	}

	case SDLK_RETURN: {
		char *data;

		data = malloc(8);
		if (data == NULL)
			break;
		data[0] = '\0';
		line = realloc(text->lines, sizeof(*text->lines) *
				(text->count + 1));
		if (line == NULL) {
			free(data);
			break;
		}
		text->lines = line;
		text->x = 0;
		text->y++;
		line += text->y;
		memmove(&line[1], &line[0], sizeof(*line) *
				(text->count - text->y));
		line->data = data;
		line->count = 0;
		text->count++;
		break;
	}

	case SDLK_HOME:
		text->x = 0;
		break;
	case SDLK_END:
		text->x = line->count;
		break;
	case SDLK_UP:
		if (text->y > 0) {
			text->y--;
			text->x = text->lines[text->y].count;
		}
		break;
	case SDLK_DOWN:
		if (text->y + 1 != text->count) {
			text->y++;
			text->x = text->lines[text->y].count;
		}
		break;
	case SDLK_LEFT:
		if (text->x > 0)
			text->x--;
		break;
	case SDLK_RIGHT:
		if (text->x != line->count)
			text->x++;
		break;

	case SDLK_BACKSPACE:
		if (text->x == 0) {
			if (line->count > 0 || text->count == 1)
				break;
			text->count--;
			memmove(&line[0], &line[1], sizeof(*line) *
					(text->count - text->y));
			text->x = text->lines[text->y].count;
			break;
		}
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

static void window_renderlines(Window *window)
{
	SDL_Renderer *renderer;
	struct text *text;
	SDL_Surface *surface;
	SDL_Texture *texture;
	SDL_Color textColor;
	SDL_Rect rect;

	renderer = window->renderer;
	text = &window->text;
	textColor = (SDL_Color) { 205, 140, 0, 255 };
	rect.x = 0;
	rect.y = 0;
	for (size_t i = 0; i < text->count; i++) {
		struct line *const line = &text->lines[i];
		int w, h;

		TTF_SizeUTF8(window->font, &line->data[text->x], &w, &h);
		surface = TTF_RenderUTF8_Solid(window->font, line->data,
				textColor);
		if (surface != NULL) {
			texture = SDL_CreateTextureFromSurface(renderer, surface);
			rect.w = surface->w;
			rect.h = surface->h;
			SDL_RenderCopy(renderer, texture, NULL, &rect);
			rect.y += surface->h;
			SDL_FreeSurface(surface);
			SDL_DestroyTexture(texture);
		} else {
			rect.y += h;
			rect.w = w;
			rect.h = h;
		}
		if (i == text->y) {
			const SDL_Rect caret = {
				rect.x + rect.w - w, rect.y - h,
				2, rect.h
			};
			SDL_SetRenderDrawColor(renderer, 200, 200, 200, 255);
			SDL_RenderFillRect(renderer, &caret);
		}
	}
}

static void window_renderplot(Window *window)
{
	SDL_Renderer *renderer;
	SDL_Color textColor;
	SDL_Surface *surface;
	SDL_Texture *texture;
	SDL_Rect rect;
	SDL_Surface *plot;
	Uint32 dark, light;
	Uint32 *pixels;
	number_t invZoom;
	Sint32 tx, ty;
	Sint32 cellSize;
	char buf[800];

	renderer = window->renderer;
	textColor = (SDL_Color) { 205, 140, 0, 255 };
	plot = window->plot;
	SDL_LockSurface(plot);
	dark = SDL_MapRGB(plot->format, 0, 60, 255);
	light = SDL_MapRGB(plot->format, 0, 60, 155);
	pixels = plot->pixels;
	for (size_t i = 0; i < (size_t) plot->w * (size_t) plot->h; i++)
		pixels[i] = SDL_MapRGB(plot->format, 14, 10, 25);
	invZoom = 1 / window->zoom;

	cellSize = 100 * window->zoom;
	cellSize %= 200;
	if (cellSize < 50)
		cellSize = 200 - cellSize;

	tx = (Sint32) (window->zoom * window->translation.x) % cellSize;
	ty = (Sint32) (window->zoom * window->translation.y) % cellSize;
	if (window->translation.x > 0)
		tx -= cellSize;
	if (window->translation.y > 0)
		ty -= cellSize;

	for (Sint32 i = -1; i <= plot->w / cellSize; i++) {
		const Sint32 x = i * cellSize - tx;
		for (Sint32 j = 0; j < plot->h; j++) {
			for (Sint32 n = 0; n < 4; n++) {
				const Sint32 nx = x + n * cellSize / 4;
				if (nx < 0 || nx >= plot->w)
					continue;
				pixels[nx + j * plot->w] = n == 0 ? dark :
					light;
			}
		}
	}

	for (Sint32 i = -1; i <= plot->h / cellSize; i++) {
		const Sint32 y = i * cellSize - ty;
		for (Sint32 j = 0; j < plot->w; j++) {
			for (Sint32 n = 0; n < 4; n++) {
				const Sint32 ny = y + n * cellSize / 4;
				if (ny < 0 || ny >= plot->h)
					continue;
				pixels[j + ny * plot->w] = n == 0 ? dark :
					light;
			}
		}
	}

	number_t f(number_t x, number_t y)
	{
		return x * x + y * y - 9;
	}
	for (Sint32 i = 0; i < plot->w; i++) {
		for (Sint32 j = 0; j < plot->h; j++) {
			number_t x, y;
			Sint32 config;

			x = i * invZoom + window->translation.x;
			y = -(j * invZoom + window->translation.y);
			const number_t cells[4] = {
				f(x, y),
				f(x + invZoom, y),
				f(x + invZoom, y - invZoom),
				f(x, y - invZoom),
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
	}

	SDL_UnlockSurface(plot);

	/* numbers on the x axis */
	for (Sint32 i = -1; i <= plot->w / cellSize; i++) {
		const Sint32 x = i * cellSize - tx;
		sprintf(buf, "%.1LF", x * invZoom + window->translation.x);
		surface = TTF_RenderUTF8_Solid(window->font, buf, textColor);
		rect = (SDL_Rect) {
			.x = x - surface->w / 2,
			.y = -window->translation.y * window->zoom,
			.w = surface->w,
			.h = surface->h,
		};
		if (rect.y < 0)
			rect.y = 0;
		else if (rect.y > plot->h - surface->h)
			rect.y = plot->h - surface->h;
		SDL_BlitSurface(surface, NULL, window->plot, &rect);
		SDL_FreeSurface(surface);
	}

	/* numbers on the y axis */
	for (Sint32 i = -1; i <= plot->h / cellSize; i++) {
		const Sint32 y = i * cellSize - ty;
		sprintf(buf, "%.1LF", y * invZoom + window->translation.y);
		surface = TTF_RenderUTF8_Solid(window->font, buf, textColor);
		rect = (SDL_Rect) {
			.x = -window->translation.x * window->zoom,
			.y = y - surface->h / 2,
			.w = surface->w,
			.h = surface->h,
		};
		if (rect.x < 0)
			rect.x = 0;
		else if (rect.x > plot->w - surface->w)
			rect.x = plot->w - surface->w;
		SDL_BlitSurface(surface, NULL, window->plot, &rect);
		SDL_FreeSurface(surface);
	}

	texture = SDL_CreateTextureFromSurface(renderer, plot);
	rect = (SDL_Rect) { 160, 0, plot->w - 160, plot->h };
	SDL_RenderCopy(renderer, texture, &rect, &rect);
	SDL_DestroyTexture(texture);
}

static void window_render(Window *window)
{
	window_renderlines(window);
	window_renderplot(window);
}

int window_show(Window *window)
{
	Uint64 start, end, ticks;
	SDL_Event event;
	const number_t zoomFactor = 1.1;

	SDL_StartTextInput();
	start = SDL_GetTicks64();
	while (1) {
		SDL_SetRenderDrawColor(window->renderer, 0, 0, 0, 0);
		SDL_RenderClear(window->renderer);
		end = SDL_GetTicks64();
		ticks = end - start;
		start = end;
		(void) ticks; /* TODO: */
		while (SDL_PollEvent(&event)) {
			switch (event.type) {
			case SDL_QUIT:
				SDL_StopTextInput();
				return 0;
			case SDL_KEYDOWN:
				window_handlekeyboard(window, &event.key);
				break;
			case SDL_MOUSEMOTION:
				if (!(event.motion.state & SDL_BUTTON_LMASK))
					break;
				window->translation.x -= event.motion.xrel / window->zoom;
				window->translation.y -= event.motion.yrel / window->zoom;
				break;
			case SDL_MOUSEWHEEL: {
				number_t oldZoom;
				int mx, my;
				number_t x, y;

				oldZoom = window->zoom;
				window->zoom *= event.wheel.y > 0 ?
					zoomFactor : 1 / zoomFactor;
				if (window->zoom < 1e-6)
					window->zoom = 1e-6;

				SDL_GetMouseState(&mx, &my);
				x = mx / oldZoom + window->translation.x;
				y = my / oldZoom + window->translation.y;
				window->translation.x = x - mx / window->zoom;
				window->translation.y = y - my / window->zoom;
				break;
			}
			case SDL_TEXTINPUT:
				window_inputtext(window, event.text.text);
				break;
			}
		}
		window_render(window);
		SDL_RenderPresent(window->renderer);
	}
}
