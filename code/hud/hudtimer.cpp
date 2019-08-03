//------------------------------------------------------------------------------------------
//!
//!	\file lifeclock.cpp
//!
//------------------------------------------------------------------------------------------

// Necessary includes
#include "hud/hudtimer.h"
#include "objectdatabase/dataobject.h"
#include "gfx/renderer.h"
#include "gfx/renderstates.h"
#include "gui/guimanager.h"
#include "gfx/texturemanager.h"

// ---- Forward references ----



// ---- Interfaces ----
START_STD_INTERFACE( TimerRenderDef )
	DEFINE_INTERFACE_INHERITANCE(CBlendableHudUnitDef)
	COPY_INTERFACE_FROM(CBlendableHudUnitDef)

	PUBLISH_VAR_AS		(m_fTopLeftX, TopLeftX)
	PUBLISH_VAR_AS		(m_fTopLeftY, TopLeftY)
	PUBLISH_VAR_AS		(m_fOverallScale, OverallScale)

	PUBLISH_VAR_AS		(m_fFadeSpeed, FadeSpeed)

	PUBLISH_GLOBAL_ENUM_AS		(m_eMaxUnits, MaxTimerUnits, TIMER_UNITS)

	PUBLISH_VAR_AS		(m_obTimeColour, TimeColour)
	PUBLISH_VAR_AS		(m_obTimeGlowColour, TimeGlowColour)
	PUBLISH_GLOBAL_ENUM_AS		(m_eTimeBlendmode, TimeBlendmode, EFFECT_BLENDMODE)
	PUBLISH_GLOBAL_ENUM_AS		(m_eTimeGlowBlendmode, TimeGlowBlendmode, EFFECT_BLENDMODE)

	PUBLISH_VAR_AS		(m_obDaysPos, DaysPos)
	PUBLISH_VAR_AS		(m_fDaysWidth, DaysWidth)
	PUBLISH_VAR_AS		(m_fDaysHeight, DaysHeight)
	PUBLISH_GLOBAL_ENUM_AS		(m_eDaysZeros, DaysZeros, TIMER_LEADING_ZERO)

	PUBLISH_VAR_AS		(m_obHrsPos, HrsPos)
	PUBLISH_VAR_AS		(m_fHrsWidth, HrsWidth)
	PUBLISH_VAR_AS		(m_fHrsHeight, HrsHeight)
	PUBLISH_GLOBAL_ENUM_AS		(m_eHrsZeros, HrsZeros, TIMER_LEADING_ZERO)

	PUBLISH_VAR_AS		(m_obMinsPos, MinsPos)
	PUBLISH_VAR_AS		(m_fMinsWidth, MinsWidth)
	PUBLISH_VAR_AS		(m_fMinsHeight, MinsHeight)
	PUBLISH_GLOBAL_ENUM_AS		(m_eMinsZeros, MinsZeros, TIMER_LEADING_ZERO)

	PUBLISH_VAR_AS		(m_obSecsPos, SecsPos)
	PUBLISH_VAR_AS		(m_fSecsWidth, SecsWidth)
	PUBLISH_VAR_AS		(m_fSecsHeight, SecsHeight)
	PUBLISH_GLOBAL_ENUM_AS		(m_eSecsZeros, SecsZeros, TIMER_LEADING_ZERO)

	PUBLISH_VAR_AS		(m_obDaysDigitsPos, DaysDigitsPos)
	PUBLISH_VAR_AS		(m_obHrsDigitsPos, HrsDigitsPos)
	PUBLISH_VAR_AS		(m_obMinsDigitsPos, MinsDigitsPos)
	PUBLISH_VAR_AS		(m_obSecsDigitsPos, SecsDigitsPos)

	PUBLISH_VAR_AS		(m_fDigitWidth, DigitWidth)
	PUBLISH_VAR_AS		(m_fDigitHeight, DigitHeight)
	PUBLISH_VAR_AS		(m_fDigitSpacing, DigitSpacing)
	PUBLISH_VAR_AS		(m_obDigitColour, DigitColour)

	PUBLISH_VAR_WITH_DEFAULT_AS (m_obDaysImage, "hud/DAYS.dds", DaysImage)
	PUBLISH_VAR_WITH_DEFAULT_AS (m_obHoursImage, "hud/HRS.dds", HoursImage)
	PUBLISH_VAR_WITH_DEFAULT_AS (m_obMinsImage, "hud/MINS.dds", MinsImage)
	PUBLISH_VAR_WITH_DEFAULT_AS (m_obSecsImage, "hud/MINS.dds", SecsImage)
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

	PUBLISH_VAR_WITH_DEFAULT_AS (m_fTimeToCatchUp, 2.0f, TimeToCatchUp)

	DECLARE_POSTCONSTRUCT_CALLBACK ( PostConstruct )
