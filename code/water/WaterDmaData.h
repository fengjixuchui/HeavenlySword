//--------------------------------------------------
//!
//!	\file WaterDmaData.h
//!	Shared PPU/SPU structs 
//!
//--------------------------------------------------

#ifndef _WATERDMADATA_H_
#define _WATERDMADATA_H_

#include <string.h>

class CEntity;

#ifndef _RELEASE
struct SPU_DebugLine
{
	CPoint	a;
	CPoint	b;

	uint32_t col;
	uint32_t m_padd32[3];
};

static const unsigned int MAX_DEBUG_LINES = 1500;
#endif


static const uint32_t WATER_SPU_PARAM_WINSTANCE		= 0;
static const uint32_t WATER_SPU_PARAM_WAVEARRAY		= 1;
static const uint32_t WATER_SPU_PARAM_VERTARRAY		= 2;
static const uint32_t WATER_SPU_PARAM_ATTACKARRAY	= 3;
static const uint32_t WATER_SPU_PARAM_BUOYARRAY		= 4;


static const uint32_t MAX_NUM_WAVES					= 50;
static const uint32_t MAX_NUM_BUOYS					= 50;

//--------------------------------------------------
//!
//! Water intance data.
//! This contains all instance-specific info for the
//! spu update. 
//!
//--------------------------------------------------
struct WaterInstanceDma
{
	CMatrix				m_obWorldMatrix;			//!< (in) water-to-world
	CMatrix				m_obWorldMatrixInv;			//!< (in) world-to-water

	float				m_fTime;					//!< (in) current time
	float				m_fTimeStep;				//!< (in) update step

	float				m_fWidth;					//!< (in) width in world space
	float				m_fLength;					//!< (in) length in world space
	float				m_fResolution;				//!< (in) grid resolution

	uint32_t			m_iGridSize[2];				//!< (in) < rows, columns>

	float				m_fVScale;					//!< (in) global vertical scale

	float				m_fMaxHeight;				//!< (out) max grid height this frame
	float				m_fMinHeight;				//!< (out) min grid height this frame

#ifndef _RELEASE
	uint32_t*			m_iNumDebugLines;			//!< (in/out) point that must be atomically updated
	SPU_DebugLine*		m_pDebugLineBuffer;			//!< (out) line array [MAX_DEBUG_LINES]
#endif
};



//--------------------------------------------------
//!
//!	Self-contained wave generation info
//! these implicitly define a wave over the whole
//! water surface. Thus, it is easy to partially
//! evaluate them  on sub-grids due to SPU LS limitations
//!
//--------------------------------------------------
enum WaveFlags
{
	// type
	kWF_Type_Circular			= ( 1 << 0 ),
	kWF_Type_Directional		= ( 1 << 1 ),
	kWF_Type_RegularAll			= ( kWF_Type_Circular|kWF_Type_Directional ),

	kWF_Type_Attack0			= ( 1 << 2 ),			//!< 
	kWF_Type_Attack1			= ( 1 << 3 ),			//!<
	kWF_Type_Attack2			= ( 1 << 4 ),			//!<
	kWF_Type_AttackAll			= ( kWF_Type_Attack0|kWF_Type_Attack1|kWF_Type_Attack2 ),

	kWF_Type_All				= ( kWF_Type_RegularAll|kWF_Type_AttackAll ),

	// control
	kWF_Control_Invalid			= ( 1 << 12 ),			//!< invalid waves are ignored
};

struct WaveDma
{
	CDirection			m_obDirection;					//!< (in) direction
	CPoint				m_obOrigin;						//!< (in) origin (current position is implicit)
	
	union
	{
		struct 
		{
		} m_Circular;

		struct
		{
		} m_Directional;

		struct 
		{
			float		m_fWidth;						//!< (in) width of wall
			float		m_fFalloff;						//!< (in) edge fallof
		} m_Attack0;

		struct 
		{
			float		m_fBackTrail;						//!< (in) trail left behind
			float		m_fFrontTrail;
		} m_Attack1;

		struct 
		{
			float		m_fRadius;
		} m_Attack2;

	} m_TypeSpecific;

	float				m_fPhase;						//!< (in) self-explanatory
	float				m_fAmplitude;					//!< (in) self-explanatory
	float				m_fFrequency;					//!< (in) self-explanatory
	float				m_fSharpness;					//!< (in) self-explanatory

	float				m_fAttLinear;					//!< (in) distance-to-origin linear attenuation 
	float				m_fAttQuadratic;				//!< (in) distance-to-origin quadratic attenuation 
	float				m_fFadeInTime;					//!< (in) self-explanatory
	float				m_fFadeOutTime;					//!< (in) self-explanatory

	float				m_fAge;							//!< (in/out) self-explanatory
	float				m_fMaxAge;						//!< (in) self-explanatory

	int					m_iFlags;						//!< (in/out) wave-specific flags


	bool	IsValid( void ) const	{ return !(m_iFlags & kWF_Control_Invalid); }
	void	Invalidate( void )		{ m_iFlags |= kWF_Control_Invalid; }
	void	Validate( void )		{ m_iFlags &= ~kWF_Control_Invalid; }
	void	Clear( void )			{ memset(this, 0, sizeof(WaveDma)); Invalidate(); }
	void	Reset( void )			{ m_fAge = 0.0f; }

	void	Update( float fTimeStep )
	{
		m_fAge += fTimeStep;
		if ( m_fAge > m_fMaxAge )
		{
			Clear();
		}
	}
};




//--------------------------------------------------
//!
//!	water vertex struct 
//!	NOTE: it should be CPoint, float[3], float[3] but the
//! compiler seems to align them anyway so we'll 
//! have to live with v128 types for now 
//! sizeof(VertexDma) MUST be equal to the corresponding
//! ProceduralVB element in WaterInstance
//!
//--------------------------------------------------
struct VertexDma
{
	CPoint				m_position;						//!< (in/out)
	CDirection			m_binormal;						//!< (out)
	CDirection			m_tangent;						//!< (out)
};


//////////////////////////////////////////////////////////////////
//			Tmp and subject to change soon
//////////////////////////////////////////////////////////////////
struct BuoyDma
{
	CMatrix			m_obLocalMatrix;		//!< local, as in water-local
	CDirection		m_obUpDirection;

	float			m_fTravelSpeed;
	float			m_fBuoyancy;			//!< [0.0f, 1.0f]
	int				m_iFlags;
};

enum BuoyFlags
{
	kBF_SyncIn			= ( 1 << 0 ),
	kBF_SyncOut			= ( 1 << 1 ),
	kBF_WaterSpace		= ( 1 << 2 ),
	kBF_WorldSpace		= ( 1 << 3 ),

	kBF_Control_Invalid			= ( 1 << 8 ),

	kBF_Control_SyncInOut		= ( kBF_SyncIn|kBF_SyncOut ),
};
//////////////////////////////////////////////////////////////////


#endif // end of _WATERDMADATA_H_
