typedef struct window {
	SDL_Window *sdl;
	SDL_Renderer *renderer;
	SDL_Surface *plot;
	const Uint8 *keys;
	TTF_Font *font;
	struct text {
		struct line {
			char *data;
			size_t count;
			/* either variable or function */
			size_t address;
		} *lines;
		size_t count;
		size_t x, y;
	} text;
	Vector translation;
	number_t zoom;
	MathContext math;
} Window;

int window_init(Window *window);
int window_show(Window *window);
