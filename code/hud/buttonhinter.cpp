//------------------------------------------------------------------------------------------
//!
//!	\file buttonhinter.cpp
//!
//------------------------------------------------------------------------------------------

// Necessary includes
#include "hud/buttonhinter.h"
#include "objectdatabase/dataobject.h"
#include "gfx/renderer.h"
#include "gfx/graphicsdevice.h"
#include "hud/hudmanager.h"
#include "gui/guimanager.h"
#include "objectdatabase/dataobject.h"
#include "game/interactioncomponent.h"
#include "game/movementcontrollerinterface.h"
#include "game/inputcomponent.h"

// ---- Forward references ----

// ---- Interfaces ----
START_STD_INTERFACE( ButtonHinterRenderDef )
	PUBLISH_VAR_AS( m_fTopLeftX, TopLeftX )
	PUBLISH_VAR_AS( m_fTopLeftY, TopLeftY )
	PUBLISH_VAR_AS( m_fButtonSize, ButtonSize )
	PUBLISH_VAR_AS( m_fAlphaHigh, AlphaHigh )
	PUBLISH_VAR_AS( m_fAlphaLow, AlphaLow )
	PUBLISH_VAR_AS( m_fAlphaTime, AlphaTime )
	PUBLISH_VAR_AS( m_fBlendTime, BlendTime )
END_STD_INTERFACE

void ForceLinkFunctionButtonHinter()
{
	ntPrintf("!ATTN! Calling ForceLinkFunctionButtonHinter() !ATTN!\n");
}

//------------------------------------------------------------------------------------------
//!
//!	ButtonHinter::ButtonHinter
//!
//------------------------------------------------------------------------------------------
ButtonHinter::ButtonHinter()
{
	for (int i = 0; i < (int)AB_NUM; i++)
	{
		m_abHint[i] = false;
		m_abLastHint[i] = false;
	}
}

//------------------------------------------------------------------------------------------
//!
//!	ButtonHinter::~ButtonHinter
//!
//------------------------------------------------------------------------------------------
ButtonHinter::~ButtonHinter()
{
	m_aobEntList.clear();
}

//------------------------------------------------------------------------------------------
//!
//!	ButtonHinter::SetHintForButton
//!
//------------------------------------------------------------------------------------------
void ButtonHinter::SetHintForButton(VIRTUAL_BUTTON_TYPE eButton, bool bHint)
{
	// note false is set every frame, so that a true cannot be overwitten by a later hinting system
	if ( bHint )
		m_abHint[(int)eButton] = true;
}

//------------------------------------------------------------------------------------------
//!
//!	ButtonHinter::RegisterHintEntity
//!
//------------------------------------------------------------------------------------------
void ButtonHinter::RegisterHintEntity ( CEntity* pobEnt )
{
	if ( pobEnt )
	{
		m_aobEntList.push_back( pobEnt );
	}
}

//------------------------------------------------------------------------------------------
//!
//!	ButtonHinter::RemoveHintEntity
//!
//------------------------------------------------------------------------------------------
void ButtonHinter::RemoveHintEntity ( CEntity* pobEnt )
{
	if ( pobEnt )
	{
		ntstd::Vector<CEntity*>::iterator entIt =
	    ntstd::find(m_aobEntList.begin(), 
					m_aobEntList.end(), 
					pobEnt);

		if ( entIt != m_aobEntList.end() )
		{
			m_aobEntList.erase( entIt );
		}
	}
}

//------------------------------------------------------------------------------------------
//!
//!	ButtonHinter::ClearHintEntities
//!
//------------------------------------------------------------------------------------------
void ButtonHinter::ClearHintEntities ( void )
{
	m_aobEntList.clear();
}


