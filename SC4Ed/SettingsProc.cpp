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

#define IsCheck(i) SendMessage(GetDlgItem(hWnd, i), BM_GETCHECK, NULL, NULL) == BST_CHECKED

BOOL CALLBACK SettingsProc(HWND hWnd, UINT message, WPARAM wParam, LPARAM lParam)
{
	switch (message)
	{
	case WM_INITDIALOG:
		SetWindowText(GetDlgItem(hWnd, IDC_EMUPATH), set.emupath);
		SendMessage(GetDlgItem(hWnd, IDC_LOADROMSTARTUP), BM_SETCHECK, set.romAtStartup, NULL);
		SetDlgItemInt(hWnd, IDC_DROPX, set.dropXOffset, TRUE);
		SetDlgItemInt(hWnd, IDC_DROPY, set.dropYOffset, TRUE);
		break;
	case WM_COMMAND: {
		switch (LOWORD(wParam))
		{
		case IDC_SETTINGS_BROWSE:
			if (OpenFileDialog(hWnd, "SNES Emulator\0*.exe\0", set.emupath)) {
				SetWindowText(GetDlgItem(hWnd, IDC_EMUPATH), set.emupath);
			}
			break;
		case IDC_LOADROMSTARTUP:
			set.romAtStartup = IsCheck(IDC_LOADROMSTARTUP);
			break;
		}

		if (LOWORD(wParam) == IDC_DROPX) {
			TCHAR text[13];
			GetWindowText(GetDlgItem(hWnd, IDC_DROPX), text, 12);
			if (text != "")
				set.dropXOffset = atoi(text);
			RepaintAll();
		}
		else if (LOWORD(wParam) == IDC_DROPY) {
			TCHAR text[13];
			GetWindowText(GetDlgItem(hWnd, IDC_DROPY), text, 12);
			if (text != "")
				set.dropYOffset = atoi(text);
			RepaintAll();
		}

		break;
	}
	case WM_CLOSE:
		EndDialog(hWnd, 0);
		break;
	}
	return 0;
}
