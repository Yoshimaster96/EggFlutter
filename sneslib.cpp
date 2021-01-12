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
	int hi = (addr&0x800000)?0x200000:0;
	if(addr&0x400000) {
		return hi|(addr&0x3FFFFF);
	} else {
		return hi|(addr&0x7FFF)|((addr&0x7F0000)>>1);
	}
}
DWORD convAddr_PCtoSNES_YI(DWORD addr) {
	if(addr>0x200000) {
		return addr+0xA00000;
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
	//Init palette table
	for(int i=0; i<16; i++) {
		palette_sorter_t entry;
		entry.paletteIndex = i;
		entry.numPixelsWithValue = 0;
		palette.push_back(entry);
	}
	//Count pixels (NOTE: Here we are assuming numLines is a multiple of 8 for simplicity)
	for(int i=0; i<(numLines<<7); i++) {
		palette[src[i]].numPixelsWithValue++;
	}
	//Sort palette
	std::sort(palette.begin(),palette.end(),paletteSorter_opLt);
	//For each line, check whether type 0 or type 1 is the smallest
	for(int j=0; j<numLines; j++) {
		int cmdlen0 = 0,cmdlen1 = 0;
		//Determine size for line type 0
		//TODO
		//Determine size for line type 1 (skipped if this is the first line)
		if(j) {
			//TODO
		}
		//Use smallest (unless first line in which case type 0 is always used)
		if(j==0 || cmdlen0<cmdlen1) {
			//TODO
		} else {
			//TODO
		}
	}
	return dstOff;
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
		//Get line type and adjust bit offset
		int lineType = readBits(&src[srcOff],srcBitOff,1);
		srcBitOff++;
		if(srcBitOff>=8) {
			srcBitOff -= 8;
			srcOff++;
		}
		//Line type 1
		if(lineType) {
			for(int i=0x80; i>=0;) {
				//Get length
				int cmdlen = 0,cmdbits = 0;
				while(true) {
					//Get last bit flag and adjust bit offset
					int flag = readBits(&src[srcOff],srcBitOff,1);
					srcBitOff++;
					if(srcBitOff>=8) {
						srcBitOff -= 8;
						srcOff++;
					}
					if(flag==0) break;
					//Get actual bit and adjust bit offset
					int bit = readBits(&src[srcOff],srcBitOff,1);
					srcBitOff++;
					if(srcBitOff>=8) {
						srcBitOff -= 8;
						srcOff++;
					}
					cmdlen |= bit<<cmdbits;
					cmdbits++;
				}
				cmdlen |= 1<<cmdbits;
				//Get command and adjust bit offset
				int cmd = readBits(&src[srcOff],srcBitOff,2);
				srcBitOff += 2;
				if(srcBitOff>=8) {
					srcBitOff -= 8;
					srcOff++;
				}
				//Process command
				switch(cmd) {
					//Command 0 (copy from previous line until color change, for n sections)
					case 0: {
						//TODO
						break;
					}
					//Command 1 (copy from previous line until color change, plus n pixels*)
					case 1: {
						//TODO
						break;
					}
					//Command 2 (RLE mode)
					case 2: {
						//Get color index and adjust bit offset
						int cidx = reverseBitOrder(readBits(&src[srcOff],srcBitOff,3),3);
						srcBitOff += 3;
						if(srcBitOff>=8) {
							srcBitOff -= 8;
							srcOff++;
						}
						//If index is 7, get actual color and adjust bit offset
						if(cidx==7) {
							cidx = reverseBitOrder(readBits(&src[srcOff],srcBitOff,4),4);
							srcBitOff += 4;
							if(srcBitOff>=8) {
								srcBitOff -= 8;
								srcOff++;
							}
						//Otherwise, get color from palette
						} else {
							cidx = palette[cidx];
						}
						//Output RLE pixels
						for(int n=0; n<cmdlen; n++) {
							curLine[i--] = cidx;
						}
						break;
					}
					//Command 3 (copy from previous line until color change, minus n pixels*)
					case 3: {
						//TODO
						break;
					}
				}
			}
		//Line type 0
		} else {
			for(int i=0x80; i>=0;) {
				//Get length
				int cmdlen = 0,cmdbits = 0;
				while(true) {
					//Get last bit flag and adjust bit offset
					int flag = readBits(&src[srcOff],srcBitOff,1);
					srcBitOff++;
					if(srcBitOff>=8) {
						srcBitOff -= 8;
						srcOff++;
					}
					if(flag==0) break;
					//Get actual bit and adjust bit offset
					int bit = readBits(&src[srcOff],srcBitOff,1);
					srcBitOff++;
					if(srcBitOff>=8) {
						srcBitOff -= 8;
						srcOff++;
					}
					cmdlen |= bit<<cmdbits;
					cmdbits++;
				}
				cmdlen |= 1<<cmdbits;
				//Get color index and adjust bit offset
				int cidx = reverseBitOrder(readBits(&src[srcOff],srcBitOff,3),3);
				srcBitOff += 3;
				if(srcBitOff>=8) {
					srcBitOff -= 8;
					srcOff++;
				}
				//If index is 7, get actual color and adjust bit offset
				if(cidx==7) {
					cidx = reverseBitOrder(readBits(&src[srcOff],srcBitOff,4),4);
					srcBitOff += 4;
					if(srcBitOff>=8) {
						srcBitOff -= 8;
						srcOff++;
					}
				//Otherwise, get color from palette
				} else {
					cidx = palette[cidx];
				}
				//Output RLE pixels
				for(int n=0; n<cmdlen; n++) {
					curLine[i--] = cidx;
				}
			}
		}
		//Output line
		//TODO
		//Set previous line buffer
		memcpy(prevLine,curLine,0x80);
	}
}

