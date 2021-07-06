#include "main.h"

/////////////////
//LEVEL LOADING//
/////////////////
void loadLevel() {
	//Get level pointer and load data
	DWORD objectAddr = romBuf[0x0BF7C3+(curLevel*6)]|(romBuf[0x0BF7C4+(curLevel*6)]<<8)|(romBuf[0x0BF7C5+(curLevel*6)]<<16);
	DWORD spriteAddr = romBuf[0x0BF7C6+(curLevel*6)]|(romBuf[0x0BF7C7+(curLevel*6)]<<8)|(romBuf[0x0BF7C8+(curLevel*6)]<<16);
	objectAddr = convAddr_SNEStoPC_YI(objectAddr);
	spriteAddr = convAddr_SNEStoPC_YI(spriteAddr);
	if(curLevel==0x38) {
		memset(levelHeader,0,10);
		levelHeader[8] = 0x78;
		levelHeader[9] = 0x80;
	} else {
		memcpy(levelHeader,&romBuf[objectAddr],10);
		objectAddr += 10;
	}
	initOtherObjectBuffers();
	initOtherSpriteBuffers();
	objectAddr += loadObjects(&romBuf[objectAddr]);
	drawObjects();
	objectAddr++;
	memset(screenExits,0xFF,0x200);
	while(true) {
		int page = romBuf[objectAddr++];
		if(page==0xFF) break;
		page <<= 2;
		screenExits[page] = romBuf[objectAddr++];
		screenExits[page+1] = romBuf[objectAddr++];
		screenExits[page+2] = romBuf[objectAddr++];
		screenExits[page+3] = romBuf[objectAddr++];
	}
	loadSprites(&romBuf[spriteAddr]);
	drawSprites();
	//Load other stuff
	loadMap8();
	loadMap16();
	loadPalette();
	loadBackground();
}
void saveLevel() {
	//Determine level size
	BYTE tempBufObj[0x8000],tempBufSp[0x8000];
	int objectSize = 0;
	if(curLevel!=0x38) {
		memcpy(tempBufObj,levelHeader,10);
		objectSize = 10;
	}
	objectSize += saveObjects(&tempBufObj[objectSize]);
	tempBufObj[objectSize++] = 0xFF;
	for(int i=0; i<0x200; i+=4) {
		if(screenExits[i]!=0xFF) {
			tempBufObj[objectSize++] = i>>2;
			for(int j=0; j<4; j++) {
				tempBufObj[objectSize++] = screenExits[i+j];
			}
		}
	}
	tempBufObj[objectSize++] = 0xFF;
	int spriteSize = saveSprites(tempBufSp);
	tempBufSp[spriteSize++] = 0xFF;
	tempBufSp[spriteSize++] = 0xFF;
	//Clear previously existing level data
	DWORD oldObjectAddr = romBuf[0x0BF7C3+(curLevel*6)]|(romBuf[0x0BF7C4+(curLevel*6)]<<8)|(romBuf[0x0BF7C5+(curLevel*6)]<<16);
	DWORD oldSpriteAddr = romBuf[0x0BF7C6+(curLevel*6)]|(romBuf[0x0BF7C7+(curLevel*6)]<<8)|(romBuf[0x0BF7C8+(curLevel*6)]<<16);
	if(oldObjectAddr&0x800000) {
		oldObjectAddr = convAddr_SNEStoPC_YI(oldObjectAddr);
		int oldObjectSize = romBuf[oldObjectAddr-4]|(romBuf[oldObjectAddr-3]<<8);
		memset(&romBuf[oldObjectAddr-8],0,oldObjectSize+9);
	}
	if(oldSpriteAddr&0x800000) {
		oldSpriteAddr = convAddr_SNEStoPC_YI(oldSpriteAddr);
		int oldSpriteSize = romBuf[oldSpriteAddr-4]|(romBuf[oldSpriteAddr-3]<<8);
		memset(&romBuf[oldSpriteAddr-8],0,oldSpriteSize+9);
	}
	//Find an area to write level data to and save data
	DWORD saveOffsObj = findFreespace(objectSize);
	DWORD saveOffsSp = findFreespace(spriteSize);
	memcpy(&romBuf[saveOffsObj],tempBufObj,objectSize);
	memcpy(&romBuf[saveOffsSp],tempBufSp,spriteSize);
	saveOffsObj = convAddr_PCtoSNES_YI(saveOffsObj);
	saveOffsSp = convAddr_PCtoSNES_YI(saveOffsSp);
	romBuf[0x0BF7C3+(curLevel*6)] = saveOffsObj&0xFF;
	romBuf[0x0BF7C4+(curLevel*6)] = (saveOffsObj>>8)&0xFF;
	romBuf[0x0BF7C5+(curLevel*6)] = (saveOffsObj>>16)&0xFF;
	romBuf[0x0BF7C6+(curLevel*6)] = saveOffsSp&0xFF;
	romBuf[0x0BF7C7+(curLevel*6)] = (saveOffsSp>>8)&0xFF;
	romBuf[0x0BF7C8+(curLevel*6)] = (saveOffsSp>>16)&0xFF;
}

///////////
//DIALOGS//
///////////
//Helper functions for reading/writing hex values
DWORD getHexVal_dlg(HWND hwnd,int control) {
	char dlgStr[256];
	GetDlgItemTextA(hwnd,control,dlgStr,256);
	DWORD ret;
	sscanf(dlgStr,"%X",&ret);
	return ret;
}
void setHexVal_dlg(HWND hwnd,int control,DWORD val) {
	char dlgStr[256];
	snprintf(dlgStr,256,"%X",val);
	SetDlgItemTextA(hwnd,control,dlgStr);
}
void enableItem_dlg(HWND hwnd,int control,BOOL enable) {
	HWND hwndDlgItem = GetDlgItem(hwnd,control);
	EnableWindow(hwndDlgItem,enable);
}

//Helper functions for reading/writing in-game text
char YIToANSITable[0x100] = {
	  0,  0,  0,  0,  0,  0,  0,  0,  0,  0,  0,   0,  0,  0,  0,  0,
	  0,  0,  0,  0,  0,  0,  0,  0,  0,  0,  0,   0,  0,  0,  0,  0,
	  0,  0,  0,  0,  0,  0,':',';',  0,  0,  0,'\'',  0,  0,  0,  0,
	  0,  0,  0,  0,  0,  0,'=',  0,  0,  0,  0,   0,  0,  0,  0,  0,
	  0,  0,  0,  0,  0,  0,  0,  0,  0,  0,  0,   0,  0,  0,  0,  0,
	  0,  0,  0,  0,  0,  0,  0,  0,  0,  0,  0,   0,  0,  0,  0,  0,
	  0,  0,  0,  0,  0,  0,  0,  0,  0,  0,  0,   0,  0,  0,  0,  0,
	  0,  0,  0,  0,  0,  0,  0,  0,  0,  0,  0,   0,  0,  0,  0,  0,
	  0,  0,  0,  0,  0,  0,  0,  0,  0,  0,  0,   0,  0,  0,  0,  0,
	  0,  0,  0,  0,  0,  0,  0,  0,  0,  0,  0,   0,  0,  0,  0,  0,
	'0','1','2','3','4','5','6','7','8','9','A', 'B','C','D','E','F',
	'G','H','I','J','K','L','M','N','O','P','Q', 'R','S','T','U','V',
	'W','X','Y','Z',  0,  0,'?','!',  0,'-',  0,   0,  0,  0,'~',',',
	' ',  0,  0,  0,  0,  0,  0,  0,'a','b','c', 'd','e','f','g','h',
	'i','j','k','l','m','n','o','p','q','r','s', 't','u','v','w','x',
	'y','z',  0,'.',  0,  0,  0,  0,  0,  0,  0,   0,  0,  0,  0,  0};
BYTE ANSIToYITable[0x80] = {
	0xFF,0xFF,0xFF,0xFF,0xFF,0xFF,0xFF,0xFF,0xFF,0xFF,0xFF,0xFF,0xFF,0xFF,0xFF,0xFF,
	0xFF,0xFF,0xFF,0xFF,0xFF,0xFF,0xFF,0xFF,0xFF,0xFF,0xFF,0xFF,0xFF,0xFF,0xFF,0xFF,
	0xD0,0xC7,0xFF,0xFF,0xFF,0xFF,0xFF,0x2B,0xFF,0xFF,0xFF,0xFF,0xCF,0xC9,0xD3,0xFF,
	0xA0,0xA1,0xA2,0xA3,0xA4,0xA5,0xA6,0xA7,0xA8,0xA9,0x26,0x27,0xFF,0x36,0xFF,0xC6,
	0xFF,0xAA,0xAB,0xAC,0xAD,0xAE,0xAF,0xB0,0xB1,0xB2,0xB3,0xB4,0xB5,0xB6,0xB7,0xB8,
	0xB9,0xBA,0xBB,0xBC,0xBD,0xBE,0xBF,0xC0,0xC1,0xC2,0xC3,0xFF,0xFF,0xFF,0xFF,0xFF,
	0xFF,0xD8,0xD9,0xDA,0xDB,0xDC,0xDD,0xDE,0xDF,0xE0,0xE1,0xE2,0xE3,0xE4,0xE5,0xE6,
	0xE7,0xE8,0xE9,0xEA,0xEB,0xEC,0xED,0xEE,0xEF,0xF0,0xF1,0xFF,0xFF,0xFF,0xCE,0xFF};

int readGameText(BYTE * data,char * str) {
	char strBuf[256];
	int srcOff = 0, dstOff = 0;
	int lineType = 0;
	while(true) {
		BYTE code = data[srcOff++];
		if(code==0xFD) break;
		if(code==0xFE) {
			if(lineType==1) {
				str[dstOff++] = '\r';
				str[dstOff++] = '\n';
			}
			str[dstOff++] = '\\';
			str[dstOff++] = 'l';
			BYTE param = data[srcOff++];
			snprintf(strBuf,256,"%02X",param);
			str[dstOff++] = strBuf[0];
			str[dstOff++] = strBuf[1];
			param = data[srcOff++];
			snprintf(strBuf,256,"%02X",param);
			str[dstOff++] = strBuf[0];
			str[dstOff++] = strBuf[1];
			lineType = 2;
		} else if(code==0xFF) {
			BYTE ctrlCode = data[srcOff++];
			if(ctrlCode==0xFF) break;
			if(lineType==1) {
				str[dstOff++] = '\r';
				str[dstOff++] = '\n';
			}
			str[dstOff++] = '\\';
			str[dstOff++] = 'm';
			snprintf(strBuf,256,"%02X",ctrlCode);
			str[dstOff++] = strBuf[0];
			str[dstOff++] = strBuf[1];
			if(ctrlCode==0x60) {
				for(int i=0; i<7; i++) {
					BYTE param = data[srcOff++];
					snprintf(strBuf,256,"%02X",param);
					str[dstOff++] = strBuf[0];
					str[dstOff++] = strBuf[1];
				}
			}
			lineType = 2;
		} else {
			if(lineType==2) {
				str[dstOff++] = '\r';
				str[dstOff++] = '\n';
			}
			char cnv = YIToANSITable[code];
			if(cnv) {
				str[dstOff++] = cnv;
			} else {
				str[dstOff++] = '\\';
				str[dstOff++] = 'x';
				snprintf(strBuf,256,"%02X",code);
				str[dstOff++] = strBuf[0];
				str[dstOff++] = strBuf[1];
			}
			lineType = 1;
		}
	}
	return dstOff;
}
int writeGameText(BYTE * data,char * str) {
	char strBuf[256];
	int srcOff = 0, dstOff = 0;
	while(true) {
		char code = str[srcOff++];
		if(code==0) break;
		if(code=='\\') {
			char ctrlCode = str[srcOff++];
			if(ctrlCode=='x' || ctrlCode=='X') {
				strBuf[0] = str[srcOff++];
				strBuf[1] = str[srcOff++];
				strBuf[2] = 0;
				int param;
				sscanf(strBuf,"%X",&param);
				data[dstOff++] = param;
			} else if(ctrlCode=='l' || ctrlCode=='L') {
				data[dstOff++] = 0xFE;
				int param;
				for(int i=0; i<2; i++) {
					strBuf[0] = str[srcOff++];
					strBuf[1] = str[srcOff++];
					strBuf[2] = 0;
					sscanf(strBuf,"%X",&param);
					data[dstOff++] = param;
				}
			} else if(ctrlCode=='m' || ctrlCode=='M') {
				data[dstOff++] = 0xFF;
				strBuf[0] = str[srcOff++];
				strBuf[1] = str[srcOff++];
				strBuf[2] = 0;
				int param;
				sscanf(strBuf,"%X",&param);
				data[dstOff++] = param;
				if(param==0x60) {
					for(int i=0; i<7; i++) {
						strBuf[0] = str[srcOff++];
						strBuf[1] = str[srcOff++];
						strBuf[2] = 0;
						sscanf(strBuf,"%X",&param);
						data[dstOff++] = param;
					}
				}
			} else {
				srcOff--;
			}
		} else {
			BYTE cnv = ANSIToYITable[code&0x7F];
			if(cnv!=0xFF) {
				data[dstOff++] = cnv;
			}
		}
	}
	return dstOff;
}

