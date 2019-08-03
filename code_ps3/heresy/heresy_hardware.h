#if !defined( HERESY_HARDWARE_H )
#define HERESY_HARDWARE_H


enum HERESY_NUMBERS
{
	VERTEX_STREAMS	= 16,		// 16 vertex streams
	TEXTURE_UNITS	= 16,		// 16 texture units
	VERTEX_CONSTANT_WINDOW_SIZE = 192,	// 192 float vertex constant window
	VERTEX_SHADER_SIZE	= (0x1680 - 0x0B80),
};

enum HERESY_PUSHBUFFER_CONTROL
{
	HPBC_CALL		 = 0x00000002,
	HPBC_RET		 = 0x00020000,
	HPBC_JUMP		 = 0x20000000,
	HPBC_NOINCREMENT = 0x40000000,
};

enum HERESY_RAM_LOCATION
{
	HRL_LOCAL_GDDR	= 0x0,
	HRL_MAIN_XDDR	= 0x1,
};

enum HERESY_FRAGMENT_RAM_LOCATION
{
	HFRL_LOCAL_GDDR	= 0x1,
	HFRL_MAIN_XDDR	= 0x2,
};

enum HERESY_RENDER_TARGET_FORMAT
{
	HRTF_NONE				= 0x00, //!< Guesswork!
	HRTF_X1R5G5B5_Z1R5G5B5 	= 0x01,
	HRTF_X1R5G5B5_O1R5G5B5 	= 0x02,
	HRTF_R5G6B5				= 0x03,
	HRTF_X8R8G8B8_Z8R8G8B8	= 0x04,
	HRTF_X8R8G8B8_O8R8G8B8 	= 0x05,
// 0x6
// 0x7
	HRTF_A8R8G8B8			= 0x08,
	HRTF_B8					= 0x09,
	HRTF_G8B8				= 0x0A,
	HRTF_F_W16Z16Y16X16		= 0x0B,
	HRTF_F_W32Z32Y32X32		= 0x0C,
	HRTF_F_X32				= 0x0D,
	HRTF_X8B8G8R8_Z8B8G8R8	= 0x0E,
	HRTF_X8B8G8R8_O8B8G8R8	= 0x0F,
	HRTF_A8B8G8R8			= 0x10,
};

enum HERESY_DEPTH_FORMAT
{
	HDF_NONE				= 0x0,	//!< Guesswork!
	HDF_Z16					= 0x1,
	HDF_Z24S8				= 0x2,
};

enum HERESY_RENDER_TARGET_FLAGS
{
	HRTF_PITCH				= (1 << 0),
	HRTF_SWIZZLE			= (1 << 1),
};

enum HERESY_RENDER_TARGET_MSSA
{
	HRTM_1_SAMPLE_CENTER			=	0x0,
// 0x1 horizontal?
// 0x2 vertical?
	HRTM_2_SAMPLE_DIAGONAL			=	0x3,
	HRTM_4_SAMPLE_SQUARE			=	0x4,
	HRTM_4_SAMPLE_SQUARE_ROTATED	=	0x5,
};
enum HERESY_RENDER_TARGET_SELECT
{
// NOTE I expect its actually a bit field and a flags like so
	HTRS_TARGET_0		= 0x01,
	HTRS_TARGET_1		= 0x02,
	HTRS_TARGET_2		= 0x04,
	HTRS_TARGET_3		= 0x08,
	HRTS_MRT_ENABLE		= 0x10,
// but libGCM define them like this
//	HRTS_TARGET_0		= 0x01,
	HRTS_TARGET_01		= 0x02,
	HRTS_TARGET_MRT01	= 0x13,
	HRTS_TARGET_MRT012	= 0x17,
	HRTS_TARGET_MRT0123	= 0x1e,

};
enum HERESY_FUNC
{
    // SetAlphaFunc/DepthFunc/StencilFunc
    HF_NEVER				= 0x0200,
    HF_LESS					= 0x0201,
    HF_EQUAL				= 0x0202,
    HF_LEQUAL				= 0x0203,
    HF_GREATER				= 0x0204,
    HF_NOTEQUAL				= 0x0205,
    HF_GEQUAL				= 0x0206,
    HF_ALWAYS				= 0x0207,
};

