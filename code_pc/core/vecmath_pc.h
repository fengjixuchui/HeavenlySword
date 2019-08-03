/***************************************************************************************************
*
*	$Header:: /game/vecmath.h 25    21/07/03 16:37 Dean                                            $
*
*	Vector Maths 
*
*	You'll notice that inlines come *after* the classes. This makes things a lot clearer.
*
*	NOTES
*
*	You'll see that most of the methods available return vectors (and matrices) by value. This means 
*	that memory reads & writes can be eliminated by the compiler as it sees fit. Common operations on
*	vectors tend to just use a few SSE registers, with a final write to memory - even when stringing
*	together multiple operations.
*
*	CHANGES
*
*	18/11/2002	Dean	Created
*
***************************************************************************************************/

#ifndef	_VECMATH_PC_H
#define	_VECMATH_PC_H

// always use SSE on PC now
#include <xmmintrin.h>
typedef	__m128	__m128_arg;

#include "maths.h"

/*
#ifdef _DEBUG
#define	_DEBUG_VECMATH					// Enables isnan() checking on all floats in vector class.
#endif
*/

typedef	__m128 v128;
typedef const v128& v128_arg;

class	CVectorBase;
class	CVector;
class	CMatrix;

class	CDirection;
class	CPoint;
class	CQuat;

// If the standard SSE shuffle parameter macro isn't there, define it..
#ifndef	_MM_SHUFFLE
#define _MM_SHUFFLE(fp3,fp2,fp1,fp0) (((fp3) << 6) | ((fp2) << 4) | ((fp1) << 2) | ((fp0)))
#endif	//_MM_SHUFFLE

// As suggested by Simon, here's a macro that has shuffle parameters in the order you'd expect..
#define _MM_SSE_SHUFFLE(x, y, z, w)		( ( x ) | ( ( y ) << 2 ) | ( ( z ) << 4 ) | ( ( w ) << 6 ) )

// And here are some SIMD macros for rotating quadwords
#define _mm_ror_ps(vec,i)	\
	(((i)%4) ? (_mm_shuffle_ps(vec,vec, _MM_SHUFFLE((unsigned char)(i+3)%4,(unsigned char)(i+2)%4,(unsigned char)(i+1)%4,(unsigned char)(i+0)%4))) : (vec))
#define _mm_rol_ps(vec,i)	\
	(((i)%4) ? (_mm_shuffle_ps(vec,vec, _MM_SHUFFLE((unsigned char)(7-i)%4,(unsigned char)(6-i)%4,(unsigned char)(5-i)%4,(unsigned char)(4-i)%4))) : (vec))

class	CVecMath
{
	friend	class CVectorBase;
	friend	class CVector;
	friend	class CDirection;
	friend	class CPoint;
	friend	class CQuat;
	friend	class CMatrix;
	
public:
	static	const	CDirection&	GetXAxis( void )			{ return *( ( CDirection* )&m_gauiIdentityMatrix[ 0 * 4 ] ); };
	static	const	CDirection&	GetYAxis( void )			{ return *( ( CDirection* )&m_gauiIdentityMatrix[ 1 * 4 ] ); };
	static	const	CDirection&	GetZAxis( void )			{ return *( ( CDirection* )&m_gauiIdentityMatrix[ 2 * 4 ] ); };
	static	const	CQuat&		GetQuatIdentity( void )		{ return *( ( CQuat* )&m_gauiIdentityMatrix[ 3 * 4 ] ); };
	static	const	CMatrix&	GetIdentity( void )			{ return *( ( CMatrix* )m_gauiIdentityMatrix ); };
	static	const	CVector&	GetZeroVector( void )		{ return *( ( CVector* )m_gauiZero ); };
	static	const	CDirection&	GetZeroDirection( void )	{ return *( ( CDirection* )m_gauiZero ); };
	static	const	CPoint&		GetZeroPoint( void )		{ return *( ( CPoint* )m_gauiZero ); };

protected:
	ALIGNTO_PREFIX(16) static	const	uint32_t	m_gauiIdentityMatrix[ 16 ] ALIGNTO_POSTFIX(16);	// Identity matrix. Also used to get individual axes
	ALIGNTO_PREFIX(16) static	const	uint32_t	m_gauiZero[ 4 ] ALIGNTO_POSTFIX(16);				// Zero vector/direction/point

	ALIGNTO_PREFIX(16) static	const	uint32_t	m_gauiMaskOffW[ 4 ] ALIGNTO_POSTFIX(16);			// All components except W contain 0xffffffff
	ALIGNTO_PREFIX(16) static	const	uint32_t	m_gauiOneInW[ 4 ] ALIGNTO_POSTFIX(16);				// All components hold 0x00000000, except W which holds 0x3f800000
	ALIGNTO_PREFIX(16) static	const	uint32_t	m_gauiMaskOffSign[ 4 ] ALIGNTO_POSTFIX(16);			// Each component holds 0x7fffffff
	ALIGNTO_PREFIX(16) static	const	uint32_t	m_gauiInvalidValue[ 4 ] ALIGNTO_POSTFIX(16);		// Each component holds QNaN equivalent (0xffffffff)
	ALIGNTO_PREFIX(16) static	const	uint32_t	m_gauiQuatConjugateMask[ 4 ] ALIGNTO_POSTFIX(16);	// Each component holds 0x80000000, except W which holds 0x00000000

	ALIGNTO_PREFIX(16) static	const	uint32_t	m_gauiPNPNMask[ 4 ] ALIGNTO_POSTFIX(16);			// Mask with positive/negative/positive/negative sign control bits
	ALIGNTO_PREFIX(16) static	const	uint32_t	m_gauiNPNPMask[ 4 ] ALIGNTO_POSTFIX(16);			// Mask with negative/positive/negative/positive sign control bits
};

CVector		operator * ( float fScalar, const CVector& obVector );
CDirection	operator * ( float fScalar, const CDirection& obDirection );
CPoint		operator * ( float fScalar, const CPoint& obPoint );
CQuat		operator * ( float fScalar, const CQuat& obQuat );


// Single Entry Enumerations used to enable on-construction initialisation of vectors and matrices
// to clear and identity values. CONSTRUCT_CLEAR can be specified as a constructor argument to 
// all types, while CONSTRUCT_IDENTITY can only be used on CMatrix and CQuat objects.

enum	CLEAR_CONSTRUCT_MODE
{
	CONSTRUCT_CLEAR,
};

enum	DONT_CLEAR_CONSTRUCT_MODE
{
	CONSTRUCT_DONT_CLEAR,			//!< for maths function to clear by default this disables it i.e. AABB
};

enum	IDENTITY_CONSTRUCT_MODE
{
	CONSTRUCT_IDENTITY,
};

// used by some of the bounding volume types to construct a infinite (or at least very big) sized volume
enum INFINITE_CONSTRUCT_MODE
{
	CONSTRUCT_INFINITE_NEGATIVE, //!< a volume that has massive negative volume, so any unions will create a valid volume
	CONSTRUCT_INFINITE_POSITIVE, //!< really really big volume, inteersection will shrink volume to appropiate size
};

/***************************************************************************************************
*
*	CLASS			CVectorBase
*
*	DESCRIPTION		Defines storage requirements common to all derived vector types, along with
*					common accessors. The constructor is private to prevent people from creating 
*					instances of this object type, whereas all derived types are friends - meaning
*					they can call any base construction functionality.
*
*	NOTES			This object occupies 16 Bytes of aligned storage.
*
***************************************************************************************************/