END_STD_INTERFACE


void ForceLinkFunctionHUDTimer()
{
	ntPrintf("!ATTN! Calling ForceLinkFunctionHUDTimer() !ATTN!\n");
}

///////////////////////////////////////////
//
// TimerRenderDef
//
///////////////////////////////////////////
TimerRenderDef::TimerRenderDef()
{
	m_eMaxUnits = TIMER_HOURS;
	m_eDaysZeros = TIMER_ALWAYS;
	m_eHrsZeros = TIMER_ALWAYS;
	m_eMinsZeros = TIMER_ALWAYS;
	m_eSecsZeros = TIMER_ALWAYS;

	m_fTopLeftX = 0.5f;
	m_fTopLeftY = 0.5f;

	m_fOverallScale = 1.0f;
}

CHudUnit* TimerRenderDef::CreateInstance( void ) const
{
	return NT_NEW_CHUNK (Mem::MC_MISC) TimerRenderer( (TimerRenderDef*)this );
}


//------------------------------------------------------------------------------------------
//!
//!	TimerRenderDef::PostConstruct
//! PostConstruct
//!
//------------------------------------------------------------------------------------------
void TimerRenderDef::PostConstruct( void )
{
	if ( !ntStr::IsNull ( m_obDaysImage ) )
		TextureManager::Get().LoadTexture_Neutral( m_obDaysImage.GetString() );

	if ( !ntStr::IsNull ( m_obHoursImage ) )
		TextureManager::Get().LoadTexture_Neutral( m_obHoursImage.GetString() );

	if ( !ntStr::IsNull ( m_obMinsImage ) )
		TextureManager::Get().LoadTexture_Neutral( m_obMinsImage.GetString() );

	if ( !ntStr::IsNull ( m_obSecsImage ) )
		TextureManager::Get().LoadTexture_Neutral( m_obSecsImage.GetString() );
	
	for(int iCount=0; iCount<10; iCount++)
	{
		TextureManager::Get().LoadTexture_Neutral( m_aobDigitImage[iCount].GetString() );
	}
}

///////////////////////////////////////////
//
// LifeclockRender
//
///////////////////////////////////////////
bool TimerRenderer::Initialise( void )
{
	ntAssert ( m_pobRenderDef );

	CPoint obPos ( m_pobRenderDef->m_fTopLeftX, m_pobRenderDef->m_fTopLeftY, 0.0f);
	SetPosition ( obPos );

	m_obDays.SetTexture( m_pobRenderDef->m_obDaysImage );
	m_obHours.SetTexture( m_pobRenderDef->m_obHoursImage );
	m_obMins.SetTexture( m_pobRenderDef->m_obMinsImage );
	m_obSecs.SetTexture( m_pobRenderDef->m_obSecsImage );
	
	for(int iCount=0; iCount<10; iCount++)
	{
		m_aobDigit[iCount].SetTexture( m_pobRenderDef->m_aobDigitImage[iCount] );
	}

	return true;
}

#define TIMER_TOLERANCE (1.75f)
bool TimerRenderer::TimerUpdate( float fTimeStep, float fScalar )
{
	ntAssert ( m_pobRenderDef );

	// Update base unit
	CBlendableHudUnit::Update( fTimeStep );

	// The timer counts backwards
	m_dLastTime -= fTimeStep * fScalar;
	
	// Is a catch up in progress?
	if ( m_fCatchupTime > 0.0f )
	{
		m_fCatchupTime -= fTimeStep;

		if ( m_fCatchupTime < 0.0f)
		{
			m_fCatchupTime = 0.0f;
			m_dLastTime = m_dAimTime;
			m_dTimeDelta = 0.0f;
		}
		else
		{
			m_dLastTime += ( m_dTimeDelta / (double)m_pobRenderDef->m_fTimeToCatchUp ) * fTimeStep;
		}
	}
	else
	{
		if ( ( m_dLastTime < (m_dAimTime - TIMER_TOLERANCE) ) || ( m_dLastTime > (m_dAimTime + TIMER_TOLERANCE) ) )
		{
			m_dTimeDelta = m_dAimTime - m_dLastTime;
			m_fCatchupTime = m_pobRenderDef->m_fTimeToCatchUp;
		}
		else
		{
			m_dLastTime = m_dAimTime;
		}
	}

	return true;
}

