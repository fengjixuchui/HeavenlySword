//--------------------------------------------------
//!
//!	\file colour_function.cpp
//!	Assorted editable objects that represent colour
//! functions used by effects
//!
//--------------------------------------------------

#include "colour_function.h"
#include "gfx/TextureReader.h"
#include "effect_shims.h"
#include "effect_util.h"
#include "effect_manager.h"
#include "functioncurve.h"
#include "functiongraph.h"

#include "gfx/surfacemanager.h"
#include "gfx/hardwarecaps.h"
#include "core/visualdebugger.h"
#include "core/timer.h"
#include "core/listtools.h"
#include "input/mouse.h"
#include "objectdatabase/neteditinterface.h"
#include "objectdatabase/dataobject.h"

START_STD_INTERFACE( ColourFunction_Lerp )
	
	ILIGHT( ColourFunction_Lerp, ColourStart )
	ILIGHT( ColourFunction_Lerp, ColourEnd )

	IFLOAT( ColourFunction_Lerp, AlphaStart )
	IFLOAT( ColourFunction_Lerp, AlphaEnd )

	DECLARE_POSTCONSTRUCT_CALLBACK( PostConstruct )
	DECLARE_EDITORCHANGEVALUE_CALLBACK( EditorChangeValue )
	DECLARE_EDITORCHANGEPARENT_CALLBACK( EditorChangeParent )
	DECLARE_DEBUGRENDER_FROMNET_CALLBACK( DebugRender )

END_STD_INTERFACE

START_STD_INTERFACE( ColourFunction_Palette )

	ILIGHT( ColourFunction_Palette, Colour0 )
	IFLOAT( ColourFunction_Palette, Alpha0 )
	IFLOAT( ColourFunction_Palette, Time0 )

	ILIGHT( ColourFunction_Palette, Colour1 )
	IFLOAT( ColourFunction_Palette, Alpha1 )
	IFLOAT( ColourFunction_Palette, Time1 )

	ILIGHT( ColourFunction_Palette, Colour2 )
	IFLOAT( ColourFunction_Palette, Alpha2 )
	IFLOAT( ColourFunction_Palette, Time2 )

	ILIGHT( ColourFunction_Palette, Colour3 )
	IFLOAT( ColourFunction_Palette, Alpha3 )
	IFLOAT( ColourFunction_Palette, Time3 )

	ILIGHT( ColourFunction_Palette, Colour4 )
	IFLOAT( ColourFunction_Palette, Alpha4 )
	IFLOAT( ColourFunction_Palette, Time4 )

	ILIGHT( ColourFunction_Palette, Colour5 )
	IFLOAT( ColourFunction_Palette, Alpha5 )
	IFLOAT( ColourFunction_Palette, Time5 )

	ILIGHT( ColourFunction_Palette, Colour6 )
	IFLOAT( ColourFunction_Palette, Alpha6 )
	IFLOAT( ColourFunction_Palette, Time6 )

	ILIGHT( ColourFunction_Palette, Colour7 )
	IFLOAT( ColourFunction_Palette, Alpha7 )
	IFLOAT( ColourFunction_Palette, Time7 )

	ILIGHT( ColourFunction_Palette, Colour8 )
	IFLOAT( ColourFunction_Palette, Alpha8 )
	IFLOAT( ColourFunction_Palette, Time8 )

	ILIGHT( ColourFunction_Palette, Colour9 )
	IFLOAT( ColourFunction_Palette, Alpha9 )
	IFLOAT( ColourFunction_Palette, Time9 )

	DECLARE_POSTCONSTRUCT_CALLBACK( PostConstruct )
	DECLARE_EDITORCHANGEVALUE_CALLBACK( EditorChangeValue )
	DECLARE_EDITORCHANGEPARENT_CALLBACK( EditorChangeParent )
	DECLARE_DEBUGRENDER_FROMNET_CALLBACK( DebugRender )

END_STD_INTERFACE

