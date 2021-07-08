#ifndef BACKGROUND_H
#define BACKGROUND_H

//Includes
#include "map8.h"

//Variables
extern HWND hwndBackground;
extern bool wvisBackground;
extern DWORD * bmpDataBg;

//Functions
void loadBackground();
void updateBackground();
//Window
LRESULT CALLBACK WndProc_Background(HWND hwnd,UINT msg,WPARAM wParam,LPARAM lParam);

#endif

