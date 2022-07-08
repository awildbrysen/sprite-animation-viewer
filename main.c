#include <stdio.h>
#include <stdlib.h>

#include <SDL.h>
#include <SDL_image.h>
#include <SDL_ttf.h>
#include <SDL_syswm.h>

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

void RenderText(SDL_Renderer *renderer, TTF_Font *font, char *text, int x, int y)
{
	for (int i = 0; i < strlen(text); i++)
	{
		char c = text[i];
		struct GlyphPosition p = GlyphMapping[(int)c];
		SDL_Rect src = { p.x, p.y, GlyphWidth, GlyphHeight };
		SDL_Rect dst = { x + (i * GlyphWidth), y, GlyphWidth, GlyphHeight };
		SDL_RenderCopy(renderer, GlyphAtlas, &src, &dst);
	}
}

struct TextInput 
{
	char *currentinput;
	int textsize;
	int maxtextsize;

	char *label;
	int labeltextsize;

	int id;

	int x;
	int y;
	int w;
	int h;
};

void drawTextInput(SDL_Window *window, SDL_Renderer *renderer, TTF_Font *font, struct TextInput *ti, struct TextInput *currentlyFocussedInput, int cursorIndex)
{
	int r, g, b, a;
	SDL_GetRenderDrawColor(renderer, &r, &g, &b, &a);

	if (currentlyFocussedInput != NULL && ti->id == currentlyFocussedInput->id) 
	{
		SDL_SetRenderDrawColor(renderer, 0, 0, 255, 255);
	}
	else
	{
		SDL_SetRenderDrawColor(renderer, 255, 0, 0, 255);
	}

	if ((strlen(ti->currentinput) * GlyphWidth) > ti->w)
	{
		ti->w = ti->textsize * GlyphWidth;
	}

	SDL_Rect rect;
	rect.x = ti->x;
	rect.y = ti->y;
	rect.w = ti->w;
	rect.h = ti->h;

	SDL_RenderDrawRect(renderer, &rect);

	SDL_SetTextureBlendMode(GlyphAtlas, SDL_BLENDMODE_BLEND);
	
	// Render cursor
	if (currentlyFocussedInput != NULL && ti->id == currentlyFocussedInput->id) 
	{
		SDL_Rect cursor;
		cursor.x = rect.x + 4 + cursorIndex * GlyphWidth;
		cursor.y = rect.y + 4;
		cursor.w = GlyphWidth;
		cursor.h = GlyphHeight;

		SDL_SetRenderDrawColor(renderer, 0, 176, 0, 255);
		SDL_RenderFillRect(renderer, &cursor);
	}

	RenderText(renderer, font, ti->currentinput, ti->x + 4, ti->y + 4);


	// draw label
	int labelwidth = (GlyphWidth * ti->labeltextsize) + 4;
	for (int i = 0; i < ti->labeltextsize; i++)
	{
		char c = ti->label[i];
		struct GlyphPosition p = GlyphMapping[(int)c];
		SDL_Rect src = { p.x, p.y, GlyphWidth, GlyphHeight };
		SDL_Rect dst = { (ti->x - labelwidth) + (i * GlyphWidth), ti->y + 4, GlyphWidth, GlyphHeight };
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

	SDL_SysWMinfo sysinfo;
	SDL_VERSION(&sysinfo.version);
	if (!SDL_GetWindowWMInfo(window, &sysinfo))
	{
		printf("Failed to retrieve system info.\n", SDL_GetError());
		return 1;
	}

	HWND windowhandle = sysinfo.info.win.window;

	SDL_Renderer *renderer = SDL_CreateRenderer(window, -1, SDL_RENDERER_ACCELERATED);

	TTF_Font *f = TTF_OpenFont("font.ttf", 14);
	if (!f)
	{
		printf("Failed to open font: %s\n", TTF_GetError());
		return 1;
	}
	TTF_SizeText(f, "a", &GlyphWidth, &GlyphHeight);

	GlyphAtlas = createGlyphAtlas(renderer, f);

	SDL_Texture *texture = IMG_LoadTexture(renderer, "example.png");

	int startTime = SDL_GetTicks();
	int animationRate = 12;
	int animationLength = 4;

	int ww, wh;
	SDL_GetWindowSize(window, &ww, &wh);

	// define text dialog for animation rate
	struct TextInput tiAnimationRate;
	tiAnimationRate.id = 1;

	tiAnimationRate.w = (GlyphWidth * 2) + 8;
	tiAnimationRate.h = GlyphHeight + 8;
	tiAnimationRate.x = ww - tiAnimationRate.w - 16;
	tiAnimationRate.y = 10;

	tiAnimationRate.textsize = 2;
	tiAnimationRate.maxtextsize = 3;
	tiAnimationRate.currentinput = (char*)malloc(sizeof(char) * tiAnimationRate.textsize);
	strcpy(tiAnimationRate.currentinput, "4");

	tiAnimationRate.labeltextsize = 15;
	tiAnimationRate.label = (char*)malloc(sizeof(char) * tiAnimationRate.labeltextsize);
	strcpy(tiAnimationRate.label, "Animation rate");

	// define text dialog for animation frame count
	struct TextInput tiFrameCount;
	tiFrameCount.id = 2;

	tiFrameCount.w = (GlyphWidth * 2) + 8;
	tiFrameCount.h = GlyphHeight + 8;
	tiFrameCount.x = ww - tiFrameCount.w - 16;
	tiFrameCount.y = tiAnimationRate.y + tiAnimationRate.h + 4;

	tiFrameCount.textsize = 2;
	tiFrameCount.maxtextsize = 3;
	tiFrameCount.currentinput = (char*)malloc(sizeof(char) * tiFrameCount.textsize);
	strcpy(tiFrameCount.currentinput, "4");

	tiFrameCount.labeltextsize = 22;
	tiFrameCount.label = (char*)malloc(sizeof(char) * tiFrameCount.labeltextsize);
	strcpy(tiFrameCount.label, "Animation frame count");

	// define text dialog for which file is being opened
	struct TextInput tiFilePath;
	tiFilePath.id = 3;
	
	tiFilePath.textsize = 55;
	tiFilePath.maxtextsize = 244;
	tiFilePath.currentinput = (char*)malloc(sizeof(char) * tiFilePath.textsize);
	strcpy(tiFilePath.currentinput, "D:/Code/spritesheet-animation-sdl/skeleton-idle.png");
	texture = IMG_LoadTexture(renderer, tiFilePath.currentinput);

	tiFilePath.w = tiFilePath.textsize * GlyphWidth;
	tiFilePath.h = GlyphHeight + 8;
	tiFilePath.x = 16 + (GlyphWidth * 5);
	tiFilePath.y = wh - 16 - tiFilePath.h;

	tiFilePath.labeltextsize = 6;
	tiFilePath.label = (char*)malloc(sizeof(char) * tiFilePath.labeltextsize);
	strcpy(tiFilePath.label, "File:");

	struct TextInput tiImageDimension;
	tiImageDimension.id = 4;

	tiImageDimension.textsize = 3;
	tiImageDimension.maxtextsize = 4;
	tiImageDimension.currentinput = (char*)malloc(sizeof(char)*tiImageDimension.textsize);
	strcpy(tiImageDimension.currentinput, "32");

	tiImageDimension.w = (GlyphWidth * 2) + 8;
	tiImageDimension.h = GlyphHeight + 8;
	tiImageDimension.x = ww - tiImageDimension.w - 16;
	tiImageDimension.y = tiFrameCount.y + tiFrameCount.h + 4;

	tiImageDimension.labeltextsize = 16;
	tiImageDimension.label = (char*)malloc(sizeof(char) * tiImageDimension.labeltextsize);
	strcpy(tiImageDimension.label, "Image dimension");


	// bottom-right color picker
	// Square with white or black border and the current background color inside

	SDL_Rect colorpickerRect;
	colorpickerRect.w = 25;
	colorpickerRect.h = 25;
	colorpickerRect.x = ww - colorpickerRect.w - 16;
	colorpickerRect.y = wh - 16 - colorpickerRect.h;

	char *colorpickerLabel = (char*)malloc(sizeof(char) * 12);
	strcpy(colorpickerLabel, "Background:");
			
	SDL_StopTextInput();

	// TODO: This shouldn't just be the text input but all general types of input.
	// 		f.e. the background color should be operatable through this as well
	struct TextInput *currentlyFocussedInput;
	currentlyFocussedInput = NULL;

	int cursorIndex = -1;

	SDL_Color backgroundColor = { 4, 32, 39, 255 };

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
				if (currentlyFocussedInput == NULL) break;
				if (strlen(currentlyFocussedInput->currentinput) + strlen(e.text.text) >= currentlyFocussedInput->maxtextsize) break;
				if (strlen(currentlyFocussedInput->currentinput) + strlen(e.text.text) >= currentlyFocussedInput->textsize)
				{
					currentlyFocussedInput->textsize = currentlyFocussedInput->textsize + 5;
					currentlyFocussedInput->currentinput = (char*)realloc(currentlyFocussedInput->currentinput, currentlyFocussedInput->textsize);
				}

				int addLength = strlen(e.text.text);
				for (int i = strlen(currentlyFocussedInput->currentinput) + 1; i >= cursorIndex + addLength; i--)
				{
					currentlyFocussedInput->currentinput[i] = currentlyFocussedInput->currentinput[i - 1];
				}

				for (int i = 0; i < addLength; i++)
				{
					currentlyFocussedInput->currentinput[cursorIndex + i] = e.text.text[i];
				}
				cursorIndex += 1;

				// trigger reload of the loaded image
				if (currentlyFocussedInput->id == tiFilePath.id)
				{
					tiFilePath.w = tiFilePath.textsize * GlyphWidth; // change size of text input
					// load different image file
					texture = IMG_LoadTexture(renderer, currentlyFocussedInput->currentinput);
				}
			break;
			case SDL_KEYDOWN:
				
				if (currentlyFocussedInput != NULL)
				{
					// Input editing
					
					if (e.key.keysym.scancode == SDL_SCANCODE_LEFT)
					{
						cursorIndex -= 1;
						if (cursorIndex < 0) cursorIndex = 0;
					}
					if (e.key.keysym.scancode == SDL_SCANCODE_RIGHT)
					{
						cursorIndex += 1;
						if (cursorIndex >= strlen(currentlyFocussedInput->currentinput))
						{
							cursorIndex = strlen(currentlyFocussedInput->currentinput);
						}
					}
					
					if (e.key.keysym.scancode == SDL_SCANCODE_BACKSPACE)
					{
						if (strlen(currentlyFocussedInput->currentinput) != 0 && cursorIndex > 0)
						{
							for (int i = 0; i < currentlyFocussedInput->textsize - (cursorIndex - 1); i++)
							{
								if (strlen(currentlyFocussedInput->currentinput) < cursorIndex - 1 + i) continue;
								currentlyFocussedInput->currentinput[cursorIndex - 1 + i] = currentlyFocussedInput->currentinput[cursorIndex + i];
							}
							if (cursorIndex <= strlen(currentlyFocussedInput->currentinput) + 1) 
							{
								currentlyFocussedInput->textsize = currentlyFocussedInput->textsize - 1;
								cursorIndex -= 1;
							}

							// trigger reload of the loaded image, duplicate of :305-:310
							if (currentlyFocussedInput->id == tiFilePath.id)
							{
								tiFilePath.w = tiFilePath.textsize * GlyphWidth; // change size of text input
								// load different image file
								texture = IMG_LoadTexture(renderer, currentlyFocussedInput->currentinput);
							}
						}
					}
					if (e.key.keysym.scancode == SDL_SCANCODE_RETURN)
					{
						currentlyFocussedInput = NULL;
					}
				}

			break;
			case SDL_MOUSEBUTTONDOWN:
				SDL_MouseButtonEvent mbevent = e.button;
				if (mbevent.button == SDL_BUTTON_LEFT)
				{

					if (colorpickerRect.x < mbevent.x && ((colorpickerRect.x + colorpickerRect.w) > mbevent.x))
					{
						if (colorpickerRect.y < mbevent.y && ((colorpickerRect.y + colorpickerRect.h) > mbevent.y))
						{
							// TODO: Open color picker
							backgroundColor = (SDL_Color){ 255, 0, 0, 255 };
							break;
						}
					}

					if (tiAnimationRate.x < mbevent.x && ((tiAnimationRate.x + tiAnimationRate.w) > mbevent.x))
					{
						if (tiAnimationRate.y < mbevent.y && ((tiAnimationRate.y + tiAnimationRate.h) > mbevent.y))
						{
							currentlyFocussedInput = &tiAnimationRate;
						}
						else if (tiFrameCount.y < mbevent.y && ((tiFrameCount.y + tiFrameCount.h) > mbevent.y))
						{
							currentlyFocussedInput = &tiFrameCount;
						} else if (tiImageDimension.y < mbevent.y && ((tiImageDimension.y + tiImageDimension.h) > mbevent.y))
						{
							currentlyFocussedInput = &tiImageDimension;
						}
					}
					else if (tiFilePath.x < mbevent.x && ((tiFilePath.x + tiFilePath.w) > mbevent.x))
					{
						if (tiFilePath.y < mbevent.y && ((tiFilePath.y + tiFilePath.h) > mbevent.y))
						{
							currentlyFocussedInput = &tiFilePath;
						}
					}
					else
					{
						currentlyFocussedInput = NULL;
					}
				}

				if (currentlyFocussedInput != NULL) 
				{
					printf("Currently focussed input: %d\n", currentlyFocussedInput->id);
					int len = strlen(currentlyFocussedInput->currentinput);
					cursorIndex = len - 1;
					SDL_StartTextInput();
				} 
				else 
				{
					printf("No focussed editor\n");
					SDL_StopTextInput();
				}
			break;
			}
		}

		// SDL_SetRenderDrawColor(renderer, 4, 32, 39, 255);
		SDL_SetRenderDrawColor(renderer, backgroundColor.r, backgroundColor.g, backgroundColor.b, backgroundColor.a);
		SDL_RenderClear(renderer);

		drawTextInput(window, renderer, f, &tiAnimationRate, currentlyFocussedInput, cursorIndex);
		drawTextInput(window, renderer, f, &tiFrameCount, currentlyFocussedInput, cursorIndex);
		drawTextInput(window, renderer, f, &tiFilePath, currentlyFocussedInput, cursorIndex);
		drawTextInput(window, renderer, f, &tiImageDimension, currentlyFocussedInput, cursorIndex);

		// Color picker button
		SDL_RenderFillRect(renderer, &colorpickerRect);
		SDL_SetRenderDrawColor(renderer, 255, 255, 255, 255);
		SDL_RenderDrawRect(renderer, &colorpickerRect);

			// Draw label
		int cpll = strlen(colorpickerLabel);
		int labelwidth = (GlyphWidth * cpll) + 4;
		for (int i = 0; i < cpll; i++)
		{
			char c = colorpickerLabel[i];
			struct GlyphPosition p = GlyphMapping[(int)c];
			SDL_Rect src = { p.x, p.y, GlyphWidth, GlyphHeight };
			SDL_Rect dst = { (colorpickerRect.x - labelwidth) + (i * GlyphWidth), colorpickerRect.y + 4, GlyphWidth, GlyphHeight };
			SDL_RenderCopy(renderer, GlyphAtlas, &src, &dst);
		}

		Uint64 ticks = SDL_GetTicks64();
		animationRate = atoi(tiAnimationRate.currentinput);
		int frameCount = atoi(tiFrameCount.currentinput);
		if (frameCount == 0) frameCount = 1;
		int p = ((ticks - startTime) * animationRate / 1000) % frameCount;

		int dimension = atoi(tiImageDimension.currentinput);

		SDL_Rect src;
		src.x = p * dimension;
		src.y = 0;
		src.w = dimension;
		src.h = dimension;

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
