#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#if !defined( PLATFORM_PS3 )
#	include "basetypes_pc.h"
#	include "stdarg.h"
#else
#	include <stdint.h>
#endif

#include "heresy_hardware.h"
#include "heresy_disassemble.h"

void myprintf( const char* fmt, ... )
{
	char buffer[10*1024];
	va_list ap;
	va_start( ap, fmt );
	vsprintf( buffer, fmt, ap );
	va_end(ap);
	fprintf( g_PushBufferFH, buffer );
}
#define DUMP_COMMAND( x )	myprintf x

//-=-=-=-=-=-=-=-=-=-=-=-
// New info by looking at ice (I suspected this, so I'm feeling fucking clever at the mo :-) )
// the hardware moves N payload bytes to an register incrementing each time... so rather
// than setting each register with a command you can set a continous array of register with a
// single push buffer command
//-=-=-=-=-=-=-=-=-=-=-=-

//-=-=-=-=-=-=-=-=-=-=-=-
// Also worked out the top nibble 0x4 value, its NOT increment. Allowing  you to repeat commands
// to the same register with one command. Used by Flush Texture Cache and IndexArrayParams
//-=-=-=-=-=-=-=-=-=-=-=-

// used to check if the payload is the 'usual' size, ICE and libgcm have differences (set vertex shader constant) that this would have discovered
#define CHECK_PAYLOAD( n )												\
{																		\
	unsigned int iPayload_chk = (regSelect & 0x0FFF0000) >> 16;		\
	if( iPayload_chk != (n*0x4) )										\
	{																	\
		DUMP_COMMAND( ("-----------------------\n ") );					\
		DUMP_COMMAND( ("	Strange Payload!!\n ") );					\
		DUMP_COMMAND( ("	Dumping as unknown as well so we can have a look see\n ") );	\
		DecodeSemiUnknownCommand( pCommand );											\
		DUMP_COMMAND( ("-----------------------\n " ) );								\
	}																	\
}																	\


/* PUSH BUFFER COMMAND

NOP = 0x0
JUMP = (OFFSET & ~0xF)
CALL = (OFFSET & ~0xF) | 0x2
RETURN = 0x00020000
*/

//! A NOP push buffer command does nothing obviously
void DecodeNOP( uint32_t reg,uint32_t* val )
{
	DUMP_COMMAND( ("NOP\n") );
}


//! A RSX_NOOP push buffer command does nothing obviously
//! this is used for debug markers though
void DecodeNOOP( uint32_t reg,uint32_t* val )
{
	DUMP_COMMAND( ("\tRSX_NOOP 0x%x\n", reg, *val ) );
}

void DecodeSemiUnknownCommand( uint32_t reg,uint32_t* val )
{
	DUMP_COMMAND( ("\tSemi Unknown Command - Register 0x%x =  0x%x\n", reg, *val ) );
}


void DecodeUnknownCommand( uint32_t reg,uint32_t* val )
{
	DUMP_COMMAND( ("\tUnknown Command - Register 0x%x =  0x%x\n", reg, *val ) );
}

void DecodeJump( uint32_t command )
{
	// JUMP = (OFFSET | 0x20000000)
	uint32_t iOffset = command & ~0x20000000;

	DUMP_COMMAND( ("\tJump 0x%x\n", iOffset ) );
}

void DecodeReturn( uint32_t command  )
{
	// RETURN = 0x00020000
	// NOTE: Not sure how this gets selected yet As this 'seems' to be
	// the same as some commands... Needs more investigation...
	DUMP_COMMAND( ("\tRET \n") );
}

void DecodeCall( uint32_t command )
{
	// CALL = (OFFSET & ~0xF) | 0x2
	uint32_t iOffset = command & ~0xf;

	DUMP_COMMAND( ("\tCALL 0x%x\n", iOffset ) );
}


//! SET_REFERENCE
//! this a SetReference command
//! Its sets the reference register in RSX to a the specified value
void DecodeSetReference( uint32_t reg,uint32_t* val )
{
	// |-------------|
	// | Reference   |
	// |-------------|
	DUMP_COMMAND( ("\tSetReference 0x%x\n", val ) );
}
//! DMA_SELECT
//! this a DMASelect command, its tells the next DMA Label command which
//! GPU index register you wish to wait on etc.
void DecodeDMASelect( uint32_t reg,uint32_t* val )
{
	// |-------------|
	// | Index << 4  |
	// |-------------|
	uint32_t iIndex = *val >> 4;
	DUMP_COMMAND( ("\tDMA Select 0x%x\n", iIndex ) );

}
//! WAIT_FOR_LABEL
//! Use a DMASelect to select the index and then issue this to 
//! cause the GPU to wait until that index register gets iRef put into it
void DecodeWaitForLabel( uint32_t reg,uint32_t* val )
{
	//|-------------|
	//| Reference   |
	//|-------------|
	DUMP_COMMAND( ("\tWait For Label 0x%x\n", val ) );
}

//! WRITE_COMMAND_LABEL
//! Use use DMASelect to select the index and then issue this to
//! cause the GPU to write the reference into the specified index when
//! RSX has read this command
void DecodeWriteCommandLabel( uint32_t reg,uint32_t* val )
{
	// |-------------|
	// | Reference   |
	// |-------------|
	DUMP_COMMAND( ("\tWrite Command Label 0x%x\n", val ) );
}

//! CLEAR
//! Issue a memory clear using the current clear parameters
void DecodeClearCommand( uint32_t reg,uint32_t* val )
{
	// |-------------|
	// | ?????	     |
	// |-------------|
	DUMP_COMMAND( ("\tClear 0x%x\n", val ) );
}

//! RENDER_TARGET_LOC_1
//! Set which memory pool (MAIN or LOCAL) Render Target 1 is in
void DecodeRenderTargetLoc1( uint32_t reg,uint32_t* val )
{
	// LOC = 0x0 == LOCAL RAM
	//       0x1 == MAIN_RAM
	// |---------------|
	// | 0xFEED | LOC  |
	// |---------------|
	uint32_t iLoc = (*val & 0x0000FFFF);
	DUMP_COMMAND( ("\tRender Target 1 in %s \n", (iLoc == HRL_LOCAL_GDDR) ? "Local(GDDR) RAM" : "Main(XDDR) RAM" ) );
}

//! RENDER_TARGET_LOC_0
//! Set which memory pool (MAIN or LOCAL) Render Target 0 is in
void DecodeRenderTargetLoc0( uint32_t reg,uint32_t* val )
{
	// LOC = 0x0 == LOCAL RAM
	//       0x1 == MAIN_RAM
	// |---------------|
	// | 0xFEED | LOC  |
	// |---------------|
	uint32_t iLoc = (*val & 0x0000FFFF);
	DUMP_COMMAND( ("\tRender Target 0 in %s\n", (iLoc == HRL_LOCAL_GDDR) ? "Local(GDDR) RAM" : "Main(XDDR) RAM" ) );
}

//! RENDER_TARGET_LOC_2
//! Set which memory pool (MAIN or LOCAL) Render Target 0 is in
void DecodeRenderTargetLoc2( uint32_t reg,uint32_t* val )
{
	// LOC = 0x0 == LOCAL RAM
	//       0x1 == MAIN_RAM
	// |---------------|
	// | 0xFEED | LOC  |
	// |---------------|
	uint32_t iLoc = (*val & 0x0000FFFF);
	DUMP_COMMAND( ("\tRender Target 2 in %s\n", (iLoc == HRL_LOCAL_GDDR) ? "Local(GDDR) RAM" : "Main(XDDR) RAM" ) );

}
//! RENDER_TARGET_LOC_3
//! Set which memory pool (MAIN or LOCAL) Render Target 0 is in
void DecodeRenderTargetLoc3( uint32_t reg,uint32_t* val )
{
	// LOC = 0x0 == LOCAL RAM
	//       0x1 == MAIN_RAM
	// |---------------|
	// | 0xFEED | LOC  |
	// |---------------|
	uint32_t iLoc = (*val & 0x0000FFFF);
	DUMP_COMMAND( ("\tRender Target 3 in %s\n", (iLoc == HRL_LOCAL_GDDR) ? "Local(GDDR) RAM" : "Main(XDDR) RAM" ) );
}

//! RENDER_TARGET_RECT_HORIZ
//! set the render target rectangle, x and width.
//! Each is a 16 bit integer
void DecodeRenderTargetRectHoriz( uint32_t reg,uint32_t* val )
{
	// |---------------|
	// | width<<16 | x |
	// |---------------|
	uint16_t iWidth		= (*val & 0xFFFF0000) >> 16;
	uint16_t iX			= (*val & 0x0000FFFF);
	DUMP_COMMAND( ("\tRender Target Rect Horiz X - %d, Width - %d\n", iX, iWidth ) );

}
//! RENDER_TARGET_RECT_VERT
//! set the render target rectangle, y and height.
//! Each is a 16 bit integer
void DecodeRenderTargetRectVert( uint32_t reg,uint32_t* val )
{
	// |---------------|
	// | height<<16 | y|
	// |---------------|
	uint16_t iHeight	= (*val & 0xFFFF0000) >> 16;
	uint16_t iY			= (*val & 0x0000FFFF);
	DUMP_COMMAND( ("\tRender Target Rect Vert Y - %d, Height - %d\n", iY, iHeight ) );
}

//! RENDER_TARGET_FORMAT
//! Set most of the render target format data
//! the paramater is a tight packed register
void DecodeRenderTargetFormat( uint32_t reg,uint32_t* val )
{
	// |---------------|
	// | Format		   |
	// |---------------|
	// |----------------|----------------|----------------|----------------|
	// | 8 bit		    | 8 bit          | 4 bit  | 4 bit | 3bit | 5 bit   |
	// |----------------|----------------|--------|-------|------|---------|
	// | Log2(			|Log2(		     | MSAA   | Flags |Depth | Colour  |
	// |    NextPow(	|   NextPow(	 |--------|-------|----------------|
	// |       height ))|      width ))  |0 = CNTR|    S P|z16 =1| COLOUR  |
	// |                |                |1 = DIAG|    w i|z24 =2|	FMT    |
	// |                |                |2 = SC4 |    i t|      |	       |
	// |                |                |3 = SR4 |    z c|      |	       |
	// |                |                |        |    l h|      |	       |
	// |----------------|----------------|----------------|----------------|
	uint8_t iLog2Height	= (*val & 0xFF000000) >> 24;
	uint8_t iLog2Width	= (*val & 0x00FF0000) >> 16;
	uint8_t iMSAA		= (*val & 0x0000F000) >> 12;
	uint8_t iFlags		= (*val & 0x00000F00) >> 8;
	uint8_t iDepthFmt	= (*val & 0x000000E0) >> 5;
	uint8_t iColourFmt	= (*val & 0x0000001F) >> 0;

	uint32_t iHeight	= 0x1 << iLog2Height;
	uint32_t iWidth		= 0x1 << iLog2Width;

	DUMP_COMMAND( ("\tRender Target Format :  Width - %d, Height - %d\n", iWidth, iHeight ) );
	DUMP_COMMAND( ("\t					 :  Log2Width - %d, Log2Height - %d\n", iLog2Height, iLog2Width ) );
	DUMP_COMMAND( ("\t					 :  Depth Buffer Format : ") ); 
	switch( iDepthFmt )
	{
	case HDF_Z16:
		DUMP_COMMAND( ("\t16 Bit Depth \n") ); break;
	case HDF_Z24S8:
		DUMP_COMMAND( ("\t24 Bit Depth 8 bit stencil\n") ); break;
	default:
		DUMP_COMMAND( ("\tUnknown Depth Buffer format 0x%x\n", iDepthFmt) ); break;
	}

	DUMP_COMMAND( ("\t					 :  Colour Buffer Format : ") ); 
	switch( iColourFmt )
	{
	case HRTF_X1R5G5B5_Z1R5G5B5:
		DUMP_COMMAND( ("\tHRTF_A8B8G8R8X1R5G5B5_Z1R5G5B5 Colour\n") ); break;
	case HRTF_X1R5G5B5_O1R5G5B5:
		DUMP_COMMAND( ("\tX1R5G5B5_O1R5G5B5 Colour\n") ); break;
	case HRTF_R5G6B5:
		DUMP_COMMAND( ("\tR5G6B5 Colour\n") ); break;
	case HRTF_X8R8G8B8_Z8R8G8B8:
		DUMP_COMMAND( ("\tX8R8G8B8_Z8R8G8B8 Colour\n") ); break;
	case HRTF_X8R8G8B8_O8R8G8B8:
		DUMP_COMMAND( ("\tX8R8G8B8_O8R8G8B8 Colour\n") ); break;
	case HRTF_A8R8G8B8:
		DUMP_COMMAND( ("\tA8R8G8B8 Colour\n") ); break;
	case HRTF_B8:
		DUMP_COMMAND( ("\tB8 Colour\n") ); break;
	case HRTF_G8B8:
		DUMP_COMMAND( ("\tG8B8 Colour\n") ); break;
	case HRTF_F_W16Z16Y16X16:
		DUMP_COMMAND( ("\tF_W16Z16Y16X16 Colour\n") ); break;
	case HRTF_F_W32Z32Y32X32:
		DUMP_COMMAND( ("\tF_W32Z32Y32X32 Colour\n") ); break;
	case HRTF_F_X32:
		DUMP_COMMAND( ("\tF_X32 Colour\n") ); break;
	case HRTF_X8B8G8R8_Z8B8G8R8:
		DUMP_COMMAND( ("\tX8B8G8R8_Z8B8G8R8 Colour\n") ); break;
	case HRTF_X8B8G8R8_O8B8G8R8:
		DUMP_COMMAND( ("\tX8B8G8R8_O8B8G8R8 Colour\n") ); break;
	case HRTF_A8B8G8R8:
		DUMP_COMMAND( ("\tA8B8G8R8 Colour\n") ); break;
	default:
		DUMP_COMMAND( ("\tUnknown Colour Buffer format 0x%x\n", iColourFmt) ); break;
	}

	DUMP_COMMAND( ("\t					 :  MSAA Mode : ") );
	switch( iMSAA )
	{
	case HRTM_1_SAMPLE_CENTER:
		DUMP_COMMAND( ("\t1 Sample Centered\n") ); break;
	case HRTM_2_SAMPLE_DIAGONAL:
		DUMP_COMMAND( ("\t2 Sample Diagonal\n") ); break;
	case HRTM_4_SAMPLE_SQUARE:
		DUMP_COMMAND( ("\t4 Sample Square\n") ); break;
	case HRTM_4_SAMPLE_SQUARE_ROTATED:
		DUMP_COMMAND( ("\t4 Sample Square Rotated\n") ); break;
	default:
		DUMP_COMMAND( ("\tUnknown MSAA Mode\n") ); break;
	};
	if( iFlags & HRTF_PITCH )
	{
		DUMP_COMMAND( ("\t					 :  Render target has pitch\n") );
	}
	if( iFlags & HRTF_SWIZZLE )
	{
		DUMP_COMMAND( ("\t					 :  Render target is swizzled\n") );
	}
}

