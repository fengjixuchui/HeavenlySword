//------------------------------------------------------------------------------------------
//!
//!	\file buttonhinter.cpp
//!
//------------------------------------------------------------------------------------------

// Necessary includes
#include "hud/hudimage.h"
#include "objectdatabase/dataobject.h"
#include "gfx/renderer.h"
#include "hud/hudmanager.h"
#include "gfx/texturemanager.h"
#include "gui/guimanager.h"

#include "core/timer.h"

// ---- Forward references ----

// ---- Interfaces ----
START_CHUNKED_INTERFACE( HudImageRenderDef, Mem::MC_MISC )
	PUBLISH_VAR_AS( m_obBaseOverlay.m_fTopLeftX, TopLeftX )
	PUBLISH_VAR_AS( m_obBaseOverlay.m_fTopLeftY, TopLeftY )
	PUBLISH_VAR_AS( m_obBaseOverlay.m_obColour,   Colour )
	PUBLISH_VAR_AS( m_fWidth, Width )
	PUBLISH_VAR_AS( m_fHeight, Height )
	PUBLISH_CONTAINER_AS( m_aobFrameList, FrameList)
	/*PUBLISH_VAR_AS( m_aobFrameList[0], Frame1 )
	PUBLISH_VAR_AS( m_aobFrameList[1], Frame2 )
	PUBLISH_VAR_AS( m_aobFrameList[2], Frame3 )
	PUBLISH_VAR_AS( m_aobFrameList[3], Frame4 )
	PUBLISH_VAR_AS( m_aobFrameList[4], Frame5 )
	PUBLISH_VAR_AS( m_aobFrameList[5], Frame6 )
	PUBLISH_VAR_AS( m_aobFrameList[6], Frame7 )
	PUBLISH_VAR_AS( m_aobFrameList[7], Frame8 )
	PUBLISH_VAR_AS( m_aobFrameList[8], Frame9 )
	PUBLISH_VAR_AS( m_aobFrameList[9], Frame10 )
	PUBLISH_VAR_AS( m_iTotalFrames, TotalFrames )*/

	PUBLISH_VAR_AS( m_bLooping, Looping )
	PUBLISH_VAR_AS( m_iFramesPerSecond, FramesPerSecond )
	PUBLISH_GLOBAL_ENUM_AS			(m_eBlendMode, BlendMode, EFFECT_BLENDMODE)
	PUBLISH_VAR_WITH_DEFAULT_AS( m_fBlendTime, 0.0f, BlendTime )

	DECLARE_POSTCONSTRUCT_CALLBACK (PostConstruct)
END_STD_INTERFACE

void ForceLinkFunctionHUDImage()
{
	ntPrintf("!ATTN! Calling ForceLinkFunctionHUDImage() !ATTN!\n");
}

//------------------------------------------------------------------------------------------
//!
//!	HudImageRenderDef::HudImageRenderDef
//!
//------------------------------------------------------------------------------------------
HudImageRenderDef::HudImageRenderDef()
:	m_fWidth( 0.25f )
,	m_fHeight( 0.25f )
,	m_bLooping ( true )
,	m_iFramesPerSecond ( 0 )
,	m_fBlendTime ( 0.0f )
,	m_eBlendMode ( EBM_LERP )
{
}

CHudUnit* HudImageRenderDef::CreateInstance( void ) const
{
	return NT_NEW_CHUNK (Mem::MC_MISC) HudImageRenderer( (HudImageRenderDef*)this );
}

//------------------------------------------------------------------------------------------
//!
//!	HudImageRenderDef::~HudImageRenderDef
//!
//------------------------------------------------------------------------------------------
HudImageRenderDef::~HudImageRenderDef()
{
}

void HudImageRenderDef::PostConstruct( void )
{
	// Cache images to avoid load stutter.
	for ( FrameIter obIt = m_aobFrameList.begin(); obIt != m_aobFrameList.end(); obIt++)
	{
		TextureManager::Get().LoadTexture_Neutral( obIt->GetString() );
	}

	/*for ( u_int obIt = 0; obIt < m_iTotalFrames; obIt++)
	{
		obImageSprite.SetTexture( m_aobFrameList[obIt] );
	}*/
}

//------------------------------------------------------------------------------------------
//!
//!	HudImageRenderer::HudImageRenderer
//!
//------------------------------------------------------------------------------------------
HudImageRenderer::HudImageRenderer(HudImageRenderDef*  pobRenderDef)
:	m_pobRenderDef( pobRenderDef )
,	m_iOverlayFilter ( 0 )
,	m_fOverlayParam ( 0.0f )
,	m_fOverlayParamInvLength ( 0.0f )
,	m_bPendingOverlay ( false )
,	m_fFrameTime ( 0.0f )
,	m_fCurrentTime ( 0.0f )
{
	m_eUnitState = STATE_INACTIVE;
}

