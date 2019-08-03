//--------------------------------------------------
//!
//!	\file effecttrail_simple.cpp
//!	This is a dummy holding a few doxygen comments.
//!
//--------------------------------------------------

#include "effecttrail_simple.h"
#include "effecttrail_buffer.h"
#include "colour_function.h"
#include "anim/transform.h"
#include "gfx/textureatlas.h"
#include "gfx/texturemanager.h"
#include "objectdatabase/neteditinterface.h"
#include "objectdatabase/dataobject.h"

START_STD_INTERFACE( EffectTrail_SimpleDef )

	I2STRING	( m_texName,					TextureName )
	I2FLOAT		( m_fFadeTime,					FadeTime )
	I2FLOAT		( m_fLifeTime,					LifeTime )
	I2FLOAT		( m_fEdgesPerSecond,			EdgesPerSecond )
	I2FLOAT		( m_fTexRepeatPeriod,			TexRepeatPeriod(s) )
	I2REFERENCE	( m_pEdgeDefinition,			EdgeDefinition )
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
//!	EffectTrail_SimpleDefResources::ctor
//!
//--------------------------------------------------
EffectTrail_SimpleDefResources::EffectTrail_SimpleDefResources() :
	m_pFadePalette(0),
	m_pCrossPalette(0)
{
	EffectResourceMan::Get().RegisterResource( *this );
}

EffectTrail_SimpleDefResources::~EffectTrail_SimpleDefResources()
{
	EffectResourceMan::Get().ReleaseResource( *this );
}

//--------------------------------------------------
//!
//!	EffectTrail_SimpleDefResources::GenerateResources
//!
//--------------------------------------------------
void EffectTrail_SimpleDefResources::GenerateResources()
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
//!	EffectTrail_SimpleDefResources::ResourcesOutOfDate
//! per frame callback to see if we need regenerating
//!
//--------------------------------------------------
bool EffectTrail_SimpleDefResources::ResourcesOutOfDate() const
{
	if ((m_pFadePalette) && (m_pFadePalette->HasChanged()))
		m_bRequireRefresh = true;

	if ((m_pCrossPalette) && (m_pCrossPalette->HasChanged()))
		m_bRequireRefresh = true;

	return m_bRequireRefresh;
}




//--------------------------------------------------
//!
//!	EffectTrail_SimpleDef::ctor
//!
//--------------------------------------------------
EffectTrail_SimpleDef::EffectTrail_SimpleDef() :
	m_fFadeTime(1.0f),
	m_fLifeTime(-1.0f),
	m_fEdgesPerSecond(100.0f),
	m_fTexRepeatPeriod(1.0f),
	m_fCullRadius(10.0f),
	m_pEdgeDefinition(0)
{}

//--------------------------------------------------
//!
//!	EffectTrail_SimpleDef::PostConstruct
//! Check for sensible values
//!
//--------------------------------------------------
void EffectTrail_SimpleDef::PostConstruct()
{
	m_fFadeTime = ntstd::Max( m_fFadeTime, 0.1f );
	m_fEdgesPerSecond = ntstd::Clamp( m_fEdgesPerSecond, 0.0f, 10000.0f );

	m_iMaxEdges = (u_int)( m_fEdgesPerSecond * m_fFadeTime );
	m_iMaxEdges = ntstd::Max( m_iMaxEdges, 2u );

	ResolveTrailTextureMode();
	LoadTexture();
}

//--------------------------------------------------
//!
//!	EffectTrail_SimpleDef::ResolveTrailTextureMode
//! Get what type of texture mode we are
//!
//--------------------------------------------------
void EffectTrail_SimpleDef::ResolveTrailTextureMode()
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
//!	EffectTrail_SimpleDef::LoadTexture
//!	Load any additional resources (atlas, texture)
//!
//--------------------------------------------------
void EffectTrail_SimpleDef::LoadTexture()
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
bool EffectTrail_SimpleDef::EditorChangeValue( CallBackParameter param, CallBackParameter )
{
	CHashedString pName(param);

	if (HASH_STRING_FADEPALETTE == pName || HASH_STRING_CROSSPALETTE == pName)
	{
		m_resources.MarkForRefresh();
	}

	PostConstruct();

	if (HASH_STRING_TEXTURENAME == pName || HASH_STRING_FADETIME == pName || HASH_STRING_EDGESPERSECOND == pName || HASH_STRING_EDGEDEFINITION  == pName)
	{
		m_resetSet.ResetThings();
	}

	return true;
}




