/***************************************************************************************************
*
*	$Header:: /game/lookatcontroller.h 2     13/08/03 17:22 Wil                                    $
*
*
*
*	CHANGES
*
*	11/8/2003	Wil	Created
*
***************************************************************************************************/

#ifndef LOOKAT_CONTROLLER_H
#define LOOKAT_CONTROLLER_H

static const float FOCAL_DISTANCE_HACK = 10.0f;

class BasicCamera;
class CPointSmoother;
class PointTransform;
class SmootherDef;
class CPointConverger;
class CConvergerDef;
class CCurveRail;
class Transform;

/***************************************************************************************************
*
*	CLASS			LACDebugDef
*
*	DESCRIPTION		way of making LAC's without file defs
*
***************************************************************************************************/
struct LACDebugDef
{
	LACDebugDef() :
		m_pobPOISmooth(0),
		m_pobPOITrans(0),
		m_bPOIFixedOffset(false),
		m_obTargetOffset(CONSTRUCT_CLEAR),
		m_bBound(false),
		m_obBoundMax(CONSTRUCT_CLEAR),
		m_obBoundMin(CONSTRUCT_CLEAR)
	{
	}

	CPointSmoother*		m_pobPOISmooth;
	PointTransform*		m_pobPOITrans;
	bool				m_bPOIFixedOffset;
	CDirection			m_obTargetOffset;
	bool				m_bBound;
	CPoint				m_obBoundMax;
	CPoint				m_obBoundMin;
};

/***************************************************************************************************
*
*	CLASS			LookAtController
*
*	DESCRIPTION		Generates a camera orientation based on the input POI
*
***************************************************************************************************/
class LookAtControllerDef
{
public:
	LookAtControllerDef();
	virtual ~LookAtControllerDef(){};

	PointTransform*		m_pobPOITrans;
	// scee.sbashow: for offsets applied to LAC, use these to 
	// 					converge smoothen these out when these offsets abruptly change,
	//					like when they go back to zero
	CConvergerDef* 		m_pobOffsetConvergerDef;
	SmootherDef*		m_pobPOISmootherDef;
	bool				m_bPOIFixedOffset;

	CDirection			m_obTargetOffset;
	CPoint				m_obBoundMax;
	CPoint				m_obBoundMin;

	bool				m_bBound;

	virtual class LookAtController* Create(const BasicCamera* pParent) = 0;
};

class LookAtController
{
public:
	LookAtController(const BasicCamera* pParent, const LookAtControllerDef& def);
	virtual ~LookAtController();

	virtual void PostConstruct() {Reset();}

	// The entire public interface must be virtual to allow an editing wrapper with the same interface
	virtual CMatrix		Update(const CPoint& obCurrPos, float fTimeChange) = 0;
	virtual void		Render();
	virtual void		RenderInfo(int iX, int iY);
	virtual void		Reset(bool bRebuildSmoothers = false);

	virtual CPoint		GetLastTracked() const	                {ntAssert(m_bLastTrackedValid);   return m_obLastTracked;}
	virtual CMatrix		GetLastTransform() const                {ntAssert(m_bLastTransformValid); return m_obLastTransform;}

protected:
	void				CalcLastTracked(float fTimeChange);
	CMatrix				ModifyLookat(const CMatrix& obMat);

	PointTransform*		GetPOITrans() {return m_pobPOITrans;}

	void				SetLastTracked(const CPoint& obTrack)		{m_bLastTrackedValid = true; m_obLastTracked = obTrack;}
	void				SetLastTransform(const CMatrix& obTrans)	{m_bLastTransformValid = true; m_obLastTransform = obTrans;}
	void				SetTarget(const CPoint& obTarget)			{m_bTargetValid = true; m_obTarget = obTarget;}

protected:
	const BasicCamera*     m_pParent;          // Remember where we came from.

	bool                   m_bLastTrackedValid;
	CPoint                 m_obLastTracked;

	bool                   m_bTargetValid;
	CPoint                 m_obTarget;

	bool                   m_bLastTransformValid;
	CMatrix                m_obLastTransform;

	CPointSmoother*        m_pobPOISmooth;
	PointTransform*        m_pobPOITrans;
	
	bool                   m_bPOIFixedOffset;
	CDirection             m_obTargetOffset;

	bool                   m_bBound;
	CPoint                 m_obBoundMax;
	CPoint                 m_obBoundMin;

	CPointConverger*		m_pobOffsetConverger;

	friend class DebugChaseCamera;
};

/***************************************************************************************************
*
*	CLASS			LACFixedPos
*
*	DESCRIPTION		Simplest LAC, looks at a fixed pos only
*
***************************************************************************************************/
class LACFixedPosDef : public LookAtControllerDef
{
public:
	CPoint	m_obPosition;

	virtual class LookAtController* Create(const BasicCamera* pParent);
};

class LACFixedPos : public LookAtController
{
public:
	LACFixedPos(const LACFixedPosDef& def, const BasicCamera* pParent);

	virtual CMatrix			Update(const CPoint& obCurrPos, float fTimeChange);
	virtual void			Render();

private:
	LACFixedPosDef m_def;
};

