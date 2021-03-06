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

#ifndef _PEEP_H_
#define _PEEP_H_

#include "rct2.h"

#define PEEP_MAX_THOUGHTS 5

enum PEEP_TYPE {
	PEEP_TYPE_GUEST,
	PEEP_TYPE_STAFF
};

enum {
	PEEP_FLAGS_TRACKING = 8
};

enum PEEP_THOUGHT_TYPE {
	PEEP_THOUGHT_TYPE_NONE = 255
};

typedef struct {
	uint8 type;
	uint8 item;
	uint8 pad_3;
	uint8 pad_4;
} rct_peep_thought;

typedef struct {
	uint8 sprite_identifier;		// 0x00
	uint8 pad_01;
	uint16 pad_02;
	uint16 next;					// 0x04
	uint16 pad_06;
	uint8 var_08;
	uint8 pad_09;
	sint16 var_0A;
	uint16 var_0C;
	uint8 var_0E[0x14];
	uint16 name_string_idx;			// 0x22
	uint16 next_x;					// 0x24
	uint16 next_y;					// 0x26
	uint16 next_z;					// 0x28
	uint8 var_2A;
	uint8 state;					// 0x2B
	uint8 pad_2C;
	uint8 sprite_type;				// 0x2D
	uint8 type;						// 0x2E
	uint8 var_2F;
	uint8 var_30;
	uint8 var_31;
	uint8 pad_32[0x08];
	uint8 happiness;				// 0x3A
	uint8 pad_03B[0x2D];
	uint8 current_ride;				// 0x68
	uint8 var_6A;
	uint8 var_6B;
	uint8 pad_6C[0x30];
	uint32 id;						// 0x9C
	uint8 pad_A0[0x10];
	rct_peep_thought thoughts[PEEP_MAX_THOUGHTS];	// 0xB0
	uint16 pad_C4;
	uint8 var_C6;
	uint8 pad_C7;
	uint32 var_C8;
	uint8 pad_CC[0x2A];
	uint8 balloon_colour;			// 0xF6
	uint8 umbrella_colour;			// 0xF7
	uint8 hat_colour;				// 0xF8
} rct_peep;

int peep_get_staff_count();
void peep_update_all();

#endif
