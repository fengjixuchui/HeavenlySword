/*
 * Copyright (c) 2003-2005 Naughty Dog, Inc.
 * A Wholly Owned Subsidiary of Sony Computer Entertainment, Inc.
 * Use and distribution without consent strictly prohibited
 */

#include "icekbd.h"
#include <cstdio>

namespace Ice
{
	static CellKbInfo s_info; // Connection information buffer.
	CellKbData s_keyboardBuffer; // Keyboard data buffer.

	static const int kMaxKeyboards = 2;

/*	Removed in 0_9_0: cellKbSetArrangement
	static I32 s_arrangement[kMaxKeyboards];
 */
	static I32 s_readMode[kMaxKeyboards];

	static U8  s_oldStatus[kMaxKeyboards];
}

int Ice::KeyboardInit()
{
	int ret = cellKbInit(kMaxKeyboards);
	if (ret != 0) {
		::printf("ERROR: cellKbInit() error %d\n", ret);
		return -1;
	}

	for (int i = 0; i < kMaxKeyboards; i++) {
		s_oldStatus[i] = 0;

/*	Removed in 0_9_0: cellKbSetRepeat, cellKbSetArrangement
		if ((ret = cellKbSetRepeat (i, 30, 2)) != CELL_KB_OK) {
            ::printf("Error(%d) : cannot set key repeat, kno = %d\n", ret, i);
            cellKbEnd();
            return -1;
        }

        s_arrangement[i] = CELL_KB_ARRANGEMENT_101;
        if ((ret = cellKbSetArrangement(i, CELL_KB_ARRANGEMENT_101)) != CELL_KB_OK)
		{
            ::printf("Error(%d) : cannot set key arrangement, kno = %d\n", ret, i);
            cellKbEnd();
            return -1;
        }
*/
        if ((ret = cellKbSetReadMode(i, CELL_KB_RMODE_INPUTCHAR)) != CELL_KB_OK)
		{
            ::printf("Error(%d) : cannot set read mode, kno = %d\n", ret, i);
            cellKbEnd();
            return -1;
        }
        if ((ret = cellKbSetCodeType(i, CELL_KB_CODETYPE_ASCII)) != CELL_KB_OK)
		{
            ::printf("Error(%d) : cannot set code type, kno = %d\n", ret, i);
            cellKbEnd();
            return -1;
        }
    }

	return 0;
}