//--------------------------------------------------
//!
//!	EffectTrail_Simple ctor
//!
//--------------------------------------------------
EffectTrail_Simple::EffectTrail_Simple( const EffectTrail_SimpleDef* pDef, const Transform* pTransform ) :
	m_pDef( pDef ),
	m_pTransform( pTransform ),
	m_pTrail( 0 )
{
	ntError( m_pDef );
	ntError( m_pTransform );
	ntError( m_pDef->m_pEdgeDefinition );

	m_pEdge = m_pDef->m_pEdgeDefinition;

	Reset(false);
	m_pDef->m_resetSet.RegisterThingToReset(this);
}


//--------------------------------------------------
//!
//!	EffectTrail_Simple ctor
//!
//--------------------------------------------------
EffectTrail_Simple::EffectTrail_Simple( const EffectTrail_SimpleDef* pDef,
										const EffectTrail_EdgeDef* pEdge,
										const Transform* pTransform ) :
	m_pDef( pDef ),
	m_pTransform( pTransform ),
	m_pTrail( 0 )
{
	ntError( m_pDef );
	ntError( m_pTransform );

	m_pEdge = pEdge ? pEdge : m_pDef->m_pEdgeDefinition;
	ntError( m_pEdge );

	Reset(false);
	m_pDef->m_resetSet.RegisterThingToReset(this);
}

//--------------------------------------------------
//!
//!	EffectTrail_Simple dtor
//!
//--------------------------------------------------
EffectTrail_Simple::~EffectTrail_Simple()
{
	Reset(true);
	m_pDef->m_resetSet.UnRegisterThingToReset(this);
}

//--------------------------------------------------
//!
//!	EffectTrail_Simple::RetriveEmitterFrame
//!
//--------------------------------------------------
void EffectTrail_Simple::RetriveEmitterFrame()
{
	ntAssert( m_pEdge );
	m_currFrame = m_pTransform->GetWorldMatrix();
	m_currAverageEdgePos = m_averageEdgePos * m_currFrame;
	m_pEdge->m_debugRenderMat = m_currFrame;
}

//--------------------------------------------------
//!
//!	EffectTrail_Simple::reset
//!
//--------------------------------------------------
void EffectTrail_Simple::Reset( bool bDestructor )
{
	m_fLastAge = 0.0f;
	m_fCurrAge = 0.0f;
	m_fLastEmitTime = 0.0f;
	m_fAccumulator = 0.0f;

	if (m_pTrail)
	{
		NT_DELETE_CHUNK( Mem::MC_EFFECTS, m_pTrail );
	}

	if (!bDestructor)
	{
		// setup our particle handler
		m_pTrail = NT_NEW_CHUNK( Mem::MC_EFFECTS ) EffectTrailBuffer( m_pDef, m_pEdge ); 
		m_renderstates.m_bDisablePolyCulling = true;

		// pull out our average edge position
		ntAssert( m_pEdge->m_res.GetVectorArray() );

		CVector acc( CONSTRUCT_CLEAR );
		for (u_int i = 0; i < m_pEdge->m_res.GetNumPoints(); i++)
			acc += m_pEdge->m_res.GetVectorArray()[i];

		acc *= (1.0f / m_pEdge->m_res.GetNumPoints());
		
		// deliberately using float3 constructor as i want a valid W()
		m_averageEdgePos.Clear();
		m_averageEdgePos = CPoint( acc.X(), acc.Y(), acc.Z() );

		// get initial transform
		RetriveEmitterFrame();
		m_framePosAncient = m_currFrame.GetTranslation();
	}
}