ALIGNTO_PREFIX(16) class CVectorBase
{
	friend	class CVector;
	friend	class CDirection;
	friend	class CPoint;
	friend	class CQuat;
	friend	class CMatrix;

	friend	CVector		operator * ( float fScalar, const CVector& obVector );
	friend	CDirection	operator * ( float fScalar, const CDirection& obDirection );
	friend	CPoint		operator * ( float fScalar, const CPoint& obPoint );
	friend	CQuat		operator * ( float fScalar, const CQuat& obQuat );

	friend	CVector		operator * ( const CVector& obVector, const CMatrix& obMatrix );
	friend	CPoint		operator * ( const CPoint& obPoint, const CMatrix& obMatrix );
	friend	CDirection	operator * ( const CDirection& obDirection, const CMatrix& obMatrix );


public:
	
	// SIMD Accessors
	__m128&			Quadword()							{ return SIMDVector.m_qVector; };
	__m128			QuadwordValue() const				{ CheckVector(); return SIMDVector.m_qVector; };

	// Float Accessors
	const float&	X() const							{ CheckVector(); return SIMDVector.m_afVal[ 0 ]; };
	const float&	Y() const							{ CheckVector(); return SIMDVector.m_afVal[ 1 ]; };
	const float&	Z() const							{ CheckVector(); return SIMDVector.m_afVal[ 2 ]; };
	const float&	W() const							{ CheckVector(); return SIMDVector.m_afVal[ 3 ]; };
	float&	X()											{ return SIMDVector.m_afVal[ 0 ]; };
	float&	Y() 										{ return SIMDVector.m_afVal[ 1 ]; };
	float&	Z()											{ return SIMDVector.m_afVal[ 2 ]; };
	float&	W()											{ return SIMDVector.m_afVal[ 3 ]; };

	// Array accessors
	const float&	operator []( int iIndex ) const; 
	float&			operator []( int iIndex );

	const float&	Index( int iIndex ) const; 
	float&			Index( int iIndex );


	// Vector debugging
#ifdef	_DEBUG_VECMATH
	void			CheckVector( void ) const;
#else
	void			CheckVector( void ) const	{};
#endif	//_DEBUG_VECMATH

	void			SetFromNTColor( uint32_t col );
	uint32_t		GetNTColor( void ) const;

protected:

	// Constructors
	CVectorBase();


	// Static SIMD operators (used as a base for real operations)

	static	__m128	SIMDAdd( __m128_arg qVector0, __m128_arg qVector1 );
	static	__m128	SIMDSub( __m128_arg qVector0, __m128_arg qVector1 );
	static	__m128	SIMDMul( __m128_arg qVector0, __m128_arg qVector1 );
	static	__m128	SIMDDiv( __m128_arg qVector0, __m128_arg qVector1 );
	static	__m128	SIMDMin( __m128_arg qVector0, __m128_arg qVector1 );
	static	__m128	SIMDMax( __m128_arg qVector0, __m128_arg qVector1 );
	static	__m128	SIMDDp3( __m128_arg qVector0, __m128_arg qVector1 );
	static	__m128	SIMDDp4( __m128_arg qVector0, __m128_arg qVector1 );

protected:

	// Initialisation
	void	Clear();

private:

	union	SIMD_UNION
	{
		__m128	m_qVector;
		float	m_afVal[ 4 ];
	};
	
	SIMD_UNION	SIMDVector;
} ALIGNTO_POSTFIX(16);


/***************************************************************************************************
*
*	CLASS			CVector
*
*	DESCRIPTION		Full 4-element vector type.
*
*	NOTES			This object occupies 16 Bytes of aligned storage. Other vector types (excluding
*					quaternions) are derived from this..
*
***************************************************************************************************/

class	CVector	: public CVectorBase
{
public:
	
	// Constructors
	CVector();
	CVector( float fX, float fY, float fZ, float fW );
	CVector( const CVector& obVector );

	// Explicit Constructors
	explicit	CVector( float fValue );
	explicit	CVector( CLEAR_CONSTRUCT_MODE eClear );
	explicit	CVector( const float* pfData );
	explicit	CVector( const CPoint& obPoint );
	explicit	CVector( const CDirection& obDirection );
	explicit	CVector( const CQuat& obQuat );
	explicit	CVector( v128 v ) { Quadword() = v; }

	// Initialisation
	void	Clear() { CVectorBase::Clear(); };

	// Assignment
	CVector& operator =	 ( const CVector& obVector );
	CVector& operator += ( const CVector& obVector );
	CVector& operator -= ( const CVector& obVector );
	CVector& operator *= ( const CVector& obVector );
	CVector& operator *= ( float fScalar );
	CVector& operator /= ( const CVector& obVector );
	CVector& operator /= ( float fDivisor );

	// Unary operators
	CVector	operator + () const;
	CVector	operator - () const;

	// Binary operators
	CVector	operator +	( const CVector& obVector ) const;
	CVector	operator -	( const CVector& obVector ) const;
	CVector	operator *	( const CVector& obVector ) const;
	CVector	operator *	( float fScalar ) const;
	CVector operator /	( const CVector& obVector ) const;
	CVector	operator /	( float fDivisor ) const;

	// Const operations
	float	Length( void ) const;
	float	LengthSquared( void ) const;
	float	Dot( const CVector& obVector ) const;
	CVector	Abs( void ) const;
	CVector	Min( const CVector& obVector ) const;
	CVector	Max( const CVector& obVector ) const;

	// Static operations
	static	CVector	Lerp( const CVector& obVector0, const CVector& obVector1, float fT );

private:

	// Equivalence operators 
	bool	operator == ( const CVector& obVector ) const;
	bool	operator != ( const CVector& obVector ) const;
};


/***************************************************************************************************
*
*	CLASS			CDirection
*
*	DESCRIPTION		A 3-element vector used to define a direction in 3D space. 
*
*	NOTES			This object occupies 16 Bytes of aligned storage.
*
***************************************************************************************************/

class	CDirection	: public CVectorBase
{
public:

	// Constructors
	CDirection();
	CDirection( float fPhi, float fTheta );
	CDirection( float fX, float fY, float fZ );
	CDirection( const CDirection& obDirection );

	// Explicit Constructors
	explicit	CDirection( CLEAR_CONSTRUCT_MODE eClear );
	explicit	CDirection( const float* pfData );
	explicit	CDirection( const CPoint& obPoint );
	explicit	CDirection( const CVector& obVector );
	explicit	CDirection( v128 v ) { Quadword() = v; }

	// Initialisation
	void	Clear() { CVectorBase::Clear(); };

	// Assignment
	CDirection& operator =	( const CDirection& obDirection );
	CDirection& operator += ( const CDirection& obDirection );
	CDirection& operator -= ( const CDirection& obDirection );
	CDirection& operator *= ( const CDirection& obDirection );
	CDirection& operator *= ( float fScalar );
	CDirection& operator /= ( float fDivisor );

	// Unary operators
	CDirection	operator + () const;
	CDirection	operator - () const;

	// Binary operators
	CDirection	operator +	( const CDirection& obDirection ) const;
	CDirection	operator -	( const CDirection& obDirection ) const;
	CDirection	operator *	( const CDirection& obDirection ) const;
	CDirection	operator *	( float fScalar ) const;
	CDirection	operator /	( float fDivisor ) const;
	CPoint		operator *	( const CPoint& obPoint ) const;
	CPoint		operator +	( const CPoint& obPoint ) const;
	CPoint		operator -	( const CPoint& obPoint ) const;

	// Const operations
	float		Length( void ) const;
	float		LengthSquared( void ) const;
	float		Dot( const CDirection& obDirection ) const;
	CDirection	Abs( void ) const;
	CDirection	Min( const CDirection& obDirection ) const;
	CDirection	Max( const CDirection& obDirection ) const;
	CDirection	Cross( const CDirection& obDirection1 ) const;
	CDirection	GetPerpendicular( void ) const;

	// Non-Const operations
	bool		Normalise( void );
	float		CalculateRotationAxis( const CDirection& obFrom, const CDirection& obTo );
	
	// Static operations
	static	CDirection	Lerp( const CDirection& obDirection0, const CDirection& obDirection1, float fT );

	// Comparison operation
	bool	Compare( const CDirection& obDirection, float fSquaredTolerance ) const;

private:

	// Equivalence operators 
	bool		operator == ( const CDirection& obDirection ) const;
	bool		operator != ( const CDirection& obDirection ) const;
};


/***************************************************************************************************
*
*	CLASS			CPoint
*
*	DESCRIPTION		A 3-element vector used to define a point in 3D space. 
*
*	NOTES			This object occupies 16 Bytes of aligned storage.
*
***************************************************************************************************/

class	CPoint	: public CVectorBase
{
public:

	// Constructors
	CPoint();
	CPoint( float fX, float fY, float fZ );
	CPoint( const CPoint& obPoint );

	// Explicit Constructors
	explicit	CPoint( CLEAR_CONSTRUCT_MODE eClear );
	explicit	CPoint( const float* pfData );
	explicit	CPoint( const CDirection& obDirection );
	explicit	CPoint( const CVector& obVector );
	explicit	CPoint( v128 v ) { Quadword() = v; }

	// Initialisation
	void	Clear() { CVectorBase::Clear(); };

