/***************************************************************************************************
*
*	$Header:: /game/curverail.h 1     11/08/03 16:30 Wil                                           $
*
*
*
*	CHANGES
*
*	11/8/2003	Wil	Created
*
***************************************************************************************************/

#ifndef CURVERAIL_H
#define CURVERAIL_H

// External declarations
class CCurveInterface;
class CCurveEditor;

// For inherited classes

/***************************************************************************************************
*
*	CLASS			CurveRailDef
*
*	DESCRIPTION		Curve rail properties
*
***************************************************************************************************/
class CurveRailDef
{
public:
	HAS_INTERFACE(CurveRailDef)

	#define CR_SPEED	10.0f
	#define CR_SPRING	5.0f
	#define CR_DAMP		0.5f
	#define CR_DIST		2.0f

	// Welder constructor
	CurveRailDef();

	void	SetXZOnly(bool bXZOnly)		{m_bXZOnly = bXZOnly;}
	void	SetNearest(bool bNearest)		{m_bNearest = bNearest;}
	void	SetAvoidance(float fAvoidance){m_fAvoidance = fAvoidance;}

	bool		GetXZOnly()		const	{return m_bXZOnly;}
	bool		GetNearest()	const	{return m_bNearest;}
	float		GetAvoidance()	const	{return m_fAvoidance;}
	const CCurveEditor*	GetCurve() const {return m_pobCurve;}

	float		GetSpeed()		const	{return m_fSpeed;}
	float		GetSpring()		const	{return m_fSpring;}
	float		GetDamp()		const	{return m_fDamp;}

private:
	friend class CurveRailDefI;
	CCurveEditor* m_pobCurve;
	float	m_fSpeed;
	float	m_fSpring;
	float	m_fDamp;
	bool	m_bXZOnly;
	bool	m_bNearest;
	float	m_fAvoidance;
};

/***************************************************************************************************
*
*	CLASS			CCurveRail
*
*	DESCRIPTION		curve based positioning unit
*
***************************************************************************************************/
class CCurveRail
{
public:
	CCurveRail(const CurveRailDef& obDef);
	~CCurveRail();

	void	TrackPoint(const CPoint& obTarget, float fTimeChange);
	void	TrackTarget(float fTargetVal, const CPoint& obAvoidPos, float fTimeChange);

	void	Render();
	void	Reset() {m_bInitialised = false;}
	
	CPoint	GetFocusPoint()		const	{ntAssert(m_bInitialised); return m_obFocusPos;}
	CPoint	GetTargetPoint()	const	{ntAssert(m_bInitialised); return m_obTargetPos;}
	CPoint	GetConvergePoint()	const	{ntAssert(m_bInitialised); return m_obConvergePos;}
	CPoint	GetDampedPoint()	const	{ntAssert(m_bInitialised); return m_obDampedPos;}

	float	GetTargetDistSq()	const	{ntAssert(m_bInitialised); return m_fTargetDistSq;}
	float	GetConvergeDistSq() const	{ntAssert(m_bInitialised); return m_fConvergeDistSq;}
	float	GetDampedDistSq()	const	{ntAssert(m_bInitialised); return m_fDampedDistSq;}

	float	GetTargetVal()		const	{ntAssert(m_bInitialised); return m_fTargetVal;}
	float	GetConvergeVal()	const	{ntAssert(m_bInitialised); return m_fConvergeVal;}
	float	GetDampedVal()		const	{ntAssert(m_bInitialised); return m_fDampedVal;}

	const CurveRailDef& GetDef() const  {return m_obDef;}

private:
	void	Initialise(const CPoint& obTarget);
	void	Initialise(float fTargetVal, const CPoint& obAvoidPos);
	void	ConstructCore();

	CCurveRail&	operator = (const CCurveRail& obCopy);

	void	AdjustForProximity(float fMin);
	float	GetDistSqToFocusPoint(const CPoint& obPos)
	{
		CDirection obTemp = m_obFocusPos ^ obPos;
		if (m_obDef.GetXZOnly()) obTemp.Y() = 0.0f;
		return obTemp.LengthSquared();
	}

	const CurveRailDef		m_obDef;
	bool					m_bInitialised;

	const CCurveEditor*		m_pobCurve;
	CPoint					m_obFocusPos;		// position we care about in the world

	CPoint					m_obTargetPos;		// closest point to focus
	float					m_fTargetVal;		// spline val of closeset point to focus
	float					m_fTargetDistSq;	// distance this point is to the focus
	
	CPoint					m_obConvergePos;	// position on spline moving towards focus
	float					m_fConvergeVal;		// spline val of converging position
	float					m_fConvergeDistSq;	// distance this point is to the focus

	CPoint					m_obDampedPos;		// dampened convergence position
	float					m_fDampedVal;		// dampened convergence value
	float					m_fDampedValVel;	// dampened convergence value velocity
	float					m_fDampedDistSq;	// distance this point is to the focus
};

#endif // CURVERAIL_H
