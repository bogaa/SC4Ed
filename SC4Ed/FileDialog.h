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

#ifndef _FILE_DIALOG_
#define _FILE_DIALOG_

#include "stdafx.h"

LPTSTR OpenFileDialog(HWND hWnd, LPCTSTR filter, LPSTR fileName);
LPTSTR SaveFileDialog(HWND hWnd, LPCTSTR filter, LPSTR fileName);
LPBYTE OpenFileRam(LPTSTR fileName, LPDWORD size);

#endif