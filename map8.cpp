#include "map8.h"

HWND hwndMap8;
bool wvisMap8 = false;
BYTE map8Buffer[0x19000];
BYTE fontBuffer[0x2000];
BYTE commonBuffer[0x3D000];
int map8Anim = 0;
int map8State = 0;
bool vW6_map8 = false;

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
	if(vW6_map8) bg1Ts += 0x10;
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
	//Load common animated tiles
	unpackGfx4BPP(&commonBuffer[0x00000],&romBuf[0x120000],0x800);
	unpackGfx2BPP(&commonBuffer[0x20000],&romBuf[0x168000],0x680);
	unpackGfx4BPP(&commonBuffer[0x3A000],&romBuf[0x16E800],0xC0);
	//Init animation frame
	map8Anim = 0;
	updateMap8();
}
void updateMap8_common() {
	if((map8Anim&3)==0) {
		int offset = (map8Anim&0x0C)<<2;
		for(int i=0; i<4; i++) {
			//Get states
			int animCaps = romBuf[0x00561D+offset];
			int offset2 = offset;
			if(animCaps&map8State) offset2 += 2;
			//Copy over
			int src = romBuf[0x0055DD+offset2]|(romBuf[0x0055DE +offset2]<<8);
			src <<= 1;
			int dst = 0x9000+(i<<8);
			memcpy(&map8Buffer[dst],&commonBuffer[src],0x100);
			//Move on to next entry
			offset += 4;
		}
	}
}
void updateMap8_00() {
	updateMap8_common();
	if(map8Anim==0) memset(&map8Buffer[0x8000],0,0x1000);
}
void updateMap8_01() {
	updateMap8_common();
	if((map8Anim&3)==0) {
		int offset = (map8Anim&0x0C)<<9;
		int src = 0x22000+offset;
		memcpy(&map8Buffer[0x11800],&commonBuffer[src],0x800);
	}
}
void updateMap8_02() {
	updateMap8_common();
	if((map8Anim&3)==0) {
		int offset = (map8Anim&0x0C)<<8;
		int src = 0x1A000+offset;
		memcpy(&map8Buffer[0x8000],&commonBuffer[src],0x200);
		memcpy(&map8Buffer[0x8200],&commonBuffer[src+0x1000],0x200);
		memcpy(&map8Buffer[0x8400],&commonBuffer[src+0x200],0x200);
		memcpy(&map8Buffer[0x8600],&commonBuffer[src+0x1200],0x200);
		memcpy(&map8Buffer[0x8800],&commonBuffer[0x18000],0x200);
		memcpy(&map8Buffer[0x8A00],&commonBuffer[0x18400],0x200);
		memcpy(&map8Buffer[0x8C00],&commonBuffer[0x18200],0x200);
		memcpy(&map8Buffer[0x8E00],&commonBuffer[0x18600],0x200);
	}
}
void updateMap8_03() {
	updateMap8_common();
	if((map8Anim&1)==0) {
		int offset = (map8Anim&6)<<10;
		int src = 0x24000+offset;
		memcpy(&map8Buffer[0x11800],&commonBuffer[src],0x800);
	}
}
void updateMap8_05() {
	updateMap8_common();
	if((map8Anim&1)==0) {
		int offset = (map8Anim%28)<<10;
		if(offset>(14<<10)) offset = (28<<10)-offset;
		int src = 0x26000+offset;
		memcpy(&map8Buffer[0x11800],&commonBuffer[src],0x800);
	}
}
void updateMap8_06() {
	updateMap8_common();
	if((map8Anim%3)==0) {
		int offset = ((map8Anim/3)&7)<<11;
		int src = 0x26000+offset;
		memcpy(&map8Buffer[0x11800],&commonBuffer[src],0x800);
	}
}
void updateMap8_07() {
	updateMap8_common();
	if((map8Anim%6)==0) {
		int offset = ((map8Anim/6)&3)<<10;
		int bg1Ts = ((levelHeader[0]&7)<<1)|(levelHeader[1]>>7);
		if(bg1Ts==10) {
			int src = 0x3A800+offset;
			memcpy(&map8Buffer[0x8000],&commonBuffer[src],0x200);
			memcpy(&map8Buffer[0x8200],&commonBuffer[src+0x1000],0x200);
			memcpy(&map8Buffer[0x8400],&commonBuffer[src+0x200],0x200);
			memcpy(&map8Buffer[0x8600],&commonBuffer[src+0x1200],0x200);
		} else {
			int src = 0x19000+offset;
			memcpy(&map8Buffer[0x8000],&commonBuffer[src],0x200);
			memcpy(&map8Buffer[0x8200],&commonBuffer[src+0x4800],0x200);
			memcpy(&map8Buffer[0x8400],&commonBuffer[src+0x200],0x200);
			memcpy(&map8Buffer[0x8600],&commonBuffer[src+0x4A00],0x200);
		}
	}
}
void updateMap8_08() {
	updateMap8_common();
	if((map8Anim&3)==0) {
		int offset = (map8Anim&0x0C)<<8;
		int src = 0x1C800+offset;
		memcpy(&map8Buffer[0x8000],&commonBuffer[src],0x200);
		memcpy(&map8Buffer[0x8400],&commonBuffer[src+0x200],0x200);
	}
}
void updateMap8_09() {
	updateMap8_common();
	if((map8Anim&1)==0) {
		int offset = (map8Anim&6)<<10;
		int src = 0x20000+offset;
		memcpy(&map8Buffer[0x11800],&commonBuffer[src],0x800);
	}
}
void updateMap8_0A() {
	updateMap8_common();
	if((map8Anim&3)==0) {
		int offset = (map8Anim&0x1C)<<9;
		int src = 0x2C000+offset;
		memcpy(&map8Buffer[0x11800],&commonBuffer[src],0x800);
	}
}
const int animMap8ButterflyCmp[14] = {0,5,7,9,
	11,13,15,17,
	22,24,26,28,
	30,32};
