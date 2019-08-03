/***************************************************************************************************
*
*	$Header:: /game/matrixtweaker.cpp 1     11/08/03 16:31 Wil                                     $
*
*
*
*	CHANGES
*
*	11/8/2003	Wil	Created
*
***************************************************************************************************/

#include "camera/camtools.h"
#include "camera/matrixtweaker.h"
#include "game/shellconfig.h"

#include "core/visualdebugger.h" 

CMatrixTweaker::CMatrixTweaker(const CMatrixTweakerDef& obDef) :
	m_obDef(obDef),
	m_obLongConverger(m_obDef.m_obConDef),
	m_obLatConverger(m_obDef.m_obConDef)
{
	m_pobLongSmoother = NT_NEW_CHUNK( Mem::MC_CAMERA ) CSmoother(m_obDef.m_obSmoothDef);
	m_pobLatSmoother = NT_NEW_CHUNK( Mem::MC_CAMERA ) CSmoother(m_obDef.m_obSmoothDef);
	ntAssert(m_pobLatSmoother);
	ntAssert(m_pobLongSmoother);

	Reset();
}

CMatrixTweaker::~CMatrixTweaker()
{
	NT_DELETE_CHUNK(Mem::MC_CAMERA, m_pobLongSmoother );
	NT_DELETE_CHUNK(Mem::MC_CAMERA, m_pobLatSmoother );
}

/***************************************************************************************************
*	
*	FUNCTION		CMatrixTweaker::Reset
*
*	DESCRIPTION		reset the tweaker
*
***************************************************************************************************/
void	CMatrixTweaker::Reset()
{
	m_obLastSrc.SetIdentity();
	m_obLastTweaked.SetIdentity();

	m_fLongOffset = 0.0f;
	m_fLatOffset = 0.0f;

	m_obLongConverger.Reset();
	m_obLatConverger.Reset();

	m_pobLongSmoother->Reset();
	m_pobLatSmoother ->Reset();

	m_bInitialised = false;
};

/***************************************************************************************************
*	
*	FUNCTION		CMatrixTweaker::Render
*
*	DESCRIPTION		render stuff
*
***************************************************************************************************/
void	CMatrixTweaker::RenderInfo(int iX, int iY)
{
	CCamUtil::DebugPrintf(iX, iY, "CMatrixTweaker: %s", "NULL");//m_obDef.GetName());
}

/***************************************************************************************************
*	
*	FUNCTION		CMatrixTweaker::ApplyTweak
*
*	DESCRIPTION		change the dir we're looking at based on the right analogue stick
*
*	NOTES			can be applied relative to the old camera, or generateg with a normal Y
*					axis (i.e roll can be preserved or not)
*
***************************************************************************************************/
CMatrix	CMatrixTweaker::ApplyTweak(const CMatrix& obSrc, float fTimeChange)
{
	m_bInitialised = true;
	m_obLastSrc = obSrc;

	// call virtual tweaker calc func
	CalculateLatLongOffsets(fTimeChange);

	// clamp offsets
	m_fLongOffset = ntstd::Clamp(m_fLongOffset, -m_obDef.GetMaxLong(), m_obDef.GetMaxLong());
	m_fLatOffset = ntstd::Clamp(m_fLatOffset, -m_obDef.GetMaxLat(), m_obDef.GetMaxLat());

	// converge then re-clamp
	float fLongOffset, fLatOffset;

	m_pobLongSmoother->Update(m_fLongOffset, fTimeChange);
	m_pobLatSmoother->Update(m_fLatOffset, fTimeChange);

	fLongOffset = m_pobLongSmoother->GetTargetMean();
	fLatOffset = m_pobLatSmoother->GetTargetMean();

	fLongOffset = m_obLongConverger.Update(fLongOffset, fTimeChange);
	fLatOffset = m_obLatConverger.Update(fLatOffset, fTimeChange);

	fLongOffset = ntstd::Clamp(fLongOffset, -m_obDef.GetMaxLong(), m_obDef.GetMaxLong());
	fLatOffset = ntstd::Clamp(fLatOffset, -m_obDef.GetMaxLat(), m_obDef.GetMaxLat());

	// get old spherical look at
	float fLat = 0.0f, fLong = 0.0f;
	CCamUtil::SphericalFromCartesian(m_obLastSrc.GetZAxis(), fLat, fLong);

	// add our offsets
	fLat += fLatOffset * DEG_TO_RAD_VALUE;
	fLong += fLongOffset * DEG_TO_RAD_VALUE;

	// construct our new matrix
	if(m_obDef.GetYPreserve())
	{
		CPoint obNewZ = CCamUtil::CartesianFromSpherical(fLat, fLong) + m_obLastSrc.GetTranslation();

		CCamUtil::CreateFromPoints(m_obLastTweaked, m_obLastSrc.GetTranslation(), obNewZ, m_obLastSrc.GetYAxis());
	}
	else
	{
		// make sure we dont go over the poles
		fLat = ntstd::Clamp(fLat, -(89.0f * DEG_TO_RAD_VALUE), (89.0f * DEG_TO_RAD_VALUE));

		CPoint obNewZ = CCamUtil::CartesianFromSpherical(fLat, fLong) + m_obLastSrc.GetTranslation();

		CCamUtil::CreateFromPoints(m_obLastTweaked, m_obLastSrc.GetTranslation(), obNewZ);
	}

	return m_obLastTweaked;
}

