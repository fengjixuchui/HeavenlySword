//--------------------------------------------------
//!
//!	\file renderstate_block.h
//!	Render state management object for effects
//!
//--------------------------------------------------

#ifndef GFX_RS_BLOCK_H
#define GFX_RS_BLOCK_H

#include "editable/enumlist.h"
#include "gfx/renderstates.h"

class ColourFunction;

//--------------------------------------------------
//!
//! RenderStateDef
//! Welder editable bits
//!	
//--------------------------------------------------
class RenderStateDef
{
public:
	RenderStateDef() :
		m_bZWriteEnable( false ),
		m_bAlphaTestEnable( false ),
		m_fAlphaTestRef( 0.0f ),
		m_alphaTestFunc( ECF_GREATER ),  
		m_blendMode( EBM_OVERWRITE ),
		m_renderType( ERT_LOW_DYNAMIC_RANGE ),
		m_pTimeOfDayMod( NULL )
	{}

	bool	m_bZWriteEnable;
	bool	m_bAlphaTestEnable;
	float	m_fAlphaTestRef;		// 0.0f -> 1.0f
	
	EFFECT_CMPFUNC		m_alphaTestFunc;
	EFFECT_BLENDMODE	m_blendMode;
	EFFECT_RENDER_TYPE	m_renderType;

	ColourFunction*		m_pTimeOfDayMod;
};

//--------------------------------------------------
//!
//! RenderStateBlock
//! Used by effects to set and clear renderstate type
//! things
//!	
//--------------------------------------------------
class RenderStateBlock
{
public:
	RenderStateBlock() :
		m_bPointSprite( false ),
		m_bZTestEnable( true ),
		m_bZWriteEnable( false ),
		m_bDisablePolyCulling( false ),
		m_bAlphaTestEnable( false ),
		m_fAlphaTestRef( 0.0f ),
		m_alphaTestFunc( ECF_GREATER ),  
		m_blendMode( EBM_OVERWRITE ),
		m_renderType( ERT_LOW_DYNAMIC_RANGE ),
		m_pTimeOfDayMod( NULL )
	{}
	
	RenderStateBlock& operator = (const RenderStateDef& def)
	{
		m_bZWriteEnable = def.m_bZWriteEnable;
		m_bAlphaTestEnable = def.m_bAlphaTestEnable;
		m_fAlphaTestRef = def.m_fAlphaTestRef;
		m_alphaTestFunc = def.m_alphaTestFunc;
		m_blendMode = def.m_blendMode;
		m_renderType = def.m_renderType;
		m_pTimeOfDayMod = def.m_pTimeOfDayMod;
		return *this;
	}
		
	bool	m_bPointSprite;
	bool	m_bZTestEnable;
	bool	m_bZWriteEnable;
	bool	m_bDisablePolyCulling;
	bool	m_bAlphaTestEnable;
	float	m_fAlphaTestRef;		// 0.0f -> 1.0f
	
	EFFECT_CMPFUNC		m_alphaTestFunc;
	EFFECT_BLENDMODE	m_blendMode;
	EFFECT_RENDER_TYPE	m_renderType;

	ColourFunction*		m_pTimeOfDayMod;

	void	SetRenderstates();
	void	ClearRenderstates();

	static void SetBlendMode( EFFECT_BLENDMODE mode );

private:
	static inline GFX_ALPHA_TEST_MODE ConvertCmpFunc( EFFECT_CMPFUNC func )
	{
		switch ( func )
		{
			case ECF_NEVER:				return	GFX_ALPHATEST_NONE;
			case ECF_LESS:				return	GFX_ALPHATEST_LESS;
			case ECF_EQUAL:				return	GFX_ALPHATEST_EQUAL;
			case ECF_LESSEQUAL:			return	GFX_ALPHATEST_LESSEQUAL;
			case ECF_GREATER:			return	GFX_ALPHATEST_GREATER;
			case ECF_NOTEQUAL:			return	GFX_ALPHATEST_NOTEQUAL;
			case ECF_GREATEREQUAL:		return	GFX_ALPHATEST_GREATEREQUAL;
			case ECF_ALWAYS:			return	GFX_ALPHATEST_ALWAYS;

			default:
				ntAssert_p(0,("Unrecognised alpha test function: %d", func));
				return GFX_ALPHATEST_NONE;
		}
	};
};

#endif //_RS_BLOCK_H