enum HERESY_BLEND_FUNC
{
	// SetBlendFunc
    HBF_ZERO							= 0x0000,
    HBF_ONE								= 0x0001,
    HBF_SRC_COLOR						= 0x0300,
    HBF_ONE_MINUS_SRC_COLOR				= 0x0301,
    HBF_SRC_ALPHA						= 0x0302,
    HBF_ONE_MINUS_SRC_ALPHA				= 0x0303,
    HBF_DST_ALPHA						= 0x0304,
    HBF_ONE_MINUS_DST_ALPHA				= 0x0305,
    HBF_DST_COLOR						= 0x0306,
    HBF_ONE_MINUS_DST_COLOR				= 0x0307,
    HBF_SRC_ALPHA_SATURATE				= 0x0308,
    HBF_CONSTANT_COLOR					= 0x8001,
    HBF_ONE_MINUS_CONSTANT_COLOR		= 0x8002,
    HBF_CONSTANT_ALPHA					= 0x8003,
    HBF_ONE_MINUS_CONSTANT_ALPHA		= 0x8004,
};

enum HERESY_BLEND_EQUATION
{
    HBE_BLEND_COLOR                    = 0x8005, //!< Have no idea??
    HBE_FUNC_ADD                       = 0x8006,
    HBE_MIN                            = 0x8007,
    HBE_MAX                            = 0x8008,
    HBE_BLEND_EQUATION                 = 0x8009, //!< Have no idea?
    HBE_FUNC_SUBTRACT                  = 0x800A,
    HBE_FUNC_REVERSE_SUBTRACT          = 0x800B,
};

enum HERESY_STENCIL_OP
{
	// SetStencilOp
    HSO_KEEP                        = 0x1E00,
    HSO_REPLACE                     = 0x1E01,
    HSO_INCR                        = 0x1E02,
    HSO_DECR                        = 0x1E03,
    HSO_INCR_WRAP                   = 0x8507,
    HSO_DECR_WRAP                   = 0x8508,
};

enum HERESY_POYLGON_MODE
{
	//SetPolygonMode
	HPM_POINT						= 0x1B00,
	HPM_LINE						= 0x1B01,
	HPM_FILL						= 0x1B02,
};

enum HERESY_SHADE_MODE
{
	// SetShadeMode
    HSM_FLAT                           = 0x1D00,
    HSM_SMOOTH                         = 0x1D01,
};

enum HERESY_LOGIC_OP
{
    HLO_CLEAR                          = 0x1500,
    HLO_AND                            = 0x1501,
    HLO_AND_REVERSE                    = 0x1502,
    HLO_COPY                           = 0x1503,
    HLO_AND_INVERTED                   = 0x1504,
    HLO_NOOP                           = 0x1505,
    HLO_XOR                            = 0x1506,
    HLO_OR                             = 0x1507,
    HLO_NOR                            = 0x1508,
    HLO_EQUIV                          = 0x1509,
    HLO_INVERT                         = 0x150A,
    HLO_OR_REVERSE                     = 0x150B,
    HLO_COPY_INVERTED                  = 0x150C,
    HLO_OR_INVERTED                    = 0x150D,
    HLO_NAND                           = 0x150E,
    HLO_SET                            = 0x150F,
};

enum HERESY_VERTEX_FORMAT
{
	HVF_NORMALISED_INT16				= 0x1,		//!< 16 bit <-> -1, 1
	HVF_FLOAT							= 0x2,		//!< 32 bit float
	HVF_FLOAT16							= 0x3,		//!< 16 bit float
	HVF_NORMALISED_UNSIGNED_BYTE		= 0x4,		//!< 8 bit <-> -1, 1
	HVF_INT16							= 0x5,		//!< 16 bit <-> -32K, 32K
	HVF_11_11_10						= 0x6,		//!< 32 bit with 11,11,10
	HVF_UNSIGNED_BYTE					= 0x7,		//!< 8 bit <-> 0, 255

};

