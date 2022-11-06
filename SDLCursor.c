// Phragware SDL Cursor Designer
SDL_Cursor* SetSDLCursor(void)
{
 char* cursor[] =
{
"X                               "
"XX                              "
"X.X                             "
"X..X                            "
"X.X.X                           "
"X.XX.X                          "
"X.XXX.X                         "
"X.XXXX.X                        "
"X.XXXXX.X                       "
"X.XXXXXX.X                      "
"X.XXXXXXX.X                     "
"X.XXXXXXXX.X                    "
"X.XXXXX.XXX.X                   "
"X.XXXX.X..XX.X                  "
"X.XXX.X XX..X.X                 "
"X.XXX.X   XX...X                "
"X.XX.X      XX..X               "
"X.XX.X        XX.X              "
"X.X.X           XX              "
"X..X                            "
"X..X                            "
"X.X                             "
"XX                              "
"                                "
"                                "
"                                "
"                                "
"                                "
"                                "
"                                "
"                                "
"                                "
"0,0"

 };
 
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
 switch (cursor[row][col])
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
 
 SDL_sscanf(cursor[row], "%d,%d", &hot_x, &hot_y);
 SDL_Cursor* new_cursor = SDL_CreateCursor(data, mask, 32, 32, hot_x, hot_y);
 SDL_ShowCursor(SDL_ENABLE);
 SDL_SetCursor(new_cursor);
 return new_cursor;
 }
