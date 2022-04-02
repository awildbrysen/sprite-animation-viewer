#include <stdio.h>
#include <stdlib.h>
#include <SDL.h>
#include <SDL_image.h>
#include <SDL_ttf.h>

struct GlyphPosition 
{
    int x;
    int y;
};

int GlyphWidth, GlyphHeight;
SDL_Texture *GlyphAtlas;
struct GlyphPosition GlyphMapping[128];

SDL_Texture *createGlyphAtlas(SDL_Renderer *renderer, TTF_Font *font)
{
	SDL_Texture* texture = SDL_CreateTexture(
		renderer,
		SDL_PIXELFORMAT_RGBA32,
		SDL_TEXTUREACCESS_TARGET,
		2048,
		2048
	);
	SDL_SetRenderTarget(renderer, texture);

	for (int i = 0; i < 128; i++) {
		char c = (char)i;

		SDL_Rect rect = { i * GlyphWidth, 0, GlyphWidth, GlyphHeight };
		SDL_Color color = { 255, 255, 255, 255 };
		SDL_Surface* surface = TTF_RenderGlyph_Blended(font, c, color);
		if (!surface)
		{
			printf("Failed rendering to surface: %s\n", SDL_GetError());
			continue;
		}
		SDL_Texture* chartexture = SDL_CreateTextureFromSurface(renderer, surface);
		if (!chartexture)
		{
			printf("Failed creating texture from surface for Glyph Atlas: %s\n", SDL_GetError());
			continue;
		}

		SDL_RenderCopy(renderer, chartexture, NULL, &rect);
		struct GlyphPosition position = { rect.x, rect.y };
		GlyphMapping[i] = position;
	}

	SDL_SetRenderTarget(renderer, NULL);
	return texture;
}

void drawInputRect(SDL_Window *window, SDL_Renderer *renderer, int ww, char *currentText, int textSize, TTF_Font *font)
{
	int r, g, b, a;
	SDL_GetRenderDrawColor(renderer, &r, &g, &b, &a);

	SDL_SetRenderDrawColor(renderer, 255, 0, 0, 255);

	SDL_Rect rect;
	rect.x = ww - 122;
	rect.y = 10;
	rect.w = 120;
	rect.h = 30; // TODO: a bit bigger than GlyphHeight
	SDL_RenderDrawRect(renderer, &rect);

	SDL_SetTextureBlendMode(GlyphAtlas, SDL_BLENDMODE_BLEND);
	
	for (int i = 0; i < textSize; i++)
	{
		char c = currentText[i];
		struct GlyphPosition p = GlyphMapping[(int)c];
		SDL_Rect src = { p.x, p.y, GlyphWidth, GlyphHeight };
		SDL_Rect dst = { (ww - 118) + (i * GlyphWidth), 12, GlyphWidth, GlyphHeight }; // TODO: y should be the position that gives equal space at the top and bottom
		SDL_RenderCopy(renderer, GlyphAtlas, &src, &dst);
	}

	SDL_SetRenderDrawColor(renderer, r, g, b, a);
}

int main(int argc, char *argv[]) 
{
	SDL_Init(SDL_INIT_EVERYTHING);
	IMG_Init(IMG_INIT_PNG);
	if (TTF_Init() == -1)
	{
		printf("TTF_Init failed: %s\n", TTF_GetError());
		return 1;
	}

	SDL_Window *window = SDL_CreateWindow("spranim", SDL_WINDOWPOS_UNDEFINED, SDL_WINDOWPOS_UNDEFINED, 1280, 720, SDL_WINDOW_SHOWN);
	if (window == NULL)
	{
		printf("Failed to create window: %s\n", SDL_GetError());
		return 1;
	}

	SDL_Renderer *renderer = SDL_CreateRenderer(window, -1, SDL_RENDERER_ACCELERATED);

	TTF_Font *f = TTF_OpenFont("font.ttf", 14);
	if (!f)
	{
		printf("Failed to open font: %s\n", TTF_GetError());
		return 1;
	}
	TTF_SizeText(f, "a", &GlyphWidth, &GlyphHeight);

	GlyphAtlas = createGlyphAtlas(renderer, f);

	SDL_Texture *texture = IMG_LoadTexture(renderer, "sheet.png");

	int startTime = SDL_GetTicks();
	int animationRate = 12;
	int animationLength = 4;

	int ww, wh;
	SDL_GetWindowSize(window, &ww, &wh);

	int textSize = 2;
	int maxTextSize = 3;	
	char *text = (char*)malloc(sizeof(char) * textSize);
	strcpy(text, "4");

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
			case SDL_TEXTINPUT:
				if (strlen(text) + strlen(e.text.text) >= maxTextSize) break;
				if (strlen(text) + strlen(e.text.text) >= textSize)
				{
					textSize = maxTextSize;
					text = (char*)realloc(text, textSize);
				}
				strcat(text, e.text.text);
			break;
			case SDL_KEYDOWN:
				if (e.key.keysym.scancode == SDL_SCANCODE_BACKSPACE)
				{
					if (strlen(text) != 0) text[strlen(text) - 1] = '\0';
				}
			break;
			}
		}

		SDL_SetRenderDrawColor(renderer, 4, 32, 39, 255);
		SDL_RenderClear(renderer);

		drawInputRect(window, renderer, ww, text, textSize, f);

		Uint64 ticks = SDL_GetTicks64();
		animationRate = atoi(text);
		int p = ((ticks - startTime) * animationRate / 1000) % 4;

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
	TTF_Quit();
	SDL_Quit();
	return 0;
}