enum HERESY_PRIM_TYPE
{
    HPT_POINTS							= 0x1,
    HPT_LINES							= 0x2,
    HPT_LINE_LOOP						= 0x3,
    HPT_LINE_STRIP						= 0x4,
    HPT_TRIANGLES						= 0x5,
    HPT_TRIANGLE_STRIP					= 0x6,
    HPT_TRIANGLE_FAN					= 0x7,
    HPT_QUADS							= 0x8,
    HPT_QUAD_STRIP						= 0x9,
    HPT_POLYGON							= 0x10,
};

enum HERESY_INDEX_TYPE
{
	HIT_16_BIT							= 0x10,
	HIT_32_BIT							= 0x00,
};

enum HERESY_CULL_FACE
{
	HCF_FRONT							= 0x0404,
	HCF_BACK							= 0x0405,
	HCF_FRONT_AND_BACK					= 0x0408,
};

enum HERESY_FRONT_FACE
{
    HFF_CW                             = 0x0900,
    HFF_CCW                            = 0x0901,
};

enum HERESY_TEXTURE_FORMAT
{
	// these are ored with main format
	HTF_LOCAL_RAM					 = 0x0001,
	HTF_MAIN_RAM					 = 0x0002,
	HTF_CUBEMAP						 = 0x0004,
	HTF_UNKNOWN						 = 0x0008, // must always be ON so far...
	HTF_1D							 = 0x0010,
	HTF_2D							 = 0x0020,
	HTF_3D							 = 0x0030,
	HTF_DIM_MASK					 = 0x0030, // mask to get dimensions
	HTF_UNNORMALISED				 = 0x2000,
	HTF_LINEAR_MEM					 = 0x4000,

	// main format
	HTF_B8                           = 0x8100,
	HTF_A1R5G5B5                     = 0x8200,
	HTF_A4R4G4B4                     = 0x8300,
	HTF_R5G6B5                       = 0x8400,
	HTF_A8R8G8B8                     = 0x8500,
	HTF_COMPRESSED_DXT1              = 0x8600,
	HTF_COMPRESSED_DXT23             = 0x8700,
	HTF_COMPRESSED_DXT45             = 0x8800,
	HTF_G8B8                         = 0x8B00,
	HTF_R6G5B5                       = 0x8F00,
	HTF_DEPTH24_D8                   = 0x9000,
	HTF_DEPTH24_D8_FLOAT             = 0x9100,
	HTF_DEPTH16                      = 0x9200,
	HTF_DEPTH16_FLOAT                = 0x9300,
	HTF_X16                          = 0x9400,
	HTF_Y16_X16                      = 0x9500,
	HTF_R5G5B5A1                     = 0x9700,
	HTF_COMPRESSED_HILO8             = 0x9800,
	HTF_COMPRESSED_HILO_S8           = 0x9900,
	HTF_W16_Z16_Y16_X16_FLOAT        = 0x9A00,
	HTF_W32_Z32_Y32_X32_FLOAT        = 0x9B00,
	HTF_X32_FLOAT                    = 0x9C00,
	HTF_D1R5G5B5                     = 0x9D00,
	HTF_D8R8G8B8                     = 0x9E00,
	HTF_Y16_X16_FLOAT                = 0x9F00,

	// mask to remove all the non main format bits..
	HTF_FORMAT_MASK					 = 0x9F00,
};

enum HERESY_TEXTURE_REMAP_BIAS
{
	HTRB_UNSIGNED_REMAP_BIAS		= 0x100
};

enum HERESY_TEXTURE_ADDRESS
{
	HTA_WRAP						= 1,
	HTA_MIRROR						= 2,
	HTA_CLAMP_TO_EDGE 				= 3,
	HTA_BORDER 						= 4,
	HTA_CLAMP 						= 5,
	HTA_MIRROR_ONCE_CLAMP_TO_EDGE 	= 6,
	HTA_MIRROR_ONCE_BORDER			= 7,
	HTA_MIRROR_ONCE_CLAMP			= 8,
};

