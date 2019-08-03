/***************************************************************************************************
*
*	$Header:: /game/smoother.cpp 2     12/08/03 11:01 Wil                                          $
*
*
*
*	CHANGES
*
*	11/8/2003	Wil	Created
*
***************************************************************************************************/

#include "core/visualdebugger.h"

#include "camera/smoother.h"
#include "objectdatabase/dataobject.h"

START_CHUNKED_INTERFACE( SmootherDef, Mem::MC_CAMERA)
	PUBLISH_VAR_WITH_DEFAULT_AS(m_iNumSamples, 20, NumSamples)
	PUBLISH_VAR_WITH_DEFAULT_AS(m_fTightness, 0.5f, Tightness)
	PUBLISH_VAR_WITH_DEFAULT_AS(m_fLagFrac, 0.25f, LagFrac)
END_STD_INTERFACE

SmootherDef::SmootherDef()
: m_iNumSamples(FD_MID_SAMPLES), 
  m_fTightness(FD_MID_TIGHTNESS), 
  m_fLagFrac(FD_MID_LAGFRAC)
{
}


/***************************************************************************************************
*	
*	FUNCTION		CSmoother::CSmoother
*
*	DESCRIPTION		construct our smoothing object
*
***************************************************************************************************/
CSmoother::CSmoother(const SmootherDef& def)
:	m_obDef(CSmootherDef(def.GetNumSamples(), def.GetTightness(), def.GetLagFrac())),
	m_bInitialised(false)
{
	AllocBuffers();
}


CSmoother::CSmoother(const CSmoother& obCopy) :
	m_obDef("NULL")//obCopy.m_obDef.GetName())
{
	NT_MEMCPY(this, &obCopy, sizeof(CSmoother));
	AllocBuffers();

	for (int i = 0; i < m_obDef.GetNumSamples(); i++)
	{
		m_pfSampleTimes[i] = obCopy.m_pfSampleTimes[i];
		m_pfSampleDurs[i] = obCopy.m_pfSampleDurs[i];
		m_pfTargetBuff[i] = obCopy.m_pfTargetBuff[i];
		m_pfVelocityBuff[i] = obCopy.m_pfVelocityBuff[i];
	}
};

/***************************************************************************************************
*	
*	FUNCTION		CSmoother::AllocBuffers
*
*	DESCRIPTION		alloc memory
*
***************************************************************************************************/
void	CSmoother::AllocBuffers()
{
	float* pfData = NT_NEW_ARRAY_CHUNK( Mem::MC_CAMERA ) float [m_obDef.GetNumSamples() * 4];
	ntAssert(pfData);

	m_pfSampleTimes = pfData;

	pfData += m_obDef.GetNumSamples();
	m_pfSampleDurs = pfData;

	pfData += m_obDef.GetNumSamples();
	m_pfTargetBuff = pfData;

	pfData += m_obDef.GetNumSamples();
	m_pfVelocityBuff = pfData;
}

/***************************************************************************************************
*	
*	FUNCTION		CSmoother::~CPointSmoother
*
*	DESCRIPTION		cleanup
*
***************************************************************************************************/
CSmoother::~CSmoother()
{
	NT_DELETE_ARRAY_CHUNK(Mem::MC_CAMERA, m_pfSampleTimes );
};

/***************************************************************************************************
*	
*	FUNCTION		CSmoother::Initialise
*
*	DESCRIPTION		setup our interp values
*
***************************************************************************************************/
void	CSmoother::Initialise(float fNewVal)
{
	m_bInitialised = true;

	m_iLastSample = 0;
	m_iCurrentSample = 1;
	
	m_fEmitTimer = 0.0f;
	m_fSampleLength = 0.0f;

	m_fOldTargetMean = fNewVal;
	m_fNewTargetMean = fNewVal;
	m_fTargetMean = fNewVal;
//	m_fLaggedVal = fNewVal;

	m_fOldVelocityMean = 0.0f;
	m_fNewVelocityMean = 0.0f;
	m_fVelocityMean = 0.0f;
//	m_fLaggedVel = 0.0f;

	m_fTimeSinceInit = 0.0f;

	for (int i = 0; i < m_obDef.GetNumSamples(); i++)
	{
		m_pfSampleTimes[i] = m_fTimeSinceInit;
		m_pfSampleDurs[i] = 0.0f;

		m_pfTargetBuff[i] = fNewVal;
		m_pfVelocityBuff[i] = 0.0f;
	}
}

