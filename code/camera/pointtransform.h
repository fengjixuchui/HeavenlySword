//------------------------------------------------------------------------------------------
//!                                                                                         
//!	\file pointtransform.h                                                                  
//!                                                                                         
//------------------------------------------------------------------------------------------

#ifndef POINT_TRANS_H
#define POINT_TRANS_H

#ifndef _CURVEEDITOR_H
#include "camera/curveeditor.h"
#endif

//------------------------------------------------------------------------------------------
// External Declarations                                                                    
//------------------------------------------------------------------------------------------
class CamVolume;
class PointTransform;
class CamSceneElementMan;


/***************************************************************************************************
*
*	CLASS			PointTransform
*
*	DESCRIPTION		Generates a position based on the input point
*
***************************************************************************************************/
class PointTransformDef
{
public:
	virtual ~PointTransformDef(){};
	virtual PointTransform* Create() = 0;
};


class PointTransform
{
public:
	enum PT_TYPE
	{
		GUIDE_CURVE			= 0,
		RANGE_CLAMP			= 1,
		VOLUME_CLAMP		= 2,

		PT_TYPE_MAX,
	};

	PointTransform() {Reset();}
	virtual ~PointTransform(){};
	
	virtual		CPoint	Update(const CPoint& obTracking, const CamSceneElementMan& obCamSceneElem, float fTimeStep) = 0;
	virtual		void	Render() = 0;

	// The entire public interface needs to be virtual so we can wrap this with a pc editor thingy
	virtual CPoint		GetLastTracked()		const	{ntAssert(m_bTrackInit); return m_obLastTracked;}
	virtual CPoint		GetLastTransform()	const	{ntAssert(m_bTransInit); return m_obLastTransform;}

	virtual PT_TYPE		GetTransformType()	const	{ntAssert(0); return PT_TYPE_MAX;}
	
	virtual void Reset()
	{
		m_bTrackInit = false;
		m_bTransInit = false;
	}

protected:
	const char* GetTypeString()
	{
		switch(GetTransformType())
		{
		case GUIDE_CURVE:		return "GUIDE_CURVE";
		case RANGE_CLAMP:		return "RANGE_CLAMP";
		case VOLUME_CLAMP:		return "VOLUME_CLAMP";
		case PT_TYPE_MAX:		return "PT_TYPE_MAX";
		}
	};

	void		SetLastTracked(const CPoint& obTrack)		{m_obLastTracked = obTrack; m_bTrackInit = true;}
	void		SetLastTransform(const CPoint& obTrans)	{m_obLastTransform = obTrans; m_bTransInit = true;}
	bool		TrackingValid() const					{return (m_bTrackInit && m_bTransInit);}

private:
	bool		m_bTrackInit;
	bool		m_bTransInit;
	CPoint		m_obLastTracked;
	CPoint		m_obLastTransform;
};

/***************************************************************************************************
*
*	CLASS			PTGuide
*
*	DESCRIPTION		Generates a position based on the input point via a guide curve
*
***************************************************************************************************/
class PTGuideDef : public PointTransformDef
{
public:
	PTGuideDef() {m_pobGuideCurve = 0;}
	virtual PointTransform* Create();

	const CCurveEditor* m_pobGuideCurve;
};


class PTGuide : public PointTransform
{
public:
	PTGuide(const PTGuideDef& def);

	virtual void Reset()
	{
		PointTransform::Reset();
		m_bGuideInit = false;
	}

	
	virtual		CPoint	Update(const CPoint& obTracking, const CamSceneElementMan& obCamSceneElem, float fTimeStep);
	virtual		void	Render();

	float					GetGuideVal()	{ntAssert(m_bGuideInit); return m_fTargetVal;}
	const CCurveInterface*	GetCurve()		{ntAssert(m_bGuideInit); return static_cast<const CCurveInterface*>(m_obDef.m_pobGuideCurve);}

	virtual PT_TYPE		GetTransformType()	const	{return GUIDE_CURVE;}

private:
	bool					m_bGuideInit;
	u_int					m_uiGuideSubdivs;
	float					m_fTargetVal;
	const PTGuideDef&		m_obDef;
};

/***************************************************************************************************
*
*	CLASS			PTRange
*
*	DESCRIPTION		Generates a position based on the input point via range clamping
*
***************************************************************************************************/
class PTRangeDef : public PointTransformDef
{
public:
	CPoint m_obClampMin;
	CPoint m_obClampMax;

	virtual PointTransform* Create();
};

class PTRange : public PointTransform
{
public:
	PTRange(const PTRangeDef& def);

	virtual		CPoint	Update(const CPoint& obTracking, const CamSceneElementMan& obCamSceneElem, float fTimeStep);
	virtual		void	Render();

	virtual PT_TYPE		GetTransformType()	const	{return RANGE_CLAMP;}

private:
	CPoint		m_obClampMin;
	CPoint		m_obClampMax;
};

/***************************************************************************************************
*
*	CLASS			PTVolume
*
*	DESCRIPTION		Generates a position based on the input point via volume surface projection
*
***************************************************************************************************/
class PTVolumeDef : public PointTransformDef
{
public:
	const	CamVolume*	m_pobVolume;
	float	m_fConvergeSpeed;
	float 	m_fConvergeSpring;
	float	m_fConvergeDamp;
	virtual PointTransform* Create();
};

class CPointConverger;

class PTVolume : public PointTransform
{
public:
	PTVolume(const PTVolumeDef& def);
	~PTVolume();

	virtual		CPoint	Update(const CPoint& obTracking, const CamSceneElementMan& obCamSceneElem, float fTimeStep);
	virtual		void	Render();

	virtual PT_TYPE		GetTransformType()	const	{return VOLUME_CLAMP;}

	virtual void Reset();

private:
	CPointConverger*	m_pobPointConverger;
	const CamVolume*	m_pobLastVolumeClamped;
	const CamVolume*	m_pobNewVolumeClamped;
	int					m_iLastSurface;
	int					m_iNewSurface;

	bool				m_bToLastKnown;
	bool				m_bPointConvergerInUse;
	CPoint				m_obLastConvergedPoint;
	CPoint				m_obLastKnownPoint;

	const	CamVolume*	m_pobVolume;
};

#endif // POINT_TRANS_H
