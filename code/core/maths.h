/***************************************************************************************************
*
*   $Header:: /game/maths.h 10    2/06/03 14:02 Giles                                              $
*
*	Header for static maths class
*
*	CHANGES		
*
*	15/05/2001	dean	Created
*
***************************************************************************************************/

#ifndef	_MATHS_H
#define	_MATHS_H

// Constants (these are in global scope on purpose - they are statically linked - see MSVC docs)

const	float	PI					=	3.1415926535897932384626433832795f;
const	float	TWO_PI				=	6.283185307179586476925286766559f;
const	float	HALF_PI				=	1.5707963267948966192313216916398f;
const	float	QUARTER_PI			=	0.785398163397448309615660845819876f;
const	float	ONE_OVER_PI			=	0.31830988618379067153776752674503f;
const	float	ROOT_TWO			=	1.4142135623730950488016887242097f;
const	float	ROOT_HALF			=	0.70710678118654752440084436210485f;
const	float	DEG_TO_RAD_VALUE	=	0.01745329f;
const	float	RAD_TO_DEG_VALUE	=	57.29578f;
const	float	MAX_POS_FLOAT		=	FLT_MAX;
const	float	MIN_NEG_FLOAT		=	-FLT_MAX;
const	float	LOG2				=	0.69314718056f;
const	float	LOG10				=	2.30258509299f;
const	float	MATH_E              =   2.71828182845904523536f;

const	uint32_t QNAN_BITS			=	0xffffffff;

const	float	EPSILON				=	1.0e-05f;				// Changing this will break lots of code. Don't do it.

// Abstractions for use in project code

//#define	_SAFE_BUT_SLOW

#define	fsqrtf(f) 		sqrtf( f )
#define	frsqrtf(f)		( 1.0f / sqrtf( f ) )
#define	fsinf(f) 		sinf( f )
#define	fcosf(f) 		cosf( f )
#define	ftanf(f) 		tanf( f )
#define	fasinf(f)		asinf((f))
#define	facosf(f) 		acosf((f))
//#define	fasinf(f)		CMaths::SafeAsinf( f )
//#define	facosf(f) 		CMaths::SafeAcosf( f )
#define	fatanf(f)		atanf( f )
#define	fatan2f(f,g) 	atan2f( f ,  g )

/*#if defined(_SAFE_BUT_SLOW) || ( !defined( _USE_SSE ) )

#define	fsqrtf(f) 		CMaths::Sqrtf((f))
#define	frsqrtf(f)		CMaths::Rsqrtf((f))
#define	fsinf(f) 		::sinf((f))
#define	fcosf(f) 		::cosf((f))
#define	ftanf(f) 		::tanf((f))
#define	fasinf(f)		::asinf((f))
#define	facosf(f) 		::acosf((f))
#define	fatanf(f)		::atanf((f))
#define	fatan2f(f,g) 	::atan2f((f), (g))

#else

#define	fsqrtf(f) 		CMaths::Sqrtf((f))
#define	frsqrtf(f)		CMaths::Rsqrtf((f))
#define	fsinf(f) 		CMaths::Sinf((f))
#define	fcosf(f) 		CMaths::Cosf((f))
#define	ftanf(f) 		CMaths::Tanf((f))
#define	fasinf(f)		CMaths::Asinf((f))
#define	facosf(f) 		CMaths::Acosf((f))
#define	fatanf(f)		CMaths::Atanf((f))
#define	fatan2f(f,g) 	::atan2f((f), (g))

// Let's have a go at stopping people from using the C library functions..
#define	sqrtf			please_use_fsqrtf
#define	rsqrtf			please_use_frsqrtf
#define	sinf			please_use_fsinf
#define	cosf			please_use_fcosf
#define	tanf			please_use_ftanf
#define	asinf			please_use_fasinf
#define	acosf			please_use_facosf
#define	atanf			please_use_fatanf

#endif	//_SAFE_BUT_SLOW*/


/***************************************************************************************************
*
*	CLASS			CMaths
*
*	DESCRIPTION		Contains static maths functionality (sin/cos) to be used in preference to the
*					standard GNU floating point stuff (which is so..very...slow..).
*
***************************************************************************************************/

class CMaths
{
public:

	// 30.7.04 Added safe versions of trig functions to prevent indefinates on some builds.
	// 
	static inline float SafeAsinf( float x )
	{
		x = ntstd::Clamp( x, -1.0f, 1.0f );
		return asinf( x );
	}

