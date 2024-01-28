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

//#include "SC4ED.h"
#include "SC4Core.h"

#include <algorithm>
#include <functional>

#include <vector>
#include <deque>
#include <map>
#include <tuple>
#include <fstream>
#include <string>
#include <sstream>

const char *levelsName[][0x44] = {
	{
		"1-1-1", "1-1-2", "1-2-1", "1-2-2", "1-2-3", "1-3-1", "1-3-2", "1-3-3",
		"2-1-1", "2-2-1", "2-3-1", "2-3-2",
		"3-1-1", "3-2-1", "3-3-1", "3-3-2", "3-3-3", "3-3-4",
		"4-1-1", "4-1-2", "4-1-3", "4-2-1", "4-3-1", "4-4-1",
		"5-1-1", "5-2-1",
		"6-1-1", "6-1-2", "6-2-1", "6-2-2", "6-2-3", "6-3-1", "6-3-2", "6-3-3", "6-3-4",
		"7-1-1", "7-1-2", "7-1-3", "7-2-1", "7-2-2", "7-2-3", "7-2-4",
		"8-1-1", "8-1-2", "8-2-1", "8-2-2",
		"9-1-1", "9-1-2", "9-1-3", "9-2-1", "9-2-2", "9-2-3", "9-2-4", "9-2-5", "9-2-6",
		"A-1-1", "A-1-2", "A-1-3", "A-1-4", "A-2-1",
		"B-1-1", "B-1-2", "B-2-1", "B-3-1", "B-3-2", "B-3-3", "B-4-1", "B-4-2",
	},
	{
		"Stage 1",
		"Stage 2",
		"Stage 3",
		"Stage 4",
		"Stage 5",
		"Stage 6",
	},
	{
		"Stage 1",
		"Stage 2",
		"Stage 3",
		"Stage 4",
		"Stage 5",
		"Stage 6",
		"Stage 7",
		"Stage 8",
		"Stage 9",
		"Stage 10",
		"Stage 11",
		"Stage 12",
		"Stage 13",
		"Stage 14",
		"Stage 15",
	},

};

static const UINT32	crc32Table[256] =
{
	0x00000000, 0x77073096, 0xee0e612c, 0x990951ba, 0x076dc419, 0x706af48f,
	0xe963a535, 0x9e6495a3, 0x0edb8832, 0x79dcb8a4, 0xe0d5e91e, 0x97d2d988,
	0x09b64c2b, 0x7eb17cbd, 0xe7b82d07, 0x90bf1d91, 0x1db71064, 0x6ab020f2,
	0xf3b97148, 0x84be41de, 0x1adad47d, 0x6ddde4eb, 0xf4d4b551, 0x83d385c7,
	0x136c9856, 0x646ba8c0, 0xfd62f97a, 0x8a65c9ec, 0x14015c4f, 0x63066cd9,
	0xfa0f3d63, 0x8d080df5, 0x3b6e20c8, 0x4c69105e, 0xd56041e4, 0xa2677172,
	0x3c03e4d1, 0x4b04d447, 0xd20d85fd, 0xa50ab56b, 0x35b5a8fa, 0x42b2986c,
	0xdbbbc9d6, 0xacbcf940, 0x32d86ce3, 0x45df5c75, 0xdcd60dcf, 0xabd13d59,
	0x26d930ac, 0x51de003a, 0xc8d75180, 0xbfd06116, 0x21b4f4b5, 0x56b3c423,
	0xcfba9599, 0xb8bda50f, 0x2802b89e, 0x5f058808, 0xc60cd9b2, 0xb10be924,
	0x2f6f7c87, 0x58684c11, 0xc1611dab, 0xb6662d3d, 0x76dc4190, 0x01db7106,
	0x98d220bc, 0xefd5102a, 0x71b18589, 0x06b6b51f, 0x9fbfe4a5, 0xe8b8d433,
	0x7807c9a2, 0x0f00f934, 0x9609a88e, 0xe10e9818, 0x7f6a0dbb, 0x086d3d2d,
	0x91646c97, 0xe6635c01, 0x6b6b51f4, 0x1c6c6162, 0x856530d8, 0xf262004e,
	0x6c0695ed, 0x1b01a57b, 0x8208f4c1, 0xf50fc457, 0x65b0d9c6, 0x12b7e950,
	0x8bbeb8ea, 0xfcb9887c, 0x62dd1ddf, 0x15da2d49, 0x8cd37cf3, 0xfbd44c65,
	0x4db26158, 0x3ab551ce, 0xa3bc0074, 0xd4bb30e2, 0x4adfa541, 0x3dd895d7,
	0xa4d1c46d, 0xd3d6f4fb, 0x4369e96a, 0x346ed9fc, 0xad678846, 0xda60b8d0,
	0x44042d73, 0x33031de5, 0xaa0a4c5f, 0xdd0d7cc9, 0x5005713c, 0x270241aa,
	0xbe0b1010, 0xc90c2086, 0x5768b525, 0x206f85b3, 0xb966d409, 0xce61e49f,
	0x5edef90e, 0x29d9c998, 0xb0d09822, 0xc7d7a8b4, 0x59b33d17, 0x2eb40d81,
	0xb7bd5c3b, 0xc0ba6cad, 0xedb88320, 0x9abfb3b6, 0x03b6e20c, 0x74b1d29a,
	0xead54739, 0x9dd277af, 0x04db2615, 0x73dc1683, 0xe3630b12, 0x94643b84,
	0x0d6d6a3e, 0x7a6a5aa8, 0xe40ecf0b, 0x9309ff9d, 0x0a00ae27, 0x7d079eb1,
	0xf00f9344, 0x8708a3d2, 0x1e01f268, 0x6906c2fe, 0xf762575d, 0x806567cb,
	0x196c3671, 0x6e6b06e7, 0xfed41b76, 0x89d32be0, 0x10da7a5a, 0x67dd4acc,
	0xf9b9df6f, 0x8ebeeff9, 0x17b7be43, 0x60b08ed5, 0xd6d6a3e8, 0xa1d1937e,
	0x38d8c2c4, 0x4fdff252, 0xd1bb67f1, 0xa6bc5767, 0x3fb506dd, 0x48b2364b,
	0xd80d2bda, 0xaf0a1b4c, 0x36034af6, 0x41047a60, 0xdf60efc3, 0xa867df55,
	0x316e8eef, 0x4669be79, 0xcb61b38c, 0xbc66831a, 0x256fd2a0, 0x5268e236,
	0xcc0c7795, 0xbb0b4703, 0x220216b9, 0x5505262f, 0xc5ba3bbe, 0xb2bd0b28,
	0x2bb45a92, 0x5cb36a04, 0xc2d7ffa7, 0xb5d0cf31, 0x2cd99e8b, 0x5bdeae1d,
	0x9b64c2b0, 0xec63f226, 0x756aa39c, 0x026d930a, 0x9c0906a9, 0xeb0e363f,
	0x72076785, 0x05005713, 0x95bf4a82, 0xe2b87a14, 0x7bb12bae, 0x0cb61b38,
	0x92d28e9b, 0xe5d5be0d, 0x7cdcefb7, 0x0bdbdf21, 0x86d3d2d4, 0xf1d4e242,
	0x68ddb3f8, 0x1fda836e, 0x81be16cd, 0xf6b9265b, 0x6fb077e1, 0x18b74777,
	0x88085ae6, 0xff0f6a70, 0x66063bca, 0x11010b5c, 0x8f659eff, 0xf862ae69,
	0x616bffd3, 0x166ccf45, 0xa00ae278, 0xd70dd2ee, 0x4e048354, 0x3903b3c2,
	0xa7672661, 0xd06016f7, 0x4969474d, 0x3e6e77db, 0xaed16a4a, 0xd9d65adc,
	0x40df0b66, 0x37d83bf0, 0xa9bcae53, 0xdebb9ec5, 0x47b2cf7f, 0x30b5ffe9,
	0xbdbdf21c, 0xcabac28a, 0x53b39330, 0x24b4a3a6, 0xbad03605, 0xcdd70693,
	0x54de5729, 0x23d967bf, 0xb3667a2e, 0xc4614ab8, 0x5d681b02, 0x2a6f2b94,
	0xb40bbe37, 0xc30c8ea1, 0x5a05df1b, 0x2d02ef8d
};

const char SC4Core::expandedROMString[] = "EXPANDED ROM "; // 13 characters + termination
const WORD SC4Core::expandedROMVersion = 4;
const unsigned SC4Core::expandedROMHeaderSize = 0x40;

const WORD SC4Core::levelFormatVersion = 2;

#define ReadByte(offset) (*((BYTE*)(rom+(offset))))
#define ReadWord(offset) (*((WORD*)(rom+(offset))))
#define ReadDWord(offset) (*((DWORD*)(rom+(offset))))
#define SReadByte(offset) (*((BYTE*)(rom+SNESCore::snes2pc(offset))))
#define SReadWord(offset) (*((WORD*)(rom+SNESCore::snes2pc(offset))))
#define SReadDWord(offset) (*((DWORD*)(rom+SNESCore::snes2pc(offset))))
#define SReadLpByte(offset) (((LPBYTE)(rom+SNESCore::snes2pc(offset))))

const long p_layout[3] = {0x868D24, 0x868888, 0x8689B3};
const long p_scenes[3] = {0x868D93, 0x8688F7, 0x868A22};
const long p_blocks[3] = {0x868E02, 0x868966, 0x868A91};
const long p_maps  [3] = {0x868E71, 0x8689D5, 0x868B00};
const long p_collis[3] = {0x868EE0, 0x868A44, 0x868B6F};
const long p_checkp[3] = {0x86A780, 0x86A4C5, 0x86A8E4};
const long p_palett[3] = {0x868133, 0x86817A, 0x868180};
const long p_font  [3] = {0x878dc9, 0x889D7E, 0x868000};
const long p_unknow[3] = {0x86A1D5,     NULL,     NULL}; // Unknow
const long p_gfxcfg[3] = {0x86F56F, 0x86F831, 0x86F3C3};
const long p_gfxpos[3] = {0x86F6F7,	0x86F9FF, 0x86F730};
const long p_events[3] = {0x80D81B, 0x85947C, 0x81AE9A};
const long p_borders[3]= {NULL, 0x82EBE9, 0x83DE43};
const long p_locks[3]  = {0x81B783, 0x82FAE4, 0x83F2CC};
//const long p_properties[3] = {0x80F8F3, 0x80FB8E, 0x86E28E};
const long p_properties[3] = { NULL, NULL, 0x86E28E };
const long p_spriteAssembly[3] = { 0x8D8000, 0x8D8000, 0x8D8000 };
const long p_spriteOffset[3] = { 0x86A5E4, 0x86A34D, 0x86E28E };
const long p_objOffset[3] = { 0x86DE9B, 0x86A34D, NULL };

// enemy
const long p_gfxobj[3] = {0x86ACEE, 0xAAB2D4, 0x888623};
const long p_gfxpal[3] = {0x86ACF1, 0xAAB2D7, 0x888626};
// TODO: sprite
// TODO: misc object

// capsule
const long p_capsulepos[3] = {NULL,0x86D6F1,NULL};

const long p_blayout[3] = {0x868F4F, 0x868AB3, 0x868BDE};
const long p_bscenes[3] = {0x868FBE, 0x868B22, 0x868C4D};
const long p_bblocks[3] = {0x86902D, 0x868B91, 0x868CBC};

// collusionByteNames
std::map<unsigned, std::string> TileTypeMap = {
	{ 0x00, "All" },
	{ 0xC8, "Empty" },
	{ 0xCA, "Platform(FG?)" },
	{ 0xCC, "Step(FG?)" },
	{ 0xCE, "Platform(BG?)" },
	{ 0xD0, "Platform Half Height" },
	{ 0xD2, "?????" }, // level 24 special?
	{ 0xD4, "Mud/Slime/Blood" },
	{ 0xD6, "?????" },
	{ 0xD8, "Slope 15deg Change (gold pile)" },
	{ 0xDA, "Slope 30deg Low (2-1-1)" }, // platform rounded edge?
	{ 0xDC, "Slope 15deg Straight (gold pile)" },
	{ 0xDE, "Slope 30deg High (2-1-1)" },
	{ 0xE0, "Step(BG?)" },
	{ 0xE2, "Spike" },
	{ 0xE4, "Background" },
};	

struct sTileInfo {
	unsigned num = 0;
	BYTE value = 0;
	std::map<BYTE, unsigned> count;
};

static bool TileSort(const sTileInfo &a, const sTileInfo &b) {
	return a.value < b.value;
}

WORD SC4Core::GetTileType(WORD tileNum, int levelNum) {
	WORD type = 0;
	unsigned l = (levelNum == -1 ? level : levelNum);

	if (!expandedROM) {
		WORD offset = *LPWORD(rom + SNESCore::snes2pc(0x85C2D0) + 2 * l);
		WORD minValue = *LPWORD(rom + SNESCore::snes2pc(0x850000 + offset + (0xC8 - 0xC8)));
		WORD maxValue = *LPWORD(rom + SNESCore::snes2pc(0x850000 + offset + (0xE4 - 0xC8)));


		if (tileNum < minValue) {
			type = 0xC8;
		}
		else if (tileNum >= maxValue) {
			type = 0xE4;
		}
		else {
			for (unsigned i = 4; i < 0x1E; i += 2) {
				WORD value = *LPWORD(rom + SNESCore::snes2pc(0x850000 + offset + i));
				if (tileNum < value) {
					type = 0xC6 + i;
					break;
				}
			}
		}
	}
	else {
		type = *(rom + snes2pc(0xA28000) + (l % 0x20) * 0x400 + (l / 0x20) * 0x8000 + tileNum) * 2 + 0xC8;
	}

	return type;
}

// collusinMaps2rom 
void SC4Core::SetTileType(WORD tileNum, WORD type, int levelNum) {
	unsigned l = (levelNum == -1 ? level : levelNum);

	if (expandedROM) {
		*(rom + snes2pc(0xA28000) + (l % 0x20) * 0x400 + (l / 0x20) * 0x8000 + tileNum) = type;
	}
}

// camLockTable2rom
CameraLockInfo &SC4Core::GetCameraLock(unsigned subId) {
	return *(CameraLockInfo *)(rom + snes2pc(0xA58000 + 0x20 * 0xC * level + 0xC * subId + 0));
}

// exitTable2rom
LevelExitInfo &SC4Core::GetLevelExit(unsigned subId, int levelNum) {
	unsigned l = (levelNum == -1 ? level : levelNum);

	return *(LevelExitInfo *)(rom + snes2pc(0xA68000 + 0x40 * 0x4 * l + 0x4 * subId + 0));
}

// transitTable2rom
LevelTransitionInfo &SC4Core::GetLevelTransition(unsigned exit, int levelNum) {
	unsigned l = (levelNum == -1 ? level : levelNum);

	return *(LevelTransitionInfo *)(rom + snes2pc(0xA0C000 + 0x10 * l + 0x2 * exit));
}

// entranceTable2rom
LevelEntranceInfo &SC4Core::GetLevelEntrance(unsigned entrance, int levelNum) {
	unsigned l = (levelNum == -1 ? level : levelNum);

	return *(LevelEntranceInfo *)(rom + snes2pc(0xA78000 + 0x100 * l + 0x20 * entrance));
}


void SC4Core::GenerateExpandedOffset() {
	expandedOffset.clear();

	// re-add the expanded offsets
	std::vector<DWORD> addrList;
	BYTE buf[0x10000];
	ZeroMemory(buf, sizeof(buf));

	// levels
	if (type == 0) {
		for (unsigned i = 0; i < numLevels; i++) {
			DWORD addr;
			////addrList.push_back(0x81B5D9); // bg tile map
			//addrList.push_back(0x86889C); // bg tiles
			addr = 0x86 << 16 | *LPWORD(rom + SNESCore::snes2pc(0x86889C) + (i * 2));
			addrList.push_back(addr);
			//addrList.push_back(0x868B45); // bg tiles
			addr = 0x86 << 16 | *LPWORD(rom + SNESCore::snes2pc(0x868B45) + (i * 2));
			addrList.push_back(addr);
			//addrList.push_back(0x868000); // bg map indices
			addr = 0x86 << 16 | *LPWORD(rom + SNESCore::snes2pc(0x868000) + (i * 2));
			addrList.push_back(addr);
			//addrList.push_back(0x86831E);
			addr = 0x86 << 16 | *LPWORD(rom + SNESCore::snes2pc(0x86831e) + (i * 2));
			addrList.push_back(addr);
		}

		addrList.push_back(region == 0 ? 0x81B3E2 : 0x81B3B6); // default whip, powerup, candle, orb, etc
	}
	else if (type == 1) {
		for (unsigned i = 0; i < numLevels; i++) {
			DWORD addr;
			//// C3
			// ???
			if (i < numLevels - 1) {
				addr = 0x85 << 16 | *LPWORD(rom + SNESCore::snes2pc(0x8587F1) + ((i + 1) * 2));
				addrList.push_back(addr);
			}
			// ???
			addr = 0x85 << 16 | *LPWORD(rom + SNESCore::snes2pc(0x8588E1) + ((i + 1) * 2));
			addrList.push_back(addr);
			// ???
			addr = 0x85 << 16 | *LPWORD(rom + SNESCore::snes2pc(0x8588ED) + ((i + 1) * 2));
			addrList.push_back(addr);

			addr = 0x85 << 16 | *LPWORD(rom + SNESCore::snes2pc(0x858A0D) + (i * 2));
			addrList.push_back(addr);
		}
	}
	else if (type == 2) {

		for (unsigned i = 0; i < numLevels; i++) {
			DWORD addr;

			// X (0)
			addr = 0x81 << 16 | *LPWORD(rom + SNESCore::snes2pc(0x81AADA) + (i * 2));
			addrList.push_back(addr);

			// not loaded on level 0
			//addr = 0x81 << 16 | *LPWORD(rom + SNESCore::snes2pc(0x81AEB8) + (level * 2));
			//addrList.push_back(addr);

			//addr = 0x81 << 16 | *LPWORD(rom + SNESCore::snes2pc(0x81A22A) + (level * 2));
			//addrList.push_back(addr);

			// X (2)
			addr = 0x81 << 16 | *LPWORD(rom + SNESCore::snes2pc(0x81A24C) + ((i + 1) * 2));
			addrList.push_back(addr);

			// X (2)
			// FIXME: only 0x5 of these
			addr = 0x81 << 16 | *LPWORD(rom + SNESCore::snes2pc(0x81A254) + ((0 + 1) * 2));
			addrList.push_back(addr);

			// X (0)
			// FIXME: only 0xA of these
			addr = 0x81 << 16 | *LPWORD(rom + SNESCore::snes2pc(0x81A25E) + (i * 2));
			addrList.push_back(addr);

			//addr = 0x81 << 16 | *LPWORD(rom + SNESCore::snes2pc(0x818937) + (level * 2));
			//addrList.push_back(addr);
			// G3
		}

		addrList.push_back(0x8183B2); // dynamic tiles
	}

	std::map<DWORD, DWORD> addrMap;

	unsigned count = 0;
	for (auto a : addrList) {
		DWORD addr = a;
		if ((addr & 0xFFFF) != 0xFFFF) {
			LPBYTE gfxPtr = (rom + SNESCore::snes2pc(addr));

			// FIXME: the actual check is against 0?
			if (*LPWORD(gfxPtr) != 0xFFFF) {
				bool oneAddr = *gfxPtr == 2;
				unsigned addrBytes = (*gfxPtr == 0 || *gfxPtr == 2 || *gfxPtr == 3) ? 2 : 3;
				//if (*gfxPtr == 7) return;
				gfxPtr += 2;

				while (*LPWORD(gfxPtr) != 0xFFFF) {
					DWORD dstAddr = *LPDWORD(gfxPtr) & (addrBytes == 3 ? 0xFFFFFF : 0xFFFF);
					WORD dstOffset = dstAddr & 0xFFFF;
					gfxPtr += addrBytes;
					addr = *LPDWORD(gfxPtr) & 0xFFFFFF;
					gfxPtr += 3;;

					WORD size = *LPWORD(rom + SNESCore::snes2pc(addr));
					addr += 2;

					auto origSize = GFXRLE(rom, buf, SNESCore::snes2pc(addr), size, type);
					if (origSize) {
						expandedOffset[count >> 2][dstAddr].first = SNESCore::snes2pc(addr) + 1;
						expandedOffset[count >> 2][dstAddr].second = origSize;
					}

					if (oneAddr) { break; }
				}
			}
		}
		count++;
	}

}

BYTE SC4Core::CheckROM()
{
	LoadHeader();
	LPBYTE expandedHeader = nullptr;
	expandedROM = false;
	expandedOffset.clear();

	if (!strncmp(header->title, "SUPER CASTLEVANIA 4", strlen("SUPER CASTLEVANIA 4"))) {
		type = 0;
		region = 0;
		numLevels = 0x44;
		eventBank = 0x85;
		eventOffsetLimit = 0xBCF8;
	}
	else if (!strncmp(header->title, "AKUMAJO DRACULA", strlen("AKUMAJO DRACULA"))) {
		type = 0;
		region = 1;
		numLevels = 0x44;
		eventBank = 0x85;
		eventOffsetLimit = 0xBCF8;
		// FIXME: check for expandedROM and reload expandedOffset
	}
	else if (!strncmp(header->title, "CONTRA3 THE ALIEN WARS", strlen("CONTRA3 THE ALIEN WARS"))) {
		type = 1;
		region = 0;
		numLevels = 0x6;
		eventBank = 0x85;
		eventOffsetLimit = 0x0000;
	}
	else if (!strncmp(header->title, "gradius 3", strlen("gradius 3"))) {
		type = 2;
		region = 0;
		numLevels = 0xA;
		eventBank = 0x81;
		eventOffsetLimit = 0xD5D8;
	}
	else {
		type = 0xFF;
	}

	if (type != 0xFF) {
		expandedROM = (header->romSize == 0xC) && (romSize == 0x400000) && strcmp("EXPANDED ROM  ", (char *)(rom + 0x180000 + 0x8000 - expandedROMHeaderSize));
		if (expandedROM) {
			expandedHeader = rom + 0x100000 + 0x8000 - expandedROMHeaderSize;

			GenerateExpandedOffset();

			// update events location
			if (*LPWORD(expandedHeader + 0xE) >= 3 && type != 1) {
				eventBank = 0xA1;
				eventOffsetLimit = 0x10000;
			}
		}
	}

	return type + 1;
}

bool SC4Core::GetBaseAddr(unsigned mode, DWORD addr, DWORD &baseAddr, WORD &newSize) {
	baseAddr = addr;
	newSize = 0;
	bool found = false;

	if (type == 0) {
		switch (mode) {
			// multiple levels share a partial set of tiles making this painful to setup.
		case 0x0000:
			// 2 bytes
			// TODO: figure out which one is VRAM
			if (0x2000 <= addr && addr < 0x4000) {
				baseAddr = 0x2000;
				newSize = 0x4000;
				found = true;
			}
			break;
			//case 0x0000:
		case 0x0002:
		case 0x0003:
		case 0xA001:
			break;
		default:
			// 3 bytes
			if (0x7E6000 <= addr && addr < 0x7E8000) {
				baseAddr = 0x7E6000;
				newSize = 0x2000;
				found = true;
			}
			else if (0x7E8000 <= addr && addr < 0x7EA000) {
				baseAddr = 0x7E8000;
				newSize = 0x2000;
				found = true;
			}
			else if (0x7EC000 <= addr && addr < 0x7EE000) {
				baseAddr = 0x7EC000;
				newSize = 0x2000;
				found = true;
			}
			else if (0x7EE000 <= addr && addr < 0x7F0000) {
				baseAddr = 0x7EE000;
				newSize = 0x2000;
				found = true;
			}
			// Simon needs to span multiple banks so skip combining for now
			// Should be as simple as defining multiple ranges, but having to split
			// into 3 regions so a bank isn't crossed is not ideal.  May be able to support
			// bank crossing in decompression code expanded ROM.
			//else if (0x7F0000 <= addr && addr < 0x7FE000) {
			//	baseAddr = 0x7EE000;
			//	newSize = 0x2000;
			//	found = true;
			//}
			break;
		}
	}
	else if (type == 1) {
		switch (mode) {
			// multiple levels share a partial set of tiles making this painful to setup.
		case 0x0000:
			// 2 bytes
			// TODO: figure out which one is VRAM
			if (0x3000 <= addr && addr < 0x4000) {
				baseAddr = 0x3000;
				newSize = 0x2000;
				found = true;
			}
			break;
			//case 0x0000:
		case 0x0002:
		case 0x0003:
		case 0xA001:
			break;
		default:
			// 3 bytes
			//if (0x7E6000 <= addr && addr < 0x7E8000) {
			//	baseAddr = 0x7E6000;
			//	newSize = 0x2000;
			//	found = true;
			//}
			//else if (0x7E8000 <= addr && addr < 0x7EA000) {
			//	baseAddr = 0x7E8000;
			//	newSize = 0x2000;
			//	found = true;
			//}
			//else if (0x7EC000 <= addr && addr < 0x7EE000) {
			//	baseAddr = 0x7EC000;
			//	newSize = 0x2000;
			//	found = true;
			//}
			//else if (0x7EE000 <= addr && addr < 0x7F0000) {
			//	baseAddr = 0x7EE000;
			//	newSize = 0x2000;
			//	found = true;
			//}
			if (0x7E5000 <= addr && addr < 0x7E7000) {
				baseAddr = 0x7E5000;
				newSize = 0x2000;
				found = true;
			}
			if (0x7E7000 <= addr && addr < 0x7E9000) {
				baseAddr = 0x7E7000;
				newSize = 0x2000;
				found = true;
			}
			//if (0x7EB000 <= addr && addr < 0x7ED000) {
			//	baseAddr = 0x7EB000;
			//	newSize = 0x2000;
			//	found = true;
			//}
			if (0x7F0000 <= addr && addr < 0x7F8000) {
				baseAddr = 0x7F0000;
				newSize = 0x8000;
				found = true;
			}
			if (0x7F8000 <= addr && addr < 0x800000) {
				baseAddr = 0x7F8000;
				newSize = 0x8000;
				found = true;
			}
			break;
		}
	}
	else if (type == 2) { // FIXME:
		switch (mode) {
			// multiple levels share a partial set of tiles making this painful to setup.
		case 0x0000:
			// 2 bytes
			// TODO: figure out which one is VRAM
			//if (0x3000 <= addr && addr < 0x4000) {
			//	baseAddr = 0x3000;
			//	newSize = 0x2000;
			//	found = true;
			//}
			break;
			//case 0x0000:
		case 0x0002:
		case 0x0003:
		case 0xA001:
			break;
		default:
			// 3 bytes
			//if (0x7E6000 <= addr && addr < 0x7E8000) {
			//	baseAddr = 0x7E6000;
			//	newSize = 0x2000;
			//	found = true;
			//}
			//else if (0x7E8000 <= addr && addr < 0x7EA000) {
			//	baseAddr = 0x7E8000;
			//	newSize = 0x2000;
			//	found = true;
			//}
			//else if (0x7EC000 <= addr && addr < 0x7EE000) {
			//	baseAddr = 0x7EC000;
			//	newSize = 0x2000;
			//	found = true;
			//}
			//else if (0x7EE000 <= addr && addr < 0x7F0000) {
			//	baseAddr = 0x7EE000;
			//	newSize = 0x2000;
			//	found = true;
			//}
			if (0x7EA000 <= addr && addr < 0x7EB000) {
				baseAddr = 0x7EA000;
				newSize = 0x1000;
				found = true;
			}
			if (0x7EB000 <= addr && addr < 0x7EC000) {
				baseAddr = 0x7EB000;
				newSize = 0x1000;
				found = true;
			}
			if (0x7EE000 <= addr && addr < 0x7EF000) {
				baseAddr = 0x7EE000;
				newSize = 0x1000;
				found = true;
			}
			if (0x7EF000 <= addr && addr < 0x7F0000) {
				baseAddr = 0x7EF000;
				newSize = 0x1000;
				found = true;
			}
			// maybe to 0x7FC0000?
			if (0x7F0000 <= addr && addr < 0x7FB9C0) {
				baseAddr = 0x7F0000;
				newSize = 0xB9C0;
				found = true;
			}
			break;
		}
	}

	return found;
}

