/*
 * Copyright (c) 2003-2005 Naughty Dog, Inc. 
 * A Wholly Owned Subsidiary of Sony Computer Entertainment, Inc.
 * Use and distribution without consent strictly prohibited
 */

#ifndef ICE_KEYBOARD_H
#define ICE_KEYBOARD_H

#include "icebase.h"
#include <cell/keyboard.h>

namespace Ice
{
	extern CellKbData g_keyboardBuffer; // Keyboard data buffer.

	int KeyboardInit();
	int KeyboardUpdate();
	int KeyboardShutdown();
}


#endif // ICE_KEYBOARD_H

