typedef struct window {
	SDL_Window *sdl;
	SDL_Renderer *renderer;
	const Uint8 *keys;
	TTF_Font *font;
	struct text {
		struct line {
			char *data;
			size_t count;
		} *lines;
		size_t count;
		size_t x, y;
	} text;
	Vector translation;
	number_t zoom;
} Window;

int window_init(Window *window);
int window_show(Window *window);