const int animMap8ButterflyVal[14] = {0x0000,0x0800,0x1000,0x1800,
	0x2000,0x2800,0x3000,0x3800,
	0x3000,0x2800,0x2000,0x1800,
	0x1000,0x0800};	
void updateMap8_0B() {
	updateMap8_common();
	if((map8Anim&3)==0) {
		int offset = (map8Anim&0x0C)<<8;
		int src = 0x1A000+offset;
		memcpy(&map8Buffer[0x8000],&commonBuffer[src],0x200);
		memcpy(&map8Buffer[0x8200],&commonBuffer[src+0x1000],0x200);
		memcpy(&map8Buffer[0x8400],&commonBuffer[src+0x200],0x200);
		memcpy(&map8Buffer[0x8600],&commonBuffer[src+0x1200],0x200);
		memcpy(&map8Buffer[0x8800],&commonBuffer[0x18000],0x200);
		memcpy(&map8Buffer[0x8A00],&commonBuffer[0x18400],0x200);
		memcpy(&map8Buffer[0x8C00],&commonBuffer[0x18200],0x200);
		memcpy(&map8Buffer[0x8E00],&commonBuffer[0x18600],0x200);
	}
	for(int i=0; i<14; i++) {
		if((map8Anim%34)==animMap8ButterflyCmp[i]) {
			int offset = animMap8ButterflyVal[i];
			int src = 0x2C000+offset;
			memcpy(&map8Buffer[0x11800],&commonBuffer[src],0x800);
			break;
		}
	}
}
const int animMap8WaterCmp[6] = {0,8,14,20,28,34};
const int animMap8WaterVal[6] = {0,4,8,12,8,4};
void updateMap8_0C() {
	updateMap8_common();
	for(int i=0; i<6; i++) {
		if((map8Anim%40)==animMap8WaterCmp[i]) {
			int offset = animMap8WaterVal[i];
			int offset1 = romBuf[0x005A29+offset]|(romBuf[0x005A2A+offset]<<8);
			int offset2 = romBuf[0x005A41+offset]|(romBuf[0x005A42+offset]<<8);
			int src1 = offset1<<1;
			int src2 = offset2<<1;
			memcpy(&map8Buffer[0x8000],&commonBuffer[src1],0x200);
			memcpy(&map8Buffer[0x8200],&commonBuffer[src2],0x200);
			memcpy(&map8Buffer[0x8400],&commonBuffer[src1+0x200],0x200);
			memcpy(&map8Buffer[0x8600],&commonBuffer[src2+0x200],0x200);
			break;
		}
	}
}
void updateMap8_0D() {
	updateMap8_common();
	if((map8Anim%6)==0) {
		int offset = ((map8Anim/6)&3)<<10;
		int bg1Ts = ((levelHeader[0]&7)<<1)|(levelHeader[1]>>7);
		if(bg1Ts==10) {
			int src = 0x3A800+offset;
			memcpy(&map8Buffer[0x8000],&commonBuffer[src],0x200);
			memcpy(&map8Buffer[0x8200],&commonBuffer[src+0x1000],0x200);
			memcpy(&map8Buffer[0x8400],&commonBuffer[src+0x200],0x200);
			memcpy(&map8Buffer[0x8600],&commonBuffer[src+0x1200],0x200);
		} else {
			int src = 0x19000+offset;
			memcpy(&map8Buffer[0x8000],&commonBuffer[src],0x200);
			memcpy(&map8Buffer[0x8200],&commonBuffer[src+0x4800],0x200);
			memcpy(&map8Buffer[0x8400],&commonBuffer[src+0x200],0x200);
			memcpy(&map8Buffer[0x8600],&commonBuffer[src+0x4A00],0x200);
		}
	}
	if((map8Anim%3)==0) {
		int offset = ((map8Anim/3)&7)<<11;
		int src = 0x26000+offset;
		memcpy(&map8Buffer[0x11800],&commonBuffer[src],0x800);
	}
}
void updateMap8_0E() {
	updateMap8_common();
	for(int i=0; i<6; i++) {
		if((map8Anim%40)==animMap8WaterCmp[i]) {
			int offset = animMap8WaterVal[i];
			int offset1 = romBuf[0x005A29+offset]|(romBuf[0x005A2A+offset]<<8);
			int offset2 = romBuf[0x005A41+offset]|(romBuf[0x005A42+offset]<<8);
			int src1 = offset1<<1;
			int src2 = offset2<<1;
			memcpy(&map8Buffer[0x8000],&commonBuffer[src1],0x200);
			memcpy(&map8Buffer[0x8200],&commonBuffer[src2],0x200);
			memcpy(&map8Buffer[0x8400],&commonBuffer[src1+0x200],0x200);
			memcpy(&map8Buffer[0x8600],&commonBuffer[src2+0x200],0x200);
			break;
		}
	}
	int offset = (map8Anim&7)<<11;
	int src = 0x26000+offset;
	memcpy(&map8Buffer[0x11800],&commonBuffer[src],0x800);
}
void updateMap8_0F() {
	updateMap8_common();
	if((map8Anim%6)==0) {
		int offset = ((map8Anim/6)&3)<<11;
		int src = 0x2A000+offset;
		memcpy(&map8Buffer[0x11800],&commonBuffer[src],0x800);
	}
}
void updateMap8_10() {
	updateMap8_common();
	//TODO
}
void updateMap8_11() {
	updateMap8_common();
	for(int i=0; i<6; i++) {
		if((map8Anim%40)==animMap8WaterCmp[i]) {
			int offset = animMap8WaterVal[i];
			int offset1 = romBuf[0x005A29+offset]|(romBuf[0x005A2A+offset]<<8);
			int offset2 = romBuf[0x005A41+offset]|(romBuf[0x005A42+offset]<<8);
			int src1 = offset1<<1;
			int src2 = offset2<<1;
			memcpy(&map8Buffer[0x8000],&commonBuffer[src1],0x200);
			memcpy(&map8Buffer[0x8200],&commonBuffer[src2],0x200);
			memcpy(&map8Buffer[0x8400],&commonBuffer[src1+0x200],0x200);
			memcpy(&map8Buffer[0x8600],&commonBuffer[src2+0x200],0x200);
			break;
		}
	}
	if((map8Anim&1)==0) {
		int offset = (map8Anim&6)<<10;
		int src = 0x24000+offset;
		memcpy(&map8Buffer[0x11800],&commonBuffer[src],0x800);
	}
}
void updateMap8_unused() {}
void (*map8UpdateFunc[0x40])() = {
	//00
	updateMap8_00,updateMap8_01,updateMap8_02,updateMap8_03,
	updateMap8_unused,updateMap8_05,updateMap8_06,updateMap8_07,
	updateMap8_08,updateMap8_09,updateMap8_0A,updateMap8_0B,
	updateMap8_0C,updateMap8_0D,updateMap8_0E,updateMap8_0F,
	//10
	updateMap8_10,updateMap8_11,updateMap8_unused,updateMap8_unused,
	updateMap8_unused,updateMap8_unused,updateMap8_unused,updateMap8_unused,
	updateMap8_unused,updateMap8_unused,updateMap8_unused,updateMap8_unused,
	updateMap8_unused,updateMap8_unused,updateMap8_unused,updateMap8_unused,
	//20
	updateMap8_unused,updateMap8_unused,updateMap8_unused,updateMap8_unused,
	updateMap8_unused,updateMap8_unused,updateMap8_unused,updateMap8_unused,
	updateMap8_unused,updateMap8_unused,updateMap8_unused,updateMap8_unused,
	updateMap8_unused,updateMap8_unused,updateMap8_unused,updateMap8_unused,
	//30
	updateMap8_unused,updateMap8_unused,updateMap8_unused,updateMap8_unused,
	updateMap8_unused,updateMap8_unused,updateMap8_unused,updateMap8_unused,
	updateMap8_unused,updateMap8_unused,updateMap8_unused,updateMap8_unused,
	updateMap8_unused,updateMap8_unused,updateMap8_unused,updateMap8_unused};
