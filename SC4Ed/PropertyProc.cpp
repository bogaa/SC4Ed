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

#include "SC4Ed.h"
//#include "Emulator.h"
#include <sstream>
#include <map>

#define IsCheck(i) SendMessage(GetDlgItem(hWnd, i), BM_GETCHECK, NULL, NULL) == BST_CHECKED

static bool InitOk = false;

static std::map<unsigned, std::tuple<unsigned, std::vector<unsigned>>> AddressMap = {
	{ IDC_PROPERTY_LIVES,			{ 2, { 0x8094DB } } },
	{ IDC_PROPERTY_CONTINUE_LIVES,  { 2, { 0x8CFD9B } } },
	{ IDC_PROPERTY_TIMER,           { 2, { 0x000000 } } },

	{ IDC_PROPERTY_WHIPLENGTH,      { 2, { 0x000000 } } },
	{ IDC_PROPERTY_WHIPFULLDAMAGE,  { 2, { 0x000000 } } },
	{ IDC_PROPERTY_WHIPPARTIALDAMAGE,{ 2, { 0x000000 } } },

	{ IDC_PROPERTY_MOVEPIXEL,       { 2, { 0x000000, 0x000000 } } },
	{ IDC_PROPERTY_MOVESUBPIXEL,    { 2, { 0x000000 } } },

	{ IDC_PROPERTY_CAMERA_LEFT,     { 2, { 0x000000 } } },
	{ IDC_PROPERTY_CAMERA_RIGHT,    { 2, { 0x000000 } } },
	{ IDC_PROPERTY_CAMERA_TOP,      { 2, { 0x000000 } } },
	{ IDC_PROPERTY_CAMERA_BOTTOM,   { 2, { 0x000000 } } },
	{ IDC_PROPERTY_CAMERA_SPEED_X,   { 2, { 0x000000 } } },
	{ IDC_PROPERTY_CAMERA_SPEED_Y,   { 2, { 0x000000 } } },
	{ IDC_PROPERTY_CAMERA_POINTER_DONNO,   { 2, { 0x000000 } } },
	
	{ IDC_PROPERTY_LEVEL_TYPE,		{ 2, { 0x000000 } } },
	{ IDC_PROPERTY_LEVEL_DEATH_LEVEL,{ 1, { 0x000000 } } },
	{ IDC_PROPERTY_LEVEL_CONTINUE_LEVEL,{ 1, { 0x000000 } } },
	{ IDC_PROPERTY_LEVEL_MUSIC,{ 1, { 0x000000 } } },	
	{ IDC_PROPERTY_LEVEL_LAYER_MASK,{ 2, { 0x000000 } } },
	{ IDC_PROPERTY_LEVEL_LAYER_BEHAVIER,{ 2, { 0x000000 } } },
	{ IDC_PROPERTY_LEVEL_EVENT_DIRECTION,{ 1, { 0x000000 } } },
	{ IDC_PROPERTY_LEVEL_BG_ANIMATION_0,{ 2, { 0x000000 } } },
	{ IDC_PROPERTY_LEVEL_BG_ANIMATION_1,{ 2, { 0x000000 } } },
	{ IDC_PROPERTY_LEVEL_PALETTE_ANIMATION,{ 2, { 0x000000 } } },
	{ IDC_PROPERTY_LEVEL_ENEMY_SET_ID,{ 2, { 0x000000 } } },
	{ IDC_PROPERTY_LEVEL_ENEMY_SET,{ 2, { 0x000000 } } },

	{ IDC_PROPERTY_CHECKPOINT_STATE0,{ 1,{ 0x000000 } } },
	{ IDC_PROPERTY_CHECKPOINT_STATE1,{ 2,{ 0x000000 } } },
	{ IDC_PROPERTY_CHECKPOINT_XPOS,{ 2,{ 0x000000 } } },
	{ IDC_PROPERTY_CHECKPOINT_YPOS,{ 2,{ 0x000000 } } },
	{ IDC_PROPERTY_CHECKPOINT_CAM0XPOS,{ 2,{ 0x000000 } } },
	{ IDC_PROPERTY_CHECKPOINT_CAM0YPOS,{ 2,{ 0x000000 } } },
	{ IDC_PROPERTY_CHECKPOINT_CAM1XPOS,{ 2,{ 0x000000 } } },
	{ IDC_PROPERTY_CHECKPOINT_CAM1YPOS,{ 2,{ 0x000000 } } },

	{ IDC_PROPERTY_CAMERALOCK_DIRECTION,{ 2,{ 0x000000 } } },
	{ IDC_PROPERTY_CAMERALOCK_DIRADDR,{ 2,{ 0x000000 } } },
	{ IDC_PROPERTY_CAMERALOCK_CMPADDR,{ 2,{ 0x000000 } } },
	{ IDC_PROPERTY_CAMERALOCK_CMPVALUE,{ 2,{ 0x000000 } } },
	{ IDC_PROPERTY_CAMERALOCK_STVALUE,{ 2,{ 0x000000 } } },
	{ IDC_PROPERTY_CAMERALOCK_STADDR,{ 2,{ 0x000000 } } },

	{ IDC_PROPERTY_NEXTLEVEL_NEXTLEVEL,{ 1,{ 0x000000 } } },
	{ IDC_PROPERTY_NEXTLEVEL_ENTRANCENUM,{ 1,{ 0x000000 } } },
	{ IDC_PROPERTY_NEXTLEVEL_DEATHNUM,{ 1,{ 0x000000 } } },

	{ IDC_PROPERTY_EXITCHECK_TYPE,{ 1,{ 0x000000 } } },
	{ IDC_PROPERTY_EXITCHECK_EXITNUM,{ 1,{ 0x000000 } } },
	{ IDC_PROPERTY_EXITCHECK_CMPVALUE,{ 2,{ 0x000000 } } },

};