START_STD_INTERFACE( ColourFunction_Curves )

	PUBLISH_PTR_WITH_DEFAULT_AS	( m_pobRFunction, RFunction, FunctionCurve_User )
	PUBLISH_PTR_WITH_DEFAULT_AS	( m_pobGFunction, GFunction, FunctionCurve_User )
	PUBLISH_PTR_WITH_DEFAULT_AS	( m_pobBFunction, BFunction, FunctionCurve_User )
	PUBLISH_PTR_WITH_DEFAULT_AS	( m_pobAFunction, AFunction, FunctionCurve_User )
	
	PUBLISH_PTR_CONTAINER_AS(	m_obObjects,	Objects )

	DECLARE_POSTCONSTRUCT_CALLBACK( PostConstruct )
	DECLARE_EDITORCHANGEVALUE_CALLBACK( EditorChangeValue )
	DECLARE_EDITORCHANGEPARENT_CALLBACK( EditorChangeParent )
	DECLARE_DEBUGRENDER_FROMNET_CALLBACK( DebugRender )
	DECLARE_AUTOCONSTRUCT_CALLBACK( AutoConstruct );

END_STD_INTERFACE

//--------------------------------------------------
//!
//! ColourFunction::GenerateTexture
//! Make a texture called pName containing our
//! colour function.
//!
//--------------------------------------------------
Texture::Ptr ColourFunction::GenerateTexture( TEXGEN_MODE eMODE, u_int iResolution, bool bHDR, bool bSaveToDisk, const char* pName )
{
	// create texture of the right format
	Texture::Ptr result;
	GFXFORMAT format = GF_ARGB8;

	if (bHDR)
	{
		if ( HardwareCapabilities::Get().IsValidTextureFormat(GF_ABGR16F) )
			format = GF_ABGR16F;
		else if ( HardwareCapabilities::Get().IsValidTextureFormat(GF_ABGR32F) )
			format = GF_ABGR32F;
	}

	result = SurfaceManager::Get().CreateTexture( iResolution, 1, format );

	// lock it and fill it in with our colour values
	uint32_t pitch;
	TextureReader it( result->CPULock2D(pitch), format );
	
	for (u_int i = 0; i < iResolution; i++, it.Next() )	
	{
		float fU = _R(i) / (iResolution - 1);

		switch (eMODE)
		{
		default:
		case TEXGEN_NORMAL:
			it.Set( GetColour( fU ) );
			break;

		case TEXGEN_GREYSCALE_R:
			{
				float fChannel = GetColour( fU ).X();
				it.Set( CVector(fChannel,fChannel,fChannel,1.0f));
			}
			break;

		case TEXGEN_GREYSCALE_G:
			{
				float fChannel = GetColour( fU ).Y();
				it.Set( CVector(fChannel,fChannel,fChannel,1.0f));
			}
			break;

		case TEXGEN_GREYSCALE_B:
			{
				float fChannel = GetColour( fU ).Z();
				it.Set( CVector(fChannel,fChannel,fChannel,1.0f));
			}
			break;

		case TEXGEN_GREYSCALE_A:
			{
				float fChannel = GetColour( fU ).W();
				it.Set( CVector(fChannel,fChannel,fChannel,1.0f));
			}
			break;
		}
	}
	
	result->CPUUnlock2D();

#ifdef PLATFORM_PC // FIXME_WIL

	// now dump it to file 
	if (bSaveToDisk)
		result->m_Platform.SaveToDisk( pName, D3DXIFF_DDS, true, true );

#endif

	return result;
}

//--------------------------------------------------
//!
//! ColourFunction::GenerateDebugTextures
//! Just used in our debug render functions
//!
//--------------------------------------------------
void ColourFunction::GenerateDebugTextures()
{
	if (m_iLastGenerationTick != CTimer::Get().GetSystemTicks())
	{
		m_pDebugHDRTexture = GenerateTexture( TEXGEN_NORMAL, iDEBUG_RESOLUTION, true, false );
		m_pDebugLDRTexture = GenerateTexture( TEXGEN_NORMAL, iDEBUG_RESOLUTION, false, false );
		m_pDebugAlphaTexture = GenerateTexture( TEXGEN_GREYSCALE_A, iDEBUG_RESOLUTION, true, false );
		m_bDebugRangesCalced = false;
		m_iLastGenerationTick = CTimer::Get().GetSystemTicks();
	}
}

