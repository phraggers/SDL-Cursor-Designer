
#include <SDL2/SDL.h>

static int Width = 1200;
static int Height = 800;
static int CharW = 36;
static int CharH = 73;
static int SheetW = 720;
static int SheetH = 365;

typedef struct { int X; int Y; } XY;

typedef struct
{
	SDL_Window* Window;
	SDL_Renderer* Renderer;
	SDL_Texture* FontSprite;
	XY FontMap[0xff];
} S_State;
S_State* State;

void FillFontMap()
{
	if (!State) return;

	char* TextMap = SDL_malloc(100);
	if (!TextMap) return;
	XY* TextMapXY = SDL_malloc(100);
	if (!TextMapXY)
	{
		SDL_free(TextMap);
		return;
	}

	for (int i = 0; i < 10; i++)  TextMap[i] = 0x30 + i; //0-9
	for (int i = 10; i < 36; i++) TextMap[i] = 0x61 + (i - 10); // a-z
	for (int i = 36; i < 62; i++) TextMap[i] = 0x41 + (i - 36); // A-Z
	for (int i = 93; i < 100; i++) TextMap[i] = '\0'; // blanks
	TextMap[62] = '!'; TextMap[63] = '@'; TextMap[64] = '#'; TextMap[65] = '$';
	TextMap[66] = '%'; TextMap[67] = '^'; TextMap[68] = '&'; TextMap[69] = '*';
	TextMap[70] = '('; TextMap[71] = ')'; TextMap[72] = '-'; TextMap[73] = '=';
	TextMap[74] = '_'; TextMap[75] = '+'; TextMap[76] = '['; TextMap[77] = ']';
	TextMap[78] = '{'; TextMap[79] = '}'; TextMap[80] = ';'; TextMap[81] = '\'';
	TextMap[82] = '\\'; TextMap[83] = ':'; TextMap[84] = '\"'; TextMap[85] = '|';
	TextMap[86] = ','; TextMap[87] = '.'; TextMap[88] = '/'; TextMap[89] = '<';
	TextMap[90] = '>'; TextMap[91] = '?'; TextMap[92] = ' ';

	int Yinc = -1;
	for (int i = 0; i < 100; i++)
	{
		if (i % 20 == 0) Yinc++;
		TextMapXY[i].X = CharW * (i % 20);
		TextMapXY[i].Y = CharH * Yinc;
	}

	for (int i = 0; i < 0xff; i++)
	{
		for (int j = 0; j < 100; j++)
		{
			if (i == TextMap[j])
			{
				State->FontMap[i].X = TextMapXY[j].X;
				State->FontMap[i].Y = TextMapXY[j].Y;
				break;
			}
		}
	}

	SDL_free(TextMap);
	SDL_free(TextMapXY);
}

void Text(char* _Text, int _x, int _y, float _Size)
{
	int Length = 0;
	{
		char* c = _Text;
		while (*c != 0)
		{
			Length++;
			c++;
		}
	}

	for (int i = 0; i < Length; i++)
	{
		SDL_Rect SrcRect = {State->FontMap[_Text[i]].X, State->FontMap[_Text[i]].Y, CharW, CharH};
		SDL_Rect DstRect = { _x + ((int)((float)CharW * _Size) * i), _y,
			(int)((float)CharW * _Size), (int)((float)CharH * _Size)};
		SDL_RenderCopy(State->Renderer, State->FontSprite, &SrcRect, &DstRect);
	}

}

int main(int argc, char** argv)
{
	if (SDL_Init(SDL_INIT_EVERYTHING) < 0)
	{
		SDL_Log(SDL_GetError());
		return -1;
	}

	State = SDL_malloc(sizeof(S_State));
	if (!State) return -1;
	SDL_memset(State, 0, sizeof(S_State));

	State->Window = SDL_CreateWindow("SDLCursorDesigner", SDL_WINDOWPOS_CENTERED,
									 SDL_WINDOWPOS_CENTERED, Width, Height,
									 SDL_WINDOW_SHOWN | SDL_WINDOW_ALLOW_HIGHDPI);

	if (!State->Window)
	{
		SDL_Log(SDL_GetError());
		return -1;
	}

	State->Renderer = SDL_CreateRenderer(State->Window, -1, SDL_RENDERER_ACCELERATED | SDL_RENDERER_PRESENTVSYNC);

	if (!State->Renderer)
	{
		SDL_Log(SDL_GetError());
		return -1;
	}

	FillFontMap();

	SDL_Surface* FontSpriteSurface = SDL_LoadBMP("FontSpriteCascadiaMonoBold_36-73.bmp");

	if (!FontSpriteSurface)
	{
		SDL_Log(SDL_GetError());
		return -1;
	}

	State->FontSprite = SDL_CreateTextureFromSurface(State->Renderer, FontSpriteSurface);

	if (!State->FontSprite)
	{
		SDL_Log(SDL_GetError());
		return -1;
	}

	SDL_FreeSurface(FontSpriteSurface);

	int Running = 1;

	while (Running)
	{
		SDL_Event Event;
		while (SDL_PollEvent(&Event))
		{
			if (Event.type == SDL_QUIT)
			{
				Running = 0;
			}
		}

		SDL_SetRenderDrawColor(State->Renderer, 0, 0, 0, 0xff);
		SDL_RenderClear(State->Renderer);

		Text("Phraggers' SDL Cursor Designer", (Width / 2) - (((int)((float)CharW * 0.5f) * 30) / 2), 10, 0.5f);

		SDL_RenderPresent(State->Renderer);
	}

	SDL_DestroyTexture(State->FontSprite);
	SDL_DestroyRenderer(State->Renderer);
	SDL_DestroyWindow(State->Window);
	SDL_free(State);
	SDL_Quit();
	return 0;
}