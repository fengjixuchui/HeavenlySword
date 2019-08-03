/*
 * Copyright (c) 2005 Naughty Dog, Inc.
 * A Wholly Owned Subsidiary of Sony Computer Entertainment, Inc.
 * Use and distribution without consent strictly prohibited
 *
 * The purpose of this file is to be a container for all decompression
 * functions.
 */

#include "icemeshinternal.h"

#ifndef __SPU__
// Global constant data table.
// This is currently used by the custom compress/decompress routines and the X11Y11Z10 routines.
MeshGlobalConstData g_meshGlobalConstData = {
	{ 0xFFE00000, 0xFFE00000, 0xFFC00000, 0x00000000 },			// m_x11y11z10_decomp_mask
	{ 0x00000005, 0x00000002, 0x00000000, 0x00000000 },			// m_x11y11z10_decomp_shifts
	{ 2048.0f / 2047.0f, 2048.0f / 2047.0f, 1024.0f / 1023.0f, 0.0f },	// m_x11y11z10_decomp_scale
	{ 1.0f / 2047.0f, 1.0f / 2047.0f, 1.0f / 1023.0f, 1.0f },		// m_x11y11z10_decomp_bias
	{ -5, -2, 0, 0 },							// m_x11y11z10_comp_shifts
	{ 0x3F7FE000, 0x3F7FE000, 0x3F7FC000, 0x00000000 },// m_x11y11z10_comp_scale
	{ 0xBA000000, 0xBA000000, 0xBA800000, 0x00000000 },// m_x11y11z10_comp_bias
};
#endif

