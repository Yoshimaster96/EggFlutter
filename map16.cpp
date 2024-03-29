#include "map16.h"

HWND hwndMap16;
bool wvisMap16 = false;
BYTE map16Buffer[0x80000];

void loadMap16() {
	//Load Map16 tiles
	for(int i=0; i<0x80000; i+=2) {
		map16Buffer[i] = 0xFF;
		map16Buffer[i+1] = 0x18;
	}
	for(int i=0; i<0xA7; i++) {
		WORD curPageOff = romBuf[0x0C32A4+(i<<1)]|(romBuf[0x0C32A5+(i<<1)]<<8);
		WORD nextPageOff = romBuf[0x0C32A6+(i<<1)]|(romBuf[0x0C32A7+(i<<1)]<<8);
		if(i==0xA6) nextPageOff = 0xA228;
		int pageSize = nextPageOff-curPageOff;
		memcpy(&map16Buffer[i<<11],&romBuf[0x0C33F2+curPageOff],pageSize);
	}
}

void dispMap16Tile(DWORD * pixelBuf,int width,int height,WORD tile,int offsX,int offsY,BYTE inv) {
	RECT clipRect = {0,0,8,8};
	WORD td = map16Buffer[tile<<3]|(map16Buffer[(tile<<3)|1]<<8);
	dispMap8Tile(pixelBuf,width,height,(td>>8)&0xDC,td&0x3FF,offsX,offsY,&clipRect,inv);
	td = map16Buffer[(tile<<3)|2]|(map16Buffer[(tile<<3)|3]<<8);
	dispMap8Tile(pixelBuf,width,height,(td>>8)&0xDC,td&0x3FF,offsX+8,offsY,&clipRect,inv);
	td = map16Buffer[(tile<<3)|4]|(map16Buffer[(tile<<3)|5]<<8);
	dispMap8Tile(pixelBuf,width,height,(td>>8)&0xDC,td&0x3FF,offsX,offsY+8,&clipRect,inv);
	td = map16Buffer[(tile<<3)|6]|(map16Buffer[(tile<<3)|7]<<8);
	dispMap8Tile(pixelBuf,width,height,(td>>8)&0xDC,td&0x3FF,offsX+8,offsY+8,&clipRect,inv);
}

///////////////////
//WINDOW FUNCTION//
///////////////////
HDC				hdcMap16;
HBITMAP			hbmpMap16;
DWORD *			bmpDataMap16;
WORD map16Base = 0;
RECT invRect_map16 = {0,0,0x200,0x200};

//Main drawing code
void updateEntireScreen_map16() {
	memset(bmpDataMap16,0x80,0x10000*sizeof(DWORD));
	for(int j=0; j<0x10; j++) {
		for(int i=0; i<0x10; i++) {
			dispMap16Tile(bmpDataMap16,0x100,0x100,map16Base+i+(j<<4),i<<4,j<<4,false);
		}
	}
}

//Message loop function
LRESULT CALLBACK WndProc_Map16(HWND hwnd,UINT msg,WPARAM wParam,LPARAM lParam) {
	switch(msg) {
		//Creation and destruction of window(s)
		case WM_CREATE: {
			//Create objects
			hdcMap16 = GetDC(hwnd);
			BITMAPINFO bmi;
			memset(&bmi.bmiHeader,0,sizeof(BITMAPINFOHEADER));
			bmi.bmiHeader.biSize			= sizeof(BITMAPINFOHEADER);
			bmi.bmiHeader.biPlanes			= 1;
			bmi.bmiHeader.biBitCount		= 32;
			bmi.bmiHeader.biCompression		= BI_RGB;
			bmi.bmiHeader.biWidth			= 0x100;
			bmi.bmiHeader.biHeight			= -0x100;
			hbmpMap16 = CreateDIBSection(hdcMap16,&bmi,DIB_RGB_COLORS,(void**)&bmpDataMap16,NULL,0);
			memset(bmpDataMap16,0,0x10000*sizeof(DWORD));
			break;
		}
		case WM_DESTROY: {
			//Free objects
			DeleteDC(hdcMap16);
			DeleteObject(hbmpMap16);
			break;
		}
		case WM_CLOSE: {
			//Simply hide the window
			ShowWindow(hwnd,SW_HIDE);
			wvisMap16 = false;
			break;
		}
		//Updating
		case WM_PAINT: {
			//Update
			updateEntireScreen_map16();
			//Blit screen
			PAINTSTRUCT ps;
			BeginPaint(hwnd,&ps);
			HDC hdcMem = CreateCompatibleDC(hdcMap16);
			HBITMAP hbmpOld = (HBITMAP)SelectObject(hdcMem,hbmpMap16);
			StretchBlt(hdcMap16,0,0,0x200,0x200,hdcMem,0,0,0x100,0x100,SRCCOPY);
			SelectObject(hdcMem,hbmpOld);
			DeleteDC(hdcMem);
			EndPaint(hwnd,&ps);
			break;
		}
		//Input
		case WM_KEYDOWN: {
			switch(wParam) {
				case VK_DOWN: {
					if(map16Base<0xA600) {
						if(GetAsyncKeyState(VK_CONTROL)&0x8000) map16Base += 0x100;
						else map16Base += 0x10;
						if(map16Base>0xA600) map16Base = 0xA600;
						InvalidateRect(hwnd,&invRect_map16,false);
						UpdateWindow(hwnd);
					}
					break;
				}
				case VK_UP: {
					if(map16Base) {
						if(GetAsyncKeyState(VK_CONTROL)&0x8000) map16Base -= 0x100;
						else map16Base -= 0x10;
						if(map16Base>0xA600) map16Base = 0;
						InvalidateRect(hwnd,&invRect_map16,false);
						UpdateWindow(hwnd);
					}
					break;
				}
			}
			break;
		}
		case WM_SETFOCUS: {
			childFocus = true;
			break;
		}
		default: {
			return DefWindowProcA(hwnd,msg,wParam,lParam);
		}
	}
	return 0;
}