/***************************************************************************************************
*	
*	FUNCTION		CSmoother::AdjustMeanValueSample
*
*	DESCRIPTION		Calculate the new mean value via discrete sampling
*
***************************************************************************************************/
void	CSmoother::AdjustMeanValueSample(float fNewVal, float fTimeChange)
{
	// we always update our absolute time
	m_fTimeSinceInit += fTimeChange;

	// never do this if we're stopped or running backwards..
	if(fTimeChange <= 0.0f)
		return;

	// get info on this time step
	float fEmitTime = 0.0f;
	float fEmitInterval = m_obDef.GetTightness() / m_obDef.GetNumSamples();

	float fStartTime = m_pfSampleTimes[m_iLastSample];
	float fRange = m_fTimeSinceInit - fStartTime;

	// weird bug where times get out of synch
	ntAssert(fRange >= 0.0f);

	// calc starting value to interp from and direction to interp in
	float fStartVal = m_pfTargetBuff[m_iLastSample];
	float fGradient = fNewVal - fStartVal;
	
	// see if its time to emit a new point into the buffer
	m_fEmitTimer += fTimeChange;
	while (m_fEmitTimer >= fEmitInterval)
	{
		m_fEmitTimer -= fEmitInterval;
		fEmitTime += fEmitInterval;

		// update our current samples
		m_pfSampleTimes[m_iCurrentSample] = ntstd::Clamp(fEmitTime + fStartTime, fStartTime, m_fTimeSinceInit);
		m_pfTargetBuff[m_iCurrentSample] = fStartVal;

		if((fRange > EPSILON) && (fabsf(fGradient) > EPSILON))
		{
			float fInterp = ntstd::Clamp(fEmitTime / fRange, 0.0f, 1.0f);
			m_pfTargetBuff[m_iCurrentSample] += fGradient * fInterp;
		}

		// get latest sample interval
		m_pfSampleDurs[m_iCurrentSample] = m_pfSampleTimes[m_iCurrentSample] - m_pfSampleTimes[m_iLastSample];
		m_pfVelocityBuff[m_iLastSample] = m_pfTargetBuff[m_iCurrentSample] - m_pfTargetBuff[m_iLastSample];

		// get duration of current samples
		m_fSampleLength = 0.0f;
		for (int i = 0; i < m_obDef.GetNumSamples(); i++)
			m_fSampleLength += m_pfSampleDurs[i];

		// get time averaged mean values and velocities
		m_fOldTargetMean = m_fNewTargetMean;
		m_fOldVelocityMean = m_fNewVelocityMean;

		m_fNewTargetMean = 0.0f;
		m_fNewVelocityMean = 0.0f;

		for (int i = 0; i < m_obDef.GetNumSamples(); i++)
		{
			m_fNewTargetMean += m_pfTargetBuff[i] * m_pfSampleDurs[i];
			m_fNewVelocityMean += m_pfVelocityBuff[i] * m_pfSampleDurs[i];
		}

		if(m_fSampleLength > EPSILON)
		{
			m_fNewTargetMean /= m_fSampleLength;
			m_fNewVelocityMean /= m_fSampleLength;
		}

		// update next sample index and emit time
		m_iLastSample = m_iCurrentSample;
		m_iCurrentSample++;
		if(m_iCurrentSample == m_obDef.GetNumSamples())
			m_iCurrentSample = 0;
	}

	// weird bug where times get out of synch, shoudl be fixed by clamping
//	ntAssert(m_pfVelocityBuff[m_iLastSample] < 1000000.0f);
	ntAssert(m_pfSampleTimes[m_iLastSample] <= m_fTimeSinceInit);

	// get actual target and velocity mean (velocity in units per second)
	m_fTargetMean = ((m_fNewTargetMean - m_fOldTargetMean) * (m_fEmitTimer / fEmitInterval)) + m_fOldTargetMean;
	m_fVelocityMean = ((m_fNewVelocityMean - m_fOldVelocityMean) * (m_fEmitTimer / fEmitInterval)) + m_fOldVelocityMean;
	m_fVelocityMean *= 1.0f / fEmitInterval;
}