//Dialog updaters
void updateDialog_editEntrances(HWND hwnd) {
	int tlevIdx = SendDlgItemMessageA(hwnd,20,CB_GETCURSEL,0,0);
	setHexVal_dlg(hwnd,21,romBuf[0x0BF471+(tlevIdx<<2)]);
	setHexVal_dlg(hwnd,23,romBuf[0x0BF472+(tlevIdx<<2)]);
	setHexVal_dlg(hwnd,24,romBuf[0x0BF473+(tlevIdx<<2)]);
}
void updateDialog_editEntrances2(HWND hwnd) {
	setHexVal_dlg(hwnd,23,romBuf[0x0BF551+(curLevel<<1)]);
	setHexVal_dlg(hwnd,24,romBuf[0x0BF552+(curLevel<<1)]);
}
void updateDialog_editExits(HWND hwnd) {
	int tscrIdx = SendDlgItemMessageA(hwnd,20,CB_GETCURSEL,0,0);
	tscrIdx <<= 2;
	if(screenExits[tscrIdx]<=0xE9) {
		SendDlgItemMessageA(hwnd,25,BM_SETCHECK,(WPARAM)BST_CHECKED,0);
		if(screenExits[tscrIdx]<=0xDD) {
			setHexVal_dlg(hwnd,21,screenExits[tscrIdx]);
			SendDlgItemMessageA(hwnd,22,CB_SETCURSEL,screenExits[tscrIdx+3],0);
		} else {
			setHexVal_dlg(hwnd,21,screenExits[tscrIdx+3]);
			SendDlgItemMessageA(hwnd,22,CB_SETCURSEL,screenExits[tscrIdx]-0xD3,0);
		}
		setHexVal_dlg(hwnd,23,screenExits[tscrIdx+1]);
		setHexVal_dlg(hwnd,24,screenExits[tscrIdx+2]);
		enableItem_dlg(hwnd,21,TRUE);
		enableItem_dlg(hwnd,22,TRUE);
		enableItem_dlg(hwnd,23,TRUE);
		enableItem_dlg(hwnd,24,TRUE);
	} else {
		SendDlgItemMessageA(hwnd,25,BM_SETCHECK,(WPARAM)BST_UNCHECKED,0);
		SetDlgItemTextA(hwnd,21,"");
		SendDlgItemMessageA(hwnd,22,CB_SETCURSEL,0,0);
		SetDlgItemTextA(hwnd,23,"");
		SetDlgItemTextA(hwnd,24,"");
		enableItem_dlg(hwnd,21,FALSE);
		enableItem_dlg(hwnd,22,FALSE);
		enableItem_dlg(hwnd,23,FALSE);
		enableItem_dlg(hwnd,24,FALSE);
	}
}
void updateDialog_editHeader(HWND hwnd) {
	int bgCol = levelHeader[0]>>3;
	int bg1Ts = ((levelHeader[0]&7)<<1)|(levelHeader[1]>>7);
	int bg1Pal = (levelHeader[1]&0x7C)>>2;
	int bg2Ts = ((levelHeader[1]&3)<<3)|(levelHeader[2]>>5);
	int bg2Pal = ((levelHeader[2]&0x1F)<<1)|(levelHeader[3]>>7);
	int bg3Ts = (levelHeader[3]&0x7E)>>1;
	int bg3Pal = ((levelHeader[3]&1)<<5)|(levelHeader[4]>>3);
	int spTs = ((levelHeader[4]&7)<<4)|(levelHeader[5]>>4);
	int spPal = levelHeader[5]&0xF;
	int animTs = ((levelHeader[6]&7)<<3)|(levelHeader[7]>>5);
	int animPal = levelHeader[7]&0x1F;
	int levMode = levelHeader[6]>>3;
	int bgScrl = levelHeader[8]>>3;
	int music = ((levelHeader[8]&7)<<1)|(levelHeader[9]>>7);
	int itemMem = (levelHeader[9]&0x60)>>5;
	SendDlgItemMessageA(hwnd,20,CB_SETCURSEL,bgCol,0);
	SendDlgItemMessageA(hwnd,40,CB_SETCURSEL,bg1Ts,0);
	SendDlgItemMessageA(hwnd,41,CB_SETCURSEL,bg1Pal,0);
	SendDlgItemMessageA(hwnd,42,CB_SETCURSEL,bg2Ts,0);
	SendDlgItemMessageA(hwnd,43,CB_SETCURSEL,bg2Pal,0);
	SendDlgItemMessageA(hwnd,44,CB_SETCURSEL,bg3Ts,0);
	SendDlgItemMessageA(hwnd,45,CB_SETCURSEL,bg3Pal,0);
	SendDlgItemMessageA(hwnd,46,CB_SETCURSEL,spTs,0);
	SendDlgItemMessageA(hwnd,47,CB_SETCURSEL,spPal,0);
	SendDlgItemMessageA(hwnd,48,CB_SETCURSEL,animTs,0);
	SendDlgItemMessageA(hwnd,49,CB_SETCURSEL,animPal,0);
	SendDlgItemMessageA(hwnd,21,CB_SETCURSEL,levMode,0);
	SendDlgItemMessageA(hwnd,22,CB_SETCURSEL,bgScrl,0);
	SendDlgItemMessageA(hwnd,23,CB_SETCURSEL,music,0);
	SendDlgItemMessageA(hwnd,24,CB_SETCURSEL,itemMem,0);
}
void updateDialog_editLevNames(HWND hwnd) {
	char textBuf[0x8000];
	int tMsgIdx = SendDlgItemMessageA(hwnd,20,CB_GETCURSEL,0,0);
	DWORD messageOffset = romBuf[0x1149BC+(tMsgIdx<<1)]|(romBuf[0x1149BD+(tMsgIdx<<1)]<<8)|(romBuf[0x115348+tMsgIdx]<<16);
	messageOffset = convAddr_SNEStoPC_YI(messageOffset);
	if(messageOffset!=0x110000) {
		int messageLen = readGameText(&romBuf[messageOffset],textBuf);
		textBuf[messageLen] = 0;
	} else {
		textBuf[0] = 0;
	}
	SetDlgItemTextA(hwnd,25,textBuf);
}
void updateDialog_editLevMessages(HWND hwnd) {
	char textBuf[0x8000];
	int tMsgIdx = SendDlgItemMessageA(hwnd,20,CB_GETCURSEL,0,0);
	int tMsgIdx2 = SendDlgItemMessageA(hwnd,21,CB_GETCURSEL,0,0);
	tMsgIdx = (tMsgIdx<<2)|tMsgIdx2;
	DWORD messageOffset = romBuf[0x1110DB+(tMsgIdx<<1)]|(romBuf[0x1110DC+(tMsgIdx<<1)]<<8)|(romBuf[0x115390+tMsgIdx]<<16);
	messageOffset = convAddr_SNEStoPC_YI(messageOffset);
	if(messageOffset!=0x110000) {
		int messageLen = readGameText(&romBuf[messageOffset],textBuf);
		textBuf[messageLen] = 0;
	} else {
		textBuf[0] = 0;
	}
	SetDlgItemTextA(hwnd,25,textBuf);
}

//Dialog functions
LPCSTR worldLevelStrings[56] = {
"1-1","1-2","1-3","1-4","1-5","1-6","1-7","1-8","1-E",
"2-1","2-2","2-3","2-4","2-5","2-6","2-7","2-8","2-E",
"3-1","3-2","3-3","3-4","3-5","3-6","3-7","3-8","3-E",
"4-1","4-2","4-3","4-4","4-5","4-6","4-7","4-8","4-E",
"5-1","5-2","5-3","5-4","5-5","5-6","5-7","5-8","5-E",
"6-1","6-2","6-3","6-4","6-5","6-6","6-7","6-8","6-E",
"Intro","Tutorial"};
LPCSTR actionMinigameStrings[23] = {
"Action 00: Do Nothing",
"Action 01: Skiing",
"Action 02: Horizontal Pipe Exit Right",
"Action 03: Horizontal Pipe Exit Left",
"Action 04: Vertical Pipe Exit Down",
"Action 05: Vertical Pipe Exit Up",
"Action 06: Walk Right",
"Action 07: Walk Left",
"Action 08: Fall Down",
"Action 09: Jump Up",
"Action 0A: Raphael Raven Intro",
"Mini 00: Throw Balloons (4 Buttons)",
"Mini 01: Throw Balloons (5 Buttons)",
"Mini 02: Throw Balloons (6 Buttons)",
"Mini 03: Unused",
"Mini 04: Gather Coins",
"Mini 05: Pop Balloons (Static Platforms)",
"Mini 06: Pop Balloons (Moving Platforms)",
"Mini 07: Unused",
"Mini 08: Unused",
"Mini 09: Seed Spitting",
"Mini 0A: Seed Spitting 2P",
"Mini 0B: Throw Balloons 2P (4 Buttons)"};
LPCSTR headerBg1TilesetStrings[16] = {
"00: Cave 1",
"01: Forest 1",
"02: Pond",
"03: 3D Stone",
"04: Snow",
"05: Jungle",
"06: Castle 1",
"07: Grass 1",
"08: Cave 2",
"09: Forest 2",
"0A: Castle 2",
"0B: Sewer",
"0C: Flower Garden",
"0D: Sky",
"0E: Castle 3",
"0F: Grass 2"};
LPCSTR headerBg2TilesetStrings[32] = {
"00: Cave Waterfall",
"01: Woods",
"02: Pond",
"03: 3D Stone & Lava",
"04: Forest & Mountains",
"05: Forest",
"06: Castle Waterfalls & Candles",
"07: Tropical Mountains",
"08: Forest",
"09: Jungle Mountains",
"0A: Waterfall",
"0B: Distant Ground",
"0C: Boggy Woods",
"0D: Night Sky",
"0E: Grass",
"0F: Volcano",
"10: Jungle Mountains",
"11: Unused",
"12: Ocean",
"13: Cave Crystals",
"14: Castle Cobwebs",
"15: Sky Mountains",
"16: BG2 HDMA sprites",
"17: Unused",
"18: Eerie Cave Forest",
"19: Castle Stones",
"1A: Sky Mountains",
"1B: Nothing",
"1C: Smiley Mountains",
"1D: Round Mountains",
"1E: Forest",
"1F: Baby Bowser's Room"};
LPCSTR headerBg3TilesetStrings[64] = {
"00: Nothing",
"01: Pond Water",
"02: BG3 HDMA sprites 1",
"03: Clouds",
"04: Clouds",
"05: BG3 HDMA sprites 2",
"06: BG3 HDMA sprites 3",
"07: BG3 HDMA sprites 4",
"08: Unused",
"09: Unused",
"0A: Cross Section",
"0B: Unused",
"0C: Shine",
"0D: Clouds & Mountains",
"0E: Boggy Woods",
"0F: Sky & Mountains",
"10: Sky Clouds",
"11: Hookbill Koopa's room fog",
"12: Night Sky & Raphael Raven's Moon",
"13: Water Tide",
"14: Jungle",
"15: Cave Crystals",
"16: Shark Chomp",
"17: Rocks",
"18: Castle Torches",
"19: Snowstorm",
"1A: Goonies",
"1B: Flower Garden",
"1C: Spotlight",
"1D: Water Tide",
"1E: Moon, Clouds & Mountains",
"1F: Kamek's magic shower",
"20: Bush & Palms",
"21: Prince Froggy's throat",
"22: Clouds & Mist",
"23: Sun",
"24: Starry Night",
"25: Boss Room",
"26: Boss pop effect",
"27: Forest",
"28: White Stars",
"29: Clouds",
"2A: Giant Moon",
"2B: Clouds",
"2C: Mist",
"2D: Mist",
"2E: Clouds",
"2F: Sky Clouds",
"30: Unused",
"31: Unused",
"32: Unused",
"33: Unused",
"34: Unused",
"35: Unused",
"36: Unused",
"37: Unused",
"38: Unused",
"39: Unused",
"3A: Unused",
"3B: Unused",
"3C: Unused",
"3D: Unused",
"3E: Unused",
"3F: Unused"};
LPCSTR headerAnimTilesetStrings[32] = {
"00: Common",
"01: BG3 Pond Water",
"02: BG2 Clouds",
"03: BG3 Smiley Clouds",
"04: Nothing",
"05: BG3 Water Tide",
"06: BG3 Castle Torches & Clouds",
"07: BG1 Castle Lava",
"08: BG1 Icy Water",
"09: BG3 Snowstorm",
"0A: BG3 Goonies",
"0B: BG2 Clouds, BG3 Butterflies",
"0C: BG1 Water",
"0D: BG1 Castle Lava, BG3 Castle Torches & Clouds",
"0E: BG1 Water, BG3 Castle Torches & Clouds",
"0F: BG3 Clouds",
"10: Unused",
"11: BG1 Water, BG3 Smiley Clouds",
"12: Unused",
"13: Unused",
"14: Unused",
"15: Unused",
"16: Unused",
"17: Unused",
"18: Unused",
"19: Unused",
"1A: Unused",
"1B: Unused",
"1C: Unused",
"1D: Unused",
"1E: Unused",
"1F: Unused"};
LPCSTR headerMusicStrings[16] = {
"00: Flower Garden",
"01: Jungle",
"02: Castle/Fortress",
"03: Before Boss Room",
"04: Underground",
"05: Kamek's Theme, Mini Boss",
"06: Bonus Game",
"07: Before Boss Room",
"08: Kamek's Theme, Big Boss",
"09: Big Boss",
"0A: Athletic",
"0B: Super Baby Mario",
"0C: Nothing",
"0D: Before Boss Room",
"0E: Unused",
"0F: Unused"};
LPCSTR levelNameStrings[72] = {
"1-1","1-2","1-3","1-4","1-5","1-6","1-7","1-8","1-E","Unused","Unused","Tutorial",
"2-1","2-2","2-3","2-4","2-5","2-6","2-7","2-8","2-E","Unused","Unused","Unused",
"3-1","3-2","3-3","3-4","3-5","3-6","3-7","3-8","3-E","Unused","Unused","Unused",
"4-1","4-2","4-3","4-4","4-5","4-6","4-7","4-8","4-E","Unused","Unused","Unused",
"5-1","5-2","5-3","5-4","5-5","5-6","5-7","5-8","5-E","Unused","Unused","Unused",
"6-1","6-2","6-3","6-4","6-5","6-6","6-7","6-8","6-E","Unused","Unused","Unused"};
LPCSTR levelMessageStrings[75] = {
"1-1","1-2","1-3","1-4","1-5","1-6","1-7","1-8","World 1 Misc. 1","World 1 Misc. 2","World 1 Misc. 3","World 1 Misc. 4",
"2-1","2-2","2-3","2-4","2-5","2-6","2-7","2-8","World 2 Misc. 1","World 2 Misc. 2","World 2 Misc. 3","World 2 Misc. 4",
"3-1","3-2","3-3","3-4","3-5","3-6","3-7","3-8","World 3 Misc. 1","World 3 Misc. 2","World 3 Misc. 3","World 3 Misc. 4",
"4-1","4-2","4-3","4-4","4-5","4-6","4-7","4-8","World 4 Misc. 1","World 4 Misc. 2","World 4 Misc. 3","World 4 Misc. 4",
"5-1","5-2","5-3","5-4","5-5","5-6","5-7","5-8","World 5 Misc. 1","World 5 Misc. 2","World 5 Misc. 3","World 5 Misc. 4",
"6-1","6-2","6-3","6-4","6-5","6-6","6-7","6-8","World 6 Misc. 1","World 6 Misc. 2","World 6 Misc. 3","World 6 Misc. 4",
"Minigames 1","Minigames 2","Minigames 3"};

