#include "Emulator.h"
#include "libretro.h"
#include <sstream>
#include <fstream>
#include <iomanip>

// NOTE: a lot of code used from LMSW (Thanks).
// This code needs a rewrite to use messaging more cleanly and remove volatile types.  Also, the emu thread should
// be separated out

#define RAM_ADDR(addr) ((addr) - 0x7E0000)

std::unique_ptr<Emulator> Emulator::emu = std::unique_ptr<Emulator>(new Emulator);

bool retro_load(LPCWSTR dllName);

struct ThreadParam {
	int id = -1;
	HANDLE h;
};

static DWORD WINAPI ThreadProc(LPVOID param) {
	MSG msg;
	PeekMessage(&msg, (HWND)-1, Emulator::Message::FIRST, Emulator::Message::LAST, PM_NOREMOVE);
	auto p = (ThreadParam *)param;
	AttachThreadInput(GetCurrentThreadId(), p->id, TRUE);
	int mainThreadId = p->id;
	SetEvent(p->h);
	int saveStateSize = 0;

	LPCWSTR dllName = L"retro.dll";

	//LPCWSTR dllName = L"snes9x_next_libretro.dll";
	if (!retro_load(LPCWSTR(dllName))) {
		std::stringstream ss;
		ss << "Couldn't open " << dllName << ".  Please check the directory for the correct library";
		Emulator::Instance()->retroLoadError = ss.str();
		Emulator::Instance()->snesState = Emulator::SnesState::INVALID;
		PostThreadMessage(mainThreadId, (int)Emulator::Instance()->snesState, 0, 0);
		ExitThread(1);
	}

	if (retro_api_version() != RETRO_API_VERSION) {
		std::stringstream ss;
		ss << "retro_api_version() does not match RETRO_API_VERSION.  Incompatible DLL." << retro_api_version() << " != " << RETRO_API_VERSION;
		Emulator::Instance()->retroLoadError = ss.str();
		Emulator::Instance()->snesState = Emulator::SnesState::INVALID;
		PostThreadMessage(mainThreadId, (int)Emulator::Instance()->snesState, 0, 0);
		ExitThread(1);
	}

	Emulator::Instance()->audio_init(32040.5); //??
	retro_set_environment(Emulator::Environment);
	retro_init();
	retro_set_video_refresh(Emulator::VideoRefresh);
	retro_set_audio_sample(Emulator::AudioSample);
	retro_set_audio_sample_batch(Emulator::AudioSampleBatch);
	retro_set_input_poll(Emulator::InputPoll);
	retro_set_input_state(Emulator::InputRead);
	//retro_set_frame_time_callback(Emulator::FrameTime);
	retro_set_controller_port_device(0, RETRO_DEVICE_JOYPAD);
	Emulator::Instance()->snesState = Emulator::SnesState::OFF;

	// setup palette
	for (int b = 0; b<32; b++)
	{
		for (int g = 0; g<64; g++)
		{
			for (int r = 0; r<32; r++)
			{
				int index = b << 11 | g << 5 | r << 0;
				//Emulator::Instance()->palette[index] = Emulator::Instance()->pal5to8[Emulator::Instance()->palettetype][b] << 16 | /*Emulator::Instance()->pal5to8[Emulator::Instance()->palettetype][g] << 8*/ ((g << 2) << 8) | Emulator::Instance()->pal5to8[Emulator::Instance()->palettetype][r] << 0;
				Emulator::Instance()->palette[index] = ((b << 3) << 16) | ((g << 2) << 8) | ((r << 3) << 0);
			}
		}
	}

	int frame = 0;
	while (true) {
		bool found = true;

		if (Emulator::Instance()->snesState < Emulator::SnesState::ON) {
			GetMessage(&msg, (HWND)-1, Emulator::Message::FIRST, Emulator::Message::LAST);
		}
		else {
			found = PeekMessage(&msg, (HWND)-1, Emulator::Message::FIRST, Emulator::Message::LAST, PM_REMOVE);
		}

		if (found) {
			MSG msg2;

			if (((msg.message == Emulator::Message::PAUSE || msg.message == Emulator::Message::STEP)
				&& PeekMessage(&msg2, (HWND)-1, Emulator::Message::LOADROM, Emulator::Message::PAUSE, PM_NOREMOVE))
				|| PeekMessage(&msg2, (HWND)-1, Emulator::Message::LOADROM, Emulator::Message::LOADROM, PM_NOREMOVE)
				|| PeekMessage(&msg2, (HWND)-1, msg.message, msg.message, PM_NOREMOVE)) {
				//if (msg.lParam) free((void*)msg.lParam);
				continue;
			}

			switch (msg.message) {
			case Emulator::Message::INIT: {
				// setup double buffer
				memset(Emulator::Instance()->buffer, 0, 2 * 256 * 224 * 4);
				Emulator::Instance()->currentBuffer = 0;

				// set current states to init
				Emulator::Instance()->snesState = Emulator::SnesState::INIT;

				PostThreadMessage(mainThreadId, (int)Emulator::Instance()->snesState, 0, 0);

				break;
			}
			case Emulator::Message::LOADROM: {
				Emulator::Instance()->AcquireFromMain();
				//emuReadOrder.load(std::memory_order_acquire);

				Emulator::Instance()->snesState = Emulator::SnesState::BUSY;
				if (Emulator::Instance()->snesState > Emulator::SnesState::OFF) {
					retro_unload_game();
				}
				retro_game_info gameInfo = { "", (void*)msg.lParam, msg.wParam, NULL };
				retro_load_game(&gameInfo);
				//free((void *)msg.lParam);

				Emulator::Instance()->wram = (LPBYTE)retro_get_memory_data(RETRO_MEMORY_SYSTEM_RAM);
				Emulator::Instance()->vram = (LPWORD)retro_get_memory_data(RETRO_MEMORY_VIDEO_RAM);

				if (!Emulator::Instance()->wram || !Emulator::Instance()->vram) {
					Emulator::Instance()->snesState = Emulator::SnesState::INVALID;
					retro_deinit();
					//audio_term();
					std::stringstream ss;
					ss << "libretro doesn't provide needed functions. VRAM=" << Emulator::Instance()->vram << " WRAM=" << Emulator::Instance()->wram;
					Emulator::Instance()->retroLoadError = ss.str();
					PostThreadMessage(mainThreadId, (int)Emulator::Instance()->snesState, 0, 0);
					ExitThread(1);
				}
				saveStateSize = retro_serialize_size();
				if (!saveStateSize) {
					Emulator::Instance()->snesState = Emulator::SnesState::INVALID;
					retro_deinit();
					Emulator::Instance()->audio_term();
					std::stringstream ss;
					ss << "libretro doesn't provide savestate support.";
					Emulator::Instance()->retroLoadError = ss.str();
					PostThreadMessage(mainThreadId, (int)Emulator::Instance()->snesState, 0, 0);
					ExitThread(1);
				}
				Emulator::Instance()->saveState = std::unique_ptr<BYTE[]>(new BYTE[saveStateSize]);

				// TODO: setup initial RAM state?

				// TODO: setup level exit RAM state?

				// TODO: setup RAM routine?

				// TODO: setup jump to RAM routine?

				// preload save state
				//retro_serialize(Emulator::Instance()->preloadSaveState.get(), saveStateLen);

				Emulator::Instance()->snesState = Emulator::SnesState::NOLEVEL;
				PostThreadMessage(mainThreadId, (int)Emulator::Instance()->snesState, 0, 0);
				break;
			}
			case Emulator::Message::RELOADROM: {
				Emulator::Instance()->audio_clear();
				Emulator::Instance()->snesState = Emulator::SnesState::BUSY;
				// TODO: get save state
				retro_unload_game();
				retro_game_info gameInfo = { "", (void*)msg.lParam, msg.wParam, NULL };
				retro_load_game(&gameInfo);
				//free((void *)msg.lParam);
				// TODO: reload save state
				// FIXME: this won't work for things in RAM.  E.g. event structures
				//retro_unserialize();

				Emulator::Instance()->wram = (LPBYTE)retro_get_memory_data(RETRO_MEMORY_SYSTEM_RAM);
				Emulator::Instance()->vram = (LPWORD)retro_get_memory_data(RETRO_MEMORY_VIDEO_RAM);
				PostThreadMessage(mainThreadId, (int)Emulator::Instance()->snesState, 0, 0);
				break;
			}
			case Emulator::Message::LOADLEVEL: {
				Emulator::Instance()->audio_clear();
				Emulator::Instance()->snesState = Emulator::SnesState::BUSY;

				// TODO: setup level exit RAM state?

				// TODO: setup level number

				// TODO: wait?

				// TODO: pre init

				Emulator::Instance()->snesState = Emulator::SnesState::FINALIZE;
				retro_run();
				if (PeekMessage(&msg2, (HWND)-1, Emulator::Message::LOADLEVEL, Emulator::Message::LOADLEVEL, PM_NOREMOVE)) {
					break;
				}

				// TODO: post init?
				Emulator::Instance()->snesState = Emulator::SnesState::ON;
				PostThreadMessage(mainThreadId, (int)Emulator::Instance()->snesState, 0, 0);

				break;
			}
			case Emulator::Message::PAUSE: {
				Emulator::Instance()->audio_clear();
				if (Emulator::Instance()->snesState == Emulator::SnesState::PAUSE) {
					Emulator::Instance()->snesState = Emulator::SnesState::ON;
				}
				else {
					Emulator::Instance()->snesState = Emulator::SnesState::PAUSE;
#if 1
					static std::stringstream ss;
					static unsigned count = 0;
					ss.str("");
					ss.clear();
					ss << "===============" << '\n';
					ss << "DEBUG OUTPUT: " << count++ << '\n';
					ss << "===============" << '\n';
					//for (unsigned i = 0x7E0200; i < 0x7E0F00; i += 0x40) {
					//	WORD index = *LPBYTE(Emulator::Instance()->wram + RAM_ADDR(i + 0x10));
					//	WORD offset = *LPWORD(Emulator::Instance()->wram + RAM_ADDR(i + 0x00));
					//	ss << std::hex << "case 0x" << index << ": offset = 0x" << offset << "; break;" << '\n';
					//}
					static std::vector<std::tuple<DWORD, DWORD>> printAddr;
					if (nmmx.type == 0) {
						printAddr = { std::make_tuple(0x7E0540, 0x7E0580), std::make_tuple(0x7E1280, 0x7E12C0), std::make_tuple(0x7E12C0, 0x7E1300) };
					}
					else if (nmmx.type == 1) {

					}
					else if (nmmx.type == 2) {
						printAddr = { std::make_tuple(0x7E0200, 0x7E0240), std::make_tuple(0x7E08C0, 0x7E0F00), std::make_tuple(0x7E1280, 0x7E12C0), std::make_tuple(0x7E12C0, 0x7E1300) };
					}

					ss << std::hex;
					for (auto &t : printAddr) {
						ss << "0x" << std::get<0>(t) << '\n';
						for (unsigned i = std::get<0>(t); i < std::get<1>(t); i += 0x10) {
							for (unsigned j = 0; j < 0x10; j++) {
								WORD value = *LPBYTE(Emulator::Instance()->wram + RAM_ADDR(i + j));
								ss << std::setw(2) << std::setfill('0') << value << ' ';
							}
							ss << '\n';
						}
					}
					ss << "===============" << '\n';
					OutputDebugString(ss.str().c_str());
					//CreateDialog(NULL, ss.str().c_str(), "Message", MB_USERICON);
#endif
				}
				break;
			}
			case Emulator::Message::UPDATESAVESTATE: {
				Emulator::Instance()->AcquireFromMain();
				//emuReadOrder.load(std::memory_order_acquire);

				char *fileName = (char *)msg.lParam;
				bool save = (bool)msg.wParam;
				bool fail = false;

				if (save) {
					std::ofstream f;
					f.clear();
					f.open(fileName, std::ios_base::out | std::ios_base::binary);
					if (f.good()) {
						fail = !retro_serialize(Emulator::Instance()->saveState.get(), saveStateSize);
						if (!fail) {
							f.write((char *)Emulator::Instance()->saveState.get(), saveStateSize);
							fail = f.fail();
							f.close();
						}
					}
				}
				else {
					std::ifstream f;
					f.clear();
					f.open(fileName, std::ios_base::in | std::ios_base::binary);
					if (f.good()) {
						f.read((char *)Emulator::Instance()->saveState.get(), saveStateSize);
						fail = f.fail();
						f.close();
						if (!fail) {
							fail = !retro_unserialize(Emulator::Instance()->saveState.get(), saveStateSize);
						}
					}
				}
				delete[] fileName;
				break;
			}
			case Emulator::Message::VOLUME: {
				int v = msg.wParam;
				Emulator::Instance()->dsb_b->SetVolume(v);
				break;
			}
			case Emulator::Message::STEP: {
				auto m = Emulator::Instance()->mute;
				Emulator::Instance()->mute = false;
				retro_run();
				Emulator::Instance()->mute = m;
				break;
			}
			case Emulator::Message::TERM: {
				if (Emulator::Instance()->snesState != Emulator::SnesState::OFF) {
					retro_unload_game();
				}
				Emulator::Instance()->audio_term();
				retro_deinit();
				return 0;
				break;
			}
			}

		}
		else if (Emulator::Instance()->snesState >= Emulator::SnesState::ON /* && check rom? */) {
			retro_run();

			// check for crash?

			// TODO: post init?
		}
	}

	return -1;
}