	// Assignment
	CPoint& operator =	( const CPoint& obPoint );
	CPoint& operator += ( const CPoint& obPoint );
	CPoint& operator -= ( const CPoint& obPoint );
	CPoint& operator *= ( const CPoint& obPoint );
	CPoint& operator += ( const CDirection& obDirection );
	CPoint& operator -= ( const CDirection& obDirection );
	CPoint& operator *= ( float fScalar );
	CPoint& operator /= ( float fDivisor );

	// Unary operators
	CPoint	operator + () const;
	CPoint	operator - () const;

	// Binary operators
	CPoint operator +	( const CPoint& obPoint ) const;
	CPoint operator -	( const CPoint& obPoint ) const;
	CPoint operator *	( const CPoint& obPoint ) const;
	CPoint operator *	( float fScalar ) const;
	CPoint operator /	( float fDivisor ) const;
	CPoint operator *	( const CDirection& obDirection ) const;
	CPoint operator +	( const CDirection& obDirection ) const;
	CPoint operator -	( const CDirection& obDirection ) const;

	// Special Binary operator
	CDirection operator ^ ( const CPoint& obPoint ) const;

	// Const operations
	float	Length( void ) const;
	float	LengthSquared( void ) const;
	float	Dot( const CPoint& obPoint ) const;
	float	Dot( const CDirection &direction ) const;
	CPoint	Abs( void ) const;
	CPoint	Min( const CPoint& obPoint ) const;
	CPoint	Max( const CPoint& obPoint ) const;

	// Static operations
	static	CPoint	Lerp( const CPoint& obPoint0, const CPoint& obPoint1, float fT );

	// Comparison operation
	bool	Compare( const CPoint& obPoint, float fSquaredTolerance ) const;
	
private:

	// Equivalence operators 
	bool	operator == ( const CPoint& obPoint ) const;
	bool	operator != ( const CPoint& obPoint ) const;
};


/***************************************************************************************************
*
*	CLASS			CQuat
*
*	DESCRIPTION		A 4-element vector used to define a quaternion. 
*
*	NOTES			This object occupies 16 Bytes of aligned storage.
*
***************************************************************************************************/

class	CQuat	: public CVectorBase
{
public:

	// Constructors
	CQuat();
	CQuat( float fX, float fY, float fZ, float fW );
	CQuat( const CQuat& obQuat );
	CQuat( const CDirection& obAxis, float fAngle );
	CQuat( const CDirection& obRotateFrom, const CDirection& obRotateTo );

	// Explicit Constructors
	explicit	CQuat( CLEAR_CONSTRUCT_MODE eClear );
	explicit	CQuat( IDENTITY_CONSTRUCT_MODE eIdentity );
	explicit	CQuat( const float* pfData );
	explicit	CQuat( const CMatrix& obMatrix );	
	explicit	CQuat( const CVector& obVector );
	explicit	CQuat( v128 v ) { Quadword() = v; }

	// Initialisation
	void	SetIdentity( void );

	// Assignment
	CQuat& operator =  ( const CQuat& obQuat );
	CQuat& operator *= ( const CQuat& obQuat );
	CQuat& operator *= ( float fScalar );
	CQuat& operator /= ( float fDivisor );

	// Unary operators
	CQuat	operator + () const;
	CQuat	operator - () const;
	CQuat	operator ~ () const;

	// Binary operators
	CQuat operator * ( const CQuat& obQuat ) const;
	CQuat operator * ( float fScalar ) const;
	CQuat operator / ( float fDivisor ) const;

	// Non-Const operations
	bool			Normalise( void );

	// Const operations
	bool			IsNormalised( void ) const;
	float			Dot( const CQuat& obQuat ) const;
	void			GetAxisAndAngle( CDirection& obAxis, float& fAngle ) const;

	// Static operations
	static	CQuat	RotationBetween( const CDirection& obRotateFrom, const CDirection& obRotateTo );
	static	CQuat	Slerp( const CQuat& obQuat0, const CQuat& obQuat1, float fT );

private:

	// These are only for use internally.. nobody should attempt to use externally.
	CQuat& operator += ( const CQuat& obQuat );
	CQuat& operator -= ( const CQuat& obQuat );

	CQuat operator + ( const CQuat& obQuat ) const;
	CQuat operator - ( const CQuat& obQuat ) const;

	// Equivalence operators 
	bool	operator == ( const CQuat& obQuat ) const;
	bool	operator != ( const CQuat& obQuat ) const;
};


/***************************************************************************************************
*
*	CLASS			CMatrix
*
*	DESCRIPTION		A 4x4 matrix.
*
*	NOTES			This object occupies ( 4 * 16 ) Bytes of aligned storage.
*
***************************************************************************************************/

ALIGNTO_PREFIX(16) class CMatrix
{
public:

	// Constructors
	CMatrix();
	CMatrix( const CMatrix& obMatrix );

	explicit CMatrix( const CQuat& obRotation, const CPoint& obTranslation = CVecMath::GetZeroPoint() );
	CMatrix( const CDirection& obAxis, float fAngle );

	CMatrix(	float fVal00, float fVal01, float fVal02, float fVal03,
				float fVal10, float fVal11, float fVal12, float fVal13,
				float fVal20, float fVal21, float fVal22, float fVal23,
				float fVal30, float fVal31, float fVal32, float fVal33 );

	// Explicit Constructors
	explicit	CMatrix( CLEAR_CONSTRUCT_MODE eClear );
	explicit	CMatrix( IDENTITY_CONSTRUCT_MODE eIdentity );
	explicit	CMatrix( const float* pfData );
			
	// Copy constructor
	CMatrix& operator =	( const CMatrix& obMatrix );

	// Casting
	operator float* ();
	operator const float* () const;

	// Initialisation
	void		Clear( void );
	void		SetIdentity( void );
	CMatrix		GetTranspose( void ) const;
	CMatrix		GetAffineInverse( void ) const;
	CMatrix		GetFullInverse( void ) const;
	
	// note this leaves 
	void		SetFromQuat( const CQuat& obRotation );
	void		SetFromAxisAndAngle( const CDirection& obAxis, float fAngle );

	// Accessors

	const	CVector&	operator[] ( int iIndex ) const;
	CVector&			operator[] ( int iIndex );

	const	CVector&	Row( int iRow ) const;
	CVector&			Row( int iRow );

	const	CDirection&	GetXAxis() const;
	void				SetXAxis( const CDirection& obXAxis );

	const	CDirection&	GetYAxis() const;
	void				SetYAxis( const CDirection& obYAxis );

	const	CDirection&	GetZAxis() const;
	void				SetZAxis( const CDirection& obZAxis );

	const	CPoint&		GetTranslation() const;
	void				SetTranslation( const CPoint& obTranslation );

	//	Axis Creation
	void				BuildXAxis( void );
	void				BuildYAxis( void );
	void				BuildZAxis( void );

	// Multiplication
	CMatrix&	operator *=	( const CMatrix& obMatrix );

	// Other functionality
	void		Perspective( float fFieldOfViewY, float fAspectRatio, float fNearClipDistance, float fFarClipDistance );
	void		Perspective2( float fWidth, float fHeight, float fNearClipDistance, float fFarClipDistance );

	static CMatrix Lerp( const CMatrix& obMatrix0, const CMatrix& obMatrix1, float fT );

	// Debug Functionality
#ifdef	_DEBUG_VECMATH
	void		CheckAffineTransform( void ) const
	{
		ntAssert( ( Row( 0 ).W() == 0.0f ) && ( Row( 1 ).W() == 0.0f ) && ( Row( 2 ).W() == 0.0f ) && ( Row( 3 ).W() == 1.0f ) );
	}
#else
	void		CheckAffineTransform( void ) const {};
#endif	//_DEBUG_VECMATH

protected:
	CVector		m_aobRow[ 4 ];
} ALIGNTO_POSTFIX(16);


// Globally scoped multiply operations

CMatrix				operator *	( const CMatrix& obMatrixL, const CMatrix& obMatrixR );
CVector				operator *	( const CVector& obVector, const CMatrix& obMatrix );
CDirection			operator *	( const CDirection& obDirection, const CMatrix& obMatrix );
CPoint				operator *	( const CPoint& obPoint, const CMatrix& obMatrix );


/***************************************************************************************************
*
*	CLASS			CVectorBase	-	Inlines
*
***************************************************************************************************/

// ---- Constructors

