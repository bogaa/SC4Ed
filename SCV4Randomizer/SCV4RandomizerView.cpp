
// SCV4RandomizerView.cpp : implementation of the CSCV4RandomizerView class
//

#include "stdafx.h"
// SHARED_HANDLERS can be defined in an ATL project implementing preview, thumbnail
// and search filter handlers and allows sharing of document code with that project.
#ifndef SHARED_HANDLERS
#include "SCV4Randomizer.h"
#endif

#include "SCV4RandomizerDoc.h"
#include "SCV4RandomizerView.h"

#include <vector>
#include "MainFrm.h"

#include <iostream>
#include <sstream>
#include <algorithm>
#include <random>
#include <deque>
#include <iomanip>
#include <iterator>
#include <map>
#include <array>

#ifdef _DEBUG
#define new DEBUG_NEW
#endif

// CUSTOM

#define KEYS_DEF \
	KEY_DEF(IDC_COMMON_RANDOMBEHAVIOR),\
	KEY_DEF(IDC_SIMON_HEALTH),\
	KEY_DEF(IDC_SIMON_SPEED),\
	KEY_DEF(IDC_SIMON_ACCELERATION),\
	KEY_DEF(IDC_SIMON_WHIPDAMAGE),\
	KEY_DEF(IDC_SIMON_WHIPLENGTH),\
	KEY_DEF(IDC_SIMON_ITEMDAMAGE),\
	KEY_DEF(IDC_LEVEL_ORDER),\
	KEY_DEF(IDC_LEVEL_REVERSE),\
	KEY_DEF(IDC_LEVEL_CANDLE),\
	KEY_DEF(IDC_LEVEL_CRUSHERS),\
	KEY_DEF(IDC_LEVEL_ITEMGFX),\
	KEY_DEF(IDC_ENEMY_HEALTH),\
	KEY_DEF(IDC_ENEMY_DAMAGE),\
	KEY_DEF(IDC_ENEMY_SPEED),\
	KEY_DEF(IDC_ENEMY_BEHAVIOR),\
	KEY_DEF(IDC_ENEMY_ID),\
	KEY_DEF(IDC_BOSS_HEALTH),\
	KEY_DEF(IDC_BOSS_DAMAGE),\
	KEY_DEF(IDC_BOSS_SPEED),\
	KEY_DEF(IDC_BOSS_BEHAVIOR),

//KEY_DEF(IDC_LEVEL_TIMER), \

// Checkboxes
#define KEY_DEF(id) { id, #id }
static std::vector<std::tuple<unsigned, std::string>> checkBoxes = {
	KEYS_DEF
};
#undef KEY_DEF

static std::vector<std::string> difficultyNames = {
	"Easy",
	"Medium",
	"Hard",
	"Furious",
};

static std::vector<std::string> typeNames = {
	"Dynamic",
	"Static",
};

// Functions
static double GetRandom(double start, double limit) {
	return (((double)std::rand() / (RAND_MAX + 1.0) * (limit - start)) + start);
}

static int GetRandomRange(int start, int end) {
	return std::floor(((double)std::rand() / (RAND_MAX + 1.0) * (((double)(end + 1)) - (double)start)) + (double)start);
}

template<typename T>
T GetRandomValue(std::vector<T> v) {
	std::random_shuffle(v.begin(), v.end());
	return v[0];
}

static bool GetVersion(std::string &version) {
	char szModPath[MAX_PATH];
	szModPath[0] = '\0';
	GetModuleFileName(NULL, szModPath, sizeof(szModPath));
	DWORD h;
	DWORD size = GetFileVersionInfoSize(szModPath, &h);
	if (size > 0) {
		BYTE* pbBuf = static_cast<BYTE*>(alloca(size));
		if (GetFileVersionInfo(szModPath, h, size, pbBuf))
		{
			UINT uiSize;
			BYTE* lpb;
			if (VerQueryValue(pbBuf,
				"\\VarFileInfo\\Translation",
				(void**)&lpb,
				&uiSize))
			{
				WORD* lpw = (WORD*)lpb;
				CString strQuery;
				strQuery.Format("\\StringFileInfo\\%04x%04x\\ProductVersion", lpw[0], lpw[1]);
				if (VerQueryValue(pbBuf,
					const_cast<LPSTR>((LPCSTR)strQuery),
					(void**)&lpb,
					&uiSize) && uiSize > 0)
				{
					version = (LPCSTR)lpb;
					return true;
				}
			}
		}

	}

	return false;
}

// CSCV4RandomizerView

IMPLEMENT_DYNCREATE(CSCV4RandomizerView, CFormView)

BEGIN_MESSAGE_MAP(CSCV4RandomizerView, CFormView)
	ON_BN_CLICKED(IDC_RANDOMIZE, &CSCV4RandomizerView::OnBnClickedRandomize)
	ON_NOTIFY(NM_CUSTOMDRAW, IDC_COMMON_DIFFICULTY, &CSCV4RandomizerView::OnNMCustomdrawCommonDifficulty)
	ON_EN_CHANGE(IDC_COMMON_SEED, &CSCV4RandomizerView::OnEnChangeCommonSeed)
	ON_WM_DESTROY()
END_MESSAGE_MAP()

// CSCV4RandomizerView construction/destruction

CSCV4RandomizerView::CSCV4RandomizerView()
	: CFormView(IDD_SCV4RANDOMIZER_FORM)
{
	// TODO: add construction code here

}

CSCV4RandomizerView::~CSCV4RandomizerView()
{
}

void CSCV4RandomizerView::DoDataExchange(CDataExchange* pDX)
{
	CFormView::DoDataExchange(pDX);
}

BOOL CSCV4RandomizerView::PreCreateWindow(CREATESTRUCT& cs)
{
	// TODO: Modify the Window class or styles here by modifying
	//  the CREATESTRUCT cs

	return CFormView::PreCreateWindow(cs);
}

BOOL CSCV4RandomizerView::PreTranslateMessage(MSG *pMsg) {
	ToolTip.RelayEvent(pMsg);

	return CFormView::PreTranslateMessage(pMsg);
}

void CSCV4RandomizerView::OnInitialUpdate()
{
	CFormView::OnInitialUpdate();
	GetParentFrame()->RecalcLayout();
	ResizeParentToFit();

	// CUSTOM

	// init check boxes
	for (auto &i : checkBoxes) {
		CButton *b = (CButton*)GetDlgItem(std::get<unsigned>(i));
		ASSERT(b);
		bool defZero = std::get<unsigned>(i) == IDC_SIMON_SPEED || std::get<unsigned>(i) == IDC_SIMON_ACCELERATION;
		b->SetCheck(theApp.GetProfileInt(_T("Settings"), std::get<std::string>(i).c_str(), defZero ? 0 : 1));
	}

	// init combo box
	CComboBox *c = (CComboBox*)GetDlgItem(IDC_COMMON_RANDOMTYPE);
	ASSERT(c);
	for (auto &s : typeNames) {
		c->AddString(s.c_str());
	}
	c->SetCurSel(theApp.GetProfileInt(_T("Settings"), _T("IDC_COMMON_RANDTYPE"), 0));

	// init difficulty
	CSliderCtrl *difficulty = (CSliderCtrl *)GetDlgItem(IDC_COMMON_DIFFICULTY);
	difficulty->SetRangeMin(0);
	difficulty->SetRangeMax(difficultyNames.size() - 1);
	difficulty->SetPos(theApp.GetProfileInt(_T("Settings"), _T("IDC_COMMON_DIFFICULTY"), 1));

	// init version
	std::string v;
	auto ok = GetVersion(v);
	v = "Version: " + v;
	SetDlgItemText(IDC_VERSION, ok ? v.c_str() : "???");

	// init tooltip
	if (!ToolTip.Create(this)) {
		TRACE0("Unable to create the ToolTip!");
	}
	else {
		std::vector<std::tuple<unsigned, unsigned>> toolTips = {
			{ IDC_COMMON_DIFFICULTY, IDS_COMMON_DIFFICULTY },
			{ IDC_COMMON_RANDOMBEHAVIOR, IDS_COMMON_RANDOMBEHAVIOR },
			{ IDC_COMMON_RANDOMTYPE, IDS_COMMON_RANDOMTYPE },
			{ IDC_COMMON_SEED, IDS_COMMON_SEED },
			{ IDC_SIMON_HEALTH, IDS_SIMON_HEALTH },
			{ IDC_SIMON_ACCELERATION, IDS_SIMON_ACCELERATION },
			{ IDC_SIMON_SPEED, IDS_SIMON_SPEED },
			{ IDC_SIMON_WHIPDAMAGE, IDS_SIMON_WHIPDAMAGE },
			{ IDC_SIMON_WHIPLENGTH, IDS_SIMON_WHIPLENGTH },
			{ IDC_SIMON_ITEMDAMAGE, IDS_SIMON_ITEMDAMAGE },
			{ IDC_LEVEL_ORDER, IDS_LEVEL_ORDER },
			{ IDC_LEVEL_REVERSE, IDS_LEVEL_REVERSE },
			//{ IDC_LEVEL_TIMER, IDS_LEVEL_TIMER },
			{ IDC_LEVEL_CANDLE, IDS_LEVEL_CANDLE },
			{ IDC_LEVEL_CRUSHERS, IDS_LEVEL_CRUSHERS },
			{ IDC_LEVEL_ITEMGFX, IDS_LEVEL_ITEMGFX },
			{ IDC_ENEMY_HEALTH, IDS_ENEMY_HEALTH },
			{ IDC_ENEMY_DAMAGE, IDS_ENEMY_DAMAGE },
			{ IDC_ENEMY_SPEED, IDS_ENEMY_SPEED },
			{ IDC_ENEMY_BEHAVIOR, IDS_ENEMY_BEHAVIOR },
			{ IDC_ENEMY_ID, IDS_ENEMY_ID },
			{ IDC_BOSS_HEALTH, IDS_BOSS_HEALTH },
			{ IDC_BOSS_DAMAGE, IDS_BOSS_DAMAGE },
			{ IDC_BOSS_SPEED, IDS_BOSS_SPEED },
			{ IDC_BOSS_BEHAVIOR, IDS_BOSS_BEHAVIOR },
			{ IDC_CODE, IDS_CODE },
		};

		for (auto &t : toolTips) {
			ToolTip.AddTool(GetDlgItem(std::get<0>(t)), std::get<1>(t));
		}

		ToolTip.Activate(TRUE);
	}


	mCodeEdit.SubclassDlgItem(IDC_CODE, this);
}

// CSCV4RandomizerView diagnostics

#ifdef _DEBUG
void CSCV4RandomizerView::AssertValid() const
{
	CFormView::AssertValid();
}

void CSCV4RandomizerView::Dump(CDumpContext& dc) const
{
	CFormView::Dump(dc);
}

CSCV4RandomizerDoc* CSCV4RandomizerView::GetDocument() const // non-debug version is inline
{
	ASSERT(m_pDocument->IsKindOf(RUNTIME_CLASS(CSCV4RandomizerDoc)));
	return (CSCV4RandomizerDoc*)m_pDocument;
}
#endif //_DEBUG

void CSCV4RandomizerView::AddCode(std::string code) {
	bool valid = true;
	std::vector<std::string> tokens;
	std::stringstream ss(code);
	std::string s;
	while (std::getline(ss, s, '-')) {
		tokens.push_back(s);
	}

	// perform checks
	// count
	valid &= tokens.size() == 5;
	// version
	std::string version;
	valid &= GetVersion(version);
	valid &= version == tokens[0];
	// difficulty
	valid &= (std::find(difficultyNames.begin(), difficultyNames.end(), tokens[2]) - difficultyNames.begin()) < difficultyNames.size();
	// random type
	valid &= (std::find(typeNames.begin(), typeNames.end(), tokens[3]) - typeNames.begin()) < typeNames.size();
	// checkboxes
	valid &= (std::count(tokens[4].begin(), tokens[4].end(), '0') + std::count(tokens[4].begin(), tokens[4].end(), '1')) == checkBoxes.size();

	if (valid) {
		// set seed
		SetDlgItemText(IDC_COMMON_SEED, tokens[1].c_str());

		DWORD x = (std::find(difficultyNames.begin(), difficultyNames.end(), tokens[2]) - difficultyNames.begin());
		CSliderCtrl *difficulty = (CSliderCtrl *)GetDlgItem(IDC_COMMON_DIFFICULTY);
		difficulty->SetPos(x);

		x = (std::find(typeNames.begin(), typeNames.end(), tokens[3]) - typeNames.begin());
		CComboBox *c = (CComboBox*)GetDlgItem(IDC_COMMON_RANDOMTYPE);
		c->SetCurSel(x);

		// set checkboxes
		auto it = checkBoxes.begin();
		for (auto &c : tokens[4]) {
			ss.str("");
			ss.clear();
			ss << c;
			ss >> x;
			CButton *b = (CButton*)GetDlgItem(std::get<unsigned>(*it));
			it++;
			b->SetCheck(x);
		}

		// set code
		SetDlgItemText(IDC_CODE, code.c_str());

		// randomize
		OnBnClickedRandomize();
	}

	AfxMessageBox(valid ? (LPCTSTR)"Code OK" : (LPCTSTR)"Code FAILED.  Make sure version number matches first string");
}


// CSCV4RandomizerView message handlers
#include <sstream>