void Emulator::Init() {
	// setup any config state (best done with menu/registry)
	emuState = Emulator::EmuState::OFF;

	// create a new thread with handler
	mainThreadId = GetCurrentThreadId();
	auto p = new ThreadParam;
	p->id = mainThreadId;
	p->h = CreateEvent(NULL, 0, 0, NULL);
	threadHandle = CreateThread(NULL, 0, ThreadProc, p, 0, &threadId);
	WaitForSingleObject(p->h, INFINITE);
	delete p;

	PostThreadMessage(threadId, Message::INIT, (WPARAM)mainThreadId, (LPARAM)0);

	MSG msg;
	GetMessage(&msg, (HWND)-1, (int)Emulator::SnesState::FIRST, (int)Emulator::SnesState::LAST);
	if (msg.wParam == (int)SnesState::INVALID) {
		MessageBox(hWID[0], "Internal emulaor error.", retroLoadError.c_str(), MB_ICONERROR);
	}

	// make sure thread sets init
	//while (Emulator::Instance()->snesState == Emulator::SnesState::INIT) Sleep(10);
	//if (Emulator::Instance()->snesState == Emulator::SnesState::INVALID) {
	//	CloseHandle(threadHandle);
	//}

	mute = false;
}

void Emulator::LoadRom(LPBYTE rom, unsigned size) {
	// Make sure latest ROM is saved?  Should be done outside the emulator.

	// Copy ROM
	localRom = std::unique_ptr<BYTE[]>(new BYTE[size]);
	memcpy(localRom.get(), rom, size);

	if (nmmx.type == 0) {
		// Setup any initial ROM hacks to start up the stage
		*LPBYTE(localRom.get() + SNESCore::snes2pc(0x8089A2) + 0) = 0xA9;
		*LPBYTE(localRom.get() + SNESCore::snes2pc(0x8089A2) + 1) = nmmx.level;
		*LPBYTE(localRom.get() + SNESCore::snes2pc(0x8089A2) + 2) = 0x00;
		*LPBYTE(localRom.get() + SNESCore::snes2pc(0x8089A2) + 3) = 0x85;
		*LPBYTE(localRom.get() + SNESCore::snes2pc(0x8089A2) + 4) = 0x86;

		*LPBYTE(localRom.get() + SNESCore::snes2pc(0x8089F8) + 0) = 0x14;
		*LPBYTE(localRom.get() + SNESCore::snes2pc(0x8089FB) + 0) = 0x70;

		// Setup jump table to point at stage select
		*LPBYTE(localRom.get() + SNESCore::snes2pc(0x808627) + 0) = 0x00;
		*LPBYTE(localRom.get() + SNESCore::snes2pc(0x808627) + 1) = 0xFF;

		// death counter
		*LPBYTE(localRom.get() + SNESCore::snes2pc(0x8094DB) + 0) = 0x01;
		*LPBYTE(localRom.get() + SNESCore::snes2pc(0x8CFD9B) + 0) = 0x01;
		*LPBYTE(localRom.get() + SNESCore::snes2pc(0x8280A1) + 0) = 0x69;
		*LPBYTE(localRom.get() + SNESCore::snes2pc(0x8280A2) + 0) = 0x00;

		// LDA #$4
		unsigned offset = SNESCore::snes2pc(0x80FF00);
		*LPBYTE(localRom.get() + offset++) = 0xA9;
		*LPBYTE(localRom.get() + offset++) = 0x04;
		*LPBYTE(localRom.get() + offset++) = 0x00;
		// STA $32
		*LPBYTE(localRom.get() + offset++) = 0x85;
		*LPBYTE(localRom.get() + offset++) = 0x32;
		// RTL
		*LPBYTE(localRom.get() + offset++) = 0x6B;

		//*LPBYTE(localRom.get() + SNESCore::snes2pc(0x808627) + 0) = 0x40;
		//*LPBYTE(localRom.get() + SNESCore::snes2pc(0x808627) + 1) = 0x87;

		//*LPBYTE(localRom.get() + SNESCore::snes2pc(0x808627) + 2) = 0x40;
		//*LPBYTE(localRom.get() + SNESCore::snes2pc(0x808627) + 3) = 0x87;

		//*LPBYTE(localRom.get() + SNESCore::snes2pc(0x808627) + 4) = 0x40;
		//*LPBYTE(localRom.get() + SNESCore::snes2pc(0x808627) + 5) = 0x87;

		//*LPBYTE(localRom.get() + SNESCore::snes2pc(0x808627) + 6) = 0x40;
		//*LPBYTE(localRom.get() + SNESCore::snes2pc(0x808627) + 7) = 0x87;

		//*LPBYTE(localRom.get() + SNESCore::snes2pc(0x808627) + 2) = 0x63;
		//*LPBYTE(localRom.get() + SNESCore::snes2pc(0x808627) + 3) = 0x9A;

		// Remove STZ $86
		*LPBYTE(localRom.get() + SNESCore::snes2pc(0x8094D5) + 0) = 0xEA;
		*LPBYTE(localRom.get() + SNESCore::snes2pc(0x8094D5) + 1) = 0xEA;

		// Remove name entering
		*LPBYTE(localRom.get() + SNESCore::snes2pc(0x8094B4) + 0) = 0xEA;
		*LPBYTE(localRom.get() + SNESCore::snes2pc(0x8094B4) + 1) = 0xEA;
		*LPBYTE(localRom.get() + SNESCore::snes2pc(0x8094B4) + 2) = 0xEA;
		*LPBYTE(localRom.get() + SNESCore::snes2pc(0x8094B4) + 3) = 0xEA;

		*LPBYTE(localRom.get() + SNESCore::snes2pc(0x8094B4) + 6) = 0xEA;
		*LPBYTE(localRom.get() + SNESCore::snes2pc(0x8094B4) + 7) = 0xEA;

		for (unsigned i = 0; i < 0x44; ++i) {
			*LPBYTE(localRom.get() + SNESCore::snes2pc(nmmx.region == 0 ? 0x81B395 : 0x81B369) + i) = i == 0x43 ? 0x42 : i;
		}
	}
	else if (nmmx.type == 1) {
		// set initial level
		*LPBYTE(localRom.get() + SNESCore::snes2pc(0x80A652) + 0) = nmmx.level + 1;
	}
	else if (nmmx.type == 2) {
		DWORD a = 0x8085BB;
		// JSR
		*LPBYTE(localRom.get() + SNESCore::snes2pc(a++)) = 0x20;
		*LPBYTE(localRom.get() + SNESCore::snes2pc(a++)) = 0xB0;
		*LPBYTE(localRom.get() + SNESCore::snes2pc(a++)) = 0xFF;
		*LPBYTE(localRom.get() + SNESCore::snes2pc(a++)) = 0xEA;

		a = 0x80FFB0;
		// LDA #04
		*LPBYTE(localRom.get() + SNESCore::snes2pc(a++)) = 0xA9;
		*LPBYTE(localRom.get() + SNESCore::snes2pc(a++)) = 0x04;
		// STA D,$3E
		*LPBYTE(localRom.get() + SNESCore::snes2pc(a++)) = 0x85;
		*LPBYTE(localRom.get() + SNESCore::snes2pc(a++)) = 0x3E;
		// LDA level
		*LPBYTE(localRom.get() + SNESCore::snes2pc(a++)) = 0xA9;
		*LPBYTE(localRom.get() + SNESCore::snes2pc(a++)) = nmmx.level;
		// STA D,$86
		*LPBYTE(localRom.get() + SNESCore::snes2pc(a++)) = 0x85;
		*LPBYTE(localRom.get() + SNESCore::snes2pc(a++)) = 0x86;
		// RTS
		*LPBYTE(localRom.get() + SNESCore::snes2pc(a++)) = 0x60;

		a = 0x80D80D;
		// invincible
		*LPBYTE(localRom.get() + SNESCore::snes2pc(a++)) = 0x01;
	}

	// Pass the rom to the thread
	//emuReadOrder.store(true, std::memory_order_release);
	ReleaseToEmu();

	PostThreadMessage(threadId, Message::LOADROM, (WPARAM)size, (LPARAM)localRom.get());
	MSG msg;
	GetMessage(&msg, (HWND)-1, (int)Emulator::SnesState::FIRST, (int)Emulator::SnesState::LAST);
	if (msg.wParam == (int)SnesState::INVALID) {
		MessageBox(hWID[0], "Internal emulaor error.", retroLoadError.c_str(), MB_ICONERROR);
	}
	//while (Emulator::Instance()->snesState != SnesState::NOLEVEL) {}
}
void  Emulator::LoadLevel(int num) {
	emuState = EmuState::ON;
	PostThreadMessage(threadId, Message::LOADLEVEL, (WPARAM)num, (LPARAM)0);
	MSG msg;
	GetMessage(&msg, (HWND)-1, (int)Emulator::SnesState::FIRST, (int)Emulator::SnesState::LAST);
	if (msg.wParam == (int)SnesState::INVALID) {
		MessageBox(hWID[0], "Internal emulaor error.", retroLoadError.c_str(), MB_ICONERROR);
	}
}
void Emulator::UpdateSaveState(bool store, unsigned slot) {
	char *fileName = new char[MAX_PATH + 1];
	//strcpy(fileName, nmmx.filePath);
	_splitpath(nmmx.filePath, NULL, NULL, fileName, NULL);
	std::stringstream ss;
	ss << ".00" << slot;
	std::string s = ss.str();
	strcat(fileName, s.c_str());

	//emuReadOrder.store(true, std::memory_order_release);
	ReleaseToEmu();
	PostThreadMessage(threadId, Message::UPDATESAVESTATE, (WPARAM)store, (LPARAM)fileName);
}