//--------------------------------------------------
//!
//!	draw what we look like
//!
//--------------------------------------------------
void ColourFunction::RenderDebugTex( float fWidth, float fHeight, float fStartX, float fStartY )
{
#ifndef _GOLD_MASTER
	if	(
		(!m_pDebugHDRTexture) ||
		(!m_pDebugLDRTexture) ||
		(!m_pDebugAlphaTexture)
		)
		GenerateDebugTextures();
	
	// HDR tex first
	{
		ScreenSprite* pSprite = NT_NEW_CHUNK ( Mem::MC_EFFECTS ) ScreenSprite;

		float fVPWidth = g_VisualDebug->GetDebugDisplayWidth();
		float fVPHeight = g_VisualDebug->GetDebugDisplayHeight();

		pSprite->SetPosition( CPoint( fVPWidth * fStartX, fVPHeight * fStartY,  0.0f ) );
		pSprite->SetWidth( fVPWidth * fWidth );
		pSprite->SetHeight( fVPHeight * fHeight );
		pSprite->SetTexture( m_pDebugHDRTexture );
		pSprite->SetColour(0xffffffff);

		ScreenSpriteShim* pShim = NT_NEW_CHUNK ( Mem::MC_EFFECTS ) ScreenSpriteShim( pSprite );
		pShim->m_bHDR = true;
		EffectManager::Get().AddEffect( pShim );
	}

	// LDR tex next
	{
		ScreenSprite* pSprite = NT_NEW_CHUNK ( Mem::MC_EFFECTS ) ScreenSprite;

		float fVPWidth = g_VisualDebug->GetDebugDisplayWidth();
		float fVPHeight = g_VisualDebug->GetDebugDisplayHeight();

		pSprite->SetPosition( CPoint( fVPWidth * fStartX, fVPHeight * (fStartY + (fHeight*1.1f)),  0.0f ) );
		pSprite->SetWidth( fVPWidth * fWidth );
		pSprite->SetHeight( fVPHeight * fHeight );
		pSprite->SetTexture( m_pDebugLDRTexture );
		pSprite->SetColour(0xffffffff);

		ScreenSpriteShim* pShim = NT_NEW_CHUNK ( Mem::MC_EFFECTS )  ScreenSpriteShim( pSprite );
		pShim->m_bHDR = false;
		EffectManager::Get().AddEffect( pShim );
	}

	// alpha greyscale last
	{
		ScreenSprite* pSprite = NT_NEW_CHUNK ( Mem::MC_EFFECTS ) ScreenSprite;

		float fVPWidth = g_VisualDebug->GetDebugDisplayWidth();
		float fVPHeight = g_VisualDebug->GetDebugDisplayHeight();

		pSprite->SetPosition( CPoint( fVPWidth * fStartX, fVPHeight * (fStartY + (fHeight*2.2f)),  0.0f ) );
		pSprite->SetWidth( fVPWidth * fWidth );
		pSprite->SetHeight( fVPHeight * fHeight );
		pSprite->SetTexture( m_pDebugAlphaTexture );
		pSprite->SetColour(0xffffffff);

		ScreenSpriteShim* pShim = NT_NEW_CHUNK ( Mem::MC_EFFECTS )  ScreenSpriteShim( pSprite );
		pShim->m_bHDR = false;
		EffectManager::Get().AddEffect( pShim );
	}
#endif
}

//--------------------------------------------------
//!
//!	get our debug extents
//!
//--------------------------------------------------
void	ColourFunction::CalcDebugBounds()
{
	if (!m_bDebugRangesCalced)
	{
		m_fMax = -MAX_POS_FLOAT;

		for (int i = 0; i < iDEBUG_RESOLUTION; i++)
		{
			float fU = _R(i) / (iDEBUG_RESOLUTION-1);
			CPoint rgb( GetColour( fU ) );

			for (int j = 0; j < 3; j++)
			{
				float fVal = rgb[j];

				if (fVal > m_fMax)
					m_fMax = fVal;
			}
		}

		m_fMax = (m_fMax > 1.0f) ? m_fMax : 1.0f;
		m_bDebugRangesCalced = true;
	}
}

//--------------------------------------------------
//!
//!	draw what we look like as curves
//!
//--------------------------------------------------
void ColourFunction::RenderDebugGraph( float fminX, float fmaxX, float fminY, float fmaxY )
{
#ifndef _GOLD_MASTER
	// get curve bounds
	CalcDebugBounds();

	// draw our colour curves
	FunctionGraph graph( fminX, fmaxX, fminY, fmaxY );
	graph.DrawBounds(m_borderCol);

	for (int i = 0; i < iDEBUG_RESOLUTION; i++)
	{
		float fU = _R(i)	/ iDEBUG_RESOLUTION;
		float fV = _R(i+1)	/ iDEBUG_RESOLUTION;

		CVector	rgba_start( GetColour( fU ) );
		CVector	rgba_end( GetColour( fV ) );
		
		for (int j = 0; j < 3; j++)
		{
			CPoint curr( fU, (rgba_start[j]) / m_fMax, 0.0f );
			CPoint next( fV, (rgba_end[j]) / m_fMax, 0.0f );

			graph.DrawLine( curr, next, m_aDebugCurveCol[j] );
		}

		// alpha is a special case, as we should always range from 0 -> 1
		CPoint curr( fU, rgba_start.W(), 0.0f );
		CPoint next( fV, rgba_end.W(), 0.0f );

		graph.DrawLine( curr, next, m_aDebugCurveCol[3] );
	}

	// draw some scales ?
#endif
}