enum HERESY_TEXTURE_ZFUNC
{
	HTZF_NEVER			= 0,
	HTZF_LESS			= 1,
	HTZF_EQUAL			= 2,
	HTZF_LEQUAL			= 3,
	HTZF_GREATER		= 4,
	HTZF_NOTEQUAL		= 5,
	HTZF_GEQUAL			= 6,
	HTZF_ALWAYS			= 7,
};

enum HERESY_TEXTURE_ANISOTROPY
{
	// SetTextureControl
	HTA_MAX_ANISO_1	= (0),
	HTA_MAX_ANISO_2	= (1),
	HTA_MAX_ANISO_4	= (2),
	HTA_MAX_ANISO_6	= (3),
	HTA_MAX_ANISO_8	= (4),
	HTA_MAX_ANISO_10	= (5),
	HTA_MAX_ANISO_12	= (6),
	HTA_MAX_ANISO_16	= (7),
};

enum HERESY_TEXTURE_FILTER
{
	HTF_NEAREST				= 1,
	HTF_LINEAR				= 2,
	HTF_NEAREST_NEAREST		= 3,
	HTF_LINEAR_NEAREST		= 4,
	HTF_NEAREST_LINEAR		= 5,
	HTF_LINEAR_LINEAR		= 6,
};

enum HERESY_CLEAR_MASK
{
	HCK_Z			= (1<<0),
	HCK_STENCIL		= (1<<1),
	HCK_RED			= (1<<4),
	HCK_GREEN		= (1<<5),
	HCK_BLUE 		= (1<<6),
	HCK_ALPHA		= (1<<7),

	HCK_COLOUR			= (HCK_RED | HCK_GREEN | HCK_BLUE | HCK_ALPHA),
	HCK_DEPTHSTENCIL	= (HCK_Z | HCK_STENCIL),
	HCK_MASK   		= (0x7f),

};

enum HERESY_FLUSH_TEXTURE_CACHE
{
	HFTC_FRAGMENT	= 0x1,
	HFTC_VERTEX		= 0x2
};

enum HERESY_FREQUENCY_DIVIDER
{
	HFD_MODULO		= 0x1
};

// this enum needs fixing now I've noticed the pattern
enum RSX_REGISTERS
{
	SET_REFERENCE				= 0x0050,

	// these are important feedback registers, I don't completely understand them yet...

	GCM_INIT_8					= 0x0060, // 4 bytes of 0x666b0001
	DMA_SELECT					= 0x0064,
	WAIT_FOR_LABEL				= 0x0068,
	WRITE_COMMAND_LABEL			= 0x006C,

	RSX_NOOP					= 0x0100,
	PERF_TRIG0					= 0x0110,
	PERF_TRIG1					= 0x0140,

	GCM_INIT_9					= 0x01A4, // 4 bytes of 0x666B0000
	GCM_INIT_10					= 0x01A8, // 4 bytes of 0x666C0000
	GCM_INIT_11					= 0x01AC, // 4 bytes of 0x0

// GCM INIT_0 starts with 0x40000, 4 bytes of 0x31337
	GCM_INIT_1					= 0x0180, // 4 byte of 0x666a0000
	GCM_INIT_2					= 0x0184, // 8 byte of 0xfeed0000, 0xfeed0001
// GCM_INIT_3 sets RENDER_TARGET_LOC_0 to 0xfeed0000 (OS reserved memory?)
// GCM_INIT_4 sets RENDER_TARGET_LOC_1 to 0xfeed0000 (OS reserved memory?)
// GCM_INIT_5 sets RENDER_TARGET_LOC_2 to 0xfeed0000, 0xfeed0000 (OS reserved memory?)

	RENDER_TARGET_LOC_1			= 0x018C,
	GCM_INIT_6					= 0x0190, // 4 byte of 0x0
	RENDER_TARGET_LOC_0			= 0x0194,
	GCM_INIT_7					= 0x019C, // 8 byte of 0xfeed0000, 0xfeed0001

	RENDER_TARGET_LOC_2			= 0x01B4,
	RENDER_TARGET_LOC_3			= 0x01B8,

