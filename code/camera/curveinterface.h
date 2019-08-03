/***************************************************************************************************
*
*	$Header:: /game/curveinterface.h 1     11/08/03 16:30 Wil                                      $
*
*
*
*	CHANGES
*
*	11/8/2003	Wil	Created
*
***************************************************************************************************/

#ifndef CURVE_INTERFACE_H
#define CURVE_INTERFACE_H


// base curve type classes
class CCurveInterface;
class CTimeCurveInterface;

// To make the code more descriptive
#define CUBIC_COEFFICIENTS ( 4 )

// forward declared template classes
template<class CurveType, class NameTrait> class CTemplateBezier;
template<class CurveType, class NameTrait> class CTemplateHermite;
template<class CurveType, class NameTrait> class CTemplateCatmull;
template<class CurveType, class NameTrait> class CTemplateUBS;

// these names are nessecary to get the right typename into the template for file IO
struct SBezierName		{ static const char* gpcName; };
struct SHermiteName		{ static const char* gpcName; };
struct SCatmullName		{ static const char* gpcName; };
struct SUBSName			{ static const char* gpcName; };

struct STimeBezierName	{ static const char* gpcName; };
struct STimeHermiteName { static const char* gpcName; };
struct STimeCatmullName { static const char* gpcName; };
struct STimeUBSName		{ static const char* gpcName; };

// these are all the normal / time based variants we need.
typedef	CTemplateBezier<CCurveInterface,SBezierName>					CCubicBezier;
typedef	CTemplateHermite<CCurveInterface,SHermiteName>					CCubicHermite;
typedef	CTemplateCatmull<CCurveInterface,SCatmullName>					CCubicCatmull;
typedef	CTemplateUBS<CCurveInterface,SUBSName>							CCubicUBS;

typedef	CTemplateBezier<CTimeCurveInterface,STimeBezierName>			CCubicTimeBezier;
typedef	CTemplateHermite<CTimeCurveInterface,STimeHermiteName>			CCubicTimeHermite;
typedef	CTemplateCatmull<CTimeCurveInterface,STimeCatmullName>			CCubicTimeCatmull;
typedef	CTemplateUBS<CTimeCurveInterface,STimeUBSName>					CCubicTimeUBS;

// other misc curves we have defined
class CNurbs;
class CTimeNurbs;
class CPosRotCurve;

/***************************************************************************************************
*
*	CLASS			CCurveTimeModule
*
*	DESCRIPTION		Controls how times are mapped to CTimeCurveInterface.
*					Takes a normalised time, works out the place within 'real' curve time
*
*	NOTES			Has NumSegments + 1 times as input
*
***************************************************************************************************/
class	CCurveTimeModule
{
public:
	CCurveTimeModule( u_int uiTimes, const float* pfTimes, bool bOwnsData = false );
	~CCurveTimeModule( void ) { NT_DELETE_ARRAY_CHUNK(Mem::MC_CAMERA, m_pfTimes ); }

	u_int	GetSegmentAndTime( float& fU )	const;
	u_int	GetNumTimes( void )				const	{ return m_uiNumTimes; }
	float	GetTime( u_int uiTime )			const	{ ntAssert(uiTime < m_uiNumTimes); return m_pfTimes[uiTime]; }		
	float	GetStartTime( void )			const	{ return GetTime(0); }
	float	GetEndTime( void )				const	{ return GetTime(GetNumTimes()-1); }
	float	GetRange( void )				const	{ return GetEndTime() - GetStartTime() ; }
	float	GetDuration( u_int uiSeg )		const	{ return GetTime(uiSeg+1) - GetTime(uiSeg); }

private:
	u_int			m_uiNumTimes;
	const float*	m_pfTimes;
};





/***************************************************************************************************
*
*	CLASS			CCurveInterface
*
*	DESCRIPTION		Abstract Interface to curve objects.
*
*	NOTES			Publicly:
*						Defines evaluation interfaces to all curves.
*						Provides basic info for all curves (length, open, spans, control verts).
*						Provides conversion functions for most curves (eg bezier from hermite)
*						Provides utility querys like get closest point, converge on target value etc.
*						Provides some debug render methods
*						Provides a static loader method for instantiating all curves from a file
*
*					Protectedly:
*						Provides two constructors, one for loading from a file, the other for debug
*						Provides some file IO functions for member vars
*						Provides some internal member var accesors to aid debugging and conversion
*
***************************************************************************************************/
class CCurveInterface : public CNonCopyable
{
public:
	friend class CTimeCurveInterface;			// this is where we could do with sideways inheritance...
	friend class CPosRotCurve;				// likewise as above

