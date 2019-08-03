/***********************************************************************************************
*!		scee.sbashow: added module
*
*	DESCRIPTION		The overlay scheme, currently used with hudimage.
*
*	NOTES			Implements how overlays are applied to images
*					 (and themselves), and also how some are updated.
*
************************************************************************************************/


#include "hud/hudoverlay.h"
#include "hud/hudimage.h"

HudImageOverlaySlap::HudImageOverlaySlap():
	m_obColour( CVector( 1.0f, 1.0f, 1.0f, 1.0f) ),
	m_fTopLeftX( 0.5f ),
	m_fTopLeftY( 0.5f ),
	m_fScale(1.0f),
	m_bRendering(true)
{
}

void  HudImageOverlaySlap::ApplyImmOverlay( HudImageOverlaySlap& obTarg, 
											  const HudImageOverlaySlap& obSource, 
											  int iFilter)
{
	if (iFilter==OF_ALL)
	{
		obTarg = obSource;
	}
	else
	{
		if (iFilter&OF_COLOUR)
		{
			obTarg.m_obColour = obSource.m_obColour;
		}

		if (iFilter&OF_SCALE)
		{
			obTarg.m_fScale = obSource.m_fScale;
		}

		if (iFilter&OF_POS)
		{
			obTarg.m_fTopLeftX = obSource.m_fTopLeftX;
			obTarg.m_fTopLeftY = obSource.m_fTopLeftY;
		}

		if (iFilter&OF_RENDFLAG)
		{
			obTarg.m_bRendering = obSource.m_bRendering;
		}
	}
}

void HudImageOverlaySlap::InterpToOverlay(HudImageOverlaySlap& obTarg,
											const HudImageOverlaySlap& obSource,
											const HudImageOverlaySlap& obFinal,
											float fInterp,
											int iFilter)
{
	if (iFilter&OF_COLOUR)
	{
		obTarg.m_obColour = 
			obSource.m_obColour*(1.0f-fInterp) + obFinal.m_obColour*fInterp;
	}

	if (iFilter&OF_SCALE)
	{
		obTarg.m_fScale = 	
			obSource.m_fScale*(1.0f-fInterp) + obFinal.m_fScale*fInterp;
	}
    
	if (iFilter&OF_POS)
	{
		obTarg.m_fTopLeftX = 
			obSource.m_fTopLeftX*(1.0f-fInterp) + obFinal.m_fTopLeftX*fInterp;
		obTarg.m_fTopLeftY = 
			obSource.m_fTopLeftY*(1.0f-fInterp) + obFinal.m_fTopLeftY*fInterp;
	}

	if (iFilter&OF_RENDFLAG)
	{
		if (obFinal.m_bRendering)
		{
			if (fInterp>0.0f)
			{
				obTarg.m_bRendering = true;
			}
			else
			{
				obTarg.m_bRendering = 
					obSource.m_bRendering;
			}
		}
		else if (fInterp==1.0f)
		{
			obTarg.m_bRendering = false;
		}
		else
		{
			obTarg.m_bRendering = 
				obSource.m_bRendering;
		}
	}
}


//------------------------------------------------------------------------------------------
//!
//!	HudImageOverlayIncremental::Update
//! Incremental overlays can be updated if they have a characteristic time.
//!
//------------------------------------------------------------------------------------------
bool HudImageOverlayIncremental::Update( float fTimeStep )
{
	if(m_fCharacteristicTime>0.0f)
	{
		m_fTimeGoing+=fTimeStep;

		if (m_fTimeGoing>m_fCharacteristicTime)
		{
			if (m_eOverlayMode==OM_LINEAR)
			{
				m_fCharacteristicTime = 0.0f;
				m_fTimeGoing = 0.0f;
				m_fCurrParam = 1.0f;
				return true;
			}

			m_fTimeGoing -= m_fCharacteristicTime;
		}

		switch(this->m_eOverlayMode)
		{
			case OM_LINEAR:
				{
					m_fCurrParam = 
						1.0f - CMaths::SmoothStep(m_fTimeGoing/m_fCharacteristicTime);
				}
				break;
			case OM_OSCILLATE:
				{
					m_fCurrParam = 
						fsinf(m_fTimeGoing*(2.0f*PI)/m_fCharacteristicTime);
				}
				break;
		}

		return false;
	}
	else
	{
		m_fCurrParam = 1.0f;
		return true;
	}
}
	
//------------------------------------------------------------------------------------------
//!
//!	HudImageOverlayIncremental::ApplyTo
//! Incremental overlays are applied to snap overlays - which in turn get applied to the image.
//!
//------------------------------------------------------------------------------------------

const HudImageOverlaySlap 
	HudImageOverlayIncremental::ApplyTo(const HudImageOverlaySlap& obIn)
{
	HudImageOverlaySlap obSnap(obIn);

	if (m_iFilter&OF_POS)
	{
		obSnap.m_fTopLeftX+=m_fDX*m_fApplicationWeight*m_fCurrParam;
		obSnap.m_fTopLeftY+=m_fDY*m_fApplicationWeight*m_fCurrParam;
	}

	if (m_iFilter&OF_SCALE)
	{
		obSnap.m_fScale*=(1.0f+(m_fDfScale-1.0f)*m_fApplicationWeight*m_fCurrParam);
    }

	if (m_iFilter&OF_COLOUR)
	{
		obSnap.m_obColour+= m_obDColour*m_fCurrParam;
		obSnap.m_obColour = obSnap.m_obColour.Max(CVector(0.0f,0.0f,0.0f,0.0f));
		obSnap.m_obColour = obSnap.m_obColour.Min(CVector(1.0f,1.0f,1.0f,1.0f));
	}

	return obSnap;
}

