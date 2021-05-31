#ifndef SPRITE_H
#define SPRITE_H

//Includes
#include "background.h"

//Structs
typedef struct {
	BYTE props;
	DWORD tile;
	int offsX,offsY;
} sprite_tile_t;
typedef struct {
	BYTE data[8];
	int dataSize;
	bool selected;
	std::vector<sprite_tile_t> tiles;
} sprite_t;
typedef struct {
	std::vector<sprite_t> sprites;
} level_sprite_data_ctx_t;

//Variables
extern HWND hwndSprite;
extern bool wvisSprite;

//Functions
//Sprite drawing
int setSpriteContext(int ctx);
void drawSprites();
void dispSprites(DWORD * pixelBuf,int width,int height,RECT rect);
void initOtherSpriteBuffers();
//Sprite management
void loadSprites(BYTE * data);
int saveSprites(BYTE * data);
int selectSprites(RECT rect);
void clearSpriteSelection();
void insertSprites(int x,int y);
void deleteSprites();
void moveSprites(int dx,int dy);
int focusSprite(int x,int y,WORD * cursor);
//Window
LRESULT CALLBACK WndProc_Sprite(HWND hwnd,UINT msg,WPARAM wParam,LPARAM lParam);

#endif