inline	CVectorBase::CVectorBase()
{
#ifdef	_DEBUG_VECMATH
	Quadword() = _mm_load_ps( ( float * )&CVecMath::m_gauiInvalidValue );
#endif	//_DEBUG_VECMATH
}

// ---- Initialisation

inline	void CVectorBase::Clear( void )
{
	Quadword() = _mm_setzero_ps();
}

// ---- Array access

inline	const float& CVectorBase::operator[] ( int iIndex ) const
{
	ntAssert( iIndex < 4 );
	return *( float *)( &SIMDVector.m_afVal[ iIndex ] );
}

inline	float&	CVectorBase::operator[] ( int iIndex )
{
	ntAssert( iIndex < 4 );
	return *( float *)( &SIMDVector.m_afVal[ iIndex ] );
}

inline	const float& CVectorBase::Index( int iIndex ) const
{
	ntAssert( iIndex < 4 );
	return *( float *)( &SIMDVector.m_afVal[ iIndex ] );
}

inline	float&	CVectorBase::Index( int iIndex )
{
	ntAssert( iIndex < 4 );
	return *( float *)( &SIMDVector.m_afVal[ iIndex ] );
}

inline	void	CVectorBase::SetFromNTColor( uint32_t col )
{
	NTCOLOUR_EXTRACT_FLOATS( col, X(), Y(), Z(), W() );
}

inline	uint32_t	CVectorBase::GetNTColor( void ) const
{
	return NTCOLOUR_FROM_FLOATS( X(), Y(), Z(), W() );	
}

// ---- 

inline	__m128	CVectorBase::SIMDAdd( __m128_arg qVector0, __m128_arg qVector1 )
{
	return _mm_add_ps( qVector0, qVector1 );
}

inline	__m128	CVectorBase::SIMDSub( __m128_arg qVector0, __m128_arg qVector1 )
{
	return _mm_sub_ps( qVector0, qVector1 );
}

inline	__m128	CVectorBase::SIMDMul( __m128_arg qVector0, __m128_arg qVector1 )
{
	return _mm_mul_ps( qVector0, qVector1 );
}

inline	__m128	CVectorBase::SIMDDiv( __m128_arg qVector0, __m128_arg qVector1 )
{
	return _mm_div_ps( qVector0, qVector1 );
}

inline	__m128	CVectorBase::SIMDMin( __m128_arg qVector0, __m128_arg qVector1 )
{
	return _mm_min_ps( qVector0, qVector1 );
}

inline	__m128	CVectorBase::SIMDMax( __m128_arg qVector0, __m128_arg qVector1 )
{
	return _mm_max_ps( qVector0, qVector1 );
}

inline	__m128	CVectorBase::SIMDDp3( __m128_arg qVector0, __m128_arg qVector1 )
{
	__m128	qTemp;
	qTemp	= _mm_mul_ps( qVector0, qVector1 );
	qTemp	= _mm_add_ss( _mm_shuffle_ps( qTemp, qTemp,  _MM_SSE_SHUFFLE( 1, 0, 0, 0 ) ), _mm_add_ss( _mm_movehl_ps( qTemp, qTemp ), qTemp ) );
	return qTemp;
}

inline	__m128	CVectorBase::SIMDDp4( __m128_arg qVector0, __m128_arg qVector1 )
{
	__m128	qTemp;
	qTemp	= _mm_mul_ps( qVector0, qVector1 );
	qTemp	= _mm_add_ps( _mm_movehl_ps( qTemp, qTemp ), qTemp );
	qTemp	= _mm_add_ss( _mm_shuffle_ps( qTemp, qTemp,  _MM_SSE_SHUFFLE( 1, 0, 0, 0 ) ), qTemp );
	return qTemp;
}

/***************************************************************************************************
*
*	CLASS			CVector	-	Inlines
*
***************************************************************************************************/

// ---- Constructors

inline	CVector::CVector()
{
}

inline	CVector::CVector( CLEAR_CONSTRUCT_MODE )
{
	Clear();
}

inline	CVector::CVector( const float* pfData )
{
	ntAssert( pfData );
	Quadword() = _mm_setr_ps( pfData[ 0 ], pfData[ 1 ], pfData[ 2 ], pfData[ 3 ] );
}

inline	CVector::CVector( float fX, float fY, float fZ, float fW )
{
	ALIGNTO_PREFIX(16) float ALIGNTO_POSTFIX(16) afConstruct[ 4 ] = { fX, fY, fZ, fW };
	Quadword() = _mm_load_ps( afConstruct );
}

inline	CVector::CVector( float fValue )
{
	Quadword() = _mm_set1_ps( fValue );;
}

inline	CVector::CVector( const CVector& obVector )
{
	Quadword() = obVector.QuadwordValue();
}

inline	CVector::CVector( const CPoint& obPoint )
{
	Quadword() = obPoint.QuadwordValue();
}

inline	CVector::CVector( const CDirection& obDirection )
{
	Quadword() = obDirection.QuadwordValue();
}

inline	CVector::CVector( const CQuat& obQuat )
{
	Quadword() = obQuat.QuadwordValue();
}

// ---- Assignment

inline	CVector&	CVector::operator = ( const CVector& obVector )
{
	Quadword() = obVector.QuadwordValue();
	return *this;
}

inline	CVector&	CVector::operator += ( const CVector& obVector )
{
	Quadword() = SIMDAdd( this->QuadwordValue(), obVector.QuadwordValue() );
	return *this;
}

inline	CVector&	CVector::operator -= ( const CVector& obVector )
{
	Quadword() = SIMDSub( this->QuadwordValue(), obVector.QuadwordValue() );
	return *this;
}

inline	CVector&	CVector::operator *= ( const CVector& obVector )
{
	Quadword() = SIMDMul( this->QuadwordValue(), obVector.QuadwordValue() );
	return *this;
}

inline	CVector&	CVector::operator *= ( float fScalar )
{
	__m128	qScalar = _mm_set1_ps( fScalar );
	Quadword() = SIMDMul( this->QuadwordValue(), qScalar );
	return *this;
}

inline	CVector&	CVector::operator /= ( const CVector& obVector )
{
	Quadword() = SIMDDiv( this->QuadwordValue(), obVector.QuadwordValue() );
	return *this;
}

inline	CVector&	CVector::operator /= ( float fDivisor )
{
	__m128	qScalar = _mm_set1_ps( 1.0f / fDivisor );
	Quadword() = SIMDMul( this->QuadwordValue(), qScalar );
	return *this;
}

// ---- Unary operators

inline	CVector		CVector::operator + () const
{
	return *this;
}

inline	CVector		CVector::operator - () const
{
	CVector	obOutput;
	obOutput.Quadword() = SIMDSub( _mm_setzero_ps(), QuadwordValue() );
	return obOutput;
}

// ---- Binary operators

inline	CVector		CVector::operator + ( const CVector& obVector ) const
{
	CVector obOutput;
	obOutput.Quadword() = SIMDAdd( this->QuadwordValue(), obVector.QuadwordValue() );
	return obOutput;
}

inline	CVector		CVector::operator - ( const CVector& obVector ) const
{
	CVector obOutput;
	obOutput.Quadword() = SIMDSub( this->QuadwordValue(), obVector.QuadwordValue() );
	return obOutput;
}

inline	CVector		CVector::operator * ( const CVector& obVector ) const
{
	CVector obOutput;
	obOutput.Quadword() = SIMDMul( this->QuadwordValue(), obVector.QuadwordValue() );
	return obOutput;
}

inline	CVector		CVector::operator * ( float fScalar ) const
{
	CVector	obOutput;
	__m128	qScalar = _mm_set1_ps( fScalar );
	obOutput.Quadword() = SIMDMul( this->QuadwordValue(), qScalar );
	return obOutput;
}

inline	CVector		CVector::operator /	 ( const CVector& obVector ) const
{
	CVector	obOutput;
	obOutput.Quadword() = SIMDDiv( this->QuadwordValue(), obVector.QuadwordValue() );
	return obOutput;
}

inline	CVector		CVector::operator / ( float fDivisor ) const
{
	CVector	obOutput;
	__m128	qScalar = _mm_set1_ps( 1.0f / fDivisor );
	obOutput.Quadword() = SIMDMul( this->QuadwordValue(), qScalar );
	return obOutput;
}

// ---- Friends

inline	CVector		operator * ( float fScalar, const CVector& obVector )
{
	CVector	obOutput;
	__m128	qScalar = _mm_set1_ps( fScalar );
	obOutput.Quadword() = CVectorBase::SIMDMul( obVector.QuadwordValue(), qScalar );
	return obOutput;
}

