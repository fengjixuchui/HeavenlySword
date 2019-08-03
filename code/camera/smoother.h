/***************************************************************************************************
*
*	$Header:: /game/smoother.h 2     12/08/03 11:01 Wil                                            $
*
*
*
*	CHANGES
*
*	11/8/2003	Wil	Created
*
***************************************************************************************************/

#ifndef SMOOTHER_H
#define SMOOTHER_H

// size of sample buffer: affects how smoothly our average will change by
#define FD_MIN_SAMPLES 5
#define FD_MID_SAMPLES 20
#define FD_MAX_SAMPLES 60

// how 'tight' i.e how close the average is to the last emited point (smaller == tighter)
#define FD_MIN_TIGHTNESS 0.01f
#define FD_MID_TIGHTNESS 0.5f
#define FD_MAX_TIGHTNESS 2.0f

// how time lagged our lagged average will be (1.0 == none, ->0.0f = very)
#define FD_MIN_LAGFRAC 0.01f
#define FD_MID_LAGFRAC 0.25f
#define FD_MAX_LAGFRAC 1.0f


class SmootherDef
{
public:
	HAS_INTERFACE(SmootherDef)
	SmootherDef();
	
	int		GetNumSamples()	const {return m_iNumSamples;}
	float	GetTightness()	const {return m_fTightness;}
	float	GetLagFrac()	const {return m_fLagFrac;}

protected:
	friend class SmootherDefI;
	int		m_iNumSamples;
	float	m_fTightness;
	float	m_fLagFrac;
};

/***************************************************************************************************
*
*	CLASS			CSmootherDef
*
*	DESCRIPTION		defines the point smoother properties
*					nb, these max and mins for samples and tightness are appropriate for the camera,
*					not set in stone...
*
***************************************************************************************************/
class CSmootherDef : public SmootherDef
{
public:
	CSmootherDef(const char*) {}
	
	CSmootherDef(int nSamps, float fTightness, float fLagFrac)
	{
		m_iNumSamples = nSamps;
		m_fTightness  = fTightness;
		m_fLagFrac    = fLagFrac;
	}
	
	void	SetNumSamples(int iNumSamples)
	{
		ntAssert(iNumSamples >= FD_MIN_SAMPLES);
		ntAssert(iNumSamples <= FD_MAX_SAMPLES);
		m_iNumSamples = iNumSamples;
	}

	void	SetTightness(float fTightness)
	{
		ntAssert(fTightness >= FD_MIN_TIGHTNESS);
		ntAssert(fTightness <= FD_MAX_TIGHTNESS);
		m_fTightness = fTightness;
	}

	void	SetLagFrac(float fLagFrac)
	{
		ntAssert(fLagFrac >= FD_MIN_LAGFRAC);
		ntAssert(fLagFrac <= FD_MAX_LAGFRAC);
		m_fLagFrac = fLagFrac;
	}
};

/***************************************************************************************************
*
*	CLASS			CSmoother
*
*	DESCRIPTION		Takes a new sample value and stores it in a buffer in its interpolated postion.
*					From this it calculates an average value at the current moment in time
*					and the average velocity.
*					It also calculates a time lagged version of these.
*
*	NOTES			-If used when time is frozen or reversed, the state of the smoother will not be
*					changed.
*					-Update should be called every frame, internally it works out when to emit
*					a new value into the buffer.
*
***************************************************************************************************/
class CSmoother
{
public:
	CSmoother(const SmootherDef& def);
	CSmoother(const CSmoother& obCopy);
	~CSmoother();


	void	Update(float fNewTarget, float fTimeChange);
	void	Reset() {m_bInitialised = false;}

	float	GetTargetMean()	const	{ntAssert(m_bInitialised); return m_fTargetMean;}
	float	GetVelocityMean()	const	{ntAssert(m_bInitialised); return m_fVelocityMean;}

	// this functionality is depricated as of (03.03.04) WD
//	float	GetLaggedVal()	const	{ntAssert(m_bInitialised); return m_fLaggedVal;}
//	float	GetLaggedVel()	const	{ntAssert(m_bInitialised); return m_fLaggedVel;}
	
private:
	CSmoother&	operator = (const CSmoother& obCopy);

