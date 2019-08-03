//------------------------------------------------------------------------------------------
//!
//!	\file stylebar.cpp
//!
//------------------------------------------------------------------------------------------

// Necessary includes
#include "game/combatstyle.h"
#include "hud/hudmanager.h"
#include "hud/stylelabel.h"
#include "objectdatabase/dataobject.h"

#include "game/nsmanager.h"
#include "gui/guimanager.h"

#include "hud/messagedata.h"

#include "core/visualdebugger.h"

// ---- Forward references ----

// ---- Interfaces ----

START_STD_INTERFACE( StyleLabelRenderDef )
	DEFINE_INTERFACE_INHERITANCE(CBlendableHudUnitDef)
	COPY_INTERFACE_FROM(CBlendableHudUnitDef)

	PUBLISH_VAR_WITH_DEFAULT_AS( m_fTopLeftX, 0.5f, TopLeftX )
	PUBLISH_VAR_WITH_DEFAULT_AS( m_fTopLeftY, 0.5f, TopLeftY )
	PUBLISH_VAR_WITH_DEFAULT_AS( m_fScale, 1.0f, OverallScale )
	PUBLISH_VAR_WITH_DEFAULT_AS( m_fPositiveYOffset, -0.1f, PositiveYOffset )
	PUBLISH_VAR_WITH_DEFAULT_AS( m_fNegitiveYOffset,  0.1f, NegitiveYOffset )

	PUBLISH_VAR_AS(	m_obPositiveColour, PositiveColour)
	PUBLISH_VAR_AS(	m_obNegativeColour, NegativeColour)

	PUBLISH_VAR_WITH_DEFAULT_AS( m_fOnScreenTime,  1.0f, OnScreenTime )
	PUBLISH_VAR_WITH_DEFAULT_AS( m_obStyleLabelFont, "Body", StyleLabelFont )
	
	PUBLISH_VAR_WITH_DEFAULT_AS( m_obPositiveVelocity,		CPoint(CONSTRUCT_CLEAR),	PositiveVelocity)
	PUBLISH_VAR_WITH_DEFAULT_AS( m_obPositiveAcceleration,	CPoint(CONSTRUCT_CLEAR),	PositiveAcceleration)
	
	PUBLISH_VAR_WITH_DEFAULT_AS( m_obNegativeVelocity,		CPoint(CONSTRUCT_CLEAR),	NegativeVelocity)
	PUBLISH_VAR_WITH_DEFAULT_AS( m_obNegativeAcceleration,	CPoint(CONSTRUCT_CLEAR),	NegativeAcceleration)
	
	PUBLISH_GLOBAL_ENUM_AS		(m_eBlendMode, BlendMode, EFFECT_BLENDMODE)
END_STD_INTERFACE

void ForceLinkFunctionStyleLabel()
{
	ntPrintf("!ATTN! Calling ForceLinkFunctionStyleLabel() !ATTN!\n");
}

//------------------------------------------------------------------------------------------
//!
//!	StyleLabelRenderDef::StyleLabelRenderDef
//! Construction
//!
//------------------------------------------------------------------------------------------
StyleLabelRenderDef::StyleLabelRenderDef( )
:	m_fTopLeftX( 0.1f )
,	m_fTopLeftY( 0.9f )
,	m_fScale( 1.0f )
,	m_obPositiveColour( 1.0f, 1.0f, 1.0f, 1.0f)
,	m_obNegativeColour( 1.0f, 1.0f, 1.0f, 1.0f)
{
	m_eBlendMode = EBM_LERP;
}


//------------------------------------------------------------------------------------------
//!
//!	StyleLabelRenderDef::~StyleLabelRenderDef
//! Destruction
//!
//------------------------------------------------------------------------------------------
StyleLabelRenderDef::~StyleLabelRenderDef( void )
{
}

//------------------------------------------------------------------------------------------
//!
//!	StyleLabelRenderDef::CreateInstance
//! Create instance of renderer
//!
//------------------------------------------------------------------------------------------

CHudUnit* StyleLabelRenderDef::CreateInstance( void ) const
{
	return NT_NEW_CHUNK (Mem::MC_MISC) StyleLabelRenderer( (StyleLabelRenderDef*)this );
}

//------------------------------------------------------------------------------------------
//!
//!	StyleLabelRenderer::StyleLabelRenderer
//! Construction
//!
//------------------------------------------------------------------------------------------
StyleLabelRenderer::StyleLabelRenderer( StyleLabelRenderDef*  pobRenderDef )
:	CBlendableHudUnit ( pobRenderDef )
,	m_pobDef ( pobRenderDef )
,	m_pobTextRenderer ( 0 )
,	m_fCurrScreenTime ( 0.0f )
#ifdef _DEBUG
,	m_fPosOffset ( 0.0f )
#endif
,	m_obCurrPosition(CONSTRUCT_CLEAR)
,	m_obCurrVelocity(CONSTRUCT_CLEAR)
,	m_obCurrAcceleration(CONSTRUCT_CLEAR)
{
}

//------------------------------------------------------------------------------------------
//!
//!	StyleLabelRenderer::~StyleLabelRenderer
//! Destruction
//!
//------------------------------------------------------------------------------------------
StyleLabelRenderer::~StyleLabelRenderer( void )
{
	if (m_pobTextRenderer)
	{
		NT_DELETE_CHUNK ( Mem::MC_MISC, m_pobTextRenderer );
	}
}