// ---- Const operations

inline	float	CVector::Length( void ) const
{
	float	fOutput;
	__m128	qTemp = CVectorBase::SIMDDp4( QuadwordValue(), QuadwordValue() );
	qTemp	= _mm_sqrt_ss( qTemp );												
	_mm_store_ss( &fOutput, qTemp );
	return fOutput;
}

inline	float	CVector::LengthSquared( void ) const
{
	float	fOutput;
	__m128	qTemp = CVectorBase::SIMDDp4( QuadwordValue(), QuadwordValue() );
	_mm_store_ss( &fOutput, qTemp );
	return fOutput;
}

inline	float	CVector::Dot( const CVector& obVector ) const
{
	float	fOutput;
	__m128	qTemp = CVectorBase::SIMDDp4( QuadwordValue(), obVector.QuadwordValue() );
	_mm_store_ss( &fOutput, qTemp );
	return fOutput;
}

inline	CVector	CVector::Abs( void ) const
{
	CVector	obOutput;
	obOutput.Quadword() = _mm_and_ps( QuadwordValue(), *( __m128* )&CVecMath::m_gauiMaskOffSign );
	return obOutput;
}

inline	CVector	CVector::Min( const CVector& obVector ) const
{
	CVector	obOutput;
	obOutput.Quadword() = SIMDMin( QuadwordValue(), obVector.QuadwordValue() );
	return obOutput;
}

inline	CVector	CVector::Max( const CVector& obVector ) const
{
	CVector	obOutput;
	obOutput.Quadword() = SIMDMax( QuadwordValue(), obVector.QuadwordValue() );
	return obOutput;
}

// ---- Non-Const operations

// ---- Static operations

inline	CVector	CVector::Lerp( const CVector& obVector0, const CVector& obVector1, float fT )
{
	return ( obVector0 + ( ( obVector1 - obVector0 ) * fT ) );
}


/***************************************************************************************************
*
*	CLASS			CDirection	-	Inlines
*
***************************************************************************************************/

// ---- Constructors

inline	CDirection::CDirection()
{
}

inline	CDirection::CDirection( CLEAR_CONSTRUCT_MODE )
{
	Clear();
}

inline	CDirection::CDirection( const float* pfData )
{
	ntAssert( pfData );
	Quadword() = _mm_setr_ps( pfData[ 0 ], pfData[ 1 ], pfData[ 2 ], 0.0f );
}

inline	CDirection::CDirection( float fX, float fY, float fZ )
{
	ALIGNTO_PREFIX(16) float ALIGNTO_POSTFIX(16)	afConstruct[ 4 ] = { fX, fY, fZ, 0.0f };
	Quadword() = _mm_load_ps( afConstruct );
}

inline	CDirection::CDirection( const CDirection& obDirection )
{
	Quadword() = obDirection.QuadwordValue();
}

inline	CDirection::CDirection( const CPoint& obPoint )
{
	Quadword() = obPoint.QuadwordValue();
}

inline	CDirection::CDirection( const CVector& obVector )
{
	Quadword() = obVector.QuadwordValue();
}

// ---- Assignment

inline	CDirection&	CDirection::operator = ( const CDirection& obDirection )
{
	Quadword() = obDirection.QuadwordValue();
	return *this;
}

inline	CDirection&	CDirection::operator += ( const CDirection& obDirection )
{
	Quadword() = SIMDAdd( this->QuadwordValue(), obDirection.QuadwordValue() );
	return *this;
}

inline	CDirection&	CDirection::operator -= ( const CDirection& obDirection )
{
	Quadword() = SIMDSub( this->QuadwordValue(), obDirection.QuadwordValue() );
	return *this;
}

inline	CDirection&	CDirection::operator *= ( const CDirection& obDirection )
{
	Quadword() = SIMDMul( this->QuadwordValue(), obDirection.QuadwordValue() );
	return *this;
}

inline	CDirection&	CDirection::operator *= ( float fScalar )
{
	__m128	qScalar = _mm_set1_ps( fScalar );
	Quadword() = SIMDMul( this->QuadwordValue(), qScalar );
	return *this;
}

inline	CDirection&	CDirection::operator /= ( float fDivisor )
{
	__m128	qScalar = _mm_set1_ps( 1.0f / fDivisor );
	Quadword() = SIMDMul( this->QuadwordValue(), qScalar );
	return *this;
}

// ---- Unary operators

inline	CDirection		CDirection::operator + () const
{
	return *this;
}

inline	CDirection		CDirection::operator - () const
{
	CDirection	obOutput;
	obOutput.Quadword() = SIMDSub( _mm_setzero_ps(), QuadwordValue() );
	return obOutput;
}

// ---- Binary operators

inline	CDirection		CDirection::operator + ( const CDirection& obDirection ) const
{
	CDirection obOutput;
	obOutput.Quadword() = SIMDAdd( this->QuadwordValue(), obDirection.QuadwordValue() );
	return obOutput;
}

inline	CDirection		CDirection::operator - ( const CDirection& obDirection ) const
{
	CDirection obOutput;
	obOutput.Quadword() = SIMDSub( this->QuadwordValue(), obDirection.QuadwordValue() );
	return obOutput;
}

inline	CDirection		CDirection::operator * ( const CDirection& obDirection ) const
{
	CDirection obOutput;
	obOutput.Quadword() = SIMDMul( this->QuadwordValue(), obDirection.QuadwordValue() );
	return obOutput;
}

inline	CDirection		CDirection::operator * ( float fScalar ) const
{
	CDirection	obOutput;
	__m128	qScalar = _mm_set1_ps( fScalar );
	obOutput.Quadword() = SIMDMul( this->QuadwordValue(), qScalar );
	return obOutput;
}

inline	CDirection		CDirection::operator / ( float fDivisor ) const
{
	CDirection	obOutput;
	__m128	qScalar = _mm_set1_ps( 1.0f / fDivisor );
	obOutput.Quadword() = SIMDMul( this->QuadwordValue(), qScalar );
	return obOutput;
}

inline	CPoint		CDirection::operator * ( const CPoint& obPoint ) const
{
	CPoint obOutput;
	obOutput.Quadword() = SIMDMul( this->QuadwordValue(), obPoint.QuadwordValue() );
	return obOutput;
}

inline	CPoint		CDirection::operator + ( const CPoint& obPoint ) const
{
	CPoint obOutput;
	obOutput.Quadword() = SIMDAdd( this->QuadwordValue(), obPoint.QuadwordValue() );
	return obOutput;
}

inline	CPoint		CDirection::operator - ( const CPoint& obPoint ) const
{
	CPoint obOutput;
	obOutput.Quadword() = SIMDSub( this->QuadwordValue(), obPoint.QuadwordValue() );
	return obOutput;
}

// ---- Friends

inline	CDirection		operator * ( float fScalar, const CDirection& obDirection )
{
	CDirection	obOutput;
	__m128	qScalar = _mm_set1_ps( fScalar );
	obOutput.Quadword() = CVectorBase::SIMDMul( obDirection.QuadwordValue(), qScalar );
	return obOutput;
}

// ---- Const operations

inline	float	CDirection::Length( void ) const
{
	float	fOutput;
	__m128	qTemp = CVectorBase::SIMDDp3( QuadwordValue(), QuadwordValue() );
	qTemp	= _mm_sqrt_ss( qTemp );												
	_mm_store_ss( &fOutput, qTemp );
	return fOutput;
}

inline	float	CDirection::LengthSquared( void ) const
{
	float	fOutput;
	__m128	qTemp = CVectorBase::SIMDDp3( QuadwordValue(), QuadwordValue() );
	_mm_store_ss( &fOutput, qTemp );
	return fOutput;
}

inline	float	CDirection::Dot( const CDirection& obDirection ) const
{
	float	fOutput;
	__m128	qTemp = CVectorBase::SIMDDp3( QuadwordValue(), obDirection.QuadwordValue() );
	_mm_store_ss( &fOutput, qTemp );
	return fOutput;
}

inline	CDirection	CDirection::Abs( void ) const
{
	CDirection	obOutput;
	obOutput.Quadword() = _mm_and_ps( QuadwordValue(), *( __m128* )&CVecMath::m_gauiMaskOffSign );
	return obOutput;
}