//! RENDER_TARGET0_PITCH
//! Set the pitch of render target 0
void DecodeRenderTarget0Pitch( uint32_t reg,uint32_t* val )
{
	// |----------|
	// |RT0 Pitch |
	// |----------|
	uint32_t iRT0Pitch	= *val;
	DUMP_COMMAND( ("\tRender target 0 Pitch %d\n", iRT0Pitch) );
}
//! RENDER_TARGET0_OFFSET
//! Set the offeset of render target 0
void DecodeRenderTarget0Offset( uint32_t reg,uint32_t* val )
{
	// |-----------|
	// |RT0 Offset |
	// |-----------|
	uint32_t iRT0Offs	= *val;
	DUMP_COMMAND( ("\tRender target 0 Offset 0x%x\n", iRT0Offs) );
}
//! DEPTH_STENCIL_OFFSET
//! Set the offset of the depth stencil target
void DecodeDepthStencilOffset( uint32_t reg,uint32_t* val )
{
	// |-------------|
	// |Depth Offset |
	// |-------------|
	uint32_t iDepthOffs	= *val;
	DUMP_COMMAND( ("\tDepth Stencil Offset 0x%x\n", iDepthOffs) );
}
//! RENDER_TARGET1_OFFSET
//! Set the offeset of render target 1
void DecodeRenderTarget1Offset( uint32_t reg,uint32_t* val )
{
	// |-----------|
	// |RT1 Offset |
	// |-----------|
	uint32_t iRT1Offs	= *val;
	DUMP_COMMAND( ("\tRender target 1 Offset 0x%x\n", iRT1Offs) );
}
//! RENDER_TARGET1_PITCH
//! Set the pitch of render target 1
void DecodeRenderTarget1Pitch( uint32_t reg,uint32_t* val )
{
	// |----------|
	// |RT1 Pitch |
	// |----------|
	uint32_t iRT1Pitch	= *val;
	DUMP_COMMAND( ("\tRender target 1 Pitch %d\n", iRT1Pitch) );
}

//! RENDER_TARGET_SELECT
//! Which colour render targets are going to be used
void DecodeRenderTargetSelect( uint32_t reg,uint32_t* val )
{
	// |-------------|
	// | HRT_SELECT  |
	// |-------------|
	uint32_t iSelect = *val;
	if(iSelect == 0x2)
	{
		DUMP_COMMAND( ("\tRender Target Select using non MRT 0 and 1 output??\n") );
		return;
	}
	if( iSelect & HRTS_MRT_ENABLE)
	{
		DUMP_COMMAND( ("\tRender Target Select: Render Targets %s %s %s %s enabled\n", 
					((iSelect & HTRS_TARGET_0) ? "0" :""),
					((iSelect & HTRS_TARGET_1) ? "1" :""),
					((iSelect & HTRS_TARGET_2) ? "2" :""),
					((iSelect & HTRS_TARGET_3) ? "3" :"") )
			);
	} else
	{
		if( iSelect == HTRS_TARGET_0)
		{
			DUMP_COMMAND( ("\tRender Target Select RT 0 only\n") );
		}
	}
}
//! DEPTH_STENCIL_PITCH
//! The pitch of the depth stencil buffer
void DecodeDepthStencilPitch( uint32_t reg,uint32_t* val )
{
	// |-------------|
	// | Pitch       |
	// |-------------|
	uint32_t iPitch = *val;
	DUMP_COMMAND( ("\tDepth Stencil Pitch %d \n", iPitch ) );
}

//! INVALIDATE_ZCULL
//! causes the zcull data to be discarded
//! Only know value for iParam is 0
void DecodeInvalidateZCull( uint32_t reg,uint32_t* val )
{
	// |-------------|
	// | Unknown =0x0|
	// |-------------|
	uint32_t iParam = *val;
	if( iParam == 0x0 )
	{
		DUMP_COMMAND( ("\tInvalidateZCull\n") );
	} else
	{
		DUMP_COMMAND( ("\tInvalidateZCull 0x%x\n",iParam) );
	}
}

//! RENDER_TARGET2_PITCH
//! Set the pitch of render target 2
void DecodeRenderTarget2Pitch( uint32_t reg,uint32_t* val )
{
	// |----------|
	// |RT2 Pitch |
	// |----------|
	uint32_t iRT2Pitch	= *val;
	DUMP_COMMAND( ("\tRender target 2 Pitch %d\n", iRT2Pitch) );
}
//! RENDER_TARGET3_PITCH
//! Set the pitch of render target 3
void DecodeRenderTarget3Pitch( uint32_t reg,uint32_t* val )
{
	// |----------|
	// |RT3 Pitch |
	// |----------|
	uint32_t iRT3Pitch	= *val;
	DUMP_COMMAND( ("\tRender target 3 Pitch %d\n", iRT3Pitch) );
}

//! RENDER_TARGET2_OFFSET
//! Set the offeset of render target 2
void DecodeRenderTarget2Offset( uint32_t reg,uint32_t* val )
{
	// |-----------|
	// |RT2 Offset |
	// |-----------|
	uint32_t iRT2Offs	= *val;
	DUMP_COMMAND( ("\tRender target 2 Offset 0x%x\n", iRT2Offs) );
}
//! RENDER_TARGET3_OFFSET
//! Set the offeset of render target 3
void DecodeRenderTarget3Offset( uint32_t reg,uint32_t* val )
{
	// |-----------|
	// |RT3 Offset |
	// |-----------|
	uint32_t iRT3Offs	= *val;
	DUMP_COMMAND( ("\tRender target 3 Offset 0x%x\n", iRT3Offs) );
}

//! RENDER_TARGET_POS_OFFSET
//! Set the position offset of the render targets
//! this appears to mirror the data in RENDER_TARGET_RECT but presumable
//! this is some reason for it being repeated and seperate
void DecodeRenderTargetPosOffset( uint32_t reg,uint32_t* val )
{
	// |-------------|
	// | y<<16|x	 |
	// |-------------|
	uint16_t iY = (*val & 0xFFFF0000) >> 16;
	uint16_t iX = (*val & 0x0000FFFF);
	DUMP_COMMAND( ("\tRenderTargetPosOffset x %d y %d\n", iX, iY) );

}

//! DITHER_ENABLE
void DecodeDitherEnable( uint32_t reg,uint32_t* val )
{
	// |-------------|
	// | enable	     |
	// |-------------|
	uint32_t iEnable = *val;
	DUMP_COMMAND( ("\tDitherEnable %x\n", iEnable) );
}

//! ALPHA_TEST_ENABLE
void DecodeAlphaTestEnable( uint32_t reg,uint32_t* val )
{
	// |-------------|
	// | enable	   |
	// |-------------|
	uint32_t iEnable = *val;
	DUMP_COMMAND( ("\tAlphaTestEnable %x\n", iEnable) );
}
//! ALPHA_FUNC
void DecodeAlphaFunc( uint32_t reg,uint32_t* val )
{
	// |-------------|
	// | Alpha Func  |
	// |-------------|
	uint32_t iFunc = *val;
	DUMP_COMMAND( ("\tAlphaFunc : ") );
	switch( iFunc )
	{
	case HF_NEVER:
		DUMP_COMMAND( ("NEVER\n") ); break;
	case HF_LESS:
		DUMP_COMMAND( ("LESS\n") ); break;
	case HF_EQUAL:
		DUMP_COMMAND( ("EQUAL\n") ); break;
	case HF_LEQUAL:
		DUMP_COMMAND( ("LEQUAL\n") ); break;
	case HF_GREATER:
		DUMP_COMMAND( ("GREATER\n") ); break;
	case HF_NOTEQUAL:
		DUMP_COMMAND( ("NOTEQUAL\n") ); break;
	case HF_GEQUAL:
		DUMP_COMMAND( ("GEQUAL\n") ); break;
	case HF_ALWAYS:
		DUMP_COMMAND( ("ALWAYS\n") ); break;
	}
}
//! reference val, the 32 bit value is copied into the push buffer
//! but only the alpha portion makes any sense...
void DecodeAlphaRef( uint32_t reg,uint32_t* val )
{
	// |-------------|
	// | Alpha Ref   |
	// |-------------|
	uint32_t iRef = *val;
	DUMP_COMMAND( ("\tAlpha Ref	  : 0x%x\n",iRef) );
}

//! BLEND_ENABLE
void DecodeBlendEnable( uint32_t reg,uint32_t* val )
{
	// |-------------|
	// | enable	   |
	// |-------------|
	uint32_t iEnable = *val;
	DUMP_COMMAND( ("\tBlendEnable %x\n", iEnable) );
}

//! there are 4 blend function, colour and alpha for both source and dest
//! these are packed into 2 integer
//! BLEND_FUNC_ALPHA
void DecodeBlendFuncAlpha( uint32_t reg,uint32_t* val )
{
	// |-------------|
	// | Alpha Func  |
	// | SRC--|-DST--|
	// |-------------|
	uint32_t iAlphaFunc			= *val;
	uint16_t iSrcAlphaFunc		= (iAlphaFunc & 0xFFFF0000)>>16;
	uint16_t iDestAlphaFunc		= (iAlphaFunc & 0x0000FFFF);

	DUMP_COMMAND( ("\tBlendFuncAlpha :\n") );
	switch( iSrcAlphaFunc )
	{
	case HBF_ZERO:
		DUMP_COMMAND( ("\t	SrcAlphaFunc : ZERO\n") ); break;
	case HBF_ONE:
		DUMP_COMMAND( ("\t	SrcAlphaFunc : ONE\n") ); break;
	case HBF_SRC_COLOR:
		DUMP_COMMAND( ("\t	SrcAlphaFunc : SRC_COLOR\n") ); break;
	case HBF_ONE_MINUS_SRC_COLOR:
		DUMP_COMMAND( ("\t	SrcAlphaFunc : ONE_MINUS_SRC_COLOR\n") ); break;
	case HBF_SRC_ALPHA:
		DUMP_COMMAND( ("\t	SrcAlphaFunc : SRC_ALPHA\n") ); break;
	case HBF_ONE_MINUS_SRC_ALPHA:
		DUMP_COMMAND( ("\t	SrcAlphaFunc : ONE_MINUS_SRC_ALPHA\n") ); break;
	case HBF_DST_ALPHA:
		DUMP_COMMAND( ("	SrcAlphaFunc : DST_ALPHA\n") ); break;
	case HBF_ONE_MINUS_DST_ALPHA:
		DUMP_COMMAND( ("\t	SrcAlphaFunc : ONE_MINUS_DST_ALPHA\n") ); break;
	case HBF_DST_COLOR:
		DUMP_COMMAND( ("\t	SrcAlphaFunc : DST_COLOR\n") ); break;
	case HBF_ONE_MINUS_DST_COLOR:
		DUMP_COMMAND( ("\t	SrcAlphaFunc : ONE_MINUS_DST_COLOR\n") ); break;
	case HBF_SRC_ALPHA_SATURATE:
		DUMP_COMMAND( ("\t	SrcAlphaFunc : SRC_ALPHA_SATURATE\n") ); break;
	case HBF_CONSTANT_COLOR:
		DUMP_COMMAND( ("\t	SrcAlphaFunc : CONSTANT_COLOR\n") ); break;
	case HBF_ONE_MINUS_CONSTANT_COLOR:
		DUMP_COMMAND( ("\t	SrcAlphaFunc : ONE_MINUS_CONSTANT_COLOR\n") ); break;
	case HBF_CONSTANT_ALPHA:
		DUMP_COMMAND( ("	SrcAlphaFunc : CONSTANT_ALPHA\n") ); break;
	case HBF_ONE_MINUS_CONSTANT_ALPHA:
		DUMP_COMMAND( ("\t	SrcAlphaFunc : ONE_MINUS_CONSTANT_ALPHA\n") ); break;
	}
	switch( iDestAlphaFunc )
	{
	case HBF_ZERO:
		DUMP_COMMAND( ("\t	DestAlphaFunc : ZERO\n") ); break;
	case HBF_ONE:
		DUMP_COMMAND( ("\t	DestAlphaFunc : ONE\n") ); break;
	case HBF_SRC_COLOR:
		DUMP_COMMAND( ("\t	DestAlphaFunc : SRC_COLOR\n") ); break;
	case HBF_ONE_MINUS_SRC_COLOR:
		DUMP_COMMAND( ("\t	DestAlphaFunc : ONE_MINUS_SRC_COLOR\n") ); break;
	case HBF_SRC_ALPHA:
		DUMP_COMMAND( ("\t	DestAlphaFunc : SRC_ALPHA\n") ); break;
	case HBF_ONE_MINUS_SRC_ALPHA:
		DUMP_COMMAND( ("\t	DestAlphaFunc : ONE_MINUS_SRC_ALPHA\n") ); break;
	case HBF_DST_ALPHA:
		DUMP_COMMAND( ("\t	DestAlphaFunc : DST_ALPHA\n") ); break;
	case HBF_ONE_MINUS_DST_ALPHA:
		DUMP_COMMAND( ("\t	DestAlphaFunc : ONE_MINUS_DST_ALPHA\n") ); break;
	case HBF_DST_COLOR:
		DUMP_COMMAND( ("\t	DestAlphaFunc : DST_COLOR\n") ); break;
	case HBF_ONE_MINUS_DST_COLOR:
		DUMP_COMMAND( ("\t	DestAlphaFunc : ONE_MINUS_DST_COLOR\n") ); break;
	case HBF_SRC_ALPHA_SATURATE:
		DUMP_COMMAND( ("\t	DestAlphaFunc : SRC_ALPHA_SATURATE\n") ); break;
	case HBF_CONSTANT_COLOR:
		DUMP_COMMAND( ("\t	DestAlphaFunc : CONSTANT_COLOR\n") ); break;
	case HBF_ONE_MINUS_CONSTANT_COLOR:
		DUMP_COMMAND( ("\t	DestAlphaFunc : ONE_MINUS_CONSTANT_COLOR\n") ); break;
	case HBF_CONSTANT_ALPHA:
		DUMP_COMMAND( ("\t	DestAlphaFunc : CONSTANT_ALPHA\n") ); break;
	case HBF_ONE_MINUS_CONSTANT_ALPHA:
		DUMP_COMMAND( ("\t	DestAlphaFunc : ONE_MINUS_CONSTANT_ALPHA\n") ); break;
	}
}