INT_PTR DlgProc_dOpenLevelId(HWND hwnd,UINT msg,WPARAM wParam,LPARAM lParam) {
	switch(msg) {
		case WM_INITDIALOG: {
			//Add icon
			SendMessageA(hwnd,WM_SETICON,ICON_SMALL,(LPARAM)hiconMain);
			//Limit to 2 characters
			SendDlgItemMessageA(hwnd,20,EM_SETLIMITTEXT,2,0);
			//Init control values
			setHexVal_dlg(hwnd,20,curLevel);
			return TRUE;
		}
		case WM_CLOSE: {
			//Exit with code 0 (do nothing)
			EndDialog(hwnd,0);
			break;
		}
		case WM_COMMAND: {
			switch(LOWORD(wParam)) {
				case IDCANCEL: {
					//Have WM_CLOSE handle this
					SendMessageA(hwnd,WM_CLOSE,0,0);
					break;
				}
				case IDOK: {
					BYTE lev = getHexVal_dlg(hwnd,20);
					if(lev<=0xDD) {
						//Set level ID and exit with code 1 (load level)
						curLevel = lev;
						EndDialog(hwnd,1);
					} else {
						//Have WM_CLOSE handle this
						SendMessageA(hwnd,WM_CLOSE,0,0);
					}
					break;
				}
			}
			break;
		}
	}
	return 0;
}
INT_PTR DlgProc_dEditEntrances(HWND hwnd,UINT msg,WPARAM wParam,LPARAM lParam) {
	switch(msg) {
		case WM_INITDIALOG: {
			//Add icon
			SendMessageA(hwnd,WM_SETICON,ICON_SMALL,(LPARAM)hiconMain);
			//Init combo boxes
			for(int i=0; i<56; i++) {
				SendDlgItemMessageA(hwnd,20,CB_ADDSTRING,0,(LPARAM)worldLevelStrings[i]);
			}
			SendDlgItemMessageA(hwnd,20,CB_SETCURSEL,0,0);
			//Limit to 2 characters
			SendDlgItemMessageA(hwnd,21,EM_SETLIMITTEXT,2,0);
			SendDlgItemMessageA(hwnd,23,EM_SETLIMITTEXT,2,0);
			SendDlgItemMessageA(hwnd,24,EM_SETLIMITTEXT,2,0);
			//Init control values
			updateDialog_editEntrances(hwnd);
			return TRUE;
		}
		case WM_CLOSE: {
			//Exit with code 0 (do nothing)
			EndDialog(hwnd,0);
			break;
		}
		case WM_COMMAND: {
			switch(LOWORD(wParam)) {
				case 20: {
					if(HIWORD(wParam)==CBN_SELCHANGE) {
						updateDialog_editEntrances(hwnd);
					}
					break;
				}
				case IDCANCEL: {
					//Have WM_CLOSE handle this
					SendMessageA(hwnd,WM_CLOSE,0,0);
					break;
				}
				case IDOK: {
					int tlevIdx = SendDlgItemMessageA(hwnd,20,CB_GETCURSEL,0,0);
					BYTE lev = getHexVal_dlg(hwnd,21);
					BYTE xpos = getHexVal_dlg(hwnd,23);
					BYTE ypos = getHexVal_dlg(hwnd,24);
					if(lev<=0xDD && ypos<=0x7F) {
						//Set level entrance info and exit with code 1 (entrances changed)
						romBuf[0x0BF471+(tlevIdx<<2)] = lev;
						romBuf[0x0BF472+(tlevIdx<<2)] = xpos;
						romBuf[0x0BF473+(tlevIdx<<2)] = ypos;
						EndDialog(hwnd,1);
					} else {
						//Have WM_CLOSE handle this
						SendMessageA(hwnd,WM_CLOSE,0,0);
					}
					break;
				}
			}
			break;
		}
	}
	return 0;
}
INT_PTR DlgProc_dEditEntrances2(HWND hwnd,UINT msg,WPARAM wParam,LPARAM lParam) {
	switch(msg) {
		case WM_INITDIALOG: {
			//Add icon
			SendMessageA(hwnd,WM_SETICON,ICON_SMALL,(LPARAM)hiconMain);
			//Limit to 2 characters
			SendDlgItemMessageA(hwnd,23,EM_SETLIMITTEXT,2,0);
			SendDlgItemMessageA(hwnd,24,EM_SETLIMITTEXT,2,0);
			//Init control values
			updateDialog_editEntrances2(hwnd);
			return TRUE;
		}
		case WM_CLOSE: {
			//Exit with code 0 (do nothing)
			EndDialog(hwnd,0);
			break;
		}
		case WM_COMMAND: {
			switch(LOWORD(wParam)) {
				case IDCANCEL: {
					//Have WM_CLOSE handle this
					SendMessageA(hwnd,WM_CLOSE,0,0);
					break;
				}
				case IDOK: {
					BYTE xpos = getHexVal_dlg(hwnd,23);
					BYTE ypos = getHexVal_dlg(hwnd,24);
					if(ypos<=0x7F) {
						//Set level midpoint info and exit with code 1 (entrances changed)
						romBuf[0x0BF551+(curLevel<<1)] = xpos;
						romBuf[0x0BF552+(curLevel<<1)] = ypos;
						EndDialog(hwnd,1);
					} else {
						//Have WM_CLOSE handle this
						SendMessageA(hwnd,WM_CLOSE,0,0);
					}
					break;
				}
			}
			break;
		}
	}
	return 0;
}
INT_PTR DlgProc_dEditExits(HWND hwnd,UINT msg,WPARAM wParam,LPARAM lParam) {
	switch(msg) {
		case WM_INITDIALOG: {
			//Add icon
			SendMessageA(hwnd,WM_SETICON,ICON_SMALL,(LPARAM)hiconMain);
			//Init combo boxes
			char dlgStr[256];
			for(int i=0; i<128; i++) {
				snprintf(dlgStr,256,"%02X",i);
				SendDlgItemMessageA(hwnd,20,CB_ADDSTRING,0,(LPARAM)dlgStr);
			}
			SendDlgItemMessageA(hwnd,20,CB_SETCURSEL,0,0);
			for(int i=0; i<23; i++) {
				SendDlgItemMessageA(hwnd,22,CB_ADDSTRING,0,(LPARAM)actionMinigameStrings[i]);
			}
			//Limit to 2 characters
			SendDlgItemMessageA(hwnd,21,EM_SETLIMITTEXT,2,0);
			SendDlgItemMessageA(hwnd,23,EM_SETLIMITTEXT,2,0);
			SendDlgItemMessageA(hwnd,24,EM_SETLIMITTEXT,2,0);
			//Init control values
			updateDialog_editExits(hwnd);
			return TRUE;
		}
		case WM_CLOSE: {
			//Exit with code 0 (do nothing)
			EndDialog(hwnd,0);
			break;
		}
		case WM_COMMAND: {
			switch(LOWORD(wParam)) {
				case 20: {
					if(HIWORD(wParam)==CBN_SELCHANGE) {
						updateDialog_editExits(hwnd);
					}
					break;
				}
				case 25: {
					if(HIWORD(wParam)==BN_CLICKED) {
						bool enabled = SendDlgItemMessageA(hwnd,25,BM_GETCHECK,0,0)==BST_CHECKED;
						enabled = !enabled;
						SendDlgItemMessageA(hwnd,25,BM_SETCHECK,(WPARAM)(enabled?BST_CHECKED:BST_UNCHECKED),0);
						enableItem_dlg(hwnd,21,enabled);
						enableItem_dlg(hwnd,22,enabled);
						enableItem_dlg(hwnd,23,enabled);
						enableItem_dlg(hwnd,24,enabled);
					}
					break;
				}
				case IDCANCEL: {
					//Have WM_CLOSE handle this
					SendMessageA(hwnd,WM_CLOSE,0,0);
					break;
				}
				case IDOK: {
					int tscrIdx = SendDlgItemMessageA(hwnd,20,CB_GETCURSEL,0,0);
					tscrIdx <<= 2;
					bool enabled = SendDlgItemMessageA(hwnd,25,BM_GETCHECK,0,0)==BST_CHECKED;
					BYTE lev = getHexVal_dlg(hwnd,21);
					int amIdx = SendDlgItemMessageA(hwnd,22,CB_GETCURSEL,0,0);
					BYTE xpos = getHexVal_dlg(hwnd,23);
					BYTE ypos = getHexVal_dlg(hwnd,24);
					if(lev<=0xDD && ypos<=0x7F) {
						//Set screen exit info and exit with code 1 (exits changed)
						if(enabled) {
							if(amIdx<=10) {
								screenExits[tscrIdx] = lev;
								screenExits[tscrIdx+3] = amIdx;
							} else {
								screenExits[tscrIdx] = amIdx+0xD3;
								screenExits[tscrIdx+3] = lev;
							}
							screenExits[tscrIdx+1] = xpos;
							screenExits[tscrIdx+2] = ypos;
						} else {
							screenExits[tscrIdx] = 0xFF;
							screenExits[tscrIdx+1] = 0xFF;
							screenExits[tscrIdx+2] = 0xFF;
							screenExits[tscrIdx+3] = 0xFF;
						}
						EndDialog(hwnd,1);
					} else {
						//Have WM_CLOSE handle this
						SendMessageA(hwnd,WM_CLOSE,0,0);
					}
					break;
				}
			}
			break;
		}
	}
	return 0;
}
INT_PTR DlgProc_dEditHeader(HWND hwnd,UINT msg,WPARAM wParam,LPARAM lParam) {
	switch(msg) {
		case WM_INITDIALOG: {
			//Add icon
			SendMessageA(hwnd,WM_SETICON,ICON_SMALL,(LPARAM)hiconMain);
			//Init combo boxes
			char dlgStr[256];
			for(int i=0; i<16; i++) {
				snprintf(dlgStr,256,"Color %02X",i);
				SendDlgItemMessageA(hwnd,20,CB_ADDSTRING,0,(LPARAM)dlgStr);
			}
			for(int i=0; i<16; i++) {
				snprintf(dlgStr,256,"Gradient %02X",i);
				SendDlgItemMessageA(hwnd,20,CB_ADDSTRING,0,(LPARAM)dlgStr);
			}
			for(int i=0; i<16; i++) {
				SendDlgItemMessageA(hwnd,40,CB_ADDSTRING,0,(LPARAM)headerBg1TilesetStrings[i]);
			}
			for(int i=0; i<32; i++) {
				snprintf(dlgStr,256,"%02X",i);
				SendDlgItemMessageA(hwnd,41,CB_ADDSTRING,0,(LPARAM)dlgStr);
				SendDlgItemMessageA(hwnd,49,CB_ADDSTRING,0,(LPARAM)dlgStr);
				SendDlgItemMessageA(hwnd,21,CB_ADDSTRING,0,(LPARAM)dlgStr);
				SendDlgItemMessageA(hwnd,22,CB_ADDSTRING,0,(LPARAM)dlgStr);
			}
			for(int i=0; i<32; i++) {
				SendDlgItemMessageA(hwnd,42,CB_ADDSTRING,0,(LPARAM)headerBg2TilesetStrings[i]);
			}
			for(int i=0; i<64; i++) {
				snprintf(dlgStr,256,"%02X",i);
				SendDlgItemMessageA(hwnd,43,CB_ADDSTRING,0,(LPARAM)dlgStr);
				SendDlgItemMessageA(hwnd,45,CB_ADDSTRING,0,(LPARAM)dlgStr);
			}
			for(int i=0; i<64; i++) {
				SendDlgItemMessageA(hwnd,44,CB_ADDSTRING,0,(LPARAM)headerBg3TilesetStrings[i]);
			}
			for(int i=0; i<128; i++) {
				snprintf(dlgStr,256,"%02X: %02X,%02X,%02X,%02X,%02X,%02X",i,
					romBuf[0x003039+(i*6)],romBuf[0x00303A+(i*6)],romBuf[0x00303B+(i*6)],
					romBuf[0x00303C+(i*6)],romBuf[0x00303D+(i*6)],romBuf[0x00303E +(i*6)]);
				SendDlgItemMessageA(hwnd,46,CB_ADDSTRING,0,(LPARAM)dlgStr);
			}
			for(int i=0; i<16; i++) {
				snprintf(dlgStr,256,"%02X",i);
				SendDlgItemMessageA(hwnd,47,CB_ADDSTRING,0,(LPARAM)dlgStr);
			}
			for(int i=0; i<32; i++) {
				SendDlgItemMessageA(hwnd,48,CB_ADDSTRING,0,(LPARAM)headerAnimTilesetStrings[i]);
			}
			for(int i=0; i<16; i++) {
				SendDlgItemMessageA(hwnd,23,CB_ADDSTRING,0,(LPARAM)headerMusicStrings[i]);
			}
			for(int i=0; i<4; i++) {
				snprintf(dlgStr,256,"%d",i);
				SendDlgItemMessageA(hwnd,24,CB_ADDSTRING,0,(LPARAM)dlgStr);
			}
			//Init control values
			updateDialog_editHeader(hwnd);
			return TRUE;
		}
		case WM_CLOSE: {
			//Exit with code 0 (do nothing)
			EndDialog(hwnd,0);
			break;
		}
		case WM_COMMAND: {
			switch(LOWORD(wParam)) {
				case IDCANCEL: {
					//Have WM_CLOSE handle this
					SendMessageA(hwnd,WM_CLOSE,0,0);
					break;
				}
				case IDOK: {
					//Set level header info and exit with code 1 (header changed)
					int bgCol = SendDlgItemMessageA(hwnd,20,CB_GETCURSEL,0,0);
					int bg1Ts = SendDlgItemMessageA(hwnd,40,CB_GETCURSEL,0,0);
					int bg1Pal = SendDlgItemMessageA(hwnd,41,CB_GETCURSEL,0,0);
					int bg2Ts = SendDlgItemMessageA(hwnd,42,CB_GETCURSEL,0,0);
					int bg2Pal = SendDlgItemMessageA(hwnd,43,CB_GETCURSEL,0,0);
					int bg3Ts = SendDlgItemMessageA(hwnd,44,CB_GETCURSEL,0,0);
					int bg3Pal = SendDlgItemMessageA(hwnd,45,CB_GETCURSEL,0,0);
					int spTs = SendDlgItemMessageA(hwnd,46,CB_GETCURSEL,0,0);
					int spPal = SendDlgItemMessageA(hwnd,47,CB_GETCURSEL,0,0);
					int animTs = SendDlgItemMessageA(hwnd,48,CB_GETCURSEL,0,0);
					int animPal = SendDlgItemMessageA(hwnd,49,CB_GETCURSEL,0,0);
					int levMode = SendDlgItemMessageA(hwnd,21,CB_GETCURSEL,0,0);
					int bgScrl = SendDlgItemMessageA(hwnd,22,CB_GETCURSEL,0,0);
					int music = SendDlgItemMessageA(hwnd,23,CB_GETCURSEL,0,0);
					int itemMem = SendDlgItemMessageA(hwnd,24,CB_GETCURSEL,0,0);
					levelHeader[0] = (bgCol<<3)|(bg1Ts>>1);
					levelHeader[1] = (bg1Ts<<7)|(bg1Pal<<2)|(bg2Ts>>3);
					levelHeader[2] = (bg2Ts<<5)|(bg2Pal>>1);
					levelHeader[3] = (bg2Pal<<7)|(bg3Ts<<1)|(bg3Pal>>5);
					levelHeader[4] = (bg3Pal<<3)|(spTs>>4);
					levelHeader[5] = (spTs<<4)|spPal;
					levelHeader[6] = (levMode<<3)|(animTs>>3);
					levelHeader[7] = (animTs<<5)|animPal;
					levelHeader[8] = (bgScrl<<3)|(music>>1);
					levelHeader[9] = (music<<7)|(itemMem<<5);
					EndDialog(hwnd,1);
					break;
				}
			}
			break;
		}
	}
	return 0;
}
INT_PTR DlgProc_dEditLevNames(HWND hwnd,UINT msg,WPARAM wParam,LPARAM lParam) {
	switch(msg) {
		case WM_INITDIALOG: {
			//Add icon
			SendMessageA(hwnd,WM_SETICON,ICON_SMALL,(LPARAM)hiconMain);
			//Init combo boxes
			for(int i=0; i<72; i++) {
				SendDlgItemMessageA(hwnd,20,CB_ADDSTRING,0,(LPARAM)levelNameStrings[i]);
			}
			SendDlgItemMessageA(hwnd,20,CB_SETCURSEL,0,0);
			//Init control values
			updateDialog_editLevNames(hwnd);
			return TRUE;
		}
		case WM_CLOSE: {
			//Exit with code 0 (do nothing)
			EndDialog(hwnd,0);
			break;
		}
		case WM_COMMAND: {
			switch(LOWORD(wParam)) {
				case 20: {
					if(HIWORD(wParam)==CBN_SELCHANGE) {
						updateDialog_editLevNames(hwnd);
					}
					break;
				}
				case IDCANCEL: {
					//Have WM_CLOSE handle this
					SendMessageA(hwnd,WM_CLOSE,0,0);
					break;
				}
				case IDOK: {
					//Convert message data
					BYTE tempBufMsgData[0x8000];
					char tempBufMsgStr[0x8000];
					GetDlgItemTextA(hwnd,25,tempBufMsgStr,0x8000);
					int messageLen = writeGameText(tempBufMsgData,tempBufMsgStr);
					tempBufMsgData[messageLen++] = 0xFD;
					//Clear old message data
					int tMsgIdx = SendDlgItemMessageA(hwnd,20,CB_GETCURSEL,0,0);
					DWORD oldMessageOffset = romBuf[0x1149BC+(tMsgIdx<<1)]|(romBuf[0x1149BD+(tMsgIdx<<1)]<<8)|(romBuf[0x115348+tMsgIdx]<<16);
					if(oldMessageOffset&0x800000) {
						oldMessageOffset = convAddr_SNEStoPC_YI(oldMessageOffset);
						int oldMessageSize = romBuf[oldMessageOffset-4]|(romBuf[oldMessageOffset-3]<<8);
						memset(&romBuf[oldMessageOffset-8],0,oldMessageSize+9);
					}
					//Write message data to ROM and exit with code 1 (message changed)
					DWORD messageOffset = findFreespace(messageLen);
					memcpy(&romBuf[messageOffset],tempBufMsgData,messageLen);
					messageOffset = convAddr_PCtoSNES_YI(messageOffset);
					romBuf[0x1149BC+(tMsgIdx<<1)] = messageOffset&0xFF;
					romBuf[0x1149BD+(tMsgIdx<<1)] = (messageOffset>>8)&0xFF;
					romBuf[0x115348+tMsgIdx] = (messageOffset>>16)&0xFF;
					EndDialog(hwnd,1);
					break;
				}
			}
			break;
		}
	}
	return 0;
}
INT_PTR DlgProc_dEditLevMessages(HWND hwnd,UINT msg,WPARAM wParam,LPARAM lParam) {
	switch(msg) {
		case WM_INITDIALOG: {
			//Add icon
			SendMessageA(hwnd,WM_SETICON,ICON_SMALL,(LPARAM)hiconMain);
			//Init combo boxes
			char dlgStr[256];
			for(int i=0; i<75; i++) {
				SendDlgItemMessageA(hwnd,20,CB_ADDSTRING,0,(LPARAM)levelMessageStrings[i]);
			}
			SendDlgItemMessageA(hwnd,20,CB_SETCURSEL,0,0);
			for(int i=0; i<4; i++) {
				snprintf(dlgStr,256,"Message %02X",i);
				SendDlgItemMessageA(hwnd,21,CB_ADDSTRING,0,(LPARAM)dlgStr);
			}
			SendDlgItemMessageA(hwnd,21,CB_SETCURSEL,0,0);
			//Init control values
			updateDialog_editLevMessages(hwnd);
			return TRUE;
		}
		case WM_CLOSE: {
			//Exit with code 0 (do nothing)
			EndDialog(hwnd,0);
			break;
		}
		case WM_COMMAND: {
			switch(LOWORD(wParam)) {
				case 20: 
				case 21: {
					if(HIWORD(wParam)==CBN_SELCHANGE) {
						updateDialog_editLevMessages(hwnd);
					}
					break;
				}
				case IDCANCEL: {
					//Have WM_CLOSE handle this
					SendMessageA(hwnd,WM_CLOSE,0,0);
					break;
				}
				case IDOK: {
					//Convert message data
					BYTE tempBufMsgData[0x8000];
					char tempBufMsgStr[0x8000];
					GetDlgItemTextA(hwnd,25,tempBufMsgStr,0x8000);
					int messageLen = writeGameText(tempBufMsgData,tempBufMsgStr);
					tempBufMsgData[messageLen++] = 0xFD;
					//Clear old message data
					int tMsgIdx = SendDlgItemMessageA(hwnd,20,CB_GETCURSEL,0,0);
					int tMsgIdx2 = SendDlgItemMessageA(hwnd,21,CB_GETCURSEL,0,0);
					tMsgIdx = (tMsgIdx<<2)|tMsgIdx2;
					DWORD oldMessageOffset = romBuf[0x1110DB+(tMsgIdx<<1)]|(romBuf[0x1110DC+(tMsgIdx<<1)]<<8)|(romBuf[0x115390+tMsgIdx]<<16);
					if(oldMessageOffset&0x800000) {
						oldMessageOffset = convAddr_SNEStoPC_YI(oldMessageOffset);
						int oldMessageSize = romBuf[oldMessageOffset-4]|(romBuf[oldMessageOffset-3]<<8);
						memset(&romBuf[oldMessageOffset-8],0,oldMessageSize+9);
					}
					//Write message data to ROM and exit with code 1 (message changed)
					DWORD messageOffset = findFreespace(messageLen);
					memcpy(&romBuf[messageOffset],tempBufMsgData,messageLen);
					messageOffset = convAddr_PCtoSNES_YI(messageOffset);
					romBuf[0x1110DB+(tMsgIdx<<1)] = messageOffset&0xFF;
					romBuf[0x1110DC+(tMsgIdx<<1)] = (messageOffset>>8)&0xFF;
					romBuf[0x115390+tMsgIdx] = (messageOffset>>16)&0xFF;
					EndDialog(hwnd,1);
					break;
				}
			}
			break;
		}
	}
	return 0;
}

