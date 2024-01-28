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

#include <algorithm>

#define IsCheck(i) SendMessage(GetDlgItem(hWnd, i), BM_GETCHECK, NULL, NULL) == BST_CHECKED

#define IDC_EVENT_BLOCK 0x9100
#define IDC_EVENT_NUM 0x9101
#define IDC_EVENT_TYPE 0x9102
#define IDC_EVENT_MATCH 0x9103
#define IDC_EVENT_XPOS 0x9104
#define IDC_EVENT_YPOS 0x9105
#define IDC_EVENT_EVENTID 0x9106
#define IDC_EVENT_EVENTSUBID 0x9107
#define IDC_EVENT_MATCHY 0x9108
#define IDC_EVENT_UNKNOWN 0x9109

#define IDC_EVENT_LEFT 0x9108
#define IDC_EVENT_RIGHT 0x9109
#define IDC_EVENT_TOP 0x910A
#define IDC_EVENT_BOTTOM 0x910B
#define IDC_EVENT_LOCKNUM 0x910C
#define IDC_EVENT_DIRECTION 0x910D
#define IDC_EVENT_POSITION 0x910E
#define IDC_EVENT_LOCKOFFSET 0x910F

#define IDC_EVENT_EVENTSORT 0x9110

#define IDC_EVENT_GFXNUM 0x9120
#define IDC_EVENT_GFXID 0x9121
#define IDC_EVENT_GFXVRAMOFFSET 0x9122
#define IDC_EVENT_GFXPALETTE 0x9123
#define IDC_EVENT_GFXUNKNOWN 0x9124

//#define IDC_EVENT_NAME 0x9109


//static SpinBox eventsBlock;
static SpinBox eventsNum;
static SpinBox eventsType;
static SpinBox eventsMatch;
static SpinBox eventsXpos;
static SpinBox eventsYpos;
static SpinBox eventsEventId;
static SpinBox eventsEventSubId;
static SpinBox eventsUnknown;

//static SpinBox eventsLeft;
//static SpinBox eventsRight;
//static SpinBox eventsTop;
//static SpinBox eventsBottom;
//static SpinBox eventsLockNum;
//static SpinBox eventsLockOffset;
//static SpinBox eventsDirection;
//static SpinBox eventsPosition;

//static SpinBox eventsMove;

//static SpinBox eventsGfxNum;
//static SpinBox eventsGfxId;
//static SpinBox eventsGfxVramOffset;
//static SpinBox eventsGfxPalette;
//static SpinBox eventsGfxUnknown;

//static LPBYTE lockBase = NULL;
//static unsigned numLocks = 0;
//static unsigned currentLockOffset = 0;

//static int dEventBump = 0;
//static int dEventBumpOld = -1;

static void PrevEvent(HWND hWnd);
static void NextEvent(HWND hWnd);

static void CenterEvent() {

	bool validEvent = !nmmx.eventTable.empty();

	if (validEvent) {
		unsigned currentEvent = eventsNum.GetPos();
		auto iter = nmmx.eventTable.begin();
		std::advance(iter, currentEvent);
		auto &event = *iter;

		// move the main window
		SetWindowScrollPosition(hWID[0], event.xpos, event.ypos);
	}

}