void Emulator::SetVolume(int v) {
	PostThreadMessage(threadId, Message::VOLUME, (WPARAM)v, NULL);
}

Emulator::EmuState Emulator::GetState() {
	// figure out current Emulator::Instance() state and pass that back

	return emuState;
}
void Emulator::GetFrameState(FrameState &s) {
	AcquireFromEmu();
	s.xpos = frameState.xpos;
	s.ypos = frameState.ypos;
	s.borderLeft = frameState.borderLeft;
	s.borderRight = frameState.borderRight;
	s.borderTop = frameState.borderTop;
	s.borderBottom = frameState.borderBottom;
	s.buffer = buffer[currentBuffer];
	s.levelNum = frameState.levelNum;
	s.graphicsNum = frameState.graphicsNum;
	s.tileNum = frameState.tileNum;
	s.paletteNum = frameState.paletteNum;

	s.state0 = frameState.state0;
	s.state1 = frameState.state1;
	s.s_xpos = frameState.s_xpos;
	s.s_ypos = frameState.s_ypos;
	s.c0_xpos = frameState.c0_xpos;
	s.c0_ypos = frameState.c0_ypos;
	s.c1_xpos = frameState.c1_xpos;
	s.c1_ypos = frameState.c1_ypos;

	s.lockState = frameState.lockState;
	s.A0 = frameState.A0;
	s.A2 = frameState.A2;
	s.A4 = frameState.A4;
	s.A6 = frameState.A6;
	s.A8 = frameState.A8;
	s.AA = frameState.AA;
}
void Emulator::Pause() {
	// tell the Emulator::Instance() thread to pause
	if (emuState == EmuState::ON) {
		PostThreadMessage(threadId, Message::PAUSE, 2, 0);
		emuState = EmuState::PAUSE;
	}
	else if (emuState == EmuState::PAUSE) {
		PostThreadMessage(threadId, Message::PAUSE, 0, 0);
		emuState = EmuState::ON;
	}
}
void Emulator::Stop() {
	// tell the Emulator::Instance() thread to stop
	if (emuState != EmuState::OFF) {
		PostThreadMessage(threadId, Message::PAUSE, 1, 0);
		emuState = EmuState::OFF;
	}
}
void Emulator::Step() {
	// tell the Emulator::Instance() thread to step
}
void Emulator::Terminate() {
	// tell the Emulator::Instance() thread to terminate
	if (emuState != EmuState::OFF) {
		Stop();
		PostThreadMessage(threadId, Message::TERM, 0, 0);
		WaitForSingleObject(threadHandle, INFINITE);
		CloseHandle(threadHandle);
		Reset();
	}
}

