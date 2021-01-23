#include "background.h"

HWND hwndBackground;
bool wvisBackground = false;
BYTE bg2Buffer[0x1000],bg3Buffer[0x800];

void loadBackground2() {
	memset(bg2Buffer,0xFF,0x1000);
	int bg2Ts = ((levelHeader[1]&3)<<3)|(levelHeader[2]>>5);
	WORD bg2Idx = romBuf[0x00E711+(bg2Ts<<1)]|(romBuf[0x00E712+(bg2Ts<<1)]<<8);
	bg2Idx = romBuf[0x00E752+bg2Idx];
	DWORD gfxAddr = convAddr_SNEStoPC_YI(getLZ1Address(bg2Idx));
	decompressLZ1(bg2Buffer,&romBuf[gfxAddr]);
}
void loadBackground3() {
	memset(bg3Buffer,0xFF,0x800);
	int bg3Ts = (levelHeader[3]&0x7E)>>1;
	if(bg3Ts!=0 && bg3Ts<=0x2F) {
		WORD bg3Idx = romBuf[0x00E90A+(bg3Ts*3)]|(romBuf[0x00E90B+(bg3Ts*3)]<<8);
		DWORD gfxAddr = convAddr_SNEStoPC_YI(getLZ1Address(bg3Idx));
		decompressLZ1(bg3Buffer,&romBuf[gfxAddr]);
	}
}

///////////////////
//WINDOW FUNCTION//
///////////////////
HDC				hdcBg;
HBITMAP			hbmpBg;
DWORD *			bmpDataBg;

//Main drawing code
void updateEntireScreen_bg() {
	//TODO
}

//Message loop function
LRESULT CALLBACK WndProc_Background(HWND hwnd,UINT msg,WPARAM wParam,LPARAM lParam) {
	switch(msg) {
		//Creation and destruction of window(s)
		case WM_CREATE: {
			//Create objects
			//TODO
			break;
		}
		case WM_DESTROY: {
			//Free objects
			//TODO
			break;
		}
		case WM_CLOSE: {
			//Simply hide the window
			ShowWindow(hwnd,SW_HIDE);
			wvisBackground = false;
			break;
		}
		//Updating
		case WM_PAINT: {
			//TODO
			break;
		}
		default: {
			return DefWindowProc(hwnd,msg,wParam,lParam);
		}
	}
	return 0;
}

