#ifndef SPRITE_H
#define SPRITE_H

//Includes
#include "background.h"

//Structs
typedef struct {
	BYTE data[8];
	int dataSize;
	bool selected;
	std::vector<int> occupiedTiles;
} sprite_t;
typedef struct {
	BYTE props;
	DWORD tile;
	int offsX,offsY;
	RECT clip;
	sprite_t * assocSprite;
} sprite_tile_t;
typedef struct {
	std::vector<sprite_t> sprites;
	std::vector<sprite_tile_t> tilemap[0x8000];
	BYTE invalid[0x8000];
} level_sprite_data_ctx_t;

//Variables
extern HWND hwndSprite;
extern bool wvisSprite;

//Functions
//Sprite drawing
int setSpriteContext(int ctx);
void drawSprites();
void dispSprites(DWORD * pixelBuf,int width,int height,RECT * rect);
void initOtherSpriteBuffers();
//Sprite management
int loadSprites(BYTE * data);
int saveSprites(BYTE * data);
void getInvalidSprites(BYTE * data);
void selectSprites(RECT * rect);
void clearSpriteSelection();
void insertSprites(int x,int y);
void deleteSprites();
void selectTopSprite(int x,int y);
void moveSprites(int dx,int dy);
//Window
int focusSprite(int x,int y,UINT * cursor,char * text);
LRESULT CALLBACK WndProc_Sprite(HWND hwnd,UINT msg,WPARAM wParam,LPARAM lParam);

#endif