/***************************************************************************************************
*	
*	FUNCTION		CCameraElasticTweaker::CalculateLatLongOffsets
*
*	DESCRIPTION		change the dir we're looking at based on the right analogue stick
*
***************************************************************************************************/
void	CCameraElasticTweaker::CalculateLatLongOffsets(float fTimeChange)
{
	UNUSED(fTimeChange);

	float fXFrac = 0.0f;
	float fYFrac = 0.0f;

	// Using the motion controller
	// Need to expose values if we like this behaviour - JML
	static const float s_fBegin = .15f;
	static const float s_fMax   = .2f;
	if(CInputHardware::Get().GetPadContext() == PAD_CONTEXT_GAME)
	{
		if(g_ShellOptions->m_bUseMotionSensor_ForCams)
		{
			float fX     = CInputHardware::Get().GetPad(m_obDef.GetPad()).GetSensorFilteredMag(PAD_SENSOR_ACCEL_X, PAD_SENSOR_FILTER_EXPONENTIAL_5);
			float fY     = CInputHardware::Get().GetPad(m_obDef.GetPad()).GetSensorFilteredMag(PAD_SENSOR_ACCEL_Z, PAD_SENSOR_FILTER_EXPONENTIAL_5);
			float fTotal = fsqrtf(fX*fX + fY*fY);

			if(fTotal > s_fBegin)
			{
				// Total Magnitude Factor
				//float fFrac = CCamUtil::Sigmoid((fTotal-s_fBegin), (s_fMax-s_fBegin));
				float fFrac = clamp((fTotal-s_fBegin) / (s_fMax-s_fBegin), 0.f, 1.f);

				fXFrac = (1/fabsf(fX+fY))*fX;
				fYFrac = (1/fabsf(fX+fY))*-fY;
				float fSigma = max(fabsf(fXFrac), fabsf(fYFrac));

				fXFrac = fFrac*(1/fSigma)*fXFrac;
				fYFrac = fFrac*(1/fSigma)*fYFrac;
			}

			//g_VisualDebug->Printf2D(50.0f, 450.0f, DC_WHITE, 0, "(*%.2f*, *%.2f)->(%.2f, %.2f) (%.2f)", fX, fY, fYFrac, fXFrac, fTotal);
		}
		else
		{
			if(CInputHardware::Get().GetPad(m_obDef.GetPad()).GetHeld() & PAD_LEFT)  fXFrac -= 1.0f;
			if(CInputHardware::Get().GetPad(m_obDef.GetPad()).GetHeld() & PAD_RIGHT) fXFrac += 1.0f;
			if(CInputHardware::Get().GetPad(m_obDef.GetPad()).GetHeld() & PAD_UP)    fYFrac -= 1.0f;
			if(CInputHardware::Get().GetPad(m_obDef.GetPad()).GetHeld() & PAD_DOWN)  fYFrac += 1.0f;
		}
	}

	
	m_fLongOffset = fXFrac * -m_obDef.GetMaxLong();

	if(m_obDef.GetYReverse())
		m_fLatOffset = -fYFrac * m_obDef.GetMaxLat();
	else
		m_fLatOffset = fYFrac * m_obDef.GetMaxLat();


}

/***************************************************************************************************
*	
*	FUNCTION		CCameraManualTweaker::CalculateLatLongOffsets
*
*	DESCRIPTION		change the dir we're looking at based on the right analogue stick
*
***************************************************************************************************/
void	CCameraManualTweaker::CalculateLatLongOffsets(float fTimeChange)
{
	if(CInputHardware::Get().GetPad(m_obDef.GetPad()).GetPressed() * PAD_RIGHT_THUMB)
		m_bLookaround = !m_bLookaround;
	
	float fIncSpeed = m_obDef.m_obConDef.GetSpeed() * fTimeChange;

	if(m_bLookaround)
	{

		float fXFrac = 0.0f;
		float fYFrac = 0.0f;

		// Only allow tweaking if inside game context
		if(CInputHardware::Get().GetPadContext() == PAD_CONTEXT_GAME)
		{
			if(CInputHardware::Get().GetPad(m_obDef.GetPad()).GetHeld() & PAD_LEFT)  fXFrac -= 1.0f;
			if(CInputHardware::Get().GetPad(m_obDef.GetPad()).GetHeld() & PAD_RIGHT) fXFrac += 1.0f;
			if(CInputHardware::Get().GetPad(m_obDef.GetPad()).GetHeld() & PAD_UP)    fYFrac -= 1.0f;
			if(CInputHardware::Get().GetPad(m_obDef.GetPad()).GetHeld() & PAD_DOWN)  fYFrac += 1.0f;
		}

		m_fLongOffset += fXFrac * -fIncSpeed;

		if(m_obDef.GetYReverse())
			m_fLatOffset -= fYFrac * fIncSpeed;
		else
			m_fLatOffset += fYFrac * fIncSpeed;

	}
	else
	{
		m_fLongOffset = 0.0f;
		m_fLatOffset = 0.0f;
	}
}

