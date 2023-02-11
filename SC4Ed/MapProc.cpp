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

#define ID_SP_COLLISION     6000

//00: Background
//01: 14Åã Uphill 1
//02: 14Åã Uphill 2
//03: 14Åã Downhill 1
//04: 14Åã Downhill 2
//05: 07Åã Uphill 1
//06: 07Åã Uphill 2
//07: 07Åã Uphill 3
//08: 07Åã Uphill 4
//09: 07Åã Downhill 1
//0A: 07Åã Downhill 2
//0B: 07Åã Downhill 3
//0C: 07Åã Downhill 4
//0D: Water
//0E: Water Surface
//11: Mud (Sting Chameleon)
//12: Ladder
//13: End of ladder
//34: Solid (Used in X1 intro stage, seems the same as 3B)
//35: Solid (Used in X1 intro stage, seems the same as 3B)
//36: Solid (Can't climb)
//37: Solid, Conveyor (left)
//38: Solid, Conveyor (right)
//39: Solid, goes below uphill tiles (both 1 and 2)
//3A: Solid, goes below downhill tiles (both 1 and 2)
//3B: Solid
//3C: Breakable blocks (X1, with head / leg upgrade)
//3D: Doors
//3E: Non-lethal Spikes
//3F: Lethal Spikes
//40 Bitflag: Conveyors
//80 Bitflag: Slippery (Crystal Snail, Blizzard Buffalo)
//	Examples:	83: 14Åã Downhill 1, Slippery
//			84: 14Åã Downhill 2, Slippery
//			BA: Solid, goes below slippery downhill tiles
//			BB: Slippery floor (Blizzard Buffalo)
//45: 14Åã, 1/4, Conveyor (left)
//46: 14Åã, 2/4, Conveyor (left)
//47: 14Åã, 3/4, Conveyor (left)
//48: 14Åã, 4/4, Conveyor (left)
//49: 14Åã, 1/4, Conveyor (right)
//4A: 14Åã, 2/4, Conveyor (right)
//4B: 14Åã, 3/4, Conveyor (right)
//4C: 14Åã, 4/4, Conveyor (right)

// Control variables:
ScrollBar mapScroll;
SpinBox mapTilePalette, mapTileIndex, mapCollision;

// General variables:
BYTE selection = 0;
WORD dMapSelected=0;
WORD dMapOverMouse=0;
WORD dMapTextWrite=0;

// Drawing variables:
HDC hMapDC, hMapBack;
HBITMAP backBufferMapProc;

RECT rMapEdit = {288, 32, 288 + 64, 32 + 64};
RECT rectMapTileFocus[] = {288, 32, 288+32, 32+32,
	288+32, 32, 288+64, 32+32,
	288, 32+32, 288+32, 32+64,
	288+32, 32+32, 288+64, 32+64};

static void UpdateMapEdit(HWND hWnd, BYTE sel)
{
	WORD map = *(LPWORD)(nmmx.rom + nmmx.pMaps + (dMapSelected<<3) + sel*2);
	mapCollision.SetPos(*(LPBYTE)(nmmx.rom + nmmx.pCollisions + dMapSelected), true);
	mapTileIndex.SetPos(map & 0x3FF, true);
	mapTilePalette.SetPos((map >> 10) & 7, true);
	SendMessage(GetDlgItem(hWnd, IDC_MAPUPLAYER),BM_SETCHECK, map & 0x2000 ? BST_CHECKED : BST_UNCHECKED, 0);
	SendMessage(GetDlgItem(hWnd, IDC_MAPMIRROR), BM_SETCHECK, map & 0x4000 ? BST_CHECKED : BST_UNCHECKED, 0);
	SendMessage(GetDlgItem(hWnd, IDC_MAPFLIP),   BM_SETCHECK, map & 0x8000 ? BST_CHECKED : BST_UNCHECKED, 0);
	InvalidateRect(hWnd, NULL, false);
}

static void UpdateMapWrite(HWND hWnd)
{
	WORD map = 0;
	map |= mapTileIndex.GetPos();
	map |= mapTilePalette.GetPos() << 10;
	map |= SendMessage(GetDlgItem(hWnd, IDC_MAPUPLAYER), BM_GETCHECK, NULL, NULL) == BST_CHECKED ? 0x2000 : 0;
	map |= SendMessage(GetDlgItem(hWnd, IDC_MAPMIRROR), BM_GETCHECK, NULL, NULL) == BST_CHECKED ? 0x4000 : 0;
	map |= SendMessage(GetDlgItem(hWnd, IDC_MAPFLIP), BM_GETCHECK, NULL, NULL) == BST_CHECKED ? 0x8000 : 0;
	*(LPBYTE)(nmmx.rom + nmmx.pCollisions + dMapSelected) = mapCollision.GetPos();
	*(LPWORD)(nmmx.rom + nmmx.pMaps + (dMapSelected<<3) + selection*2) = map;
	render.RefreshMapCache(dMapSelected);
	InvalidateRect(hWnd, NULL, false);
}

