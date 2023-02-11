#ifndef _SC4ED_
#define _SC4ED_

#include "stdafx.h"

#include "SC4Core.h"
#include "SC4EDCore.h"
#include "SNESCore.h"
#include "NDSCore.h"
#include "RenderED.h"
#include "CompressionCore.h"
////////////////////////
#include "Settings.h"
#include "FileDialog.h"
#include "WinCtrl.h"
#include "Toolbar.h"
#include "resource.h"
#include "ResourceX.h"
#define byte unsigned char
#define u16 unsigned short
#define u32 unsigned int

static const unsigned UPDATEWINDOW = WM_APP + 4096;

struct FrameState {
	int xpos = 0;
	int ypos = 0;
	int borderLeft;
	int borderRight;
	int borderTop;
	int borderBottom;
	int graphicsNum = 0;
	int tileNum = 0;
	int paletteNum = 0;
	int levelNum = 0;

	// things to record
	// state0
	WORD state0 = 0xFFFF;
	// state1
	WORD state1 = 0xFFFF;
	// xpos
	WORD s_xpos = 0xFFFF;
	// ypos
	WORD s_ypos = 0xFFFF;
	// c0_xpos
	WORD c0_xpos = 0xFFFF;
	// c0_ypos
	WORD c0_ypos = 0xFFFF;
	// c1_xpos
	WORD c1_xpos = 0xFFFF;
	// c1_ypos
	WORD c1_ypos = 0xFFFF;

	// lockState
	WORD lockState = 0xFFFF;
	// A0
	WORD A0 = 0xFFFF;
	// A2
	WORD A2 = 0xFFFF;
	// A4
	WORD A4 = 0xFFFF;
	// A6
	WORD A6 = 0xFFFF;
	// A8
	WORD A8 = 0xFFFF;
	// AA
	WORD AA = 0xFFFF;
	// Coll
	// -

	unsigned *buffer = nullptr;
};

enum DrawThreadMessage {
	FIRST = WM_APP + 5120,
	DRAW = FIRST,
	SYNC,
	QUIT,
	LAST = QUIT,
	DRAWREFRESH,
};

struct DrawInfo {
	HBITMAP *levelBuffer = nullptr;
	HBITMAP *eventBuffer = nullptr;

	bool drawLevelBuffer = false;
	bool drawCollisionIndex = false;
	bool drawBackground = false;
	bool drawEventInfo = false;
	bool drawEmu = false;
	bool drawCheckpointInfo = false;
};

// MegaED X.cpp
extern bool drawLevelBuffer;

extern RenderED render;
extern HINSTANCE hInst;
extern HWND hWID[10];
void ResetRepaintEmu();
void RepaintEmu(int x, int y, int tileNum, int graphicsNum, int paletteNum, int level);
void RepaintAll();
void RefreshLevel();

// DebugInfo.cpp
extern char out[0x40];
extern unsigned char debugInfoIndex;
extern bool debugEnabled;
void WriteDebugInfo(HDC);

// WinControls
void ShowLastError(DWORD);
// Super Functions
void SetWindowPosition(HWND, SHORT x, SHORT y);
void SetWindowScrollPosition(HWND, int x, int y);

HBITMAP CreateBitmapCache(HWND hWnd, int width, int height);
void ErrorBox(HWND, LPCTSTR, DWORD);
void InitToolBar(HWND hWnd);
void InitStatusBar(HWND hWnd);
void memdwcopy(DWORD *dst, DWORD *src, int size);
void print(LPSTR text);
void print(LPSTR text, int param);
void FileDebugOut(void *data, DWORD dataSize, STRING fileName);
void CreateBitmapCache();
void RenderImage(HDC hdc, int x, int y, WORD image[], WORD param);
void RenderImageZoom(HDC hdc, int x, int y, WORD image[], int size);
bool OpenFileDialogCore(HWND hWnd, CSTRING filter, SC4EDCore *romcore);
void GetFrameState(FrameState &s);
void SetVolume(int v);

extern SC4Core nmmx;

extern const unsigned char vrambase[];
extern const int vrambase_size;

extern const LPSTR sWindowClass;

#endif