	RENDER_TARGET_RECT_HORIZ	= 0x0200,
	RENDER_TARGET_RECT_VERT		= 0x0204,
	RENDER_TARGET_FORMAT		= 0x0208,

	RENDER_TARGET0_PITCH		= 0x020C,
	RENDER_TARGET0_OFFSET		= 0x0210,
	DEPTH_STENCIL_OFFSET		= 0x0214,
	RENDER_TARGET1_OFFSET		= 0x0218,
	RENDER_TARGET1_PITCH		= 0x021C,
	RENDER_TARGET_SELECT		= 0x0220,
	DEPTH_STENCIL_PITCH			= 0x022C,

	INVALIDATE_ZCULL			= 0x0234,

	RENDER_TARGET2_PITCH		= 0x0280,
	RENDER_TARGET3_PITCH		= 0x0284,
	RENDER_TARGET2_OFFSET		= 0x0288,
	RENDER_TARGET3_OFFSET		= 0x028C,

	// GCM_INIT_13 sets SURFACE_POS_OFFSET to 0x0 
	RENDER_TARGET_POS_OFFSET			= 0x02B8,
	GCM_INIT_15					= 0x02BC,	// 4 bytes of 0
	GCM_INIT_16					= 0x02C0,	// 8 bytes of 0xfff0000, 0xfff0000
	GCM_INIT_17					= 0x02C8,	// 8 bytes of 0xfff0000, 0xfff0000
	GCM_INIT_18					= 0x02D0,	// 8 bytes of 0xfff0000, 0xfff0000
	GCM_INIT_19					= 0x02D8,	// 8 bytes of 0xfff0000, 0xfff0000
	GCM_INIT_20					= 0x02E0,	// 8 bytes of 0xfff0000, 0xfff0000
	GCM_INIT_21					= 0x02E8,	// 8 bytes of 0xfff0000, 0xfff0000
	GCM_INIT_22					= 0x02F0,	// 8 bytes of 0xfff0000, 0xfff0000
	GCM_INIT_23					= 0x02F8,	// 8 bytes of 0xfff0000, 0xfff0000

	DITHER_ENABLE				= 0x0300,
	ALPHA_TEST_ENABLE			= 0x0304,
	ALPHA_FUNC					= 0x0308,
	ALPHA_REF					= 0x030C,
	BLEND_ENABLE				= 0x0310,
	BLEND_FUNC_ALPHA			= 0x0314,
	BLEND_FUNC_COLOUR			= 0x0318,
	BLEND_COLOUR0				= 0x031C,
	BLEND_EQUATION				= 0x0320,
	COLOUR_MASK					= 0x0324,
	STENCIL_TEST_ENABLE			= 0x0328,
	STENCIL_WRITE_MASK			= 0x032C,
	STENCIL_FUNC				= 0x0330,
	STENCIL_REF					= 0x0334,
	STENCIL_MASK				= 0x0338,

	TWO_SIDED_STENCIL_ENABLE	= 0x0348,
	BACK_STENCIL_MASK			= 0x034C,

	BACK_STENCIL_OP_FAIL		= 0x035C,
	BACK_STENCIL_OP_DEPTH_FAIL	= 0x0360,
	BACK_STENCIL_OP_DEPTH_PASS	= 0x0364,
	SHADE_MODE					= 0x0368,
	BLEND_ENABLE_MRT			= 0x036C,
	COLOUR_MASK_MRT				= 0x0370,
	LOGIC_OP_ENABLE				= 0x0374,
	LOGIC_OP					= 0x0378,
	BLEND_COLOUR1				= 0x037C,
	DEPTH_BOUNDS_ENABLE			= 0x0380,
	DEPTH_BOUNDS_NEAR			= 0x0384,
	DEPTH_BOUNDS_FAR			= 0x0388,

	VIEWPORT_NEAR				= 0x0394,
	VIEWPORT_FAR				= 0x0398,

	LINE_WIDTH					= 0x03B8,

	STENCIL_OP_FAIL				= 0x044C,
	STENCIL_OP_DEPTH_FAIL		= 0x0450,
	STENCIL_OP_DEPTH_PASS		= 0x0454,