void Emulator::audio_init(double sample_rate) {
	sampleRate = sample_rate;
	ds = 0;
	dsb_p = 0;
	dsb_b = 0;

	device.buffer = 0;
	device.bufferOffset = 0;
	device.readRing = 0;
	device.writeRing = 0;
	device.distance = 0;

	device.rings = 8;
	device.latency = unsigned(floor(sampleRate * 60 / double(device.rings) / 1000.0 + 0.5));
	device.buffer = new uint32_t[device.latency * device.rings];
	device.bufferOffset = 0;

	DirectSoundCreate(0, &ds, 0);
	ds->SetCooperativeLevel(GetDesktopWindow(), DSSCL_PRIORITY);

	memset(&dsbd, 0, sizeof(dsbd));
	dsbd.dwSize = sizeof(dsbd);
	dsbd.dwFlags = DSBCAPS_PRIMARYBUFFER;
	dsbd.dwBufferBytes = 0;
	dsbd.lpwfxFormat = 0;
	ds->CreateSoundBuffer(&dsbd, &dsb_p, 0);

	memset(&wfx, 0, sizeof(wfx));
	wfx.wFormatTag = WAVE_FORMAT_PCM;
	wfx.nChannels = 2;
	wfx.nSamplesPerSec = DWORD(floor(sampleRate));
	wfx.wBitsPerSample = 16;
	wfx.nBlockAlign = wfx.wBitsPerSample / 8 * wfx.nChannels;
	wfx.nAvgBytesPerSec = wfx.nSamplesPerSec * wfx.nBlockAlign;
	dsb_p->SetFormat(&wfx);

	memset(&dsbd, 0, sizeof(dsbd));
	dsbd.dwSize = sizeof(dsbd);
	dsbd.dwFlags = DSBCAPS_GETCURRENTPOSITION2 | DSBCAPS_CTRLFREQUENCY | DSBCAPS_GLOBALFOCUS | DSBCAPS_LOCSOFTWARE | DSBCAPS_CTRLVOLUME;
	dsbd.dwBufferBytes = device.latency * device.rings * sizeof(uint32_t);
	dsbd.guid3DAlgorithm = GUID_NULL;
	dsbd.lpwfxFormat = &wfx;
	ds->CreateSoundBuffer(&dsbd, &dsb_b, 0);
	dsb_b->SetFrequency(DWORD(floor(sampleRate)));
	dsb_b->SetCurrentPosition(0);

	audio_clear();

	dsb_b->SetVolume(set.volume);
}
void Emulator::audio_sample(int16_t left, int16_t right) {
	device.buffer[device.bufferOffset++] = left + (right << 16);
	if (device.bufferOffset < device.latency) return;
	device.bufferOffset = 0;

	DWORD pos, size;
	void *output;

	if (1) {
		//if(settings.synchronize == true) {
		//wait until playback buffer has an empty ring to write new audio data to
		while (device.distance >= (int)device.rings - 1) {
			HRESULT err = dsb_b->GetCurrentPosition(0, &pos);
			if (err)
			{
				if (err == DSERR_BUFFERLOST)//workaround if something (*coughzsnescough*) pulls the primary sound buffer
				{
					audio_term();
					audio_init(sampleRate);
				}
				if (err == DSERR_OTHERAPPHASPRIO)//in case zsomething is still running; it'll throw FPS exactly everywhere, but whatever. it's better than crashing
				{
					static bool warned = false;
					if (!warned)
					{
						MessageBox(NULL, "Something is demanding exclusive rights of the sound on this machine. "
							"LMSW needs sound synchronization to keep the FPS sane. Please disable this other program, "
							"or LMSW's speed will only be limited by your CPU speed.", "LMSW", MB_OK | MB_ICONSTOP);
						warned = true;
					}
				}
				return;
			}
			unsigned activering = pos / (device.latency * 4);
			if (activering == device.readRing) {
				//if(settings.synchronize == false) Sleep(1);
				Sleep(10);
				continue;
			}

			//subtract number of played rings from ring distance counter
			device.distance -= (device.rings + activering - device.readRing) % device.rings;
			device.readRing = activering;

			if (device.distance < 2) {
				//buffer underflow; set max distance to recover quickly
				device.distance = device.rings - 1;
				device.writeRing = (device.rings + device.readRing - 1) % device.rings;
				break;
			}
		}
	}

	device.writeRing = (device.writeRing + 1) % device.rings;
	device.distance = (device.distance + 1) % device.rings;

	if (dsb_b->Lock(device.writeRing * device.latency * 4, device.latency * 4, &output, &size, 0, 0, 0) == DS_OK) {
		memcpy(output, device.buffer, device.latency * 4);
		dsb_b->Unlock(output, size, 0, 0);
	}
}

