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
#include <sstream>

// Control variables:
static ScrollBar mapScroll, tileScroll;
static SpinBox mapTilePalette, mapTileIndex, mapCollision;

// General variables:
static BYTE selection = 0;
static BYTE dSelectionOverMouse = 0;
static WORD dMapSelected = 0;
static WORD dMapOverMouse = 0;
static WORD dMapTextWrite = 0;
static WORD dMapCopy = -1;

static WORD dTileSelected = 0;
static WORD dTileSave = 0;
static WORD dTileOverMouse = 0;
static WORD dTileTextWrite = 0;

// Drawing variables:
static HDC hMapDC, hMapBack, hTileBack, hRenderBack;
static HBITMAP backBufferMapProc, backBufferTileProc, backBufferRenderProc;

static RECT rectTileFocus;
static RECT rectMapFocus;

static RECT rMapEdit = { 288, 20, 288 + 63, 20 + 63 };
static RECT rectTileSave = { 330, 124, 330 + 23, 124 + 23 };

static bool dIgnoreRefresh = false;

static WORD GetMap(WORD block, BYTE sel) {
	unsigned blockMap = block;
	unsigned blockOffset = ((blockMap & (nmmx.numBlocks - 1)) << 5) + (nmmx.type == 0 ? 0x2000 : nmmx.type == 1 ? 0x1000 : 0x3000) + nmmx.mapBase /* FE */;
	unsigned tileOffset = sel << 1;

	return *LPWORD(nmmx.ram + blockOffset + tileOffset);

}

static void SetMap(WORD block, BYTE sel, WORD map) {
	if (nmmx.expandedROM && nmmx.expandedOffset.count(nmmx.level) && nmmx.expandedOffset[nmmx.level].count(0x7E2000 + nmmx.mapBaseAdder + nmmx.mapBase)) {
		unsigned blockMap = dMapSelected;
		unsigned blockOffset = ((blockMap & (nmmx.numBlocks - 1)) << 5);

		*(LPWORD)(nmmx.rom + nmmx.expandedOffset[nmmx.level][0x7E2000 + nmmx.mapBase].first + blockOffset + 2 * sel) = map;
	}
}

static void UpdateMapEdit(HWND hWnd, BYTE sel)
{
	dIgnoreRefresh = true;
	WORD map = GetMap(dMapSelected, sel);
	//mapCollision.SetPos(*(LPBYTE)(nmmx.rom + nmmx.pCollisions + dMapSelected), true);
	mapTileIndex.SetPos(map & 0x3FF, true);
	mapTilePalette.SetPos((map >> 10) & 7, true);
	SendMessage(GetDlgItem(hWnd, IDC_MAPUPLAYER), BM_SETCHECK, map & 0x2000 ? BST_CHECKED : BST_UNCHECKED, 0);
	SendMessage(GetDlgItem(hWnd, IDC_MAPMIRROR), BM_SETCHECK, map & 0x4000 ? BST_CHECKED : BST_UNCHECKED, 0);
	SendMessage(GetDlgItem(hWnd, IDC_MAPFLIP), BM_SETCHECK, map & 0x8000 ? BST_CHECKED : BST_UNCHECKED, 0);
	dIgnoreRefresh = false;
}

//static void UpdateMapWrite(HWND hWnd, BYTE sel)
//{
//	WORD map = 0;
//	map |= mapTileIndex.GetPos();
//	map |= mapTilePalette.GetPos() << 10;
//	map |= SendMessage(GetDlgItem(hWnd, IDC_MAPUPLAYER), BM_GETCHECK, NULL, NULL) == BST_CHECKED ? 0x2000 : 0;
//	map |= SendMessage(GetDlgItem(hWnd, IDC_MAPMIRROR), BM_GETCHECK, NULL, NULL) == BST_CHECKED ? 0x4000 : 0;
//	map |= SendMessage(GetDlgItem(hWnd, IDC_MAPFLIP), BM_GETCHECK, NULL, NULL) == BST_CHECKED ? 0x8000 : 0;
//	//*(LPBYTE)(nmmx.rom + nmmx.pCollisions + dMapSelected) = mapCollision.GetPos();
//	//*(LPWORD)(nmmx.rom + nmmx.pMaps + (dMapSelected << 3) + sel * 2) = map;
//	SetMap(dMapSelected, selection, map);
//}

