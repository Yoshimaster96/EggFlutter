#include "object.h"

HWND hwndObject;
bool wvisObject = false;
level_object_data_ctx_t objectContexts[2];
int curObjCtx = 0;

//////////////////////////////
//OBJECT OCCUPY REGISTRATION//
//////////////////////////////
void addObjectTile(object_t * o,WORD tile,int offset) {
	if(offset >= 0 && offset < 0x8000) {
		//Store in object
		o->occupiedTiles.push_back(offset);
		//Set tile in tilemap for current context
		objectContexts[curObjCtx].tilemap[offset] = tile;
		objectContexts[curObjCtx].assocObjects[offset].push_back(o);
	}
}

////////////////////////////
//OBJECT DRAWING FUNCTIONS//
////////////////////////////
//Helper function for drawing text
void drawObjectText(object_t * o,char * text) {
	//TODO
}

//Helper functions (for calculating tile offsets)
inline int getBaseMap16Offset(object_t * o) {
	//TODO
}
inline int offsetMap16Right(int curOffs) {
	//TODO
}
inline int offsetMap16Left(int curOffs) {
	//TODO
}
inline int offsetMap16Down(int curOffs) {
	//TODO
}
inline int offsetMap16Up(int curOffs) {
	//TODO
}

//Overlapped tile handlers
//TODO

//Extended objects
//TODO
//Standard objects
//TODO

//Object function pointer table and updater
//TODO
int setObjectContext(int ctx) {
	int prevCtx = curObjCtx;
	curObjCtx = ctx;
	return prevCtx;
}
void drawObjects() {
	//TODO
}
void dispObjects(DWORD * pixelBuf,int width,int height) {
	//TODO
}

/////////////////////
//OBJECT MANAGEMENT//
/////////////////////
//Predicate for deletion
bool object_delPred(object_t & un) {
	return un.selected;
}

//Load/save
int loadObjects(BYTE * data) {
	//Clear buffers
	for(int i=0; i<0x8000; i++) {
		objectContexts[curObjCtx].assocObjects[i].clear();
	}
	objectContexts[curObjCtx].objects.clear();
	//Init stuff
	int curSz = 0;
	//Reload buffer with object data
	while(true) {
		//Create entry
		object_t entry;
		//Get data bytes and check for end
		entry.data[0] = *data++;
		if(entry.data[0]==0xFF) break;
		entry.data[1] = *data++;
		entry.data[2] = *data++;
		entry.data[3] = *data++;
		entry.dataSize = 4;
		if(entry.data[0] && romBuf[0x0904EC+entry.data[0]]&2) {
			entry.data[4] = *data++;
			entry.dataSize = 5;
		}
		//Init other elements to sane values
		entry.selected = false;
		entry.occupiedTiles.clear();
		//Push back
		objectContexts[curObjCtx].objects.push_back(entry);
	}
	//Draw object data
	drawObjects();
	return curSz;
}
int saveObjects(BYTE * data) {
	//Init stuff
	int curSz = 0;
	//Save sprite data
	for(int n=0; n<objectContexts[curObjCtx].objects.size(); n++) {
		//Shorten the names of these for convenience
		BYTE * thisData = objectContexts[curObjCtx].objects[n].data;
		int thisDataSz = objectContexts[curObjCtx].objects[n].dataSize;
		//Copy data
		memcpy(&data[curSz],thisData,thisDataSz);
		//Increment stuff
		curSz += thisDataSz;
	}
}

//Manipulation (internal)
void addToObjectSelection(int index) {
	//Select object
	object_t thisObject = objectContexts[0].objects[index];
	thisObject.selected = true;
	//Mark occupied tiles as invalid
	for(int i = 0; i < thisObject.occupiedTiles.size(); i++) {
		objectContexts[0].invalidObjects[thisObject.occupiedTiles[i]] = true;
	}
}
void removeFromObjectSelection(int index) {
	//Deselect object
	object_t thisObject = objectContexts[0].objects[index];
	thisObject.selected = false;
	//Mark occupied tiles as invalid
	for(int i = 0; i < thisObject.occupiedTiles.size(); i++) {
		objectContexts[0].invalidObjects[thisObject.occupiedTiles[i]] = true;
	}
}
void clearObjectSelection() {
	//Deselect all objects
	for(int n = 0; n < objectContexts[0].objects.size(); n++) {
		object_t thisObject = objectContexts[0].objects[n];
		thisObject.selected = false;
		//Mark occupied tiles as invalid
		for(int i = 0; i < thisObject.occupiedTiles.size(); i++) {
			objectContexts[0].invalidObjects[thisObject.occupiedTiles[i]] = true;
		}
	}
}

//Manipulation
int selectObjects(RECT rect,bool ctrl) {
	//TODO
}
void insertObjects(int x,int y) {
	//TODO
}
void deleteObjects() {
	//Invalidate occupied tiles
	for(int n = 0; n < objectContexts[0].objects.size(); n++) {
		object_t thisObject = objectContexts[0].objects[n];
		if(thisObject.selected) {
			//Mark occupied tiles as invalid
			for(int i = 0; i < thisObject.occupiedTiles.size(); i++) {
				objectContexts[0].invalidObjects[thisObject.occupiedTiles[i]] = true;
			}
		}
	}
	//Delete selected sprites
	remove_if(objectContexts[0].objects.begin(),objectContexts[0].objects.end(),object_delPred);
}
void moveObjects(int dx,int dy) {
	//TODO
}
void resizeObjects(int dx,int dy) {
	//TODO
}
void increaseObjectZ() {
	//TODO
}
void decreaseObjectZ() {
	//TODO
}

///////////////////
//WINDOW FUNCTION//
///////////////////
HDC				hdcObj;
HBITMAP			hbmpObj;
DWORD *			bmpDataObj;

LRESULT CALLBACK WndProc_Object(HWND hwnd,UINT msg,WPARAM wParam,LPARAM lParam) {
	switch(msg) {
		//Creation and destruction of window(s)
		case WM_CREATE: {
			//Add controls
			//TODO
			//Create objects
			//TODO
			break;
		}
		case WM_DESTROY: {
			//Free objects
			//TODO
			break;
		}
		case WM_CLOSE: {
			//Simply hide the window
			ShowWindow(hwnd,SW_HIDE);
			wvisObject = false;
			break;
		}
		//Updating
		case WM_PAINT: {
			//TODO
			break;
		}
		//Input
		case WM_COMMAND: {
			//TODO
			break;
		}
		default: {
			return DefWindowProc(hwnd,msg,wParam,lParam);
		}
	}
	return 0;
}

