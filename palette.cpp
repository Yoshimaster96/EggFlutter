#include "palette.h"

HWND hwndPalette;
bool wvisPalette = false;
DWORD paletteBuffer[0x100];
DWORD paletteAnimBuffer[0x1000];
DWORD gradientBuffer[0x18];
int paletteAnim = 0;

void loadPalette() {
	//Load palette
	memset(paletteBuffer,0,0x100*sizeof(DWORD));
	//BG1 default
	for(int j=0; j<3; j++) {
		for(int i=0; i<11; i++) {
			int offs = i+(j*11);
			WORD col = romBuf[0x1FA27C+(offs<<1)]|(romBuf[0x1FA27D+(offs<<1)]<<8);
			paletteBuffer[0x11+i+(j<<4)] = convColor_SNEStoRGB(col);
		}
	}
	//BG1 palette
	int bg1Pal = (levelHeader[1]&0x7C)>>2;
	int bg1PalBase = romBuf[0x003874+(bg1Pal<<1)]|(romBuf[0x003875+(bg1Pal<<1)]<<8);
	bg1PalBase += 0x1FA000;
	for(int j=0; j<2; j++) {
		for(int i=0; i<15; i++) {
			int offs = i+(j*15);
			WORD col = romBuf[bg1PalBase+(offs<<1)]|(romBuf[bg1PalBase+1+(offs<<1)]<<8);
			paletteBuffer[0x41+i+(j<<4)] = convColor_SNEStoRGB(col);
		}
	}
	bg1PalBase += 0x3C;
	for(int j=0; j<3; j++) {
		for(int i=0; i<4; i++) {
			int offs = i+(j*4);
			WORD col = romBuf[bg1PalBase+(offs<<1)]|(romBuf[bg1PalBase+1+(offs<<1)]<<8);
			paletteBuffer[0x1C+i+(j<<4)] = convColor_SNEStoRGB(col);
		}
	}
	//BG2 palette
	int bg2Pal = ((levelHeader[2]&0x1F)<<1)|(levelHeader[3]>>7);
	int bg2PalBase = romBuf[0x0038F4+(bg2Pal<<1)]|(romBuf[0x0038F5+(bg2Pal<<1)]<<8);
	bg2PalBase += 0x1FA000;
	for(int j=0; j<2; j++) {
		for(int i=0; i<15; i++) {
			int offs = i+(j*15);
			WORD col = romBuf[bg2PalBase+(offs<<1)]|(romBuf[bg2PalBase+1+(offs<<1)]<<8);
			paletteBuffer[0x61+i+(j<<4)] = convColor_SNEStoRGB(col);
		}
	}
	//BG3 palette
	int bg3Pal = ((levelHeader[3]&1)<<5)|(levelHeader[4]>>3);
	int bg3PalBase = romBuf[0x003974+(bg3Pal<<1)]|(romBuf[0x003975+(bg3Pal<<1)]<<8);
	bg3PalBase += 0x1FA000;
	for(int i=0; i<15; i++) {
		WORD col = romBuf[bg3PalBase+(i<<1)]|(romBuf[bg3PalBase+1+(i<<1)]<<8);
		paletteBuffer[1+i] = convColor_SNEStoRGB(col);
	}
	//Sprite default
	for(int j=0; j<5; j++) {
		for(int i=0; i<15; i++) {
			int offs = i+(j*15);
			WORD col = romBuf[0x1FA1C8+(offs<<1)]|(romBuf[0x1FA1C9+(offs<<1)]<<8);
			paletteBuffer[0x81+i+(j<<4)] = convColor_SNEStoRGB(col);
		}
	}
	//Yoshi
	for(int i=0; i<15; i++) {
		WORD col = romBuf[0x1FA040+(i<<1)]|(romBuf[0x1FA041+(i<<1)]<<8);
		paletteBuffer[0xD1+i] = convColor_SNEStoRGB(col);
	}
	//Sprite palette
	int spPal = levelHeader[5]&0xF;
	int spPalBase = romBuf[0x0039F4+(spPal<<1)]|(romBuf[0x0039F5+(spPal<<1)]<<8);
	spPalBase += 0x1FA000;
	for(int j=0; j<2; j++) {
		for(int i=0; i<15; i++) {
			int offs = i+(j*15);
			WORD col = romBuf[spPalBase+(offs<<1)]|(romBuf[spPalBase+1+(offs<<1)]<<8);
			paletteBuffer[0xE1+i+(j<<4)] = convColor_SNEStoRGB(col);
		}
	}
	//Load gradient colors
	int bgCol = levelHeader[0]>>3;
	if(bgCol&0x10) {
		bgCol &= 0xF;
		int bgColBase = romBuf[0x00D575+(bgCol<<2)]|(romBuf[0x00D576+(bgCol<<2)]<<8);
		bgColBase += 0x1F0000;
		for(int i=0; i<0x18; i++) {
			int si = 0x17-i;
			WORD col = romBuf[bgColBase+(si<<1)]|(romBuf[bgColBase+1+(si<<1)]<<8);
			gradientBuffer[i] = convColor_SNEStoRGB(col);
		}
	} else {
		WORD col = romBuf[0x1FA130+(bgCol<<1)]|(romBuf[0x1FA131+(bgCol<<1)]<<8);
		for(int i=0; i<0x18; i++) {
			gradientBuffer[i] = convColor_SNEStoRGB(col);
		}
	}
	//Load palette animation
	memset(paletteAnimBuffer,0xFF,0x1000*sizeof(DWORD));
	//TODO
	updatePalette();
}
void updatePalette() {
	int frameOffset = (paletteAnim&0xF)<<8;
	for(int i=0; i<0x100; i++) {
		DWORD color = paletteAnimBuffer[frameOffset|i];
		if(color!=0xFFFFFFFF) {
			paletteBuffer[i] = color;
		}
	}
}

