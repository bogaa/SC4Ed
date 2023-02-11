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
#include <cctype>

const unsigned char hexCharIndex[0x10] = {0x30, 0x31, 0x32, 0x33, 0x34, 0x35, 0x36, 0x37, 0x38, 0x39, 0x41, 0x42, 0x43, 0x44, 0x45, 0x46};
const BYTE _bigTextAlloc[][26*2] = {
	{
	0x80, 0x90, // A
	0x81, 0x91, // B
	0x82, 0x92, // C
	0x83, 0x93, // D
	0x84, 0x94, // E
	0x84, 0xB5, // F
	0x85, 0x95, // G
	0x86, 0x96, // H
	0x87, 0x97, // I
	0x00, 0x00, // J ***
	0x88, 0x98, // K
	0x89, 0x99, // L
	0x8A, 0x9A, // M
	0x8B, 0x9B, // N
	0x8C, 0x9C, // O
	0x8D, 0x9D, // P
	0x00, 0x00, // Q ***
	0x8E, 0x9E, // R
	0x8F, 0x9F, // S
	0xA0, 0x97, // T
	0xA1, 0xB1, // U
	0x00, 0x00, // V ***
	0xA2, 0xB2, // W
	0x00, 0x00, // X ***
	0xA3, 0xB3, // Y
	0x00, 0x00, // Z ***
	},
	{
	0x80, 0x90, // A
	0x81, 0x91, // B
	0x82, 0x92, // C
	0x83, 0x93, // D
	0x84, 0x94, // E
	0x85, 0x95, // F
	0x86, 0x96, // G
	0x87, 0x97, // H
	0x88, 0x98, // I
	0x89, 0x99, // J
	0x8A, 0x9A, // K
	0x8B, 0x9B, // L
	0x8C, 0x9C, // M
	0x8D, 0x9D, // N
	0x8E, 0x9E, // O
	0x8F, 0x9F, // P
	0xA0, 0xB0, // Q
	0xA1, 0xB1, // R
	0xA2, 0xB2, // S
	0xA3, 0xB3, // T
	0xA4, 0xB4, // U
	0xA5, 0xB5, // V
	0xA6, 0xB6, // W
	0xA7, 0xB7, // X
	0xA8, 0xB8, // Y
	0xA9, 0xB9, // Z
	},
	{
	0x80, 0x90, // A
	0x81, 0x91, // B
	0x82, 0x92, // C
	0x83, 0x93, // D
	0x84, 0x94, // E
	0x85, 0x95, // F
	0x86, 0x96, // G
	0x87, 0x97, // H
	0x88, 0x98, // I
	0x89, 0x99, // J
	0x8A, 0x9A, // K
	0x8B, 0x9B, // L
	0x8C, 0x9C, // M
	0x8D, 0x9D, // N
	0x8E, 0x9E, // O
	0x8F, 0x9F, // P
	0xA0, 0xB0, // Q
	0xA1, 0xB1, // R
	0xA2, 0xB2, // S
	0xA3, 0xB3, // T
	0xA4, 0xB4, // U
	0xA5, 0xB5, // V
	0xA6, 0xB6, // W
	0xA7, 0xB7, // X
	0xA8, 0xB8, // Y
	0xA9, 0xB9, // Z
	},
};

