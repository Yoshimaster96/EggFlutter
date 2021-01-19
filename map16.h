#ifndef MAP16_H
#define MAP16_H

//Includes
#include "map8.h"

//Variables
extern HWND hwndMap16;

//Functions
void loadMap16();
void saveMap16();
void drawMap16Tile(DWORD * pixelBuf,int width,int height,WORD tile,POINT offs);
//Window
LRESULT CALLBACK WndProc_Map16(HWND hwnd,UINT msg,WPARAM wParam,LPARAM lParam);

#endif

