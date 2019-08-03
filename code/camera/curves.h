/***************************************************************************************************
*
*	$Header:: /game/curves.h 1     11/08/03 16:30 Wil                                              $
*
*
*
*	CHANGES
*
*	11/8/2003	Wil	Created
*
***************************************************************************************************/

#ifndef CUBIC_CURVE_H
#define CUBIC_CURVE_H

#include "curveinterface.h"
#include "curvetemplates.h"
#include "camera/camutils.h"

/***************************************************************************************************
*
*	CLASS			CPosRotCurve
*
*	DESCRIPTION		Curve containing both postition and rotation curves
*
***************************************************************************************************/
class CPosRotCurve : public CCurveInterface
{
public:
	CPosRotCurve( CCurveInterface& obPosCurve, CCurveInterface& obRotCurve, ROT_TYPE eROT ) :
		CCurveInterface( obPosCurve.HasPolynomial(), obPosCurve.GetNumSpan(), obPosCurve.GetOpen(), obPosCurve.GetInvalidLength() ),
		m_eROT( eROT ),
		m_pobPosCurve( &obPosCurve ),
		m_pobRotCurve( &obRotCurve )
	{}


	~CPosRotCurve( void )
	{
		NT_DELETE_CHUNK(Mem::MC_CAMERA, m_pobPosCurve );
		NT_DELETE_CHUNK(Mem::MC_CAMERA, m_pobRotCurve );
	}

	virtual CVector	Evaluate( float fU )	const { return m_pobPosCurve->Evaluate( fU ); }
	virtual CVector	Derivative1( float fU )	const { return m_pobPosCurve->Derivative1( fU ); }
	virtual CVector	Derivative2( float fU )	const { return m_pobPosCurve->Derivative2( fU ); }

	virtual u_int	GetNumCV( void )				const { return m_pobPosCurve->GetNumCV(); }
	virtual const	CVector GetCV( u_int uiIndex )	const { return m_pobPosCurve->GetCV(uiIndex); }

	virtual void	Render( void )		const;
	virtual void	RenderCVs( void )	const { return m_pobPosCurve->RenderCVs(); }
//	virtual const char*	Dump( void )	const;
	
	CMatrix					GetRotation( float fU ) const;

	const CCurveInterface&	GetPosCurve( void ) const { return *m_pobPosCurve; }
	const CCurveInterface&	GetRotCurve( void ) const { return *m_pobRotCurve; }
	ROT_TYPE				GetRotType( void ) const { return m_eROT; }

private:
	virtual void GetCoeffs( int, CVector* ) const { ntAssert(0); }

	ROT_TYPE			m_eROT;
	CCurveInterface*	m_pobPosCurve;
	CCurveInterface*	m_pobRotCurve;
};




/***************************************************************************************************
*
*	CLASS			CNurbsBase
*
*	DESCRIPTION		Class incorporating functionality for nurbs curves.
*					These nurbs can be created from maya exported data, in which case additional knots
*					are added at the start and end of the curve.
*					They are defined by degree (normally cubic but others are supported), an array of
*					control vertexes and an array of knot values.
*
*	NOTES			For 1 span Cubic:
*						degree = 3;
*						order = degree + 1 = 4;
*						numCV = 4;
*						max CV = numCV - 1 = 3;
*						num Knot = max CV + order + 1 = numCV + degree + 1 = 8;
*						num Span = max CV - order + 2 = numCV - degree = 1;
*
***************************************************************************************************/
class CNurbsBase
{
public:
	CNurbsBase( void )
	{
		m_bInvalid = true;
	}

	CNurbsBase( u_int uiDegree, u_int uiNumCV, const CVector* pobVerts, const float* pfKnots, bool bOwnsData = false )
	{
		m_bInvalid = true;
		ConstructCore( uiDegree, uiNumCV, pobVerts, pfKnots, bOwnsData );
	}

	~CNurbsBase( void )
	{
		NT_DELETE_ARRAY_CHUNK(Mem::MC_CAMERA, m_pobData );
		NT_DELETE_ARRAY_CHUNK(Mem::MC_CAMERA, m_pfKnots );
	}

	CVector				Eval( float fU )				const;	
	CVector				Eval1stDerivative( float fU )	const;
	CVector				Eval2ndDerivative( float fU )	const;

