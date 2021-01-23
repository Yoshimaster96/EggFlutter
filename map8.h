#ifndef MAP8_H
#define MAP8_H

//Includes
#include "palette.h"

//Variables
extern HWND hwndMap8;
extern bool wvisMap8;

//Functions
DWORD getLZ1Address(int gfxFile);
DWORD getLZ16Address(int gfxFile);
void loadMap8();
void updateMap8(int frame);
void updateMap8Sw(int state);
void dispMap8Tile(DWORD * pixelBuf,int width,int height,BYTE props,WORD tile,POINT offs);
void dispMap8Char(DWORD * pixelBuf,int width,int height,char c,POINT offs);
//Window
LRESULT CALLBACK WndProc_Map8(HWND hwnd,UINT msg,WPARAM wParam,LPARAM lParam);

#endif