void updateMap8() {
	int animTs = ((levelHeader[6]&7)<<3)|(levelHeader[7]>>5);
	map8UpdateFunc[animTs]();
	map8Anim++;
}
void updateMap8Sw(int state) {
	map8State = state;
	map8Anim = 0;
	updateMap8();
}
void updateMap8W6(bool dark) {
	vW6_map8 = dark;
	loadMap8();
}

void dispMap8Pixel(DWORD * pixelBuf,int width,int height,DWORD color,int idx,int offsX,int offsY,BYTE inv) {
	if(idx) {
		if(inv&0x08) {
			color ^= 0xFFFFFF;
		}
		DWORD ored = 0;
		if(inv&0x04) ored |= 0xFF0000;
		if(inv&0x02) ored |= 0x00FF00;
		if(inv&0x01) ored |= 0x0000FF;
		color |= ored;
		putPixel(pixelBuf,width,height,color,offsX,offsY);
	} else {
		if(inv&0x80) {
			invertPixel(pixelBuf,width,height,offsX,offsY);
		}
		DWORD ored = 0;
		if(inv&0x04) ored |= 0xFF0000;
		if(inv&0x02) ored |= 0x00FF00;
		if(inv&0x01) ored |= 0x0000FF;
		hilitePixel(pixelBuf,width,height,ored,offsX,offsY);
	}
}
void dispMap8Tile(DWORD * pixelBuf,int width,int height,BYTE props,WORD tile,int offsX,int offsY,RECT * clip,BYTE inv) {
	bool flipV = props&0x80;
	bool flipH = props&0x40;
	int palette = (props&0x3C)<<2;
	if(props&2) palette >>= 2;
	bool size = props&1;
	if(size) {
		for(int j=clip->top; j<clip->bottom; j++) {
			for(int i=clip->left; i<clip->right; i++) {
				int si = i&7;
				int sj = j&7;
				int sx = flipH?(7-si):si;
				int sy = flipV?(7-sj):sj;
				int toff = ((i&8)>>3)|((j&8)<<1);
				if(flipH) toff ^= 0x01;
				if(flipV) toff ^= 0x10;
				int dx = offsX+i;
				int dy = offsY+j;
				if(tile&0x2000) {
					int idx = getIndexFromTile(commonBuffer,(tile+toff)&0x1FFF,{sx,sy});
					dispMap8Pixel(pixelBuf,width,height,paletteBuffer[palette|idx],idx,dx,dy,inv);
				} else {
					int idx = getIndexFromTile(map8Buffer,tile+toff,{sx,sy});
					dispMap8Pixel(pixelBuf,width,height,paletteBuffer[palette|idx],idx,dx,dy,inv);
				}
			}
		}
	} else {
		for(int j=clip->top; j<clip->bottom; j++) {
			for(int i=clip->left; i<clip->right; i++) {
				int sx = flipH?(7-i):i;
				int sy = flipV?(7-j):j;
				int dx = offsX+i;
				int dy = offsY+j;
				if(tile&0x2000) {
					int idx = getIndexFromTile(commonBuffer,tile&0x1FFF,{sx,sy});
					dispMap8Pixel(pixelBuf,width,height,paletteBuffer[palette|idx],idx,dx,dy,inv);
				} else {
					int idx = getIndexFromTile(map8Buffer,tile,{sx,sy});
					dispMap8Pixel(pixelBuf,width,height,paletteBuffer[palette|idx],idx,dx,dy,inv);
				}
			}
		}
	}
}
void dispMap8Char(DWORD * pixelBuf,int width,int height,DWORD fgCol,DWORD bgCol,char c,int offsX,int offsY,RECT * clip,BYTE inv) {
	for(int j=clip->top; j<clip->bottom; j++) {
		for(int i=clip->left; i<clip->right; i++) {
			int dx = offsX+i;
			int dy = offsY+j;
			int idx = getIndexFromTile(fontBuffer,c,{i,j});
			DWORD col = (idx==3)?fgCol:bgCol;
			dispMap8Pixel(pixelBuf,width,height,col,idx,dx,dy,inv);
		}
	}
}