void DecodeBlendFuncColour( uint32_t reg,uint32_t* val )
{
	// |-------------|
	// | Colour Func |
	// | SRC--|-DST--|
	// |-------------|
	uint32_t iColourFunc		= *val;
	uint16_t iSrcColourFunc		= (iColourFunc & 0xFFFF0000)>>16;
	uint16_t iDestColourFunc	= (iColourFunc & 0x0000FFFF);

	DUMP_COMMAND( ("\tBlendFuncColour :\n") );
	switch( iSrcColourFunc )
	{
	case HBF_ZERO:
		DUMP_COMMAND( ("\t	SrcColourFunc : ZERO\n") ); break;
	case HBF_ONE:
		DUMP_COMMAND( ("\t	SrcColourFunc : ONE\n") ); break;
	case HBF_SRC_COLOR:
		DUMP_COMMAND( ("\t	SrcColourFunc : SRC_COLOR\n") ); break;
	case HBF_ONE_MINUS_SRC_COLOR:
		DUMP_COMMAND( ("\t	SrcColourFunc : ONE_MINUS_SRC_COLOR\n") ); break;
	case HBF_SRC_ALPHA:
		DUMP_COMMAND( ("\t	SrcColourFunc : SRC_ALPHA\n") ); break;
	case HBF_ONE_MINUS_SRC_ALPHA:
		DUMP_COMMAND( ("\t	SrcColourFunc : ONE_MINUS_SRC_ALPHA\n") ); break;
	case HBF_DST_ALPHA:
		DUMP_COMMAND( ("\t	SrcColourFunc : DST_ALPHA\n") ); break;
	case HBF_ONE_MINUS_DST_ALPHA:
		DUMP_COMMAND( ("\t	SrcColourFunc : ONE_MINUS_DST_ALPHA\n") ); break;
	case HBF_DST_COLOR:
		DUMP_COMMAND( ("\t	SrcColourFunc : DST_COLOR\n") ); break;
	case HBF_ONE_MINUS_DST_COLOR:
		DUMP_COMMAND( ("\t	SrcColourFunc : ONE_MINUS_DST_COLOR\n") ); break;
	case HBF_SRC_ALPHA_SATURATE:
		DUMP_COMMAND( ("\t	SrcColourFunc : SRC_ALPHA_SATURATE\n") ); break;
	case HBF_CONSTANT_COLOR:
		DUMP_COMMAND( ("\t	SrcColourFunc : CONSTANT_COLOR\n") ); break;
	case HBF_ONE_MINUS_CONSTANT_COLOR:
		DUMP_COMMAND( ("\t	SrcColourFunc : ONE_MINUS_CONSTANT_COLOR\n") ); break;
	case HBF_CONSTANT_ALPHA:
		DUMP_COMMAND( ("\t	SrcColourFunc : CONSTANT_ALPHA\n") ); break;
	case HBF_ONE_MINUS_CONSTANT_ALPHA:
		DUMP_COMMAND( ("\t	SrcColourFunc : ONE_MINUS_CONSTANT_ALPHA\n") ); break;
	}
	switch( iDestColourFunc )
	{
	case HBF_ZERO:
		DUMP_COMMAND( ("\t	DestColourFunc : ZERO\n") ); break;
	case HBF_ONE:
		DUMP_COMMAND( ("\t	DestColourFunc : ONE\n") ); break;
	case HBF_SRC_COLOR:
		DUMP_COMMAND( ("\t	DestColourFunc : SRC_COLOR\n") ); break;
	case HBF_ONE_MINUS_SRC_COLOR:
		DUMP_COMMAND( ("\t	DestColourFunc : ONE_MINUS_SRC_COLOR\n") ); break;
	case HBF_SRC_ALPHA:
		DUMP_COMMAND( ("\t	DestColourFunc : SRC_ALPHA\n") ); break;
	case HBF_ONE_MINUS_SRC_ALPHA:
		DUMP_COMMAND( ("\t	DestColourFunc : ONE_MINUS_SRC_ALPHA\n") ); break;
	case HBF_DST_ALPHA:
		DUMP_COMMAND( ("\t	DestColourFunc : DST_ALPHA\n") ); break;
	case HBF_ONE_MINUS_DST_ALPHA:
		DUMP_COMMAND( ("\t	DestColourFunc : ONE_MINUS_DST_ALPHA\n") ); break;
	case HBF_DST_COLOR:
		DUMP_COMMAND( ("\t	DestColourFunc : DST_COLOR\n") ); break;
	case HBF_ONE_MINUS_DST_COLOR:
		DUMP_COMMAND( ("\t	DestColourFunc : ONE_MINUS_DST_COLOR\n") ); break;
	case HBF_SRC_ALPHA_SATURATE:
		DUMP_COMMAND( ("\t	DestColourFunc : SRC_ALPHA_SATURATE\n") ); break;
	case HBF_CONSTANT_COLOR:
		DUMP_COMMAND( ("\t	DestColourFunc : CONSTANT_COLOR\n") ); break;
	case HBF_ONE_MINUS_CONSTANT_COLOR:
		DUMP_COMMAND( ("\t	DestColourFunc : ONE_MINUS_CONSTANT_COLOR\n") ); break;
	case HBF_CONSTANT_ALPHA:
		DUMP_COMMAND( ("\t	DestColourFunc : CONSTANT_ALPHA\n") ); break;
	case HBF_ONE_MINUS_CONSTANT_ALPHA:
		DUMP_COMMAND( ("\t	DestColourFunc : ONE_MINUS_CONSTANT_ALPHA\n") ); break;
	}
}
//! BLEND_COLOUR0
//! this is either 4 channel ARGB8 or 2 channel FP16 GR but I can't 
//! tell without tracking the current render target format...
void DecodeBlendColour0( uint32_t reg,uint32_t* val )
{
	// |-------------|
	// | Colour      |
	// |-------------|
	uint32_t iColour = *val;
	DUMP_COMMAND( ("\tBlend Colour 0 0x%x )\n", iColour ) );

}

//! BLEND_EQUATION
//! for some reason tgere are two enum values that don't make any sense...
void DecodeBlendEquation( uint32_t reg,uint32_t* val )
{
	// |-------------|
	// | AlEq | ColEq|
	// |-------------|
	uint32_t iAlphaEq = (*val & 0xFFFF0000) >> 16;
	uint32_t iColourEq = (*val & 0x0000FFFF);
	switch( iAlphaEq )
	{
	case HBE_BLEND_COLOR:
		DUMP_COMMAND( ("\tBlend Equation : Alpha Blend Colour??? \n") ); break;
	case HBE_FUNC_ADD:
		DUMP_COMMAND( ("\tBlend Equation : Alpha ADD \n") ); break;
	case HBE_MIN:
		DUMP_COMMAND( ("\tBlend Equation : Alpha MIN \n") ); break;
	case HBE_MAX:
		DUMP_COMMAND( ("\tBlend Equation : Alpha MAX \n") ); break;
	case HBE_BLEND_EQUATION:
		DUMP_COMMAND( ("\tBlend Equation : Alpha Blend Equation??? \n") ); break;
	case HBE_FUNC_SUBTRACT:
		DUMP_COMMAND( ("\tBlend Equation : Alpha SUB \n") ); break;
	case HBE_FUNC_REVERSE_SUBTRACT:
		DUMP_COMMAND( ("\tBlend Equation : Alpha REVERSE SUB \n") ); break;
	}
	switch( iColourEq )
	{
	case HBE_BLEND_COLOR:
		DUMP_COMMAND( ("\tBlend Equation : Colour Blend Colour??? \n") ); break;
	case HBE_FUNC_ADD:
		DUMP_COMMAND( ("\tBlend Equation : Colour ADD \n") ); break;
	case HBE_MIN:
		DUMP_COMMAND( ("\tBlend Equation : Colour MIN \n") ); break;
	case HBE_MAX:
		DUMP_COMMAND( ("\tBlend Equation : Colour MAX \n") ); break;
	case HBE_BLEND_EQUATION:
		DUMP_COMMAND( ("\tBlend Equation : Colour Blend Equation??? \n") ); break;
	case HBE_FUNC_SUBTRACT:
		DUMP_COMMAND( ("\tBlend Equation : Colour SUB \n") ); break;
	case HBE_FUNC_REVERSE_SUBTRACT:
		DUMP_COMMAND( ("tBlend Equation : Colour REVERSE SUB \n") ); break;
	}

}

//! COLOUR_MASK
//! Channel Mask for RenderTarget 0
void DecodeColourMask( uint32_t reg,uint32_t* val )
{
	// |---------------------|
	// | ColourMask		   |
	// | Blue	On	  = 0x1<<0 |
	// | Green On	  = 0x1<<8 |
	// | Red On	  = 0x1<<16|
	// | Alpha On	  = 0x1<<24|
	// |---------------------|
	uint32_t iColourMask = *val ;
	bool bBlueEnable = !!(iColourMask & 0x1);
	bool bGreenEnable = !!(iColourMask & 0x100);
	bool bRedEnable = !!(iColourMask & 0x10000);
	bool bAlphaEnable = !!(iColourMask & 0x1000000);
	DUMP_COMMAND( ("\tColour Mask for RT0 %s %s %s %s enabled\n",
						(bBlueEnable ? "Blue" : ""),
						(bGreenEnable ? "Green" : ""),
						(bRedEnable ? "Red" : ""),
						(bAlphaEnable ? "Alpha" : "") ) );

}
//! STENCIL_TEST_ENABLE
void DecodeStencilTestEnable( uint32_t reg,uint32_t* val )
{
	// |-------------|
	// | enable	   |
	// |-------------|
	uint32_t iEnable = *val;
	DUMP_COMMAND( ("\tStencil Test Enable %x\n", iEnable) );
}

//! STENCIL_WRITE_MASK
void DecodeStencilWriteMask( uint32_t reg,uint32_t* val )
{
	// |-------------|
	// | mask	   |
	// |-------------|
	uint32_t iRef = *val;
	DUMP_COMMAND( ("\tStencil Write Mask%x\n", iRef) );
}

//! STENCIL_FUNC
void DecodeStencilFunc( uint32_t reg,uint32_t* val )
{
	// |-------------|
	// | func	     |
	// |-------------|
	uint32_t iFunc = *val;
	switch( iFunc )
	{
	case HF_NEVER:
		DUMP_COMMAND( ("\tStencilFunc: NEVER\n") ); break;
	case HF_LESS:
		DUMP_COMMAND( ("\tStencilFunc: LESS\n") ); break;
	case HF_EQUAL:
		DUMP_COMMAND( ("\tStencilFunc: EQUAL\n") ); break;
	case HF_LEQUAL:
		DUMP_COMMAND( ("\tStencilFunc: LEQUAL\n") ); break;
	case HF_GREATER:
		DUMP_COMMAND( ("\tStencilFunc: GREATER\n") ); break;
	case HF_NOTEQUAL:
		DUMP_COMMAND( ("\tStencilFunc: NOTEQUAL\n") ); break;
	case HF_GEQUAL:
		DUMP_COMMAND( ("\tStencilFunc: GEQUAL\n") ); break;
	case HF_ALWAYS:
		DUMP_COMMAND( ("\tStencilFunc: ALWAYS\n") ); break;
	}
}
//! STENCIL_REF
void DecodeStencilRef( uint32_t reg,uint32_t* val )
{	
	// |-------------|
	// | ref	     |
	// |-------------|
	uint32_t iRef = *val;
	DUMP_COMMAND( ("\tStencil Ref: 0x%x\n",iRef) );
}
//! STENCIL_MASK
void DecodeStencilMask( uint32_t reg,uint32_t* val )
{	
	// |-------------|
	// | mask	     |
	// |-------------|
	uint32_t iMask = *val;
	DUMP_COMMAND( ("\tStencil Mask: 0x%x\n",iMask) );
}

//! TWO_SIDED_STENCIL_ENABLE
void DecodeTwoSidedStencilTestEnable( uint32_t reg,uint32_t* val )
{
	// |-------------|
	// | enable	   |
	// |-------------|
	uint32_t iEnable = *val;
	DUMP_COMMAND( ("atTwo Sided Stencil Test Enable %x\n", iEnable) );
}
//! BACK_STENCIL_MASK
void DecodeBackStencilMask( uint32_t reg,uint32_t* val )
{
	// |-------------|
	// | mask	     |
	// |-------------|
	uint32_t iRef = *val;
	DUMP_COMMAND( ("\tBack Stencil Mask %x\n", iRef) );
}

//! BACK_STENCIL_OP_FAIL
void DecodeBackStencilOpFail( uint32_t reg,uint32_t* val )
{
	// |-------------|
	// | fail		 |
	// |-------------|
	uint32_t iFail = *val;
	switch( iFail )
	{
	case HSO_KEEP:
		DUMP_COMMAND( ("\tBack Stencil fail op: KEEP\n") ); break;
	case HSO_REPLACE:
		DUMP_COMMAND( ("\tBack Stencil fail op: REPLACE\n") ); break;
	case HSO_INCR:
		DUMP_COMMAND( ("\tBack Stencil fail op: INCR\n") ); break;
	case HSO_DECR:
		DUMP_COMMAND( ("\tBack Stencil fail op: DECR\n") ); break;
	case HSO_INCR_WRAP:
		DUMP_COMMAND( ("\tBack Stencil fail op: INCR_WRAP\n") ); break;
	case HSO_DECR_WRAP:
		DUMP_COMMAND( ("\tBack Stencil fail op: DECR_WRAP\n") ); break;
	};
}
//! BACK_STENCIL_OP_DEPTH_FAIL
void DecodeBackStencilOpDepthFail( uint32_t reg,uint32_t* val )
{
	// |-------------|
	// | depth fail	 |
	// |-------------|
	uint32_t iDepthFail = *val;

	switch( iDepthFail )
	{
	case HSO_KEEP:
		DUMP_COMMAND( ("\tBack Stencil Depth fail op: KEEP\n") ); break;
	case HSO_REPLACE:
		DUMP_COMMAND( ("\tBack Stencil Depth fail op: REPLACE\n") ); break;
	case HSO_INCR:
		DUMP_COMMAND( ("\tBack Stencil Depth fail op: INCR\n") ); break;
	case HSO_DECR:
		DUMP_COMMAND( ("\tBack Stencil Depth fail op : DECR\n") ); break;
	case HSO_INCR_WRAP:
		DUMP_COMMAND( ("\tBack Stencil Depth fail op: INCR_WRAP\n") ); break;
	case HSO_DECR_WRAP:
		DUMP_COMMAND( ("\tBack Stencil Depth fail op: DECR_WRAP\n") ); break;
	};
}
//! BACK_STENCIL_OP_DEPTH_PASS
void DecodeBackStencilOpDepthPass( uint32_t reg,uint32_t* val )
{
	// |-------------|
	// | depth pass	 |
	// |-------------|
	uint32_t iDepthPass = *val;

	switch( iDepthPass )
	{
	case HSO_KEEP:
		DUMP_COMMAND( ("\tBack Stencil Depth pass op: KEEP\n") ); break;
	case HSO_REPLACE:
		DUMP_COMMAND( ("\tBack Stencil Depth pass op: REPLACE\n") ); break;
	case HSO_INCR:
		DUMP_COMMAND( ("\tBack Stencil Depth pass op: INCR\n") ); break;
	case HSO_DECR:
		DUMP_COMMAND( ("\tBack Stencil Depth pass op: DECR\n") ); break;
	case HSO_INCR_WRAP:
		DUMP_COMMAND( ("\tBack Stencil Depth pass op: INCR_WRAP\n") ); break;
	case HSO_DECR_WRAP:
		DUMP_COMMAND( ("\tBack Stencil Depth pass op: DECR_WRAP\n") ); break;
	};
}

//! SHADE_MODE
void DecodeShadeMode( uint32_t reg,uint32_t* val )
{
	//|-------------|
	//| Mode		   |
	//|-------------|
	uint32_t iMode = *val;
	DUMP_COMMAND( ("\tShadeMode: %s\n", ((iMode == HSM_FLAT) ? "Flat" : "Smooth")) );
}
//! BLEND_ENABLE_MRT
void DecodeBlendEnableMRT( uint32_t reg,uint32_t* val )
{
	// |---------------------------|
	// | Enable bit	             |
	// | 28 bit| 4 bit		     |
	// |       | 3    2	 1	  0  |
	// | 0x0   |MRT3 MRT2 MRT1 0x0 |
	// |---------------------------|
	uint32_t iEnableBits = *val;
	DUMP_COMMAND( ("\tBlendEnableMRT: %s\n",	((iEnableBits &  HTRS_TARGET_1) ? "RT1" : ""),
											((iEnableBits &  HTRS_TARGET_2) ? "RT2" : ""),
											((iEnableBits &  HTRS_TARGET_3) ? "RT3" : "")) );
}

