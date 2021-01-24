#include "map8.h"

HWND hwndMap8;
bool wvisMap8 = false;
BYTE map8Buffer[0x19000];
BYTE fontBuffer[0x2000];
int map8Anim = 0;

DWORD getLZ1Address(int gfxFile) {
	return romBuf[0x03795E +(gfxFile*3)]|(romBuf[0x03795F+(gfxFile*3)]<<8)|(romBuf[0x037960+(gfxFile*3)]<<16);	
}
DWORD getLZ16Address(int gfxFile) {
	return romBuf[0x037C79+(gfxFile*3)]|(romBuf[0x037C7A+(gfxFile*3)]<<8)|(romBuf[0x037C7B+(gfxFile*3)]<<16);	
}

void loadMap8() {
	BYTE tempBuffer[0x8000];
	memset(map8Buffer,0,0x19000);
	//Load global tiles
	DWORD gfxAddr = convAddr_SNEStoPC_YI(getLZ16Address(0x19));
	decompressLZ16(&map8Buffer[0x2000],&romBuf[gfxAddr],0x40);
	gfxAddr = convAddr_SNEStoPC_YI(getLZ16Address(0x12));
	decompressLZ16(&map8Buffer[0x8800],&romBuf[gfxAddr],0x10);
	gfxAddr = convAddr_SNEStoPC_YI(getLZ16Address(0x76));
	decompressLZ16(&map8Buffer[0x9400],&romBuf[gfxAddr],0x18);
	gfxAddr = convAddr_SNEStoPC_YI(getLZ16Address(0x72));
	decompressLZ16(&map8Buffer[0x12000],&romBuf[gfxAddr],0x80);
	//Load BG1 tiles
	int bg1Ts = ((levelHeader[0]&7)<<1)|(levelHeader[1]>>7);
	gfxAddr = convAddr_SNEStoPC_YI(getLZ1Address(romBuf[0x002F3B+(bg1Ts*3)]));
	decompressLZ1(tempBuffer,&romBuf[gfxAddr]);
	unpackGfx4BPP(&map8Buffer[0],tempBuffer,0x80);
	gfxAddr = convAddr_SNEStoPC_YI(getLZ1Address(romBuf[0x002F39+(bg1Ts*3)]));
	decompressLZ1(tempBuffer,&romBuf[gfxAddr]);
	unpackGfx4BPP(&map8Buffer[0x4000],tempBuffer,0x80);
	gfxAddr = convAddr_SNEStoPC_YI(getLZ1Address(romBuf[0x002F3A+(bg1Ts*3)]));
	decompressLZ1(tempBuffer,&romBuf[gfxAddr]);
	unpackGfx4BPP(&map8Buffer[0x6000],tempBuffer,0x80);
	//Load BG2 tiles
	int bg2Ts = ((levelHeader[1]&3)<<3)|(levelHeader[2]>>5);
	gfxAddr = convAddr_SNEStoPC_YI(getLZ16Address(romBuf[0x002F99+(bg2Ts<<1)]));
	decompressLZ16(&map8Buffer[0xA000],&romBuf[gfxAddr],0x40);
	gfxAddr = convAddr_SNEStoPC_YI(getLZ16Address(romBuf[0x002F9A+(bg2Ts<<1)]));
	decompressLZ16(&map8Buffer[0xC000],&romBuf[gfxAddr],0x40);
	//Load BG3 tiles
	int bg3Ts = (levelHeader[3]&0x7E)>>1;
	if(bg3Ts<=0x2F) {
		gfxAddr = convAddr_SNEStoPC_YI(getLZ1Address(romBuf[0x002FD9+(bg3Ts<<1)]));
		decompressLZ1(tempBuffer,&romBuf[gfxAddr]);
		unpackGfx2BPP(&map8Buffer[0xE000],tempBuffer,0x80);
		gfxAddr = convAddr_SNEStoPC_YI(getLZ1Address(romBuf[0x002FDA+(bg3Ts<<1)]));
		decompressLZ1(tempBuffer,&romBuf[gfxAddr]);
		unpackGfx2BPP(&map8Buffer[0x10000],tempBuffer,0x80);
	}
	//Load sprite tiles
	int spTs = ((levelHeader[4]&7)<<4)|(levelHeader[5]>>4);
	gfxAddr = convAddr_SNEStoPC_YI(getLZ16Address(romBuf[0x003039+(spTs*6)]));
	decompressLZ16(&map8Buffer[0x16000],&romBuf[gfxAddr],0x10);
	gfxAddr = convAddr_SNEStoPC_YI(getLZ16Address(romBuf[0x00303A+(spTs*6)]));
	decompressLZ16(&map8Buffer[0x16800],&romBuf[gfxAddr],0x10);
	gfxAddr = convAddr_SNEStoPC_YI(getLZ16Address(romBuf[0x00303B+(spTs*6)]));
	decompressLZ16(&map8Buffer[0x17000],&romBuf[gfxAddr],0x10);
	gfxAddr = convAddr_SNEStoPC_YI(getLZ16Address(romBuf[0x00303C+(spTs*6)]));
	decompressLZ16(&map8Buffer[0x17800],&romBuf[gfxAddr],0x10);
	gfxAddr = convAddr_SNEStoPC_YI(getLZ16Address(romBuf[0x00303D+(spTs*6)]));
	decompressLZ16(&map8Buffer[0x18000],&romBuf[gfxAddr],0x10);
	gfxAddr = convAddr_SNEStoPC_YI(getLZ16Address(romBuf[0x00303E +(spTs*6)]));
	decompressLZ16(&map8Buffer[0x18800],&romBuf[gfxAddr],0x10);
}
void updateMap8(int frame) {
	//TODO
}
void updateMap8Sw(int state) {
	//TODO
}