//--------------------------------------------------
//!
//! ColourFunction_Lerp ctor
//!
//--------------------------------------------------
ColourFunction_Lerp::ColourFunction_Lerp() :
	m_obColourStart(1.0f,1.0f,1.0f,1.0f),
	m_obColourEnd(1.0f,1.0f,1.0f,1.0f),
	m_fAlphaStart(1.0f),
	m_fAlphaEnd(1.0f)
{
}

ColourFunction_Lerp::~ColourFunction_Lerp()
{
}

//--------------------------------------------------
//!
//!	validate our values
//!
//--------------------------------------------------
void ColourFunction_Lerp::PostConstruct()
{
	m_obColourStart.W() = ntstd::Max( m_obColourStart.W(), 0.0f );
	m_obColourEnd.W() = ntstd::Max( m_obColourEnd.W(), 0.0f );
	m_fAlphaStart = ntstd::Clamp( m_fAlphaStart, 0.0f, 1.0f );
	m_fAlphaEnd = ntstd::Clamp( m_fAlphaEnd, 0.0f, 1.0f );
}

//--------------------------------------------------
//!
//!	refresh our internal state after editing
//!
//--------------------------------------------------
bool ColourFunction_Lerp::EditorChangeValue( CallBackParameter, CallBackParameter )
{
	PostConstruct();
	GenerateDebugTextures();
	m_bHasChanged = true;
	return true;
}

//--------------------------------------------------
//!
//!	refresh our internal state after sub object editing
//!
//--------------------------------------------------
void ColourFunction_Lerp::EditorChangeParent()
{
	PostConstruct();
	GenerateDebugTextures();
}

//--------------------------------------------------
//!
//!	draw what we look like
//!
//--------------------------------------------------
void ColourFunction_Lerp::DebugRender( void )
{
	// draw our debug textures
	RenderDebugTex( 0.6f,  0.02f, 0.5f, 0.82f );

	// draw our colour curves
	RenderDebugGraph( 0.2f, 0.8f, 0.2f, 0.8f );
}









//--------------------------------------------------
//!
//! ColourFunction_Palette ctor
//!
//--------------------------------------------------
ColourFunction_Palette::ColourFunction_Palette() :
	m_pGenerated( 0 )
{
	m_data[0].Set( &m_obColour0, &m_fAlpha0, &m_fTime0 );
	m_data[1].Set( &m_obColour1, &m_fAlpha1, &m_fTime1 );
	m_data[2].Set( &m_obColour2, &m_fAlpha2, &m_fTime2 );
	m_data[3].Set( &m_obColour3, &m_fAlpha3, &m_fTime3 );
	m_data[4].Set( &m_obColour4, &m_fAlpha4, &m_fTime4 );
	m_data[5].Set( &m_obColour5, &m_fAlpha5, &m_fTime5 );
	m_data[6].Set( &m_obColour6, &m_fAlpha6, &m_fTime6 );
	m_data[7].Set( &m_obColour7, &m_fAlpha7, &m_fTime7 );
	m_data[8].Set( &m_obColour8, &m_fAlpha8, &m_fTime8 );
	m_data[9].Set( &m_obColour9, &m_fAlpha9, &m_fTime9 );

	for (int i = 0; i < MAX_PALETTE_ENTRIES; i++)
	{
		*m_data[i].m_pLight = CVector( 1.0f, 1.0f, 1.0f, 1.0f );
		*m_data[i].m_pAlpha = 1.0f;
		*m_data[i].m_pTime = -1.0f;
	}
}

//--------------------------------------------------
//!
//! ColourFunction_Palette ctor
//!
//--------------------------------------------------
ColourFunction_Palette::~ColourFunction_Palette()
{
	if (m_pGenerated)
	{
		NT_DELETE_CHUNK( Mem::MC_EFFECTS, m_pGenerated );
	}
}