//! COLOUR_MASK_MRT
//! 4 bits per MRT, 1 bit for RGBA. RT0 appears not to be used and should be 0
//! All 3 are set in one go
//! i.e. 0xFFF0 is all channels for MRT 1, 2, 3 (MRT0 is set using COLOUR_MASK register)
void DecodeColourMaskMRT( uint32_t reg,uint32_t* val )
{
	//|---------------------|
	//| ColourMask		   |
	//| Alph	On	  = 0x1<<MRTn) |
	//| Red On	  = 0x2<<MRTn) |
	//| Green On	  = 0x4<<MRTn)|
	//| Blue On	  = 0x8<<MRTn)|
	//|---------------------|
	uint32_t iColourMask = *val;

	iColourMask >>= 4;
	DUMP_COMMAND( ("\tColour Mask MRT\n") );
	for( int i= 0;i <3;i++ )
	{
		bool bBlueEnable = !!(iColourMask & 0x8);
		bool bGreenEnable = !!(iColourMask & 0x4);
		bool bRedEnable = !!(iColourMask & 0x2);
		bool bAlphaEnable = !!(iColourMask & 0x1);
		DUMP_COMMAND( ("\t\tRT%d has %s %s %s %s enabled\n", (i+1),
							(bBlueEnable ? "Blue" : ""),
							(bGreenEnable ? "Green" : ""),
							(bRedEnable ? "Red" : ""),
							(bAlphaEnable ? "Alpha" : "") ) );
		iColourMask >>= 4;
	}
}
//! LOGIC_OP_ENABLE
void DecodeLogicEnable( uint32_t reg,uint32_t* val )
{
	// |-------------|
	// | enable	   |
	// |-------------|
	uint32_t iEnable = *val;
	DUMP_COMMAND( ("\tLogic Enable %x\n", iEnable) );
}
//! LOGIC_OP
void DecodeLogicOp( uint32_t reg,uint32_t* val )
{
	// |-------------|
	// | Logic Op    |
	// |-------------|
	uint32_t iOp = *val;
	DUMP_COMMAND( ("\tLogic Op:") );
	switch( iOp )
	{
	case HLO_CLEAR:
		DUMP_COMMAND( ("		CLEAR\n") ); break;
	case HLO_AND:
		DUMP_COMMAND( ("		AND\n") ); break;
	case HLO_AND_REVERSE:
		DUMP_COMMAND( ("		AND REVERSE\n") ); break;
	case HLO_COPY:
		DUMP_COMMAND( ("		COPY\n") ); break;
	case HLO_AND_INVERTED:
		DUMP_COMMAND( ("		AND INVERTED\n") ); break;
	case HLO_NOOP:
		DUMP_COMMAND( ("		NOP\n") ); break;
	case HLO_XOR:
		DUMP_COMMAND( ("		XOR\n") ); break;
	case HLO_OR:
		DUMP_COMMAND( ("		 OR\n") ); break;
	case HLO_NOR:
		DUMP_COMMAND( ("		 NOR\n") ); break;
	case HLO_EQUIV:
		DUMP_COMMAND( ("		 EQUIV\n") ); break;
	case HLO_INVERT:
		DUMP_COMMAND( ("		 INVERT\n") ); break;
	case HLO_OR_REVERSE:
		DUMP_COMMAND( ("		 OR REVERSE\n") ); break;
	case HLO_COPY_INVERTED:
		DUMP_COMMAND( ("		 COPY INVERTED\n") ); break;
	case HLO_OR_INVERTED:
		DUMP_COMMAND( ("		 OR INVERTED\n") ); break;
	case HLO_NAND:
		DUMP_COMMAND( ("		 NAND\n") ); break;
	case HLO_SET:
		DUMP_COMMAND( ("		 SET\n") ); break;
	};
}
//! BLEND_COLOUR1
//! 2 channel FP16 AB 
//! Note: I just dump is as Hex cos I don't have an FP16 thingy here
void DecodeBlendColour1( uint32_t reg,uint32_t* val )
{
	// |-------------|
	// | Colour      |
	// |-------------|
	uint32_t iColour = *val;
	DUMP_COMMAND( ("\tBlend Colour 1 0x%x )\n", iColour ) );
}

//! DEPTH_BOUNDS_ENABLE
void DecodeDepthBoundsEnable( uint32_t reg,uint32_t* val )
{
	//|-------------|
	//| Colour      |
	//|-------------|
	uint32_t iColour = *val;
	DUMP_COMMAND( ("\tBlend Colour 1 0x%x )\n", iColour ) );
}

//! DEPTH_BOUNDS_NEAR
void DecodeDepthBoundsNear( uint32_t reg, uint32_t* val )
{
	// |-------------|
	// | near        |
	// |-------------|
	float fNear = *((float*)val);
	DUMP_COMMAND( ("\tDepthBounds Near %f\n", fNear) );
}
//! DEPTH_BOUNDS_FAR
void DecodeDepthBoundsFar( uint32_t reg,uint32_t* val )
{
	// |-------------|
	// | far         |
	// |-------------|
	float fFar = *((float*)val);
	DUMP_COMMAND( ("\tDepthBounds Far %f\n", fFar) );
}
//! VIEWPORT_NEAR
void DecodeViewportNear( uint32_t reg, uint32_t* val )
{
	// |-------------|
	// | near        |
	// |-------------|
	float fNear = *((float*)val);
	DUMP_COMMAND( ("\tViewport Near %f\n", fNear) );
}
//! VIEWPORT_FAR
void DecodeViewportFar( uint32_t reg,uint32_t* val )
{
	// |-------------|
	// | far         |
	// |-------------|
	float fFar = *((float*)val);
	DUMP_COMMAND( ("\tViewport Far %f\n", fFar) );
}
//! LINE_WIDTH
void DecodeLineWidth( uint32_t reg,uint32_t* val )
{
	// |-------------
	// | LIne WIdth 9.3 fixed
	// |-------------
	uint32_t iWidth = *val;
	DUMP_COMMAND( ("\tLine Width 0x%x\n", iWidth ) );
}

//! STENCIL_OP_FAIL
void DecodeStencilOpFail( uint32_t reg,uint32_t* val )
{
	// |-------------|
	// | fail		 |
	// |-------------|
	uint32_t iFail = *val;
	switch( iFail )
	{
	case HSO_KEEP:
		DUMP_COMMAND( ("\t Stencil fail op: KEEP\n") ); break;
	case HSO_REPLACE:
		DUMP_COMMAND( ("\t Stencil fail op: REPLACE\n") ); break;
	case HSO_INCR:
		DUMP_COMMAND( ("\t Stencil fail op: INCR\n") ); break;
	case HSO_DECR:
		DUMP_COMMAND( ("\t Stencil fail op: DECR\n") ); break;
	case HSO_INCR_WRAP:
		DUMP_COMMAND( ("\t Stencil fail op: INCR_WRAP\n") ); break;
	case HSO_DECR_WRAP:
		DUMP_COMMAND( ("\t Stencil fail op: DECR_WRAP\n") ); break;
	};
}
//! STENCIL_OP_DEPTH_FAIL
void DecodeStencilOpDepthFail( uint32_t reg,uint32_t* val )
{
	// |-------------|
	// | depth fail	 |
	// |-------------|
	uint32_t iDepthFail = *val;

	switch( iDepthFail )
	{
	case HSO_KEEP:
		DUMP_COMMAND( ("\tStencil Depth fail op: KEEP\n") ); break;
	case HSO_REPLACE:
		DUMP_COMMAND( ("\tStencil Depth fail op: REPLACE\n") ); break;
	case HSO_INCR:
		DUMP_COMMAND( ("\tStencil Depth fail op: INCR\n") ); break;
	case HSO_DECR:
		DUMP_COMMAND( ("\tStencil Depth fail op : DECR\n") ); break;
	case HSO_INCR_WRAP:
		DUMP_COMMAND( ("\tStencil Depth fail op: INCR_WRAP\n") ); break;
	case HSO_DECR_WRAP:
		DUMP_COMMAND( ("\tStencil Depth fail op: DECR_WRAP\n") ); break;
	};
}
//! STENCIL_OP_DEPTH_PASS
void DecodeStencilOpDepthPass( uint32_t reg,uint32_t* val )
{
	// |-------------|
	// | depth pass	 |
	// |-------------|
	uint32_t iDepthPass = *val;

	switch( iDepthPass )
	{
	case HSO_KEEP:
		DUMP_COMMAND( ("\tStencil Depth pass op: KEEP\n") ); break;
	case HSO_REPLACE:
		DUMP_COMMAND( ("\tStencil Depth pass op: REPLACE\n") ); break;
	case HSO_INCR:
		DUMP_COMMAND( ("\tStencil Depth pass op: INCR\n") ); break;
	case HSO_DECR:
		DUMP_COMMAND( ("\tStencil Depth pass op: DECR\n") ); break;
	case HSO_INCR_WRAP:
		DUMP_COMMAND( ("\tStencil Depth pass op: INCR_WRAP\n") ); break;
	case HSO_DECR_WRAP:
		DUMP_COMMAND( ("\tStencil Depth pass op: DECR_WRAP\n") ); break;
	};
}
//! SCISSOR_RECT_HORIZ
//! set the scissor rectangle, x and width.
//! Each is a 16 bit integer
void DecodeScissorRectHoriz( uint32_t reg,uint32_t* val )
{
	// |---------------|
	// | width<<16 | x |
	// |---------------|
	uint16_t iWidth		= (*val & 0xFFFF0000) >> 16;
	uint16_t iX			= (*val & 0x0000FFFF);

	DUMP_COMMAND( ("\tScissor Rect Horiz X - %d, Width - %d\n", iX, iWidth ) );
}
//! SCISSOR_RECT_VERT
//! set the viewport rectangle, y and height.
//! Each is a 16 bit integer
void DecodeScissorRectVert( uint32_t reg,uint32_t* val )
{
	// |---------------|
	// | height<<16 | y|
	// |---------------|
	uint16_t iHeight	= (*val & 0xFFFF0000) >> 16;
	uint16_t iY			= (*val & 0x0000FFFF);
	DUMP_COMMAND( ("\tScissor Rect Vert Y - %d, Height - %d\n", iY, iHeight ) );
}

//! VIEWPORT_RECT_HORIZ
//! set the viewport rectangle, x and width.
//! Each is a 16 bit integer
void DecodeViewportRectHoriz( uint32_t reg,uint32_t* val )
{
	// |---------------|
	// | width<<16 | x |
	// |---------------|
	uint16_t iWidth		= (*val & 0xFFFF0000) >> 16;
	uint16_t iX			= (*val & 0x0000FFFF);

	DUMP_COMMAND( ("\tViewport Rect Horiz X - %d, Width - %d\n", iX, iWidth ) );
}
//! VIEWPORT_RECT_VERT
//! set the viewport rectangle, y and height.
//! Each is a 16 bit integer
void DecodeViewportRectVert( uint32_t reg,uint32_t* val )
{
	// |---------------|
	// | height<<16 | y|
	// |---------------|
	uint16_t iHeight	= (*val & 0xFFFF0000) >> 16;
	uint16_t iY			= (*val & 0x0000FFFF);
	DUMP_COMMAND( ("\tViewport Rect Vert Y - %d, Height - %d\n", iY, iHeight ) );
}

//! VIEWPORT_SCALE_X
void DecodeViewportScaleX( uint32_t reg,uint32_t* val )
{
	// |---|
	// | x | 
	// |---|
	float fX = *((float*)val);
	DUMP_COMMAND( ("\tViewport Scale X - %f\n", fX) );
}
//! VIEWPORT_SCALE_Y
void DecodeViewportScaleY( uint32_t reg,uint32_t* val )
{
	// |---|
	// | Y | 
	// |---|
	float fY = *((float*)val);
	DUMP_COMMAND( ("\tViewport Scale Y - %f\n", fY) );
}
//! VIEWPORT_SCALE_Z
void DecodeViewportScaleZ( uint32_t reg,uint32_t* val )
{
	// |---|
	// | Z | 
	// |---|
	float fZ = *((float*)val);
	DUMP_COMMAND( ("\tViewport Scale Z - %f\n", fZ) );
}
//! VIEWPORT_SCALE_W
void DecodeViewportScaleW( uint32_t reg,uint32_t* val )
{
	// |---|
	// | W | 
	// |---|
	float fW = *((float*)val);
	DUMP_COMMAND( ("\tViewport Scale W - %f\n", fW) );
}

//! VIEWPORT_POS_OFFSET_X
void DecodeViewportPosOffsetX( uint32_t reg,uint32_t* val )
{
	// |---|
	// | x | 
	// |---|
	float fX = *((float*)val);
	DUMP_COMMAND( ("\tViewport Pos Offset X - %f\n", fX) );
}
//! VIEWPORT_POS_OFFSET_Y
void DecodeViewportPosOffsetY( uint32_t reg,uint32_t* val )
{
	// |---|
	// | Y | 
	// |---|
	float fY = *((float*)val);
	DUMP_COMMAND( ("\tViewport Pos Offset Y - %f\n", fY) );
}
//! VIEWPORT_POS_OFFSET_Z
void DecodeViewportPosOffsetZ( uint32_t reg,uint32_t* val )
{
	// |---|
	// | Z | 
	// |---|
	float fZ = *((float*)val);
	DUMP_COMMAND( ("\tViewport Pos Offset Z - %f\n", fZ) );
}
//! VIEWPORT_POS_OFFSET_W
void DecodeViewportPosOffsetW( uint32_t reg,uint32_t* val )
{
	// |---|
	// | W | 
	// |---|
	float fW = *((float*)val);
	DUMP_COMMAND( ("\tViewport Pos Offset W - %f\n", fW) );
}

//! POLYGON_OFFSET_ENABLE
void DecodePolygonOffsetEnable( uint32_t reg,uint32_t* val )
{
	// |-------------|
	// | enable	   |
	// |-------------|
	uint32_t iEnable = *val;
	DUMP_COMMAND( ("\tPolygon Offset Enable %x\n", iEnable) );
}

//! DEPTH_FUNC
void DecodeDepthFunc( uint32_t reg,uint32_t* val )
{
	// |-------------|
	// | func	       |
	// |-------------|
	uint32_t iFunc = *val;
	DUMP_COMMAND( ("\tDepthFunc : ") );
	switch( iFunc )
	{
	case HF_NEVER:
		DUMP_COMMAND( ("NEVER\n") ); break;
	case HF_LESS:
		DUMP_COMMAND( ("LESS\n") ); break;
	case HF_EQUAL:
		DUMP_COMMAND( ("EQUAL\n") ); break;
	case HF_LEQUAL:
		DUMP_COMMAND( ("LEQUAL\n") ); break;
	case HF_GREATER:
		DUMP_COMMAND( ("GREATER\n") ); break;
	case HF_NOTEQUAL:
		DUMP_COMMAND( ("NOTEQUAL\n") ); break;
	case HF_GEQUAL:
		DUMP_COMMAND( ("GEQUAL\n") ); break;
	case HF_ALWAYS:
		DUMP_COMMAND( ("ALWAYS\n") ); break;
	}
}

//! DEPTH_WRITE_ENABLE
void DecodeDepthWriteEnable( uint32_t reg,uint32_t* val )
{
	// |-------------|
	// | enable	   |
	// |-------------|
	uint32_t iEnable = *val;
	DUMP_COMMAND( ("\tDepth Write  Enable %x\n", iEnable) );
}

//! DEPTH_TEST_ENABLE
void DecodeDepthTestEnable( uint32_t reg,uint32_t* val )
{
	// |-------------|
	// | enable	   |
	// |-------------|
	uint32_t iEnable = *val;
	DUMP_COMMAND( ("\tDepth Test Enable %x\n", iEnable) );
}
//! POLYGON_OFFSET_FACTOR
void DecodePolygonOffsetFactor( uint32_t reg,uint32_t* val )
{
	//|--------|
	//| factor |
	//|--------|
	float fFactor		= *((float*)val);

	DUMP_COMMAND( ("\tPolygon Offset Factor - %f\n", fFactor) );
}

//! POLYGON_OFFSET_UNITS
void DecodePolygonOffsetUnits( uint32_t reg,uint32_t* val )
{
	//|--------|
	//| units |
	//|--------|
	float fUnits		= *((float*)val);

	DUMP_COMMAND( ("\tPolygon Offset Units - %f\n", fUnits) );
}

//! TEXTURE0_BRILINEAR
void DecodeTextureBrilinear( uint32_t reg,uint32_t* val )
{
	//|--------|
	//| unknown |
	//|--------|
	uint32_t iIndex = ((reg&0x0000FFFF) - TEXTURE0_BRILINEAR) / 0x4;
	uint32_t iUnknown		= *val;

	DUMP_COMMAND( ("\tTexture %d Brilinear - 0x%x\n", iIndex, iUnknown) );
}

//! UPLOAD_VERTEX_SHADER
void DecodeUploadVertexShader( uint32_t reg,uint32_t* val )
{
	// |-------------|
	// | data        |
	// |-------------|
	uint32_t iIndex = ((reg&0x0000FFFF) - UPLOAD_VERTEX_SHADER) / 0x4;
	uint32_t iData = *val;

	DUMP_COMMAND( ("\tUpload Vertex Shader: Index 0x%x - 0x%x\n", iIndex, iData ) );
}