//////////////
//MENU STUFF//
//////////////
bool hasSmcHeader;
char romFilename[256];
HWND hwndMain,hwndTooltip;
HMENU hmenuMain;

//View states
bool eObj = true,eSp = false;
bool vObj = true,vSp = true;
bool vEnt = true,vExit = false,vW6 = false,vGrid = false,vAnim = false;
bool vSwA = false,vSwB = false;

//Helper functions
void updateMenu() {
	UINT enableState = isRomOpen?0:MF_GRAYED;
	//Enable/disable menu items
	//that depend on ROM being open
	//File
	EnableMenuItem(hmenuMain,1001,enableState);
	EnableMenuItem(hmenuMain,1002,enableState);
	EnableMenuItem(hmenuMain,1003,enableState);
	EnableMenuItem(hmenuMain,1010,enableState);
	EnableMenuItem(hmenuMain,1011,enableState);
	EnableMenuItem(hmenuMain,1020,enableState);
	EnableMenuItem(hmenuMain,1021,enableState);
	EnableMenuItem(hmenuMain,1022,enableState);
	//Edit
	EnableMenuItem(hmenuMain,1100,enableState);
	EnableMenuItem(hmenuMain,1101,enableState);
	//View
	EnableMenuItem(hmenuMain,1200,enableState);
	EnableMenuItem(hmenuMain,1201,enableState);
	EnableMenuItem(hmenuMain,1210,enableState);
	EnableMenuItem(hmenuMain,1211,enableState);
	EnableMenuItem(hmenuMain,1212,enableState);
	EnableMenuItem(hmenuMain,1213,enableState);
	EnableMenuItem(hmenuMain,1220,enableState);
	EnableMenuItem(hmenuMain,1221,enableState);
	EnableMenuItem(hmenuMain,1222,enableState);
	//Tools
	EnableMenuItem(hmenuMain,1300,enableState);
	EnableMenuItem(hmenuMain,1301,enableState);
	EnableMenuItem(hmenuMain,1302,enableState);
	EnableMenuItem(hmenuMain,1310,enableState);
	EnableMenuItem(hmenuMain,1311,enableState);
	EnableMenuItem(hmenuMain,1312,enableState);
	//Window
	EnableMenuItem(hmenuMain,1400,enableState);
	EnableMenuItem(hmenuMain,1401,enableState);
	EnableMenuItem(hmenuMain,1410,enableState);
	EnableMenuItem(hmenuMain,1411,enableState);
	EnableMenuItem(hmenuMain,1412,enableState);
	EnableMenuItem(hmenuMain,1413,enableState);
	//Update Edit Object/Sprite check state
	//and relevant enables/disables
	CheckMenuItem(hmenuMain,1100,eObj?MF_CHECKED:0);
	CheckMenuItem(hmenuMain,1101,eSp?MF_CHECKED:0);
	UINT enableObj = eObj?0:MF_GRAYED;
	enableObj |= enableState;
	EnableMenuItem(hmenuMain,1110,enableObj);
	EnableMenuItem(hmenuMain,1111,enableObj);
	//Update checked state for View menu items
	CheckMenuItem(hmenuMain,1200,vObj?MF_CHECKED:0);
	CheckMenuItem(hmenuMain,1201,vSp?MF_CHECKED:0);
	CheckMenuItem(hmenuMain,1210,vEnt?MF_CHECKED:0);
	CheckMenuItem(hmenuMain,1211,vExit?MF_CHECKED:0);
	CheckMenuItem(hmenuMain,1212,vGrid?MF_CHECKED:0);
	CheckMenuItem(hmenuMain,1213,vAnim?MF_CHECKED:0);
	CheckMenuItem(hmenuMain,1220,vW6?MF_CHECKED:0);
	CheckMenuItem(hmenuMain,1221,vSwA?MF_CHECKED:0);
	CheckMenuItem(hmenuMain,1222,vSwB?MF_CHECKED:0);
}
inline BOOL prompt(LPCSTR title,LPCSTR msg) {
	return (MessageBoxA(hwndMain,msg,title,MB_ICONWARNING|MB_YESNO) == IDYES);
}
inline BOOL promptSave() {
	return (MessageBoxA(hwndMain,"Level data has been modified. Are you sure?","Unsaved changes!",MB_ICONWARNING|MB_YESNO) == IDYES);
}
inline void updateEntireScreen() {
	RECT rect = {xCurScroll,yCurScroll,xCurScroll+xCurSize,yCurScroll+yCurSize};
	updateRect(rect);
	GetWindowRect(hwndMain,&rect);
	InvalidateRect(hwndMain,&rect,false);
}
inline void updateDialogs() {
	RECT rect = {0,0,256,384};
	if(wvisObject) {
		InvalidateRect(hwndObject,&rect,false);
		UpdateWindow(hwndObject);
	}
	if(wvisSprite) {
		InvalidateRect(hwndSprite,&rect,false);
		UpdateWindow(hwndSprite);
	}
	rect = {0,0,512,512};
	if(wvisMap8) {
		InvalidateRect(hwndMap8,&rect,false);
		UpdateWindow(hwndMap8);
	}
	if(wvisPalette) {
		InvalidateRect(hwndPalette,&rect,false);
		UpdateWindow(hwndPalette);
	}
	rect = {0,0,512,512};
	if(wvisMap16) {
		InvalidateRect(hwndMap16,&rect,false);
		UpdateWindow(hwndMap16);
	}
	rect = {0,0,1024,1024};
	if(wvisBackground) {
		InvalidateRect(hwndBackground,&rect,false);
		UpdateWindow(hwndBackground);
	}
}