/***************************************************************************************************
*
*	CLASS			LACFixedDir
*
*	DESCRIPTION		Simple LAC, looks in a fixed direction only
*
***************************************************************************************************/
class LACFixedDirDef : public LookAtControllerDef
{
public:
	CDirection m_obDirection;

	virtual class LookAtController* Create(const BasicCamera* pParent);
};

class LACFixedDir : public LookAtController
{
public:
	LACFixedDir(const LACFixedDirDef& def, const BasicCamera* pParent);

	virtual CMatrix Update(const CPoint& obCurrPos, float fTimeChange);
	virtual void    Render() {};

private:
	const LACFixedDirDef& m_def;
};

/***************************************************************************************************
*
*	CLASS			LACFixedYPR
*
*	DESCRIPTION		Simple LAC, oriented via a matrix defined by yaw, pitch and roll
*
***************************************************************************************************/
class LACFixedYPRDef : public LookAtControllerDef
{
public:
	CDirection m_obYawPitchRoll;

	virtual class LookAtController* Create(const BasicCamera* pParent);
};

class	LACFixedYPR : public LookAtController
{
public:
	LACFixedYPR(const LACFixedYPRDef& def, const BasicCamera* pParent);

	virtual CMatrix			Update(const CPoint& obCurrPos, float fTimeChange);
	virtual void			Render() {};

private:
	const LACFixedYPRDef& m_def;
};

/***************************************************************************************************
*
*	CLASS			LACPOIRel
*
*	DESCRIPTION		looks at the point of interest
*
***************************************************************************************************/
class LACPOIRelDef : public LookAtControllerDef
{
public:
	// Nothing here...

	virtual class LookAtController* Create(const BasicCamera* pParent);
};

class LACPOIRel : public LookAtController
{
public:
	LACPOIRel(const BasicCamera* pParent, const LACPOIRelDef& def);

	virtual CMatrix Update(const CPoint& obCurrPos, float fTimeChange);

};

/***************************************************************************************************
*
*	CLASS			LACRotRail
*
*	DESCRIPTION		Gets the rotation from a closest point rail
*
***************************************************************************************************/
class LACRotRailDef : public LookAtControllerDef
{
public:
	CCurveRail*			m_pobRail;
	class CPosRotCurve* m_pobRotCurve;

	virtual class LookAtController* Create(const BasicCamera* pParent);
};

class LACRotRail : public LookAtController
{
public:
	LACRotRail(const LACRotRailDef& def, const BasicCamera* pParent);
	virtual ~LACRotRail();

	virtual CMatrix			Update(const CPoint& obCurrPos, float fTimeChange);
	virtual void			Render();
	virtual void			Reset();

private:
	const LACRotRailDef& m_def;
	float				 m_fCurrPos;
};

/***************************************************************************************************
*
*	CLASS			LACRotGuide
*
*	DESCRIPTION		Gets the rotation from the point transform guide mapped to our rail
*
***************************************************************************************************/
class LACRotGuideDef : public LookAtControllerDef
{
public:
	CCurveRail*			m_pobRail;
	class CPosRotCurve* m_pobRotCurve;

	virtual class LookAtController* Create(const BasicCamera* pParent);
};

class LACRotGuide : public LookAtController
{
public:
	LACRotGuide(const LACRotGuideDef& def, const BasicCamera* pParent);
	virtual ~LACRotGuide();

	virtual CMatrix			Update(const CPoint& obCurrPos, float fTimeChange);
	virtual void			Render();
	virtual void			Reset();

private:
	const LACRotGuideDef& m_def;
	float				  m_fCurrPos;
};

/***************************************************************************************************
*
*	CLASS			LACGuideRail
*
*	DESCRIPTION		Gets the rotation from the PointTransform, maps this to our rail and looks at
*					the rail...
*
***************************************************************************************************/
class LACGuideRailDef : public LookAtControllerDef
{
public:
	CCurveRail*			m_pobRail;

	virtual class LookAtController* Create(const BasicCamera* pParent);
};

class LACGuideRail : public LookAtController
{
public:
	LACGuideRail(const LACGuideRailDef& def, const BasicCamera* pParent);
	virtual ~LACGuideRail();

	virtual CMatrix			Update(const CPoint& obCurrPos, float fTimeChange);
	virtual void			Render();
	virtual void			Reset();
	
private:
	const LACGuideRailDef& m_def;
	float				   m_fCurrPos;
};


/***************************************************************************************************
*
*	CLASS			LACRail
*
*	DESCRIPTION		Looks at the closest point rail
*
***************************************************************************************************/
class LACRailDef : public LookAtControllerDef
{
public:
	CCurveRail*			m_pobRail;

	virtual class LookAtController* Create(const BasicCamera* pParent);
};

class LACRail : public LookAtController
{
public:
	LACRail(const LACRailDef& def, const BasicCamera* pParent);
	virtual ~LACRail();

	virtual CMatrix			Update(const CPoint& obCurrPos, float fTimeChange);
	virtual void			Render();
	virtual void			Reset();

private:
	const LACRailDef& m_def;
};

#endif // LOOKAT_CONTROLLER_H