//--------------------------------------------------
//!
//! ColourFunction_Palette ctor
//!
//--------------------------------------------------
CVector	ColourFunction_Palette::GetColour( float fU ) const
{
	CVector result( 1.0f, 1.0f, 1.0f, 1.0f );

	if (m_pGenerated)
		result = m_pGenerated->Evaluate( fU );

	return result;
}

//--------------------------------------------------
//!
//! ColourFunction_Palette ctor
//!
//--------------------------------------------------
void ColourFunction_Palette::PostConstruct()
{
	ntstd::List<PaletteNode*, Mem::MC_EFFECTS> temp;

	for (int i = 0; i < MAX_PALETTE_ENTRIES; i++)
	{
		if ( *m_data[i].m_pTime >= 0.0f )
		{
			// this node is valid, stick it in the list
			PaletteNode* pNew = NT_NEW_CHUNK ( Mem::MC_EFFECTS ) PaletteNode( m_data[i].GetColour(), *m_data[i].m_pTime );
			temp.push_back( pNew );
		}
	}

	if (temp.size() > 1)
	{
		// whip through our list of nodes and construct a fitted curve from the results.
		bubble_sort( temp.begin(), temp.end(), CComparator_PaletteNode_LT() );

		// make sure our times increase
		ntstd::List<PaletteNode*, Mem::MC_EFFECTS >::iterator curr = temp.begin();
		ntstd::List<PaletteNode*, Mem::MC_EFFECTS >::iterator next = curr;
		next++;

		for ( ; (curr != temp.end()) && (next != temp.end()); ++curr, ++next )
		{
			if ( (*next)->m_fTime < (*curr)->m_fTime )
				(*next)->m_fTime = (*curr)->m_fTime + (EPSILON * 2.0f);

			if (((*next)->m_fTime - (*curr)->m_fTime) < EPSILON)
				(*next)->m_fTime += EPSILON * 2.0f;
		}

		TimeCurve_Fitted fitter;

		for (	ntstd::List<PaletteNode*, Mem::MC_EFFECTS >::iterator it = temp.begin();
				it != temp.end(); ++it )
		{						
			fitter.AddSampleToFit( (*it)->m_rgba, (*it)->m_fTime );
		}

		if (fitter.Finalise())
		{
			if (m_pGenerated)
			{
				NT_DELETE_CHUNK( Mem::MC_EFFECTS, m_pGenerated );
			}
			m_pGenerated = fitter.Reset( true );
		}
	}

	while (!temp.empty())
	{
		NT_DELETE_CHUNK( Mem::MC_EFFECTS, temp.back() );
		temp.pop_back();
	}
}

//--------------------------------------------------
//!
//!	refresh our internal state after editing
//!
//--------------------------------------------------
bool ColourFunction_Palette::EditorChangeValue( CallBackParameter,CallBackParameter )
{
	PostConstruct();
	GenerateDebugTextures();
	m_bHasChanged = true;
	return true;
}

//--------------------------------------------------
//!
//!	refresh our internal state after sub object editing
//!
//--------------------------------------------------
void ColourFunction_Palette::EditorChangeParent()
{
	PostConstruct();
	GenerateDebugTextures();
}

//--------------------------------------------------
//!
//!	draw what we look like
//!
//--------------------------------------------------
void ColourFunction_Palette::DebugRender( void )
{
	// draw our debug textures
	RenderDebugTex( 0.6f, 0.02f, 0.5f, 0.82f );

	// draw our colour curves
	RenderDebugGraph( 0.2f, 0.8f, 0.2f, 0.8f );
}








//--------------------------------------------------
//!
//! ColourFunction_Curves ctor
//!
//--------------------------------------------------
ColourFunction_Curves::ColourFunction_Curves() :
	m_pobRFunction(0),
	m_pobGFunction(0),
	m_pobBFunction(0),
	m_pobAFunction(0),
	m_eOveride(OS_NONE),
	m_pOverideTemp(0)
{
	m_bForceNewDebugTex = false;
}

//--------------------------------------------------
//!
//! ColourFunction_Curves dtor
//!
//--------------------------------------------------
ColourFunction_Curves::~ColourFunction_Curves()
{
	CleanOveride();
}

