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

#ifndef _SC4EDCore_H_
#define _SC4EDCore_H_

//#include "SC4ED.h"
#include "CompressionCore.h"

class SC4EDCore
{
private:
	HWND hWnd;
	HANDLE hFile;
public:
	LPBYTE rom;
	DWORD romSize;
	WORD dummyHeader;
	char filePath[MAX_PATH];

	SC4EDCore();
	~SC4EDCore();
	void GetHWND(HWND);
	void FreeRom();
	bool LoadNewRom(LPCSTR);
	bool SaveRom(LPCSTR);
	bool SaveAsRom(LPCSTR);

	virtual void Init();
	virtual void Save();
	virtual void Exit();
};

#endif