bool SC4Core::ExpandROM() {
	// Check if it's the original ROM
	// 1) total ROM size
	// 2) checksum
	if (header->romSize != (type == 2 ? 0x9 : 0xA) || region != 0x0) {
		return false;
	}

	// expand ROM to 4MB
	header->romSize = 0xC;
	romSize = 0x400000;

	memset(rom + (type == 2 ? 0x80000 : 0x100000), 0xFF, romSize - (type == 2 ? 0x80000 : 0x100000));

	LPBYTE expandedRomHeaderAddress = rom + 0x100000 + 0x8000 - expandedROMHeaderSize;
	strcpy((char*)(expandedRomHeaderAddress), expandedROMString);
	expandedRomHeaderAddress += 0xE;
	memcpy(expandedRomHeaderAddress, &expandedROMVersion, 2);
	expandedRomHeaderAddress += 0x2;
	memset(expandedRomHeaderAddress, 0x00, 2);
	unsigned zeroCompressedAddr = SNESCore::pc2snes(expandedRomHeaderAddress - rom);
	expandedRomHeaderAddress += 0x2;

	unsigned romAddr = 0x108000;

	if (type == 0) {
		// expand events
		for (unsigned i = 0; i < numLevels; i++) {
			DWORD pEvents = snes2pc(SReadWord(p_events[type] + i * 2) | (eventBank << 16));
			LPBYTE pevent = rom + pEvents;
			auto oldpevent = pevent;

			// fix table
			*LPWORD(rom + snes2pc(p_events[type] + i * 2)) = romAddr - 0x100000;

			while (*LPDWORD(pevent) != 0xFFFFFFFF) {
				memcpy(rom + romAddr, pevent, 6);
				memset(pevent, 0xFF, 6);

				// get the next event
				pevent += 6;
				romAddr += 6;
			}

			// double event byte count + terminator
			// just terminator.  allow addevent/deleteevent to take care of updates
			unsigned count = /*(pevent - oldpevent) + */8;
			memset(rom + romAddr, 0xFF, count);
			romAddr += count;
		}
		// make sure we didn't use too much space
		if (romAddr >= 0x110000) return false;

		// fix the bank addresses
		eventBank = 0xA1;
		*LPBYTE(rom + snes2pc(0x80D42B) + 0) = eventBank;
		*LPBYTE(rom + snes2pc(0x80D4DA) + 0) = eventBank;
		*LPBYTE(rom + snes2pc(0x80D76A) + 0) = eventBank;
		eventOffsetLimit = 0x10000;

		romAddr = 0x110000;
		// <COLLISIONS> Use 3 banks
		for (unsigned i = 0; i < numLevels; i++) {
			for (unsigned t = 0; t < 0x400; t++) {
				*(rom + romAddr + (i % 0x20) * 0x400 + (i / 0x20) * 0x8000 + t) = (GetTileType(t, i) - 0xC8) / 2;
			}
		}
		DWORD a = 0;

		// replace collision setup with table base setup in $C8
		a = 0x86B859;
		// LDA #2
		*LPBYTE(rom + snes2pc(a++)) = 0xA9;
		*LPBYTE(rom + snes2pc(a++)) = 0x02;
		*LPBYTE(rom + snes2pc(a++)) = 0x00;
		// STA $CC
		*LPBYTE(rom + snes2pc(a++)) = 0x85;
		*LPBYTE(rom + snes2pc(a++)) = 0xCC;
		// INC
		*LPBYTE(rom + snes2pc(a++)) = 0x1A;
		// STA $CE
		*LPBYTE(rom + snes2pc(a++)) = 0x85;
		*LPBYTE(rom + snes2pc(a++)) = 0xCE;
		// INC
		*LPBYTE(rom + snes2pc(a++)) = 0x1A;
		// STA $D0
		*LPBYTE(rom + snes2pc(a++)) = 0x85;
		*LPBYTE(rom + snes2pc(a++)) = 0xD0;
		// INC
		*LPBYTE(rom + snes2pc(a++)) = 0x1A;
		// STA $D2
		*LPBYTE(rom + snes2pc(a++)) = 0x85;
		*LPBYTE(rom + snes2pc(a++)) = 0xD2;
		// INC
		*LPBYTE(rom + snes2pc(a++)) = 0x1A;
		// STA $D4
		*LPBYTE(rom + snes2pc(a++)) = 0x85;
		*LPBYTE(rom + snes2pc(a++)) = 0xD4;
		// INC
		*LPBYTE(rom + snes2pc(a++)) = 0x1A;
		// STA $D6
		*LPBYTE(rom + snes2pc(a++)) = 0x85;
		*LPBYTE(rom + snes2pc(a++)) = 0xD6;
		// INC
		*LPBYTE(rom + snes2pc(a++)) = 0x1A;
		// STA $D8
		*LPBYTE(rom + snes2pc(a++)) = 0x85;
		*LPBYTE(rom + snes2pc(a++)) = 0xD8;
		// INC
		*LPBYTE(rom + snes2pc(a++)) = 0x1A;
		// STA $DA
		*LPBYTE(rom + snes2pc(a++)) = 0x85;
		*LPBYTE(rom + snes2pc(a++)) = 0xDA;
		// INC
		*LPBYTE(rom + snes2pc(a++)) = 0x1A;
		// STA $DC
		*LPBYTE(rom + snes2pc(a++)) = 0x85;
		*LPBYTE(rom + snes2pc(a++)) = 0xDC;
		// INC
		*LPBYTE(rom + snes2pc(a++)) = 0x1A;
		// STA $DE
		*LPBYTE(rom + snes2pc(a++)) = 0x85;
		*LPBYTE(rom + snes2pc(a++)) = 0xDE;
		// INC
		*LPBYTE(rom + snes2pc(a++)) = 0x1A;
		// STA $E0
		*LPBYTE(rom + snes2pc(a++)) = 0x85;
		*LPBYTE(rom + snes2pc(a++)) = 0xE0;
		// INC
		*LPBYTE(rom + snes2pc(a++)) = 0x1A;
		// STA $E2
		*LPBYTE(rom + snes2pc(a++)) = 0x85;
		*LPBYTE(rom + snes2pc(a++)) = 0xE2;
		// INC
		*LPBYTE(rom + snes2pc(a++)) = 0x1A;
		// STA $E4
		*LPBYTE(rom + snes2pc(a++)) = 0x85;
		*LPBYTE(rom + snes2pc(a++)) = 0xE4;

		// LDA $86
		*LPBYTE(rom + snes2pc(a++)) = 0xA5;
		*LPBYTE(rom + snes2pc(a++)) = 0x86;
		// AND #001F
		*LPBYTE(rom + snes2pc(a++)) = 0x29;
		*LPBYTE(rom + snes2pc(a++)) = 0x1F;
		*LPBYTE(rom + snes2pc(a++)) = 0x00;
		// XBA
		*LPBYTE(rom + snes2pc(a++)) = 0xEB;
		// ASL
		*LPBYTE(rom + snes2pc(a++)) = 0x0A;
		// ASL
		*LPBYTE(rom + snes2pc(a++)) = 0x0A;
		// CLC
		*LPBYTE(rom + snes2pc(a++)) = 0x18;
		// ADC #8000
		*LPBYTE(rom + snes2pc(a++)) = 0x69;
		*LPBYTE(rom + snes2pc(a++)) = 0x00;
		*LPBYTE(rom + snes2pc(a++)) = 0x80;
		// STA $C8
		*LPBYTE(rom + snes2pc(a++)) = 0x85;
		*LPBYTE(rom + snes2pc(a++)) = 0xC8;
		// LDA $86
		*LPBYTE(rom + snes2pc(a++)) = 0xA5;
		*LPBYTE(rom + snes2pc(a++)) = 0x86;
		// LSR
		*LPBYTE(rom + snes2pc(a++)) = 0x4A;
		// LSR
		*LPBYTE(rom + snes2pc(a++)) = 0x4A;
		// LSR
		*LPBYTE(rom + snes2pc(a++)) = 0x4A;
		// LSR
		*LPBYTE(rom + snes2pc(a++)) = 0x4A;
		// LSR
		*LPBYTE(rom + snes2pc(a++)) = 0x4A;
		// CLC
		*LPBYTE(rom + snes2pc(a++)) = 0x18;
		// ADC #00A2
		*LPBYTE(rom + snes2pc(a++)) = 0x69;
		*LPBYTE(rom + snes2pc(a++)) = 0xA2;
		*LPBYTE(rom + snes2pc(a++)) = 0x00;
		// STA $CA
		*LPBYTE(rom + snes2pc(a++)) = 0x85;
		*LPBYTE(rom + snes2pc(a++)) = 0xCA;
		// RTL
		*LPBYTE(rom + snes2pc(a++)) = 0x6B;
		while (a < 0x86B8B3) {
			*LPBYTE(rom + snes2pc(a++)) = 0xFF;
		}

		// FIX first collision table
		a = 0x86A3B3;
		// PHY
		*LPBYTE(rom + snes2pc(a++)) = 0x5A;
		// TAY
		*LPBYTE(rom + snes2pc(a++)) = 0xA8;
		// LDA [$C8],y
		*LPBYTE(rom + snes2pc(a++)) = 0xB7;
		*LPBYTE(rom + snes2pc(a++)) = 0xC8;
		// PLY
		*LPBYTE(rom + snes2pc(a++)) = 0x7A;
		// AND #00FF
		*LPBYTE(rom + snes2pc(a++)) = 0x29;
		*LPBYTE(rom + snes2pc(a++)) = 0xFF;
		*LPBYTE(rom + snes2pc(a++)) = 0x00;
		// CMP #2
		*LPBYTE(rom + snes2pc(a++)) = 0xC9;
		*LPBYTE(rom + snes2pc(a++)) = 0x02;
		*LPBYTE(rom + snes2pc(a++)) = 0x00;
		// BCC
		*LPBYTE(rom + snes2pc(a++)) = 0x90;
		*LPBYTE(rom + snes2pc(a++)) = 0x73;
		// CMP $E4
		*LPBYTE(rom + snes2pc(a++)) = 0xC5;
		*LPBYTE(rom + snes2pc(a++)) = 0xE4;
		// BEQ
		*LPBYTE(rom + snes2pc(a++)) = 0xF0;
		*LPBYTE(rom + snes2pc(a++)) = 0x68;

		// (2-3)
		// CMP #4
		*LPBYTE(rom + snes2pc(a++)) = 0xC9;
		*LPBYTE(rom + snes2pc(a++)) = 0x04;
		*LPBYTE(rom + snes2pc(a++)) = 0x00;
		// BCC #XOR
		*LPBYTE(rom + snes2pc(a++)) = 0x90;
		*LPBYTE(rom + snes2pc(a++)) = 0x68; // FIXME
		// (4-5)
		// CMP #6
		*LPBYTE(rom + snes2pc(a++)) = 0xC9;
		*LPBYTE(rom + snes2pc(a++)) = 0x06;
		*LPBYTE(rom + snes2pc(a++)) = 0x00;
		// BCC #NOXOR
		*LPBYTE(rom + snes2pc(a++)) = 0x90;
		*LPBYTE(rom + snes2pc(a++)) = 0x65; // FIXME
		// BNE #NEXT
		*LPBYTE(rom + snes2pc(a++)) = 0xD0;
		*LPBYTE(rom + snes2pc(a++)) = 0x05; // FIXME
		// LDA #FFFF
		*LPBYTE(rom + snes2pc(a++)) = 0xA9;
		*LPBYTE(rom + snes2pc(a++)) = 0xFF;
		*LPBYTE(rom + snes2pc(a++)) = 0xFF;
		// BRA #XOR
		*LPBYTE(rom + snes2pc(a++)) = 0x80;
		*LPBYTE(rom + snes2pc(a++)) = 0x5C; // FIXME
		// (7-B)
		// CMP #C
		*LPBYTE(rom + snes2pc(a++)) = 0xC9;
		*LPBYTE(rom + snes2pc(a++)) = 0x0C;
		*LPBYTE(rom + snes2pc(a++)) = 0x00;
		// BCC #XOR
		*LPBYTE(rom + snes2pc(a++)) = 0x90;
		*LPBYTE(rom + snes2pc(a++)) = 0x57; // FIXME
		// BNE #LDA6
		*LPBYTE(rom + snes2pc(a++)) = 0xD0;
		*LPBYTE(rom + snes2pc(a++)) = 0x4B; // FIXME
		// LDA #8000
		*LPBYTE(rom + snes2pc(a++)) = 0xA9;
		*LPBYTE(rom + snes2pc(a++)) = 0x00;
		*LPBYTE(rom + snes2pc(a++)) = 0x80;
		// BRA #XOR
		*LPBYTE(rom + snes2pc(a++)) = 0x80;
		*LPBYTE(rom + snes2pc(a++)) = 0x50; // FIXME

		//// change to BNE
		//*LPBYTE(rom + snes2pc(0x86A3C6)) = 0xD0;
		//*LPBYTE(rom + snes2pc(0x86A3CF)) = 0xD0;
		//*LPBYTE(rom + snes2pc(0x86A3D8)) = 0xD0;
		//*LPBYTE(rom + snes2pc(0x86A3E1)) = 0xD0;
		//*LPBYTE(rom + snes2pc(0x86A3EA)) = 0xD0;
		//*LPBYTE(rom + snes2pc(0x86A3F3)) = 0xD0;
		//*LPBYTE(rom + snes2pc(0x86A3FC)) = 0xD0;
		//*LPBYTE(rom + snes2pc(0x86A405)) = 0xD0;
		//*LPBYTE(rom + snes2pc(0x86A40E)) = 0xD0;
		//*LPBYTE(rom + snes2pc(0x86A417)) = 0xD0;
		//*LPBYTE(rom + snes2pc(0x86A420)) = 0xD0;

		// FIX second collision table
		a = 0x86A4DF;
		// PHY
		*LPBYTE(rom + snes2pc(a++)) = 0x5A;
		// TAY
		*LPBYTE(rom + snes2pc(a++)) = 0xA8;
		// LDA [$C8],y
		*LPBYTE(rom + snes2pc(a++)) = 0xB7;
		*LPBYTE(rom + snes2pc(a++)) = 0xC8;
		// AND #00FF
		*LPBYTE(rom + snes2pc(a++)) = 0x29;
		*LPBYTE(rom + snes2pc(a++)) = 0xFF;
		*LPBYTE(rom + snes2pc(a++)) = 0x00;
		// PLY
		*LPBYTE(rom + snes2pc(a++)) = 0x7A;
		// PHA
		*LPBYTE(rom + snes2pc(a++)) = 0x48;
		// CMP #0002
		*LPBYTE(rom + snes2pc(a++)) = 0xC9;
		*LPBYTE(rom + snes2pc(a++)) = 0x02;
		*LPBYTE(rom + snes2pc(a++)) = 0x00;
		// BCC - branch if already 0,1
		*LPBYTE(rom + snes2pc(a++)) = 0x90;
		*LPBYTE(rom + snes2pc(a++)) = 0x0F; // FIXME
		// CMP $CC
		*LPBYTE(rom + snes2pc(a++)) = 0xC5;
		*LPBYTE(rom + snes2pc(a++)) = 0xCC;
		// BEQ - branch if 2 (#8000)
		*LPBYTE(rom + snes2pc(a++)) = 0xF0;
		*LPBYTE(rom + snes2pc(a++)) = 0x05; // FIXME
		// LDA #0000 - >= 3 (#0)
		*LPBYTE(rom + snes2pc(a++)) = 0xA9;
		*LPBYTE(rom + snes2pc(a++)) = 0x00;
		*LPBYTE(rom + snes2pc(a++)) = 0x00;
		// BRA
		*LPBYTE(rom + snes2pc(a++)) = 0x80;
		*LPBYTE(rom + snes2pc(a++)) = 0x06; // FIXME
		// LDA #8000
		*LPBYTE(rom + snes2pc(a++)) = 0xA9;
		*LPBYTE(rom + snes2pc(a++)) = 0x00;
		*LPBYTE(rom + snes2pc(a++)) = 0x80;
		// NOP
		*LPBYTE(rom + snes2pc(a++)) = 0xEA;

		a = 0x86A50B;
		// PLA
		*LPBYTE(rom + snes2pc(a++)) = 0x68;
		// CMP #000E
		*LPBYTE(rom + snes2pc(a++)) = 0xC9;
		*LPBYTE(rom + snes2pc(a++)) = 0x0E;
		*LPBYTE(rom + snes2pc(a++)) = 0x00;
		// BCS (LDA #0)
		*LPBYTE(rom + snes2pc(a++)) = 0xB0;
		*LPBYTE(rom + snes2pc(a++)) = 0x06; // FIXME
		// CMP $CE
		*LPBYTE(rom + snes2pc(a++)) = 0xC5;
		*LPBYTE(rom + snes2pc(a++)) = 0xCE;
		// BEQ (LDA #1)
		*LPBYTE(rom + snes2pc(a++)) = 0xF0;
		*LPBYTE(rom + snes2pc(a++)) = 0x07; // FIXME
		// BCS (LDA #8000)
		*LPBYTE(rom + snes2pc(a++)) = 0xB0;
		*LPBYTE(rom + snes2pc(a++)) = 0x0A; // FIXME
		// fall through covers <= $CC
		// LDA #0000
		*LPBYTE(rom + snes2pc(a++)) = 0xA9;
		*LPBYTE(rom + snes2pc(a++)) = 0x00;
		*LPBYTE(rom + snes2pc(a++)) = 0x00;
		// BRA
		*LPBYTE(rom + snes2pc(a++)) = 0x80;
		*LPBYTE(rom + snes2pc(a++)) = 0x0C; // FIXME
		// LDA #0001
		*LPBYTE(rom + snes2pc(a++)) = 0xA9;
		*LPBYTE(rom + snes2pc(a++)) = 0x01;
		*LPBYTE(rom + snes2pc(a++)) = 0x00;
		// BRA
		*LPBYTE(rom + snes2pc(a++)) = 0x80;
		*LPBYTE(rom + snes2pc(a++)) = 0x07; // FIXME
		// LDA #8000
		*LPBYTE(rom + snes2pc(a++)) = 0xA9;
		*LPBYTE(rom + snes2pc(a++)) = 0x00;
		*LPBYTE(rom + snes2pc(a++)) = 0x80;
		// NOP
		*LPBYTE(rom + snes2pc(a++)) = 0xEA;
		*LPBYTE(rom + snes2pc(a++)) = 0xEA;

		if (romAddr >= 0x128000) return false;

		// <expand camera locks> $20
		romAddr = 0x128000;
		//memcpy(rom + romAddr, rom + SNESCore::snes2pc(0x81B783), 0xC * 0x20);
		for (unsigned i = 0; i < numLevels; i++) {
			memcpy(rom + romAddr + i * 0xC * 0x20, rom + SNESCore::snes2pc(0x81B783), 0xC * 0x20);
		}
		memset(rom + SNESCore::snes2pc(0x81B783), 0xFF, 0xC * 0x20);

		a = 0x81B783;
		*LPBYTE(rom + snes2pc(0x828B9D + 0)) = 0x5C;
		*LPDWORD(rom + snes2pc(0x828B9D + 1)) = a;

		// STA DP,X
		*LPBYTE(rom + snes2pc(a++)) = 0x95;
		*LPBYTE(rom + snes2pc(a++)) = 0x12;
		// LDA $86
		*LPBYTE(rom + snes2pc(a++)) = 0xA5;
		*LPBYTE(rom + snes2pc(a++)) = 0x86;
		// XBA
		*LPBYTE(rom + snes2pc(a++)) = 0xEB;
		// CLC
		*LPBYTE(rom + snes2pc(a++)) = 0x18;
		// ADC DP,X
		*LPBYTE(rom + snes2pc(a++)) = 0x75;
		*LPBYTE(rom + snes2pc(a++)) = 0x12;
		// STA DP,X
		*LPBYTE(rom + snes2pc(a++)) = 0x95;
		*LPBYTE(rom + snes2pc(a++)) = 0x12;
		// LDA $86
		*LPBYTE(rom + snes2pc(a++)) = 0xA5;
		*LPBYTE(rom + snes2pc(a++)) = 0x86;
		// XBA
		*LPBYTE(rom + snes2pc(a++)) = 0xEB;
		// LSR
		*LPBYTE(rom + snes2pc(a++)) = 0x4A;
		// CLC
		*LPBYTE(rom + snes2pc(a++)) = 0x18;
		// ADC DP,X
		*LPBYTE(rom + snes2pc(a++)) = 0x75;
		*LPBYTE(rom + snes2pc(a++)) = 0x12;
		// TAY
		*LPBYTE(rom + snes2pc(a++)) = 0xA8;
		// PHB
		*LPBYTE(rom + snes2pc(a++)) = 0x8B;
		// LDA #BANKBANK
		*LPBYTE(rom + snes2pc(a++)) = 0xA9;
		*LPBYTE(rom + snes2pc(a++)) = 0xA5;
		*LPBYTE(rom + snes2pc(a++)) = 0xA5;
		// PHA
		*LPBYTE(rom + snes2pc(a++)) = 0x48;
		// PLB
		*LPBYTE(rom + snes2pc(a++)) = 0xAB;
		// PLB
		*LPBYTE(rom + snes2pc(a++)) = 0xAB;
		// LDA $800M,y
		*LPBYTE(rom + snes2pc(a++)) = 0xB9;
		*LPBYTE(rom + snes2pc(a++)) = 0x00;
		*LPBYTE(rom + snes2pc(a++)) = 0x80;
		// STA N, x
		*LPBYTE(rom + snes2pc(a++)) = 0x95;
		*LPBYTE(rom + snes2pc(a++)) = 0x12;
		// LDA $800M,y
		*LPBYTE(rom + snes2pc(a++)) = 0xB9;
		*LPBYTE(rom + snes2pc(a++)) = 0x02;
		*LPBYTE(rom + snes2pc(a++)) = 0x80;
		// STA N, x
		*LPBYTE(rom + snes2pc(a++)) = 0x95;
		*LPBYTE(rom + snes2pc(a++)) = 0x34;
		// LDA $800M,y
		*LPBYTE(rom + snes2pc(a++)) = 0xB9;
		*LPBYTE(rom + snes2pc(a++)) = 0x04;
		*LPBYTE(rom + snes2pc(a++)) = 0x80;
		// STA N, x
		*LPBYTE(rom + snes2pc(a++)) = 0x95;
		*LPBYTE(rom + snes2pc(a++)) = 0x36;
		// LDA $800M,y
		*LPBYTE(rom + snes2pc(a++)) = 0xB9;
		*LPBYTE(rom + snes2pc(a++)) = 0x06;
		*LPBYTE(rom + snes2pc(a++)) = 0x80;
		// STA N, x
		*LPBYTE(rom + snes2pc(a++)) = 0x95;
		*LPBYTE(rom + snes2pc(a++)) = 0x38;
		// LDA $800M,y
		*LPBYTE(rom + snes2pc(a++)) = 0xB9;
		*LPBYTE(rom + snes2pc(a++)) = 0x08;
		*LPBYTE(rom + snes2pc(a++)) = 0x80;
		// STA N, x
		*LPBYTE(rom + snes2pc(a++)) = 0x95;
		*LPBYTE(rom + snes2pc(a++)) = 0x3A;
		// LDA $800M,y
		*LPBYTE(rom + snes2pc(a++)) = 0xB9;
		*LPBYTE(rom + snes2pc(a++)) = 0x0A;
		*LPBYTE(rom + snes2pc(a++)) = 0x80;
		// STA N, x
		*LPBYTE(rom + snes2pc(a++)) = 0x95;
		*LPBYTE(rom + snes2pc(a++)) = 0x3C;
		// PLB
		*LPBYTE(rom + snes2pc(a++)) = 0xAB;
		// RTL
		*LPBYTE(rom + snes2pc(a++)) = 0x6B;

		// <expand level transition checks> $40
		romAddr = 0x130000;
		for (unsigned i = 0; i < numLevels; i++) {
			memcpy(rom + romAddr + i * 0x40 * 0x4, rom + SNESCore::snes2pc(0x81C07D), 0x4 * 0x15);
		}
		memset(rom + SNESCore::snes2pc(0x81C07D), 0xFF, 0x4 * 0x15);

		a = 0x82BAAC;
		// add JMP
		*LPBYTE(rom + snes2pc(a++)) = 0x5C;
		*LPBYTE(rom + snes2pc(a++)) = 0x7D;
		*LPBYTE(rom + snes2pc(a++)) = 0xC0;
		*LPBYTE(rom + snes2pc(a++)) = 0x81;

		a = 0x81C07D;
		// STA DP,X
		*LPBYTE(rom + snes2pc(a++)) = 0x95;
		*LPBYTE(rom + snes2pc(a++)) = 0x12;
		// LDA $86
		*LPBYTE(rom + snes2pc(a++)) = 0xA5;
		*LPBYTE(rom + snes2pc(a++)) = 0x86;
		// XBA
		*LPBYTE(rom + snes2pc(a++)) = 0xEB;
		// CLC
		*LPBYTE(rom + snes2pc(a++)) = 0x18;
		// ADC DP,X
		*LPBYTE(rom + snes2pc(a++)) = 0x75;
		*LPBYTE(rom + snes2pc(a++)) = 0x12;
		// TAY
		*LPBYTE(rom + snes2pc(a++)) = 0xA8;
		// PHX
		*LPBYTE(rom + snes2pc(a++)) = 0xDA;
		// TYX
		*LPBYTE(rom + snes2pc(a++)) = 0xBB;
		// LDA long,X
		*LPBYTE(rom + snes2pc(a++)) = 0xBF;
		*LPBYTE(rom + snes2pc(a++)) = 0x00;
		*LPBYTE(rom + snes2pc(a++)) = 0x80;
		*LPBYTE(rom + snes2pc(a++)) = 0xA6;
		// AND #FF
		*LPBYTE(rom + snes2pc(a++)) = 0x29;
		*LPBYTE(rom + snes2pc(a++)) = 0xFF;
		*LPBYTE(rom + snes2pc(a++)) = 0x00;
		// PLX
		*LPBYTE(rom + snes2pc(a++)) = 0xFA;
		// STA DP,X
		*LPBYTE(rom + snes2pc(a++)) = 0x95;
		*LPBYTE(rom + snes2pc(a++)) = 0x12;
		// PHX
		*LPBYTE(rom + snes2pc(a++)) = 0xDA;
		// TYX
		*LPBYTE(rom + snes2pc(a++)) = 0xBB;
		// LDA long,X
		*LPBYTE(rom + snes2pc(a++)) = 0xBF;
		*LPBYTE(rom + snes2pc(a++)) = 0x01;
		*LPBYTE(rom + snes2pc(a++)) = 0x80;
		*LPBYTE(rom + snes2pc(a++)) = 0xA6;
		// AND #FF
		*LPBYTE(rom + snes2pc(a++)) = 0x29;
		*LPBYTE(rom + snes2pc(a++)) = 0xFF;
		*LPBYTE(rom + snes2pc(a++)) = 0x00;
		// PLX
		*LPBYTE(rom + snes2pc(a++)) = 0xFA;
		// STA DP,X
		*LPBYTE(rom + snes2pc(a++)) = 0x95;
		*LPBYTE(rom + snes2pc(a++)) = 0x3C;
		// PHX
		*LPBYTE(rom + snes2pc(a++)) = 0xDA;
		// TYX
		*LPBYTE(rom + snes2pc(a++)) = 0xBB;
		// LDA long,X
		*LPBYTE(rom + snes2pc(a++)) = 0xBF;
		*LPBYTE(rom + snes2pc(a++)) = 0x02;
		*LPBYTE(rom + snes2pc(a++)) = 0x80;
		*LPBYTE(rom + snes2pc(a++)) = 0xA6;
		// PLX
		*LPBYTE(rom + snes2pc(a++)) = 0xFA;
		// STA DP,X
		*LPBYTE(rom + snes2pc(a++)) = 0x95;
		*LPBYTE(rom + snes2pc(a++)) = 0x3E;
		// RTL
		*LPBYTE(rom + snes2pc(a++)) = 0x6B;

		// <expand entrances> $8 * $20 * $44
		romAddr = 0x138000;
		for (unsigned i = 0; i < numLevels; i++) {
			// entrance 0
			// death/state0
			*LPWORD(rom + romAddr + i * 0x100 + 0x00 + 0x0) = (*LPWORD(rom + snes2pc(0x85BE08 + 2 * i)) >> 0) & 0x00FF;
			// X
			*LPWORD(rom + romAddr + i * 0x100 + 0x00 + 0x2) = *LPWORD(rom + snes2pc(0x85BE90 + 4 * i));
			// Y
			*LPWORD(rom + romAddr + i * 0x100 + 0x00 + 0x4) = *LPWORD(rom + snes2pc(0x85BE92 + 4 * i));
			// C0_X
			*LPWORD(rom + romAddr + i * 0x100 + 0x00 + 0x6) = *LPWORD(rom + snes2pc(0x85CB92 + 4 * i));
			// C0_Y
			*LPWORD(rom + romAddr + i * 0x100 + 0x00 + 0x8) = *LPWORD(rom + snes2pc(0x85CB94 + 4 * i));
			// C1_X
			*LPWORD(rom + romAddr + i * 0x100 + 0x00 + 0xA) = *LPWORD(rom + snes2pc(0x85CDB2 + 4 * i));
			// C1_Y
			*LPWORD(rom + romAddr + i * 0x100 + 0x00 + 0xC) = *LPWORD(rom + snes2pc(0x85CDB4 + 4 * i));
			// state1
			*LPWORD(rom + romAddr + i * 0x100 + 0x00 + 0xE) = *LPWORD(rom + snes2pc(0x85C138 + 2 * i));

			// entrance 1
			// death/state0
			*LPWORD(rom + romAddr + i * 0x100 + 0x20 + 0x0) = (*LPWORD(rom + snes2pc(0x85BE08 + 2 * i)) >> 8) & 0x00FF;
			// X
			*LPWORD(rom + romAddr + i * 0x100 + 0x20 + 0x2) = *LPWORD(rom + snes2pc(0x85BFA0 + 4 * i));
			// Y
			*LPWORD(rom + romAddr + i * 0x100 + 0x20 + 0x4) = *LPWORD(rom + snes2pc(0x85BFA2 + 4 * i));
			// C0_X
			*LPWORD(rom + romAddr + i * 0x100 + 0x20 + 0x6) = *LPWORD(rom + snes2pc(0x85CCA2 + 4 * i));
			// C0_Y
			*LPWORD(rom + romAddr + i * 0x100 + 0x20 + 0x8) = *LPWORD(rom + snes2pc(0x85CCA4 + 4 * i));
			// C1_X
			*LPWORD(rom + romAddr + i * 0x100 + 0x20 + 0xA) = *LPWORD(rom + snes2pc(0x85CEC2 + 4 * i));
			// C1_Y
			*LPWORD(rom + romAddr + i * 0x100 + 0x20 + 0xC) = *LPWORD(rom + snes2pc(0x85CEC4 + 4 * i));
			// state1
			*LPWORD(rom + romAddr + i * 0x100 + 0x20 + 0xE) = *LPWORD(rom + snes2pc(0x85C1C0 + 2 * i));

			// secrets
			if (i == 0x33) {
				// death/state0
				*LPWORD(rom + romAddr + i * 0x100 + 0x40 + 0x0) = (*LPWORD(rom + snes2pc(0x85BE08 + 2 * i)) >> 0) & 0x00FF;
				// X
				*LPWORD(rom + romAddr + i * 0x100 + 0x40 + 0x2) = 0x266;
				// Y
				*LPWORD(rom + romAddr + i * 0x100 + 0x40 + 0x4) = 0x35;
				// C0_X
				*LPWORD(rom + romAddr + i * 0x100 + 0x40 + 0x6) = 0x1E6;
				// C0_Y
				*LPWORD(rom + romAddr + i * 0x100 + 0x40 + 0x8) = 0x0;
				// C1_X
				*LPWORD(rom + romAddr + i * 0x100 + 0x40 + 0xA) = 0x184;
				// C1_Y
				*LPWORD(rom + romAddr + i * 0x100 + 0x40 + 0xC) = 0x0;
				// state1
				*LPWORD(rom + romAddr + i * 0x100 + 0x40 + 0xE) = *LPWORD(rom + snes2pc(0x85C138 + 2 * i));
			}
			else if (i == 0x1C) {
				// death/state0
				*LPWORD(rom + romAddr + i * 0x100 + 0x40 + 0x0) = (*LPWORD(rom + snes2pc(0x85BE08 + 2 * i)) >> 0) & 0x00FF;
				// X
				*LPWORD(rom + romAddr + i * 0x100 + 0x40 + 0x2) = 0x790;
				// Y
				*LPWORD(rom + romAddr + i * 0x100 + 0x40 + 0x4) = 0xA5;
				// C0_X
				*LPWORD(rom + romAddr + i * 0x100 + 0x40 + 0x6) = 0x710;
				// C0_Y
				*LPWORD(rom + romAddr + i * 0x100 + 0x40 + 0x8) = 0x0;
				// C1_X
				*LPWORD(rom + romAddr + i * 0x100 + 0x40 + 0xA) = 0x773;
				// C1_Y
				*LPWORD(rom + romAddr + i * 0x100 + 0x40 + 0xC) = 0x0;
				// state1
				*LPWORD(rom + romAddr + i * 0x100 + 0x40 + 0xE) = *LPWORD(rom + snes2pc(0x85C138 + 2 * i));
			}

			// camera lock - assume all entrances want the same
			for (unsigned j = 0; j < ((i == 0x33 || i == 0x1C) ? 3 : 2); j++) {
				// state
				*LPWORD(rom + romAddr + i * 0x100 + j * 0x20 + 0x10) = *LPWORD(rom + snes2pc(0x85C0B0 + 2 * i));
				// A0
				*LPWORD(rom + romAddr + i * 0x100 + j * 0x20 + 0x12) = *LPWORD(rom + snes2pc(0x85CFD2 + 2 * i));
				// A2
				*LPWORD(rom + romAddr + i * 0x100 + j * 0x20 + 0x14) = *LPWORD(rom + snes2pc(0x85D05A + 2 * i));
				// A4
				*LPWORD(rom + romAddr + i * 0x100 + j * 0x20 + 0x16) = *LPWORD(rom + snes2pc(0x85D0E2 + 2 * i));
				// A6
				*LPWORD(rom + romAddr + i * 0x100 + j * 0x20 + 0x18) = *LPWORD(rom + snes2pc(0x85D16A + 2 * i));
				// A8
				*LPWORD(rom + romAddr + i * 0x100 + j * 0x20 + 0x1A) = *LPWORD(rom + snes2pc(0x85D1F2 + 2 * i));
				// AA
				*LPWORD(rom + romAddr + i * 0x100 + j * 0x20 + 0x1C) = *LPWORD(rom + snes2pc(0x85D27A + 2 * i));
				// COLL
				*LPWORD(rom + romAddr + i * 0x100 + j * 0x20 + 0x1E) = *LPWORD(rom + snes2pc(0x85C2D0 + 2 * i));
			}
		}

		// Needed for ending cutscene
		//memset(rom + SNESCore::snes2pc(0x85BE08), 0xFF, 0x2 * numLevels);
		//memset(rom + SNESCore::snes2pc(0x85C138), 0xFF, 0x2 * numLevels);
		memset(rom + SNESCore::snes2pc(0x85BE90), 0xFF, 0x4 * numLevels);
		memset(rom + SNESCore::snes2pc(0x85CB92), 0xFF, 0x4 * numLevels);
		memset(rom + SNESCore::snes2pc(0x85CDB2), 0xFF, 0x4 * numLevels);
		memset(rom + SNESCore::snes2pc(0x85C1C0), 0xFF, 0x2 * numLevels);
		memset(rom + SNESCore::snes2pc(0x85BFA0), 0xFF, 0x4 * numLevels);
		memset(rom + SNESCore::snes2pc(0x85CCA2), 0xFF, 0x4 * numLevels);
		memset(rom + SNESCore::snes2pc(0x85CEC2), 0xFF, 0x4 * numLevels);

		// replace code
		a = 0x86B60D;
		// check if $33 or $1C requiring STZ $13D4 and INC of $1602/$1604
		// fix secret exits to use entrance 2
		// LDA $86
		*LPBYTE(rom + snes2pc(a++)) = 0xA5;
		*LPBYTE(rom + snes2pc(a++)) = 0x86;
		// CMP #33
		*LPBYTE(rom + snes2pc(a++)) = 0xC9;
		*LPBYTE(rom + snes2pc(a++)) = 0x33;
		*LPBYTE(rom + snes2pc(a++)) = 0x00;
		// BNE next
		*LPBYTE(rom + snes2pc(a++)) = 0xD0;
		// FIXME
		*LPBYTE(rom + snes2pc(a++)) = 0x13;
		// LDA $1604
		*LPBYTE(rom + snes2pc(a++)) = 0xAD;
		*LPBYTE(rom + snes2pc(a++)) = 0x04;
		*LPBYTE(rom + snes2pc(a++)) = 0x16;
		// CMP #1
		*LPBYTE(rom + snes2pc(a++)) = 0xC9;
		*LPBYTE(rom + snes2pc(a++)) = 0x01;
		*LPBYTE(rom + snes2pc(a++)) = 0x00;
		// BNE normal
		*LPBYTE(rom + snes2pc(a++)) = 0xD0;
		// FIXME
		*LPBYTE(rom + snes2pc(a++)) = 0x20;
		// INC $1604
		*LPBYTE(rom + snes2pc(a++)) = 0xEE;
		*LPBYTE(rom + snes2pc(a++)) = 0x04;
		*LPBYTE(rom + snes2pc(a++)) = 0x16;
		// LDA #2
		*LPBYTE(rom + snes2pc(a++)) = 0xA9;
		*LPBYTE(rom + snes2pc(a++)) = 0x02;
		*LPBYTE(rom + snes2pc(a++)) = 0x00;
		// overwrite $13D4
		*LPBYTE(rom + snes2pc(a++)) = 0x8D;
		*LPBYTE(rom + snes2pc(a++)) = 0xD4;
		*LPBYTE(rom + snes2pc(a++)) = 0x13;
		// BRA done
		*LPBYTE(rom + snes2pc(a++)) = 0x80;
		// FIXME
		*LPBYTE(rom + snes2pc(a++)) = 0x18;
		// .next
		// CMP #1C
		*LPBYTE(rom + snes2pc(a++)) = 0xC9;
		*LPBYTE(rom + snes2pc(a++)) = 0x1C;
		*LPBYTE(rom + snes2pc(a++)) = 0x00;
		// BNE normal
		*LPBYTE(rom + snes2pc(a++)) = 0xD0;
		// FIXME
		*LPBYTE(rom + snes2pc(a++)) = 0x10;
		// LDA $1602
		*LPBYTE(rom + snes2pc(a++)) = 0xAD;
		*LPBYTE(rom + snes2pc(a++)) = 0x02;
		*LPBYTE(rom + snes2pc(a++)) = 0x16;
		// CMP #1
		*LPBYTE(rom + snes2pc(a++)) = 0xC9;
		*LPBYTE(rom + snes2pc(a++)) = 0x01;
		*LPBYTE(rom + snes2pc(a++)) = 0x00;
		// BNE normal
		*LPBYTE(rom + snes2pc(a++)) = 0xD0;
		// FIXME
		*LPBYTE(rom + snes2pc(a++)) = 0x08;
		// INC $1602
		*LPBYTE(rom + snes2pc(a++)) = 0xEE;
		*LPBYTE(rom + snes2pc(a++)) = 0x02;
		*LPBYTE(rom + snes2pc(a++)) = 0x16;
		// LDA #2
		*LPBYTE(rom + snes2pc(a++)) = 0xA9;
		*LPBYTE(rom + snes2pc(a++)) = 0x02;
		*LPBYTE(rom + snes2pc(a++)) = 0x00;
		// BRA done
		*LPBYTE(rom + snes2pc(a++)) = 0x80;
		// FIXME
		*LPBYTE(rom + snes2pc(a++)) = 0x03;
		// .normal
		// LDA $13D4
		*LPBYTE(rom + snes2pc(a++)) = 0xAD;
		*LPBYTE(rom + snes2pc(a++)) = 0xD4;
		*LPBYTE(rom + snes2pc(a++)) = 0x13;
		// .done
		// ASL
		*LPBYTE(rom + snes2pc(a++)) = 0x0A;
		// ASL
		*LPBYTE(rom + snes2pc(a++)) = 0x0A;
		// ASL
		*LPBYTE(rom + snes2pc(a++)) = 0x0A;
		// ASL
		*LPBYTE(rom + snes2pc(a++)) = 0x0A;
		// ASL
		*LPBYTE(rom + snes2pc(a++)) = 0x0A;
		// XBA
		*LPBYTE(rom + snes2pc(a++)) = 0xEB;
		// CLC
		*LPBYTE(rom + snes2pc(a++)) = 0x18;
		// ADC $86
		*LPBYTE(rom + snes2pc(a++)) = 0x65;
		*LPBYTE(rom + snes2pc(a++)) = 0x86;
		// XBA
		*LPBYTE(rom + snes2pc(a++)) = 0xEB;
		// TAX
		*LPBYTE(rom + snes2pc(a++)) = 0xAA;
		// LDA long $0,X
		*LPBYTE(rom + snes2pc(a++)) = 0xBF;
		*LPBYTE(rom + snes2pc(a++)) = 0x00;
		*LPBYTE(rom + snes2pc(a++)) = 0x80;
		*LPBYTE(rom + snes2pc(a++)) = 0xA7;
		// AND #007F
		*LPBYTE(rom + snes2pc(a++)) = 0x29;
		*LPBYTE(rom + snes2pc(a++)) = 0x7F;
		*LPBYTE(rom + snes2pc(a++)) = 0x00;
		// STA $552
		*LPBYTE(rom + snes2pc(a++)) = 0x8D;
		*LPBYTE(rom + snes2pc(a++)) = 0x52;
		*LPBYTE(rom + snes2pc(a++)) = 0x05;
		// LDA long $0,X
		*LPBYTE(rom + snes2pc(a++)) = 0xBF;
		*LPBYTE(rom + snes2pc(a++)) = 0x00;
		*LPBYTE(rom + snes2pc(a++)) = 0x80;
		*LPBYTE(rom + snes2pc(a++)) = 0xA7;
		// ASL
		*LPBYTE(rom + snes2pc(a++)) = 0x0A;
		// ASL
		*LPBYTE(rom + snes2pc(a++)) = 0x0A;
		// XBA
		*LPBYTE(rom + snes2pc(a++)) = 0xEB;
		// AND #2
		*LPBYTE(rom + snes2pc(a++)) = 0x29;
		*LPBYTE(rom + snes2pc(a++)) = 0x02;
		*LPBYTE(rom + snes2pc(a++)) = 0x00;
		// STA $57A
		*LPBYTE(rom + snes2pc(a++)) = 0x8D;
		*LPBYTE(rom + snes2pc(a++)) = 0x7A;
		*LPBYTE(rom + snes2pc(a++)) = 0x05;
		// LDA long $2,X
		*LPBYTE(rom + snes2pc(a++)) = 0xBF;
		*LPBYTE(rom + snes2pc(a++)) = 0x02;
		*LPBYTE(rom + snes2pc(a++)) = 0x80;
		*LPBYTE(rom + snes2pc(a++)) = 0xA7;
		// STA $54A
		*LPBYTE(rom + snes2pc(a++)) = 0x8D;
		*LPBYTE(rom + snes2pc(a++)) = 0x4A;
		*LPBYTE(rom + snes2pc(a++)) = 0x05;
		// LDA long $4,X
		*LPBYTE(rom + snes2pc(a++)) = 0xBF;
		*LPBYTE(rom + snes2pc(a++)) = 0x04;
		*LPBYTE(rom + snes2pc(a++)) = 0x80;
		*LPBYTE(rom + snes2pc(a++)) = 0xA7;
		// STA $54E
		*LPBYTE(rom + snes2pc(a++)) = 0x8D;
		*LPBYTE(rom + snes2pc(a++)) = 0x4E;
		*LPBYTE(rom + snes2pc(a++)) = 0x05;
		// LDA long $6,X
		*LPBYTE(rom + snes2pc(a++)) = 0xBF;
		*LPBYTE(rom + snes2pc(a++)) = 0x06;
		*LPBYTE(rom + snes2pc(a++)) = 0x80;
		*LPBYTE(rom + snes2pc(a++)) = 0xA7;
		// STA $128A
		*LPBYTE(rom + snes2pc(a++)) = 0x8D;
		*LPBYTE(rom + snes2pc(a++)) = 0x8A;
		*LPBYTE(rom + snes2pc(a++)) = 0x12;
		// STA $1280
		*LPBYTE(rom + snes2pc(a++)) = 0x8D;
		*LPBYTE(rom + snes2pc(a++)) = 0x80;
		*LPBYTE(rom + snes2pc(a++)) = 0x12;
		// LDA long $8,X
		*LPBYTE(rom + snes2pc(a++)) = 0xBF;
		*LPBYTE(rom + snes2pc(a++)) = 0x08;
		*LPBYTE(rom + snes2pc(a++)) = 0x80;
		*LPBYTE(rom + snes2pc(a++)) = 0xA7;
		// STA $12A2
		*LPBYTE(rom + snes2pc(a++)) = 0x8D;
		*LPBYTE(rom + snes2pc(a++)) = 0xA2;
		*LPBYTE(rom + snes2pc(a++)) = 0x12;
		// STA $1298
		*LPBYTE(rom + snes2pc(a++)) = 0x8D;
		*LPBYTE(rom + snes2pc(a++)) = 0x98;
		*LPBYTE(rom + snes2pc(a++)) = 0x12;
		// LDA long $A,X
		*LPBYTE(rom + snes2pc(a++)) = 0xBF;
		*LPBYTE(rom + snes2pc(a++)) = 0x0A;
		*LPBYTE(rom + snes2pc(a++)) = 0x80;
		*LPBYTE(rom + snes2pc(a++)) = 0xA7;
		// STA $12CA
		*LPBYTE(rom + snes2pc(a++)) = 0x8D;
		*LPBYTE(rom + snes2pc(a++)) = 0xCA;
		*LPBYTE(rom + snes2pc(a++)) = 0x12;
		// STA $12C0
		*LPBYTE(rom + snes2pc(a++)) = 0x8D;
		*LPBYTE(rom + snes2pc(a++)) = 0xC0;
		*LPBYTE(rom + snes2pc(a++)) = 0x12;
		// LDA long $C,X
		*LPBYTE(rom + snes2pc(a++)) = 0xBF;
		*LPBYTE(rom + snes2pc(a++)) = 0x0C;
		*LPBYTE(rom + snes2pc(a++)) = 0x80;
		*LPBYTE(rom + snes2pc(a++)) = 0xA7;
		// STA $12E2
		*LPBYTE(rom + snes2pc(a++)) = 0x8D;
		*LPBYTE(rom + snes2pc(a++)) = 0xE2;
		*LPBYTE(rom + snes2pc(a++)) = 0x12;
		// STA $12D8
		*LPBYTE(rom + snes2pc(a++)) = 0x8D;
		*LPBYTE(rom + snes2pc(a++)) = 0xD8;
		*LPBYTE(rom + snes2pc(a++)) = 0x12;
		// LDA long $E,X
		*LPBYTE(rom + snes2pc(a++)) = 0xBF;
		*LPBYTE(rom + snes2pc(a++)) = 0x0E;
		*LPBYTE(rom + snes2pc(a++)) = 0x80;
		*LPBYTE(rom + snes2pc(a++)) = 0xA7;
		// STA $578
		*LPBYTE(rom + snes2pc(a++)) = 0x8D;
		*LPBYTE(rom + snes2pc(a++)) = 0x78;
		*LPBYTE(rom + snes2pc(a++)) = 0x05;
		// PHX
		*LPBYTE(rom + snes2pc(a++)) = 0xDA;
		// TAX
		*LPBYTE(rom + snes2pc(a++)) = 0xAA;
		// LDA $1FA15,X
		*LPBYTE(rom + snes2pc(a++)) = 0xBF;
		*LPBYTE(rom + snes2pc(a++)) = 0x15;
		*LPBYTE(rom + snes2pc(a++)) = 0xFA;
		*LPBYTE(rom + snes2pc(a++)) = 0x01;
		// STA $544
		*LPBYTE(rom + snes2pc(a++)) = 0x8D;
		*LPBYTE(rom + snes2pc(a++)) = 0x44;
		*LPBYTE(rom + snes2pc(a++)) = 0x05;
		// PLX
		*LPBYTE(rom + snes2pc(a++)) = 0xFA;
		// BRA done2
		*LPBYTE(rom + snes2pc(a++)) = 0x80;
		// FIXME
		*LPBYTE(rom + snes2pc(a++)) = 0x19;

		//.branchInto
		// STA $578
		*LPBYTE(rom + snes2pc(a++)) = 0x8D;
		*LPBYTE(rom + snes2pc(a++)) = 0x78;
		*LPBYTE(rom + snes2pc(a++)) = 0x05;
		// TAX
		*LPBYTE(rom + snes2pc(a++)) = 0xAA;
		// LDA $1FA15,X
		*LPBYTE(rom + snes2pc(a++)) = 0xBF;
		*LPBYTE(rom + snes2pc(a++)) = 0x15;
		*LPBYTE(rom + snes2pc(a++)) = 0xFA;
		*LPBYTE(rom + snes2pc(a++)) = 0x01;
		// STA $544
		*LPBYTE(rom + snes2pc(a++)) = 0x8D;
		*LPBYTE(rom + snes2pc(a++)) = 0x44;
		*LPBYTE(rom + snes2pc(a++)) = 0x05;
		// LDA $13D4
		*LPBYTE(rom + snes2pc(a++)) = 0xAD;
		*LPBYTE(rom + snes2pc(a++)) = 0xD4;
		*LPBYTE(rom + snes2pc(a++)) = 0x13;
		// ASL
		*LPBYTE(rom + snes2pc(a++)) = 0x0A;
		// ASL
		*LPBYTE(rom + snes2pc(a++)) = 0x0A;
		// ASL
		*LPBYTE(rom + snes2pc(a++)) = 0x0A;
		// ASL
		*LPBYTE(rom + snes2pc(a++)) = 0x0A;
		// ASL
		*LPBYTE(rom + snes2pc(a++)) = 0x0A;
		// XBA
		*LPBYTE(rom + snes2pc(a++)) = 0xEB;
		// CLC
		*LPBYTE(rom + snes2pc(a++)) = 0x18;
		// ADC $86
		*LPBYTE(rom + snes2pc(a++)) = 0x65;
		*LPBYTE(rom + snes2pc(a++)) = 0x86;
		// XBA
		*LPBYTE(rom + snes2pc(a++)) = 0xEB;
		// TAX
		*LPBYTE(rom + snes2pc(a++)) = 0xAA;
		// .done2
		// camera lock
		// LDA long $10,X
		*LPBYTE(rom + snes2pc(a++)) = 0xBF;
		*LPBYTE(rom + snes2pc(a++)) = 0x10;
		*LPBYTE(rom + snes2pc(a++)) = 0x80;
		*LPBYTE(rom + snes2pc(a++)) = 0xA7;
		// STA $542
		*LPBYTE(rom + snes2pc(a++)) = 0x8D;
		*LPBYTE(rom + snes2pc(a++)) = 0x42;
		*LPBYTE(rom + snes2pc(a++)) = 0x05;
		// STZ D,$9C
		*LPBYTE(rom + snes2pc(a++)) = 0x64;
		*LPBYTE(rom + snes2pc(a++)) = 0x9C;
		// LDA long $12,X
		*LPBYTE(rom + snes2pc(a++)) = 0xBF;
		*LPBYTE(rom + snes2pc(a++)) = 0x12;
		*LPBYTE(rom + snes2pc(a++)) = 0x80;
		*LPBYTE(rom + snes2pc(a++)) = 0xA7;
		// STA D,$A0
		*LPBYTE(rom + snes2pc(a++)) = 0x85;
		*LPBYTE(rom + snes2pc(a++)) = 0xA0;
		// LDA long $14,X
		*LPBYTE(rom + snes2pc(a++)) = 0xBF;
		*LPBYTE(rom + snes2pc(a++)) = 0x14;
		*LPBYTE(rom + snes2pc(a++)) = 0x80;
		*LPBYTE(rom + snes2pc(a++)) = 0xA7;
		// STA D,$A2
		*LPBYTE(rom + snes2pc(a++)) = 0x85;
		*LPBYTE(rom + snes2pc(a++)) = 0xA2;
		// LDA long $16,X
		*LPBYTE(rom + snes2pc(a++)) = 0xBF;
		*LPBYTE(rom + snes2pc(a++)) = 0x16;
		*LPBYTE(rom + snes2pc(a++)) = 0x80;
		*LPBYTE(rom + snes2pc(a++)) = 0xA7;
		// STA D,$A4
		*LPBYTE(rom + snes2pc(a++)) = 0x85;
		*LPBYTE(rom + snes2pc(a++)) = 0xA4;
		// LDA long $18,X
		*LPBYTE(rom + snes2pc(a++)) = 0xBF;
		*LPBYTE(rom + snes2pc(a++)) = 0x18;
		*LPBYTE(rom + snes2pc(a++)) = 0x80;
		*LPBYTE(rom + snes2pc(a++)) = 0xA7;
		// STA D,$A6
		*LPBYTE(rom + snes2pc(a++)) = 0x85;
		*LPBYTE(rom + snes2pc(a++)) = 0xA6;
		// LDA long $1A,X
		*LPBYTE(rom + snes2pc(a++)) = 0xBF;
		*LPBYTE(rom + snes2pc(a++)) = 0x1A;
		*LPBYTE(rom + snes2pc(a++)) = 0x80;
		*LPBYTE(rom + snes2pc(a++)) = 0xA7;
		// STA D,$A8
		*LPBYTE(rom + snes2pc(a++)) = 0x85;
		*LPBYTE(rom + snes2pc(a++)) = 0xA8;
		// LDA long $1C,X
		*LPBYTE(rom + snes2pc(a++)) = 0xBF;
		*LPBYTE(rom + snes2pc(a++)) = 0x1C;
		*LPBYTE(rom + snes2pc(a++)) = 0x80;
		*LPBYTE(rom + snes2pc(a++)) = 0xA7;
		// STA D,$AA
		*LPBYTE(rom + snes2pc(a++)) = 0x85;
		*LPBYTE(rom + snes2pc(a++)) = 0xAA;
		// LDA long $1E,X
		*LPBYTE(rom + snes2pc(a++)) = 0xBF;
		*LPBYTE(rom + snes2pc(a++)) = 0x1E;
		*LPBYTE(rom + snes2pc(a++)) = 0x80;
		*LPBYTE(rom + snes2pc(a++)) = 0xA7;
		// TAX
		*LPBYTE(rom + snes2pc(a++)) = 0xAA;
		// BRA
		*LPBYTE(rom + snes2pc(a++)) = 0x82;
		// FIXME
		*LPBYTE(rom + snes2pc(a++)) = 0xA2;
		*LPBYTE(rom + snes2pc(a++)) = 0x00;

		// fix BRL
		*LPWORD(rom + snes2pc(0x86B60B)) = 0x009D;

		// fix event check
		a = 0x80D419;
		*LPBYTE(rom + snes2pc(a++)) = 0x22;
		*LPBYTE(rom + snes2pc(a++)) = 0x92;
		*LPBYTE(rom + snes2pc(a++)) = 0xCB;
		*LPBYTE(rom + snes2pc(a++)) = 0x85;
		*LPBYTE(rom + snes2pc(a++)) = 0xEA;
		a = 0x85CB92;
		// PHA
		*LPBYTE(rom + snes2pc(a++)) = 0x48;
		// LDA $13D4
		*LPBYTE(rom + snes2pc(a++)) = 0xAD;
		*LPBYTE(rom + snes2pc(a++)) = 0xD4;
		*LPBYTE(rom + snes2pc(a++)) = 0x13;
		// AND #0001
		*LPBYTE(rom + snes2pc(a++)) = 0x29;
		*LPBYTE(rom + snes2pc(a++)) = 0x01;
		*LPBYTE(rom + snes2pc(a++)) = 0x00;
		// BEQ
		*LPBYTE(rom + snes2pc(a++)) = 0xF0;
		*LPBYTE(rom + snes2pc(a++)) = 0x06;
		// PLA
		*LPBYTE(rom + snes2pc(a++)) = 0x68;
		// EOR #0001
		*LPBYTE(rom + snes2pc(a++)) = 0x49;
		*LPBYTE(rom + snes2pc(a++)) = 0x01;
		*LPBYTE(rom + snes2pc(a++)) = 0x00;
		// BRA
		*LPBYTE(rom + snes2pc(a++)) = 0x80;
		*LPBYTE(rom + snes2pc(a++)) = 0x01;
		// PLA
		*LPBYTE(rom + snes2pc(a++)) = 0x68;
		// STA D,$9A
		*LPBYTE(rom + snes2pc(a++)) = 0x85;
		*LPBYTE(rom + snes2pc(a++)) = 0x9A;
		// RTL
		*LPBYTE(rom + snes2pc(a++)) = 0x6B;

		// <support arbitrary level transitions>
		// write array for level linkage
		//memset(rom + SNESCore::snes2pc(0xA0C000), 0xFF, sizeof(WORD) * nmmx.numLevels);
		for (unsigned i = 0; i < numLevels; i++) {
			//*LPWORD(rom + SNESCore::snes2pc(0xA0C000 + 0x000 + 2 * i)) = i + ((i == 29 || i == 53) ? 2 : 1);
			//*LPWORD(rom + SNESCore::snes2pc(0xA0C000 + 0x100 + 2 * i)) = i - ((i == 31 || i == 55) ? 2 : 1);
			//*LPWORD(rom + SNESCore::snes2pc(0xA0C000 + 0x200 + 2 * i)) = 0x0;
			//*LPWORD(rom + SNESCore::snes2pc(0xA0C000 + 0x300 + 2 * i)) = 0x1;

			// exit 0
			*LPBYTE(rom + SNESCore::snes2pc(0xA0C000 + 0x10 * i + 0x0)) = i + ((i == 29 || i == 53) ? 2 : 1);
			*LPBYTE(rom + SNESCore::snes2pc(0xA0C000 + 0x10 * i + 0x1)) = 0x0; // entrance

			// exit 1
			*LPBYTE(rom + SNESCore::snes2pc(0xA0C000 + 0x10 * i + 0x2)) = i - ((i == 31 || i == 55) ? 2 : 1);
			*LPBYTE(rom + SNESCore::snes2pc(0xA0C000 + 0x10 * i + 0x3)) = 0x1; // entrance

			// secrets
			if (i == 30) *LPBYTE(rom + SNESCore::snes2pc(0xA0C000 + 0x10 * i + 0x2)) = 28;
			if (i == 30) *LPBYTE(rom + SNESCore::snes2pc(0xA0C000 + 0x10 * i + 0x3)) = 0x2;
			if (i == 54) *LPBYTE(rom + SNESCore::snes2pc(0xA0C000 + 0x10 * i + 0x2)) = 51;
			if (i == 54) *LPBYTE(rom + SNESCore::snes2pc(0xA0C000 + 0x10 * i + 0x3)) = 0x2;
		}

		*LPWORD(rom + SNESCore::snes2pc(0x82BB13)) = 0xBB1B; // forward
		*LPWORD(rom + SNESCore::snes2pc(0x82BB15)) = 0xBB1B; // backward
		// jump straight to the handler
		//*LPBYTE(rom + SNESCore::snes2pc(0x82BAD8)) += 0x1A;
		//*LPBYTE(rom + SNESCore::snes2pc(0x82BAEF)) += 0x1A;
		//*LPBYTE(rom + SNESCore::snes2pc(0x82BAF7)) += 0x1A;
		//*LPBYTE(rom + SNESCore::snes2pc(0x82BAFF)) += 0x1A;

		a = 0x82BB03;
		// JSR 
		*LPBYTE(rom + SNESCore::snes2pc(a++)) = 0x20;
		*LPBYTE(rom + SNESCore::snes2pc(a++)) = 0x41;
		*LPBYTE(rom + SNESCore::snes2pc(a++)) = 0xBB;

		a = 0x82BB1B;
		// PHX
		*LPBYTE(rom + SNESCore::snes2pc(a++)) = 0xDA;
		// LDA $86
		*LPBYTE(rom + SNESCore::snes2pc(a++)) = 0xA5;
		*LPBYTE(rom + SNESCore::snes2pc(a++)) = 0x86;
		// ASL
		*LPBYTE(rom + SNESCore::snes2pc(a++)) = 0x0A;
		// ASL
		*LPBYTE(rom + SNESCore::snes2pc(a++)) = 0x0A;
		// ASL
		*LPBYTE(rom + SNESCore::snes2pc(a++)) = 0x0A;
		// CLC
		*LPBYTE(rom + SNESCore::snes2pc(a++)) = 0x18;
		// ADC $3C
		*LPBYTE(rom + SNESCore::snes2pc(a++)) = 0x75;
		*LPBYTE(rom + SNESCore::snes2pc(a++)) = 0x3C;
		// ASL
		*LPBYTE(rom + SNESCore::snes2pc(a++)) = 0x0A;
		// TAX
		*LPBYTE(rom + SNESCore::snes2pc(a++)) = 0xAA;
		// LDA long, X
		*LPBYTE(rom + SNESCore::snes2pc(a++)) = 0xBF;
		*LPBYTE(rom + SNESCore::snes2pc(a++)) = 0x00;
		*LPBYTE(rom + SNESCore::snes2pc(a++)) = 0xC0;
		*LPBYTE(rom + SNESCore::snes2pc(a++)) = 0xA0;
		// AND #00FF
		*LPBYTE(rom + SNESCore::snes2pc(a++)) = 0x29;
		*LPBYTE(rom + SNESCore::snes2pc(a++)) = 0xFF;
		*LPBYTE(rom + SNESCore::snes2pc(a++)) = 0x00;
		// STA $86
		*LPBYTE(rom + SNESCore::snes2pc(a++)) = 0x85;
		*LPBYTE(rom + SNESCore::snes2pc(a++)) = 0x86;
		// LDA long, X
		*LPBYTE(rom + SNESCore::snes2pc(a++)) = 0xBF;
		*LPBYTE(rom + SNESCore::snes2pc(a++)) = 0x00;
		*LPBYTE(rom + SNESCore::snes2pc(a++)) = 0xC0;
		*LPBYTE(rom + SNESCore::snes2pc(a++)) = 0xA0;
		// XBA
		*LPBYTE(rom + SNESCore::snes2pc(a++)) = 0xEB;
		// AND #00FF
		*LPBYTE(rom + SNESCore::snes2pc(a++)) = 0x29;
		*LPBYTE(rom + SNESCore::snes2pc(a++)) = 0xFF;
		*LPBYTE(rom + SNESCore::snes2pc(a++)) = 0x00;
		// PLX
		*LPBYTE(rom + SNESCore::snes2pc(a++)) = 0xFA;
		// STA $13D4
		*LPBYTE(rom + SNESCore::snes2pc(a++)) = 0x8D;
		*LPBYTE(rom + SNESCore::snes2pc(a++)) = 0xD4;
		*LPBYTE(rom + SNESCore::snes2pc(a++)) = 0x13;
		// LDA #06
		*LPBYTE(rom + SNESCore::snes2pc(a++)) = 0xA9;
		*LPBYTE(rom + SNESCore::snes2pc(a++)) = 0x06;
		*LPBYTE(rom + SNESCore::snes2pc(a++)) = 0x00;
		// STA $70
		*LPBYTE(rom + SNESCore::snes2pc(a++)) = 0x85;
		*LPBYTE(rom + SNESCore::snes2pc(a++)) = 0x70;
		// RTL
		*LPBYTE(rom + SNESCore::snes2pc(a++)) = 0x6B;

		// CMP #8
		*LPBYTE(rom + SNESCore::snes2pc(a++)) = 0xC9;
		*LPBYTE(rom + SNESCore::snes2pc(a++)) = 0x08;
		*LPBYTE(rom + SNESCore::snes2pc(a++)) = 0x00;
		// BCS to return
		*LPBYTE(rom + SNESCore::snes2pc(a++)) = 0xB0;
		*LPBYTE(rom + SNESCore::snes2pc(a++)) = 0x03;
		// LDA #0
		*LPBYTE(rom + SNESCore::snes2pc(a++)) = 0xA9;
		*LPBYTE(rom + SNESCore::snes2pc(a++)) = 0x00;
		*LPBYTE(rom + SNESCore::snes2pc(a++)) = 0x00;
		// .return
		// AND #3
		*LPBYTE(rom + SNESCore::snes2pc(a++)) = 0x29;
		*LPBYTE(rom + SNESCore::snes2pc(a++)) = 0x03;
		*LPBYTE(rom + SNESCore::snes2pc(a++)) = 0x00;
		// RTS
		*LPBYTE(rom + SNESCore::snes2pc(a++)) = 0x60;

		//*LPWORD(rom + SNESCore::snes2pc(0x82BB13)) = 0xBB33; // forward // FIXME
		//*LPWORD(rom + SNESCore::snes2pc(0x82BB15)) = 0xBB1B; // backward
		//// write new level sequencing code (also needs to take care of 28/30)
		//a = 0x82BB1B;
		//// Backward
		////a = 0x82BB2F;
		//// PHX
		//*LPBYTE(rom + SNESCore::snes2pc(a++)) = 0xDA;
		//// LDA $86
		//*LPBYTE(rom + SNESCore::snes2pc(a++)) = 0xA5;
		//*LPBYTE(rom + SNESCore::snes2pc(a++)) = 0x86;
		//// ASL
		//*LPBYTE(rom + SNESCore::snes2pc(a++)) = 0x0A;
		//// TAX
		//*LPBYTE(rom + SNESCore::snes2pc(a++)) = 0xAA;
		//// LDA long, X
		//*LPBYTE(rom + SNESCore::snes2pc(a++)) = 0xBF;
		//*LPBYTE(rom + SNESCore::snes2pc(a++)) = 0x00;
		//*LPBYTE(rom + SNESCore::snes2pc(a++)) = 0xC1;
		//*LPBYTE(rom + SNESCore::snes2pc(a++)) = 0xA0;
		//// STA $86
		//*LPBYTE(rom + SNESCore::snes2pc(a++)) = 0x85;
		//*LPBYTE(rom + SNESCore::snes2pc(a++)) = 0x86;
		//// SEC
		//*LPBYTE(rom + SNESCore::snes2pc(a++)) = 0x38;
		//// TXA
		//*LPBYTE(rom + SNESCore::snes2pc(a++)) = 0x8A;
		//// SBC $1E * 2
		//*LPBYTE(rom + SNESCore::snes2pc(a++)) = 0xE9;
		//*LPBYTE(rom + SNESCore::snes2pc(a++)) = 0x3C;
		//*LPBYTE(rom + SNESCore::snes2pc(a++)) = 0x00;
		//// BEQ
		//*LPBYTE(rom + SNESCore::snes2pc(a++)) = 0xF0;
		//*LPBYTE(rom + SNESCore::snes2pc(a++)) = 0x15; // FIXME
		//// LDA long, X
		//*LPBYTE(rom + SNESCore::snes2pc(a++)) = 0xBF;
		//*LPBYTE(rom + SNESCore::snes2pc(a++)) = 0x00;
		//*LPBYTE(rom + SNESCore::snes2pc(a++)) = 0xC3;
		//*LPBYTE(rom + SNESCore::snes2pc(a++)) = 0xA0;
		//// BRA
		//*LPBYTE(rom + SNESCore::snes2pc(a++)) = 0x80;
		//*LPBYTE(rom + SNESCore::snes2pc(a++)) = 0x0F;
	
		//// Forward
		//// PHX
		//*LPBYTE(rom + SNESCore::snes2pc(a++)) = 0xDA;
		//// LDA $86
		//*LPBYTE(rom + SNESCore::snes2pc(a++)) = 0xA5;
		//*LPBYTE(rom + SNESCore::snes2pc(a++)) = 0x86;
		//// ASL
		//*LPBYTE(rom + SNESCore::snes2pc(a++)) = 0x0A;
		//// TAX

		//*LPBYTE(rom + SNESCore::snes2pc(a++)) = 0xAA;
		//// LDA long, X
		//*LPBYTE(rom + SNESCore::snes2pc(a++)) = 0xBF;
		//*LPBYTE(rom + SNESCore::snes2pc(a++)) = 0x00;
		//*LPBYTE(rom + SNESCore::snes2pc(a++)) = 0xC0;
		//*LPBYTE(rom + SNESCore::snes2pc(a++)) = 0xA0;
		//// STA $86
		//*LPBYTE(rom + SNESCore::snes2pc(a++)) = 0x85;
		//*LPBYTE(rom + SNESCore::snes2pc(a++)) = 0x86;
		//// LDA long, X
		//*LPBYTE(rom + SNESCore::snes2pc(a++)) = 0xBF;
		//*LPBYTE(rom + SNESCore::snes2pc(a++)) = 0x00;
		//*LPBYTE(rom + SNESCore::snes2pc(a++)) = 0xC2;
		//*LPBYTE(rom + SNESCore::snes2pc(a++)) = 0xA0;
	
		//// PLX
		//*LPBYTE(rom + SNESCore::snes2pc(a++)) = 0xFA;
		//// STA $13D4
		//*LPBYTE(rom + SNESCore::snes2pc(a++)) = 0x8D;
		//*LPBYTE(rom + SNESCore::snes2pc(a++)) = 0xD4;
		//*LPBYTE(rom + SNESCore::snes2pc(a++)) = 0x13;
		//// LDA #06
		//*LPBYTE(rom + SNESCore::snes2pc(a++)) = 0xA9;
		//*LPBYTE(rom + SNESCore::snes2pc(a++)) = 0x06;
		//*LPBYTE(rom + SNESCore::snes2pc(a++)) = 0x00;
		//// STA $70
		//*LPBYTE(rom + SNESCore::snes2pc(a++)) = 0x85;
		//*LPBYTE(rom + SNESCore::snes2pc(a++)) = 0x70;
		//// RTL
		//*LPBYTE(rom + SNESCore::snes2pc(a++)) = 0x6B;

		// Courtyard exit
		// fix LDA of 1 to allow courtyard->anywhere
		a = 0x8CFF8C;
		// LDA #6
		*LPBYTE(rom + SNESCore::snes2pc(a++)) = 0xA9;
		*LPBYTE(rom + SNESCore::snes2pc(a++)) = 0x06;
		*LPBYTE(rom + SNESCore::snes2pc(a++)) = 0x00;
		// JSL FFC0
		*LPBYTE(rom + SNESCore::snes2pc(a++)) = 0x22;
		*LPBYTE(rom + SNESCore::snes2pc(a++)) = 0xC0;
		*LPBYTE(rom + SNESCore::snes2pc(a++)) = 0xFF;
		*LPBYTE(rom + SNESCore::snes2pc(a++)) = 0x82;
		// NOPs
		*LPBYTE(rom + SNESCore::snes2pc(a++)) = 0xEA;
		*LPBYTE(rom + SNESCore::snes2pc(a++)) = 0xEA;
		*LPBYTE(rom + SNESCore::snes2pc(a++)) = 0xEA;

		// CutScene
		// remove increment
		a = 0x82BB5D;
		*LPBYTE(rom + SNESCore::snes2pc(a++)) = 0xEA;
		*LPBYTE(rom + SNESCore::snes2pc(a++)) = 0xEA;
		// jmp to load
		a = 0x82BB69;
		*LPBYTE(rom + SNESCore::snes2pc(a++)) = 0x4C;
		*LPBYTE(rom + SNESCore::snes2pc(a++)) = 0xC0;
		*LPBYTE(rom + SNESCore::snes2pc(a++)) = 0xFF;
	
		a = 0x82FFC0;
		// STA D, $70
		*LPBYTE(rom + SNESCore::snes2pc(a++)) = 0x85;
		*LPBYTE(rom + SNESCore::snes2pc(a++)) = 0x70;
		// PHX
		*LPBYTE(rom + SNESCore::snes2pc(a++)) = 0xDA;
		// LDA $86
		*LPBYTE(rom + SNESCore::snes2pc(a++)) = 0xA5;
		*LPBYTE(rom + SNESCore::snes2pc(a++)) = 0x86;
		// ASL
		*LPBYTE(rom + SNESCore::snes2pc(a++)) = 0x0A;
		// ASL
		*LPBYTE(rom + SNESCore::snes2pc(a++)) = 0x0A;
		// ASL
		*LPBYTE(rom + SNESCore::snes2pc(a++)) = 0x0A;
		// ASL
		*LPBYTE(rom + SNESCore::snes2pc(a++)) = 0x0A;
		// TAX
		*LPBYTE(rom + SNESCore::snes2pc(a++)) = 0xAA;
		// LDA long, X
		*LPBYTE(rom + SNESCore::snes2pc(a++)) = 0xBF;
		*LPBYTE(rom + SNESCore::snes2pc(a++)) = 0x00;
		*LPBYTE(rom + SNESCore::snes2pc(a++)) = 0xC0;
		*LPBYTE(rom + SNESCore::snes2pc(a++)) = 0xA0;
		// AND #00FF
		*LPBYTE(rom + SNESCore::snes2pc(a++)) = 0x29;
		*LPBYTE(rom + SNESCore::snes2pc(a++)) = 0xFF;
		*LPBYTE(rom + SNESCore::snes2pc(a++)) = 0x00;
		// STA $86
		*LPBYTE(rom + SNESCore::snes2pc(a++)) = 0x85;
		*LPBYTE(rom + SNESCore::snes2pc(a++)) = 0x86;
		// LDA long, X
		*LPBYTE(rom + SNESCore::snes2pc(a++)) = 0xBF;
		*LPBYTE(rom + SNESCore::snes2pc(a++)) = 0x00;
		*LPBYTE(rom + SNESCore::snes2pc(a++)) = 0xC0;
		*LPBYTE(rom + SNESCore::snes2pc(a++)) = 0xA0;
		// XBA
		*LPBYTE(rom + SNESCore::snes2pc(a++)) = 0xEB;
		// AND #00FF
		*LPBYTE(rom + SNESCore::snes2pc(a++)) = 0x29;
		*LPBYTE(rom + SNESCore::snes2pc(a++)) = 0xFF;
		*LPBYTE(rom + SNESCore::snes2pc(a++)) = 0x00;
		// STA $13D4
		*LPBYTE(rom + SNESCore::snes2pc(a++)) = 0x8D;
		*LPBYTE(rom + SNESCore::snes2pc(a++)) = 0xD4;
		*LPBYTE(rom + SNESCore::snes2pc(a++)) = 0x13;
		// PLX
		*LPBYTE(rom + SNESCore::snes2pc(a++)) = 0xFA;
		// RTL
		*LPBYTE(rom + SNESCore::snes2pc(a++)) = 0x6B;
	
		// rotating level
		// tweak branch to go to next RTL
		//a = 0x83E65B;
		//*LPBYTE(rom + SNESCore::snes2pc(a++)) += 1;
		// jmp to load
		a = 0x83E65F;
		*LPBYTE(rom + SNESCore::snes2pc(a++)) = 0x22;
		*LPBYTE(rom + SNESCore::snes2pc(a++)) = 0xC0;
		*LPBYTE(rom + SNESCore::snes2pc(a++)) = 0xFF;
		*LPBYTE(rom + SNESCore::snes2pc(a++)) = 0x82;

		//a = 0x83FFD8;
		//// PHX
		//*LPBYTE(rom + SNESCore::snes2pc(a++)) = 0xDA;
		//// LDA $86
		//*LPBYTE(rom + SNESCore::snes2pc(a++)) = 0xA5;
		//*LPBYTE(rom + SNESCore::snes2pc(a++)) = 0x86;
		//// ASL
		//*LPBYTE(rom + SNESCore::snes2pc(a++)) = 0x0A;
		//// TAX
		//*LPBYTE(rom + SNESCore::snes2pc(a++)) = 0xAA;
		//// LDA long, X
		//*LPBYTE(rom + SNESCore::snes2pc(a++)) = 0xBF;
		//*LPBYTE(rom + SNESCore::snes2pc(a++)) = 0x00;
		//*LPBYTE(rom + SNESCore::snes2pc(a++)) = 0xC0;
		//*LPBYTE(rom + SNESCore::snes2pc(a++)) = 0xA0;
		//// STA $86
		//*LPBYTE(rom + SNESCore::snes2pc(a++)) = 0x85;
		//*LPBYTE(rom + SNESCore::snes2pc(a++)) = 0x86;
		//// LDA long, X
		//*LPBYTE(rom + SNESCore::snes2pc(a++)) = 0xBF;
		//*LPBYTE(rom + SNESCore::snes2pc(a++)) = 0x00;
		//*LPBYTE(rom + SNESCore::snes2pc(a++)) = 0xC2;
		//*LPBYTE(rom + SNESCore::snes2pc(a++)) = 0xA0;
		//// STA $13D4
		//*LPBYTE(rom + SNESCore::snes2pc(a++)) = 0x8D;
		//*LPBYTE(rom + SNESCore::snes2pc(a++)) = 0xD4;
		//*LPBYTE(rom + SNESCore::snes2pc(a++)) = 0x13;
		//// PLX
		//*LPBYTE(rom + SNESCore::snes2pc(a++)) = 0xFA;
		//// RTL
		//*LPBYTE(rom + SNESCore::snes2pc(a++)) = 0x6B;

		// other CutScene
		// CutScene
		a = 0x85FF38;
		// LDA #03
		*LPBYTE(rom + SNESCore::snes2pc(a++)) = 0xA9;
		*LPBYTE(rom + SNESCore::snes2pc(a++)) = 0x03;
		*LPBYTE(rom + SNESCore::snes2pc(a++)) = 0x00;
		// JML
		*LPBYTE(rom + SNESCore::snes2pc(a++)) = 0x5C;
		*LPBYTE(rom + SNESCore::snes2pc(a++)) = 0xC0;
		*LPBYTE(rom + SNESCore::snes2pc(a++)) = 0xFF;
		*LPBYTE(rom + SNESCore::snes2pc(a++)) = 0x82;

		a = 0x85FF61;
		*LPBYTE(rom + SNESCore::snes2pc(a++)) = 0xEE;
		*LPBYTE(rom + SNESCore::snes2pc(a++)) = 0xE8;
		*LPBYTE(rom + SNESCore::snes2pc(a++)) = 0x13;
		*LPBYTE(rom + SNESCore::snes2pc(a++)) = 0x4C;
		*LPBYTE(rom + SNESCore::snes2pc(a++)) = 0xC0;
		*LPBYTE(rom + SNESCore::snes2pc(a++)) = 0xFF;

		a = 0x85FFC0;
		// PHX
		*LPBYTE(rom + SNESCore::snes2pc(a++)) = 0xDA;
		// LDA $86
		*LPBYTE(rom + SNESCore::snes2pc(a++)) = 0xA5;
		*LPBYTE(rom + SNESCore::snes2pc(a++)) = 0x86;
		// ASL
		*LPBYTE(rom + SNESCore::snes2pc(a++)) = 0x0A;
		// ASL
		*LPBYTE(rom + SNESCore::snes2pc(a++)) = 0x0A;
		// ASL
		*LPBYTE(rom + SNESCore::snes2pc(a++)) = 0x0A;
		// ASL
		*LPBYTE(rom + SNESCore::snes2pc(a++)) = 0x0A;
		// TAX
		*LPBYTE(rom + SNESCore::snes2pc(a++)) = 0xAA;
		// LDA long, X
		*LPBYTE(rom + SNESCore::snes2pc(a++)) = 0xBF;
		*LPBYTE(rom + SNESCore::snes2pc(a++)) = 0x00;
		*LPBYTE(rom + SNESCore::snes2pc(a++)) = 0xC0;
		*LPBYTE(rom + SNESCore::snes2pc(a++)) = 0xA0;
		// AND #00FF
		*LPBYTE(rom + SNESCore::snes2pc(a++)) = 0x29;
		*LPBYTE(rom + SNESCore::snes2pc(a++)) = 0xFF;
		*LPBYTE(rom + SNESCore::snes2pc(a++)) = 0x00;
		// STA $86
		*LPBYTE(rom + SNESCore::snes2pc(a++)) = 0x85;
		*LPBYTE(rom + SNESCore::snes2pc(a++)) = 0x86;
		// LDA long, X
		*LPBYTE(rom + SNESCore::snes2pc(a++)) = 0xBF;
		*LPBYTE(rom + SNESCore::snes2pc(a++)) = 0x00;
		*LPBYTE(rom + SNESCore::snes2pc(a++)) = 0xC0;
		*LPBYTE(rom + SNESCore::snes2pc(a++)) = 0xA0;
		// XBA
		*LPBYTE(rom + SNESCore::snes2pc(a++)) = 0xEB;
		// AND #00FF
		*LPBYTE(rom + SNESCore::snes2pc(a++)) = 0x29;
		*LPBYTE(rom + SNESCore::snes2pc(a++)) = 0xFF;
		*LPBYTE(rom + SNESCore::snes2pc(a++)) = 0x00;
		// STA $13D4
		*LPBYTE(rom + SNESCore::snes2pc(a++)) = 0x8D;
		*LPBYTE(rom + SNESCore::snes2pc(a++)) = 0xD4;
		*LPBYTE(rom + SNESCore::snes2pc(a++)) = 0x13;
		// PLX
		*LPBYTE(rom + SNESCore::snes2pc(a++)) = 0xFA;
		// RTL
		*LPBYTE(rom + SNESCore::snes2pc(a++)) = 0x6B;

		// Medusa
		a = 0x86C4A7;
		// shift up STZ and LDA
		*LPBYTE(rom + SNESCore::snes2pc(a)) = *LPBYTE(rom + SNESCore::snes2pc(a + 2)); a++;
		*LPBYTE(rom + SNESCore::snes2pc(a)) = *LPBYTE(rom + SNESCore::snes2pc(a + 2)); a++;
		*LPBYTE(rom + SNESCore::snes2pc(a)) = *LPBYTE(rom + SNESCore::snes2pc(a + 2)); a++;
		*LPBYTE(rom + SNESCore::snes2pc(a)) = *LPBYTE(rom + SNESCore::snes2pc(a + 2)); a++;
		*LPBYTE(rom + SNESCore::snes2pc(a)) = *LPBYTE(rom + SNESCore::snes2pc(a + 2)); a++;
		*LPBYTE(rom + SNESCore::snes2pc(a)) = *LPBYTE(rom + SNESCore::snes2pc(a + 2)); a++;
		// long jump to $82 handler
		*LPBYTE(rom + SNESCore::snes2pc(a++)) = 0x5C;
		*LPBYTE(rom + SNESCore::snes2pc(a++)) = 0xC0;
		*LPBYTE(rom + SNESCore::snes2pc(a++)) = 0xFF;
		*LPBYTE(rom + SNESCore::snes2pc(a++)) = 0x82;

		// Knight
		a = 0x84E80E;
		// shift up STZ and LDA
		*LPBYTE(rom + SNESCore::snes2pc(a)) = *LPBYTE(rom + SNESCore::snes2pc(a + 7)); a++;
		*LPBYTE(rom + SNESCore::snes2pc(a)) = *LPBYTE(rom + SNESCore::snes2pc(a + 7)); a++;
		*LPBYTE(rom + SNESCore::snes2pc(a)) = *LPBYTE(rom + SNESCore::snes2pc(a + 7)); a++;
		*LPBYTE(rom + SNESCore::snes2pc(a)) = *LPBYTE(rom + SNESCore::snes2pc(a + 7)); a++;
		*LPBYTE(rom + SNESCore::snes2pc(a)) = *LPBYTE(rom + SNESCore::snes2pc(a + 7)); a++;
		*LPBYTE(rom + SNESCore::snes2pc(a)) = *LPBYTE(rom + SNESCore::snes2pc(a + 7)); a++;
		*LPBYTE(rom + SNESCore::snes2pc(a)) = *LPBYTE(rom + SNESCore::snes2pc(a + 7)); a++;
		*LPBYTE(rom + SNESCore::snes2pc(a)) = *LPBYTE(rom + SNESCore::snes2pc(a + 7)); a++;
		*LPBYTE(rom + SNESCore::snes2pc(a)) = *LPBYTE(rom + SNESCore::snes2pc(a + 7)); a++;
		// LDA #B
		*LPBYTE(rom + SNESCore::snes2pc(a++)) = 0xA9;
		*LPBYTE(rom + SNESCore::snes2pc(a++)) = 0x0B;
		*LPBYTE(rom + SNESCore::snes2pc(a++)) = 0x00;
		// long jump to $82 handler
		*LPBYTE(rom + SNESCore::snes2pc(a++)) = 0x5C;
		*LPBYTE(rom + SNESCore::snes2pc(a++)) = 0xC0;
		*LPBYTE(rom + SNESCore::snes2pc(a++)) = 0xFF;
		*LPBYTE(rom + SNESCore::snes2pc(a++)) = 0x82;

		// level 1
		a = 0x83E31A;
		// shift up LDA
		*LPBYTE(rom + SNESCore::snes2pc(a)) = *LPBYTE(rom + SNESCore::snes2pc(a + 2)); a++;
		*LPBYTE(rom + SNESCore::snes2pc(a)) = *LPBYTE(rom + SNESCore::snes2pc(a + 2)); a++;
		*LPBYTE(rom + SNESCore::snes2pc(a)) = *LPBYTE(rom + SNESCore::snes2pc(a + 2)); a++;
		// long jump to $82 handler
		*LPBYTE(rom + SNESCore::snes2pc(a++)) = 0x5C;
		*LPBYTE(rom + SNESCore::snes2pc(a++)) = 0xC0;
		*LPBYTE(rom + SNESCore::snes2pc(a++)) = 0xFF;
		*LPBYTE(rom + SNESCore::snes2pc(a++)) = 0x82;

		// puwexil
		a = 0x86C51E;
		// shift up LDA
		*LPBYTE(rom + SNESCore::snes2pc(a)) = *LPBYTE(rom + SNESCore::snes2pc(a + 2)); a++;
		*LPBYTE(rom + SNESCore::snes2pc(a)) = *LPBYTE(rom + SNESCore::snes2pc(a + 2)); a++;
		*LPBYTE(rom + SNESCore::snes2pc(a)) = *LPBYTE(rom + SNESCore::snes2pc(a + 2)); a++;
		*LPBYTE(rom + SNESCore::snes2pc(a)) = *LPBYTE(rom + SNESCore::snes2pc(a + 2)); a++;
		*LPBYTE(rom + SNESCore::snes2pc(a)) = *LPBYTE(rom + SNESCore::snes2pc(a + 2)); a++;
		*LPBYTE(rom + SNESCore::snes2pc(a)) = *LPBYTE(rom + SNESCore::snes2pc(a + 2)); a++;
		// long jump to $82 handler
		*LPBYTE(rom + SNESCore::snes2pc(a++)) = 0x5C;
		*LPBYTE(rom + SNESCore::snes2pc(a++)) = 0xC0;
		*LPBYTE(rom + SNESCore::snes2pc(a++)) = 0xFF;
		*LPBYTE(rom + SNESCore::snes2pc(a++)) = 0x82;

		// mummy
		a = 0x86C18C;
		// shift up *
		*LPBYTE(rom + SNESCore::snes2pc(a)) = *LPBYTE(rom + SNESCore::snes2pc(a + 2)); a++;
		*LPBYTE(rom + SNESCore::snes2pc(a)) = *LPBYTE(rom + SNESCore::snes2pc(a + 2)); a++;
		*LPBYTE(rom + SNESCore::snes2pc(a)) = *LPBYTE(rom + SNESCore::snes2pc(a + 2)); a++;
		*LPBYTE(rom + SNESCore::snes2pc(a)) = *LPBYTE(rom + SNESCore::snes2pc(a + 2)); a++;
		*LPBYTE(rom + SNESCore::snes2pc(a)) = *LPBYTE(rom + SNESCore::snes2pc(a + 2)); a++;
		*LPBYTE(rom + SNESCore::snes2pc(a)) = *LPBYTE(rom + SNESCore::snes2pc(a + 2)); a++;
		*LPBYTE(rom + SNESCore::snes2pc(a)) = *LPBYTE(rom + SNESCore::snes2pc(a + 2)); a++;
		*LPBYTE(rom + SNESCore::snes2pc(a)) = *LPBYTE(rom + SNESCore::snes2pc(a + 2)); a++;
		*LPBYTE(rom + SNESCore::snes2pc(a)) = *LPBYTE(rom + SNESCore::snes2pc(a + 2)); a++;
		*LPBYTE(rom + SNESCore::snes2pc(a)) = *LPBYTE(rom + SNESCore::snes2pc(a + 2)); a++;
		*LPBYTE(rom + SNESCore::snes2pc(a)) = *LPBYTE(rom + SNESCore::snes2pc(a + 2)); a++;
		*LPBYTE(rom + SNESCore::snes2pc(a)) = *LPBYTE(rom + SNESCore::snes2pc(a + 2)); a++;
		// long jump to $82 handler
		*LPBYTE(rom + SNESCore::snes2pc(a++)) = 0x5C;
		*LPBYTE(rom + SNESCore::snes2pc(a++)) = 0xC0;
		*LPBYTE(rom + SNESCore::snes2pc(a++)) = 0xFF;
		*LPBYTE(rom + SNESCore::snes2pc(a++)) = 0x82;

		// death
		a = 0x86C2EB;
		// shift up *
		*LPBYTE(rom + SNESCore::snes2pc(a)) = *LPBYTE(rom + SNESCore::snes2pc(a + 2)); a++;
		*LPBYTE(rom + SNESCore::snes2pc(a)) = *LPBYTE(rom + SNESCore::snes2pc(a + 2)); a++;
		*LPBYTE(rom + SNESCore::snes2pc(a)) = *LPBYTE(rom + SNESCore::snes2pc(a + 2)); a++;
		*LPBYTE(rom + SNESCore::snes2pc(a)) = *LPBYTE(rom + SNESCore::snes2pc(a + 2)); a++;
		*LPBYTE(rom + SNESCore::snes2pc(a)) = *LPBYTE(rom + SNESCore::snes2pc(a + 2)); a++;
		*LPBYTE(rom + SNESCore::snes2pc(a)) = *LPBYTE(rom + SNESCore::snes2pc(a + 2)); a++;
		// long jump to $82 handler
		*LPBYTE(rom + SNESCore::snes2pc(a++)) = 0x5C;
		*LPBYTE(rom + SNESCore::snes2pc(a++)) = 0xC0;
		*LPBYTE(rom + SNESCore::snes2pc(a++)) = 0xFF;
		*LPBYTE(rom + SNESCore::snes2pc(a++)) = 0x82;

		// remove increment on bat
		a = 0x86BA3E;
		*LPBYTE(rom + SNESCore::snes2pc(a++)) = 0xEA;
		*LPBYTE(rom + SNESCore::snes2pc(a++)) = 0xEA;

		// reverse level support
		// death
		a = 0x8280AF;
		// JSL
		*LPBYTE(rom + SNESCore::snes2pc(a++)) = 0x22;
		*LPBYTE(rom + SNESCore::snes2pc(a++)) = 0x90;
		*LPBYTE(rom + SNESCore::snes2pc(a++)) = 0xBE;
		*LPBYTE(rom + SNESCore::snes2pc(a++)) = 0x85;
		// NOP
		*LPBYTE(rom + SNESCore::snes2pc(a++)) = 0xEA;

		// continue
		a = 0x8CFD57;
		// JSL
		*LPBYTE(rom + SNESCore::snes2pc(a++)) = 0x22;
		*LPBYTE(rom + SNESCore::snes2pc(a++)) = 0x90;
		*LPBYTE(rom + SNESCore::snes2pc(a++)) = 0xBE;
		*LPBYTE(rom + SNESCore::snes2pc(a++)) = 0x85;
		// NOP
		*LPBYTE(rom + SNESCore::snes2pc(a++)) = 0xEA;

		// new death entrance code
		a = 0x85BE90;
		// PHX
		*LPBYTE(rom + SNESCore::snes2pc(a++)) = 0xDA;
		// CMP $86
		*LPBYTE(rom + SNESCore::snes2pc(a++)) = 0xC5;
		*LPBYTE(rom + SNESCore::snes2pc(a++)) = 0x86;
		// BEQ
		*LPBYTE(rom + SNESCore::snes2pc(a++)) = 0xF0;
		*LPBYTE(rom + SNESCore::snes2pc(a++)) = 0x07;
		// STA $86
		*LPBYTE(rom + SNESCore::snes2pc(a++)) = 0x85;
		*LPBYTE(rom + SNESCore::snes2pc(a++)) = 0x86;
		// LDA #0000
		*LPBYTE(rom + SNESCore::snes2pc(a++)) = 0xA9;
		*LPBYTE(rom + SNESCore::snes2pc(a++)) = 0x00;
		*LPBYTE(rom + SNESCore::snes2pc(a++)) = 0x00;
		// BRA done
		*LPBYTE(rom + SNESCore::snes2pc(a++)) = 0x80;
		*LPBYTE(rom + SNESCore::snes2pc(a++)) = 0x16;
		// LDA $13D4
		*LPBYTE(rom + snes2pc(a++)) = 0xAD;
		*LPBYTE(rom + snes2pc(a++)) = 0xD4;
		*LPBYTE(rom + snes2pc(a++)) = 0x13;
		// ASL
		*LPBYTE(rom + snes2pc(a++)) = 0x0A;
		// ASL
		*LPBYTE(rom + snes2pc(a++)) = 0x0A;
		// ASL
		*LPBYTE(rom + snes2pc(a++)) = 0x0A;
		// ASL
		*LPBYTE(rom + snes2pc(a++)) = 0x0A;
		// ASL
		*LPBYTE(rom + snes2pc(a++)) = 0x0A;
		// XBA
		*LPBYTE(rom + snes2pc(a++)) = 0xEB;
		// CLC
		*LPBYTE(rom + snes2pc(a++)) = 0x18;
		// ADC $86
		*LPBYTE(rom + snes2pc(a++)) = 0x65;
		*LPBYTE(rom + snes2pc(a++)) = 0x86;
		// XBA
		*LPBYTE(rom + snes2pc(a++)) = 0xEB;
		// TAX
		*LPBYTE(rom + snes2pc(a++)) = 0xAA;
		// LDA long $0,X
		*LPBYTE(rom + snes2pc(a++)) = 0xBF;
		*LPBYTE(rom + snes2pc(a++)) = 0x00;
		*LPBYTE(rom + snes2pc(a++)) = 0x80;
		*LPBYTE(rom + snes2pc(a++)) = 0xA7;
		// XBA
		*LPBYTE(rom + snes2pc(a++)) = 0xEB;
		// AND #00FF
		*LPBYTE(rom + snes2pc(a++)) = 0x29;
		*LPBYTE(rom + snes2pc(a++)) = 0xFF;
		*LPBYTE(rom + snes2pc(a++)) = 0x00;
		// .done
		// STA $13D4
		*LPBYTE(rom + SNESCore::snes2pc(a++)) = 0x8D;
		*LPBYTE(rom + SNESCore::snes2pc(a++)) = 0xD4;
		*LPBYTE(rom + SNESCore::snes2pc(a++)) = 0x13;
		// PLX
		*LPBYTE(rom + SNESCore::snes2pc(a++)) = 0xFA;
		// RTL
		*LPBYTE(rom + SNESCore::snes2pc(a++)) = 0x6B;

		// force graphics reload on level transition
		*LPWORD(rom + SNESCore::snes2pc(0x86B55D)) = 0xEAEA;
		*LPWORD(rom + SNESCore::snes2pc(0x86B593)) = 0xEAEA;
	}
	else if (type == 1) {

	}
	else if (type == 2) {
		DWORD a = 0;
		DWORD addr = 0;

		// expand events
		for (unsigned i = 0; i < numLevels; i++) {
			DWORD pEvents = snes2pc(SReadWord(p_events[type] + i * 2) | (eventBank << 16));
			LPBYTE pevent = rom + pEvents;
			auto oldpevent = pevent;

			// fix table
			*LPWORD(rom + snes2pc(p_events[type] + i * 2)) = romAddr - 0x100000;

			while (*LPWORD(pevent) != 0xFFFF) {
				memcpy(rom + romAddr, pevent, 4);
				memset(pevent, 0xFF, 4);

				// get the next event
				pevent += 4;
				romAddr += 4;
			}

			// double event byte count + terminator
			// just terminator.  allow addevent/deleteevent to take care of updates
			unsigned count = /*(pevent - oldpevent) + */4;
			memset(rom + romAddr, 0xFF, count);
			romAddr += count;
		}
		// make sure we didn't use too much space
		if (romAddr >= 0x110000) return false;

		// fix the bank addresses
		eventBank = 0xA1;
		*LPBYTE(rom + snes2pc(0x80F766) + 0) = eventBank;

		// need to make room to change LDs to long
		a = 0x80FA00;
		// LDA 3,Y
		*LPBYTE(rom + snes2pc(a++)) = 0xB9;
		*LPBYTE(rom + snes2pc(a++)) = 0x03;
		*LPBYTE(rom + snes2pc(a++)) = 0x00;
		// AND #FF
		*LPBYTE(rom + snes2pc(a++)) = 0x29;
		*LPBYTE(rom + snes2pc(a++)) = 0xFF;
		*LPBYTE(rom + snes2pc(a++)) = 0x00;
		// TAX
		*LPBYTE(rom + snes2pc(a++)) = 0xAA;
		// RTS
		*LPBYTE(rom + snes2pc(a++)) = 0x60;

		// fix - jmp to shortcut, move up load and make direct
		a = 0x80F8D7;
		// PHX
		//*LPBYTE(rom + snes2pc(a++)) = 0xDA;
		// JSR $FA00
		*LPBYTE(rom + snes2pc(a++)) = 0x20;
		*LPBYTE(rom + snes2pc(a++)) = 0x00;
		*LPBYTE(rom + snes2pc(a++)) = 0xFA;
		// LDA $81XXXX, X
		*LPBYTE(rom + snes2pc(a++)) = 0xBF;
		*LPBYTE(rom + snes2pc(a++)) = 0xB8;
		*LPBYTE(rom + snes2pc(a++)) = 0xAE;
		*LPBYTE(rom + snes2pc(a++)) = 0x81;
		// TAX
		*LPBYTE(rom + snes2pc(a++)) = 0xAA;
		// PLA
		//*LPBYTE(rom + snes2pc(a++)) = 0x68;
		// PADDING NOP
		*LPBYTE(rom + snes2pc(a++)) = 0xEA;
		*LPBYTE(rom + snes2pc(a++)) = 0xEA;

		a = 0x80FA20;
		// PHB
		*LPBYTE(rom + snes2pc(a++)) = 0x8B;
		// LDA #0101
		*LPBYTE(rom + snes2pc(a++)) = 0xA9;
		*LPBYTE(rom + snes2pc(a++)) = 0x01;
		*LPBYTE(rom + snes2pc(a++)) = 0x01;
		// PHA
		*LPBYTE(rom + snes2pc(a++)) = 0x48;
		// PLB
		*LPBYTE(rom + snes2pc(a++)) = 0xAB;
		// PLB
		*LPBYTE(rom + snes2pc(a++)) = 0xAB;
		// LoadPalette*
		*LPBYTE(rom + snes2pc(a++)) = 0x22;
		*LPBYTE(rom + snes2pc(a++)) = 0x6E;
		*LPBYTE(rom + snes2pc(a++)) = 0x9D;
		*LPBYTE(rom + snes2pc(a++)) = 0x00;
		// PLB
		*LPBYTE(rom + snes2pc(a++)) = 0xAB;
		// RTS
		*LPBYTE(rom + snes2pc(a++)) = 0x60;

		// fix - jmp to shortcut, move up load and make direct
		a = 0x80F8F1;
		// PHX
		*LPBYTE(rom + snes2pc(a++)) = 0xDA;
		// JSR $FA00
		*LPBYTE(rom + snes2pc(a++)) = 0x20;
		*LPBYTE(rom + snes2pc(a++)) = 0x00;
		*LPBYTE(rom + snes2pc(a++)) = 0xFA;
		// LDA $81XXXX, X
		*LPBYTE(rom + snes2pc(a++)) = 0xBF;
		*LPBYTE(rom + snes2pc(a++)) = 0xEE;
		*LPBYTE(rom + snes2pc(a++)) = 0xAE;
		*LPBYTE(rom + snes2pc(a++)) = 0x81;
		// TAY
		*LPBYTE(rom + snes2pc(a++)) = 0xA8;
		// PLX
		*LPBYTE(rom + snes2pc(a++)) = 0xFA;
		// wrapper JSL
		*LPBYTE(rom + snes2pc(a++)) = 0x20;
		*LPBYTE(rom + snes2pc(a++)) = 0x20;
		*LPBYTE(rom + snes2pc(a++)) = 0xFA;
		// PADDING NOP
		*LPBYTE(rom + snes2pc(a++)) = 0xEA;
		*LPBYTE(rom + snes2pc(a++)) = 0xEA;

		// fix - jmp to shortcut, move up load and make direct
		a = 0x80F91A;
		// PHX
		//*LPBYTE(rom + snes2pc(a++)) = 0xDA;
		// JSR $FA00
		*LPBYTE(rom + snes2pc(a++)) = 0x20;
		*LPBYTE(rom + snes2pc(a++)) = 0x00;
		*LPBYTE(rom + snes2pc(a++)) = 0xFA;
		// LDA $81XXXX, X
		*LPBYTE(rom + snes2pc(a++)) = 0xBF;
		*LPBYTE(rom + snes2pc(a++)) = 0x4E;
		*LPBYTE(rom + snes2pc(a++)) = 0xAF;
		*LPBYTE(rom + snes2pc(a++)) = 0x81;
		// TAX
		*LPBYTE(rom + snes2pc(a++)) = 0xAA;
		// PLA
		//*LPBYTE(rom + snes2pc(a++)) = 0x68;
		// PADDING NOP
		*LPBYTE(rom + snes2pc(a++)) = 0xEA;
		*LPBYTE(rom + snes2pc(a++)) = 0xEA;
		*LPBYTE(rom + snes2pc(a++)) = 0xEA;

		// fix function call to have absolute
		a = 0x808734;
		// PHX
		*LPBYTE(rom + snes2pc(a++)) = 0xDA;
		// LDX D,$5A
		*LPBYTE(rom + snes2pc(a++)) = 0xA6;
		*LPBYTE(rom + snes2pc(a++)) = 0x5A;
		// LDA $811F2D,X
		*LPBYTE(rom + snes2pc(a++)) = 0xBF;
		*LPBYTE(rom + snes2pc(a++)) = 0x2D;
		*LPBYTE(rom + snes2pc(a++)) = 0x1F;
		*LPBYTE(rom + snes2pc(a++)) = 0x81;
		// PLX
		*LPBYTE(rom + snes2pc(a++)) = 0xFA;
		// TAY - to set codes
		*LPBYTE(rom + snes2pc(a++)) = 0xA8;
		//// PHA
		//*LPBYTE(rom + snes2pc(a++)) = 0x48;
		//// PLA
		//*LPBYTE(rom + snes2pc(a++)) = 0x68;
		// BPL
		*LPBYTE(rom + snes2pc(a++)) = 0x10;
		*LPBYTE(rom + snes2pc(a++)) = 0x07;
		// BIT #4000
		*LPBYTE(rom + snes2pc(a++)) = 0x89;
		*LPBYTE(rom + snes2pc(a++)) = 0x00;
		*LPBYTE(rom + snes2pc(a++)) = 0x40;
		// BEQ
		*LPBYTE(rom + snes2pc(a++)) = 0xF0;
		*LPBYTE(rom + snes2pc(a++)) = 0x07;
		// RTL
		*LPBYTE(rom + snes2pc(a++)) = 0x6B;
		//*LPBYTE(rom + snes2pc(a++)) = 0xEA;

		// second function call to be absolute
		a = 0x80FA10;
		// AND #00FC
		*LPBYTE(rom + snes2pc(a++)) = 0x29;
		*LPBYTE(rom + snes2pc(a++)) = 0xFC;
		*LPBYTE(rom + snes2pc(a++)) = 0x00;
		// ORA D,$58
		*LPBYTE(rom + snes2pc(a++)) = 0x05;
		*LPBYTE(rom + snes2pc(a++)) = 0x58;
		// TAX
		*LPBYTE(rom + snes2pc(a++)) = 0xAA;
		// TAY
		*LPBYTE(rom + snes2pc(a++)) = 0xA8;
		// RTS
		*LPBYTE(rom + snes2pc(a++)) = 0x60;

		a = 0x809745;
		// JSR $FA10
		*LPBYTE(rom + snes2pc(a++)) = 0x20;
		*LPBYTE(rom + snes2pc(a++)) = 0x10;
		*LPBYTE(rom + snes2pc(a++)) = 0xFA;
		// LDA $81XXXX, X
		*LPBYTE(rom + snes2pc(a++)) = 0xBF;
		*LPBYTE(rom + snes2pc(a++)) = 0x37;
		*LPBYTE(rom + snes2pc(a++)) = 0x89;
		*LPBYTE(rom + snes2pc(a++)) = 0x81;
		// TAX
		*LPBYTE(rom + snes2pc(a++)) = 0xAA;
		// TYA
		*LPBYTE(rom + snes2pc(a++)) = 0x98;

		eventOffsetLimit = 0x10000;

		//romAddr = 0x110000;
		// TODO: add other things
	}

	// <DECOMPRESS>
	// 1) decompress to the next location
	BYTE buf[0x20000];

	ZeroMemory(buf, sizeof(buf));

	std::vector<std::tuple<DWORD, unsigned>> addrList;

	if (type == 0) {
		romAddr = 0x14001D;
		// levels
		for (unsigned i = 0; i < numLevels; i++) {
			DWORD addr;
			// BG0/1 tiles
			addr = 0x86 << 16 | *LPWORD(rom + SNESCore::snes2pc(0x86889C) + (i * 2));
			addrList.push_back({ addr, i });
			// level sprites
			addr = 0x86 << 16 | *LPWORD(rom + SNESCore::snes2pc(0x868B45) + (i * 2));
			addrList.push_back({ addr, i });
			// block data
			addr = 0x86 << 16 | *LPWORD(rom + SNESCore::snes2pc(0x868000) + (i * 2));
			addrList.push_back({ addr, i });
			// scene data
			addr = 0x86 << 16 | *LPWORD(rom + SNESCore::snes2pc(0x86831e) + (i * 2));
			addrList.push_back({ addr, i });
		}

		// bosses
		for (unsigned i = 0; i < 0xC; i++) {
			DWORD addr;
			addr = 0x81 << 16 | *LPWORD(rom + SNESCore::snes2pc(region == 0 ? 0x81B9D6 : 0x81B9A0) + (i * 2));
			addrList.push_back({ addr, numLevels });
		}

		// music
		for (unsigned i = 0; i < 0x23; i++) {
			DWORD addr;
			addr = 0x81 << 16 | *LPWORD(rom + SNESCore::snes2pc(region == 0 ? 0x81B610 : 0x81B5DA) + (i * 2));
			addrList.push_back({ addr, numLevels });
		}

		// miscellaneous stuff in bank $81
		if (region == 0) {
			addrList.push_back({0x818B5D, numLevels });
			addrList.push_back({0x81B3D9, numLevels }); // English FONT/HUD
			addrList.push_back({0x81B3E2, numLevels });
			addrList.push_back({0x81B3EB, numLevels });
			addrList.push_back({0x81B3FE, numLevels });
			addrList.push_back({0x81B411, numLevels });
			addrList.push_back({0x81B424, numLevels });
			addrList.push_back({0x81B437, numLevels });
			addrList.push_back({0x81B440, numLevels });
			addrList.push_back({0x81B449, numLevels });
			addrList.push_back({0x81B455, numLevels });
			addrList.push_back({0x81B457, numLevels });
			addrList.push_back({0x81B465, numLevels });
			addrList.push_back({0x81B47D, numLevels });
			addrList.push_back({0x81B49A, numLevels });
			addrList.push_back({0x81B4A3, numLevels });
			addrList.push_back({0x81B4AC, numLevels }); // simon
			addrList.push_back({0x81B4E6, numLevels });
			addrList.push_back({0x81B503, numLevels });
			addrList.push_back({0x81B50C, numLevels });
			addrList.push_back({0x81B51A, numLevels });
			addrList.push_back({0x81B537, numLevels });
			addrList.push_back({0x81B545, numLevels });
			addrList.push_back({0x81B567, numLevels });
			addrList.push_back({0x81B5A7, numLevels });
			addrList.push_back({0x81B5CE, numLevels });
			addrList.push_back({0x81B5D7, numLevels });
			addrList.push_back({0x81B5F9, numLevels });
			addrList.push_back({0x81E267, numLevels });
			addrList.push_back({0x81E843, numLevels });
			addrList.push_back({0x81F349, numLevels });
			addrList.push_back({0x81F352, numLevels });
			addrList.push_back({0x81FA19, numLevels });
			addrList.push_back({0x81FA77, numLevels });
			addrList.push_back({0x81FACC, numLevels });
			addrList.push_back({0x81F9DC, numLevels });
		}
		else if (region == 1) {
			addrList.push_back({0x818B31, numLevels });
			addrList.push_back({0x81B3AD, numLevels }); // English FONT/HUD
			addrList.push_back({0x81B3B6, numLevels });
			addrList.push_back({0x81B3BF, numLevels });
			addrList.push_back({0x81B3D2, numLevels });
			addrList.push_back({0x81B3E5, numLevels });
			addrList.push_back({0x81B3F8, numLevels });
			addrList.push_back({0x81B40B, numLevels });
			addrList.push_back({0x81B414, numLevels });
			addrList.push_back({0x81B41D, numLevels });
			addrList.push_back({0x81B42B, numLevels });
			addrList.push_back({0x81B439, numLevels });
			addrList.push_back({0x81B451, numLevels });
			addrList.push_back({0x81B46E, numLevels });
			addrList.push_back({0x81B477, numLevels });
			addrList.push_back({0x81B480, numLevels }); // simon
			addrList.push_back({0x81B4BA, numLevels });
			addrList.push_back({0x81B4D7, numLevels });
			addrList.push_back({0x81B4E0, numLevels });
			addrList.push_back({0x81B4EE, numLevels });
			addrList.push_back({0x81B50B, numLevels });
			addrList.push_back({0x81B519, numLevels });
			addrList.push_back({0x81B536, numLevels });
			addrList.push_back({0x81B576, numLevels });
			addrList.push_back({0x81B598, numLevels });
			addrList.push_back({0x81B5A1, numLevels });
			addrList.push_back({0x81B5C3, numLevels });
			addrList.push_back({0x81E25D, numLevels });
			addrList.push_back({0x81E839, numLevels });
			addrList.push_back({0x81F09F, numLevels });
			addrList.push_back({0x81F0A8, numLevels });
			addrList.push_back({0x81FA0F, numLevels });
			addrList.push_back({0x81FA6D, numLevels });
			addrList.push_back({0x81FAC2, numLevels });
			addrList.push_back({0x81F9D2, numLevels });
		}
	}
	else if (type == 1) {
		DWORD addr = 0;
		romAddr = 0x20001D;

		// level stuff
		for (unsigned i = 0; i < numLevels - 1; i++) {
			// ???
			addr = 0x85 << 16 | *LPWORD(rom + SNESCore::snes2pc(0x8587F1) + ((i + 1) * 2));
			addrList.push_back({addr, i });
		}
		for (unsigned i = 0; i < numLevels; i++) {
			// ???
			addr = 0x85 << 16 | *LPWORD(rom + SNESCore::snes2pc(0x8588E1) + ((i + 1) * 2));
			addrList.push_back({addr, i });
		}
		for (unsigned i = 0; i < numLevels; i++) {
			// ???
			addr = 0x85 << 16 | *LPWORD(rom + SNESCore::snes2pc(0x8588ED) + ((i + 1) * 2));
			addrList.push_back({addr, i });
		}
		for (unsigned i = 0; i < 7; i++) {
			// ???
			addr = 0x85 << 16 | *LPWORD(rom + SNESCore::snes2pc(0x85F364) + (i * 2));
			addrList.push_back({addr, numLevels });
		}
		for (unsigned i = 0; i < 15; i++) {
			// FIXME: 10,14 looks fine, but doesn't decode right
			if (i == 10 || i == 14) break;
			// ???
			addr = 0x85 << 16 | *LPWORD(rom + SNESCore::snes2pc(0x858A0D) + (i * 2));
			addrList.push_back({addr, numLevels });
		}

		for (unsigned i = 0; i < 3; i++) {
			// ??? - $1E48 index
			addr = 0x85 << 16 | *LPWORD(rom + SNESCore::snes2pc(0x8585CB) + (i * 2));
			addrList.push_back({addr, numLevels });
		}

		for (unsigned i = 0; i < 12; i++) {
			// ??? - D,$3A index
			addr = 0x85 << 16 | *LPWORD(rom + SNESCore::snes2pc(0x859E4A) + (i * 2));
			addrList.push_back({addr, numLevels });
		}

		for (unsigned i = 0; i < 7; i++) {
			// ??? - $1C84 index
			addr = 0x85 << 16 | *LPWORD(rom + SNESCore::snes2pc(0x85A7AF) + (i * 2));
			addrList.push_back({addr, numLevels });
		}

		// misc
		addrList.push_back({0x858527, numLevels });
		addrList.push_back({0x858530, numLevels });
		addrList.push_back({0x858539, numLevels });
		addrList.push_back({0x858542, numLevels });
		addrList.push_back({0x8587AB, numLevels });
		addrList.push_back({0x858976, numLevels });
		addrList.push_back({0x858AD4, numLevels });
		addrList.push_back({0x858ADD, numLevels });
		addrList.push_back({0x859FBA, numLevels });
		addrList.push_back({0x85A84B, numLevels });
		addrList.push_back({0x85AA1F, numLevels });
		addrList.push_back({0x85CF2A, numLevels });
		addrList.push_back({0x85CF33, numLevels });
		addrList.push_back({0x85CF41, numLevels });
		addrList.push_back({0x85CFCF, numLevels });
		addrList.push_back({0x85CFD8, numLevels });
		addrList.push_back({0x85D01F, numLevels });
		addrList.push_back({0x85D073, numLevels });
		addrList.push_back({0x85D07C, numLevels });
		addrList.push_back({0x85F513, numLevels });
		addrList.push_back({0x85F55B, numLevels });
	}
	else if (type == 2) {
		DWORD addr = 0;
		romAddr = 0x20001D;

		for (unsigned i = 0; i < numLevels; i++) {
			addr = 0x81 << 16 | *LPWORD(rom + SNESCore::snes2pc(0x81AADA) + (i * 2));
			addrList.push_back({addr, i });
		}

		for (unsigned i = 0; i < 0x1B; i++) {
			addr = 0x81 << 16 | *LPWORD(rom + SNESCore::snes2pc(0x81AEB8) + (i * 2));
			addrList.push_back({addr, numLevels });
		}

		for (unsigned i = 2; i < 0x16; i++) {
			addr = 0x81 << 16 | *LPWORD(rom + SNESCore::snes2pc(0x81A22A) + (i * 2));
			addrList.push_back({addr, numLevels });
		}

		for (unsigned i = 0; i < 0x13; i++) {
			addr = 0x81 << 16 | *LPWORD(rom + SNESCore::snes2pc(0x81A24C) + (i * 2));
			addrList.push_back({addr, numLevels });
		}

		for (unsigned i = 0; i < 0x5; i++) {
			addr = 0x81 << 16 | *LPWORD(rom + SNESCore::snes2pc(0x81A254) + (i * 2));
			addrList.push_back({addr, numLevels });
		}

		for (unsigned i = 0; i < 0xA; i++) {
			addr = 0x81 << 16 | *LPWORD(rom + SNESCore::snes2pc(0x81A25E) + (i * 2));
			addrList.push_back({addr, numLevels });
		}

		for (unsigned i = 0; i < 0x3A; i++) {
			addr = 0x81 << 16 | *LPWORD(rom + SNESCore::snes2pc(0x818937) + (i * 2));
			addrList.push_back({addr, numLevels });
		}

		addrList.push_back({0x81839F, numLevels });
		addrList.push_back({0x818920, numLevels });
		addrList.push_back({0x8183B2, numLevels });
		addrList.push_back({0x81A6E6, numLevels });
		addrList.push_back({0x81840D, numLevels });
		addrList.push_back({0x818443, numLevels });
		addrList.push_back({0x818451, numLevels });
		addrList.push_back({0x8183E6, numLevels });
		//addrList.push_back({0x8185BE, numLevels });
		addrList.push_back({0x8185B3, numLevels });
		addrList.push_back({0x81846D, numLevels });
	}

	std::map<DWORD, DWORD> addrMap;
	// combineMap[level][ramAddr] = romAddr
	std::map<unsigned, std::map<DWORD, DWORD>> combineMap;

	for (auto t : addrList) {
		auto a = std::get<DWORD>(t);
		auto l = std::get<unsigned>(t);
		if (!a) continue;

		DWORD tableAddr = a;

		if ((tableAddr & 0xFFFF) != 0xFFFF) {
			LPBYTE gfxPtr = (rom + SNESCore::snes2pc(tableAddr));

			// FIXME: the actual check is against 0?
			if (*LPWORD(gfxPtr) != 0xFFFF) {
				WORD gfx = *LPWORD(gfxPtr) & ~0x0300;
				// TODO: this check needs to be cleaned up.
				unsigned addrBytes = (gfx == 0 || gfx == 0xA001 || gfx == 2 || gfx == 3 || (type == 2 && gfx == 0x8001)) ? 2 : 3;

				gfxPtr += 2;

				bool skipSize = false;

				std::deque<LPBYTE> combineAddrList;
				bool done = false;
				while (*LPWORD(gfxPtr) != 0xFFFF && !done) {
					if (skipSize) {
						skipSize = false;
						gfxPtr += 2;
					}

					DWORD dstAddr = *LPDWORD(gfxPtr) & (addrBytes == 3 ? 0xFFFFFF : 0xFFFF);
					WORD dstOffset = dstAddr & 0xFFFF;
					gfxPtr += addrBytes;
					DWORD srcAddr = *LPDWORD(gfxPtr) & 0xFFFFFF;
					gfxPtr += 3;

					if (gfx == 2 && dstOffset == 0xFFFE) {
						// rewind and fix the source/dest
						gfxPtr -= 3;

						dstAddr = *LPDWORD(gfxPtr) & 0xFFFFFF;
						dstOffset = dstAddr & 0xFFFF;

						gfxPtr += addrBytes;
						srcAddr = *LPDWORD(gfxPtr) & 0xFFFFFF;
						gfxPtr += 3;

						skipSize = true;

						continue;
					}

					WORD size = *LPWORD(rom + SNESCore::snes2pc(srcAddr));
					srcAddr += 2;

					if (srcAddr >= SNESCore::pc2snes(0x100000)) continue;

					if (addrMap.count(srcAddr & 0xFFFFFF) && l != 0x1B && l != 0x34) {
						unsigned snesAddr = addrMap[srcAddr & 0xFFFFFF];
						memcpy(gfxPtr - 3, &snesAddr, 3);

						// store the offset to the pointers that point here
						for (unsigned i = 0; i < 3; i++) {
							DWORD val = 0xFFFFFFFF;
							LPBYTE offset = rom + snes2pc(snesAddr) - (4 * (i + 1));
							if (!memcmp(offset, &val, 3)) {
								DWORD ptrOffset = (gfxPtr - 3) - rom;
								memcpy(offset, &ptrOffset, 3);
								break;
							}
						}

						if (gfx == 0xA001 || (gfx == 0x8001 && type == 2)) {
							done = true;
						}
						continue;
					}

					//if (combineAddrList.empty()) {
						ZeroMemory(buf, sizeof(buf));
					//}

					// combine blocks and scenes into full $2000 region
					DWORD baseAddr = 0;
					DWORD baseBuf = 0;
					WORD newSize = 0;
					unsigned origSize = 0;
					bool setBase = false;

					if (GetBaseAddr(gfx, dstAddr, baseAddr, newSize) && (l < numLevels || type == 2)) {
						baseBuf = (dstAddr - baseAddr) * (addrBytes == 3 ? 1 : 2);
						dstAddr = baseAddr;
						// is it ok to always pad regions?
						if (combineMap[l].count(baseAddr)) {
							// set the first to the new location
							//memcpy(combineAddrList.front(), &snesAddr, 3);
							// make sure the base is the full region base
							//*LPWORD(combineAddrList.front() - addrBytes) = baseAddr;
							DWORD snesAddr = combineMap[l][baseAddr];

							for (unsigned i = 0; i < 3; i++) {
								DWORD val = 0xFFFFFFFF;
								LPBYTE offset = rom + snes2pc(snesAddr) - (4 * (i + 1));
								if (!memcmp(offset, &val, 3)) {
									DWORD ptrOffset = (gfxPtr - 3) - rom;
									memcpy(offset, &ptrOffset, 3);
									break;
								}
							}

							GFXRLE(rom, rom + snes2pc(snesAddr) + 3 + baseBuf, SNESCore::snes2pc(srcAddr), size, type);
							memcpy(gfxPtr - 3, &zeroCompressedAddr, 3);
							continue;
						}
						else {
							setBase = true;
						}
					}

					origSize = GFXRLE(rom, buf + baseBuf, SNESCore::snes2pc(srcAddr), size, type);
					if (setBase) origSize = newSize;

					//if (GetBaseAddr(gfx, dstAddr, baseAddr, newSize) && l < numLevels) {
					//	// add the current address to the list of addresses to combine
					//	combineAddrList.push_back(gfxPtr - 3);
					//	// WRAM is byte address all others are word addresses
					//	baseBuf = (dstAddr - baseAddr) * (addrBytes == 3 ? 1 : 2);
					//}

					//auto origSize = GFXRLE(rom, buf + baseBuf, SNESCore::snes2pc(srcAddr), size, type);

					//DWORD nextBaseAddr = 0;
					//WORD tempSize = 0;
					//GetBaseAddr(gfx, *LPDWORD(gfxPtr)  & (addrBytes == 3 ? 0xFFFFFF : 0xFFFF), nextBaseAddr, tempSize)) {
					//if (!combineAddrList.empty()) {
					//	if (*LPWORD(gfxPtr) != 0xFFFF && baseAddr == nextBaseAddr) {
					//		continue;
					//	}
					//	dstAddr = baseAddr;
					//	dstOffset = dstAddr & 0xFFFF;
					//	origSize = newSize;
					//}

					// add some extra space for the JP logo
					if (type == 0 && a == 0x81B439) {
						origSize += 0x200;
					}

					// Pad out tiles to $2000 regions.  There is multiple re-use of background tiles at $3000
					// offset so this is easier than trying to splice things together.
					if (type == 0 && (gfx == 0 && (2 * baseAddr == GetTileVramByteAddr(l)) && (l < 0x42))) {
						if (origSize < 0x2000) origSize = 0x2000;
					}

					// round up to the next 0x1D
					//while ((romAddr % 0x20) != 0x1D) romAddr++;
					auto offset = (romAddr % 0x20);
					romAddr += (0x1D <= offset ? 0x1D : 0x3D) - offset;

					//// make sure uncompressed data doesn't cross a bank
					//if ((romAddr / 0x8000) != ((romAddr + origSize + 2) / 0x8000)) {
					//	romAddr = (romAddr + 0x8000) & 0xFF8000;
					//	romAddr += 0x1D;
					//}

					if ((romAddr + origSize + 2 < romSize)) {
						// rewrite source pointer to current address
						unsigned snesAddr = SNESCore::pc2snes(romAddr);
						memcpy(gfxPtr - 3, &snesAddr, 3);

						if (type != 2 && combineAddrList.size() <= 1) {
							addrMap[srcAddr & 0xFFFFFF] = snesAddr & 0xFFFFFF;
						}

						// store the offset to the pointers that point here
						for (unsigned i = 0; i < 1; i++) {
							DWORD val = 0xFFFFFFFF;
							LPBYTE offset = rom + romAddr - (4 * (i + 1));
							if (!memcmp(offset, &val, 3)) {
								// we should always get here
								DWORD ptrOffset = (gfxPtr - 3) - rom;
								memcpy(offset, &ptrOffset, 3);
								break;
							}
						}

						// copy uncompressed data
						unsigned currentSize = 2;
						//memcpy(rom + romAddr, buf, origSize);

						for (unsigned j = 0; j < origSize; j++) {
							if (j /*% 0x400*/ == 0) {
								*(rom + romAddr + currentSize++) = 0x80;

								//// hack to track multiple compressed regions forming the same data
								//if (levelNum < 0x44) {
								//	expandedOffset[levelNum][dstAddr].first = romAddr + currentSize;
								//	expandedOffset[levelNum][dstAddr].second = origSize;

								//}
								//else if (a == (region == 0 ? 0x81B3E2 : 0x81B3B6)) {
								//	// default whip, powerup, candle, orb, etc
								//	expandedOffset[0x44][dstAddr].first = romAddr + currentSize;
								//	expandedOffset[0x44][dstAddr].second = origSize;
								//}
							}
							*(rom + romAddr + currentSize++) = buf[j];
						}
						*LPWORD(rom + romAddr) = currentSize;
						// since we are tile aligned then set previous 29 Bytes to 0xFF
						// helps to distinguish control information in tile editor

						if (combineAddrList.empty()) {
							BYTE testbuf[0x10000];
							auto testorigSize = GFXRLE(rom + romAddr, testbuf, 2, currentSize, type);
							if (testorigSize == origSize) {
								for (unsigned k = 0; k < origSize; k++) {
									if (buf[k] != testbuf[k]) {
										int test = 0;
										test++;
									}
								}
							}

							if (setBase) {
								combineMap[l][dstAddr] = pc2snes(romAddr);

								// make sure the base is the full region base
								*LPWORD(gfxPtr - 3 - addrBytes) = baseAddr;
							}
						}
						else {
							// set the first to the new location
							memcpy(combineAddrList.front(), &snesAddr, 3);
							// make sure the base is the full region base
							*LPWORD(combineAddrList.front() - addrBytes) = baseAddr;

							for (unsigned i = 0; i < 3; i++) {
								DWORD val = 0xFFFFFFFF;
								LPBYTE offset = rom + snes2pc(snesAddr) - (4 * (i + 1));
								if (!memcmp(offset, &val, 3)) {
									DWORD ptrOffset = combineAddrList.front() - rom;
									memcpy(offset, &ptrOffset, 3);
									break;
								}
							}

							combineAddrList.pop_front();
							for (auto &a : combineAddrList) {
								// fill in a separate address for the 
								memcpy(a, &zeroCompressedAddr, 3);
							}
							combineAddrList.clear();
						}

						// remove size and control byte
						unsigned newSize = currentSize - 3 + 0x20;
						// pad to next tile size and add back 3 bytes
						newSize = ((newSize + 0x20 - 1) / 0x20) * 0x20 + 3;
						memset(rom + romAddr + currentSize, 0x00, newSize - currentSize);
						romAddr += newSize;
					}

					if (gfx == 0xA001 || (gfx == 0x8001 && type == 2)) {
						done = true;
					}
				}
			}
		}
	}

	GenerateExpandedOffset();

	// Set the remaining part of memory to FFs
	//memset(rom + romAddr + 0x20, 0xFF, romSize - romAddr - 0x20);

	// fitting in existing space (only ~4 bytes remaining)
	// maintaining bank crossing against multiple calls ($00 gets reloaded with base)
	// - could only support crossing one bank (<$8000 bytes long).  Probably ok except for full character.
	// supporting bank crossing
	// - if sum of offset + count == 0, OR in #8000
	// - can't change $00 because we will lose that information after <=$400 bytes.  May be able to store it.
	// - Don't want to use another piece of memory because it needs to be maintained across frames.
	// - Still can't allow bank to cross on first 3 bytes
	// - need to increment starting address by 2
	// 1F00 address?
	// 1F06 compressed bytes read
	// 1F08 total compressed bytes
	// 1F0C $400 (max bytes to read this frame)
	// 1F14 current state of reading (decompression type in progress)

	// JSL/RTL to handle bank crossing

	// First check that regions not crossing banks still function.
	// Introduce bank change and see if that fixes things.

	// Verify JMP and BRA targets.
	// Verify INC of memory sets Z bit
	// Verify bank increment works
	// Verify multi-segment decompression/copy still works

#define DECOMP16BIT 1
	DWORD addr = 0;
	// Need $32 bytes prior to $F0
	// modify jmp address
	addr = type == 0 ? 0x8286DB : type == 1 ? 0x809538 : 0x809C6B;
	*LPBYTE(rom + snes2pc(addr++)) = 0x5C;
	*LPBYTE(rom + snes2pc(addr++)) = 0x70;
	*LPBYTE(rom + snes2pc(addr++)) = 0xFF;
	*LPBYTE(rom + snes2pc(addr++)) = 0x83;

	addr = 0x83FF70;

	// COMPRESSED TYPE COMPARE - 14
	//CMP #$A0
	*LPBYTE(rom + snes2pc(addr++)) = 0xC9;
	*LPBYTE(rom + snes2pc(addr++)) = 0xA0;
	//BCC $03
	*LPBYTE(rom + snes2pc(addr++)) = 0x90;
	*LPBYTE(rom + snes2pc(addr++)) = 0x04;
	//JMP $86DF // bank $82
	*LPBYTE(rom + snes2pc(addr++)) = 0x5C;
	*LPBYTE(rom + snes2pc(addr++)) = type == 0 ? 0xDF : type == 1 ? 0x3C : 0x6F;
	*LPBYTE(rom + snes2pc(addr++)) = type == 0 ? 0x86 : type == 1 ? 0x95 : 0x9C;
	*LPBYTE(rom + snes2pc(addr++)) = type == 0 ? 0x82 : type == 1 ? 0x80 : 0x80;
	//CMP #$80    ; check for exactly 80 (no length)
	*LPBYTE(rom + snes2pc(addr++)) = 0xC9;
	*LPBYTE(rom + snes2pc(addr++)) = 0x80;
	//BEQ $03
	*LPBYTE(rom + snes2pc(addr++)) = 0xF0;
	*LPBYTE(rom + snes2pc(addr++)) = 0x04;
	//JMP $8754    ; go to regular code // bank $82
	*LPBYTE(rom + snes2pc(addr++)) = 0x5C;
	*LPBYTE(rom + snes2pc(addr++)) = type == 0 ? 0x54 : type == 1 ? 0xB1 : 0xE4;
	*LPBYTE(rom + snes2pc(addr++)) = type == 0 ? 0x87 : type == 1 ? 0x95 : 0x9C;
	*LPBYTE(rom + snes2pc(addr++)) = type == 0 ? 0x82 : type == 1 ? 0x80 : 0x80;

	// INIT - 6
	//LDX D,6
	*LPBYTE(rom + snes2pc(addr++)) = 0xA6;
	*LPBYTE(rom + snes2pc(addr++)) = 0x06;
	//LDY D,$C
	*LPBYTE(rom + snes2pc(addr++)) = 0xA4;
	*LPBYTE(rom + snes2pc(addr++)) = 0x0C;

	// New code to see if this is the first chunk decompressing
#if DECOMP16BIT == 1
	//PHP
	*LPBYTE(rom + snes2pc(addr++)) = 0x08;
	// REP #20
	*LPBYTE(rom + snes2pc(addr++)) = 0xC2;
	*LPBYTE(rom + snes2pc(addr++)) = 0x20;
#endif

	//LDA D,14
	*LPBYTE(rom + snes2pc(addr++)) = 0xA5;
	*LPBYTE(rom + snes2pc(addr++)) = 0x14;
#if DECOMP16BIT == 1
	//AND FF
	*LPBYTE(rom + snes2pc(addr++)) = 0x29;
	*LPBYTE(rom + snes2pc(addr++)) = 0xFF;
	*LPBYTE(rom + snes2pc(addr++)) = 0x00;
#endif
	//BNE LOOP
	*LPBYTE(rom + snes2pc(addr++)) = 0xD0;
	*LPBYTE(rom + snes2pc(addr++)) = 0x09;
	// JSR
	*LPBYTE(rom + snes2pc(addr++)) = 0x20;
	*LPBYTE(rom + snes2pc(addr++)) = 0xC8; // FIXME
	*LPBYTE(rom + snes2pc(addr++)) = 0xFF;
	// JSR
	*LPBYTE(rom + snes2pc(addr++)) = 0x20;
	*LPBYTE(rom + snes2pc(addr++)) = 0xC8; // FIXME
	*LPBYTE(rom + snes2pc(addr++)) = 0xFF;
	// JSR
	*LPBYTE(rom + snes2pc(addr++)) = 0x20;
	*LPBYTE(rom + snes2pc(addr++)) = 0xC8; // FIXME
	*LPBYTE(rom + snes2pc(addr++)) = 0xFF;

	//LDA #$6
	*LPBYTE(rom + snes2pc(addr++)) = 0xA9;
	*LPBYTE(rom + snes2pc(addr++)) = 0x06;
#if DECOMP16BIT == 1
	*LPBYTE(rom + snes2pc(addr++)) = 0x00;
#endif
	//STA D,14
	*LPBYTE(rom + snes2pc(addr++)) = 0x85;
	*LPBYTE(rom + snes2pc(addr++)) = 0x14;

	// LOOP - ??
	// Address is incremented so indexed is not necessary
	//LDA [D,0]
	*LPBYTE(rom + snes2pc(addr++)) = 0xA7;
	*LPBYTE(rom + snes2pc(addr++)) = 0x00;
	//STA (D,3),Y
	*LPBYTE(rom + snes2pc(addr++)) = 0x91;
	*LPBYTE(rom + snes2pc(addr++)) = 0x03;
	//INX
	*LPBYTE(rom + snes2pc(addr++)) = 0xE8;
	//INY
	*LPBYTE(rom + snes2pc(addr++)) = 0xC8;

	// inline part of the JSR to save time
#if DECOMP16BIT == 1
	//INC D,0 - 2
	*LPBYTE(rom + snes2pc(addr++)) = 0xE6;
	*LPBYTE(rom + snes2pc(addr++)) = 0x00;
	////BNE next - 2
	//*LPBYTE(rom + snes2pc(addr++)) = 0xD0;
	//*LPBYTE(rom + snes2pc(addr++)) = 0x05;
	//// JSR to partial bank change
	//*LPBYTE(rom + snes2pc(addr++)) = 0x20;
	//*LPBYTE(rom + snes2pc(addr++)) = 0xD4; // FIXME
	//*LPBYTE(rom + snes2pc(addr++)) = 0xFF;
	////BRA
	//*LPBYTE(rom + snes2pc(addr++)) = 0x80;
	//*LPBYTE(rom + snes2pc(addr++)) = 0x09;
	//CPX D,$08
	*LPBYTE(rom + snes2pc(addr++)) = 0xE4;
	*LPBYTE(rom + snes2pc(addr++)) = 0x08;
	//BCS end
	*LPBYTE(rom + snes2pc(addr++)) = 0xB0;
	*LPBYTE(rom + snes2pc(addr++)) = 0x13; // $10
	//INX
	*LPBYTE(rom + snes2pc(addr++)) = 0xE8;
	//INY
	*LPBYTE(rom + snes2pc(addr++)) = 0xC8;
	//INC D,0 - 2
	*LPBYTE(rom + snes2pc(addr++)) = 0xE6;
	*LPBYTE(rom + snes2pc(addr++)) = 0x00;
	//BNE next - 2
	*LPBYTE(rom + snes2pc(addr++)) = 0xD0;
	*LPBYTE(rom + snes2pc(addr++)) = 0x03;
	// JSR to partial bank change
	*LPBYTE(rom + snes2pc(addr++)) = 0x20;
	*LPBYTE(rom + snes2pc(addr++)) = 0xCC; // FIXME
	*LPBYTE(rom + snes2pc(addr++)) = 0xFF;
#else
	// REP #20
	*LPBYTE(rom + snes2pc(addr++)) = 0xC2;
	*LPBYTE(rom + snes2pc(addr++)) = 0x20;
	//INC D,0 - 2
	*LPBYTE(rom + snes2pc(addr++)) = 0xE6;
	*LPBYTE(rom + snes2pc(addr++)) = 0x00;
	// SEP #20
	*LPBYTE(rom + snes2pc(addr++)) = 0xE2;
	*LPBYTE(rom + snes2pc(addr++)) = 0x20;
	//BNE next - 2
	*LPBYTE(rom + snes2pc(addr++)) = 0xD0;
	*LPBYTE(rom + snes2pc(addr++)) = 0x03;
	// JSR to partial bank change
	*LPBYTE(rom + snes2pc(addr++)) = 0x20;
	*LPBYTE(rom + snes2pc(addr++)) = 0xD0; // FIXME
	*LPBYTE(rom + snes2pc(addr++)) = 0xFF;
#endif

	// CHECK FOR COMPLETION
	//CPX D,$08
	*LPBYTE(rom + snes2pc(addr++)) = 0xE4;
	*LPBYTE(rom + snes2pc(addr++)) = 0x08;
	//BCS
	*LPBYTE(rom + snes2pc(addr++)) = 0xB0;
	*LPBYTE(rom + snes2pc(addr++)) = 0x06;
	//CPY D,$1C
	*LPBYTE(rom + snes2pc(addr++)) = 0xC4;
	*LPBYTE(rom + snes2pc(addr++)) = 0x1C;
	//BCS end
	*LPBYTE(rom + snes2pc(addr++)) = 0xB0;
	*LPBYTE(rom + snes2pc(addr++)) = 0x04; // $10
	//BRA
	*LPBYTE(rom + snes2pc(addr++)) = 0x80;
#if DECOMP16BIT == 1
	*LPBYTE(rom + snes2pc(addr++)) = 0xE1;
#else
	*LPBYTE(rom + snes2pc(addr++)) = 0xE5;
#endif
	// POST LOOP HANDLER - ??
	//STZ D,14
	*LPBYTE(rom + snes2pc(addr++)) = 0x64;
	*LPBYTE(rom + snes2pc(addr++)) = 0x14;
	//STX D,6
	*LPBYTE(rom + snes2pc(addr++)) = 0x86;
	*LPBYTE(rom + snes2pc(addr++)) = 0x06;
	//STY D,$C
	*LPBYTE(rom + snes2pc(addr++)) = 0x84;
	*LPBYTE(rom + snes2pc(addr++)) = 0x0C;
#if DECOMP16BIT == 1
	//PHP
	*LPBYTE(rom + snes2pc(addr++)) = 0x28;
#endif
	//JMP $86A2 // post loop handler
	*LPBYTE(rom + snes2pc(addr++)) = 0x5C;
	*LPBYTE(rom + snes2pc(addr++)) = type == 0 ? 0xA2 : type == 1 ? 0xFF : 0x32;
	*LPBYTE(rom + snes2pc(addr++)) = type == 0 ? 0x86 : type == 1 ? 0x94 : 0x9C;
	*LPBYTE(rom + snes2pc(addr++)) = type == 0 ? 0x82 : type == 1 ? 0x80 : 0x80;

	addr = 0x83FFC8;

	// INCREMENT ADDRESS AND BANK (if necessary)
#if DECOMP16BIT == 0
	// REP #20
	*LPBYTE(rom + snes2pc(addr++)) = 0xC2;
	*LPBYTE(rom + snes2pc(addr++)) = 0x20;
#endif
	//INC D,0 - 2
	*LPBYTE(rom + snes2pc(addr++)) = 0xE6;
	*LPBYTE(rom + snes2pc(addr++)) = 0x00;
#if DECOMP16BIT == 0
	// SEP #20
	*LPBYTE(rom + snes2pc(addr++)) = 0xE2;
	*LPBYTE(rom + snes2pc(addr++)) = 0x20;
	//BNE next - 2
	*LPBYTE(rom + snes2pc(addr++)) = 0xD0;
	*LPBYTE(rom + snes2pc(addr++)) = 0x06;
#else
	//BNE next - 2
	*LPBYTE(rom + snes2pc(addr++)) = 0xD0;
	*LPBYTE(rom + snes2pc(addr++)) = 0x0A;
#endif
	// increment the bank
	//INC D,2 - 2
	*LPBYTE(rom + snes2pc(addr++)) = 0xE6;
	*LPBYTE(rom + snes2pc(addr++)) = 0x02;
	// set high order bit for lorom ROM accesses
#if DECOMP16BIT == 1
	// SEP #20
	*LPBYTE(rom + snes2pc(addr++)) = 0xE2;
	*LPBYTE(rom + snes2pc(addr++)) = 0x20;
#endif
	//LDA #80 - 2
	*LPBYTE(rom + snes2pc(addr++)) = 0xA9;
	*LPBYTE(rom + snes2pc(addr++)) = 0x80;
	//STA D,1 - 2
	*LPBYTE(rom + snes2pc(addr++)) = 0x85;
	*LPBYTE(rom + snes2pc(addr++)) = 0x01;
#if DECOMP16BIT == 1
	// REP #20
	*LPBYTE(rom + snes2pc(addr++)) = 0xC2;
	*LPBYTE(rom + snes2pc(addr++)) = 0x20;
#endif
	// fall through after incrementing bank
	// RTS
	*LPBYTE(rom + snes2pc(addr++)) = 0x60;

	// rewrite the branch to a JMP
	addr = type == 0 ? 0x8286AE : type == 1 ? 0x80950B : 0x809C3E;
	*LPBYTE(rom + snes2pc(addr++)) = 0x5C;
	*LPBYTE(rom + snes2pc(addr++)) = 0xEE;
	*LPBYTE(rom + snes2pc(addr++)) = 0xFF;
	*LPBYTE(rom + snes2pc(addr++)) = 0x83;
	*LPBYTE(rom + snes2pc(addr++)) = 0xEA;

	addr = 0x83FFEE;

	// BNE to CMP
	//BNE $03
	*LPBYTE(rom + snes2pc(addr++)) = 0xD0;
	*LPBYTE(rom + snes2pc(addr++)) = 0x04;
	// JMP to #$1 handler
	*LPBYTE(rom + snes2pc(addr++)) = 0x5C;
	*LPBYTE(rom + snes2pc(addr++)) = type == 0 ? 0x9C : type == 1 ? 0xF9 : 0x2C;
	*LPBYTE(rom + snes2pc(addr++)) = type == 0 ? 0x87 : type == 1 ? 0x95 : 0x9D;
	*LPBYTE(rom + snes2pc(addr++)) = type == 0 ? 0x82 : type == 1 ? 0x80 : 0x80;
	// CMP to #6
	*LPBYTE(rom + snes2pc(addr++)) = 0xC9;
	*LPBYTE(rom + snes2pc(addr++)) = 0x06;
	// BEQ $3
	*LPBYTE(rom + snes2pc(addr++)) = 0xF0;
	*LPBYTE(rom + snes2pc(addr++)) = 0x04;
	// JMP to CMP for #2
	*LPBYTE(rom + snes2pc(addr++)) = 0x5C;
	*LPBYTE(rom + snes2pc(addr++)) = type == 0 ? 0xB3 : type == 1 ? 0x10 : 0x43;
	*LPBYTE(rom + snes2pc(addr++)) = type == 0 ? 0x86 : type == 1 ? 0x95 : 0x9C;
	*LPBYTE(rom + snes2pc(addr++)) = type == 0 ? 0x82 : type == 1 ? 0x80 : 0x80;
	// JMP to new handler
	*LPBYTE(rom + snes2pc(addr++)) = 0x4C;
	*LPBYTE(rom + snes2pc(addr++)) = 0x80;
	*LPBYTE(rom + snes2pc(addr++)) = 0xFF;

	// zero out unknown field
	if (type == 0) {
		auto currLevel = level;
		for (unsigned i = 0; i < numLevels; ++i) {
			level = i;
			LoadLevel();
			unsigned eventNum = 0;
			for (auto &e : eventTable) {
				if (eventNum) {
					if (e.type == e.unknown) {
						e.unknown = 3;
					}
					else if (e.unknown) {
						e.unknown = e.unknown;
					}
				}
				eventNum++;
			}
			SaveEvents();
		}
		level = currLevel;

#if 0
		DWORD a = 0x80FF30;
		// JSL FindAvailActiveEnemy
		*LPBYTE(rom + snes2pc(a++)) = 0x22;
		*LPBYTE(rom + snes2pc(a++)) = 0xF1;
		*LPBYTE(rom + snes2pc(a++)) = 0xD7;
		*LPBYTE(rom + snes2pc(a++)) = 0x00;
		// BCC next
		*LPBYTE(rom + snes2pc(a++)) = 0x90;
		*LPBYTE(rom + snes2pc(a++)) = 0x08;
		// RTL
		*LPBYTE(rom + snes2pc(a++)) = 0x6B;
		// JSL FindAvailableCandleSlot
		*LPBYTE(rom + snes2pc(a++)) = 0x22;
		*LPBYTE(rom + snes2pc(a++)) = 0x05;
		*LPBYTE(rom + snes2pc(a++)) = 0xD8;
		*LPBYTE(rom + snes2pc(a++)) = 0x00;
		// BCC next
		*LPBYTE(rom + snes2pc(a++)) = 0x90;
		*LPBYTE(rom + snes2pc(a++)) = 0x01;
		// RTL
		*LPBYTE(rom + snes2pc(a++)) = 0x6B;
		// .next
		// LDA D,$88
		*LPBYTE(rom + snes2pc(a++)) = 0xA5;
		*LPBYTE(rom + snes2pc(a++)) = 0x88;
		// BNE last
		*LPBYTE(rom + snes2pc(a++)) = 0xD0;
		*LPBYTE(rom + snes2pc(a++)) = 0x09;
		// LDA 2,Y
		*LPBYTE(rom + snes2pc(a++)) = 0xB9;
		*LPBYTE(rom + snes2pc(a++)) = 0x02;
		*LPBYTE(rom + snes2pc(a++)) = 0x00;
		// AND #10
		*LPBYTE(rom + snes2pc(a++)) = 0x29;
		*LPBYTE(rom + snes2pc(a++)) = 0x01;
		*LPBYTE(rom + snes2pc(a++)) = 0x00;
		// BEQ last
		*LPBYTE(rom + snes2pc(a++)) = 0xF0;
		*LPBYTE(rom + snes2pc(a++)) = 0x01;
		// SEC
		*LPBYTE(rom + snes2pc(a++)) = 0x38;
		// .last
		// RTL
		*LPBYTE(rom + snes2pc(a++)) = 0x6B;

		a = 0x80D6AD;
		*LPBYTE(rom + snes2pc(a++)) = 0x30;
		*LPBYTE(rom + snes2pc(a++)) = 0xFF;
		*LPBYTE(rom + snes2pc(a++)) = 0x80;

		a = 0x80D6B4;
		*LPBYTE(rom + snes2pc(a++)) = 0x37;
		*LPBYTE(rom + snes2pc(a++)) = 0xFF;
		*LPBYTE(rom + snes2pc(a++)) = 0x80;

		a = 0x80D772;
		*LPBYTE(rom + snes2pc(a++)) = 0x37;
		*LPBYTE(rom + snes2pc(a++)) = 0xFF;
		*LPBYTE(rom + snes2pc(a++)) = 0x80;
#else
		DWORD a = 0x80FF30;
		// JSL FindAvailActiveEnemy
		*LPBYTE(rom + snes2pc(a++)) = 0x22;
		*LPBYTE(rom + snes2pc(a++)) = 0xF1;
		*LPBYTE(rom + snes2pc(a++)) = 0xD7;
		*LPBYTE(rom + snes2pc(a++)) = 0x00;
		// BCC next
		*LPBYTE(rom + snes2pc(a++)) = 0x90;
		*LPBYTE(rom + snes2pc(a++)) = 0x08;
		// RTS
		*LPBYTE(rom + snes2pc(a++)) = 0x60;
		// JSL FindAvailableCandleSlot
		*LPBYTE(rom + snes2pc(a++)) = 0x22;
		*LPBYTE(rom + snes2pc(a++)) = 0x05;
		*LPBYTE(rom + snes2pc(a++)) = 0xD8;
		*LPBYTE(rom + snes2pc(a++)) = 0x00;
		// BCC next
		*LPBYTE(rom + snes2pc(a++)) = 0x90;
		*LPBYTE(rom + snes2pc(a++)) = 0x01;
		// RTS
		*LPBYTE(rom + snes2pc(a++)) = 0x60;
		// .next
		// LDA 2,Y
		*LPBYTE(rom + snes2pc(a++)) = 0xB9;
		*LPBYTE(rom + snes2pc(a++)) = 0x02;
		*LPBYTE(rom + snes2pc(a++)) = 0x00;
		// AND #03
		*LPBYTE(rom + snes2pc(a++)) = 0x29;
		*LPBYTE(rom + snes2pc(a++)) = 0x03;
		*LPBYTE(rom + snes2pc(a++)) = 0x00;
		// BEQ fail
		*LPBYTE(rom + snes2pc(a++)) = 0xF0;
		*LPBYTE(rom + snes2pc(a++)) = 0x0C;
		// CMP #03
		*LPBYTE(rom + snes2pc(a++)) = 0xC9;
		*LPBYTE(rom + snes2pc(a++)) = 0x03;
		*LPBYTE(rom + snes2pc(a++)) = 0x00;
		// BEQ ok
		*LPBYTE(rom + snes2pc(a++)) = 0xF0;
		*LPBYTE(rom + snes2pc(a++)) = 0x0A;
		// EOR $88
		*LPBYTE(rom + snes2pc(a++)) = 0x45;
		*LPBYTE(rom + snes2pc(a++)) = 0x88;
		// AND #01
		*LPBYTE(rom + snes2pc(a++)) = 0x29;
		*LPBYTE(rom + snes2pc(a++)) = 0x01;
		*LPBYTE(rom + snes2pc(a++)) = 0x00;
		// BEQ ok
		*LPBYTE(rom + snes2pc(a++)) = 0xF0;
		*LPBYTE(rom + snes2pc(a++)) = 0x03;
		// .fail
		// CLC
		*LPBYTE(rom + snes2pc(a++)) = 0x18;
		// PLA
		*LPBYTE(rom + snes2pc(a++)) = 0x68;
		// RTL
		*LPBYTE(rom + snes2pc(a++)) = 0x6B;
		// .ok
		// CLC
		*LPBYTE(rom + snes2pc(a++)) = 0x18;
		// RTS
		*LPBYTE(rom + snes2pc(a++)) = 0x60;

		// custom
		a = 0x80D6AC;
		*LPBYTE(rom + snes2pc(a++)) = 0x20;
		*LPBYTE(rom + snes2pc(a++)) = 0x30;
		*LPBYTE(rom + snes2pc(a++)) = 0xFF;
		*LPBYTE(rom + snes2pc(a++)) = 0xEA;
		a = 0x80D6B3;
		*LPBYTE(rom + snes2pc(a++)) = 0x20;
		*LPBYTE(rom + snes2pc(a++)) = 0x37;
		*LPBYTE(rom + snes2pc(a++)) = 0xFF;
		*LPBYTE(rom + snes2pc(a++)) = 0xEA;

		// enemy
		a = 0x80D6FC;
		*LPBYTE(rom + snes2pc(a++)) = 0x20;
		*LPBYTE(rom + snes2pc(a++)) = 0x30;
		*LPBYTE(rom + snes2pc(a++)) = 0xFF;
		*LPBYTE(rom + snes2pc(a++)) = 0xEA;

		// candle
		a = 0x80D771;
		*LPBYTE(rom + snes2pc(a++)) = 0x20;
		*LPBYTE(rom + snes2pc(a++)) = 0x37;
		*LPBYTE(rom + snes2pc(a++)) = 0xFF;
		*LPBYTE(rom + snes2pc(a++)) = 0xEA;

#endif
	}

	expandedROM = true;

	return romAddr < romSize;
}