//--------------------------------------------------
//!
//! ColourFunction_Curves ctor
//!
//--------------------------------------------------
CVector	ColourFunction_Curves::GetColour( float fU ) const
{
	CVector result( m_pobRFunction->EvaluateScaledAndOffset( fU ),
					m_pobGFunction->EvaluateScaledAndOffset( fU ),
					m_pobBFunction->EvaluateScaledAndOffset( fU ),
					m_pobAFunction->EvaluateScaledAndOffset( fU ) );

	if (m_pOverideTemp)
	{
		switch( m_eOveride )
		{
		default:
		case OS_NONE:
			break;

		case OS_RED:
			result.X() =	(m_pOverideTemp->Evaluate( fU ).X() * m_pobRFunction->m_fScale) +
							m_pobRFunction->m_fOffset;
			break;

		case OS_GREEN:
			result.Y() =	(m_pOverideTemp->Evaluate( fU ).X() * m_pobGFunction->m_fScale) +
							m_pobGFunction->m_fOffset;
			break;

		case OS_BLUE:
			result.Z() =	(m_pOverideTemp->Evaluate( fU ).X() * m_pobBFunction->m_fScale) +
							m_pobBFunction->m_fOffset;
			break;

		case OS_ALPHA:
			result.W() =	(m_pOverideTemp->Evaluate( fU ).X() * m_pobAFunction->m_fScale) +
							m_pobAFunction->m_fOffset;
			break;
		}
	}

	result.W() = ntstd::Clamp( result.W(), 0.0f, 1.0f );
	return result;
}

//--------------------------------------------------
//!
//!	Add new objects for serialising
//!
//--------------------------------------------------
void ColourFunction_Curves::PostConstruct()
{
}

void ColourFunction_Curves::AutoConstruct( const DataInterfaceField* pField )
{
	static const float aValues[] = { 1.0f, 1.0f };
	static const float aTimes[] = { 0.0f, 1.0f };

	if( pField->GetName() == "RFunction" )
	{
		m_pobRFunction->InitialiseToArrayValues( 2, aValues, aTimes );
	} else
	if( pField->GetName() == "GFunction" )
	{
		m_pobGFunction->InitialiseToArrayValues( 2, aValues, aTimes );
	} else
	if( pField->GetName() == "BFunction" )
	{
		m_pobBFunction->InitialiseToArrayValues( 2, aValues, aTimes );
	} else
	if( pField->GetName() == "AFunction" )
	{
		m_pobAFunction->InitialiseToArrayValues( 2, aValues, aTimes );
	}
}

//--------------------------------------------------
//!
//!	refresh our internal state after editing
//!
//--------------------------------------------------
bool ColourFunction_Curves::EditorChangeValue( CallBackParameter, CallBackParameter )
{
	PostConstruct();
	GenerateDebugTextures();
	m_bHasChanged = true;
	m_bForceNewDebugTex = false;
	return true;
}

//--------------------------------------------------
//!
//!	refresh our internal state after sub object editing
//!
//--------------------------------------------------
void ColourFunction_Curves::EditorChangeParent()
{
	PostConstruct();
	GenerateDebugTextures();
	m_bForceNewDebugTex = false;
}

//--------------------------------------------------
//!
//!	startup some overide stuff
//!
//--------------------------------------------------
void ColourFunction_Curves::StartOveride( OVERIDE_STATUS type )
{
	ntError( m_overideSamples.empty() );
	ntError( m_pOverideTemp == 0 );
	ntError( m_eOveride == OS_NONE );

	m_eOveride = type;
	m_fLastTime = -1.0f;

	m_aDebugCurveCol[0] = NTCOLOUR_ARGB(0x80, 0x80, 0, 0);
	m_aDebugCurveCol[1] = NTCOLOUR_ARGB(0x80, 0, 0x80, 0);
	m_aDebugCurveCol[2] = NTCOLOUR_ARGB(0x80, 0, 0, 0x80);
	m_aDebugCurveCol[3] = NTCOLOUR_ARGB(0x80, 0x80, 0x80, 0x80);
	m_borderCol = NTCOLOUR_ARGB(0x80, 0x80, 0, 0);

	switch( m_eOveride )
	{
	case OS_RED:
		m_aDebugCurveCol[0] = NTCOLOUR_ARGB(0xff, 0xff, 0, 0);
		m_pOverideFunc = m_pobRFunction;
		break;

	case OS_GREEN:	m_aDebugCurveCol[1] = NTCOLOUR_ARGB(0xff, 0, 0xff, 0);
		m_pOverideFunc = m_pobGFunction;
		break;

	case OS_BLUE:	m_aDebugCurveCol[2] = NTCOLOUR_ARGB(0xff, 0, 0, 0xff);
		m_pOverideFunc = m_pobBFunction;
		break;

	case OS_ALPHA:	m_aDebugCurveCol[3] = NTCOLOUR_ARGB(0xff, 0xff, 0xff, 0xff);
		m_pOverideFunc = m_pobAFunction;
		break;

	default:
		ntAssert(0);
	}
}