//Functions for menu items
BYTE manifestXmlBuf[0x10000];
DWORD manifestXmlBufSize;
BYTE manifestBmlBuf[0x10000];
DWORD manifestBmlBufSize;

//File
void onOpen() {
	//Prompt save
	if(isRomOpen && !isRomSaved) {
		if(!promptSave()) return;
	}
	OPENFILENAMEA ofn;
	memset(&ofn,0,sizeof(OPENFILENAMEA));
	memset(romFilename,0,sizeof(romFilename));
	ofn.lStructSize	 = sizeof(OPENFILENAMEA);
	ofn.hwndOwner	 = hwndMain;
	ofn.lpstrFile	 = romFilename;
	ofn.nMaxFile	 = 256;
	ofn.lpstrTitle	 = "Open ROM";
	ofn.lpstrFilter	 = "SNES ROM Image (*.smc,*.sfc)\0*.smc;*.sfc\0";
	ofn.Flags		 = OFN_PATHMUSTEXIST|OFN_FILEMUSTEXIST;
	if(GetOpenFileNameA(&ofn)) {
		hasSmcHeader = (romFilename[ofn.nFileExtension+1] == 'm');
		//Load ROM
#ifdef YI_4MB_MODE
		memset(romBuf,0,0x400000);
#else
		memset(romBuf,0,0x800000);
#endif
		FILE * fp = fopen(romFilename,"rb");
		fseek(fp,0,SEEK_END);
		long fileSize = ftell(fp);
		fseek(fp,(fileSize&0x200)?0x200:0,SEEK_SET);
		fread(romBuf,1,fileSize&(~0x200),fp);
		fclose(fp);
		//Check ROM
		if(checkRom()) {
			isRomOpen = true;
			isRomSaved = true;
			updateMenu();
			//Load level
			loadLevel();
			updateDialogs();
			updateEntireScreen();
		}
	}
}
void onClose() {
	//Prompt save
	if(isRomOpen) {
		if(!isRomSaved) {
			if(!promptSave()) return;
		}
		isRomOpen = false;
		updateMenu();
		ShowWindow(hwndObject,SW_HIDE);
		ShowWindow(hwndSprite,SW_HIDE);
		ShowWindow(hwndMap8,SW_HIDE);
		ShowWindow(hwndMap16,SW_HIDE);
		ShowWindow(hwndPalette,SW_HIDE);
		ShowWindow(hwndBackground,SW_HIDE);
		wvisObject = false;
		wvisSprite = false;
		wvisMap8 = false;
		wvisMap16 = false;
		wvisPalette = false;
		wvisBackground = false;
		updateDialogs();
		updateEntireScreen();
	}
}
void onSave() {
	if(isRomOpen) {
		//Save level
		saveLevel();
		//Save manifest files
		//TODO
		//Save ROM
		FILE * fp = fopen(romFilename,"wb");
		if(hasSmcHeader) {
			for(int i=0; i<0x200; i++) {
				putc(0,fp);
			}
		}
		fwrite(romBuf,1,0x800000,fp);
		fclose(fp);
		isRomSaved = true;
	}
}
void onSaveAs() {
	if(isRomOpen) {
		OPENFILENAMEA ofn;
		memset(&ofn,0,sizeof(OPENFILENAMEA));
		memset(romFilename,0,sizeof(romFilename));
		ofn.lStructSize	 = sizeof(OPENFILENAMEA);
		ofn.hwndOwner	 = hwndMain;
		ofn.lpstrFile	 = romFilename;
		ofn.nMaxFile	 = 256;
		ofn.lpstrTitle	 = "Save ROM";
		ofn.lpstrFilter	 = "SNES ROM Image (*.smc,*.sfc)\0*.smc;*.sfc\0";
		if(GetSaveFileNameA(&ofn)) {
			hasSmcHeader = (romFilename[ofn.nFileExtension+1] == 'm');
			//Save level
			saveLevel();
			//Save manifest files
			//TODO
			//Save ROM
			FILE * fp = fopen(romFilename,"wb");
			if(hasSmcHeader) {
				for(int i=0; i<0x200; i++) {
					putc(0,fp);
				}
			}
			fwrite(romBuf,1,0x800000,fp);
			fclose(fp);
			isRomSaved = true;
		}
	}
}
void onQuit() {
	//Have WM_CLOSE handle this
	SendMessageA(hwndMain,WM_CLOSE,0,0);
}
void onImportLevel() {
	//Prompt save
	if(isRomOpen) {
		if(!isRomSaved) {
			if(!promptSave()) return;
		}
		OPENFILENAMEA ofn;
		char lfStr[256];
		memset(&ofn,0,sizeof(OPENFILENAMEA));
		memset(lfStr,0,256);
		ofn.lStructSize	 = sizeof(OPENFILENAMEA);
		ofn.hwndOwner	 = hwndMain;
		ofn.lpstrFile	 = lfStr;
		ofn.nMaxFile	 = 256;
		ofn.lpstrTitle	 = "Open Level File";
		ofn.lpstrFilter	 = "YI Level File (*.ylv)\0*.ylv;\0";
		ofn.Flags		 = OFN_PATHMUSTEXIST|OFN_FILEMUSTEXIST;
		if(GetOpenFileNameA(&ofn)) {
			//Load level file
			BYTE levelFile[0x10000];
			FILE * fp = fopen(lfStr,"rb");
			fseek(fp,0,SEEK_END);
			long fileSize = ftell(fp);
			rewind(fp);
			fread(levelFile,1,fileSize,fp);
			fclose(fp);
			//Get level pointer and load data
			DWORD objectAddr = levelFile[0]|(levelFile[1]<<8)|(levelFile[2]<<16)|(levelFile[3]<<24);
			DWORD spriteAddr = levelFile[8]|(levelFile[9]<<8)|(levelFile[10]<<16)|(levelFile[11]<<24);
			memcpy(levelHeader,&levelFile[objectAddr],10);
			objectAddr += 10;
			initOtherObjectBuffers();
			initOtherSpriteBuffers();
			objectAddr += loadObjects(&levelFile[objectAddr]);
			drawObjects();
			objectAddr++;
			memset(screenExits,0xFF,0x200);
			while(true) {
				int page = levelFile[objectAddr++];
				if(page==0xFF) break;
				page <<= 2;
				screenExits[page] = levelFile[objectAddr++];
				screenExits[page+1] = levelFile[objectAddr++];
				screenExits[page+2] = levelFile[objectAddr++];
				screenExits[page+3] = levelFile[objectAddr++];
			}
			loadSprites(&levelFile[spriteAddr]);
			drawSprites();
			//Load other stuff
			isRomSaved = false;
			loadMap8();
			loadMap16();
			loadPalette();
			loadBackground();
			updateDialogs();
			updateEntireScreen();
		}
	}
}
void onExportLevel() {
	if(isRomOpen) {
		OPENFILENAMEA ofn;
		char lfStr[256];
		memset(&ofn,0,sizeof(OPENFILENAMEA));
		memset(lfStr,0,256);
		ofn.lStructSize	 = sizeof(OPENFILENAMEA);
		ofn.hwndOwner	 = hwndMain;
		ofn.lpstrFile	 = lfStr;
		ofn.nMaxFile	 = 256;
		ofn.lpstrTitle	 = "Save Level File";
		ofn.lpstrFilter	 = "YI Level File (*.ylv)\0*.ylv;\0";
		if(GetSaveFileNameA(&ofn)) {
			//Determine level size
			BYTE tempBufObj[0x8000],tempBufSp[0x8000];
			memcpy(tempBufObj,levelHeader,10);
			int objectSize = 10;
			objectSize += saveObjects(&tempBufObj[10]);
			tempBufObj[objectSize++] = 0xFF;
			for(int i=0; i<0x200; i+=4) {
				if(screenExits[i]!=0xFF) {
					tempBufObj[objectSize++] = i>>2;
					for(int j=0; j<4; j++) {
						tempBufObj[objectSize++] = screenExits[i+j];
					}
				}
			}
			tempBufObj[objectSize++] = 0xFF;
			int spriteSize = saveSprites(tempBufSp);
			tempBufSp[spriteSize++] = 0xFF;
			tempBufSp[spriteSize++] = 0xFF;
			//Write level file data
			int objectOffset = 0x80;
			int spriteOffset = objectOffset+objectSize;
			FILE * fp = fopen(lfStr,"wb");
			putc(objectOffset,fp);
			putc(objectOffset>>8,fp);
			putc(objectOffset>>16,fp);
			putc(objectOffset>>24,fp);
			putc(objectSize,fp);
			putc(objectSize>>8,fp);
			putc(objectSize>>16,fp);
			putc(objectSize>>24,fp);
			putc(spriteOffset,fp);
			putc(spriteOffset>>8,fp);
			putc(spriteOffset>>16,fp);
			putc(spriteOffset>>24,fp);
			putc(spriteSize,fp);
			putc(spriteSize>>8,fp);
			putc(spriteSize>>16,fp);
			putc(spriteSize>>24,fp);
			for(int i=0; i<0x70; i++) {
				putc(0,fp);
			}
			fwrite(tempBufObj,1,objectSize,fp);
			fwrite(tempBufSp,1,spriteSize,fp);
			fclose(fp);
		}
	}
}
void onOpenLevel() {
	//Prompt save
	if(isRomOpen) {
		if(!isRomSaved) {
			if(!promptSave()) return;
		}
		if(DialogBoxA(NULL,MAKEINTRESOURCE(IDD_OPEN_LEVEL_ID),hwndMain,(DLGPROC)DlgProc_dOpenLevelId)) {
			isRomSaved = true;
			loadLevel();
			updateDialogs();
			updateEntireScreen();
		}
	}
}
void onNextLevel() {
	//Prompt save
	if(isRomOpen) {
		if(!isRomSaved) {
			if(!promptSave()) return;
		}
		if(curLevel!=0xDD) curLevel++;
		isRomSaved = true;
		loadLevel();
		updateDialogs();
		updateEntireScreen();
	}
}
void onPrevLevel() {
	//Prompt save
	if(isRomOpen) {
		if(!isRomSaved) {
			if(!promptSave()) return;
		}
		if(curLevel) curLevel--;
		isRomSaved = true;
		loadLevel();
		updateDialogs();
		updateEntireScreen();
	}
}
//Edit
void onEditObj() {
	clearSpriteSelection();
	eObj = true;
	eSp = false;
	updateMenu();
	updateEntireScreen();
}
void onEditSp() {
	clearObjectSelection();
	eSp = true;
	eObj = false;
	updateMenu();
	updateEntireScreen();
}
void onIncZ() {
	increaseObjectZ();
	isRomSaved = false;
	drawObjects();
	updateEntireScreen();
}
void onDecZ() {
	decreaseObjectZ();
	isRomSaved = false;
	drawObjects();
	updateEntireScreen();
}
//View
void onViewObj() {
	vObj = !vObj;
	updateMenu();
	updateEntireScreen();
}
void onViewSp() {
	vSp = !vSp;
	updateMenu();
	updateEntireScreen();
}
void onViewEnt() {
	vEnt = !vEnt;
	updateMenu();
	updateEntireScreen();
}
void onViewExit() {
	vExit = !vExit;
	updateMenu();
	updateEntireScreen();
}
void onViewGrid() {
	vGrid = !vGrid;
	updateMenu();
	updateEntireScreen();
}
void onViewAnim() {
	vAnim = !vAnim;
	updateMenu();
}
void onViewW6() {
	vW6 = !vW6;
	updateMenu();
	updateMap8W6(vW6);
	updatePaletteW6(vW6);
	updateBackground();
	updateDialogs();
	updateEntireScreen();
}
void onViewSwA() {
	vSwA = !vSwA;
	updateMenu();
	updateMap8Sw((vSwA?0x08:0)|(vSwB?0x10:0));
	updateBackground();
	updateDialogs();
	updateEntireScreen();
}
void onViewSwB() {
	vSwB = !vSwB;
	updateMenu();
	updateMap8Sw((vSwA?0x08:0)|(vSwB?0x10:0));
	updateBackground();
	updateDialogs();
	updateEntireScreen();
}
//Tools
void onChgEnt() {
	if(DialogBoxA(NULL,MAKEINTRESOURCE(IDD_EDIT_ENTRANCE),hwndMain,(DLGPROC)DlgProc_dEditEntrances) && vEnt) {
		isRomSaved = false;
		updateEntireScreen();
	}
}
void onChgEnt2() {
	if(DialogBoxA(NULL,MAKEINTRESOURCE(IDD_EDIT_ENTRANCE2),hwndMain,(DLGPROC)DlgProc_dEditEntrances2) && vEnt) {
		isRomSaved = false;
		updateEntireScreen();
	}
}
void onChgExit() {
	if(DialogBoxA(NULL,MAKEINTRESOURCE(IDD_EDIT_EXIT),hwndMain,(DLGPROC)DlgProc_dEditExits) && vExit) {
		isRomSaved = false;
		updateEntireScreen();
	}
}
void onChgHead() {
	if(DialogBoxA(NULL,MAKEINTRESOURCE(IDD_EDIT_HEADER),hwndMain,(DLGPROC)DlgProc_dEditHeader)) {
		isRomSaved = false;
		initOtherObjectBuffers();
		initOtherSpriteBuffers();
		drawObjects();
		drawSprites();
		loadMap8();
		loadMap16();
		loadPalette();
		loadBackground();
		updateDialogs();
		updateEntireScreen();
	}
}
void onChgLevName() {
	DialogBoxA(NULL,MAKEINTRESOURCE(IDD_EDIT_LEV_NAME),hwndMain,(DLGPROC)DlgProc_dEditLevNames);
}
void onChgLevMsg() {
	DialogBoxA(NULL,MAKEINTRESOURCE(IDD_EDIT_LEV_MSG),hwndMain,(DLGPROC)DlgProc_dEditLevMessages);
}
//Window
void onSelObj() {
	ShowWindow(hwndObject,SW_SHOWNORMAL);
	wvisObject = true;
}
void onSelSp() {
	ShowWindow(hwndSprite,SW_SHOWNORMAL);
	wvisSprite = true;
}
void onEditMap8() {
	ShowWindow(hwndMap8,SW_SHOWNORMAL);
	wvisMap8 = true;
}
void onEditMap16() {
	ShowWindow(hwndMap16,SW_SHOWNORMAL);
	wvisMap16 = true;
}
void onEditPal() {
	ShowWindow(hwndPalette,SW_SHOWNORMAL);
	wvisPalette = true;
}
void onEditBg() {
	ShowWindow(hwndBackground,SW_SHOWNORMAL);
	wvisBackground = true;
}

