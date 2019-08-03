//------------------------------------------------------------------------------------------
//!
//!	\file lifeclock.cpp
//!
//------------------------------------------------------------------------------------------

// Necessary includes
#include "hud/gamecounter.h"
#include "objectdatabase/dataobject.h"
#include "gfx/renderer.h"
#include "gfx/renderstates.h"
#include "gui/guimanager.h"
#include "game/nsmanager.h"
#include "movies/movieinstance.h"
#include "movies/movieplayer.h"
#include "game/combatstyle.h"
#include "hud/messagedata.h"
#include "gfx/texturemanager.h"

#ifdef PLATFORM_PS3
#	include "army/armymanager.h"
#endif

// ---- Forward references ----


// ---- Interfaces ----
START_STD_INTERFACE( GameCounterRenderDef )
	DEFINE_INTERFACE_INHERITANCE(CBlendableHudUnitDef)
	COPY_INTERFACE_FROM(CBlendableHudUnitDef)

	PUBLISH_VAR_AS		(m_fTopLeftX, TopLeftX)
	PUBLISH_VAR_AS		(m_fTopLeftY, TopLeftY)
	PUBLISH_VAR_AS		(m_fOverallScale, OverallScale)

	PUBLISH_VAR_AS		(m_fBackgroundWidth, BackgroundWidth)
	PUBLISH_VAR_AS		(m_fBackgroundHeight, BackgroundHeight)

	PUBLISH_VAR_AS		(m_obDigitsPos, DigitsPos)
	
	PUBLISH_VAR_AS		(m_fDigitWidth, DigitWidth)
	PUBLISH_VAR_AS		(m_fDigitHeight, DigitHeight)
	PUBLISH_VAR_AS		(m_fDigitSpacing, DigitSpacing)
	PUBLISH_VAR_AS		(m_obDigitColour, DigitColour)

	PUBLISH_VAR_WITH_DEFAULT_AS (m_bReversed, false, Reversed)

	PUBLISH_VAR_WITH_DEFAULT_AS (m_obGameData, "KILLS", GameData)
	PUBLISH_VAR_WITH_DEFAULT_AS (m_obInitialValue, "", InitialValue)

	PUBLISH_VAR_WITH_DEFAULT_AS (m_obLabelTextID, "HUD_INFO_BODYCOUNT", LabelTextID)
	
	
	PUBLISH_VAR_WITH_DEFAULT_AS (m_fTextPosX, 0.0f, TextPosX)
	PUBLISH_VAR_WITH_DEFAULT_AS (m_fTextPosY, 0.0f, TextPosY)
	PUBLISH_VAR_WITH_DEFAULT_AS (m_obFont, "Body", Font)

	PUBLISH_VAR_WITH_DEFAULT_AS (m_obBackgroundImage, "hud/timer_base_colour_alpha_npow2_nomip.dds", BackgroundImage )
	PUBLISH_VAR_WITH_DEFAULT_AS (m_aobDigitImage[0], "hud/00_colour_alpha_nomip.dds", Digit0Image)
	PUBLISH_VAR_WITH_DEFAULT_AS (m_aobDigitImage[1], "hud/01_colour_alpha_nomip.dds", Digit1Image)
	PUBLISH_VAR_WITH_DEFAULT_AS (m_aobDigitImage[2], "hud/02_colour_alpha_nomip.dds", Digit2Image)
	PUBLISH_VAR_WITH_DEFAULT_AS (m_aobDigitImage[3], "hud/03_colour_alpha_nomip.dds", Digit3Image)
	PUBLISH_VAR_WITH_DEFAULT_AS (m_aobDigitImage[4], "hud/04_colour_alpha_nomip.dds", Digit4Image)
	PUBLISH_VAR_WITH_DEFAULT_AS (m_aobDigitImage[5], "hud/05_colour_alpha_nomip.dds", Digit5Image)
	PUBLISH_VAR_WITH_DEFAULT_AS (m_aobDigitImage[6], "hud/06_colour_alpha_nomip.dds", Digit6Image)
	PUBLISH_VAR_WITH_DEFAULT_AS (m_aobDigitImage[7], "hud/07_colour_alpha_nomip.dds", Digit7Image)
	PUBLISH_VAR_WITH_DEFAULT_AS (m_aobDigitImage[8], "hud/08_colour_alpha_nomip.dds", Digit8Image)
	PUBLISH_VAR_WITH_DEFAULT_AS (m_aobDigitImage[9], "hud/09_colour_alpha_nomip.dds", Digit9Image)

	PUBLISH_VAR_WITH_DEFAULT_AS (m_obBackgroundGlowImage, "", BackgroundGlowImage )
	PUBLISH_VAR_WITH_DEFAULT_AS (m_fGlowWidth,		0.0f,	GlowWidth)
	PUBLISH_VAR_WITH_DEFAULT_AS (m_fGlowHeight,		0.0f,	GlowHeight)
	PUBLISH_VAR_WITH_DEFAULT_AS (m_fGlowXOffset,	0.0f,	GlowXOffset)
	PUBLISH_VAR_WITH_DEFAULT_AS (m_fGlowYOffset,	0.0f,	GlowYOffset)
	PUBLISH_VAR_WITH_DEFAULT_AS (m_fGlowAlphaHigh,	0.0f,	GlowAlphaHigh)
	PUBLISH_VAR_WITH_DEFAULT_AS (m_fGlowAlphaLow,	0.0f,	GlowAlphaLow)

	PUBLISH_GLOBAL_ENUM_AS	(m_eBlendmode, Blendmode, EFFECT_BLENDMODE)
	PUBLISH_VAR_WITH_DEFAULT_AS (m_fCatchUpTime, 2.0f, CatchUpTime)

	DECLARE_POSTCONSTRUCT_CALLBACK ( PostConstruct )