	SCISSOR_RECT_HORIZ			= 0x08C0,
	SCISSOR_RECT_VERT			= 0x08C4,

	FRAGMENT_SHADER_ADDRESS		= 0x08E4,

	VIEWPORT_RECT_HORIZ			= 0x0A00,
	VIEWPORT_RECT_VERT			= 0x0A04,

	VIEWPORT_SCALE_X			= 0x0A20, // NOTE GCM pushing these twice for no 'apparent' reason
	VIEWPORT_SCALE_Y			= 0x0A24, // NOTE GCM pushing these twice for no 'apparent' reason
	VIEWPORT_SCALE_Z			= 0x0A28, // NOTE GCM pushing these twice for no 'apparent' reason
	VIEWPORT_SCALE_W			= 0x0A2C, // NOTE GCM pushing these twice for no 'apparent' reason
	VIEWPORT_POS_OFFSET_X		= 0x0A30, // NOTE GCM pushing these twice for no 'apparent' reason
	VIEWPORT_POS_OFFSET_Y		= 0x0A34, // NOTE GCM pushing these twice for no 'apparent' reason
	VIEWPORT_POS_OFFSET_Z		= 0x0A38, // NOTE GCM pushing these twice for no 'apparent' reason
	VIEWPORT_POS_OFFSET_W		= 0x0A3C, // NOTE GCM pushing these twice for no 'apparent' reason

	POLYGON_OFFSET_ENABLE		= 0x0A68,
	DEPTH_FUNC					= 0x0A6C,
	DEPTH_WRITE_ENABLE			= 0x0A70,
	DEPTH_TEST_ENABLE			= 0x0A74,
	POLYGON_OFFSET_FACTOR		= 0x0A78,
	POLYGON_OFFSET_UNITS		= 0x0A7C,

	// note ICE sets these with every texture (1 per stage) to the same value. libgcm sets then init...
	// 16 each 4 bytes ICE always sets them to 0x2dc4. libgcm has been known to set then 0x8 or 0xbc8...
	// ends 0xB40
	TEXTURE0_BRILINEAR			= 0x0B00,

	FRAGMENT_CONTROL_0			= 0x0B40,	//!< from ice shader 0x0 = on (interpolators???) 
	FRAGMENT_CONTROL_1			= 0x0B44,	//!< NOT UNDERSTOOD YET
	FRAGMENT_CONTROL_2			= 0x0B48,	//!< NOT UNDERSTOOD YET
	FRAGMENT_CONTROL_3			= 0x0B4C,	//!< NOT UNDERSTOOD YET
	FRAGMENT_CONTROL_4			= 0x0B50,	//!< NOT UNDERSTOOD YET
	FRAGMENT_CONTROL_5			= 0x0B54,	//!< NOT UNDERSTOOD YET
	FRAGMENT_CONTROL_6			= 0x0B58,	//!< NOT UNDERSTOOD YET
	FRAGMENT_CONTROL_7			= 0x0B5C,	//!< NOT UNDERSTOOD YET
	FRAGMENT_CONTROL_8			= 0x0B60,	//!< NOT UNDERSTOOD YET
	FRAGMENT_CONTROL_9			= 0x0B64,	//!< NOT UNDERSTOOD YET

	UPLOAD_VERTEX_SHADER		= 0x0B80,	//!< speculations 2816 bytes of vshader program?
											//!< from here to 0x1680???

	// array of vertex stream registers to get one add 0x4*N where N is the stream index
	VERTEXSTREAM0_OFFSET			= 0x1680,
	// end VERTEX_DATA_OFFSET @ 0x17C0

	FLUSH_0						= 0x1710,	// a flush consist of 1 flush_0 and 3 flush_1
	FLUSH_1						= 0x1714,

	// array of vertex stream registers to get one add 0x4*N where N is the stream index
	VERTEXSTREAM0_FORMAT		= 0x1740,
	// end VERTEX_DATA_ARRAY @ 0x1780

	TIME_STAMP					= 0x1800,

