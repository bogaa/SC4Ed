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

#include "CompressionCore.h"
#include <cstdio>

static void ComputeKMP(BYTE *src, int *table, int maxLength) {
	table[0] = -1;
	table[1] = 0;

	int i = 2;
	int j = 0;

	while (i < maxLength) {
		if (src[i - 1] == src[j]) {
			table[i++] = ++j;
		}
		else if (j > 0) {
			j = table[j];
		}
		else {
			table[i++] = 0;
		}
	}
}

typedef struct matchPair {
	unsigned offset;
	unsigned length;
} matchPair;

#define MAX_LENGTH 63
#define MIN_LENGTH 3
#define WINDOW_SIZE 1023

static matchPair Find(BYTE *src, const unsigned windowStart, const unsigned uncompStart, const unsigned size) {
	matchPair match;
	unsigned i, m;
	int table[MAX_LENGTH];
	BYTE *localSrc = src + uncompStart;
	BYTE *localWindow = src + windowStart;

	ComputeKMP(localSrc, table, min(size - uncompStart, MAX_LENGTH));

	match.length = 0;
	match.offset = 0;
	m = 0;
	i = 0;

	while (m < uncompStart - windowStart) {

		if (localSrc[i] == localWindow[m + i]) {
			++i;

			if (i == MAX_LENGTH) {
				match.length = MAX_LENGTH;
				match.offset = uncompStart - windowStart - m;
				break;
			}
			else if (uncompStart + i == size) {
				// special case to handle when we matched up all the way to the end of the source
				// this must be the longest match so we can exit out
				match.length = i;
				match.offset = uncompStart - windowStart - m;
				break;
			}
		}
		else {
			if (i > match.length) {
				match.length = i;
				match.offset = uncompStart - windowStart - m;
			}

			m += i - table[i];
			//++m;

			if (table[i] > 0) {
				i = table[i];
			}
			else {
				i = 0;
			}
		}
	}

	return match;
}

int GFXRLE(BYTE* rom, BYTE *dest, int pointer, int size, int type, bool obj)
{
	int oldPointer = pointer;
	
	if (true) {
		// SC4 has 5 modes
		// < 80 - LZ
		// < A0 - uncomp
		// < C0 - 8b to 16b pad to 0
		// < E0 - RLE
		// else - RLE 0
		unsigned writeIndex = 0;

		while (pointer - oldPointer < (size - 2)) {
			BYTE readSize = pointer - oldPointer + 2;
			WORD tempAddr = 0x8708 + readSize;
			BYTE control = rom[pointer++];
			BYTE localSize = control & 0x1F;

			if (control < 0x80) {
				// doesn't AND with 0x1F
				localSize = (control >> 2) + 2;
				short src = (WORD(control) << 8) | WORD(rom[pointer++]);
				src = (writeIndex & 0xFC00) | ((src - 0x3DF) & 0x3FF);
				if (src >= writeIndex) {
					src -= 0x400;
				}
				for (int i = 0; i < localSize; ++i) {
					dest[writeIndex++] = (i + src < 0) ? 0 : dest[src + i];
					//if (pointer >= size) break;
				}
			}
			else if (control < 0xA0) {
				// uncomp
				for (unsigned i = 0; i < (control == 0x80 ? MAXUINT32 : localSize); ++i) {
					BYTE value = rom[pointer++];
					dest[writeIndex++] = value;
					if (pointer - oldPointer >= size - 2) {
						break;
					}
				}
			}
			else if (control < 0xC0) {
				localSize += 2;
				// 16b pad with 0
				for (unsigned i = 0; i < localSize; ++i) {
					dest[writeIndex++] = 0;
					dest[writeIndex++] = rom[pointer++];
					//if (pointer >= size) break;
				}
			}
			else if (control < 0xE0) {
				localSize += 2;
				// RLE
				BYTE value = rom[pointer++];
				for (unsigned i = 0; i < localSize; ++i) {
					dest[writeIndex++] = value;
					//if (pointer >= size) break;
				}
			}
			else {
				localSize += 2;
				// RLE w/ 0
				for (unsigned i = 0; i < localSize; ++i) {
					dest[writeIndex++] = 0;
					//if (pointer >= size) break;
				}
			}
		}

		return writeIndex;
	}
	else if (type == 0) {
		int writeIndex = 0;

		for (int i = 0; i < size >> 3; i++)
		{
			byte control = rom[pointer++];
			byte data = rom[pointer++];
			for (int j = 0; j < 8; j++)
			{
				if (control & 0x80)
					dest[writeIndex++] = rom[pointer++];
				else
					dest[writeIndex++] = data;
				control <<= 1;
			}
		}
	}
	else {
		// X2 and X3 use a more complicated dictionary compression where chunks of uncompressed data
		// can be repeated using an encoded relative offset and length.  This is basically LZSS with
		// a snes-friendly file format.

		byte control = rom[pointer++];
		unsigned bitPos = 7;
		unsigned count = 0;
		unsigned writeIndex = 0;

		while (count < size) {
			if (control & (1 << bitPos)) {
				// length
				byte currentByte = rom[pointer++];
				unsigned length = currentByte >> 2;
				unsigned offset = 0;
				unsigned baseWriteIndex = writeIndex;

				count += length;
				offset = ((currentByte & 0x3) << 8) + rom[pointer++];

				for (unsigned i = 0; length != 0; ++i) {
					dest[writeIndex++] = dest[(baseWriteIndex - offset) + i];
					length--;
				}
			}
			else {
				dest[writeIndex++] = rom[pointer++];
				count++;
			}

			if (!bitPos) {
				control = rom[pointer++];
				bitPos = 7;
			}
			else {
				bitPos--;
			}
		}
	}

	return pointer - oldPointer;
}