UINT16 SC4Core::GetChecksum() {
	UINT16 sum = 0;
	for (unsigned i = 0; i < romSize; i++) {
		// ignore $30 byte header
		if (i < 0x7FB0 || i >= (0x7FB0 + 0x30)) {
			sum += rom[i];
		}
	}

	return sum;
}

UINT32 SC4Core::GetCRC32() {
	UINT32 crc = 0xFFFFFFFF;
	for (unsigned i = 0; i < romSize; i++) {
		crc = ((crc >> 8) & 0xFFFFFF) ^ crc32Table[(crc ^ rom[i]) & 0xFF];
	}
	return ~crc;
}

DWORD SC4Core::GetFontPointer()
{
	return snes2pc(p_font[type]);
}

WORD SC4Core::ConvertWordBCDToDec(WORD x) {
	return ((x >> 12) & 0xF) * 1000 + ((x >> 8) & 0xF) * 100 + ((x >> 4) & 0xF) * 10 + (x & 0xF);
}
WORD SC4Core::ConvertWordDecToBCD(WORD x) {
	std::stringstream ss;
	ss << std::dec << "0x" << x;
	ss >> std::hex >> x;

	return x;
}

DWORD SC4Core::GetCheckPointBasePointer()
{
	return snes2pc(p_checkp[type] + SReadWord(p_checkp[type] + SReadWord(p_checkp[type] + level * 2) + 0 * 2));
}
DWORD SC4Core::GetCheckPointPointer()
{
	return snes2pc(p_checkp[type] + SReadWord(p_checkp[type] + SReadWord(p_checkp[type] + level*2) + point*2));
}

