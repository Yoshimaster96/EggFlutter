#include "map16.h"

HWND hwndMap16;
bool wvisMap16 = false;
BYTE map16Buffer[0x80000];

void loadMap16() {
	//Load Map16 tiles
	memset(map16Buffer,0,0x80000);
	for(int i=0; i<0xA7; i++) {
		WORD curPageOff = romBuf[0x0C32A4+(i<<1)]|(romBuf[0x0C32A5+(i<<1)]<<8);
		WORD nextPageOff = romBuf[0x0C32A6+(i<<1)]|(romBuf[0x0C32A7+(i<<1)]<<8);
		if(i==0xA6) nextPageOff = 0xA228;
		int pageSize = nextPageOff-curPageOff;
		memcpy(&map16Buffer[i<<11],&romBuf[0x0C33F2+curPageOff],pageSize);
	}
}

void dispMap16Tile(DWORD * pixelBuf,int width,int height,WORD tile,POINT offs) {
	int offsX = offs.x;
	int offsY = offs.y;
	WORD td = map16Buffer[tile<<3]|(map16Buffer[(tile<<3)|1]<<8);
	dispMap8Tile(pixelBuf,width,height,(td>>8)&0xDC,td&0x3FF,{offsX,offsY});
	td = map16Buffer[(tile<<3)|2]|(map16Buffer[(tile<<3)|3]<<8);
	dispMap8Tile(pixelBuf,width,height,(td>>8)&0xDC,td&0x3FF,{offsX+8,offsY});
	td = map16Buffer[(tile<<3)|4]|(map16Buffer[(tile<<3)|5]<<8);
	dispMap8Tile(pixelBuf,width,height,(td>>8)&0xDC,td&0x3FF,{offsX,offsY+8});
	td = map16Buffer[(tile<<3)|6]|(map16Buffer[(tile<<3)|7]<<8);
	dispMap8Tile(pixelBuf,width,height,(td>>8)&0xDC,td&0x3FF,{offsX+8,offsY+8});
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
	memset(bmpDataMap16,0,0x10000*sizeof(DWORD));
	for(int j=0; j<0x10; j++) {
		for(int i=0; i<0x10; i++) {
			dispMap16Tile(bmpDataMap16,0x100,0x100,map16Base+i+(j<<4),{i<<4,j<<4});
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
					if(GetAsyncKeyState(VK_CONTROL)&0x8000) {
						if(map16Base<0xA600) {
							map16Base += 0x100;
							if(map16Base>0xA600) map16Base = 0xA600;
							InvalidateRect(hwnd,&invRect_map16,false);
							UpdateWindow(hwnd);
						}
					} else {
						if(map16Base<0xA600) {
							map16Base += 0x10;
							ScrollWindowEx(hwnd,0,-16,NULL,NULL,NULL,NULL,SW_INVALIDATE);
							UpdateWindow(hwnd);
						}
					}
					break;
				}
				case VK_UP: {
					if(GetAsyncKeyState(VK_CONTROL)&0x8000) {
						if(map16Base) {
							map16Base -= 0x100;
							if(map16Base<0) map16Base = 0;
							InvalidateRect(hwnd,&invRect_map16,false);
							UpdateWindow(hwnd);
						}
					} else {
						if(map16Base) {
							map16Base -= 0x10;
							ScrollWindowEx(hwnd,0,16,NULL,NULL,NULL,NULL,SW_INVALIDATE);
							UpdateWindow(hwnd);
						}
					}
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

