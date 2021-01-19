#ifndef OBJECT_H
#define OBJECT_H

//Includes
#include "map16.h"

//Structs
typedef struct {
	BYTE data[8];
	int dataSize;
	bool selected;
	std::string text;
	std::vector<int> occupiedTiles;
} object_t;
typedef struct {
	std::vector<object_t> objects;
	std::vector<object_t*> assocObjects[0x8000];
	WORD tilemap[0x8000];
	bool invalidObjects[0x8000];
} level_object_data_ctx_t;

//Functions
//Object drawing
int setObjectContext(int ctx);
void drawObjects();
void dispObjects(DWORD * pixelBuf,int width,int height);
//Object management
void loadObjects(BYTE * data);
int saveObjects(BYTE * data);
int selectObjects(RECT rect,bool ctrl);
void addToObjectSelection(int index);
void removeFromObjectSelection(int index);
void clearObjectSelection();
void insertObjects(int x,int y);
void deleteObjects();
void moveObjects(int dx,int dy);
void resizeObjects(int dx,int dy);
void increaseObjectZ();
void decreaseObjectZ();
//Window
LRESULT CALLBACK WndProc_Object(HWND hwnd,UINT msg,WPARAM wParam,LPARAM lParam);

#endif