void SC4Core::LoadFont()
{
	BYTE textCache[0x2000];
	GFXRLE(rom, textCache, GetFontPointer() + 2, *LPWORD(rom + GetFontPointer()), type);

	for(int i=0; i<0x20; i++) // Decompress the 32 colors
		fontPalCache[i] = Get16Color(snes2pc(type == 0 ? 0x86c98c : type == 1 ? 0x8880AC : 0x848002) + i*2); // 0x2D3E0
	for (int i = 0; i < 0x100; i++) { // Decompress all 256 tiles in ram
		int tempChar = (type == 0) ? i : (type == 1) ? i : i;
		tile2bpp2raw(textCache + (i * 0x10), fontCache + (tempChar * 0x40) + 0x400);
	}
	return;
}
void SC4Core::SetLevel(WORD iLevel, WORD iPoint)
{
	level = iLevel;
	point = iPoint;

	tileCmpSize = 0;
	tileDec.clear();
	dynDecTable.clear();
	dynPalTable.clear();

	objLoadOffset = 0;
	tileLoadOffset = 0;
	palLoadOffset = 0;
}

//extern HWND hWID[10];
//DWORD ppppp;
void SC4Core::LoadLevel(bool refresh)
{
	numLevels = type == 0 ? 0x44 : type == 1 ? 0x6 : 0xA;
	numBlocks = type == 0 ? 0x100 : type == 1 ? 0x200 : 0x80;
	mapBase = type == 2 ? 0x7000 : 0x4000;
	mapBaseAdder = type == 0 ? 0x0000 : type == 1 ? 0x1000 : 0x0000;
	numTiles = type == 0 ? 0x200 : type == 1 ? 0x400 : 0x400;

	if (!refresh) {
		//MessageBox(hWID[0], "LoadEvents()", "Test", MB_ICONERROR);
		LoadEvents();
	}

	LoadVRAM();

	LoadTilesAndPalettes();

	//MessageBox(hWID[0], "LoadLayout()", "Test", MB_ICONERROR);
	LoadLayout();
	//MessageBox(hWID[0], "LoadLevelLayout()", "Test", MB_ICONERROR);
	LoadLevelLayout();
}
void SC4Core::LoadBackground()
{
	WORD pLevel = level*3;
	pLayout     = snes2pc(SReadDWord(p_blayout[type] + pLevel));
	pScenes     = snes2pc(SReadDWord(p_bscenes[type] + pLevel));
	pBlocks     = snes2pc(SReadDWord(p_bblocks[type] + pLevel));
	mapBase = type == 2 ? 0xB000 : 0xA000; // type == 0 ? 0xA000 : type == 1 ? 0x9000 : 0x0;
	mapBaseAdder = type == 0 ? 0x0000 : type == 1 ? 0x1000 : 0x0000;
	LoadVRAM();
	LoadTilesAndPalettes();
	LoadLayout();
	LoadLevelLayout();
}
void SC4Core::LoadTilesAndPalettes()
{
	// Load Palettes
	//for (int i = 0x0 / 2, j = 0; i< 0x0 / 2 + 0x140 / 2; i++, j+=2)
	//	palCache[i] = Get16Color(SNESCore::snes2pc(0x86CE50) + j);
	//for (int i = 0x2 / 2, j = 0; i< 0x2 / 2 + 0x1E / 2; i++, j += 2)
	//	palCache[i] = Get16Color(SNESCore::snes2pc(0x86C98E) + j);
	//for (int i = 0x1C0 / 2, j = 0; i< 0x1C0 / 2 + 0x40 / 2; i++, j += 2)
	//	palCache[i] = Get16Color(SNESCore::snes2pc(0x86DF44) + j);

	//for (int i = 0x2 / 2, j = 0; i< 0x2 / 2 + 0x1E / 2; i++, j += 2)
	//	palCache[i] = Get16Color(SNESCore::snes2pc(0x86C98E) + j);
	//for (int i = 0x100 / 2, j = 0; i< 0x100 / 2 + 0xE0 / 2; i++, j += 2)
	//	palCache[i] = Get16Color(SNESCore::snes2pc(0x86C8AC) + j);
	//for(int i=0x40 / 2, j = 0; i<0x40 / 2 + 0xC0 / 2; i++, j += 2)
	//	palCache[i] = Get16Color(SNESCore::snes2pc(0x86DF86) + j);
	//for (int i = 0x1E0 / 2, j = 0; i<0x1E0 / 2 + 0x20 / 2; i++, j += 2)
	//	palCache[i] = Get16Color(SNESCore::snes2pc(0x86E048) + j);
	//for (int i = 0xDC / 2, j = 0; i<0xDC / 2 + 0x4 / 2; i++, j += 2)
	//	palCache[i] = Get16Color(SNESCore::snes2pc(0x86E070) + j);

	ZeroMemory(palCache, sizeof(palCache));
	ZeroMemory(palCacheOffset, sizeof(palCacheOffset));

	std::vector<DWORD> addrList;
	if (type == 0x0) {
		addrList.push_back(0x818715);
		//addrList.push_back(0x818B75);
		addrList.push_back((0x81 << 16) | *LPWORD(rom + SNESCore::snes2pc(0x8693E7 + level * 2)));
		addrList.push_back((0x81 << 16) | *LPWORD(rom + SNESCore::snes2pc(0x86946F + level * 2)));
	}
	else if (type == 0x1) {
		//addrList.push_back(0x8587C3);
		//addrList.push_back(0x8587E1);
		//addrList.push_back(0x8587EA);
		//addrList.push_back(0x8587D8);
		//addrList.push_back(0x85854B);
		//addrList.push_back(0x858412);

		//addrList.push_back(0x858AC7);
		//addrList.push_back(0x85A815);
		//addrList.push_back(0x85A830);

		//addrList.push_back(0x85854B);

		//addrList.push_back(0x858412);
		addrList.push_back(0x858AC7);
		addrList.push_back((0x85 << 16) | *LPWORD(rom + SNESCore::snes2pc(0x858A21 + *LPBYTE(rom + SNESCore::snes2pc(0x8589CC + (level + 1))))));

		// FIXME: 
		//addrList.push_back((0x85 << 16) | *LPWORD(rom + SNESCore::snes2pc(0x85CD1B + 0 * level * 2)));
		//addrList.push_back((0x85 << 16) | *LPWORD(rom + SNESCore::snes2pc(0x85CD21 + level * 2)));
	}
	else if (type == 0x2) {
		addrList.push_back(0x818B6E);
		addrList.push_back(0x818B9D);
		addrList.push_back(0x818C19);
		addrList.push_back(0x818BF2);
		addrList.push_back((0x81 << 16) | *LPWORD(rom + SNESCore::snes2pc(0x81A21A + level * 2)));
	}

	addrList.insert(addrList.end(), dynPalTable.begin(), dynPalTable.end());

	for (auto addr : addrList) {
		if (*(rom + SNESCore::snes2pc(addr))) {
			BYTE t = *(rom + SNESCore::snes2pc(addr));
			addr += t == 4 ? 0 : 1;

			if (t != 1) continue;

			// weird extra 0.  seems like a countdown timer for loading palette?
			addr += 2;
			while (WORD srcOffset = *LPWORD(rom + SNESCore::snes2pc(addr))) {
				addr += 2;
				WORD dstOffset = *LPWORD(rom + SNESCore::snes2pc(addr));
				addr += 2;

				unsigned palIndex = (dstOffset - 0x2200) / 2;
				DWORD srcAddr = ((type == 0x0 ? 0x86 : type == 0x1 ? 0x88 : 0x84) << 16) | srcOffset;
				WORD size = *LPWORD(rom + SNESCore::snes2pc(srcAddr)) + 1;
				srcAddr += 2;
				for (int i = palIndex, j = 0; i < palIndex + size / 2; i++, j += 2) {
					palCache[i] = Get16Color(SNESCore::snes2pc(srcAddr) + j);
					palCacheOffset[i] = SNESCore::snes2pc(srcAddr) + j;
				}

			}
		}
	}

	//pPalBase = snes2pc(p_palett[type]);
	//DWORD configPointer = snes2pc(SReadWord(p_palett[type] + level*2 + 0x60) | 0x860000);
	//BYTE colorsToLoad = rom[configPointer++];
	//if (type==2)
	//	pPalette = snes2pc(ReadWord(configPointer++) | 0x8C0000);
	//else
	//	pPalette = snes2pc(ReadWord(configPointer++) | 0x850000);

	//for(int i=0; i<16; i++)
	//	palCache[i] = Get16Color(0x10000 + i*2);
	////for(int i=0; i<(colorsToLoad>>4); i++)
	////	palettesOffset[i] = (DWORD)pPalette + i*0x20;

	//// load sprite palettes
	//for (int i = 0; i<NUM_SPRITE_PALETTES * 16; i++)
	//	palSpriteCache[i] = Get16Color(/*0x2b900*/ 0x2a000 + i * 2);

	//memcpy(vram, vrambase, 0x200);

	////LoadPaletteDynamic();
	//LoadGFXs();
	//LoadTiles();

	ZeroMemory(vramCache, sizeof(vramCache));

	for (int i = 0; i < (0x20000 >> 5); i++)
		tile4bpp2raw(vram + (i << 5), vramCache + (i << 6));

	if (isMode7()) {
		// mode7 tiles
		for (int i = 0; i < (0x6000 >> 6); i++)
			tileMode72raw(vram + (i << 6), vramCache + (i << 6));
	}
}