void TimerRenderer::SetTime( double dTime, bool bForce ) 
{ 
	m_dAimTime = dTime; 
	
	if ( bForce )
	{
		m_dLastTime = dTime; 
	}	
};


bool TimerRenderer::Render( void )
{
	ntAssert ( m_pobRenderDef );

	// If deactive - 'cos we should render Enter and Exit states too
	if ( IsDeactive() )
		return true;
	
	CPoint obPos;
	
	float fScreenHeight = CGuiManager::Get().BBHeight();
	float fScreenWidth = CGuiManager::Get().BBWidth();

	float fResolutionScale = fScreenWidth/1280.0f;

	m_fScale = m_pobRenderDef->m_fOverallScale;

	// BEGIN RENDERING
	
	float fMissingXOffset = 0.0f;

	switch ( m_pobRenderDef->m_eMaxUnits )
	{
	case TIMER_SECS:
		fMissingXOffset = m_pobRenderDef->m_obSecsDigitsPos.X();
		break;

	case TIMER_MINS:
		fMissingXOffset = m_pobRenderDef->m_obMinsDigitsPos.X();
		break;

	case TIMER_HOURS:
		fMissingXOffset = m_pobRenderDef->m_obHrsDigitsPos.X();
		break;

	case TIMER_DAYS:
	default:
		fMissingXOffset = 0.0f;

		break;
	}
	fMissingXOffset *= m_fScale;

	int iDays, iHours, iMinutes;
	float fSeconds;
	bool bLeadingZero = false;
	
	GetClockTime(m_dLastTime, iDays, iHours, iMinutes, fSeconds);

	// Timer
	{
		RenderStateBlock::SetBlendMode( m_pobRenderDef->m_eTimeBlendmode );

		CVector obColour;
		obColour = m_pobRenderDef->m_obTimeColour;
		obColour.W() *= m_fOverallAlpha;

		if ( m_pobRenderDef->m_eMaxUnits >= TIMER_DAYS )
		{
			//Days
			obPos.X() = (m_pobRenderDef->m_obDaysPos.X()*m_fScale+m_obPosition.X()-fMissingXOffset)*fScreenWidth;
			obPos.Y() = (m_pobRenderDef->m_obDaysPos.Y()*m_fScale+m_obPosition.Y())*fScreenHeight;
			obPos.Z() = m_pobRenderDef->m_obDaysPos.Z()*m_fScale;

			m_obDays.SetPosition(obPos);
			m_obDays.SetWidth(m_pobRenderDef->m_fDaysWidth*m_fScale * fResolutionScale);
			m_obDays.SetHeight(m_pobRenderDef->m_fDaysHeight*m_fScale * fResolutionScale);
			m_obDays.SetColour(obColour.GetNTColor());
			m_obDays.Render();

			CPoint obPos;
			obPos.X() = (m_pobRenderDef->m_obDaysDigitsPos.X()*m_fScale+(m_obPosition.X())-fMissingXOffset)*fScreenWidth;
			obPos.Y() = (m_pobRenderDef->m_obDaysDigitsPos.Y()*m_fScale+m_obPosition.Y())*fScreenHeight;
			obPos.Z() = m_pobRenderDef->m_obDaysDigitsPos.Z()*m_fScale;
	
			bLeadingZero = ( m_pobRenderDef->m_eDaysZeros == TIMER_ALWAYS );
			
			RenderDigits( iDays > 0 ? iDays : 0, obPos, bLeadingZero);
		}

		if ( m_pobRenderDef->m_eMaxUnits >= TIMER_HOURS )
		{
			// Hours
			obPos.X() = (m_pobRenderDef->m_obHrsPos.X()*m_fScale+m_obPosition.X()-fMissingXOffset)*fScreenWidth;
			obPos.Y() = (m_pobRenderDef->m_obHrsPos.Y()*m_fScale+m_obPosition.Y())*fScreenHeight;
			obPos.Z() = m_pobRenderDef->m_obHrsPos.Z()*m_fScale;

			m_obHours.SetPosition(obPos);
			m_obHours.SetWidth(m_pobRenderDef->m_fDaysWidth*m_fScale * fResolutionScale);
			m_obHours.SetHeight(m_pobRenderDef->m_fDaysHeight*m_fScale * fResolutionScale);
			m_obHours.SetColour(obColour.GetNTColor());
			m_obHours.Render();

			obPos.X() = (m_pobRenderDef->m_obHrsDigitsPos.X()*m_fScale+(m_obPosition.X())-fMissingXOffset)*fScreenWidth;
			obPos.Y() = (m_pobRenderDef->m_obHrsDigitsPos.Y()*m_fScale+m_obPosition.Y())*fScreenHeight;
			obPos.Z() = m_pobRenderDef->m_obHrsDigitsPos.Z()*m_fScale;

			bLeadingZero = ( m_pobRenderDef->m_eHrsZeros == TIMER_ALWAYS ) ||
				( ( m_pobRenderDef->m_eHrsZeros == TIMER_DYNAMIC ) && ( iDays > 0 ) );

			RenderDigits( iHours > 0 ? iHours : 0, obPos, bLeadingZero);
		}
	
		if ( m_pobRenderDef->m_eMaxUnits >= TIMER_MINS )
		{
			// Mins
			obPos.X() = (m_pobRenderDef->m_obMinsPos.X()*m_fScale+m_obPosition.X()-fMissingXOffset)*fScreenWidth;
			obPos.Y() = (m_pobRenderDef->m_obMinsPos.Y()*m_fScale+m_obPosition.Y())*fScreenHeight;
			obPos.Z() = m_pobRenderDef->m_obMinsPos.Z()*m_fScale;

			m_obMins.SetPosition(obPos);
			m_obMins.SetWidth(m_pobRenderDef->m_fMinsWidth*m_fScale * fResolutionScale);
			m_obMins.SetHeight(m_pobRenderDef->m_fMinsHeight*m_fScale * fResolutionScale);
			m_obMins.SetColour(obColour.GetNTColor());
			m_obMins.Render();

			obPos.X() = (m_pobRenderDef->m_obMinsDigitsPos.X()*m_fScale+(m_obPosition.X())-fMissingXOffset)*fScreenWidth;
			obPos.Y() = (m_pobRenderDef->m_obMinsDigitsPos.Y()*m_fScale+m_obPosition.Y())*fScreenHeight;
			obPos.Z() = m_pobRenderDef->m_obMinsDigitsPos.Z()*m_fScale;

			bLeadingZero = ( m_pobRenderDef->m_eMinsZeros == TIMER_ALWAYS ) ||
				( ( m_pobRenderDef->m_eMinsZeros == TIMER_DYNAMIC ) && ( iHours > 0 ) );
		
			RenderDigits( iMinutes > 0 ? iMinutes : 0, obPos, bLeadingZero);
		}

		if ( m_pobRenderDef->m_eMaxUnits >= TIMER_SECS )
		{
			// Secs
			/*obPos.X() = (m_pobRenderDef->m_obSecsPos.X()+m_obPosition.X()-fMissingXOffset)*fScreenWidth;
			obPos.Y() = (m_pobRenderDef->m_obSecsPos.Y()+m_obPosition.Y())*fScreenHeight;
			obPos.Z() = m_pobRenderDef->m_obSecsPos.Z();

			m_obSecs.SetPosition(obPos);
			m_obSecs.SetWidth(m_pobRenderDef->m_fSecsWidth*m_fScale * fResolutionScale);
			m_obSecs.SetHeight(m_pobRenderDef->m_fSecsHeight*m_fScale * fResolutionScale);
			m_obSecs.SetColour(obColour.GetNTColor());
			m_obSecs.Render();*/

			obPos.X() = (m_pobRenderDef->m_obSecsDigitsPos.X()*m_fScale+(m_obPosition.X())-fMissingXOffset)*fScreenWidth;
			obPos.Y() = (m_pobRenderDef->m_obSecsDigitsPos.Y()*m_fScale+m_obPosition.Y())*fScreenHeight;
			obPos.Z() = m_pobRenderDef->m_obSecsDigitsPos.Z()*m_fScale;
			
			bLeadingZero = ( m_pobRenderDef->m_eSecsZeros == TIMER_ALWAYS ) ||
				( ( m_pobRenderDef->m_eSecsZeros == TIMER_DYNAMIC ) && ( iMinutes > 0 ) );
		
			RenderDigits( fSeconds > 0.0f ? (int)fSeconds : 0, obPos, bLeadingZero);
		}
	}

	// FINISH RENDERING
	Renderer::Get().SetBlendMode( GFX_BLENDMODE_OVERWRITE );

	return true;
}