void CSCV4RandomizerView::OnBnClickedRandomize()
{
	CMainFrame *pMainWnd = (CMainFrame *)AfxGetMainWnd();
	// Set status busy
	pMainWnd->SetMessageText((UINT)AFX_IDS_BUSYMESSAGE);

	bool error = false;

	// open ROM
	nmmx.FreeRom();
	if (!nmmx.LoadNewRom(theApp.FileName.c_str()))
	{
		nmmx.FreeRom();
		return;
	}

	if (0) {
		std::stringstream ss;
		ss.str("");

		for (unsigned i = 0; i < nmmx.numLevels; ++i) {
			nmmx.level = i;
			nmmx.LoadLevel();

			ss << levelsName[nmmx.type][i];

			auto it = nmmx.eventTable.begin();
			while (it != nmmx.eventTable.end()) {
				auto &event = *it;

				if (event.type == 1) {
					switch (event.eventId) {
					//case 0x1A: ss << " D";  break;
					//case 0x1B: ss << " A";  break;
					//case 0x1C: ss << " H";  break;
					//case 0x1D: ss << " C";  break;
					//case 0x1E: ss << " S";  break;

					case 0x1A:
					case 0x1B:
					case 0x1C:
					case 0x1D:
					case 0x1E:
						ss << ' ' << std::hex << event.match << std::dec;

					default: break;
					}
				}

				it++;
			}
			ss << '\n';

		}

		AfxMessageBox(ss.str().c_str());
	}

	if (!nmmx.CheckROM() || (!nmmx.expandedROM && !nmmx.ExpandROM()))
	{
		//MessageBox(hWnd, "Wrong ROM. Please open MMX1, X2 or X3 ROM.", "Error", MB_ICONERROR);
		AfxMessageBox((LPCTSTR)"Error opening or expanding ROM");
		nmmx.FreeRom();
		return;
	}

	nmmx.LoadLevel(0);

	// common variable
	DWORD a = 0;
	// seed random
	CString t;
	unsigned seed = 0;
	std::stringstream ss;
	GetDlgItemText(IDC_COMMON_SEED, t);
	ss << std::hex << t;
	if (std::string(t) == "" || (ss.str().length() > 8 || !(ss >> std::hex >> seed) || !ss.eof())) seed = time(NULL);
	std::srand(seed);

	// check random type
	CComboBox *c = (CComboBox*)GetDlgItem(IDC_COMMON_RANDOMTYPE);
	bool dynamic = typeNames[c->GetCurSel()] == "Dynamic";

	CButton *b = nullptr;

	b = (CButton*)GetDlgItem(IDC_COMMON_RANDOMBEHAVIOR);
	if (b && b->GetCheck()) {
		// try a seed that supports a full 16b cycle
		//*LPWORD(nmmx.rom + SNESCore::snes2pc(0x8081AC)) = 0x002D;
	}

	b = (CButton*)GetDlgItem(IDC_SIMON_HEALTH);
	if (b && b->GetCheck()) {
		CSliderCtrl *difficulty = (CSliderCtrl *)GetDlgItem(IDC_COMMON_DIFFICULTY);
		std::string d = difficultyNames[difficulty->GetPos()];
		unsigned min = d == "Easy" ? 16 : d == "Medium" ? 14 : d == "Hard" ? 12 : 6;
		unsigned max = d == "Easy" ? 20 : d == "Medium" ? 17 : d == "Hard" ? 15 : 10;

		std::vector<DWORD> sHealth = { 0x8094E0, 0x809555 };
		for (auto &a : sHealth) {
			WORD v = *LPWORD(nmmx.rom + SNESCore::snes2pc(a));
			v = GetRandomRange(min, max);

			*LPWORD(nmmx.rom + SNESCore::snes2pc(a)) = v;
		}
	}

	b = (CButton*)GetDlgItem(IDC_SIMON_SPEED);
	if (b && b->GetCheck()) {
		// Get a scaling factor
		unsigned v = GetRandomRange(0x1, 0xC) << 12;

		unsigned newXSub = 0;
		unsigned newX = 0;

		// randomize walking speed
		newXSub = (0x4000 + v) % 0x8000;
		newX = 0x1 + (0x4000 + v) / 0x8000;
		*LPWORD(nmmx.rom + SNESCore::snes2pc(0x80A65F)) = newXSub; // xpossub
		*LPWORD(nmmx.rom + SNESCore::snes2pc(0x80A665)) = newX; // xpos
		*LPWORD(nmmx.rom + SNESCore::snes2pc(0x80A678)) = 0xFFFF - newXSub; // xnegsub
		*LPWORD(nmmx.rom + SNESCore::snes2pc(0x80A67E)) = 0xFFFF - newX; // xneg

		// randomize crouching speed
		unsigned crouchMult = GetRandomRange(1, 2);
		newXSub = (0x6000 + v*crouchMult) % 0x8000;
		newX = 0x0 + (0x6000 + v*crouchMult) / 0x8000;
		*LPWORD(nmmx.rom + SNESCore::snes2pc(0x80A6FF)) = 0x0 + newXSub; // xpossub
		*LPWORD(nmmx.rom + SNESCore::snes2pc(0x80A705)) = 0x0 + newX; // xpos
		*LPWORD(nmmx.rom + SNESCore::snes2pc(0x80A710)) = 0xFFFF - newXSub; // xnegsub
		*LPWORD(nmmx.rom + SNESCore::snes2pc(0x80A716)) = 0xFFFF - newX; // xneg

		// randomize jumping speed
		newXSub = (0x4000 + v) % 0x8000;
		newX = 0x1 + (0x4000 + v) / 0x8000;
		//*LPWORD(nmmx.rom + SNESCore::snes2pc(0x80A8F0)) = newXSub; // xpossub
		//*LPWORD(nmmx.rom + SNESCore::snes2pc(0x80A905)) = newXSub; // xpossub
		*LPWORD(nmmx.rom + SNESCore::snes2pc(0x80A916)) = newXSub; // xpossub
		*LPWORD(nmmx.rom + SNESCore::snes2pc(0x80A90B)) = newX; // xpos
		*LPWORD(nmmx.rom + SNESCore::snes2pc(0x80A910)) = newX; // xpos
		//*LPWORD(nmmx.rom + SNESCore::snes2pc(0x80A921)) = 0xFFFF - newXSub; // xnegsub
		//*LPWORD(nmmx.rom + SNESCore::snes2pc(0x80A936)) = 0xFFFF - newXSub; // xnegsub
		*LPWORD(nmmx.rom + SNESCore::snes2pc(0x80A947)) = 0xFFFF - newXSub; // xnegsub
		*LPWORD(nmmx.rom + SNESCore::snes2pc(0x80A93C)) = 0xFFFF - newX; // xneg
		*LPWORD(nmmx.rom + SNESCore::snes2pc(0x80A941)) = 0xFFFF - newX; // xneg

		// randomize stair speed
		newXSub = (0xC000 + v) % 0x8000;
		newX = 0x0 + (0xC000 + v) / 0x8000;
		*LPWORD(nmmx.rom + SNESCore::snes2pc(0x80A8C2)) = newXSub; // xpossub
		*LPWORD(nmmx.rom + SNESCore::snes2pc(0x80A8C8)) = newX; // xpos
		*LPWORD(nmmx.rom + SNESCore::snes2pc(0x80A8A0)) = 0xFFFF - newXSub; // xnegsub
		*LPWORD(nmmx.rom + SNESCore::snes2pc(0x80A8A6)) = 0xFFFF - newX; // xneg

		// randomize ring speed
		PSHORT adder;
		adder = PSHORT(nmmx.rom + SNESCore::snes2pc(0x81900F));
		float m = GetRandomValue<float>({0.0, 0.5, 1.0, 2.0});
		for (unsigned i = 0; i < 0x4; i++, adder++) {
			// less speed adder means slower
			if (m != 0.0) {
				*adder *= m;
			}
		}
		adder = PSHORT(nmmx.rom + SNESCore::snes2pc(0x819017));
		for (unsigned i = 0; i < 0x40; i++, adder++) {
			// less speed adder means slower
			*adder *= m;
		}

		// remove the scripted animation on stairs
		*LPWORD(nmmx.rom + SNESCore::snes2pc(0x8095D6)) = 0x0000;
		*LPWORD(nmmx.rom + SNESCore::snes2pc(0x8095DE)) = 0x0000;

		// fill in 4-4 gap
		nmmx.level = 0x17;
		nmmx.LoadLevel();
		nmmx.SetSceneBlock(6, 2, 0, 1, 0x3);
		nmmx.SetSceneBlock(6, 2, 0, 2, 0x7);
	}

	b = (CButton*)GetDlgItem(IDC_SIMON_ACCELERATION);
	if (b && b->GetCheck()) {
		WORD w = GetRandomRange(3, 6) << 12;
		*LPWORD(nmmx.rom + SNESCore::snes2pc(0x80A73E)) = w;

		// fill in 4-4 gap
		nmmx.level = 0x17;
		nmmx.LoadLevel();
		nmmx.SetSceneBlock(6, 2, 0, 1, 0x3);
		nmmx.SetSceneBlock(6, 2, 0, 2, 0x7);
	}

	b = (CButton*)GetDlgItem(IDC_SIMON_WHIPDAMAGE);
	if (b && b->GetCheck()) {
		// 0x81A6EC
		// ROPE, LIMP_ROPE, CHAIN0, LIMP_CHAIN0, CHAIN1, LIMP_CHAIN1
		// BLANK, DAGGER, AXE, VIAL, CROSS, STOPWATCH
		CSliderCtrl *difficulty = (CSliderCtrl *)GetDlgItem(IDC_COMMON_DIFFICULTY);
		std::string d = difficultyNames[difficulty->GetPos()];
		double min = d == "Easy" ? 1.2 : d == "Medium" ? 0.8 : d == "Hard" ? 0.8 : 0.5;
		double max = d == "Easy" ? 1.5 : d == "Medium" ? 1.1 : d == "Hard" ? 1.0 : 0.8;
		for (unsigned i = 0; i < 0x6; i++) {
			a = 0x81A6EC + 2 * i;
			WORD v = *LPWORD(nmmx.rom + SNESCore::snes2pc(a));
			v *= GetRandom(min + (i >= 4 ? 0.3 : 0), max + (i >= 4 ? 0.3 : 0));
			*LPWORD(nmmx.rom + SNESCore::snes2pc(a)) = v;
		}
	}

	b = (CButton*)GetDlgItem(IDC_SIMON_WHIPLENGTH);
	if (b && b->GetCheck()) {
		// 0x819261 - whipping/limp length 2-8
		// 0x8190FB - swinging length XF where X is 2=3,3=5,4=7,5=9,6=B,7=D,8=F
		// 0x80DF28 - LDA #0002 -> #0000
		// 0x80DF00 - Add code for JSR
		CSliderCtrl *difficulty = (CSliderCtrl *)GetDlgItem(IDC_COMMON_DIFFICULTY);
		std::string d = difficultyNames[difficulty->GetPos()];
		int min = d == "Easy" ? 6 : d == "Medium" ? 4 : d == "Hard" ? 3 : 2;
		int max = d == "Easy" ? 8 : d == "Medium" ? 8 : d == "Hard" ? 8 : 8;
		for (unsigned i = 1; i < 0x3; i++) {
			WORD length = GetRandomRange(min, max);

			a = 0x819261 + 2 * i;
			*LPWORD(nmmx.rom + SNESCore::snes2pc(a)) = length;
			a = 0x8190FB + 2 * i;
			*LPWORD(nmmx.rom + SNESCore::snes2pc(a)) = ((2 * length - 1) << 4) | 0x0F;
		}

		// make leather whip the longest... just for fun
		*LPWORD(nmmx.rom + SNESCore::snes2pc(0x819261)) = 0x0008;
		*LPWORD(nmmx.rom + SNESCore::snes2pc(0x8190FB)) = 0x00FF;
		// fix LDA
		*LPWORD(nmmx.rom + SNESCore::snes2pc(0x80DF29)) = 0x0000;

		// move the ring that causes the bug
		nmmx.level = 56;
		nmmx.LoadLevel();
		auto it = nmmx.eventTable.begin();
		if (nmmx.eventTable.size() > 8) {
			std::advance(it, 8);
			if (it != nmmx.eventTable.end()
				&& it->type == 0x2
				&& it->eventId == 0x3
				&& it->eventSubId == 0x0
				&& it->xpos == 0xA0
				) {
				it->xpos = 0x90;
				nmmx.SaveEvents();
			}
		}
	}

	b = (CButton*)GetDlgItem(IDC_SIMON_ITEMDAMAGE);
	if (b && b->GetCheck()) {
		// 0x81A6EC
		// ROPE, LIMP_ROPE, CHAIN0, LIMP_CHAIN0, CHAIN1, LIMP_CHAIN1
		// BLANK, DAGGER, AXE, VIAL, CROSS, STOPWATCH
		CSliderCtrl *difficulty = (CSliderCtrl *)GetDlgItem(IDC_COMMON_DIFFICULTY);
		std::string d = difficultyNames[difficulty->GetPos()];
		double min = d == "Easy" ? 1.2 : d == "Medium" ? 0.9 : d == "Hard" ? 0.8 : 0.6;
		double max = d == "Easy" ? 1.5 : d == "Medium" ? 1.0 : d == "Hard" ? 0.9 : 0.7;
		for (unsigned i = 0x7; i < 0xC; i++) {
			a = 0x81A6EC + 2 * i;
			WORD v = *LPWORD(nmmx.rom + SNESCore::snes2pc(a));
			v *= GetRandom(min, max);
			*LPWORD(nmmx.rom + SNESCore::snes2pc(a)) = v;
		}
	}

	//b = (CButton*)GetDlgItem(IDC_BOSS_HEALTH);
	//if (b && b->GetCheck()) {
	//	std::vector<DWORD> bHealth = { 0x8095BF, 0x82D763 };
	//	for (auto &a : bHealth) {
	//		WORD v = *LPWORD(nmmx.rom + SNESCore::snes2pc(a));
	//		v = GetRandom(12, 14);

	//		*LPWORD(nmmx.rom + SNESCore::snes2pc(a)) = v;
	//	}

	//}

	b = (CButton*)GetDlgItem(IDC_BOSS_BEHAVIOR);
	if (b && b->GetCheck()) {
		// <ROWDAIN>
		// Head movement
		for (unsigned i = 0; i < 0xB; i++) {
			*LPWORD(nmmx.rom + SNESCore::snes2pc(0x81C4D1 + 2 * i)) = GetRandomValue<WORD>({ 0x10, 0x30, 0x50, 0x60, 0x70, 0x80, 0x90, 0xA0, 0xB0, 0xC0, 0xD0, 0xF0 });
		}
		// Fireball frequency
		*LPWORD(nmmx.rom + SNESCore::snes2pc(0x82E0BB)) = GetRandomValue<WORD>({ /*0x7F,*/ 0x3F, 0x1F });
		// Horse left limit
		*LPWORD(nmmx.rom + SNESCore::snes2pc(0x82E1F9)) = GetRandomValue<WORD>({ 0x760, 0x770 });

		// <MEDUSA>
		// Limits
		*LPWORD(nmmx.rom + SNESCore::snes2pc(0x82ED33)) = GetRandomValue<WORD>({ 0xEE0, 0xEF0/*, 0xF00*/ });
		*LPWORD(nmmx.rom + SNESCore::snes2pc(0x82ECF4)) = GetRandomValue<WORD>({ 0xE50, 0xE60/*, 0xE70*/ });
		// Ground snakes
		*LPWORD(nmmx.rom + SNESCore::snes2pc(0x82EE4D)) = GetRandomValue<WORD>({ /*0xFF, */0x7F, 0x3F });
		// Thrown snakes
		*LPWORD(nmmx.rom + SNESCore::snes2pc(0x82EE8A)) = GetRandomValue<WORD>({ 0x3F, 0x1F });
		*LPWORD(nmmx.rom + SNESCore::snes2pc(0x82EE8D)) = 0x08;

		// <VIPER>
		for (unsigned i = 0; i < 0xB; i++) {
			*LPWORD(nmmx.rom + SNESCore::snes2pc(0x81C6BE + 2 * i)) = GetRandomValue<WORD>({ 0x00, 0x10, 0x20, 0x30, 0x40, 0x50, 0x60, 0x70, 0x80 });
			*LPWORD(nmmx.rom + SNESCore::snes2pc(0x81C75C + 2 * i)) = GetRandomValue<WORD>({ 0x00, 0x10, 0x20, 0x30, 0x40, 0x50, 0x60, 0x70, 0x80 });
		}

		// <PUWEXIL>
		// start position
		//*LPWORD(nmmx.rom + SNESCore::snes2pc(0x82F2D1)) = GetRandomRange(0x20, 0x30);
	}

	b = (CButton*)GetDlgItem(IDC_BOSS_SPEED);
	if (b && b->GetCheck()) {
		SHORT v = 0;
		SHORT newX = 0;
		SHORT newXSub = 0;

		// <ROWDAIN>
		// walking speed
		v = GetRandomRange(5, 7) << 12;
		*LPWORD(nmmx.rom + SNESCore::snes2pc(0x82DB33)) = 0x8000 - v;
		*LPWORD(nmmx.rom + SNESCore::snes2pc(0x82DB9A)) = 0x8000 + v;

		// <MEDUSA>
		// walking speed
		v = GetRandomRange(1, 2);
		for (unsigned i = 0; i < 4; i++) {
			newX = 0x1 + v;
			//*LPWORD(nmmx.rom + SNESCore::snes2pc(0x81CA7A + i * 4 + 2)) = newXSub;
			*LPWORD(nmmx.rom + SNESCore::snes2pc(0x81CA7A + i * 4 + 0)) = newX;
		}
		for (unsigned i = 0; i < 4; i++) {
			newX = 0xFFFF - v;
			//*LPWORD(nmmx.rom + SNESCore::snes2pc(0x81CA6A + i * 4 + 2)) = newXSub;
			*LPWORD(nmmx.rom + SNESCore::snes2pc(0x81CA6A + i * 4 + 0)) = newX;
		}

		// <VIPER>
		*LPWORD(nmmx.rom + SNESCore::snes2pc(0x82E55B)) = GetRandomRange(0x80, 0xA0);
		*LPWORD(nmmx.rom + SNESCore::snes2pc(0x82E834)) = GetRandomRange(0xA0, 0xC0);

		// <PUWEXIL>
		// gas
		*LPWORD(nmmx.rom + SNESCore::snes2pc(0x82F4C7)) = 0xFFFC;
		*LPWORD(nmmx.rom + SNESCore::snes2pc(0x82F4D2)) = 0x0004;
		// move up/down
		v = GetRandomRange(4, 5) << 12;
		*LPWORD(nmmx.rom + SNESCore::snes2pc(0x82F38D)) = 0xA000 + v;
		v = GetRandomRange(4, 7) << 12;
		*LPWORD(nmmx.rom + SNESCore::snes2pc(0x82F398)) = v;
		// arc
		v = GetRandomRange(0x8, 0xA) << 8;
		*LPWORD(nmmx.rom + SNESCore::snes2pc(0x82F3DD)) = v;
		*LPWORD(nmmx.rom + SNESCore::snes2pc(0x82F445)) = v;

		// <KORANOT>
		// rocks
		v = GetRandomRange(4, 8) << 12;
		*LPWORD(nmmx.rom + SNESCore::snes2pc(0x85EE27)) = v;
		// speed multiplier
		v = GetRandomRange(0x80, 0x100);
		*LPWORD(nmmx.rom + SNESCore::snes2pc(0x85E90E)) = v;

		// <DANCERS>
		// X move
		v = GetRandomRange(0x1000, 0x2000);
		*LPWORD(nmmx.rom + SNESCore::snes2pc(0x85DFAF)) = v;
		*LPWORD(nmmx.rom + SNESCore::snes2pc(0x85DFE0)) = v;
		// Y move
		v = GetRandomRange(0x800, 0xE00);
		*LPWORD(nmmx.rom + SNESCore::snes2pc(0x85DF34)) = v;
		*LPWORD(nmmx.rom + SNESCore::snes2pc(0x85DF65)) = v;
		// Sword
		v = GetRandomRange(3, 5);
		*LPWORD(nmmx.rom + SNESCore::snes2pc(0x85E025)) = 0xFFFF - v;
		*LPWORD(nmmx.rom + SNESCore::snes2pc(0x85E033)) = v;

		// <KNIGHT>
		for (unsigned i = 0; i < 10; i++) {
			// walk speed
			*LPWORD(nmmx.rom + SNESCore::snes2pc(0x81D1E7 + 2 * i)) *= GetRandom(1.5, 3.0);
			// sword walk speed
			*LPWORD(nmmx.rom + SNESCore::snes2pc(0x81D253 + 2 * i)) *= GetRandom(1.5, 3.0);
		}
	}

	b = (CButton*)GetDlgItem(IDC_ENEMY_HEALTH);
	if (b && b->GetCheck()) {
		CSliderCtrl *difficulty = (CSliderCtrl *)GetDlgItem(IDC_COMMON_DIFFICULTY);
		std::string d = difficultyNames[difficulty->GetPos()];
		int min = d == "Easy" ? 1 : d == "Medium" ? -1 : d == "Hard" ? -1 : 0;
		int max = d == "Easy" ? 0 : d == "Medium" ?  1 : d == "Hard" ?  2 : 3;

		if (dynamic) memset(nmmx.rom + SNESCore::snes2pc(0xA0C500), 0x00, 0x80 * 0x4 * sizeof(SHORT));

		for (unsigned i = 0; i < 0x80; i++) {
			a = 0x81AC00 + 2 * i;
			SHORT v = *PSHORT(nmmx.rom + SNESCore::snes2pc(a));
			if (v && v != 0x7FFF) {
				if (!dynamic) {
					v += GetRandomRange(min, max);
					*PSHORT(nmmx.rom + SNESCore::snes2pc(a)) = (v > 0) ? v : 1;
				}
				else {
					// get whip damage
					SHORT w = min(v, 0x30); // default whip damage
					for (unsigned j = 0; j < 4; j++) {
						SHORT d = w * GetRandomRange(min, max);
						*PSHORT(nmmx.rom + SNESCore::snes2pc(0xA0C500 + 8 * i + 2 * j)) = (d + v > 0) ? d : 1 - v;
					}
				}
			}
		}

		if (dynamic) {
			// update code
			// insert code
			a = 0x80D74E;
			// BRL (replace)
			*LPBYTE(nmmx.rom + SNESCore::snes2pc(a++)) = 0x82;
			*LPBYTE(nmmx.rom + SNESCore::snes2pc(a++)) = 0x51;
			*LPBYTE(nmmx.rom + SNESCore::snes2pc(a++)) = 0x28;

			a = 0x80FFA2;
			// PHX
			*LPBYTE(nmmx.rom + SNESCore::snes2pc(a++)) = 0xDA;
			// LDA $E8
			*LPBYTE(nmmx.rom + SNESCore::snes2pc(a++)) = 0xA5;
			*LPBYTE(nmmx.rom + SNESCore::snes2pc(a++)) = 0xE8;
			// AND length-1
			*LPBYTE(nmmx.rom + SNESCore::snes2pc(a++)) = 0x29;
			*LPWORD(nmmx.rom + SNESCore::snes2pc(a++)) = 0x03;
			a++;
			// ASL
			*LPBYTE(nmmx.rom + SNESCore::snes2pc(a++)) = 0x0A;
			// STA $06
			*LPBYTE(nmmx.rom + SNESCore::snes2pc(a++)) = 0x95;
			*LPBYTE(nmmx.rom + SNESCore::snes2pc(a++)) = 0x06;
			// TYA
			*LPBYTE(nmmx.rom + SNESCore::snes2pc(a++)) = 0x98;
			// ASL
			*LPBYTE(nmmx.rom + SNESCore::snes2pc(a++)) = 0x0A;
			// ASL
			*LPBYTE(nmmx.rom + SNESCore::snes2pc(a++)) = 0x0A;
			// ADC $06
			*LPBYTE(nmmx.rom + SNESCore::snes2pc(a++)) = 0x75;
			*LPBYTE(nmmx.rom + SNESCore::snes2pc(a++)) = 0x06;
			// TAX
			*LPBYTE(nmmx.rom + SNESCore::snes2pc(a++)) = 0xAA;
			// LDA base value
			*LPBYTE(nmmx.rom + SNESCore::snes2pc(a++)) = 0xB9;
			*LPBYTE(nmmx.rom + SNESCore::snes2pc(a++)) = 0x00;
			*LPBYTE(nmmx.rom + SNESCore::snes2pc(a++)) = 0xAC;
			// ADC long, X
			*LPBYTE(nmmx.rom + SNESCore::snes2pc(a++)) = 0x7F;
			*LPBYTE(nmmx.rom + SNESCore::snes2pc(a++)) = 0x00;
			*LPBYTE(nmmx.rom + SNESCore::snes2pc(a++)) = 0xC5;
			*LPBYTE(nmmx.rom + SNESCore::snes2pc(a++)) = 0xA0;
			// PLX
			*LPBYTE(nmmx.rom + SNESCore::snes2pc(a++)) = 0xFA;
			// BRL
			*LPBYTE(nmmx.rom + SNESCore::snes2pc(a++)) = 0x82;
			*LPBYTE(nmmx.rom + SNESCore::snes2pc(a++)) = 0x95;
			*LPBYTE(nmmx.rom + SNESCore::snes2pc(a++)) = 0xD7;

		}
	}

	b = (CButton*)GetDlgItem(IDC_ENEMY_SPEED);
	if (b && b->GetCheck()) {
		unsigned newXSub = 0;
		unsigned newX = 0;
		unsigned newYSub = 0;
		unsigned newY = 0;
		SHORT v = 0;

		// skeleton xsub
		v = GetRandomRange(0, 4) << 12;
		*LPWORD(nmmx.rom + SNESCore::snes2pc(0x80F3F7)) = v;
		*LPWORD(nmmx.rom + SNESCore::snes2pc(0x80F517)) = 0xFFFF - v;

		// bat
		v = GetRandomRange(0, 1);
		newX = v;
		//*LPWORD(nmmx.rom + SNESCore::snes2pc(0x86C60B)) = newXSub; // xpossub
		*LPWORD(nmmx.rom + SNESCore::snes2pc(0x86C610)) = newX; // xpos
		//*LPWORD(nmmx.rom + SNESCore::snes2pc(0x86C61B)) = 0xFFFF - newXSub; // xnegsub
		*LPWORD(nmmx.rom + SNESCore::snes2pc(0x86C620)) = 0xFFFF - newX; // xneg

		// medusa head
		v = GetRandomRange(0, 3) << 12;
		// X
		newXSub = (0x3000 + v) % 0x8000;
		newX = 0x1 + (0x3000 + v) / 0x8000;
		*LPWORD(nmmx.rom + SNESCore::snes2pc(0x82C1F1)) = newXSub; // xpossub
		*LPWORD(nmmx.rom + SNESCore::snes2pc(0x82C1F6)) = newX; // xpos
		*LPWORD(nmmx.rom + SNESCore::snes2pc(0x82C201)) = 0xFFFF - newXSub; // xnegsub
		*LPWORD(nmmx.rom + SNESCore::snes2pc(0x82C206)) = 0xFFFF - newX; // xneg
		// Y
		v = GetRandomRange(2, 5) << 12;
		newYSub = (0x4000 + v) % 0x8000;
		newY = 0x3 + (0x4000 + v) / 0x8000;
		*LPWORD(nmmx.rom + SNESCore::snes2pc(0x82C1DE)) = newYSub; // xpossub
		*LPWORD(nmmx.rom + SNESCore::snes2pc(0x82C1E3)) = newY; // xpos
		// allow "random" pixel offset by getting rid of carry flag modifiers
		*LPBYTE(nmmx.rom + SNESCore::snes2pc(0x82C4C7)) = 0xEA;
		*LPBYTE(nmmx.rom + SNESCore::snes2pc(0x82C4CE)) = 0xEA;
		// slightly shift starting locations
		v = GetRandomRange(-5, -2);
		newX = 0x120 + v;
		*LPWORD(nmmx.rom + SNESCore::snes2pc(0x82C4C9)) = newX; // xpos
		v = GetRandomRange(0, 3);
		newX = 0x20 + v;
		*LPWORD(nmmx.rom + SNESCore::snes2pc(0x82C4D0)) = newX; // xpos
	}

#if 0
	// bring in new game enemies
	for (unsigned i = 0; i < nmmx.numLevels; i++) {
		nmmx.level = i;
		nmmx.LoadLevel();

		std::vector<WORD> id;
		for (auto &e : nmmx.eventTable) {
			if (e.type == 0x0 && (e.match & 0x4)) {
				if (GetRandom(0, 1.0) < 0.9) {
					e.match &= ~0x4;
				}
			}
		}
		nmmx.SaveEvents();
	}
#endif

#if 0
	b = (CButton*)GetDlgItem(IDC_ENEMY_ID);
	if (b && b->GetCheck()) {
		for (unsigned i = 0; i < nmmx.numLevels; i++) {
			nmmx.level = i;
			nmmx.LoadLevel();

			std::set<WORD> activeEnemyId;
			nmmx.GetActiveEnemyId(activeEnemyId);
			// remove the boss if it exists
			activeEnemyId.erase(0x2A);

			// only look for ids that are enemies current in level
			std::vector<WORD> id;
			for (auto &e : nmmx.eventTable) {
				if (e.type == 0x0) {
					if (activeEnemyId.count(e.eventId)) {
						id.push_back(e.eventId);
					}
				}
			}

			if (id.empty()) continue;

			for (auto &e : nmmx.eventTable) {
				if (e.type == 0x0 && e.eventId != 0x0 && e.eventId != 0x2A) {
					std::random_shuffle(id.begin(), id.end());
					e.eventId = id.front();
				}
			}
			nmmx.SaveEvents();
		}
	}
#endif



	// make the lives counter a death counter
	*LPBYTE(nmmx.rom + SNESCore::snes2pc(0x8094DB) + 0) = 0x01;
	*LPBYTE(nmmx.rom + SNESCore::snes2pc(0x8CFD9B) + 0) = 0x01;
	*LPBYTE(nmmx.rom + SNESCore::snes2pc(0x8280A1) + 0) = 0x69;
	*LPBYTE(nmmx.rom + SNESCore::snes2pc(0x8280A2) + 0) = 0x00;
	// get rid of the timer
	// STZ
	*LPBYTE(nmmx.rom + SNESCore::snes2pc(0x86B53D) + 0) = 0x9C;
	// fall through always for timer
	*LPBYTE(nmmx.rom + SNESCore::snes2pc(0x80DA80) + 0) = 0xEA;
	*LPBYTE(nmmx.rom + SNESCore::snes2pc(0x80DA80) + 1) = 0xEA;
	// fall through for stairs
	*LPBYTE(nmmx.rom + SNESCore::snes2pc(0x82BA8D) + 0) = 0xEA;
	*LPBYTE(nmmx.rom + SNESCore::snes2pc(0x82BA8D) + 1) = 0xEA;
	// remove score extra life on score
	*LPBYTE(nmmx.rom + SNESCore::snes2pc(0x80DF74) + 0) = 0x80;
	*LPBYTE(nmmx.rom + SNESCore::snes2pc(0x80DF74) + 1) = 0x21;
	//*LPBYTE(nmmx.rom + SNESCore::snes2pc(0x80DF74) + 2) = 0xEA;
	//// swap death counter into timer
	//*LPBYTE(nmmx.rom + SNESCore::snes2pc(0x80C756) + 0) = 0xA5;
	//*LPBYTE(nmmx.rom + SNESCore::snes2pc(0x80C756) + 1) = 0x7C;
	//*LPBYTE(nmmx.rom + SNESCore::snes2pc(0x80C756) + 2) = 0x3A;
	//// force lives counter to be 0
	//*LPBYTE(nmmx.rom + SNESCore::snes2pc(0x80C776) + 0) = 0xA9;
	//*LPBYTE(nmmx.rom + SNESCore::snes2pc(0x80C776) + 1) = 0x00;
	//*LPBYTE(nmmx.rom + SNESCore::snes2pc(0x80C776) + 2) = 0x00;
	//*LPBYTE(nmmx.rom + SNESCore::snes2pc(0x80C776) + 3) = 0xEA;
	//*LPBYTE(nmmx.rom + SNESCore::snes2pc(0x80C776) + 4) = 0xEA;
	//*LPBYTE(nmmx.rom + SNESCore::snes2pc(0x80C776) + 5) = 0xEA;
	//*LPBYTE(nmmx.rom + SNESCore::snes2pc(0x80C776) + 6) = 0xEA;
	//*LPBYTE(nmmx.rom + SNESCore::snes2pc(0x80C776) + 7) = 0xEA;
	//*LPBYTE(nmmx.rom + SNESCore::snes2pc(0x80C776) + 8) = 0xEA;
	//*LPBYTE(nmmx.rom + SNESCore::snes2pc(0x80C776) + 9) = 0xEA;
	// 3 digit death counter
	*LPBYTE(nmmx.rom + SNESCore::snes2pc(0x80C785) + 0) = 0x1B;

	//b = (CButton*)GetDlgItem(IDC_LEVEL_TIMER);
	//if (b && b->GetCheck()) {
	//	CSliderCtrl *difficulty = (CSliderCtrl *)GetDlgItem(IDC_COMMON_DIFFICULTY);
	//	std::string d = difficultyNames[difficulty->GetPos()];
	//	//double min = d == "Easy" ? 1.2 : d == "Medium" ? 0.9 : d == "Hard" ? 0.8 : 0.6;
	//	//double max = d == "Easy" ? 1.5 : d == "Medium" ? 1.0 : d == "Hard" ? 0.9 : 0.7;
	//	double min = 1.2;
	//	double max = 1.5;
	//	for (unsigned i = 0; i < nmmx.numLevels; i++) {
	//		DWORD a = 0x85BCF8 + 2 * i;
	//		WORD v = *LPWORD(nmmx.rom + SNESCore::snes2pc(a));
	//		v = SC4Core::ConvertWordBCDToDec(v);
	//		v *= GetRandom(min, max);
	//		v = SC4Core::ConvertWordDecToBCD(v);
	//		*LPWORD(nmmx.rom + SNESCore::snes2pc(a)) = 0x999; // v;
	//	}
	//}

	// 0x01 = Some enemy that sits
	// 0x02 = <Crash> Jumping enemy that crashes when touched
	// 0x03 = Null
	// 0x04 = Null
	// 0x05 = Null
	// 0x06 = <Crash>
	// 0x07 = <Crash> Flying enemy
	// 0x08 = <Crash> Dancing enemy.  Teleports to dancing stage.
	// 0x09 = <Crash> Floor enemy.  Teleports to stage.
	// 0x0A = Running enemy
	// 0x0B = <Crash> Simon clone
	// 0x0C = Flying enemy (can't hit)
	// 0x0D = Null
	// 0x0E = <Crash> Candle
	// 0x0F = <Crash> Floating enemy
	// 0x10 = <Crash> Floating enemy.  Bad crash.
	// 0x11 = <Crash> Enemy
	// 0x12 = <Crash> Enemy
	// 0x13 = Null
	// 0x14 = <Crash> Castle side
	// 0x15 = <Crash> Null
	// 0x16 = <Crash> Enemy
	// 0x17 = Null
	// 0x32 = <Crash> Enemy hittable
	// 0x42 = <Crash> Enemy
	// 0x52 = <Crash> Enemy
	// 0x72 = <Crash> Enemy
	// 0x82 = <Crash> Enemy
	// 0x92 = <Crash> Enemy
	// 0xB2 = <Crash> Enemy
	// 0xC2 = <Crash> Enemy
	// 0xD2 = <Crash> Enemy
	// 0xF2 = <Crash> Enemy

	// candle drops
	// out of ~1024
	std::vector<std::tuple<double, WORD>> candleWeight = {
		{  150, 0x00 }, // = Null
		{  300, 0x18 }, // = Small Heart
		{   50, 0x19 }, // = Big Heart
		{   20, 0x1A }, // = Dagger
		{   20, 0x1B }, // = Axe
		{   20, 0x1C }, // = Holy Water
		{   25, 0x1D }, // = Cross
		{   15, 0x1E }, // = Stopwatch
		{    5, 0x1F }, // = Rosary
		{    5, 0x20 }, // = Potion (Invincible)
		{   80, 0x21 }, // = Whip Upgrade
		{   80, 0x22 }, // = Money (100)
		{    2, 0x23 }, // = Double
		{    2, 0x24 }, // = Triple
		{   30, 0x25 }, // = Small Meat
		{   10, 0x26 }, // = Large Meat
		{/*2*/0, 0x27 }, // = Orb
		{/*2*/0, 0x28 }, // = 1Up
		{   60, 0x62 }, // = Money (300)
		{   40, 0xA2 }, // = Money (500)
		{   20, 0xE2 }, // = Money (700)
	};

	// normalize
	double total = 0;
	for (auto &f : candleWeight) { total += std::get<double>(f); }
	double subTotal = 0;
	for (auto &f : candleWeight) { std::get<double>(f) /= total; std::get<double>(f) += subTotal; subTotal = std::get<double>(f); }

	b = (CButton*)GetDlgItem(IDC_LEVEL_CANDLE);
	if (b && b->GetCheck()) {
		if (!dynamic) {
			for (unsigned i = 0; i < nmmx.numLevels; i++) {
				nmmx.level = i;
				nmmx.LoadLevel();
				for (auto &e : nmmx.eventTable) {
					if (e.type == 0x1) {
						// candle
						// 
						double r = GetRandom(0.0, 1.0);
						unsigned index = 0;
						for (unsigned j = 0; j < candleWeight.size(); j++) {
							auto &f = candleWeight[j];
							if (r <= std::get<double>(f)) {
								e.eventId = std::get<WORD>(f);
								break;
							}
						}
					}
				}
				nmmx.SaveEvents();
			}
		}
		else {
			a = 0;

			// insert table
			unsigned offset = 0;
			unsigned length = 0x1000;
			a = 0xA08000;
			for (auto &f : candleWeight) {
				for (; offset < (std::get<double>(f) * length); offset++) {
					*LPBYTE(nmmx.rom + SNESCore::snes2pc(a + offset)) = std::get<WORD>(f);
				}
			}
			std::random_shuffle(nmmx.rom + SNESCore::snes2pc(a + 0), nmmx.rom + SNESCore::snes2pc(a + length));

			// insert code
			a = 0x80E08E;
			// JSL
			*LPBYTE(nmmx.rom + SNESCore::snes2pc(a++)) = 0x22;
			*LPBYTE(nmmx.rom + SNESCore::snes2pc(a++)) = 0x80;
			*LPBYTE(nmmx.rom + SNESCore::snes2pc(a++)) = 0xFF;
			*LPBYTE(nmmx.rom + SNESCore::snes2pc(a++)) = 0x80;
			// STA D, $10, X
			*LPBYTE(nmmx.rom + SNESCore::snes2pc(a++)) = 0x95;
			*LPBYTE(nmmx.rom + SNESCore::snes2pc(a++)) = 0x10;
			// NOP
			*LPBYTE(nmmx.rom + SNESCore::snes2pc(a++)) = 0xEA;

			a = 0x80FF80;
			// PHX
			*LPBYTE(nmmx.rom + SNESCore::snes2pc(a++)) = 0xDA;
			// generate new randoms
			// PHD
			*LPBYTE(nmmx.rom + SNESCore::snes2pc(a++)) = 0x0B;
			// JSL
			*LPBYTE(nmmx.rom + SNESCore::snes2pc(a++)) = 0x22;
			*LPBYTE(nmmx.rom + SNESCore::snes2pc(a++)) = 0x89;
			*LPBYTE(nmmx.rom + SNESCore::snes2pc(a++)) = 0x81;
			*LPBYTE(nmmx.rom + SNESCore::snes2pc(a++)) = 0x80;
			// PLD
			*LPBYTE(nmmx.rom + SNESCore::snes2pc(a++)) = 0x2B;
			// LDA $E8
			*LPBYTE(nmmx.rom + SNESCore::snes2pc(a++)) = 0xA5;
			*LPBYTE(nmmx.rom + SNESCore::snes2pc(a++)) = 0xEA;
			// AND length-1
			*LPBYTE(nmmx.rom + SNESCore::snes2pc(a++)) = 0x29;
			*LPWORD(nmmx.rom + SNESCore::snes2pc(a++)) = length - 1;
			a++;
			// TAX
			*LPBYTE(nmmx.rom + SNESCore::snes2pc(a++)) = 0xAA;
			// LDA long, X
			*LPBYTE(nmmx.rom + SNESCore::snes2pc(a++)) = 0xBF;
			*LPBYTE(nmmx.rom + SNESCore::snes2pc(a++)) = 0x00;
			*LPBYTE(nmmx.rom + SNESCore::snes2pc(a++)) = 0x80;
			*LPBYTE(nmmx.rom + SNESCore::snes2pc(a++)) = 0xA0;
			// PLX
			*LPBYTE(nmmx.rom + SNESCore::snes2pc(a++)) = 0xFA;
			// AND #3F
			*LPBYTE(nmmx.rom + SNESCore::snes2pc(a++)) = 0x29;
			*LPBYTE(nmmx.rom + SNESCore::snes2pc(a++)) = 0x3F;
			*LPBYTE(nmmx.rom + SNESCore::snes2pc(a++)) = 0x00;
			// RTL
			*LPBYTE(nmmx.rom + SNESCore::snes2pc(a++)) = 0x6B;

			// never drop whip upgrade from small heart
			*LPBYTE(nmmx.rom + SNESCore::snes2pc(0x80E057)) = 0x80;
			// convert double/triple to small/big heart
			//*LPBYTE(nmmx.rom + SNESCore::snes2pc(0x80E04D)) = 0x18;
			// always drop double/triple independent of whip
			*LPBYTE(nmmx.rom + SNESCore::snes2pc(0x80E027)) = 0xEA;
			*LPBYTE(nmmx.rom + SNESCore::snes2pc(0x80E028)) = 0xEA;
			*LPBYTE(nmmx.rom + SNESCore::snes2pc(0x80E02F)) = 0xEA;
			*LPBYTE(nmmx.rom + SNESCore::snes2pc(0x80E030)) = 0xEA;

			// dynamic drac secret drops
			// JSL
			a = 0x86C758;
			*LPBYTE(nmmx.rom + SNESCore::snes2pc(a++)) = 0x22;
			*LPBYTE(nmmx.rom + SNESCore::snes2pc(a++)) = 0x80;
			*LPBYTE(nmmx.rom + SNESCore::snes2pc(a++)) = 0xFF;
			*LPBYTE(nmmx.rom + SNESCore::snes2pc(a++)) = 0x80;
			// CMP #18
			*LPBYTE(nmmx.rom + SNESCore::snes2pc(a++)) = 0xC9;
			*LPBYTE(nmmx.rom + SNESCore::snes2pc(a++)) = 0x18;
			*LPBYTE(nmmx.rom + SNESCore::snes2pc(a++)) = 0x00;
			// BNE +1
			*LPBYTE(nmmx.rom + SNESCore::snes2pc(a++)) = 0xD0;
			*LPBYTE(nmmx.rom + SNESCore::snes2pc(a++)) = 0x01;
			// INC
			*LPBYTE(nmmx.rom + SNESCore::snes2pc(a++)) = 0x1A;
			// TAY
			*LPBYTE(nmmx.rom + SNESCore::snes2pc(a++)) = 0xA8;
			// BRA drop a lot
			*LPBYTE(nmmx.rom + SNESCore::snes2pc(a++)) = 0x80;
			*LPBYTE(nmmx.rom + SNESCore::snes2pc(a++)) = 0x43;
		}

		// fix up graphics for leather whip if we allow random whip upgrade drops
		a = 0x80DF11;
		// JSR to new code
		*LPBYTE(nmmx.rom + SNESCore::snes2pc(a++)) = 0x20;
		*LPBYTE(nmmx.rom + SNESCore::snes2pc(a++)) = 0x70;
		*LPBYTE(nmmx.rom + SNESCore::snes2pc(a++)) = 0xFF;

		a = 0x80FF70;
		// LDX #B3FE
		*LPBYTE(nmmx.rom + SNESCore::snes2pc(a++)) = 0xA2;
		*LPBYTE(nmmx.rom + SNESCore::snes2pc(a++)) = 0xFE;
		*LPBYTE(nmmx.rom + SNESCore::snes2pc(a++)) = 0xB3;
		// LDA #92
		*LPBYTE(nmmx.rom + SNESCore::snes2pc(a++)) = 0xA5;
		*LPBYTE(nmmx.rom + SNESCore::snes2pc(a++)) = 0x92;
		// CMP #2
		*LPBYTE(nmmx.rom + SNESCore::snes2pc(a++)) = 0xC9;
		*LPBYTE(nmmx.rom + SNESCore::snes2pc(a++)) = 0x02;
		*LPBYTE(nmmx.rom + SNESCore::snes2pc(a++)) = 0x00;
		// BNE +3
		*LPBYTE(nmmx.rom + SNESCore::snes2pc(a++)) = 0xD0;
		*LPBYTE(nmmx.rom + SNESCore::snes2pc(a++)) = 0x03;
		// LDX #B411
		*LPBYTE(nmmx.rom + SNESCore::snes2pc(a++)) = 0xA2;
		*LPBYTE(nmmx.rom + SNESCore::snes2pc(a++)) = 0x11;
		*LPBYTE(nmmx.rom + SNESCore::snes2pc(a++)) = 0xB4;
		// RTS
		*LPBYTE(nmmx.rom + SNESCore::snes2pc(a++)) = 0x60;
	}

	b = (CButton*)GetDlgItem(IDC_LEVEL_ITEMGFX);
	if (b && b->GetCheck()) {
		a = nmmx.expandedOffset[0x44][0x6200].first;
		// randomize the subweapon graphics
		std::array<unsigned, 5> subweaponOffset = { 0x6, 0x8, 0xA, 0x24, 0x28 };
		// D, A, V, C, W
		std::vector<unsigned> subweaponOrder = { 0, 1, 2, 3, 4 };

		unsigned subweaponTemp[5][4][8];
		for (unsigned i = 0; i < subweaponOrder.size(); i++) {
			memcpy(subweaponTemp[subweaponOrder[i]][0], nmmx.rom + a + (subweaponOffset[i] + 0x00) * 0x20, 0x20);
			memcpy(subweaponTemp[subweaponOrder[i]][1], nmmx.rom + a + (subweaponOffset[i] + 0x01) * 0x20, 0x20);
			memcpy(subweaponTemp[subweaponOrder[i]][2], nmmx.rom + a + (subweaponOffset[i] + 0x10) * 0x20, 0x20);
			memcpy(subweaponTemp[subweaponOrder[i]][3], nmmx.rom + a + (subweaponOffset[i] + 0x11) * 0x20, 0x20);
		}
		std::random_shuffle(subweaponOrder.begin(), subweaponOrder.end());
		for (unsigned i = 0; i < subweaponOrder.size(); i++) {
			memcpy(nmmx.rom + a + (subweaponOffset[i] + 0x00) * 0x20, subweaponTemp[subweaponOrder[i]][0], 0x20);
			memcpy(nmmx.rom + a + (subweaponOffset[i] + 0x01) * 0x20, subweaponTemp[subweaponOrder[i]][1], 0x20);
			memcpy(nmmx.rom + a + (subweaponOffset[i] + 0x10) * 0x20, subweaponTemp[subweaponOrder[i]][2], 0x20);
			memcpy(nmmx.rom + a + (subweaponOffset[i] + 0x11) * 0x20, subweaponTemp[subweaponOrder[i]][3], 0x20);
		}

		// copy over extra graphics for vial and cross
		for (auto i : { 2, 3 }) {
			if (i != subweaponOrder[i]) {
				memcpy(nmmx.rom + a + (subweaponOffset[i] + 2 + 0x00) * 0x20, subweaponTemp[subweaponOrder[i]][0], 0x20);
				memcpy(nmmx.rom + a + (subweaponOffset[i] + 2 + 0x01) * 0x20, subweaponTemp[subweaponOrder[i]][1], 0x20);
				memcpy(nmmx.rom + a + (subweaponOffset[i] + 2 + 0x10) * 0x20, subweaponTemp[subweaponOrder[i]][2], 0x20);
				memcpy(nmmx.rom + a + (subweaponOffset[i] + 2 + 0x11) * 0x20, subweaponTemp[subweaponOrder[i]][3], 0x20);
			}
		}
	}

	b = (CButton*)GetDlgItem(IDC_LEVEL_CRUSHERS);
	if (b && b->GetCheck()) {
		for (unsigned i = 0; i < 1; i++) {
			LPWORD pCrusher = LPWORD(nmmx.rom + SNESCore::snes2pc(0x81C15B + (4 * 0xA) * i));
			// randomize drop speed
			WORD v = GetRandomRange(0, 1);
			*(pCrusher + 0 * 5 + 0) >>= v;
			*(pCrusher + 0 * 5 + 4) <<= v;

			// randomize up speed
			v = GetRandomRange(0, 1);
			*(pCrusher + 1 * 5 + 0) >>= v;
			*(pCrusher + 1 * 5 + 4) -= v;

			// randomize wait time
			v = *(pCrusher + 2 * 5 + 0);
			v = GetRandomRange(v - 10, v + 10);
			*(pCrusher + 2 * 5 + 0) = v;
		}
	}


	// right (2, 14, 18), 26, 31, 35, 38, 42, 44, 46
	// left 4, 29, 37, 43, 48
	// none 10, 11, 12, 22, 25
	b = (CButton*)GetDlgItem(IDC_LEVEL_ORDER);
	if (b && b->GetCheck()) {
		// 2 (needs step), 5 (boring), x14 (needs step), 18, 26, 31, 35 (needs step), 38, 42, 44 (crushers, steps)
		// 46 (needs steps), 49 (step), 55 (reverse gear), 60
		std::vector<WORD> right     = { 2,5,18,26,31,35,38,42,44,46,49,55,60 };
		std::vector<WORD> leftRight = { 3,6,14,15,16,19,27,28,29,32,33,36,39,40,47,50,51,52,56,57 };
		// 4 (move turret), x29, 37 (move skeleton), 43 (crusher event, step?), 48 (steps, move skeleton)
		// 58 (boring), 61 (reverse crumbling?)
		std::vector<WORD> left      = { 4,7,17,20,34,37,41,43,45,48,53,58,61 };
		// 8 (boring), x10, x11, x12 (needs 2 steps), x21 (rotation), 22 (need to reverse flow of drops)
		// 24 (boring), x25
		std::vector<WORD> none      = { 1,8,9,10,11,12,13,21,22,23,24,25,59,62,63,64,65 };
		std::vector<WORD> reverse   = { 3,6,10,11,12,14,15,16,19,21,22,25,27,28,29,32,33,36,39,40,47,50,51,52,56,57 };
		std::vector<WORD> reverseAllowed = reverse;

		// 30 and 54 are special.  30 has to be 2 after.  Hopefully 54 can be ignored (event handles it)
		std::deque<std::vector<WORD>> levelGroups;

		ASSERT(left.size() == right.size());

		// place rights
		std::random_shuffle(right.begin(), right.end());
		for (auto &l : right) {
			levelGroups.push_back({ l });
		}
		// pair lefts with rights
		std::random_shuffle(left.begin(), left.end());
		auto it = levelGroups.begin();
		for (auto &l : left) {
			it->push_back(l);
			it++;
		}
		// insert leftRights
		std::random_shuffle(leftRight.begin(), leftRight.end());
		for (auto &l : leftRight) {
			random_shuffle(levelGroups.begin(), levelGroups.end());
			auto &g = levelGroups.front();
			g.insert(g.begin() + 1, l);
		}
		// insert none
		for (auto &l : none) {
			levelGroups.push_front({ l });
		}
		random_shuffle(levelGroups.begin(), levelGroups.end());
		// end levels
		levelGroups.push_front({ 0 });
		levelGroups.push_back({ 66,67 });

		//// Forward direction state
		//for (unsigned i = 0; i < nmmx.numLevels; ++i) {
		//	*LPWORD(nmmx.rom + SNESCore::snes2pc(0xA0C200 + 2 * i)) = 0x0;
		//}
		//// Reverse direction state
		//for (unsigned i = 0; i < nmmx.numLevels; ++i) {
		//	*LPWORD(nmmx.rom + SNESCore::snes2pc(0xA0C300 + 2 * i)) = 0x1;
		//}
		//// Death/Continue forward direction
		//for (unsigned i = 0; i < nmmx.numLevels; ++i) {
		//	*LPWORD(nmmx.rom + SNESCore::snes2pc(0xA0C400 + 2 * i)) = 0x0;
		//}

		b = (CButton*)GetDlgItem(IDC_LEVEL_REVERSE);
		if (b->GetCheck()) {
			EventInfo e;
			// add reverse support
			//--------------------

			// 14
			// add event
			nmmx.level = 14;
			nmmx.LoadLevel();
			// copy exit event
			auto it = nmmx.eventTable.begin();
			std::advance(it, 0x14);
			e = *it;
			e.xpos = 0x18;
			e.ypos = 0x80;
			nmmx.AddEvent(0x1, e);
			nmmx.SortEvents();
			it = nmmx.eventTable.begin();
			std::advance(it, 0x2);
			(*it).eventSubId = 0x16;
			nmmx.SaveEvents();
			nmmx.GetLevelExit((*it).eventSubId).type = 0x3;
			nmmx.GetLevelExit((*it).eventSubId).exitNum = 0x1;
			nmmx.GetLevelExit((*it).eventSubId).cmpValue = 0xC;

			// 27 chandelier
			// copy swinging event
			nmmx.level = 27;
			nmmx.LoadLevel();
			// copy exit event
			it = nmmx.eventTable.begin();
			std::advance(it, 0x17);
			e = *it;
			e.xpos = 0x8C;
			e.ypos = 0x5C;
			nmmx.AddEvent(0x14, e);
			nmmx.SortEvents();
			it = nmmx.eventTable.begin();
			std::advance(it, 0x3);
			(*it).match = 0x16;
			nmmx.SaveEvents();

			// Add state/direction
			nmmx.GetLevelEntrance(1).deathState0 = 0x010A;
			nmmx.GetLevelEntrance(1).xpos = 0x0080;
			nmmx.GetLevelEntrance(1).ypos = 0x0055;
			nmmx.GetLevelEntrance(1).c0_xpos = 0x0060;
			//nmmx.GetLevelEntrance(1).c0_ypos = 0x0000;
			nmmx.GetLevelEntrance(1).c1_xpos = 0x0060;
			//nmmx.GetLevelEntrance(1).c1_ypos = 0x0000;

			//*LPWORD(nmmx.rom + SNESCore::snes2pc(0x85BE08 + 0x36)) = 0x0A8A;
			//// Add position
			//*LPWORD(nmmx.rom + SNESCore::snes2pc(0x85BFA0 + 0x6C + 0)) = 0x0080;
			//*LPWORD(nmmx.rom + SNESCore::snes2pc(0x85BFA0 + 0x6C + 2)) = 0x0055;
			//// Add camera
			//*LPWORD(nmmx.rom + SNESCore::snes2pc(0x85CCA2 + 0x6C + 0)) = 0x0060;
			//*LPWORD(nmmx.rom + SNESCore::snes2pc(0x85CEC2 + 0x6C + 0)) = 0x0060;

			// 28 after chandelier
			nmmx.level = 28;
			nmmx.LoadLevel();
			// copy exit event
			it = nmmx.eventTable.begin();
			std::advance(it, 0x14);
			e = *it;
			e.xpos = 0x9E0;
			e.ypos = 0x1C;
			e.eventSubId = 0x15;
			// add left exit
			nmmx.GetLevelExit(e.eventSubId).type = 0x4;
			nmmx.GetLevelExit(e.eventSubId).exitNum = 0x1;
			nmmx.GetLevelExit(e.eventSubId).cmpValue = 0x9F5;

			nmmx.AddEvent(0x14, e);
			nmmx.SortEvents();
			nmmx.SaveEvents();

			// 29
			nmmx.level = 29;
			nmmx.LoadLevel();
			// add entrance
			nmmx.GetLevelEntrance(1).deathState0 = 0x018A;
			nmmx.GetLevelEntrance(1).state1 = 0x0002;
			nmmx.GetLevelEntrance(1).xpos = 0x06E0;
			nmmx.GetLevelEntrance(1).ypos = 0x0045;
			nmmx.GetLevelEntrance(1).c0_xpos = 0x0600;
			nmmx.GetLevelEntrance(1).c0_ypos = 0x0000;
			nmmx.GetLevelEntrance(1).c1_xpos = 0x04D0;
			nmmx.GetLevelEntrance(1).c1_ypos = 0x0000;

			//// direction
			//*LPWORD(nmmx.rom + SNESCore::snes2pc(0x85BE08 + 29 * 2 + 0)) = 0x8A0A;
			//*LPWORD(nmmx.rom + SNESCore::snes2pc(0x85C1C0 + 29 * 2 + 0)) = 0x0002;
			//// Add position
			//*LPWORD(nmmx.rom + SNESCore::snes2pc(0x85BFA0 + 29 * 4 + 0)) = 0x06E0;
			//*LPWORD(nmmx.rom + SNESCore::snes2pc(0x85BFA0 + 29 * 4 + 2)) = 0x0045;
			//// Add camera
			//*LPWORD(nmmx.rom + SNESCore::snes2pc(0x85CCA2 + 29 * 4 + 0)) = 0x0600;
			//*LPWORD(nmmx.rom + SNESCore::snes2pc(0x85CCA2 + 29 * 4 + 2)) = 0x0000;
			//*LPWORD(nmmx.rom + SNESCore::snes2pc(0x85CEC2 + 29 * 4 + 0)) = 0x04D0;
			//*LPWORD(nmmx.rom + SNESCore::snes2pc(0x85CEC2 + 29 * 4 + 2)) = 0x0000;


			// 51
			// make sure blocks appear after secret
			//*LPBYTE(nmmx.rom + SNESCore::snes2pc(0x86B6C7 + 0)) = 0xEE;
			//*LPBYTE(nmmx.rom + SNESCore::snes2pc(0x86B6C7 + 1)) = 0xD4;
			//*LPBYTE(nmmx.rom + SNESCore::snes2pc(0x86B6C7 + 2)) = 0x13;
			//*LPBYTE(nmmx.rom + SNESCore::snes2pc(0x86B6C7 + 3)) = 0xEE;
			//--------------------

			// No longer needed because we set the direction correct on death.
			//// stop death abuse by leaving direction state the same at death.
			//*LPBYTE(nmmx.rom + SNESCore::snes2pc(0x8280B1 + 0)) = 0xEA;
			//*LPBYTE(nmmx.rom + SNESCore::snes2pc(0x8280B1 + 1)) = 0xEA;
			//*LPBYTE(nmmx.rom + SNESCore::snes2pc(0x8280B1 + 2)) = 0xEA;

			//*LPBYTE(nmmx.rom + SNESCore::snes2pc(0x8CFD59 + 0)) = 0xEA;
			//*LPBYTE(nmmx.rom + SNESCore::snes2pc(0x8CFD59 + 1)) = 0xEA;
			//*LPBYTE(nmmx.rom + SNESCore::snes2pc(0x8CFD59 + 2)) = 0xEA;

			std::vector<WORD> reverseTemp;
			for (auto &l : reverse) {
				// bias towards climbing
				if ((b->GetCheck() == BST_INDETERMINATE) || (GetRandom(0.0, 1.0) < ((l == 36) ? 0.8 : (l == 52 || l == 56) ? 0.2 : 0.6))) { // FIXME
					reverseTemp.push_back(l);
					//*LPWORD(nmmx.rom + SNESCore::snes2pc(0xA0C400 + 2 * l)) = 0x1;
					nmmx.GetLevelEntrance(1, l).deathState0 &= 0x00FF;
					nmmx.GetLevelEntrance(1, l).deathState0 |= 0x0100;
					if (l == 2) {
						nmmx.level = l;
						nmmx.LoadLevel();

						auto it = nmmx.eventTable.begin();
						if (nmmx.eventTable.size() > 0x12) {
							std::advance(it, 0x12);
							it->xpos = 0x2B0;
							it->ypos = 0x70;

							nmmx.SortEvents();
							nmmx.SaveEvents();
						}

						nmmx.SetSceneBlock(2, 0, 5, 3, 0x00);
						nmmx.SetSceneBlock(2, 0, 5, 4, 0x1D | 0x4000);
					}
					else if (l == 5) {
						nmmx.level = l;
						nmmx.LoadLevel();

						nmmx.SetSceneBlock(2, 0, 3, 0, 0x09);
						nmmx.SetSceneBlock(2, 0, 4, 1, 0x09);
						nmmx.SetSceneBlock(2, 0, 5, 2, 0x09);
						nmmx.SetSceneBlock(2, 0, 6, 3, 0x09);
					}
					else if (l == 6) {
						// move event
						nmmx.level = l;
						nmmx.LoadLevel();
						auto it = nmmx.eventTable.begin();
						if (nmmx.eventTable.size() > 0x1C) {
							std::advance(it, 0x1C);
							it->ypos = 0xA0;

							nmmx.SortEvents();
							nmmx.SaveEvents();
						}

						// change blocks
						nmmx.SetSceneBlock(5, 0, 3, 4, 0x3F | 0x8000);
						nmmx.SetSceneBlock(5, 0, 4, 4, 0x04);

						nmmx.SetSceneBlock(5, 0, 6, 5, 0x3F);
						nmmx.SetSceneBlock(5, 0, 7, 5, 0x29);
						nmmx.SetSceneBlock(5, 0, 6, 6, 0x04);
						nmmx.SetSceneBlock(5, 0, 7, 6, 0x04);
					}
					else if (l == 10) {
						nmmx.level = l;
						nmmx.LoadLevel();
						it = nmmx.eventTable.begin();
						// setup water direction
						std::advance(it, 1);
						(*it).xpos = 0x76A;
						(*it).ypos = 0x236;
						it = nmmx.eventTable.begin();
						std::advance(it, 0x1D);
						// setup exit 
						(*it).xpos = 0x80;
						(*it).ypos = 0x80;
						(*it).eventSubId = 0x17;
						//// copy camera lock
						//it = nmmx.eventTable.begin();
						//std::advance(it, 0xD);
						//e = *it;
						//e.xpos = 0xC;
						//memcpy(nmmx.rom + SNESCore::snes2pc(0xA58000 + 0xC * 0x20), nmmx.rom + SNESCore::snes2pc(0xA58000 + 0xC * e.eventSubId), 0xC);
						//*LPWORD(nmmx.rom + SNESCore::snes2pc(0xA58000 + 0xC * (*it).eventSubId + 0)) = 0x1;
						//*LPWORD(nmmx.rom + SNESCore::snes2pc(0xA58000 + 0xC * (*it).eventSubId + 6)) = 0xC;
						//e.eventSubId = 0x20;
						//nmmx.AddEvent(1, e);
						// delete right exit
						//nmmx.DelEvent(nmmx.eventTable.size() - 1);
						// add left exit
						nmmx.GetLevelExit((*it).eventSubId).type = 0x3;
						nmmx.GetLevelExit((*it).eventSubId).exitNum = 0x1;
						nmmx.GetLevelExit((*it).eventSubId).cmpValue = 0xC;

						// Add state/direction
						//*LPWORD(nmmx.rom + SNESCore::snes2pc(0x85BE08 + 0x14)) = 0x0000;
						//// Add position
						//*LPWORD(nmmx.rom + SNESCore::snes2pc(0x85BFA0 + 0x6C + 0)) = 0x0080;
						//*LPWORD(nmmx.rom + SNESCore::snes2pc(0x85BFA0 + 0x6C + 2)) = 0x0055;
						//// Add camera
						//*LPWORD(nmmx.rom + SNESCore::snes2pc(0x85CCA2 + 0x6C + 0)) = 0x0060;

						nmmx.SortEvents();
						nmmx.SaveEvents();

						// setup initial camera lock to 0x1e0
						nmmx.GetLevelEntrance(1).lockBottom = 0x01E0;
						//*LPWORD(nmmx.rom + SNESCore::snes2pc(0x85D16A + 2 * l)) = 0x1E0;
					}
					else if (l == 11) {
						nmmx.level = l;
						nmmx.LoadLevel();
						it = nmmx.eventTable.begin();
						// setup water direction
						std::advance(it, 2);
						(*it).xpos = 0x780;
						(*it).ypos = 0x3D0;
						// setup exit
						it = nmmx.eventTable.end();
						it--;
						(*it).xpos = 0x30;
						(*it).ypos = 0x30;
						(*it).eventSubId = 0x18;
						//*LPWORD(nmmx.rom + SNESCore::snes2pc(0xA58000 + 0xC * (*it).eventSubId + 0)) = 0x1;
						//*LPWORD(nmmx.rom + SNESCore::snes2pc(0xA58000 + 0xC * (*it).eventSubId + 6)) = 0xC;
						// add left exit
						nmmx.GetLevelExit((*it).eventSubId).type = 0x3;
						nmmx.GetLevelExit((*it).eventSubId).exitNum = 0x1;
						nmmx.GetLevelExit((*it).eventSubId).cmpValue = 0xC;

						// direction
						nmmx.GetLevelEntrance(1).deathState0 = 0x0180;
						nmmx.GetLevelEntrance(1).state1 = 0x0002;
						nmmx.GetLevelEntrance(1).xpos = 0x07D0;
						nmmx.GetLevelEntrance(1).ypos = 0x03B1;
						nmmx.GetLevelEntrance(1).c0_xpos = 0x0700;
						nmmx.GetLevelEntrance(1).c0_ypos = 0x0320;
						nmmx.GetLevelEntrance(1).c1_xpos = 0x0700;
						nmmx.GetLevelEntrance(1).c1_ypos = 0x0320;

						//*LPWORD(nmmx.rom + SNESCore::snes2pc(0x85BE08 + l * 2 + 0)) = 0x8000;
						//*LPWORD(nmmx.rom + SNESCore::snes2pc(0x85C1C0 + l * 2 + 0)) = 0x0002;
						//// Add position
						//*LPWORD(nmmx.rom + SNESCore::snes2pc(0x85BFA0 + l * 4 + 0)) = 0x07D0;
						//*LPWORD(nmmx.rom + SNESCore::snes2pc(0x85BFA0 + l * 4 + 2)) = 0x03B1;
						//// Add camera
						//*LPWORD(nmmx.rom + SNESCore::snes2pc(0x85CCA2 + l * 4 + 0)) = 0x0700;
						//*LPWORD(nmmx.rom + SNESCore::snes2pc(0x85CCA2 + l * 4 + 2)) = 0x0320;
						//*LPWORD(nmmx.rom + SNESCore::snes2pc(0x85CEC2 + l * 4 + 0)) = 0x0700;
						//*LPWORD(nmmx.rom + SNESCore::snes2pc(0x85CEC2 + l * 4 + 2)) = 0x0320;

						nmmx.SortEvents();
						nmmx.SaveEvents();
					}
					else if (l == 12) {
						nmmx.level = l;
						nmmx.LoadLevel();

						nmmx.SetSceneBlock(5, 0, 5, 5, 0x00);
						nmmx.SetSceneBlock(5, 0, 6, 5, 0x43);
						nmmx.SetSceneBlock(5, 0, 7, 5, 0x00);
						nmmx.SetSceneBlock(5, 0, 5, 6, 0x52);
						nmmx.SetSceneBlock(5, 0, 6, 6, 0x52);
						nmmx.SetSceneBlock(5, 0, 7, 6, 0x52);

						nmmx.SetSceneBlock(7, 1, 2, 0, 0x00);
						nmmx.SetSceneBlock(7, 1, 3, 0, 0x3A | 0x8000);
						nmmx.SetSceneBlock(7, 1, 4, 0, 0x3A);
						nmmx.SetSceneBlock(7, 1, 2, 1, 0x50);
						nmmx.SetSceneBlock(7, 1, 3, 1, 0x3A | 0x8000 | 0x4000);
						nmmx.SetSceneBlock(7, 1, 4, 1, 0x3A | 0x4000);
						nmmx.SetSceneBlock(7, 1, 1, 3, 0x3F);
						nmmx.SetSceneBlock(7, 1, 2, 3, 0x00);
						nmmx.SetSceneBlock(7, 1, 1, 4, 0x4E);
						nmmx.SetSceneBlock(7, 1, 2, 4, 0x50);

						nmmx.SetSceneBlock(0xA, 1, 0, 4, 0x23 | 0x8000);
						nmmx.SetSceneBlock(0xA, 1, 0, 5, 0x43);

						it = nmmx.eventTable.end();
						it--;
						(*it).xpos = 0x30;
						(*it).ypos = 0x80;
						(*it).eventSubId = 0x19;
						//*LPWORD(nmmx.rom + SNESCore::snes2pc(0xA58000 + 0xC * (*it).eventSubId + 0)) = 0x1;
						//*LPWORD(nmmx.rom + SNESCore::snes2pc(0xA58000 + 0xC * (*it).eventSubId + 6)) = 0xC;
						// add left exit
						nmmx.GetLevelExit((*it).eventSubId).type = 0x3;
						nmmx.GetLevelExit((*it).eventSubId).exitNum = 0x1;
						nmmx.GetLevelExit((*it).eventSubId).cmpValue = 0xC;

						// add entrance
						nmmx.GetLevelEntrance(1).deathState0 = 0x0180;
						nmmx.GetLevelEntrance(1).state1 = 0x0002;
						nmmx.GetLevelEntrance(1).xpos = 0x0EE0;
						nmmx.GetLevelEntrance(1).ypos = 0x0145;
						nmmx.GetLevelEntrance(1).c0_xpos = 0x0E00;
						nmmx.GetLevelEntrance(1).c0_ypos = 0x00D5;
						nmmx.GetLevelEntrance(1).c1_xpos = 0x0599;
						nmmx.GetLevelEntrance(1).c1_ypos = 0x00BA;

						//// direction
						//*LPWORD(nmmx.rom + SNESCore::snes2pc(0x85BE08 + l * 2 + 0)) = 0x8000;
						//*LPWORD(nmmx.rom + SNESCore::snes2pc(0x85C1C0 + l * 2 + 0)) = 0x0002;
						//// Add position
						//*LPWORD(nmmx.rom + SNESCore::snes2pc(0x85BFA0 + l * 4 + 0)) = 0x0EE0;
						//*LPWORD(nmmx.rom + SNESCore::snes2pc(0x85BFA0 + l * 4 + 2)) = 0x0145;
						//// Add camera
						//*LPWORD(nmmx.rom + SNESCore::snes2pc(0x85CCA2 + l * 4 + 0)) = 0x0E00;
						//*LPWORD(nmmx.rom + SNESCore::snes2pc(0x85CCA2 + l * 4 + 2)) = 0x00D5;
						//*LPWORD(nmmx.rom + SNESCore::snes2pc(0x85CEC2 + l * 4 + 0)) = 0x0599;
						//*LPWORD(nmmx.rom + SNESCore::snes2pc(0x85CEC2 + l * 4 + 2)) = 0x00BA;

						// setup initial camera lock
						nmmx.GetLevelEntrance(1).lockBottom = 0x0120;
						//*LPWORD(nmmx.rom + SNESCore::snes2pc(0x85D16A + 2 * l)) = 0x0120;

						nmmx.SortEvents();
						nmmx.SaveEvents();

					}
					else if (l == 14) {
						nmmx.level = l;
						nmmx.LoadLevel();

						// add crumbling block
						auto it = nmmx.eventTable.begin();
						std::advance(it, 0x11);
						e = *it;
						e.xpos = 0x348;
						e.ypos = 0x58;
						nmmx.AddEvent(0x11, e);

						nmmx.SortEvents();
						nmmx.SaveEvents();

						nmmx.SetSceneBlock(1, 0, 1, 2, 0x00);
						nmmx.SetSceneBlock(1, 0, 0, 3, 0x1D);
						nmmx.SetSceneBlock(1, 0, 1, 3, 0x07);
						nmmx.SetSceneBlock(1, 0, 0, 4, 0x00);
					}
					else if (l == 21) {
						nmmx.level = l;
						nmmx.LoadLevel();

						// force platform graphics to not disappear
						*LPBYTE(nmmx.rom + SNESCore::snes2pc(0x83D2DC + 0)) = 0x6B;
						// fix pointer table for sequence
						a = 0x83E45C;
						*LPWORD(nmmx.rom + SNESCore::snes2pc(a++)) = 0xE47C; a++;
						*LPWORD(nmmx.rom + SNESCore::snes2pc(a++)) = 0xE60B; a++;
						*LPWORD(nmmx.rom + SNESCore::snes2pc(a++)) = 0xE61C; a++;
						*LPWORD(nmmx.rom + SNESCore::snes2pc(a++)) = 0xE4BC; a++;
						*LPWORD(nmmx.rom + SNESCore::snes2pc(a++)) = 0xE4F6; a++;
						*LPWORD(nmmx.rom + SNESCore::snes2pc(a++)) = 0xE52F; a++;
						*LPWORD(nmmx.rom + SNESCore::snes2pc(a++)) = 0xE53C; a++;
						*LPWORD(nmmx.rom + SNESCore::snes2pc(a++)) = 0xE562; a++;
						*LPWORD(nmmx.rom + SNESCore::snes2pc(a++)) = 0xE5B8; a++;
						*LPWORD(nmmx.rom + SNESCore::snes2pc(a++)) = 0xE5E9; a++;
						*LPWORD(nmmx.rom + SNESCore::snes2pc(a++)) = 0xE52F; a++;
						*LPWORD(nmmx.rom + SNESCore::snes2pc(a++)) = 0xE53C; a++;
						*LPWORD(nmmx.rom + SNESCore::snes2pc(a++)) = 0xE562; a++;
						*LPWORD(nmmx.rom + SNESCore::snes2pc(a++)) = 0xE5F7; a++;
						*LPWORD(nmmx.rom + SNESCore::snes2pc(a++)) = 0xE654; a++;
						*LPWORD(nmmx.rom + SNESCore::snes2pc(a++)) = 0xE664; a++;
						// rotate clockwise
						*LPWORD(nmmx.rom + SNESCore::snes2pc(0x83E489)) = 0x0100;
						// setup initial rotation/init
						a = 0x83E491;
						*LPBYTE(nmmx.rom + SNESCore::snes2pc(a++)) = 0xDA;
						*LPBYTE(nmmx.rom + SNESCore::snes2pc(a++)) = 0x5A;
						*LPBYTE(nmmx.rom + SNESCore::snes2pc(a++)) = 0x22;
						*LPBYTE(nmmx.rom + SNESCore::snes2pc(a++)) = 0x93;
						*LPBYTE(nmmx.rom + SNESCore::snes2pc(a++)) = 0x8B;
						*LPBYTE(nmmx.rom + SNESCore::snes2pc(a++)) = 0x00;
						*LPBYTE(nmmx.rom + SNESCore::snes2pc(a++)) = 0x4C;
						*LPBYTE(nmmx.rom + SNESCore::snes2pc(a++)) = 0xDD;
						*LPBYTE(nmmx.rom + SNESCore::snes2pc(a++)) = 0xD2; // FIXME

						*LPBYTE(nmmx.rom + SNESCore::snes2pc(0x83E4BD)) = 0x4A;
						a = 0x83E4C0;
						*LPBYTE(nmmx.rom + SNESCore::snes2pc(a++)) = 0x90;
						*LPBYTE(nmmx.rom + SNESCore::snes2pc(a++)) = 0x01;
						*LPBYTE(nmmx.rom + SNESCore::snes2pc(a++)) = 0x90;
						a = 0x83E4CB;
						*LPBYTE(nmmx.rom + SNESCore::snes2pc(a++)) = 0xEA;
						*LPBYTE(nmmx.rom + SNESCore::snes2pc(a++)) = 0xEA;
						*LPBYTE(nmmx.rom + SNESCore::snes2pc(a++)) = 0xEA;
						*LPBYTE(nmmx.rom + SNESCore::snes2pc(a++)) = 0xEA;
						*LPBYTE(nmmx.rom + SNESCore::snes2pc(a++)) = 0xEA;

						// INC
						*LPBYTE(nmmx.rom + SNESCore::snes2pc(0x83E577)) = 0xEE;

						// reset state
						a = 0x83E5FE;
						*LPBYTE(nmmx.rom + SNESCore::snes2pc(a++)) = 0x64;
						*LPBYTE(nmmx.rom + SNESCore::snes2pc(a++)) = 0x80;
						*LPBYTE(nmmx.rom + SNESCore::snes2pc(a++)) = 0x64;
						*LPBYTE(nmmx.rom + SNESCore::snes2pc(a++)) = 0x00;
						*LPBYTE(nmmx.rom + SNESCore::snes2pc(a++)) = 0xEA;
						*LPBYTE(nmmx.rom + SNESCore::snes2pc(a++)) = 0xA9;
						*LPBYTE(nmmx.rom + SNESCore::snes2pc(a++)) = 0x25;
						a = 0x83E60B;
						*LPBYTE(nmmx.rom + SNESCore::snes2pc(a++)) = 0xEA;
						*LPBYTE(nmmx.rom + SNESCore::snes2pc(a++)) = 0xEA;
						*LPBYTE(nmmx.rom + SNESCore::snes2pc(a++)) = 0xEA;
						*LPBYTE(nmmx.rom + SNESCore::snes2pc(a++)) = 0xEA;

						*LPBYTE(nmmx.rom + SNESCore::snes2pc(0x83E64D)) = 0x08;

						// fix init state
						a = 0x83D2DD;
						*LPBYTE(nmmx.rom + SNESCore::snes2pc(a++)) = 0x7A;
						*LPBYTE(nmmx.rom + SNESCore::snes2pc(a++)) = 0xFA;
						*LPBYTE(nmmx.rom + SNESCore::snes2pc(a++)) = 0xF6;
						*LPBYTE(nmmx.rom + SNESCore::snes2pc(a++)) = 0x12;
						*LPBYTE(nmmx.rom + SNESCore::snes2pc(a++)) = 0xA9;
						*LPBYTE(nmmx.rom + SNESCore::snes2pc(a++)) = 0x25;
						*LPBYTE(nmmx.rom + SNESCore::snes2pc(a++)) = 0x00;
						*LPBYTE(nmmx.rom + SNESCore::snes2pc(a++)) = 0x85;
						*LPBYTE(nmmx.rom + SNESCore::snes2pc(a++)) = 0x00;
						*LPBYTE(nmmx.rom + SNESCore::snes2pc(a++)) = 0x1A;
						*LPBYTE(nmmx.rom + SNESCore::snes2pc(a++)) = 0x1A;
						*LPBYTE(nmmx.rom + SNESCore::snes2pc(a++)) = 0x85;
						*LPBYTE(nmmx.rom + SNESCore::snes2pc(a++)) = 0x02;
						*LPBYTE(nmmx.rom + SNESCore::snes2pc(a++)) = 0x4C;
						*LPBYTE(nmmx.rom + SNESCore::snes2pc(a++)) = 0x9A;
						*LPBYTE(nmmx.rom + SNESCore::snes2pc(a++)) = 0xE4;

						nmmx.GetLevelEntrance(0).xpos = 0x0050;
						nmmx.GetLevelEntrance(0).ypos = 0x0080;
						nmmx.GetLevelEntrance(0).c0_xpos = 0x0030;
						nmmx.GetLevelEntrance(0).c0_ypos = 0x0060;

						nmmx.GetLevelEntrance(1).xpos = 0x0050;
						nmmx.GetLevelEntrance(1).ypos = 0x0080;
						nmmx.GetLevelEntrance(1).c0_xpos = 0x0030;
						nmmx.GetLevelEntrance(1).c0_ypos = 0x0060;

						//// Add position
						//*LPWORD(nmmx.rom + SNESCore::snes2pc(0x85BE90 + l * 4 + 0)) = 0x0050;
						//*LPWORD(nmmx.rom + SNESCore::snes2pc(0x85BE90 + l * 4 + 2)) = 0x0080;
						//// Add camera
						//*LPWORD(nmmx.rom + SNESCore::snes2pc(0x85CB92 + l * 4 + 0)) = 0x0030;
						//*LPWORD(nmmx.rom + SNESCore::snes2pc(0x85CB92 + l * 4 + 2)) = 0x0060;
					}
					else if (l == 22) {
						nmmx.level = l;
						nmmx.LoadLevel();

						it = nmmx.eventTable.begin();
						std::advance(it, 1);
						(*it).xpos = 0x7E0;

						it = nmmx.eventTable.end();
						it--;
						(*it).xpos = 0x10;
						(*it).eventSubId = 0x1A;

						// add left exit
						nmmx.GetLevelExit((*it).eventSubId).type = 0x3;
						nmmx.GetLevelExit((*it).eventSubId).exitNum = 0x1;
						nmmx.GetLevelExit((*it).eventSubId).cmpValue = 0xC;

						// add entrance
						nmmx.GetLevelEntrance(1).deathState0 = 0x018A;
						nmmx.GetLevelEntrance(1).state1 = 0x0002;
						nmmx.GetLevelEntrance(1).xpos = 0x07C0;
						nmmx.GetLevelEntrance(1).ypos = 0x00A5;
						nmmx.GetLevelEntrance(1).c0_xpos = 0x0700;
						nmmx.GetLevelEntrance(1).c0_ypos = 0x0000;
						nmmx.GetLevelEntrance(1).c1_xpos = 0x0700;
						nmmx.GetLevelEntrance(1).c1_ypos = 0x0000;

						//// direction
						//*LPWORD(nmmx.rom + SNESCore::snes2pc(0x85BE08 + l * 2 + 0)) = 0x8A00;
						//*LPWORD(nmmx.rom + SNESCore::snes2pc(0x85C1C0 + l * 2 + 0)) = 0x0002;
						//// Add position
						//*LPWORD(nmmx.rom + SNESCore::snes2pc(0x85BFA0 + l * 4 + 0)) = 0x07C0;
						//*LPWORD(nmmx.rom + SNESCore::snes2pc(0x85BFA0 + l * 4 + 2)) = 0x00A5;
						//// Add camera
						//*LPWORD(nmmx.rom + SNESCore::snes2pc(0x85CCA2 + l * 4 + 0)) = 0x0700;
						//*LPWORD(nmmx.rom + SNESCore::snes2pc(0x85CCA2 + l * 4 + 2)) = 0x0000;
						//*LPWORD(nmmx.rom + SNESCore::snes2pc(0x85CEC2 + l * 4 + 0)) = 0x0700;
						//*LPWORD(nmmx.rom + SNESCore::snes2pc(0x85CEC2 + l * 4 + 2)) = 0x0000;

						// reverse falling platforms
						a = 0x82C0C7;
						*LPBYTE(nmmx.rom + SNESCore::snes2pc(a++)) = 0x22;
						*LPBYTE(nmmx.rom + SNESCore::snes2pc(a++)) = 0xB5;
						*LPBYTE(nmmx.rom + SNESCore::snes2pc(a++)) = 0xFF;
						*LPBYTE(nmmx.rom + SNESCore::snes2pc(a++)) = 0x04;
						*LPBYTE(nmmx.rom + SNESCore::snes2pc(a++)) = 0xEA;
						*LPBYTE(nmmx.rom + SNESCore::snes2pc(a++)) = 0xEA;

						a = 0x84FFB5;
						*LPBYTE(nmmx.rom + SNESCore::snes2pc(a++)) = 0xAD;
						*LPBYTE(nmmx.rom + SNESCore::snes2pc(a++)) = 0xD4;
						*LPBYTE(nmmx.rom + SNESCore::snes2pc(a++)) = 0x13;
						*LPBYTE(nmmx.rom + SNESCore::snes2pc(a++)) = 0xF0;
						*LPBYTE(nmmx.rom + SNESCore::snes2pc(a++)) = 0x07;
						*LPBYTE(nmmx.rom + SNESCore::snes2pc(a++)) = 0xB5;
						*LPBYTE(nmmx.rom + SNESCore::snes2pc(a++)) = 0x0A;
						*LPBYTE(nmmx.rom + SNESCore::snes2pc(a++)) = 0x38;
						*LPBYTE(nmmx.rom + SNESCore::snes2pc(a++)) = 0xED;
						*LPBYTE(nmmx.rom + SNESCore::snes2pc(a++)) = 0x4A;
						*LPBYTE(nmmx.rom + SNESCore::snes2pc(a++)) = 0x05;
						*LPBYTE(nmmx.rom + SNESCore::snes2pc(a++)) = 0x6B;
						*LPBYTE(nmmx.rom + SNESCore::snes2pc(a++)) = 0xAD;
						*LPBYTE(nmmx.rom + SNESCore::snes2pc(a++)) = 0x4A;
						*LPBYTE(nmmx.rom + SNESCore::snes2pc(a++)) = 0x05;
						*LPBYTE(nmmx.rom + SNESCore::snes2pc(a++)) = 0x38;
						*LPBYTE(nmmx.rom + SNESCore::snes2pc(a++)) = 0xF5;
						*LPBYTE(nmmx.rom + SNESCore::snes2pc(a++)) = 0x0A;
						*LPBYTE(nmmx.rom + SNESCore::snes2pc(a++)) = 0x6B;

						*LPBYTE(nmmx.rom + SNESCore::snes2pc(0x82C0D0)) = 0x50;

						// reverse skeleton spawn
						a = 0x83D9DF;
						*LPBYTE(nmmx.rom + SNESCore::snes2pc(a++)) = 0xAD;
						*LPBYTE(nmmx.rom + SNESCore::snes2pc(a++)) = 0x4A;
						*LPBYTE(nmmx.rom + SNESCore::snes2pc(a++)) = 0x05;
						*LPBYTE(nmmx.rom + SNESCore::snes2pc(a++)) = 0x38;
						*LPBYTE(nmmx.rom + SNESCore::snes2pc(a++)) = 0xF5;
						*LPBYTE(nmmx.rom + SNESCore::snes2pc(a++)) = 0x0A;

						nmmx.SortEvents();
						nmmx.SaveEvents();
					}
					else if (l == 25) {
						nmmx.level = l;
						nmmx.LoadLevel();

						it = nmmx.eventTable.end();
						it--;
						(*it).xpos = 0x20;
						(*it).ypos = 0x160;
						(*it).eventSubId = 0x1B;

						// add left exit
						nmmx.GetLevelExit((*it).eventSubId).type = 0x3;
						nmmx.GetLevelExit((*it).eventSubId).exitNum = 0x1;
						nmmx.GetLevelExit((*it).eventSubId).cmpValue = 0xC;

						// add entrance
						nmmx.GetLevelEntrance(1).deathState0 = 0x018A;
						nmmx.GetLevelEntrance(1).state1 = 0x0002;
						nmmx.GetLevelEntrance(1).xpos = 0x07CA;
						nmmx.GetLevelEntrance(1).ypos = 0x0085;
						nmmx.GetLevelEntrance(1).c0_xpos = 0x0700;
						nmmx.GetLevelEntrance(1).c0_ypos = 0x0000;
						nmmx.GetLevelEntrance(1).c1_xpos = 0x0700;
						nmmx.GetLevelEntrance(1).c1_ypos = 0x0000;

						//// direction
						//*LPWORD(nmmx.rom + SNESCore::snes2pc(0x85BE08 + l * 2 + 0)) = 0x8A00;
						//*LPWORD(nmmx.rom + SNESCore::snes2pc(0x85C1C0 + l * 2 + 0)) = 0x0002;
						//// Add position
						//*LPWORD(nmmx.rom + SNESCore::snes2pc(0x85BFA0 + l * 4 + 0)) = 0x07CA;
						//*LPWORD(nmmx.rom + SNESCore::snes2pc(0x85BFA0 + l * 4 + 2)) = 0x0085;
						//// Add camera
						//*LPWORD(nmmx.rom + SNESCore::snes2pc(0x85CCA2 + l * 4 + 0)) = 0x0700;
						//*LPWORD(nmmx.rom + SNESCore::snes2pc(0x85CCA2 + l * 4 + 2)) = 0x0000;
						//*LPWORD(nmmx.rom + SNESCore::snes2pc(0x85CEC2 + l * 4 + 0)) = 0x0700;
						//*LPWORD(nmmx.rom + SNESCore::snes2pc(0x85CEC2 + l * 4 + 2)) = 0x0000;

						nmmx.SortEvents();
						nmmx.SaveEvents();
					
						// setup initial camera lock
						nmmx.GetLevelEntrance(1).lockBottom = 0x0000;
						//*LPWORD(nmmx.rom + SNESCore::snes2pc(0x85D16A + 2 * l)) = 0x0000;
					}
					else if (l == 28) {
						// fix secret to be an odd entrance
						nmmx.level = l;
						nmmx.LoadLevel();
						nmmx.GetLevelEntrance(3) = nmmx.GetLevelEntrance(2);
						nmmx.GetLevelEntrance(3).deathState0 &= 0x00FF;
						nmmx.GetLevelEntrance(3).deathState0 |= 0x0100;
						// flip direction
						//nmmx.GetLevelEntrance(3).deathState0 ^= 0x80;

						nmmx.level = 30;
						nmmx.LoadLevel();
						nmmx.GetLevelTransition(1).entranceNum = 3;
						// the secret transitions are hardcoded
						*LPWORD(nmmx.rom + SNESCore::snes2pc(0x86B638)) = 3;
					}
					else if (l == 29) {
						nmmx.level = l;
						nmmx.LoadLevel();

						nmmx.SetSceneBlock(0, 0, 6, 3, 0x07);
						nmmx.SetSceneBlock(0, 0, 6, 4, 0x58);
					}
					else if (l == 36) {
						nmmx.level = l;
						nmmx.LoadLevel();

						nmmx.SetSceneBlock(0, 3, 4, 5, 0x05);
						nmmx.SetSceneBlock(0, 3, 4, 6, 0x31);

						nmmx.SetLevelType(0x7, l);
					}
					else if (l == 39) {
						nmmx.level = l;
						nmmx.LoadLevel();
						if (nmmx.eventTable.size() > 0x17) {
							auto it = nmmx.eventTable.begin();
							std::advance(it, 0x14);
							it->xpos = 0x2B0;

							it = nmmx.eventTable.begin();
							std::advance(it, 0x18);
							it->ypos = 0x40;

							nmmx.SortEvents();
							nmmx.SaveEvents();
						}

						// change blocks
						nmmx.SetSceneBlock(2, 0, 3, 2, 0x0D);
						nmmx.SetSceneBlock(2, 0, 4, 2, 0x55);
						nmmx.SetSceneBlock(2, 0, 5, 2, 0x00);
						nmmx.SetSceneBlock(2, 0, 3, 3, 0x3B);
						nmmx.SetSceneBlock(2, 0, 4, 3, 0x62);
						nmmx.SetSceneBlock(2, 0, 5, 3, 0x00);
						nmmx.SetSceneBlock(2, 0, 6, 2, 0x00);
						nmmx.SetSceneBlock(2, 0, 7, 2, 0x00);
						nmmx.SetSceneBlock(2, 0, 7, 3, 0x67);
					}
					else if (l == 43) {
						nmmx.level = l;
						nmmx.LoadLevel();

						nmmx.SetSceneBlock(5, 0, 7, 3, 0x28);
						nmmx.SetSceneBlock(5, 0, 7, 4, 0x24);
						nmmx.SetSceneBlock(6, 0, 0, 3, 0x00);
						nmmx.SetSceneBlock(6, 0, 0, 4, 0x28);
						nmmx.SetSceneBlock(6, 0, 0, 5, 0x22);
						nmmx.SetSceneBlock(6, 0, 1, 5, 0x2F);
						nmmx.SetSceneBlock(6, 0, 0, 6, 0x22);
						nmmx.SetSceneBlock(6, 0, 1, 6, 0x4D);
					}
					else if (l == 44) {
						// FIXME
					}
					else if (l == 46) {
						nmmx.level = l;
						nmmx.LoadLevel();

						nmmx.SetSceneBlock(0, 0, 5, 4, 0x10);
						nmmx.SetSceneBlock(0, 0, 5, 5, 0x23);
					}
					else if (l == 47) {
						nmmx.level = l;
						nmmx.LoadLevel();
						if (nmmx.eventTable.size() > 0x24) {
							auto it = nmmx.eventTable.begin();
							std::advance(it, 0x1);
							it->ypos = 0x2C;

							nmmx.SortEvents();
							nmmx.SaveEvents();
						}

						// change blocks
						nmmx.SetSceneBlock(0, 0, 0, 0, 0x0B);
						nmmx.SetSceneBlock(0, 0, 1, 0, 0x09);
						nmmx.SetSceneBlock(0, 0, 0, 1, 0x0D);
						nmmx.SetSceneBlock(0, 0, 1, 1, 0x0C);
						nmmx.SetSceneBlock(0, 0, 0, 2, 0x0F);
						nmmx.SetSceneBlock(0, 0, 1, 2, 0x0E);
						nmmx.SetSceneBlock(0, 0, 0, 3, 0x00);
						nmmx.SetSceneBlock(0, 0, 0, 4, 0x3B);
						nmmx.SetSceneBlock(0, 0, 1, 4, 0x3A | 0x8000);
						nmmx.SetSceneBlock(0, 0, 2, 4, 0x2F);
						nmmx.SetSceneBlock(0, 0, 3, 4, 0x00);
						nmmx.SetSceneBlock(0, 0, 1, 5, 0x3D | 0x8000);
						nmmx.SetSceneBlock(0, 0, 2, 5, 0x3C | 0x8000);
						nmmx.SetSceneBlock(0, 0, 3, 5, 0x2F);
					}
					else if (l == 48) {
						nmmx.level = l;
						nmmx.LoadLevel();
						if (nmmx.eventTable.size() > 0x4E) {
							auto it = nmmx.eventTable.begin();
							std::advance(it, 0x19);
							it->xpos = 0x2B0;
							it = nmmx.eventTable.begin();
							std::advance(it, 0x43);
							it->xpos = 0x7AB;

							nmmx.SortEvents();
							nmmx.SaveEvents();
						}

						// change blocks
						nmmx.SetSceneBlock(2, 0, 4, 4, 0x10);
						nmmx.SetSceneBlock(2, 0, 4, 5, 0x23);
					}
					else if (l == 49) {
						nmmx.level = l;
						nmmx.LoadLevel();
						if (nmmx.eventTable.size() > 0xD) {
							auto it = nmmx.eventTable.begin();
							std::advance(it, 0xD);
							it->ypos = 0x48;

							nmmx.SortEvents();
							nmmx.SaveEvents();
						}

						// change blocks
						nmmx.SetSceneBlock(0, 0, 5, 3, 0x06);
						nmmx.SetSceneBlock(0, 0, 6, 3, 0x04);
						nmmx.SetSceneBlock(0, 0, 5, 4, 0x0F);
					}
					else if (l == 50) {
						nmmx.level = l;
						nmmx.LoadLevel();
						if (nmmx.eventTable.size() > 0x19) {
							auto it = nmmx.eventTable.begin();
							std::advance(it, 0xD);
							it->xpos = 0xB0;

							nmmx.SortEvents();
							nmmx.SaveEvents();
						}

						nmmx.SetSceneBlock(2, 0, 3, 3, 0x00);
					}
					else if (l == 51) {
						nmmx.level = l;
						nmmx.LoadLevel();

						nmmx.SetSceneBlock(0, 0, 2, 3, 0x00);
						nmmx.SetSceneBlock(0, 0, 2, 4, 0x2D);
						nmmx.SetSceneBlock(1, 0, 0, 3, 0x2E);
						nmmx.SetSceneBlock(1, 0, 0, 4, 0x2C);
						nmmx.SetSceneBlock(1, 0, 0, 5, 0x3C);

						// fix secret to be an odd entrance
						nmmx.GetLevelEntrance(3) = nmmx.GetLevelEntrance(2);
						nmmx.GetLevelEntrance(3).deathState0 &= 0x00FF;
						nmmx.GetLevelEntrance(3).deathState0 |= 0x0100;
						// flip direction
						//nmmx.GetLevelEntrance(3).deathState0 ^= 0x80;

						nmmx.level = 54;
						nmmx.LoadLevel();
						nmmx.GetLevelTransition(0).entranceNum = 3;
						// the secret transitions are hardcoded
						*LPWORD(nmmx.rom + SNESCore::snes2pc(0x86B620)) = 3;

						// set entrance to 1 to allow events to trigger
						//*LPWORD(nmmx.rom + SNESCore::snes2pc(0xA0C000 + 2 * l)) = 0x1;
						//nmmx.GetLevelTransition(0).entranceNum = 1;
					}
					else if (l == 52 || l == 56) {
						nmmx.SetLevelType(0x8, l);
					}
					else if (l == 55) {
						// FIXME: make gear run backwards
					}
					else if (l == 57) {
						// randomly reverse the gears in 56
						for (auto i : { 56, 58 }) {
							nmmx.level = i;
							nmmx.LoadLevel();
							for (auto &e : nmmx.eventTable) {
								if (e.type == 2 && e.eventId == 0x7C && e.eventSubId == 0x2) {
									if (GetRandom(0, 1.0) < 0.5) {
										e.eventSubId |= 0x1;
									}
								}
							}
							nmmx.SaveEvents();
						}

						// reverse the gear
						a = 0;
						a = 0x86BB79;
						// JMP
						*LPBYTE(nmmx.rom + SNESCore::snes2pc(a++)) = 0x4C;
						*LPBYTE(nmmx.rom + SNESCore::snes2pc(a++)) = 0xD0;
						*LPBYTE(nmmx.rom + SNESCore::snes2pc(a++)) = 0xFF;

						a = 0x86FFD0;
						// LDA D,$14,X (subId)
						*LPBYTE(nmmx.rom + SNESCore::snes2pc(a++)) = 0xB5;
						*LPBYTE(nmmx.rom + SNESCore::snes2pc(a++)) = 0x14;
						// BIT #1
						*LPBYTE(nmmx.rom + SNESCore::snes2pc(a++)) = 0x89;
						*LPBYTE(nmmx.rom + SNESCore::snes2pc(a++)) = 0x01;
						*LPBYTE(nmmx.rom + SNESCore::snes2pc(a++)) = 0x00;
						// BEQ
						*LPBYTE(nmmx.rom + SNESCore::snes2pc(a++)) = 0xD0;
						*LPBYTE(nmmx.rom + SNESCore::snes2pc(a++)) = 0x12;  // FIXME
						// LDA $558
						*LPBYTE(nmmx.rom + SNESCore::snes2pc(a++)) = 0xAD;
						*LPBYTE(nmmx.rom + SNESCore::snes2pc(a++)) = 0x58;
						*LPBYTE(nmmx.rom + SNESCore::snes2pc(a++)) = 0x05;
						// CLC
						*LPBYTE(nmmx.rom + SNESCore::snes2pc(a++)) = 0x18;
						// ADC 34
						*LPBYTE(nmmx.rom + SNESCore::snes2pc(a++)) = 0x75;
						*LPBYTE(nmmx.rom + SNESCore::snes2pc(a++)) = 0x34;
						// STA $558
						*LPBYTE(nmmx.rom + SNESCore::snes2pc(a++)) = 0x8D;
						*LPBYTE(nmmx.rom + SNESCore::snes2pc(a++)) = 0x58;
						*LPBYTE(nmmx.rom + SNESCore::snes2pc(a++)) = 0x05;
						// LDA $55A
						*LPBYTE(nmmx.rom + SNESCore::snes2pc(a++)) = 0xAD;
						*LPBYTE(nmmx.rom + SNESCore::snes2pc(a++)) = 0x5A;
						*LPBYTE(nmmx.rom + SNESCore::snes2pc(a++)) = 0x05;
						// ADC $36
						*LPBYTE(nmmx.rom + SNESCore::snes2pc(a++)) = 0x75;
						*LPBYTE(nmmx.rom + SNESCore::snes2pc(a++)) = 0x36;
						// STA $55A
						*LPBYTE(nmmx.rom + SNESCore::snes2pc(a++)) = 0x8D;
						*LPBYTE(nmmx.rom + SNESCore::snes2pc(a++)) = 0x5A;
						*LPBYTE(nmmx.rom + SNESCore::snes2pc(a++)) = 0x05;
						// RTL
						*LPBYTE(nmmx.rom + SNESCore::snes2pc(a++)) = 0x6B;
						// LDA $558
						*LPBYTE(nmmx.rom + SNESCore::snes2pc(a++)) = 0xAD;
						*LPBYTE(nmmx.rom + SNESCore::snes2pc(a++)) = 0x58;
						*LPBYTE(nmmx.rom + SNESCore::snes2pc(a++)) = 0x05;
						// SEC
						*LPBYTE(nmmx.rom + SNESCore::snes2pc(a++)) = 0x38;
						// SBC 34
						*LPBYTE(nmmx.rom + SNESCore::snes2pc(a++)) = 0xF5;
						*LPBYTE(nmmx.rom + SNESCore::snes2pc(a++)) = 0x34;
						// STA $558
						*LPBYTE(nmmx.rom + SNESCore::snes2pc(a++)) = 0x8D;
						*LPBYTE(nmmx.rom + SNESCore::snes2pc(a++)) = 0x58;
						*LPBYTE(nmmx.rom + SNESCore::snes2pc(a++)) = 0x05;
						// LDA $55A
						*LPBYTE(nmmx.rom + SNESCore::snes2pc(a++)) = 0xAD;
						*LPBYTE(nmmx.rom + SNESCore::snes2pc(a++)) = 0x5A;
						*LPBYTE(nmmx.rom + SNESCore::snes2pc(a++)) = 0x05;
						// SBC $36
						*LPBYTE(nmmx.rom + SNESCore::snes2pc(a++)) = 0xF5;
						*LPBYTE(nmmx.rom + SNESCore::snes2pc(a++)) = 0x36;
						// STA $55A
						*LPBYTE(nmmx.rom + SNESCore::snes2pc(a++)) = 0x8D;
						*LPBYTE(nmmx.rom + SNESCore::snes2pc(a++)) = 0x5A;
						*LPBYTE(nmmx.rom + SNESCore::snes2pc(a++)) = 0x05;
						// RTL
						*LPBYTE(nmmx.rom + SNESCore::snes2pc(a++)) = 0x6B;

						// HACK: put the visual reversal in the spot where the physical reversal used to be
						// JSR
						a = 0x86BC04;
						*LPBYTE(nmmx.rom + SNESCore::snes2pc(a++)) = 0x20;
						*LPBYTE(nmmx.rom + SNESCore::snes2pc(a++)) = 0x7C;
						*LPBYTE(nmmx.rom + SNESCore::snes2pc(a++)) = 0xBB;

						a = 0x86BB7C;
						// PHX
						*LPBYTE(nmmx.rom + SNESCore::snes2pc(a++)) = 0xDA;
						// LDX D,$14,Y (subId)
						*LPBYTE(nmmx.rom + SNESCore::snes2pc(a++)) = 0xB6;
						*LPBYTE(nmmx.rom + SNESCore::snes2pc(a++)) = 0x14;
						// TXA
						*LPBYTE(nmmx.rom + SNESCore::snes2pc(a++)) = 0x8A;
						// PLX
						*LPBYTE(nmmx.rom + SNESCore::snes2pc(a++)) = 0xFA;
						// LSR
						*LPBYTE(nmmx.rom + SNESCore::snes2pc(a++)) = 0x4A;
						// LDA D,$3E,X
						*LPBYTE(nmmx.rom + SNESCore::snes2pc(a++)) = 0xB5;
						*LPBYTE(nmmx.rom + SNESCore::snes2pc(a++)) = 0x3E;
						// BCS
						*LPBYTE(nmmx.rom + SNESCore::snes2pc(a++)) = 0xB0;
						*LPBYTE(nmmx.rom + SNESCore::snes2pc(a++)) = 0x02; // FIXME
						// INC
						*LPBYTE(nmmx.rom + SNESCore::snes2pc(a++)) = 0x1A;
						// RTS
						*LPBYTE(nmmx.rom + SNESCore::snes2pc(a++)) = 0x60;
						// DEC
						*LPBYTE(nmmx.rom + SNESCore::snes2pc(a++)) = 0x3A;
						// RTS
						*LPBYTE(nmmx.rom + SNESCore::snes2pc(a++)) = 0x60;

						//// fix up all the branches to point to a different RTL
						//*LPBYTE(nmmx.rom + SNESCore::snes2pc(0x86BB54)) += 0x7;
						//*LPBYTE(nmmx.rom + SNESCore::snes2pc(0x86BB96)) += 0x7;
						//*LPBYTE(nmmx.rom + SNESCore::snes2pc(0x86BB9E)) += 0x7;
						//*LPBYTE(nmmx.rom + SNESCore::snes2pc(0x86BBA6)) += 0x7;
						//*LPBYTE(nmmx.rom + SNESCore::snes2pc(0x86BBAF)) += 0x7;
						//*LPBYTE(nmmx.rom + SNESCore::snes2pc(0x86BBBD)) += 0x7;

					}
					else if (l == 60) {
						nmmx.level = l;
						nmmx.LoadLevel();

						if (nmmx.eventTable.size() > 0x7) {
							auto it = nmmx.eventTable.begin();
							std::advance(it, 0x7);
							it->xpos = 0x70;

							nmmx.SortEvents();
							nmmx.SaveEvents();
						}

						nmmx.SetSceneBlock(0, 1, 4, 0, 0x1F);
						nmmx.SetSceneBlock(0, 1, 4, 1, 0xA0);

						nmmx.SetLevelType(0x7, l);
					}
				}
			}
			reverse = reverseTemp;
		}
		else {
			reverse.clear();
		}

		// remove 21 since it always looks like it's in the forward direction
		auto it21 = std::find(reverse.begin(), reverse.end(), 21);
		if (it21 != reverse.end()) reverse.erase(it21);

		// write array for level linkage
		//memset(nmmx.rom + SNESCore::snes2pc(0xA0C000), 0xFF, sizeof(WORD) * nmmx.numLevels);
		WORD prevLevel= 0;
		for (auto &g : levelGroups) {
			for (auto &l : g) {
				if (prevLevel != l) {
					// determine if the level should be flipped
					bool prevReverse = find(reverse.begin(), reverse.end(), prevLevel) != reverse.end();
					bool currReverse = find(reverse.begin(), reverse.end(), l) != reverse.end();
					//*LPWORD(nmmx.rom + SNESCore::snes2pc(0xA0C000 + (prevReverse ? 0x100 : 0x000) + 2 * prevLevel)) = l;
					//*LPWORD(nmmx.rom + SNESCore::snes2pc(0xA0C000 + (prevReverse ? 0x300 : 0x200) + 2 * prevLevel)) = currReverse ? 0x1 : 0x0;
					nmmx.GetLevelTransition(prevReverse ? 1 : 0, prevLevel).nextLevel = l;
					nmmx.GetLevelTransition(prevReverse ? 1 : 0, prevLevel).entranceNum = currReverse ? 0x1 : 0x0;
				}
				prevLevel = l;
			}
		}

		//// fix LDA of 1 to allow entrance->anywhere
		//a = 0x8CFF8C;
		//// LDA #6
		//*LPBYTE(nmmx.rom + SNESCore::snes2pc(a++)) = 0xA9;
		//*LPBYTE(nmmx.rom + SNESCore::snes2pc(a++)) = 0x06;
		//*LPBYTE(nmmx.rom + SNESCore::snes2pc(a++)) = 0x00;
		//// JSL FFC0
		//*LPBYTE(nmmx.rom + SNESCore::snes2pc(a++)) = 0x22;
		//*LPBYTE(nmmx.rom + SNESCore::snes2pc(a++)) = 0xC0;
		//*LPBYTE(nmmx.rom + SNESCore::snes2pc(a++)) = 0xFF;
		//*LPBYTE(nmmx.rom + SNESCore::snes2pc(a++)) = 0x82;
		//// NOPs
		//*LPBYTE(nmmx.rom + SNESCore::snes2pc(a++)) = 0xEA;
		//*LPBYTE(nmmx.rom + SNESCore::snes2pc(a++)) = 0xEA;
		//*LPBYTE(nmmx.rom + SNESCore::snes2pc(a++)) = 0xEA;

		//*LPWORD(nmmx.rom + SNESCore::snes2pc(0x8CFF8D)) = levelGroups[1][0];

		prevLevel = 0;
		std::reverse(levelGroups.begin(), levelGroups.end());
		for (auto &g : levelGroups) {
			std::reverse(g.begin(), g.end());
			for (auto &l : g) {
				if (prevLevel != 0) {
					// determine if the level should be flipped
					bool prevReverse = find(reverse.begin(), reverse.end(), prevLevel) != reverse.end();
					bool currReverse = find(reverse.begin(), reverse.end(), l) != reverse.end();

					//*LPWORD(nmmx.rom + SNESCore::snes2pc(0xA0C000 + (prevReverse ? 0x000 : 0x100) + 2 * prevLevel)) = l;
					//*LPWORD(nmmx.rom + SNESCore::snes2pc(0xA0C000 + (prevReverse ? 0x200 : 0x300) + 2 * prevLevel)) = currReverse ? 0x0 : 0x1;
					nmmx.GetLevelTransition(prevReverse ? 0 : 1, prevLevel).nextLevel = l;
					nmmx.GetLevelTransition(prevReverse ? 0 : 1, prevLevel).entranceNum = currReverse ? 0x0 : 0x1;
				}
				prevLevel = l;
			}
		}

		std::vector<WORD> foundLevel;
		for (unsigned l = 0; l < nmmx.numLevels; l++) {
			foundLevel.push_back(l);
		}

		for (unsigned l = 0, lTest= 0; l < nmmx.numLevels && lTest != nmmx.numLevels; l++) {
			bool levelNone = find(none.begin(), none.end(), lTest) != none.end();
			bool levelLeft = find(left.begin(), left.end(), lTest) != left.end();
			bool levelLeftRight = find(leftRight.begin(), leftRight.end(), lTest) != leftRight.end();
			bool levelRight = find(right.begin(), right.end(), lTest) != right.end();
			bool levelReverse = find(reverse.begin(), reverse.end(), lTest) != reverse.end();
			bool levelReverseAllowed = find(reverseAllowed.begin(), reverseAllowed.end(), lTest) != reverseAllowed.end();

			unsigned nextLevel = nmmx.GetLevelTransition(levelReverse ? 1 : 0, lTest).nextLevel;
			unsigned nextLevelEntrance = nmmx.GetLevelTransition(levelReverse ? 1 : 0, lTest).entranceNum;

			bool nextLevelNone = find(none.begin(), none.end(), nextLevel) != none.end();
			bool nextLevelLeft = find(left.begin(), left.end(), nextLevel) != left.end();
			bool nextLevelLeftRight = find(leftRight.begin(), leftRight.end(), nextLevel) != leftRight.end();
			bool nextLevelRight = find(right.begin(), right.end(), nextLevel) != right.end();
			bool nextLevelReverse = find(reverse.begin(), reverse.end(), nextLevel) != reverse.end();
			bool nextLevelReverseAllowed = find(reverseAllowed.begin(), reverseAllowed.end(), nextLevel) != reverseAllowed.end();

			if ( nextLevelEntrance == 1 && nextLevelNone && !nextLevelReverse) {
				error = true;
			}
			if (nextLevelEntrance == 1 && nextLevelNone && !nextLevelReverseAllowed) {
				error = true;
			}
			if (levelReverse && !levelReverseAllowed) {
				error = true;
			}
			// don't include 2 secrets
			if (nextLevel >= nmmx.numLevels && l < nmmx.numLevels - 3) {
				error = true;
			}
			auto foundIt = find(foundLevel.begin(), foundLevel.end(), lTest);
			if (foundIt == foundLevel.end()) {
				error = true;
			}
			else {
				foundLevel.erase(foundIt);
			}

			lTest = nmmx.GetLevelTransition(levelReverse ? 1 : 0, lTest).nextLevel;
		}

		//// secret 30.  54 should be automatic through special event
		//*LPWORD(nmmx.rom + SNESCore::snes2pc(0xA0C100 + 2 * 30)) = 28;

		// fix checkpoints to be per level.  could be done to left instead to make it harder
		for (unsigned i = 0; i < nmmx.numLevels; ++i) {
			*LPBYTE(nmmx.rom + SNESCore::snes2pc(nmmx.region == 0 ? 0x81B395 : 0x81B369) + i) = i == 0x43 ? 0x42 : i;
		}

		// fix continues to be per level.  could be done to start of group (or randomly to left) to make it harder.
		for (unsigned i = 0; i < nmmx.numLevels; ++i) {
			*LPBYTE(nmmx.rom + SNESCore::snes2pc(nmmx.region == 0 ? 0x81FBAC : 0x81FBAC) + i) = i == 0x43 ? 0x42 : i;
		}

		//*LPWORD(nmmx.rom + SNESCore::snes2pc(0x82BB13)) = 0xBB33; // forward // FIXME
		//*LPWORD(nmmx.rom + SNESCore::snes2pc(0x82BB15)) = 0xBB1B; // backward

		//// write new level sequencing code (also needs to take care of 28/30)
		//a = 0x82BB1B;
		//// Backward
		////a = 0x82BB2F;
		//// PHX
		//*LPBYTE(nmmx.rom + SNESCore::snes2pc(a++)) = 0xDA;
		//// LDA $86
		//*LPBYTE(nmmx.rom + SNESCore::snes2pc(a++)) = 0xA5;
		//*LPBYTE(nmmx.rom + SNESCore::snes2pc(a++)) = 0x86;
		//// ASL
		//*LPBYTE(nmmx.rom + SNESCore::snes2pc(a++)) = 0x0A;
		//// TAX
		//*LPBYTE(nmmx.rom + SNESCore::snes2pc(a++)) = 0xAA;
		//// LDA long, X
		//*LPBYTE(nmmx.rom + SNESCore::snes2pc(a++)) = 0xBF;
		//*LPBYTE(nmmx.rom + SNESCore::snes2pc(a++)) = 0x00;
		//*LPBYTE(nmmx.rom + SNESCore::snes2pc(a++)) = 0xC1;
		//*LPBYTE(nmmx.rom + SNESCore::snes2pc(a++)) = 0xA0;
		//// STA $86
		//*LPBYTE(nmmx.rom + SNESCore::snes2pc(a++)) = 0x85;
		//*LPBYTE(nmmx.rom + SNESCore::snes2pc(a++)) = 0x86;
		//// SEC
		//*LPBYTE(nmmx.rom + SNESCore::snes2pc(a++)) = 0x38;
		//// TXA
		//*LPBYTE(nmmx.rom + SNESCore::snes2pc(a++)) = 0x8A;
		//// SBC $1E * 2
		//*LPBYTE(nmmx.rom + SNESCore::snes2pc(a++)) = 0xE9;
		//*LPBYTE(nmmx.rom + SNESCore::snes2pc(a++)) = 0x3C;
		//*LPBYTE(nmmx.rom + SNESCore::snes2pc(a++)) = 0x00;
		//// BEQ
		//*LPBYTE(nmmx.rom + SNESCore::snes2pc(a++)) = 0xF0;
		//*LPBYTE(nmmx.rom + SNESCore::snes2pc(a++)) = 0x15; // FIXME
		//// LDA long, X
		//*LPBYTE(nmmx.rom + SNESCore::snes2pc(a++)) = 0xBF;
		//*LPBYTE(nmmx.rom + SNESCore::snes2pc(a++)) = 0x00;
		//*LPBYTE(nmmx.rom + SNESCore::snes2pc(a++)) = 0xC3;
		//*LPBYTE(nmmx.rom + SNESCore::snes2pc(a++)) = 0xA0;
		//// BRA
		//*LPBYTE(nmmx.rom + SNESCore::snes2pc(a++)) = 0x80;
		//*LPBYTE(nmmx.rom + SNESCore::snes2pc(a++)) = 0x0F; // FIXME

		//// Forward
		//// PHX
		//*LPBYTE(nmmx.rom + SNESCore::snes2pc(a++)) = 0xDA;
		//// LDA $86
		//*LPBYTE(nmmx.rom + SNESCore::snes2pc(a++)) = 0xA5;
		//*LPBYTE(nmmx.rom + SNESCore::snes2pc(a++)) = 0x86;
		//// ASL
		//*LPBYTE(nmmx.rom + SNESCore::snes2pc(a++)) = 0x0A;
		//// TAX
		//*LPBYTE(nmmx.rom + SNESCore::snes2pc(a++)) = 0xAA;
		//// LDA long, X
		//*LPBYTE(nmmx.rom + SNESCore::snes2pc(a++)) = 0xBF;
		//*LPBYTE(nmmx.rom + SNESCore::snes2pc(a++)) = 0x00;
		//*LPBYTE(nmmx.rom + SNESCore::snes2pc(a++)) = 0xC0;
		//*LPBYTE(nmmx.rom + SNESCore::snes2pc(a++)) = 0xA0;
		//// STA $86
		//*LPBYTE(nmmx.rom + SNESCore::snes2pc(a++)) = 0x85;
		//*LPBYTE(nmmx.rom + SNESCore::snes2pc(a++)) = 0x86;
		//// LDA long, X
		//*LPBYTE(nmmx.rom + SNESCore::snes2pc(a++)) = 0xBF;
		//*LPBYTE(nmmx.rom + SNESCore::snes2pc(a++)) = 0x00;
		//*LPBYTE(nmmx.rom + SNESCore::snes2pc(a++)) = 0xC2;
		//*LPBYTE(nmmx.rom + SNESCore::snes2pc(a++)) = 0xA0;

		//// PLX
		//*LPBYTE(nmmx.rom + SNESCore::snes2pc(a++)) = 0xFA;
		//// STA $13D4
		//*LPBYTE(nmmx.rom + SNESCore::snes2pc(a++)) = 0x8D;
		//*LPBYTE(nmmx.rom + SNESCore::snes2pc(a++)) = 0xD4;
		//*LPBYTE(nmmx.rom + SNESCore::snes2pc(a++)) = 0x13;
		//// LDA #06
		//*LPBYTE(nmmx.rom + SNESCore::snes2pc(a++)) = 0xA9;
		//*LPBYTE(nmmx.rom + SNESCore::snes2pc(a++)) = 0x06;
		//*LPBYTE(nmmx.rom + SNESCore::snes2pc(a++)) = 0x00;
		//// STA $70
		//*LPBYTE(nmmx.rom + SNESCore::snes2pc(a++)) = 0x85;
		//*LPBYTE(nmmx.rom + SNESCore::snes2pc(a++)) = 0x70;
		//// RTL
		//*LPBYTE(nmmx.rom + SNESCore::snes2pc(a++)) = 0x6B;

		//// CutScene
		//// remove increment
		//a = 0x82BB5D;
		//*LPBYTE(nmmx.rom + SNESCore::snes2pc(a++)) = 0xEA;
		//*LPBYTE(nmmx.rom + SNESCore::snes2pc(a++)) = 0xEA;
		//// jmp to load
		//a = 0x82BB69;
		//*LPBYTE(nmmx.rom + SNESCore::snes2pc(a++)) = 0x4C;
		//*LPBYTE(nmmx.rom + SNESCore::snes2pc(a++)) = 0xC0;
		//*LPBYTE(nmmx.rom + SNESCore::snes2pc(a++)) = 0xFF;

		//a = 0x82FFC0;
		//// STA D, $70
		//*LPBYTE(nmmx.rom + SNESCore::snes2pc(a++)) = 0x85;
		//*LPBYTE(nmmx.rom + SNESCore::snes2pc(a++)) = 0x70;
		//// PHX
		//*LPBYTE(nmmx.rom + SNESCore::snes2pc(a++)) = 0xDA;
		//// LDA $86
		//*LPBYTE(nmmx.rom + SNESCore::snes2pc(a++)) = 0xA5;
		//*LPBYTE(nmmx.rom + SNESCore::snes2pc(a++)) = 0x86;
		//// ASL
		//*LPBYTE(nmmx.rom + SNESCore::snes2pc(a++)) = 0x0A;
		//// TAX
		//*LPBYTE(nmmx.rom + SNESCore::snes2pc(a++)) = 0xAA;
		//// LDA long, X
		//*LPBYTE(nmmx.rom + SNESCore::snes2pc(a++)) = 0xBF;
		//*LPBYTE(nmmx.rom + SNESCore::snes2pc(a++)) = 0x00;
		//*LPBYTE(nmmx.rom + SNESCore::snes2pc(a++)) = 0xC0;
		//*LPBYTE(nmmx.rom + SNESCore::snes2pc(a++)) = 0xA0;
		//// STA $86
		//*LPBYTE(nmmx.rom + SNESCore::snes2pc(a++)) = 0x85;
		//*LPBYTE(nmmx.rom + SNESCore::snes2pc(a++)) = 0x86;
		//// LDA long, X
		//*LPBYTE(nmmx.rom + SNESCore::snes2pc(a++)) = 0xBF;
		//*LPBYTE(nmmx.rom + SNESCore::snes2pc(a++)) = 0x00;
		//*LPBYTE(nmmx.rom + SNESCore::snes2pc(a++)) = 0xC2;
		//*LPBYTE(nmmx.rom + SNESCore::snes2pc(a++)) = 0xA0;
		//// STA $13D4
		//*LPBYTE(nmmx.rom + SNESCore::snes2pc(a++)) = 0x8D;
		//*LPBYTE(nmmx.rom + SNESCore::snes2pc(a++)) = 0xD4;
		//*LPBYTE(nmmx.rom + SNESCore::snes2pc(a++)) = 0x13;
		//// PLX
		//*LPBYTE(nmmx.rom + SNESCore::snes2pc(a++)) = 0xFA;
		//// RTL
		//*LPBYTE(nmmx.rom + SNESCore::snes2pc(a++)) = 0x6B;

		//// rotating level
		//// tweak branch to go to next RTL
		//a = 0x83E65B;
		//*LPBYTE(nmmx.rom + SNESCore::snes2pc(a++)) += 1;
		//// jmp to load
		//a = 0x83E661;
		//*LPBYTE(nmmx.rom + SNESCore::snes2pc(a++)) = 0x4C;
		//*LPBYTE(nmmx.rom + SNESCore::snes2pc(a++)) = 0xD8;
		//*LPBYTE(nmmx.rom + SNESCore::snes2pc(a++)) = 0xFF;

		//a = 0x83FFD8;
		//// PHX
		//*LPBYTE(nmmx.rom + SNESCore::snes2pc(a++)) = 0xDA;
		//// LDA $86
		//*LPBYTE(nmmx.rom + SNESCore::snes2pc(a++)) = 0xA5;
		//*LPBYTE(nmmx.rom + SNESCore::snes2pc(a++)) = 0x86;
		//// ASL
		//*LPBYTE(nmmx.rom + SNESCore::snes2pc(a++)) = 0x0A;
		//// TAX
		//*LPBYTE(nmmx.rom + SNESCore::snes2pc(a++)) = 0xAA;
		//// LDA long, X
		//*LPBYTE(nmmx.rom + SNESCore::snes2pc(a++)) = 0xBF;
		//*LPBYTE(nmmx.rom + SNESCore::snes2pc(a++)) = 0x00;
		//*LPBYTE(nmmx.rom + SNESCore::snes2pc(a++)) = 0xC0;
		//*LPBYTE(nmmx.rom + SNESCore::snes2pc(a++)) = 0xA0;
		//// STA $86
		//*LPBYTE(nmmx.rom + SNESCore::snes2pc(a++)) = 0x85;
		//*LPBYTE(nmmx.rom + SNESCore::snes2pc(a++)) = 0x86;
		//// LDA long, X
		//*LPBYTE(nmmx.rom + SNESCore::snes2pc(a++)) = 0xBF;
		//*LPBYTE(nmmx.rom + SNESCore::snes2pc(a++)) = 0x00;
		//*LPBYTE(nmmx.rom + SNESCore::snes2pc(a++)) = 0xC2;
		//*LPBYTE(nmmx.rom + SNESCore::snes2pc(a++)) = 0xA0;
		//// STA $13D4
		//*LPBYTE(nmmx.rom + SNESCore::snes2pc(a++)) = 0x8D;
		//*LPBYTE(nmmx.rom + SNESCore::snes2pc(a++)) = 0xD4;
		//*LPBYTE(nmmx.rom + SNESCore::snes2pc(a++)) = 0x13;
		//// PLX
		//*LPBYTE(nmmx.rom + SNESCore::snes2pc(a++)) = 0xFA;
		//// RTL
		//*LPBYTE(nmmx.rom + SNESCore::snes2pc(a++)) = 0x6B;

		//// other CutScene
		//// CutScene
		//a = 0x85FF38;
		//*LPBYTE(nmmx.rom + SNESCore::snes2pc(a++)) = 0x4C;
		//*LPBYTE(nmmx.rom + SNESCore::snes2pc(a++)) = 0xC0;
		//*LPBYTE(nmmx.rom + SNESCore::snes2pc(a++)) = 0xFF;

		//a = 0x85FF61;
		//*LPBYTE(nmmx.rom + SNESCore::snes2pc(a++)) = 0xEE;
		//*LPBYTE(nmmx.rom + SNESCore::snes2pc(a++)) = 0xE8;
		//*LPBYTE(nmmx.rom + SNESCore::snes2pc(a++)) = 0x13;
		//*LPBYTE(nmmx.rom + SNESCore::snes2pc(a++)) = 0x4C;
		//*LPBYTE(nmmx.rom + SNESCore::snes2pc(a++)) = 0xC5;
		//*LPBYTE(nmmx.rom + SNESCore::snes2pc(a++)) = 0xFF;

		//a = 0x85FFC0;
		//// LDA #03
		//*LPBYTE(nmmx.rom + SNESCore::snes2pc(a++)) = 0xA9;
		//*LPBYTE(nmmx.rom + SNESCore::snes2pc(a++)) = 0x03;
		//*LPBYTE(nmmx.rom + SNESCore::snes2pc(a++)) = 0x00;
		//// STA D, $70
		//*LPBYTE(nmmx.rom + SNESCore::snes2pc(a++)) = 0x85;
		//*LPBYTE(nmmx.rom + SNESCore::snes2pc(a++)) = 0x70;
		//// PHX
		//*LPBYTE(nmmx.rom + SNESCore::snes2pc(a++)) = 0xDA;
		//// LDA $86
		//*LPBYTE(nmmx.rom + SNESCore::snes2pc(a++)) = 0xA5;
		//*LPBYTE(nmmx.rom + SNESCore::snes2pc(a++)) = 0x86;
		//// ASL
		//*LPBYTE(nmmx.rom + SNESCore::snes2pc(a++)) = 0x0A;
		//// TAX
		//*LPBYTE(nmmx.rom + SNESCore::snes2pc(a++)) = 0xAA;
		//// LDA long, X
		//*LPBYTE(nmmx.rom + SNESCore::snes2pc(a++)) = 0xBF;
		//*LPBYTE(nmmx.rom + SNESCore::snes2pc(a++)) = 0x00;
		//*LPBYTE(nmmx.rom + SNESCore::snes2pc(a++)) = 0xC0;
		//*LPBYTE(nmmx.rom + SNESCore::snes2pc(a++)) = 0xA0;
		//// STA $86
		//*LPBYTE(nmmx.rom + SNESCore::snes2pc(a++)) = 0x85;
		//*LPBYTE(nmmx.rom + SNESCore::snes2pc(a++)) = 0x86;
		//// LDA long, X
		//*LPBYTE(nmmx.rom + SNESCore::snes2pc(a++)) = 0xBF;
		//*LPBYTE(nmmx.rom + SNESCore::snes2pc(a++)) = 0x00;
		//*LPBYTE(nmmx.rom + SNESCore::snes2pc(a++)) = 0xC2;
		//*LPBYTE(nmmx.rom + SNESCore::snes2pc(a++)) = 0xA0;
		//// STA $13D4
		//*LPBYTE(nmmx.rom + SNESCore::snes2pc(a++)) = 0x8D;
		//*LPBYTE(nmmx.rom + SNESCore::snes2pc(a++)) = 0xD4;
		//*LPBYTE(nmmx.rom + SNESCore::snes2pc(a++)) = 0x13;
		//// PLX
		//*LPBYTE(nmmx.rom + SNESCore::snes2pc(a++)) = 0xFA;
		//// RTL
		//*LPBYTE(nmmx.rom + SNESCore::snes2pc(a++)) = 0x6B;

		//// Medusa
		//a = 0x86C4A7;
		//// shift up STZ and LDA
		//*LPBYTE(nmmx.rom + SNESCore::snes2pc(a)) = *LPBYTE(nmmx.rom + SNESCore::snes2pc(a + 2)); a++;
		//*LPBYTE(nmmx.rom + SNESCore::snes2pc(a)) = *LPBYTE(nmmx.rom + SNESCore::snes2pc(a + 2)); a++;
		//*LPBYTE(nmmx.rom + SNESCore::snes2pc(a)) = *LPBYTE(nmmx.rom + SNESCore::snes2pc(a + 2)); a++;
		//*LPBYTE(nmmx.rom + SNESCore::snes2pc(a)) = *LPBYTE(nmmx.rom + SNESCore::snes2pc(a + 2)); a++;
		//*LPBYTE(nmmx.rom + SNESCore::snes2pc(a)) = *LPBYTE(nmmx.rom + SNESCore::snes2pc(a + 2)); a++;
		//*LPBYTE(nmmx.rom + SNESCore::snes2pc(a)) = *LPBYTE(nmmx.rom + SNESCore::snes2pc(a + 2)); a++;
		//// long jump to $82 handler
		//*LPBYTE(nmmx.rom + SNESCore::snes2pc(a++)) = 0x5C;
		//*LPBYTE(nmmx.rom + SNESCore::snes2pc(a++)) = 0xC0;
		//*LPBYTE(nmmx.rom + SNESCore::snes2pc(a++)) = 0xFF;
		//*LPBYTE(nmmx.rom + SNESCore::snes2pc(a++)) = 0x82;

		//// Knight
		//a = 0x84E80E;
		//// shift up STZ and LDA
		//*LPBYTE(nmmx.rom + SNESCore::snes2pc(a)) = *LPBYTE(nmmx.rom + SNESCore::snes2pc(a + 7)); a++;
		//*LPBYTE(nmmx.rom + SNESCore::snes2pc(a)) = *LPBYTE(nmmx.rom + SNESCore::snes2pc(a + 7)); a++;
		//*LPBYTE(nmmx.rom + SNESCore::snes2pc(a)) = *LPBYTE(nmmx.rom + SNESCore::snes2pc(a + 7)); a++;
		//*LPBYTE(nmmx.rom + SNESCore::snes2pc(a)) = *LPBYTE(nmmx.rom + SNESCore::snes2pc(a + 7)); a++;
		//*LPBYTE(nmmx.rom + SNESCore::snes2pc(a)) = *LPBYTE(nmmx.rom + SNESCore::snes2pc(a + 7)); a++;
		//*LPBYTE(nmmx.rom + SNESCore::snes2pc(a)) = *LPBYTE(nmmx.rom + SNESCore::snes2pc(a + 7)); a++;
		//*LPBYTE(nmmx.rom + SNESCore::snes2pc(a)) = *LPBYTE(nmmx.rom + SNESCore::snes2pc(a + 7)); a++;
		//*LPBYTE(nmmx.rom + SNESCore::snes2pc(a)) = *LPBYTE(nmmx.rom + SNESCore::snes2pc(a + 7)); a++;
		//*LPBYTE(nmmx.rom + SNESCore::snes2pc(a)) = *LPBYTE(nmmx.rom + SNESCore::snes2pc(a + 7)); a++;
		//// LDA #B
		//*LPBYTE(nmmx.rom + SNESCore::snes2pc(a++)) = 0xA9;
		//*LPBYTE(nmmx.rom + SNESCore::snes2pc(a++)) = 0x0B;
		//*LPBYTE(nmmx.rom + SNESCore::snes2pc(a++)) = 0x00;
		//// long jump to $82 handler
		//*LPBYTE(nmmx.rom + SNESCore::snes2pc(a++)) = 0x5C;
		//*LPBYTE(nmmx.rom + SNESCore::snes2pc(a++)) = 0xC0;
		//*LPBYTE(nmmx.rom + SNESCore::snes2pc(a++)) = 0xFF;
		//*LPBYTE(nmmx.rom + SNESCore::snes2pc(a++)) = 0x82;

		//// level 1
		//a = 0x83E31A;
		//// shift up LDA
		//*LPBYTE(nmmx.rom + SNESCore::snes2pc(a)) = *LPBYTE(nmmx.rom + SNESCore::snes2pc(a + 2)); a++;
		//*LPBYTE(nmmx.rom + SNESCore::snes2pc(a)) = *LPBYTE(nmmx.rom + SNESCore::snes2pc(a + 2)); a++;
		//*LPBYTE(nmmx.rom + SNESCore::snes2pc(a)) = *LPBYTE(nmmx.rom + SNESCore::snes2pc(a + 2)); a++;
		//// long jump to $82 handler
		//*LPBYTE(nmmx.rom + SNESCore::snes2pc(a++)) = 0x5C;
		//*LPBYTE(nmmx.rom + SNESCore::snes2pc(a++)) = 0xC0;
		//*LPBYTE(nmmx.rom + SNESCore::snes2pc(a++)) = 0xFF;
		//*LPBYTE(nmmx.rom + SNESCore::snes2pc(a++)) = 0x82;

		//// powexil
		//a = 0x86C51E;
		//// shift up LDA
		//*LPBYTE(nmmx.rom + SNESCore::snes2pc(a)) = *LPBYTE(nmmx.rom + SNESCore::snes2pc(a + 2)); a++;
		//*LPBYTE(nmmx.rom + SNESCore::snes2pc(a)) = *LPBYTE(nmmx.rom + SNESCore::snes2pc(a + 2)); a++;
		//*LPBYTE(nmmx.rom + SNESCore::snes2pc(a)) = *LPBYTE(nmmx.rom + SNESCore::snes2pc(a + 2)); a++;
		//*LPBYTE(nmmx.rom + SNESCore::snes2pc(a)) = *LPBYTE(nmmx.rom + SNESCore::snes2pc(a + 2)); a++;
		//*LPBYTE(nmmx.rom + SNESCore::snes2pc(a)) = *LPBYTE(nmmx.rom + SNESCore::snes2pc(a + 2)); a++;
		//*LPBYTE(nmmx.rom + SNESCore::snes2pc(a)) = *LPBYTE(nmmx.rom + SNESCore::snes2pc(a + 2)); a++;
		//// long jump to $82 handler
		//*LPBYTE(nmmx.rom + SNESCore::snes2pc(a++)) = 0x5C;
		//*LPBYTE(nmmx.rom + SNESCore::snes2pc(a++)) = 0xC0;
		//*LPBYTE(nmmx.rom + SNESCore::snes2pc(a++)) = 0xFF;
		//*LPBYTE(nmmx.rom + SNESCore::snes2pc(a++)) = 0x82;

		//// mummy
		//a = 0x86C18C;
		//// shift up *
		//*LPBYTE(nmmx.rom + SNESCore::snes2pc(a)) = *LPBYTE(nmmx.rom + SNESCore::snes2pc(a + 2)); a++;
		//*LPBYTE(nmmx.rom + SNESCore::snes2pc(a)) = *LPBYTE(nmmx.rom + SNESCore::snes2pc(a + 2)); a++;
		//*LPBYTE(nmmx.rom + SNESCore::snes2pc(a)) = *LPBYTE(nmmx.rom + SNESCore::snes2pc(a + 2)); a++;
		//*LPBYTE(nmmx.rom + SNESCore::snes2pc(a)) = *LPBYTE(nmmx.rom + SNESCore::snes2pc(a + 2)); a++;
		//*LPBYTE(nmmx.rom + SNESCore::snes2pc(a)) = *LPBYTE(nmmx.rom + SNESCore::snes2pc(a + 2)); a++;
		//*LPBYTE(nmmx.rom + SNESCore::snes2pc(a)) = *LPBYTE(nmmx.rom + SNESCore::snes2pc(a + 2)); a++;
		//*LPBYTE(nmmx.rom + SNESCore::snes2pc(a)) = *LPBYTE(nmmx.rom + SNESCore::snes2pc(a + 2)); a++;
		//*LPBYTE(nmmx.rom + SNESCore::snes2pc(a)) = *LPBYTE(nmmx.rom + SNESCore::snes2pc(a + 2)); a++;
		//*LPBYTE(nmmx.rom + SNESCore::snes2pc(a)) = *LPBYTE(nmmx.rom + SNESCore::snes2pc(a + 2)); a++;
		//*LPBYTE(nmmx.rom + SNESCore::snes2pc(a)) = *LPBYTE(nmmx.rom + SNESCore::snes2pc(a + 2)); a++;
		//*LPBYTE(nmmx.rom + SNESCore::snes2pc(a)) = *LPBYTE(nmmx.rom + SNESCore::snes2pc(a + 2)); a++;
		//*LPBYTE(nmmx.rom + SNESCore::snes2pc(a)) = *LPBYTE(nmmx.rom + SNESCore::snes2pc(a + 2)); a++;
		//// long jump to $82 handler
		//*LPBYTE(nmmx.rom + SNESCore::snes2pc(a++)) = 0x5C;
		//*LPBYTE(nmmx.rom + SNESCore::snes2pc(a++)) = 0xC0;
		//*LPBYTE(nmmx.rom + SNESCore::snes2pc(a++)) = 0xFF;
		//*LPBYTE(nmmx.rom + SNESCore::snes2pc(a++)) = 0x82;

		//// death
		//a = 0x86C2EB;
		//// shift up *
		//*LPBYTE(nmmx.rom + SNESCore::snes2pc(a)) = *LPBYTE(nmmx.rom + SNESCore::snes2pc(a + 2)); a++;
		//*LPBYTE(nmmx.rom + SNESCore::snes2pc(a)) = *LPBYTE(nmmx.rom + SNESCore::snes2pc(a + 2)); a++;
		//*LPBYTE(nmmx.rom + SNESCore::snes2pc(a)) = *LPBYTE(nmmx.rom + SNESCore::snes2pc(a + 2)); a++;
		//*LPBYTE(nmmx.rom + SNESCore::snes2pc(a)) = *LPBYTE(nmmx.rom + SNESCore::snes2pc(a + 2)); a++;
		//*LPBYTE(nmmx.rom + SNESCore::snes2pc(a)) = *LPBYTE(nmmx.rom + SNESCore::snes2pc(a + 2)); a++;
		//*LPBYTE(nmmx.rom + SNESCore::snes2pc(a)) = *LPBYTE(nmmx.rom + SNESCore::snes2pc(a + 2)); a++;
		//// long jump to $82 handler
		//*LPBYTE(nmmx.rom + SNESCore::snes2pc(a++)) = 0x5C;
		//*LPBYTE(nmmx.rom + SNESCore::snes2pc(a++)) = 0xC0;
		//*LPBYTE(nmmx.rom + SNESCore::snes2pc(a++)) = 0xFF;
		//*LPBYTE(nmmx.rom + SNESCore::snes2pc(a++)) = 0x82;

		//// remove increment on bat
		//a = 0x86BA3E;
		//*LPBYTE(nmmx.rom + SNESCore::snes2pc(a++)) = 0xEA;
		//*LPBYTE(nmmx.rom + SNESCore::snes2pc(a++)) = 0xEA;

		//// reverse level support
		//// death
		//a = 0x8280AF;
		//// JSL
		//*LPBYTE(nmmx.rom + SNESCore::snes2pc(a++)) = 0x22;
		//*LPBYTE(nmmx.rom + SNESCore::snes2pc(a++)) = 0xF0;
		//*LPBYTE(nmmx.rom + SNESCore::snes2pc(a++)) = 0xFF;
		//*LPBYTE(nmmx.rom + SNESCore::snes2pc(a++)) = 0x8C;
		//// NOP
		//*LPBYTE(nmmx.rom + SNESCore::snes2pc(a++)) = 0xEA;

		//// continue
		//a = 0x8CFD57;
		//// JSL
		//*LPBYTE(nmmx.rom + SNESCore::snes2pc(a++)) = 0x22;
		//*LPBYTE(nmmx.rom + SNESCore::snes2pc(a++)) = 0xF0;
		//*LPBYTE(nmmx.rom + SNESCore::snes2pc(a++)) = 0xFF;
		//*LPBYTE(nmmx.rom + SNESCore::snes2pc(a++)) = 0x8C;
		//// NOP
		//*LPBYTE(nmmx.rom + SNESCore::snes2pc(a++)) = 0xEA;

		//a = 0x8CFFF0;
		//// STA $86
		//*LPBYTE(nmmx.rom + SNESCore::snes2pc(a++)) = 0x85;
		//*LPBYTE(nmmx.rom + SNESCore::snes2pc(a++)) = 0x86;
		//// PHX
		//*LPBYTE(nmmx.rom + SNESCore::snes2pc(a++)) = 0xDA;
		//// ASL
		//*LPBYTE(nmmx.rom + SNESCore::snes2pc(a++)) = 0x0A;
		//// TAX
		//*LPBYTE(nmmx.rom + SNESCore::snes2pc(a++)) = 0xAA;
		//// LDA long, x
		//*LPBYTE(nmmx.rom + SNESCore::snes2pc(a++)) = 0xBF;
		//*LPBYTE(nmmx.rom + SNESCore::snes2pc(a++)) = 0x00;
		//*LPBYTE(nmmx.rom + SNESCore::snes2pc(a++)) = 0xC4;
		//*LPBYTE(nmmx.rom + SNESCore::snes2pc(a++)) = 0xA0;
		//// STA $13D4
		//*LPBYTE(nmmx.rom + SNESCore::snes2pc(a++)) = 0x8D;
		//*LPBYTE(nmmx.rom + SNESCore::snes2pc(a++)) = 0xD4;
		//*LPBYTE(nmmx.rom + SNESCore::snes2pc(a++)) = 0x13;
		//// PLX
		//*LPBYTE(nmmx.rom + SNESCore::snes2pc(a++)) = 0xFA;
		//// RTL
		//*LPBYTE(nmmx.rom + SNESCore::snes2pc(a++)) = 0x6B;

		//// force graphics reload on level transition
		//*LPWORD(nmmx.rom + SNESCore::snes2pc(0x86B55D)) = 0xEAEA;
		//*LPWORD(nmmx.rom + SNESCore::snes2pc(0x86B593)) = 0xEAEA;
	}

	// form code string
	ss.str("");
	ss.clear();

	// version
	std::string version;
	if (GetVersion(version)) {
		ss << version << '-';

		// seed
		ss << std::setw(8) << std::setfill('0') << seed << '-';

		// difficulty
		CSliderCtrl *difficulty = (CSliderCtrl *)GetDlgItem(IDC_COMMON_DIFFICULTY);
		ss << difficultyNames[difficulty->GetPos()] << '-';

		// randomtype
		CComboBox *c = (CComboBox*)GetDlgItem(IDC_COMMON_RANDOMTYPE);
		ss << typeNames[c->GetCurSel()] << '-';

		// checkboxes
		for (auto &c : checkBoxes) {
			ss << ((CButton*)GetDlgItem(std::get<unsigned>(c)))->GetCheck() ? '1' : '0';
		}
	}
	else {
		ss << "invalid";
	}
	SetDlgItemText(IDC_CODE, ss.str().c_str());

	LPBYTE codeAddress = nmmx.rom + 0x100000 + 0x8000 - nmmx.expandedROMHeaderSize - ss.str().length() - 0x10;
	strcpy((char *)codeAddress, ss.str().c_str());

	theApp.CodeName = ss.str() + ".smc";

	ss.str("");
	ss.clear();
	ss << "Checksum: " << std::hex << nmmx.GetChecksum() << " CRC32: " << nmmx.GetCRC32();
	SetDlgItemText(IDC_CHECKSUM, ss.str().c_str());

	pMainWnd->SetMessageText((UINT)(error ? AFX_IDS_ERRMESSAGE : AFX_IDS_IDLEMESSAGE));
}


