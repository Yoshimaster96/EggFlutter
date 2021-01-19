#ifndef MAP8_H
#define MAP8_H

//Includes
#include "palette.h"

//Variables
//TODO

//Functions
void loadMap8();
void updateMap8(int frame);
void drawMap8Tile(DWORD * pixelBuf,int width,int height,BYTE props,WORD tile,POINT offs);
//Window
LRESULT CALLBACK WndProc_Map8(HWND hwnd,UINT msg,WPARAM wParam,LPARAM lParam);

#endif

