/*
 * Copyright (c) 2003-2005 Naughty Dog, Inc. 
 * A Wholly Owned Subsidiary of Sony Computer Entertainment, Inc.
 * Use and distribution without consent strictly prohibited
 */

#include <cell/pad.h>
#include "icepad.h" // Must come before sys/timer.h
#include <sys/timer.h> // sleep()


using namespace Ice;


namespace Ice
{
	enum
	{
		kDeadMin	= 0x54,
		kDeadMax	= 0xAB
	};
}


PadInfo Ice::g_padInfo[kMaxPadCount];


int Ice::PadInit(void)
{
	int ret = cellPadInit(kMaxPadCount);
	if (ret != 0) {
		printf("ERROR: cellPadInit() error %d\n", ret);
		return -1;
	}

	for (int i = 0; i < kMaxPadCount; i++)
		g_padInfo[i].m_on = false;

	// NOTE: For some reason we need to wait for the pad library to
	// initialize properly, or we can get a crash inside the graphics
	// driver called by Ice::Render::Initialize() (when the call to
	// PadInit is followed by Ice::Render::Initialize, of course).
	sys_timer_sleep(1);

	return 0;
}

int Ice::PadShutdown(void)
{
	int ret = cellPadEnd();
	if (ret != 0) {
		printf("ERROR: cellPadEnd() error %d\n", ret);
		return -1;
	}

	return 0;
}

// return 0 if successful read, else non-zero if error in 1 or more pads
int Ice::PadUpdate(void)
{
	CellPadInfo padInfo;
	int            ret;

	ret = cellPadGetInfo(&padInfo);
	if (ret != 0) {
		printf("ERROR: cellPadGetInfo() error %d\n", ret);
		return -1;
	}

	int error = 0;   // ok so far

	for (int i = 0; i < kMaxPadCount; i++) {
		if (padInfo.status[i] != 0) {
			CellPadData padData;

			// Read the pad data.
			// The USB PS controller requires a call to cellPadData(), but most other pads require a call to
			// cellPadRead().  This should get data from any type of controller.
			bool readUsingPadData = true;
			ret = cellPadGetData(i, &padData);

			if (ret != CELL_PAD_OK) {
				printf("ERROR: pad %d has cellPadGetData() error %d\n", i, ret);
				error = -1;
				continue; // read remaining pads
			}

			if (!g_padInfo[i].m_on) {
				if (padData.len == 0) {
					// note you shouldn't allow m_on to be set to 1 since you won't init all the other data
					printf("ERROR: pad %d has valid cellPadRead, but padData.len == 0\n", i);
					error = -1;
					continue; // read remaining pads; 
				}

				printf("Pad %d has been found! (vendor_id=0x%04x, product_id=0x%04x)\n", i, padInfo.vendor_id[i], padInfo.product_id[i]);
				g_padInfo[i].m_on = true; // note all the remaining data will be init'd
			}

			// This error occurs a lot -- clears KeyUp and KeyDown and returns previous results
			if (padData.len == 0) {
				g_padInfo[i].m_keyUp = 0;
				g_padInfo[i].m_keyDown = 0;
				return 0;
			}

//			printf("0=0x%02x, 1=0x%02x, 2=0x%02x, 3=0x%02x, ", padData.button[0], padData.button[1], padData.button[2], padData.button[3]);
//			printf("4=0x%02x, 5=0x%02x, 6=0x%02x, 7=0x%02x, ", padData.button[4], padData.button[5], padData.button[6], padData.button[7]);
//			printf("8=0x%02x\n", padData.button[8]);


			// set keys
			U16 padOldKeyPressed = g_padInfo[i].m_keyPressed;
			g_padInfo[i].m_keyPressed = 0;

			// --- Start of USB converter-specific stuff
			if (readUsingPadData) {
				// If the data was read with a call to callPadData() then the data will always come back in the same format.
				// The Sony USB PlayStation controller (comes with devkits starting with CEB-2050) and the
				// Logitech Dual Action (SCEA FC put these in some CEB-203x and CEB-204x boxes) both use this method.
				static U8 kButtonId[16] = { 2, 2, 2, 2, 3, 3, 3, 3, 3, 3, 3, 3, 2, 2, 2, 2 };
				const U8 *buttonId = &kButtonId[0];
				static U8 kButtonMask[16] = { 0x10 /* LUp */,  0x40 /* LDown */, 0x80 /* LLeft */, 0x20 /* LRight */,
											  0x10 /* RUp */,  0x40 /* RDown */, 0x80 /* RLeft */, 0x20 /* RRight */,
											  0x04 /* L1 */,   0x01 /* L2 */,    0x08 /* R1 */,    0x02 /* R2 */,
											  0x08 /* Start*/, 0x01 /* Select*/, 0x02 /* L3 */,    0x04 /* R3 */ };
				const U8 *buttonMask = kButtonMask;

				// Handle setting bits for the buttons
				for (int k = 0, keyMask = kPadLUp; k < 16; k++, keyMask <<= 1) {
					if (padData.button[buttonId[k]] & buttonMask[k])
						g_padInfo[i].m_keyPressed |= keyMask;
				}

				// Handle the two analog sticks
				g_padInfo[i].m_rawLeftX = padData.button[6];
				g_padInfo[i].m_rawLeftY = padData.button[7];
				g_padInfo[i].m_rawRightX = padData.button[4];
				g_padInfo[i].m_rawRightY = padData.button[5];
			}

			if ((g_padInfo[i].m_rawLeftX >= kDeadMin) && (g_padInfo[i].m_rawLeftX <= kDeadMax)) {
				g_padInfo[i].m_leftX = 0.0F;
			}
			else {
				g_padInfo[i].m_leftX = (float) g_padInfo[i].m_rawLeftX / 127.5F - 1.0F;
			}

			if ((g_padInfo[i].m_rawLeftY >= kDeadMin) && (g_padInfo[i].m_rawLeftY <= kDeadMax)) {
				g_padInfo[i].m_leftY = 0.0F;
			}
			else {
				g_padInfo[i].m_leftY = (float) g_padInfo[i].m_rawLeftY / 127.5F - 1.0F;
			}

			if ((g_padInfo[i].m_rawRightY >= kDeadMin) && (g_padInfo[i].m_rawRightY <= kDeadMax)) {
				g_padInfo[i].m_rightY = 0.0F;
			}
			else {
				g_padInfo[i].m_rightY = (float) g_padInfo[i].m_rawRightY / 127.5F - 1.0F;
			}

			if ((g_padInfo[i].m_rawRightX >= kDeadMin) && (g_padInfo[i].m_rawRightX <= kDeadMax)) {
				g_padInfo[i].m_rightX = 0.0F;
			}
			else {
				g_padInfo[i].m_rightX = (float) g_padInfo[i].m_rawRightX / 127.5F - 1.0F;
			}

			// set "hit"(down) or "release"(up) status
			// note game loop must be called inbetween calls to this code or else hit & release will be missed
			g_padInfo[i].m_keyDown =  g_padInfo[i].m_keyPressed & ~padOldKeyPressed;
			g_padInfo[i].m_keyUp   = ~g_padInfo[i].m_keyPressed &  padOldKeyPressed;
		}
		else {
			if (g_padInfo[i].m_on) {
				printf("Pad %d has been removed!\n", i);
				g_padInfo[i].m_on = false;
			}
		}
	}

	return error; // 0 if no errors
}