void Emulator::audio_clear() {
	device.readRing = 0;
	device.writeRing = device.rings - 1;
	device.distance = device.rings - 1;

	device.bufferOffset = 0;
	if (device.buffer) memset(device.buffer, 0, device.latency * device.rings * 4);

	if (!dsb_b) return;
	dsb_b->Stop();
	dsb_b->SetCurrentPosition(0);

	DWORD size;
	void *output;
	dsb_b->Lock(0, device.latency * device.rings * 4, &output, &size, 0, 0, 0);
	memset(output, 0, size);
	dsb_b->Unlock(output, size, 0, 0);

	dsb_b->Play(0, 0, DSBPLAY_LOOPING);
}

void Emulator::audio_term() {
	if (device.buffer) {
		delete[] device.buffer;
		device.buffer = 0;
	}

	if (dsb_b) { dsb_b->Stop(); dsb_b->Release(); dsb_b = 0; }
	if (dsb_p) { dsb_p->Stop(); dsb_p->Release(); dsb_p = 0; }
	if (ds) { ds->Release(); ds = 0; }
}
// 128A, 12A2
#define KEYS_DEF \
	KEY_DEF(RAM_CAMERAXPOS, 0x7E1280 /*8A*/, 0x7E1380, 0x7E128A), \
	KEY_DEF(RAM_CAMERAYPOS, 0x7E1298 /*A2*/, 0x7E1398, 0x7E12A2), \
	KEY_DEF(RAM_BORDERLEFT, 0x7E00A0, NULL, NULL), \
	KEY_DEF(RAM_BORDERRIGHT, 0x7E00A2, NULL, NULL), \
	KEY_DEF(RAM_BORDERTOP, 0x7E00A4, NULL, NULL), \
	KEY_DEF(RAM_BORDERBOTTOM, 0x7E00A6, NULL, NULL), \
	KEY_DEF(RAM_GRAPHICSNUM, NULL, NULL, NULL), \
	KEY_DEF(RAM_TILENUM, NULL, NULL, NULL), \
	KEY_DEF(RAM_PALETTENUM, NULL, NULL, NULL), \
	KEY_DEF(RAM_LEVELNUM, 0x7E0086, 0x7E0086, 0x7E0086), \
	KEY_DEF(RAM_STATE0, 0x7E0552, NULL, NULL), \
	KEY_DEF(RAM_STATE0_B, 0x7E057A, NULL, NULL), \
	KEY_DEF(RAM_STATE1, 0x7E0578, NULL, NULL), \
	KEY_DEF(RAM_S_XPOS, 0x7E054A, NULL, NULL), \
	KEY_DEF(RAM_S_YPOS, 0x7E054E, NULL, NULL), \
	KEY_DEF(RAM_C0_XPOS, 0x7E1280, NULL, NULL), \
	KEY_DEF(RAM_C0_YPOS, 0x7E1298, NULL, NULL), \
	KEY_DEF(RAM_C1_XPOS, 0x7E12C0, NULL, NULL), \
	KEY_DEF(RAM_C1_YPOS, 0x7E12D8, NULL, NULL), \
	KEY_DEF(RAM_LOCKSTATE, 0x7E0542, NULL, NULL), \
	KEY_DEF(RAM_A0, 0x7E00A0, NULL, NULL), \
	KEY_DEF(RAM_A2, 0x7E00A2, NULL, NULL), \
	KEY_DEF(RAM_A4, 0x7E00A4, NULL, NULL), \
	KEY_DEF(RAM_A6, 0x7E00A6, NULL, NULL), \
	KEY_DEF(RAM_A8, 0x7E00A8, NULL, NULL), \
	KEY_DEF(RAM_AA, 0x7E00AA, NULL, NULL), \