//------------------------------------------------------------------------------------------
//!
//!	ButtonHinter::Update
//!
//------------------------------------------------------------------------------------------
void ButtonHinter::Update ( void )
{
	// Not entities to hint for - we can leave right now
	if ( m_aobEntList.empty() )
		return;

	CEntity*	pobPlayer = CEntityManager::Get().GetPlayer();

	if( pobPlayer->GetInputComponent() && pobPlayer->GetAwarenessComponent() )
	{
		bool bDirectionHeld = false;

		if ( ( pobPlayer->GetInputComponent() && pobPlayer->GetInputComponent()->IsDirectionHeld() ) )
		{
			bDirectionHeld = true;
		}

		// Get position and targeting direction of this entity
		// ----------------------------------------------------------------------
		CPoint obPosition(pobPlayer->GetHierarchy()->GetRootTransform()->GetWorldMatrix().GetTranslation());
		CDirection obDirection;

		// If the analog stick isn't held, then use their character matrix
		if( !pobPlayer->GetInputComponent()->IsDirectionHeld() )
		{
			obDirection=pobPlayer->GetHierarchy()->GetRootTransform()->GetWorldMatrix().GetZAxis();
		}
		else // Otherwise use pad direction
		{
			obDirection=pobPlayer->GetInputComponent()->GetInputDir();
		}

		const CDirection obPlaneNormalOfApproach(pobPlayer->GetHierarchy()->GetRootTransform()->GetWorldMatrix().GetYAxis());
		
		// For every hint entity
		for ( ntstd::Vector<CEntity*>::iterator obIt = m_aobEntList.begin();
				obIt != m_aobEntList.end(); obIt++ )
		{
			CUsePoint* pobBestUPointPossible;
			
			// Ensure entity is not paused
			if (!(*obIt)->IsPaused())
			{
				// Get score, passing in this position and angle
				float fScore = (*obIt)->GetInteractionComponent()->GetInteractionScore( obPosition, 
																						obDirection, 
																						obPlaneNormalOfApproach,
																						bDirectionHeld,
																						pobPlayer->ToPlayer()->IsHero() ? CUsePoint::ICT_Hero : CUsePoint::ICT_Archer,
																						pobBestUPointPossible );

				// We are able to use one of our hint entities
				if ( fScore > 0.0f )
				{
					SetHintForButton(AB_ACTION, true);
					break;
				}
			}
		}
	}
}

//------------------------------------------------------------------------------------------
//!
//!	ButtonHinterRenderDef::ButtonHinterRenderDef
//!
//------------------------------------------------------------------------------------------
ButtonHinterRenderDef::ButtonHinterRenderDef() 
:	m_fTopLeftX( 0.1f )
,	m_fTopLeftY( 0.1f )
,	m_fButtonSize( 0.15f )
,	m_fAlphaHigh ( 0.8f )
,	m_fAlphaLow	(0.6f )
,	m_fAlphaTime ( 0.25f )
,	m_fBlendTime ( 0.25f )
{
	for (int i = 0; i < (int)AB_NUM; i++)
	{
		this->m_aobButtonSprites[i] = "";
	}

	// FIX ME expose via xml
	m_aobButtonSprites[AB_ACTION] = "hud/control_cross_colour_alpha_nomip.dds";
	m_aobButtonSprites[AB_GRAB] = "hud/control_circle_colour_alpha_nomip.dds";
	m_aobButtonSprites[AB_ATTACK_FAST] = "hud/control_triangle_colour_alpha_nomip.dds";
	m_aobButtonSprites[AB_ATTACK_MEDIUM] = "hud/control_square_colour_alpha_nomip.dds";
}

CHudUnit* ButtonHinterRenderDef::CreateInstance( void ) const
{
	return NT_NEW_CHUNK (Mem::MC_MISC) ButtonHinterRenderer( (ButtonHinterRenderDef*)this );
}

//------------------------------------------------------------------------------------------
//!
//!	ButtonHinterRenderDef::~ButtonHinterRenderDef
//!
//------------------------------------------------------------------------------------------
ButtonHinterRenderDef::~ButtonHinterRenderDef()
{
}


//------------------------------------------------------------------------------------------
//!
//!	ButtonHinterRenderer::Initialise
//!
//------------------------------------------------------------------------------------------
bool ButtonHinterRenderer::Initialise( void )
{
	m_pobHinter = CHud::Get().GetButtonHinter();

	return true;
}

