#include "../src/cake.h"

int main(void)
{
	Window window;

	if (window_init(&window) < 0)
		return -1;
	window_show(&window);
	//window_uninit(&window);
	return 0;
}