BOOL CALLBACK MapProc(HWND hWnd, UINT message, WPARAM wParam, LPARAM lParam)
{
	PAINTSTRUCT ps;
	
	switch (message)
	{
	case UPDATEWINDOW: {
		dMapSelected = 0;
		dMapOverMouse = 0;
		dMapTextWrite = 0;
		UpdateMapEdit(hWnd, 0);

		CHAR sIndex[11];
		wsprintf(sIndex, "Map N. %03X", dMapOverMouse);
		SetWindowText(GetDlgItem(hWnd, IDC_LMAPINDEX), sIndex);
		break;
	}
	case WM_COMMAND: {
		// this will be called when any of the edit boxes update.  Because of this, we need to be careful
		// to only save values for the particular edit box that sent the message.  There are times when other edit
		// boxes are in an intermediate state and won't have the correct value.
		bool found = false;
		switch (LOWORD(wParam))
		{
		case IDC_MAPUPLAYER: {
			found = true;
			WORD map = *(LPWORD)(nmmx.rom + nmmx.pMaps + (dMapSelected << 3) + selection * 2);
			map &= ~0x2000;
			map |= SendMessage(GetDlgItem(hWnd, IDC_MAPUPLAYER), BM_GETCHECK, NULL, NULL) == BST_CHECKED ? 0x2000 : 0;
			*(LPWORD)(nmmx.rom + nmmx.pMaps + (dMapSelected << 3) + selection * 2) = map;
			break;
		}
		case IDC_MAPMIRROR: {
			found = true;
			WORD map = *(LPWORD)(nmmx.rom + nmmx.pMaps + (dMapSelected << 3) + selection * 2);
			map &= ~0x4000;
			map |= SendMessage(GetDlgItem(hWnd, IDC_MAPMIRROR), BM_GETCHECK, NULL, NULL) == BST_CHECKED ? 0x4000 : 0;
			*(LPWORD)(nmmx.rom + nmmx.pMaps + (dMapSelected << 3) + selection * 2) = map;
			break;
		}
		case IDC_MAPFLIP: {
			found = true;
			WORD map = *(LPWORD)(nmmx.rom + nmmx.pMaps + (dMapSelected << 3) + selection * 2);
			map &= ~0x8000;
			map |= SendMessage(GetDlgItem(hWnd, IDC_MAPFLIP), BM_GETCHECK, NULL, NULL) == BST_CHECKED ? 0x8000 : 0;
			*(LPWORD)(nmmx.rom + nmmx.pMaps + (dMapSelected << 3) + selection * 2) = map;
			break;
		}
		default: {
			if (mapTileIndex.IsIDEqual((long)lParam))
			{
				found = true;
				mapTileIndex.Work(wParam);
				WORD map = *(LPWORD)(nmmx.rom + nmmx.pMaps + (dMapSelected << 3) + selection * 2);
				map &= ~0x03FF;
				map |= mapTileIndex.GetPos() & 0x3FF;
				*(LPWORD)(nmmx.rom + nmmx.pMaps + (dMapSelected << 3) + selection * 2) = map;
			}
			else if (mapTilePalette.IsIDEqual((long)lParam))
			{
				found = true;
				mapTilePalette.Work(wParam);
				WORD map = *(LPWORD)(nmmx.rom + nmmx.pMaps + (dMapSelected << 3) + selection * 2);
				map &= ~0x1C00;
				map |= (mapTilePalette.GetPos() & 0x7) << 10;
				*(LPWORD)(nmmx.rom + nmmx.pMaps + (dMapSelected << 3) + selection * 2) = map;
			}
			else if (mapCollision.IsIDEqual((long)lParam))
			{
				found = true;
				mapCollision.Work(wParam);
				*(LPBYTE)(nmmx.rom + nmmx.pCollisions + dMapSelected) = mapCollision.GetPos();
			}
		}
		}

		if (found) {
			render.RefreshMapCache(dMapSelected);
			RepaintAll();
		}
		break;
	}
	case WM_VSCROLL:

		if (mapScroll.IsIDEqual((long)lParam))
		{
			mapScroll.Work(wParam);
			if (LOWORD(wParam) != SB_ENDSCROLL)
			{
				RECT rect;
				rect.top = 0;
				rect.left = 0;
				rect.bottom = 256;
				rect.right = 256;
				InvalidateRect(hWnd, &rect, false);
			}
		}
		else if (mapTileIndex.IsIDEqual((long)lParam))
		{
			mapTileIndex.Work(wParam);
			UpdateMapWrite(hWnd);
		}
		else if (mapTilePalette.IsIDEqual((long)lParam))
		{
			mapTilePalette.Work(wParam);
			UpdateMapWrite(hWnd);
		}
		else if (mapCollision.IsIDEqual((long)lParam))
		{
			mapCollision.Work(wParam);
			UpdateMapWrite(hWnd);
		}
		break;
	case WM_LBUTTONDOWN:
		{
			if ((lParam & 0xFFFF) < 0x0100)
			{
				dMapTextWrite = false;
				dMapSelected = dMapOverMouse;
				selection = 0;
				InvalidateRect(hWnd, &rMapEdit, false);
			}
			else if (LOWORD(lParam) >= 288 && LOWORD(lParam) < 288+64)
			{
				if (HIWORD(lParam) >= 32 && HIWORD(lParam) < 32+64)
				{
					selection  = LOWORD(lParam) >= 288+32 ? 1 : 0;
					selection |= HIWORD(lParam) >=  32+32 ? 2 : 0;
				}
			}
			UpdateMapEdit(hWnd, selection);
		}
		break;
	case WM_MOUSEMOVE:
		if ((lParam & 0xFFFF) < 0x0100)
		{
			auto tempMap = (WORD)(((lParam>>4)&0xF) | ((lParam>>16)&0xF0) + (mapScroll.GetPos()<<4));
			if (tempMap < nmmx.numMaps) {
				dMapOverMouse = tempMap;
			}
			goto PRINTEXT;
		}
		else
		{
			if (dMapOverMouse == dMapSelected)
				break;
			dMapOverMouse = dMapSelected;
			goto PRINTEXT;
		}
		if (false)
		{
PRINTEXT:
			dMapTextWrite = true;
			CHAR sIndex[11];
			wsprintf(sIndex, "Map N. %03X", dMapOverMouse);
			SetWindowText(GetDlgItem(hWnd, IDC_LMAPINDEX), sIndex);
		}
		break;
	case WM_MOVE:
		RECT rc;
		GetWindowRect(hWnd, &rc);
		set.mapED.X = (SHORT)rc.left;
		set.mapED.Y = (SHORT)rc.top;
	case WM_PAINT:
		hMapDC = BeginPaint(hWnd, &ps);
		hMapBack = CreateCompatibleDC(hMapDC);
		SelectObject(hMapBack, backBufferMapProc);
		
		render.RenderMap(hMapDC, 18, 2, dMapSelected);
		StretchBlt(hMapDC, 288, 32, 64, 64, hMapDC, 288, 32, 16, 16, SRCCOPY);

		for (int i = 0; i < 0x100; i++) {
			unsigned mapNum = (mapScroll.GetPos() << 4) + i;
			render.RenderMap(hMapBack, i & 0xF, i >> 4, mapNum < nmmx.numMaps ? (mapScroll.GetPos() << 4) + i : 0);
		}

		BitBlt(hMapDC, 0, 0, 256, 256, hMapBack, 0, 0, SRCCOPY);
		DeleteDC(hMapBack);
		DrawFocusRect(hMapDC, rectMapTileFocus + selection);
		EndPaint(hWnd, &ps);
		break;
	case WM_INITDIALOG:
	{
		dMapSelected = 0;
		int sel = 0;

		hWID[3] = hWnd;
		backBufferMapProc = CreateBitmapCache(hWnd, 256, 256);
		mapScroll.Create(hWnd, 257, 0, 256);
		mapScroll.SetRange(0, (nmmx.numMaps > 0x100) ? (nmmx.numMaps - 1) / 0x10 + 1 - 0x10 : 0);
		mapCollision.Create  (hWnd, ID_SP_COLLISION,     300, 100, 62, 0, 0xFF);
		mapTileIndex.Create  (hWnd, 0x9001,   300, 150, 62, 0, nmmx.numTiles);
		mapTilePalette.Create(hWnd, 0x9002, 300, 176, 62, 0, 7);

		UpdateMapEdit(hWnd, 0);

		break;
	}
	case WM_SHOWWINDOW:
		SetWindowPosition(hWnd, set.mapED.X, set.mapED.Y);
		break;
	case WM_CLOSE:
		DeleteObject(backBufferMapProc);
		EndDialog(hWnd, 0);
		break;
	}
	return 0; 
}