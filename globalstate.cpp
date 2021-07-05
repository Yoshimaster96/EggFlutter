#include "globalstate.h"

//General variables
HICON hiconMain;
HINSTANCE hinstMain;
bool isRomOpen = false,isRomSaved = false;
BYTE romBuf[0x800000];
BYTE oldRomBuf[0x800000];
BYTE patchBuf[0x200000];
DWORD patchBufSize;
//Variables for levels
BYTE curLevel = 0;
BYTE levelHeader[10];
BYTE screenExits[0x200];

///////////////////////////////
//ROM PATCHING AND ALLOCATION//
///////////////////////////////
DWORD readVariableSizeFromPatch(int * offset) {
	DWORD data = 0, shift = 1;
	while(true) {
		int x = patchBuf[(*offset)++];
		data += (x&0x7F)*shift;
		if(x&0x80) break;
		shift <<= 7;
		data += shift;
	}
	return data;
}
void patchRom() {
	//Patch ROM
	memcpy(oldRomBuf,romBuf,0x800000);
	int srcOff = 4,dstOff = 0;
	int srcReadOff = 0,dstReadOff = 0;
	//Skip magic/input/output/manifest stuff
	readVariableSizeFromPatch(&srcOff);
	readVariableSizeFromPatch(&srcOff);
	int manifestSize = readVariableSizeFromPatch(&srcOff);
	srcOff += manifestSize;
	//Read patch data and apply
	while((srcOff+12)<patchBufSize) {
		int cmdLen = readVariableSizeFromPatch(&srcOff);
		int cmd = cmdLen&3;
		int len = cmdLen>>2;
		len++;
		switch(cmd) {
			case 0: {
				memcpy(&romBuf[dstOff],&oldRomBuf[dstOff],len);
				dstOff += len;
				break;
			}
			case 1: {
				memcpy(&romBuf[dstOff],&patchBuf[srcOff],len);
				srcOff += len;
				dstOff += len;
				break;
			}
			case 2: {
				int sOff = readVariableSizeFromPatch(&srcOff);
				int off = sOff>>1;
				if(sOff&1) off = -off;
				srcReadOff += off;
				memcpy(&romBuf[dstOff],&oldRomBuf[srcReadOff],len);
				srcReadOff += len;
				dstOff += len;
				break;
			}
			case 3: {
				int sOff = readVariableSizeFromPatch(&srcOff);
				int off = sOff>>1;
				if(sOff&1) off = -off;
				dstReadOff += off;
				for(int i=0; i<len; i++) {
					romBuf[dstOff+i] = romBuf[dstReadOff+i];
				}
				dstReadOff += len;
				dstOff += len;
				break;
			}
		}
	}
}
bool checkRom() {
	if(romBuf[0x7FD7]==11) {
		patchRom();
		return true;
	} else if(romBuf[0x7FD7]==13) {
		return true;
	}
	return false;
}
DWORD findFreespace(DWORD size) {
	//Find smallest region which is at least size+8 bytes
	DWORD smallestRegionOffset = 0x200000;
	DWORD smallestRegionSize = 0x800000;
	DWORD prevRatsEnd = 0x200000;
	for(DWORD i=0x200000; i<0x800000; i++) {
		//Search for next RATS tag
		if(romBuf[i]=='S') {
			DWORD ratsBegin = i;
			i++;
			if(romBuf[i]=='T') {
				i++;
				if(romBuf[i]=='A') {
					i++;
					if(romBuf[i]=='R') {
						i++;
						DWORD ratsSize = romBuf[i]|(romBuf[i+1]<<8);
						DWORD ratsSizeCmp = romBuf[i+2]|(romBuf[i+3]<<8);
						if((ratsSize^ratsSizeCmp)==0xFFFF) {
							i += ratsSize+4;
							if(ratsBegin!=prevRatsEnd) {
								//Make sure region does not cross bank boundary!
								if(((ratsBegin-1)&0x7F8000)==(prevRatsEnd&0x7F8000)) {
									int thisRegionSize = ratsBegin-prevRatsEnd;
									if(thisRegionSize<smallestRegionSize && thisRegionSize>=(size+8)) {
										smallestRegionSize = thisRegionSize;
										smallestRegionOffset = prevRatsEnd;
									}
								} else {
									//Check end bit after previous RATS
									int endBitRegionSize = ((prevRatsEnd+0x8000)&0x7F8000)-prevRatsEnd;
									if(endBitRegionSize<smallestRegionSize && endBitRegionSize>=(size+8)) {
										smallestRegionSize = endBitRegionSize;
										smallestRegionOffset = prevRatsEnd;
									}
									//Check beginning bit before current RATS
									int beginBitRegionSize = ratsBegin-(ratsBegin&0x7F8000);
									if(beginBitRegionSize<smallestRegionSize && endBitRegionSize>=(size+8)) {
										smallestRegionSize = beginBitRegionSize;
										smallestRegionOffset = prevRatsEnd;
									}
									//If an empty bank exists between the two, check it
									if(((ratsBegin-1)&0x7F8000)-(prevRatsEnd&0x7F8000)>=0x10000) {
										if(0x8000<smallestRegionSize && 0x8000>=(size+8)) {
											smallestRegionSize = 0x8000;
											smallestRegionOffset = (prevRatsEnd+0x8000)&0x7F8000;
										}
									}
								}
							}
							prevRatsEnd = i+1;
						}
					}
				}
			}
		}
	}
	//Check region at end of ROM
	//Make sure region does not cross bank boundary!
	//Check end bit after last RATS
	int endRegionSize = ((prevRatsEnd+0x8000)&0x7F8000)-prevRatsEnd;
	if(endRegionSize<smallestRegionSize && endRegionSize>=(size+8)) {
		smallestRegionSize = endRegionSize;
		smallestRegionOffset = prevRatsEnd;
	}
	//If an empty bank exists at the end, check it
	if(((prevRatsEnd+0x8000)&0x7F8000)!=0x800000) {
		if(0x8000<smallestRegionSize && 0x8000>=(size+8)) {
			smallestRegionSize = 0x8000;
			smallestRegionOffset = (prevRatsEnd+0x8000)&0x7F8000;
		}
	}
	//Create RATS tag and return pointer
	romBuf[smallestRegionOffset] = 'S';
	romBuf[smallestRegionOffset+1] = 'T';
	romBuf[smallestRegionOffset+2] = 'A';
	romBuf[smallestRegionOffset+3] = 'R';
	romBuf[smallestRegionOffset+4] = (size-1)&0xFF;
	romBuf[smallestRegionOffset+5] = ((size-1)>>8)&0xFF;
	romBuf[smallestRegionOffset+6] = ((size-1)&0xFF)^0xFF;
	romBuf[smallestRegionOffset+7] = (((size-1)>>8)&0xFF)^0xFF;
	return smallestRegionOffset+8;
}