static void UpdateEvent(HWND hWnd) {
	int remainingBytes = nmmx.GetEventRemaining();
	TCHAR text[80];
	sprintf(text, "Remaining Events: %d", remainingBytes);
	SetWindowText(GetDlgItem(hWID[7], IDC_REMAINING_EVENTS), text);

	unsigned currentEvent = eventsNum.GetPos();
	auto iter = nmmx.eventTable.begin();
	std::advance(iter, currentEvent);
	auto &event = *iter;

	//	EnableWindow(GetDlgItem(hWnd, IDC_EVENT_MATCH), true);
	EnableWindow(GetDlgItem(hWnd, IDC_EVENT_TYPE), true);
	EnableWindow(GetDlgItem(hWnd, IDC_EVENT_XPOS), true);
	EnableWindow(GetDlgItem(hWnd, IDC_EVENT_YPOS), true);
	EnableWindow(GetDlgItem(hWnd, IDC_EVENT_EVENTID), true);

	EnableWindow(GetDlgItem(hWnd, IDC_EVENT_TYPE), nmmx.type != 1);

	EnableWindow(GetDlgItem(hWnd, IDC_EVENT_SORT), nmmx.type != 1);
	EnableWindow(GetDlgItem(hWnd, IDC_EVENT_ADD), nmmx.type != 1);
	EnableWindow(GetDlgItem(hWnd, IDC_EVENT_DELETE), nmmx.type != 1);

	EnableWindow(GetDlgItem(hWnd, IDC_EVENT_SLOT), false);//nmmx.type == 2);

	//EnableWindow(GetDlgItem(hWnd, IDC_EVENT_NAME), true);


	eventsMatch.SetPos(event.match);
	eventsType.SetPos(event.type);
	eventsXpos.SetPos(event.xpos);
	eventsYpos.SetPos(event.ypos);
	eventsEventId.SetPos(event.eventId);
	eventsEventSubId.SetPos(event.eventSubId);
	eventsUnknown.SetPos(event.unknown);

	bool r = false;

	auto &e = event;
	if (nmmx.type == 2 && e.type == 3 && e.eventId == 8) {
		WORD addr = *LPWORD(nmmx.rom + SNESCore::snes2pc(0x81AF82 + e.eventSubId));
		addr = SReadWord(nmmx, 0x810000 | (addr + 2));
		if (addr) {
			addr = *LPWORD(nmmx.rom + SNESCore::snes2pc(0x810000 | (addr + 4)));

			nmmx.tileDec.clear();
			while (SReadWord(nmmx, 0x810000 | (addr + 0))) {
				DWORD tileDecSrc = SReadDWord(nmmx, 0x810000 | (addr + 6)) & 0xFFFFFF;
				DWORD tileDecDst = SReadWord(nmmx, 0x810000 | (addr + 4));
				DWORD tileDecSize = SReadWord(nmmx, 0x810000 | (addr + 9));

				nmmx.tileDec.push_back({ tileDecSrc, tileDecDst, tileDecSize });
				addr += 0xB;
			}
			r = true;
		}
	}
	else if (!nmmx.tileDec.empty()) {
		nmmx.tileDec.clear();
		r = true;
	}

	if (nmmx.type == 2 && e.type == 3 && e.eventId == 2) {
		nmmx.dynDecTable.clear();

		nmmx.dynDecTable.push_back(0x810000 | SReadWord(nmmx, 0x81AEB8 + e.eventSubId));
		r = true;
	}
	else if (!nmmx.dynDecTable.empty()) {
		nmmx.dynDecTable.clear();
		r = true;
	}

	if (nmmx.type == 2 && e.type == 3 && e.eventId == 3) {
		nmmx.dynPalTable.clear();

		nmmx.dynPalTable.push_back(0x810000 | SReadWord(nmmx, 0x81AEEE + e.eventSubId));
		r = true;
	}
	else if (!nmmx.dynPalTable.empty()) {
		nmmx.dynPalTable.clear();
		r = true;
	}

	if (r)
		RefreshLevel();
}

static void UpdateBlock(HWND hWnd) {
	if (!nmmx.eventTable.empty()) {
		// enable the event number and setup the default value
		EnableWindow(GetDlgItem(hWnd, IDC_EVENT_NUM), true);

	    eventsNum.SetRange(0, nmmx.eventTable.size() - 1);
	//	//SendMessage(GetDlgItem(hWnd, IDC_EVENT_NUM), UDM_SETRANGE, 0, eventList.size() - 1);
	}
	else {
		EnableWindow(GetDlgItem(hWnd, IDC_EVENT_NUM), false);
	}

	UpdateEvent(hWnd);
}

static void WriteState(HWND hWnd, int window = -1) {
	bool validEvent = !nmmx.eventTable.empty();

	if (validEvent) {
		unsigned currentEvent = eventsNum.GetPos();
		auto iter = nmmx.eventTable.begin();
		std::advance(iter, currentEvent);
		auto &event = *iter;

		if (window == -1 || window == IDC_EVENT_MATCH) {
			event.match = eventsMatch.GetPos();
		}
		if (window == -1 || window == IDC_EVENT_TYPE) {
			event.type = eventsType.GetPos();
		}
		if (window == -1 || window == IDC_EVENT_XPOS) {
			event.xpos = eventsXpos.GetPos();
		}
		if (window == -1 || window == IDC_EVENT_YPOS) {
			event.ypos = eventsYpos.GetPos();
		}
		if (window == -1 || window == IDC_EVENT_EVENTID) {
			event.eventId = eventsEventId.GetPos();
		}
		if (window == -1 || window == IDC_EVENT_EVENTSUBID) {
			event.eventSubId = eventsEventSubId.GetPos();
		}
		if (window == -1 || window == IDC_EVENT_UNKNOWN) {
			event.unknown = eventsUnknown.GetPos();
		}

	}
}

