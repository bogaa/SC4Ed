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

#ifndef NDSCORE_H
#define NDSCORE_H

#include "stdafx.h"
//#include "SC4ED.h"

class NDSCore
{
private:
public:
	static void tile4bpp2raw(LPBYTE src, LPBYTE dst);
	static void raw2tile4bpp(LPBYTE src, LPBYTE dst);
};

#endif