void SC4Core::LoadGFXs()
{
	//pGfx = snes2pc(p_gfxpos[type]);
	//pGfxObj = p_gfxobj[type] ? snes2pc(p_gfxobj[type]) : 0x0;
	//pGfxPal = p_gfxobj[type] ? snes2pc(p_gfxpal[type]) : 0x0;
	//pSpriteAssembly = p_spriteAssembly[type] ? snes2pc(p_spriteAssembly[type]) : 0x0;
	//pSpriteOffset[0] = p_objOffset[type] ? snes2pc(p_objOffset[type]) : 0x0;
	//pSpriteOffset[1] = p_objOffset[type] ? snes2pc(p_objOffset[type]) : 0x0;
	//pSpriteOffset[3] = p_spriteOffset[type] ? snes2pc(p_spriteOffset[type]) : 0x0;

	//DWORD pConfigGfx = snes2pc(SReadWord(p_gfxcfg[type] + level*2 + 4) | 0x86 << 16);
	//BYTE gfxID = rom[pConfigGfx];
	//tileCmpSize = ReadWord(pConfigGfx+1);
	//tileCmpDest = (ReadWord(pConfigGfx+3)<<1) - 0x2000;
	//tileCmpPos = snes2pc(SReadDWord(p_gfxpos[type] + gfxID * 5 + 2));
	//tileCmpRealSize = GFXRLE(rom, vram+tileCmpDest, tileCmpPos, tileCmpSize, type);

	//ZeroMemory(spriteCache, NUM_SPRITE_TILES * 64);
	//for (unsigned i = 0; i < (sizeof(tram) >> 5); ++i) {
	//	tile4bpp2raw(tram + (i << 5), spriteCache + (i << 6));
	//}



}
void SC4Core::LoadTiles()
{
//	BYTE tileSelect = 0; // *checkpointInfoTable[point].tileLoad + tileLoadOffset;
//
//	unsigned tileOffset = (type == 0) ? 0x321D5
//		: (type == 1) ? 0x31D6A
//		: 0x32085; /*0x1532D4;*/
//
//// find bounds of dynamic tiles
//	tileDecStart = 0x400;
//	tileDecEnd = 0;
//	numDecs = 0;
//	for (unsigned i = 0; i < 0x40; ++i) {
//		int tbaseIndex = ReadWord(tileOffset + level * 2) + i * 2;
//		int tmainIndex = ReadWord(tileOffset + tbaseIndex);
//
//		numDecs++;
//
//		auto size = ReadWord(tileOffset + tmainIndex);
//		if (size == NULL) {
//			if (i == 0) {
//				continue;
//			}
//			else {
//				break;
//			}
//		}
//		auto pos = (ReadWord(tileOffset + tmainIndex + 2) << 1) - 0x2000;
//
//		unsigned start = pos / 0x20;
//		unsigned end = (size + pos) / 0x20;
//		if (start < tileDecStart) {
//			tileDecStart = start;
//		}
//		if (end > tileDecEnd) {
//			tileDecEnd = end;
//		}
//	}
//
//	// Is it right to start from 1 all the time?  Or do we need to check 0, too?
//	for (unsigned i = 0; i <= tileSelect; ++i) {
//		int baseIndex = ReadWord(tileOffset + level * 2) + i * 2;
//		int mainIndex = ReadWord(tileOffset + baseIndex);
//
//		tileDecSize = ReadWord(tileOffset + mainIndex);
//		if (tileDecSize == NULL) continue;
//		tileDecDest = (ReadWord(tileOffset + mainIndex + 2) << 1) - 0x2000;
//		auto addr = ReadDWord(tileOffset + mainIndex + 4) & 0xFFFFFF;
//		tileDecPos = snes2pc(addr);
//
//		if (tileDecDest + tileDecSize > (DWORD)0x10000)
//		{
//			MessageBox(NULL, "VRAM overflow.", "Error", MB_ICONERROR);
//		}
//		// skip the load if it's to the RAM address
//		// This happens in X3 when zero first appears.  It's not obvious how it handles these tiles
//		// Pointer = 0x86A134/86A136
//		if (addr != 0x7F0000) {
//			memcpy(vram + tileDecDest, rom + tileDecPos, tileDecSize);
//		}
//	}
}
void SC4Core::SortTiles() {
	//enum eSort {
	//	SORT_NONE,
	//	SORT_MIN,
	//	SORT_MAX,
	//	SORT_MEDIAN,
	//	SORT_MEAN,
	//	SORT_MODE,
	//	SORT_TOTAL
	//};

	//byte tvram[0x8000];
	//std::vector<unsigned> sortTypes;

	//if (!sortOk) {
	//	sortTypes.push_back(SORT_NONE);
	//}
	//else {
	//	sortTypes.push_back(SORT_NONE);
	//	//sortTypes.push_back(SORT_MIN);
	//	sortTypes.push_back(SORT_MAX);
	//	//sortTypes.push_back(SORT_MEDIAN);
	//	//sortTypes.push_back(SORT_MEAN);
	//	sortTypes.push_back(SORT_MODE);
	//}

	//WORD newSize = 0;  //GFXRLECmp(vram + 0x200, tvram, tileCmpSize, type);
	//WORD tileRemap[0x400];

	//for (auto sortType : sortTypes) {
	//	//ZeroMemory(srcram, 0x8000);
	//	//memcpy(srcram, vram + 0x200, tileCmpSize);

	//	sTileInfo tileInfo[0x400];

	//	for (unsigned i = 0; i < 0x400; ++i) {
	//		tileInfo[i].num = i;
	//		tileInfo[i].value = sortType == SORT_MIN ? 0xFF : 0x0;

	//		for (unsigned p = 0; p < 32; ++p) {
	//			BYTE value = vram[32 * i + p];
	//			switch (sortType) {
	//			case SORT_NONE:
	//				// do nothing
	//				break;
	//			case SORT_MIN:
	//				tileInfo[i].value = min(tileInfo[i].value, value);
	//				break;
	//			case SORT_MAX:
	//				tileInfo[i].value = max(tileInfo[i].value, value);
	//				break;
	//			case SORT_MEDIAN:
	//				tileInfo[i].count[value]++;
	//				if (p == 31) {
	//					unsigned num = 0;
	//					for (auto &count : tileInfo[i].count) {
	//						num += count.second;
	//						if (num >= 16) {
	//							tileInfo[i].value = count.first;
	//							break;
	//						}
	//					}
	//				}
	//				break;
	//			case SORT_MEAN:
	//				tileInfo[i].value += value;
	//				if (p == 31) {
	//					tileInfo[i].value /= 32;
	//				}
	//				break;
	//			case SORT_MODE:
	//				tileInfo[i].count[value]++;
	//				if (p == 31) {
	//					unsigned num = 0;
	//					for (auto &count : tileInfo[i].count) {
	//						if (count.second > num) {
	//							tileInfo[i].value = count.first;
	//							num = count.second;
	//						}
	//					}
	//				}
	//				break;
	//			default:
	//				break;
	//			}
	//		}
	//	}

	//	// sort based on type.  skip the first 16 tiles since they go uncompressed.  Same goes for the last set of tiles.
	//	unsigned start = 0x200 / 0x20;
	//	unsigned end = 0x400 - (0x8000 - 0x200 - tileCmpSize) / 0x20;

	//	if (tileDecStart >= end || tileDecEnd <= start) {
	//		// sort full thing
	//		std::sort(tileInfo + start, tileInfo + end, TileSort);
	//	}
	//	else if (tileDecStart <= start && tileDecEnd >= end) {
	//		// can't sort anything
	//	}
	//	else {
	//		if (start < tileDecStart) {
	//			std::sort(tileInfo + start, tileInfo + tileDecStart, TileSort);
	//		}
	//		if (tileDecEnd < end) {
	//			std::sort(tileInfo + tileDecEnd, tileInfo + end, TileSort);
	//		}
	//	}

	//	// move the tiles in memory 
	//	byte sortram[0x8000];
	//	for (unsigned i = 0; i < 0x400; ++i) {
	//		memcpy(sortram + i * 32, vram + 32 * tileInfo[i].num, 32);
	//	}

	//	// try compressing
	//	byte cmpram[0x8000];
	//	auto tempSize = GFXRLECmp(sortram + 0x200, cmpram, tileCmpSize, type);

	//	if (tempSize < newSize || sortType == SORT_NONE) {
	//		newSize = tempSize;
	//		memcpy(tvram, sortram, sizeof(tvram));

	//		for (unsigned i = 0; i < 0x400; ++i) {
	//			tileRemap[tileInfo[i].num] = i;
	//		}
	//	}
	//}

	//// copy the best sorted data
	//memcpy(vram, tvram, sizeof(tvram));

	//if (sortOk) {
	//	// fix blocks
	//	for (unsigned i = 0; i < numMaps; ++i) {
	//		for (unsigned j = 0; j < 4; ++j) {
	//			LPWORD tileOffset = LPWORD(rom + pMaps + (i << 3) + j * 2);
	//			unsigned tile = *tileOffset & 0x3FF;
	//			*tileOffset &= ~0x3FF;
	//			*tileOffset |= tileRemap[tile] & 0x3FF;
	//		}
	//	}
	//}

}
void SC4Core::LoadEvents() {
	eventTable.clear();
	totalEvents = 0;

	if (!p_events[type]) return;

	if (type == 0) {
		//if (p_borders[type]) pBorders = SReadWord(p_borders[type] + level * 2) | ((p_borders[type] >> 16) << 16);
		if (p_locks[type]) pLocks = snes2pc(p_locks[type]);
		//if (p_capsulepos[type]) pCapsulePos = snes2pc(p_capsulepos[type]);

		DWORD pEvents = snes2pc(SReadWord(p_events[type] + level * 2) | (eventBank << 16));
		LPBYTE pevent = rom + pEvents;
		auto oldpevent = pevent;

		// AABBCC
		// A = 14b = xpos | 2b = event tpe
		// B = 14b = ypos | 2b = ???
		// C = 8b = spawn test index (indexes into 1500 in RAM)
		while (*LPDWORD(pevent) != 0xFFFFFFFF) {
			EventInfo event;

			event.type = *LPWORD(pevent + 0) & 0x3;
			event.xpos = *LPWORD(pevent + 0) & 0xFFC;
			event.unknown = *LPWORD(pevent + 2) & 0x3;
			event.ypos = *LPWORD(pevent + 2) & 0xFFC;

			if (event.type == 0 || event.type == 2) {
				// enemy
				// custom
				event.eventId = *LPWORD(pevent + 4) & 0xFF;
				event.eventSubId = (*LPWORD(pevent + 4) >> 8) & 0xFF;
				event.match = (*(pevent + 3) >> 4 << 4) | (*(pevent + 1) >> 4);
			}
			else if (event.type == 1) {
				// candle
				event.eventId = (*LPWORD(pevent + 4) >> 8) & 0xFF;
				event.eventSubId = (*(pevent + 3) >> 4 << 4) | (*(pevent + 1) >> 4);
				event.match = *LPWORD(pevent + 4) & 0xFF;
			}

			//event.match = (*(pevent + 3) >> 4 << 4) | (*(pevent + 1) >> 4);
			//event.type = *LPWORD(pevent + 0) & 0x3;
			//event.xpos = *LPWORD(pevent + 0) & 0xFFC;
			//event.ypos = *LPWORD(pevent + 2) & 0xFFC;
			//event.unknown = *LPWORD(pevent + 2) & 0x3;
			//event.eventId = *LPWORD(pevent + 4) & 0xFF;
			//event.eventSubId = (*LPWORD(pevent + 4) >> 8) & 0xFF;

			eventTable.emplace_back(event);

			// get the next event
			pevent += 6;
			totalEvents++;
		}

		// include termination
		eventSize = pevent - oldpevent + 4;

		// count total events
		while (*LPDWORD(pevent + 6) == 0xFFFFFFFF) {
			totalEvents++;
			pevent += 6;
		}
	}
	else if (type == 1) {

		if (level == 1 || level == 3 || level == 4) return;

		DWORD pEvents = snes2pc(SReadWord(p_events[type] + level * 2) | (eventBank << 16));
		LPBYTE pevent = rom + pEvents;
		DWORD pEventsLimit = snes2pc(*LPWORD(pevent) | (eventBank << 16));
		LPBYTE plimitevent = rom + pEventsLimit;

		auto oldpevent = pevent;

		// ABCDEEFFGG
		// A = 8b = mask
		// B = 8b = id
		// C = 8b = max slot >> 4
		// D = 8b = min slot >> 4
		// EE = xpos
		// FF = ypos
		// GG = subid

		unsigned sceneNum = 0;
		while (pevent != plimitevent) {
			DWORD a = snes2pc(*LPWORD(pevent) | (eventBank << 16));
			LPBYTE pe = rom + a;

			while (*pe != 0xFF) {
				EventInfo event;

				event.match = *LPBYTE(pe + 0);
				event.eventId = *LPBYTE(pe + 1);
				event.unknown = *LPWORD(pe + 2);
				event.xpos = *LPWORD(pe + 4) + sceneNum * 0x100;
				event.ypos = *LPWORD(pe + 6);
				event.eventSubId = *LPWORD(pe + 8);
				pe += 0xA;

				eventTable.emplace_back(event);

				// get the next event
				totalEvents++;
			}
			pevent += 2;

			eventSize = pe - oldpevent;
			sceneNum++;
		}
	}
	else if (type == 2) {
		DWORD pEvents = snes2pc(SReadWord(p_events[type] + level * 2) | (eventBank << 16));
		LPBYTE pevent = rom + pEvents;
		auto oldpevent = pevent;

		// AABBCC
		// A = 14b = xpos | 2b = event tpe
		// B = 14b = ypos | 2b = ???
		// C = 8b = spawn test index (indexes into 1500 in RAM)
		while (*LPWORD(pevent) != 0xFFFF) {
			EventInfo event;

			//event.match = 0;
			event.type = *LPWORD(pevent + 2) & 0x3;
			event.xpos = ((*LPWORD(pevent + 0) & 0xFFC0) >> 3); // -0x28;

			if (event.type == 0 || event.type == 1) {
				event.ypos = (*LPWORD(pevent + 2) & 0xFC) + ((*LPWORD(pevent + 0) & 0x20) << 3); // ypos
				event.eventId = *LPWORD(pevent + 3) & 0x7F; // id
				event.eventSubId = *LPWORD(pevent + 0) & 0x1F; // slot
				event.unknown = *LPWORD(pevent + 2) & 0x8000;
			}
			else if (event.type == 2) {
				event.ypos = *LPWORD(pevent + 2) & 0x00FC;
				event.eventId = *LPWORD(pevent + 3) & 0x007F;
				event.eventSubId = *LPWORD(pevent + 0) & 0x003F;
				event.unknown = *LPWORD(pevent + 2) & 0x8000;
			}
			else if (event.type == 3) {
				event.ypos = 0;
				event.eventId = *LPBYTE(pevent + 0) & 0x3F; // id
				event.eventSubId = *LPBYTE(pevent + 3) & 0xFF;
				event.unknown = *LPBYTE(pevent + 2) & 0xFC;
			}

			////event.spawnIndex = ((*LPWORD(pevent + 2) >> 8) & 0xF0) | ((*LPWORD(pevent + 0) >> 12) & 0x0F);
			//event.eventId = *LPWORD(pevent + 3) & 0xFF;
			////event.eventSubId = (*LPWORD(pevent + 4) >> 8) & 0xFF;
			//event.unknown = *LPWORD(pevent + 0) & 0x3F;

			eventTable.emplace_back(event);

			// get the next event
			pevent += 4;
			totalEvents++;
		}

		// include termination
		eventSize = pevent - oldpevent + 2;

		// count total events
		while (*LPWORD(pevent + 2) == 0xFFFF) {
			totalEvents++;
			pevent += 2;
		}

	}
}

