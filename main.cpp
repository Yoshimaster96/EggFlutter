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
	loadBackground2();
	loadBackground3();
}
void saveLevel() {
	//Determine level size
	BYTE tempBufObj[0x8000],tempBufSp[0x8000];
	memcpy(tempBufObj,levelHeader,10);
	int objectSize = saveObjects(&tempBufObj[10]);
	tempBufObj[10+objectSize] = 0xFF;
	objectSize += 11;
	for(int i=0; i<0x200; i+=4) {
		if(screenExits[i]!=0xFF) {
			tempBufObj[objectSize++] = i>>2;
			for(int j=0; j<4; j++) {
				tempBufObj[objectSize++] = screenExits[i++];
			}
			objectSize += 5;
		}
	}
	tempBufObj[objectSize++] = 0xFF;
	int spriteSize = saveSprites(tempBufSp);
	tempBufSp[spriteSize++] = 0xFF;
	tempBufSp[spriteSize++] = 0xFF;
	//Find an area to write level data to
	//TODO
	//Save level data
	//TODO
}

///////////
//DIALOGS//
///////////
TCHAR strBuf_dlg[256];

//Helper functions for reading/writing hex values
DWORD getHexVal_dlg(HWND hwnd,int control) {
	GetDlgItemText(hwnd,control,strBuf_dlg,256);
	DWORD ret;
	_sntscanf(strBuf_dlg,sizeof(strBuf_dlg),"%X",&ret);
	return ret;
}
void setHexVal_dlg(HWND hwnd,int control,DWORD val) {
	_sntprintf(strBuf_dlg,sizeof(strBuf_dlg),"%X",val);
	SetDlgItemText(hwnd,control,strBuf_dlg);
}

//Dialog updaters
void updateDialog_editEntrances(HWND hwnd) {
	HWND hwndEditEntCb = GetDlgItem(hwnd,20);
	int tlevIdx = SendMessage(hwndEditEntCb,CB_GETCURSEL,0,0);
	setHexVal_dlg(hwnd,21,romBuf[0x0BF471+(tlevIdx<<2)]);
	setHexVal_dlg(hwnd,23,romBuf[0x0BF472+(tlevIdx<<2)]);
	setHexVal_dlg(hwnd,24,romBuf[0x0BF473+(tlevIdx<<2)]);
}
void updateDialog_editEntrances2(HWND hwnd) {
	//TODO
}
void updateDialog_editExits(HWND hwnd) {
	//TODO
}
void updateDialog_editHeader(HWND hwnd) {
	//TODO
}
void updateDialog_editLevNames(HWND hwnd) {
	//TODO
}
void updateDialog_editLevMessages(HWND hwnd) {
	//TODO
}

