#include <stdio.h>
#include <SDL.h>
#include <SDL_image.h>

int main(int argc, char *argv[]) 
{
	SDL_Window *window = NULL;
	SDL_Renderer *renderer = NULL;

	SDL_Init(SDL_INIT_EVERYTHING);
	IMG_Init(IMG_INIT_PNG);

	window = SDL_CreateWindow("spranim", SDL_WINDOWPOS_UNDEFINED, SDL_WINDOWPOS_UNDEFINED, 1280, 720, SDL_WINDOW_SHOWN);
	if (window == NULL)
	{
		printf("Failed to create window: %s\n", SDL_GetError());
		return 1;
	}

	renderer = SDL_CreateRenderer(window, -1, SDL_RENDERER_ACCELERATED);

	SDL_Texture *texture = IMG_LoadTexture(renderer, "sheet.png");

	int running = 1;
	while (running)
	{
		SDL_Event e;
		while (SDL_PollEvent(&e))
		{
			switch (e.type)
			{
			case SDL_QUIT:
				running = 0;
			break;
			}
		}

		SDL_SetRenderDrawColor(renderer, 4, 32, 39, 255);
		SDL_RenderClear(renderer);

		Uint64 ticks = SDL_GetTicks64();
		int p = (ticks / 1000) % 4;

		SDL_Rect src;
		src.x = p * 16;
		src.y = 0;
		src.w = 16;
		src.h = 16;

		SDL_Rect dst;
		dst.x = 50;
		dst.y = 50;
		dst.w = 64;
		dst.h = 64;

		SDL_RenderCopy(renderer, texture, &src, &dst);

		SDL_RenderPresent(renderer);
	}

	SDL_DestroyWindow(window);
	SDL_Quit();
	return 0;
}
