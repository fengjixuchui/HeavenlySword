///////////////////////////////////////////////////////////////////////  
//	SpeedTreeRT runtime configuration #defines
//
//	(c) 2003 IDV, Inc.
//
//	*** INTERACTIVE DATA VISUALIZATION (IDV) PROPRIETARY INFORMATION ***
//
//	This software is supplied under the terms of a license agreement or
//	nondisclosure agreement with Interactive Data Visualization and may
//	not be copied or disclosed except in accordance with the terms of
//	that agreement.
//
//      Copyright (c) 2001-2003 IDV, Inc.
//      All Rights Reserved.
//
//		IDV, Inc.
//		1233 Washington St. Suite 610
//		Columbia, SC 29201
//		Voice: (803) 799-1699
//		Fax:   (803) 931-0320
//		Web:   http://www.idvinc.com


#ifndef _SPEEDTREECONFIG_H_
#define _SPEEDTREECONFIG_H_




// texture coordinates (enable this define for DirectX-based engines)
#define WRAPPER_FLIP_T_TEXCOORD

// loading from STF or clones/instances? (enable ONE of the two below)
#define WRAPPER_FOREST_FROM_STF
//#define WRAPPER_FOREST_FROM_INSTANCES

#if defined WRAPPER_FOREST_FROM_STF && defined WRAPPER_FOREST_FROM_INSTANCES
	#error Please define exactly one loading mechanism
#endif

// billboard modes 
#define WRAPPER_BILLBOARD_MODE
//#define WRAPPER_RENDER_HORIZONTAL_BILLBOARD

//#define SPEEDTREE_COLLECT_STATS


#endif // end of _SPEEDTREECONFIG_H_