//Tables used for menu processing
#define NUM_COMMANDS (10+4+9+6+6)
void (*cmMenuFunc[NUM_COMMANDS])() = {
//File
	onOpen,onClose,onSave,onSaveAs,onQuit,
	onImportLevel,onExportLevel,
	onOpenLevel,onNextLevel,onPrevLevel,
//Edit
	onEditObj,onEditSp,
	onIncZ,onDecZ,
//View
	onViewObj,onViewSp,
	onViewEnt,onViewExit,onViewGrid,onViewAnim,
	onViewW6,onViewSwA,onViewSwB,
//Tools
	onChgEnt,onChgEnt2,onChgExit,onChgHead,
	onChgLevName,onChgLevMsg,
//Window
	onSelObj,onSelSp,
	onEditMap8,onEditMap16,onEditPal,onEditBg};
int cmMenuCommand[NUM_COMMANDS] = {
//File
	1000,1001,1002,1003,1004,
	1010,1011,
	1020,1021,1022,
//Edit
	1100,1101,
	1110,1111,
//View
	1200,1201,
	1210,1211,1212,1213,
	1220,1221,1222,
//Tools
	1300,1301,1302,
	1310,1311,1312,
//Window
	1400,1401,
	1410,1411,1412,1413};

/////////////////////
//MAIN WINDOW STUFF//
/////////////////////
HDC				hdcMain;
HBITMAP			hbmpMain;
DWORD *			bmpDataMain;
int xCurScroll = 0,xMaxScroll = 0,xCurSize = 640;
int yCurScroll = 0,yMaxScroll = 0,yCurSize = 480;
bool dragFlag = false;
int selOp = 4;
POINT selpCur,selpPrev = {0,0};

//Helper function for drawing Yoshi sprite
WORD yoshiSpriteTileData[66] = {
	0x614C,0x614D,0x613D,0x4328,0x4329,0x63D5,
	0x6504,0x6524,0x6506,0x6526,0xFFFF,0xFFFF,
	0x413F,0x404F,0x614C,0x614D,0x613D,0x6390,
	0x013F,0x004F,0x214C,0x214D,0x213D,0x2390,
	0x6357,0x4068,0x23DB,0x23DC,0x0068,0xFFFF,
	0x6357,0x4068,0x23DB,0x23DC,0x0068,0xFFFF,
	0x413F,0x404F,0x614C,0x614D,0x613D,0x6390,
	0x013F,0x004F,0x214C,0x214D,0x213D,0x2390,
	0x400F,0x6396,0x613D,0x614D,0x614C,0x400F,
	0xE114,0xE104,0xE103,0x6390,0x405F,0x405F,
	0x400C,0x413F,0x6174,0x6173,0x6164,0x63B6};
int yoshiSpriteOffsXData[66] = {
	 11,  3,  3,  9,  1,  0,
	  8,  8, -8, -8,  0,  0,
	  9, -1, 12,  4,  4,  0,
	 -1,  9,-12, -4, -4,  0,
	  0,  9, -4,  4,  0,  0,
	  0,  9, -4,  4,  0,  0,
	  9, -1, 12,  4,  4,  0,
	 -1,  9,-12, -4, -4,  0,
	  8,  0,  1,  1,  9, 13,
	  3,  3, 11,  0, -2,  3,
	  4, 15,  3, 11,  3,  0};
int yoshiSpriteOffsYData[66] = {
	  2,  2, -6, 24, 24, 16,
	  4, 20,  4, 20,  0,  0,
	 24, 24,  2,  2, -6, 15,
	 24, 24,  2,  2, -6, 15,
	  4, 22, 14, 14, 24,  0,
	  4, 22, 14, 14, 24,  0,
	 24, 24,  2,  2, -6, 15,
	 24, 24,  2,  2, -6, 15,
	 21, 12, -6,  2,  2, 21,
	 -7,  1,  1, 15, 26, 26,
	 18, 18,  2,  2, -6, 14};
void dispYoshiSprite(int action,int x,int y) {
	for(int n=5; n>=0; n--) {
		int tp = yoshiSpriteTileData[(action*6)+n];
		if(tp==0xFFFF) continue;
		int tile = (tp&0x07FF)|0x2000;
		int props = ((tp&0xC000)>>8)|((tp&0x2000)>>13)|(0xD<<2);
		int xpos = x+yoshiSpriteOffsXData[(action*6)+n];
		int ypos = y+yoshiSpriteOffsYData[(action*6)+n];
		dispMap8Tile(bmpDataMain,0x1000,0x800,props,tile,{xpos,ypos},false);
	}
}

//Extra UI drawing stuff
void dispEntrances(RECT rect) {
	char strBuf[256];
	//Draw main entrances
	for(int i=0; i<56; i++) {
		if(romBuf[0x0BF471+(i<<2)]==curLevel) {
			int xpos = romBuf[0x0BF472+(i<<2)]<<4;
			int ypos = romBuf[0x0BF473+(i<<2)]<<4;
			dispYoshiSprite(0,xpos,ypos);
			snprintf(strBuf,256,"Main Entrance to %s",worldLevelStrings[i]);
			for(int n=0; n<strlen(strBuf); n++) {
				dispMap8Char(bmpDataMain,0x1000,0x800,0xFFFFFF,0xFF,strBuf[n],{xpos+(n<<3),ypos},false);
			}
		}
	}
	//Draw midway entrance
	int xpos = romBuf[0x0BF551+(curLevel<<1)]<<4;
	int ypos = romBuf[0x0BF552+(curLevel<<1)]<<4;
	dispYoshiSprite(0,xpos,ypos);
	snprintf(strBuf,256,"Midway Entrance to Level %02X",curLevel);
	for(int n=0; n<strlen(strBuf); n++) {
		dispMap8Char(bmpDataMain,0x1000,0x800,0xFFFFFF,0xFF,strBuf[n],{xpos+(n<<3),ypos},false);
	}
}
void dispExits(RECT rect) {
	int minx = std::max((int)(rect.left&0x7F00),0);
	int miny = std::max((int)(rect.top&0x7F00),0);
	int maxx = std::min((int)((rect.right&0x7F00)+0x100),0x1000);
	int maxy = std::min((int)((rect.bottom&0x7F00)+0x100),0x800);
	for(int j=miny; j<maxy; j+=0x100) {
		for(int i=minx; i<maxx; i+=0x100) {
			//Draw screen borders
			for(int n=0; n<0x100; n++) {
				putPixel(bmpDataMain,0x1000,0x800,0xFF,{i+n,j});
				putPixel(bmpDataMain,0x1000,0x800,0xFF,{i,j+n});
				putPixel(bmpDataMain,0x1000,0x800,0xFF,{i+n,j+0x01});
				putPixel(bmpDataMain,0x1000,0x800,0xFF,{i+0x01,j+n});
				putPixel(bmpDataMain,0x1000,0x800,0xFF,{i+n,j+0xFE});
				putPixel(bmpDataMain,0x1000,0x800,0xFF,{i+0xFE,j+n});
				putPixel(bmpDataMain,0x1000,0x800,0xFF,{i+n,j+0xFF});
				putPixel(bmpDataMain,0x1000,0x800,0xFF,{i+0xFF,j+n});
			}
			//Highlight screens which have exits, and draw screen exit info text
			int screen = (i>>8)|(j>>4);
			char strBuf[256];
			int screenDest = screenExits[screen<<2];
			if(screenDest<0xE9) {
				for(int l=0x02; l<0xFE; l++) {
					for(int k=0x02; k<0xFE; k++) {
						hilitePixel(bmpDataMain,0x1000,0x800,0x80,{i+k,j+l});
					}
				}
				if(screenDest<=0xDD) {
					snprintf(strBuf,256,"%02X: Exit to Level %02X",screen,screenDest);
				} else {
					snprintf(strBuf,256,"%02X: Exit to Mini %02X",screen,screenDest-0xDE);
				}
			} else {
				snprintf(strBuf,256,"%02X",screen);
			}
			for(int n=0; n<strlen(strBuf); n++) {
				dispMap8Char(bmpDataMain,0x1000,0x800,0xFFFFFF,0xFF,strBuf[n],{i+(n<<3)+2,j+2},false);
			}
		}
	}
}
void dispGrid(RECT rect) {
	int minx = std::max((int)(rect.left&0x7FF0),0);
	int miny = std::max((int)(rect.top&0x7FF0),0);
	int maxx = std::min((int)((rect.right&0x7FF0)+0x10),0x1000);
	int maxy = std::min((int)((rect.bottom&0x7FF0)+0x10),0x800);
	for(int j=miny; j<maxy; j+=0x10) {
		for(int i=minx; i<maxx; i+=0x10) {
			for(int n=0; n<0x10; n++) {
				putPixel(bmpDataMain,0x1000,0x800,0xFFFFFF,{i+n,j});
				putPixel(bmpDataMain,0x1000,0x800,0xFFFFFF,{i,j+n});
				putPixel(bmpDataMain,0x1000,0x800,0xFFFFFF,{i+n,j+0xF});
				putPixel(bmpDataMain,0x1000,0x800,0xFFFFFF,{i+0xF,j+n});
			}
		}
	}
}

//Main drawing code
void updateRect(RECT rect) {
	if(isRomOpen) {
		//Fill background
		for(int j=rect.top; j<rect.bottom; j++) {
			DWORD rowColor = gradientBuffer[0x17];
			if(j<0x120) rowColor = gradientBuffer[0];
			else if(j<0x6E0) {
				int idx = (j-0x120)>>6;
				int fac1 = (j-0x120)&0x3F;
				int fac0 = 0x40-fac1;
				DWORD r0 = gradientBuffer[idx]&0xFF0000;
				DWORD g0 = gradientBuffer[idx]&0xFF00;
				DWORD b0 = gradientBuffer[idx]&0xFF;
				DWORD r1 = gradientBuffer[idx+1]&0xFF0000;
				DWORD g1 = gradientBuffer[idx+1]&0xFF00;
				DWORD b1 = gradientBuffer[idx+1]&0xFF;
				DWORD r = ((fac0*r0)+(fac1*r1))>>6;
				r &= 0xFF0000;
				DWORD g = ((fac0*g0)+(fac1*g1))>>6;
				g &= 0xFF00;
				DWORD b = ((fac0*b0)+(fac1*b1))>>6;
				rowColor = r|g|b;
				//rowColor &= 0xF8F8F8;
				//rowColor |= (rowColor>>5)&0x070707;
			}
			for(int i=rect.left; i<rect.right; i++) {
				putPixel(bmpDataMain,0x1000,0x800,rowColor,{i,j});
			}
		}
		//Draw objects
		if(vObj) {
			dispObjects(bmpDataMain,0x1000,0x800,rect);
		}
		//Draw sprites
		if(vSp) {
			dispSprites(bmpDataMain,0x1000,0x800,rect);
		}
		//Draw entrances
		if(vEnt) {
			dispEntrances(rect);
		}
		//Draw grid
		if(vGrid) {
			dispGrid(rect);
		}
		//Draw exits
		if(vExit) {
			dispExits(rect);
		}
		//Draw selection rectangle
		if(dragFlag && selOp==4) {
			int minx = std::min(selpCur.x,selpPrev.x);
			int miny = std::min(selpCur.y,selpPrev.y);
			int maxx = std::max(selpCur.x,selpPrev.x);
			int maxy = std::max(selpCur.y,selpPrev.y);
			for(int i=minx; i<maxx; i++) {
				invertPixel(bmpDataMain,0x1000,0x800,{i,miny});
				invertPixel(bmpDataMain,0x1000,0x800,{i+1,maxy});
			}
			for(int j=miny; j<maxy; j++) {
				invertPixel(bmpDataMain,0x1000,0x800,{minx,j+1});
				invertPixel(bmpDataMain,0x1000,0x800,{maxx,j});
			}
		}
	} else {
		//Fill with black
		fillImage(bmpDataMain,0x1000,0x800,0);
	}
}