	static inline float SafeAcosf( float x )
	{
		x = ntstd::Clamp( x, -1.0f, 1.0f );
		return acosf( x );
	}

/*	// Floating point to integer (taken from rwcore.h)
	static inline ToInt( float  fValue )
	{
	    __asm cvttss2si eax, [ fValue ]
	}

	// Fast version of (int)floorf(fX)
	static inline int IntFloor( float fX )
	{
		int iX = ToInt( fX );
		return( ( fX >= 0.0f ) ? iX : ( -1 + ( iX ) ) );
	}

	// Calculate square root of input
	static inline float Sqrtf( float fX )
	{
		__m128	qTemp = _mm_sqrt_ss( _mm_load_ss( &fX ) );

		float	fResult;
		_mm_store_ss( &fResult, qTemp );
		return fResult;
	}
	
	// Calculate reciprocal square root of input
	static inline float Rsqrtf( float fX )
	{
		__m128	qTemp = _mm_rsqrt_ss( _mm_load_ss( &fX ) );

		float	fResult;
		_mm_store_ss( &fResult, qTemp );
		return fResult;
	}

	// Calculate sine of angle (angle in radians)
	static float	Sinf( float fRadians );

	// Calculate cosine of angle (angle in radians)
	static float	Cosf( float fRadians );

	// Calculate tangent of input
	static float	Tanf( float fRadians );

	// Calculate arc tangent of x
	static float	Atanf( float fX );

	// Calculate arc cosine of input (input from -1 to 1)
	static inline float	Acosf( float fInput )
	{
		if ( fInput < 1.0f )
			return	2.0f * fatanf( frsqrtf( ( 1.0f + fInput ) * ( 1.0f / ( 1.0f - fInput ) ) ) );
		else
			return  0.0f;
	}

	// Calculate arc sine of input
	static inline float Asinf( float fInput )
	{
		if ( fInput < 1.0f )
			return	fatanf( fInput * frsqrtf( ( 1.0f - fInput ) * ( 1.0f + fInput ) ) );
		else
			return	HALF_PI;
	}

	// Calculate exponent of input
	static	float	CMaths::Expf( float fInput );

	// Calculate binary exponent of input
	static	float	CMaths::Exp2f( float fInput );

	// Calculate logarithm of input
	static	float	CMaths::Logf( float fInput );

	// Calculate the binary logarithm of input
	static	float	CMaths::Log2f( float fInput );

	// Calculate fX raised to the power of fY
	static	float	CMaths::Powf( float fX, float fY );*/

	// Return sin and cos of same angle
	static inline void SinCos( float fAngle, float& fSin, float& fCos )
	{
		fSin = fsinf( fAngle );
		fCos = fcosf( fAngle );
	}

	// Return a LERP.
	static inline float Lerp( float fA, float fB, float fT )
	{
		return fA + ( fB - fA )*fT;
	}
	
	// Return smoothstep of value
	static inline float	SmoothStep( float fValue )
	{
		return fValue * fValue * (3.0f - 2.0f*fValue);
	}
};

/***************************************************************************************************
*
*	CLASS			CQuadraticSolver
*
*	DESCRIPTION		Solves a quadratic equation with floating point coefficients.
*
***************************************************************************************************/

class CQuadraticSolver
{
public:
	//! Constructs a default solver.
	CQuadraticSolver() : m_fA(0.0f), m_fB(0.0f), m_fC(0.0f), m_bUnsolved(true) {}
	
	//! Sets up to solve the equation ( a*x^2 + b*x + c = 0 ).
	CQuadraticSolver( float fA, float fB, float fC ) : m_fA(fA), m_fB(fB), m_fC(fC), m_bUnsolved(true) {}


	//! Sets the coefficients for the equation ( a*x^2 + b*x + c = 0 ).
	void SetCoefficients( float fA, float fB, float fC );


	//! Gets the number of solutions.
	int GetNumSolutions() const;

	//! Gets the solution with index iIndex.
	float GetSolution( int iIndex ) const;


private:
	void Solve() const;

	float m_fA, m_fB, m_fC;				//!< The coefficients.
	
	mutable bool m_bUnsolved;			//!< True if we haven't tried to solve yet.
	mutable int m_iNumSolutions;		//!< The number of solutions, zero for none.
	mutable float m_afSolutions[2];		//!< The solutions.
};

#endif	//_MATHS_H