inline	CDirection	CDirection::Min( const CDirection& obDirection ) const
{
	CDirection	obOutput;
	obOutput.Quadword() = SIMDMin( QuadwordValue(), obDirection.QuadwordValue() );
	return obOutput;
}

inline	CDirection	CDirection::Max( const CDirection& obDirection ) const
{
	CDirection	obOutput;
	obOutput.Quadword() = SIMDMax( QuadwordValue(), obDirection.QuadwordValue() );
	return obOutput;
}

inline	CDirection	CDirection::Cross( const CDirection& obDirection1 ) const
{
	// Implemented as per Simon's observation that we can do the cross product in y, z, x order and save
	// two shuffles..

	__m128		qShuffledDir0;
	__m128		qShuffledDir1;
	CDirection	obOutput;

	qShuffledDir0 = _mm_shuffle_ps( this->QuadwordValue(), this->QuadwordValue(), _MM_SSE_SHUFFLE( 1, 2, 0, 3 ) );
	qShuffledDir1 = _mm_shuffle_ps( obDirection1.QuadwordValue(), obDirection1.QuadwordValue(), _MM_SSE_SHUFFLE( 1, 2, 0, 3 ) );

	obOutput.Quadword() = _mm_mul_ps( this->QuadwordValue(), qShuffledDir1 );
	obOutput.Quadword() = _mm_sub_ps( obOutput.Quadword(), _mm_mul_ps( qShuffledDir0, obDirection1.QuadwordValue() ) );

	obOutput.Quadword() = _mm_shuffle_ps( obOutput.Quadword(), obOutput.Quadword(), _MM_SSE_SHUFFLE( 1, 2, 0, 3 ) );

	return obOutput;
}

// ---- Non-Const operations


// ---- Static operations

inline	CDirection	CDirection::Lerp( const CDirection& obDirection0, const CDirection& obDirection1, float fT )
{
	return ( obDirection0 + ( ( obDirection1 - obDirection0 ) * fT ) );
}

// ---- Comparison operation

inline	bool	CDirection::Compare( const CDirection& obDirection, float fSquaredTolerance ) const
{
	float	fOutput;
	__m128	qTemp;

	qTemp	= CVectorBase::SIMDSub( obDirection.QuadwordValue(), QuadwordValue() );
	qTemp	= CVectorBase::SIMDDp3( qTemp, qTemp );
	_mm_store_ss( &fOutput, qTemp );

	if ( fOutput < fSquaredTolerance )
		return true;
	else
		return false;
}


/***************************************************************************************************
*
*	CLASS			CPoint	-	Inlines
*
***************************************************************************************************/

// ---- Constructors

inline	CPoint::CPoint()
{
}

inline	CPoint::CPoint( CLEAR_CONSTRUCT_MODE )
{
	Clear();
}

inline	CPoint::CPoint( const float* pfData )
{
	ntAssert( pfData );
	Quadword() = _mm_setr_ps( pfData[ 0 ], pfData[ 1 ], pfData[ 2 ], 0.0f );
}

inline	CPoint::CPoint( float fX, float fY, float fZ )
{
	ALIGNTO_PREFIX(16)float	ALIGNTO_POSTFIX(16) afConstruct[ 4 ] = { fX, fY, fZ, 0.0f } ;
	Quadword() = _mm_load_ps( afConstruct );
}

inline	CPoint::CPoint( const CPoint& obPoint )
{
	Quadword() = obPoint.QuadwordValue();
}

inline	CPoint::CPoint( const CDirection& obDirection )
{
	Quadword() = obDirection.QuadwordValue();
}

inline	CPoint::CPoint( const CVector& obVector )
{
	Quadword() = obVector.QuadwordValue();
}

// ---- Assignment

inline	CPoint&	CPoint::operator = ( const CPoint& obPoint )
{
	Quadword() = obPoint.QuadwordValue();
	return *this;
}

inline	CPoint&	CPoint::operator += ( const CPoint& obPoint )
{
	Quadword() = SIMDAdd( this->QuadwordValue(), obPoint.QuadwordValue() );
	return *this;
}

inline	CPoint&	CPoint::operator -= ( const CPoint& obPoint )
{
	Quadword() = SIMDSub( this->QuadwordValue(), obPoint.QuadwordValue() );
	return *this;
}

inline	CPoint&	CPoint::operator *= ( const CPoint& obPoint )
{
	Quadword() = SIMDMul( this->QuadwordValue(), obPoint.QuadwordValue() );
	return *this;
}

inline	CPoint&	CPoint::operator += ( const CDirection& obDirection )
{
	Quadword() = SIMDAdd( this->QuadwordValue(), obDirection.QuadwordValue() );
	return *this;
}

inline	CPoint&	CPoint::operator -= ( const CDirection& obDirection )
{
	Quadword() = SIMDSub( this->QuadwordValue(), obDirection.QuadwordValue() );
	return *this;
}

inline	CPoint&	CPoint::operator *= ( float fScalar )
{
	__m128	qScalar = _mm_set1_ps( fScalar );
	Quadword() = SIMDMul( this->QuadwordValue(), qScalar );
	return *this;
}

inline	CPoint&	CPoint::operator /= ( float fDivisor )
{
	__m128	qScalar = _mm_set1_ps( 1.0f / fDivisor );
	Quadword() = SIMDMul( this->QuadwordValue(), qScalar );
	return *this;
}

// ---- Unary operators

inline	CPoint		CPoint::operator + () const
{
	return *this;
}

inline	CPoint		CPoint::operator - () const
{
	CPoint	obOutput;
	obOutput.Quadword() = SIMDSub( _mm_setzero_ps(), QuadwordValue() );
	return obOutput;
}

// ---- Binary operators

inline	CPoint		CPoint::operator + ( const CPoint& obPoint ) const
{
	CPoint obOutput;
	obOutput.Quadword() = SIMDAdd( this->QuadwordValue(), obPoint.QuadwordValue() );
	return obOutput;
}

inline	CPoint		CPoint::operator - ( const CPoint& obPoint ) const
{
	CPoint obOutput;
	obOutput.Quadword() = SIMDSub( this->QuadwordValue(), obPoint.QuadwordValue() );
	return obOutput;
}

inline	CPoint		CPoint::operator * ( const CPoint& obPoint ) const
{
	CPoint obOutput;
	obOutput.Quadword() = SIMDMul( this->QuadwordValue(), obPoint.QuadwordValue() );
	return obOutput;
}

inline	CPoint		CPoint::operator * ( float fScalar ) const
{
	CPoint	obOutput;
	__m128	qScalar = _mm_set1_ps( fScalar );
	obOutput.Quadword() = SIMDMul( this->QuadwordValue(), qScalar );
	return obOutput;
}

inline	CPoint		CPoint::operator / ( float fDivisor ) const
{
	CPoint	obOutput;
	__m128	qScalar = _mm_set1_ps( 1.0f / fDivisor );
	obOutput.Quadword() = SIMDMul( this->QuadwordValue(), qScalar );
	return obOutput;
}

inline	CPoint		CPoint::operator * ( const CDirection& obDirection ) const
{
	CPoint obOutput;
	obOutput.Quadword() = SIMDMul( this->QuadwordValue(), obDirection.QuadwordValue() );
	return obOutput;
}

inline	CPoint		CPoint::operator + ( const CDirection& obDirection ) const
{
	CPoint obOutput;
	obOutput.Quadword() = SIMDAdd( this->QuadwordValue(), obDirection.QuadwordValue() );
	return obOutput;
}

inline	CPoint		CPoint::operator - ( const CDirection& obDirection ) const
{
	CPoint obOutput;
	obOutput.Quadword() = SIMDSub( this->QuadwordValue(), obDirection.QuadwordValue() );
	return obOutput;
}

// ---- Special Binary operators

inline	CDirection	CPoint::operator ^ ( const CPoint& obPoint ) const
{
	CDirection obOutput;
	obOutput.Quadword() = SIMDSub( this->QuadwordValue(), obPoint.QuadwordValue() );
	return obOutput;
}

// ---- Friends

inline	CPoint		operator * ( float fScalar, const CPoint& obPoint )
{
	CPoint	obOutput;
	__m128	qScalar = _mm_set1_ps( fScalar );
	obOutput.Quadword() = CVectorBase::SIMDMul( obPoint.QuadwordValue(), qScalar );
	return obOutput;
}

// ---- Const operations