END_STD_INTERFACE


void ForceLinkFunctionGameCounterRender()
{
	ntPrintf("!ATTN! Calling ForceLinkFunctionGameCounterRender() !ATTN!\n");
}

///////////////////////////////////////////
//
// BodyCount::Update()
// Move elsewhere if a more logical place becomes apparent
//
///////////////////////////////////////////
void BodyCount::Update( void )
{
	u_int iBodyCount = 0;

#ifdef PLATFORM_PS3
	if ( ArmyManager::Exists() )
	{
		iBodyCount += ArmyManager::Get().GetBodyCount();
	}
#endif

	if (StyleManager::Exists() )
	{
		iBodyCount += (u_int)StyleManager::Get().GetStats().m_iKills;
	}

	if (m_pobCallbackEnt && (int)iBodyCount >= m_iTarget)
	{
		Message msgBodyCount(msg_bodycount_complete);
		m_pobCallbackEnt->GetMessageHandler()->QueueMessage( msgBodyCount );	

		m_pobCallbackEnt = 0;
		m_iTarget = 0;
	}

	MessageDataManager* pobMsgDataMan = CHud::Get().GetMessageDataManager();
	ntAssert( pobMsgDataMan );
	
	pobMsgDataMan->SetValue( CHashedString("KILLS"), (int)iBodyCount );
}

///////////////////////////////////////////
//
// BodyCount::BodycountCallback()
// Callback to pobCallbackEnt when Target met
//
///////////////////////////////////////////
void BodyCount::BodycountCallback ( int iTarget, CEntity* pobCallbackEnt )
{
	m_iTarget = iTarget;
	m_pobCallbackEnt = pobCallbackEnt;
}

///////////////////////////////////////////
//
// GameCounterRenderDef
//
///////////////////////////////////////////
GameCounterRenderDef::GameCounterRenderDef()
{
}

CHudUnit* GameCounterRenderDef::CreateInstance( void ) const
{
	return NT_NEW_CHUNK (Mem::MC_MISC) GameCounterRenderer( (GameCounterRenderDef*)this );
}

//------------------------------------------------------------------------------------------
//!
//!	GameCounterRenderDef::PostConstruct
//! PostConstruct
//!
//------------------------------------------------------------------------------------------
void GameCounterRenderDef::PostConstruct( void )
{
	if ( !ntStr::IsNull ( m_obBackgroundImage ) )
		TextureManager::Get().LoadTexture_Neutral( m_obBackgroundImage.GetString() );

	if ( !ntStr::IsNull ( m_obBackgroundGlowImage ) )
		TextureManager::Get().LoadTexture_Neutral( m_obBackgroundGlowImage.GetString() );
	
	for(int iCount=0; iCount<10; iCount++)
	{
		TextureManager::Get().LoadTexture_Neutral( m_aobDigitImage[iCount].GetString() );
	}
}