	u_int				GetOrder( void )				const { ntAssert( !m_bInvalid ); return m_uiDegree + 1; }
	u_int				GetDegree( void )				const { ntAssert( !m_bInvalid ); return m_uiDegree; }
	u_int				GetNumKnot( void )				const { ntAssert( !m_bInvalid ); return m_uiNumKnot; }
	u_int				GetNumSpans( void )				const { ntAssert( !m_bInvalid ); return m_uiNumSpan; }
	float				GetKnot( u_int uiIndex )		const { ntAssert( !m_bInvalid ); ntAssert(uiIndex<GetNumKnot());		return m_pfKnots[uiIndex]; }
	u_int				GetNumControlV( void )			const { ntAssert( !m_bInvalid ); return m_uiNumCV; } //GetNumSpan()+GetDegree(); }
	const CVector		GetControlV( u_int uiIndex )	const { ntAssert( !m_bInvalid ); ntAssert(uiIndex<GetNumControlV());	return m_pobData[uiIndex]; }

protected:
	void				ConstructCore( u_int uiDegree, u_int uiNumCV, const CVector* pobVerts, const float* pfKnots, bool bOwnsVerts );

	float				CoxDeBoor( float fU, int iKnotIndex, int iOrder )			const;
	float				CoxDeBoor1stDer( float fU, int iKnotIndex, int iOrder )		const;
	float				CoxDeBoor2ndDer( float fU, int iKnotIndex, int iOrder )		const;

	inline float		CDBLeftTerm( float fU, int iKnotIndex, int iOrder )			const;
	inline float		CDBRightTerm( float fU, int iKnotIndex, int iOrder )		const;
	inline float		CDBLeftTermDer( float& fFirstDer, float fU, int iKnotIndex, int iOrder )	const;
	inline float		CDBRightTermDer( float& fFirstDer, float fU, int iKnotIndex, int iOrder )	const;

	u_int				FindSpan( float fU ) const;
	float				GetMinKnot( void )	const { return GetKnot( GetDegree() ); } 
	float				GetMaxKnot( void )	const { return GetKnot( GetNumControlV() ); }
	float				GetRange( void )	const { return GetMaxKnot() - GetMinKnot(); }

private:
	bool				m_bInvalid;
	u_int				m_uiNumSpan;
	u_int				m_uiDegree;
	u_int				m_uiNumCV;		// GetNumSpan()+GetDegree()
	u_int				m_uiNumKnot;
	const CVector*		m_pobData;
	const float*		m_pfKnots;
};

/***************************************************************************************************
*	
*	FUNCTION		CNurbsBase::CDBLeftTerm
*
*	DESCRIPTION		calc left term of cox-de-boor algorithm
*
***************************************************************************************************/
inline	float	CNurbsBase::CDBLeftTerm( float fU, int iKnotIndex, int iOrder ) const
{
	float fDenom = GetKnot(iKnotIndex + iOrder - 1) - GetKnot(iKnotIndex);
	return ((fDenom > 0.0f) ? ((fU - GetKnot(iKnotIndex)) / fDenom) : 0.0f);
};

/***************************************************************************************************
*	
*	FUNCTION		CNurbsBase::CDBRightTerm
*
*	DESCRIPTION		calc right term of cox-de-boor algorithm
*
***************************************************************************************************/
inline	float	CNurbsBase::CDBRightTerm( float fU, int iKnotIndex, int iOrder ) const
{
	float fDenom = GetKnot(iKnotIndex + iOrder) - GetKnot(iKnotIndex + 1);
	return ((fDenom > 0.0f) ? ( ((GetKnot(iKnotIndex + iOrder) - fU)) / fDenom) : 0.0f);
};

/***************************************************************************************************
*	
*	FUNCTION		CNurbsBase::CDBLeftTermDer
*
*	DESCRIPTION		calc left term of cox-de-boor algorithm and its 1st derivative
*
***************************************************************************************************/
inline	float	CNurbsBase::CDBLeftTermDer( float& fFirstDer, float fU, int iKnotIndex, int iOrder ) const
{
	float fDenom = GetKnot(iKnotIndex + iOrder - 1) - GetKnot(iKnotIndex);

	if (fDenom)
	{
		fFirstDer = 1.0f / fDenom;
		return (fFirstDer * (fU - GetKnot(iKnotIndex)));
	}
	else
	{
		fFirstDer = 0.0f;
		return fFirstDer;
	}
};