	virtual ~CCurveInterface( void ) {};
	
	// evaluation functions
	virtual CVector	Evaluate( float fU )	const;
	virtual CVector	Derivative1( float fU )	const;
	virtual CVector	Derivative2( float fU )	const;
	
	virtual CPoint		GetPoint( float fU )		 const { return CPoint( Evaluate(fU) ); }	
	virtual CDirection	Get1stDerivative( float fU ) const { return CDirection( Derivative1(fU) ); }	
	virtual CDirection	Get2ndDerivative( float fU ) const { return CDirection( Derivative2(fU) ); }	

	// query curve properties - these are virtual for CCurveEditor since it will hold no useful data itself
	virtual float		GetLength( void )	const { if (!HasValidLength()) CalcLength(); return GetInvalidLength(); }
	virtual bool		GetOpen( void )		const { return m_bOpen; }
	virtual u_int		GetNumSpan( void )	const { return m_uiNumSpan; }
		
	virtual u_int	GetNumCV( void)		const = 0;
	virtual const CVector GetCV( u_int uiIndex ) const = 0;

	// conversion functions
	CCubicBezier*	CreateBezier( void )	const;
	CCubicHermite*	CreateHermite( void )	const;
	CCubicCatmull*	CreateCatmull( void )	const;
	CCubicUBS*		CreateUBS( void )		const;
	CNurbs*			CreateNurbs( void )		const	{ ntAssert(0); return NULL; };
	
	// position relative queries
	float	GetDistSq( float fU, const CPoint& obPos )	const { return (GetPoint(fU) - obPos).LengthSquared(); }
	float	GetDistSqXZ( float fU, const CPoint& obPos ) const
	{
		CPoint obTemp = GetPoint(fU) - obPos; obTemp.Y() = 0.0f;
		return obTemp.LengthSquared();
	}
	float	GetNearFar( const CPoint& obPoint,
						float& fDistanceSq,
						bool bXZOnly = false,
						bool bNear = true,
						float fDistSqTolerance = 0.0f,
						float fTimeTolerance = 0.001f ) const;
	float	GetNearestLinear(	const CPoint& obPoint,
								float& fDistanceSq,
								u_int iCurveSubDivs = 2,
								u_int iSectionSubDivs = 10,
								float fDistSqTolerance = 0.0f ) const;
	float	GetNearestAccurate( const CPoint& obPoint,
								float& fDistanceSq,
								u_int iSubDivs = 2,
								float fDistTolerance = EPSILON,
								float fCosTolerance = 0.001f ) const;
	float	GetNearestNewton(	float fStart,
								const CPoint& obPoint,
								float& fDistanceSq,
								float fDistTolerance = EPSILON,
								float fCosTolerance = 0.001f ) const;
	
	// target value calculations
	float	ConvergeOnTargetByFixed(	float fCurrVal,
										float fTargetVal,
										float fIncrement,
										bool bXZOnly = false,
										float fTolerance = 0.001f ) const;
	float	ConvergeOnTargetAvoiding(	float fCurrVal,
										float fTargetVal,
										float fOffset,
										CPoint& obAvoid,
										bool bXZOnly = false,
										float fTolerance = 0.001f ) const;

	// debug functions
	virtual void		Render( void )				const;
	virtual void		RenderPath( void )			const;
	virtual void		RenderTans( void )			const;
	virtual void		RenderCVs( void )			const;

	// We need to know whether an item is really serialised
	virtual bool IsSerialised( void ) const { return true; }

	// Provide a static method to destroy these items based on whether they are serialised
	static void Destroy( CCurveInterface* pobItem )
	{
		// Check that all is well
		ntAssert( pobItem );
		
		// Delete the item if it was built by the serialiser
		if ( pobItem->IsSerialised() )
		{
			NT_DELETE_CHUNK(Mem::MC_CAMERA, pobItem );
		}
	}

protected:
	// this is supposed to be the usual one, used in conjunction with a load from file
	CCurveInterface( bool bHasPolynomial ) :
		m_bInvalid( true ),
		m_bHasPolynomial( bHasPolynomial )
	{}