// return 0 if successful read, else non-zero if error in 1 or more keyboards
int Ice::KeyboardUpdate()
{
	I32 ret;

	if ((ret = cellKbGetInfo(&s_info)) != CELL_KB_OK)
	{
		::printf("Error(%d) : cellKbGetInfo\n", ret);
		return -1;
	}

	for (I64 i = 0; i < kMaxKeyboards; i++)
	{
		if (s_info.status[i] == 0) {
			continue;
		}

		/*
		 * keyboard i has been connected
		 */
/*	Removed in 0_9_0: cellKbSetLEDMode
		if (s_oldStatus[i] == 0)
		{
			/ *
			 * the previous data shows that keyboard was not yet
			 * connected, so it is a new connection
			 * /
			::printf("New keyboard %d is connected\n", (int)i);
			ret = cellKbSetLEDMode(i, CELL_KB_LED_MODE_AUTO1);
			if (ret != CELL_KB_OK) {
				::printf("Error(%d) : cannot set led mode, kno = %d\n", ret, (int)i);
			}
		}
*/

		/*
		 * Read keyboard data
		 */
		ret = cellKbRead(i, &s_keyboardBuffer);
		if (CELL_KB_OK != ret) {
			::printf("Error(%d) : cellKbRead, kno = %d\n", ret, (int)i);
			continue;
		}

		if (s_keyboardBuffer.len == 0)
			continue;

		for (I64 j = 0; j < s_keyboardBuffer.len; j++) {
/* Removed in 0_9_0: cellKbSetArrangement
			if (s_keyboardBuffer.keycode[j] ==(CELL_KEYC_F2 | CELL_KB_RAWDAT)) {
				if (s_arrangement[i] == CELL_KB_MAPPING_101) {
					::printf("usbkeybd%d : *106-key arrangement*\n", (int)i);
					s_arrangement[i] = CELL_KB_ARRANGEMENT_106;
				}
				else {
					::printf("usbkeybd%d : *101-key arrangement*\n", (int)i);
					s_arrangement[i] = CELL_KB_ARRANGEMENT_101;
				}
				ret = cellKbSetArrangement(i, s_arrangement[i]);
				if (ret != CELL_KB_OK) {
					::printf("Error(%d) : cannot set arrangement, kno = %d\n", ret, (int)i);
				}
			}
			if (s_keyboardBuffer.keycode[j] == (CELL_KEYC_KANA | CELL_KB_RAWDAT)) {
				switch (s_arrangement[i]) {
				case CELL_KB_MAPPING_106:
					s_arrangement[i] = CELL_KB_ARRANGEMENT_106_KANA;
					::printf("usbkeybd%d : *KANA MODE*\n", (int)i);
					break;
				case CELL_KB_MAPPING_106_KANA:
					s_arrangement[i] = CELL_KB_ARRANGEMENT_106;
					::printf("usbkeybd%d : *ALPHABET MODE*\n", (int)i);
					break;
				default:
					break;
				}
				ret = cellKbSetArrangement(i, s_arrangement[i]);
				if (ret != CELL_KB_OK) {
					::printf("Error(%d) : cannot set arrangement, kno = %d\n", ret, (int)i);
				}
			}
*/
			/*
			 * when the F1 key has been pressed, it switches between
			 * packet mode and character mode
			 */
			if (s_keyboardBuffer.keycode[j] == (CELL_KEYC_F1 | CELL_KB_RAWDAT)) {
				switch (s_readMode[i]) {
				case CELL_KB_RMODE_INPUTCHAR:
					s_readMode[i] = CELL_KB_RMODE_PACKET;
					::printf("usbkeybd%d : *PACKET MODE*\n", (int)i);
					break;
				case CELL_KB_RMODE_PACKET:
					s_readMode[i] = CELL_KB_RMODE_INPUTCHAR;
					::printf("usbkeybd%d : *INPUTCHAR MODE*\n", (int)i);
					break;
				default:
					break;
				}
				ret = cellKbSetReadMode(i, s_readMode[i]);
				if (ret != CELL_KB_OK) {
					::printf("Error(%d) : cannot set read mode, kno = %d\n", ret, (int)i);
				}
			}

			if (s_keyboardBuffer.len > 0)
			{
				::printf("Input :'");
				for (I64 iChar = 0; iChar < s_keyboardBuffer.len; iChar++) {
					U16 key = s_keyboardBuffer.keycode[iChar];

					if ((key & 0x00ff) == '\n') {
						::printf("\n");
					}
					else if (key <= 127) {
						::printf("%c", key);
					}
				}
				::printf("'\n");
			}

#if 0
			/*
			 * Ctrl+Alt+Delete ends the keyboard operations and
			 * unloads modules
			 */
			if ((s_keyboardBuffer.keycode[j] == (CELL_KEYC_DELETE | CELL_KB_RAWDAT)) &&
				(s_keyboardBuffer.mkey & (CELL_KB_MKEY_L_ALT | CELL_KB_MKEY_R_ALT)) &&
				(s_keyboardBuffer.mkey & (CELL_KB_MKEY_L_CTRL | CELL_KB_MKEY_R_CTRL)))
			{
				ret = cellKbEnd();
				if (ret == CELL_KB_OK)
					::printf("USB keyboard sample end, success\n");
				else
					::printf("Error(%d) : USB keyboard sample end\n", ret);
				return ret;
			}
#endif
		}
	}

	for (I64 i = 0; i < kMaxKeyboards; i++)
	{
		s_oldStatus[i] = s_info.status[i];
	}

	return 0;
}

int Ice::KeyboardShutdown()
{
	return 0;
}
