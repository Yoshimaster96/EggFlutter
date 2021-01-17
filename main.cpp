#include "main.h"

/////////////////
//LEVEL LOADING//
/////////////////
void loadLevel() {
	//TODO
}
void saveLevel() {
	//TODO
}

///////////
//DIALOGS//
///////////
//Helper functions for reading/writing hex values
TCHAR strBuf_dlg[256];
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

//Dialog functions
LRESULT CALLBACK DlgProc_dOpenLevelId(HWND hwnd,UINT msg,WPARAM wParam,LPARAM lParam) {
	switch(msg) {
		case WM_INITDIALOG: {
			//Add icon
			SendMessage(hwnd,WM_SETICON,ICON_SMALL,(LPARAM)hiconMain);
			//Limit to 2 characters
			HWND hwndOpenByIdEt = GetDlgItem(hwnd,20);
			SendMessage(hwndOpenByIdEt,EM_SETLIMITTEXT,2,0);
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
					if(lev>=0x00 && lev<=0xDD) {
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
LRESULT CALLBACK DlgProc_dEditEntrances(HWND hwnd,UINT msg,WPARAM wParam,LPARAM lParam) {
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
			//TODO
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
			//TODO
			return TRUE;
		}
		case WM_CLOSE: {
			//Exit with code 0 (do nothing)
			EndDialog(hwnd,0);
			break;
		}
		case WM_COMMAND: {
			//TODO
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
			//TODO
			break;
		}
	}
	return 0;
}
LRESULT CALLBACK DlgProc_dEditGfx(HWND hwnd,UINT msg,WPARAM wParam,LPARAM lParam) {
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
			//TODO
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
			//TODO
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
			//TODO
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
			//TODO
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
HMENU hmenuMain;
HWND hwndMain;

//View states
bool eObj = true,eSp = false;
bool vObj = true,vBg2 = true,vBg3 = true,vSp = true;
bool vEnt = true,vExit = false,vGrid = false,vAnim = false;
bool vObjHex = false,vSpHex = false;
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
	EnableMenuItem(hmenuMain,1103,enableState);
	//View
	EnableMenuItem(hmenuMain,1200,enableState);
	EnableMenuItem(hmenuMain,1201,enableState);
	EnableMenuItem(hmenuMain,1202,enableState);
	EnableMenuItem(hmenuMain,1203,enableState);
	EnableMenuItem(hmenuMain,1210,enableState);
	EnableMenuItem(hmenuMain,1211,enableState);
	EnableMenuItem(hmenuMain,1212,enableState);
	EnableMenuItem(hmenuMain,1213,enableState);
	EnableMenuItem(hmenuMain,1220,enableState);
	EnableMenuItem(hmenuMain,1223,enableState);
	EnableMenuItem(hmenuMain,1230,enableState);
	EnableMenuItem(hmenuMain,1231,enableState);
	//Tools
	EnableMenuItem(hmenuMain,1300,enableState);
	EnableMenuItem(hmenuMain,1301,enableState);
	EnableMenuItem(hmenuMain,1302,enableState);
	EnableMenuItem(hmenuMain,1310,enableState);
	EnableMenuItem(hmenuMain,1311,enableState);
	EnableMenuItem(hmenuMain,1320,enableState);
	EnableMenuItem(hmenuMain,1321,enableState);
	//Window
	EnableMenuItem(hmenuMain,1400,enableState);
	EnableMenuItem(hmenuMain,1403,enableState);
	EnableMenuItem(hmenuMain,1410,enableState);
	EnableMenuItem(hmenuMain,1411,enableState);
	EnableMenuItem(hmenuMain,1412,enableState);
	EnableMenuItem(hmenuMain,1413,enableState);
	//Update Edit Object/Sprite check state
	//and relevant enables/disables
	CheckMenuItem(hmenuMain,1100,eObj?MF_CHECKED:0);
	CheckMenuItem(hmenuMain,1103,eSp?MF_CHECKED:0);
	UINT enableObj = eObj?0:MF_GRAYED;
	UINT enableSp = eSp?0:MF_GRAYED;
	enableObj |= enableState;
	enableSp |= enableState;
	EnableMenuItem(hmenuMain,1110,enableObj);
	EnableMenuItem(hmenuMain,1111,enableObj);
	EnableMenuItem(hmenuMain,1120,enableObj);
	EnableMenuItem(hmenuMain,1121,enableSp);
	//Update checked state for View menu items
	CheckMenuItem(hmenuMain,1200,vObj?MF_CHECKED:0);
	CheckMenuItem(hmenuMain,1201,vBg2?MF_CHECKED:0);
	CheckMenuItem(hmenuMain,1202,vBg3?MF_CHECKED:0);
	CheckMenuItem(hmenuMain,1203,vSp?MF_CHECKED:0);
	CheckMenuItem(hmenuMain,1210,vEnt?MF_CHECKED:0);
	CheckMenuItem(hmenuMain,1211,vExit?MF_CHECKED:0);
	CheckMenuItem(hmenuMain,1212,vGrid?MF_CHECKED:0);
	CheckMenuItem(hmenuMain,1213,vAnim?MF_CHECKED:0);
	CheckMenuItem(hmenuMain,1220,vObjHex?MF_CHECKED:0);
	CheckMenuItem(hmenuMain,1221,vSpHex?MF_CHECKED:0);
	CheckMenuItem(hmenuMain,1230,vSwA?MF_CHECKED:0);
	CheckMenuItem(hmenuMain,1231,vSwB?MF_CHECKED:0);
}
inline BOOL prompt(LPCSTR title,LPCSTR msg) {
	return (MessageBox(hwndMain,msg,title,MB_ICONWARNING|MB_YESNO) == IDYES);
}
inline BOOL promptSave() {
	return (MessageBox(hwndMain,"Level data has been modified. Are you sure?","Unsaved changes!",MB_ICONWARNING|MB_YESNO) == IDYES);
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
		memset(romBuf,0,0x400000);
		FILE * fp = _tfopen(strBuf_main,"rb");
		fseek(fp,0,SEEK_END);
		long fileSize = ftell(fp);
		fseek(fp,(fileSize&0x200)?0x200:0,SEEK_SET);
		fread(romBuf,1,fileSize&(~0x200),fp);
		fclose(fp);
		//Check ROM
		if(checkRom()) {
			initAllocTable();
			isRomOpen = true;
			isRomSaved = true;
			updateMenu();
			//Load level
			loadLevel();
			SendMessage(hwndMain,2000,0,0);
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
		//TODO
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
		fwrite(romBuf,1,0x400000,fp);
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
			fwrite(romBuf,1,0x400000,fp);
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
			SendMessage(hwndMain,2000,0,0);
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
			SendMessage(hwndMain,2000,0,0);
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
			SendMessage(hwndMain,2000,0,0);
		}
	}
}
//Edit
void onEditObj() {
	//TODO
	eObj = true;
	eSp = false;
	updateMenu();
	//TODO
}
void onEditSp() {
	//TODO
	eSp = true;
	eObj = false;
	updateMenu();
	//TODO
}
void onIncZ() {
	//TODO
}
void onDecZ() {
	//TODO
}
void onManualObj() {
	//TODO
}
void onManualSp() {
	//TODO
}
//View
void onViewObj() {
	vObj = !vObj;
	//TODO
	updateMenu();
	//TODO
}
void onViewBg2() {
	vBg2 = !vBg2;
	updateMenu();
	//TODO
}
void onViewBg3() {
	vBg3 = !vBg3;
	updateMenu();
	//TODO
}
void onViewSp() {
	vSp = !vSp;
	//TODO
	updateMenu();
	//TODO
}
void onViewEnt() {
	vEnt = !vEnt;
	updateMenu();
	//TODO
}
void onViewExit() {
	vExit = !vExit;
	updateMenu();
	//TODO
}
void onViewGrid() {
	vGrid = !vGrid;
	updateMenu();
	//TODO
}
void onViewAnim() {
	vAnim = !vAnim;
	updateMenu();
	//TODO
}
void onViewObjHex() {
	vObjHex = !vObjHex;
	updateMenu();
	//TODO
}
void onViewSpHex() {
	vSpHex = !vSpHex;
	updateMenu();
	//TODO
}
void onViewSwA() {
	vSwA = !vSwA;
	updateMenu();
	//TODO
}
void onViewSwB() {
	vSwB = !vSwB;
	updateMenu();
	//TODO
}
//Tools
void onChgEnt() {
	if(DialogBox(NULL,MAKEINTRESOURCE(IDD_EDIT_ENTRANCE),hwndMain,DlgProc_dEditEntrances) && vEnt) {
		//TODO
	}
}
void onChgEnt2() {
	if(DialogBox(NULL,MAKEINTRESOURCE(IDD_EDIT_ENTRANCE2),hwndMain,DlgProc_dEditEntrances2) && vEnt) {
		//TODO
	}
}
void onChgExit() {
	if(DialogBox(NULL,MAKEINTRESOURCE(IDD_EDIT_EXIT),hwndMain,DlgProc_dEditExits) && vExit) {
		//TODO
	}
}
void onChgGfx() {
	if(DialogBox(NULL,MAKEINTRESOURCE(IDD_EDIT_GFX),hwndMain,DlgProc_dEditGfx)) {
		//TODO
	}
}
void onChgHead() {
	DialogBox(NULL,MAKEINTRESOURCE(IDD_EDIT_HEADER),hwndMain,DlgProc_dEditHeader);
}
void onChgLevName() {
	DialogBox(NULL,MAKEINTRESOURCE(IDD_EDIT_LEV_NAME),hwndMain,DlgProc_dEditLevNames);
}
void onChgLevMsg() {
	DialogBox(NULL,MAKEINTRESOURCE(IDD_EDIT_LEV_MSG),hwndMain,DlgProc_dEditLevMessages);
}
//Window
void onSelObj() {
	//TODO
}
void onSelSp() {
	//TODO
}
void onEditMap8() {
	//TODO
}
void onEditMap16() {
	//TODO
}
void onEditBg() {
	//TODO
}
void onEditPal() {
	//TODO
}

//Tables used for menu processing
#define NUM_COMMANDS (10+6+12+7+6)
void (*cmMenuFunc[NUM_COMMANDS])() = {
//File
	onOpen,onClose,onSave,onSaveAs,onQuit,
	onImportLevel,onExportLevel,
	onOpenLevel,onNextLevel,onPrevLevel,
//Edit
	onEditObj,onEditSp,
	onIncZ,onDecZ,
	onManualObj,onManualSp,
//View
	onViewObj,onViewBg2,onViewBg3,onViewSp,
	onViewEnt,onViewExit,onViewGrid,onViewAnim,
	onViewObjHex,onViewSpHex,
	onViewSwA,onViewSwB,
//Tools
	onChgEnt,onChgEnt2,onChgExit,
	onChgGfx,onChgHead,
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
	1100,1103,
	1110,1111,
	1120,1121,
//View
	1200,1201,1202,1203,
	1210,1211,1212,1213,
	1220,1223,
	1230,1231,
//Tools
	1300,1301,1302,
	1310,1311,
	1320,1321,
//Window
	1400,1403,
	1410,1411,1412,1413};

/////////////////////
//MAIN WINDOW STUFF//
/////////////////////
HDC				hdcMain;
HBITMAP			hbmpMain;
DWORD*			bmpDataMain;

int				xCurScroll = 0,xMaxScroll = 0,xCurSize = 640;
int 			yCurScroll = 0,yMaxScroll = 0,yCurSize = 480;

//Helper functions (for determining invalid regions)
//TODO

//Extra UI drawing stuff
void dispEntrances(RECT rect) {
	//TODO
}
void dispExits(RECT rect) {
	//Draw screen borders
	//TODO
	//Highlight screens which have exits
	//TODO
	//Draw screen exit info text
	//TODO
}
void dispGrid(RECT rect) {
	//TODO
}
void dispObjHexVals(RECT rect) {
	//TODO
}
void dispSpHexVals(RECT rect) {
	//TODO
}

//Main drawing code
void updateRect(RECT rect) {
	//Draw layer 3 (if priority below)
	//TODO
	//Draw layer 2
	//TODO
	//Draw layer 3 (if priority middle)
	//TODO
	//Draw objects
	//TODO
	if(vObjHex) {
		dispObjHexVals(rect);
	}
	//Draw sprites
	//TODO
	if(vSpHex) {
		dispSpHexVals(rect);
	}
	//Draw layer 3 (if priority above)
	//TODO
	//Draw entrances
	if(vEnt) {
		dispEntrances(rect);
	}
	//Draw exits
	if(vExit) {
		dispExits(rect);
	}
	//Draw grid
	if(vGrid) {
		dispGrid(rect);
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
			SystemParametersInfo(SPI_SETFOCUSBORDERWIDTH,0,(LPVOID)2,0);
			SystemParametersInfo(SPI_SETFOCUSBORDERHEIGHT,0,(LPVOID)2,0);
			//Have message 2000 handle this
			SendMessage(hwnd,2000,0,0);
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
			
			SendMessage(hwnd,2002,0,0);
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
			
			SendMessage(hwnd,2002,0,0);
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
			
			SendMessage(hwnd,2002,0,0);
			break;
		}
		//Menu input
		case WM_COMMAND: {
			switch(LOWORD(wParam)) {
				case 2000: {
					//TODO
					break;
				}
				case 2001: {
					//TODO
					break;
				}
				case 2002: {
					//TODO
					break;
				}
				default: {
					for(int i=0; i<NUM_COMMANDS; i++) {
						if(cmMenuCommand[i] == LOWORD(wParam)) {
							(*cmMenuFunc[i])();
							break;
						}
					}
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
	hiconMain = LoadIcon(hInstance,MAKEINTRESOURCE(IDI_ICON_MAIN));
	
	//Register window classes
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
	
	//Create windows
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