//--------------------------------------------------
//!
//!	EffectTrail_Simple::WaitingForResources
//!
//--------------------------------------------------
bool EffectTrail_Simple::WaitingForResources() const
{
	return m_pDef->m_resources.ResourcesOutOfDate();
}

//--------------------------------------------------
//!
//!	EffectTrail_Simple::Update
//!
//--------------------------------------------------
bool EffectTrail_Simple::UpdateEffect()
{
	float fTimeDelta = GetNextTimeDelta();

	// this is okay here as we're only using debug prims, which are delayed render...
	// it's here as render can be disabled completley if the right blend modes are set.
	if (CNetEditInterface::Get().GetSelected() == ObjectDatabase::Get().GetDataObjectFromPointer(m_pDef) )
	{
		m_pTrail->DebugRender();
		const_cast<EffectTrail_EdgeDef*>(m_pEdge)->DebugRender();
	}

	// make sure our culling information is uptodate
	m_fCullingRadius = m_pDef->m_fCullRadius;
	m_cullingOrigin = m_currAverageEdgePos;

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
		// make sure our emitter frame is uptodate
		CMatrix oldFrame = m_currFrame;
		RetriveEmitterFrame();
		
		// fit a catmull to our recently retrieved frame translation
		CVector coeffs[4];
		static const float gafBasisMat[4][4] =
		{
			{ -0.5f,  1.5f, -1.5f,  0.5f },
			{  1.0f, -2.5f,  2.0f, -0.5f },
			{ -0.5f,  0.0f,  0.5f,  0.0f },
			{  0.0f,  1.0f,  0.0f,  0.0f }
		};
		
		CDirection lastFrameVel = oldFrame.GetTranslation() ^ m_framePosAncient;
		CDirection thisFrameVel = m_currFrame.GetTranslation() ^ oldFrame.GetTranslation();
		CDirection frameAcc = thisFrameVel - lastFrameVel;
		CDirection nextFrameVel = thisFrameVel + frameAcc;
		CPoint nextPos = m_currFrame.GetTranslation() + nextFrameVel;

		// four points for curr catmull are; pos from 2 frames ago, last pos,
		// this pos, and a predicted pos based on constant acc
		for (int i = 0; i < 4; i++)
		{
			coeffs[i].Clear();
			coeffs[i] += CVector(m_framePosAncient)				* gafBasisMat[i][0];
			coeffs[i] += CVector(oldFrame.GetTranslation())		* gafBasisMat[i][1];
			coeffs[i] += CVector(m_currFrame.GetTranslation())	* gafBasisMat[i][2];
			coeffs[i] += CVector(nextPos)						* gafBasisMat[i][3];
		}

		m_framePosAncient = oldFrame.GetTranslation();

		// see how many to emit 
		float fEmitInterval = 1.0f / m_pDef->m_fEdgesPerSecond;
		m_fAccumulator += fTimeDelta * m_pDef->m_fEdgesPerSecond;

		// slerp orientation over the frame
		CQuat startQuat = CQuat(oldFrame);
		CQuat endQuat = CQuat(m_currFrame);

		while (m_fAccumulator >= 1.0f)
		{
			m_fLastEmitTime += fEmitInterval;
			m_fLastEmitTime = ntstd::Clamp( m_fLastEmitTime, m_fLastAge, m_fCurrAge );

			float fNormalisedT = (m_fLastEmitTime - m_fLastAge) / fTimeDelta;

			CMatrix interpolated;
			interpolated.SetFromQuat( CQuat::Slerp( startQuat, endQuat, fNormalisedT ) );

			// evaluate our catmull for the frame translation
			CVector pos = coeffs[0];
			for (int j = 1; j < 4; j++)
			{
				pos *= fNormalisedT;
				pos += coeffs[j];
			}
			pos.W() = 0.0f;
			interpolated.SetTranslation( CPoint( pos ) );

			m_pTrail->EmitEdge( m_fLastEmitTime, interpolated );
			m_fAccumulator-=1.0f;
		}
	}

	return m_bKillMeNow;
}
