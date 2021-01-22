#include "map16.h"

HWND hwndMap16;
BYTE map16Buffer[0x80000];

void loadMap16() {
	//Load Map16 tiles
	memset(map16Buffer,0,0x80000);
	for(int i=0; i<0x100; i++) {
		WORD curPageOff = romBuf[0x0C32A4]|(romBuf[0x0C32A5]<<8);
		WORD nextPageOff = romBuf[0x0C32A4]|(romBuf[0x0C32A5]<<8);
		if(i==0xA6) nextPageOff = 0xA228;
		int pageSize = nextPageOff-curPageOff;
		memcpy(&map16Buffer[i<<11],&romBuf[0x0CB3F2+curPageOff],pageSize);
	}
}

void dispMap16Tile(DWORD * pixelBuf,int width,int height,WORD tile,POINT offs) {
	int offsX = offs.x;
	int offsY = offs.y;
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

