#include "palette.h"

HWND hwndPalette;
DWORD paletteBuffer[0x100];
DWORD paletteAnimBuffer[0x1000];

void loadPalette() {
	//Load palette
	memset(paletteBuffer,0,0x100*sizeof(DWORD));
	//TODO
	//Load palette animation
	memset(paletteAnimBuffer,0xFF,0x1000*sizeof(DWORD));
	//TODO
}
void updatePalette(int frame) {
	int frameOffset = (frame&0xF)<<8;
	for(int i=0; i<0x100; i++) {
		DWORD color = paletteAnimBuffer[frameOffset|i];
		if(color!=0xFFFF) {
			paletteBuffer[i] = color;
		}
	}
}

///////////////////
//WINDOW FUNCTION//
///////////////////
LRESULT CALLBACK WndProc_Palette(HWND hwnd,UINT msg,WPARAM wParam,LPARAM lParam) {
	switch(msg) {
		//TODO
	}
}