#define KEY_DEF(id, x1, x2, x3) id
enum keys { KEYS_DEF };
#undef KEY_DEF

#define KEY_DEF(id, x1, x2, x3) { x1, x2, x3 }
static const long ramAddr[][3] = { KEYS_DEF };
#undef KEY_DEF
#undef KEYS_DEF

void Emulator::VideoRefresh(const void *data, unsigned width, unsigned height, size_t pitch) {
	uint8_t buffer = Emulator::Instance()->currentBuffer ^ 1;
	int widthskip = (width == 512) ? 1 : 0;
	for (int y = 0; y<224; y++)
	{
		uint32_t * outputLine = Emulator::Instance()->buffer[buffer] + y * 256;
		const uint16_t *inputLine = ((const uint16_t*)data) + y*pitch / 2;
		for (int x = 0; x < 256; x++) *outputLine++ = Emulator::Instance()->palette[inputLine[x << widthskip]];
	}
	// previous camera position matches current rendered video frame
	static int prevXpos = 0;
	static int prevYpos = 0;

	int xpos = prevXpos;
	int ypos = prevYpos;

	prevXpos = ramAddr[RAM_CAMERAXPOS][nmmx.type] ? *LPWORD(Emulator::Instance()->wram + RAM_ADDR(ramAddr[RAM_CAMERAXPOS][nmmx.type]) + 0) - 0 /* *LPWORD(Emulator::Instance()->wram + RAM_ADDR(0x7E128E) + 0)*/ : 0;
	prevYpos = ramAddr[RAM_CAMERAYPOS][nmmx.type] ? *LPWORD(Emulator::Instance()->wram + RAM_ADDR(ramAddr[RAM_CAMERAYPOS][nmmx.type]) + 0) - 0 /* *LPWORD(Emulator::Instance()->wram + RAM_ADDR(0x7E12A6) + 0)*/ : 0;

	int borderLeft = ramAddr[RAM_BORDERLEFT][nmmx.type] ? *LPWORD(Emulator::Instance()->wram + RAM_ADDR(ramAddr[RAM_BORDERLEFT][nmmx.type])) : 0;
	int borderRight = ramAddr[RAM_BORDERRIGHT][nmmx.type] ? *LPWORD(Emulator::Instance()->wram + RAM_ADDR(ramAddr[RAM_BORDERRIGHT][nmmx.type]))+256 : 0;
	int borderTop = ramAddr[RAM_BORDERTOP][nmmx.type] ? *LPWORD(Emulator::Instance()->wram + RAM_ADDR(ramAddr[RAM_BORDERTOP][nmmx.type])) : 0;
	int borderBottom = ramAddr[RAM_BORDERBOTTOM][nmmx.type] ? *LPWORD(Emulator::Instance()->wram + RAM_ADDR(ramAddr[RAM_BORDERBOTTOM][nmmx.type]))+224 : 0;

	int graphicsNum = ramAddr[RAM_GRAPHICSNUM][nmmx.type] ? *LPBYTE(Emulator::Instance()->wram + RAM_ADDR(ramAddr[RAM_GRAPHICSNUM][nmmx.type])) : 0;
	int tileNum = ramAddr[RAM_TILENUM][nmmx.type] ? *LPBYTE(Emulator::Instance()->wram + RAM_ADDR(ramAddr[RAM_TILENUM][nmmx.type])) : 0;
	int paletteNum = ramAddr[RAM_PALETTENUM][nmmx.type] ? *LPBYTE(Emulator::Instance()->wram + RAM_ADDR(ramAddr[RAM_PALETTENUM][nmmx.type])) : 0;
	int levelNum = ramAddr[RAM_LEVELNUM][nmmx.type] ? *LPBYTE(Emulator::Instance()->wram + RAM_ADDR(ramAddr[RAM_LEVELNUM][nmmx.type])) : 0;

	if (nmmx.type == 1) levelNum--;

	Emulator::Instance()->currentBuffer = buffer;
	Emulator::Instance()->frameState.xpos = xpos;
	Emulator::Instance()->frameState.ypos = ypos;
	Emulator::Instance()->frameState.borderLeft = borderLeft;
	Emulator::Instance()->frameState.borderRight = borderRight;
	Emulator::Instance()->frameState.borderTop = borderTop;
	Emulator::Instance()->frameState.borderBottom = borderBottom;
	Emulator::Instance()->frameState.graphicsNum = graphicsNum;
	Emulator::Instance()->frameState.tileNum = tileNum;
	Emulator::Instance()->frameState.paletteNum = paletteNum;
	Emulator::Instance()->frameState.levelNum = levelNum;

	auto state0 = ramAddr[RAM_STATE0][nmmx.type] ? *LPWORD(Emulator::Instance()->wram + RAM_ADDR(ramAddr[RAM_STATE0][nmmx.type])) : 0xFFFF;
	state0 = state0 == 0xB ? 0xA : 0x0;
	auto state0_b = ramAddr[RAM_STATE0_B][nmmx.type] ? *LPWORD(Emulator::Instance()->wram + RAM_ADDR(ramAddr[RAM_STATE1][nmmx.type])) : 0xFFFF;
	Emulator::Instance()->frameState.state0 = (state0 | ((state0_b & 0x2) << 6));
	Emulator::Instance()->frameState.state1 = ramAddr[RAM_STATE1][nmmx.type] ? *LPWORD(Emulator::Instance()->wram + RAM_ADDR(ramAddr[RAM_STATE1][nmmx.type])) : 0xFFFF;
	Emulator::Instance()->frameState.s_xpos = ramAddr[RAM_S_XPOS][nmmx.type] ? *LPWORD(Emulator::Instance()->wram + RAM_ADDR(ramAddr[RAM_S_XPOS][nmmx.type])) : 0xFFFF;
	Emulator::Instance()->frameState.s_ypos = ramAddr[RAM_S_YPOS][nmmx.type] ? *LPWORD(Emulator::Instance()->wram + RAM_ADDR(ramAddr[RAM_S_YPOS][nmmx.type])) : 0xFFFF;
	Emulator::Instance()->frameState.c0_xpos = ramAddr[RAM_C0_XPOS][nmmx.type] ? *LPWORD(Emulator::Instance()->wram + RAM_ADDR(ramAddr[RAM_C0_XPOS][nmmx.type])) : 0xFFFF;
	Emulator::Instance()->frameState.c0_ypos = ramAddr[RAM_C0_YPOS][nmmx.type] ? *LPWORD(Emulator::Instance()->wram + RAM_ADDR(ramAddr[RAM_C0_YPOS][nmmx.type])) : 0xFFFF;
	Emulator::Instance()->frameState.c1_xpos = ramAddr[RAM_C1_XPOS][nmmx.type] ? *LPWORD(Emulator::Instance()->wram + RAM_ADDR(ramAddr[RAM_C1_XPOS][nmmx.type])) : 0xFFFF;
	Emulator::Instance()->frameState.c1_ypos = ramAddr[RAM_C1_YPOS][nmmx.type] ? *LPWORD(Emulator::Instance()->wram + RAM_ADDR(ramAddr[RAM_C1_YPOS][nmmx.type])) : 0xFFFF;

	Emulator::Instance()->frameState.lockState = ramAddr[RAM_LOCKSTATE][nmmx.type] ? *LPWORD(Emulator::Instance()->wram + RAM_ADDR(ramAddr[RAM_LOCKSTATE][nmmx.type])) : 0xFFFF;
	Emulator::Instance()->frameState.A0 = ramAddr[RAM_A0][nmmx.type] ? *LPWORD(Emulator::Instance()->wram + RAM_ADDR(ramAddr[RAM_A0][nmmx.type])) : 0xFFFF;
	Emulator::Instance()->frameState.A2 = ramAddr[RAM_A2][nmmx.type] ? *LPWORD(Emulator::Instance()->wram + RAM_ADDR(ramAddr[RAM_A2][nmmx.type])) : 0xFFFF;
	Emulator::Instance()->frameState.A4 = ramAddr[RAM_A4][nmmx.type] ? *LPWORD(Emulator::Instance()->wram + RAM_ADDR(ramAddr[RAM_A4][nmmx.type])) : 0xFFFF;
	Emulator::Instance()->frameState.A6 = ramAddr[RAM_A6][nmmx.type] ? *LPWORD(Emulator::Instance()->wram + RAM_ADDR(ramAddr[RAM_A6][nmmx.type])) : 0xFFFF;
	Emulator::Instance()->frameState.A8 = ramAddr[RAM_A8][nmmx.type] ? *LPWORD(Emulator::Instance()->wram + RAM_ADDR(ramAddr[RAM_A8][nmmx.type])) : 0xFFFF;
	Emulator::Instance()->frameState.AA = ramAddr[RAM_AA][nmmx.type] ? *LPWORD(Emulator::Instance()->wram + RAM_ADDR(ramAddr[RAM_AA][nmmx.type])) : 0xFFFF;

	//emuWriteOrder.store(true, std::memory_order_release);
	Emulator::Instance()->ReleaseToMain();
	SendNotifyMessage(hWID[0], Emulator::Message::VIDEOREFRESH, (WPARAM)0, (LPARAM)0);
}