void SC4Core::SortEvents() {
	if (!p_events[type]) return;

	struct sortLess {
	public:
		sortLess(bool sortY) : SortY(sortY) { }

		bool operator()(const EventInfo &a, const EventInfo &b) const {
			return (!SortY) ? (a.xpos < b.xpos) : (a.ypos < b.ypos);
		}
	private:
		bool SortY = false;
	};

	eventTable.sort(sortLess(levelWidth < levelHeight));
}

void SC4Core::SlotEvents() {
	if (!p_events[type]) return;

	unsigned currentSlot = 0;

	for (auto &e : eventTable) {
		if (type == 0) {
		}
		else if (type == 1) {

		}
		else if (type == 2) {
			if (e.type == 0) {
				//if (e.eventId == 0x1C) {
				//	currentSlot = 0;
				//	e.eventSubId = currentSlot;
				//	currentSlot += 0x8;
				//}
				//else {
					// single enemy
					e.eventSubId = currentSlot;
					currentSlot++;
				//}
			}
			else if (e.type == 1) {
				// reset the slot with some space for the special type
				currentSlot = 8;
			}
			else if (e.type == 2) {
				// multiple spawn
				unsigned slotsRequired = max(1, *LPBYTE(rom + snes2pc(0x81AF8A + 4 * (e.eventId - 1) + 3)));
				//if (currentSlot + slotsRequired > 0x1A)
				//	currentSlot = 0;
				e.eventSubId = currentSlot;
				currentSlot += slotsRequired;
			}

			currentSlot %= 0x1A;
		}
	}
}

