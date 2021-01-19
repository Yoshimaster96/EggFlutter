#include "map16.h"

HWND hwndMap16;
BYTE map16Buffer[0x80000];
DWORD pageBehaviorBuffer[0x100];

void loadMap16() {
	//TODO

}
void saveMap16() {
	//TODO
}
void drawMap16Tile(DWORD * pixelBuf,int width,int height,WORD tile,POINT offs) {
	//TODO
}

///////////////////
//WINDOW FUNCTION//
///////////////////
LRESULT CALLBACK WndProc_Map16(HWND hwnd,UINT msg,WPARAM wParam,LPARAM lParam) {
	switch(msg) {
		//TODO
	}
}

