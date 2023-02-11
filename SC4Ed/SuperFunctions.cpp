/*
This program is free software: you can redistribute it and/or modify
it under the terms of the GNU General Public License as published by
the Free Software Foundation, either version 3 of the License, or
(at your option) any later version.
This program is distributed in the hope that it will be useful,
but WITHOUT ANY WARRANTY; without even the implied warranty of
MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.  See the
GNU General Public License for more details.
You should have received a copy of the GNU General Public License
along with this program.  If not, see <http://www.gnu.org/licenses/>.
*/

#include "SC4ED.h"
#include "DrawED.h"
#include <atomic>
#include <sstream>

// Imports
extern bool drawBackground;
extern DWORD drawThreadId;
extern HBITMAP backBuffer;
extern HBITMAP levelBuffer[2];
extern HBITMAP eventBuffer[2];

extern bool drawLevelBuffer;
extern bool drawCheckpointInfo;
extern bool drawLevelInfo;
extern bool drawEventInfo;
extern bool drawBackground;
extern bool drawCollisionIndex;
extern bool drawEmu;
extern bool drawNewLevel;

static int oldGraphicsNum = 0;
static int oldTileNum = 0;
static int oldPaletteNum = 0;
static int oldLevel = 0;

void DrawThreadFlushMessageQueue() {
	HANDLE h = CreateEvent(NULL, 0, 0, NULL);
	PostThreadMessage(drawThreadId, DrawThreadMessage::SYNC, (WPARAM)0, (LPARAM)h);
	WaitForSingleObject(h, INFINITE);
}

void ResetRepaintEmu() {
	oldGraphicsNum = 0;
	oldTileNum = 0;
	oldPaletteNum = 0;
	oldLevel = 0;
}

void RepaintEmu(int x, int y, int graphicsNum, int tileNum, int paletteNum, int level) {
	extern short cameraX;
	extern short cameraY;
	extern short winWidth;
	extern short winHeight;

	if ((level != oldLevel) && level < nmmx.numLevels) {
		nmmx.SetLevel(level, 0);

		nmmx.tileLoadOffset = tileNum;
		nmmx.palLoadOffset = paletteNum;
		nmmx.objLoadOffset = graphicsNum;
		oldTileNum = tileNum;
		oldPaletteNum = paletteNum;
		oldGraphicsNum = graphicsNum;
		oldLevel = level;

		drawNewLevel = true;
		if (drawBackground)
			nmmx.LoadBackground();
		else
			nmmx.LoadLevel(false);
		render.RefreshMapCache();

		if (drawNewLevel) {
			for (unsigned i = 1; i < 10; ++i) {
				if (hWID[i]) {
					SendMessage(hWID[i], UPDATEWINDOW, (WPARAM)0, (LPARAM)0);
				}
			}
			drawNewLevel = false;
		}

		if (backBuffer != NULL) DeleteObject(backBuffer);
		backBuffer = NULL;

		RepaintAll();
	}
	else if (tileNum != oldTileNum || paletteNum != oldPaletteNum) {
		nmmx.tileLoadOffset = tileNum;
		nmmx.palLoadOffset = paletteNum;
		oldTileNum = tileNum;
		oldPaletteNum = paletteNum;
		RefreshLevel();
	}
	if (graphicsNum != oldGraphicsNum) {
		nmmx.objLoadOffset = graphicsNum;
		oldGraphicsNum = graphicsNum;
		RepaintAll();
	}

	RECT displayRect;
	displayRect.left = 0;
	displayRect.right = winWidth - 1;
	displayRect.top = 28;
	displayRect.bottom = winHeight - 28 - 1;
	InvalidateRect(hWID[0], &displayRect, false);

	RECT rect;
	rect.left = max(0, x - 256 / 4);
	rect.right = min(nmmx.levelWidth * 128 * 2, x + 256 + 24 + 256 / 4);
	rect.top = max(0, y - 256 / 4);
	rect.bottom = min(nmmx.levelHeight * 128 * 2, y + 224 + 24 + 224 / 4);

	if ((rect.left < cameraX)
		|| (rect.right >= cameraX + winWidth)
		|| (rect.top < cameraY)
		|| (rect.bottom >= cameraY + winHeight)) {
		SetWindowScrollPosition(hWID[0], x + 128, y + 112);
	}

}

void RepaintAll()
{
	DrawInfo *d = new DrawInfo;
	drawLevelBuffer = true;
	d->drawLevelBuffer = drawLevelBuffer;
	d->drawBackground = drawBackground;
	d->drawCollisionIndex = drawCollisionIndex;
	d->drawEmu = drawEmu;
	d->drawEventInfo = drawEventInfo;
	d->drawCheckpointInfo = drawCheckpointInfo;
	d->levelBuffer = levelBuffer;
	d->eventBuffer = eventBuffer;
	if (drawEmu) {
		DrawED::Instance()->ReleaseToDraw();
		PostThreadMessage(drawThreadId, DrawThreadMessage::DRAW, (WPARAM)0, (LPARAM)d);
	}
	else {
		for (unsigned i = 0; i < 10; ++i) {
			if (hWID[i]) {
				InvalidateRect(hWID[i], NULL, false);
			}
		}
		delete d;
	}
	//}
}