///////////////////
//WINDOW FUNCTION//
///////////////////
HDC				hdcMap8;
HBITMAP			hbmpMap8;
DWORD *			bmpDataMap8;
WORD map8Base = 0;
BYTE map8Pal = 0;
RECT invRect_map8 = {0,0,0x100,0x100};

//Main drawing code
void updateEntireScreen_map8() {
	RECT clipRect = {0,0,8,8};
	memset(bmpDataMap8,0x80,0x4000*sizeof(DWORD));
	for(int j=0; j<0x10; j++) {
		WORD row = map8Base+(j<<4);
		BYTE props = map8Pal<<2;
		if(row>=0x480) props |= 0x20;
		else if(row>=0x380) props |= 2;
		for(int i=0; i<0x10; i++) {
			dispMap8Tile(bmpDataMap8,0x80,0x80,props,row|i,i<<3,j<<3,&clipRect,0);
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
			switch(wParam) {
				case VK_DOWN: {
					if(map8Base<0x540) {
						if(GetAsyncKeyState(VK_CONTROL)&0x8000) map8Base += 0x100;
						else map8Base += 0x10;
						if(map8Base>0x540) map8Base = 0x540;
						InvalidateRect(hwnd,&invRect_map8,false);
						UpdateWindow(hwnd);
					}
					break;
				}
				case VK_UP: {
					if(map8Base) {
						if(GetAsyncKeyState(VK_CONTROL)&0x8000) map8Base -= 0x100;
						else map8Base -= 0x10;
						if(map8Base>0x540) map8Base = 0;
						InvalidateRect(hwnd,&invRect_map8,false);
						UpdateWindow(hwnd);
					}
					break;
				}
				case VK_RIGHT: {
					map8Pal++;
					map8Pal &= 7;
					InvalidateRect(hwnd,&invRect_map8,false);
					UpdateWindow(hwnd);
					break;
				}
				case VK_LEFT: {
					map8Pal--;
					map8Pal &= 7;
					InvalidateRect(hwnd,&invRect_map8,false);
					UpdateWindow(hwnd);
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