BOOL CALLBACK BlockProc(HWND hWnd, UINT message, WPARAM wParam, LPARAM lParam)
{
	PAINTSTRUCT ps;

	auto pt = MAKEPOINTS(lParam);
	POINT p = { pt.x, pt.y };

	switch (message)
	{
	case UPDATEWINDOW: {
		selection = 0;
		dSelectionOverMouse = 0;
		dMapSelected = 0;
		dMapOverMouse = 0;
		dMapTextWrite = 0;
		dMapCopy = -1;

		dTileSelected = 0;
		dTileSave = 0;
		dTileOverMouse = 0;
		dTileTextWrite = 0;

		mapScroll.SetPos(0);
		tileScroll.SetPos(0);

		UpdateMapEdit(hWnd, 0);

		CHAR sIndex[13];
		sprintf(sIndex, "Block N. %03X", dMapOverMouse);
		SetWindowText(GetDlgItem(hWnd, IDC_LMAPINDEX), sIndex);
		sprintf(sIndex, "Tile N. %03X", dTileOverMouse);
		SetWindowText(GetDlgItem(hWnd, IDC_LMAPTILEINDEX), sIndex);

		break;
	}
	case WM_COMMAND: {
		// this will be called when any of the edit boxes update.  Because of this, we need to be careful
		// to only save values for the particular edit box that sent the message.  There are times when other edit
		// boxes are in an intermediate state and won't have the correct value.
		bool found = false;
		switch (LOWORD(wParam))
		{
		case IDC_MAP_COPY: {
			dMapCopy = dMapSelected;
			break;
		}
		case IDC_MAP_PASTE: {
			if (dMapCopy != -1) {
				found = true;
				for (unsigned i = 0; i < 16; i++) {
					WORD map = GetMap(dMapCopy, i);
					SetMap(dMapSelected, i, map);
				}
			}
			break;
		}
		case IDC_BLOCK_TYPE_COMBO: {
			InvalidateRect(hWnd, NULL, false);
			break;
		}
		case IDC_MAPUPLAYER: {
			found = true;
			//UpdateMapWrite(hWnd, selection);
			WORD map = GetMap(dMapSelected, selection);
			map &= ~0x2000;
			map |= SendMessage(GetDlgItem(hWnd, IDC_MAPUPLAYER), BM_GETCHECK, NULL, NULL) == BST_CHECKED ? 0x2000 : 0;
			SetMap(dMapSelected, selection, map);
			break;
		}
		case IDC_MAPMIRROR: {
			found = true;
			//UpdateMapWrite(hWnd, selection);
			WORD map = GetMap(dMapSelected, selection);
			map &= ~0x4000;
			map |= SendMessage(GetDlgItem(hWnd, IDC_MAPMIRROR), BM_GETCHECK, NULL, NULL) == BST_CHECKED ? 0x4000 : 0;
			SetMap(dMapSelected, selection, map);
			break;
		}
		case IDC_MAPFLIP: {
			found = true;
			//UpdateMapWrite(hWnd, selection);
			WORD map = GetMap(dMapSelected, selection);
			map &= ~0x8000;
			map |= SendMessage(GetDlgItem(hWnd, IDC_MAPFLIP), BM_GETCHECK, NULL, NULL) == BST_CHECKED ? 0x8000 : 0;
			SetMap(dMapSelected, selection, map);
			break;
		}
		default: {
			if (mapTileIndex.IsIDEqual((long)lParam))
			{
				found = true;
				mapTileIndex.Work(wParam);
				WORD map = GetMap(dMapSelected, selection);
				map &= ~0x03FF;
				map |= mapTileIndex.GetPos() & 0x3FF;
				SetMap(dMapSelected, selection, map);
			}
			else if (mapTilePalette.IsIDEqual((long)lParam))
			{
				found = true;
				mapTilePalette.Work(wParam);
				WORD map = GetMap(dMapSelected, selection);
				map &= ~0x1C00;
				map |= (mapTilePalette.GetPos() & 0x7) << 10;
				SetMap(dMapSelected, selection, map);
			}
			//else if (mapCollision.IsIDEqual((long)lParam))
			//{
			//	found = true;
			//	mapCollision.Work(wParam);
			//	*(LPBYTE)(nmmx.rom + nmmx.pCollisions + dMapSelected) = mapCollision.GetPos();
			//}
			break;
		}
		}

		//if (found) {
		//	render.RefreshMapCache(dMapSelected);
		//	RepaintAll();
		//}

		if (!dIgnoreRefresh && found) {
			RefreshLevel();
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
		else if (tileScroll.IsIDEqual((long)lParam))
		{
			tileScroll.Work(wParam);
			if (LOWORD(wParam) != SB_ENDSCROLL)
			{
				RECT rect;
				rect.top = 0;
				rect.left = 380;
				rect.bottom = 256;
				rect.right = 380 + 256;
				//InvalidateRect(hWnd, &rect, false);
				InvalidateRect(hWnd, NULL, false);
			}
		}
		else if (mapTileIndex.IsIDEqual((long)lParam))
		{
			mapTileIndex.Work(wParam);
		}
		else if (mapTilePalette.IsIDEqual((long)lParam))
		{
			mapTilePalette.Work(wParam);
		}
		else if (mapCollision.IsIDEqual((long)lParam))
		{
			mapCollision.Work(wParam);
		}
		break;
	case WM_LBUTTONDOWN: {
		if (PtInRect(&rMapEdit, p))
		{
			if (selection != dSelectionOverMouse) {
				selection = dSelectionOverMouse;
				UpdateMapEdit(hWnd, selection);
			}

			WORD map = GetMap(dMapSelected, selection);
			if ((map & 0x3FF) != dTileSelected) {
				dTileSave = map & 0x3FF;
			}

			mapTileIndex.SetPos(dTileSelected);
			//UpdateMapWrite(hWnd, selection);
			//render.RefreshMapCache(dMapSelected);
			RepaintAll();
		}
		break;
	}
	case WM_RBUTTONDOWN:
	{
		if (pt.x < 0x100 && pt.y < 0x100)
		{
			dMapTextWrite = false;
			dMapSelected = dMapOverMouse;
			selection = 0;

			UpdateMapEdit(hWnd, selection);

			InvalidateRect(hWnd, NULL, false);
		}
		else if (pt.x >= 380 && pt.y < 0x100)
		{
			dTileTextWrite = false;
			dTileSelected = dTileOverMouse;

			InvalidateRect(hWnd, NULL, false);
		}
		else if (PtInRect(&rMapEdit, p))
		{
			if (selection != dSelectionOverMouse) {
				selection = dSelectionOverMouse;
				UpdateMapEdit(hWnd, selection);
			}
			else {
				dTileOverMouse = mapTileIndex.GetPos();
				dTileSelected = dTileOverMouse;
			}

			UpdateMapEdit(hWnd, selection);
			tileScroll.SetPos(dTileSelected >> 4);
			RepaintAll();
		}
		else if (PtInRect(&rectTileSave, p)) {
			dTileOverMouse = dTileSave;
			dTileSelected = dTileOverMouse;
			InvalidateRect(hWnd, NULL, false);
		}
	}
	break;
	case WM_MOUSEMOVE:
		if (pt.x < 0x100 && pt.y < 0x100)
		{
			auto tempMap = (WORD)(((pt.x) >> 5) + (pt.y >> 5 << 3) + (mapScroll.GetPos() << 3)); 
			if (tempMap < nmmx.numBlocks) {
				dMapOverMouse = tempMap;
			}
			goto PRINTEXT;
		}
		else if (pt.x >= 380 && pt.y < 0x100)
		{
			auto tempTile = (WORD)(((pt.x - 380) >> 4) + (pt.y >> 4 << 4) + (tileScroll.GetPos() << 4));
			if (tempTile < 0x400) {
				dTileOverMouse = tempTile;
			}
			goto PRINTEXT;
		}
		else if (PtInRect(&rMapEdit, p))
		{
			p.x -= rMapEdit.left;
			p.y -= rMapEdit.top;

			unsigned s = (p.x / 16) + (p.y / 16) * 4;
			if (s != dSelectionOverMouse) {
				dSelectionOverMouse = s;
				//UpdateMapEdit(hWnd, selection);
				//InvalidateRect(hWnd, NULL, false);
			}
		}
		else
		{
			if (dMapOverMouse == dMapSelected && dTileOverMouse == dTileSelected)
				break;
			dMapOverMouse = dMapSelected;
			dTileOverMouse = dTileSelected;
			goto PRINTEXT;
		}
		if (false)
		{
		PRINTEXT:
			dMapTextWrite = true;
			CHAR sIndex[15];
			sprintf(sIndex, "Block N. %03X", dMapOverMouse);
			SetWindowText(GetDlgItem(hWnd, IDC_LMAPINDEX), sIndex);
			sprintf(sIndex, "Tile N. %03X", dTileOverMouse);
			SetWindowText(GetDlgItem(hWnd, IDC_LMAPTILEINDEX), sIndex);
		}
		break;
	case WM_MOVE:
		RECT rc;
		GetWindowRect(hWnd, &rc);
		set.mapED.X = (SHORT)rc.left;
		set.mapED.Y = (SHORT)rc.top;
	case WM_PAINT: {
		hMapDC = BeginPaint(hWnd, &ps);
		hMapBack = CreateCompatibleDC(hMapDC);
		SelectObject(hMapBack, backBufferMapProc);
		hTileBack = CreateCompatibleDC(hMapDC);
		SelectObject(hTileBack, backBufferTileProc);
		hRenderBack = CreateCompatibleDC(hMapDC);
		SelectObject(hRenderBack, backBufferRenderProc);

		render.RenderBlock(hRenderBack, 0, 0, dMapSelected);
		StretchBlt(hMapDC, 288, 20, 64, 64, hRenderBack, 0, 0, 32, 32, SRCCOPY);

		for (int i = 0; i < 0x100; i++) {
			unsigned mapNum = (mapScroll.GetPos() << 3) + i;
			render.RenderBlock(hMapBack, i & 0x7, i >> 3, mapNum < nmmx.numBlocks ? (mapScroll.GetPos() << 3) + i : 0);
		}

		auto sel = SendMessage(GetDlgItem(hWnd, IDC_BLOCK_TYPE_COMBO), CB_GETCURSEL, (WPARAM)0, (LPARAM)0);
		auto it = TileTypeMap.begin();
		std::advance(it, sel);

		WORD map = GetMap(dMapSelected, selection);
		WORD pal = (map >> 10) & 7;
		for (unsigned i = 0; i < 0x100; i++) {
			unsigned tileNum = (tileScroll.GetPos() << 4) + i;
			if (sel && (nmmx.GetTileType(tileNum) != it->first))
				tileNum = 0;
			
			render.RenderTile(hTileBack, i % 0x10, i >> 4, TILE(true || (tileNum < nmmx.numTiles) ? tileNum : 0, pal, 0, 0, 0));
		}
		//auto brush = CreateSolidBrush(RGB(255, 8, 127));
		//for (unsigned i = 0; i < 0x200; i++) {
		//	unsigned tileNum = (tileScroll.GetPos() << 4) + i;
		//	if (sel && (nmmx.GetTileType(tileNum) == it->first)) {
		//		RECT r;
		//		r.left = (i % 0x10) * 8 - 1;
		//		r.right = r.left + 8 + 2;
		//		r.top = (i >> 4) * 8 - 1;
		//		r.bottom = r.top + 8 + 2;

		//		FrameRect(hTileBack, &r, brush);
		//	}
		//}
		//DeleteObject(brush);
		//for (unsigned i = 0; i < 0x200; i++) {
		//	unsigned tileNum = (tileScroll.GetPos() << 4) + i;
		//	if (sel && (nmmx.GetTileType(tileNum) == it->first)) {
		//		render.RenderTile(hTileBack, i % 0x10, i >> 4, TILE(true || (tileNum < nmmx.numTiles) ? tileNum : 0, pal, 0, 0, 0));
		//	}
		//}

		StretchBlt(hMapDC, 380, 0, 256, 256, hTileBack, 0, 0, 128, 128, SRCCOPY);

		BitBlt(hMapDC, 0, 0, 256, 256, hMapBack, 0, 0, SRCCOPY);

		render.RenderTile(hRenderBack, 0, 0, TILE(dTileSelected, pal, 0, 0, 0));
		StretchBlt(hMapDC, 292, 124, 24, 24, hRenderBack, 0, 0, 8, 8, SRCCOPY);

		render.RenderTile(hRenderBack, 0, 0, TILE(dTileSave, pal, 0, 0, 0));
		StretchBlt(hMapDC, 330, 124, 24, 24, hRenderBack, 0, 0, 8, 8, SRCCOPY);

		// focus rectangles
		rectMapFocus.left = ((dMapSelected & 0x7) << 5);
		rectMapFocus.right = rectMapFocus.left + 32;
		rectMapFocus.top = (dMapSelected / 8 - mapScroll.GetPos()) << 5;
		rectMapFocus.bottom = rectMapFocus.top + 32;
		DrawFocusRect(hMapDC, &rectMapFocus);

		rectTileFocus.left = 380 + ((dTileSelected & 0xf) << 4);
		rectTileFocus.right = rectTileFocus.left + 16;
		rectTileFocus.top = (dTileSelected / 16 - tileScroll.GetPos()) << 4;
		rectTileFocus.bottom = rectTileFocus.top + 16;
		DrawFocusRect(hMapDC, &rectTileFocus);

		RECT r;
		r.left = rMapEdit.left + (selection % 4) * 16;
		r.right = r.left + 16;
		r.top = rMapEdit.top + (selection / 4) * 16;
		r.bottom = r.top + 16;
		DrawFocusRect(hMapDC, &r);

		DeleteDC(hMapBack);
		DeleteDC(hTileBack);
		DeleteDC(hRenderBack);
		EndPaint(hWnd, &ps);
		break;
	}
	case WM_INITDIALOG:
	{
		dMapSelected = 0;
		int sel = 0;

		hWID[4] = hWnd;
		backBufferMapProc = CreateBitmapCache(hWnd, 256, 256);
		backBufferTileProc = CreateBitmapCache(hWnd, 256, 256);
		backBufferRenderProc = CreateBitmapCache(hWnd, 64, 64);
		mapScroll.Create(hWnd, 257, 0, 256);
		mapScroll.SetRange(0, (nmmx.numBlocks > 0x40) ? (nmmx.numBlocks - 1) / 0x8 + 1 - 0x8 : 0);
		//mapCollision.Create(hWnd, ID_SP_COLLISION, 300, 88, 62, 0, 0xFF);
		mapTileIndex.Create(hWnd, 0xF001, 300, 150, 62, 0, 0x3FF);
		mapTilePalette.Create(hWnd, 0xF002, 300, 176, 62, 0, 7);

		tileScroll.Create(hWnd, 380 + 257, 0, 256);
		tileScroll.SetRange(0, nmmx.numTiles > 0x100 ? nmmx.numTiles / 16 - 16 : 0);

		std::stringstream ss;
		for (auto &m : TileTypeMap) {
			ss.str("");
			ss << "$" << std::hex << std::uppercase << m.first << ": " << std::nouppercase << m.second;
			SendMessage(GetDlgItem(hWnd, IDC_BLOCK_TYPE_COMBO), CB_ADDSTRING, (WPARAM)0, (LPARAM)ss.str().c_str());
		}
		SendMessage(GetDlgItem(hWnd, IDC_BLOCK_TYPE_COMBO), CB_SETCURSEL, (WPARAM)0, (LPARAM)0);

		UpdateMapEdit(hWnd, 0);

		break;
	}
	case WM_SHOWWINDOW:
		SetWindowPosition(hWnd, set.blkED.X, set.blkED.Y);
		break;
	case WM_CLOSE:
		DeleteObject(backBufferMapProc);
		DeleteObject(backBufferTileProc);
		DeleteObject(backBufferRenderProc);
		EndDialog(hWnd, 0);
		break;
	}
	return 0;
}

//// Control variables:
//ScrollBar blockScroll;
//SpinBox mapsAlloc[0x10];
//SpinBox palsAlloc[0x10];
//
//// General variables:
//BYTE blockSelection  = 0;
//unsigned dBlockSelected  = 0;
//unsigned dBlockOverMouse = 0;
//unsigned dBlockTextWrite = 0;
//static bool dIgnoreRefresh = false;
//
//// Drawing vars
//HDC hBlockDC, hBlockBack;
//HBITMAP backBufferBlockProc;
//
//extern RECT rMapEdit;
//extern RECT rectMapTileFocus[];
//
//static void UpdateBlockEdit(HWND hWnd) {
//	dIgnoreRefresh = true;
//	for (int i = 0; i < 0x10; i++) {
//		unsigned blockMap = dBlockSelected;
//		unsigned blockOffset = ((blockMap & 0xFF) << 5) + 0x2000 + nmmx.mapBase /* FE */;
//		unsigned tileOffset = i << 1;
//		WORD tileMap = *LPWORD(nmmx.ram + blockOffset + tileOffset);
//		mapsAlloc[i].SetPos(tileMap & 0x3FF, true);
//		palsAlloc[i].SetPos((tileMap >> 10) & 0x7, true);
//	}
//	dIgnoreRefresh = false;
//}
//
//static void UpdateBlockWrite(HWND hWnd, int blockNum = -1)
//{
//	if (nmmx.expandedROM && nmmx.expandedOffset.count(nmmx.level) && nmmx.expandedOffset[nmmx.level].count(0x2000 + nmmx.mapBase)) {
//		unsigned blockMap = dBlockSelected;
//		unsigned blockOffset = ((blockMap & 0xFF) << 5);
//
//		if (blockNum == -1) {
//			for (unsigned i = 0; i < 0x20; i += 2) {
//				WORD oldValue = 0;// *(LPWORD)(nmmx.rom + nmmx.expandedOffset[nmmx.level][0x2000 + nmmx.mapBase].first + blockOffset + i);
//				*(LPWORD)(nmmx.rom + nmmx.expandedOffset[nmmx.level][0x2000 + nmmx.mapBase].first + blockOffset + i) = (oldValue & 0xE000) | (palsAlloc[i >> 1].GetPos() << 10) | mapsAlloc[i >> 1].GetPos();
//			}
//		}
//		else {
//			WORD oldValue = 0;// *(LPWORD)(nmmx.rom + nmmx.expandedOffset[nmmx.level][0x2000 + nmmx.mapBase].first + blockOffset + 2 * blockNum);
//			*(LPWORD)(nmmx.rom + nmmx.expandedOffset[nmmx.level][0x2000 + nmmx.mapBase].first + blockOffset + 2 * blockNum) = (oldValue & 0xE000) | (palsAlloc[blockNum].GetPos() << 10) | mapsAlloc[blockNum].GetPos();
//		}
//	}
//}
//BOOL CALLBACK BlockProc(HWND hWnd, UINT message, WPARAM wParam, LPARAM lParam)
//{
//	PAINTSTRUCT ps;
//
//	switch (message)
//	{
//	case UPDATEWINDOW: {
//		blockSelection = 0;
//		dBlockSelected = 0;
//		dBlockOverMouse = 0;
//		blockScroll.SetPos(0);
//
//		UpdateBlockEdit(hWnd);
//
//		CHAR sIndex[13];
//		sprintf_s(sIndex, "Block N. %03X", dBlockOverMouse);
//		SetWindowText(GetDlgItem(hWnd, IDC_LBLOCKINDEX), sIndex);
//
//		break;
//	}
//	case WM_COMMAND:
//	{
//		bool found = false;
//		for (int i = 0; i < 0x10; i++) {
//			if (mapsAlloc[i].IsIDEqual((long)lParam))
//			{
//				found = true;
//				mapsAlloc[i].Work(wParam);
//				UpdateBlockWrite(hWnd, i);
//			}
//			if (palsAlloc[i].IsIDEqual((long)lParam))
//			{
//				found = true;
//				palsAlloc[i].Work(wParam);
//				UpdateBlockWrite(hWnd, i);
//			}
//		}
//
//		if (!dIgnoreRefresh && found) {
//			RefreshLevel();
//		}
//		break;
//	}
//	case WM_VSCROLL:
//		if (blockScroll.IsIDEqual((long)lParam))
//		{
//			blockScroll.Work(wParam);
//			if (LOWORD(wParam) != SB_ENDSCROLL)
//			{
//				RECT rect;
//				rect.top = 0;
//				rect.left = 0;
//				rect.bottom = 256;
//				rect.right = 256;
//				InvalidateRect(hWnd, &rect, false);
//			}
//		}
//		else
//		{
//			for (int i = 0; i < 0x10; i++) {
//				if (mapsAlloc[i].IsIDEqual((long)lParam))
//				{
//					mapsAlloc[i].Work(wParam);
//				}
//				if (palsAlloc[i].IsIDEqual((long)lParam))
//				{
//					palsAlloc[i].Work(wParam);
//				}
//			}
//		}
//		break;
//	case WM_LBUTTONDOWN:
//			if ((lParam & 0xFFFF) < 0x0100)
//			{
//				dBlockTextWrite = false;
//				dBlockSelected = dBlockOverMouse;
//				InvalidateRect(hWnd, &rMapEdit, false);
//
//				UpdateBlockEdit(hWnd);
//
//				RefreshLevel();
//			}
//			break;
//	case WM_MOUSEMOVE:
//		if ((lParam & 0xFFFF) < 0x0100)
//		{
//			auto tempBlock = (WORD)(((lParam >> 5) & 0xF) | ((lParam >> 18) & 0xF8) + (blockScroll.GetPos() << 3));
//			if (tempBlock < nmmx.numBlocks) {
//				dBlockOverMouse = tempBlock;
//			}
//			goto PRINTEXT;
//		}
//		else
//		{
//			if (dBlockOverMouse == dBlockSelected)
//				break;
//			dBlockOverMouse = dBlockSelected;
//			goto PRINTEXT;
//		}
//		if (false)
//		{
//PRINTEXT:
//			dBlockTextWrite = true;
//			CHAR sIndex[13];
//			sprintf_s(sIndex, "Block N. %03X", dBlockOverMouse);
//			SetWindowText(GetDlgItem(hWnd, IDC_LBLOCKINDEX), sIndex);
//		}
//		break;
//	case WM_MOVE:
//		RECT rc;
//		GetWindowRect(hWnd, &rc);
//		set.blkED.X = (SHORT)rc.left;
//		set.blkED.Y = (SHORT)rc.top;
//	case WM_PAINT:
//		hBlockDC = BeginPaint(hWnd, &ps);
//		hBlockBack = CreateCompatibleDC(hBlockDC);
//		SelectObject(hBlockBack, backBufferBlockProc);
//		
//		render.RenderBlock(hBlockDC, 9, 1, dBlockSelected);
//		StretchBlt(hBlockDC, 288, 32, 64, 64, hBlockDC, 288, 32, 32, 32, SRCCOPY);
//
//		for (int i = 0; i < 0x100; i++) {
//			unsigned blockNum = (blockScroll.GetPos() << 3) + i;
//			render.RenderBlock(hBlockBack, i & 0x7, i >> 3, blockNum < nmmx.numBlocks ? (blockScroll.GetPos() << 3) + i : 0);
//		}
//
//		BitBlt(hBlockDC, 0, 0, 256, 256, hBlockBack, 0, 0, SRCCOPY);
//		DeleteDC(hBlockBack);
//		EndPaint(hWnd, &ps);
//		break;
//	case WM_INITDIALOG:
//		dBlockSelected = 0;
//		hWID[4] = hWnd;
//		backBufferBlockProc = CreateBitmapCache(hWnd, 256, 256);
//		blockScroll.Create(hWnd, 257, 0, 256);
//		blockScroll.SetRange(0, nmmx.numBlocks > 0x40 ? (nmmx.numBlocks - 1) / 8 + 1 - 0x8 : 0);
//
//		dIgnoreRefresh = true;
//		for (int i = 0; i < 0x10; i++) {
//			mapsAlloc[i].Create(hWnd, 0x9000 + i, 296 + (i % 4) * 64, 100 + ((i / 4) * 24), 64, 0, nmmx.numTiles);
//			unsigned blockMap = dBlockSelected;
//			unsigned blockOffset = ((blockMap & 0xFF) << 5) + 0x2000 + nmmx.mapBase /* FE */;
//			unsigned tileOffset = i << 1;
//			WORD tileMap = *LPWORD(nmmx.ram + blockOffset + tileOffset);
//			mapsAlloc[i].SetPos(tileMap & 0x3FF, true);
//
//			palsAlloc[i].Create(hWnd, 0x9A00 + i, 296 + (i % 4) * 64, 200 + ((i / 4) * 24), 64, 0, 7);
//			palsAlloc[i].SetPos((tileMap >> 10) & 0x7, true);
//		}
//		dIgnoreRefresh = false;
//		RefreshLevel();
//		break;
//	case WM_SHOWWINDOW:
//		SetWindowPosition(hWnd, set.blkED.X, set.blkED.Y);
//		break;
//	case WM_CLOSE:
//		EndDialog(hWnd, 0);
//		break;
//	}
//	return 0; 
//}