//! VERTEX_DATA_OFFSET
//! There are 16 version of this register each 4 bytes apart.
//! Top bit is main or local or offset
void DecodeVertexDataOffset( uint32_t reg,uint32_t* val )
{
	// |-------------|-------------|
	// | Register    | Offset	   |
	// | Index 0x4*N | Top bit = 0x1 = main 0x0 = local
	// |-------------|-------------|
	uint32_t iIndex = ((reg&0x0000FFFF) - VERTEXSTREAM0_OFFSET) / 0x4;
	uint32_t iLocation = (*val & 0x80000000) >> 31;
	uint32_t iOffset = *val & ~0x80000000;

	DUMP_COMMAND( ("\tVertexDataOffset Register %i, %s, 0x%x\n", iIndex, (iLocation == HRL_MAIN_XDDR) ? "Main XDDR" : "Local GDDR", iOffset ) );
}

//! FLUSH_0
void DecodeFlush0( uint32_t reg,uint32_t* val )
{
	// |-------------|
	// | unknown so far 0	   |
	// |-------------|
	uint32_t iUnknown = *val;
	DUMP_COMMAND( ("\tFlush0 %x\n", iUnknown) );
}
//! FLUSH_1
void DecodeFlush1( uint32_t reg,uint32_t* val )
{
	// |-------------|
	// | unknown so far 0	   |
	// |-------------|
	uint32_t iUnknown = *val;
	DUMP_COMMAND( ("\tFlush1 %x\n", iUnknown) );
}

//! VERTEXSTREAM0_FORMAT
//! There are 16 version of this register each 4 bytes apart.
//! Each 
void DecodeVertexDataArray( uint32_t reg,uint32_t* val )
{
	// |-------------|--------------------------------------------------------------|
	// | Register    | Data															|
	// | Index 0x4*N | 16 bit freq | 8 bit stride | 4 bit num coords | 4 bit format |
	// |-------------|-------------|--------------|------------------|--------------|
	uint32_t iIndex = ((reg&0x0000FFFF) - VERTEXSTREAM0_FORMAT) / 0x4;
	uint16_t iFreq = (*val & 0xFFFF000) >> 16;
	uint8_t iStride = (*val & 0x0000FF00) >> 8;
	uint8_t iNumCoords = (*val & 0x000000F0) >> 4;
	uint32_t iFormat = (*val & 0x0000000F) >> 0;

	DUMP_COMMAND( ("\tVertexDataArray Register %i, Freq %d, Stride %d,  Format %d x ", iIndex, iFreq, iStride, iNumCoords) );
	switch( iFormat )
	{
	case HVF_NORMALISED_INT16:
		DUMP_COMMAND( ("NORMALISED_INT16\n") ); break;
	case HVF_FLOAT:
		DUMP_COMMAND( ("FLOAT\n") ); break;
	case HVF_FLOAT16:
		DUMP_COMMAND( ("FLOAT16\n") ); break;
	case HVF_NORMALISED_UNSIGNED_BYTE:
		DUMP_COMMAND( ("NORMALISED_UNSIGNED_BYTE\n") ); break;
	case HVF_INT16:
		DUMP_COMMAND( ("INT16\n") ); break;
	case HVF_11_11_10:
		DUMP_COMMAND( ("11_11_10\n") ); break;
	case HVF_UNSIGNED_BYTE:
		DUMP_COMMAND( ("UNSIGNED_BYTE\n") ); break;
	}
}
//! TIME_STAMP
void DecodeTimeStamp( uint32_t reg,uint32_t* val )
{
	// Note I'm not quite sure how the register works yet
	// libgcm claims its a 7 bit index but that doesn't appear 
	// to be what get put in the register (2 bits always seem to be set..
	// |-------------|
	// | register	 |
	// |-------------|
	uint32_t iRegister = *val;
	DUMP_COMMAND( ("\tTime Stamp Register %x\n", iRegister) );
}

//! DRAW
//! first time set the primitive type, second time issues the draw wiht 0x0?
void DecodeDraw( uint32_t reg,uint32_t* val )
{
	// |-------------|
	// | Prim Type   |
	// |-------------|
	uint32_t iType = *val;
	DUMP_COMMAND( ("\tDraw ") );
	switch( iType )
	{
	case 0:
		DUMP_COMMAND( ("0x0\n") ); break;
	case HPT_POINTS:
		DUMP_COMMAND( ("POINTS\n") ); break;
	case HPT_LINES:
		DUMP_COMMAND( ("LINES\n") ); break;
	case HPT_LINE_LOOP:
		DUMP_COMMAND( ("LINE_LOOP\n") ); break;
	case HPT_LINE_STRIP:
		DUMP_COMMAND( ("LINE_STRIP\n") ); break;
	case HPT_TRIANGLES:
		DUMP_COMMAND( ("TRIANGLES\n") ); break;
	case HPT_TRIANGLE_STRIP:
		DUMP_COMMAND( ("TRIANGLE_STRIP\n") ); break;
	case HPT_TRIANGLE_FAN:
		DUMP_COMMAND( ("TRIANGLE_FAN\n") ); break;
	case HPT_QUADS:
		DUMP_COMMAND( ("QUADS\n") ); break;
	case HPT_QUAD_STRIP:
		DUMP_COMMAND( ("QUAD_STRIP\n") ); break;
	case HPT_POLYGON:
		DUMP_COMMAND( ("POLYGON\n") ); break;
	}
}


//! VERTEX_ARRAY_PARAMS
//! Issue upto 256 prims after a draw call
void DecodeVertexArrayParams( uint32_t reg,uint32_t* val )
{
	// |----------------------------|
	// |(count-1)<<24 | FirstVert   |
	// |----------------------------|
	uint32_t iCount = ((*val & 0xFF000000)>>24)+1;
	uint32_t iFirstVert =  (*val & 0x00FFFFFF);
	DUMP_COMMAND( ("\tVertex Array Params: Count %d, First Vertex %d\n", iCount, iFirstVert) );
}
//! INDEX_ARRAY_OFFSET
//! Where the index array memory lives
void DecodeIndexArrayOffset( uint32_t reg,uint32_t* val )
{
	// |-------------|
	// | offset      |
	// |-------------|
	uint32_t iOffset = *val;
	DUMP_COMMAND( ("\tIndex Array Offset 0x%x\n", iOffset) );

}
//! INDEX_ARRAY_TYPE
void DecodeIndexArrayType( uint32_t reg,uint32_t* val )
{
	// lowest bit of param 2 is location, next bit is 16 or 32 bit index
	// |-------------|
	// |0x00 = 32 bit index local|
	// |0x01 = 32 bit index main|
	// |0x10 = 16 bit index local|
	// |0x11 = 16 bit index main|
	// |-------------|
	uint32_t iType = *val;
	DUMP_COMMAND( ("\tIndex Array Type: Location %s Type %s\n", 
					(iType & HRL_MAIN_XDDR) ? "Main XDDR" : "Local GDDR",
					(iType & HIT_16_BIT) ? "16 Bit indices" : "32 Bit indices") );

}

//! INDEX_ARRAY_PARAMS
//! Issue upto 256 prims after a draw call
//! generally this is used with REGISTER_REPEAT (0x40000000) set to issue > 256
void DecodeIndexArrayParams( uint32_t reg,uint32_t* val )
{
	// |----------------------------|
	// |(count-1)<<24 |  FirstVert  |
	// |----------------------------|
	uint32_t iCount = ((*val & 0xFF000000)>>24)+1;
	uint32_t iFirstIndex=  (*val & 0x00FFFFFF);
	DUMP_COMMAND( ("\tIndex Array Params : Count %d, First Index %d\n", iCount, iFirstIndex) );

}

//! CULL_FACE
void DecodeCullFace( uint32_t reg,uint32_t* val )
{
	// |-------------|
	// | Cull Face   |
	// |-------------|
	uint32_t iCull = *val;
	DUMP_COMMAND( ("\tCull Face : ") );
	switch( iCull )
	{
	case HCF_FRONT:
		DUMP_COMMAND( ("FRONT\n") ); break;
	case HCF_BACK:
		DUMP_COMMAND( ("BACK\n") ); break;
	case HCF_FRONT_AND_BACK:
		DUMP_COMMAND( ("FRONT AND BACK\n") ); break;
	}
}
//! FRONT_FACE
void DecodeFrontFace( uint32_t reg,uint32_t* val )
{
	// |-------------|
	// | FrontFace   |
	// |-------------|
	uint32_t iCull = *val;
	DUMP_COMMAND( ("\tFront Face : ") );
	switch( iCull )
	{
	case HFF_CW:
		DUMP_COMMAND( ("Clockwise\n") ); break;
	case HFF_CCW:
		DUMP_COMMAND( ("Counter Clockwise\n") ); break;
	}
}
//! CULL_FACE_ENABLE
void DecodeCullFaceEnable( uint32_t reg,uint32_t* val )
{
	// |-------------|
	// | enable	     |
	// |-------------|
	uint32_t iEnable = *val;
	DUMP_COMMAND( ("\tCull Face Enable %x\n", iEnable) );
}

//! TEXTURE0_SIZE_DP
void DecodeTextureSizeDP( uint32_t reg,uint32_t* val )
{
	// |-------------|
	// | depth<<20|pitch	   |
	// |-------------|
	uint32_t iIndex = ((reg&0x0000FFFF) - TEXTURE0_SIZE_DP) / 0x4;
	uint32_t iDepth = (*val & 0xFFF00000)>>20;
	uint32_t iPitch = (*val & 0x000FFFFF);

	DUMP_COMMAND( ("\tTexture %i Depth %d Pitch %d\n", iIndex, iDepth, iPitch) );
}

//! TEXTURE0_OFFSET
void DecodeTextureOffset( uint32_t reg,uint32_t* val )
{
	// |-------------|
	// | offset	   |
	// |-------------|
	uint32_t iIndex = ((reg&0x0000FFFF) - TEXTURE0_OFFSET) / 0x20;
	uint32_t iOffset = *val;
	DUMP_COMMAND( ("\tTexture %i Offset 0x%x\n", iIndex, iOffset) );
}

//! TEXTURE0_FORMAT
void DecodeTextureFormat( uint32_t reg,uint32_t* val )
{
	// |-------------
	// | format	   
	// | 16 bit | 8 bit | 8 bit | 4 bit | 4 bit |
	// | 0x0 | mipmaps | format | Dims | Flags   |
	// |-------------

	// see HERESY_TEXTURE_FORMAT for the main bits
	// note HTF_UNKNOWN should be always set to 1 so far...
	// Flags bit 0 and 1 = 0x1 = LOCAL, 0x2 = MAIN RAM
	//		 bit 2 = cube map
	//		 bit 3 = 0x1 - UNKNOWN
	// Dims = 0x1 = 1D, 0x2 = 2D, 0x3 = 3D
	// Format
	// bits 7 6 5 4 3 2 1 0
	//			1 = linear
	//        1 = un-normalised
	// mask 1 0 0 1 1 1 1 1 = mask for formats as lib gcm
	// bits 7 6 5 4 3 2 1 0
	// |-------------|-------------|
	uint32_t iIndex = ((reg&0x0000FFFF) - TEXTURE0_FORMAT) / 0x20;
	uint32_t iFormat = *val;
	DUMP_COMMAND( ("\tTexture %i Format : ", iIndex) );

	DUMP_COMMAND( ("Location %s ",	(iFormat & HTF_LOCAL_RAM) ? "Local GDDR" : "",
						  			(iFormat & HTF_MAIN_RAM) ?  "Main XDDR" : "ERROR!" ) );
	DUMP_COMMAND( ("%dD %s ", ((iFormat & HTF_DIM_MASK) >> 4),
							(iFormat & HTF_CUBEMAP) ? "CubeMap" : "" ) );
	DUMP_COMMAND( ("%s %s",	(iFormat & HTF_UNNORMALISED) ? "Un-normalised" : "",
						  	(iFormat & HTF_LINEAR_MEM) ?  "Linear " : "" ) );
	DUMP_COMMAND( ("\n") );

	switch( iFormat & HTF_FORMAT_MASK )
	{
	case HTF_B8: 
		DUMP_COMMAND( ("\t                  : B8 \n") ); break;
	case HTF_A1R5G5B5: 
		DUMP_COMMAND( ("\t                  : HTF_A1R5G5B5 \n") ); break;
	case HTF_A4R4G4B4: 
		DUMP_COMMAND( ("\t                  : HTF_A4R4G4B4 \n") ); break;
	case HTF_R5G6B5: 
		DUMP_COMMAND( ("\t                  : HTF_R5G6B5 \n") ); break;
	case HTF_A8R8G8B8: 
		DUMP_COMMAND( ("\t                  : HTF_A8R8G8B8 \n") ); break;
	case HTF_COMPRESSED_DXT1: 
		DUMP_COMMAND( ("\t                  : HTF_COMPRESSED_DXT1 \n") ); break;
	case HTF_COMPRESSED_DXT23: 
		DUMP_COMMAND( ("\t                  : HTF_COMPRESSED_DXT23 \n") ); break;
	case HTF_COMPRESSED_DXT45: 
		DUMP_COMMAND( ("\t                  : HTF_COMPRESSED_DXT45 \n") ); break;
	case HTF_G8B8: 
		DUMP_COMMAND( ("\t                  : HTF_G8B8 \n") ); break;
	case HTF_R6G5B5: 
		DUMP_COMMAND( ("\t                  : HTF_R6G5B5 \n") ); break;
	case HTF_DEPTH24_D8: 
		DUMP_COMMAND( ("\t                  : HTF_DEPTH24_D8 \n") ); break;
	case HTF_DEPTH24_D8_FLOAT: 
		DUMP_COMMAND( ("\t                  : HTF_DEPTH24_D8_FLOAT \n") ); break;
	case HTF_DEPTH16: 
		DUMP_COMMAND( ("\t                  : HTF_DEPTH16 \n") ); break;
	case HTF_DEPTH16_FLOAT: 
		DUMP_COMMAND( ("\t                  : HTF_DEPTH16_FLOAT \n") ); break;
	case HTF_X16: 
		DUMP_COMMAND( ("\t                  : HTF_X16 \n") ); break;
	case HTF_Y16_X16: 
		DUMP_COMMAND( ("\t                  : HTF_Y16_X16 \n") ); break;
	case HTF_R5G5B5A1: 
		DUMP_COMMAND( ("\t                  : HTF_R5G5B5A1 \n") ); break;
	case HTF_COMPRESSED_HILO8: 
		DUMP_COMMAND( ("\t                  : HTF_COMPRESSED_HILO8 \n") ); break;
	case HTF_COMPRESSED_HILO_S8: 
		DUMP_COMMAND( ("\t                  : HTF_COMPRESSED_HILO_S8 \n") ); break;
	case HTF_W16_Z16_Y16_X16_FLOAT: 
		DUMP_COMMAND( ("\t                  : HTF_W16_Z16_Y16_X16_FLOAT \n") ); break;
	case HTF_W32_Z32_Y32_X32_FLOAT: 
		DUMP_COMMAND( ("\t                  : HTF_W32_Z32_Y32_X32_FLOAT \n") ); break;
	case HTF_X32_FLOAT: 
		DUMP_COMMAND( ("\t                  : HTF_X32_FLOAT \n") ); break;
	case HTF_D1R5G5B5: 
		DUMP_COMMAND( ("\t                  : HTF_D1R5G5B5 \n") ); break;
	case HTF_D8R8G8B8: 
		DUMP_COMMAND( ("\t                  : HTF_D8R8G8B8 \n") ); break;
	case HTF_Y16_X16_FLOAT: 
		DUMP_COMMAND( ("\t                  : HTF_Y16_X16_FLOAT \n") ); break;
	};
}

