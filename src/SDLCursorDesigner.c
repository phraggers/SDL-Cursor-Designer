
#include <SDL2/SDL.h>
#include "font.h"

static int Width = 1200;
static int Height = 800;
static int CharW = 36;
static int CharH = 73;
static int SheetW = 720;
static int SheetH = 365;

typedef struct { int X; int Y; } XY;
typedef struct { Uint8 R; Uint8 G; Uint8 B; } PixelColor;

typedef struct
{
	SDL_Window* Window;
	SDL_Renderer* Renderer;
	SDL_Texture* FontSprite;
	XY FontMap[0xff];
	SDL_Cursor* Cursor;
	SDL_Rect PixelSetArea;
	SDL_Rect PixelSelectors[32][32];
	PixelColor PixelSelColor[32][32];
	char CursorPixels[33][33];
} S_State;
S_State* State;

static int InitFontMap()
{
	if (!State) return 0;

	char* TextMap = SDL_malloc(100);
	if (!TextMap) return 0;
	XY* TextMapXY = SDL_malloc(100);
	if (!TextMapXY)
	{
		SDL_free(TextMap);
		return 0;
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

	return 1;
}

static void Text(char* _Text, int _x, int _y, float _Size)
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

static void InitCursorPixels()
{
	char* arrow[] = {
		 "X                               ",
		 "XX                              ",
		 "X.X                             ",
		 "X..X                            ",
		 "X...X                           ",
		 "X....X                          ",
		 "X.....X                         ",
		 "X......X                        ",
		 "X.......X                       ",
		 "X........X                      ",
		 "X.....XXXXX                     ",
		 "X..X..X                         ",
		 "X.X X..X                        ",
		 "XX  X..X                        ",
		 "X    X..X                       ",
		 "     X..X                       ",
		 "      X..X                      ",
		 "      X..X                      ",
		 "       XX                       ",
		 "                                ",
		 "                                ",
		 "                                ",
		 "                                ",
		 "                                ",
		 "                                ",
		 "                                ",
		 "                                ",
		 "                                ",
		 "                                ",
		 "                                ",
		 "                                ",
		 "                                ",
  "0,0"
	};

	for (int col = 0; col < 33; col++)
	{
		for (int row = 0; row < 33; row++)
		{
			State->CursorPixels[col][row] = arrow[col][row];
		}
	}
}

static void SetCursorPixels()
{
	if (!State) return;

	for (int row = 0; row < 32; row++)
	{
		for (int col = 0; col < 32; col++)
		{
			if (State->PixelSelColor[col][row].R == 0 &&
				State->PixelSelColor[col][row].G == 0 &&
				State->PixelSelColor[col][row].B == 0)
			{
				State->CursorPixels[row][col] = 'X';
			}

			else if (State->PixelSelColor[col][row].R == 0xff &&
					 State->PixelSelColor[col][row].G == 0xff &&
					 State->PixelSelColor[col][row].B == 0xff)
			{
				State->CursorPixels[row][col] = '.';
			}

			else
			{
				State->CursorPixels[row][col] = ' ';
			}
		}
	}
}

static void SetCursor()
{
	if (!State) return;

	int i, row, col;
	Uint8 data[4 * 32];
	Uint8 mask[4 * 32];
	int hot_x, hot_y;

	i = -1;
	for (row = 0; row < 32; ++row)
	{
		for (col = 0; col < 32; ++col)
		{
			if (col % 8)
			{
				data[i] <<= 1;
				mask[i] <<= 1;
			}
			else
			{
				++i;
				data[i] = mask[i] = 0;
			}
			switch (State->CursorPixels[row][col])
			{
				case 'X':
					data[i] |= 0x01;
					mask[i] |= 0x01;
					break;
				case '.':
					mask[i] |= 0x01;
					break;
				case ' ':
					break;
			}
		}
	}

	SDL_sscanf(State->CursorPixels[row], "%d,%d", &hot_x, &hot_y);

	if (State->Cursor) SDL_FreeCursor(State->Cursor);
	State->Cursor = SDL_CreateCursor(data, mask, 32, 32, hot_x, hot_y);
	SDL_ShowCursor(SDL_ENABLE);
	SDL_SetCursor(State->Cursor);
}

static void SetCursorSelClr()
{
	if (!State) return;

	for (int row = 0; row < 32; row++)
	{
		for (int col = 0; col < 32; col++)
		{
			if (State->CursorPixels[row][col] == 'X')
			{
				State->PixelSelColor[col][row].R = 0;
				State->PixelSelColor[col][row].G = 0;
				State->PixelSelColor[col][row].B = 0;
			}

			else if (State->CursorPixels[row][col] == '.')
			{
				State->PixelSelColor[col][row].R = 0xff;
				State->PixelSelColor[col][row].G = 0xff;
				State->PixelSelColor[col][row].B = 0xff;
			}

			else
			{
				State->PixelSelColor[col][row].R = 32;
				State->PixelSelColor[col][row].G = 32;
				State->PixelSelColor[col][row].B = 32;
			}
		}
	}
}

static int MouseInRect(int* _mx, int* _my, SDL_Rect* _rect)
{
	if (*_mx >= _rect->x && *_mx <= _rect->x + _rect->w)
	{
		if (*_my >= _rect->y && *_my <= _rect->y + _rect->h)
		{
			return 1;
		}
		else return 0;
	}
	else return 0;
}

static void OutputCursorCode()
{
	char* filepart1 = "SDL_Cursor* SetSDLCursor(void)\n{\n char* cursor[] =\n{\n";
	char* filepart2 = "\n };\n \n int i, row, col;\n Uint8 data[4 * 32];\n Uint8 mask[4 * 32];\n int hot_x, hot_y;\n \n i = -1;\n for (row = 0; row < 32; ++row)\n {\n for (col = 0; col < 32; ++col)\n {\n if (col % 8)\n {\n data[i] <<= 1;\n mask[i] <<= 1;\n }\n else\n {\n ++i;\n data[i] = mask[i] = 0;\n }\n switch (cursor[row][col])\n {\n case 'X':\n data[i] |= 0x01;\n mask[i] |= 0x01;\n break;\n case '.':\n mask[i] |= 0x01;\n break;\n case ' ':\n break;\n }\n }\n }\n \n SDL_sscanf(cursor[row], \"%d,%d\", &hot_x, &hot_y);\n SDL_Cursor* new_cursor = SDL_CreateCursor(data, mask, 32, 32, hot_x, hot_y);\n SDL_ShowCursor(SDL_ENABLE);\n SDL_SetCursor(new_cursor);\n return new_cursor;\n }\n";

	{ // clear file if exists
		SDL_RWops* fileout = SDL_RWFromFile("SDLCursor.c", "wb");
		if (!fileout)
		{
			SDL_ShowSimpleMessageBox(SDL_MESSAGEBOX_ERROR, "Error!", "Failed to write file!", 0);
			return;
		}
		SDL_RWwrite(fileout, "// Phragware SDL Cursor Designer\n", 1, 33);
		SDL_RWclose(fileout);
	}

	{ // write file
		SDL_RWops* fileout = SDL_RWFromFile("SDLCursor.c", "ab");
		if (!fileout)
		{
			SDL_ShowSimpleMessageBox(SDL_MESSAGEBOX_ERROR, "Error!", "Failed to write file!", 0);
			return;
		}

		SDL_RWwrite(fileout, filepart1, 1, SDL_strlen(filepart1));
		for (int i = 0; i < 33; i++)
		{
			SDL_RWwrite(fileout, "\"", 1, 1);
			SDL_RWwrite(fileout, State->CursorPixels[i], 1, SDL_strlen(State->CursorPixels[i]));
			SDL_RWwrite(fileout, "\",\n", 1, 3);
		}
		SDL_RWwrite(fileout, filepart2, 1, SDL_strlen(filepart2));

		SDL_RWclose(fileout);
	}

	SDL_ShowSimpleMessageBox(SDL_MESSAGEBOX_INFORMATION, "Success!", "Wrote file SDLCursor.c", 0);
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

	if (!InitFontMap())
	{
		SDL_Log("Error Filling Font Map");
		return -1;
	}

	SDL_Surface* FontSpriteSurface;
	{
		SDL_RWops* fontrw = SDL_RWFromMem(font, sizeof(font));
		FontSpriteSurface = SDL_LoadBMP_RW(fontrw, 0);
		SDL_RWclose(fontrw);
	}

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

	State->PixelSetArea.x = (Width / 2) - 320;
	State->PixelSetArea.y = (Height / 2) - 320;
	State->PixelSetArea.w = 640;
	State->PixelSetArea.h = 640;

	for (int row = 0; row < 32; row++)
	{
		for (int col = 0; col < 32; col++)
		{
			State->PixelSelectors[col][row].x = State->PixelSetArea.x + ((State->PixelSetArea.w / 32) * col);
			State->PixelSelectors[col][row].y = State->PixelSetArea.y + ((State->PixelSetArea.h / 32) * row);
			State->PixelSelectors[col][row].w = State->PixelSetArea.w / 32;
			State->PixelSelectors[col][row].h = State->PixelSetArea.h / 32;
		}
	}

	InitCursorPixels();
	SetCursorSelClr();
	SetCursor();

	SDL_Rect ResetBtnRect = { Width - 240, Height - 145 - 64*2 - 10*2, 200, 64 };
	SDL_Rect PreviewBtnRect = { Width - 240, Height - 145 - 64 - 10, 200, 64 };
	SDL_Rect GenerateBtnRect = { Width - 240, Height - 145, 200, 64 };

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

			if (Event.type == SDL_MOUSEBUTTONUP)
			{
				int mx, my;
				SDL_GetMouseState(&mx, &my);

				if (MouseInRect(&mx, &my, &State->PixelSetArea))
				{
					for (int row = 0; row < 32; row++)
					{
						for (int col = 0; col < 32; col++)
						{
							if (MouseInRect(&mx, &my, &State->PixelSelectors[row][col]))
							{
								if (State->PixelSelColor[row][col].R == 0 &&
									State->PixelSelColor[row][col].G == 0 &&
									State->PixelSelColor[row][col].B == 0)
								{
									State->PixelSelColor[row][col].R = 0xff;
									State->PixelSelColor[row][col].G = 0xff;
									State->PixelSelColor[row][col].B = 0xff;
								}

								else if (State->PixelSelColor[row][col].R == 0xff &&
										 State->PixelSelColor[row][col].G == 0xff &&
										 State->PixelSelColor[row][col].B == 0xff)
								{
									State->PixelSelColor[row][col].R = 32;
									State->PixelSelColor[row][col].G = 32;
									State->PixelSelColor[row][col].B = 32;
								}

								else if (State->PixelSelColor[row][col].R == 32 &&
										 State->PixelSelColor[row][col].G == 32 &&
										 State->PixelSelColor[row][col].B == 32)
								{
									State->PixelSelColor[row][col].R = 0;
									State->PixelSelColor[row][col].G = 0;
									State->PixelSelColor[row][col].B = 0;
								}

								break;
							}
						}
					}
				}

				else if (MouseInRect(&mx, &my, &ResetBtnRect))
				{
					InitCursorPixels();
					SetCursorSelClr();
					SetCursor();
				}

				else if (MouseInRect(&mx, &my, &PreviewBtnRect))
				{
					SetCursorPixels();
					SetCursor();
				}

				else if (MouseInRect(&mx, &my, &GenerateBtnRect))
				{
					SetCursorPixels();
					SetCursor();
					OutputCursorCode();
				}
			}
		}

		SDL_SetRenderDrawColor(State->Renderer, 16, 16, 16, 0xff);
		SDL_RenderClear(State->Renderer);

		Text("Phragware SDL Cursor Designer", (Width / 2) - (((int)((float)CharW * 0.5f) * 29) / 2), 10, 0.5f);
		SDL_SetRenderDrawColor(State->Renderer, 0, 0, 0, 0xff);
		SDL_RenderFillRect(State->Renderer, &State->PixelSetArea);

		for (int row = 0; row < 32; row++)
		{
			for (int col = 0; col < 32; col++)
			{
				SDL_SetRenderDrawColor(State->Renderer, State->PixelSelColor[col][row].R, State->PixelSelColor[col][row].G, State->PixelSelColor[col][row].B, 0xff);
				SDL_RenderFillRect(State->Renderer, &State->PixelSelectors[col][row]);
			}
		}

		// Reset
		SDL_SetRenderDrawColor(State->Renderer, 100, 12, 5, 0xff);
		SDL_RenderFillRect(State->Renderer, &ResetBtnRect);
		Text("Reset", ResetBtnRect.x + (ResetBtnRect.w / 2) - (((int)((float)CharW * 0.5f) * 5) / 2),
			 ResetBtnRect.y + 14, 0.5f);

		// Preview
		SDL_SetRenderDrawColor(State->Renderer, 10, 52, 32, 0xff);
		SDL_RenderFillRect(State->Renderer, &PreviewBtnRect);
		Text("Preview", PreviewBtnRect.x + (PreviewBtnRect.w / 2) - (((int)((float)CharW * 0.5f) * 7) / 2),
			 PreviewBtnRect.y + 14, 0.5f);

		// Generate
		SDL_SetRenderDrawColor(State->Renderer, 0, 64, 12, 0xff);
		SDL_RenderFillRect(State->Renderer, &GenerateBtnRect);
		Text("Generate", GenerateBtnRect.x + (GenerateBtnRect.w / 2) - (((int)((float)CharW * 0.5f) * 8) / 2),
			 GenerateBtnRect.y + 14, 0.5f);

		SDL_RenderPresent(State->Renderer);
	}

	SDL_DestroyTexture(State->FontSprite);
	SDL_DestroyRenderer(State->Renderer);
	SDL_DestroyWindow(State->Window);
	SDL_free(State);
	SDL_Quit();
	return 0;
}