#ifndef MAP8_H
#define MAP8_H

//Includes
#include "palette.h"

//Variables
extern HWND hwndMap8;
extern bool wvisMap8;
extern BYTE fontBuffer[0x2000];

//Functions
DWORD getLZ1Address(int gfxFile);
DWORD getLZ16Address(int gfxFile);
void loadMap8();
void updateMap8();
void updateMap8Sw(int state);
void updateMap8W6(bool dark);
void dispMap8Pixel(DWORD * pixelBuf,int width,int height,DWORD color,int idx,int offsX,int offsY,BYTE inv);
void dispMap8Tile(DWORD * pixelBuf,int width,int height,BYTE props,WORD tile,int offsX,int offsY,BYTE inv);
void dispMap8Char(DWORD * pixelBuf,int width,int height,DWORD fgCol,DWORD bgCol,char c,int offsX,int offsY,BYTE inv);
//Window
LRESULT CALLBACK WndProc_Map8(HWND hwnd,UINT msg,WPARAM wParam,LPARAM lParam);

#endif