//! TEXTURE0_ADDRESS
void DecodeTextureAddress( uint32_t reg,uint32_t* val )
{
	//|-------------|
	//| offset	   |
	//|-------------|
	uint32_t iIndex = ((reg&0x0000FFFF) - TEXTURE0_ADDRESS) / 0x20;
	uint32_t iSMode = (*val & 0x0000000F);
	uint32_t iTMode = (*val & 0x00000F00) >> 8;
	uint32_t iRemapBias =  *val & HTRB_UNSIGNED_REMAP_BIAS;
	uint32_t iRMode = (*val & 0x000F0000) >> 16;
	uint32_t iShadCmp= (*val & 0x0F0000000) >> 24;

	DUMP_COMMAND( ("\tTexture %i Address : \n", iIndex) );
	DUMP_COMMAND( ("\t                   : s - WRAP\n") );
	switch( iSMode )
	{
	case HTA_WRAP:
		DUMP_COMMAND( ("\t                   : s - WRAP\n") ); break;
	case HTA_MIRROR:
		DUMP_COMMAND( ("\t                   : s - MIRROR\n") ); break;
	case HTA_CLAMP_TO_EDGE:
		DUMP_COMMAND( ("\t                   : s - CLAMP_TO_EDGE\n") ); break;
	case HTA_BORDER:
		DUMP_COMMAND( ("\t                   : s - BORDER\n") ); break;
	case HTA_CLAMP:
		DUMP_COMMAND( ("\t                   : s - CLAMP\n") ); break;
	case HTA_MIRROR_ONCE_CLAMP_TO_EDGE:
		DUMP_COMMAND( ("\t                   : s - MIRROR_ONCE_CLAMP_TO_EDGE\n") ); break;
	case HTA_MIRROR_ONCE_BORDER:
		DUMP_COMMAND( ("\t                   : s - MIRROR_ONCE_BORDER\n") ); break;
	case HTA_MIRROR_ONCE_CLAMP:
		DUMP_COMMAND( ("\t                   : s - MIRROR_ONCE_CLAMP\n") ); break;
	}
	switch( iTMode )
	{
	case HTA_WRAP:
		DUMP_COMMAND( ("\t                   : t - WRAP\n") ); break;
	case HTA_MIRROR:
		DUMP_COMMAND( ("\t                   : t - MIRROR\n") ); break;
	case HTA_CLAMP_TO_EDGE:
		DUMP_COMMAND( ("\t                   : t - CLAMP_TO_EDGE\n") ); break;
	case HTA_BORDER:
		DUMP_COMMAND( ("\t                   : t - BORDER\n") ); break;
	case HTA_CLAMP:
		DUMP_COMMAND( ("\t                   : t - CLAMP\n") ); break;
	case HTA_MIRROR_ONCE_CLAMP_TO_EDGE:
		DUMP_COMMAND( ("\t                   : t - MIRROR_ONCE_CLAMP_TO_EDGE\n") ); break;
	case HTA_MIRROR_ONCE_BORDER:
		DUMP_COMMAND( ("\t                   : t - MIRROR_ONCE_BORDER\n") ); break;
	case HTA_MIRROR_ONCE_CLAMP:
		DUMP_COMMAND( ("\t                   : t - MIRROR_ONCE_CLAMP\n") ); break;
	}
	switch( iRMode )
	{
	case HTA_WRAP:
		DUMP_COMMAND( ("\t                   : r - WRAP\n") ); break;
	case HTA_MIRROR:
		DUMP_COMMAND( ("\t                   : r - MIRROR\n") ); break;
	case HTA_CLAMP_TO_EDGE:
		DUMP_COMMAND( ("\t                   : r - CLAMP_TO_EDGE\n") ); break;
	case HTA_BORDER:
		DUMP_COMMAND( ("\t                   : r - BORDER\n") ); break;
	case HTA_CLAMP:
		DUMP_COMMAND( ("\t                   : r - CLAMP\n") ); break;
	case HTA_MIRROR_ONCE_CLAMP_TO_EDGE:
		DUMP_COMMAND( ("\t                   : r - MIRROR_ONCE_CLAMP_TO_EDGE\n") ); break;
	case HTA_MIRROR_ONCE_BORDER:
		DUMP_COMMAND( ("\t                   : r - MIRROR_ONCE_BORDER\n") ); break;
	case HTA_MIRROR_ONCE_CLAMP:
		DUMP_COMMAND( ("\t                   : r - MIRROR_ONCE_CLAMP\n") ); break;
	}
	DUMP_COMMAND( ("\t                   : %s\n", iRemapBias ? "Remap  to [-1,1]" : "Remap to [0,1]") );
	switch( iShadCmp )
	{
	case HTZF_NEVER:
		DUMP_COMMAND( ("\t                   :  Depth Func NEVER\n") ); break;
	case HTZF_LESS:
		DUMP_COMMAND( ("\t                   :  Depth Func LESS\n") ); break;
	case HTZF_EQUAL:
		DUMP_COMMAND( ("\t                   :  Depth Func EQUAL\n") ); break;
	case HTZF_LEQUAL:
		DUMP_COMMAND( ("\t                   :  Depth Func LEQUAL\n") ); break;
	case HTZF_GREATER:
		DUMP_COMMAND( ("\t                   :  Depth Func GREATER\n") ); break;
	case HTZF_NOTEQUAL:
		DUMP_COMMAND( ("\t                   :  Depth Func NOTEQUAL\n") ); break;
	case HTZF_GEQUAL:
		DUMP_COMMAND( ("\t                   :  Depth Func GEQUAL\n") ); break;
	case HTZF_ALWAYS:
		DUMP_COMMAND( ("\t                   : Depth Func	 ALWAYS\n") ); break;
	}
}
//! TEXTURE0_CONTROL
void DecodeTextureControl( uint32_t reg,uint32_t* val )
{
	// |-------------|
	// | offset	   |
	// |-------------|
	uint32_t iIndex = ((reg&0x0000FFFF) - TEXTURE0_CONTROL) / 0x20;
	uint32_t iAniso = (*val & 0x00000070) >> 4;
	uint32_t iMaxLod = (*val & 0x00000780) >> 7;
	uint32_t iMinLod = (*val & 0x00078000) >> 15;
	uint32_t iEnable = (*val & 0x80000000) >> 31;

	switch( iAniso )
	{
	case HTA_MAX_ANISO_1: iAniso = 1; break;
	case HTA_MAX_ANISO_16: iAniso = 16; break;
	default:
		iAniso = 2 * iAniso; break;
	}
	DUMP_COMMAND( ("\tTexture %i Control: 0x%x\n", iIndex, *val) );
	DUMP_COMMAND( ("\t                  : Max Aniso %d MaxLod %d MinLod %d\n", iAniso, iMaxLod, iMinLod) );
	if( iEnable )
	{
		DUMP_COMMAND( ("\t                  : ENABLED\n") );
	} else
	{
		DUMP_COMMAND( ("\t                  : Disabled\n") );
	}
}

//! TEXTURE0_REMAP
void DecodeTextureRemap( uint32_t reg,uint32_t* val )
{	
	// TODO Decode the remap bits
	// |-------------|
	// | Reference   |
	// |-------------|
	uint32_t iIndex = ((reg&0x0000FFFF) - TEXTURE0_REMAP) / 0x20;
	uint32_t iRemap = *val;
	DUMP_COMMAND( ("\tTexture %i Remap: 0x%x\n",iIndex, iRemap) );
}

//! TEXTURE0_FILTER
void DecodeTextureFilter( uint32_t reg,uint32_t* val )
{
	// |-------------|
	// | offset	   |
	// |-------------|
	uint32_t iIndex = ((reg&0x0000FFFF) - TEXTURE0_FILTER) / 0x20;
	uint32_t iLodBias = (*val & 0x00000FFF);
	uint32_t iConstTwo = (*val & 0x0000F000);
	uint32_t iMinFilter = (*val & 0x00FF0000)>>16;
	uint32_t iMagFilter = (*val & 0xFF000000)>>24;

	DUMP_COMMAND( ("\tTexture %i Filter: Lod Bias %d\n", iIndex, iLodBias) );
	switch( iMinFilter )
	{
	case HTF_NEAREST:
		DUMP_COMMAND( ("\t                 : Min Filter  NEAREST\n") ); break;
	case HTF_LINEAR:
		DUMP_COMMAND( ("\t                 : Min Filter  LINEAR\n") ); break;
	case HTF_NEAREST_NEAREST:
		DUMP_COMMAND( ("                 : Min Filter  NEAREST_NEAREST\n") ); break;
	case HTF_LINEAR_NEAREST:
		DUMP_COMMAND( ("\t                 : Min Filter  LINEAR_NEAREST\n") ); break;
	case HTF_NEAREST_LINEAR:
		DUMP_COMMAND( ("\t                 : Min Filter  NEAREST_LINEAR\n") ); break;
	case HTF_LINEAR_LINEAR:
		DUMP_COMMAND( ("\t                 : Min Filter  LINEAR_LINEAR\n") ); break;
	}
	switch( iMagFilter )
	{
	case HTF_NEAREST:
		DUMP_COMMAND( ("\t                 : Mag Filter  NEAREST\n") ); break;
	case HTF_LINEAR:
		DUMP_COMMAND( ("\t                 : Mag Filter  LINEAR\n") ); break;
	case HTF_NEAREST_NEAREST:
		DUMP_COMMAND( ("\t                 : Mag Filter  NEAREST_NEAREST\n") ); break;
	case HTF_LINEAR_NEAREST:
		DUMP_COMMAND( ("\t                 : Mag Filter  LINEAR_NEAREST\n") ); break;
	case HTF_NEAREST_LINEAR:
		DUMP_COMMAND( ("\t                 : Mag Filter  NEAREST_LINEAR\n") ); break;
	case HTF_LINEAR_LINEAR:
		DUMP_COMMAND( ("\t                 : Mag Filter  LINEAR_LINEAR\n") ); break;
	}

	if( iConstTwo != 0x2000 )
	{
		DUMP_COMMAND( ("\t                  : Const Two not 2 !!! its 0x%x\n", iConstTwo) );
	}
}


//! TEXTURE0_SIZE_DP
void DecodeTextureSizeWH( uint32_t reg,uint32_t* val )
{
	// |-------------|
	// | width<<16|height	   |
	// |-------------|
	uint32_t iIndex = ((reg&0x0000FFFF) - TEXTURE0_SIZE_WH) / 0x20;
	uint32_t iWidth= (*val & 0xFFFF0000)>>16;
	uint32_t iHeight = (*val & 0x0000FFFF);

	DUMP_COMMAND( ("\tTexture %i Size Width %d Height %d\n", iIndex, iWidth, iHeight) );
}
//! TEXTURE0_BORDER_COLOUR
void DecodeTextureBorderColour( uint32_t reg,uint32_t* val )
{
	// -------------|
	//  colour	    |
	// -------------|
	uint32_t iIndex = ((reg&0x0000FFFF) - TEXTURE0_SIZE_WH) / 0x20;
	uint32_t iCol= *val;

	DUMP_COMMAND( ("\tTexture %i Border Colour 0x%x\n", iIndex, iCol) );
}

//! TEXTURE0_COLOURKEYCONTROL
void DecodeTextureColourKeyControl( uint32_t reg,uint32_t* val )
{
	// |-------------|
	// | width<<16|height	   |
	// |-------------|
	uint32_t iIndex = ((reg&0x0000FFFF) - TEXTURE0_COLOURKEYCONTROL) / 0x4;
	uint32_t iVal= (*val);

	DUMP_COMMAND( ("\tTexture Colour Key Control %d\n", iIndex, iVal) );
}
//! VERTEX_DATA_4F
void DecodeVertexData4F( uint32_t reg,uint32_t* val )
{
	//|-----|-----|-----|-----|
	//| X   | Y   | Z   | W   |
	//|-----|-----|-----|-----|
	uint32_t iIndex = ((reg&0x0000FFF0) - VERTEX_DATA_4F_X) / 0x10;
	float fX = ((float*)val)[0];
	float fY = ((float*)val)[1];
	float fZ = ((float*)val)[2];
	float fW = ((float*)val)[3];

	DUMP_COMMAND( ("\tDecodeVertexData4F %i < %f, %f, %f, %f>\n", iIndex, fX, fY, fZ, fW) );

}


//! VERTEX_DATA_4F_X
void DecodeVertexData4F_X( uint32_t reg,uint32_t* val )
{
	//|-----|
	//| x   |
	//|-----|
	uint32_t iIndex = ((reg&0x0000FFF0) - VERTEX_DATA_4F_X) / 0x10;
	float fX = *((float*)val);

	DUMP_COMMAND( ("\tDecodeVertexData4F %i X - %f\n", iIndex, fX) );
}
//! VERTEX_DATA_4F_Y
void DecodeVertexData4F_Y( uint32_t reg,uint32_t* val )
{
	//|-----|
	//| Y   |
	//|-----|
	uint32_t iIndex = ((reg&0x0000FFF0) - VERTEX_DATA_4F_X) / 0x10;
	float fY = *((float*)val);

	DUMP_COMMAND( ("\tDecodeVertexData4F %i Y - %f\n", iIndex, fY) );
}
//! VERTEX_DATA_4F_Z
void DecodeVertexData4F_Z( uint32_t reg,uint32_t* val )
{
	//|-----|
	//| Z   |
	//|-----|
	uint32_t iIndex = ((reg&0x0000FFF0) - VERTEX_DATA_4F_X) / 0x10;
	float fZ = *((float*)val);

	DUMP_COMMAND( ("\tDecodeVertexData4F %i Z - %f\n", iIndex, fZ) );
}
//! VERTEX_DATA_4F_W
void DecodeVertexData4F_W( uint32_t reg,uint32_t* val )
{
	//|-----|
	//| W   |
	//|-----|
	uint32_t iIndex = ((reg&0x0000FFF0) - VERTEX_DATA_4F_X) / 0x10;
	float fW = *((float*)val);

	DUMP_COMMAND( ("\tDecodeVertexData4F %i W - %f\n", iIndex, fW) );
}



//! FRAGMENT_SHADER_REG not understood yet
void DecodeFragmentShaderSelect( uint32_t reg,uint32_t* val )
{
	// seen 0x2008440 as unknown
	// |-------------|
	// | unknown	   |
	// |-------------|
	DUMP_COMMAND( ("\tFragmentShaderSelect 0x%x\n", *val) );
}
//! BACKEND_SELECT
void DecodeBackendSelect( uint32_t reg,uint32_t* val )
{
	// |-------------|
	// | index << 4  |
	// |-------------|
	uint32_t iIndex = *val >> 4;
	DUMP_COMMAND( ("\tBackendSelect Index %i", iIndex) );
}

//! WRITEBACK_END_LABEL
//! Use a BackendSelect to select the index and then issue this to 
//! the all back end processing (ROP output) before this command to finish
//! Allows you the GPU or CPU to perform a wait for rendering to finish
void DecodeWritebackEndLabel( uint32_t reg,uint32_t* val )
{
	// libGCM has a bug in its implementation I believe...
	// |-------------|
	// | Reference   |
	// |-------------|
	uint32_t iRef = *val;
	DUMP_COMMAND( ("\tWritebackEnd Label 0x%x\n", iRef ) );
}


//! WRITE_TEXTURE_LABEL
void DecodeWriteTexureLabel( uint32_t reg,uint32_t* val )
{
	// |-------------|
	// | Reference   |
	// |-------------|
	uint32_t iRef = *val;
	DUMP_COMMAND( ("\tWriteTexure Label 0x%x\n", iRef ) );
}

//! MSAA_ENABLE
void DecodeMSAAEnable( uint32_t reg,uint32_t* val )
{
	// |-------------|
	// | enable   |
	// |-------------|
	uint32_t iEnable = *val & 0x0000FFFF;
	DUMP_COMMAND( ("\tMSAAEnable %d\n", iEnable ) );
}


