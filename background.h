#ifndef BACKGROUND_H
#define BACKGROUND_H

//Includes
#include "map16.h"

//Variables
extern HWND hwndBackground;
extern HBITMAP hbmpBg2,hbmpBg3;
extern DWORD * bmpDataBg2,* bmpDataBg3;

//Functions
void loadBackground2();
void loadBackground3();
//Window
LRESULT CALLBACK WndProc_Background(HWND hwnd,UINT msg,WPARAM wParam,LPARAM lParam);

#endif