inline	float	CPoint::Length( void ) const
{
	float	fOutput;
	__m128	qTemp = CVectorBase::SIMDDp3( QuadwordValue(), QuadwordValue() );
	qTemp	= _mm_sqrt_ss( qTemp );												
	_mm_store_ss( &fOutput, qTemp );
	return fOutput;
}

inline	float	CPoint::LengthSquared( void ) const
{
	float	fOutput;
	__m128	qTemp = CVectorBase::SIMDDp3( QuadwordValue(), QuadwordValue() );
	_mm_store_ss( &fOutput, qTemp );
	return fOutput;
}

inline	float	CPoint::Dot( const CPoint& obPoint ) const
{
	float	fOutput;
	__m128	qTemp = CVectorBase::SIMDDp3( QuadwordValue(), obPoint.QuadwordValue() );
	_mm_store_ss( &fOutput, qTemp );
	return fOutput;
}

inline	float	CPoint::Dot( const CDirection &direction ) const
{
	float	fOutput;
	__m128	qTemp = CVectorBase::SIMDDp3( QuadwordValue(), direction.QuadwordValue() );
	_mm_store_ss( &fOutput, qTemp );
	return fOutput;
}

inline	CPoint	CPoint::Abs( void ) const
{
	CPoint	obOutput;
	obOutput.Quadword() = _mm_and_ps( QuadwordValue(), *( __m128* )&CVecMath::m_gauiMaskOffSign );
	return obOutput;
}

inline	CPoint	CPoint::Min( const CPoint& obPoint ) const
{
	CPoint	obOutput;
	obOutput.Quadword() = SIMDMin( QuadwordValue(), obPoint.QuadwordValue() );
	return obOutput;
}

inline	CPoint	CPoint::Max( const CPoint& obPoint ) const
{
	CPoint	obOutput;
	obOutput.Quadword() = SIMDMax( QuadwordValue(), obPoint.QuadwordValue() );
	return obOutput;
}

// ---- Non-Const operations

// ---- Static operations

inline	CPoint	CPoint::Lerp( const CPoint& obPoint0, const CPoint& obPoint1, float fT ) 
{
	return ( obPoint0 + ( ( obPoint1 - obPoint0 ) * fT ) );
}

// ---- Comparison operation

inline	bool	CPoint::Compare( const CPoint& obPoint, float fSquaredTolerance ) const
{
	float	fOutput;
	__m128	qTemp;

	qTemp	= CVectorBase::SIMDSub( obPoint.QuadwordValue(), QuadwordValue() );
	qTemp	= CVectorBase::SIMDDp3( qTemp, qTemp );
	_mm_store_ss( &fOutput, qTemp );

	if ( fOutput < fSquaredTolerance )
		return true;
	else
		return false;
}

/***************************************************************************************************
*
*	CLASS			CQuat	-	Inlines
*
***************************************************************************************************/

// ---- Construction

inline	CQuat::CQuat()
{
	Quadword() = _mm_set1_ps( 1.0f );
}

inline	CQuat::CQuat( CLEAR_CONSTRUCT_MODE )
{
	Clear();
}

inline	CQuat::CQuat( IDENTITY_CONSTRUCT_MODE )
{
	// hououou, nasty bug...
	//Quadword() = _mm_set_ss( 1.0f );
	SetIdentity();
}

inline	CQuat::CQuat( const float* pfData )
{
	ntAssert( pfData );
	Quadword() = _mm_setr_ps( pfData[ 0 ], pfData[ 1 ], pfData[ 2 ], pfData[ 3 ] );
}

inline	CQuat::CQuat( float fX, float fY, float fZ, float fC )
{
	ALIGNTO_PREFIX(16) float ALIGNTO_POSTFIX(16)	afConstruct[ 4 ] = { fX, fY, fZ, fC } ;
	Quadword() = _mm_load_ps( afConstruct );
}

inline	CQuat::CQuat( const CQuat& obQuat )
{
	Quadword() = obQuat.QuadwordValue();
}

inline	CQuat::CQuat( const CVector& obVector )
{
	Quadword() = obVector.QuadwordValue();
}

inline	CQuat::CQuat( const CDirection& obRotateFrom, const CDirection& obRotateTo )
{
	*this = RotationBetween( obRotateFrom, obRotateTo );
}

inline	void	CQuat::SetIdentity( void )
{
	__m128 qOne = _mm_set_ss( 1.0f );
	Quadword() = _mm_shuffle_ps( qOne, qOne, _MM_SSE_SHUFFLE( 1, 1, 1, 0 ) );
}


// ---- Assignment

inline	CQuat&	CQuat::operator = ( const CQuat& obQuat )
{
	Quadword() = obQuat.QuadwordValue();
	return *this;
}

inline	CQuat&	CQuat::operator += ( const CQuat& obQuat )
{
	Quadword() = SIMDAdd( this->QuadwordValue(), obQuat.QuadwordValue() );
	return *this;
}

inline	CQuat&	CQuat::operator -= ( const CQuat& obQuat )
{
	Quadword() = SIMDSub( this->QuadwordValue(), obQuat.QuadwordValue() );
	return *this;
}

inline	CQuat&	CQuat::operator *= ( const CQuat& obQuat )
{	
	CQuat	obThis = *this;

	X() = ( obThis.W() * obQuat.X() ) + ( obThis.X() * obQuat.W() ) + ( obThis.Y() * obQuat.Z() ) - ( obThis.Z() * obQuat.Y() );
	Y() = ( obThis.W() * obQuat.Y() ) - ( obThis.X() * obQuat.Z() ) + ( obThis.Y() * obQuat.W() ) + ( obThis.Z() * obQuat.X() );
	Z() = ( obThis.W() * obQuat.Z() ) + ( obThis.X() * obQuat.Y() ) - ( obThis.Y() * obQuat.X() ) + ( obThis.Z() * obQuat.W() );
	W() = ( obThis.W() * obQuat.W() ) - ( obThis.X() * obQuat.X() ) - ( obThis.Y() * obQuat.Y() ) - ( obThis.Z() * obQuat.Z() );
	return *this;
}

inline	CQuat&	CQuat::operator *= ( float fScalar )
{
	__m128	qScalar = _mm_set1_ps( fScalar );
	Quadword() = SIMDMul( this->QuadwordValue(), qScalar );
	return *this;
}

inline	CQuat&	CQuat::operator /= ( float fDivisor )
{
	__m128	qScalar = _mm_set1_ps( 1.0f / fDivisor );
	Quadword() = SIMDMul( this->QuadwordValue(), qScalar );
	return *this;
}

// ---- Unary operators

inline	CQuat		CQuat::operator + () const
{
	return *this;
}

inline	CQuat		CQuat::operator - () const
{
	CQuat	obOutput;
	obOutput.Quadword() = SIMDSub( _mm_setzero_ps(), QuadwordValue() );
	return obOutput;
}

inline	CQuat		CQuat::operator ~ () const
{
	CQuat	obOutput;
	obOutput.Quadword() = _mm_xor_ps( QuadwordValue(), *( __m128* )&CVecMath::m_gauiQuatConjugateMask );
	return obOutput;
}

// ---- Binary operators

inline	CQuat		CQuat::operator + ( const CQuat& obQuat ) const
{
	CQuat obOutput;
	obOutput.Quadword() = SIMDAdd( this->QuadwordValue(), obQuat.QuadwordValue() );
	return obOutput;
}

inline	CQuat		CQuat::operator - ( const CQuat& obQuat ) const
{
	CQuat obOutput;
	obOutput.Quadword() = SIMDSub( this->QuadwordValue(), obQuat.QuadwordValue() );
	return obOutput;
}

inline	CQuat		CQuat::operator * ( const CQuat& obQuat ) const
{
	CQuat	obOutput;

	obOutput.X() = ( W() * obQuat.X() ) + ( X() * obQuat.W() ) + ( Y() * obQuat.Z() ) - ( Z() * obQuat.Y() );
	obOutput.Y() = ( W() * obQuat.Y() ) - ( X() * obQuat.Z() ) + ( Y() * obQuat.W() ) + ( Z() * obQuat.X() );
	obOutput.Z() = ( W() * obQuat.Z() ) + ( X() * obQuat.Y() ) - ( Y() * obQuat.X() ) + ( Z() * obQuat.W() );
	obOutput.W() = ( W() * obQuat.W() ) - ( X() * obQuat.X() ) - ( Y() * obQuat.Y() ) - ( Z() * obQuat.Z() );
	return obOutput;
}