/***************************************************************************************************
*	
*	FUNCTION		CNurbsBase::CDBRightTermDer
*
*	DESCRIPTION		calc right term of cox-de-boor algorithm and its 1st derivative
*
***************************************************************************************************/
inline	float	CNurbsBase::CDBRightTermDer( float& fFirstDer, float fU, int iKnotIndex, int iOrder ) const
{
	float fDenom = GetKnot(iKnotIndex + iOrder) - GetKnot(iKnotIndex + 1);

	if (fDenom)
	{
		fFirstDer = -1.0f / fDenom;
		return (fFirstDer * -(GetKnot(iKnotIndex + iOrder) - fU));
	}
	else
	{
		fFirstDer = 0.0f;
		return fFirstDer;
	}
};





/***************************************************************************************************
*
*	CLASS			CNurbs
*
*	DESCRIPTION		Derived from a normal CCurveInterface and CNurbsBase
*
***************************************************************************************************/
class CNurbs : public CCurveInterface, public CNurbsBase
{
public:
	CNurbs( bool bOpen, u_int uiDegree, u_int uiNumCV, const CVector* pobVerts, const float* pfKnots, bool bOwnsData = false,  float fLength = -1.0f ) :
		CCurveInterface( false, uiNumCV - uiDegree, bOpen, fLength ),
		CNurbsBase( uiDegree, uiNumCV, pobVerts, pfKnots, bOwnsData )
	{
		ntAssert( GetNumControlV() == GetNumSpan()+GetDegree() );
		ntAssert( GetNumSpans() == GetNumSpan() );
	}


	virtual u_int			GetNumCV( void )		const { return GetNumControlV(); }
	virtual const CVector	GetCV( u_int uiIndex )	const { return GetControlV(uiIndex); }
	virtual const char*		Dump( void )			const;

	virtual CVector			Evaluate( float fU )	const { return Eval(fU); }
	virtual CVector			Derivative1( float fU )	const { return Eval1stDerivative(fU); }
	virtual CVector			Derivative2( float fU )	const { return Eval2ndDerivative(fU); }

private:
	virtual void GetCoeffs( int, CVector* ) const { ntAssert(0); }
};
	




/***************************************************************************************************
*
*	CLASS			CTimeNurbs
*
*	DESCRIPTION		Derived from a CTimeCurveInterface and CNurbsBase
*
***************************************************************************************************/
class CTimeNurbs : public CTimeCurveInterface, public CNurbsBase
{
public:
	CTimeNurbs( float* pfTimes, bool bOwnsTimes, bool bOpen, u_int uiDegree, u_int uiNumCV,
				const CVector* pobVerts, const float* pfKnots, bool bOwnsData = false,  float fLength = -1.0f ) :
		CTimeCurveInterface( pfTimes, bOwnsTimes, false, uiNumCV - uiDegree, bOpen, fLength ),
		CNurbsBase( uiDegree, uiNumCV, pobVerts, pfKnots, bOwnsData )
	{
		ntAssert( GetNumControlV() == GetNumSpan()+GetDegree() );
		ntAssert( GetNumSpans() == GetNumSpan() );
	}


	virtual u_int			GetNumCV( void )		const { return GetNumControlV(); }
	virtual const CVector	GetCV( u_int uiIndex )	const { return GetControlV(uiIndex); }
	virtual const char*		Dump( void )			const;

	virtual CVector			Evaluate( float fU )	const { ReparamatiseTime( fU ); return Eval(fU); }
	virtual CVector			Derivative1( float fU )	const { ReparamatiseTime( fU ); return Eval1stDerivative(fU); }
	virtual CVector			Derivative2( float fU )	const { ReparamatiseTime( fU ); return Eval2ndDerivative(fU); }

private:
	void					ReparamatiseTime( float& fU ) const
	{
		// this is the same conversion as CTimeCurveInterface normally makes
		u_int uiSegment = GetTimeModule().GetSegmentAndTime( fU ) +  GetDegree();

		// get the knot range for this segment
		float fSpanRange = GetKnot(uiSegment+1) - GetKnot(uiSegment);

		// convert back to normalised nurbs units
		fU = ( (fU * fSpanRange) + GetKnot(uiSegment) ) / GetRange();
	}

	virtual void GetCoeffs( int, CVector* ) const { ntAssert(0); }
};


#endif