static void SetupMap(HWND hWnd) {
	std::get<1>(AddressMap[IDC_PROPERTY_TIMER]).assign(1, 0x85BCF8 + 2 * nmmx.level);
	std::get<1>(AddressMap[IDC_PROPERTY_TIMER]).assign(1, 0x85BCF8 + 2 * nmmx.level);
	auto sel = SendMessage(GetDlgItem(hWnd, IDC_PROPERTY_WHIP_COMBO), CB_GETCURSEL, (WPARAM)0, (LPARAM)0);
	std::get<1>(AddressMap[IDC_PROPERTY_WHIPLENGTH]).assign(1, 0x819261 + 2 * sel);
	std::get<1>(AddressMap[IDC_PROPERTY_WHIPFULLDAMAGE]).assign(1, 0x81A6EC + 4 * sel);
	std::get<1>(AddressMap[IDC_PROPERTY_WHIPPARTIALDAMAGE]).assign(1, 0x81A6EC + 4 * sel + 2);

	switch (SendMessage(GetDlgItem(hWnd, IDC_PROPERTY_MOVE_COMBO), CB_GETCURSEL, (WPARAM)0, (LPARAM)0)) {
	case 0:
		std::get<1>(AddressMap[IDC_PROPERTY_MOVESUBPIXEL]).assign(1, 0x80A65F);
		std::get<1>(AddressMap[IDC_PROPERTY_MOVEPIXEL]).assign(1, 0x80A665);
		break;
	case 1:
		std::get<1>(AddressMap[IDC_PROPERTY_MOVESUBPIXEL]).assign(1, 0x80A916);
		std::get<1>(AddressMap[IDC_PROPERTY_MOVEPIXEL]).assign(1, 0x80A90B);
		std::get<1>(AddressMap[IDC_PROPERTY_MOVEPIXEL]).push_back(0x80A910);
		break;
	case 2:
		std::get<1>(AddressMap[IDC_PROPERTY_MOVESUBPIXEL]).assign(1, 0x80A6FF);
		std::get<1>(AddressMap[IDC_PROPERTY_MOVEPIXEL]).assign(1, 0x80A705);
		break;
	case 3:
		std::get<1>(AddressMap[IDC_PROPERTY_MOVESUBPIXEL]).assign(1, 0x80A8C2);
		std::get<1>(AddressMap[IDC_PROPERTY_MOVEPIXEL]).assign(1, 0x80A8C8);
		break;
	case 4:
		std::get<1>(AddressMap[IDC_PROPERTY_MOVESUBPIXEL]).assign(1, 0x80A73E);
		std::get<1>(AddressMap[IDC_PROPERTY_MOVEPIXEL]).assign(1, 0x0);
		break;
	}

	std::get<1>(AddressMap[IDC_PROPERTY_TIMER]).assign(1, 0x85BCF8 + 2 * nmmx.level);
	std::get<1>(AddressMap[IDC_PROPERTY_LEVEL_TYPE]).assign(1, 0x868296 + 2 * nmmx.level);
	std::get<1>(AddressMap[IDC_PROPERTY_LEVEL_DEATH_LEVEL]).assign(1, (nmmx.region == 0 ? 0x81B395 : 0x81B369) + nmmx.level);
	std::get<1>(AddressMap[IDC_PROPERTY_LEVEL_CONTINUE_LEVEL])	.assign(1, (nmmx.region == 0 ? 0x81FBAC : 0x81FBAC) + nmmx.level);
	std::get<1>(AddressMap[IDC_PROPERTY_LEVEL_MUSIC]).assign(1, 0x8097C3 + nmmx.level);	
	std::get<1>(AddressMap[IDC_PROPERTY_LEVEL_LAYER_BEHAVIER]).assign(1, 0x85C846 + 2 * nmmx.level);
	std::get<1>(AddressMap[IDC_PROPERTY_LEVEL_EVENT_DIRECTION]).assign(1, 0x80D8A3 + nmmx.level);
	std::get<1>(AddressMap[IDC_PROPERTY_LEVEL_LAYER_MASK]).assign(1, 0x85C7BE + 2 * nmmx.level);
	std::get<1>(AddressMap[IDC_PROPERTY_LEVEL_BG_ANIMATION_0]).assign(1, 0x85ca82 + 2 * nmmx.level);
	std::get<1>(AddressMap[IDC_PROPERTY_LEVEL_BG_ANIMATION_1]).assign(1, 0x85cb0a + 2 * nmmx.level);
	std::get<1>(AddressMap[IDC_PROPERTY_LEVEL_PALETTE_ANIMATION]).assign(1, 0x86946f + 2 * nmmx.level);
	std::get<1>(AddressMap[IDC_PROPERTY_LEVEL_ENEMY_SET_ID]).assign(1, 0x868BCD + 2 * nmmx.level);
	std::get<1>(AddressMap[IDC_PROPERTY_LEVEL_ENEMY_SET]).assign(1, 0x868B45 + 2 * nmmx.level);

	EnableWindow(GetDlgItem(hWnd, IDC_PROPERTY_CAMERALOCK_COMBO), nmmx.expandedROM);
	EnableWindow(GetDlgItem(hWnd, IDC_PROPERTY_CAMERALOCK_DIRECTION), nmmx.expandedROM);
	EnableWindow(GetDlgItem(hWnd, IDC_PROPERTY_CAMERALOCK_DIRADDR), nmmx.expandedROM);
	EnableWindow(GetDlgItem(hWnd, IDC_PROPERTY_CAMERALOCK_STADDR), nmmx.expandedROM);
	EnableWindow(GetDlgItem(hWnd, IDC_PROPERTY_CAMERALOCK_STVALUE), nmmx.expandedROM);
	EnableWindow(GetDlgItem(hWnd, IDC_PROPERTY_CAMERALOCK_CMPVALUE), nmmx.expandedROM);
	EnableWindow(GetDlgItem(hWnd, IDC_PROPERTY_CAMERALOCK_CMPADDR), nmmx.expandedROM);

	EnableWindow(GetDlgItem(hWnd, IDC_PROPERTY_NEXTLEVEL_COMBO), nmmx.expandedROM);
	EnableWindow(GetDlgItem(hWnd, IDC_PROPERTY_NEXTLEVEL_NEXTLEVEL), nmmx.expandedROM);
	EnableWindow(GetDlgItem(hWnd, IDC_PROPERTY_NEXTLEVEL_ENTRANCENUM), nmmx.expandedROM);
	EnableWindow(GetDlgItem(hWnd, IDC_PROPERTY_NEXTLEVEL_DEATHNUM), nmmx.expandedROM);

	EnableWindow(GetDlgItem(hWnd, IDC_PROPERTY_EXITCHECK_COMBO), nmmx.expandedROM);
	EnableWindow(GetDlgItem(hWnd, IDC_PROPERTY_EXITCHECK_TYPE), nmmx.expandedROM);
	EnableWindow(GetDlgItem(hWnd, IDC_PROPERTY_EXITCHECK_EXITNUM), nmmx.expandedROM);
	EnableWindow(GetDlgItem(hWnd, IDC_PROPERTY_EXITCHECK_CMPVALUE), nmmx.expandedROM);

	EnableWindow(GetDlgItem(hWnd, IDC_PROPERTY_CHECKPOINT_STATE0), nmmx.expandedROM);
	EnableWindow(GetDlgItem(hWnd, IDC_PROPERTY_CHECKPOINT_STATE1), nmmx.expandedROM);
	EnableWindow(GetDlgItem(hWnd, IDC_PROPERTY_CHECKPOINT_XPOS), nmmx.expandedROM);
	EnableWindow(GetDlgItem(hWnd, IDC_PROPERTY_CHECKPOINT_YPOS), nmmx.expandedROM);
	EnableWindow(GetDlgItem(hWnd, IDC_PROPERTY_CHECKPOINT_CAM0XPOS), nmmx.expandedROM);
	EnableWindow(GetDlgItem(hWnd, IDC_PROPERTY_CHECKPOINT_CAM0YPOS), nmmx.expandedROM);
	EnableWindow(GetDlgItem(hWnd, IDC_PROPERTY_CHECKPOINT_CAM1XPOS), nmmx.expandedROM);
	EnableWindow(GetDlgItem(hWnd, IDC_PROPERTY_CHECKPOINT_CAM1YPOS), nmmx.expandedROM);

	EnableWindow(GetDlgItem(hWnd, IDC_PROPERTY_CAMERA_LEFT), nmmx.expandedROM);
	EnableWindow(GetDlgItem(hWnd, IDC_PROPERTY_CAMERA_RIGHT), nmmx.expandedROM);
	EnableWindow(GetDlgItem(hWnd, IDC_PROPERTY_CAMERA_TOP), nmmx.expandedROM);
	EnableWindow(GetDlgItem(hWnd, IDC_PROPERTY_CAMERA_BOTTOM), nmmx.expandedROM);

	if (nmmx.expandedROM) {
		auto lockNum = SendMessage(GetDlgItem(hWnd, IDC_PROPERTY_CAMERALOCK_COMBO), CB_GETCURSEL, (WPARAM)0, (LPARAM)0);
		std::get<1>(AddressMap[IDC_PROPERTY_CAMERALOCK_DIRECTION]).assign(1, 0xA58000 + 0xC * 0x20 * nmmx.level + 0xC * lockNum + 0x0);
		std::get<1>(AddressMap[IDC_PROPERTY_CAMERALOCK_DIRADDR]).assign(1, 0xA58000 + 0xC * 0x20 * nmmx.level + 0xC * lockNum + 0x2);
		std::get<1>(AddressMap[IDC_PROPERTY_CAMERALOCK_CMPADDR]).assign(1, 0xA58000 + 0xC * 0x20 * nmmx.level + 0xC * lockNum + 0x4);
		std::get<1>(AddressMap[IDC_PROPERTY_CAMERALOCK_CMPVALUE]).assign(1, 0xA58000 + 0xC * 0x20 * nmmx.level + 0xC * lockNum + 0x6);
		std::get<1>(AddressMap[IDC_PROPERTY_CAMERALOCK_STVALUE]).assign(1, 0xA58000 + 0xC * 0x20 * nmmx.level + 0xC * lockNum + 0x8);
		std::get<1>(AddressMap[IDC_PROPERTY_CAMERALOCK_STADDR]).assign(1, 0xA58000 + 0xC * 0x20 * nmmx.level + 0xC * lockNum + 0xA);

		auto direction = SendMessage(GetDlgItem(hWnd, IDC_PROPERTY_NEXTLEVEL_COMBO), CB_GETCURSEL, (WPARAM)0, (LPARAM)0);
		std::get<1>(AddressMap[IDC_PROPERTY_NEXTLEVEL_NEXTLEVEL]).assign(1, 0xA0C000 + 0x10 * nmmx.level + 0x2 * direction + 0x0);
		std::get<1>(AddressMap[IDC_PROPERTY_NEXTLEVEL_ENTRANCENUM]).assign(1, 0xA0C000 + 0x10 * nmmx.level + 0x2 * direction + 0x1);

		auto exitNum = SendMessage(GetDlgItem(hWnd, IDC_PROPERTY_EXITCHECK_COMBO), CB_GETCURSEL, (WPARAM)0, (LPARAM)0);
		std::get<1>(AddressMap[IDC_PROPERTY_EXITCHECK_TYPE]).assign(1, 0xA68000 + 0x40 * 0x4 * nmmx.level + 0x4 * exitNum + 0);
		std::get<1>(AddressMap[IDC_PROPERTY_EXITCHECK_EXITNUM]).assign(1, 0xA68000 + 0x40 * 0x4 * nmmx.level + 0x4 * exitNum + 1);
		std::get<1>(AddressMap[IDC_PROPERTY_EXITCHECK_CMPVALUE]).assign(1, 0xA68000 + 0x40 * 0x4 * nmmx.level + 0x4 * exitNum + 2);

		auto entranceNum = SendMessage(GetDlgItem(hWnd, IDC_PROPERTY_CHECKPOINT_COMBO), CB_GETCURSEL, (WPARAM)0, (LPARAM)0);
		std::get<1>(AddressMap[IDC_PROPERTY_CHECKPOINT_STATE0]).assign(1, 0xA78000 + 0x100 * nmmx.level + 0x20 * entranceNum + 0x0);
		std::get<1>(AddressMap[IDC_PROPERTY_CHECKPOINT_STATE1]).assign(1, 0xA78000 + 0x100 * nmmx.level + 0x20 * entranceNum + 0xE);
		std::get<1>(AddressMap[IDC_PROPERTY_CHECKPOINT_XPOS]).assign(1, 0xA78000 + 0x100 * nmmx.level + 0x20 * entranceNum + 0x2);
		std::get<1>(AddressMap[IDC_PROPERTY_CHECKPOINT_YPOS]).assign(1, 0xA78000 + 0x100 * nmmx.level + 0x20 * entranceNum + 0x4);
		std::get<1>(AddressMap[IDC_PROPERTY_CHECKPOINT_CAM0XPOS]).assign(1, 0xA78000 + 0x100 * nmmx.level + 0x20 * entranceNum + 0x6);
		std::get<1>(AddressMap[IDC_PROPERTY_CHECKPOINT_CAM0YPOS]).assign(1, 0xA78000 + 0x100 * nmmx.level + 0x20 * entranceNum + 0x8);
		std::get<1>(AddressMap[IDC_PROPERTY_CHECKPOINT_CAM1XPOS]).assign(1, 0xA78000 + 0x100 * nmmx.level + 0x20 * entranceNum + 0xA);
		std::get<1>(AddressMap[IDC_PROPERTY_CHECKPOINT_CAM1YPOS]).assign(1, 0xA78000 + 0x100 * nmmx.level + 0x20 * entranceNum + 0xC);

		std::get<1>(AddressMap[IDC_PROPERTY_CAMERA_LEFT]).assign(1, 0xA78000 + 0x100 * nmmx.level + 0x20 * entranceNum + 0x12);
		std::get<1>(AddressMap[IDC_PROPERTY_CAMERA_RIGHT]).assign(1, 0xA78000 + 0x100 * nmmx.level + 0x20 * entranceNum + 0x14);
		std::get<1>(AddressMap[IDC_PROPERTY_CAMERA_TOP]).assign(1, 0xA78000 + 0x100 * nmmx.level + 0x20 * entranceNum + 0x16);
		std::get<1>(AddressMap[IDC_PROPERTY_CAMERA_BOTTOM]).assign(1, 0xA78000 + 0x100 * nmmx.level + 0x20 * entranceNum + 0x18);
		std::get<1>(AddressMap[IDC_PROPERTY_CAMERA_SPEED_X]).assign(1, 0xA78000 + 0x100 * nmmx.level + 0x20 * entranceNum + 0x1A);
		std::get<1>(AddressMap[IDC_PROPERTY_CAMERA_SPEED_Y]).assign(1, 0xA78000 + 0x100 * nmmx.level + 0x20 * entranceNum + 0x1C);
		std::get<1>(AddressMap[IDC_PROPERTY_CAMERA_POINTER_DONNO]).assign(1, 0xA78000 + 0x100 * nmmx.level + 0x20 * entranceNum + 0x1E);
		std::get<1>(AddressMap[IDC_PROPERTY_NEXTLEVEL_DEATHNUM]).assign(1, 0xA78000 + 0x100 * nmmx.level + 0x20 * entranceNum + 0x1);

	}


}