bool SC4Core::isMode7(int levelNum) {
	return GetLevelType(levelNum) == 0x5;
}

void SC4Core::LoadVRAM() {
	if (level < 0x42) ZeroMemory(vram, 0x20000);
	if (level < 0x42) ZeroMemory(ram, 0x20000);

	unsigned size = 0;

	std::vector<DWORD> addrList;

	// levels
	if (type == 0x0) {
		DWORD addr;
		// BG0/1 tiles
		addr = 0x86 << 16 | *LPWORD(rom + SNESCore::snes2pc(0x86889C) + (level * 2));
		addrList.push_back(addr);
		// level sprites
		addr = 0x86 << 16 | *LPWORD(rom + SNESCore::snes2pc(0x868B45) + (level * 2));
		addrList.push_back(addr);
		// block data
		addr = 0x86 << 16 | *LPWORD(rom + SNESCore::snes2pc(0x868000) + (level * 2));
		addrList.push_back(addr);
		// scene data
		addr = 0x86 << 16 | *LPWORD(rom + SNESCore::snes2pc(0x86831e) + (level * 2));
		addrList.push_back(addr);

		addrList.push_back(region == 0 ? 0x81B3E2 : 0x81B3B6); // default whip, powerup, candle, orb, etc
	}
	else if (type == 0x1) {
		DWORD addr;
		//// C3
		//// ???
		//addr = 0x85 << 16 | *LPWORD(rom + SNESCore::snes2pc(0x8587F1) + ((level + 1) * 2));
		//addrList.push_back(addr);
		// wram - player tiles
		addr = 0x85 << 16 | *LPWORD(rom + SNESCore::snes2pc(0x8588E1) + ((level + 1) * 2));
		addrList.push_back(addr);
		// wram - level
		addr = 0x85 << 16 | *LPWORD(rom + SNESCore::snes2pc(0x8588ED) + ((level + 1) * 2));
		addrList.push_back(addr);

		addrList.push_back(0x858ADD);
		addrList.push_back(0x858AD4);
		// tiles
		if (level == 1) {

		}
		else if (level == 4) {
			addrList.push_back(0x85CF33);
			addrList.push_back(0x85D01F);
		}
		else {
			auto index = level;
			switch (level) {
			case 0: index = 0; break;
			case 1: index = 0; break;
			case 2: index = 2; break;
			case 3: index = 4; break;
			case 4: index = 0; break;
			case 5: index = 6; break;
			}
			addr = 0x85 << 16 | *LPWORD(rom + SNESCore::snes2pc(0x858A0D) + index);
			addrList.push_back(addr);
		}
	}
	else if (type == 0x2) {
		DWORD addr;

		// ship and other tiles
		//addrList.push_back(0x8185B3);
		addrList.push_back(0x81846D);
		addrList.push_back(0x81840D);

		// dynamic tiles
		addrList.push_back(0x8183B2);

		// standard enemies
		addrList.push_back(0x8183E6);

		// X (0)
		addr = 0x81 << 16 | *LPWORD(rom + SNESCore::snes2pc(0x81AADA) + (level * 2));
		addrList.push_back(addr);

		// not loaded on level 0
		//addr = 0x81 << 16 | *LPWORD(rom + SNESCore::snes2pc(0x81AEB8) + (level * 2));
		//addrList.push_back(addr);

		//addr = 0x81 << 16 | *LPWORD(rom + SNESCore::snes2pc(0x81A22A) + (level * 2));
		//addrList.push_back(addr);

		// X (2)
		addr = 0x81 << 16 | *LPWORD(rom + SNESCore::snes2pc(0x81A24C) + ((level+1) * 2));
		addrList.push_back(addr);

		// X (2)
		addr = 0x81 << 16 | *LPWORD(rom + SNESCore::snes2pc(0x81A254) + ((level+1) * 2));
		addrList.push_back(addr);

		// X (0)
		addr = 0x81 << 16 | *LPWORD(rom + SNESCore::snes2pc(0x81A25E) + (level * 2));
		addrList.push_back(addr);

		//addr = 0x81 << 16 | *LPWORD(rom + SNESCore::snes2pc(0x818937) + (level * 2));
		//addrList.push_back(addr);
		// G3

		// Ship and other sprites in VRAM

	}

	// add dynamic tiles
	addrList.insert(addrList.end(), dynDecTable.begin(), dynDecTable.end());

	//addrList.push_back(region == 0 ? 0x81B411 : 0x81B3E5); // upgrade whip?
	//if (index != 0x5) {
	//	addrList.push_back(region == 0 ? 0x81F9DC : 0x81F9D2); // HUD
	//}
	//addrList.push_back(region == 0 ? 0x81B4A3 : 0x81B477);

	//// load the boss if it exists
	//bool bossFound = false;
	//DWORD bossAddr = 0x0;
	//for (auto &e: eventTable) {
	//	if ((e.type == 0x0 || e.type == 0x2) && e.eventId == 0x2A) {
	//		unsigned i = e.eventSubId & ~0x80;
	//		DWORD addr;
	//		addr = 0x81 << 16 | *LPWORD(rom + SNESCore::snes2pc(region == 0 ? 0x81B9D6 : 0x81B9A0) + (i * 2));
	//		addrList.push_back(addr);

	//		bossFound = true;
	//		bossAddr = addr;
	//	}
	//}

	for (auto a : addrList) {
		DWORD addr = a;
		if ((addr & 0xFFFF) != 0xFFFF) {
			LPBYTE gfxPtr = (rom + SNESCore::snes2pc(addr));

			if (*LPWORD(gfxPtr) != 0xFFFF) {
				bool oneAddr = *gfxPtr == 2 || (type == 2 && a == 0x81840D);
				unsigned addrBytes = (*gfxPtr == 0 || *gfxPtr == 2 || *gfxPtr == 3) ? 2 : 3;
				//if (*gfxPtr == 7) return;
				gfxPtr += 2;

				while (*LPWORD(gfxPtr) != 0xFFFF) {
					DWORD dstAddr = *LPDWORD(gfxPtr) & 0xFFFFFF;
					DWORD vramOffset = dstAddr & 0xFFFF;
					gfxPtr += addrBytes;
					addr = *LPDWORD(gfxPtr);
					gfxPtr += 3;

					WORD size = *LPWORD(rom + SNESCore::snes2pc(addr));
					addr += 2;
					//if (vramOffset == 0x3000) { continue; }

					if (addrBytes == 0x2) { // || (index == 0x5 && (vramOffset == 0x0000 || vramOffset == 0x1000 || vramOffset == 0x2000 || vramOffset == 0x3000 || ((vramOffset & 0xF000) == 0x6000)))) {
						//if (bossFound && bossAddr == a) vramOffset = 0x8000;
						auto origSize = GFXRLE(rom, vram + (vramOffset * 2), SNESCore::snes2pc(addr), size, type);
						origSize = origSize;
					}

					if (addrBytes == 3) {
						auto origSize = GFXRLE(rom, ram + ((dstAddr - 0x7E0000) * (addrBytes == 0 ? 2 : 1)), SNESCore::snes2pc(addr), size, type);
						origSize = origSize;
					}

					if (oneAddr) { break; }
				}
			}
		}
	}

	for (auto &t : tileDec) {
		auto tileDecSrc = std::get<0>(t);
		auto tileDecDst = std::get<1>(t);
		auto tileDecSize = std::get<2>(t);
		memcpy(vram + tileDecDst * 2, ram + (tileDecSrc - 0x7E0000), tileDecSize);
	}

	//std::ofstream is;
	//is.open("vram.out", std::ios_base::out | std::ios_base::binary);
	//for (unsigned i = 0; i < sizeof(vram); ++i) {
	//	is << vram[i];
	//}
	//is.close();

	//ZeroMemory(spriteCache, NUM_SPRITE_TILES * 64);
	//for (unsigned i = 0; i < (sizeof(tram) >> 5); ++i) {
	//	tile4bpp2raw(tram + (i << 5), spriteCache + (i << 6));
	//}
}
unsigned SC4Core::GetEventSize() {
	if (!p_events[type]) return 0;

	unsigned size = eventTable.size() * 6 + 4;

	return size;
}
unsigned SC4Core::GetOrigEventSize() {
	static WORD origEventSize[][0x44] = {
		{ 0x70, 0x124, 0xca, 0x8e, 0x8e, 0xdc, 0xbe, 0x46, 0x124, 0x1c0, 0xb8, 0xd6, 0x2c2, 0x1b4, 0x9a, 0xb8,
		0xac, 0xc4, 0xdc, 0x100, 0xa0, 0x16, 0x2ec, 0x124, 0x112, 0xdc, 0x124, 0xb8, 0x178, 0x106, 0x9a, 0x88, 0xa0,
		0xe8, 0x34, 0x14e, 0xee, 0xbe, 0x130, 0x9a, 0xa0, 0x76, 0x16c, 0x1f6, 0x100, 0x1f6, 0xdc, 0xe2, 0x1e4, 0x94,
		0xa0, 0xbe, 0x14e, 0x5e, 0x7c, 0x46, 0xf4, 0xbe, 0x1ba, 0x58, 0xa0, 0xac, 0x1d8, 0x4c, 0x2e, 0x40, 0x16, 0x40 },
		{ 0x0 },
		{ 0x0 } 
	};
	return origEventSize[type][level];
}
void SC4Core::LoadPaletteDynamic()
{
	unsigned paletteOffset = (type == 0) ? 0x32260
						   : (type == 1) ? 0x31DD1
						   : 0x32172;

	WORD iLevel = level & 0xFF;
	BYTE palSelect = 0; // *checkpointInfoTable[point].palLoad + palLoadOffset;
	for (unsigned i = 0; i <= palSelect; ++i) {
		int baseIndex = ReadWord(paletteOffset + iLevel * 2) + i * 2;
		int mainIndex = ReadWord(paletteOffset + baseIndex);
		int writeTo = 0;
		int colorPointer = 0;

		while (true)
		{
			colorPointer = ReadWord(paletteOffset + mainIndex);
			if (colorPointer == 0xFFFF)
				break;
			writeTo = (ReadWord(paletteOffset + 0x2 + mainIndex) & 0xFF);
			if (writeTo > 0x7F)
			{
				MessageBox(NULL, "Palette overflow.", "Error", MB_ICONERROR);
				return;
			}

			palettesOffset[writeTo >> 4] = snes2pc(colorPointer | (type == 2 ? 0x8C0000 : 0x850000));
			for (int i = 0; i < 0x10; i++)
			{
				palCache[writeTo + i] = Convert16Color(ReadWord(snes2pc((type == 2 ? 0x8C0000 : 0x850000) | colorPointer + i * 2)));
			}
			mainIndex += 3;
		}
	}
}

WORD SC4Core::GetLevelType(int levelNum) {
	switch (type) {
	case 0: return *LPWORD(rom + SNESCore::snes2pc(0x868296) + 2 * (levelNum == -1 ? level : levelNum));
	case 1: return 0;
	case 2: return 0;
	}
}

void SC4Core::SetLevelType(WORD type, int levelNum) {
	*LPWORD(rom + SNESCore::snes2pc(0x868296) + 2 * (levelNum == -1 ? level : levelNum)) = type;
}

WORD SC4Core::GetTileVramByteAddr(int levelNum) {
	switch (type) {
	case 0: return (GetLevelType(levelNum) == 0x13) ? 0x2000 : (isMode7(levelNum)) ? 0x0000 : 0x4000;
	case 1: return 0x0000;
	case 2: return 0x8000;
	}
}

RECT SC4Core::GetBoundingBox(const EventInfo &event) {
	RECT rect;

	//if (event.type == 0x2 && event.eventId == 0 && pLocks) {
	//	if (pLocks) {
	//		// look up the subid to get the camera lock
	//		auto borderOffset = *LPWORD(rom + SNESCore::snes2pc(pBorders) + 2 * event.eventSubId);
	//		LPBYTE base = rom + SNESCore::snes2pc(borderOffset | ((pBorders >> 16) << 16));

	//		rect.right = *LPWORD(base);
	//		base += 2;
	//		rect.left = *LPWORD(base);
	//		base += 2;
	//		rect.bottom = *LPWORD(base);
	//		base += 2;
	//		rect.top = *LPWORD(base);
	//		base += 2;
	//	}
	//}
	//else if (event.type == 0x2 && (event.eventId >= 0x15 && event.eventId <= 0x18)) {
	//	// draw green line
	//	rect.left = event.xpos + ((event.eventId & 0x8) ? -128 : -5);
	//	rect.top = event.ypos + (!(event.eventId & 0x8) ? -112 : -5);
	//	rect.bottom = event.ypos + (!(event.eventId & 0x8) ? 112 : 5);
	//	rect.right = event.xpos + ((event.eventId & 0x8) ? 128 : 5);
	//}
	//else if (pSpriteAssembly && pSpriteOffset[event.type]
	//	&& (event.type != 1 || (type == 0 && event.eventId == 0x21))
	//	&& (event.type != 0 || (event.eventId == 0xB && event.eventSubId == 0x4))
	//	&& !(type == 1 && event.eventId == 0x2) // something near the arm doesn't have graphics
	//	) {
	//	// draw associated object sprite

	//	unsigned assemblyNum = *(rom + pSpriteOffset[event.type] + ((event.eventId - 1) * (type == 2 ? 5 : 2)));

	//	// workarounds for some custom types
	//	if (type == 0 && event.type == 1 && event.eventId == 0x21) {
	//		// X1 highway trucks/cars
	//		assemblyNum = ((event.eventSubId & 0x30) >> 4) + 0x3A;
	//	}
	//	else if (event.type == 0 && event.eventId == 0xB && event.eventSubId == 0x4) {
	//		// X1 heart tank
	//		assemblyNum = 0x38;
	//	}

	//	unsigned mapAddr = *LPDWORD(rom + SNESCore::snes2pc(*LPDWORD(rom + pSpriteAssembly + assemblyNum * 3)) + 0);

	//	LPBYTE baseMap = rom + SNESCore::snes2pc(mapAddr);
	//	BYTE tileCnt = *baseMap++;

	//	RECT boundingBox;
	//	boundingBox.left = LONG_MAX;
	//	boundingBox.right = 0;
	//	boundingBox.bottom = 0;
	//	boundingBox.top = LONG_MAX;

	//	for (unsigned i = 0; i < tileCnt; ++i) {
	//		auto map = baseMap + (tileCnt - i - 1) * 4;
	//		char xpos = 0;
	//		char ypos = 0;
	//		unsigned tile = 0;
	//		unsigned info = 0;
	//		unsigned attr = 0;

	//		if (type == 0) {
	//			xpos = *map++;
	//			ypos = *map++;
	//			tile = *map++;
	//			info = *map++;
	//		}
	//		else {
	//			xpos = map[1];
	//			ypos = map[2];
	//			tile = map[3];
	//			info = map[0];

	//			map += 4;
	//		}

	//		if (type == 2) {
	//			// temporary fix for the boss sprites that have assembly information that is off by 0x20 or 0x40.
	//			tile -= (assemblyNum == 0x61 || assemblyNum == 0x92) ? 0x20 :
	//				(assemblyNum == 0x68 || assemblyNum == 0x79 || assemblyNum == 0xae) ? 0x40 :
	//				0x0;
	//			tile &= 0xFF;
	//		}

	//		bool largeSprite = (info & 0x20) ? true : false;

	//		for (unsigned j = 0; j < (largeSprite ? 4 : 1); ++j) {
	//			int xposOffset = (j % 2) * 8;
	//			int yposOffset = (j / 2) * 8;

	//			int screenX = event.xpos + xpos + xposOffset;
	//			int screenY = event.ypos + ypos + yposOffset;

	//			if (screenX < boundingBox.left) boundingBox.left = screenX;
	//			if (boundingBox.right < screenX + 8) boundingBox.right = screenX + 8;
	//			if (screenY < boundingBox.top) boundingBox.top = screenY;
	//			if (boundingBox.bottom < screenY + 8) boundingBox.bottom = screenY + 8;
	//		}
	//	}

	//	rect = boundingBox;
	//}
	//else {
	if (type == 0) {
		rect.left = event.xpos - 8;
		rect.top = event.ypos - 8;
		rect.bottom = event.ypos + 8;
		rect.right = event.xpos + 8;
	}
	else if (type == 1) {
		rect.left = event.xpos - 8;
		rect.top = event.ypos - 8;
		rect.bottom = event.ypos + 8;
		rect.right = event.xpos + 8;
	}
	else if (type == 2) {
		rect.left = event.xpos - 0x18;
		rect.top = event.ypos - 0x18;
		rect.bottom = event.ypos + 0x18;
		rect.right = event.xpos + 0x18;
	}
	//}

	return rect;
}
void SC4Core::LoadLevelLayout()
{
	bool step = false;
	unsigned short index = 0;
	WORD writeIndex = 0;

	ZeroMemory(mapRam, sizeof(mapRam));
	ZeroMemory(mapping, sizeof(mapping));

	// Load other things O.o
	//writeIndex = SReadWord(0x868D20 + step*2);
	//for(int s=0; s<sceneUsed; s++)l
	for (int y = 0; y < levelHeight; y++) {
		for (int x = 0; x < levelWidth; x++) {
			//LPBYTE tileMapTable = rom + SNESCore::snes2pc(type == 0 ? 0x86A549 : type == 1 ? 0x80CD0A : 0x0) +  0 * 0x80 * y;

#if 1
			for (int blockY = 0; blockY < 0x8; blockY++) {
				for (int blockX = 0; blockX < 0x8; blockX++) {
					unsigned blockIndex = 0;
					unsigned blockMap = 0;

					if (type == 0) {
						blockIndex = (y * levelWidth + x) * 0x80 + (blockY * 0x8 + blockX) * 0x2 + mapBase;
						blockMap = *LPWORD(ram + 0x4000 + blockIndex); // blockNum?
					}
					else if (type == 1) {
						blockIndex = (y * levelWidth + x) * 0x80 + (blockY * 0x8 + blockX) * 0x2 + mapBase;
						blockMap = *LPWORD(ram + 0x5000 + blockIndex); // blockNum?
					}
					else if (type == 2) {
						blockIndex = (y * levelWidth + x) * 0x40 + (blockY * 0x8 + blockX) * 0x1 + mapBase;
						blockMap = *LPWORD(ram + 0x4000 + blockIndex) & 0xFF; // blockNum?
					}

					mapping[(y * levelWidth + x) * 0x40 + (blockY * 0x8 + blockX)] = blockMap;
				}
			}

#else
			// setup tile map
			for (int i = 0; i < 0x20; i++) {
				// two sets of backgrounds in X
				unsigned offset = max((i << 3) + (y * levelWidth + x) * (type == 2 ? 0x40 : 0x100), 0);
				//unsigned tileMapOffset = offset & 0x18;
				//unsigned vramOffset = ((offset & 0xFF00) << 2) | ((offset >> 3) & 0x1F);
				//unsigned blockIndex = (((offset & (type == 2 ? 0xFF00 : 0xFF00)) >> (type == 2 ? 0 : 1)) | (offset >> (type == 2 ? 5 : 4)) & (type == 2 ? 0x7 : 0xE)) + mapBase /* FE */;
				unsigned blockIndex = mapBase;
				//unsigned blockIndex = (offset >> 5) + mapBase /* FE */;

				for (unsigned j = 0; j < 0x38 + 0x8; j += 8, blockIndex += (type == 2 ? 0x8 : 0x10)) {
					// 224 tiles in Y
					unsigned blockMap = 0;
					unsigned srcOffset = 0;

					if (type == 0) {
						blockMap = *LPWORD(ram + 0x4000 + blockIndex); // blockNum?
						srcOffset = ((blockMap & (numBlocks - 1)) << 5) + 0x2000 + mapBase /* FE */;
					}
					else if (type == 1) {
						blockMap = *LPWORD(ram + 0x5000 + blockIndex); // blockNum?
						//srcOffset = (blockMap & 0x3FFF) + 0x1000 + mapBase /* FE */;
						srcOffset = ((blockMap & (numBlocks - 1)) << 5) + 0x1000 + mapBase /* FE */;
					}
					else if (type == 2) {
						blockMap = *LPWORD(ram + 0x3000 + blockIndex) & 0xFF; // blockNum?
						srcOffset = ((blockMap & (numBlocks - 1)) << 5) + 0x2000 + mapBase /* FE */;
					}

					if (i % 4 == 0) {
						mapping[writeIndex + j + (i >> 2)] = blockMap;
					}

					//unsigned tileFlip = blockMap & 0xC000;
					//tileFlip = ((tileFlip & 0x8000) >> 1) | ((tileFlip & 0x4000) << 1);
					//unsigned blockFlip = tileFlip;
					//blockFlip = ((blockFlip << 0x8) & 0xFF00) | ((blockFlip >> 0x8) & 0x00FF);
					//blockFlip >>= 1;
					//unsigned tileMapOffsetCurrent = tileMapOffset ^ blockFlip;

					//if (isMode7()) {
					//	blockFlip = 0;
					//	tileFlip = 0;
					//	tileMapOffsetCurrent = tileMapOffset;
					//}

					//for (unsigned k = 0; k < 8; k += 2) {
					//	WORD tileOffset = *LPWORD(tileMapTable + tileMapOffsetCurrent + k);
					//	WORD tileMap = *LPWORD(ram + srcOffset + tileOffset);
					//	//*LPWORD(vram + vramOffset * 2 + ((j + k) << 5)) = tileMap;
					//	*LPWORD(mapRam + vramOffset * 2 + ((j + k) << 5)) = tileMap ^ tileFlip;
					//}
				}
			}
			writeIndex += 0x40;

#endif
			//LPWORD takeBlock = (LPWORD)(mapRam + (y * levelWidth + x) * (32 * 32 * 2));

			//for (int y = 0; y < 32; y++)
			//{
			//	for (int x = 0; x < 32; x++)
			//	{
			//		// FIXME: mode7
			//		mapping[writeIndex + 0x00] = *takeBlock++;
			//		//mapping[writeIndex + 0x01] = *takeBlock++;
			//		//mapping[writeIndex + 0x10] = *takeBlock++;
			//		//mapping[writeIndex + 0x11] = *takeBlock++;
			//		//writeIndex += 2;
			//		writeIndex++;
			//	}
			//	//writeIndex += 0x10;
			//}
		}
	}
}
void SC4Core::LoadGraphicsChange() {
	//// just setup the palette for now
	//graphicsToPalette.clear();
	//graphicsToAssembly.clear();

	//WORD levelOffset = *LPWORD(rom + pGfxObj + level * 2);
	//WORD objOffset = *LPWORD(rom + pGfxObj + levelOffset + 0 * 2);

	//// add 0
	//unsigned graphicsNum = *(rom + pGfxObj + objOffset);
	//while (graphicsNum != 0xFF) {
	//	if (!graphicsToPalette.count(graphicsNum)) {
	//		unsigned palOffset = *LPWORD(rom + pGfxPal + objOffset);
	//		graphicsToPalette[graphicsNum] = palOffset;
	//	}

	//	objOffset += 6;
	//	graphicsNum = *(rom + pGfxObj + objOffset);

	//}

	//numGfxIds = 0;
	//unsigned numBossTeleports = 0;
	//for (auto &eventList : eventTable) {
	//	for (auto &event : eventList) {
	//		if (event.type == 0x2 && (event.eventId == 0x15 || event.eventId == 0x18)) {
	//			unsigned subId = event.eventSubId;

	//			if (((subId >> 0) & 0xF) >= numGfxIds) numGfxIds = ((subId >> 0) & 0xF) + 1;
	//			if (((subId >> 4) & 0xF) >= numGfxIds) numGfxIds = ((subId >> 4) & 0xF) + 1;

	//			for (unsigned i = 0; i < 2; ++i) {
	//				objOffset = *LPWORD(rom + pGfxObj + levelOffset + (((event.eventSubId >> (i * 4)) & 0xF)) * 2);

	//				graphicsNum = *(rom + pGfxObj + objOffset);
	//				while (graphicsNum != 0xFF) {
	//					if (!graphicsToPalette.count(graphicsNum)) {
	//						unsigned palOffset = *LPWORD(rom + pGfxPal + objOffset);
	//						graphicsToPalette[graphicsNum] = palOffset;
	//					}

	//					objOffset += 6;
	//					graphicsNum = *(rom + pGfxObj + objOffset);

	//				}
	//			}
	//		}
	//		else if (event.type == 0x3) {
	//			unsigned spriteIndex = *(rom + pSpriteOffset[event.type] + ((event.eventId - 1) * (type == 2 ? 5 : 2)) + 1); //(spriteOffset == 0x32) ? 0x30 : 0x1F;
	//			BYTE spriteAssembly = *(rom + pSpriteOffset[event.type] + ((event.eventId - 1) * (type == 2 ? 5 : 2)));

	//			if (!graphicsToAssembly.count(spriteIndex)) {
	//				graphicsToAssembly[spriteIndex] = spriteAssembly;
	//			}
	//		}
	//		else if (  (type == 1 && event.type == 0x1 && event.eventId == 0x40)
	//				|| (type == 2 && event.type == 0x0 && event.eventId == 0xD)) {
	//			numBossTeleports++;
	//		}
	//	}
	//}


	//// load the boss
	//objOffset = *LPWORD(rom + pGfxObj + levelOffset + numGfxIds * 2);
	//graphicsNum = *(rom + pGfxObj + objOffset);
	//while (graphicsNum != 0xFF) {
	//	if (!graphicsToPalette.count(graphicsNum)) {
	//		unsigned palOffset = *LPWORD(rom + pGfxPal + objOffset);
	//		graphicsToPalette[graphicsNum] = palOffset;
	//	}

	//	objOffset += 6;
	//	graphicsNum = *(rom + pGfxObj + objOffset);

	//}

	//// test to load "everything".  this misses increments from some boss doors as it assumes there is only one
	//if (numGfxIds) {
	//	numGfxIds++;
	//	for (unsigned i = 0; i < numGfxIds; ++i) {
	//		objOffset = *LPWORD(rom + pGfxObj + levelOffset + i * 2);
	//		graphicsNum = *(rom + pGfxObj + objOffset);
	//		while (graphicsNum != 0xFF) {
	//			if (!graphicsToPalette.count(graphicsNum)) {
	//				unsigned palOffset = *LPWORD(rom + pGfxPal + objOffset);
	//				graphicsToPalette[graphicsNum] = palOffset;
	//			}

	//			objOffset += 6;
	//			graphicsNum = *(rom + pGfxObj + objOffset);

	//		}
	//	}
	//}
	//if (numBossTeleports) {
	//	numBossTeleports++;
	//	for (unsigned i = 0; i < numBossTeleports; ++i) {
	//		objOffset = *LPWORD(rom + pGfxObj + levelOffset + i * 2);
	//		graphicsNum = *(rom + pGfxObj + objOffset);
	//		while (graphicsNum != 0xFF) {
	//			if (!graphicsToPalette.count(graphicsNum)) {
	//				unsigned palOffset = *LPWORD(rom + pGfxPal + objOffset);
	//				graphicsToPalette[graphicsNum] = palOffset;
	//			}

	//			objOffset += 6;
	//			graphicsNum = *(rom + pGfxObj + objOffset);
	//		}
	//	}
	//}
}
void SC4Core::SetSceneBlock(unsigned sceneX, unsigned sceneY, unsigned blockX, unsigned blockY, WORD blockNum) {
	unsigned blockOffset = (sceneY * levelWidth + sceneX) * 0x80 + (blockY * 0x8 + blockX) * 0x2;
	auto it = expandedOffset[level].find(0x7E4000 + mapBase);
	//it--;
	unsigned romOffset = it->second.first;
	blockOffset -= it->first - (0x7E4000 + mapBase);
	*LPWORD(rom + romOffset + blockOffset) = blockNum;
}
WORD SC4Core::GetSceneBlock(unsigned sceneX, unsigned sceneY, unsigned blockX, unsigned blockY) {
	unsigned blockOffset = (sceneY * levelWidth + sceneX) * 0x80 + (blockY * 0x8 + blockX) * 0x2;
	auto it = expandedOffset[level].find(0x7E4000 + mapBase);
	//it--;
	unsigned romOffset = it->second.first;
	blockOffset -= it->first - (0x7E4000 + mapBase);
	return *LPWORD(rom + romOffset + blockOffset);
}
void SC4Core::GetActiveEnemyId(std::set<WORD> &id) {
	// per enemy graphics identification - required because some ids share gfx
	WORD levelOffset = *LPWORD(rom + SNESCore::snes2pc(0x868BCD + 2 * level));
	LPBYTE spriteLoad = rom + SNESCore::snes2pc(0x860000 + levelOffset);
	BYTE num = *spriteLoad++;

	std::set<WORD> gfxOffsetTable;
	for (unsigned i = 0; i < num; i++) {
		BYTE index = *spriteLoad++;
		BYTE count = *(rom + SNESCore::snes2pc(0x81AA80 + index));
		WORD gfxOffsetCurrent = *LPWORD(rom + SNESCore::snes2pc(0x81A900 + 3 * index));

		gfxOffsetTable.insert(gfxOffsetCurrent);
	}

	for (unsigned i = 0; i < 0x80; i++) {
		WORD gfxOffset = *LPWORD(rom + SNESCore::snes2pc(0x81A900 + 3 * i));

		if (gfxOffsetTable.count(gfxOffset)) {
			id.insert(i);
		}
	}
}