//------------------------------------------------------------------------------------------
//!
//!	StyleLabelRenderer::Initialise
//!
//------------------------------------------------------------------------------------------
bool StyleLabelRenderer::Initialise( StyleEvent& obEvent, float fPosOffset )
{
	ntAssert ( m_pobDef );

	// Scale our heights and positions accordingly
	//float fBBHeight = CGuiManager::Get().BBHeight();
	float fBBWidth = CGuiManager::Get().BBWidth();

	float fTextScale = fBBWidth/1280.0f;

	m_obEvent = obEvent;

#ifdef _DEBUG
	m_fPosOffset = fPosOffset;
#endif

	m_fCurrScreenTime = m_pobDef->m_fOnScreenTime;

	ntAssert ( m_obEvent.m_pobDef );

	if (! ntStr::IsNull( m_obEvent.m_pobDef->m_obEventString ) )
	{
		m_obTextRenderDef.m_obStringTextID = m_obEvent.m_pobDef->m_obEventString;
		m_obTextRenderDef.m_obFontName = m_pobDef->m_obStyleLabelFont;
		m_obTextRenderDef.m_fTopLeftX = fPosOffset;
		m_obTextRenderDef.m_fTopLeftY = m_pobDef->m_fTopLeftY;
		m_obTextRenderDef.m_fOverallScale = fTextScale;
		if ( m_obEvent.GetStyleValue() >= 0.0f )
		{
			m_obTextRenderDef.m_fTopLeftY += m_pobDef->m_fPositiveYOffset;
			m_obTextColour = m_pobDef->m_obPositiveColour;
			m_obCurrVelocity = m_pobDef->m_obPositiveVelocity;
			m_obCurrAcceleration = m_pobDef->m_obPositiveAcceleration;
		}
		else
		{
			m_obTextRenderDef.m_fTopLeftY += m_pobDef->m_fNegitiveYOffset;
			m_obTextColour = m_pobDef->m_obNegativeColour;
			m_obCurrVelocity = m_pobDef->m_obNegativeVelocity;
			m_obCurrAcceleration = m_pobDef->m_obNegativeAcceleration;
		}

		m_obCurrPosition = CPoint ( m_obTextRenderDef.m_fTopLeftX, m_obTextRenderDef.m_fTopLeftY, 0.0f);
		m_pobTextRenderer = (HudTextRenderer*)m_obTextRenderDef.CreateInstance();
		
		CHashedString obKey("STYLE");
		CHud::Get().GetMessageDataManager()->SetValue( obKey, m_obEvent.GetStyleValue() );
		m_pobTextRenderer->Initialise();	
	}

	BeginEnter();

	return true;
}

//------------------------------------------------------------------------------------------
//!
//!	StyleLabelRenderer::Render
//!
//------------------------------------------------------------------------------------------
bool StyleLabelRenderer::Render( void )
{
	if ( NSManager::Get().IsNinjaSequencePlaying() )
		return true;

	ntAssert (m_pobDef);
	ntAssert (m_obEvent.m_pobDef);

	// If deactive - 'cos we should render Enter and Exit states too
	if ( IsDeactive() )
		return true;

	//RenderStateBlock::SetBlendMode(m_pobDef->m_eBlendMode);

#ifdef _DEBUG
	// Scale our heights and positions accordingly
	float fBBHeight = CGuiManager::Get().BBHeight();
	float fBBWidth = CGuiManager::Get().BBWidth();

	if ( m_obEvent.m_iStyleValue > 0 )
	{
		g_VisualDebug->Printf2D( m_pobDef->m_fTopLeftX * fBBWidth,  m_pobDef->m_fTopLeftY * fBBHeight, \
			DC_GREEN, 0, "%s +%i\n", ntStr::GetString( ObjectDatabase::Get().GetNameFromPointer( m_obEvent.m_pobDef ) ), m_obEvent.m_iStyleValue );
	}
	else
	{
		g_VisualDebug->Printf2D( m_pobDef->m_fTopLeftX * fBBWidth, m_pobDef->m_fTopLeftY * fBBHeight, \
			DC_RED, 0, "%s %i\n", ntStr::GetString( ObjectDatabase::Get().GetNameFromPointer( m_obEvent.m_pobDef ) ), m_obEvent.m_iStyleValue );
	}

	g_VisualDebug->Printf2D( m_fPosOffset * fBBWidth, m_pobDef->m_fTopLeftY * fBBHeight, DC_WHITE, 0, "|\n");

#endif // _DEBUG

	if (m_pobTextRenderer)
	{
		CVector obTextColour( m_obTextColour );
		obTextColour.W() *= m_fOverallAlpha;
		m_pobTextRenderer->SetColour( obTextColour );
		m_pobTextRenderer->Render();
	}
	
	return true;
}

//------------------------------------------------------------------------------------------
//!
//!	StyleLabelRenderer::Update
//! Update
//!
//------------------------------------------------------------------------------------------
bool StyleLabelRenderer::Update( float fTimeStep )
{
	ntAssert( m_pobDef );

	// Update base unit
	CBlendableHudUnit::Update( fTimeStep );
	
	if ( m_fCurrScreenTime > 0.0f )
	{
		m_fCurrScreenTime -= fTimeStep;

		if ( m_fCurrScreenTime <= 0.0f )
		{
			BeginExit( true );
		}
	}

	// All done, request that I'm removed now
	if ( IsDeactive() )
		return false;

	m_obCurrVelocity += m_obCurrAcceleration * fTimeStep;
	m_obCurrPosition += m_obCurrVelocity * fTimeStep;

	if (m_pobTextRenderer)
	{
		m_pobTextRenderer->Update( fTimeStep );
		m_pobTextRenderer->SetPosition( m_obCurrPosition.X(), m_obCurrPosition.Y() );
	}
	
	return true;
}
