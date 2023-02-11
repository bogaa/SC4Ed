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

#include "Emulator.h"
#include "SC4ED.h"
#include "Mmsystem.h"
#include "DrawED.h"

#include <algorithm>
#include <map>
#include <vector>
#include <sstream>

SC4Core nmmx;
RenderED render;

HWND hWID[10];
HINSTANCE hInst;
Toolbar tb;
// Procedures
BOOL CALLBACK PalettesProc (HWND, UINT, WPARAM, LPARAM);
BOOL CALLBACK TilesProc    (HWND, UINT, WPARAM, LPARAM);
BOOL CALLBACK MapProc      (HWND, UINT, WPARAM, LPARAM);
BOOL CALLBACK BlockProc    (HWND, UINT, WPARAM, LPARAM);
BOOL CALLBACK SceneProc    (HWND, UINT, WPARAM, LPARAM);
BOOL CALLBACK SpriteProc   (HWND, UINT, WPARAM, LPARAM);
BOOL CALLBACK ImportParam  (HWND, UINT, WPARAM, LPARAM);
BOOL CALLBACK SettingsProc (HWND, UINT, WPARAM, LPARAM);
BOOL CALLBACK EventProc    (HWND, UINT, WPARAM, LPARAM);
BOOL CALLBACK CheckpointProc(HWND, UINT, WPARAM, LPARAM);
BOOL CALLBACK PropertyProc (HWND, UINT, WPARAM, LPARAM);
BOOL CALLBACK AboutProc    (HWND, UINT, WPARAM, LPARAM);
BOOL CALLBACK InternalEmulatorSettingsProc(HWND, UINT, WPARAM, LPARAM);

// Imports
//extern void RepaintAll();
extern void LoadLevel(WORD level);
extern void SaveLevel(WORD level);
extern void RefreshLevel();
extern void ScrollProc(HWND, bool, WPARAM, short*);
extern void SetWindowTitle(HWND, CSTRING);
extern bool IsFileExist(LPSTR);
extern void FreezeMenu(HWND, WORD id, bool freeze);
extern void UpdateWindow();

// Paint vars
PAINTSTRUCT ps;
HDC hDC, hBackDC, hLevelDC, hEventDC;
HBITMAP backBuffer = NULL, levelBuffer[2] = { NULL, NULL }, eventBuffer[2] = { NULL, NULL };
unsigned currentBuffer = 0;
HBITMAP bmpTeleport, bmpXSprite;
HBRUSH hBrushBlack;

DWORD drawThreadId = -1;
HANDLE drawThreadHandle;

short winWidth, winHeight;
short cameraX, cameraY;
bool drawLevelBuffer = false;
bool drawCheckpointInfo = false;
bool drawLevelInfo = true;
bool drawEventInfo = true;
bool drawBackground = false;
bool drawCollisionIndex = false;
bool drawEmu = false;
bool drawForceEmu = false;
bool drawNewLevel = true;
unsigned cameraEvent = 0;

char   importName[MAX_PATH];
LPBYTE importFile;
DWORD  importSize;

static POINTS mousePos;
static unsigned mouseEventBlockNum = 0;
static unsigned mouseEventEventNum = 0;
static RECT mouseEventRect;
static std::string mouseEventString;
static bool mouseEventDrag = false;

static int saveStateCount = 0;
static bool saveStateSave = false;
static int saveState = 0;

static EventInfo *GetEvent() {
	EventInfo *ret = nullptr;

	TCHAR text[13];
	unsigned currentBlockNum = 0;
	unsigned currentEventNum = 0;

	if (IsWindow(hWID[7])) {
		GetWindowText(GetDlgItem(hWID[7], 0x9101), text, 12);
		sscanf(text, "%x", &currentEventNum);

		bool validEvent = !nmmx.eventTable.empty() && currentEventNum < nmmx.eventTable.size();
		if (validEvent) {
			auto iter = nmmx.eventTable.begin();
			std::advance(iter, currentEventNum);
			ret = &*iter;
		}
	}

	return ret;
}

void GetFrameState(FrameState &s) {
	Emulator::Instance()->GetFrameState(s);
}

void SetVolume(int v) {
	Emulator::Instance()->SetVolume(v);
}

