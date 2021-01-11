#include "sneslib.h"

//////////////////////
//GRAPHICS FUNCTIONS//
//////////////////////
//Palette conversion
DWORD convColor_SNEStoRGB(WORD col) {
	//Get 5-bit components
	BYTE r = col&0x1F;
	BYTE g = (col>>5)&0x1F;
	BYTE b = (col>>10)&0x1F;
	//Convert components from 5-bit to 8-bit
	r = (r<<3)|(r>>2);
	g = (g<<3)|(g>>2);
	b = (b<<3)|(b>>2);
	//Combine components and return result
	return (r<<16)|(g<<8)|b;
}
WORD convColor_RGBtoSNES(DWORD col) {
	//Get 8-bit components
	BYTE r = (col>>16)&0xFF;
	BYTE g = (col>>8)&0xFF;
	BYTE b = col&0xFF;
	//Convert components from 8-bit to 5-bit
	r >>= 3;
	g >>= 3;
	b >>= 3;
	//Combine components and return result
	return r|(g<<5)|(b<<10);
}

//Packing/unpacking graphics
void packGfx2BPP(BYTE * dst,BYTE * src,int numTiles) {
	for(int n = 0; n < numTiles; n++) {
		memset(&dst[n<<4],0,16);
		for(int j = 0; j < 8; j++) {
			for(int i = 0; i < 8; i++) {
				int shift = 7-i;
				BYTE idx = src[(n<<6)|(j<<3)|i];
				BYTE b0 = (idx>>0)&1;
				BYTE b1 = (idx>>1)&1;
				dst[(n<<4)|(j<<1)|0] |= b0<<shift;
				dst[(n<<4)|(j<<1)|1] |= b1<<shift;
			}
		}
	}
}
void packGfx4BPP(BYTE * dst,BYTE * src,int numTiles) {
	for(int n = 0; n < numTiles; n++) {
		memset(&dst[n<<5],0,32);
		for(int j = 0; j < 8; j++) {
			for(int i = 0; i < 8; i++) {
				int shift = 7-i;
				BYTE idx = src[(n<<6)|(j<<3)|i];
				BYTE b0 = (idx>>0)&1;
				BYTE b1 = (idx>>1)&1;
				BYTE b2 = (idx>>2)&1;
				BYTE b3 = (idx>>3)&1;
				dst[(n<<5)|(j<<1)|0x00] |= b0<<shift;
				dst[(n<<5)|(j<<1)|0x01] |= b1<<shift;
				dst[(n<<5)|(j<<1)|0x10] |= b2<<shift;
				dst[(n<<5)|(j<<1)|0x11] |= b3<<shift;
			}
		}
	}
}
void unpackGfx2BPP(BYTE * dst,BYTE * src,int numTiles) {
	for(int n = 0; n < numTiles; n++) {
		memset(&dst[n<<6],0,64);
		for(int j = 0; j < 8; j++) {
			for(int i = 0; i < 8; i++) {
				int shift = 7-i;
				BYTE b0 = (src[(n<<4)|(j<<1)|0]>>shift)&1;
				BYTE b1 = (src[(n<<4)|(j<<1)|1]>>shift)&1;
				dst[(n<<6)|(j<<3)|i] = b0|(b1<<1);
			}
		}
	}
}
void unpackGfx4BPP(BYTE * dst,BYTE * src,int numTiles) {
	for(int n = 0; n < numTiles; n++) {
		memset(&dst[n<<6],0,64);
		for(int j = 0; j < 8; j++) {
			for(int i = 0; i < 8; i++) {
				int shift = 7-i;
				BYTE b0 = (src[(n<<5)|(j<<1)|0x00]>>shift)&1;
				BYTE b1 = (src[(n<<5)|(j<<1)|0x01]>>shift)&1;
				BYTE b2 = (src[(n<<5)|(j<<1)|0x02]>>shift)&1;
				BYTE b3 = (src[(n<<5)|(j<<1)|0x03]>>shift)&1;
				dst[(n<<6)|(j<<3)|i] = b0|(b1<<1)|(b2<<2)|(b3<<3);
			}
		}
	}
}