///////////////////////////////////////////
//
// GameCounterRenderer
//
///////////////////////////////////////////
GameCounterRenderer::~GameCounterRenderer()
{
	if (m_pobTextRenderer)
		NT_DELETE_CHUNK ( Mem::MC_MISC, m_pobTextRenderer);
}

bool GameCounterRenderer::Initialise( void )
{
	ntAssert ( m_pobRenderDef );

	CPoint obPos ( m_pobRenderDef->m_fTopLeftX, m_pobRenderDef->m_fTopLeftY, 0.0f);

	if ( !ntStr::IsNull ( m_pobRenderDef->m_obBackgroundImage ) )
		m_obBackground.SetTexture( m_pobRenderDef->m_obBackgroundImage );

	if ( !ntStr::IsNull ( m_pobRenderDef->m_obBackgroundGlowImage ) )
		m_obGlow.SetTexture( m_pobRenderDef->m_obBackgroundGlowImage );
	
	for(int iCount=0; iCount<10; iCount++)
	{
		m_aobDigit[iCount].SetTexture( m_pobRenderDef->m_aobDigitImage[iCount] );
	}

	if ( !ntStr::IsNull( m_pobRenderDef->m_obLabelTextID ) )
	{
		m_obTextRenderDef.m_obStringTextID = m_pobRenderDef->m_obLabelTextID;
		m_obTextRenderDef.m_obFontName = m_pobRenderDef->m_obFont;
		m_obTextRenderDef.m_fTopLeftX = m_pobRenderDef->m_fTopLeftX + m_pobRenderDef->m_fTextPosX;
		m_obTextRenderDef.m_fTopLeftY = m_pobRenderDef->m_fTopLeftY + m_pobRenderDef->m_fTextPosY;

		m_pobTextRenderer = (HudTextRenderer*)m_obTextRenderDef.CreateInstance();
		m_pobTextRenderer->Initialise();
	}

	return true;
}

bool GameCounterRenderer::Update( float fTimeStep )
{
	ntAssert ( m_pobRenderDef );

	// Update base unit
	CBlendableHudUnit::Update( fTimeStep );

	
	if ( IsDeactive() )
		return !m_bRemoveOnExit;

	// Text 
	if ( m_pobTextRenderer)
	{
		// Position - for testing only
		m_pobTextRenderer->SetPosition( m_pobRenderDef->m_fTopLeftX + m_pobRenderDef->m_fTextPosX, 
										m_pobRenderDef->m_fTopLeftY + m_pobRenderDef->m_fTextPosY );
		
		m_pobTextRenderer->Update( fTimeStep );
	}

	// See what the count is this frame
	int iGameCounterThisUpdate = 0;

	MessageDataManager* pobMsgDataMan = CHud::Get().GetMessageDataManager();
	ntAssert( pobMsgDataMan );
	
	pobMsgDataMan->GetValue( m_pobRenderDef->m_obGameData, iGameCounterThisUpdate );

	// Are we doing a catch up?
	if ( m_fCatchUpTime > 0.0f )
	{
		m_fCatchUpTime -= fTimeStep;
	
		if ( m_fCatchUpTime < 0.0f )
		{
			m_fCatchUpTime = 0.0f;
			m_iGameCounter = m_iLastGameCounter = m_iAimGameCounter; 
		}
		else
		{
			float fCountDelta = (float)(m_iAimGameCounter - m_iLastGameCounter);
			float fCatchUpTemp = (m_fCatchUpTime/m_pobRenderDef->m_fCatchUpTime);
			m_iGameCounter = m_iLastGameCounter + 
				(int)ceil( fCountDelta * (1.0f - fCatchUpTemp * fCatchUpTemp ) );
		}
	}
	// Has the count changed?
	else if ( iGameCounterThisUpdate != m_iGameCounter )
	{
		// Start any change effects
		m_fCatchUpTime = m_pobRenderDef->m_fCatchUpTime;
	
		m_iAimGameCounter = iGameCounterThisUpdate;
	}

	return true;
}