	// this is a debug interface so we can hand construct our curves
	CCurveInterface( bool bHasPolynomial, u_int uiNumSpan, bool bOpen, float fLength = -1.0f ) :
		m_uiNumSpan( uiNumSpan ),	
		m_bInvalid( false ),
		m_bHasPolynomial( bHasPolynomial ),
		m_bOpen( bOpen ),
		m_fLength( fLength )
	{ ntAssert(m_uiNumSpan); }


	// curve property queries
	bool			HasPolynomial( void )		const { return m_bHasPolynomial; }
	bool			HasValidLength( void )		const { return (m_fLength >= 0.0f); }
	float			GetInvalidLength( void)		const { return m_fLength; }
	float			GetSampleInterval( void )	const { return (0.05f / GetNumSpan()); }

private:
	void			CalcLength( void ) const;
	inline u_int	GetSegmentAndTime( float& fU ) const;	
	virtual void	GetCoeffs( int iSegment, CVector* pobCoeffs ) const = 0;

	float	GetNearFarHighRez(	u_int uiMin,
								u_int uiMax,
								const CPoint& obPoint,
								float& fDistanceSq,
								bool bXZOnly,
								bool bNear,
								float fDistSqTolerance,
								float fTimeTolerance ) const;
	float	GetNearestLinHighRez(	float fMin,
									float fMax,
									const CPoint& obPoint,
									float& fDistanceSq,
									u_int iSectionSubDivs,
									float fDistSqTolerance ) const;

	u_int			m_uiNumSpan;			// number of spans in the curve
	bool			m_bInvalid;				// curve is not setup correctly yet
	bool			m_bHasPolynomial;		// curve can generate polynomial coeffs (and hence can be converted)
	bool			m_bOpen;				// change how get nearest point operates
	mutable float	m_fLength;				// length of the curve
	
	// conversion matrices
	static const float	gafPoly2Bezier[4][4];
	static const float	gafPoly2Hermite[4][4];
	static const float	gafPoly2Catmull[4][4];
	static const float	gafPoly2CubicUBS[4][4];
};





/***************************************************************************************************
*
*	CLASS			CTimeCurveInterface
*
*	DESCRIPTION		Abstract Time-based Interface to curve objects.
*
*	NOTES			Publicly:
*						Overules base evaluation interfaces to all polynomial curves.
*						Adds conversion functions for all time curves (eg bezier from hermite)
*						Adds an interface to the time module, if curve length queries are required
*
*					Protectedly:
*						Provides two constructors, one for loading from a file, the other for debug
*						Provides some file IO functions for member vars
*
***************************************************************************************************/
class CTimeCurveInterface : public CCurveInterface
{
public:
	virtual ~CTimeCurveInterface( void ) { ntAssert(m_pobTimeModule); NT_DELETE_CHUNK(Mem::MC_CAMERA, m_pobTimeModule ); }

	// evaluation functions
	virtual CVector	Evaluate( float fU )	const;
	virtual CVector	Derivative1( float fU )	const;
	virtual CVector	Derivative2( float fU )	const;
	
	const	CCurveTimeModule&	GetTimeModule( void ) const { return *m_pobTimeModule; }
/*
	// conversion functions
	CCubicTimeBezier*	CreateTimeBezier( void )	const;
	CCubicTimeHermite*	CreateTimeHermite( void )	const;
	CCubicTimeCatmull*	CreateTimeCatmull( void )	const;
	CCubicTimeUBS*		CreateTimeUBS( void )		const;
	CTimeNurbs*			CreateTimeNurbs( void )		const	{ ntAssert(0); return NULL; };
*/

protected:
	// this is supposed to be the usual one, used in conjunction with a load from file
	CTimeCurveInterface( bool bHasPolynomial ) :
		CCurveInterface( bHasPolynomial )
	{
		m_pobTimeModule = NULL;
	}

	// this is a debug interface so we can hand construct our curves
	CTimeCurveInterface( float* pfTimes, bool bOwnsTimes, bool bHasPolynomial, u_int uiNumSpan, bool bOpen, float fLength = -1.0f ) :
		CCurveInterface(  bHasPolynomial, uiNumSpan, bOpen, fLength )
	{
		m_pobTimeModule = NT_NEW_CHUNK( Mem::MC_CAMERA ) CCurveTimeModule( uiNumSpan+1, pfTimes, bOwnsTimes );
		ntAssert( m_pobTimeModule );
	}


private:
	CCurveTimeModule*	m_pobTimeModule;
};

#endif
