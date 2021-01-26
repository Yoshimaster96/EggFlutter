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
		WORD bg3Idx = romBuf[0x00E907+(bg3Ts*3)]|(romBuf[0x00E908+(bg3Ts*3)]<<8);
		if(bg3Idx) {
			DWORD gfxAddr = convAddr_SNEStoPC_YI(getLZ1Address(bg3Idx));
			decompressLZ1(bg3Buffer,&romBuf[gfxAddr]);
		}
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
	memset(bmpDataBg,0,0x100000*sizeof(DWORD));
	for(int j=0; j<0x40; j++) {
		for(int i=0; i<0x20; i++) {
			WORD tile = i+(j<<5);
			WORD td = bg2Buffer[tile<<1]|(bg2Buffer[(tile<<1)|1]<<8);
			dispMap8Tile(bmpDataBg,0x400,0x400,((td>>8)&0xDC)|1,td&0x3FF,{i<<4,j<<4});
		}
	}
	for(int j=0; j<0x20; j++) {
		for(int i=0; i<0x20; i++) {
			WORD tile = i+(j<<5);
			WORD td = bg3Buffer[tile<<1]|(bg3Buffer[(tile<<1)|1]<<8);
			dispMap8Tile(bmpDataBg,0x400,0x400,((td>>8)&0xDC)|3,(td&0x3FF)+0x280,{0x200+(i<<4),j<<4});
		}
	}
}

//Message loop function
LRESULT CALLBACK WndProc_Background(HWND hwnd,UINT msg,WPARAM wParam,LPARAM lParam) {
	switch(msg) {
		//Creation and destruction of window(s)
		case WM_CREATE: {
			//Create objects
			hdcBg = GetDC(hwnd);
			BITMAPINFO bmi;
			memset(&bmi.bmiHeader,0,sizeof(BITMAPINFOHEADER));
			bmi.bmiHeader.biSize			= sizeof(BITMAPINFOHEADER);
			bmi.bmiHeader.biPlanes			= 1;
			bmi.bmiHeader.biBitCount		= 32;
			bmi.bmiHeader.biCompression		= BI_RGB;
			bmi.bmiHeader.biWidth			= 0x400;
			bmi.bmiHeader.biHeight			= -0x400;
			hbmpBg = CreateDIBSection(hdcBg,&bmi,DIB_RGB_COLORS,(void**)&bmpDataBg,NULL,0);
			memset(bmpDataBg,0,0x100000*sizeof(DWORD));
			break;
		}
		case WM_DESTROY: {
			//Free objects
			DeleteDC(hdcBg);
			DeleteObject(hbmpBg);
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
			//Update
			updateEntireScreen_bg();
			//Blit screen
			PAINTSTRUCT ps;
			BeginPaint(hwnd,&ps);
			HDC hdcMem = CreateCompatibleDC(hdcBg);
			HBITMAP hbmpOld = (HBITMAP)SelectObject(hdcMem,hbmpBg);
			BitBlt(hdcBg,0,0,0x400,0x400,hdcMem,0,0,SRCCOPY);
			SelectObject(hdcMem,hbmpOld);
			DeleteDC(hdcMem);
			EndPaint(hwnd,&ps);
			break;
		}
		default: {
			return DefWindowProc(hwnd,msg,wParam,lParam);
		}
	}
	return 0;
}

