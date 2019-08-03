//--------------------------------------------------
//!
//!	\file effecttrail_line.cpp
//!	This is a dummy holding a few doxygen comments.
//!
//--------------------------------------------------

#include "effecttrail_line.h"
#include "effecttrail_linebuffer.h"
#include "objectdatabase/dataobject.h"
#include "anim/transform.h"
#include "gfx/levellighting.h"
#include "colour_function.h"
#include "gfx/TextureAtlas.h"
#include "gfx/texturemanager.h"
#include "objectdatabase/neteditinterface.h"

START_STD_INTERFACE( EffectTrail_LineDef )

	I2STRING	( m_texName,					TextureName )

	I2POINT		( m_offset,						Offset )
	I2FLOAT		( m_fWidth,						LineWidth )
	I2FLOAT		( m_fFadeTime,					FadeTime )
	I2FLOAT		( m_fLifeTime,					LifeTime )
	I2FLOAT		( m_fEmitPerSecond,				EmitPerSecond )
	I2FLOAT		( m_fTexRepeatPeriod,			TexRepeatPeriod(s) )
	I2REFERENCE	( m_resources.m_pFadePalette,	FadePalette )
	I2REFERENCE	( m_resources.m_pCrossPalette,	CrossPalette )

	// Effect Renderstate parameters
	//----------------------------------------
	I2ENUM		( m_rsDef.m_renderType,			RS.RenderMode, EFFECT_RENDER_TYPE )
	I2ENUM		( m_rsDef.m_blendMode,			RS.AlphaBlendMode, EFFECT_BLENDMODE )
	I2BOOL		( m_rsDef.m_bZWriteEnable,		RS.DepthWriteEnabled )
	I2BOOL		( m_rsDef.m_bAlphaTestEnable,	RS.AlphaTestEnabled )
	I2FLOAT		( m_rsDef.m_fAlphaTestRef,		RS.AlphaTestValue )
	I2ENUM		( m_rsDef.m_alphaTestFunc,		RS.AlphaTestFunction, EFFECT_CMPFUNC )
	I2REFERENCE	( m_rsDef.m_pTimeOfDayMod,		RS.TimeOfDayPalette )

	DECLARE_POSTCONSTRUCT_CALLBACK( PostConstruct )
	DECLARE_EDITORCHANGEVALUE_CALLBACK( EditorChangeValue )
END_STD_INTERFACE




//--------------------------------------------------
//!
//!	EffectTrail_LineDefResources::ctor
//!
//--------------------------------------------------
EffectTrail_LineDefResources::EffectTrail_LineDefResources() :
	m_pFadePalette(0),
	m_pCrossPalette(0)
{
	EffectResourceMan::Get().RegisterResource( *this );
}

EffectTrail_LineDefResources::~EffectTrail_LineDefResources()
{
	EffectResourceMan::Get().ReleaseResource( *this );
}

//--------------------------------------------------
//!
//!	EffectTrail_LineDefResources::GenerateResources
//!
//--------------------------------------------------
void EffectTrail_LineDefResources::GenerateResources()
{
	if (m_fadePalette)
		m_fadePalette.Reset();
	
	if (m_crossPalette)
		m_crossPalette.Reset();

	if (m_pFadePalette)
		m_fadePalette = m_pFadePalette->GenerateTexture( ColourFunction::TEXGEN_NORMAL, 256, true, false );

	if (m_pCrossPalette)
		m_crossPalette = m_pCrossPalette->GenerateTexture( ColourFunction::TEXGEN_NORMAL, 256, true, false );

	ResourcesOutOfDate(); // this flushes any erronious refresh detects
	m_bRequireRefresh = false;
}

//--------------------------------------------------
//!
//!	EffectTrail_LineDefResources::ResourcesOutOfDate
//! per frame callback to see if we need regenerating
//!
//--------------------------------------------------
bool EffectTrail_LineDefResources::ResourcesOutOfDate() const
{
	if ((m_pFadePalette) && (m_pFadePalette->HasChanged()))
		m_bRequireRefresh = true;

	if ((m_pCrossPalette) && (m_pCrossPalette->HasChanged()))
		m_bRequireRefresh = true;

	return m_bRequireRefresh;
}




//--------------------------------------------------
//!
//!	EffectTrail_LineDef::ctor
//!
//--------------------------------------------------
EffectTrail_LineDef::EffectTrail_LineDef() :
	m_offset( CONSTRUCT_CLEAR ),
	m_fWidth(0.1f),
	m_fFadeTime(1.0f),
	m_fLifeTime(-1.0f),
	m_fEmitPerSecond(100.0f),
	m_fTexRepeatPeriod(1.0f),
	m_fCullRadius(10.0f)
{}