void dispMap8Tile(DWORD * pixelBuf,int width,int height,BYTE props,WORD tile,POINT offs) {
	int offsX = offs.x;
	int offsY = offs.y;
	bool flipV = props&0x80;
	bool flipH = props&0x40;
	int palette = (props&0x3C)<<2;
	if(props&2) palette >>= 2;
	bool size = props&1;
	if(size) {
		for(int j=0; j<16; j++) {
			for(int i=0; i<16; i++) {
				int si = i&7;
				int sj = j&7;
				int sx = flipH?(7-si):si;
				int sy = flipV?(7-sj):sj;
				int toff = ((i&8)>>3)|((j&8)<<1);
				if(flipH) toff ^= 0x01;
				if(flipV) toff ^= 0x10;
				int dx = offsX+i;
				int dy = offsY+j;
				int idx = getIndexFromTile(map8Buffer,tile+toff,{sx,sy});
				if(idx) {
					putPixel(pixelBuf,width,height,paletteBuffer[palette|idx],{dx,dy});
				}
			}
		}
	} else {
		for(int j=0; j<8; j++) {
			for(int i=0; i<8; i++) {
				int sx = flipH?(7-i):i;
				int sy = flipV?(7-j):j;
				int dx = offsX+i;
				int dy = offsY+j;
				int idx = getIndexFromTile(map8Buffer,tile,{sx,sy});
				if(idx) {
					putPixel(pixelBuf,width,height,paletteBuffer[palette|idx],{dx,dy});
				}
			}
		}
	}
}
void dispMap8Char(DWORD * pixelBuf,int width,int height,char c,POINT offs) {
	int offsX = offs.x;
	int offsY = offs.y;
	//TODO
}

///////////////////
//WINDOW FUNCTION//
///////////////////
HDC				hdcMap8;
HBITMAP			hbmpMap8;
DWORD *			bmpDataMap8;
WORD map8Base = 0;
BYTE map8Pal = 0;

//Main drawing code
void updateEntireScreen_map8() {
	memset(bmpDataMap8,0,0x4000*sizeof(DWORD));
	for(int j=0; j<0x10; j++) {
		WORD row = map8Base+(j<<4);
		BYTE props = map8Pal<<2;
		if(row>=0x480) props |= 0x20;
		else if(row>=0x380) props |= 2;
		for(int i=0; i<0x10; i++) {
			dispMap8Tile(bmpDataMap8,0x80,0x80,props,row|i,{i<<3,j<<3});
		}
	}
}

//Message loop function
LRESULT CALLBACK WndProc_Map8(HWND hwnd,UINT msg,WPARAM wParam,LPARAM lParam) {
	switch(msg) {
		//Creation and destruction of window(s)
		case WM_CREATE: {
			//Create objects
			hdcMap8 = GetDC(hwnd);
			BITMAPINFO bmi;
			memset(&bmi.bmiHeader,0,sizeof(BITMAPINFOHEADER));
			bmi.bmiHeader.biSize			= sizeof(BITMAPINFOHEADER);
			bmi.bmiHeader.biPlanes			= 1;
			bmi.bmiHeader.biBitCount		= 32;
			bmi.bmiHeader.biCompression		= BI_RGB;
			bmi.bmiHeader.biWidth			= 0x80;
			bmi.bmiHeader.biHeight			= -0x80;
			hbmpMap8 = CreateDIBSection(hdcMap8,&bmi,DIB_RGB_COLORS,(void**)&bmpDataMap8,NULL,0);
			memset(bmpDataMap8,0,0x4000*sizeof(DWORD));
			break;
		}
		case WM_DESTROY: {
			//Free objects
			DeleteDC(hdcMap8);
			DeleteObject(hbmpMap8);
			break;
		}
		case WM_CLOSE: {
			//Simply hide the window
			ShowWindow(hwnd,SW_HIDE);
			wvisMap8 = false;
			break;
		}
		//Updating
		case WM_PAINT: {
			//Update
			updateEntireScreen_map8();
			//Blit screen
			PAINTSTRUCT ps;
			BeginPaint(hwnd,&ps);
			HDC hdcMem = CreateCompatibleDC(hdcMap8);
			HBITMAP hbmpOld = (HBITMAP)SelectObject(hdcMem,hbmpMap8);
			StretchBlt(hdcMap8,0,0,0x100,0x100,hdcMem,0,0,0x80,0x80,SRCCOPY);
			SelectObject(hdcMem,hbmpOld);
			DeleteDC(hdcMem);
			EndPaint(hwnd,&ps);
			break;
		}
		//Input
		case WM_KEYDOWN: {
			RECT rect = {0,0,256,256};
			switch(wParam) {
				case VK_DOWN: {
					if(map8Base<0x540) {
						map8Base += 0x10;
						ScrollWindowEx(hwnd,0,-8,NULL,NULL,NULL,NULL,SW_INVALIDATE);
						UpdateWindow(hwnd);
					}
					break;
				}
				case VK_UP: {
					if(map8Base) {
						map8Base -= 0x10;
						ScrollWindowEx(hwnd,0,8,NULL,NULL,NULL,NULL,SW_INVALIDATE);
						UpdateWindow(hwnd);
					}
					break;
				}
				case VK_RIGHT: {
					map8Pal++;
					map8Pal &= 7;
					InvalidateRect(hwnd,&rect,false);
					UpdateWindow(hwnd);
					break;
				}
				case VK_LEFT: {
					map8Pal--;
					map8Pal &= 7;
					InvalidateRect(hwnd,&rect,false);
					UpdateWindow(hwnd);
					break;
				}
			}
			break;
		}
		default: {
			return DefWindowProc(hwnd,msg,wParam,lParam);
		}
	}
	return 0;
}

