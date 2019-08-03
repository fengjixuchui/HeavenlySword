/***************************************************************************************************
*
*	$Header:: /game/converger.h 1     11/08/03 16:30 Wil                                           $
*
*
*
*	CHANGES
*
*	11/8/2003	Wil	Created
*
***************************************************************************************************/

#ifndef CONVERGER_H
#define CONVERGER_H

//#include "core/timer.h"
#include "camera/camutils.h"

class ConvergerDef
{
public:
	HAS_INTERFACE(ConvergerDef)
	ConvergerDef();
	virtual ~ConvergerDef(){};

	virtual void PostConstruct();

	float		GetSpeed()		const {return m_fSpeedUp;}
	float		GetSpeedUp()	const {return m_fSpeedUp;}
	float		GetSpeedDown()	const {return m_fSpeedDown;}
	float		GetSpring()		const {return m_fSpring;}
	float		GetDamp()		const {return m_fDamp;}
	
	bool		UseDamp()		const {return m_bUseDamp;}

protected:
	friend class ConvergerDefI;
	bool		m_bUseDamp;
	float		m_fDamp;
	float		m_fSpeedUp;
	float		m_fSpeedDown;
	float		m_fSpring;
};

/***************************************************************************************************
*
*	CLASS			CConvergerDef
*
*	DESCRIPTION		Defines parameters of the converger
*
***************************************************************************************************/
class CConvergerDef : public ConvergerDef
{
public:
	#define CD_MIN_SPEED	0.0f
	#define CD_MIN_SPRING	0.0001f
	#define CD_MIN_DAMP		0.0001f
	#define CD_MAX_DAMP		1.0f
	
	CConvergerDef(const char*) {;}

	void	SetSpeed( float fSpeed )
	{
		ntAssert( fSpeed > CD_MIN_SPEED );
		m_fSpeedUp = m_fSpeedDown = fSpeed;
	}

	void	SetSpeed( float fSpeedUp, float fSpeedDown )
	{
		ntAssert( fSpeedUp > CD_MIN_SPEED && fSpeedDown > CD_MIN_SPEED );
		m_fSpeedUp = fSpeedUp;
		m_fSpeedDown = fSpeedDown;
	}

	void	SetSpring( float fSpring )
	{
		if (fSpring < CD_MIN_SPRING)
			m_bUseDamp = false;
		m_fSpring = fSpring;
	}

	void	SetDamp( float fDamp )
	{
		if ((fDamp > CD_MAX_DAMP) || (fDamp < CD_MIN_DAMP))
			m_bUseDamp = false;

		m_fDamp = fDamp;
	}
};

/***************************************************************************************************
*
*	CLASS			CConverger
*
*	DESCRIPTION		Handy convergence class that converges a variable to a discontinous target with
*					given speed, with additional spring dampening.
*
*	NOTES			-Can wrap around the variable donain, in which case a min and max must be supplied
*					-if wrap is not required, min and max are ignored, so you should check your ranges.
*
***************************************************************************************************/
class CConverger
{
public:
	CConverger(const ConvergerDef& def, bool bWrap = false, float fMin = 0.0f, float fMax = 0.0f) :
		m_obDef(def),
		m_bWrap(bWrap),
		m_fMin(fMin),
		m_fMax(fMax)
	{
		ConstructCore();
	}

	
	void	ConstructCore( void )
	{
		m_bInitialised = false;
		m_fSpringVel = 0.0f;
		if (m_bWrap)
		{
			ntAssert( fabsf(m_fMax - m_fMin) > EPSILON );
		}
	}

	
	void	Reset( void ) { m_bInitialised = false; }

	float	Update( float fTarget, float fTimeChange )
	{
		if (m_bInitialised)
		{
			float fSpeed = (fTarget > m_fDampedVal ? m_obDef.GetSpeedUp() : m_obDef.GetSpeedDown()) * fTimeChange;

			m_fLastTarget = fTarget;
			m_fConvergeVal = CCamUtil::ConvergeByFixedDegree( m_bWrap, m_fConvergeVal, m_fLastTarget, fSpeed, m_fMin, m_fMax );
			
			if (m_obDef.UseDamp())
			{
//				static float fLastVal = fTarget;
				m_fDampedVal = CCamUtil::ConvergeBySpringDamp( m_bWrap, m_fDampedVal, m_fConvergeVal, m_obDef.GetSpring(), m_obDef.GetDamp(), m_fSpringVel, m_fMin, m_fMax, fTimeChange );
			}
			else
				m_fDampedVal = m_fConvergeVal;

			return m_fDampedVal;
		}
		else
		{
			return Initialise( fTarget );
		}
	}

	void	SetRange( float fMin, float fMax )	{ ntAssert(m_bWrap); ntAssert( fabsf(m_fMax - m_fMin) > EPSILON ); m_fMin = fMin; m_fMax = fMax; }
	float	GetTarget( void )	 const { ntAssert(m_bInitialised); return m_fLastTarget; }
	float	GetConverged( void ) const { ntAssert(m_bInitialised); return m_fConvergeVal; }
	float	GetDamped( void )	 const { ntAssert(m_bInitialised); return m_fDampedVal; }

	bool	GetInitialised( void ) const { return m_bInitialised; }

	float	Initialise( float fTarget )
	{
		ntAssert(!m_bInitialised);
		m_bInitialised = true;
		m_fSpringVel = 0.0f;
		m_fLastTarget = m_fConvergeVal = m_fDampedVal = fTarget;
		return m_fDampedVal;
	}

private:
	CConverger&	operator = (const CConverger& obCopy);

	const	ConvergerDef m_obDef;
	bool	m_bInitialised;
	bool	m_bWrap;
	float	m_fSpringVel;
	float	m_fMin;
	float	m_fMax;
	float	m_fLastTarget;
	float	m_fConvergeVal;
	float	m_fDampedVal;
};

/***************************************************************************************************
*
*	CLASS			CPointConverger
*
*	DESCRIPTION		Handy convergence class that converges a point to a discontinous target with
*					given speed, with additional spring dampening.
*
***************************************************************************************************/
class CPointConverger
{
public:
	CPointConverger( const CConvergerDef& obDef ) :
		m_obDef( obDef ),
		m_bInitialised( false ) {}

	CPointConverger(const ConvergerDef& def) :
		m_obDef(def),
		m_bInitialised(false) {}

	void	Reset( void ) { m_bInitialised = false; }

	CPoint	Update( const CPoint& obTarget, float fTimeChange );
	CPoint	GetTarget( void )	 const { ntAssert(m_bInitialised); return m_obLastTarget; }
	CPoint	GetConverged( void ) const { ntAssert(m_bInitialised); return m_obConvergeVal; }
	CPoint	GetDamped( void )	 const { ntAssert(m_bInitialised); return m_obDampedVal; }
	const ConvergerDef& GetDef( void ) const	{	return m_obDef;		}
	bool	GetInitialised( void ) const { return m_bInitialised; }

	CPoint	Initialise( const CPoint& obTarget )
	{
		ntAssert(!m_bInitialised);
		m_bInitialised = true;
		m_obSpringVel.Clear();
		m_obLastTarget = m_obConvergeVal = m_obDampedVal = obTarget;
		return m_obDampedVal;
	}

private:
	CPointConverger&	operator = (const CPointConverger& obCopy);

	const		ConvergerDef m_obDef;
	bool		m_bInitialised;
	CDirection	m_obSpringVel;
	CPoint		m_obLastTarget;
	CPoint		m_obConvergeVal;
	CPoint		m_obDampedVal;
};

#endif // CONVERGER_H