WORD GFXRLECmp(BYTE* src, BYTE *dest, int size, int type)
{
	BYTE *odest = dest;

	if (type == 0) {
		byte maxCount = 0;
		byte bArr[0x100];
		byte control = 0xFF;
		ZeroMemory(bArr, 0x100);
		for (int i = 0; i < size >> 3; i++)
		{
			byte *config = config = dest;
			byte data;
			for (int j = 0; j<8; j++)
			{
				bArr[src[j]]++;
				if (bArr[src[j]] > maxCount)
				{
					maxCount = bArr[src[j]];
					data = src[j];
				}
			}
			maxCount = 0;
			ZeroMemory(bArr, 0x100);

			dest = dest + 2;
			for (int i = 0; i < 8; i++)
			{
				if (data == *src)
					control ^= 0x80 >> i;
				else
					*dest++ = *src;
				*src++;
			}
			*config++ = control;
			*config = data;
			control = 0xFF;
		}
	}
	else {
		// LZSS compression
		byte *control = dest;
		byte flag = 0x80;
		unsigned windowIndex = 0;
		unsigned uncompressedIndex = 0;
		matchPair match;

		if (size) {
			match = Find(src, windowIndex, uncompressedIndex, size);
#if 0
			FILE *out = fopen("out.txt", "w");

			for (unsigned test = 0; test < size; ++test) {
				if (test % 16 == 0) {
					fprintf(out, "\n");
				}
				fprintf(out, " %02x", src[test]);
			}

			fclose(out);
#endif
			while (uncompressedIndex < size) {

				if (flag == 0x80) {
					// write out control word
					control = dest++;
					*control = 0x0;
				}

				if (match.length > size) {
					// we reached end of data so make sure to force length.  Should this even happen?  Probably should be an assert.
					match.length = size;
				}

				if (match.length < MIN_LENGTH) {
					// write out just the symbol
					*dest++ = src[uncompressedIndex++];
				}
				else {
					BYTE tuple = 0;

					// write out compressed length (top 6b), offset/distance (bottom 10b)
					*control |= flag;
					// length + 2b of offset
					tuple = (match.length << 2) | (match.offset >> 8);
					*dest++ = tuple;
					tuple = (match.offset & 0xff);
					*dest++ = tuple;

					uncompressedIndex += match.length;
				}

				flag >>= 1;

				if (!flag) {
					// reset for the next control word
					flag = 0x80;
				}

				if (uncompressedIndex - windowIndex > WINDOW_SIZE) {
					// advance the window
					windowIndex = uncompressedIndex - WINDOW_SIZE;
				}

				if (uncompressedIndex < size) {
					match = Find(src, windowIndex, uncompressedIndex, size);
				}
			}
		}
	}

	return (WORD)(dest-odest);
}

int LayoutRLE(BYTE width, BYTE height, BYTE *sceneUsed, BYTE *src, BYTE *dst)
{
	bool cType;
	BYTE counter = 0;
	SHORT writeIndex = 3;
    for(int i=0; i<width*height;)
    {
        byte buf = src[i++];
        counter++;
        do
        {
            if (i >= width*height)
            {
                goto WRITE;
            }
            else
            {
                if (*sceneUsed < src[i])
                    *sceneUsed = src[i]+1;
                if (src[i] == buf)
                {
                    if (counter == 1)
                    {
                        cType = true;
                        counter |= 0x80;
						counter++;
                        i++;
                    }
                    else if (cType)
                    {
                        counter++;
                        i++;
                    }
                    else
                        goto WRITE;
                }
                else if (src[i] == buf+counter)
                {
                    if (counter == 1)
                    {
                        cType = false;
						counter++;
                        i++;
                    }
                    else if (!cType)
                    {
                        counter++;
                        i++;
                    }
                    else
                        goto WRITE;
                }
                else if (counter == 0x7E)
                {
                    goto WRITE;
                }
                else
                {
    WRITE:
                    dst[writeIndex++] = counter;
                    dst[writeIndex++] = buf;
                    counter = 0;
                }
            }
        } while(counter);
    }
	dst[0] = width;
	dst[1] = height;
	dst[2] = *sceneUsed;
	return writeIndex;
}