//Pixel manipulation
inline void fillImage(DWORD * pixelBuf,int width,int height,DWORD color) {
	int numPixels = width*height;
	for(int i = 0; i < numPixels; i++) {
		pixelBuf[i] = color;
	}
}
inline void putPixel(DWORD * pixelBuf,int width,int height,DWORD color,int offsX,int offsY) {
	if(offsX >= 0 && offsY >= 0 && offsX < width && offsY < height) {
		pixelBuf[(offsY*width)+offsX] = color;
	}
}
inline void invertPixel(DWORD * pixelBuf,int width,int height,int offsX,int offsY) {
	if(offsX >= 0 && offsY >= 0 && offsX < width && offsY < height) {
		pixelBuf[(offsY*width)+offsX] ^= 0xFFFFFF;
	}
}
inline void hilitePixel(DWORD * pixelBuf,int width,int height,DWORD color,int offsX,int offsY) {
	if(offsX >= 0 && offsY >= 0 && offsX < width && offsY < height) {
		DWORD orig = pixelBuf[(offsY*width)+offsX];
		DWORD r = (orig&0xFF0000)+(color&0xFF0000);
		if(r > 0xFF0000) r = 0xFF0000;
		DWORD g = (orig&0xFF00)+(color&0xFF00);
		if(g > 0xFF00) g = 0xFF00;
		DWORD b = (orig&0xFF)+(color&0xFF);
		if(b > 0xFF) b = 0xFF;
		pixelBuf[(offsY*width)+offsX] = r|g|b;
	}
}
inline void blendPixel(DWORD * pixelBuf,int width,int height,DWORD color,int offsX,int offsY) {
	if(offsX >= 0 && offsY >= 0 && offsX < width && offsY < height) {
		DWORD orig = pixelBuf[(offsY*width)+offsX];
		DWORD r = (orig&0xFF0000)+(color&0xFF0000);
		r = (r>>1)&0xFF0000;
		DWORD g = (orig&0xFF00)+(color&0xFF00);
		g = (g>>1)&0xFF00;
		DWORD b = (orig&0xFF)+(color&0xFF);
		b >>= 1;
		pixelBuf[(offsY*width)+offsX] = color;
	}
}

//Tile drawing
inline int getIndexFromTile(BYTE * tileBuf,DWORD tile,int offsX,int offsY) {
	return tileBuf[(tile<<6)|(offsX<<3)|offsY];
}
void drawTile(DWORD * pixelBuf,int width,int height,DWORD * palBuf,BYTE * tileBuf,BOOL flipV,BOOL flipH,int palette,DWORD tile,int offsX,int offsY) {
	for(int j = 0; j < 8; j++) {
		for(int i = 0; i < 8; i++) {
			int sx = flipH?(7-i):i;
			int sy = flipV?(7-j):j;
			int dx = offsX+i;
			int dy = offsY+j;
			int idx = getIndexFromTile(tileBuf,tile,sx,sy);
			if(idx != 0) {
				putPixel(pixelBuf,width,height,palBuf[palette|idx],dx,dy);
			}
		}
	}
}
void drawTileBlend(DWORD * pixelBuf,int width,int height,DWORD * palBuf,BYTE * tileBuf,BOOL flipV,BOOL flipH,int palette,DWORD tile,int offsX,int offsY) {
	for(int j = 0; j < 8; j++) {
		for(int i = 0; i < 8; i++) {
			int sx = flipH?(7-i):i;
			int sy = flipV?(7-j):j;
			int dx = offsX+i;
			int dy = offsY+j;
			int idx = getIndexFromTile(tileBuf,tile,sx,sy);
			if(idx != 0) {
				blendPixel(pixelBuf,width,height,palBuf[palette|idx],dx,dy);
			}
		}
	}
}

//////////////////////
//ADDRESS CONVERSION//
//////////////////////
DWORD convAddr_SNEStoPC_YI(DWORD addr) {
	if(addr&0x800000) {
		//TODO
		return 0;
	} else if(addr&0x400000) {
		return addr&0x3FFFFF;
	} else {
		return (addr&0x7FFF)|((addr&0x7F0000)>>1);
	}
}
DWORD convAddr_PCtoSNES_YI(DWORD addr) {
	if(addr>0x200000) {
		//TODO
		return 0;
	} else if(addr>0xC0000) {
		return addr|0x400000;
	} else {
		return (addr&0x7FFF)|((addr&0x3F8000)<<1)|0x8000;
	}
}