const BITMAPINFO _bmpInfo = {sizeof(BITMAPINFOHEADER), 8, -8, 1, 16, BI_RGB, 0, 0, 0, 0, 0};
const BITMAPINFO _bmpInfoEmu = { sizeof(BITMAPINFOHEADER), 256, -224, 1, 32, BI_RGB, 0, 0, 0, 0, 0 };
RenderED::~RenderED()
{
	Destroy();
}
void RenderED::Init(SC4Core *mmxParam)
{
	Destroy();
	pmmx = mmxParam;
	void *xdata;
	void *sxdata;
	void *exdata;
	hBmpTile = CreateDIBSection(NULL, &_bmpInfo, 0, &xdata, 0, 0);
	hBmpSprite = CreateDIBSection(NULL, &_bmpInfo, 0, &sxdata, 0, 0);
	hBmpEmu = CreateDIBSection(NULL, &_bmpInfoEmu, 0, &exdata, 0, 0);
	data = (LPWORD)xdata;
	sdata = (LPWORD)sxdata;
	edata = (LPDWORD)exdata;
}
void RenderED::Destroy()
{
	pmmx = NULL;
	if (hBmpTile != NULL) DeleteObject(hBmpTile);
	if (hBmpSprite != NULL) DeleteObject(hBmpSprite);
	if (hBmpEmu != NULL) DeleteObject(hBmpEmu);
	if (dcWork[0] != NULL) DeleteDC(dcWork[0]);
	if (dcWork[1] != NULL) DeleteDC(dcWork[1]);
	dcWork[0]   = NULL;
	dcWork[1] = NULL;
	hBmpTile = NULL;
	hBmpSprite = NULL;
	hBmpEmu  = NULL;
	data     = NULL;
	sdata    = NULL;
	edata    = NULL;
}
void RenderED::CreateMapCache(HWND hWND)
{
	hWnd = hWND;
	HDC dcMain = GetDC(hWnd);
	// FIXME:
	hBmpMaps[0] = CreateCompatibleBitmap(dcMain, 1 << ((nmmx.type == 0 ? 8 : nmmx.type == 1 ? 9 : 8) + 5), 32); //(32 * 32 * 0x20 * 0x20 * 8), 8);//0x400 * 16 * 8, 16);
	hBmpMaps[1] = CreateCompatibleBitmap(dcMain, 1 << ((nmmx.type == 0 ? 8 : nmmx.type == 1 ? 9 : 8) + 5), 32); //(32 * 32 * 0x20 * 0x20 * 8), 8);//0x400 * 16 * 8, 16);
	dcWork[0] = CreateCompatibleDC(dcMain);
	dcWork[1] = CreateCompatibleDC(dcMain);
	SelectObject(dcWork[0], hBmpMaps[0]);
	SelectObject(dcWork[1], hBmpMaps[1]);
	RefreshMapCache();
	ReleaseDC(hWnd, dcMain);
}
void RenderED::RefreshMapCache()
{
	//	LPWORD map = (LPWORD)(pmmx->mapRam + 0x0000);
	//for(int i=0; i<0x400*pmmx->sceneUsed; i++)
	//{
	//RenderTile(dcWork[currentBuffer ^ 1], i + 0, 0, *map); map++;
	//}
	// at most 0x0-0xFF blocks
	for (int i = 0; i < nmmx.numBlocks; i++)
	{
		unsigned blockMap = i;
		unsigned blockOffset = 0;
		if (nmmx.type == 0) {
			blockOffset = ((blockMap & (nmmx.numBlocks - 1)) << 5) + 0x2000 + nmmx.mapBase /* FE */;
		}
		else if (nmmx.type == 1) {
			blockOffset = ((blockMap & (nmmx.numBlocks - 1)) << 5) + 0x1000 + nmmx.mapBase /* FE */;
			//blockOffset = ((blockMap & (nmmx.numBlocks - 1)) << 5) + 0x5000 + nmmx.mapBase /* FE */;
		}
		else if (nmmx.type == 2) {
			blockOffset = ((blockMap & (nmmx.numBlocks - 1)) << 5) + 0x3000 + nmmx.mapBase /* FE */;
		}

		for (int y = 0; y < 4; y++) {
			for (int x = 0; x < 4; x++) {
				unsigned tileOffset = (x << 1) + (y << 3);
				WORD tileMap = *LPWORD(nmmx.ram + blockOffset + tileOffset);

				RenderTile(dcWork[currentBuffer ^ 1], (i << 2) + x, y, tileMap);
			}
		}
		//RenderTile(dcWork[currentBuffer ^ 1], i*2 + 1, 0, *map++);
		//RenderTile(dcWork[currentBuffer ^ 1], i*2 + 0, 1, *map++);
		//RenderTile(dcWork[currentBuffer ^ 1], i*2 + 1, 1, *map++);
	}
	currentBuffer ^= 1;
	//BitBlt(dcWork, 0, 0, 0x400 * 16, 16, dcWork2, 0, 0, SRCCOPY);
}
void RenderED::RefreshMapCache(WORD index)
{
	LPWORD map = (LPWORD)(pmmx->vram + 0x0000) + (index*4);
	index <<= 1;
	RenderTile(dcWork[currentBuffer], index + 0, 0, *map++);
	RenderTile(dcWork[currentBuffer], index + 1, 0, *map++);
	RenderTile(dcWork[currentBuffer], index + 0, 1, *map++);
	RenderTile(dcWork[currentBuffer], index + 1, 1, *map++);
}
void RenderED::RenderTile(HDC hdc, int x, int y, WORD tile)
{
	LPWORD xdata = data;
	HDC dcBmp = CreateCompatibleDC(hdc);
	BYTE palette = (tile >> 6) & 0x70;

	// base address
	if (pmmx->isMode7()) {
		tile &= 0x00FF;
		palette = 0;
	}

	unsigned c = (pmmx->isMode7()) ? (tile & 0xFF) : (tile & 0x3FF);
	//if (c >= 0x80) c = 0x7F;
	LPBYTE image = pmmx->vramCache + pmmx->GetTileVramByteAddr()*2 + ((c) << 6);

	for (int i = 0; i < 0x40; i++, image++) {
		auto v = *image;
		*xdata++ = pmmx->palCache[v | palette];
	}
	SelectObject(dcBmp, hBmpTile);
	if (tile & 0x4000) StretchBlt(dcBmp, 0, 0, 8, 8, dcBmp, 7, 0, -8, 8, SRCCOPY);
	if (tile & 0x8000) StretchBlt(dcBmp, 0, 0, 8, 8, dcBmp, 0, 7, 8, -8, SRCCOPY);

	//const COLORREF color = GetPixel(dcBmp, 0, 0);

	BitBlt(hdc, x << 3, y << 3, 8, 8, dcBmp, 0, 0, SRCCOPY);
	DeleteDC(dcBmp);
}
void RenderED::RenderSpriteTile(HDC hdc, int x, int y, unsigned tile, unsigned tileOffset, unsigned shift, unsigned spalette, bool transparent, COLORREF backColor)
{
	LPWORD xdata = data;
	HDC dcBmp = CreateCompatibleDC(hdc);

	unsigned vramByteAddr = nmmx.type == 0 ? 0xC000 : nmmx.type == 1 ? 0x0000 : 0x4000;

	unsigned c = (tileOffset & 0x3FF);
	//if (c >= 0x80) c = 0x7F;
	LPBYTE image = pmmx->vramCache + vramByteAddr * 2 + ((c) << 6);

	// get background color
	auto restore = SaveDC(dcBmp);

	for (int i = 0; i < 0x40; i++) {
		*xdata++ = pmmx->palCache[0x80 | (spalette << 4)];
		//*xdata++ = pmmx->palSpriteCache[spalette << 4];
	}
	SelectObject(dcBmp, hBmpTile);
	const COLORREF color = GetPixel(dcBmp, 0, 0);

	RestoreDC(dcBmp, restore);

	xdata = data;
	for (int i = 0; i < 0x40; i++) {
		*xdata++ = pmmx->palCache[*image++ | 0x80 | (spalette << 4)];
		//*xdata++ = pmmx->palSpriteCache[*image++ | (spalette << 4)];
	}
	SelectObject(dcBmp, hBmpTile);
	if (tile & 0x4000) StretchBlt(dcBmp, 0, 0, 8, 8, dcBmp, 7, 0, -8, 8, SRCCOPY);
	if (tile & 0x8000) StretchBlt(dcBmp, 0, 0, 8, 8, dcBmp, 0, 7, 8, -8, SRCCOPY);
	if (!transparent) {
		BitBlt(hdc, x, y, 8, 8, dcBmp, 0, 0, SRCCOPY);
	}
	else {
		HBITMAP hBmpMask;
		BITMAP bm;

		// create mask
		GetObject(hBmpTile, sizeof(BITMAP), &bm);
		hBmpMask = CreateBitmap(bm.bmWidth, bm.bmHeight, 1, 1, NULL);

		HDC hdcMem2 = CreateCompatibleDC(hdc);
		//SelectObject(hdcMem, hBmpTile);
		SelectObject(hdcMem2, hBmpMask);

		SetBkColor(dcBmp, color);

		BitBlt(hdcMem2, 0, 0, bm.bmWidth, bm.bmHeight, dcBmp, 0, 0, SRCCOPY);
		BitBlt(dcBmp, 0, 0, bm.bmWidth, bm.bmHeight, hdcMem2, 0, 0, SRCINVERT);

		if (backColor != RGB(0, 0, 0)) {
			RECT rect;
			rect.left = x;
			rect.right = x + bm.bmWidth;
			rect.top = y;
			rect.bottom = y + bm.bmHeight;
			auto brush = CreateSolidBrush(backColor);
			FrameRect(hdc, &rect, brush);
			DeleteObject(brush);
		}

		BitBlt(hdc, x, y, bm.bmWidth, bm.bmHeight, hdcMem2, 0, 0, SRCAND);
		BitBlt(hdc, x, y, bm.bmWidth, bm.bmHeight, dcBmp, 0, 0, SRCPAINT);
		
		DeleteObject(hBmpMask);
		DeleteDC(hdcMem2);
	}
	DeleteDC(dcBmp);
}
void RenderED::RenderMap(HDC hdc, int x, int y, int index)
{
	BitBlt(hdc, x<<3, y<<3, 8, 8, dcWork[currentBuffer], index<<3, 0, SRCCOPY);
	//TransparentBlt(hdc, x<<4, y<<4, 16, 16, dcWork[currentBuffer], index<<4, 0, 16, 16, RGB(0xF7, 0, 0xFF));
}
void RenderED::RenderBlock(HDC hdc, int x, int y, WORD index)
{
#if 0
	LPWORD pmap = (LPWORD)(pmmx->rom + pmmx->pBlocks) + index*4;
	x <<= 1;
	y <<= 1;
	RenderMap(hdc, x + 0, y + 0, *pmap++);
	RenderMap(hdc, x + 1, y + 0, *pmap++);
	RenderMap(hdc, x + 0, y + 1, *pmap++);
	RenderMap(hdc, x + 1, y + 1, *pmap++);
#else
	unsigned blockNum = index & (nmmx.numBlocks - 1);
	StretchBlt(hdc, x << 5, y << 5, 32, 32, dcWork[currentBuffer], (blockNum << 5) + ((index & 0x8000) ? 31 : 0), (index & 0x4000) ? 31 : 0, (index & 0x8000) ? -32 : 32, (index & 0x4000) ? -32 : 32, SRCCOPY);
	//StretchBlt(hdc, x << 5, y << 5, 32, 32, dcWork[currentBuffer], (blockNum << 5) + 0, 0, 32, 32, SRCCOPY);
#endif
}
void RenderED::RenderScene(HDC hdc, int x, int y, WORD index)
{
	x <<= 3;
	y <<= 3;
	LPWORD pblock = (LPWORD)(pmmx->rom + pmmx->pScenes) + (index<<6);
	for(int iy=0; iy<8; iy++)
		for(int ix=0; ix<8; ix++)
			RenderBlock(hdc, x + ix, y + iy, *pblock++);
}
void RenderED::RenderSceneEx(HDC hdc, int x, int y, int index)
{
#if 0
	x <<= 5;
	y *= 32;
	LPWORD pmap = pmmx->mapping + (index<<10);
	for(int iy=0; iy<32; iy++)
		for(int ix=0; ix<32; ix++)
			RenderMap(hdc, x + ix, y + iy, /**pmap++*/ index * 0x400 + iy * 32 + ix);
#else 
	x <<= 3;
	y <<= 3;

	LPWORD pmap = pmmx->mapping + (index << 6);
	for (int iy = 0; iy < 8; iy++) {
		for (int ix = 0; ix < 8; ix++) {
			WORD m = *pmap++;
			if (nmmx.type == 1) m = ((m & 0x8000) >> 1) | ((m & 0x4000) << 1) | ((m & ~0xC000) >> 5);
			if (nmmx.type == 2) m = ((m & 0x0080) << 7) | ((m & ~0xFF00));
			RenderBlock(hdc, x + ix, y + iy, /*iy * 8 + ix);*/m);
		}
	}
#endif
}
void RenderED::RenderEmu(HDC hdc, const struct FrameState &s) {
	memcpy(edata, s.buffer, 256 * 224 * 4);  // this looks wrong
	//LPDWORD xdata = data;
	HDC dcBmp = CreateCompatibleDC(hdc);
	//BYTE palette = (tile >> 6) & 0x70;
	//LPBYTE image = pmmx->vramCache + ((tile & 0x3FF) << 6);
	//for (int i = 0; i<0x40; i++)
	//	*xdata++ = pmmx->palCache[*image++ | palette];

	SelectObject(dcBmp, hBmpEmu);
	BitBlt(hdc, s.xpos, s.ypos, 256, 224, dcBmp, 0, 0, SRCCOPY);
	RECT rect;
	rect.left = s.xpos - 1;
	rect.right = s.xpos + 256 + 1;
	rect.top = s.ypos - 1;
	rect.bottom = s.ypos + 224 + 1;
	DrawFocusRect(hdc, &rect);
	DeleteDC(dcBmp);
}
void RenderED::RenderEvent(HDC hDC, int x, int y, BYTE type, BYTE id, BYTE subId, bool simplified, int highlight) {
	if (type == 0x1 && nmmx.type == 0 && nmmx.region == 0x0) {
		//std::vector<std::tuple<double, WORD>> candleWeight = {
		//	{ 200, 0x00 }, // = Null
		//	{ 300, 0x18 }, // = Small Heart
		//	{ 50, 0x19 }, // = Big Heart
		//	{ 20, 0x1A }, // = Dagger
		//	{ 20, 0x1B }, // = Axe
		//	{ 20, 0x1C }, // = Vial
		//	{ 20, 0x1D }, // = Cross
		//	{ 20, 0x1E }, // = Stopwatch
		//	{ 5, 0x1F }, // = Kill All
		//	{ 5, 0x20 }, // = Potion (Invincible)
		//	{ 50, 0x21 }, // = Whip Upgrade
		//	{ 80, 0x22 }, // = Money (100)
		//	{ 50, 0x23 }, // = Double
		//	{ 50, 0x24 }, // = Triple
		//	{ 30, 0x25 }, // = Small Meat
		//	{ 10, 0x26 }, // = Large Meat
		//	{/*2*/0, 0x27 }, // = Orb
		//	{ 2, 0x28 }, // = 1Up
		//	{ 60, 0x62 }, // = Money (300)
		//	{ 40, 0xA2 }, // = Money (500)
		//	{ 20, 0xE2 }, // = Money (700)
		//};
		WORD offset = 0;

		// render the candle
		offset = *LPWORD(nmmx.rom + SNESCore::snes2pc(0x81A654 + 0));
		RenderObject(hDC, x, y, 0, 0, offset);

		// render the drop
		offset = *LPWORD(nmmx.rom + SNESCore::snes2pc(0x81A654 + (((id & 0x3F) - 0x16) << 1)));
		RenderObject(hDC, x + set.dropXOffset, y + set.dropYOffset, 0, 0, offset);
	}
	else if ((type == 0x0 || type == 0x2) && nmmx.type == 0 && nmmx.region == 0) {
		WORD offset = 0x0;

		if (type == 0) {
			switch (id) {
			case 0x07: offset = 0x916B; break; // medusa head
			case 0x08: offset = 0x89D1; break; // ghost
			case 0x09: offset = 0x8BEA; break; // porcupine
			case 0x0A: offset = 0x8CFA; break; // undead dog
			case 0x0B: offset = 0x8D70; break; // turret
			case 0x4E:
			case 0x0C: offset = 0x8D61; break; // bat
			case 0x0F: offset = 0x8EAB; break; // book
			case 0x10: offset = 0x8EB9; break; // crow
			case 0x11: offset = 0x8AF4; break; // skeleton
			case 0x12: offset = 0x8AF4; break; // skeleton
			case 0x2A: offset = 0xAD7D; break; // first boss
			case 0x2C: offset = 0x99EE; break; // wall monster
			case 0x30: offset = 0x8DEF; break; // frog
			case 0x31: offset = 0x93BE; break; // skeleton
			case 0x32: offset = 0x8A41; break; // tentacles
			case 0x33: offset = 0x8B66; break; // coffin
			case 0x34: offset = 0x8D89; break; // cave golem
			case 0x35: offset = 0x9A8C; break; // plant
			case 0x36: offset = 0x9AC7; break; // ceiling skeleton
			case 0x3C: offset = 0x8AA3; break; // leaf thing
			case 0x3E: offset = 0x8976; break; // gargyle
			case 0x42: offset = 0x90D7; break; // table
			case 0x43: offset = 0x97BB; break; // spider
			case 0x44: offset = 0xA5D1; break; // stalagtite
			case 0x52: offset = 0x8F5C; break; // axe knight
			case 0x54: offset = 0x9226; break; // ghoul thing
			case 0x56: offset = 0x919D; break; // skeleton
			case 0x59: offset = 0x9390; break; // spear knight
			case 0x5A: offset = 0x8E41; break; // female ghost
			case 0x5B: offset = 0x8E6B; break; // male ghost
			case 0x60: offset = 0xA50C; break; // vegas
			case 0x69: offset = 0x9470; break; // skeleton
			case 0x6B: offset = 0x8D50; break; // candle stealing dog
			case 0x6D: offset = 0x965A; break; // invicible round thing
			case 0x6E: offset = 0x96BE; break; // arm
			case 0x6F:
			case 0x71: offset = 0x962E; break; // horse head
			case 0x72: offset = 0x9717; break; // eyeball
			case 0x73: offset = 0x96E4; break; // club guy
			case 0x74: offset = 0x9730; break; // flame worm
			case 0x75: offset = 0x973F; break; // flying imp
			case 0x76: offset = 0x984C; break; // dancing couple ghost
			case 0x7C: offset = 0xE47C; break; // gear
			case 0x7E: offset = 0x95EB; break; // headless sword guy
			case 0x7F: offset = 0x9B59; break; // stone golem // 9B8B
			default:
				offset = 0x0;
				break;
			}
		}
		else if (type == 0x2) {
			switch (id) {
			case 0x03: offset = 0xA4F8; break; // ring
			case 0x37: offset = 0xA5A5; break; // 1-1-1 breaking block
			case 0x3B: offset = 0xA54A; break; // flipping thing
			case 0x17: offset = 0xA6EF; break; // moving platform
			case 0x3A: offset = 0xA597; break; // dropping waterfall platform
			case 0x06: offset = 0xA74B; break; // dropping mode7 platform
			case 0x16: offset = 0xA682; break; // mode7 spike thing
			case 0x4D: offset = 0xA7BC; break; // chandilier
			case 0x5F: offset = 0x9DD9; break;
			case 0x4A: offset = 0xA754; break;
			case 0x5C: offset = 0x9A6A; break; // hands // 93BE
			case 0x61: offset = 0xA85B; break; // swinging spike platform
			case 0x62: offset = 0xA84A; break; // up/down spike thing
			//case 0x7C: offset = 0xC3F3; break;
			case 0x5D: offset = 0x94C7; break; // bone dragon/snake thing
			default:
				offset = 0x0;
				break;
			}
		}

		// 4D 0xA7D5,0xA7BC // chandelier

		// calculate position

		// per enemy graphics identification - required because some ids share gfx
		WORD gfxOffset = *LPWORD(nmmx.rom + SNESCore::snes2pc(0x81A900 + 3 * id));
		WORD levelOffset = *LPWORD(nmmx.rom + SNESCore::snes2pc(0x868BCD + 2 * nmmx.level));
		LPBYTE spriteLoad = nmmx.rom + SNESCore::snes2pc(0x860000 + levelOffset);
		BYTE num = *spriteLoad++;

		bool found = false;
		unsigned slotNum = 0;
		//bool bossGfx = false;

		for (unsigned i = 0; i < num && offset; i++) {
			BYTE index = *spriteLoad++;
			BYTE count = *(nmmx.rom + SNESCore::snes2pc(0x81AA80 + index));
			WORD gfxOffsetCurrent = *LPWORD(nmmx.rom + SNESCore::snes2pc(0x81A900 + 3 * index));

			if (gfxOffset == gfxOffsetCurrent) {
				found = true;
				break;
			}

			slotNum += count;
		}

#if 0
		if ((type == 0x0 || type == 0x2) && id == 0x2A) {
			// boss
			bossGfx = true;
			switch (subId) {
			case 0x02: offset = 0xB230; found = true; break; // medusa
			default:
				offset = 0x0;
				break;
			}
		}
#endif

		if (found) {
			WORD slotOffset = *LPWORD(nmmx.rom + SNESCore::snes2pc(0x819534 + 0x13A0 + 2 * slotNum));
			RenderObject(hDC, x, y, id, 0, offset, slotOffset);
		}
		else {
			// not loaded
			RECT rect;
			rect.left = x - 5;
			rect.top = y - 5;
			rect.bottom = y + 5;
			rect.right = x + 5;

			HBRUSH brush;
			switch (type) {
			case 0: brush = CreateSolidBrush(RGB(0, 0, 255)); break;
			case 1: brush = CreateSolidBrush(RGB(0, 255, 0)); break;
			case 2: brush = CreateSolidBrush(RGB(255, 8, 127)); break;
			default: brush = CreateSolidBrush(RGB(0, 255, 255)); break;
			}
			FrameRect(hDC, &rect, brush);
			DeleteObject(brush);
		}
	}
	else if ((type == 0x0 || type == 0x1) && nmmx.type == 2 && nmmx.region == 0) {
		WORD offset = 0x0;

		if (type == 0 || type == 1) {
			switch (id) {
				//case 0x0A: offset = 0x8721; break;
			case 0x1C: offset = 0x0; break;
			default:
				//offset = 0x0;
				offset = *LPWORD(nmmx.rom + SNESCore::snes2pc(0x81B2BE + 2 * id));
				break;
			}
		}

		// calculate position
		bool found = offset && !(  id == 0x21 || id == 0x2C || id == 0x54 || id == 0x55 || id == 0x5E || id == 0x6D || id == 0x73
								|| id == 0x74 || id == 0x26 || id == 0x36 || id == 0x3C || id == 0x7C || id == 0x47 || id == 0x10
								|| id == 0x48 || id == 0x4B || id == 0x49 || id == 0x4A || id == 0x4E || id == 0x60 || id == 0x5F
								|| id == 0x7D || id == 0x53 || id == 0x4C || id == 0x7E || id == 0x63 || id == 0x67 || id == 0x5C
								|| id == 0x5B || id == 0x65 || id == 0x66 || id == 0x7A
								);
		unsigned slotNum = 0;

		// fix for offsetting of standard enemy types
		x -= 0x18;
		y -= 0x10;

		if (found) {
			WORD slotOffset = (id < 0x10 || id == 0x61) ? 0 : 0x100;//*LPWORD(nmmx.rom + SNESCore::snes2pc(0x819534 + 0x13A0 + 2 * slotNum));
			RenderObject(hDC, x, y, id, 0, offset, slotOffset);
		}
		else {
			// not loaded
			RECT rect;
			rect.left = x - 5;
			rect.top = y - 5;
			rect.bottom = y + 5;
			rect.right = x + 5;

			HBRUSH brush;
			switch (type) {
			case 0: brush = CreateSolidBrush(RGB(0, 0, 255)); break;
			case 1: brush = CreateSolidBrush(RGB(0, 255, 0)); break;
			case 2: brush = CreateSolidBrush(RGB(255, 8, 127)); break;
			default: brush = CreateSolidBrush(RGB(0, 255, 255)); break;
			}
			FrameRect(hDC, &rect, brush);
			DeleteObject(brush);
		}
	}
	else {
		RECT rect;
		rect.left = x - 5;
		rect.top = y - 5;
		rect.bottom = y + 5;
		rect.right = x + 5;

		HBRUSH brush;
		switch (type) {
		case 0: brush = CreateSolidBrush(RGB(0, 0, 255)); break;
		case 1: brush = CreateSolidBrush(RGB(0, 255, 0)); break;
		case 2: brush = CreateSolidBrush(RGB(255, 8, 127)); break;
		default: brush = CreateSolidBrush(RGB(0, 255, 255)); break;
		}
		FrameRect(hDC, &rect, brush);
		DeleteObject(brush);
	}
}
void RenderED::RenderObject(HDC hDC, int x, int y, unsigned gfxNum, unsigned palNum, int assemblyNum, int slotOffset) {
	BYTE tram[0x20000];

	unsigned current = 0;
	unsigned size = 0;
	unsigned frame = 0;

	unsigned offset = SNESCore::snes2pc(((nmmx.type == 0 ? 0x84 : nmmx.type == 1 ? 0x0 : 0x83) << 16) | assemblyNum);

	LPBYTE tileBase = nmmx.rom + offset;
	unsigned tileCnt = *tileBase++;

	if (assemblyNum != -1) {
		RECT boundingBox;
		boundingBox.left = LONG_MAX;
		boundingBox.right = 0;
		boundingBox.bottom = 0;
		boundingBox.top = LONG_MAX;

		for (unsigned i = 0; i < tileCnt; ++i) {
			char xRel = *tileBase++;
			char yRel = *tileBase++;
			WORD map = *LPWORD(tileBase);
			tileBase += 2;

			unsigned tile = (map & 0xFF) + slotOffset;
			unsigned info = (map >> 8) & 0xFF;

			bool largeSprite = (info & 0x20) ? true : false;

			unsigned h = (info >> 6) & 0x1;
			unsigned v = (info >> 7) & 0x1;
			unsigned pal = (info >> 1) & 0x7;

			for (unsigned j = 0; j < (largeSprite ? 4 : 1); ++j) {
				int xposOffset = (j % 2) * 8;
				int yposOffset = (j / 2) * 8;
				unsigned tileOffset = (largeSprite) ? (j ^ (h ? 0x1 : 0x00) ^ (v ? 0x2 : 0x00)) : j;

				int screenX = x + xRel + xposOffset;
				int screenY = y + yRel + yposOffset;

				render.RenderSpriteTile(hDC, screenX, screenY, TILE(0, 0, 0, h, v), (tile + (tileOffset % 2) * 1 + (tileOffset / 2) * 16), 0, pal, true);

				if (screenX < boundingBox.left) boundingBox.left = screenX;
				if (boundingBox.right < screenX + 8) boundingBox.right = screenX + 8;
				if (screenY < boundingBox.top) boundingBox.top = screenY;
				if (boundingBox.bottom < screenY + 8) boundingBox.bottom = screenY + 8;
			}
		}
	}
	else {
		for (int i = 0; i < 400; i++) {
			unsigned tileNum = i;
			render.RenderSpriteTile(hDC, (i % 20) << 3, (i / 20) << 3, TILE(0, 0, 0, 0, 0), tileNum, 0, 0);
		}
	}

}
void RenderED::ShowCollisionIndex(HDC hdc, int x, int y, WORD index)
{
	x <<= 4;
	y <<= 4;
	LPWORD pmap = pmmx->mapping + (index<<8);
	LPBYTE collision = (LPBYTE)(nmmx.rom + nmmx.pCollisions);
	for(int iy=0; iy<16; iy++)
		for(int ix=0; ix<16; ix++, pmap++)
			if (collision[*pmap])
				render.PutH(hdc, (ix + x)*2, (iy + y)*2, collision[*pmap], (ix&1) + 5);
}

