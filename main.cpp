#include "main.h"

/////////////////
//LEVEL LOADING//
/////////////////
//TODO

///////////
//DIALOGS//
///////////
//TODO

//////////////
//MENU STUFF//
//////////////
bool hasSmcHeader;
TCHAR strBuf_main[256];
HMENU hmenuMain;
HWND hwndMain;

//Helper functions
void updateMenu() {
	//Enable/disable menu items
	//that depend on ROM being open
	//TODO
	//Update Edit Object/Sprite check state
	//and relevant enables/disables
	//TODO
	//Update checked state for View menu items
	//TODO
}
inline BOOL prompt(LPCSTR title,LPCSTR msg) {
	return (MessageBox(hwndMain,msg,title,MB_ICONWARNING|MB_YESNO) == IDYES);
}
inline BOOL promptSave() {
	return (MessageBox(hwndMain,"Level data has been modified. Are you sure?","Unsaved changes!",MB_ICONWARNING|MB_YESNO) == IDYES);
}

//Functions for menu items
//TODO

//Tables used for menu processing
#define NUM_COMMANDS (10+6+12+5+6)
const void (*cmMenuFunc[NUM_COMMANDS])() = {};
int cmMenuCommand[NUM_COMMANDS] = {};

/////////////////////
//MAIN WINDOW STUFF//
/////////////////////
HDC				hdcMain;
BITMAPINFO		bmiMain;
HBITMAP			hbmpMain;
PAINTSTRUCT		psMain;
DWORD*			bmpDataMain;

int				xCurScroll = 0,xMaxScroll = 0;
int 			yCurScroll = 0,yMaxScroll = 0;

//Helper functions (for determining invalid regions)
//TODO

//Extra UI drawing stuff
void drawEntrances() {
	//TODO
}
void drawExits() {
	//TODO
}
void drawGrid() {
	//TODO
}
void drawObjHexVals() {
	//TODO
}
void drawSpHexVals() {
	//TODO
}

//Main drawing code
//TODO

//Message loop function
LRESULT CALLBACK WndProc(HWND hwnd,UINT msg,WPARAM wParam,LPARAM lParam) {
	switch(msg) {
		//Creation and destruction of window(s)
		case WM_CREATE: {
			//Create objects
			hdcMain = GetDC(hwnd);
			memset(&bmiMain.bmiHeader,0,sizeof(BITMAPINFOHEADER));
			bmiMain.bmiHeader.biSize			= sizeof(BITMAPINFOHEADER);
			bmiMain.bmiHeader.biPlanes			= 1;
			bmiMain.bmiHeader.biBitCount		= 32;
			bmiMain.bmiHeader.biCompression		= BI_RGB;
			bmiMain.bmiHeader.biSizeImage		= 0;
			bmiMain.bmiHeader.biClrUsed			= 0;
			bmiMain.bmiHeader.biClrImportant	= 0;
			bmiMain.bmiHeader.biWidth			= 0x1000;
			bmiMain.bmiHeader.biHeight			= -0x800;
			CreateDIBSection(hdcMain,&bmiMain,DIB_RGB_COLORS,(void**)&bmpDataMain,NULL,0);
			//Have message 2000 handle this
			SendMessage(hwnd,2000,0,0);
			break;
		}
		case WM_DESTROY: {
			//Free objects
			DeleteDC(hdcMain);
			DeleteObject((HGDIOBJ)hbmpMain);
			//Close program
			PostQuitMessage(0);
			break;
		}
		case WM_CLOSE: {
			//Prompt save
			//TODO
			//Destroy window(s)
			DestroyWindow(hwnd);
			break;
		}
		//Updating
		case WM_PAINT: {
			//Start painting
			BeginPaint(hwnd,&psMain);
			//Update invalidated tiles
			//TODO
			//Finish painting
			EndPaint(hwnd,&psMain);
			break;
		}
		case WM_SIZE: {
			//Resize visible region and invalidate
			SCROLLINFO si;
			memset(&si,0,sizeof(SCROLLINFO));
			si.cbSize		= sizeof(SCROLLINFO);
			si.fMask		= SIF_RANGE|SIF_POS;
			
			xMaxScroll = std::max(0x1000-LOWORD(lParam),0);
			xCurScroll = std::min(xCurScroll,xMaxScroll);
			si.nMax			= xMaxScroll;
			si.nPos			= xCurScroll;
			SetScrollInfo(hwnd,SB_HORZ,&si,TRUE);
			
			yMaxScroll = std::max(0x800-HIWORD(lParam),0);
			yCurScroll = std::min(yCurScroll,yMaxScroll);
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
			
			SCROLLINFO si;
			memset(&si,0,sizeof(SCROLLINFO));
			si.cbSize		= sizeof(SCROLLINFO);
			si.fMask		= SIF_POS;
			
			xCurScroll = xNewScroll;
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
			
			SCROLLINFO si;
			memset(&si,0,sizeof(SCROLLINFO));
			si.cbSize		= sizeof(SCROLLINFO);
			si.fMask		= SIF_POS;
			
			yCurScroll = yNewScroll;
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