bool GameCounterRenderer::Render( void )
{
	ntAssert ( m_pobRenderDef );

	// If deactive - 'cos we should render Enter and Exit states too
	if ( IsDeactive() )
		return true;
	
	CPoint obPos;
	
	float fScreenHeight = CGuiManager::Get().BBHeight();
	float fScreenWidth = CGuiManager::Get().BBWidth();

	//float fResolutionScale = fScreenWidth/1280.0f;

	m_fScale = m_pobRenderDef->m_fOverallScale;

	// BEGIN RENDERING
	{
		RenderStateBlock::SetBlendMode( m_pobRenderDef->m_eBlendmode );

		// Background
		if ( !ntStr::IsNull ( m_pobRenderDef->m_obBackgroundImage ) )
		{
			m_obBackground.SetWidth  ( m_pobRenderDef->m_fBackgroundWidth  * m_fScale * fScreenWidth);
			m_obBackground.SetHeight ( m_pobRenderDef->m_fBackgroundHeight * m_fScale * fScreenHeight);
			m_obBackground.SetPosition ( CPoint ( m_pobRenderDef->m_fTopLeftX * fScreenWidth, m_pobRenderDef->m_fTopLeftY * fScreenHeight, 0.0f ) );
			m_obBackground.SetColour( CVector( 1.0f, 1.0f, 1.0f, 0.8f ) );
			m_obBackground.Render();
		}

		// Glow
		if ( !ntStr::IsNull ( m_pobRenderDef->m_obBackgroundGlowImage ) && (m_fCatchUpTime > 0.0f) )
		{

			
			m_obGlow.SetTexture( m_pobRenderDef->m_obBackgroundGlowImage );

			m_fCurrGlowAlpha = m_pobRenderDef->m_fGlowAlphaLow + CMaths::SmoothStep(m_fCatchUpTime / m_pobRenderDef->m_fCatchUpTime )
									* ( m_pobRenderDef->m_fGlowAlphaHigh - m_pobRenderDef->m_fGlowAlphaLow );

			m_obGlow.SetWidth  ( m_pobRenderDef->m_fGlowWidth  * m_fScale * fScreenWidth);
			m_obGlow.SetHeight ( m_pobRenderDef->m_fGlowHeight * m_fScale * fScreenHeight);
			m_obGlow.SetPosition ( CPoint ( (m_pobRenderDef->m_fTopLeftX + m_pobRenderDef->m_fGlowXOffset) * fScreenWidth, 
												(m_pobRenderDef->m_fTopLeftY + m_pobRenderDef->m_fGlowYOffset) * fScreenHeight, 0.0f ) );
			m_obGlow.SetColour( CVector( 1.0f, 1.0f, 1.0f, m_fCurrGlowAlpha ) );
			m_obGlow.Render();
		}

		// Digits
		CVector obColour;
		obColour = m_pobRenderDef->m_obDigitColour;
		obColour.W() *= m_fOverallAlpha;

		CPoint obPos;
		obPos.X() = ( m_pobRenderDef->m_obDigitsPos.X() * m_fScale + m_pobRenderDef->m_fTopLeftX ) * fScreenWidth;
		obPos.Y() = ( m_pobRenderDef->m_obDigitsPos.Y() * m_fScale + m_pobRenderDef->m_fTopLeftY ) * fScreenHeight;
		obPos.Z() = m_pobRenderDef->m_obDigitsPos.Z() * m_fScale;

		int iDigits = 0;

		if ( m_pobRenderDef->m_bReversed )
		{
			MessageDataManager* pobMsgDataMan = CHud::Get().GetMessageDataManager();
			ntAssert( pobMsgDataMan );
			
			int iInitalValue = 0;
			pobMsgDataMan->GetValue( m_pobRenderDef->m_obInitialValue, iInitalValue );

			iDigits = iInitalValue - m_iGameCounter;
			if ( iDigits < 0 ) 
				iDigits = 0;
		}
		else
		{
			iDigits = m_iGameCounter;
		}

		RenderDigits( iDigits, obPos, false);
	}

	// Text
	if ( m_pobTextRenderer)
	{
		m_pobTextRenderer->Render();
	}


	// FINISH RENDERING
	Renderer::Get().SetBlendMode( GFX_BLENDMODE_OVERWRITE );

	return true;
}

