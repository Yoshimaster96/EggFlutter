#ifndef MAP8_H
#define MAP8_H

//Includes
#include "palette.h"

//Variables
extern HWND hwndMap8;

//Functions
void loadMap8();
void updateMap8(int frame);
void dispMap8Tile(DWORD * pixelBuf,int width,int height,BYTE props,WORD tile,POINT offs);
void dispText(DWORD * pixelBuf,int width,int height,char * text,POINT offs);
//Window
LRESULT CALLBACK WndProc_Map8(HWND hwnd,UINT msg,WPARAM wParam,LPARAM lParam);

#endif