static void ConvertMouseToInputSpace( float& fX, float& fY )
{
	static const float fMouseXMin = 0.2f;
	static const float fMouseXRange = 0.6f;

	static const float fMouseYMin = 0.2f;
	static const float fMouseYRange = 0.6f;

	fX = (MouseInput::Get().GetMousePos().X() - fMouseXMin) / fMouseXRange;
	fY = (MouseInput::Get().GetMousePos().Y() - fMouseYMin) / fMouseYRange;

	fX = ntstd::Clamp( fX, 0.0f, 1.0f );
	fY = ntstd::Clamp( fY, 0.0f, 1.0f );

	fY = 1.0f - fY; // invert Y direction
}

//--------------------------------------------------
//!
//!	update some overide stuff
//!
//--------------------------------------------------
void ColourFunction_Curves::UpdateOveride()
{
	ntError( m_eOveride != OS_NONE );
	bool bRefreshReq = false;

	// check for new samples
	float fX, fY;
	ConvertMouseToInputSpace(fX, fY);

	if ( MouseInput::Get().GetButtonState(MOUSE_LEFT).GetHeld() )
	{
		// adjust Y to fit in graph scale
		if (m_eOveride != OS_ALPHA)
		{
			float fGraphMin, fGraphMax;
			GetDebugBounds( fGraphMin, fGraphMax );
			fY = ((fGraphMax - fGraphMin) * fY) + fGraphMin;
		}

		// now bring back into curve space
		fY -= m_pOverideFunc->m_fOffset;
		fY /= m_pOverideFunc->m_fScale;

		if (fX > m_fLastTime)
		{
			m_fLastTime = fX;
			m_overideSamples.push_back( NT_NEW CVector( fX, fY, 0.0f, 0.0f ) );
			bRefreshReq = true;
		}
		else
		{
			ntError( !m_overideSamples.empty() );
			m_overideSamples.back()->Y() = fY;
		}
	}

	// generate our current temporary fitted curve
	if(bRefreshReq)
	{
		if (m_pOverideTemp)
		{
			NT_DELETE_CHUNK( Mem::MC_EFFECTS, m_pOverideTemp );
			m_pOverideTemp = 0;
		}

		if (!m_overideSamples.empty())
		{
			FunctionCurve_Fitted fitter(m_pOverideFunc->m_fTolerance);
			float fLastTime = -1.0f;

			for (	ntstd::List< CVector*, Mem::MC_EFFECTS >::iterator it = m_overideSamples.begin();
					it  != m_overideSamples.end(); ++it )
			{
				fitter.AddSampleToFit( (*it)->Y(), (*it)->X() );
				fLastTime = (*it)->X();
			}

			if(fLastTime < 1.0f)
			{
				// add a tail consisting of the original curve
				float fStep = 1.0f / _R( iDEBUG_RESOLUTION );

				const FunctionCurve_Fitted* pCurve = m_pOverideFunc->GetFittedCurve();
				ntError( pCurve );

				for ( float fU = ntstd::Max( fLastTime+fStep, 0.0f ); (fU + fStep) < 1.0f; fU += fStep )
					fitter.AddSampleToFit( pCurve->Evaluate(fU), fU );

				fitter.AddSampleToFit( pCurve->Evaluate(1.0f), 1.0f );
			}

			if (fitter.Finalise())
				m_pOverideTemp = fitter.Reset( true );
		}

		// finally update our textures
		GenerateDebugTextures();
		m_bForceNewDebugTex = false;
	}

	// if m_pOverideTemp is null at this point, we have no samples or the fitting has failed.
	bool bExitOveride = false;

	// finally check to see if we should finish our overide code...
	if ( MouseInput::Get().GetButtonState(MOUSE_LEFT).GetReleased() )
		bExitOveride = true;

	switch( m_eOveride )
	{
	case OS_RED:
		if (!CInputHardware::Get().GetKeyboard().IsKeyHeld( KEYC_Q, KEYM_CTRL ))
			bExitOveride = true;
		break;

	case OS_GREEN:
		if (!CInputHardware::Get().GetKeyboard().IsKeyHeld( KEYC_W, KEYM_CTRL ))
			bExitOveride = true;
		break;

	case OS_BLUE:
		if (!CInputHardware::Get().GetKeyboard().IsKeyHeld( KEYC_E, KEYM_CTRL ))
			bExitOveride = true;
		break;

	case OS_ALPHA:
		if (!CInputHardware::Get().GetKeyboard().IsKeyHeld( KEYC_R, KEYM_CTRL ))
			bExitOveride = true;
		break;

	default:
		ntAssert(0);
	}

	if (bExitOveride)
	{
		// yup, cleanup
		if (m_pOverideTemp)
		{
			m_pOverideFunc->ForceCurve( m_pOverideTemp );
			m_pOverideTemp = 0;
		}

		CleanOveride();

		m_aDebugCurveCol[0] = NTCOLOUR_ARGB(0xff, 0xff, 0, 0);
		m_aDebugCurveCol[1] = NTCOLOUR_ARGB(0xff, 0, 0xff, 0);
		m_aDebugCurveCol[2] = NTCOLOUR_ARGB(0xff, 0, 0, 0xff);
		m_aDebugCurveCol[3] = NTCOLOUR_ARGB(0xff, 0xff, 0xff, 0xff);
		m_borderCol = NTCOLOUR_ARGB(0xff, 0, 0, 0);

		m_bHasChanged = true;
	}
}