static void AddEvent(HWND hWnd) {
	// copy selected event
	unsigned currentEvent = eventsNum.GetPos();
	auto iter = nmmx.eventTable.begin();
	std::advance(iter, currentEvent);

	EventInfo event = *iter;
	event.xpos += 16;
	if (nmmx.AddEvent(eventsNum.GetPos() + 1, event)) {
		eventsNum.SetRange(0, nmmx.eventTable.size() - 1);
		eventsNum.SetPos(eventsNum.GetPos() + 1);
		UpdateBlock(hWnd);
	}
}

static void DeleteEvent(HWND hWnd) {
	if (nmmx.DelEvent(eventsNum.GetPos())) {
		if (eventsNum.GetPos() >= nmmx.eventTable.size()) eventsNum.SetPos(eventsNum.GetPos() - 1);
		eventsNum.SetRange(0, nmmx.eventTable.size() - 1);
		UpdateBlock(hWnd);
	}
}

static void PrevEvent(HWND hWnd) {
	//unsigned currentBlock = eventsBlock.GetPos();
	unsigned currentEvent = eventsNum.GetPos();

	if (currentEvent > 0) {
		eventsNum.SetPos(currentEvent - 1);
	}

	UpdateEvent(hWnd);
	CenterEvent();
}
static void NextEvent(HWND hWnd) {
	//unsigned currentBlock = eventsBlock.GetPos();

	unsigned currentEvent = eventsNum.GetPos();

	if (currentEvent < nmmx.eventTable.size() - 1) {
		eventsNum.SetPos(currentEvent + 1);
	}

	UpdateEvent(hWnd);
	CenterEvent();
}

static void SortEvent(HWND hWnd) {

	nmmx.SortEvents();
	UpdateEvent(hWnd);
	CenterEvent();

}

static void SlotEvent(HWND hWnd) {

	nmmx.SlotEvents();
	UpdateEvent(hWnd);
	CenterEvent();

}