void RenderED::CreateFontCache(HWND hWND)
{
	HDC dcMain = GetDC(hWnd);
	hFont = CreateCompatibleBitmap(dcMain, 0x100 * 8, 64);
	dcFont = CreateCompatibleDC(dcMain);
	SelectObject(dcFont, hFont);
	RefreshFontCache();
	ReleaseDC(hWnd, dcMain);
}
void RenderED::RefreshFontCache()
{
	for(int c=0; c<8; c++)
	{
		for(int i=0; i<0x100; i++)
		{
			LPWORD xdata = data;
			HDC dcBmp = CreateCompatibleDC(dcWork[currentBuffer]);
			BYTE palette = c<<2;
			LPBYTE image = pmmx->fontCache + ((i & 0x3FF)<<6);
			for(int d=0; d<0x40; d++)
				*xdata++ = pmmx->fontPalCache[*image++ | palette];
			SelectObject(dcBmp, hBmpTile);
			if (i & 0x4000) StretchBlt(dcBmp, 0, 0, 8, 8, dcBmp, 7, 0, -8, 8, SRCCOPY);
			if (i & 0x8000) StretchBlt(dcBmp, 0, 0, 8, 8, dcBmp, 0, 7, 8, -8, SRCCOPY);
			BitBlt(dcFont, i*8, c*8, 8, 8, dcBmp, 0, 0, SRCCOPY);
			DeleteDC(dcBmp);
		}
	}
}
void RenderED::PutC(HDC hdc, int x, int y, unsigned char ch, char color)
{
	TransparentBlt(hdc, x*8, y*8, 8, 8, dcFont, ch * 8, color*8, 8, 8, 0);
}
void RenderED::PutH(HDC hdc, int x, int y, unsigned char hx, char color)
{

	//TransparentBlt(hdc, x*8 + 0, y*8, 8, 8, dcFont, hexCharIndex[hx>>4 ] * 8, color*8, 8, 8, 0);
	//TransparentBlt(hdc, x*8 + 8, y*8, 8, 8, dcFont, hexCharIndex[hx&0xF] * 8, color*8, 8, 8, 0);
	SetBkMode(hdc, TRANSPARENT);

	std::string str;
	str += hexCharIndex[hx >> 4];
	str += hexCharIndex[hx & 0xF];

	// couldn't get the Path drawing to work so draw some black values behind
#if 1
	auto textColor = SetTextColor(hdc, RGB(0, 0, 0));
	for (int i = -1; i < 2; i+=2) {
		for (int j = -1; j < 2; j+=2) {
			ExtTextOut(hdc, x * 8 + i, y * 8 + j, 0, NULL, str.c_str(), strlen(str.c_str()), NULL);
		}
	}
	SetTextColor(hdc, (color & 0x1) ? RGB(205, 255, 0) : RGB(255, 105, 180));
	ExtTextOut(hdc, x * 8, y * 8, 0, NULL, str.c_str(), strlen(str.c_str()), NULL);
#else
	auto textColor = SetTextColor(hdc, (color & 0x1) ? RGB(205, 255, 0) : RGB(255, 105, 180));
	ExtTextOut(hdc, x * 8, y * 8, 0, NULL, str.c_str(), strlen(str.c_str()), NULL);

	//auto pen = CreatePen(PS_SOLID, 1, RGB(0, 0, 0));
	//auto brush = CreateSolidBrush((color & 0x1) ? RGB(205, 255, 0) : RGB(255, 105, 180));

	//SelectObject(hdc, pen);
	//SelectObject(hdc, brush);
	SetTextColor(hdc, RGB(0, 0, 0));
	ExtTextOut(hdc, x * 8, y * 8, 0, NULL, str.c_str(), strlen(str.c_str()), NULL);

	//DeleteObject(pen);
	//DeleteObject(brush);
#endif

	SetTextColor(hdc, textColor);
}
void RenderED::Print(HDC hdc, int x, int y, LPCSTR text, char color)
{
	int len = (int)strlen(text);
	auto t = GetPixel(dcFont, 0, 0);
	for (int i = 0; i < len; i++) {
		char c = toupper(*text++);
		c = 0x10 + (std::isalnum(c) ? (c - (isdigit(c) ? 0x2F : nmmx.type == 0 ? 0x36 : nmmx.type == 1 ? (0x5 + (c == 'P' ? 0x40 : c >= 'P' ? 1 : 0)) : 0x36)) : (c == '=' ? (nmmx.type == 0 ? 0x25 : nmmx.type == 1 ? 0x31 : 0x25) : c == '-' ? (nmmx.type == 0 ? 0x26 : nmmx.type == 1 ? 0x31 : 0x26) : 0x0));
		TransparentBlt(hdc, (x + i) * 8, y * 8, 8, 8, dcFont, c * 8, color*8, 8, 8, t);
	}
}
void RenderED::PrintBig(HDC hdc, int x, int y, LPCSTR text, char color)
{
	int len = (int)strlen(text);
	for(int i=0; i<len; i++, text++)
	{
		if (*text != 0x20)
		{
			TransparentBlt(hdc, (x+i)*8, y*8 + 0, 8, 8, dcFont, _bigTextAlloc[pmmx->type][(*text-0x41)*2 + 0] * 8, color*8, 8, 8, 0);
			TransparentBlt(hdc, (x+i)*8, y*8 + 8, 8, 8, dcFont, _bigTextAlloc[pmmx->type][(*text-0x41)*2 + 1] * 8, color*8, 8, 8, 0);
		}
		else
		{
			TransparentBlt(hdc, (x+i)*8, y*8 + 0, 8, 8, dcFont, ' ' * 8, color*8, 8, 8, 0);
			TransparentBlt(hdc, (x+i)*8, y*8 + 8, 8, 8, dcFont, ' ' * 8, color*8, 8, 8, 0);
		}
	}
}
void RenderED::PrintCopyright(HDC hdc, int x, int y, char color)
{
	BitBlt(hdc, x*8 + 0, y*8 + 0, 8, 8, dcFont, 0x7B *8, color*8, SRCCOPY);
	BitBlt(hdc, x*8 + 8, y*8 + 0, 8, 8, dcFont, 0x7C *8, color*8, SRCCOPY);
	BitBlt(hdc, x*8 + 0, y*8 + 8, 8, 8, dcFont, 0x7D *8, color*8, SRCCOPY);
	BitBlt(hdc, x*8 + 8, y*8 + 8, 8, 8, dcFont, 0x7E *8, color*8, SRCCOPY);
}