LRESULT CALLBACK WndProc(HWND hWnd, UINT message, WPARAM wParam, LPARAM lParam)
{
	RECT rc;
	switch (message)
	{
	case WM_PAINT:
	{
		hDC = BeginPaint(hWnd, &ps);

		//MessageBox(hWID[0], "Paint Start", "Test", MB_ICONERROR);

		hBackDC  = CreateCompatibleDC(NULL);
		hLevelDC = CreateCompatibleDC(NULL);
		hEventDC = CreateCompatibleDC(NULL);

		if (!drawEmu) {
			// TODO: avoid current race when emulator is off
			DrawInfo d;
			d.drawLevelBuffer = drawLevelBuffer;
			d.eventBuffer = eventBuffer;
			d.levelBuffer = levelBuffer;
			d.drawBackground = drawBackground;
			d.drawCheckpointInfo = drawCheckpointInfo;
			d.drawCollisionIndex = drawCollisionIndex;
			d.drawEmu = drawEmu;
			d.drawEventInfo = drawEventInfo;
			DrawED::Instance()->DrawBuffers(&d, currentBuffer);
			drawLevelBuffer = false;
		}

		auto newBuffer = backBuffer == NULL;
		if (newBuffer) {
			backBuffer = CreateCompatibleBitmap(hDC, nmmx.levelWidth << 8, nmmx.levelHeight << 8);

			//if (false && !drawEmu) {
				auto brush = CreateSolidBrush(RGB(0, 0, 0));
				RECT rect;
				rect.left = 0;
				rect.right = winWidth - 1;
				rect.top = 0;
				rect.bottom = winHeight - 1;
				FillRect(hDC, &rect, brush);
				DeleteObject(brush);
			//}

		}

		SelectObject(hBackDC,  backBuffer);
		SelectObject(hLevelDC, levelBuffer[currentBuffer]);
		SelectObject(hEventDC, eventBuffer[currentBuffer]);

		auto trueWinWidth = min((short(nmmx.levelWidth) << 8) - cameraX, winWidth, 0);
		auto trueWinHeight = min((short(nmmx.levelHeight) << 8) - cameraY, winHeight, 0);

		if (true || newBuffer) {
			FillRect(hBackDC, &ps.rcPaint, hBrushBlack);
		}

		BitBlt(hBackDC, 0, 0, trueWinWidth, trueWinHeight, hLevelDC, cameraX, cameraY, SRCCOPY);

		//RECT r = ps.rcPaint;
		//r.top += 28;
		if (drawLevelInfo) WriteDebugInfo(hBackDC);
		if (drawEmu) {
			// fill in the emulator graphics
			FrameState s;
			Emulator::Instance()->GetFrameState(s);
			s.xpos -= cameraX;
			s.ypos -= cameraY;
			s.borderLeft -= cameraX;
			s.borderRight -= cameraX;
			s.borderTop -= cameraY;
			s.borderBottom -= cameraY;
			render.RenderEmu(hBackDC, s);

			if (drawEventInfo) {
				RECT borderRect;
				borderRect.left = s.borderLeft += 1;
				borderRect.right = s.borderRight -= 1;
				borderRect.top = s.borderTop += 1;
				borderRect.bottom = s.borderBottom -= 1;
				auto brush = CreateSolidBrush(RGB(255, 255, 0));
				FrameRect(hBackDC, &borderRect, brush);
				DeleteObject(brush);
			}

			char out[0x40];
			sprintf(out, "%i", saveState);
			render.Print(hBackDC, 0, 13, out, 0);

			if (saveStateCount) {
				char saveStateString[0x40];
				sprintf(saveStateString, " %s", saveStateSave ? "SAVE" : "LOAD");
				render.Print(hBackDC, 2, 13, saveStateString, 0);
			}
		}

		if (drawEventInfo) {
			TransparentBlt(hBackDC, 0, 0, trueWinWidth, trueWinHeight, hEventDC, cameraX, cameraY, trueWinWidth, trueWinHeight, RGB(0,0,0));
			//BitBlt(hBackDC, 0, 0, winWidth, winHeight, hEventDC, cameraX, cameraY, SRCCOPY);
		}

		BitBlt(hDC, 0, 28, winWidth, winHeight, hBackDC, 0, 0, SRCCOPY);
		DeleteDC(hBackDC);
		DeleteDC(hLevelDC);
		DeleteDC(hEventDC);

		//MessageBox(hWID[0], "Paint End", "Test", MB_ICONERROR);

		EndPaint(hWnd, &ps);
		break;
	}
	case WM_MOVE:
		if (!IsZoomed(hWnd))
		{
			GetWindowRect(hWnd, &rc);
			set.WinMainPos.X = (SHORT)rc.left;
			set.WinMainPos.Y = (SHORT)rc.top;
		}
		break;
	case WM_MOUSEMOVE:
	{
		if (GetKeyState(VK_RBUTTON) < 0) {
			auto origPt = MAKEPOINTS(lParam);
			auto pt = origPt;

			pt.x -= mousePos.x;
			pt.y -= mousePos.y;

			pt.x /= 16; pt.x *= 16;
			pt.y /= 16; pt.y *= 16;

			cameraX -= pt.x;
			cameraY -= pt.y;

			if (cameraX < 0) cameraX = 0;
			if (cameraY < 0) cameraY = 0;
			if (cameraX > (nmmx.levelWidth << 8)) cameraX = nmmx.levelWidth << 8;
			if (cameraY > (nmmx.levelHeight << 8)) cameraY = nmmx.levelHeight << 8;

			SetScrollPos(hWnd, SB_HORZ, cameraX, true);
			SetScrollPos(hWnd, SB_VERT, cameraY, true);
			InvalidateRect(hWnd, NULL, false);

			if (pt.x) { mousePos.x = origPt.x; }
			if (pt.y) { mousePos.y = origPt.y; }
		}
		else if (GetKeyState(VK_MBUTTON) < 0) {
			if (mouseEventDrag) {
				auto event = GetEvent();
				if (event) {
					auto pt = MAKEPOINTS(lParam);
					pt.x += cameraX;
					pt.y += cameraY - 28;

					TCHAR text[13];
					sprintf(text, "%x", pt.x);
					SetWindowText(GetDlgItem(hWID[7], 0x9104), text);
					sprintf(text, "%x", pt.y);
					SetWindowText(GetDlgItem(hWID[7], 0x9105), text);

					//InvalidateRect(hWnd, NULL, false);
					RepaintAll();
				}
			}
		}
		break;
	}
	case WM_RBUTTONDOWN:
	{
		// record the pos for moving the scene
		mousePos = MAKEPOINTS(lParam);
	}
	break;
	case WM_RBUTTONDBLCLK:
	case WM_MBUTTONDOWN:
	{
		// check if the current event
		auto event = GetEvent();
		if (event) {
			//auto boundingBox = nmmx.GetBoundingBox(*event);
			auto pt = MAKEPOINTS(lParam);
			pt.x += cameraX;
			pt.y += cameraY - 28;
			//POINT p = { pt.x, pt.y };

			//if (PtInRect(&boundingBox, p)) {
				//mouseEventDrag = true;
			//}
	
			TCHAR text[13];
			sprintf(text, "%x", pt.x);
			SetWindowText(GetDlgItem(hWID[7], 0x9104), text);
			sprintf(text, "%x", pt.y);
			SetWindowText(GetDlgItem(hWID[7], 0x9105), text);

			//InvalidateRect(hWnd, NULL, false);
			RepaintAll();
		}
		break;
	}
	case WM_MBUTTONUP:
	{
		mouseEventDrag = false;
		break;
	}
	case WM_LBUTTONDOWN:
	{
		// check if we need to change the event
		if (IsWindow(hWID[7]) && !drawBackground && drawEventInfo) {
			// get current block
			auto pt = MAKEPOINTS(lParam);
			pt.x += cameraX;
			pt.y += cameraY - 28;
			POINT p = { pt.x, pt.y };
			long tempBlockNum = (pt.x >> 5);

			// get current event
			TCHAR text[13];
			long currentBlockNum = 0;
			long currentEventNum = 0;
			//GetWindowText(GetDlgItem(hWID[7], 0x9100), text, 12);
			//sscanf(text, "%x", &currentBlockNum);
			GetWindowText(GetDlgItem(hWID[7], 0x9101), text, 12);
			sscanf(text, "%x", &currentEventNum);

			bool validEvent = !nmmx.eventTable.empty() && currentEventNum < nmmx.eventTable.size();
			auto iter = nmmx.eventTable.begin();
			if (validEvent) {
				std::advance(iter, currentEventNum);
				RECT boundingBox = nmmx.GetBoundingBox(*iter);
				validEvent &= PtInRect(&boundingBox, p);
			}

			// check events near mouse
			bool foundEvent = false;

			// first try to find event after current event when there is overlap
			unsigned tempEventNum = 0;
			for (auto &e : nmmx.eventTable) {
				// try to find a new event
				RECT boundingBox = nmmx.GetBoundingBox(e);

				if (PtInRect(&boundingBox, p)) {
					//sprintf(text, "%x", i);
					//SetWindowText(GetDlgItem(hWID[7], 0x9100), text);
					sprintf(text, "%x", tempEventNum);
					SetWindowText(GetDlgItem(hWID[7], 0x9101), text);
					foundEvent = true;

					//bool r = false;

					//if (nmmx.type == 2 && e.type == 3 && e.unknown == 8) {
					//	WORD addr = *LPWORD(nmmx.rom + SNESCore::snes2pc(0x81AF82 + e.eventId));
					//	addr = SReadWord(nmmx, 0x810000 | (addr + 2));
					//	if (addr) {
					//		addr = *LPWORD(nmmx.rom + SNESCore::snes2pc(0x810000 | (addr + 4)));

					//		nmmx.tileDec.clear();
					//		while (SReadWord(nmmx, 0x810000 | (addr + 0))) {
					//			DWORD tileDecSrc = SReadDWord(nmmx, 0x810000 | (addr + 6)) & 0xFFFFFF;
					//			DWORD tileDecDst = SReadWord(nmmx, 0x810000 | (addr + 4));
					//			DWORD tileDecSize = SReadWord(nmmx, 0x810000 | (addr + 9));

					//			nmmx.tileDec.push_back({ tileDecSrc, tileDecDst, tileDecSize });
					//			addr += 0xB;
					//		}
					//		r = true;
					//	}
					//}
					//else if (!nmmx.tileDec.empty()) {
					//	nmmx.tileDec.clear();
					//	r = true;
					//}

					//if (nmmx.type == 2 && e.type == 3 && e.unknown == 2) {
					//	nmmx.dynDecTable.clear();

					//	nmmx.dynDecTable.push_back(0x810000 | SReadWord(nmmx, 0x81AEB8 + e.eventId));
					//	r = true;
					//}
					//else if (!nmmx.dynDecTable.empty()) {
					//	nmmx.dynDecTable.clear();
					//	r = true;
					//}

					//if (nmmx.type == 2 && e.type == 3 && e.unknown == 3) {
					//	nmmx.dynPalTable.clear();

					//	nmmx.dynPalTable.push_back(0x810000 | SReadWord(nmmx, 0x81AEEE + e.eventId));
					//	r = true;
					//}
					//else if (!nmmx.dynPalTable.empty()) {
					//	nmmx.dynPalTable.clear();
					//	r = true;
					//}

					//if (r) 
					//	RefreshLevel();
					//else 
						RepaintAll();
					break;
				}

				tempEventNum++;
			}
			//	for (long i = max(0, tempBlockNum - 4); (i < min(0x100, tempBlockNum + 6)) && !foundEvent; ++i) {
			//		unsigned tempEventNum = 0;
			//		for (auto &event : nmmx.eventTable[i]) {
			//			if (validEvent) {
			//				// skip up till the current event
			//				if (i == currentBlockNum && tempEventNum == currentEventNum) validEvent = false;
			//			}
			//			else {
			//				RECT boundingBox = nmmx.GetBoundingBox(event);

			//				if (PtInRect(&boundingBox, p)) {
			//					sprintf(text, "%x", i);
			//					SetWindowText(GetDlgItem(hWID[7], 0x9100), text);
			//					sprintf(text, "%x", tempEventNum);
			//					SetWindowText(GetDlgItem(hWID[7], 0x9101), text);
			//					foundEvent = true;
			//					RepaintAll();
			//					break;
			//				}
			//			}
			//			tempEventNum++;
			//		}
			//	}
			//}

			//// otherwise find new event
			//if (!foundEvent) {
			//	for (long i = max(0, tempBlockNum - 4); (i < min(0x100, tempBlockNum + 6)) && !foundEvent; ++i) {
			//		unsigned tempEventNum = 0;
			//		for (auto &event : nmmx.eventTable[i]) {
			//			RECT boundingBox = nmmx.GetBoundingBox(event);

			//			if (PtInRect(&boundingBox, p)) {
			//				sprintf(text, "%x", i);
			//				SetWindowText(GetDlgItem(hWID[7], 0x9100), text);
			//				sprintf(text, "%x", tempEventNum);
			//				SetWindowText(GetDlgItem(hWID[7], 0x9101), text);
			//				foundEvent = true;
			//				RepaintAll();
			//				break;
			//			}
			//			tempEventNum++;
			//		}
			//	}
			//}
		}
		break;
	}
	case WM_SIZE:
	{
		GetWindowRect(hWnd, &rc);
		winWidth = (short)(rc.right - rc.left);
		winHeight = (short)(rc.bottom - rc.top);
		if (!IsZoomed(hWnd))
		{
			set.WinMainSize.X = winWidth;
			set.WinMainSize.Y = winHeight;
		}
		SendMessage(GetDlgItem(hWnd, IDT_TOOLBAR), TB_AUTOSIZE, 0, 0);
		break;
	}
	case WM_COMMAND:
		switch (LOWORD(wParam))
		{

		case ID_FILE_OPEN:
			drawEmu = false;
			Emulator::Instance()->Terminate();

			char fileNameTmp[MAX_PATH];
			OpenFileDialog(hWnd, "Super Castlevania ROM (*.smc)\0*.smc\0All Files (*.*)\0*.*\0", fileNameTmp);
			if (nmmx.LoadNewRom(fileNameTmp))
			{
				if (!nmmx.CheckROM())
				{
					//MessageBox(hWnd, "Wrong ROM. Please open MMX1, X2 or X3 ROM.", "Error", MB_ICONERROR);
					MessageBox(hWnd, "Wrong ROM. Please open Super Castlevania IV (US/JP) ROM.", "Error", MB_ICONERROR);
					nmmx.FreeRom();
					RepaintAll();
					InvalidateRect(hWnd, NULL, false);
					return 0;
				}
				SetWindowTitle(hWnd, fileNameTmp);
				nmmx.LoadFont();
				//nmmx.LoadProperties();
				render.Init(&nmmx);
				render.CreateMapCache(hWnd);
				render.CreateFontCache(hWnd);
				render.RefreshFontCache();
				LoadLevel(0);
				RepaintAll();
				//SetScrollRange(hWnd, SB_HORZ, 0, (nmmx.levelWidth<<8), true);
				//SetScrollRange(hWnd, SB_VERT, 0, (nmmx.levelHeight<<8), true);
				for(int i=0; i<4; i++)
					strcpy(set.lastroms[4-i], set.lastroms[3-i]);
				strcpy(set.lastroms[0], nmmx.filePath);

				EnableMenuItem(GetMenu(hWnd), 1, MF_BYPOSITION | MF_ENABLED);
				EnableMenuItem(GetSubMenu(GetMenu(hWnd), 0), 1, MF_BYPOSITION | MF_ENABLED);
				EnableMenuItem(GetSubMenu(GetMenu(hWnd), 0), 2, MF_BYPOSITION | MF_ENABLED);
				EnableMenuItem(GetSubMenu(GetMenu(hWnd), 0), 3, MF_BYPOSITION | MF_ENABLED);
				EnableMenuItem(GetSubMenu(GetMenu(hWnd), 0), 5, MF_BYPOSITION | MF_ENABLED);
				EnableMenuItem(GetSubMenu(GetMenu(hWnd), 0), 6, MF_BYPOSITION | MF_ENABLED);
				EnableMenuItem(GetSubMenu(GetMenu(hWnd), 0), 8, MF_BYPOSITION | MF_ENABLED);
				EnableMenuItem(GetSubMenu(GetMenu(hWnd), 0), 9, MF_BYPOSITION | MF_ENABLED);

				UpdateWindow();
			}
			break;
		case ID_FILE_SAVEAS:
		{
			char fileNameTmp[MAX_PATH];
			SaveFileDialog(hWnd, nmmx.filePath, fileNameTmp);
			strcpy(nmmx.filePath, fileNameTmp);
			nmmx.SaveAsRom(nmmx.filePath);
			//break; // fall through
		}
		case ID_FILE_SAVE:
		{
//			byte tvram[0x10000];
//			ZeroMemory(nmmx.vram, 0x10000);
//			for(int i=0; i<0x400; i++)
//				nmmx.raw2tile4bpp(nmmx.vramCache+(i*0x40), nmmx.vram+(i*0x20));
//			nmmx.SortTiles();
//
//			// compressed sizes from unmodified roms
//			WORD origSize[][14] = { {0,0,0,0,0,0,0,0,0,0,0,0,0,0},
//									{0x5418,0x53CF,0x4946,0x4957,0x438A,0x4A2D,0x42D3,0x486A,0x4487,0x1A56,0x1EFA,0x1E66,0x1E78,0x0000},
//									{0x496C,0x50FF,0x5EF5,0x3EDF,0x5614,0x3208,0x541A,0x5E32,0x50B8,0x354A,0x48E3,0x3FFF,0x3A3A,0x33F9} };
//
//			WORD size = origSize[nmmx.type][nmmx.level] ? origSize[nmmx.type][nmmx.level] : nmmx.tileCmpRealSize;
//
//			WORD newSize = GFXRLECmp(nmmx.vram + 0x200, tvram, nmmx.tileCmpSize, nmmx.type);
//
//			if (newSize > size)
//				switch(MessageBox(hWnd, "The compressed tiles size is more than the original compress size.\nYou can corrupt the data of ROM if you don't know what are you doing.\nAre you sure to continue with the compression?",
//					"Warning", MB_ICONWARNING | MB_YESNOCANCEL))
//			{
//				case IDYES:
//COMPRESS_TILES:
//					memcpy(nmmx.rom + nmmx.tileCmpPos, tvram, newSize);
//					if (size > newSize)
//						ZeroMemory(nmmx.rom + nmmx.tileCmpPos + newSize, size - newSize);
//
//					break;
//				case IDNO:
//					break;
//				case IDCANCEL:
//					return 0;
//			}
//			else goto COMPRESS_TILES;
//			if (nmmx.tileDecSize) {
//				memcpy(nmmx.rom + nmmx.tileDecPos, nmmx.vram + nmmx.tileDecDest, nmmx.tileDecSize);
//			}
//
			// compressed sizes from unmodified roms
//			WORD eventSize = nmmx.GetOrigEventSize() ? nmmx.GetOrigEventSize() : nmmx.eventSize;
//			WORD newEventSize = nmmx.GetEventSize();
//
//			if (newEventSize > eventSize)
//				switch (MessageBox(hWnd, "The event size is more than the original event size.\nYou can corrupt the data of ROM if you don't know what are you doing.\nAre you sure to continue?",
//					"Warning", MB_ICONWARNING | MB_YESNOCANCEL))
//				{
//				case IDYES:
//WRITE_EVENTS:
//					// Update the events
//					break;
//				case IDNO:
//					break;
//				case IDCANCEL:
//					return 0;
//				}
//			else goto WRITE_EVENTS;

			nmmx.SaveEvents();

			//
//			nmmx.SaveSprites();

			nmmx.SaveRom(nmmx.filePath);
			LoadLevel(nmmx.level);
			break;
		}
		case ID_FILE_LOADLEVEL:
		{
			char fileNameTmp[MAX_PATH];
			if (OpenFileDialog(hWnd, "Super Castlevania Level(*.scl)\0 * .scl\0", fileNameTmp)) {
				auto ok = nmmx.LoadLevelFromFile(fileNameTmp);
				if (!ok) {
					MessageBox(hWnd, "Load FAILED", "Warning", MB_ICONWARNING);
				}
				else {
					drawNewLevel = true;
					LoadLevel(nmmx.level);
				}
			}
			break;
		}
		case ID_FILE_SAVELEVELAS:
		{
			char fileNameTmp[MAX_PATH];
			if (SaveFileDialog(hWnd, "Super Castlevania Level(*.scl)\0 * .scl\0", fileNameTmp)) {
				auto ok = nmmx.SaveLevelToFile(fileNameTmp);
				if (!ok) {
					MessageBox(hWnd, "Save FAILED", "Warning", MB_ICONWARNING);
				}
			}
			break;
		}
		case ID_FILE_EXPANDROM: {
			auto ok = nmmx.ExpandROM();
			MessageBox(hWnd, ok ? "Expansion OK" : "Expansion FAILED", "Warning", MB_ICONWARNING);
			if (ok) {
				drawNewLevel = true;
				LoadLevel(nmmx.level);
				UpdateWindow();
			}
			break;
		}
		case ID_FILE_RUNINTERNALEMULATOR: {
			Emulator::Instance()->Terminate();
			ResetRepaintEmu();
			Emulator::Instance()->Init();
			Emulator::Instance()->LoadRom(nmmx.rom, nmmx.romSize);
			Emulator::Instance()->LoadLevel(nmmx.level);
			
			saveStateCount = 0;
			saveState = 0;
			drawEmu = true;
			break;
		}
		case IDT_PLAY:
			if (IsFileExist(set.emupath))
			{
LOC_RUNEMU:
				STARTUPINFO startupInfo;
				PROCESS_INFORMATION processInfo;
				memset(&startupInfo, 0, sizeof(STARTUPINFO));
				memset(&processInfo, 0, sizeof(PROCESS_INFORMATION));
				startupInfo.cb = sizeof(STARTUPINFO);
				startupInfo.dwFlags = STARTF_USESHOWWINDOW | 0x00002000/*STARTF_PREVENTPINNING*/;
				startupInfo.wShowWindow = SW_SHOWDEFAULT;
				char cmd[MAX_PATH+1];
				sprintf(cmd, "%s \"%s\"", set.emupath, nmmx.filePath);
				CreateProcess(set.emupath, cmd, NULL, NULL, false, NORMAL_PRIORITY_CLASS, NULL, NULL, &startupInfo, &processInfo);
				//WaitForSingleObject(processInfo.hProcess, INFINITE);
			}
			else
			{
				if (OpenFileDialog(hWnd, "SNES Emulator\0*.exe\0", set.emupath))
					goto LOC_RUNEMU;
			}
			break;
		case ID_IMPORT_SNES:
			importFile = OpenFileRam(OpenFileDialog(hWnd, "Super Nintendo VRAM\0*.*\0", importName), &importSize);
			for(int i=0; i<importSize/0x20; i++)
				nmmx.tile4bpp2raw(importFile+(i*0x20), nmmx.vramCache+(i*0x40));
			Free(importFile);
			break;
		case ID_IMPORT_GEN:
			importFile = OpenFileRam(OpenFileDialog(hWnd, "Genesis VRAM\0*.*\0", NULL), &importSize);
			Free(importFile);
			break;
		case ID_IMPORT_GBA:
			importFile = OpenFileRam(OpenFileDialog(hWnd, "Gameboy Advance VRAM\0*.*\0", NULL), &importSize);
			Free(importFile);
			break;
		case IDT_BACKGROUND:
			goto LOC_BACKGROUND;
			break;
		case IDT_COLLISION:
			goto LOC_COLLISION;
			break;
		case IDT_DEBUG:
			goto LOC_DEBUG;
			break;
		case IDT_POINTERS:
			break;
		case ID_EDITOR_PALED:
			if (!FindWindow(NULL, IDS_PALETTE))
				DialogBoxParam(hInst, MAKEINTRESOURCE(IDD_PAL), 0, (DLGPROC)PalettesProc, NULL);
			break;
		case ID_EDITOR_TILESETED:
			if (!FindWindow(NULL, IDS_TILESET))
				DialogBoxParam(hInst, MAKEINTRESOURCE(IDD_TILESET), 0, (DLGPROC)TilesProc, NULL);
			break;
		case ID_EDITOR_MAPED:
			if (!FindWindow(NULL, IDS_TILEMAP))
				DialogBoxParam(hInst, MAKEINTRESOURCE(IDD_MAP), 0, (DLGPROC)MapProc, NULL);
			break;
		case ID_EDITOR_BLOCKED:
			if (!FindWindow(NULL, IDS_BLOCK))
				DialogBoxParam(hInst, MAKEINTRESOURCE(IDD_BLOCK), 0, (DLGPROC)BlockProc, NULL);
			break;
		case ID_EDITOR_SCENED:
			if (!FindWindow(NULL, IDS_SCENE))
				DialogBoxParam(hInst, MAKEINTRESOURCE(IDD_SCENE), 0, (DLGPROC)SceneProc, NULL);
			break;
		case ID_EDITOR_SPRITED:
			if (!FindWindow(NULL, IDS_SPRITE))
				DialogBoxParam(hInst, MAKEINTRESOURCE(IDD_SPRITE), 0, (DLGPROC)SpriteProc, NULL);
			break;
		case ID_FILE_SETTINGS:
			DialogBoxParam(hInst, MAKEINTRESOURCE(IDD_SETTINGS), hWnd, (DLGPROC)SettingsProc, NULL);
			break;
		case ID_EDITOR_EVENTED:
			DialogBoxParam(hInst, MAKEINTRESOURCE(IDD_EVENT), 0, (DLGPROC)EventProc, NULL);
			break;
		case ID_EDITOR_CHECKPOINTED:
			DialogBoxParam(hInst, MAKEINTRESOURCE(IDD_CHECKPOINT), 0, (DLGPROC)CheckpointProc, NULL);
			break;
		case ID_EDITOR_PROPERTYED:
			DialogBoxParam(hInst, MAKEINTRESOURCE(IDD_PROPERTY), 0, (DLGPROC)PropertyProc, NULL);
			break;
		case IDM_ABOUT:
			//DialogBoxParam(hInst, MAKEINTRESOURCE(IDD_ABOUTBOX), hWnd, (DLGPROC)AboutProc, NULL);
			break;
		case ID_FILE_INTERNALEMULATORSETTINGS:
			DialogBoxParam(hInst, MAKEINTRESOURCE(IDD_INTERNALEMULATORSETTINGS), hWnd, (DLGPROC)InternalEmulatorSettingsProc, NULL);
			break;
		case IDM_EXIT:
			DestroyWindow(hWnd);
			break;
		default:
			return DefWindowProc(hWnd, message, wParam, lParam);
		}
		break;
	case WM_KEYDOWN:
		{
			switch (LOWORD(wParam))
			{
			case VK_UP:
				if (cameraY > 0) cameraY-=16;
				SetScrollPos(hWnd, SB_VERT, cameraY, true);
				InvalidateRect(hWnd, NULL, false);
				break;
			case VK_DOWN:
				if (cameraY < (nmmx.levelHeight<<8)) cameraY+=16;
				SetScrollPos(hWnd, SB_VERT, cameraY, true);
				InvalidateRect(hWnd, NULL, false);
				break;
			case VK_LEFT:
				if (cameraX > 0) cameraX-=8;
				SetScrollPos(hWnd, SB_HORZ, cameraX, true);
				InvalidateRect(hWnd, NULL, false);
				break;
			case VK_RIGHT:
				if (cameraX < (nmmx.levelWidth<<8)) cameraX+=8;
				SetScrollPos(hWnd, SB_HORZ, cameraX, true);
				InvalidateRect(hWnd, NULL, false);
				break;
			case VK_PRIOR: {
				if (nmmx.level > 0) {
					SaveLevel(nmmx.level);
					nmmx.level--;
					set.lastLevel = nmmx.level;
					drawNewLevel = true;
					LoadLevel(nmmx.level);
					drawBackground = false;

					//SetWindowScrollPosition(hWID[0], *nmmx.checkpointInfoTable[0].camX, *nmmx.checkpointInfoTable[0].camY);
				}
				break;
			}
			case VK_NEXT: {
				if (nmmx.level < nmmx.numLevels - 1) {
					SaveLevel(nmmx.level);
					nmmx.level++;
					set.lastLevel = nmmx.level;
					drawNewLevel = true;
					LoadLevel(nmmx.level);
					drawBackground = false;

					//SetWindowScrollPosition(hWID[0], *nmmx.checkpointInfoTable[0].camX, *nmmx.checkpointInfoTable[0].camY);
				}
				break;
			}
			case VK_ESCAPE: {
				Emulator::Instance()->Terminate();
				drawEmu = false;
				RepaintAll();
				break;
			}
			case 'D':
LOC_DEBUG:
				drawLevelInfo = !drawLevelInfo;
				InvalidateRect(hWnd, NULL, false);
				break;
			case 'Q':
				if (nmmx.objLoadOffset > 0)
				{
					nmmx.objLoadOffset--;
					RefreshLevel();
				}
				break;
			case 'W':
				//if (*nmmx.checkpointInfoTable[nmmx.point].objLoad + nmmx.objLoadOffset < 0x40)
				//{
				//	nmmx.objLoadOffset++;
				//	RefreshLevel();
				//}
				break;
			case 'A':
				if (nmmx.tileLoadOffset > 0)
				{
					nmmx.tileLoadOffset--;
					RefreshLevel();
				}
				break;
			case 'S':
				//if (*nmmx.checkpointInfoTable[nmmx.point].tileLoad + nmmx.tileLoadOffset < nmmx.numDecs)
				//{
				//	nmmx.tileLoadOffset++;
				//	RefreshLevel();
				//}
				break;
			case 'Z':
				if (nmmx.palLoadOffset > 0)
				{
					nmmx.palLoadOffset--;
					RefreshLevel();
				}
				break;
			case 'X':
				//if (*nmmx.checkpointInfoTable[nmmx.point].palLoad + nmmx.palLoadOffset < 0x40)
				//{
				//	nmmx.palLoadOffset++;
				//	RefreshLevel();
				//}
				break;
			case 'B':
LOC_BACKGROUND:
				drawBackground = !drawBackground;

				UpdateWindow();
				RefreshLevel();
				break;
			case 'O':
				if (nmmx.point > 0) {
					nmmx.SetLevel(nmmx.level, nmmx.point - 1);

					nmmx.objLoadOffset = 0;
					nmmx.tileLoadOffset = 0;
					nmmx.palLoadOffset = 0;

					nmmx.LoadTilesAndPalettes();
					render.RefreshMapCache();
					RepaintAll();
				}
				break;
			case 'P':
				if (nmmx.point < nmmx.numCheckpoints - 1) {
					nmmx.SetLevel(nmmx.level, nmmx.point + 1);

					nmmx.objLoadOffset = 0;
					nmmx.tileLoadOffset = 0;
					nmmx.palLoadOffset = 0;

					nmmx.LoadTilesAndPalettes();
					render.RefreshMapCache();
					RepaintAll();
				}
				break;
			case 'E':
LOC_COLLISION:
				drawCollisionIndex = !drawCollisionIndex;
				RepaintAll();
				break;
			case 'C':
				drawCheckpointInfo = !drawCheckpointInfo;
				//InvalidateRect(hWnd, NULL, false);
				RepaintAll();
				break;
			case 'T':
				drawEventInfo = !drawEventInfo;
				RepaintAll();
				break;
			case VK_F1:
			case VK_F2:
			case VK_F3:
			case VK_F4:
			case VK_F5:
			case VK_F6:
			case VK_F7:
			case VK_F8:
			case VK_F9:
			case VK_F10: {
				if (drawEmu) {
					saveState = LOWORD(wParam) - VK_F1;
					saveStateSave = (GetKeyState(VK_SHIFT) >> 15) ? true : false;
					saveStateCount = 100;
					Emulator::Instance()->UpdateSaveState(saveStateSave, saveState);
					if (!saveStateSave) drawForceEmu = true;
					RepaintAll();
				}
				break;
			}
			case VK_F12: {
				Emulator::Instance()->Terminate();
				ResetRepaintEmu();
				Emulator::Instance()->Init();
				Emulator::Instance()->LoadRom(nmmx.rom, nmmx.romSize);
				Emulator::Instance()->LoadLevel(nmmx.level);

				saveStateCount = 0;
				saveState = 0;
				drawEmu = true;
				break;
			}
			case VK_SPACE: {
				if (drawEmu) {
					Emulator::Instance()->Pause();
				}
				break;
			}
			}

			if (LOWORD(wParam) == MapVirtualKey(set.emulatorButtons[unsigned(InternalEmulatorButtonType::NEXTSTATE)].value >> 16, MAPVK_VSC_TO_VK_EX)) {
				if (saveState < 9) saveState++;
			}
			else if (LOWORD(wParam) == MapVirtualKey(set.emulatorButtons[unsigned(InternalEmulatorButtonType::PREVSTATE)].value >> 16, MAPVK_VSC_TO_VK_EX)) {
				if (saveState > 0) saveState--;
			}
			else if (LOWORD(wParam) == MapVirtualKey(set.emulatorButtons[unsigned(InternalEmulatorButtonType::SAVESTATE)].value >> 16, MAPVK_VSC_TO_VK_EX)) {
				saveStateSave = true;
				Emulator::Instance()->UpdateSaveState(saveStateSave, saveState);
				saveStateCount = 100;
			}
			else if (LOWORD(wParam) == MapVirtualKey(set.emulatorButtons[unsigned(InternalEmulatorButtonType::LOADSTATE)].value >> 16, MAPVK_VSC_TO_VK_EX)) {
				saveStateSave = false;
				Emulator::Instance()->UpdateSaveState(saveStateSave, saveState);
				drawForceEmu = true;
				saveStateCount = 100;
				RepaintAll();
			}
		}
		break;
	case WM_SYSCOMMAND:
		if (wParam == SC_KEYMENU && lParam == 0) {
			SendMessage(hWID[0], WM_KEYDOWN, VK_F10, NULL);
		}
		else {
			if (wParam == SC_CLOSE)
				set.max = IsZoomed(hWnd);
			return DefWindowProc(hWnd, message, wParam, lParam);
		}
	case Emulator::Message::VIDEOREFRESH: {
		if (drawEmu) {
			FrameState s;
			Emulator::Instance()->GetFrameState(s);
			if (saveStateCount) --saveStateCount;
			RepaintEmu(s.xpos, s.ypos, s.graphicsNum, s.tileNum, s.paletteNum, s.levelNum);
		}
		break;
	}
	case DrawThreadMessage::DRAWREFRESH: {
		DrawED::Instance()->AcquireFromDraw();
		currentBuffer = lParam;
		InvalidateRect(hWID[0], NULL, false);
		break;
	}
	case WM_CREATE: {
		hWID[0] = hWnd;

		//MessageBox(hWID[0], "WM_CREATE", "Test", MB_ICONERROR);

		hBrushBlack = CreateSolidBrush(RGB(0, 0, 0));
		CreateBitmapCache();
		bmpTeleport = LoadBitmap(hInst, MAKEINTRESOURCE(IDB_TELEPORT));
		bmpXSprite = LoadBitmap(hInst, MAKEINTRESOURCE(IDB_XSPRITE));

		//MessageBox(hWID[0], "CREATE TOOLBAR", "Test", MB_ICONERROR);

		tb.Create(hWnd, hInst, IDB_TOOLBAR, IDT_TOOLBAR);
		tb.AddButton(TBSTYLE_SEP, 0);
		tb.AddButton(TBSTYLE_BUTTON, ID_FILE_OPEN);
		tb.AddButton(TBSTYLE_BUTTON, ID_FILE_SAVE);
		tb.AddButton(TBSTYLE_SEP, 0);
		tb.AddButton(TBSTYLE_BUTTON, IDT_PLAY);
		tb.AddButton(TBSTYLE_BUTTON, ID_FILE_SETTINGS);
		//tb.AddButton(TBSTYLE_BUTTON, ID_FILE_OFFSETS);
		//tb.AddButton(TBSTYLE_SEP, 0);
		//tb.AddButton(TBSTYLE_BUTTON, IDT_BACKGROUND);
		//tb.AddButton(TBSTYLE_BUTTON, IDT_COLLISION);
		//tb.AddButton(TBSTYLE_BUTTON, IDT_DEBUG);
		//tb.AddButton(TBSTYLE_BUTTON, IDT_POINTERS);
		//tb.AddButton(TBSTYLE_SEP, 0);
		//tb.AddButton(TBSTYLE_BUTTON, ID_EDITOR_PALED);
		//tb.AddButton(TBSTYLE_BUTTON, ID_EDITOR_TILESETED);
		//tb.AddButton(TBSTYLE_BUTTON, ID_EDITOR_MAPED);
		//tb.AddButton(TBSTYLE_BUTTON, ID_EDITOR_BLOCKED);		tb.AddButton(TBSTYLE_BUTTON, ID_EDITOR_SCENED);

		//MessageBox(hWID[0], "DISPLAY TOOLBAR", "Test", MB_ICONERROR);
		tb.Display();

		//MessageBox(hWID[0], "CHECKING ROMATSTARTUP", "Test", MB_ICONERROR);
		if (set.romAtStartup)
		{
			//MessageBox(hWID[0], "LOADING ROMATSTARTUP", "Test", MB_ICONERROR);

			if (IsFileExist(set.lastroms[0]) && nmmx.LoadNewRom(set.lastroms[0]))
			{
				if (!nmmx.CheckROM())
					return 0;
				strcpy(nmmx.filePath, set.lastroms[0]);
				SetWindowTitle(hWnd, set.lastroms[0]);
				nmmx.LoadFont();
				render.Init(&nmmx);
				render.CreateMapCache(hWnd);
				render.CreateFontCache(hWnd);
				render.RefreshFontCache();
				LoadLevel(set.lastLevel);
				SetScrollRange(hWnd, SB_HORZ, 0, (nmmx.levelWidth << 8), true);
				SetScrollRange(hWnd, SB_VERT, 0, (nmmx.levelHeight << 8), true);

				EnableMenuItem(GetMenu(hWnd), 1, MF_BYPOSITION | MF_ENABLED);
			}
		}

		//MessageBox(hWID[0], "DRAW THREAD START", "Test", MB_ICONERROR);

		auto mainThreadId = GetCurrentThreadId();
		auto p = new DrawThreadParam;
		p->id = mainThreadId;
		p->h = CreateEvent(NULL, 0, 0, NULL);
		DrawED::Instance()->ReleaseToDraw();
		drawThreadHandle = CreateThread(NULL, 0, DrawED::DrawThreadProc, p, 0, &drawThreadId);
		WaitForSingleObject(p->h, INFINITE);
		delete p;

		//MessageBox(hWID[0], "DRAW THREAD END", "Test", MB_ICONERROR);

		break;
	}
	case WM_DESTROY: {
		// kill the emulator if it's still running
		Emulator::Instance()->Terminate();
		drawEmu = false;

		PostThreadMessage(drawThreadId, DrawThreadMessage::QUIT, 0, 0);
		WaitForSingleObject(drawThreadHandle, INFINITE);
		CloseHandle(drawThreadHandle);

		DeleteObject(levelBuffer[0]);
		DeleteObject(levelBuffer[1]);
		DeleteObject(eventBuffer[0]);
		DeleteObject(eventBuffer[1]);

		DeleteObject(bmpTeleport);
		DeleteObject(bmpXSprite);
		DeleteObject(hBrushBlack);
		SaveSettings();
		PostQuitMessage(0);
		break;
	}
	case WM_HSCROLL:
	case WM_VSCROLL:
	{
		ScrollProc(hWnd, message == 0x115, wParam, message == WM_HSCROLL ? &cameraX : &cameraY);
		if (cameraY > (nmmx.levelHeight<<8)) cameraY = nmmx.levelHeight<<8;
		if (cameraX > (nmmx.levelWidth << 8)) cameraX = nmmx.levelWidth<<8;
		InvalidateRect(hWnd, NULL, false);
		break;
	}
	default:
		return DefWindowProc(hWnd, message, wParam, lParam);
	}
	return 0;
}
