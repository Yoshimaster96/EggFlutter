#include "map8.h"

HWND hwndMap8;
BYTE map8Buffer[0x19000];
BYTE fontBuffer[0x2000];

inline DWORD getLZ1Address(int gfxFile) {
	//TODO
}
inline DWORD getLZ16Address(int gfxFile) {
	//TODO
}

void loadMap8() {
	BYTE tempBuffer[0x8000];
	//Load BG1 tiles
	//TODO
	//Load BG2 tiles
	//TODO
	//Load BG3 tiles
	//TODO
	//Load sprite tiles
	//TODO
}
void updateMap8(int frame) {
	//TODO
}

void dispMap8Tile(DWORD * pixelBuf,int width,int height,BYTE props,WORD tile,POINT offs) {
	//TODO
}
void dispText(DWORD * pixelBuf,int width,int height,char * text,POINT offs) {
	//TODO
}

///////////////////
//WINDOW FUNCTION//
///////////////////
LRESULT CALLBACK WndProc_Map8(HWND hwnd,UINT msg,WPARAM wParam,LPARAM lParam) {
	switch(msg) {
		//TODO
	}
}

