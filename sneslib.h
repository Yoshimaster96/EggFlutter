#ifndef SNESLIB_H
#define SNESLIB_H

//Includes
#include <windows.h>
#include <windowsx.h>
#include <commctrl.h>
#include <tchar.h>
#include <time.h>
#include <cstdio>
#include <algorithm>
#include <vector>
#include <string>

//Functions
//Graphics functions
DWORD convColor_SNEStoRGB(WORD col);
WORD convColor_RGBtoSNES(DWORD col);
void packGfx2BPP(BYTE * dst,BYTE * src,int numTiles);
void packGfx4BPP(BYTE * dst,BYTE * src,int numTiles);
void unpackGfx2BPP(BYTE * dst,BYTE * src,int numTiles);
void unpackGfx4BPP(BYTE * dst,BYTE * src,int numTiles);
void fillImage(DWORD * pixelBuf,int width,int height,DWORD color);
void putPixel(DWORD * pixelBuf,int width,int height,DWORD color,int offsX,int offsY);
void invertPixel(DWORD * pixelBuf,int width,int height,int offsX,int offsY);
void hilitePixel(DWORD * pixelBuf,int width,int height,DWORD color,int offsX,int offsY);
int getIndexFromTile(BYTE * tileBuf,DWORD tile,POINT offs);
int getIndexFromTexture(BYTE * texBuf,POINT offs);
//Address conversion
DWORD convAddr_SNEStoPC_YI(DWORD addr);
DWORD convAddr_PCtoSNES_YI(DWORD addr);
//Compression
//DWORD compressLZ1(BYTE * dst,BYTE * src,DWORD size);
//DWORD compressLZ16(BYTE * dst,BYTE * src,DWORD numLines);
DWORD decompressLZ1(BYTE * dst,BYTE * src);
void decompressLZ16(BYTE * dst,BYTE * src,DWORD numLines);

#endif