//------------------------------------------------------------------------------------------
//!
//!	HudImageRenderer::~HudImageRenderer
//!
//------------------------------------------------------------------------------------------
HudImageRenderer::~HudImageRenderer()
{
	Reset();
}

void HudImageRenderer::Reset(float fTimeToReset)
{
	if (fTimeToReset>0.0f)
	{
		RemoveAnyIncrementalOverlays();
		ApplyOverlay(m_obSourceOverlay,
					 HudImageOverlay::OF_ALL,
					 OA_TRANS,
					 fTimeToReset);
	}
	else
	{
		Reset();
	}
}

void HudImageRenderer::Reset()
{
	ntAssert ( m_pobRenderDef );

	m_bPendingOverlay = false;

	// intialise overlay flag to 0 - positive when overlay applied
	m_fOverlayParam = 0.0f;
	m_obAppliedOverlay = 
		m_obSourceOverlay = 
			m_pobRenderDef->m_obBaseOverlay;


	RemoveAnyIncrementalOverlays();
}

//------------------------------------------------------------------------------------------
//!
//!	HudImageRenderer::Initialise
//!
//------------------------------------------------------------------------------------------
bool HudImageRenderer::Initialise( void )
{
	ntAssert ( m_pobRenderDef );

	Reset();

	// Point to first frame
	pobCurrentFrame = m_pobRenderDef->m_aobFrameList.begin();

	//m_iCurrentFrame = 0;

	// What is our frame time?
	m_fFrameTime = (m_pobRenderDef->m_iFramesPerSecond) ? (1.0f / (float)m_pobRenderDef->m_iFramesPerSecond) : 0.0f;

	return true;
}

//------------------------------------------------------------------------------------------
//!
//!	HudImageRenderer::ApplyOverlay
//!
//------------------------------------------------------------------------------------------
void HudImageRenderer::ApplyOverlay( const HudImageOverlaySlap& obApplyTo, 
									 int	iOverlayFilter,
									 OverlayApplication eApp, 
									 float fTransTime)
{

	m_iOverlayFilter = iOverlayFilter;

	// scee.sbashow: apply overlay to have effect immediately
	if (eApp==OA_IMM)
	{
		if (m_eUnitState!=STATE_ACTIVE &&
				 m_eUnitState!=STATE_INACTIVE)
		{
			m_obTargOverlay = obApplyTo;

			m_bPendingOverlay = true;
			// Will never do a transitional overlay whilst not in 
			//	the active or inactive state for immediate overlays
			m_fOverlayParamInvLength = 0.0f;
		}
		else
		{

			m_fOverlayParam = 0.0f;

			HudImageOverlaySlap::ApplyImmOverlay(m_obSourceOverlay,
													   obApplyTo,
													   m_iOverlayFilter);
			m_obAppliedOverlay = 
				m_obSourceOverlay;
		}
	}
	else
	{
		// scee.sbashow: apply overlay to have effect over time - 
		//			transitions smoothly to target overlay.
		ntAssert(fTransTime>0.0f);

		m_obTargOverlay = obApplyTo;
		
		if (m_eUnitState!=STATE_ACTIVE)
		{
			m_bPendingOverlay = true;
			// Will never do a transitional overlay whilst not in the active state.
			//	If this is the case, then store a bool and use m_fOverlayParamInvLength to store the 
			//	pending overlays time, until it becomes active - when this will commence.
			m_fOverlayParamInvLength = fTransTime;
		}
		else
		{
			m_fOverlayParam = fTransTime;
			m_fOverlayParamInvLength = 1.0f/fTransTime;

			// set the source to whatever obApplied was...
			//	if this ApplyOverlay was being done in the middle of a transition,
			//  then the source becomes whatever the transitional state, stored in m_obAppliedOverlay, was.
			m_obSourceOverlay = m_obAppliedOverlay;
		}
	}
}

//------------------------------------------------------------------------------------------
//!		Processes overlays, including incrementals
//!	HudImageRenderer::UpdateOverlays
//!
//------------------------------------------------------------------------------------------
void HudImageRenderer::ApplyIncrementalOverlay(const HudImageOverlayIncremental& obApply)
{
	HudImageOverlayIncremental* const pobIOverlay = 
		CHud::Get().IncrementalOverlayManager().Request();
	ntAssert(pobIOverlay);
	*pobIOverlay = obApply;
	m_obAppliedIncrementalOverlays.push_back(pobIOverlay);
}