//------------------------------------------------------------------------------------------
//!
//!	ButtonHinterRenderer::Update
//!
//------------------------------------------------------------------------------------------
bool ButtonHinterRenderer::Update( float fTimeChange )
{
	ntAssert ( m_pobRenderDef );
	ntAssert ( m_pobHinter );

	// Scale our images
	float fBBHeight = CGuiManager::Get().BBHeight();
	float fBBWidth = CGuiManager::Get().BBWidth();

	// Update pulse time
	m_fPulseTime += fTimeChange;
	if ( m_fPulseTime > m_pobRenderDef->m_fAlphaTime)
	{
		m_fPulseTime -= m_pobRenderDef->m_fAlphaTime;
		m_bPulseIn = !m_bPulseIn;
	}

	// Update pulse alpha
	float fAlpha = m_pobRenderDef->m_fAlphaLow + (m_pobRenderDef->m_fAlphaHigh - m_pobRenderDef->m_fAlphaLow) * CMaths::SmoothStep(m_bPulseIn ? 1.0f - (m_fPulseTime / m_pobRenderDef->m_fAlphaTime) : (m_fPulseTime / m_pobRenderDef->m_fAlphaTime) );

	// Update hint blend
	if (m_fBlendTime > 0.0f)
	{
		m_fBlendTime -= fTimeChange;

		// Finished blend
		if (m_fBlendTime <= 0.0f)
		{
			m_fBlendTime = 0.0f;
			for (int i = 0; i < (int)AB_NUM; i++)
			{	
				if (m_pobHinter->m_abLastHint[i])
					m_pobHinter->m_abLastHint[i] = m_bBlendIn;
			}
		}
	}

	for (int i = 0; i < (int)AB_NUM; i++)
	{
		// If a hint status has changed kick off a blend
		if ((m_fBlendTime <= 0.0f) && m_pobHinter->m_abHint[i] != m_pobHinter->m_abLastHint[i])
		{
			m_fBlendTime = m_pobRenderDef->m_fBlendTime;

			// note the direction of the blend
			m_bBlendIn = m_pobHinter->m_abHint[i];
			m_pobHinter->m_abLastHint[i] = true;
		}

		// if we want a hint then set up the sprite
		if (m_pobHinter->m_abLastHint[i])
		{
			m_obSprite.SetPosition( CPoint(m_pobRenderDef->m_fTopLeftX*fBBWidth, m_pobRenderDef->m_fTopLeftY*fBBHeight, 0.0f) );
			m_obSprite.SetWidth( m_pobRenderDef->m_fButtonSize*fBBHeight );
			m_obSprite.SetHeight( m_pobRenderDef->m_fButtonSize*fBBHeight );
			float fNewAlpha = fAlpha * CMaths::SmoothStep( m_bBlendIn ? 1.0f - (m_fBlendTime / m_pobRenderDef->m_fBlendTime) : (m_fBlendTime / m_pobRenderDef->m_fBlendTime) );
			CVector obColour(1.0f,1.0f,1.0f, fNewAlpha);
	
			m_obSprite.SetColour(obColour.GetNTColor());
			m_obSprite.SetTexture( (m_pobRenderDef->m_aobButtonSprites[i]) );
			//m_obSprite.Render();
		}

		// reset so we can get valuse for next frame
		m_pobHinter->m_abHint[i] = false;
	}

	return true;
}

//------------------------------------------------------------------------------------------
//!
//!	ButtonHinterRenderer::Render
//!
//------------------------------------------------------------------------------------------
bool ButtonHinterRenderer::Render( void )
{
	for (int i = 0; i < (int)AB_NUM; i++)
	{
		if (m_pobHinter->m_abLastHint[i])
		{
			Renderer::Get().SetBlendMode( GFX_BLENDMODE_LERP );
			m_obSprite.Render();
			Renderer::Get().SetBlendMode( GFX_BLENDMODE_OVERWRITE );
		}
	}

	return true;
}