//--------------------------------------------------
//!
//!	EffectTrail_LineDef::PostConstruct
//! Check for sensible values
//!
//--------------------------------------------------
void EffectTrail_LineDef::PostConstruct()
{
	m_fFadeTime = ntstd::Max( m_fFadeTime, 0.1f );
	m_fEmitPerSecond = ntstd::Clamp( m_fEmitPerSecond, 0.0f, 10000.0f );

	m_iMaxPoints = (u_int)( m_fEmitPerSecond * m_fFadeTime );
	m_iMaxPoints = ntstd::Max( m_iMaxPoints, 2u );

	ResolveTrailTextureMode();
	LoadTexture();
}

//--------------------------------------------------
//!
//!	EffectTrail_LineDef::ResolveTrailTextureMode
//! Get what type of texture mode we are
//!
//--------------------------------------------------
void EffectTrail_LineDef::ResolveTrailTextureMode()
{
	m_eTexMode = TTM_UNTEXTURED;

	if ( !ntStr::IsNull(m_texName) )
	{
		const char* pName = ntStr::GetString(m_texName);

		if	(
			( TextureAtlasManager::Get().IsAtlas(pName) ) &&
			( TextureAtlasManager::Get().Exists(pName) )
			)
		{
			m_eTexMode = TTM_ANIM_TEXTURED;
		}
		// FIXME_WIL. When we pre-load effect resources, this should
		// change to Loaded_Neutral(). This is just a level load optimisation
		else if ( TextureManager::Get().Exists_Neutral(pName) )
		{
			m_eTexMode = TTM_SIMPLE_TEXTURED;
		}
	}
}

//--------------------------------------------------
//!
//!	EffectTrail_LineDef::LoadTexture
//!	Load any additional resources (atlas, texture)
//!
//--------------------------------------------------
void EffectTrail_LineDef::LoadTexture()
{
	if ( m_eTexMode == TTM_ANIM_TEXTURED )
	{
		m_pAtlas = TextureAtlasManager::Get().GetAtlas( ntStr::GetString(m_texName) );
		ntAssert_p( m_pAtlas, ("Problem with texture atlas %s\n", ntStr::GetString(m_texName)) );

		#ifdef _DEBUG
		// make sure theyre all the same size and shape,
		u_int iNumTex = m_pAtlas->GetNumEntries();
		float fWidth = m_pAtlas->GetEntryByIndex(0)->GetWidth();
		for ( u_int i = 0; i < iNumTex; i++ )
		{
			ntAssert( m_pAtlas->GetEntryByIndex(i)->GetHeight() == 1.0f );
			ntAssert( m_pAtlas->GetEntryByIndex(i)->GetWidth() == fWidth );
		}
		#endif
	}
	else if ( m_eTexMode == TTM_SIMPLE_TEXTURED )
	{
		m_pTex = TextureManager::Get().LoadTexture_Neutral( ntStr::GetString(m_texName) );
	}
}

//--------------------------------------------------
//!
//!	refresh our internal state after editing
//!
//--------------------------------------------------
bool EffectTrail_LineDef::EditorChangeValue( CallBackParameter param, CallBackParameter )
{
	CHashedString pName(param);

	if (HASH_STRING_FADEPALETTE == pName || HASH_STRING_CROSSPALETTE == pName)
	{
		m_resources.MarkForRefresh();
	}

	PostConstruct();

	if (HASH_STRING_TEXTURENAME == pName || HASH_STRING_FADETIME == pName || HASH_STRING_LIFETIME == pName || HASH_STRING_EMITPERSECOND  == pName)
	{
		m_resetSet.ResetThings();
	}

	return true;
}





//--------------------------------------------------
//!
//!	EffectTrail_Line ctor
//!
//--------------------------------------------------
EffectTrail_Line::EffectTrail_Line( const EffectTrail_LineDef* pDef,
									const Transform* pTransform,
									const CPoint* pOffsetOveride ) :
	m_pDef( pDef ),
	m_pTransform( pTransform ),
	m_pLine( 0 )
{
	ntError( m_pDef );
	ntError( m_pTransform );

	m_bUseOffsetOveride = false;
	if (pOffsetOveride)
	{
		m_offsetOveride = *pOffsetOveride;
		m_bUseOffsetOveride = true;
	}

	Reset(false);
	m_pDef->m_resetSet.RegisterThingToReset(this);
}

//--------------------------------------------------
//!
//!	EffectTrail_Line dtor
//!
//--------------------------------------------------
EffectTrail_Line::~EffectTrail_Line()
{
	Reset(true);
	m_pDef->m_resetSet.UnRegisterThingToReset(this);
}