//------------------------------------------------------------------------------------------
//!		See comment for HudImageRenderer::UpdateOverlays
//!	HudImageRenderer::RemoveAnyOverlay
//!
//------------------------------------------------------------------------------------------
void HudImageRenderer::RemoveAnyIncrementalOverlays( void )
{
	for (ntstd::List<HudImageOverlayIncremental*>::iterator obIt = 
								m_obAppliedIncrementalOverlays.begin(); 
						obIt != m_obAppliedIncrementalOverlays.end(); ++obIt )
	{
		CHud::Get().IncrementalOverlayManager().Release(*obIt);
	}

	m_obAppliedIncrementalOverlays.clear();
}

void HudImageRenderer::DampenIncrementals(float fDampenFactor)
{
	for (ntstd::List<HudImageOverlayIncremental*>::iterator obIt = 
								m_obAppliedIncrementalOverlays.begin(); 
						obIt != m_obAppliedIncrementalOverlays.end(); ++obIt )
	{
		HudImageOverlayIncremental*  pobIO = *obIt;
		pobIO->m_fApplicationWeight*=fDampenFactor;
	}
}

//------------------------------------------------------------------------------------------
//!		Processes overlays, including incrementals, removing the latter when ended (if they end) 
//!												If they do not end, they will be removed when 
//!												the hud image has been, or RemoveAnyOverlay has been called
//!	HudImageRenderer::UpdateOverlays
//!
//------------------------------------------------------------------------------------------
bool HudImageRenderer::UpdateOverlays( float fTimeChange )
{
	if (m_fOverlayParam>0.0f)
	{
        m_fOverlayParam-=fTimeChange;
		m_fOverlayParam = max(0.0f,m_fOverlayParam);

		HudImageOverlaySlap::InterpToOverlay(m_obAppliedOverlay,
									m_obSourceOverlay,
									m_obTargOverlay,
									1.0f - (m_fOverlayParam*m_fOverlayParamInvLength), 
									m_iOverlayFilter);

		if (m_fOverlayParam==0.0f)
		{
			m_obSourceOverlay = 
				m_obAppliedOverlay;
		}
	}

    for (ntstd::List<HudImageOverlayIncremental*>::iterator obIt = 
				m_obAppliedIncrementalOverlays.begin(); 
			obIt != m_obAppliedIncrementalOverlays.end(); )
	{
		HudImageOverlayIncremental* const pobIO = *obIt;
		// update the incremental overlay
		//	returns true when it has stopped updating.
		if (pobIO->Update(fTimeChange))
		{
			obIt = m_obAppliedIncrementalOverlays.erase(obIt);
			CHud::Get().IncrementalOverlayManager().Release(pobIO);
		}
		else
		{
			obIt++;
		}
	}

	return true;
}

//------------------------------------------------------------------------------------------
//!
//!	HudImageRenderer::UpdateImage
//!
//------------------------------------------------------------------------------------------
bool HudImageRenderer::UpdateImage( float fTimeChange )
{
	m_fCurrentTime += fTimeChange;

	// Do we need the next frame?
	if ( m_fFrameTime>0.0f && (m_fCurrentTime > m_fFrameTime) )
	{
		m_fCurrentTime -= m_fFrameTime;
		++pobCurrentFrame;
		//++m_iCurrentFrame;
		
		// Did we get to the last frame?
		if ( pobCurrentFrame == m_pobRenderDef->m_aobFrameList.end() )
		{
			pobCurrentFrame = m_pobRenderDef->m_aobFrameList.begin();
		}

		/*if ( m_iCurrentFrame == m_pobRenderDef->m_iTotalFrames )
		{
			m_iCurrentFrame = 0;
		}*/
	}

	return true;
}


void HudImageRenderer::UpdateEnter( float fTimestep )
{
	// update with system time so blends & anims are not dilated by NS
	fTimestep = CTimer::Get().GetSystemTimeChange();

	if (m_fOverlayParam > 0.0f)
	{
		m_fOverlayParam -= fTimestep;
	}

    m_obAppliedOverlay.m_obColour.W() = 
		m_obSourceOverlay.m_obColour.W() *
		 ( 1.0f - CMaths::SmoothStep(m_fOverlayParam/m_pobRenderDef->m_fBlendTime) );

	if (m_fOverlayParam <= 0.0f)
	{
		m_fOverlayParam = 0.0f;
		m_obAppliedOverlay.m_obColour.W() = 
			m_obSourceOverlay.m_obColour.W();

		// now's the time to setup any pending overlay
		if (m_bPendingOverlay)
		{
			m_bPendingOverlay = false;

			if (m_fOverlayParamInvLength>0.0f)
			{
				// by setting this just before the active state, we trigger a transitional overlay
				// we had stored the trans time in m_fOverlayParamInvLength, as a temprorary holder during the enter phase
				// before the image becomeing active.
				m_fOverlayParam = m_fOverlayParamInvLength;

				m_fOverlayParamInvLength = 1.0f/m_fOverlayParam;
			}
			else
			{
				// apply immediate overlay if an immediate request 
				//	was called during enter phase.
				HudImageOverlaySlap::ApplyImmOverlay(m_obSourceOverlay,
														   m_obTargOverlay,
														   m_iOverlayFilter);
			}
		}

		SetStateActive();
	}

    UpdateImage(fTimestep);

};