/***************************************************************************************************
*	
*	FUNCTION		CSmoother::CalcLaggedValues
*
*	DESCRIPTION		Calculate the lagged value and velocity
*
***************************************************************************************************/
void	CSmoother::CalcLaggedValues(float){}
/*
void	CSmoother::CalcLaggedValues(float fTimeChange)
{
	if(fTimeChange <= 0.0f)
		return;

	float fOldLagVal = m_fLaggedVal;

	// update our smooth lag value and velocity
	
	// WD removed this in time change re-write (14.02.04)
//	float	fLagFrac = ntstd::Clamp(CTimer::Get().GetGameTimeScalar() * m_obDef.GetLagFrac(), 0.0f, 1.0f);
	float	fLagFrac = ntstd::Clamp(m_obDef.GetLagFrac(), 0.0f, 1.0f);

	m_fLaggedVal = (m_fTargetMean * fLagFrac) + (m_fLaggedVal * (1.0f - fLagFrac));
	m_fLaggedVel = (m_fLaggedVal - fOldLagVal) / fTimeChange;
}
*/
/***************************************************************************************************
*	
*	FUNCTION		CSmoother::Update
*
*	DESCRIPTION		update our new values
*
***************************************************************************************************/
void	CSmoother::Update(float fNewVal, float fTimeChange)
{
	if(!m_bInitialised)
		Initialise(fNewVal);
	else
	{
		AdjustMeanValueSample(fNewVal, fTimeChange);

//		CalcLaggedValues(fTimeChange);
	}
}




















/***************************************************************************************************
*	
*	FUNCTION		CPointSmoother::CPointSmoother
*
*	DESCRIPTION		construct our smoothing object
*
***************************************************************************************************/
CPointSmoother::CPointSmoother(const SmootherDef& def) :
	m_obDef(CSmootherDef(def.GetNumSamples(), def.GetTightness(), def.GetLagFrac())),
	m_bInitialised(false)
{
	AllocBuffers();
}


CPointSmoother::CPointSmoother(const CPointSmoother& obCopy) :
	m_obDef("NULL")//obCopy.m_obDef.GetName())
{
	NT_MEMCPY(this, &obCopy, sizeof(CPointSmoother));
	AllocBuffers();

	for (int i = 0; i < m_obDef.GetNumSamples(); i++)
	{
		m_pfSampleTimes[i] = obCopy.m_pfSampleTimes[i];
		m_pfSampleDurs[i] = obCopy.m_pfSampleDurs[i];
		m_pobTargetBuff[i] = obCopy.m_pobTargetBuff[i];
		m_pobVelocityBuff[i] = obCopy.m_pobVelocityBuff[i];
	}
};

/***************************************************************************************************
*	
*	FUNCTION		CPointSmoother::AllocBuffers
*
*	DESCRIPTION		memory allocs
*
***************************************************************************************************/
void	CPointSmoother::AllocBuffers()
{
	m_pfSampleTimes = NT_NEW_ARRAY_CHUNK( Mem::MC_CAMERA ) float [m_obDef.GetNumSamples()];
	ntAssert(m_pfSampleTimes);
	memset( m_pfSampleTimes, 0, m_obDef.GetNumSamples() * sizeof(float) );

	m_pfSampleDurs = NT_NEW_ARRAY_CHUNK( Mem::MC_CAMERA ) float [m_obDef.GetNumSamples()];
	ntAssert(m_pfSampleDurs);
	memset( m_pfSampleDurs, 0, m_obDef.GetNumSamples() * sizeof(float) );

	m_pobTargetBuff = NT_NEW_ARRAY_CHUNK( Mem::MC_CAMERA ) CPoint [m_obDef.GetNumSamples()];
	ntAssert(m_pobTargetBuff);
	memset( m_pobTargetBuff, 0, m_obDef.GetNumSamples() * sizeof(CPoint) );

	m_pobVelocityBuff = NT_NEW_ARRAY_CHUNK( Mem::MC_CAMERA ) CDirection [m_obDef.GetNumSamples()];
	ntAssert(m_pobVelocityBuff);
	memset( m_pobVelocityBuff, 0, m_obDef.GetNumSamples() * sizeof(CDirection) );
}

