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

#ifndef _SC4Core_H_
#define _SC4Core_H_

#include "SNESCore.h"
#include "NDSCore.h"

#include <set>
#include <map>
#include <list>
#include <array>
#include <vector>

#define NUM_SPRITE_TILES 0x2000
#define NUM_SPRITE_PALETTES 0x200

const char *levelsName[][0x44];

#pragma pack(push, 1)
struct EventInfo
{
	WORD match = 0; // ?
	BYTE type = 0; // 0 = enemy?, 1 = candle
	WORD xpos = 0;
	WORD ypos = 0;
	WORD eventId = 0;
	WORD eventSubId = 0;
	WORD spawnIndex = 0;
	WORD unknown = 0;
};

struct CameraLockInfo {
	WORD direction;
	WORD dirAddr;
	WORD cmpAddr;
	WORD cmpValue;
	WORD stValue;
	WORD stAddr;
};

struct LevelExitInfo {
	BYTE type;
	BYTE exitNum;
	WORD cmpValue;
};

struct LevelTransitionInfo {
	BYTE nextLevel;
	BYTE entranceNum;
};

struct LevelEntranceInfo {
	WORD deathState0;
	WORD xpos;
	WORD ypos;
	WORD c0_xpos;
	WORD c0_ypos;
	WORD c1_xpos;
	WORD c1_ypos;
	WORD state1;

	WORD lockState;
	WORD lockLeft;
	WORD lockRight;
	WORD lockTop;
	WORD lockBottom;
	WORD lockA8;
	WORD lockAA;
	WORD coll;
};
#pragma pack(pop)

extern std::map<unsigned, std::string> TileTypeMap;

class SC4Core : public SNESCore
{
private:
	bool GetBaseAddr(unsigned mode, DWORD addr, DWORD &baseAddr, WORD &newSize);

public:
	BYTE type;
	BYTE region;
	BYTE ram[0x20000];
	BYTE vram[0x20000];
	BYTE mapRam[2 * 32 * 32 * 0x20 * 0x20];
	WORD mapping[/*(0xA600 - 0x2000)/2*/ (32 * 32 * 0x20 * 0x20)];
	WORD mapBase = 0x4000;
	WORD mapBaseAdder = 0x0; // FIXME actual base addresses
	BYTE mappingBG[0xE800 - 0xA600];
	BYTE sceneLayout[0x400];

	// Palettes relative
	DWORD palettesOffset[8];
	// Tiles relative
	DWORD tileCmpPos;
	WORD  tileCmpDest;
	WORD  tileCmpSize;
	WORD  tileCmpRealSize;

	// checkpoints
	//std::vector<CheckPointInfo> checkpointInfoTable;

	// graphics to palette
	std::map<unsigned, unsigned> graphicsToPalette;
	std::map<unsigned, unsigned> graphicsToAssembly;

	unsigned objLoadOffset = 0;
	unsigned tileLoadOffset = 0;
	unsigned palLoadOffset = 0;

	WORD level = 0, point = 0;
	WORD levelWidth = 0, levelHeight = 0, sceneUsed = 0;
	DWORD pPalette, pPalBase = 0, pLayout, pScenes, pBlocks, pMaps, pCollisions, pEvents, pBorders, pLocks, pProperties, pGfx, pGfxPos = 0, pGfxObj = 0, pGfxPal = 0, pSpriteAssembly, pSpriteOffset[4] = { 0,0,0,0 };
	DWORD pCapsulePos = 0;

	unsigned numLevels = 0;
	unsigned numTiles  = 0;
	unsigned numMaps   = 0;
	unsigned numBlocks = 0;
	unsigned numDecs   = 0;
	unsigned numCheckpoints = 0;
	unsigned numGfxIds = 0;

	bool tileDecEnabled = false;
	std::vector<std::tuple<DWORD, DWORD, DWORD>> tileDec;
	std::vector<DWORD> dynDecTable;
	std::vector<DWORD> dynPalTable;

	bool sortOk = false;

	void GenerateExpandedOffset();
	BYTE CheckROM();
	bool ExpandROM();
	UINT16 GetChecksum();
	UINT32 GetCRC32();

	// expansion state
	// ROM expansion
	static const char expandedROMString[];
	static const WORD expandedROMVersion;
	static const unsigned expandedROMHeaderSize;

	bool expandedROM = false;
	unsigned expandedVersion = 0x0;
	unsigned eventBank = 0x0;
	unsigned eventOffsetLimit = 0x0;

	std::map<unsigned, std::map<unsigned, std::pair<unsigned, unsigned>>> expandedOffset;

	// level format
	static const WORD levelFormatVersion;

	// Events
	typedef std::list<EventInfo> EventList;
	EventList eventTable;
	unsigned totalEvents;
	WORD  eventSize;

	// SPRITE
	std::set<unsigned> spriteUpdate;

	// FONT
	WORD fontPalCache[0x20];
	BYTE fontCache[0x4800];
	DWORD GetFontPointer();
	void LoadFont();
	void LoadFont(int);

	static WORD ConvertWordBCDToDec(WORD x);
	static WORD ConvertWordDecToBCD(WORD x);

	DWORD GetCheckPointPointer();
	DWORD GetCheckPointBasePointer();
	unsigned GetEventSize();
	unsigned GetOrigEventSize();
	WORD GetLevelType(int levelNum = -1);
	void SetLevelType(WORD type, int levelNum = -1);
	WORD GetTileVramByteAddr(int levelNum = -1);
	RECT GetBoundingBox(const EventInfo &event);
	WORD GetTileType(WORD tileNum, int levelNum = -1);
	void SetTileType(WORD tileNum, WORD type, int levelNum = -1);
	CameraLockInfo &GetCameraLock(unsigned subId);
	LevelExitInfo &GetLevelExit(unsigned subId, int levelNum = -1);
	LevelTransitionInfo &GetLevelTransition(unsigned exitNum, int levelNum = -1);
	LevelEntranceInfo &GetLevelEntrance(unsigned entrance, int levelNum = -1);

	bool isMode7(int levelNum = -1);

	void LoadVRAM();
	void LoadGFXs();
	void LoadTiles();
	void SortTiles();
	void SetLevel(WORD, WORD);
	void LoadLevel(bool refresh = false);
	void LoadBackground();
	void LoadTilesAndPalettes();
	void LoadEvents();
	void SortEvents();
	void SlotEvents();
	//void LoadProperties();
	//void LoadCheckpoints();
	void LoadGraphicsChange();

	void SetSceneBlock(unsigned sceneX, unsigned sceneY, unsigned blockX, unsigned blockY, WORD blockNum);
	WORD GetSceneBlock(unsigned sceneX, unsigned sceneY, unsigned blockX, unsigned blockY);

	void GetActiveEnemyId(std::set<WORD> &id);

	unsigned GetEventRemaining();
	bool AddEvent(unsigned num, EventInfo &event);
	bool DelEvent(unsigned num);

	void SaveLevel();
	void SaveTiles();
	void SaveEvents();
	void SaveSprites();

	void LoadPaletteDynamic();
	void LoadLevelLayout();
	void ReallocScene(BYTE);
	void LoadLayout();

	void SwitchLevelEvent(bool);

	// level file interface
	bool LoadLevelFromFile(std::string fileName);
	bool SaveLevelToFile(std::string fileName);

};

#endif