#ifndef SNESLIB_H
#define SNESLIB_H

//Includes
#include <windows.h>
#include <tchar.h>
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
inline void fillImage(DWORD * pixelBuf,int width,int height,DWORD color);
inline void putPixel(DWORD * pixelBuf,int width,int height,DWORD color,POINT offs);
inline void invertPixel(DWORD * pixelBuf,int width,int height,POINT offs);
inline void hilitePixel(DWORD * pixelBuf,int width,int height,DWORD color,POINT offs);
inline void blendPixel(DWORD * pixelBuf,int width,int height,DWORD color,POINT offs);
inline int getIndexFromTile(BYTE * tileBuf,DWORD tile,POINT offs);
inline int getIndexFromTexture(BYTE * texBuf,POINT offs);
//Address conversion
DWORD convAddr_SNEStoPC_YI(DWORD addr);
DWORD convAddr_PCtoSNES_YI(DWORD addr);
//Compression
DWORD compressLZ1(BYTE * dst,BYTE * src,DWORD size);
DWORD compressLZ16(BYTE * dst,BYTE * src,DWORD numLines);
DWORD decompressLZ1(BYTE * dst,BYTE * src);
void decompressLZ16(BYTE * dst,BYTE * src,DWORD numLines);

#endif

