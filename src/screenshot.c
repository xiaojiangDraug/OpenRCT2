/*****************************************************************************
 * Copyright (c) 2014 Ted John
 * OpenRCT2, an open source clone of Roller Coaster Tycoon 2.
 *
 * This file is part of OpenRCT2.
 *
 * OpenRCT2 is free software: you can redistribute it and/or modify
 * it under the terms of the GNU General Public License as published by
 * the Free Software Foundation, either version 3 of the License, or
 * (at your option) any later version.
 
 * This program is distributed in the hope that it will be useful,
 * but WITHOUT ANY WARRANTY; without even the implied warranty of
 * MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.  See the
 * GNU General Public License for more details.
 
 * You should have received a copy of the GNU General Public License
 * along with this program.  If not, see <http://www.gnu.org/licenses/>.
 *****************************************************************************/

#include <windows.h>
#include "addresses.h"
#include "gfx.h"
#include "rct2.h"
#include "screenshot.h"
#include "strings.h"
#include "window_error.h"

/**
 * 
 *  rct2: 0x006E3AEC
 */
void screenshot_check()
{
	int screenshotIndex;

	if (RCT2_GLOBAL(RCT2_ADDRESS_SCREENSHOT_COUNTDOWN, uint8) != 0) {
		RCT2_GLOBAL(RCT2_ADDRESS_SCREENSHOT_COUNTDOWN, uint8)--;
		if (RCT2_GLOBAL(RCT2_ADDRESS_SCREENSHOT_COUNTDOWN, uint8) == 0) {
			RCT2_CALLPROC_EBPSAFE(0x00684218);
			screenshotIndex = screenshot_dump();
			RCT2_GLOBAL(0x013CE952, uint16) = STR_SCR_BMP;
			RCT2_GLOBAL(0x013CE952 + 2, uint16) = screenshotIndex;
			RCT2_GLOBAL(0x009A8C29, uint8) |= 1;

			window_error_open(screenshotIndex == -1 ? STR_SCREENSHOT_FAILED : STR_SCREENSHOT_SAVED_AS, -1);
			RCT2_GLOBAL(0x009A8C29, uint8) &= ~1;
			RCT2_CALLPROC_EBPSAFE(0x006843DC);
		}
	}
}

static int screenshot_get_next_path(char *path)
{
	char *placeHolder;

	strcpy(path, RCT2_ADDRESS(RCT2_ADDRESS_APP_PATH_SLASH, char));
	placeHolder = path + strlen(path);

	int i;
	for (i = 1; i < 1000; i++) {
		RCT2_GLOBAL(0x013CE952, uint16) = i;
		format_string(placeHolder, STR_SCR_BMP, 0x013CE952);

		if (GetFileAttributes(path) == INVALID_FILE_ATTRIBUTES && GetLastError() == ERROR_FILE_NOT_FOUND)
			return i;
	}

	return -1;
}

// Bitmap header structs, for cross platform purposes
typedef struct {
	uint16 bfType;
	uint32 bfSize;
	uint16 bfReserved1;
	uint16 bfReserved2;
	uint32 bfOffBits;
} BitmapFileHeader;

typedef struct {
	uint32 biSize;
	sint32 biWidth;
	sint32 biHeight;
	uint16 biPlanes;
	uint16 biBitCount;
	uint32 biCompression;
	uint32 biSizeImage;
	sint32 biXPelsPerMeter;
	sint32 biYPelsPerMeter;
	uint32 biClrUsed;
	uint32 biClrImportant;
} BitmapInfoHeader;

/**
 * 
 *  rct2: 0x00683D20
 */
int screenshot_dump()
{
	BitmapFileHeader header;
	BitmapInfoHeader info;

	int i, x, y, index, width, height, stride;
	char *buffer, path[MAX_PATH], *row, *dst;
	HFILE hFile;
	DWORD bytesWritten;

	// Get a free screenshot path
	if ((index = screenshot_get_next_path(path)) == -1)
		return -1;

	// Open file for writing
	hFile = CreateFile(path, GENERIC_WRITE, NULL, NULL, CREATE_ALWAYS, FILE_ATTRIBUTE_NORMAL, NULL);
	if (hFile == INVALID_HANDLE_VALUE)
		return -1;

	// Allocate buffer
	buffer = malloc(0xFFFF);
	if (buffer == NULL) {
		CloseHandle(hFile);
		return -1;
	}

	// Get image size
	width = RCT2_GLOBAL(RCT2_ADDRESS_SCREEN_WIDTH, uint16);
	height = RCT2_GLOBAL(RCT2_ADDRESS_SCREEN_HEIGHT, uint16);
	stride = (width + 3) & 0xFFFFFFFC;

	// File header
	memset(&header, 0, sizeof(header));
	header.bfType = 0x4D42;
	header.bfSize = height * stride + 1038;
	header.bfOffBits = 1038;

	WriteFile(hFile, &header, sizeof(header), &bytesWritten, NULL);
	if (bytesWritten != sizeof(header)) {
		CloseHandle(hFile);
		free(buffer);
	}

	// Info header
	memset(&info, 0, sizeof(info));
	info.biSize = sizeof(info);
	info.biWidth = width;
	info.biHeight = height;
	info.biPlanes = 1;
	info.biBitCount = 8;
	info.biXPelsPerMeter = 2520;
	info.biYPelsPerMeter = 2520;
	info.biClrUsed = 246;

	WriteFile(hFile, &info, sizeof(info), &bytesWritten, NULL);
	if (bytesWritten != sizeof(info)) {
		CloseHandle(hFile);
		free(buffer);
	}

	// Palette
	memset(buffer, 0, 246 * 4);
	for (i = 0; i < 246; i++) {
		buffer[i * 4 + 0] = RCT2_ADDRESS(0x01424680, uint8)[i * 4 + 0];
		buffer[i * 4 + 1] = RCT2_ADDRESS(0x01424680, uint8)[i * 4 + 1];
		buffer[i * 4 + 2] = RCT2_ADDRESS(0x01424680, uint8)[i * 4 + 2];
	}

	WriteFile(hFile, buffer, 246 * 4, &bytesWritten, NULL);
	if (bytesWritten != 246 * 4) {
		CloseHandle(hFile);
		free(buffer);
	}

	// Image, save upside down
	rct_drawpixelinfo *dpi = RCT2_ADDRESS(RCT2_ADDRESS_SCREEN_DPI, rct_drawpixelinfo);
	for (y = dpi->height - 1; y >= 0; y--) {
		row = dpi->bits + y * (dpi->width + dpi->pitch);

		memset(buffer, 0, stride);
		memcpy(buffer, row, dpi->width);

		WriteFile(hFile, buffer, stride, &bytesWritten, NULL);
		if (bytesWritten != stride) {
			CloseHandle(hFile);
			free(buffer);
		}
	}

	CloseHandle(hFile);
	free(buffer);

	return index;
}