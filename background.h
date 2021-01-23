#ifndef BACKGROUND_H
#define BACKGROUND_H

//Includes
#include "map16.h"

//Variables
extern HWND hwndBackground;
extern bool wvisBackground;
extern BYTE bg2Buffer[0x1000],bg3Buffer[0x800];

//Functions
void loadBackground2();
void loadBackground3();
//Window
LRESULT CALLBACK WndProc_Background(HWND hwnd,UINT msg,WPARAM wParam,LPARAM lParam);

#endif

