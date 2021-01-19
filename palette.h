#ifndef PALETTE_H
#define PALETTE_H

//Includes
#include "globalstate.h"

//Variables
extern HWND hwndPalette;
extern DWORD paletteBuffer[0x100];

//Functions
void loadPalette();
void updatePalette(int frame);
//Window
LRESULT CALLBACK WndProc_Palette(HWND hwnd,UINT msg,WPARAM wParam,LPARAM lParam);

#endif

