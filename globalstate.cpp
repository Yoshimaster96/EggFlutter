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
				memcpy(&romBuf[dstOff],&oldRomBuf[dstOff+off],len);
				dstOff += len;
				break;
			}
			case 3: {
				int sOff = readVariableSizeFromPatch(&srcOff);
				int off = sOff>>1;
				if(sOff&1) off = -off;
				for(int i=0; i<len; i++) {
					romBuf[dstOff+i] = romBuf[dstOff+off+i];
				}
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
							i += ratsSize+5;
							int thisRegionSize = ratsBegin-prevRatsEnd;
							if(thisRegionSize<smallestRegionSize && thisRegionSize>=(size+8)) {
								smallestRegionSize = ratsBegin-prevRatsEnd;
								smallestRegionOffset = prevRatsEnd;
							}
							prevRatsEnd = i;
						}
					}
				}
			}
		}
	}
	//Create RATS tag and return pointer
	romBuf[smallestRegionOffset] = 'S';
	romBuf[smallestRegionOffset+1] = 'T';
	romBuf[smallestRegionOffset+2] = 'A';
	romBuf[smallestRegionOffset+3] = 'R';
	romBuf[smallestRegionOffset+4] = size&0xFF;
	romBuf[smallestRegionOffset+5] = (size>>8)&0xFF;
	romBuf[smallestRegionOffset+6] = (size&0xFF)^0xFF;
	romBuf[smallestRegionOffset+7] = ((size>>8)&0xFF)^0xFF;
	return smallestRegionOffset+8;
}