/***************************************************************************************************
*	
*	FUNCTION		CPointSmoother::~CPointSmoother
*
*	DESCRIPTION		cleanup
*
***************************************************************************************************/
CPointSmoother::~CPointSmoother()
{
	NT_DELETE_ARRAY_CHUNK(Mem::MC_CAMERA, m_pfSampleTimes );
	NT_DELETE_ARRAY_CHUNK(Mem::MC_CAMERA, m_pfSampleDurs );
	NT_DELETE_ARRAY_CHUNK(Mem::MC_CAMERA, m_pobTargetBuff );
	NT_DELETE_ARRAY_CHUNK(Mem::MC_CAMERA, m_pobVelocityBuff );
};

/***************************************************************************************************
*	
*	FUNCTION		CPointSmoother::Initialise
*
*	DESCRIPTION		setup our interp values
*
***************************************************************************************************/
void	CPointSmoother::Initialise(const CPoint& obNewPos)
{
	m_bInitialised = true;

	m_iLastSample = 0;
	m_iCurrentSample = 1;
	
	m_fEmitTimer = 0.0f;
	m_fSampleLength = 0.0f;

	m_obOldTargetMean = obNewPos;
	m_obNewTargetMean = obNewPos;
	m_obTargetMean = obNewPos;
//	m_obLaggedPos = obNewPos;

	m_obOldVelocityMean.Clear();
	m_obNewVelocityMean.Clear();
	m_obVelocityMean.Clear();
//	m_obLaggedVel.Clear();

	m_fTimeSinceInit = 0.0f;

	for (int i = 0; i < m_obDef.GetNumSamples(); i++)
	{
		m_pfSampleTimes[i] = m_fTimeSinceInit;
		m_pfSampleDurs[i] = 0.0f;

		m_pobTargetBuff[i] = obNewPos;
		m_pobVelocityBuff[i].Clear();
	}
}

/***************************************************************************************************
*	
*	FUNCTION		CPointSmoother::AdjustMeanValueSample
*
*	DESCRIPTION		Calculate the new mean value via discrete sampling
*
***************************************************************************************************/
void	CPointSmoother::AdjustMeanValueSample(const CPoint& obNewPos, float fTimeChange)
{
	// we always update our absolute time
	m_fTimeSinceInit += fTimeChange;

	// never do this if we're stopped or running backwards..
	if(fTimeChange <= 0.0f)
		return;

	// get info on this time step
	float fEmitTime = 0.0f;
	float fEmitInterval = m_obDef.GetTightness() / m_obDef.GetNumSamples();

	float fStartTime = m_pfSampleTimes[m_iLastSample];
	float fRange = m_fTimeSinceInit - fStartTime;

	// weird bug where times get out of synch
	ntAssert(fRange >= 0.0f);

	// calc starting pos to interp from and direction to interp in
	CPoint obStartPos = m_pobTargetBuff[m_iLastSample];
	CDirection obMoveDir = obNewPos ^ obStartPos;

	float fLength = obMoveDir.Length();
	obMoveDir.Normalise();

	// see if its time to emit a new point into the buffer
	m_fEmitTimer += fTimeChange;
	while (m_fEmitTimer > fEmitInterval)
	{
		m_fEmitTimer -= fEmitInterval;
		fEmitTime += fEmitInterval;

		// update our current samples
		m_pfSampleTimes[m_iCurrentSample] = fEmitTime + fStartTime;
		m_pfSampleTimes[m_iCurrentSample] = ntstd::Clamp(m_pfSampleTimes[m_iCurrentSample], fStartTime, m_fTimeSinceInit);		
		
		m_pobTargetBuff[m_iCurrentSample] = obStartPos;

		if((fRange > EPSILON) && (fLength > EPSILON))
		{
			float fInterp = ntstd::Clamp(fEmitTime / fRange, 0.0f, 1.0f);
			m_pobTargetBuff[m_iCurrentSample] += obMoveDir * (fInterp * fLength);
		}

		// get latest sample interval
		m_pfSampleDurs[m_iCurrentSample] = m_pfSampleTimes[m_iCurrentSample] - m_pfSampleTimes[m_iLastSample];
		m_pobVelocityBuff[m_iLastSample] = m_pobTargetBuff[m_iCurrentSample] ^ m_pobTargetBuff[m_iLastSample];

		// get duration of current samples
		m_fSampleLength = 0.0f;
		for (int i = 0; i < m_obDef.GetNumSamples(); i++)
			m_fSampleLength += m_pfSampleDurs[i];

		// get time averaged mean position and velocities
		m_obOldTargetMean = m_obNewTargetMean;
		m_obOldVelocityMean = m_obNewVelocityMean;

		m_obNewTargetMean.Clear();
		m_obNewVelocityMean.Clear();

		for (int i = 0; i < m_obDef.GetNumSamples(); i++)
		{
			m_obNewTargetMean += m_pobTargetBuff[i] * m_pfSampleDurs[i];
			m_obNewVelocityMean += m_pobVelocityBuff[i] * m_pfSampleDurs[i];
		}

		if(m_fSampleLength)
		{
			m_obNewTargetMean /= m_fSampleLength;
			m_obNewVelocityMean /= m_fSampleLength;
		}

		// update next sample index and emit time
		m_iLastSample = m_iCurrentSample;
		m_iCurrentSample++;
		if(m_iCurrentSample == m_obDef.GetNumSamples())
			m_iCurrentSample = 0;
	}

	// weird bug where times get out of synch, should be fixed by clamping
	ntAssert(m_pobVelocityBuff[m_iLastSample].Length() < 1000000.0f);
	ntAssert(m_pfSampleTimes[m_iLastSample] <= m_fTimeSinceInit);

	// get actual target mean
	m_obTargetMean = ((m_obNewTargetMean - m_obOldTargetMean) * (m_fEmitTimer / fEmitInterval)) + m_obOldTargetMean;

	// get actual vel mean
	m_obVelocityMean = ((m_obNewVelocityMean - m_obOldVelocityMean) * (m_fEmitTimer / fEmitInterval)) + m_obOldVelocityMean;
	m_obVelocityMean *= 1.0f / fEmitInterval; // convert to metres per second
}

