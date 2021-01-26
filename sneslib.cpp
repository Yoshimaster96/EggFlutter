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
				BYTE b2 = (src[(n<<5)|(j<<1)|0x10]>>shift)&1;
				BYTE b3 = (src[(n<<5)|(j<<1)|0x11]>>shift)&1;
				dst[(n<<6)|(j<<3)|i] = b0|(b1<<1)|(b2<<2)|(b3<<3);
			}
		}
	}
}

//Pixel manipulation
void fillImage(DWORD * pixelBuf,int width,int height,DWORD color) {
	int numPixels = width*height;
	for(int i=0; i<numPixels; i++) {
		pixelBuf[i] = color;
	}
}
void putPixel(DWORD * pixelBuf,int width,int height,DWORD color,POINT offs) {
	if(offs.x>=0 && offs.y>=0 && offs.x<width && offs.y<height) {
		pixelBuf[(offs.y*width)+offs.x] = color;
	}
}
void invertPixel(DWORD * pixelBuf,int width,int height,POINT offs) {
	if(offs.x>=0 && offs.y>=0 && offs.x<width && offs.y<height) {
		pixelBuf[(offs.y*width)+offs.x] ^= 0xFFFFFF;
	}
}
void hilitePixel(DWORD * pixelBuf,int width,int height,DWORD color,POINT offs) {
	if(offs.x>=0 && offs.y>=0 && offs.x<width && offs.y<height) {
		DWORD orig = pixelBuf[(offs.y*width)+offs.x];
		DWORD r = (orig&0xFF0000)+(color&0xFF0000);
		if(r > 0xFF0000) r = 0xFF0000;
		DWORD g = (orig&0xFF00)+(color&0xFF00);
		if(g > 0xFF00) g = 0xFF00;
		DWORD b = (orig&0xFF)+(color&0xFF);
		if(b > 0xFF) b = 0xFF;
		pixelBuf[(offs.y*width)+offs.x] = r|g|b;
	}
}
void blendPixel(DWORD * pixelBuf,int width,int height,DWORD color,POINT offs) {
	if(offs.x>=0 && offs.y>=0 && offs.x<width && offs.y<height) {
		DWORD orig = pixelBuf[(offs.y*width)+offs.x];
		DWORD r = (orig&0xFF0000)+(color&0xFF0000);
		r = (r>>1)&0xFF0000;
		DWORD g = (orig&0xFF00)+(color&0xFF00);
		g = (g>>1)&0xFF00;
		DWORD b = (orig&0xFF)+(color&0xFF);
		b >>= 1;
		pixelBuf[(offs.y*width)+offs.x] = color;
	}
}

//Tile/texture drawing
int getIndexFromTile(BYTE * tileBuf,DWORD tile,POINT offs) {
	return tileBuf[(tile<<6)|offs.x|(offs.y<<3)];
}
int getIndexFromTexture(BYTE * texBuf,POINT offs) {
	return texBuf[offs.x|(offs.y<<8)];
}