//Message loop function
LRESULT CALLBACK WndProc(HWND hwnd,UINT msg,WPARAM wParam,LPARAM lParam) {
	switch(msg) {
		//Creation and destruction of window(s)
		case WM_CREATE: {
			//Setup tooltip
			hwndTooltip = CreateWindowA(TOOLTIPS_CLASS,NULL,TTS_NOPREFIX|TTS_ALWAYSTIP,
				CW_USEDEFAULT,
				CW_USEDEFAULT,
				CW_USEDEFAULT,
				CW_USEDEFAULT,
				hwnd,NULL,hinstMain,NULL);
			TOOLINFOA ti;
			memset(&ti,0,sizeof(TOOLINFOA));
			ti.cbSize		= sizeof(TOOLINFOA);
			ti.uFlags		= TTF_SUBCLASS;
			ti.hwnd			= hwnd;
			ti.uId			= 600;
			ti.rect			= {0,0,640,480};
			ti.hinst		= hinstMain;
			SendMessageA(hwndTooltip,TTM_SETMAXTIPWIDTH,0,320);
			SendMessageA(hwndTooltip,TTM_ADDTOOL,0,(LPARAM)&ti);
			//Create objects
			hdcMain = GetDC(hwnd);
			BITMAPINFO bmi;
			memset(&bmi.bmiHeader,0,sizeof(BITMAPINFOHEADER));
			bmi.bmiHeader.biSize			= sizeof(BITMAPINFOHEADER);
			bmi.bmiHeader.biPlanes			= 1;
			bmi.bmiHeader.biBitCount		= 32;
			bmi.bmiHeader.biCompression		= BI_RGB;
			bmi.bmiHeader.biWidth			= 0x1000;
			bmi.bmiHeader.biHeight			= -0x800;
			hbmpMain = CreateDIBSection(hdcMain,&bmi,DIB_RGB_COLORS,(void**)&bmpDataMain,NULL,0);
			memset(bmpDataMain,0,0x800000*sizeof(DWORD));
			break;
		}
		case WM_DESTROY: {
			//Free objects
			DeleteDC(hdcMain);
			DeleteObject(hbmpMain);
			//Close program
			PostQuitMessage(0);
			break;
		}
		case WM_CLOSE: {
			//Prompt save
			if(isRomOpen && !isRomSaved) {
				if(!promptSave()) break;
			}
			//Destroy window(s)
			DestroyWindow(hwnd);
			break;
		}
		//Updating
		case WM_PAINT: {
			//Blit screen
			PAINTSTRUCT ps;
			BeginPaint(hwnd,&ps);
			HDC hdcMem = CreateCompatibleDC(hdcMain);
			HBITMAP hbmpOld = (HBITMAP)SelectObject(hdcMem,hbmpMain);
			BitBlt(hdcMain,0,0,xCurSize,yCurSize,hdcMem,xCurScroll,yCurScroll,SRCCOPY);
			SelectObject(hdcMem,hbmpOld);
			DeleteDC(hdcMem);
			EndPaint(hwnd,&ps);
			break;
		}
		case WM_TIMER: {
			if(isRomOpen && vAnim) {
				updateMap8();
				updatePalette();
				updateBackground();
				updateDialogs();
				updateEntireScreen();
			}
			break;
		}
		case WM_SIZE: {
			//Resize visible region and invalidate
			SCROLLINFO si;
			memset(&si,0,sizeof(SCROLLINFO));
			si.cbSize		= sizeof(SCROLLINFO);
			si.fMask		= SIF_RANGE|SIF_POS;
			
			xCurSize = LOWORD(lParam);
			yCurSize = HIWORD(lParam);
			xMaxScroll = std::max(0x1000-xCurSize,0);
			yMaxScroll = std::max(0x800-yCurSize,0);
			xCurScroll = std::min(xCurScroll,xMaxScroll);
			yCurScroll = std::min(yCurScroll,yMaxScroll);
			
			si.nMax			= xMaxScroll;
			si.nPos			= xCurScroll;
			SetScrollInfo(hwnd,SB_HORZ,&si,TRUE);
			si.nMax			= yMaxScroll;
			si.nPos			= yCurScroll;
			SetScrollInfo(hwnd,SB_VERT,&si,TRUE);
			
			updateEntireScreen();
			break;
		}
		case WM_HSCROLL: {
			//Update visible region and invalidate
			int xNewScroll = xCurScroll;
			switch(LOWORD(wParam)) {
				case SB_LEFT: {
					xNewScroll = 0;
					break;
				}
				case SB_RIGHT: {
					xNewScroll = xMaxScroll;
					break;
				}
				case SB_PAGELEFT: {
					xNewScroll -= 0x100;
					break;
				}
				case SB_PAGERIGHT: {
					xNewScroll += 0x100;
					break;
				}
				case SB_LINELEFT: {
					xNewScroll -= 0x10;
					break;
				}
				case SB_LINERIGHT: {
					xNewScroll += 0x10;
					break;
				}
				case SB_THUMBTRACK:
				case SB_THUMBPOSITION: {
					xNewScroll = HIWORD(wParam);
					break;
				}
			}
			xNewScroll = std::max(0,xNewScroll);
			xNewScroll = std::min(xMaxScroll,xNewScroll);
			if(xNewScroll == xCurScroll) break;
			ScrollWindowEx(hwnd,xCurScroll-xNewScroll,0,NULL,NULL,NULL,NULL,SW_INVALIDATE);
			UpdateWindow(hwnd);
			xCurScroll = xNewScroll;
			
			SCROLLINFO si;
			memset(&si,0,sizeof(SCROLLINFO));
			si.cbSize		= sizeof(SCROLLINFO);
			si.fMask		= SIF_POS;
			si.nPos			= xCurScroll;
			SetScrollInfo(hwnd,SB_HORZ,&si,TRUE);
			
			updateEntireScreen();
			break;
		}
		case WM_VSCROLL: {
			//Update visible region and invalidate
			int yNewScroll = yCurScroll;
			switch(LOWORD(wParam)) {
				case SB_TOP: {
					yNewScroll = 0;
					break;
				}
				case SB_BOTTOM: {
					yNewScroll = yMaxScroll;
					break;
				}
				case SB_PAGEUP: {
					yNewScroll -= 0x100;
					break;
				}
				case SB_PAGEDOWN: {
					yNewScroll += 0x100;
					break;
				}
				case SB_LINEUP: {
					yNewScroll -= 0x10;
					break;
				}
				case SB_LINEDOWN: {
					yNewScroll += 0x10;
					break;
				}
				case SB_THUMBTRACK:
				case SB_THUMBPOSITION: {
					yNewScroll = HIWORD(wParam);
					break;
				}
			}
			yNewScroll = std::max(0,yNewScroll);
			yNewScroll = std::min(yMaxScroll,yNewScroll);
			if(yNewScroll == yCurScroll) break;
			ScrollWindowEx(hwnd,0,yCurScroll-yNewScroll,NULL,NULL,NULL,NULL,SW_INVALIDATE);
			UpdateWindow(hwnd);
			yCurScroll = yNewScroll;
			
			SCROLLINFO si;
			memset(&si,0,sizeof(SCROLLINFO));
			si.cbSize		= sizeof(SCROLLINFO);
			si.fMask		= SIF_POS;
			si.nPos			= yCurScroll;
			SetScrollInfo(hwnd,SB_VERT,&si,TRUE);
			
			updateEntireScreen();
			break;
		}
		//Menu input
		case WM_COMMAND: {
			for(int i=0; i<NUM_COMMANDS; i++) {
				if(cmMenuCommand[i] == LOWORD(wParam)) {
					(*cmMenuFunc[i])();
					break;
				}
			}
			break;
		}
		case WM_KEYDOWN: {
			if(wParam==VK_DELETE) {
				if(eObj) {
					deleteObjects();
					drawObjects();
				} else if(eSp) {
					deleteSprites();
					drawSprites();
				}
				updateEntireScreen();
				isRomSaved = false;
			}
			break;
		}
		//Mouse input
		case WM_MOUSEMOVE: {
			int mouseX = GET_X_LPARAM(lParam);
			int mouseY = GET_Y_LPARAM(lParam);
			int levX = mouseX+xCurScroll;
			int levY = mouseY+yCurScroll;
			levX = std::max(0,levX);
			levX = std::min(0xFFFF,levX);
			levY = std::max(0,levY);
			levY = std::min(0x7FFF,levY);
			if(dragFlag) {
				//Do fast edge scroll
				if(selOp!=4) selpPrev = selpCur;
				selpCur = {levX,levY};
				RECT clRect;
				GetClientRect(hwnd,&clRect);
				if(mouseX==0) {
					selpCur.x = std::max((long)0,selpCur.x-0x10);
					SendMessageA(hwnd,WM_HSCROLL,MAKEWPARAM(SB_LINELEFT,0),0);
				} else if(mouseX==(clRect.right-1)) {
					selpCur.x = std::max((long)0xFFFF,selpCur.x+0x10);
					SendMessageA(hwnd,WM_HSCROLL,MAKEWPARAM(SB_LINERIGHT,0),0);
				}
				if(mouseY==0) {
					selpCur.y = std::max((long)0,selpCur.y-0x10);
					SendMessageA(hwnd,WM_VSCROLL,MAKEWPARAM(SB_LINEUP,0),0);
				} else if(mouseY==(clRect.bottom-1)) {
					selpCur.y = std::max((long)0x7FFF,selpCur.y+0x10);
					SendMessageA(hwnd,WM_VSCROLL,MAKEWPARAM(SB_LINEDOWN,0),0);
				}
				//Handle selection modes
				if(selOp==4) {
					RECT selRect = {
					std::min(selpCur.x,selpPrev.x),
					std::min(selpCur.y,selpPrev.y),
					std::max(selpCur.x,selpPrev.x),
					std::max(selpCur.y,selpPrev.y)};
					if(eObj) {
						selectObjects(selRect);
					} else if(eSp) {
						selectSprites(selRect);
					}
				} else if(selOp==5) {
					int dx = (selpCur.x&0xFFF0)-(selpPrev.x&0xFFF0);
					int dy = (selpCur.y&0x7FF0)-(selpPrev.y&0x7FF0);
					if(eObj) {
						moveObjects(dx,dy);
						drawObjects();
					} else if(eSp) {
						moveSprites(dx,dy);
						drawSprites();
					}
					isRomSaved = false;
				} else {
					int dx = 0;
					int dy = 0;
					if(selOp&1) dx = (selpCur.x&0xFFF0)-(selpPrev.x&0xFFF0);
					if(selOp&2) dy = (selpCur.y&0x7FF0)-(selpPrev.y&0x7FF0);
					if(eObj) {
						resizeObjects(dx,dy);
						drawObjects();
					}
					isRomSaved = false;
				}
			} else {
				//Determine what the mode would be if we were dragging and set cursor accordingly
				UINT cursor = 0x7F00; //IDC_ARROW
				char tipText[256] = "";
				if(eObj) {
					selOp = focusObject(levX,levY,&cursor,tipText);
				} else if(eSp) {
					selOp = focusSprite(levX,levY,&cursor,tipText);
				}
				SetCursor(LoadCursorA(NULL,(LPCSTR)cursor));
				//Setup tooltip for top object/sprite
				TOOLINFOA ti;
				memset(&ti,0,sizeof(TOOLINFOA));
				ti.cbSize		= sizeof(TOOLINFOA);
				ti.uFlags		= TTF_SUBCLASS;
				ti.hwnd			= hwnd;
				ti.uId			= 600;
				ti.rect			= {mouseX,mouseY,mouseX+2,mouseY+2};
				ti.hinst		= hinstMain;
				ti.lpszText		= tipText;
				SendMessageA(hwndTooltip,TTM_NEWTOOLRECT,0,(LPARAM)&ti);
				SendMessageA(hwndTooltip,TTM_UPDATETIPTEXT,0,(LPARAM)&ti);
				SendMessageA(hwndTooltip,TTM_ACTIVATE,tipText[0]!='\0',0);
			}
			
			updateEntireScreen();
			break;
		}
		case WM_LBUTTONDOWN: {
			//Init drag
			int mouseX = GET_X_LPARAM(lParam);
			int mouseY = GET_Y_LPARAM(lParam);
			int levX = mouseX+xCurScroll;
			int levY = mouseY+yCurScroll;
			levX = std::max(0,levX);
			levX = std::min(0xFFFF,levX);
			levY = std::max(0,levY);
			levY = std::min(0x7FFF,levY);
			if(selOp==5) {
				if(eObj) {
					selectTopObject(levX,levY);
				} else if(eSp) {
					selectTopSprite(levX,levY);
				}
			}
			selpCur = selpPrev = {levX,levY};
			dragFlag = true;
			//Clip cursor
			RECT clipRect;
			GetClientRect(hwnd,&clipRect);
			ClientToScreen(hwnd,(LPPOINT)&clipRect.left);
			ClientToScreen(hwnd,(LPPOINT)&clipRect.right);
			ClipCursor(&clipRect);
			
			updateEntireScreen();
			break;
		}
		case WM_RBUTTONDOWN: {
			//Insert stuff and init drag (move mode)
			int mouseX = GET_X_LPARAM(lParam);
			int mouseY = GET_Y_LPARAM(lParam);
			int levX = mouseX+xCurScroll;
			int levY = mouseY+yCurScroll;
			levX = std::max(0,levX);
			levX = std::min(0xFFFF,levX);
			levY = std::max(0,levY);
			levY = std::min(0x7FFF,levY);
			if(eObj) {
				insertObjects(levX,levY);
				drawObjects();
			} else if(eSp) {
				insertSprites(levX,levY);
				drawSprites();
			}
			isRomSaved = false;
			selpCur = selpPrev = {levX,levY};
			dragFlag = true;
			selOp = 5;
			SetCursor(LoadCursorA(NULL,IDC_SIZEALL));
			//Clip cursor
			RECT clipRect;
			GetClientRect(hwnd,&clipRect);
			ClientToScreen(hwnd,(LPPOINT)&clipRect.left);
			ClientToScreen(hwnd,(LPPOINT)&clipRect.right);
			ClipCursor(&clipRect);
			
			updateEntireScreen();
			break;
		}
		case WM_LBUTTONUP:
		case WM_RBUTTONUP: {
			//Unclip cursor
			dragFlag = false;
			ClipCursor(NULL);
			
			updateEntireScreen();
			break;
		}
		default:
			return DefWindowProc(hwnd,msg,wParam,lParam);
	}
	return 0;
}