//--------------------------------------------------
//!
//!	cleanup some overide stuff
//!
//--------------------------------------------------
void ColourFunction_Curves::CleanOveride()
{
	m_eOveride = OS_NONE;

	if (m_pOverideTemp)
	{
		NT_DELETE_CHUNK( Mem::MC_EFFECTS, m_pOverideTemp );
	}
	m_pOverideTemp = 0;

	while ( !m_overideSamples.empty() )
	{
		NT_DELETE_CHUNK( Mem::MC_EFFECTS, m_overideSamples.back() );
		m_overideSamples.pop_back();
	}
}

//--------------------------------------------------
//!
//!	check and clear status of sub curves
//!
//--------------------------------------------------
bool ColourFunction_Curves::SubCurvesChanged() const
{
	// (calling all 4 is intentional)
	bool bChanged = false;
	bChanged = m_pobRFunction->DetectCurveChanged() ? true : bChanged;
	bChanged = m_pobGFunction->DetectCurveChanged() ? true : bChanged;
	bChanged = m_pobBFunction->DetectCurveChanged() ? true : bChanged;
	bChanged = m_pobAFunction->DetectCurveChanged() ? true : bChanged;

	return bChanged;
}

//--------------------------------------------------
//!
//!	draw what we look like
//!
//--------------------------------------------------
void ColourFunction_Curves::DebugRender( void )
{
	DataObject* pDO = CNetEditInterface::Get().GetSelected();
	FunctionCurve_User* pEdited = 0;
	if( pDO )
		pEdited = (FunctionCurve_User*) pDO->GetBasePtr();

	if	(
		(m_pobRFunction == pEdited) ||
		(m_pobGFunction == pEdited) ||
		(m_pobBFunction == pEdited) ||
		(m_pobAFunction == pEdited)
		)
		return;

	if (SubCurvesChanged())
	{
		m_bHasChanged = true;
		m_bForceNewDebugTex = true;
	}

	if (m_bForceNewDebugTex)
	{
		GenerateDebugTextures();
		m_bForceNewDebugTex = false;
	}

	// check to see if we're doing any overide stuff
	if ( m_eOveride != OS_NONE )
	{
		UpdateOveride();
	}
	else if ( MouseInput::Exists() )
	{
		if (CInputHardware::Get().GetKeyboard().IsKeyPressed( KEYC_Q, KEYM_CTRL ))
			StartOveride( OS_RED );

		else if (CInputHardware::Get().GetKeyboard().IsKeyPressed( KEYC_W, KEYM_CTRL ))
			StartOveride( OS_GREEN );

		else if (CInputHardware::Get().GetKeyboard().IsKeyPressed( KEYC_E, KEYM_CTRL ))
			StartOveride( OS_BLUE );

		else if (CInputHardware::Get().GetKeyboard().IsKeyPressed( KEYC_R, KEYM_CTRL ))
			StartOveride( OS_ALPHA );
	}

	// draw our debug textures
	RenderDebugTex( 0.6f,  0.02f, 0.5f, 0.82f );
	
	// draw our colour curves
	RenderDebugGraph( 0.2f, 0.8f, 0.2f, 0.8f );
}















