#include "palette.h"

HWND hwndPalette;
bool wvisPalette = false;
DWORD paletteBuffer[0x100];
DWORD gradientBuffer[0x18];
int paletteAnim = 0;
bool vW6_palette = false;

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
	if(vW6_palette) bg1Pal += 0x20;
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
	//Init animation frame
	paletteAnim = 0;
	updatePalette();
}
const int animPalUnusedCmp[8] = {0,20,24,26,28,30,32,34};
void updatePalette_01() {
	for(int i=0; i<8; i++) {
		if((paletteAnim%36)==animPalUnusedCmp[i]) {
			int offset = 0x1FEB4A+(0x1A*i);
			for(int j=0; j<13; j++) {
				WORD col = romBuf[offset+(j<<1)]|(romBuf[offset+1+(j<<1)]<<8);
				paletteBuffer[0x43+j] = convColor_SNEStoRGB(col);
			}
			break;
		}
	}
}
void updatePalette_02() {
	if((paletteAnim&3)==0) {
		int offset = ((paletteAnim&0xC)>>2)*6;
		offset += 0x1FA190;
		for(int j=0; j<3; j++) {
			WORD col = romBuf[offset+(j<<1)]|(romBuf[offset+1+(j<<1)]<<8);
			paletteBuffer[0x05+j] = convColor_SNEStoRGB(col);
		}
	}
}
void updatePalette_03() {
	if((paletteAnim&3)==0) {
		int offset = (paletteAnim&0xC)<<3;
		offset += 0x1FCCEA;
		for(int j=0; j<16; j++) {
			WORD col = romBuf[offset+(j<<1)]|(romBuf[offset+1+(j<<1)]<<8);
			paletteBuffer[0x70+j] = convColor_SNEStoRGB(col);
		}
	}
}
void updatePalette_04() {
	if((paletteAnim&1)==0) {
		int offset = (paletteAnim&0xE)*15;
		offset += 0x1FEA5A;
		for(int j=0; j<15; j++) {
			WORD col = romBuf[offset+(j<<1)]|(romBuf[offset+1+(j<<1)]<<8);
			paletteBuffer[0x71+j] = convColor_SNEStoRGB(col);
		}
	}
}
void updatePalette_05() {
	if((paletteAnim&3)==0) {
		int offset = (paletteAnim&0x1C)<<2;
		offset += 0x1FDA00;
		for(int j=0; j<8; j++) {
			WORD col = romBuf[offset+(j<<1)]|(romBuf[offset+1+(j<<1)]<<8);
			paletteBuffer[0x71+j] = convColor_SNEStoRGB(col);
		}
		int bg1Ts = ((levelHeader[0]&7)<<1)|(levelHeader[1]>>7);
		if(bg1Ts==0 || bg1Ts==8) {
			offset = ((paletteAnim&0x1C)>>2)*0x26;
			offset += 0x1FF5E8;
			for(int j=0; j<6; j++) {
				WORD col = romBuf[offset+(j<<1)]|(romBuf[offset+1+(j<<1)]<<8);
				paletteBuffer[0x02+j] = convColor_SNEStoRGB(col);
			}
			if(offset==0x1FF6F2) offset -= 0x26;
			for(int j=0; j<13; j++) {
				WORD col = romBuf[offset+12+(j<<1)]|(romBuf[offset+13+(j<<1)]<<8);
				paletteBuffer[0x43+j] = convColor_SNEStoRGB(col);
			}
		}
	}
}
void updatePalette_06() {
	if((paletteAnim&3)==0) {
		int offset = (paletteAnim&0x1C)<<2;
		offset += 0x1FDA00;
		for(int j=0; j<8; j++) {
			WORD col = romBuf[offset+(j<<1)]|(romBuf[offset+1+(j<<1)]<<8);
			paletteBuffer[0x71+j] = convColor_SNEStoRGB(col);
		}
		offset = ((paletteAnim&0x1C)>>2)*0x26;
		offset += 0x1FF5F4;
		if(offset==0x1FF6FE) offset -= 0x26;
		for(int j=0; j<13; j++) {
			WORD col = romBuf[offset+(j<<1)]|(romBuf[offset+1+(j<<1)]<<8);
			paletteBuffer[0x43+j] = convColor_SNEStoRGB(col);
		}
		offset = (paletteAnim&0xC)<<1;
		offset += 0x1FA150;
		for(int j=0; j<4; j++) {
			WORD col = romBuf[offset+(j<<1)]|(romBuf[offset+1+(j<<1)]<<8);
			paletteBuffer[0x53+j] = convColor_SNEStoRGB(col);
		}
	}
}
void updatePalette_07() {
	if((paletteAnim&1)==0) {
		int offset = (paletteAnim&7)<<3;
		offset += 0x1FDA00;
		for(int j=0; j<8; j++) {
			WORD col = romBuf[offset+(j<<1)]|(romBuf[offset+1+(j<<1)]<<8);
			paletteBuffer[0x71+j] = convColor_SNEStoRGB(col);
		}
	}
	if((paletteAnim&3)==0) {
		int offset = ((paletteAnim&0x1C)>>2)*0x26;
		offset += 0x1FF5F4;
		if(offset==0x1FF6FE) offset -= 0x26;
		for(int j=0; j<13; j++) {
			WORD col = romBuf[offset+(j<<1)]|(romBuf[offset+1+(j<<1)]<<8);
			paletteBuffer[0x43+j] = convColor_SNEStoRGB(col);
		}
		offset = (paletteAnim&0xC)<<1;
		offset += 0x1FA150;
		for(int j=0; j<4; j++) {
			WORD col = romBuf[offset+(j<<1)]|(romBuf[offset+1+(j<<1)]<<8);
			paletteBuffer[0x53+j] = convColor_SNEStoRGB(col);
		}
	}
}
void updatePalette_08() {
	if((paletteAnim&1)==0) {
		int offset = (paletteAnim&6)<<2;
		offset += 0x1FA150;
		for(int j=0; j<4; j++) {
			WORD col = romBuf[offset+(j<<1)]|(romBuf[offset+1+(j<<1)]<<8);
			paletteBuffer[0x53+j] = convColor_SNEStoRGB(col);
		}
	}
}
void updatePalette_09() {
	if((paletteAnim%3)==0) {
		int offset = ((paletteAnim/3)&7)<<1;
		if(offset>8) offset = 16-offset;
		offset += 0x1FC932;
		WORD col = romBuf[offset]|(romBuf[offset+1]<<8);
		paletteBuffer[0x01] = paletteBuffer[0x09] = convColor_SNEStoRGB(col);
	}
}
void updatePalette_0A() {
	if((paletteAnim&3)==0) {
		int offset = (paletteAnim&0xC)<<1;
		offset += 0x1FE2EC;
		for(int j=0; j<4; j++) {
			WORD col = romBuf[offset+(j<<1)]|(romBuf[offset+1+(j<<1)]<<8);
			paletteBuffer[0x53+j] = convColor_SNEStoRGB(col);
		}
	}
}
void updatePalette_0B() {
	if((paletteAnim&0x3F)==0) {
		int offset = (((paletteAnim&0x1C0)>>6)^7)*6;
		offset += 0x1FE30C;
		for(int j=0; j<3; j++) {
			WORD col = romBuf[offset+(j<<1)]|(romBuf[offset+1+(j<<1)]<<8);
			paletteBuffer[0x01+j] = convColor_SNEStoRGB(col);
		}
	}
}
void updatePalette_0C() {
	if((paletteAnim&0x1F)==0) {
		int offset = ((paletteAnim&0xE0)>>5)*6;
		offset += 0x1FE30C;
		for(int j=0; j<3; j++) {
			WORD col = romBuf[offset+(j<<1)]|(romBuf[offset+1+(j<<1)]<<8);
			paletteBuffer[0x01+j] = convColor_SNEStoRGB(col);
		}
	}
}
void updatePalette_0D() {
	if((paletteAnim%6)==0) {
		int offset = (paletteAnim/6)&7;
		if(offset&4) offset = (offset&3)^3;
		offset *= 6;
		offset += 0x1FEC32;
		for(int j=0; j<3; j++) {
			WORD col = romBuf[offset+(j<<1)]|(romBuf[offset+1+(j<<1)]<<8);
			paletteBuffer[0x01+j] = convColor_SNEStoRGB(col);
		}
	}
}
void updatePalette_0E() {
	if((paletteAnim&3)==0) {
		int bg1Ts = ((levelHeader[0]&7)<<1)|(levelHeader[1]>>7);
		int offset = (paletteAnim&0xC)<<1;
		if(bg1Ts==8) offset += 0x1FE2EC;
		else offset += 0x1FA150;
		for(int j=0; j<4; j++) {
			WORD col = romBuf[offset+(j<<1)]|(romBuf[offset+1+(j<<1)]<<8);
			paletteBuffer[0x53+j] = convColor_SNEStoRGB(col);
		}
		offset = (paletteAnim&0x1C)<<3;
		offset += 0x1FF77E;
		for(int j=0; j<8; j++) {
			WORD col = romBuf[offset+(j<<1)]|(romBuf[offset+1+(j<<1)]<<8);
			paletteBuffer[0x64+j] = convColor_SNEStoRGB(col);
		}
	}
}
void updatePalette_0F() {
	if((paletteAnim&1)==0) {
		int offset = ((paletteAnim&6)>>1)*6;
		offset += 0x1FF46A;
		for(int j=0; j<3; j++) {
			WORD col = romBuf[offset+(j<<1)]|(romBuf[offset+1+(j<<1)]<<8);
			paletteBuffer[0x05+j] = convColor_SNEStoRGB(col);
		}
	}
}
void updatePalette_10() {
	if((paletteAnim&3)==0) {
		int offset = ((paletteAnim&0x1C)>>4)*14;
		offset += 0x1FF6FE;
		for(int j=0; j<7; j++) {
			WORD col = romBuf[offset+(j<<1)]|(romBuf[offset+1+(j<<1)]<<8);
			paletteBuffer[0x49+j] = convColor_SNEStoRGB(col);
		}
	}
}
void updatePalette_11() {
	if((paletteAnim&3)==0) {
		int bg1Ts = ((levelHeader[0]&7)<<1)|(levelHeader[1]>>7);
		int offset = (paletteAnim&0xC)<<1;
		if(bg1Ts==8) offset += 0x1FE2EC;
		else offset += 0x1FA150;
		for(int j=0; j<4; j++) {
			WORD col = romBuf[offset+(j<<1)]|(romBuf[offset+1+(j<<1)]<<8);
			paletteBuffer[0x53+j] = convColor_SNEStoRGB(col);
		}
		offset = (paletteAnim&0x1C)<<3;
		offset += 0x1FF77E;
		for(int j=0; j<8; j++) {
			WORD col = romBuf[offset+(j<<1)]|(romBuf[offset+1+(j<<1)]<<8);
			paletteBuffer[0x64+j] = convColor_SNEStoRGB(col);
		}
	}
	if((paletteAnim%10)==0) {
		int offset = ((paletteAnim/10)&0xF)<<3;
		offset += 0x1FF95E;
		for(int j=0; j<4; j++) {
			WORD col = romBuf[offset+(j<<1)]|(romBuf[offset+1+(j<<1)]<<8);
			paletteBuffer[0x00+j] = convColor_SNEStoRGB(col);
		}
	}
}
void updatePalette_12() {
	if((paletteAnim&3)==0) {
		int offset = (paletteAnim&0xC)<<1;
		offset += 0x1FE2EC;
		for(int j=0; j<4; j++) {
			WORD col = romBuf[offset+(j<<1)]|(romBuf[offset+1+(j<<1)]<<8);
			paletteBuffer[0x53+j] = convColor_SNEStoRGB(col);
		}
		offset = ((paletteAnim&0x1C)>>4)*14;
		offset += 0x1FF6FE;
		for(int j=0; j<7; j++) {
			WORD col = romBuf[offset+(j<<1)]|(romBuf[offset+1+(j<<1)]<<8);
			paletteBuffer[0x49+j] = convColor_SNEStoRGB(col);
		}
	}
	if((paletteAnim%10)==0) {
		int offset = ((paletteAnim/10)&0xF)<<3;
		offset += 0x1FF9DE;
		for(int j=0; j<4; j++) {
			WORD col = romBuf[offset+(j<<1)]|(romBuf[offset+1+(j<<1)]<<8);
			paletteBuffer[0x00+j] = convColor_SNEStoRGB(col);
		}
	}
}
void updatePalette_13() {
	if((paletteAnim&3)==0) {
		int offset = ((paletteAnim&0xC)>>2)*6;
		offset += 0x1FA190;
		for(int j=0; j<3; j++) {
			WORD col = romBuf[offset+(j<<1)]|(romBuf[offset+1+(j<<1)]<<8);
			paletteBuffer[0x05+j] = convColor_SNEStoRGB(col);
		}
		offset = ((paletteAnim&0x1C)>>2)*0x26;
		offset += 0x1FF5F4;
		if(offset==0x1FF6FE) offset -= 0x26;
		for(int j=0; j<13; j++) {
			WORD col = romBuf[offset+(j<<1)]|(romBuf[offset+1+(j<<1)]<<8);
			paletteBuffer[0x43+j] = convColor_SNEStoRGB(col);
		}
		offset = (paletteAnim&0x1C)<<3;
		offset += 0x1FF77E;
		for(int j=0; j<8; j++) {
			WORD col = romBuf[offset+(j<<1)]|(romBuf[offset+1+(j<<1)]<<8);
			paletteBuffer[0x64+j] = convColor_SNEStoRGB(col);
		}
	}
}
void updatePalette_unused() {}
void (*paletteUpdateFunc[0x20])() = {
	//00
	updatePalette_unused,updatePalette_01,updatePalette_02,updatePalette_03,
	updatePalette_04,updatePalette_05,updatePalette_06,updatePalette_07,
	updatePalette_08,updatePalette_09,updatePalette_0A,updatePalette_0B,
	updatePalette_0C,updatePalette_0D,updatePalette_0E,updatePalette_0F,
	//10
	updatePalette_10,updatePalette_11,updatePalette_12,updatePalette_13,
	updatePalette_13,updatePalette_unused,updatePalette_unused,updatePalette_unused,
	updatePalette_unused,updatePalette_unused,updatePalette_unused,updatePalette_unused,
	updatePalette_unused,updatePalette_unused,updatePalette_unused,updatePalette_unused};
void updatePalette() {
	int animPal = levelHeader[7]&0x1F;
	paletteUpdateFunc[animPal]();
	paletteAnim++;
}
void updatePaletteW6(bool dark) {
	vW6_palette = dark;
	loadPalette();
}

///////////////////
//WINDOW FUNCTION//
///////////////////
HDC				hdcPal;
HBITMAP			hbmpPal;
DWORD *			bmpDataPal;
RECT invRect_pal = {0,0,0x100,0x100};

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
		//Input
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