	void				AllocBuffers();
	void				Initialise(float fNewVal);
	void				AdjustMeanValueSample(float fNewVal, float fTimeChange);
	void				CalcLaggedValues(float fTimeChange);

	const CSmootherDef	m_obDef;
	bool				m_bInitialised;

	float*				m_pfSampleTimes;	// times we have sampled
	float*				m_pfSampleDurs;		// Intervals between samples
	float*				m_pfTargetBuff;		// target values
	float*				m_pfVelocityBuff;	// normalised velocities

	int					m_iLastSample;		// last sample value
	int					m_iCurrentSample;	// current sample value

	float				m_fEmitTimer;		// when do we sample next?
	float				m_fSampleLength;	// current length of the samples

	float				m_fOldTargetMean;	// last mean value
	float				m_fNewTargetMean;	// next mean value
	float				m_fTargetMean;		// lerped mean value

	float				m_fOldVelocityMean;	// last mean velocity
	float				m_fNewVelocityMean;	// next mean velocity
	float				m_fVelocityMean;	// lerped mean velocity
	
	// this functionality is depricated as of (03.03.04) WD
//	float				m_fLaggedVal;		// lagged value
//	float				m_fLaggedVel;		// lagged velocity

	float				m_fTimeSinceInit;	// duration calcs
};

/***************************************************************************************************
*
*	CLASS			CPointSmoother
*
*	DESCRIPTION		Takes a new sample point and stores it in a buffer in its interpolated postion.
*					From this it calculates an average value at the current moment in time
*					and the average velocity.
*					It also calculates a time lagged version of these.
*
*	NOTES			-If used when time is frozen or reversed, the state of the smoother will not be
*					changed.
*					-Update should be called every frame, internally it works out when to emit
*					a new point into the buffer.
*
***************************************************************************************************/
class CPointSmoother
{
public:
	CPointSmoother(const SmootherDef& obDef);
	CPointSmoother(const CPointSmoother& obCopy);
	~CPointSmoother();

	void	Update(const CPoint& obNewTarget, float fTimeChange);
	void	Render() const;
	void	Reset() {m_bInitialised = false;}

	CPoint		GetTargetMean()	const	{ntAssert(m_bInitialised); return m_obTargetMean;}
	CDirection	GetVelocityMean()	const	{ntAssert(m_bInitialised); return m_obVelocityMean;}

	// this functionality is depricated as of (03.03.04) WD
//	CPoint		GetLaggedPos()	const	{ntAssert(m_bInitialised); return m_obLaggedPos;}
//	CDirection	GetLaggedVel()	const	{ntAssert(m_bInitialised); return m_obLaggedVel;}
	
private:
	CPointSmoother&	operator = (const CPointSmoother& obCopy);

	void				AllocBuffers();
	void				Initialise(const CPoint& obNewPos);
	void				AdjustMeanValueSample(const CPoint& obNewPos, float fTimeChange);
	void				CalcLaggedValues(float fTimeChange);

	const CSmootherDef	m_obDef;
	bool				m_bInitialised;

	float*				m_pfSampleTimes;	// times we have sampled
	float*				m_pfSampleDurs;		// Intervals between samples
	CPoint*				m_pobTargetBuff;	// target positions
	CDirection*			m_pobVelocityBuff;	// normalised velocities

	int					m_iLastSample;		// last sample pos
	int					m_iCurrentSample;	// current sample pos

	float				m_fEmitTimer;		// when do we sample next?
	float				m_fSampleLength;	// current length of the samples

	CPoint				m_obOldTargetMean;	// last mean position
	CPoint				m_obNewTargetMean;	// next mean position
	CPoint				m_obTargetMean;		// lerped mean position

	CDirection			m_obOldVelocityMean;// last mean velocity
	CDirection			m_obNewVelocityMean;// next mean velocity
	CDirection			m_obVelocityMean;	// lerped mean velocity

	// this functionality is depricated as of (03.03.04) WD
//	CPoint				m_obLaggedPos;		// lagged position
//	CDirection			m_obLaggedVel;		// lagged velocity

	float				m_fTimeSinceInit;	// duration calcs
};

#endif // SMOOTHER_H