/***************************************************************************************************
*	
*	FUNCTION		CPointSmoother::CalcLaggedValues
*
*	DESCRIPTION		Calculate the lagged position and velocity
*
***************************************************************************************************/
void	CPointSmoother::CalcLaggedValues(float){}
/*
void	CPointSmoother::CalcLaggedValues(float fTimeChange)
{
	if(fTimeChange <= 0.0f)
		return;

	CPoint obOldPos(m_obLaggedPos);

	// update our smooth lag pos

	// WD removed this in time change re-write (14.02.04)
//	float	fLagFrac = ntstd::Clamp(CTimer::Get().GetGameTimeScalar() * m_obDef.GetLagFrac(), 0.0f, 1.0f);
	float	fLagFrac = ntstd::Clamp(m_obDef.GetLagFrac(), 0.0f, 1.0f);
	m_obLaggedPos = (m_obTargetMean * fLagFrac) + (m_obLaggedPos * (1.0f - fLagFrac));

	// update our smooth lagged velocity
	m_obLaggedVel = (m_obLaggedPos ^ obOldPos) / fTimeChange;
}
*/
/***************************************************************************************************
*	
*	FUNCTION		CPointSmoother::Update
*
*	DESCRIPTION		Update our mean positions
*
***************************************************************************************************/
void	CPointSmoother::Update(const CPoint& obNewTarget, float fTimeChange)
{
	if(!m_bInitialised)
		Initialise(obNewTarget);
	else
	{
		AdjustMeanValueSample(obNewTarget, fTimeChange);

//		CalcLaggedValues(fTimeChange);
	}
}

/***************************************************************************************************
*	
*	FUNCTION		CPointSmoother::Render
*
*	DESCRIPTION		Debug Render 
*
***************************************************************************************************/
void CPointSmoother::Render() const
{
#ifndef _GOLD_MASTER
	ntAssert(m_bInitialised);

	// render our average and lagged positions
	g_VisualDebug->RenderSphere( CQuat( CONSTRUCT_IDENTITY ), m_obTargetMean, 0.2f, DC_RED );

	// render our average and lagged velocities
	g_VisualDebug->RenderLine( m_obTargetMean, m_obVelocityMean + m_obTargetMean, DC_RED );
#endif
}