void HudImageRenderer::UpdateInactive( float fTimestep )
{
	UNUSED ( fTimestep );
};

void HudImageRenderer::UpdateActive( float fTimestep )
{
	// update with system time so blends & anims are not dilated by NS
	fTimestep = CTimer::Get().GetSystemTimeChange();

	UpdateImage(fTimestep);
	UpdateOverlays(fTimestep);
};

void HudImageRenderer::UpdateExit( float fTimestep )
{
	// update with system time so blends & anims are not dilated by NS
	fTimestep = CTimer::Get().GetSystemTimeChange();

	if (m_fOverlayParam > 0.0f)
	{
		m_fOverlayParam -= fTimestep;
	}

	m_obAppliedOverlay.m_obColour.W()
	 = m_obSourceOverlay.m_obColour.W()*
		CMaths::SmoothStep(m_fOverlayParam/m_pobRenderDef->m_fBlendTime);

	if (m_fOverlayParam <= 0.0f)
	{
		m_fOverlayParam = 0.0f;
		m_obAppliedOverlay.m_obColour.W() = 0.0f;
		
		SetStateInactive();
	}

	UpdateImage(fTimestep);
};

void HudImageRenderer::SetStateEnter( void )
{
	CHudUnit::SetStateEnter();
	m_fOverlayParam = 
		m_pobRenderDef->m_fBlendTime;
}

void HudImageRenderer::SetStateExit( void )
{
	CHudUnit::SetStateExit();

	// if it was in mid transition, set source to applied 
	//	for possible final flick out
	m_obSourceOverlay =
		m_obAppliedOverlay; 

	m_fOverlayParam = 
		m_pobRenderDef->m_fBlendTime;
}

//------------------------------------------------------------------------------------------
//!
//!	The final phase - renders with
//!	HudImageRenderer::RenderWithOverlay
//!
//------------------------------------------------------------------------------------------
void HudImageRenderer::RenderWithOverlay(const HudImageOverlaySlap& obSlapper )
{
	// Scale our heights and positions accordingly
	float fBBHeight = CGuiManager::Get().BBHeight();
	float fBBWidth = CGuiManager::Get().BBWidth();

	CPoint obPos( fBBWidth  * obSlapper.m_fTopLeftX, 
				  fBBHeight * obSlapper.m_fTopLeftY, 0.0f );

	m_obImageSprite.SetPosition( obPos );
	m_obImageSprite.SetHeight( 	fBBHeight * m_pobRenderDef->m_fHeight * obSlapper.m_fScale );
	m_obImageSprite.SetWidth( 	fBBWidth  * m_pobRenderDef->m_fWidth  * obSlapper.m_fScale  );
	m_obImageSprite.SetColour( obSlapper.m_obColour );
	m_obImageSprite.SetTexture( *pobCurrentFrame );
	m_obImageSprite.Render();
}

//------------------------------------------------------------------------------------------
//!
//!	HudImageRenderer::Render
//!
//------------------------------------------------------------------------------------------
bool HudImageRenderer::Render( void )
{
	ntAssert( m_pobRenderDef );

	if (m_obAppliedOverlay.m_bRendering)
	{
		RenderStateBlock::SetBlendMode(m_pobRenderDef->m_eBlendMode);

		if (HasIncrementalOverlays())
		{
			HudImageOverlaySlap obTempSlapper(m_obAppliedOverlay);

			for (ntstd::List<HudImageOverlayIncremental*>::iterator obIt = 
										m_obAppliedIncrementalOverlays.begin(); 
								obIt != m_obAppliedIncrementalOverlays.end(); ++obIt )
			{
				HudImageOverlayIncremental *const pobIO = *obIt;
				obTempSlapper = pobIO->ApplyTo(obTempSlapper);
			}

			RenderWithOverlay(obTempSlapper);
		}
		else
		{
			RenderWithOverlay(m_obAppliedOverlay);
		}
	}

	return true;
}