void CSCV4RandomizerView::OnNMCustomdrawCommonDifficulty(NMHDR *pNMHDR, LRESULT *pResult)
{
	LPNMCUSTOMDRAW pNMCD = reinterpret_cast<LPNMCUSTOMDRAW>(pNMHDR);
	// TODO: Add your control notification handler code here
	CSliderCtrl *difficulty = (CSliderCtrl *)GetDlgItem(IDC_COMMON_DIFFICULTY);
	SetDlgItemText(IDC_COMMON_DIFFICULTYNAME, difficultyNames[difficulty->GetPos()].c_str());

	*pResult = 0;
}

void CSCV4RandomizerView::OnEnChangeCommonSeed()
{
	// TODO:  If this is a RICHEDIT control, the control will not
	// send this notification unless you override the CFormView::OnInitDialog()
	// function and call CRichEditCtrl().SetEventMask()
	// with the ENM_CHANGE flag ORed into the mask.

	// TODO:  Add your control notification handler code here
	CEdit *editSeed = (CEdit *)GetDlgItem(IDC_COMMON_SEED);
	CString t;
	std::stringstream ss;
	unsigned seed;

	GetDlgItemText(IDC_COMMON_SEED, t);
	ss << std::hex << t;
	auto ok = (ss.str() == "") || (ss.str().length() <= 8 && (ss >> std::hex >> seed) && ss.eof());
	SetDlgItemText(IDC_COMMON_SEEDSTATE, ok ? "OK" : "BAD");
}

void CSCV4RandomizerView::OnDestroy() {
	// write registry
	// init check boxes
	for (auto &i : checkBoxes) {
		CButton *b = (CButton*)GetDlgItem(std::get<unsigned>(i));
		ASSERT(b);
		theApp.WriteProfileInt(_T("Settings"), std::get<std::string>(i).c_str(), b->GetCheck());
	}

	// init combo box
	CComboBox *c = (CComboBox*)GetDlgItem(IDC_COMMON_RANDOMTYPE);
	ASSERT(c);
	theApp.WriteProfileInt(_T("Settings"), _T("IDC_COMMON_RANDTYPE"), c->GetCurSel());

	// init difficulty
	CSliderCtrl *difficulty = (CSliderCtrl *)GetDlgItem(IDC_COMMON_DIFFICULTY);
	theApp.WriteProfileInt(_T("Settings"), _T("IDC_COMMON_DIFFICULTY"), difficulty->GetPos());

	CFormView::OnClose();
}