//Dialog functions
LRESULT CALLBACK DlgProc_dOpenLevelId(HWND hwnd,UINT msg,WPARAM wParam,LPARAM lParam) {
	switch(msg) {
		case WM_INITDIALOG: {
			//Add icon
			SendMessage(hwnd,WM_SETICON,ICON_SMALL,(LPARAM)hiconMain);
			//Limit to 2 characters
			HWND hwndOpenByIdEt = GetDlgItem(hwnd,20);
			SendMessage(hwndOpenByIdEt,EM_SETLIMITTEXT,2,0);
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
					SendMessage(hwnd,WM_CLOSE,0,0);
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
						SendMessage(hwnd,WM_CLOSE,0,0);
					}
					break;
				}
			}
			break;
		}
	}
	return 0;
}
const TCHAR * worldStrings[56] = {
"1-1","1-2","1-3","1-4","1-5","1-6","1-7","1-8","1-E",
"2-1","2-2","2-3","2-4","2-5","2-6","2-7","2-8","2-E",
"3-1","3-2","3-3","3-4","3-5","3-6","3-7","3-8","3-E",
"4-1","4-2","4-3","4-4","4-5","4-6","4-7","4-8","4-E",
"5-1","5-2","5-3","5-4","5-5","5-6","5-7","5-8","5-E",
"6-1","6-2","6-3","6-4","6-5","6-6","6-7","6-8","6-E",
"Intro","Tutorial"};
LRESULT CALLBACK DlgProc_dEditEntrances(HWND hwnd,UINT msg,WPARAM wParam,LPARAM lParam) {
	switch(msg) {
		case WM_INITDIALOG: {
			//Add icon
			SendMessage(hwnd,WM_SETICON,ICON_SMALL,(LPARAM)hiconMain);
			//Init combo boxes
			HWND hwndEditEntCb = GetDlgItem(hwnd,20);
			for(int i=0; i<56; i++) {
				SendMessage(hwndEditEntCb,CB_ADDSTRING,0,(LPARAM)worldStrings[i]);
			}
			SendMessage(hwndEditEntCb,CB_SETCURSEL,0,0);
			//Limit to 2 characters
			HWND hwndEditEntEt = GetDlgItem(hwnd,21);
			SendMessage(hwndEditEntEt,EM_SETLIMITTEXT,2,0);
			hwndEditEntEt = GetDlgItem(hwnd,23);
			SendMessage(hwndEditEntEt,EM_SETLIMITTEXT,2,0);
			hwndEditEntEt = GetDlgItem(hwnd,24);
			SendMessage(hwndEditEntEt,EM_SETLIMITTEXT,2,0);
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
					SendMessage(hwnd,WM_CLOSE,0,0);
					break;
				}
				case IDOK: {
					HWND hwndEditEntCb = GetDlgItem(hwnd,20);
					int tlevIdx = SendMessage(hwndEditEntCb,CB_GETCURSEL,0,0);
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
						SendMessage(hwnd,WM_CLOSE,0,0);
					}
					break;
				}
			}
			break;
		}
	}
	return 0;
}
LRESULT CALLBACK DlgProc_dEditEntrances2(HWND hwnd,UINT msg,WPARAM wParam,LPARAM lParam) {
	switch(msg) {
		case WM_INITDIALOG: {
			//Add icon
			SendMessage(hwnd,WM_SETICON,ICON_SMALL,(LPARAM)hiconMain);
			//Limit to 2 characters
			HWND hwndEditEntEt2 = GetDlgItem(hwnd,23);
			SendMessage(hwndEditEntEt2,EM_SETLIMITTEXT,2,0);
			hwndEditEntEt2 = GetDlgItem(hwnd,24);
			SendMessage(hwndEditEntEt2,EM_SETLIMITTEXT,2,0);
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
					SendMessage(hwnd,WM_CLOSE,0,0);
					break;
				}
				case IDOK: {
					//TODO
					break;
				}
			}
			break;
		}
	}
	return 0;
}
LRESULT CALLBACK DlgProc_dEditExits(HWND hwnd,UINT msg,WPARAM wParam,LPARAM lParam) {
	switch(msg) {
		case WM_INITDIALOG: {
			//Add icon
			SendMessage(hwnd,WM_SETICON,ICON_SMALL,(LPARAM)hiconMain);
			//TODO
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
					SendMessage(hwnd,WM_CLOSE,0,0);
					break;
				}
				case IDOK: {
					//TODO
					break;
				}
			}
			break;
		}
	}
	return 0;
}
LRESULT CALLBACK DlgProc_dEditHeader(HWND hwnd,UINT msg,WPARAM wParam,LPARAM lParam) {
	switch(msg) {
		case WM_INITDIALOG: {
			//Add icon
			SendMessage(hwnd,WM_SETICON,ICON_SMALL,(LPARAM)hiconMain);
			//TODO
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
					SendMessage(hwnd,WM_CLOSE,0,0);
					break;
				}
				case IDOK: {
					//TODO
					break;
				}
			}
			break;
		}
	}
	return 0;
}
LRESULT CALLBACK DlgProc_dEditLevNames(HWND hwnd,UINT msg,WPARAM wParam,LPARAM lParam) {
	switch(msg) {
		case WM_INITDIALOG: {
			//Add icon
			SendMessage(hwnd,WM_SETICON,ICON_SMALL,(LPARAM)hiconMain);
			//TODO
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
					SendMessage(hwnd,WM_CLOSE,0,0);
					break;
				}
				case IDOK: {
					//TODO
					break;
				}
			}
			break;
		}
	}
	return 0;
}
LRESULT CALLBACK DlgProc_dEditLevMessages(HWND hwnd,UINT msg,WPARAM wParam,LPARAM lParam) {
	switch(msg) {
		case WM_INITDIALOG: {
			//Add icon
			SendMessage(hwnd,WM_SETICON,ICON_SMALL,(LPARAM)hiconMain);
			//TODO
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
					SendMessage(hwnd,WM_CLOSE,0,0);
					break;
				}
				case IDOK: {
					//TODO
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
TCHAR strBuf_main[256],strBuf2_main[256];
HWND hwndMain;
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
	return (MessageBox(hwndMain,msg,title,MB_ICONWARNING|MB_YESNO) == IDYES);
}
inline BOOL promptSave() {
	return (MessageBox(hwndMain,"Level data has been modified. Are you sure?","Unsaved changes!",MB_ICONWARNING|MB_YESNO) == IDYES);
}
inline void updateEntireScreen() {
	RECT rect = {xCurScroll,yCurScroll,xCurScroll+xCurSize,yCurScroll+yCurSize};
	updateRect(rect);
	GetWindowRect(hwndMain,&rect);
	InvalidateRect(hwndMain,&rect,false);
}
inline void updateDialogs() {
	RECT rect = {0,0,256,256};
	if(wvisObject) {
		InvalidateRect(hwndObject,&rect,false);
		UpdateWindow(hwndObject);
	}
	if(wvisSprite) {
		InvalidateRect(hwndSprite,&rect,false);
		UpdateWindow(hwndSprite);
	}
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
//File
void onOpen() {
	//Prompt save
	if(isRomOpen && !isRomSaved) {
		if(!promptSave()) return;
	}
	OPENFILENAME ofn;
	memset(&ofn,0,sizeof(OPENFILENAME));
	memset(strBuf_main,0,sizeof(strBuf_main));
	ofn.lStructSize	 = sizeof(OPENFILENAME);
	ofn.hwndOwner	 = hwndMain;
	ofn.lpstrFile	 = strBuf_main;
	ofn.nMaxFile	 = sizeof(strBuf_main);
	ofn.lpstrTitle	 = "Open ROM";
	ofn.lpstrFilter	 = "SNES ROM Image (*.smc,*.sfc)\0*.smc;*.sfc\0";
	ofn.Flags		 = OFN_PATHMUSTEXIST|OFN_FILEMUSTEXIST;
	if(GetOpenFileName(&ofn)) {
		hasSmcHeader = (strBuf_main[ofn.nFileExtension+1] == 'm');
		//Load ROM
		memset(romBuf,0,0x800000);
		FILE * fp = _tfopen(strBuf_main,"rb");
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
		//Save ROM
		FILE * fp = _tfopen(strBuf_main,"wb");
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
		OPENFILENAME ofn;
		memset(&ofn,0,sizeof(OPENFILENAME));
		memset(strBuf_main,0,sizeof(strBuf_main));
		ofn.lStructSize	 = sizeof(OPENFILENAME);
		ofn.hwndOwner	 = hwndMain;
		ofn.lpstrFile	 = strBuf_main;
		ofn.nMaxFile	 = sizeof(strBuf_main);
		ofn.lpstrTitle	 = "Save ROM";
		ofn.lpstrFilter	 = "SNES ROM Image (*.smc,*.sfc)\0*.smc;*.sfc\0";
		if(GetSaveFileName(&ofn)) {
			hasSmcHeader = (strBuf_main[ofn.nFileExtension+1] == 'm');
			//Save level
			saveLevel();
			//Save ROM
			FILE * fp = _tfopen(strBuf_main,"wb");
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
	SendMessage(hwndMain,WM_CLOSE,0,0);
}
void onImportLevel() {
	//Prompt save
	if(isRomOpen) {
		if(!isRomSaved) {
			if(!promptSave()) return;
		}
		OPENFILENAME ofn;
		memset(&ofn,0,sizeof(OPENFILENAME));
		memset(strBuf2_main,0,sizeof(strBuf2_main));
		ofn.lStructSize	 = sizeof(OPENFILENAME);
		ofn.hwndOwner	 = hwndMain;
		ofn.lpstrFile	 = strBuf2_main;
		ofn.nMaxFile	 = sizeof(strBuf2_main);
		ofn.lpstrTitle	 = "Open Level File";
		ofn.lpstrFilter	 = "YI Level File (*.ylv)\0*.ylv;\0";
		ofn.Flags		 = OFN_PATHMUSTEXIST|OFN_FILEMUSTEXIST;
		if(GetOpenFileName(&ofn)) {
			//TODO
		}
	}
}
void onExportLevel() {
	if(isRomOpen) {
		OPENFILENAME ofn;
		memset(&ofn,0,sizeof(OPENFILENAME));
		memset(strBuf2_main,0,sizeof(strBuf2_main));
		ofn.lStructSize	 = sizeof(OPENFILENAME);
		ofn.hwndOwner	 = hwndMain;
		ofn.lpstrFile	 = strBuf2_main;
		ofn.nMaxFile	 = sizeof(strBuf2_main);
		ofn.lpstrTitle	 = "Save Level File";
		ofn.lpstrFilter	 = "YI Level File (*.ylv)\0*.ylv;\0";
		if(GetSaveFileName(&ofn)) {
			//TODO
		}
	}
}
void onOpenLevel() {
	//Prompt save
	if(isRomOpen) {
		if(!isRomSaved) {
			if(!promptSave()) return;
		}
		if(DialogBox(NULL,MAKEINTRESOURCE(IDD_OPEN_LEVEL_ID),hwndMain,DlgProc_dOpenLevelId)) {
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
		if(curLevel!=0xDD) {
			curLevel++;
			loadLevel();
			updateDialogs();
			updateEntireScreen();
		}
	}
}
void onPrevLevel() {
	//Prompt save
	if(isRomOpen) {
		if(!isRomSaved) {
			if(!promptSave()) return;
		}
		if(curLevel) {
			curLevel--;
			loadLevel();
			updateDialogs();
			updateEntireScreen();
		}
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
	//TODO
}
void onDecZ() {
	decreaseObjectZ();
	//TODO
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
	updateDialogs();
	updateEntireScreen();
}
void onViewSwA() {
	vSwA = !vSwA;
	updateMenu();
	updateMap8Sw((vSwA?0x08:0)|(vSwB?0x10:0));
	updateDialogs();
	updateEntireScreen();
}
void onViewSwB() {
	vSwB = !vSwB;
	updateMenu();
	updateMap8Sw((vSwA?0x08:0)|(vSwB?0x10:0));
	updateDialogs();
	updateEntireScreen();
}
//Tools
void onChgEnt() {
	if(DialogBox(NULL,MAKEINTRESOURCE(IDD_EDIT_ENTRANCE),hwndMain,DlgProc_dEditEntrances) && vEnt) {
		updateEntireScreen();
	}
}
void onChgEnt2() {
	if(DialogBox(NULL,MAKEINTRESOURCE(IDD_EDIT_ENTRANCE2),hwndMain,DlgProc_dEditEntrances2) && vEnt) {
		updateEntireScreen();
	}
}
void onChgExit() {
	if(DialogBox(NULL,MAKEINTRESOURCE(IDD_EDIT_EXIT),hwndMain,DlgProc_dEditExits) && vExit) {
		updateEntireScreen();
	}
}
void onChgHead() {
	if(DialogBox(NULL,MAKEINTRESOURCE(IDD_EDIT_HEADER),hwndMain,DlgProc_dEditHeader)) {
		updateDialogs();
		updateEntireScreen();
	}
}
void onChgLevName() {
	DialogBox(NULL,MAKEINTRESOURCE(IDD_EDIT_LEV_NAME),hwndMain,DlgProc_dEditLevNames);
}
void onChgLevMsg() {
	DialogBox(NULL,MAKEINTRESOURCE(IDD_EDIT_LEV_MSG),hwndMain,DlgProc_dEditLevMessages);
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
int selOp = 0;
RECT selRect = {0,0,0,0};

//Extra UI drawing stuff
void dispEntrances(RECT rect) {
	//TODO
}
void dispExits(RECT rect) {
	int minx = rect.left&0xFF00;
	int miny = rect.top&0x7F00;
	int maxx = (rect.right&0xFF00)+0x100;
	int maxy = (rect.bottom&0x7F00)+0x100;
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
			CHAR strBuf[256];
			if(screenExits[screen<<2]!=0xFF) {
				for(int l=0x02; l<0xFE; l++) {
					for(int k=0x02; k<0xFE; k++) {
						hilitePixel(bmpDataMain,0x1000,0x800,0x80,{i+k,j+l});
					}
				}
				snprintf(strBuf,256,"%02X: Exit to level %02X",screen,screenExits[screen<<2]);
			} else {
				snprintf(strBuf,256,"%02X",screen);
			}
			for(int n=0; n<strlen(strBuf); n++) {
				dispMap8Char(bmpDataMain,0x1000,0x800,0xFFFFFF,0xFF,strBuf[n],{i+(n<<3)+2,j+2});
			}
		}
	}
}
void dispGrid(RECT rect) {
	int minx = rect.left&0xFFF0;
	int miny = rect.top&0x7FF0;
	int maxx = (rect.right&0xFFF0)+0x10;
	int maxy = (rect.bottom&0x7FF0)+0x10;
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
				case SB_LEFT: {
					yNewScroll = 0;
					break;
				}
				case SB_RIGHT: {
					yNewScroll = yMaxScroll;
					break;
				}
				case SB_PAGELEFT: {
					yNewScroll -= 0x100;
					break;
				}
				case SB_PAGERIGHT: {
					yNewScroll += 0x100;
					break;
				}
				case SB_LINELEFT: {
					yNewScroll -= 0x10;
					break;
				}
				case SB_LINERIGHT: {
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
		//Mouse input
		case WM_MOUSEMOVE: {
			//TODO
			break;
		}
		case WM_LBUTTONDOWN: {
			//TODO
			break;
		}
		case WM_RBUTTONDOWN: {
			//TODO
			break;
		}
		case WM_LBUTTONUP:
		case WM_RBUTTONUP: {
			//TODO
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
	hiconMain = LoadIcon(hInstance,MAKEINTRESOURCE(IDI_ICON_MAIN));
	BYTE * fontData = (BYTE*)LockResource(LoadResource(NULL,FindResource(NULL,MAKEINTRESOURCE(IDR_FONT_CHR),RT_RCDATA)));
	unpackGfx2BPP(fontBuffer,fontData,0x80);
	
	//Register main window class
	WNDCLASSEX wc;
	memset(&wc,0,sizeof(WNDCLASSEX));
	wc.cbSize			= sizeof(WNDCLASSEX);
	wc.lpfnWndProc		= WndProc;
	wc.hInstance		= hInstance;
	wc.hCursor			= LoadCursor(NULL,IDC_ARROW);
	wc.hbrBackground	= (HBRUSH)(COLOR_BTNFACE+1);
	wc.lpszClassName	= "NewYILevelEditor";
	wc.hIcon			= hiconMain;
	wc.hIconSm			= hiconMain;
	if(!RegisterClassEx(&wc)) {
		MessageBox(NULL,"Main window registration failed!","Error!",MB_ICONEXCLAMATION|MB_OK);
		return 0;
	}
	//Register children window classes
	WNDCLASSEX wcObj,wcSp,wcMap8,wcMap16,wcPal,wcBg;
	memset(&wcObj,0,sizeof(WNDCLASSEX));
	wcObj.cbSize			= sizeof(WNDCLASSEX);
	wcObj.lpfnWndProc		= WndProc_Object;
	wcObj.hInstance			= hInstance;
	wcObj.hCursor			= LoadCursor(NULL,IDC_ARROW);
	wcObj.hbrBackground		= (HBRUSH)(COLOR_BTNFACE+1);
	wcObj.lpszClassName		= "NewYILevelEditor_Object";
	wcObj.hIcon				= hiconMain;
	wcObj.hIconSm			= hiconMain;
	memset(&wcSp,0,sizeof(WNDCLASSEX));
	wcSp.cbSize				= sizeof(WNDCLASSEX);
	wcSp.lpfnWndProc		= WndProc_Sprite;
	wcSp.hInstance			= hInstance;
	wcSp.hCursor			= LoadCursor(NULL,IDC_ARROW);
	wcSp.hbrBackground		= (HBRUSH)(COLOR_BTNFACE+1);
	wcSp.lpszClassName		= "NewYILevelEditor_Sprite";
	wcSp.hIcon				= hiconMain;
	wcSp.hIconSm			= hiconMain;
	memset(&wcMap8,0,sizeof(WNDCLASSEX));
	wcMap8.cbSize			= sizeof(WNDCLASSEX);
	wcMap8.lpfnWndProc		= WndProc_Map8;
	wcMap8.hInstance		= hInstance;
	wcMap8.hCursor			= LoadCursor(NULL,IDC_ARROW);
	wcMap8.hbrBackground	= (HBRUSH)(COLOR_BTNFACE+1);
	wcMap8.lpszClassName	= "NewYILevelEditor_Map8";
	wcMap8.hIcon			= hiconMain;
	wcMap8.hIconSm			= hiconMain;
	memset(&wcMap16,0,sizeof(WNDCLASSEX));
	wcMap16.cbSize			= sizeof(WNDCLASSEX);
	wcMap16.lpfnWndProc		= WndProc_Map16;
	wcMap16.hInstance		= hInstance;
	wcMap16.hCursor			= LoadCursor(NULL,IDC_ARROW);
	wcMap16.hbrBackground	= (HBRUSH)(COLOR_BTNFACE+1);
	wcMap16.lpszClassName	= "NewYILevelEditor_Map16";
	wcMap16.hIcon			= hiconMain;
	wcMap16.hIconSm			= hiconMain;
	memset(&wcPal,0,sizeof(WNDCLASSEX));
	wcPal.cbSize			= sizeof(WNDCLASSEX);
	wcPal.lpfnWndProc		= WndProc_Palette;
	wcPal.hInstance			= hInstance;
	wcPal.hCursor			= LoadCursor(NULL,IDC_ARROW);
	wcPal.hbrBackground		= (HBRUSH)(COLOR_BTNFACE+1);
	wcPal.lpszClassName		= "NewYILevelEditor_Palette";
	wcPal.hIcon				= hiconMain;
	wcPal.hIconSm			= hiconMain;
	memset(&wcBg,0,sizeof(WNDCLASSEX));
	wcBg.cbSize				= sizeof(WNDCLASSEX);
	wcBg.lpfnWndProc		= WndProc_Background;
	wcBg.hInstance			= hInstance;
	wcBg.hCursor			= LoadCursor(NULL,IDC_ARROW);
	wcBg.hbrBackground		= (HBRUSH)(COLOR_BTNFACE+1);
	wcBg.lpszClassName		= "NewYILevelEditor_Background";
	wcBg.hIcon				= hiconMain;
	wcBg.hIconSm			= hiconMain;
	if(!RegisterClassEx(&wcObj) || !RegisterClassEx(&wcSp) || !RegisterClassEx(&wcMap8) || !RegisterClassEx(&wcMap16) || !RegisterClassEx(&wcPal) || !RegisterClassEx(&wcBg)) {
		MessageBox(NULL,"Children windows registration failed!","Error!",MB_ICONEXCLAMATION|MB_OK);
		return 0;
	}
	
	//Create main window
	hwndMain = CreateWindowEx(WS_EX_CLIENTEDGE,"NewYILevelEditor","EggFlutter",WS_VISIBLE|WS_OVERLAPPEDWINDOW|WS_HSCROLL|WS_VSCROLL,
		CW_USEDEFAULT,
		CW_USEDEFAULT,
		640,
		480,
		NULL,NULL,hInstance,NULL);
	if(hwndMain==NULL) {
		MessageBox(NULL,"Main window creation failed!","Error!",MB_ICONEXCLAMATION|MB_OK);
		return 0;
	}
	SetTimer(hwndMain,800,33,NULL);
	//Create children windows
	RECT refSize = {0,0,256,256};
	AdjustWindowRectEx(&refSize,WS_POPUPWINDOW|WS_CAPTION,false,WS_EX_CLIENTEDGE);
	int refWidth = refSize.right-refSize.left;
	int refHeight = refSize.bottom-refSize.top;
	hwndObject = CreateWindowEx(WS_EX_CLIENTEDGE,"NewYILevelEditor_Object","Select Object",WS_POPUPWINDOW|WS_CAPTION,
		CW_USEDEFAULT,
		CW_USEDEFAULT,
		refWidth,
		refHeight,
		hwndMain,NULL,hInstance,NULL);
	hwndSprite = CreateWindowEx(WS_EX_CLIENTEDGE,"NewYILevelEditor_Sprite","Select Sprite",WS_POPUPWINDOW|WS_CAPTION,
		CW_USEDEFAULT,
		CW_USEDEFAULT,
		refWidth,
		refHeight,
		hwndMain,NULL,hInstance,NULL);
	hwndMap8 = CreateWindowEx(WS_EX_CLIENTEDGE,"NewYILevelEditor_Map8","View 8x8 Tiles",WS_POPUPWINDOW|WS_CAPTION,
		CW_USEDEFAULT,
		CW_USEDEFAULT,
		refWidth,
		refHeight,
		hwndMain,NULL,hInstance,NULL);
	hwndPalette = CreateWindowEx(WS_EX_CLIENTEDGE,"NewYILevelEditor_Palette","View Palette",WS_POPUPWINDOW|WS_CAPTION,
		CW_USEDEFAULT,
		CW_USEDEFAULT,
		refWidth,
		refHeight,
		hwndMain,NULL,hInstance,NULL);
	refSize = {0,0,512,512};
	AdjustWindowRectEx(&refSize,WS_POPUPWINDOW|WS_CAPTION,false,WS_EX_CLIENTEDGE);
	refWidth = refSize.right-refSize.left;
	refHeight = refSize.bottom-refSize.top;
	hwndMap16 = CreateWindowEx(WS_EX_CLIENTEDGE,"NewYILevelEditor_Map16","View 16x16 Tiles",WS_POPUPWINDOW|WS_CAPTION,
		CW_USEDEFAULT,
		CW_USEDEFAULT,
		refWidth,
		refHeight,
		hwndMain,NULL,hInstance,NULL);
	refSize = {0,0,1024,1024};
	AdjustWindowRectEx(&refSize,WS_POPUPWINDOW|WS_CAPTION,false,WS_EX_CLIENTEDGE);
	refWidth = refSize.right-refSize.left;
	refHeight = refSize.bottom-refSize.top;
	hwndBackground = CreateWindowEx(WS_EX_CLIENTEDGE,"NewYILevelEditor_Background","View Background",WS_POPUPWINDOW|WS_CAPTION,
		CW_USEDEFAULT,
		CW_USEDEFAULT,
		refWidth,
		refHeight,
		hwndMain,NULL,hInstance,NULL);
	if(hwndObject==NULL || hwndSprite==NULL || hwndMap8==NULL || hwndMap16==NULL || hwndPalette==NULL || hwndBackground==NULL) {
		MessageBox(NULL,"Children windows creation failed!","Error!",MB_ICONEXCLAMATION|MB_OK);
		return 0;
	}
	
	//Setup menus
	hmenuMain = LoadMenu(hInstance,MAKEINTRESOURCE(IDM_MENU_MAIN));
	SetMenu(hwndMain,hmenuMain);
	//Setup accelerators
	HACCEL haccel = LoadAccelerators(hInstance,MAKEINTRESOURCE(IDA_MENU_MAIN));
	//Message loop
	MSG msg;
	while(GetMessage(&msg,NULL,0,0)>0) {
		if(!TranslateAccelerator(hwndMain,haccel,&msg)) {
			TranslateMessage(&msg);
			DispatchMessage(&msg);
		}
	}
	
	//Return
	return msg.wParam;
}