unsigned SC4Core::GetEventRemaining() {
	// find last event terminator
	DWORD pEvents = snes2pc(SReadWord(p_events[type] + (numLevels - 1) * 2) | (eventBank << 16));
	LPBYTE pevent = rom + pEvents;
	auto oldpevent = pevent;

	unsigned count = 0;

	if (type == 0) {
		while (*LPDWORD(pevent) != 0xFFFFFFFF) {
			pevent += 6;
		}

		DWORD pEnd = snes2pc((eventBank << 16) | (eventOffsetLimit - 4));
		LPBYTE pend = rom + pEnd;

		count = (pend - pevent) / 6;
	}
	else if (type == 1) {

	}
	else if (type == 2) {
		while (*LPWORD(pevent) != 0xFFFF) {
			pevent += 4;
		}

		DWORD pEnd = snes2pc((eventBank << 16) | (eventOffsetLimit - 2));
		LPBYTE pend = rom + pEnd;

		count = (pend - pevent) / 4;
	}

	return count;
}
bool SC4Core::AddEvent(unsigned num, EventInfo &event) {
	bool ok = (GetEventRemaining() && num && eventTable.size() >= num);

	if (ok) {
		if (type == 0) {
			if (event.type == 0x1 || event.type == 0x2) {
				// find free tag
				std::set<unsigned> tagMatch;
				for (auto &e : eventTable) {
					tagMatch.insert(e.match);
				}

				unsigned tag = 0;
				for (auto &usedTag : tagMatch) {
					if (tag == usedTag) tag++;
					else break;
				}

				event.match = tag;
			}

			auto it = eventTable.begin();
			std::advance(it, num);
			eventTable.insert(it, event);

			DWORD pEvents = snes2pc(SReadWord(p_events[type] + level * 2) | (eventBank << 16));
			LPBYTE pevent = rom + pEvents + num * 6;

			DWORD pEnd = snes2pc((eventBank << 16) | (eventOffsetLimit - 6));
			LPBYTE pend = rom + pEnd;

			// make a spot for the new event
			memmove(pevent + 6, pevent, pend - pevent);

			// update ROM state
			SaveEvents();

			for (unsigned i = level + 1; i < numLevels; i++) {
				*LPWORD(rom + snes2pc(p_events[type] + i * 2)) += 6;
			}
		}
		else if (type == 1) {

		}
		else if (type == 2) {
			auto it = eventTable.begin();
			std::advance(it, num);
			eventTable.insert(it, event);

			DWORD pEvents = snes2pc(SReadWord(p_events[type] + level * 2) | (eventBank << 16));
			LPBYTE pevent = rom + pEvents + num * 4;

			DWORD pEnd = snes2pc((eventBank << 16) | (eventOffsetLimit - 4));
			LPBYTE pend = rom + pEnd;

			// make a spot for the new event
			memmove(pevent + 4, pevent, pend - pevent);

			// update ROM state
			SaveEvents();

			for (unsigned i = level + 1; i < numLevels; i++) {
				*LPWORD(rom + snes2pc(p_events[type] + i * 2)) += 4;
			}

		}
	}

	return ok;
}
bool SC4Core::DelEvent(unsigned num) {
	bool ok = (num && eventTable.size() > num);

	if (ok) {
		if (type == 0) {
			auto it = eventTable.begin();
			std::advance(it, num);
			eventTable.erase(it);

			DWORD pEvents = snes2pc(SReadWord(p_events[type] + level * 2) | (eventBank << 16));
			LPBYTE pevent = rom + pEvents + num * 6;

			DWORD pEnd = snes2pc((eventBank << 16) + (eventOffsetLimit));
			LPBYTE pend = rom + pEnd;

			// make a spot for the new event
			memmove(pevent, pevent + 6, pend - pevent - 6);
			memset(pend - 6, 0xFF, 6);

			for (unsigned i = level + 1; i < numLevels; i++) {
				*LPWORD(rom + snes2pc(p_events[type] + i * 2)) -= 6;
			}

			// update ROM state
			SaveEvents();
		}
		else if (type == 1) {

		}
		else if (type == 2) {
			auto it = eventTable.begin();
			std::advance(it, num);
			eventTable.erase(it);

			DWORD pEvents = snes2pc(SReadWord(p_events[type] + level * 2) | (eventBank << 16));
			LPBYTE pevent = rom + pEvents + num * 4;

			DWORD pEnd = snes2pc((eventBank << 16) + (eventOffsetLimit));
			LPBYTE pend = rom + pEnd;

			// make a spot for the new event
			memmove(pevent, pevent + 4, pend - pevent - 4);
			memset(pend - 4, 0xFF, 4);

			for (unsigned i = level + 1; i < numLevels; i++) {
				*LPWORD(rom + snes2pc(p_events[type] + i * 2)) -= 4;
			}

			// update ROM state
			SaveEvents();
		}
	}

	return ok;
}
void SC4Core::ReallocScene(BYTE scene)
{
	WORD writeIndex = SReadWord(0x868D20) + 0x100 * scene;
	for(int y=0; y<8; y++)
	{
		for(int x=0; x<8; x++)
		{
			LPWORD takeBlock = (LPWORD)(rom + pScenes + (scene*0x80) + x*2 + y*0x10);
			takeBlock = (LPWORD)(rom + pBlocks + *takeBlock * 8);
				
			mapping[writeIndex + 0x00] = *takeBlock++;
			mapping[writeIndex + 0x01] = *takeBlock++;
			mapping[writeIndex + 0x10] = *takeBlock++;
			mapping[writeIndex + 0x11] = *takeBlock++;
			writeIndex += 2;
		}
		writeIndex += 0x10;
	}
}
void SC4Core::LoadLayout()
{
	if (type >= 1) {

		if (type == 1) {
			levelWidth = 32;
			levelHeight = 1;
		}
		else if (type == 2) {
			levelWidth = 32;
			levelHeight = 2;
		}
		sceneUsed = levelHeight * levelWidth;
		return;
	}

	switch (GetLevelType()) {
	case 0x0: // 1 30 31 32 35 37 46 47 51 55 57
		levelWidth = 8;
		levelHeight = 2;
		break;
	case 0x1: // 12 25
		levelWidth = 32;
		levelHeight = 2;
		break;
	case 0xD: // Same as handler 2
	case 0x2: // 10 11
		levelWidth = 16;
		levelHeight = 4;
		break;
	case 0x3: // 13
		levelWidth = 2;
		levelHeight = 32; //6
		break;
	case 0x4:
		levelWidth = 0;
		levelHeight = 0;
		break;
	case 0xF: // Same as handler 5
	case 0x5: // 21 22 27
		levelWidth = level == 27 ? 10 : 8;
		levelHeight = 2;
		break;
	case 0x6: // 0
		levelWidth = 16; // 4
		levelHeight = 1;
		break;
	case 0x7: // 17 52 56 58
		levelWidth = 1;
		levelHeight = 16; // 8
		break;
	case 0x8: // 36 44
		levelWidth = 1;
		levelHeight = 16; // 4
		break;
	case 0x9: // 2 3 4 5 6 7 18 19 20 26 28 29 33 34 38 39 40 41 48 49 50 53 54 59 61 63 64 65
		levelWidth = 16;
		levelHeight = 1;
		break;
	case 0xA: // 8 24
		levelWidth = 16;
		levelHeight = 4;
		break;
	case 0xB: // 23
		levelWidth = 16;
		levelHeight = 4;
		break;
	case 0xC: // 9
		levelWidth = 32;
		levelHeight = 2;
		break;
	case 0xE: // 14
		levelWidth = 16; // 4
		levelHeight = 1;
		break;
	case 0x10: // 42 43 45
		levelWidth = 16;
		levelHeight = 1;
		break;
	case 0x11: // 60
		levelWidth = 1;
		levelHeight = 16; // 4
		break;
	case 0x12: // 62
		levelWidth = 1;
		levelHeight = 16;
		break;
	case 0x13: // 66 67
		levelWidth = level == 66 ? 1 : 4; // workaround for garbage ??like graphicle bugs in draculas level??
		break;
	case 0x14:
		levelWidth = 0;
		levelHeight = 0;
		break;
	case 0x15:
		levelWidth = 0;
		levelHeight = 0;
		break;
	case 0x16:
		levelWidth = 0;
		levelHeight = 0;
		break;
	case 0x17:
		levelWidth = 0;
		levelHeight = 0;
		break;
	case 0x18:
		levelWidth = 0;
		levelHeight = 0;
		break;
	case 0x19:
		levelWidth = 0;
		levelHeight = 0;
		break;
	case 0x1A:
		levelWidth = 0;
		levelHeight = 0;
		break;
	case 0x1B:
		levelWidth = 0;
		levelHeight = 0;
		break;
	case 0x1C:
		levelWidth = 0;
		levelHeight = 0;
		break;
	case 0x1D:
		levelWidth = 0;
		levelHeight = 0;
		break;
	case 0x1E:
		levelWidth = 0;
		levelHeight = 0;
		break;
	case 0x1F:
		levelWidth = 0;
		levelHeight = 0;
		break;
	}

	//if (levelWidth < 4) levelWidth = 4;
	//if (levelHeight < 2) levelHeight = 2;

	sceneUsed   = levelHeight * levelWidth;

	//WORD writeIndex = 0;
	//byte ctrl;
	//while((ctrl = *playout++) != 0xFF)
	//{
	//	byte buf = *playout++;
	//	for(int i=0; i<(ctrl & 0x7F); i++)
	//	{
	//		sceneLayout[writeIndex++] = buf;
	//		if ((ctrl & 0x80) == 0)
	//			buf++;
	//	}
	//}
	return;
}
void SC4Core::SwitchLevelEvent(bool ev)
{
	WORD src, dest;
	switch(level)
	{
	case 2:
		sceneLayout[0x64] = sceneLayout[0xA0];
		sceneLayout[0x65] = sceneLayout[0xA1];
		sceneLayout[0x66] = sceneLayout[0xA2];
		sceneLayout[0x67] = sceneLayout[0xA3];
		
		sceneLayout[0x41] = sceneLayout[0xA4];
		sceneLayout[0x42] = sceneLayout[0xA5];
		sceneLayout[0x43] = sceneLayout[0xA6];
		sceneLayout[0x44] = sceneLayout[0xA7];
		break;
	case 4:
		src  = (WORD)((ReadDWord(0x3C79) & 0xFFFFFF) - 0x7EE800);
		dest = (WORD)((ReadDWord(0x3C7D) & 0xFFFFFF) - 0x7EE800);
		sceneLayout[dest] = sceneLayout[src];
		break;
	case 6:
		src  = (WORD)((ReadDWord(0x3C8B) & 0xFFFFFF) - 0x7EE800);
		dest = (WORD)((ReadDWord(0x3C8F) & 0xFFFFFF) - 0x7EE800);
		sceneLayout[dest-3] = sceneLayout[src-4];
		break;
	}
}
void SC4Core::SaveLevel() {
	//byte tvram[0x8000];
	//ZeroMemory(vram, 0x8000);
	//for (int i = 0; i<0x400; i++)
	//	raw2tile4bpp(vramCache + (i * 0x40), vram + (i * 0x20));

	//SortTiles();

	//// compressed sizes from unmodified roms
	//WORD origSize[][14] = { { 0,0,0,0,0,0,0,0,0,0,0,0,0,0 },
	//{ 0x5418,0x53CF,0x4946,0x4957,0x438A,0x4A2D,0x42D3,0x486A,0x4487,0x1A56,0x1EFA,0x1E66,0x1E78,0x0000 },
	//{ 0x496C,0x50FF,0x5EF5,0x3EDF,0x5614,0x3208,0x541A,0x5E32,0x50B8,0x354A,0x48E3,0x3FFF,0x3A3A,0x33F9 } };

	//WORD size = origSize[type][level] ? origSize[type][level] : tileCmpRealSize;

	//WORD newSize = GFXRLECmp(vram + 0x200, tvram, tileCmpSize, type);

	//if (newSize > size)
	//	switch (MessageBox(hWnd, "The compressed tiles size is more than the original compress size.\nYou can corrupt the data of ROM if you don't know what are you doing.\nAre you sure to continue with the compression?",
	//		"Warning", MB_ICONWARNING | MB_YESNOCANCEL))
	//	{
	//	case IDYES:
	//		COMPRESS_TILES:
	//			memcpy(rom + tileCmpPos, tvram, newSize);
	//			if (size > newSize)
	//				ZeroMemory(rom + tileCmpPos + newSize, size - newSize);

	//			break;
	//	case IDNO:
	//		break;
	//	case IDCANCEL:
	//		return 0;
	//	}
	//else goto COMPRESS_TILES;
	//if (tileDecSize) {
	//	memcpy(rom + tileDecPos, vram + tileDecDest, tileDecSize);
	//}

	//// compressed sizes from unmodified roms
	//WORD eventSize = GetOrigEventSize() ? GetOrigEventSize() : eventSize;
	//WORD newEventSize = GetEventSize();

	//if (newEventSize > eventSize)
	//	switch (MessageBox(hWnd, "The event size is more than the original event size.\nYou can corrupt the data of ROM if you don't know what are you doing.\nAre you sure to continue?",
	//		"Warning", MB_ICONWARNING | MB_YESNOCANCEL))
	//	{
	//	case IDYES:
	//		WRITE_EVENTS:
	//			// Update the events
	//			SaveEvents();
	//			break;
	//	case IDNO:
	//		break;
	//	case IDCANCEL:
	//		return 0;
	//	}
	//else goto WRITE_EVENTS;


	SaveTiles();
	SaveEvents();
	SaveSprites();
}
void SC4Core::SaveTiles() {

}
void SC4Core::SaveEvents() {
	if (!p_events[type]) return;

	if (type == 0) {
		DWORD pEvent = snes2pc(SReadWord(p_events[type] + level * 2) | (eventBank << 16));
		LPBYTE pevent = rom + pEvent;

		for (auto &event : eventTable) {
			// TODO: write out events

			//event.spawnIndex = ((*LPWORD(pevent + 2) >> 8) & 0xF0) | ((*LPWORD(pevent + 0) >> 12) & 0x0F);

			if (event.type == 0 || event.type == 2) {
				// enemy
				// custom
				*LPWORD(pevent + 0) = ((event.match << 12) & 0xF000) | (event.xpos & ~0xF003) | (event.type & 0x3);
				*LPWORD(pevent + 2) = ((event.match << 8) & 0xF000) | (event.ypos & ~0xF003) | (event.unknown & 0x3);
				*LPBYTE(pevent + 4) = (event.eventId & 0x00FF);
				*LPBYTE(pevent + 5) = (event.eventSubId & 0x00FF);
			}
			else if (event.type == 1) {
				// candle
				*LPWORD(pevent + 0) = ((event.eventSubId << 12) & 0xF000) | (event.xpos & ~0xF003) | (event.type & 0x3);
				*LPWORD(pevent + 2) = ((event.eventSubId << 8) & 0xF000) | (event.ypos & ~0xF003) | (event.unknown & 0x3);
				*LPBYTE(pevent + 4) = (event.match & 0x00FF);
				*LPBYTE(pevent + 5) = (event.eventId & 0x00FF);
			}
			pevent += 6;
		}

		*LPDWORD(pevent) = 0xFFFFFFFF;
	}
	else if (type == 1) {
		if (level == 1 || level == 3 || level == 4) return;

		DWORD pEvents = snes2pc(SReadWord(p_events[type] + level * 2) | (eventBank << 16));
		LPBYTE pevent = rom + pEvents;
		DWORD pEventsLimit = snes2pc(*LPWORD(pevent) | (eventBank << 16));
		LPBYTE plimitevent = rom + pEventsLimit;

		// ABCDEEFFGG
		// A = 8b = mask
		// B = 8b = id
		// C = 8b = max slot >> 4
		// D = 8b = min slot >> 4
		// EE = xpos
		// FF = ypos
		// GG = subid

		auto eventIt = eventTable.begin();

		unsigned sceneNum = 0;
		while (pevent != plimitevent) {
			DWORD a = snes2pc(*LPWORD(pevent) | (eventBank << 16));
			LPBYTE pe = rom + a;

			while (*pe != 0xFF) {
				auto event = *eventIt;

				*LPBYTE(pe + 0) = event.match;
				*LPBYTE(pe + 1) = event.eventId;
				*LPWORD(pe + 2) = event.unknown;
				*LPWORD(pe + 4) = event.xpos - sceneNum * 0x100;
				*LPWORD(pe + 6) = event.ypos;
				*LPWORD(pe + 8) = event.eventSubId;
				pe += 0xA;
				eventIt++;
			}
			pevent += 2;

			sceneNum++;
		}
	}
	else if (type == 2) {
		DWORD pEvent = snes2pc(SReadWord(p_events[type] + level * 2) | (eventBank << 16));
		LPBYTE pevent = rom + pEvent;

		for (auto &event : eventTable) {
			// TODO: write out events

			WORD x = event.xpos; // +0x28;

			//event.type = *LPWORD(pevent + 2) & 0x3;
			//event.xpos = ((*LPWORD(pevent + 0) & 0xFFC0) >> 3); // -0x28;

			if (event.type == 0 || event.type == 1) {
				//event.ypos = (*LPWORD(pevent + 2) & 0xFC) + ((*LPWORD(pevent + 0) & 0x20) << 3); // ypos
				//event.eventId = *LPWORD(pevent + 3) & 0x7F; // id
				//event.eventSubId = *LPWORD(pevent + 0) & 0x1F; // slot
				//event.unknown = *LPWORD(pevent + 2) & 0x8000;

				*LPWORD(pevent + 0) = ((x << 3) & 0xFFC0) | ((event.ypos & 0x100) >> 3) | (event.eventSubId & 0x1F);
				*LPBYTE(pevent + 2) = (event.ypos & 0xFC) | (event.type & 0x3);
				*LPBYTE(pevent + 3) = (event.eventId & 0x7F) | (event.unknown >> 8);
			}
			else if (event.type == 2) {
				//event.ypos = *LPWORD(pevent + 2) & 0x00FC;
				//event.eventId = *LPWORD(pevent + 3) & 0x007F;
				//event.eventSubId = *LPWORD(pevent + 0) & 0x003F;
				//event.unknown = *LPWORD(pevent + 2) & 0x8000;

				*LPWORD(pevent + 0) = ((x << 3) & 0xFFC0) | (event.eventSubId & 0x3F);
				*LPWORD(pevent + 2) = (event.unknown) | (event.ypos & 0x00FC) | (event.type & 0x3) | ((event.eventId & 0x7F) << 8);
			}
			else if (event.type == 3) {
				//event.ypos = 0;
				//event.eventId = *LPBYTE(pevent + 0) & 0x3F; // id
				//event.eventSubId = *LPBYTE(pevent + 3) & 0xFF;
				//event.unknown = *LPBYTE(pevent + 2) & 0xFC;

				*LPWORD(pevent + 0) = ((x << 3) & 0xFFC0) | (event.eventId & 0x3F);
				*LPBYTE(pevent + 2) = (event.unknown & 0xFC) | (event.type & 0x3);
				*LPBYTE(pevent + 3) = (event.eventSubId & 0xFF);
			}

			pevent += 4;
		}

		*LPWORD(pevent) = 0xFFFF;
	}

}
void SC4Core::SaveSprites() {

	// save the sprite cache
	for (unsigned i = 0; i < NUM_SPRITE_TILES; ++i) {
		if (spriteUpdate.count(i)) {
			raw2tile4bpp(spriteCache + (i << 6), rom + (romSize - (NUM_SPRITE_TILES << 5)) + (i << 5));
			spriteUpdate.erase(i);
		}
	}

}

bool SC4Core::LoadLevelFromFile(std::string fileName)
{
	if (!expandedROM)
		return false;

	unsigned currentOffset = 0;
	std::vector<BYTE> lData;

	// open file
	auto hLevelFile = CreateFile(fileName.c_str(), GENERIC_READ | GENERIC_WRITE, FILE_SHARE_READ, NULL, OPEN_EXISTING, 0, NULL);
	if (!hLevelFile) {
		return false;
	}
	lData.resize(GetFileSize(hLevelFile, NULL));
	OVERLAPPED overlapped;
	ZeroMemory(&overlapped, sizeof(OVERLAPPED));
	DWORD bytesR;
	if (!ReadFile(hLevelFile, &lData[0], lData.size(), &bytesR, &overlapped))
		return false;
	if (bytesR != lData.size())
		return false;

	if (hLevelFile)
		CloseHandle(hLevelFile);

	// check version
	if (*LPWORD(&lData[0]) != levelFormatVersion) return false;
	if (*LPWORD(&lData[2]) != expandedROMVersion) return false;
	WORD levelNum = *LPWORD(&lData[4]);
	currentOffset = 0x100;

	// check properties of level before making any changes
	while (currentOffset < lData.size()) {
		DWORD addr = *LPDWORD(&lData[currentOffset]);
		currentOffset += 4;
		unsigned size = *LPDWORD(&lData[currentOffset]);
		currentOffset += 4;

		if (currentOffset + size > lData.size()) {
			return false;
		}

		//memcpy(rom + snes2pc(addr), &lData[currentOffset], size);
		currentOffset += size;
	}

	currentOffset = 0x100;
	// load level
	level = levelNum;
	LoadLevel();

	while (currentOffset < lData.size()) {
		DWORD addr = *LPDWORD(&lData[currentOffset]);
		currentOffset += 4;
		unsigned size = *LPDWORD(&lData[currentOffset]);
		currentOffset += 4;

		if (currentOffset + size > lData.size()) return false;

		if (addr < 0x800000) {
			if (!expandedOffset[level].count(addr) || (expandedOffset[level][addr].second < size)) return false;
			addr = pc2snes(expandedOffset[level][addr].first);
		}

		if ((addr >> 16) == eventBank) {
			while (size > GetEventSize()) AddEvent(1, EventInfo());
			while (size < GetEventSize()) DelEvent(1);
			memcpy(rom + snes2pc(SReadWord(p_events[type] + level * 2) | (eventBank << 16)), &lData[currentOffset], size);
		}
		else {
			memcpy(rom + snes2pc(addr), &lData[currentOffset], size);
		}
		currentOffset += size;
	}

	// reload the events

	return true;
}

bool SC4Core::SaveLevelToFile(std::string fileName)
{
	bool ok = expandedROM;

	// format of file
	// o header $200B?
	// - original number
	// - type
	// o (un)compressed level source
	// - scene data
	// - block data
	// - tile data
	// o checkpoint data
	// - state
	// - player pos
	// - cam0
	// - cam1
	// o initial camera lock
	// o camera lock data (expanded)
	// - this is hard.  we would have to look through all levels and find an unused index - might not be sufficient
	// - could copy raw lock and compare to see if can find the same
	// o transition data (expanded)
	// o tile collision (expanded)
	// o events
	// [ o address tables ]
	// [ - BG data ]
	// [ - Sprites ]
	// [ - Blocks ]
	// [ - Scenes ]
	// [ o sprite map ]
	//
	// Seems easiest if we assume special events (not including camera locks and level transition) are already available

	unsigned currentOffset = 0;
	std::vector<BYTE> lData;
	// header
	// $100 B
	// - 2 version
	lData.resize(lData.size() + 0x2);
	memcpy(&lData[currentOffset], &levelFormatVersion, lData.size() - currentOffset);
	currentOffset = lData.size();
	// - 2 expanded version
	lData.resize(lData.size() + 0x2);
	memcpy(&lData[currentOffset], &expandedROMVersion, lData.size() - currentOffset);
	currentOffset = lData.size();
	// - 2 level number
	lData.resize(lData.size() + 0x2);
	memcpy(&lData[currentOffset], &level, lData.size() - currentOffset);
	currentOffset = lData.size();
	
	ok &= lData.size() <= 0x100;

	lData.resize(0x100);
	currentOffset = lData.size();

	// FORMAT: [snesRomAddr4, size4, <data>]
	std::vector<std::tuple<DWORD, unsigned>> levelAddresses = {
		// Tiles
		{ /*pc2snes(expandedOffset[level][0x002000].first)*/0x002000, 0x4000 },
		// Blocks BG0
		{ /*pc2snes(expandedOffset[level][0x7E6000].first)*/0x7E6000, 0x2000 },
		// Blocks BG1
		{ /*pc2snes(expandedOffset[level][0x7EC000].first)*/0x7EC000, 0x2000 },
		// Scenes BG0
		{ /*pc2snes(expandedOffset[level][0x7E8000].first)*/0x7E8000, 0x2000 },
		// Scenes BG1
		{ /*pc2snes(expandedOffset[level][0x7EE000].first)*/0x7EE000, 0x2000 },
		// Tile Collision
		{ pc2snes(snes2pc(0xA28000) + (level % 0x20) * 0x400 + (level / 0x20) * 0x8000), 0x400 },
		// Property: Timer
		{ 0x85BCF8 + 2 * level, 0x2 },
		//// Property: InitCameraLeft
		//{ 0x85CFD2 + 2 * level, 0x2 },
		//// Property: InitCameraRight
		//{ 0x85D05A + 2 * level, 0x2 },
		//// Property: InitCameraTop
		//{ 0x85D0E2 + 2 * level, 0x2 },
		//// Property: InitCameraBottom
		//{ 0x85D16A + 2 * level, 0x2 },
		// Property: Type
		{ 0x868296 + 2 * level, 0x2 },
		// Property: DeathLevel
		{ (region == 0 ? 0x81B395 : 0x81B369) + level, 0x1 },
		// Property: ContinueLevel
		{ (region == 0 ? 0x81FBAC : 0x81FBAC) + level, 0x1 },

		//// Property: Forward CP Data
		//// CP: state0
		//{ 0x85BE08 + 2 * level + 0, 0x1 },
		//// CP: state1
		//{ 0x85C138 + 2 * level, 0x2 },
		//// CP: player pos
		//{ 0x85BE90 + 2 * level, 0x4 },
		//// CP: cam0 pos
		//{ 0x85CB92 + 2 * level, 0x4 },
		//// CP: cam0 pos
		//{ 0x85CDB2 + 2 * level, 0x4 },

		//// Property: Reverse CP Data
		//{ 0x85BE08 + 2 * level + 1, 0x1 },
		//// CP: state1
		//{ 0x85C1C0 + 2 * level, 0x2 },
		//// CP: player pos
		//{ 0x85BFA0 + 2 * level, 0x4 },
		//// CP: cam0 pos
		//{ 0x85CCA2 + 2 * level, 0x4 },
		//// CP: cam0 pos
		//{ 0x85CEC2 + 2 * level, 0x4 },

		// camera lock data
		{ 0xA58000 + 0xC * 0x20 * level, 0xC * 0x20 },

		// transition data
		{ 0xA68000 + 0x4 * 0x40 * level, 0x4 * 0x40 },

		// entrance (checkpoint) data
		{ 0xA78000 + level * 0x100, 0x100 },

		// events (MUST BE LAST)
		{ SReadWord(p_events[type] + level * 2) | (eventBank << 16), GetEventSize() }
	};

	// record all data
	for (auto &t : levelAddresses) {
		lData.resize(lData.size() + sizeof(std::get<0>(t)));
		memcpy(&lData[currentOffset], &(std::get<0>(t)), lData.size() - currentOffset);
		currentOffset = lData.size();
		lData.resize(lData.size() + sizeof(std::get<1>(t)));
		memcpy(&lData[currentOffset], &(std::get<1>(t)), lData.size() - currentOffset);
		currentOffset = lData.size();
		lData.resize(lData.size() + std::get<1>(t));
		auto addr = std::get<0>(t);
		if (addr < 0x800000) {
			if (!expandedOffset[level].count(addr)) {
				ok &= false;
				break;
			}
			addr = pc2snes(expandedOffset[level][addr].first);
		}
		memcpy(&lData[currentOffset], rom + snes2pc(addr), lData.size() - currentOffset);
		currentOffset = lData.size();
	}

	// write out level file
	auto hLevelFile = CreateFile(fileName.c_str(), GENERIC_READ | GENERIC_WRITE, FILE_SHARE_READ, NULL, CREATE_ALWAYS, 0, NULL);
	OVERLAPPED overlapped;
	ZeroMemory(&overlapped, sizeof(OVERLAPPED));
	DWORD bytesW;
	if (!WriteFile(hLevelFile, &lData[0], lData.size(), &bytesW, &overlapped))
		ok &= false;
	if (bytesW != lData.size())
		ok &= false;

	if (hLevelFile)
		CloseHandle(hLevelFile);

	return ok;
}