	DRAW						= 0x1808,
	VERTEX_ARRAY_PARAMS			= 0x1814,
	INDEX_ARRAY_OFFSET			= 0x181C, 
	INDEX_ARRAY_TYPE			= 0x1820, 
	INDEX_ARRAY_PARAMS			= 0x1824,
	FRONT_POLYGON_MODE			= 0x1828,
	BACK_POLYGON_MODE			= 0x182C,
	CULL_FACE					= 0x1830,
	FRONT_FACE					= 0x1834,
	CULL_FACE_ENABLE			= 0x183C,



	TEXTURE0_SIZE_DP			= 0x1840,	// odd address?? expect I need to look at index
											// as I'll bet real money its not 0x20 for each i.d say 0x4

	// Texture register ranges are the 0 one + 0x20*N (where N is the texture index)
// GCM_INIT_27 to GCM_INIT_36 sets TEXTURE0 to some defaults

	TEXTURE0_OFFSET				= 0x1A00,
	TEXTURE0_FORMAT				= 0x1A04,
	TEXTURE0_ADDRESS			= 0x1A08,
	TEXTURE0_CONTROL			= 0x1A0C,
	TEXTURE0_REMAP				= 0x1A10,
	TEXTURE0_FILTER				= 0x1A14,
	TEXTURE0_SIZE_WH			= 0x1A18,
	TEXTURE0_BORDER_COLOUR		= 0x1A1C,

	// array of vertex data register to get one add 0x10*N where N is the stream index
	VERTEX_DATA_4F_X			= 0x1C00, // 16 stream end at 01CA0
	VERTEX_DATA_4F_Y			= 0x1C04, // 16 stream end at 01CA0
	VERTEX_DATA_4F_Z			= 0x1C08, // 16 stream end at 01CA0
	VERTEX_DATA_4F_W			= 0x1C0C, // 16 stream end at 01CA0

	TEXTURE0_COLOURKEYCONTROL	= 0x1D00, 


	FRAGMENT_SHADER_CONTROL		= 0x1D60,	// these 2 are used to sync up fragment uploads
	BACKEND_SELECT				= 0x1D6C,
	WRITEBACK_END_LABEL			= 0x1D70,
	WRITE_TEXTURE_LABEL			= 0x1D74,

	FRAGMENT_UPLOAD_LABEL		= 0x1D78,
	MSAA_ENABLE					= 0x1D7C,
	RENDER_TARGET_HEIGHT		= 0x1D88,
	CLEAR_DEPTH_STENCIL			= 0x1D8C,
	CLEAR_COLOUR				= 0x1D90,
	CLEAR						= 0x1D94,
	GCM_INIT_24					= 0x1D98,	// 8 bytes of 0xfff0000, 0xfff0000

	GCM_INIT_14					= 0x1DA4, // 4 bytes of 0x0

	RESTART_INDEX_ENABLE		= 0x1DAC,

	RESTART_INDEX				= 0x1DB0,

	VERTEX_CONTROL_3			= 0x1e9c,
	VERTEX_CONTROL_4			= 0x1ea0,

	POINT_SIZE					= 0x1EE0,
	POINT_SPRITE_ENABLE			= 0x1EE4,

	VERTEX_CONTROL_2			= 0x1EF8, // offset 12 from Ice::vertexProgram (speculation? interpolator control?)
	SELECT_VERTEX_CONSTANT_WINDOW		= 0x1EFC,
	VERTEX_CONSTANT_WINDOW		= 0x1F00, // a 192 float window into vertex constant I speculate

	FREQUENCY_DIVIDER_OP		= 0x1FC0,

	FLUSH_TEXTURE_CACHE			= 0x1FD8,

	VERTEX_CONTROL_0			= 0x1FF0,	//!< offset 4 from ICE::VertexProgram attribute bits ???
	VERTEX_CONTROL_1			= 0x1FF4,	//!< offset 8 from ICE::VertexProgram



	// unknown involved with flip but haven't investigated 
	DISPLAY_0					= 0xE000,
	DISPLAY_1					= 0xE804,
	DISPLAY_2					= 0xE808,
	DISPLAY_3					= 0xE810,



};

#endif // end HERESY_HARDWARE_H