//--------------------------------------------------
//!
//!	EffectTrail_Line::reset
//!
//--------------------------------------------------
void EffectTrail_Line::Reset( bool bDestructor )
{
	m_fLastAge = 0.0f;
	m_fCurrAge = 0.0f;
	m_fLastEmitTime = 0.0f;
	m_fAccumulator = 0.0f;

	if (m_pLine)
	{
		NT_DELETE_CHUNK( Mem::MC_EFFECTS, m_pLine );
	}

	if (!bDestructor)
	{
		m_pLine = NT_NEW_CHUNK ( Mem::MC_EFFECTS ) EffectTrailLineBuffer( m_pDef );

		// get initial emission position
		if (m_bUseOffsetOveride)
			m_currEmitPos = m_offsetOveride * m_pTransform->GetWorldMatrix();
		else
			m_currEmitPos = m_pDef->m_offset * m_pTransform->GetWorldMatrix();

		m_lastEmitPos = m_currEmitPos;
	}
}

//--------------------------------------------------
//!
//!	EffectTrail_Line::WaitingForResources
//!
//--------------------------------------------------
bool EffectTrail_Line::WaitingForResources() const
{
	return m_pDef->m_resources.ResourcesOutOfDate();
}

//--------------------------------------------------
//!
//!	EffectTrail_Line::Update
//!
//--------------------------------------------------
bool EffectTrail_Line::UpdateEffect()
{
	float fTimeDelta = GetNextTimeDelta();

	// this is okay here as we're only using debug prims, which are delayed render...
	// it's here as render can be disabled completley if the right blend modes are set.
	if (CNetEditInterface::Get().GetSelected() == ObjectDatabase::Get().GetDataObjectFromPointer(m_pDef) )
		m_pLine->DebugRender();

	// make sure our culling information is uptodate
	m_fCullingRadius = m_pDef->m_fCullRadius;
	m_cullingOrigin = m_currEmitPos;

	// never update with a backwards delta
	if (fTimeDelta <= 0.0f)
		return m_bKillMeNow;

	// record our last age and see if we need to emit more edges
	m_fLastAge = m_fCurrAge;
	m_fCurrAge += fTimeDelta;

	// see if we've finished or not..
	if ((m_pDef->m_fLifeTime > 0.0f) && (m_fCurrAge >= m_pDef->m_fLifeTime))
		m_bKillMeRequested = true;

	if (m_bKillMeRequested)
	{
		if ((m_fCurrAge - m_fLastEmitTime) >= m_pDef->m_fFadeTime)
			m_bKillMeNow = true;
	}
	else if (!m_bKillMeNow)
	{
		// get the latest emission position
		CPoint oldEmitPos = m_currEmitPos;
		if (m_bUseOffsetOveride)
			m_currEmitPos = m_offsetOveride * m_pTransform->GetWorldMatrix();
		else
			m_currEmitPos = m_pDef->m_offset * m_pTransform->GetWorldMatrix();
		
		// fit a catmull to our recently retrieved emit position
		CVector coeffs[4];
		static const float gafBasisMat[4][4] =
		{
			{ -0.5f,  1.5f, -1.5f,  0.5f },
			{  1.0f, -2.5f,  2.0f, -0.5f },
			{ -0.5f,  0.0f,  0.5f,  0.0f },
			{  0.0f,  1.0f,  0.0f,  0.0f }
		};
		
		CPoint nextEmitPos = (3.0f*m_currEmitPos) - (3.0f*oldEmitPos) + m_lastEmitPos;

		// four points for curr catmull are; pos from 2 frames ago, last pos,
		// this pos, and a predicted pos based on constant acc
		for (int i = 0; i < 4; i++)
		{
			coeffs[i].Clear();
			coeffs[i] += CVector(m_lastEmitPos)		* gafBasisMat[i][0];
			coeffs[i] += CVector(oldEmitPos)		* gafBasisMat[i][1];
			coeffs[i] += CVector(m_currEmitPos)		* gafBasisMat[i][2];
			coeffs[i] += CVector(nextEmitPos)		* gafBasisMat[i][3];
		}

		m_lastEmitPos = oldEmitPos;

		// see how many to emit 
		float fEmitInterval = 1.0f / m_pDef->m_fEmitPerSecond;
		m_fAccumulator += fTimeDelta * m_pDef->m_fEmitPerSecond;

		while (m_fAccumulator >= 1.0f)
		{
			m_fLastEmitTime += fEmitInterval;
			m_fLastEmitTime = ntstd::Clamp( m_fLastEmitTime, m_fLastAge, m_fCurrAge );

			float fNormalisedT = (m_fLastEmitTime - m_fLastAge) / fTimeDelta;

			// evaluate our catmull for the interpolated line position
			CVector pos = coeffs[0];
			for (int j = 1; j < 4; j++)
			{
				pos *= fNormalisedT;
				pos += coeffs[j];
			}
			pos.W() = 0.0f;

			m_pLine->EmitPoint( m_fLastEmitTime, CPoint( pos ) );
			m_fAccumulator-=1.0f;
		}
	}

	return m_bKillMeNow;
}
