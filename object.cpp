#include "object.h"

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
void drawObjectText(object_t * o,std::string text,int width) {
	o->text = text;
	//Create dummy sprites to draw text on top of
	//(needed to mark space as occupied)
	width -= 0x10;
	for(int i = 0; i < width; i += 0x10) {
		//TODO
	}
	if(width&8); //TODO
}

//Helper functions (for calculating tile offsets)
inline int getBaseMap16Offset(object_t o) {
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
void loadObjects(BYTE * data) {
	//TODO
}
int saveObjects(BYTE * data) {
	//TODO
}

//Manipulation
int selectObjects(RECT rect,bool ctrl) {
	//TODO
}
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
LRESULT CALLBACK WndProc_Object(HWND hwnd,UINT msg,WPARAM wParam,LPARAM lParam) {
	switch(msg) {
		//TODO
	}
}