void Emulator::AudioSample(int16_t left, int16_t right) {
	if (Emulator::Instance()->snesState >= SnesState::ON) {
		if (!Emulator::Instance()->mute) {
			Emulator::Instance()->audio_sample(left, right);
		}
		else {
			Emulator::Instance()->audio_sample(0, 0);
		}
	}
}

size_t Emulator::AudioSampleBatch(const int16_t *data, size_t frames) {
	for (size_t i = 0; i < frames; ++i) {
		AudioSample(data[i * 2 + 0], data[i * 2 + 1]);
	}
	return frames;
}



//int keys[16]={
//	'5',      //RETRO_DEVICE_ID_JOYPAD_B
//	'7',      //RETRO_DEVICE_ID_JOYPAD_Y
//	'M', //RETRO_DEVICE_ID_JOYPAD_SELECT
//	'O', //RETRO_DEVICE_ID_JOYPAD_START
//	'3',    //RETRO_DEVICE_ID_JOYPAD_UP
//	'4',  //RETRO_DEVICE_ID_JOYPAD_DOWN
//	'1',  //RETRO_DEVICE_ID_JOYPAD_LEFT
//	'2', //RETRO_DEVICE_ID_JOYPAD_RIGHT
//	'6',      //RETRO_DEVICE_ID_JOYPAD_A
//	'8',      //RETRO_DEVICE_ID_JOYPAD_X
//	'9',      //RETRO_DEVICE_ID_JOYPAD_L
//	'N',      //RETRO_DEVICE_ID_JOYPAD_R
//	0,        //not mapped
//	0,        //not mapped
//	0,        //not mapped
//	0,        //not mapped
//};