static void UpdateProperty(HWND hWnd) {
	DWORD v = 0;
	std::stringstream ss;

	for (auto &m : AddressMap) {
		for (auto &a : std::get<1>(m.second)) {
			if (a) {
				switch (std::get<0>(m.second)) {
				case 1: v = *LPBYTE(nmmx.rom + SNESCore::snes2pc(a)); break;
				case 2: v = *LPWORD(nmmx.rom + SNESCore::snes2pc(a)); break;
				case 4: v = *LPDWORD(nmmx.rom + SNESCore::snes2pc(a)); break;
				}

				ss << std::nouppercase << std::hex << v;
				TCHAR className[128];
				if (GetClassName(GetDlgItem(hWnd, m.first), className, sizeof(className)) && !strcmp(className, "ComboBox")) {
					SendMessage(GetDlgItem(hWnd, m.first), CB_SETCURSEL, (WPARAM)v, (LPARAM)0);
				}
				else {
					SetWindowText(GetDlgItem(hWnd, m.first), ss.str().c_str());
				}
				ss.str("");
			}
		}
	}
}

BOOL CALLBACK PropertyProc(HWND hWnd, UINT message, WPARAM wParam, LPARAM lParam)
{
	switch (message)
	{
	case UPDATEWINDOW: {
		SetupMap(hWnd);
		UpdateProperty(hWnd);
		break;
	}
	case WM_INITDIALOG: {
		hWID[8] = hWnd;

		std::vector<LPCSTR> entrances = { "0", "1", "2", "3", "4", "5", "6", "7" };

		std::stringstream ss;
		for (auto &c : {"Leather", "Chain0", "Chain1"}) {
			SendMessage(GetDlgItem(hWnd, IDC_PROPERTY_WHIP_COMBO), CB_ADDSTRING, (WPARAM)0, (LPARAM)c);
		}
		SendMessage(GetDlgItem(hWnd, IDC_PROPERTY_WHIP_COMBO), CB_SETCURSEL, (WPARAM)0, (LPARAM)0);

		for (auto &c : { "Walking", "Jumping", "Crouching", "Climbing", "Gravity" }) {
			SendMessage(GetDlgItem(hWnd, IDC_PROPERTY_MOVE_COMBO), CB_ADDSTRING, (WPARAM)0, (LPARAM)c);
		}
		SendMessage(GetDlgItem(hWnd, IDC_PROPERTY_MOVE_COMBO), CB_SETCURSEL, (WPARAM)0, (LPARAM)0);

		for (auto &c : entrances) {
			SendMessage(GetDlgItem(hWnd, IDC_PROPERTY_CHECKPOINT_COMBO), CB_ADDSTRING, (WPARAM)0, (LPARAM)c);
		}
		SendMessage(GetDlgItem(hWnd, IDC_PROPERTY_CHECKPOINT_COMBO), CB_SETCURSEL, (WPARAM)0, (LPARAM)0);

		for (unsigned i = 0; i < 0x20; i++) {
			ss.clear();
			ss.str("");
			ss << std::hex << i;
			SendMessage(GetDlgItem(hWnd, IDC_PROPERTY_CAMERALOCK_COMBO), CB_ADDSTRING, (WPARAM)0, (LPARAM)ss.str().c_str());
		}
		SendMessage(GetDlgItem(hWnd, IDC_PROPERTY_CAMERALOCK_COMBO), CB_SETCURSEL, (WPARAM)0, (LPARAM)0);

		for (auto &c : entrances) {
			SendMessage(GetDlgItem(hWnd, IDC_PROPERTY_NEXTLEVEL_COMBO), CB_ADDSTRING, (WPARAM)0, (LPARAM)c);
		}
		SendMessage(GetDlgItem(hWnd, IDC_PROPERTY_NEXTLEVEL_COMBO), CB_SETCURSEL, (WPARAM)0, (LPARAM)0);

		for (unsigned i = 0; i < 0x40; i++) {
			ss.clear();
			ss.str("");
			ss << std::hex << i;
			SendMessage(GetDlgItem(hWnd, IDC_PROPERTY_EXITCHECK_COMBO), CB_ADDSTRING, (WPARAM)0, (LPARAM)ss.str().c_str());
		}
		SendMessage(GetDlgItem(hWnd, IDC_PROPERTY_EXITCHECK_COMBO), CB_SETCURSEL, (WPARAM)0, (LPARAM)0);

		for (auto &c : { "Init (DONT USE)", "Stairs Up", "Stairs Down", "Left", "Right" }) {
			SendMessage(GetDlgItem(hWnd, IDC_PROPERTY_EXITCHECK_TYPE), CB_ADDSTRING, (WPARAM)0, (LPARAM)c);
		}
		SendMessage(GetDlgItem(hWnd, IDC_PROPERTY_EXITCHECK_TYPE), CB_SETCURSEL, (WPARAM)0, (LPARAM)0);

		for (auto &c : entrances) {
			SendMessage(GetDlgItem(hWnd, IDC_PROPERTY_EXITCHECK_EXITNUM), CB_ADDSTRING, (WPARAM)0, (LPARAM)c);
		}
		SendMessage(GetDlgItem(hWnd, IDC_PROPERTY_EXITCHECK_EXITNUM), CB_SETCURSEL, (WPARAM)0, (LPARAM)0);

		for (auto &c : entrances) {
			SendMessage(GetDlgItem(hWnd, IDC_PROPERTY_NEXTLEVEL_ENTRANCENUM), CB_ADDSTRING, (WPARAM)0, (LPARAM)c);
		}
		SendMessage(GetDlgItem(hWnd, IDC_PROPERTY_NEXTLEVEL_ENTRANCENUM), CB_SETCURSEL, (WPARAM)0, (LPARAM)0);

		for (auto &c : entrances) {
			SendMessage(GetDlgItem(hWnd, IDC_PROPERTY_NEXTLEVEL_DEATHNUM), CB_ADDSTRING, (WPARAM)0, (LPARAM)c);
		}
		SendMessage(GetDlgItem(hWnd, IDC_PROPERTY_NEXTLEVEL_DEATHNUM), CB_SETCURSEL, (WPARAM)0, (LPARAM)0);

		SetupMap(hWnd);
		UpdateProperty(hWnd);
		InitOk = true;
		RepaintAll();
		break;
	}
	case WM_COMMAND: {
		DWORD v = 0;
		std::stringstream ss;
		TCHAR t[8];
		bool found = false;

		if (AddressMap.count(LOWORD(wParam))) {
			if (InitOk) {
				TCHAR className[128];
				if (GetClassName(GetDlgItem(hWnd, LOWORD(wParam)), className, sizeof(className)) && !strcmp(className, "ComboBox")) {
					v = SendMessage(GetDlgItem(hWnd, LOWORD(wParam)), CB_GETCURSEL, (WPARAM)0, (LPARAM)0);
				}
				else {
					GetWindowText(GetDlgItem(hWnd, LOWORD(wParam)), t, sizeof(t) - 1);
					ss << std::hex << t;
					ss >> v;
				}
				for (auto &a : std::get<1>(AddressMap[LOWORD(wParam)])) {
					if (a) {
						switch (std::get<0>(AddressMap[LOWORD(wParam)])) {
						case 1: *LPBYTE(nmmx.rom + SNESCore::snes2pc(a)) = v; break;
						case 2: *LPWORD(nmmx.rom + SNESCore::snes2pc(a)) = v; break;
						case 4: *LPDWORD(nmmx.rom + SNESCore::snes2pc(a)) = v; break;
						}

						if (LOWORD(wParam) == IDC_PROPERTY_WHIPLENGTH) {
							auto sel = SendMessage(GetDlgItem(hWnd, IDC_PROPERTY_WHIP_COMBO), CB_GETCURSEL, (WPARAM)0, (LPARAM)0);
							*LPWORD(nmmx.rom + SNESCore::snes2pc(0x8190FB + 2 * sel)) = ((2 * v - 1) << 4) | 0x0F;
						}
						else if (LOWORD(wParam) == IDC_PROPERTY_MOVESUBPIXEL) {
							// take care of reverse direction
							switch (SendMessage(GetDlgItem(hWnd, IDC_PROPERTY_MOVE_COMBO), CB_GETCURSEL, (WPARAM)0, (LPARAM)0)) {
							case 0:
								*LPWORD(nmmx.rom + SNESCore::snes2pc(0x80A678)) = 0xFFFF - v;
								break;
							case 1:
								*LPWORD(nmmx.rom + SNESCore::snes2pc(0x80A947)) = 0xFFFF - v;
								break;
							case 2:
								*LPWORD(nmmx.rom + SNESCore::snes2pc(0x80A710)) = 0xFFFF - v;
								break;
							case 3:
								*LPWORD(nmmx.rom + SNESCore::snes2pc(0x80A8A0)) = 0xFFFF - v;
								break;
							case 4:
								break;
							}
						}
						else if (LOWORD(wParam) == IDC_PROPERTY_MOVEPIXEL) {
							// take care of reverse direction
							switch (SendMessage(GetDlgItem(hWnd, IDC_PROPERTY_MOVE_COMBO), CB_GETCURSEL, (WPARAM)0, (LPARAM)0)) {
							case 0:
								*LPWORD(nmmx.rom + SNESCore::snes2pc(0x80A67E)) = 0xFFFF - v;
								break;
							case 1:
								*LPWORD(nmmx.rom + SNESCore::snes2pc(0x80A93C)) = 0xFFFF - v;
								*LPWORD(nmmx.rom + SNESCore::snes2pc(0x80A941)) = 0xFFFF - v;
								break;
							case 2:
								*LPWORD(nmmx.rom + SNESCore::snes2pc(0x80A716)) = 0xFFFF - v;
								break;
							case 3:
								*LPWORD(nmmx.rom + SNESCore::snes2pc(0x80A8A6)) = 0xFFFF - v;
								break;
							case 4:
								break;
							}
						}

						found = true;
					}
				}
			}
		}
		else if (LOWORD(wParam) == IDC_PROPERTY_WHIP_COMBO || LOWORD(wParam) == IDC_PROPERTY_MOVE_COMBO || LOWORD(wParam) == IDC_PROPERTY_CHECKPOINT_COMBO || LOWORD(wParam) == IDC_PROPERTY_CAMERALOCK_COMBO || LOWORD(wParam) == IDC_PROPERTY_NEXTLEVEL_COMBO || LOWORD(wParam) == IDC_PROPERTY_EXITCHECK_COMBO) {
			SetupMap(hWnd);
			UpdateProperty(hWnd);
			if (LOWORD(wParam) == IDC_PROPERTY_MOVE_COMBO) { EnableWindow(GetDlgItem(hWnd, IDC_PROPERTY_MOVEPIXEL), SendMessage(GetDlgItem(hWnd, IDC_PROPERTY_MOVE_COMBO), CB_GETCURSEL, (WPARAM)0, (LPARAM)0) != 4); }
			found = true;
		}
		else if (LOWORD(wParam) == IDC_PROPERTY_ENTRANCE_RECORD) {
			FrameState s;
			GetFrameState(s);

			auto sel = SendMessage(GetDlgItem(hWnd, IDC_PROPERTY_CHECKPOINT_COMBO), CB_GETCURSEL, (WPARAM)0, (LPARAM)0);
			nmmx.GetLevelEntrance(sel).deathState0 = (nmmx.GetLevelEntrance(sel).deathState0 & 0xFF00) | (s.state0 & 0x00FF);
			nmmx.GetLevelEntrance(sel).state1 = s.state1;
			nmmx.GetLevelEntrance(sel).xpos = s.s_xpos;
			nmmx.GetLevelEntrance(sel).ypos = s.s_ypos;
			nmmx.GetLevelEntrance(sel).c0_xpos = s.c0_xpos;
			nmmx.GetLevelEntrance(sel).c0_ypos = s.c0_ypos;
			nmmx.GetLevelEntrance(sel).c1_xpos = s.c1_xpos;
			nmmx.GetLevelEntrance(sel).c1_ypos = s.c1_ypos;

			nmmx.GetLevelEntrance(sel).lockState = s.lockState;
			nmmx.GetLevelEntrance(sel).lockLeft = s.A0;
			nmmx.GetLevelEntrance(sel).lockRight = s.A2;
			nmmx.GetLevelEntrance(sel).lockTop = s.A4;
			nmmx.GetLevelEntrance(sel).lockBottom = s.A6;
			nmmx.GetLevelEntrance(sel).lockA8 = s.A8;
			nmmx.GetLevelEntrance(sel).lockAA = s.AA;
			nmmx.GetLevelEntrance(sel).coll = nmmx.GetLevelEntrance(0).coll;
			UpdateProperty(hWnd);
			found = true;
		}

		if (found)
			RepaintAll();
		break;
	}
	case WM_CLOSE: {
		InitOk = false;
		EndDialog(hWnd, 0);
		RepaintAll();
		break;
	}
	}
	return 0;
}