void LoadLevel(WORD level)
{
	// temporary workaround for mode7 level0
	//if (level == 0) return;

#if 0
	std::stringstream ss;
	for (unsigned i = 0; i < 0x44; i++) {
		nmmx.SetLevel(i, 0);
		nmmx.LoadLevel();
		render.RefreshMapCache();

		ss << std::hex << " 0x" << nmmx.GetEventSize() << ",";
	}
	MessageBox(hWID[0], ss.str().c_str(), "Event", MB_ICONERROR);
#endif

	nmmx.SetLevel(level, 0);
	nmmx.LoadLevel();
	render.RefreshMapCache();
	SetScrollRange(hWID[0], SB_HORZ, 0, (nmmx.levelWidth<<7)-1, true);
	SetScrollRange(hWID[0], SB_VERT, 0, (nmmx.levelHeight<<7)-1, true);

	if (backBuffer != NULL) {
		DeleteObject(backBuffer);
	}
	backBuffer = NULL;

	if (drawNewLevel) {
		for (unsigned i = 1; i < 10; ++i) {
			if (hWID[i]) {
				SendMessage(hWID[i], UPDATEWINDOW, (WPARAM)0, (LPARAM)0);
			}
		}
		drawNewLevel = false;
	}

	RepaintAll();
}
void SaveLevel(WORD level) {
	nmmx.SaveLevel();
}
void RefreshLevel()
{
	if (drawBackground)
		nmmx.LoadBackground();
	else
		nmmx.LoadLevel(true);
	render.RefreshMapCache();

	RepaintAll();
}
void ScrollProc(HWND hWnd, bool scrollbar, WPARAM wParam, short *value)
{
	switch(LOWORD(wParam))
	{
	case SB_LINEUP:
		*value-=8;
		break;
	case SB_LINEDOWN:
		*value+=8;
		break;
	case SB_PAGEUP:
		*value-=64;
		break;
	case SB_PAGEDOWN:
		*value+=64;
		break;
	case SB_THUMBPOSITION:
		*value = HIWORD(wParam);
		break;
	case SB_THUMBTRACK:
		*value = HIWORD(wParam);
		break;
	case SB_TOP:
		ErrorBox(hWnd, "SB_TOP unimplemented", 0);
		break;
	case SB_BOTTOM:
		ErrorBox(hWnd, "SB_BOTTOM unimplemented", 0);
		break;
	case SB_ENDSCROLL:
		SetScrollPos(hWnd, scrollbar, *value, true);
		break;
	}
	if (*value < 0) *value = 0;
	SetScrollPos(hWnd, scrollbar, *value, true);
}
void FreezeMenu(HWND hWnd, WORD id, bool freeze)
{
	SendMessage(GetDlgItem(hWnd, id), NULL, NULL, NULL);
}
bool IsFileExist(LPSTR fileName)
{
#ifdef WIN32
	return GetFileAttributes(fileName) != INVALID_FILE_ATTRIBUTES;
#else
	return _access(fileName, 00) == 0;
#endif
}
void SetWindowPosition(HWND hWnd, SHORT x, SHORT y)
{
	RECT rcwin;
	GetWindowRect(hWnd, &rcwin);
	SetWindowPos(hWnd, NULL, x, y, rcwin.right - rcwin.left, rcwin.bottom - rcwin.top, SWP_SHOWWINDOW);
}
void SetWindowScrollPosition(HWND hWnd, int x, int y) {
	RECT viewRect;
	GetWindowRect(hWnd, &viewRect);

	int width = (viewRect.right - viewRect.left) / 2;
	int height = (viewRect.bottom - viewRect.top) / 2;

	int xpos = (x - width > 0) ? x - width : 0;
	int ypos = (y - height > 0) ? y - height : 0;

	SetScrollPos(hWnd, SB_HORZ, xpos, true);
	PostMessage(hWnd, WM_HSCROLL, 4 + 0x10000 * xpos, 0);
	SetScrollPos(hWnd, SB_VERT, ypos, true);
	PostMessage(hWnd, WM_VSCROLL, 4 + 0x10000 * ypos, 0);
}

void UpdateWindow() {
	for (unsigned i = 1; i < 10; ++i) {
		if (hWID[i]) {
			SendMessage(hWID[i], UPDATEWINDOW, (WPARAM)0, (LPARAM)0);
		}
	}
}