//! RENDER_TARGET_HEIGHT
void DecodeRenderTargetHeight( uint32_t reg,uint32_t* val )
{
	// |-------------|
	// | height & 0xFFF   |
	// |-------------|
	uint32_t iHeight = *val & 0x00000FFF;
	DUMP_COMMAND( ("\tRender Target Height %d\n", iHeight ) );
}
//! RESTART_INDEX_ENABLE
void DecodeRestartIndexEnable( uint32_t reg,uint32_t* val )
{
	// |-------------|
	// | enable   |
	// |-------------|
	uint32_t iEnable = *val;
	DUMP_COMMAND( ("\tRestart Index Enable %d\n", iEnable ) );
}

//! RESTART_INDEX
void DecodeRestartIndex( uint32_t reg,uint32_t* val )
{
	// |-------------|
	// |index		 |
	// |-------------|
	uint32_t iIndex = *val;
	DUMP_COMMAND( ("\tRestart Index %d\n", iIndex ) );
}

//! CLEAR_DEPTH_STENCIL
void DecodeClearDepthStencil( uint32_t reg,uint32_t* val )
{
	// |-------------|
	// |clear val   |
	// |-------------|
	uint32_t iClear = *val;
	DUMP_COMMAND( ("\tClear depth Stencil Val = 0x%x\n", iClear ) );
}
//! CLEAR_COLOUR
void DecodeClearColour( uint32_t reg,uint32_t* val )
{
	// |-------------|
	// |clear val   |
	// |-------------|
	uint32_t iClear =*val;
	DUMP_COMMAND( ("\tClear Colour Colour = 0x%x\n", iClear ) );
}

//! CLEAR_SURFACE_MASK
void DecodeClearSurfaceMask( uint32_t reg, uint32_t* val )
{
	// |---------|
	// | mask    |
	// |---------|
	uint32_t iClear = *val;
	DUMP_COMMAND( ("\tClear SurfaceMask : ", iClear ) );
	if( iClear & HCK_Z )
	{
		DUMP_COMMAND( ("Z ") );
	}
	if( iClear & HCK_STENCIL )
	{
		DUMP_COMMAND( (" Stencil ") );
	}
	if( iClear & HCK_RED )
	{
		DUMP_COMMAND( (" Red ") );
	}
	if( iClear & HCK_GREEN )
	{
		DUMP_COMMAND( (" Green ") );
	}
	if( iClear & HCK_BLUE )
	{
		DUMP_COMMAND( (" Blue ") );
	}
	if( iClear & HCK_ALPHA )
	{
		DUMP_COMMAND( (" Alpha ") );
	}
	DUMP_COMMAND( ("\n") );
}

//! POINT_SIZE
void DecodePointSprite( uint32_t reg, uint32_t* val )
{
	// |-------------|
	// |clear val   |
	// |-------------|
	float fSize = *((float*)val);
	DUMP_COMMAND( ("\tPoint Sprite Size %f\n", fSize ) );
}
//! POINT_SPRITE_ENABLE
void DecodePointSpriteEnable( uint32_t reg,uint32_t* val )
{
	// |-------------|
	// | enable    |
	// |-------------|
	uint32_t iEnable = *val;
	DUMP_COMMAND( ("\tPoint Sprite Enable = %d\n", iEnable ) );
}
//! FREQUENCY_DIVIDER_OP
void DecodeFrequencyDividerOp( uint32_t reg,uint32_t* val )
{
	// |-------------|
	// | 16 bit for each stream    |
	// | if the bit is et MODULO math each DIVIDE
	// |-------------|
	uint32_t iOps = *val;
	DUMP_COMMAND( ("\tFrequency Divider Op :\n" ) );
	for( unsigned int i=0;i < 16;i++)
	{
		DUMP_COMMAND( ("\t                     : Stream %i using %s\n", i, (iOps & (HFD_MODULO<<i))?"MODULO" : "DIVIDE"  ) );
	}
}
//! FLUSH_TEXTURE_CACHE
void DecodeFlushTextureCache( uint32_t reg,uint32_t* val )
{
	// |-------------|-------------|
	// | Command     | which    |
	// |-------------|-------------|
	if( *val == HFTC_FRAGMENT )
	{
		DUMP_COMMAND( ("\tFlush Fragment Texture Cache\n") );
	} else if( *val == HFTC_VERTEX )
	{
		DUMP_COMMAND( ("\tFlush Vertex Texture Cache\n") );
	} else
	{
		DUMP_COMMAND( ("\tFlush Texture Cache Unknown 0x%x\n",*val ) );
	}
}

//! SELECT_VERTEX_CONSTANT_WINDOW
void DecodeSelectVertexConstantWindow( uint32_t reg,uint32_t* val )
{
	// there is a windows into the vertex constant register
	// you set the start index using this, you can then write
	// into (I estimate) the next 192 floats just by normal register
	// access. So ice just tacks it on this command while libgcm
	// issues a command directly 'VertexConstantWindow'
	// this is of course all speculation below is how I've observed it work
	// ICE reckons that 8*4 floats is the maximum you can set in one go...

	//--
	// okay ice and libgcm doe things different...
	// ligcm has a payload of 4 bytes with the payload
	// being the index the next UploadVertexConstant will
	// start from.
	// Ice however has a payload >4 bytes with the first
	// int being the index and the rest being the actual
	// data. Essentially UploadVertex Consant isn't needed...
	// you can all do it from select...

	// |-------------|
	// | start index |
	// |-------------|

	uint32_t iIndex = *val;
	DUMP_COMMAND( ("\tSelect Vertex Constant : Start Register = %i\n", iIndex ) );
}
//! VERTEX_CONSTANT_WINDOW
//! Select gives the initial index
void DecodeVertexConstantWindow4F( uint32_t reg,uint32_t* val )
{
	//|-----|-----|-----|-----|
	//| X   | Y   | Z   | W   |
	//|-----|-----|-----|-----|
	uint32_t iIndex = ((reg&0x0000FFF0) - VERTEX_CONSTANT_WINDOW) / 0x10;
	float fX = ((float*)val)[0];
	float fY = ((float*)val)[1];
	float fZ = ((float*)val)[2];
	float fW = ((float*)val)[3];

	DUMP_COMMAND( ("\tVertexData4F Start+%i < %f, %f, %f, %f>\n", iIndex, fX, fY, fZ, fW) );

}

//! VERTEX_CONSTANT_WINDOW
//! Select gives the initial index
void DecodeVertexConstantWindow( uint32_t reg,uint32_t* val )
{
	// TODO improve visuals of this data is a horrible dump
	// |-------------|
	// | float		 |
	// |-------------|
	float fData = *((float*)val);
	DUMP_COMMAND( ("\tVertex Constant: %f\n", fData ) );
}

//! UPLOAD_FRAGMENT_SHADER
void DecodeUploadFragmentShader( uint32_t reg,uint32_t* val )
{
	// |-------------|
	// | offset & 0x1 |
	// |-------------|
	uint32_t iAddr = *val & 0xFFFFFFFE;
	DUMP_COMMAND( ("\tUploadFragmentShader 0x%x\n", iAddr ) );
}
//! VERTEX_CONTROL_0
void DecodeVertexShaderControl0( uint32_t reg,uint32_t* val )
{
	// |-------------|
	// | unknown |
	// |-------------|
	uint32_t iUnknown = *val;
	DUMP_COMMAND( ("\tVertex Shader Control0 - 0x%x\n", iUnknown ) );
}
//! VERTEX_CONTROL_1
void DecodeVertexShaderControl1( uint32_t reg,uint32_t* val )
{
	// |-------------|
	// | unknown |
	// |-------------|
	uint32_t iUnknown = *val;
	DUMP_COMMAND( ("\tVertex Shader Control1 - 0x%x\n", iUnknown ) );
}

//! VERTEX_CONTROL_2
void DecodeVertexShaderControl2( uint32_t reg,uint32_t* val )
{
	// |-------------|
	// | unknown |
	// |-------------|
	uint32_t iUnknown = *val;
	DUMP_COMMAND( ("\tVertex Shader Control2 - 0x%x\n", iUnknown ) );
}

//! VERTEX_CONTROL_3
void DecodeVertexShaderControl3( uint32_t reg,uint32_t* val )
{
	// |-------------|
	// | unknown |
	// |-------------|
	uint32_t iUnknown = *val;
	DUMP_COMMAND( ("\tVertex Shader Control3 - 0x%x\n", iUnknown ) );
}

//! VERTEX_CONTROL_4
void DecodeVertexShaderControl4( uint32_t reg,uint32_t* val )
{
	// |-------------|
	// | unknown |
	// |-------------|
	uint32_t iUnknown = *val;
	DUMP_COMMAND( ("\tVertex Shader Control4 - 0x%x\n", iUnknown ) );
}

//! FRAGMENT_CONTROL_0
void DecodeFragmentShaderControl0( uint32_t reg,uint32_t* val )
{
	int32_t iFSCIndex = ((reg&0xFFFF) - FRAGMENT_CONTROL_0) / 0x4;
	// |-------------|
	// | unknown |
	// |-------------|
	uint32_t iUnknown = *val;
	DUMP_COMMAND( ("\tFragment Shader Control %d - 0x%x\n", iFSCIndex, iUnknown ) );
}
//! FRAGMENT_UPLOAD_LABEL
void DecodeFragmentUploadLabel( uint32_t reg,uint32_t* val )
{
	// |-------------|
	// | unknown so far 0x1 |
	// |-------------|
	uint32_t iUnknown = *val;
	DUMP_COMMAND( ("\tFragment Upload Label- 0x%x\n", iUnknown ) );
}