//////////////////////
//ADDRESS CONVERSION//
//////////////////////
DWORD convAddr_SNEStoPC_YI(DWORD addr) {
	switch(addr&0xC00000) {
		case 0x000000: {
			return (addr&0x7FFF)|((addr&0x7F0000)>>1);
			break;
		}
		case 0x400000: {
			return (addr&0x3FFFFF);
			break;
		}
		case 0x800000: {
			return (addr&0x7FFF)|((addr&0x7F0000)>>1)|0x200000;
			break;
		}
		case 0xC00000: {
			return (addr&0x3FFFFF)|0x400000;
			break;
		}
	}
}
DWORD convAddr_PCtoSNES_YI(DWORD addr) {
	if(addr>0x400000) {
		return addr|0x800000;
	} else if(addr>0x200000) {
		//TODO
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
DWORD readBitsLSB(BYTE * data,int * offset,int * bitOffset,int bitLength) {
	DWORD value = 0;
	for(int i=0; i<bitLength; i++) {
		//Append bit
		DWORD bit = (data[*offset]>>*bitOffset)&1;
		value |= bit<<i;
		//Move to next bit
		(*bitOffset)++;
		if((*bitOffset)==8) {
			*bitOffset = 0;
			(*offset)++;
		}
	}
	return value;
}
DWORD readBitsMSB(BYTE * data,int * offset,int * bitOffset,int bitLength) {
	DWORD value = 0;
	for(int i=0; i<bitLength; i++) {
		//Append bit
		DWORD bit = (data[*offset]>>*bitOffset)&1;
		value |= bit<<(bitLength-1-i);
		//Move to next bit
		(*bitOffset)++;
		if((*bitOffset)==8) {
			*bitOffset = 0;
			(*offset)++;
		}
	}
	return value;
}
void writeBitsLSB(BYTE * data,int * offset,int * bitOffset,int bitLength,DWORD value) {
	for(int i=0; i<bitLength; i++) {
		//Set bit
		data[*offset] &= ~(1<<*bitOffset);
		DWORD bit = (value>>i)&1;
		data[*offset] |= bit<<*bitOffset;
		//Move to next bit
		*bitOffset++;
		if(*bitOffset==8) {
			*bitOffset = 0;
			*offset++;
		}
	}
}
void writeBitsMSB(BYTE * data,int * offset,int * bitOffset,int bitLength,DWORD value) {
	for(int i=0; i<bitLength; i++) {
		//Set bit
		data[*offset] &= ~(1<<*bitOffset);
		DWORD bit = (value>>(bitLength-1-i))&1;
		data[*offset] |= bit<<*bitOffset;
		//Move to next bit
		*bitOffset++;
		if(*bitOffset==8) {
			*bitOffset = 0;
			*offset++;
		}
	}
}
DWORD readVariableSize(BYTE * data,int * offset,int * bitOffset) {
	DWORD value = 0;
	int bitLength = 0;
	while(true) {
		DWORD flag = readBitsLSB(data,offset,bitOffset,1);
		if(flag==0) break;
		DWORD bit = readBitsLSB(data,offset,bitOffset,1);
		value |= bit<<bitLength;
		bitLength++;
	}
	value |= 1<<bitLength;
	return value;
}
void writeVariableSize(BYTE * data,int * offset,int * bitOffset,DWORD value) {
	int bitLength = 7;
	DWORD bitMask = 0x80;
	while(bitLength>=0) {
		if(value&bitMask) break;
		bitLength--;
		bitMask >>= 1;
	}
	for(int i=0; i<bitLength; i++) {
		writeBitsLSB(data,offset,bitOffset,1,1);
		writeBitsLSB(data,offset,bitOffset,1,(value>>i)&1);
	}
	writeBitsLSB(data,offset,bitOffset,1,0);
}
DWORD readColor(BYTE * data,int * offset,int * bitOffset) {
	int palidx = readBitsMSB(data,offset,bitOffset,3);
	if(palidx==7) {
		return readBitsMSB(data,offset,bitOffset,4);
	} else {
		return palidx|0x10;
	}
}
void writeColor(BYTE * data,int * offset,int * bitOffset,DWORD value) {
	if(value&0x10) {
		writeBitsMSB(data,offset,bitOffset,3,value&7);
	} else {
		writeBitsMSB(data,offset,bitOffset,3,7);
		writeBitsMSB(data,offset,bitOffset,4,value);
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
	while(srcOff<size) {
		int cmd1len = 0,cmd1rel = 0;
		int cmd2len = 0,cmd2rel = 0;
		int cmd3len = 0,cmd3rel = 0;
		int cmd4len = 0,cmd4rel = 0;
		//Determine max length for command 1 (byte fill)
		BYTE basis1 = src[srcOff];
		int offs1 = 1;
		while((srcOff+offs1)<size) {
			if(src[srcOff+offs1] != basis1) break;
			cmd1len++;
			offs1++;
		}
		if(cmd1len>1024) cmd1len = 1024;
		if(cmd1len<32) {
			cmd1rel = cmd1len-2;
		} else {
			cmd1rel = cmd1len-3;
		}
		//Determine max length for command 2 (word fill)
		BYTE basis2 = src[srcOff+1];
		int offs2 = 2;
		while((srcOff+offs2)<size) {
			if(src[srcOff+offs2] != ((offs2&1)?basis2:basis1)) break;
			cmd2len++;
			offs2++;
		}
		if(cmd2len>1024) cmd2len = 1024;
		if(cmd2len<32) {
			cmd2rel = cmd2len-3;
		} else {
			cmd2rel = cmd2len-4;
		}
		//Determine max length for command 3 (byte fill incremented)
		BYTE basis3 = basis1+1;
		int offs3 = 1;
		while((srcOff+offs3)<size) {
			if(src[srcOff+offs3] != basis3) break;
			cmd3len++;
			basis3++;
			offs3++;
		}
		if(cmd3len>1024) cmd3len = 1024;
		if(cmd3len<32) {
			cmd3rel = cmd3len-2;
		} else {
			cmd3rel = cmd3len-3;
		}
		//Determine max length for command 4 (copy)
		int offs4 = 0;
		for(int i=0; i<dstOff; i++) {
			int j = 0;
			while((i+j)<dstOff && (srcOff+i)<size) {
				if(dst[i]!=src[srcOff+i]) break;
				j++;
				if(j>cmd4len) {
					cmd4len = j;
					offs4 = i;
				}
			}
		}
		if(cmd4len>1024) cmd4len = 1024;
		if(cmd4len<32) {
			cmd4rel = cmd4len-3;
		} else {
			cmd4rel = cmd4len-4;
		}
		//Output command which saves the most space (if any)
		if(cmd1rel>cmd2rel && cmd1rel>cmd3rel && cmd1rel>cmd4rel && cmd1rel>0) {
			//Output previous direct bytes
			while(directLen>0) {
				int thisLen = directLen;
				if(thisLen>1024) thisLen = 1024;
				if(thisLen<32) {
					dst[dstOff++] = thisLen-1;
				} else {
					dst[dstOff++] = 0xE0|((thisLen-1)>>8);
					dst[dstOff++] = (thisLen-1)&0xFF;
				}
				memcpy(&dst[dstOff],&src[directStart],thisLen);
				dstOff += thisLen;
				directLen -= thisLen;
				directStart += thisLen;
			}
			//Output command 1 (byte fill)
			if(cmd1len<32) {
				dst[dstOff++] = 0x20|(cmd1len-1);
			} else {
				dst[dstOff++] = 0xE4|((cmd1len-1)>>8);
				dst[dstOff++] = (cmd1len-1)&0xFF;
			}
			dst[dstOff++] = basis1;
			directLen = 0;
			srcOff += cmd1len;
			directStart = srcOff;
		} else if(cmd2rel>cmd3rel && cmd2rel>cmd4rel && cmd2rel>0) {
			//Output previous direct bytes
			while(directLen>0) {
				int thisLen = directLen;
				if(thisLen>1024) thisLen = 1024;
				if(thisLen<32) {
					dst[dstOff++] = thisLen-1;
				} else {
					dst[dstOff++] = 0xE0|((thisLen-1)>>8);
					dst[dstOff++] = (thisLen-1)&0xFF;
				}
				memcpy(&dst[dstOff],&src[directStart],thisLen);
				dstOff += thisLen;
				directLen -= thisLen;
				directStart += thisLen;
			}
			//Output command 2 (word fill)
			if(cmd2len<32) {
				dst[dstOff++] = 0x40|(cmd1len-1);
			} else {
				dst[dstOff++] = 0xE8|((cmd1len-1)>>8);
				dst[dstOff++] = (cmd1len-1)&0xFF;
			}
			dst[dstOff++] = basis1;
			dst[dstOff++] = basis2;
			directLen = 0;
			srcOff += cmd2len;
			directStart = srcOff;
		} else if(cmd3rel>cmd4rel && cmd3rel>0) {
			//Output previous direct bytes
			while(directLen>0) {
				int thisLen = directLen;
				if(thisLen>1024) thisLen = 1024;
				if(thisLen<32) {
					dst[dstOff++] = thisLen-1;
				} else {
					dst[dstOff++] = 0xE0|((thisLen-1)>>8);
					dst[dstOff++] = (thisLen-1)&0xFF;
				}
				memcpy(&dst[dstOff],&src[directStart],thisLen);
				dstOff += thisLen;
				directLen -= thisLen;
				directStart += thisLen;
			}
			//Output command 3 (byte fill incremented)
			if(cmd1len<32) {
				dst[dstOff++] = 0x60|(cmd1len-1);
			} else {
				dst[dstOff++] = 0xEC|((cmd1len-1)>>8);
				dst[dstOff++] = (cmd1len-1)&0xFF;
			}
			dst[dstOff++] = basis1;
			directLen = 0;
			srcOff += cmd3len;
			directStart = srcOff;
		} else if(cmd4rel>0) {
			//Output previous direct bytes
			while(directLen>0) {
				int thisLen = directLen;
				if(thisLen>1024) thisLen = 1024;
				if(thisLen<32) {
					dst[dstOff++] = thisLen-1;
				} else {
					dst[dstOff++] = 0xE0|((thisLen-1)>>8);
					dst[dstOff++] = (thisLen-1)&0xFF;
				}
				memcpy(&dst[dstOff],&src[directStart],thisLen);
				dstOff += thisLen;
				directLen -= thisLen;
				directStart += thisLen;
			}
			//Output command 4 (copy)
			if(cmd4len<32) {
				dst[dstOff++] = 0x80|(cmd1len-1);
			} else {
				dst[dstOff++] = 0xF0|((cmd1len-1)>>8);
				dst[dstOff++] = (cmd1len-1)&0xFF;
			}
			dst[dstOff++] = offs4&0xFF;
			dst[dstOff++] = offs4>>8;
			directLen = 0;
			srcOff += cmd4len;
			directStart = srcOff;
		//Otherwise, mark as direct and move on
		} else {
			directLen++;
			srcOff++;
		}
	}
	//Output remaining direct bytes and add terminator
	while(directLen>0) {
		int thisLen = directLen;
		if(thisLen>1024) thisLen = 1024;
		if(thisLen<32) {
			dst[dstOff++] = thisLen-1;
		} else {
			dst[dstOff++] = 0xE0|((thisLen-1)>>8);
			dst[dstOff++] = (thisLen-1)&0xFF;
		}
		memcpy(&dst[dstOff],&src[directStart],thisLen);
		dstOff += thisLen;
		directLen -= thisLen;
		directStart += thisLen;
	}
	dst[dstOff++] = 0xFF;
	return dstOff;
}
DWORD compressLZ16(BYTE * dst,BYTE * src,DWORD numLines) {
	int srcOff = 0,dstOff = 3;
	int dstBitOff = 4;
	std::vector<palette_sorter_t> palette;
	BYTE prevLine[0x80];
	memset(prevLine,0,0x80);
	//Init palette table
	for(int i=0; i<16; i++) {
		palette_sorter_t entry;
		entry.paletteIndex = i;
		entry.numPixelsWithValue = 0;
		palette.push_back(entry);
	}
	//Count pixel rows
	//TODO
	//Sort palette and output first 7 entries
	std::sort(palette.begin(),palette.end(),paletteSorter_opLt);
	dst[0] = (palette[1].paletteIndex<<4)|palette[0].paletteIndex;
	dst[1] = (palette[3].paletteIndex<<4)|palette[2].paletteIndex;
	dst[2] = (palette[5].paletteIndex<<4)|palette[4].paletteIndex;
	dst[3] = palette[6].paletteIndex;
	//For each line, check whether type 0 or type 1 is the smallest
	for(int j=0; j<numLines; j++) {
		BYTE curLine[0x80];
		//Construct current line buffer
		for(int i=0; i<0x80; i++) {
			int idx = ((j>>3)<<10)|((i>>3)<<6)|((j&7)<<3)|(i&7);
			curLine[i] = src[idx];
		}
		BYTE tempBuf0[0x100],tempBuf1[0x100];
		int tempOff0 = 0,tempOff1 = 0;
		int tempBitOff0 = 0,tempBitOff1 = 0;
		//Try compression for line type 0
		for(int i=0x7F; i>=0;) {
			//Determine the number of pixels taken up by this section
			int curlen = 0;
			int curidx = curLine[i--];
			while(i>=0) {
				int thisIdx = curLine[i--];
				if(thisIdx!=curidx) break;
				curlen++;
			}
			//Create RLE entry
			int curidxp = curidx;
			bool curuse = false;
			for(int n=0; n<7; n++) {
				if(curidx==palette[n].paletteIndex) {
					curidxp = n;
					curuse = true;
				}
			}
			if(curuse) {
				curidxp |= 0x10;
			}
			writeVariableSize(tempBuf0,&tempOff0,&tempBitOff0,curlen);
			writeColor(tempBuf0,&tempOff0,&tempBitOff0,curidxp);
		}
		//Try compression for line type 1
		for(int i=0x7F; i>=0;) {
			//Determine the number of pixels taken up by this section (previous)
			int si = i;
			int prevlen = 0;
			int previdx = prevLine[si--];
			while(si>=0) {
				int thisIdx = prevLine[si--];
				if(thisIdx!=previdx) break;
				prevlen++;
			}
			//Determine the number of pixels taken up by this section (current)
			si = i;
			int curlen = 0;
			int curidx = curLine[si--];
			while(si>=0) {
				int thisIdx = curLine[si--];
				if(thisIdx!=curidx) break;
				curlen++;
			}
			//If colors are different, create RLE entry and adjust bit offset
			if(previdx!=curidx) {
				writeBitsLSB(tempBuf1,&tempOff1,&tempBitOff1,2,2);
				int curidxp = curidx;
				bool curuse = false;
				for(int n=0; n<7; n++) {
					if(curidx==palette[n].paletteIndex) {
						curidxp = n;
						curuse = true;
					}
				}
				if(curuse) {
					curidxp |= 0x10;
				}
				writeVariableSize(tempBuf1,&tempOff1,&tempBitOff1,curlen);
				writeColor(tempBuf1,&tempOff1,&tempBitOff1,curidxp);
				i -= curlen;
			//If colors match but length is different, create plus/minus entry and adjust bit offset
			} else if(prevlen!=curlen) {
				int curidxp = curidx;
				bool curuse = false;
				for(int n=0; n<7; n++) {
					if(curidx==palette[n].paletteIndex) {
						curidxp = n;
						curuse = true;
					}
				}
				if(curuse) {
					curidxp |= 0x10;
				}
				if(curlen>prevlen) {
					writeBitsLSB(tempBuf1,&tempOff1,&tempBitOff1,2,1);
					writeVariableSize(tempBuf1,&tempOff1,&tempBitOff1,curlen-prevlen);
				} else {
					writeBitsLSB(tempBuf1,&tempOff1,&tempBitOff1,2,3);
					writeVariableSize(tempBuf1,&tempOff1,&tempBitOff1,prevlen-curlen);
				}
				//Intentionally bork previous buffer to replicate original behavior
				prevLine[i-curlen] = prevLine[i-prevlen];
				writeColor(tempBuf1,&tempOff1,&tempBitOff1,curidxp);
				i -= curlen;
			//If both color and length match, determine how many sections can be copied
			} else {
				int numsects = 1;
				//TODO
				writeVariableSize(tempBuf1,&tempOff1,&tempBitOff1,numsects);
				writeBitsLSB(tempBuf1,&tempOff1,&tempBitOff1,2,0);
				i -= curlen;
			}
		}
		//Output line type which is the smallest
		int cmdlen0 = (tempOff0<<3)|tempBitOff0;
		int cmdlen1 = (tempOff1<<3)|tempBitOff1;
		if(cmdlen0<=cmdlen1) {
			//Output data for line type 0
			for(int n=0; n<tempOff0; n++) {
				writeBitsLSB(dst,&dstOff,&dstBitOff,8,tempBuf0[n]);
			}
			if(tempBitOff0) {
				writeBitsLSB(dst,&dstOff,&dstBitOff,tempBitOff0,tempBuf0[tempOff0]);
			}
		} else {
			//Output data for line type 1
			for(int n=0; n<tempOff1; n++) {
				writeBitsLSB(dst,&dstOff,&dstBitOff,8,tempBuf1[n]);
			}
			if(tempBitOff1) {
				writeBitsLSB(dst,&dstOff,&dstBitOff,tempBitOff1,tempBuf1[tempOff1]);
			}
		}
	}
	return dstBitOff?(dstOff+1):dstOff;
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
				for(int i=0; i<len; i++) {
					dst[dstOff++] = (i&1)?b1:b0;
				}
				break;
			}
			//Command 3 (byte fill incremented)
			case 3: {
				BYTE b0 = src[srcOff++];
				for(int i=0; i<len; i++) {
					dst[dstOff++] = b0++;
				}
				break;
			}
			//Command 4 (copy)
			case 4: {
				int cpyOff = (src[srcOff++]<<8)|src[srcOff++];
				for(int i=0; i<len; i++) {
					dst[dstOff++] = dst[cpyOff++];
				}
				break;
			}
			//Command 7 (long mode)
			case 7: {
				cmd = (bc>>2)&7;
				len = (((bc&3)<<8)|src[srcOff++])+1;
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
						for(int i=0; i<len; i++) {
							dst[dstOff++] = (i&1)?b1:b0;
						}
						break;
					}
					//Command 3 (byte fill incremented)
					case 3: {
						BYTE b0 = src[srcOff++];
						for(int i=0; i<len; i++) {
							dst[dstOff++] = b0++;
						}
						break;
					}
					//Command 4 (copy)
					case 4: {
						int cpyOff = (src[srcOff++]<<8)|src[srcOff++];
						for(int i=0; i<len; i++) {
							dst[dstOff++] = dst[cpyOff++];
						}
						break;
					}
				}
				break;
			}
		}
	}
	return dstOff;
}
void decompressLZ16(BYTE * dst,BYTE * src,DWORD numLines) {
	int srcOff = 3,dstOff = 0;
	int srcBitOff = 4;
	BYTE palette[7];
	BYTE prevLine[0x80];
	memset(prevLine,0,0x80);
	//Get palette
	palette[0] = src[0]&0xF;
	palette[1] = src[0]>>4;
	palette[2] = src[1]&0xF;
	palette[3] = src[1]>>4;
	palette[4] = src[2]&0xF;
	palette[5] = src[2]>>4;
	palette[6] = src[3]&0xF;
	for(int j=0; j<numLines; j++) {
		BYTE curLine[0x80];
		memset(curLine,0,0x80);
		//Get line type
		int lineType = readBitsLSB(src,&srcOff,&srcBitOff,1);
		//Line type 1
		if(lineType) {
			for(int i=0x7F; i>=0;) {
				//Get length and command
				int cmdlen = readVariableSize(src,&srcOff,&srcBitOff);
				int cmd = readBitsLSB(src,&srcOff,&srcBitOff,2);
				//Process command
				switch(cmd) {
					//Command 0 (copy from previous line until color change, for n sections)
					case 0: {
						//Determine the number of pixels taken up by n sections
						int si = i;
						int cpylen = 0;
						int curidx = -1,numsects = -1;
						while(si>=0) {
							int thisIdx = prevLine[si--];
							if(thisIdx!=curidx) {
								numsects++;
								if(numsects>=cmdlen) break;
								curidx = thisIdx;
							}
							cpylen++;
						}
						//Copy pixels over
						memcpy(&curLine[i-cpylen+1],&prevLine[i-cpylen+1],cpylen);
						i -= cpylen;
						break;
					}
					//Command 1 (copy from previous line until color change, plus n pixels*)
					case 1: {
						//Determine the number of pixels taken up by this section
						int si = i;
						int cpylen = 1;
						int curidx = prevLine[si--];
						while(si>=0) {
							int thisIdx = prevLine[si--];
							if(thisIdx!=curidx) break;
							cpylen++;
						}
						//Copy pixels over
						memset(&curLine[i-(cpylen+cmdlen)+1],curidx,cpylen+cmdlen);
						//Intentionally bork previous buffer to replicate original behavior
						if((i-(cpylen+cmdlen))>=0) prevLine[i-(cpylen+cmdlen)] = prevLine[i-cpylen];
						i -= cpylen+cmdlen;
						break;
					}
					//Command 2 (RLE mode)
					case 2: {
						//Get color index
						int cidx = readColor(src,&srcOff,&srcBitOff);
						if(cidx&0x10) {
							cidx = palette[cidx&7];
						}
						//Output RLE pixels
						memset(&curLine[i-cmdlen+1],cidx,cmdlen);
						i -= cmdlen;
						break;
					}
					//Command 3 (copy from previous line until color change, minus n pixels*)
					case 3: {
						//Determine the number of pixels taken up by this section
						int si = i;
						int cpylen = 1;
						int curidx = prevLine[si--];
						while(si>=0) {
							int thisIdx = prevLine[si--];
							if(thisIdx!=curidx) break;
							cpylen++;
						}
						//Copy pixels over
						memset(&curLine[i-(cpylen-cmdlen)+1],curidx,cpylen-cmdlen);
						//Intentionally bork previous buffer to replicate original behavior
						if((i-cpylen)>=0) prevLine[i-(cpylen-cmdlen)] = prevLine[i-cpylen];
						else prevLine[i-(cpylen-cmdlen)] = 0;
						i -= cpylen-cmdlen;
						break;
					}
				}
			}
		//Line type 0
		} else {
			for(int i=0x7F; i>=0;) {
				//Get length
				int cmdlen = readVariableSize(src,&srcOff,&srcBitOff);
				//Get color index
				int cidx = readColor(src,&srcOff,&srcBitOff);
				if(cidx&0x10) {
					cidx = palette[cidx&7];
				}
				//Output RLE pixels
				memset(&curLine[i-cmdlen+1],cidx,cmdlen);
				i -= cmdlen;
			}
		}
		//Output line
		for(int i=0; i<0x80; i++) {
			int tile = ((j>>3)<<4)|(i>>3);
			int idx = (tile<<6)|((j&7)<<3)|(i&7);
			dst[idx] = curLine[i];
		}
		//Set previous line buffer
		memcpy(prevLine,curLine,0x80);
	}
}