//------------------------------------------------------------------------------------------
//!
//!	CHudConfig::RenderDigits
//!
//------------------------------------------------------------------------------------------
void TimerRenderer::RenderDigits( int iNumber, const CPoint& obPosition, bool bLeadingZero )
{
	float fScreenWidth = CGuiManager::Get().BBWidth();
	float fResolutionScale = fScreenWidth/1280.0f;

	CPoint obPos = obPosition;

	CVector obNormalColour;
	obNormalColour = m_pobRenderDef->m_obTimeColour;
	obNormalColour.W() *= m_fOverallAlpha;

	int iUnits = iNumber % 10;

	int iTens = (iNumber - iUnits)/10;
	iTens = iTens % 10;

	
	ntAssert ( iUnits < 10 && iUnits >= 0 );
	ntAssert ( iTens < 10 && iTens >= 0 );

	if (iNumber < 10 && !bLeadingZero )
		iTens = -1;

	if (iTens != -1)
	{
		RenderStateBlock::SetBlendMode( m_pobRenderDef->m_eTimeBlendmode );

		m_aobDigit[iTens].SetWidth  ( m_pobRenderDef->m_fDigitWidth  * m_fScale * fResolutionScale);
		m_aobDigit[iTens].SetHeight ( m_pobRenderDef->m_fDigitHeight * m_fScale * fResolutionScale);
		m_aobDigit[iTens].SetPosition(obPos);
		m_aobDigit[iTens].SetColour(obNormalColour.GetNTColor());
		m_aobDigit[iTens].Render();
	}

	obPos.X() += m_pobRenderDef->m_fDigitSpacing*m_fScale*fResolutionScale;

	if (iUnits!=-1)
	{
		RenderStateBlock::SetBlendMode( m_pobRenderDef->m_eTimeBlendmode );

		m_aobDigit[iUnits].SetWidth ( m_pobRenderDef->m_fDigitWidth  * m_fScale  * fResolutionScale);
		m_aobDigit[iUnits].SetHeight( m_pobRenderDef->m_fDigitHeight * m_fScale * fResolutionScale);
		m_aobDigit[iUnits].SetPosition(obPos);
		m_aobDigit[iUnits].SetColour(obNormalColour.GetNTColor());
		m_aobDigit[iUnits].Render();
	}
}