//Main entry point
int WINAPI WinMain(HINSTANCE hInstance,HINSTANCE hPrevInstance,LPSTR lpCmdLine,int nCmdShow) {
	//Load resources
	//Icon
	hiconMain = LoadIconA(hInstance,MAKEINTRESOURCE(IDI_ICON_MAIN));
	hinstMain = hInstance;
	//8x8 2BPP font
	HRSRC fontRes = FindResourceA(NULL,MAKEINTRESOURCE(IDR_FONT_CHR),RT_RCDATA);
	BYTE * fontData = (BYTE*)LockResource(LoadResource(NULL,fontRes));
	unpackGfx2BPP(fontBuffer,fontData,0x80);
	//BPS patch
	HRSRC patchRes = FindResourceA(NULL,MAKEINTRESOURCE(IDR_PATCH_BPS),RT_RCDATA);
	BYTE * patchData = (BYTE*)LockResource(LoadResource(NULL,patchRes));
	patchBufSize = SizeofResource(NULL,patchRes);
	memcpy(patchBuf,patchData,patchBufSize);
	//Manifest files
	HRSRC manifestXmlRes = FindResourceA(NULL,MAKEINTRESOURCE(IDR_MANIFEST_XML),RT_RCDATA);
	HRSRC manifestBmlRes = FindResourceA(NULL,MAKEINTRESOURCE(IDR_MANIFEST_BML),RT_RCDATA);
	BYTE * manifestXmlData = (BYTE*)LockResource(LoadResource(NULL,manifestXmlRes));
	BYTE * manifestBmlData = (BYTE*)LockResource(LoadResource(NULL,manifestBmlRes));
	manifestXmlBufSize = SizeofResource(NULL,manifestXmlRes);
	manifestBmlBufSize = SizeofResource(NULL,manifestBmlRes);
	memcpy(manifestXmlBuf,manifestXmlData,manifestXmlBufSize);
	memcpy(manifestBmlBuf,manifestBmlData,manifestBmlBufSize);
	
	//Register main window class
	WNDCLASSEXA wc;
	memset(&wc,0,sizeof(WNDCLASSEXA));
	wc.cbSize			= sizeof(WNDCLASSEXA);
	wc.lpfnWndProc		= WndProc;
	wc.hInstance		= hInstance;
	wc.hCursor			= LoadCursorA(NULL,IDC_ARROW);
	wc.hbrBackground	= (HBRUSH)(COLOR_BTNFACE+1);
	wc.lpszClassName	= "NewYILevelEditor";
	wc.hIcon			= hiconMain;
	wc.hIconSm			= hiconMain;
	if(!RegisterClassEx(&wc)) {
		MessageBoxA(NULL,"Main window registration failed!","Error!",MB_ICONEXCLAMATION|MB_OK);
		return 0;
	}
	//Register children window classes
	WNDCLASSEXA wcObj,wcSp,wcMap8,wcMap16,wcPal,wcBg;
	memset(&wcObj,0,sizeof(WNDCLASSEXA));
	wcObj.cbSize			= sizeof(WNDCLASSEXA);
	wcObj.lpfnWndProc		= WndProc_Object;
	wcObj.hInstance			= hInstance;
	wcObj.hCursor			= LoadCursorA(NULL,IDC_ARROW);
	wcObj.hbrBackground		= (HBRUSH)(COLOR_BTNFACE+1);
	wcObj.lpszClassName		= "NewYILevelEditor_Object";
	wcObj.hIcon				= hiconMain;
	wcObj.hIconSm			= hiconMain;
	memset(&wcSp,0,sizeof(WNDCLASSEXA));
	wcSp.cbSize				= sizeof(WNDCLASSEXA);
	wcSp.lpfnWndProc		= WndProc_Sprite;
	wcSp.hInstance			= hInstance;
	wcSp.hCursor			= LoadCursorA(NULL,IDC_ARROW);
	wcSp.hbrBackground		= (HBRUSH)(COLOR_BTNFACE+1);
	wcSp.lpszClassName		= "NewYILevelEditor_Sprite";
	wcSp.hIcon				= hiconMain;
	wcSp.hIconSm			= hiconMain;
	memset(&wcMap8,0,sizeof(WNDCLASSEXA));
	wcMap8.cbSize			= sizeof(WNDCLASSEXA);
	wcMap8.lpfnWndProc		= WndProc_Map8;
	wcMap8.hInstance		= hInstance;
	wcMap8.hCursor			= LoadCursorA(NULL,IDC_ARROW);
	wcMap8.hbrBackground	= (HBRUSH)(COLOR_BTNFACE+1);
	wcMap8.lpszClassName	= "NewYILevelEditor_Map8";
	wcMap8.hIcon			= hiconMain;
	wcMap8.hIconSm			= hiconMain;
	memset(&wcMap16,0,sizeof(WNDCLASSEXA));
	wcMap16.cbSize			= sizeof(WNDCLASSEXA);
	wcMap16.lpfnWndProc		= WndProc_Map16;
	wcMap16.hInstance		= hInstance;
	wcMap16.hCursor			= LoadCursorA(NULL,IDC_ARROW);
	wcMap16.hbrBackground	= (HBRUSH)(COLOR_BTNFACE+1);
	wcMap16.lpszClassName	= "NewYILevelEditor_Map16";
	wcMap16.hIcon			= hiconMain;
	wcMap16.hIconSm			= hiconMain;
	memset(&wcPal,0,sizeof(WNDCLASSEXA));
	wcPal.cbSize			= sizeof(WNDCLASSEXA);
	wcPal.lpfnWndProc		= WndProc_Palette;
	wcPal.hInstance			= hInstance;
	wcPal.hCursor			= LoadCursorA(NULL,IDC_ARROW);
	wcPal.hbrBackground		= (HBRUSH)(COLOR_BTNFACE+1);
	wcPal.lpszClassName		= "NewYILevelEditor_Palette";
	wcPal.hIcon				= hiconMain;
	wcPal.hIconSm			= hiconMain;
	memset(&wcBg,0,sizeof(WNDCLASSEXA));
	wcBg.cbSize				= sizeof(WNDCLASSEXA);
	wcBg.lpfnWndProc		= WndProc_Background;
	wcBg.hInstance			= hInstance;
	wcBg.hCursor			= LoadCursorA(NULL,IDC_ARROW);
	wcBg.hbrBackground		= (HBRUSH)(COLOR_BTNFACE+1);
	wcBg.lpszClassName		= "NewYILevelEditor_Background";
	wcBg.hIcon				= hiconMain;
	wcBg.hIconSm			= hiconMain;
	if(!RegisterClassEx(&wcObj) || !RegisterClassEx(&wcSp) || !RegisterClassEx(&wcMap8) || !RegisterClassEx(&wcMap16) || !RegisterClassEx(&wcPal) || !RegisterClassEx(&wcBg)) {
		MessageBoxA(NULL,"Children windows registration failed!","Error!",MB_ICONEXCLAMATION|MB_OK);
		return 0;
	}
	
	//Create main window
	hwndMain = CreateWindowExA(WS_EX_CLIENTEDGE,"NewYILevelEditor","EggFlutter",WS_VISIBLE|WS_OVERLAPPEDWINDOW|WS_HSCROLL|WS_VSCROLL,
		CW_USEDEFAULT,
		CW_USEDEFAULT,
		640,
		480,
		NULL,NULL,hInstance,NULL);
	if(hwndMain==NULL) {
		MessageBoxA(NULL,"Main window creation failed!","Error!",MB_ICONEXCLAMATION|MB_OK);
		return 0;
	}
	SetTimer(hwndMain,800,33,NULL);
	//Create children windows
	RECT refSize = {0,0,256,384};
	AdjustWindowRectEx(&refSize,WS_POPUPWINDOW|WS_CAPTION,false,WS_EX_CLIENTEDGE);
	int refWidth = refSize.right-refSize.left;
	int refHeight = refSize.bottom-refSize.top;
	hwndObject = CreateWindowExA(WS_EX_CLIENTEDGE,"NewYILevelEditor_Object","Select Object",WS_POPUPWINDOW|WS_CAPTION,
		CW_USEDEFAULT,
		CW_USEDEFAULT,
		refWidth,
		refHeight,
		hwndMain,NULL,hInstance,NULL);
	hwndSprite = CreateWindowExA(WS_EX_CLIENTEDGE,"NewYILevelEditor_Sprite","Select Sprite",WS_POPUPWINDOW|WS_CAPTION,
		CW_USEDEFAULT,
		CW_USEDEFAULT,
		refWidth,
		refHeight,
		hwndMain,NULL,hInstance,NULL);
	refSize = {0,0,256,256};
	AdjustWindowRectEx(&refSize,WS_POPUPWINDOW|WS_CAPTION,false,WS_EX_CLIENTEDGE);
	refWidth = refSize.right-refSize.left;
	refHeight = refSize.bottom-refSize.top;
	hwndMap8 = CreateWindowExA(WS_EX_CLIENTEDGE,"NewYILevelEditor_Map8","View 8x8 Tiles",WS_POPUPWINDOW|WS_CAPTION,
		CW_USEDEFAULT,
		CW_USEDEFAULT,
		refWidth,
		refHeight,
		hwndMain,NULL,hInstance,NULL);
	hwndPalette = CreateWindowExA(WS_EX_CLIENTEDGE,"NewYILevelEditor_Palette","View Palette",WS_POPUPWINDOW|WS_CAPTION,
		CW_USEDEFAULT,
		CW_USEDEFAULT,
		refWidth,
		refHeight,
		hwndMain,NULL,hInstance,NULL);
	refSize = {0,0,512,512};
	AdjustWindowRectEx(&refSize,WS_POPUPWINDOW|WS_CAPTION,false,WS_EX_CLIENTEDGE);
	refWidth = refSize.right-refSize.left;
	refHeight = refSize.bottom-refSize.top;
	hwndMap16 = CreateWindowExA(WS_EX_CLIENTEDGE,"NewYILevelEditor_Map16","View 16x16 Tiles",WS_POPUPWINDOW|WS_CAPTION,
		CW_USEDEFAULT,
		CW_USEDEFAULT,
		refWidth,
		refHeight,
		hwndMain,NULL,hInstance,NULL);
	refSize = {0,0,1024,1024};
	AdjustWindowRectEx(&refSize,WS_POPUPWINDOW|WS_CAPTION,false,WS_EX_CLIENTEDGE);
	refWidth = refSize.right-refSize.left;
	refHeight = refSize.bottom-refSize.top;
	hwndBackground = CreateWindowExA(WS_EX_CLIENTEDGE,"NewYILevelEditor_Background","View Background",WS_POPUPWINDOW|WS_CAPTION,
		CW_USEDEFAULT,
		CW_USEDEFAULT,
		refWidth,
		refHeight,
		hwndMain,NULL,hInstance,NULL);
	if(hwndObject==NULL || hwndSprite==NULL || hwndMap8==NULL || hwndMap16==NULL || hwndPalette==NULL || hwndBackground==NULL) {
		MessageBoxA(NULL,"Children windows creation failed!","Error!",MB_ICONEXCLAMATION|MB_OK);
		return 0;
	}
	
	//Setup menus
	hmenuMain = LoadMenuA(hInstance,MAKEINTRESOURCE(IDM_MENU_MAIN));
	SetMenu(hwndMain,hmenuMain);
	//Setup accelerators
	HACCEL haccel = LoadAcceleratorsA(hInstance,MAKEINTRESOURCE(IDA_MENU_MAIN));
	//Message loop
	MSG msg;
	while(GetMessage(&msg,NULL,0,0)>0) {
		if(!TranslateAcceleratorA(hwndMain,haccel,&msg)) {
			TranslateMessage(&msg);
			DispatchMessage(&msg);
		}
	}
	
	//Return
	return msg.wParam;
}