struct SomeClassOfDeansThatHeCantBeBotheredToNameProperlyButMustWithTheNewCompiler
{
	uint32_t				iRegister;
	void					(*pDecodeFunc)(  uint32_t, uint32_t*);
} s_PushBufferDecodeTable[] = 
{
	{ SET_REFERENCE,				DecodeSetReference },
	{ GCM_INIT_8,					DecodeSemiUnknownCommand },
	{ DMA_SELECT,					DecodeDMASelect },
	{ WAIT_FOR_LABEL,				DecodeWaitForLabel },
	{ WRITE_COMMAND_LABEL,			DecodeWriteCommandLabel },

	{ RSX_NOOP,						DecodeNOOP },
	{ CLEAR,						DecodeClearCommand },

	{ GCM_INIT_9,					DecodeSemiUnknownCommand },
	{ GCM_INIT_10,					DecodeSemiUnknownCommand },
	{ GCM_INIT_11,					DecodeSemiUnknownCommand },
	{ GCM_INIT_1,					DecodeSemiUnknownCommand },
	{ GCM_INIT_2,					DecodeSemiUnknownCommand },

	{ RENDER_TARGET_LOC_1,			DecodeRenderTargetLoc1 },
	{ GCM_INIT_6,					DecodeSemiUnknownCommand },
	{ RENDER_TARGET_LOC_0,			DecodeRenderTargetLoc0 },
	{ GCM_INIT_7,					DecodeSemiUnknownCommand },
	{ RENDER_TARGET_LOC_2,			DecodeRenderTargetLoc2 },
	{ RENDER_TARGET_LOC_3,			DecodeRenderTargetLoc3 },

	{ RENDER_TARGET_RECT_HORIZ,		DecodeRenderTargetRectHoriz },
	{ RENDER_TARGET_RECT_VERT,		DecodeRenderTargetRectVert },
	{ RENDER_TARGET_FORMAT,			DecodeRenderTargetFormat },

	{ RENDER_TARGET0_PITCH,			DecodeRenderTarget0Pitch },
	{ RENDER_TARGET0_OFFSET,		DecodeRenderTarget0Offset },
	{ DEPTH_STENCIL_OFFSET,			DecodeDepthStencilOffset },
	{ RENDER_TARGET1_OFFSET,		DecodeRenderTarget1Offset },
	{ RENDER_TARGET1_PITCH,			DecodeRenderTarget1Pitch },

	{ RENDER_TARGET_SELECT,			DecodeRenderTargetSelect },
	{ DEPTH_STENCIL_PITCH,			DecodeDepthStencilPitch },
	{ INVALIDATE_ZCULL,				DecodeInvalidateZCull },

	{ RENDER_TARGET2_PITCH,			DecodeRenderTarget2Pitch },
	{ RENDER_TARGET3_PITCH,			DecodeRenderTarget3Pitch },
	{ RENDER_TARGET2_OFFSET,		DecodeRenderTarget2Offset },
	{ RENDER_TARGET3_OFFSET,		DecodeRenderTarget3Offset },

	{ RENDER_TARGET_POS_OFFSET,		DecodeRenderTargetPosOffset },

	{ GCM_INIT_15,		DecodeSemiUnknownCommand },
	{ GCM_INIT_16,		DecodeSemiUnknownCommand },
	{ GCM_INIT_17,		DecodeSemiUnknownCommand },
	{ GCM_INIT_18,		DecodeSemiUnknownCommand },
	{ GCM_INIT_19,		DecodeSemiUnknownCommand },
	{ GCM_INIT_20,		DecodeSemiUnknownCommand },
	{ GCM_INIT_21,		DecodeSemiUnknownCommand },
	{ GCM_INIT_22,		DecodeSemiUnknownCommand },
	{ GCM_INIT_23,		DecodeSemiUnknownCommand },

	{ DITHER_ENABLE,			DecodeDitherEnable },
	{ ALPHA_TEST_ENABLE,		DecodeAlphaTestEnable },
	{ ALPHA_FUNC,				DecodeAlphaFunc },
	{ ALPHA_REF,				DecodeAlphaRef },
	{ BLEND_ENABLE,				DecodeBlendEnable },
	{ BLEND_FUNC_ALPHA,			DecodeBlendFuncAlpha },
	{ BLEND_FUNC_COLOUR,		DecodeBlendFuncColour },
	{ BLEND_COLOUR0,			DecodeBlendColour0 },
	{ BLEND_EQUATION,			DecodeBlendEquation },
	{ COLOUR_MASK,				DecodeColourMask },
	{ STENCIL_TEST_ENABLE,		DecodeStencilTestEnable },
	{ STENCIL_WRITE_MASK,		DecodeStencilWriteMask },
	{ STENCIL_FUNC,				DecodeStencilFunc },
	{ STENCIL_REF,				DecodeStencilRef },
	{ STENCIL_MASK,				DecodeStencilMask },

	{ TWO_SIDED_STENCIL_ENABLE,	DecodeTwoSidedStencilTestEnable },
	{ BACK_STENCIL_MASK,		DecodeBackStencilMask },

	{ BACK_STENCIL_OP_FAIL,		DecodeBackStencilOpFail },
	{ BACK_STENCIL_OP_DEPTH_FAIL,		DecodeBackStencilOpDepthFail },
	{ BACK_STENCIL_OP_DEPTH_PASS,		DecodeBackStencilOpDepthPass },
	{ SHADE_MODE,			DecodeShadeMode },
	{ BLEND_ENABLE_MRT,		DecodeBlendEnableMRT },
	{ COLOUR_MASK_MRT,		DecodeColourMaskMRT },
	{ LOGIC_OP_ENABLE,		DecodeLogicEnable },
	{ LOGIC_OP,				DecodeLogicOp },
	{ BLEND_COLOUR1,		DecodeBlendColour1 },
	{ DEPTH_BOUNDS_ENABLE,	DecodeDepthBoundsEnable },
	{ DEPTH_BOUNDS_NEAR,	DecodeDepthBoundsNear },
	{ DEPTH_BOUNDS_FAR,		DecodeDepthBoundsFar },

	{ VIEWPORT_NEAR,		DecodeViewportNear },
	{ VIEWPORT_FAR,			DecodeViewportFar },

	{ LINE_WIDTH,			DecodeLineWidth },

	{ STENCIL_OP_FAIL,			DecodeStencilOpFail },
	{ STENCIL_OP_DEPTH_FAIL,	DecodeStencilOpDepthFail },
	{ STENCIL_OP_DEPTH_PASS,	DecodeStencilOpDepthPass },

	{ SCISSOR_RECT_HORIZ,	DecodeScissorRectHoriz },
	{ SCISSOR_RECT_VERT,	DecodeScissorRectVert },

	{ VIEWPORT_RECT_HORIZ,	DecodeViewportRectHoriz },
	{ VIEWPORT_RECT_VERT,	DecodeViewportRectVert },

	{ VIEWPORT_SCALE_X,	DecodeViewportScaleX },
	{ VIEWPORT_SCALE_Y,	DecodeViewportScaleY },
	{ VIEWPORT_SCALE_Z,	DecodeViewportScaleZ },
	{ VIEWPORT_SCALE_W,	DecodeViewportScaleW },
	{ VIEWPORT_POS_OFFSET_X,	DecodeViewportPosOffsetX },
	{ VIEWPORT_POS_OFFSET_Y,	DecodeViewportPosOffsetY },
	{ VIEWPORT_POS_OFFSET_Z,	DecodeViewportPosOffsetZ },
	{ VIEWPORT_POS_OFFSET_W,	DecodeViewportPosOffsetW },

	{ POLYGON_OFFSET_ENABLE,	DecodePolygonOffsetEnable },
	{ DEPTH_FUNC,	DecodeDepthFunc },
	{ DEPTH_WRITE_ENABLE,	DecodeDepthWriteEnable },
	{ DEPTH_TEST_ENABLE,	DecodeDepthTestEnable },
	{ POLYGON_OFFSET_FACTOR,	DecodePolygonOffsetFactor },
	{ POLYGON_OFFSET_UNITS,	DecodePolygonOffsetUnits },

	{ TEXTURE0_BRILINEAR,	DecodeTextureBrilinear },

	{ FRAGMENT_CONTROL_0,	DecodeFragmentShaderControl0 },
	{ FRAGMENT_CONTROL_1,	DecodeFragmentShaderControl0 },
	{ FRAGMENT_CONTROL_2,	DecodeFragmentShaderControl0 },
	{ FRAGMENT_CONTROL_3,	DecodeFragmentShaderControl0 },
	{ FRAGMENT_CONTROL_4,	DecodeFragmentShaderControl0 },
	{ FRAGMENT_CONTROL_5,	DecodeFragmentShaderControl0 },
	{ FRAGMENT_CONTROL_6,	DecodeFragmentShaderControl0 },
	{ FRAGMENT_CONTROL_7,	DecodeFragmentShaderControl0 },
	{ FRAGMENT_CONTROL_8,	DecodeFragmentShaderControl0 },
	{ FRAGMENT_CONTROL_9,	DecodeFragmentShaderControl0 },
	{ UPLOAD_VERTEX_SHADER, DecodeUploadVertexShader },

	// range for 16 streams
	{ VERTEXSTREAM0_OFFSET,	DecodeVertexDataOffset },

	// range for 16 streams
	{ VERTEXSTREAM0_FORMAT,	DecodeVertexDataArray },

	{ DRAW,					DecodeDraw },
	{ VERTEX_ARRAY_PARAMS,	DecodeVertexArrayParams },
	{ INDEX_ARRAY_OFFSET,	DecodeIndexArrayOffset },
	{ INDEX_ARRAY_TYPE,		DecodeIndexArrayType },
	{ INDEX_ARRAY_PARAMS,	DecodeIndexArrayParams },


	{ CULL_FACE,	DecodeCullFace },
	{ FRONT_FACE,	DecodeFrontFace },
	{ CULL_FACE_ENABLE,	DecodeCullFaceEnable },


	{ FLUSH_0,	DecodeFlush0 },
	{ FLUSH_1,	DecodeFlush1 },

	{ TIME_STAMP,	DecodeTimeStamp },

	// range for 16 textures
	{ TEXTURE0_SIZE_DP,			DecodeTextureSizeDP },
	{ TEXTURE0_OFFSET,			DecodeTextureOffset },
	{ TEXTURE0_FORMAT,			DecodeTextureFormat },
	{ TEXTURE0_ADDRESS,			DecodeTextureAddress },
	{ TEXTURE0_CONTROL,			DecodeTextureControl },
	{ TEXTURE0_REMAP,			DecodeTextureRemap },
	{ TEXTURE0_FILTER,			DecodeTextureFilter },
	{ TEXTURE0_SIZE_WH,			DecodeTextureSizeWH },
	{ TEXTURE0_BORDER_COLOUR,	DecodeTextureBorderColour },

	// range for 16 streams
	{ VERTEX_DATA_4F_X,			DecodeVertexData4F_X },
	{ VERTEX_DATA_4F_Y,			DecodeVertexData4F_Y },
	{ VERTEX_DATA_4F_Z,			DecodeVertexData4F_Y },
	{ VERTEX_DATA_4F_W,			DecodeVertexData4F_Y },

	{ TEXTURE0_COLOURKEYCONTROL,	DecodeSemiUnknownCommand },

	{ FRAGMENT_SHADER_ADDRESS,	DecodeFragmentShaderSelect },

	{ BACKEND_SELECT,		DecodeBackendSelect },
	{ WRITEBACK_END_LABEL,	DecodeWritebackEndLabel },
	{ WRITE_TEXTURE_LABEL,	DecodeWriteTexureLabel },

	{ MSAA_ENABLE,	DecodeMSAAEnable },
	{ RENDER_TARGET_HEIGHT,	DecodeRenderTargetHeight },
	{ GCM_INIT_14,	DecodeSemiUnknownCommand },


	{ RESTART_INDEX_ENABLE,		DecodeRestartIndexEnable },
	{ RESTART_INDEX,			DecodeRestartIndex },
	{ CLEAR_DEPTH_STENCIL,		DecodeClearDepthStencil },
	{ CLEAR_COLOUR,				DecodeClearColour },
	{ CLEAR,		DecodeClearSurfaceMask },
	{ GCM_INIT_24,	DecodeSemiUnknownCommand },

	{ VERTEX_CONTROL_3,	DecodeVertexShaderControl3 },
	{ VERTEX_CONTROL_4,	DecodeVertexShaderControl4 },

	{ POINT_SIZE,	DecodePointSprite },
	{ POINT_SPRITE_ENABLE,	DecodePointSpriteEnable },

	{ FREQUENCY_DIVIDER_OP,	DecodeFrequencyDividerOp },
	{ FLUSH_TEXTURE_CACHE,	DecodeFlushTextureCache },

	{ VERTEX_CONTROL_2,	DecodeVertexShaderControl2 },
	{ SELECT_VERTEX_CONSTANT_WINDOW,	DecodeSelectVertexConstantWindow },
	// range for 192 floats
	{ VERTEX_CONSTANT_WINDOW,	DecodeVertexConstantWindow },

	{ FRAGMENT_SHADER_CONTROL,	DecodeUploadFragmentShader },
	{ VERTEX_CONTROL_0,	DecodeVertexShaderControl0 },
	{ VERTEX_CONTROL_1,	DecodeVertexShaderControl1 },

	{ FRAGMENT_UPLOAD_LABEL,	DecodeFragmentUploadLabel },

};

// true if known
// normal search and decode does range checkin, this doesn't as its used
// by normal search and decode
bool NoRangeSearchAndDecodeRegister( uint32_t iRegister, uint32_t* pData )
{
	// linear search but this doesn't need to be fast
	for(unsigned int i=0;i < sizeof(s_PushBufferDecodeTable)/ sizeof(s_PushBufferDecodeTable[0]);i++)
	{
		if( iRegister == s_PushBufferDecodeTable[i].iRegister )
		{
			s_PushBufferDecodeTable[i].pDecodeFunc( iRegister, pData );
			return true;
		}
	}

	// unknown
	DecodeUnknownCommand( iRegister, (uint32_t*)pData );
	return false;
}
// true if known
bool SearchAndDecodeRegister( uint32_t command, uint32_t iRegister, uint32_t* pData )
{
	uint32_t iPayload = (command & 0x0FFF0000) >> 16;
	// check known ranges

	// vertex shader
	int32_t iVertShaderIndex = ((iRegister&0xFFFF) - UPLOAD_VERTEX_SHADER);
	if( iVertShaderIndex>=0 && iVertShaderIndex<VERTEX_SHADER_SIZE)
	{
		DecodeUploadVertexShader( iRegister, pData );
		return true;
	}
	// vertex data offset
	int32_t iVertDataOffsetIndex = ((iRegister&0xFFFF) - VERTEXSTREAM0_OFFSET) / 0x4;
	if( iVertDataOffsetIndex>=0 && iVertDataOffsetIndex <= VERTEX_STREAMS)
	{
		DecodeVertexDataOffset( iRegister, pData );
		return true;
	}
	// vertex array offset
	int32_t iVertDataArrayIndex = ((iRegister&0xFFFF) - VERTEXSTREAM0_FORMAT) / 0x4;
	if( iVertDataArrayIndex>=0 && iVertDataArrayIndex <= VERTEX_STREAMS)
	{
		DecodeVertexDataArray( iRegister, pData );
		return true;
	}
	// TEXTURE0_COLOURKEYCONTROL
	int32_t iTexSizeckcIndex = ((iRegister&0xFFFF) - TEXTURE0_COLOURKEYCONTROL);
	if( iTexSizeckcIndex >=0 && iTexSizeckcIndex  <= TEXTURE_UNITS)
	{
		DecodeTextureColourKeyControl( iRegister, pData );
		return true;
	}

	// texture size dp
	int32_t iTexSizedpIndex = ((iRegister&0xFFFF) - TEXTURE0_SIZE_DP);
	if( iTexSizedpIndex>=0 && iTexSizedpIndex <= TEXTURE_UNITS)
	{
		DecodeTextureSizeDP( iRegister, pData );
		return true;
	}
	// TextureBrilinear ICE seems to set it to a constant (0x2dc4)
	int32_t iTexUnknownIndex = ((iRegister&0xFFFF) - TEXTURE0_BRILINEAR);
	if( iTexUnknownIndex>=0 && iTexUnknownIndex <= TEXTURE_UNITS)
	{
		DecodeTextureBrilinear( iRegister, pData );
		return true;
	}
	// texture block
	int32_t iTexIndex = ((iRegister&0xFFFF) - TEXTURE0_OFFSET) / 0x20;
	if( iTexIndex>=0 && iTexIndex<= TEXTURE_UNITS)
	{			
		switch( (iRegister & 0x1F) )
		{
		case (TEXTURE0_OFFSET & 0x1F):
			DecodeTextureOffset( iRegister, pData ); break;
		case (TEXTURE0_FORMAT & 0x1F):
			DecodeTextureFormat( iRegister, pData ); break;
		case (TEXTURE0_ADDRESS & 0x1F):
			DecodeTextureAddress( iRegister, pData ); break;
		case (TEXTURE0_CONTROL & 0x1F):
			DecodeTextureControl( iRegister, pData ); break;
		case (TEXTURE0_REMAP & 0x1F):
			DecodeTextureRemap( iRegister, pData ); break;
		case (TEXTURE0_FILTER & 0x1F):
			DecodeTextureFilter( iRegister, pData ); break;
		case (TEXTURE0_SIZE_WH & 0x1F):
			DecodeTextureSizeWH( iRegister, pData ); break;
		case (TEXTURE0_BORDER_COLOUR & 0x1F):
			DecodeTextureBorderColour( iRegister, pData ); break;
		}
		return true;
	}
	// vertex data 
	int32_t iVertDataIndex = ((iRegister&0xFFF0) - VERTEX_DATA_4F_X) / 0x10;
	if( iVertDataIndex>= 0 && iVertDataIndex <= VERTEX_STREAMS )
	{
		if( ((iRegister & 0xF) == 0 ) && (iPayload == 0x10) )
		{
			DecodeVertexData4F( iRegister, pData );
		} else
		{
			switch( (iRegister & 0xF) )
			{
			case 0:
				DecodeVertexData4F_X( iRegister, pData ); break;
			case 0x4:
				DecodeVertexData4F_Y( iRegister, pData ); break;
			case 0x8:
				DecodeVertexData4F_Z( iRegister, pData ); break;
			case 0xC:
				DecodeVertexData4F_W( iRegister, pData ); break;
			}
		}
		return true;
	}
	// vertex constant window 
	int32_t iVertConstantIndex = ((iRegister&0xFFFF) - VERTEX_CONSTANT_WINDOW);
	if( iVertConstantIndex>= 0 && iVertConstantIndex <= VERTEX_CONSTANT_WINDOW_SIZE )
	{
		if( ((iRegister & 0xF) == 0 ) && (iPayload >= 0x10+0x4) )
		{
			DecodeVertexConstantWindow4F( iRegister, pData );
		} else if( iPayload < 0x10 )
		{

			DecodeVertexConstantWindow( iRegister, pData );
		}
		return true;
	}

	return NoRangeSearchAndDecodeRegister( iRegister, pData );
}

void DisassablePushBuffer( char* pPushBuffer, unsigned int iSize )
{
	// this assumes the address given is a valid push buffer is then goes through
	// and prints each command and its parameter using printf

	const char* pEndBuffer = pPushBuffer + iSize;

	while( pPushBuffer < pEndBuffer )
	{
		unsigned int command = *((uint32_t*)pPushBuffer);
		unsigned int iPayload = (command & 0x0FFF0000) >> 16;
		unsigned int iRegister = (command & 0x0000FFFF);

		if( iRegister == RSX_NOOP )
		{
			// special case NOOP as its used for debug
			// heresy debug marker
			if( ((uint32_t*)pPushBuffer)[1] == 0xDEEDEE00 )
			{
				char txt[9];
				for( int swp = 0; swp < 8;++swp )
				{
					if( swp < 4 )
					{
						txt[3-swp] = *(pPushBuffer+8+swp);
					} else
					{
						txt[7-swp+4] = *(pPushBuffer+8+swp);
					}
				}
				txt[8] = 0;
				DUMP_COMMAND( ("Heresy Debug - %s\n", txt) );
			} else
			{
				DUMP_COMMAND( ("RSX_NOOP of %i bytes\n", iPayload) );
			}

		} else
		if( iRegister == 0 )
		{
			DecodeReturn( command );
			iPayload = 0;
		} else if( command & 0x20000000 )
		{
			DecodeJump( command );
			iPayload = 0;
		} else if( command & 0x40000000 )
		{
			if( iPayload > 0x4 )
			{
				DUMP_COMMAND( ("Non Incrementing Batched Command\n") );
			}
			uint32_t* pParams = ((uint32_t*)pPushBuffer)+1;
			// non register incrementing set
			for( unsigned i=0; i < iPayload/0x4;i++ )
			{
				// obvious as the register doesn't change this could be made
				// much more efficient... but hey its a dissambler...
				if( SearchAndDecodeRegister( command, iRegister, pParams ) == false )
				{
					break;
				}
				pParams++;
			}
			if( iPayload > 0x4 )
			{
				DUMP_COMMAND( ("End Batched Command\n") );
			}
		} else
		{
			if( iPayload > 0x4 )
			{
				DUMP_COMMAND( ("Batched Command\n") );
			}
			uint32_t* pParams = ((uint32_t*)pPushBuffer)+1;
			for( unsigned i=0; i < iPayload/0x4;i++ )
			{
				if( SearchAndDecodeRegister( command, iRegister, pParams ) == false )
				{
					break;
				}
				iRegister+=4;
				pParams++;
			}
			if( iPayload > 0x4 )
			{
				DUMP_COMMAND( ("End Batched Command\n") );
			}
		}

		pPushBuffer+=4;			// skip command
		pPushBuffer+=iPayload;	// skip payload
	}
}
