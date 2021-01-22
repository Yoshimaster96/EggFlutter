#ifndef SPRITE_H
#define SPRITE_H

//Includes
#include "background.h"

//Structs
typedef struct {
	BYTE props;
	DWORD tile;
	int offsX,offsY;
	int numOccupiedTiles;
	int occupiedTiles[4];
} sprite_tile_t;
typedef struct {
	BYTE data[8];
	int dataSize;
	bool selected;
	std::vector<sprite_tile_t> tiles;
} sprite_t;
typedef struct {
	std::vector<sprite_t> sprites;
	std::vector<sprite_t*> assocSprites[0x8000];
	bool invalidSprites[0x8000];
} level_sprite_data_ctx_t;

//Variables
extern HWND hwndSprite;

//Functions
//Sprite drawing
int setSpriteContext(int ctx);
void drawSprites();
void dispSprites(DWORD * pixelBuf,int width,int height);
//Sprite management
void loadSprites(BYTE * data);
int saveSprites(BYTE * data);
int selectSprites(RECT rect,bool ctrl);
void insertSprites(int x,int y);
void deleteSprites();
void moveSprites(int dx,int dy);
//Window
LRESULT CALLBACK WndProc_Sprite(HWND hwnd,UINT msg,WPARAM wParam,LPARAM lParam);

#endif