///////////////
//COMPRESSION//
///////////////
//Helper functions for reading/writing bitvalues (LZ16)
DWORD readBits(BYTE * data,int bitOffset,int bitLength) {
	int byteOffset = 0;
	DWORD value = 0;
	for(int i=0; i<bitLength; i++) {
		//Append bit
		DWORD bit = (data[byteOffset]>>bitOffset)&1;
		value |= bit<<i;
		//Move to next bit
		bitOffset++;
		if(bitOffset==8) {
			bitOffset = 0;
			byteOffset++;
		}
	}
	return value;
}
void writeBits(BYTE * data,int bitOffset,int bitLength,DWORD value) {
	int byteOffset = 0;
	for(int i=0; i<bitLength; i++) {
		//Set bit
		data[byteOffset] &= ~(1<<bitOffset);
		DWORD bit = (value>>i)&1;
		data[byteOffset] |= bit<<bitOffset;
		//Move to next bit
		bitOffset++;
		if(bitOffset==8) {
			bitOffset = 0;
			byteOffset++;
		}
	}
}
DWORD reverseBitOrder(DWORD value,int bitLength) {
	DWORD revVal = 0;
	for(int i=0; i<bitLength; i++) {
		DWORD bit = (value>>i)&1;
		value |= bit<<(bitLength-i-1);
	}
}

//Sort comparator for palette indexes (LZ16)
typedef struct {
	BYTE paletteIndex;
	int numPixelsWithValue;
} palette_sorter_t;
bool paletteSorter_opLt(palette_sorter_t & lhs,palette_sorter_t & rhs) {
	return (lhs.numPixelsWithValue<rhs.numPixelsWithValue);
}