float TimerRenderer::GetRenderWidth( void )
{
	ntAssert ( m_pobRenderDef );

	float fWidth;
	
	//float fScreenHeight = CGuiManager::Get().BBHeight();
	float fScreenWidth = CGuiManager::Get().BBWidth();

	m_fScale = m_pobRenderDef->m_fOverallScale;


	float fMissingXOffset = 0.0f;

	switch ( m_pobRenderDef->m_eMaxUnits )
	{
	case TIMER_SECS:
		fMissingXOffset = m_pobRenderDef->m_obSecsDigitsPos.X();
		break;

	case TIMER_MINS:
		fMissingXOffset = m_pobRenderDef->m_obMinsDigitsPos.X();
		break;

	case TIMER_HOURS:
		fMissingXOffset = m_pobRenderDef->m_obHrsDigitsPos.X();
		break;

	case TIMER_DAYS:
	default:
		fMissingXOffset = 0.0f;

		break;
	}

	// Pixels to final digits less any missing info
	fWidth = (m_pobRenderDef->m_obSecsDigitsPos.X() - fMissingXOffset) * fScreenWidth;

	// Pixels for final digits
	fWidth += 2.0f * m_pobRenderDef->m_fDigitSpacing*m_fScale;

	// This will need updating if Secs label is added

	return fWidth;
}

float TimerRenderer::GetRenderHeight( void )
{
	ntAssert ( m_pobRenderDef );

	float fHeight;
	
	// Pixels for final digits
	fHeight = m_pobRenderDef->m_fDigitHeight*m_fScale;

	return fHeight;
}

void TimerRenderer::GetClockTime(double dTime, int& iDays, int& iHours, int& iMinutes, float& fSeconds)
{
	double dSeconds = dTime;

	iDays = (int)dSeconds / 86400;
	dSeconds -= iDays * 86400.0;
	
	iHours = (int)dSeconds / 3600;
	dSeconds -= iHours * 3600.0;

	iMinutes = (int)dSeconds / 60;
	dSeconds -= iMinutes * 60.0;

	fSeconds = (float)dSeconds;
}

bool TimerRenderer::IsCatchingUp( void )
{
	return 	( m_fCatchupTime > 0.0f && m_dLastTime < m_dAimTime );
}

bool TimerRenderer::IsDraining( void )
{
	return 	( m_fCatchupTime > 0.0f && m_dLastTime > m_dAimTime );
}