///////////////////
//WINDOW FUNCTION//
///////////////////
HDC				hdcPal;
HBITMAP			hbmpPal;
DWORD *			bmpDataPal;

//Main drawing code
void updateEntireScreen_pal() {
	memcpy(bmpDataPal,paletteBuffer,0x100*sizeof(DWORD));
}

//Message loop function
LRESULT CALLBACK WndProc_Palette(HWND hwnd,UINT msg,WPARAM wParam,LPARAM lParam) {
	switch(msg) {
		//Creation and destruction of window(s)
		case WM_CREATE: {
			//Create objects
			hdcPal = GetDC(hwnd);
			BITMAPINFO bmi;
			memset(&bmi.bmiHeader,0,sizeof(BITMAPINFOHEADER));
			bmi.bmiHeader.biSize			= sizeof(BITMAPINFOHEADER);
			bmi.bmiHeader.biPlanes			= 1;
			bmi.bmiHeader.biBitCount		= 32;
			bmi.bmiHeader.biCompression		= BI_RGB;
			bmi.bmiHeader.biWidth			= 0x10;
			bmi.bmiHeader.biHeight			= -0x10;
			hbmpPal = CreateDIBSection(hdcPal,&bmi,DIB_RGB_COLORS,(void**)&bmpDataPal,NULL,0);
			memset(bmpDataPal,0,0x100*sizeof(DWORD));
			break;
		}
		case WM_DESTROY: {
			//Free objects
			DeleteDC(hdcPal);
			DeleteObject(hbmpPal);
			break;
		}
		case WM_CLOSE: {
			//Simply hide the window
			ShowWindow(hwnd,SW_HIDE);
			wvisPalette = false;
			break;
		}
		//Updating
		case WM_PAINT: {
			//Update
			updateEntireScreen_pal();
			//Blit screen
			PAINTSTRUCT ps;
			BeginPaint(hwnd,&ps);
			HDC hdcMem = CreateCompatibleDC(hdcPal);
			HBITMAP hbmpOld = (HBITMAP)SelectObject(hdcMem,hbmpPal);
			StretchBlt(hdcPal,0,0,0x100,0x100,hdcMem,0,0,0x10,0x10,SRCCOPY);
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