//Compression/decompression
DWORD compressLZ1(BYTE * dst,BYTE * src,DWORD size) {
	int srcOff = 0,dstOff = 0;
	int directStart = 0,directLen = 0;
	while(srcOff < size) {
		int cmd1len = 0,cmd1rel = 0;
		int cmd2len = 0,cmd2rel = 0;
		int cmd3len = 0,cmd3rel = 0;
		int cmd4len = 0,cmd4rel = 0;
		//Determine max length for command 1 (byte fill)
		BYTE basis1 = src[srcOff];
		int offs1 = 1;
		while(true) {
			if(src[srcOff+offs1] != basis1) break;
			cmd1len++;
			offs1++;
		}
		if(cmd1len > 1024) cmd1len = 1024;
		if(cmd1len < 32) {
			cmd1rel = cmd1len-2;
		} else {
			cmd1rel = cmd1len-3;
		}
		//Determine max length for command 2 (word fill)
		BYTE basis2 = src[srcOff+1];
		int offs2 = 2;
		while(true) {
			if(src[srcOff+offs2] != ((offs2&1)?basis2:basis1)) break;
			cmd2len++;
			offs2++;
		}
		if(cmd2len > 1024) cmd2len = 1024;
		if(cmd2len < 32) {
			cmd2rel = cmd2len-3;
		} else {
			cmd2rel = cmd2len-4;
		}
		//Determine max length for command 3 (byte fill incremented)
		BYTE basis3 = basis1+1;
		int offs3 = 1;
		while(true) {
			if(src[srcOff+offs3] != basis3) break;
			cmd3len++;
			basis3++;
			offs3++;
		}
		if(cmd3len > 1024) cmd3len = 1024;
		if(cmd3len < 32) {
			cmd3rel = cmd3len-2;
		} else {
			cmd3rel = cmd3len-3;
		}
		//Determine max length for command 4 (copy)
		//TODO
		if(cmd4len > 1024) cmd4len = 1024;
		if(cmd4len < 32) {
			cmd4rel = cmd4len-3;
		} else {
			cmd4rel = cmd4len-4;
		}
		//Output command which saves the most space (if any)
		if(cmd1rel > cmd2rel && cmd1rel > cmd3rel && cmd1rel > cmd4rel && cmd1rel > 0) {
			//Output previous direct bytes
			//TODO
			//Output command 1 (byte fill)
			if(cmd1len < 32) {
				//TODO
			} else {
				//TODO
			}
		} else if(cmd2rel > cmd3rel && cmd2rel > cmd4rel && cmd2rel > 0) {
			//Output previous direct bytes
			//TODO
			//Output command 2 (word fill)
			if(cmd2len < 32) {
				//TODO
			} else {
				//TODO
			}
		} else if(cmd3rel > cmd4rel && cmd3rel > 0) {
			//Output previous direct bytes
			//TODO
			//Output command 3 (byte fill incremented)
			if(cmd3len < 32) {
				//TODO
			} else {
				//TODO
			}
		} else if(cmd4rel > 0) {
			//Output previous direct bytes
			//TODO
			//Output command 4 (copy)
			if(cmd4len < 32) {
				//TODO
			} else {
				//TODO
			}
		//Otherwise, mark as direct and move on
		} else {
			directLen++;
			srcOff++;
		}
	}
	//Output remaining direct bytes and add terminator
	//TODO
	dst[dstOff++] = 0xFF;
	return dstOff;
}
DWORD compressLZ16(BYTE * dst,BYTE * src,DWORD size) {
	int srcOff = 0,dstOff = 0;
	int srcBitOff = 0;
	std::vector<palette_sorter_t> palette;
	BYTE prevLine[0x80];
	//TODO
}
DWORD decompressLZ1(BYTE * dst,BYTE * src) {
	int srcOff = 0,dstOff = 0;
	while(true) {
		BYTE bc = src[srcOff++];
		if(bc == 0xFF) break;
		int cmd = bc>>5;
		int len = (bc&0x1F)+1;
		switch(cmd) {
			//Command 0 (direct)
			case 0: {
				memcpy(&dst[dstOff],&src[srcOff],len);
				srcOff += len;
				dstOff += len;
				break;
			}
			//Command 1 (byte fill)
			case 1: {
				BYTE b0 = src[srcOff++];
				memset(&dst[dstOff],b0,len);
				dstOff += len;
				break;
			}
			//Command 2 (word fill)
			case 2: {
				BYTE b0 = src[srcOff++];
				BYTE b1 = src[srcOff++];
				for(int i = 0; i < len; i++) {
					dst[dstOff++] = (i&1)?b1:b0;
				}
				break;
			}
			//Command 3 (byte fill incremented)
			case 3: {
				BYTE b0 = src[srcOff++];
				for(int i = 0; i < len; i++) {
					dst[dstOff++] = b0++;
				}
				break;
			}
			//Command 4 (copy)
			case 4: {
				int cpyOff = src[srcOff++]|(src[srcOff++]<<8);
				memcpy(&dst[dstOff],&src[cpyOff],len);
				srcOff += len;
				dstOff += len;
				break;
			}
			//Command 7 (long mode)
			case 7: {
				cmd = (bc>>2)&7;
				len = ((bc&3)|src[srcOff++])+1;
				switch(cmd) {
					//Command 0 (direct)
					case 0: {
						memcpy(&dst[dstOff],&src[srcOff],len);
						srcOff += len;
						dstOff += len;
						break;
					}
					//Command 1 (byte fill)
					case 1: {
						BYTE b0 = src[srcOff++];
						memset(&dst[dstOff],b0,len);
						dstOff += len;
						break;
					}
					//Command 2 (word fill)
					case 2: {
						BYTE b0 = src[srcOff++];
						BYTE b1 = src[srcOff++];
						for(int i = 0; i < len; i++) {
							dst[dstOff++] = (i&1)?b1:b0;
						}
						break;
					}
					//Command 3 (byte fill incremented)
					case 3: {
						BYTE b0 = src[srcOff++];
						for(int i = 0; i < len; i++) {
							dst[dstOff++] = b0++;
						}
						break;
					}
					//Command 4 (copy)
					case 4: {
						int cpyOff = src[srcOff++]|(src[srcOff++]<<8);
						memcpy(&dst[dstOff],&src[cpyOff],len);
						srcOff += len;
						dstOff += len;
						break;
					}
				}
				break;
			}
		}
	}
	return dstOff;
}
void decompressLZ16(BYTE * dst,BYTE * src,DWORD size) {
	int srcOff = 0,dstOff = 0;
	int srcBitOff = 0;
	BYTE palette[7];
	BYTE prevLine[0x80];
	//TODO
}