inline	CQuat		CQuat::operator * ( float fScalar ) const
{
	CQuat	obOutput;
	__m128	qScalar = _mm_set1_ps( fScalar );
	obOutput.Quadword() = SIMDMul( this->QuadwordValue(), qScalar );
	return obOutput;
}

inline	CQuat		CQuat::operator / ( float fDivisor ) const
{
	CQuat	obOutput;
	__m128	qScalar = _mm_set1_ps( 1.0f / fDivisor );
	obOutput.Quadword() = SIMDMul( this->QuadwordValue(), qScalar );
	return obOutput;
}

// ---- Friends

inline	CQuat		operator * ( float fScalar, const CQuat& obQuat )
{
	CQuat	obOutput;
	__m128	qScalar = _mm_set1_ps( fScalar );
	obOutput.Quadword() = CVectorBase::SIMDMul( obQuat.QuadwordValue(), qScalar );
	return obOutput;
}

// ---- Const operations

inline	float	CQuat::Dot( const CQuat& obQuat ) const
{
	float	fOutput;
	__m128	qTemp = CVectorBase::SIMDDp4( QuadwordValue(), obQuat.QuadwordValue() );
	_mm_store_ss( &fOutput, qTemp );
	return fOutput;
}

/***************************************************************************************************
*
*	CLASS			CMatrix	-	Inlines
*
***************************************************************************************************/

// ---- Construction

inline	CMatrix::CMatrix()
{
}

inline	CMatrix::CMatrix( CLEAR_CONSTRUCT_MODE )
{
	Row( 0 ).Clear();
	Row( 1 ).Clear();
	Row( 2 ).Clear();
	Row( 3 ).Clear();
}

inline	CMatrix::CMatrix( IDENTITY_CONSTRUCT_MODE )
{
	SetIdentity();
}

inline	CMatrix::CMatrix( const float* pfData )
{
	ntAssert( pfData );
	Row( 0 ) = CVector( pfData + 0x00 );
	Row( 1 ) = CVector( pfData + 0x04 );
	Row( 2 ) = CVector( pfData + 0x08 );
	Row( 3 ) = CVector( pfData + 0x0c );
}

inline	CMatrix::CMatrix( const CMatrix& obMatrix )
{
	Row( 0 ) = obMatrix.Row( 0 );
	Row( 1 ) = obMatrix.Row( 1 );
	Row( 2 ) = obMatrix.Row( 2 );
	Row( 3 ) = obMatrix.Row( 3 );
}

inline	CMatrix::CMatrix(	float fVal00, float fVal01, float fVal02, float fVal03,
							float fVal10, float fVal11, float fVal12, float fVal13,
							float fVal20, float fVal21, float fVal22, float fVal23,
							float fVal30, float fVal31, float fVal32, float fVal33 )
{
	Row( 0 ) = CVector( fVal00, fVal01, fVal02, fVal03 );
	Row( 1 ) = CVector( fVal10, fVal11, fVal12, fVal13 );
	Row( 2 ) = CVector( fVal20, fVal21, fVal22, fVal23 );
	Row( 3 ) = CVector( fVal30, fVal31, fVal32, fVal33 );
}


inline	CMatrix&	CMatrix::operator = ( const CMatrix& obMatrix )
{
	Row( 0 ) = obMatrix.Row( 0 );
	Row( 1 ) = obMatrix.Row( 1 );
	Row( 2 ) = obMatrix.Row( 2 );
	Row( 3 ) = obMatrix.Row( 3 );
	return *this;
}


// ---- Initialisation

inline	void	CMatrix::Clear( void )
{
	Row( 0 ).Clear();
	Row( 1 ).Clear();
	Row( 2 ).Clear();
	Row( 3 ).Clear();
}

inline	void	CMatrix::SetIdentity( void )
{
	Row( 0 ) = *((CVector*)(&CVecMath::m_gauiIdentityMatrix[ 0 ]));
	Row( 1 ) = *((CVector*)(&CVecMath::m_gauiIdentityMatrix[ 4]));
	Row( 2 ) = *((CVector*)(&CVecMath::m_gauiIdentityMatrix[ 8 ]));
	Row( 3 ) = *((CVector*)(&CVecMath::m_gauiIdentityMatrix[ 12 ]));
}

// ---- Casting

inline	CMatrix::operator float* ()
{
	return &Row( 0 )[ 0 ];
}

inline	CMatrix::operator const float* () const
{
	return &Row( 0 )[ 0 ];
}

// ---- Static operations

inline CMatrix CMatrix::Lerp( const CMatrix& obMatrix0, const CMatrix& obMatrix1, float fT )
{
	// Create a return matrix
	CMatrix obLerpMatrix;

	// Lerp each row individually
	obLerpMatrix.Row( 0 ) = CVector::Lerp( obMatrix0.Row( 0 ), obMatrix1.Row( 0 ), fT );
	obLerpMatrix.Row( 1 ) = CVector::Lerp( obMatrix0.Row( 1 ), obMatrix1.Row( 1 ), fT );
	obLerpMatrix.Row( 2 ) = CVector::Lerp( obMatrix0.Row( 2 ), obMatrix1.Row( 2 ), fT );
	obLerpMatrix.Row( 3 ) = CVector::Lerp( obMatrix0.Row( 3 ), obMatrix1.Row( 3 ), fT );

	return obLerpMatrix;
}

// ---- Accessors

inline	const CVector& CMatrix::operator[] ( int iIndex ) const
{
	ntAssert( iIndex < 4 );
	return m_aobRow[ iIndex ];
}

inline	CVector&	CMatrix::operator[] ( int iIndex )
{
	ntAssert( iIndex < 4 );
	return m_aobRow[ iIndex ];
}

// ---- Direct Vector Accessors

inline	const CVector&	CMatrix::Row( int iRow ) const
{
	ntAssert( iRow < 4 );
	return m_aobRow[ iRow ];
}

inline	CVector&	CMatrix::Row( int iRow )
{
	ntAssert( iRow < 4 );
	return m_aobRow[ iRow ];
}

// ---- Accessors for local axes and translation
//		Note that the Set<thing>() functions set 4th column present in the CMatrix object to 
//		values that are valid for an affine matrix. 

inline	const	CDirection&	CMatrix::GetXAxis() const
{
	return *( CDirection* )&Row( 0 );
}

inline	void	CMatrix::SetXAxis( const CDirection& obXAxis )
{
	Row( 0 ).Quadword() = _mm_and_ps( obXAxis.QuadwordValue(), *( __m128* )&CVecMath::m_gauiMaskOffW );
}

inline	const	CDirection&	CMatrix::GetYAxis() const
{
	return *( CDirection* )&Row( 1 );
}

inline	void	CMatrix::SetYAxis( const CDirection& obYAxis )
{
	Row( 1 ).Quadword() = _mm_and_ps( obYAxis.QuadwordValue(), *( __m128* )&CVecMath::m_gauiMaskOffW );
}

inline	const	CDirection&	CMatrix::GetZAxis() const
{
	return *( CDirection* )&Row( 2 );
}

inline	void	CMatrix::SetZAxis( const CDirection& obZAxis )
{
	Row( 2 ).Quadword() = _mm_and_ps( obZAxis.QuadwordValue(), *( __m128* )&CVecMath::m_gauiMaskOffW );
}

inline	const	CPoint&	CMatrix::GetTranslation() const
{
	return *( CPoint* )&Row( 3 );
}

inline	void	CMatrix::SetTranslation( const CPoint& obTranslation )
{
	Row( 3 ).Quadword() = _mm_or_ps( _mm_and_ps( obTranslation.QuadwordValue(), *( __m128* )&CVecMath::m_gauiMaskOffW ), *( __m128* )&CVecMath::m_gauiOneInW );
}

// ---- Axis Creation

inline	void	CMatrix::BuildXAxis( void )
{
	CDirection obXAxis = GetYAxis().Cross( GetZAxis() );
	obXAxis.Normalise();
	SetXAxis( obXAxis );
}

inline	void	CMatrix::BuildYAxis( void )
{
	CDirection obYAxis = GetZAxis().Cross( GetXAxis() );
	obYAxis.Normalise();
	SetYAxis( obYAxis );
}

inline	void	CMatrix::BuildZAxis( void )
{
	CDirection obZAxis = GetXAxis().Cross( GetYAxis() );
	obZAxis.Normalise();
	SetZAxis( obZAxis );
}


#endif	//_VECMATH_PC_H