int keys[16] = {
	RETRO_DEVICE_ID_JOYPAD_B,
	RETRO_DEVICE_ID_JOYPAD_Y,
	RETRO_DEVICE_ID_JOYPAD_SELECT,
	RETRO_DEVICE_ID_JOYPAD_START,
	RETRO_DEVICE_ID_JOYPAD_UP,
	RETRO_DEVICE_ID_JOYPAD_DOWN,
	RETRO_DEVICE_ID_JOYPAD_LEFT,
	RETRO_DEVICE_ID_JOYPAD_RIGHT,
	RETRO_DEVICE_ID_JOYPAD_A,
	RETRO_DEVICE_ID_JOYPAD_X,
	RETRO_DEVICE_ID_JOYPAD_L,
	RETRO_DEVICE_ID_JOYPAD_R,
	0,        //not mapped
	0,        //not mapped
	0,        //not mapped
	0,        //not mapped
};


#define ModifierKeys() /* */ \
	(GetKeyState(VK_CONTROL)>>15 || GetKeyState(VK_SHIFT)>>15 || GetKeyState(VK_MENU)>>15)
#define KeyMap() /* */ \
	for (int i=0;i<16;i++) { MapKey((i), (keys[i])); }

void Emulator::PollController(int16_t * controller)
{
	for (int i = 0; i < 16; i++) controller[i] = 0;
	//	if (ModifierKeys() || Emulator::Instance()->snesState < SnesState::ON) return;
	//#define MapKey(gamepad, keyboard) controller[gamepad]=GetKeyState(keyboard)>>15
	//	KeyMap();
	//#undef MapKey
	JOYINFOEX joyInfoEx;
	ZeroMemory(&joyInfoEx, sizeof(joyInfoEx));
	joyInfoEx.dwSize = sizeof(joyInfoEx);
	joyInfoEx.dwFlags = JOY_RETURNALL;
	BYTE keyState[256];
	JOYCAPS joyCaps;
	ZeroMemory(&joyCaps, sizeof(joyCaps));

	auto joyOk = joyGetPosEx(set.joyNum, &joyInfoEx) == JOYERR_NOERROR;
	joyOk &= joyGetDevCaps(JOYSTICKID1 + set.joyNum, &joyCaps, sizeof(joyCaps)) == JOYERR_NOERROR;
	GetKeyboardState(keyState);

	for (unsigned i = 0; i < 12; i++) {
		unsigned pressed = 0;

		if (set.emulatorButtons[i].type == InternalEmulatorInputType::JOY) {
			//if (i == 4) pressed = (joyInfoEx.dwYpos == 0x0000 || joyInfoEx.dwPOV == 4500 || joyInfoEx.dwPOV == 31500 || joyInfoEx.dwPOV == JOY_POVFORWARD) ? 1 : 0;
			//if (i == 5) pressed = (joyInfoEx.dwYpos == 0xFFFF || joyInfoEx.dwPOV == 13500 || joyInfoEx.dwPOV == 22500 || joyInfoEx.dwPOV == JOY_POVBACKWARD) ? 1 : 0;
			//if (i == 6) pressed = (joyInfoEx.dwXpos == 0x0000 || joyInfoEx.dwPOV == 22500 || joyInfoEx.dwPOV == 31500 || joyInfoEx.dwPOV == JOY_POVLEFT) ? 1 : 0;
			//if (i == 7) pressed = (joyInfoEx.dwXpos == 0xFFFF || joyInfoEx.dwPOV == 4500 || joyInfoEx.dwPOV == 13500 || joyInfoEx.dwPOV == JOY_POVRIGHT) ? 1 : 0;

			if (i >= 4 && i <= 7) {
				switch (set.emulatorButtons[i].value) {
				case 32:
					pressed = (joyInfoEx.dwYpos == 0x0000) ? 1 : 0;
					break;
				case 33:
					pressed = (joyInfoEx.dwYpos == 0xFFFF) ? 1 : 0;
					break;
				case 34:
					pressed = (joyInfoEx.dwXpos == 0x0000) ? 1 : 0;
					break;
				case 35:
					pressed = (joyInfoEx.dwXpos == 0xFFFF) ? 1 : 0;
					break;
				case 36:
					pressed = (joyCaps.wCaps & JOYCAPS_HASPOV) && (joyInfoEx.dwPOV == JOY_POVFORWARD || joyInfoEx.dwPOV == 4500 || joyInfoEx.dwPOV == 31500) ? 1 : 0;
					break;
				case 37:
					pressed = (joyCaps.wCaps & JOYCAPS_HASPOV) && (joyInfoEx.dwPOV == JOY_POVBACKWARD || joyInfoEx.dwPOV == 13500 || joyInfoEx.dwPOV == 22500) ? 1 : 0;
					break;
				case 38:
					pressed = (joyCaps.wCaps & JOYCAPS_HASPOV) && (joyInfoEx.dwPOV == JOY_POVLEFT || joyInfoEx.dwPOV == 22500 || joyInfoEx.dwPOV == 31500) ? 1 : 0;
					break;
				case 39:
					pressed = (joyCaps.wCaps & JOYCAPS_HASPOV) && (joyInfoEx.dwPOV == JOY_POVRIGHT || joyInfoEx.dwPOV == 4500 || joyInfoEx.dwPOV == 13500) ? 1 : 0;
					break;
				default:
					break;
				}
			}

			if (set.emulatorButtons[i].value < 32) pressed = (joyInfoEx.dwButtons & (1 << set.emulatorButtons[i].value)) ? 1 : 0;
		}
		else if (set.emulatorButtons[i].value) {
			pressed = (keyState[MapVirtualKey(set.emulatorButtons[i].value >> 16, MAPVK_VSC_TO_VK_EX)] & 0x80) ? 1 : 0;
		}
		controller[i] = pressed;
	}

}

static int16_t controllerData[16];

#if con_snes>=con_normal
static int X = 0;
#endif

void Emulator::InputPoll() {
	for (int i = 0; i < 16; ++i) controllerData[i] = 0;
	if (Emulator::Instance()->snesState >= SnesState::ON) {
		PollController(controllerData);
	}
}

int16_t Emulator::InputRead(unsigned port, unsigned device, unsigned index, unsigned id) {
	if (port != 0) {
		return 0;
	}
	return controllerData[id];
}

//void Emulator::FrameTime(retro_usec_t t) {
//
//}