//------------------------------------------------------------------------------------------
//!
//!	GameCounterRenderer::RenderDigits
//!
//------------------------------------------------------------------------------------------
void GameCounterRenderer::RenderDigits( int iNumber, const CPoint& obPosition, bool bLeadingZero )
{
	float fScreenWidth = CGuiManager::Get().BBWidth();
	float fResolutionScale = fScreenWidth/1280.0f;

	CPoint obPos = obPosition;

	CVector obNormalColour;
	obNormalColour = m_pobRenderDef->m_obDigitColour;
	obNormalColour.W() *= m_fOverallAlpha;

	int iUnits = iNumber % 10;

	int iTens = iNumber/10;
	iTens = iTens % 10;

	int iHundreds = iNumber/100;
	iHundreds = iHundreds % 10;

	int iThousands = iNumber/1000;
	iThousands = iThousands % 10;
	
	ntAssert ( iUnits < 10 && iUnits >= 0 );
	ntAssert ( iTens < 10 && iTens >= 0 );
	ntAssert ( iHundreds < 10 && iHundreds >= 0 );
	ntAssert ( iThousands < 10 && iThousands >= 0 );

	
	if (iNumber < 1000 && !bLeadingZero )
		iThousands = -1;

	if (iThousands != -1)
	{
		m_aobDigit[iThousands].SetWidth  ( m_pobRenderDef->m_fDigitWidth  * m_fScale * fResolutionScale);
		m_aobDigit[iThousands].SetHeight ( m_pobRenderDef->m_fDigitHeight * m_fScale * fResolutionScale);
		m_aobDigit[iThousands].SetPosition(obPos);
		m_aobDigit[iThousands].SetColour(obNormalColour.GetNTColor());
		m_aobDigit[iThousands].Render();
	}

	obPos.X() += m_pobRenderDef->m_fDigitSpacing*m_fScale*fResolutionScale;
	
	if (iNumber < 100 && !bLeadingZero )
		iHundreds = -1;

	if (iHundreds != -1)
	{
		m_aobDigit[iHundreds].SetWidth  ( m_pobRenderDef->m_fDigitWidth  * m_fScale * fResolutionScale);
		m_aobDigit[iHundreds].SetHeight ( m_pobRenderDef->m_fDigitHeight * m_fScale * fResolutionScale);
		m_aobDigit[iHundreds].SetPosition(obPos);
		m_aobDigit[iHundreds].SetColour(obNormalColour.GetNTColor());
		m_aobDigit[iHundreds].Render();
	}

	obPos.X() += m_pobRenderDef->m_fDigitSpacing*m_fScale*fResolutionScale;

	if (iNumber < 10 && !bLeadingZero )
		iTens = -1;

	if (iTens != -1)
	{
		m_aobDigit[iTens].SetWidth  ( m_pobRenderDef->m_fDigitWidth  * m_fScale * fResolutionScale);
		m_aobDigit[iTens].SetHeight ( m_pobRenderDef->m_fDigitHeight * m_fScale * fResolutionScale);
		m_aobDigit[iTens].SetPosition(obPos);
		m_aobDigit[iTens].SetColour(obNormalColour.GetNTColor());
		m_aobDigit[iTens].Render();
	}

	obPos.X() += m_pobRenderDef->m_fDigitSpacing*m_fScale*fResolutionScale;

	if (iUnits!=-1)
	{
		m_aobDigit[iUnits].SetWidth ( m_pobRenderDef->m_fDigitWidth  * m_fScale  * fResolutionScale);
		m_aobDigit[iUnits].SetHeight( m_pobRenderDef->m_fDigitHeight * m_fScale * fResolutionScale);
		m_aobDigit[iUnits].SetPosition(obPos);
		m_aobDigit[iUnits].SetColour(obNormalColour.GetNTColor());
		m_aobDigit[iUnits].Render();
	}
}

/***************************************************************************************************
*
*	FUNCTION		GameCounterRenderer::UpdateActive
*
*	DESCRIPTION		
*
***************************************************************************************************/
void GameCounterRenderer::UpdateActive( float fTimestep )
{
	CBlendableHudUnit::UpdateActive ( fTimestep );

	if ( NSManager::Get().IsNinjaSequencePlaying() || MoviePlayer::Get().IsActive() )
	{
		BeginExit();

		if ( m_pobTextRenderer )
		{
			m_pobTextRenderer->BeginExit();
		}
	}
}

/***************************************************************************************************
*
*	FUNCTION		GameCounterRenderer::UpdateInactive
*
*	DESCRIPTION		
*
***************************************************************************************************/
void GameCounterRenderer::UpdateInactive( float fTimestep )
{
	CBlendableHudUnit::UpdateInactive ( fTimestep );
	
	if ( ! NSManager::Get().IsNinjaSequencePlaying() && !MoviePlayer::Get().IsActive() )
	{
		BeginEnter();	
		
		if ( m_pobTextRenderer )
		{
			m_pobTextRenderer->BeginEnter();
		}
	}
}
