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

#include <string>
#include <sstream>
#include <map>

static bool dTileTextWrite=0;
static BYTE dEditorPaletteSelect=0;
static BYTE dTilePaletteN=0, dTilePaletteNOld=1;
static WORD dTileSelected=0, dTileOverMouse=0;
static bool dTileMouseEdit = false;
static WORD dTileCopy = -1;

static ScrollBar tileScroll;
static RECT tileSelectRect;

HBITMAP backBufferTilesProc, backBufferTilesEdit;
BOOL CALLBACK TilesProc(HWND hWnd, UINT message, WPARAM wParam, LPARAM lParam)
{
	PAINTSTRUCT ps;
	HDC hDC;
	HDC hBackDC;
	int wmId, wmEvent;

	auto pt = MAKEPOINTS(lParam);

	switch (message)
	{
	case UPDATEWINDOW: {
		dTileTextWrite = 0;
		dEditorPaletteSelect = 0;
		dTilePaletteN = 0;
		dTilePaletteNOld = 1;
		dTileSelected = 0;
		dTileOverMouse = 0;
		dTileMouseEdit = false;
		dTileCopy = -1;
		SetDlgItemInt(hWnd, IDC_PALETTEN, 0, false);
		SendMessage(GetDlgItem(hWnd, IDC_TILE_TYPE_COMBO), CB_SETCURSEL, (WPARAM)0, (LPARAM)0);
		EnableWindow(GetDlgItem(hWnd, IDC_TILE_COLL_COMBO), nmmx.expandedROM && !nmmx.isMode7());
		SendMessage(GetDlgItem(hWnd, IDC_TILE_COLL_COMBO), CB_SETCURSEL, (WPARAM)((nmmx.GetTileType(dTileSelected) - 0xC8) / 2), (LPARAM)0);

		tileScroll.SetPos(0);
		break;
	}
	case WM_PAINT: {
		hDC = BeginPaint(hWnd, &ps);
		hBackDC = CreateCompatibleDC(hDC);
		SelectObject(hBackDC, backBufferTilesProc);

		RECT rect;
		rect.left = 0;
		rect.top = 0;
		rect.right = 256 - 1;
		rect.bottom = 256 - 1;

		auto b = CreateSolidBrush(RGB(0, 0, 0));
		FillRect(hBackDC, &rect, b);
		DeleteObject(b);

		auto sel = SendMessage(GetDlgItem(hWnd, IDC_TILE_TYPE_COMBO), CB_GETCURSEL, (WPARAM)0, (LPARAM)0);
		auto it = TileTypeMap.begin();
		std::advance(it, sel);

		for (unsigned i = 0; i < 0x100; i++) {
			unsigned tileNum = (tileScroll.GetPos() << 4) + i;
			if (sel && (nmmx.GetTileType(tileNum) != it->first))
				tileNum = 0;
			
			render.RenderTile(hBackDC, i % 0x10, i >> 4, TILE(tileNum, dTilePaletteN, 0, 0, 0));
		}
		StretchBlt(hDC, 0, 0, 256, 256, hBackDC, 0, 0, 128, 128, SRCCOPY);

		SelectObject(hBackDC, backBufferTilesEdit);
		render.RenderTile(hBackDC, 0, 0, TILE(dTileSelected, dTilePaletteN, 0, 0, 0));
		StretchBlt(hBackDC, 0, 0, 128, 128, hBackDC, 0, 0, 8, 8, SRCCOPY);
		BitBlt(hDC, 296, 80, 128, 128, hBackDC, 0, 0, SRCCOPY);

		tileSelectRect.left = (dTileSelected % 16) << 4;
		tileSelectRect.right = tileSelectRect.left + 16;
		tileSelectRect.top = (dTileSelected / 16 - tileScroll.GetPos()) << 4;
		tileSelectRect.bottom = tileSelectRect.top + 16;

		DrawFocusRect(hDC, &tileSelectRect);

		for (int y = 0; y < 4; y++)
			for (int x = 0; x < 4; x++)
			{
				auto brush = CreateSolidBrush(nmmx.ConvertBGRColor(nmmx.palCache[x | (y << 2) | (dTilePaletteN << 4)]));
				DeleteObject(SelectObject(hDC, brush));
				Rectangle(hDC, 296 + (x << 4), 224 + (y << 4), 296 + 18 + (x << 4), 224 + 18 + (y << 4));
			}

		RECT pRect;
		pRect.left = 296 + ((dEditorPaletteSelect % 4) << 4);
		pRect.right = pRect.left + 16;
		pRect.top = 224 + ((dEditorPaletteSelect / 4) << 4);
		pRect.bottom = pRect.top + 16;
		DrawFocusRect(hDC, &pRect);

		//DeleteObject(brush);
		DeleteDC(hBackDC);

		EndPaint(hWnd, &ps);
		break;
	}
	case WM_VSCROLL: {
		if (tileScroll.IsIDEqual((long)lParam))
		{
			tileScroll.Work(wParam);
			InvalidateRect(hWnd, NULL, false);
		}
		break;
	}
	case WM_COMMAND:
		wmId    = LOWORD(wParam);
		wmEvent = HIWORD(wParam);

		switch (wmId)
		{
		case IDC_TILE_COPY: {
			dTileCopy = dTileSelected;
			break;
		}
		case IDC_TILE_PASTE: {
			if (dTileCopy != -1 && nmmx.expandedROM && nmmx.expandedOffset.count(nmmx.level) && nmmx.expandedOffset[nmmx.level].count(nmmx.GetTileVramByteAddr() >> 1) && nmmx.level < 0x42) {
				unsigned tileSrc = (dTileCopy & 0x3FF);
				unsigned tileDst = (dTileSelected & 0x3FF);

				if (nmmx.isMode7()) {
					tileSrc &= 0x00FF;
					tileDst &= 0x00FF;
				}

				for (unsigned i = 0; i < 64; i++) {
					*(LPBYTE)(nmmx.vramCache + (nmmx.GetTileVramByteAddr() << 1) + ((tileDst) << 6) + i) = *(LPBYTE)(nmmx.vramCache + (nmmx.GetTileVramByteAddr() << 1) + ((tileSrc) << 6) + i);
				}

				unsigned baseOffset = nmmx.expandedOffset[nmmx.level][nmmx.GetTileVramByteAddr() >> 1].first;
				if (!nmmx.isMode7())
					nmmx.raw2tile4bpp((nmmx.vramCache + (nmmx.GetTileVramByteAddr() << 1) + ((tileDst) << 6)), nmmx.rom + baseOffset + (tileDst << 5));
				else
					nmmx.raw2tileMode7((nmmx.vramCache + (nmmx.GetTileVramByteAddr() << 1) + ((tileDst) << 6)), nmmx.rom + baseOffset + (tileDst << 6));

				render.RefreshMapCache();
				RepaintAll();
			}
			break;
		}
		case IDC_TILE_TYPE_COMBO: {
			InvalidateRect(hWnd, NULL, false);

			auto sel = SendMessage(GetDlgItem(hWnd, IDC_TILE_TYPE_COMBO), CB_GETCURSEL, (WPARAM)0, (LPARAM)0);
			auto it = TileTypeMap.begin();
			std::advance(it, sel);

			//EnableWindow(GetDlgItem(hWnd, IDC_TILE_ADD), false && nmmx.expandedROM && !nmmx.isMode7() && 0xC8 < it->first && it->first < 0xE4);

			break;
		}
		case IDC_TILE_COLL_COMBO: {
			InvalidateRect(hWnd, NULL, false);

			auto sel = SendMessage(GetDlgItem(hWnd, IDC_TILE_COLL_COMBO), CB_GETCURSEL, (WPARAM)0, (LPARAM)0);
			nmmx.SetTileType(dTileSelected, sel);

			break;
		}

		//case IDC_TILE_ADD: {
		//	auto sel = SendMessage(GetDlgItem(hWnd, IDC_TILE_TYPE_COMBO), CB_GETCURSEL, (WPARAM)0, (LPARAM)0);
		//	auto it = TileTypeMap.begin();
		//	std::advance(it, sel);

		//	// add one to the cutoffs of each value
		//	WORD type = it->first;
		//	WORD offset = *LPWORD(nmmx.rom + SNESCore::snes2pc(0x85C2D0) + 2 * nmmx.level);
		//	WORD tile = 0;

		//	for (unsigned i = 4; i < 0x1E; i += 2) {
		//		LPWORD value = LPWORD(nmmx.rom + SNESCore::snes2pc(0x850000 + offset + i));
		//		if (type == 0xC6 + i) {
		//			tile = *value;
		//		}
		//		
		//		if (type <= 0xC6 + i && *value != 0xFFFF) {
		//			*value += 1;
		//		}
		//	}

		//	LPWORD max = LPWORD(nmmx.rom + SNESCore::snes2pc(0x850000 + offset + (0xE4 - 0xC8)));
		//	if (*max != 0xFFFF)
		//	*max += 1;

		//	if (nmmx.isMode7())
		//		tile &= 0xFF;

		//	// shift all the tiles up one (dropping the last one)
		//	unsigned baseOffset = nmmx.expandedOffset[nmmx.level][nmmx.GetTileVramByteAddr() >> 1].first;
		//	if (!nmmx.isMode7()) {
		//		LPBYTE pTile = nmmx.rom + baseOffset + (tile << 5);
		//		memmove(pTile + 0x20, pTile, (0x200 - tile - 1) * 0x20);
		//		memset(pTile, 0x00, 0x20);
		//	}
		//	else {
		//		LPBYTE pTile = nmmx.rom + baseOffset + (tile << 6);
		//		memmove(pTile + 0x40, pTile, (0x100 - tile - 1) * 0x40);
		//		memset(pTile, 0x00, 0x40);
		//	}

		//	// fix all pointers in the block map
		//	for (int b = 0; b < 2; b++) {
		//		for (int i = 0; i < 0x100; i++) {
		//			unsigned blockMap = i;
		//			unsigned blockOffset = ((blockMap & 0xFF) << 5);

		//			for (int y = 0; y < 4; y++) {
		//				for (int x = 0; x < 4; x++) {
		//					unsigned tileOffset = (x << 1) + (y << 3);
		//					LPWORD pTileMap = (LPWORD)(nmmx.rom + nmmx.expandedOffset[nmmx.level][0x7E2000 + (b == 0 ? 0x4000 : 0xA000)].first + blockOffset + tileOffset);
		//					WORD tileMask = nmmx.isMode7() ? 0xFF : 0x3FF;
		//					WORD tempTile = *pTileMap & tileMask;
		//					if (tempTile >= tile && tempTile != tileMask) {
		//						*pTileMap = (*pTileMap & ~tileMask) | (tempTile + 1);
		//					}
		//				}
		//			}
		//		}
		//	}

		//	// fix the dynamic tile offsets
		//	std::vector<WORD> dynTileOffset = { SReadWord(nmmx, 0x85CA82 + 2 * nmmx.level), SReadWord(nmmx, 0x85CB0A + 2 * nmmx.level) };
		//	std::set<WORD> dynTileOffsetSeen;

		//	for (auto o : dynTileOffset) {
		//		if (SReadWord(nmmx, 0x810000 + o + 0)) {
		//			WORD tableOffset = SReadWord(nmmx, 0x810000 + o + 2);

		//			LPWORD lpTable = (LPWORD)SReadLpByte(nmmx, 0x810000 + tableOffset);
		//			// skip past the base entries
		//			lpTable += 2;

		//			while (*lpTable) {
		//				LPBYTE lpTableEntry = (LPBYTE)SReadLpByte(nmmx, 0x810000 + *lpTable + 0);

		//				if (   *(lpTableEntry + 0) == 0x80 && *(lpTableEntry + 1) == 0x00
		//					&& *(lpTableEntry + 2) == 0x01 && *(lpTableEntry + 3) == 0x18
		//					&& !dynTileOffsetSeen.count(*lpTable)
		//					) {
		//					WORD vramWordOffset = *LPWORD(lpTableEntry + 4);

		//					if (0x2000 <= vramWordOffset && vramWordOffset < 0x4000) {
		//						WORD tempTile = (vramWordOffset - 0x2000) / 0x10;

		//						if (tile <= tempTile) {
		//							*LPWORD(lpTableEntry + 4) += 0x10;
		//						}
		//					}
		//					dynTileOffsetSeen.insert(*lpTable);
		//				}

		//				lpTable++;
		//			}
		//		}
		//	}

		//	RefreshLevel();

		//	break;
		//}
		case IDC_PALETTEN:
			dTilePaletteN = GetDlgItemInt(hWnd, IDC_PALETTEN, NULL, false);
			if (dTilePaletteN != dTilePaletteNOld)
			{
				dTilePaletteNOld = dTilePaletteN;
				InvalidateRect(hWnd, NULL, false);
			}
			break;
		}
		break;
	case WM_LBUTTONUP: {
		render.RefreshMapCache();
		RepaintAll();
		break;
	}
	case WM_LBUTTONDOWN:
	{
	PAINT_TILE:
		SHORT x = (SHORT)lParam;
		SHORT y = (SHORT)(lParam >> 16);
		if (x >= 296 && x <= 296 + 128 && y >= 80 && y <= 80 + 128)
		{
			x = (x - 296) / 16;
			y = (y - 80) / 16;

			if (nmmx.expandedROM && nmmx.expandedOffset.count(nmmx.level) && nmmx.expandedOffset[nmmx.level].count(nmmx.GetTileVramByteAddr() >> 1) && nmmx.level < 0x42) {
				unsigned tile = (dTileSelected & 0x3FF);
				if (nmmx.isMode7()) {
					tile &= 0x00FF;
				}

				*(LPBYTE)(nmmx.vramCache + (nmmx.GetTileVramByteAddr() << 1) + ((tile) << 6) + x + (y * 8)) = dEditorPaletteSelect;
				unsigned baseOffset = nmmx.expandedOffset[nmmx.level][nmmx.GetTileVramByteAddr() >> 1].first;
				if (!nmmx.isMode7())
					nmmx.raw2tile4bpp((nmmx.vramCache + (nmmx.GetTileVramByteAddr() << 1) + ((tile) << 6)), nmmx.rom + baseOffset + (tile << 5));
				else
					nmmx.raw2tileMode7((nmmx.vramCache + (nmmx.GetTileVramByteAddr() << 1) + ((tile) << 6)), nmmx.rom + baseOffset + (tile << 6));
			}

			//*(LPBYTE)(nmmx.vramCache + ((dTileSelected & 0x3FF)<<6) + x + (y*8)) = dEditorPaletteSelect;
		}
		break;
	}
	case WM_RBUTTONDOWN:
	{
		if (pt.x < 0x100) {
			dTileTextWrite = false;
			dTileSelected = dTileOverMouse;
			EnableWindow(GetDlgItem(hWnd, IDC_TILE_COLL_COMBO), nmmx.expandedROM && !nmmx.isMode7());
			SendMessage(GetDlgItem(hWnd, IDC_TILE_COLL_COMBO), CB_SETCURSEL, (WPARAM)((nmmx.GetTileType(dTileSelected) - 0xC8) / 2), (LPARAM)0);
			InvalidateRect(hWnd, NULL, false);
		}
		else if (pt.x >= 296 && pt.x <= 296 + 64 && pt.y >= 224 && pt.y <= 224 + 64)
		{
			pt.x = (pt.x - 296) / 16;
			pt.y = (pt.y - 224) / 16;
			dEditorPaletteSelect = pt.x + (pt.y * 4);
		}

		InvalidateRect(hWnd, NULL, false);
		break;
	}
	case WM_MOUSEMOVE:
		if (wParam != 0)
			goto PAINT_TILE;
		if (pt.x < 0x100 && pt.y < 0x100)
		{
			WORD tempTile = (pt.x >> 4) + (pt.y >> 4 << 4) + (tileScroll.GetPos() << 4);
			dTileOverMouse = tempTile;
			dTileMouseEdit = false;
			goto PRINTEXT;
		}
		else
		{
			dTileMouseEdit = false; // true;
			if (dTileOverMouse == dTileSelected)
				break;
			dTileOverMouse = dTileSelected;
			goto PRINTEXT;
		}
		if (!dTileTextWrite)
		{
PRINTEXT:
			dTileTextWrite = false;
			dTileTextWrite = true;
			CHAR sIndex[30];
			sprintf(sIndex, "Tile N. %03X", dTileOverMouse);
			SetWindowText(GetDlgItem(hWnd, IDC_LTILEINDEX), sIndex);

			std::stringstream ss;
			WORD type = nmmx.GetTileType(dTileOverMouse);
			ss << std::hex << "$" << std::uppercase << type << " - " << TileTypeMap[type];

			SetWindowText(GetDlgItem(hWnd, IDC_LTILEPROP), ss.str().c_str());
		}
		break;
	case WM_MOVE:
		RECT rc;
		GetWindowRect(hWnd, &rc);
		set.tilED.X = (SHORT)rc.left;
		set.tilED.Y = (SHORT)rc.top;
		break;
	case WM_INITDIALOG:
	{
		hWID[2] = hWnd;
		SetWindowPosition(hWnd, set.tilED.X, set.tilED.Y);
		HDC tmpDC = GetDC(hWnd);
		backBufferTilesProc = CreateCompatibleBitmap(tmpDC, 256, 256);
		backBufferTilesEdit = CreateCompatibleBitmap(tmpDC, 128, 128);

		DeleteDC(tmpDC);
		SendMessage(GetDlgItem(hWnd, IDC_PALETTES), UDM_SETRANGE, 0, 7);

		tileScroll.Create(hWnd, 257, 0, 256);
		// 0x20 = 0x300 tiles
		auto max = nmmx.numTiles > 0x100 ? ((nmmx.numTiles) / 16 - 16) : 0;
		tileScroll.SetRange(0, max);

		std::stringstream ss;
		for (auto &m : TileTypeMap) {
			ss.str("");
			ss << "$" << std::hex << std::uppercase << m.first << ": " << std::nouppercase << m.second;
			SendMessage(GetDlgItem(hWnd, IDC_TILE_TYPE_COMBO), CB_ADDSTRING, (WPARAM)0, (LPARAM)ss.str().c_str());
			if (m.first)
				SendMessage(GetDlgItem(hWnd, IDC_TILE_COLL_COMBO), CB_ADDSTRING, (WPARAM)0, (LPARAM)ss.str().c_str());
		}
		SendMessage(GetDlgItem(hWnd, IDC_TILE_TYPE_COMBO), CB_SETCURSEL, (WPARAM)0, (LPARAM)0);

		// only enable adding tiles on the correct types
		//EnableWindow(GetDlgItem(hWnd, IDC_TILE_ADD), false);

		break;
	}
	case WM_CLOSE:
		DeleteObject(backBufferTilesProc);
		DeleteObject(backBufferTilesEdit);
		EndDialog(hWnd, 0);
		render.RefreshMapCache();
		RepaintAll();
		break;
	}
	return 0; 
}