BOOL CALLBACK EventProc(HWND hWnd, UINT message, WPARAM wParam, LPARAM lParam)
{
	switch (message)
	{
		PAINTSTRUCT ps;
		HDC hDC;

	case UPDATEWINDOW: {
		eventsNum.SetPos(0);
		UpdateBlock(hWnd);
		break;
	}
	case WM_PAINT: {
		hDC = BeginPaint(hWnd, &ps);

		//auto spriteDC = CreateCompatibleDC(hDC);
		//auto backBuffer = CreateCompatibleBitmap(hDC, 160, 160);
		//SelectObject(spriteDC, backBuffer);

		////if (event && event->type == 0x2 && (event->eventId == 0x15 || event->eventId == 0x18)) {
		////	unsigned gfxNum = eventsGfxId.GetPos();
		////	unsigned palNum = eventsGfxPalette.GetPos();
		////	int assemblyNum = (!IsCheck(IDC_EVENT_GFXNOASSEMBLY) && nmmx.graphicsToAssembly.count(gfxNum)) ? nmmx.graphicsToAssembly[gfxNum] : -1;

		////	render.RenderObject(spriteDC, 80, 80, gfxNum, palNum, assemblyNum, IsCheck(IDC_EVENT_TILEBUMPNEG) ? -16*dEventBump : 16*dEventBump);
		////}

		//BitBlt(hDC, 320, 240, 160, 160, spriteDC, 0, 0, SRCCOPY);
		//DeleteObject(backBuffer);
		//DeleteDC(spriteDC);

		EndPaint(hWnd, &ps);
		break;
	}
	case WM_INITDIALOG: {
		hWID[7] = hWnd;

		EnableWindow(GetDlgItem(hWnd, IDC_EVENT_NUM), false);

		eventsNum.Create(hWnd, IDC_EVENT_NUM, 90, 52, 64, 0, 0);
		eventsMatch.Create(hWnd, IDC_EVENT_MATCH, 90, 109, 64, 0, nmmx.type == 0 ? 255 : (1 << 16) - 1);
		eventsType.Create(hWnd, IDC_EVENT_TYPE, 90, 139, 64, 0, 3);
		eventsXpos.Create(hWnd, IDC_EVENT_XPOS, 90, 169, 64, 0, nmmx.type == 0 ? (1 << 13) - 1 : (1 << 16) - 1);
		eventsYpos.Create(hWnd, IDC_EVENT_YPOS, 90, 199, 64, 0, nmmx.type == 0 ? (1 << 16) - 1 : (1 << 16) - 1);
		eventsEventId.Create(hWnd, IDC_EVENT_EVENTID, 90, 229, 64, 0, 255);
		eventsEventSubId.Create(hWnd, IDC_EVENT_EVENTSUBID, 90, 259, 64, 0, nmmx.type == 0 ? 255 : (1 << 16) - 1);
		eventsUnknown.Create(hWnd, IDC_EVENT_UNKNOWN, 90, 289, 64, 0, nmmx.type == 0 ? 3 : (1 << 16) - 1);

		UpdateBlock(hWnd);

		SendMessage(GetDlgItem(hWnd, IDC_EVENT_TILEBUMP), UDM_SETRANGE, 0, 1000);

		//SetWindowText(GetDlgItem(hWnd, IDC_EMUPATH), set.emupath);
		//SendMessage(GetDlgItem(hWnd, IDC_LOADROMSTARTUP), BM_SETCHECK, set.romAtStartup, NULL);
		break;
	}
	case WM_COMMAND: {
		//bool validEvent = !nmmx.eventTable[currentBlock].empty();

		if (eventsNum.IsIDEqual((long)lParam)) {
			eventsNum.Work(wParam);
			UpdateEvent(hWnd);
			RepaintAll();
		}
		else if (eventsMatch.IsIDEqual((long)lParam)) {
			eventsMatch.Work(wParam);
			WriteState(hWnd, IDC_EVENT_MATCH);
			RepaintAll();
			//RefreshLevel();
		}
		else if (eventsType.IsIDEqual((long)lParam)) {
			eventsType.Work(wParam);
			WriteState(hWnd, IDC_EVENT_TYPE);
			RepaintAll();
			//RefreshLevel();
		}
		else if (eventsXpos.IsIDEqual((long)lParam)) {
			eventsXpos.Work(wParam);
			WriteState(hWnd, IDC_EVENT_XPOS);
			RepaintAll();
		}
		else if (eventsYpos.IsIDEqual((long)lParam)) {
			eventsYpos.Work(wParam);
			WriteState(hWnd, IDC_EVENT_YPOS);
			RepaintAll();
		}
		else if (eventsEventId.IsIDEqual((long)lParam)) {
			eventsEventId.Work(wParam);
			WriteState(hWnd, IDC_EVENT_EVENTID);
			RepaintAll();
			//RefreshLevel();
		}
		else if (eventsEventSubId.IsIDEqual((long)lParam)) {
			eventsEventSubId.Work(wParam);
			WriteState(hWnd, IDC_EVENT_EVENTSUBID);
			RepaintAll();
			//RefreshLevel();
		}
		else if (eventsUnknown.IsIDEqual((long)lParam)) {
			eventsUnknown.Work(wParam);
			WriteState(hWnd, IDC_EVENT_UNKNOWN);
			RepaintAll();
			//RefreshLevel();
		}

		switch (LOWORD(wParam)) {
		case IDC_EVENT_ADD:
			AddEvent(hWnd);
			RepaintAll();
			break;
		case IDC_EVENT_DELETE:
			DeleteEvent(hWnd);
			RepaintAll();
			break;
		case IDC_EVENT_NEXT:
			NextEvent(hWnd);
			RepaintAll();
			break;
		case IDC_EVENT_PREV:
			PrevEvent(hWnd);
			RepaintAll();
			break;
		case IDC_EVENT_SORT:
			SortEvent(hWnd);
			RepaintAll();
			break;
		case IDC_EVENT_SLOT:
			SlotEvent(hWnd);
			RepaintAll();
			break;
	//	case IDC_EVENT_NAME:
	//		NameEvent(hWID);
	//		RepaintAll();
	//		break; 
		default:
			break;
		}

		break;
	}
	case WM_VSCROLL: {
		if (eventsNum.IsIDEqual((long)lParam)) {
			eventsNum.Work(wParam);
		}
		else if (eventsType.IsIDEqual((long)lParam)) {
			eventsType.Work(wParam);
		}
		else if (eventsMatch.IsIDEqual((long)lParam)) {
			eventsMatch.Work(wParam);
		}
		else if (eventsXpos.IsIDEqual((long)lParam)) {
			eventsXpos.Work(wParam);
		}
		else if (eventsYpos.IsIDEqual((long)lParam)) {
			eventsYpos.Work(wParam);
		}
		else if (eventsEventId.IsIDEqual((long)lParam)) {
			eventsEventId.Work(wParam);
		}
		else if (eventsEventSubId.IsIDEqual((long)lParam)) {
			eventsEventSubId.Work(wParam);
		}
		else if (eventsUnknown.IsIDEqual((long)lParam)) {
			eventsUnknown.Work(wParam);
		}

		break;
	}
	case WM_CLOSE:
		EndDialog(hWnd, 0);
		RepaintAll();
		break;
	}
	return 0;
}
