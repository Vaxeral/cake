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
	window->sdl = SDL_CreateWindow(
		"My SDL2 Window",
		SDL_WINDOWPOS_UNDEFINED,
		SDL_WINDOWPOS_UNDEFINED,
		640,
		480,
		SDL_WINDOW_SHOWN
	);
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
		fprintf(stderr, "Font 'font.ttf' could not be opened: %s\n",
				TTF_GetError());
	}
	window->keys = SDL_GetKeyboardState(NULL);
	window->text.lines = malloc(8 * sizeof(*window->text.lines));
	memset(&window->text.lines[0], 0, sizeof(*window->text.lines));
	window->text.count = 1;
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

static void window_render(Window *window)
{
	struct text *text;
	SDL_Renderer *renderer;
	SDL_Surface *surface;
	SDL_Texture *texture;
	SDL_Color textColor;
	SDL_Rect rect;

	text = &window->text;
	renderer = window->renderer;
	textColor = (SDL_Color) { 255, 0, 0, 255 };
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
}

int window_show(Window *window)
{
	Uint64 start, end, ticks;
	SDL_Event event;

	SDL_SetTextInputRect(&(SDL_Rect) { 0, 0, 640, 20 });
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
			case SDL_TEXTINPUT:
				window_inputtext(window, event.text.text);
				break;
			case SDL_KEYDOWN:
				window_handlekeyboard(window, &event.key);
				break;
			}
		}
		window_render(window);
		SDL_RenderPresent(window->renderer);
	}
}
