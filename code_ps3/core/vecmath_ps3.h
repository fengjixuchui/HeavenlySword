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

#ifndef	_VECMATH_PS3_H
#define	_VECMATH_PS3_H

// for now emulate SSE on PS3
#include "core/sse.h"

#include "core/maths.h"

//#ifdef _DEBUG
//#define	_DEBUG_VECMATH					// Enables isnan() checking on all floats in vector class.
//#endif

// altivec/vmx register type definition..
typedef	vector float v128;
typedef const v128& v128_arg;


class	CVectorBase;
class	CVector;
class	CMatrix;

class	CDirection;
class	CPoint;
class	CQuat;

// Check if a float value makes sense
inline bool IsNan(float f)
{
	union { float f_; unsigned int ui_; } data;
	data.f_ = f;
	unsigned int u = data.ui_ & 0x7f800000;

	return (u == 0x7f800000) || fabs(f) > 5000000.f;
}



class	CVecMath
{
	friend	class CVectorBase;
	friend	class CVector;
	friend	class CDirection;
	friend	class CPoint;
	friend	class CQuat;
	friend	class CMatrix;
	
public:

	static	const	CDirection&	GetXAxis( void )			{return *( ( CDirection* )&m_gauiIdentityMatrix[ 0 ] ); }
	static	const	CDirection&	GetYAxis( void )			{return *( ( CDirection* )&m_gauiIdentityMatrix[ 1 ] ); }
	static	const	CDirection&	GetZAxis( void )			{return *( ( CDirection* )&m_gauiIdentityMatrix[ 2 ] ); }
	static	const	CQuat&		GetQuatIdentity( void )		{return *( ( CQuat* )&m_gauiIdentityMatrix[ 3 ] ); }
	static	const	CMatrix&	GetIdentity( void )			{return *( ( CMatrix* )&m_gauiIdentityMatrix ); }
	static	const	CVector&	GetZeroVector( void )		{return *( ( CVector* )&m_gauiZero ); }
	static	const	CDirection&	GetZeroDirection( void )	{return *( ( CDirection* )&m_gauiZero ); };
	static	const	CPoint&		GetZeroPoint( void )		{return *( ( CPoint* )&m_gauiZero ); } 

	ALIGNTO_PREFIX(16) static	const	vector float	m_vZero ALIGNTO_POSTFIX(16);
	ALIGNTO_PREFIX(16) static	const	vector float	m_vOne  ALIGNTO_POSTFIX(16);	

protected:
	ALIGNTO_PREFIX(16) static	const	vector unsigned int	m_gauiIdentityMatrix[4] ALIGNTO_POSTFIX(16);	// Identity matrix. Also used to get individual axes
	ALIGNTO_PREFIX(16) static	const	vector unsigned int	m_gauiZero ALIGNTO_POSTFIX(16);				// Zero vector/direction/point

	ALIGNTO_PREFIX(16) static	const	vector unsigned int	m_gauiMaskOffW  ALIGNTO_POSTFIX(16);			// All components except W contain 0xffffffff
	ALIGNTO_PREFIX(16) static	const	vector unsigned int	m_gauiOneInW ALIGNTO_POSTFIX(16);				// All components hold 0x00000000, except W which holds 0x3f800000
	ALIGNTO_PREFIX(16) static	const	vector unsigned int	m_gauiMaskOffSign  ALIGNTO_POSTFIX(16);			// Each component holds 0x7fffffff
	ALIGNTO_PREFIX(16) static	const	vector unsigned int	m_gauiInvalidValue  ALIGNTO_POSTFIX(16);		// Each component holds QNaN equivalent (0xffffffff)
	ALIGNTO_PREFIX(16) static	const	vector unsigned int	m_gauiQuatConjugateMask  ALIGNTO_POSTFIX(16);	// Each component holds 0x80000000, except W which holds 0x00000000

	ALIGNTO_PREFIX(16) static	const	vector unsigned int	m_gauiPNPNMask ALIGNTO_POSTFIX(16);			// Mask with positive/negative/positive/negative sign control bits
	ALIGNTO_PREFIX(16) static	const	vector unsigned int	m_gauiNPNPMask ALIGNTO_POSTFIX(16);			// Mask with negative/positive/negative/positive sign control bits

	ALIGNTO_PREFIX(16) static	const	vector unsigned char m_vShuffleYZXX ALIGNTO_POSTFIX(16);	
	ALIGNTO_PREFIX(16) static	const	vector unsigned char m_vShuffleWZYX ALIGNTO_POSTFIX(16);	
	ALIGNTO_PREFIX(16) static	const	vector unsigned char m_vShuffleZWXY ALIGNTO_POSTFIX(16);	
	ALIGNTO_PREFIX(16) static	const	vector unsigned char m_vShuffleYXWZ ALIGNTO_POSTFIX(16);	

	ALIGNTO_PREFIX(16) static	const	vector unsigned char m_vShuffleWzYx ALIGNTO_POSTFIX(16);	
	ALIGNTO_PREFIX(16) static	const	vector unsigned char m_vShuffleZWxy ALIGNTO_POSTFIX(16);	
	ALIGNTO_PREFIX(16) static	const	vector unsigned char m_vShuffleyXWz ALIGNTO_POSTFIX(16);	


	ALIGNTO_PREFIX(16) static	const	vector float m_vMulMaskPNPN ALIGNTO_POSTFIX(16);	
	ALIGNTO_PREFIX(16) static	const	vector float m_vMulMaskPPNN ALIGNTO_POSTFIX(16);	
	ALIGNTO_PREFIX(16) static	const	vector float m_vMulMaskNPPN ALIGNTO_POSTFIX(16);	


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

	friend	CVector	operator * ( const CVector& obVector, const CMatrix& obMatrix );
	friend	CPoint	operator * ( const CPoint& obPoint, const CMatrix& obMatrix );
	friend	CDirection	operator * ( const CDirection& obDirection, const CMatrix& obMatrix );


public:
	// SIMD Accessors
	v128&			Quadword()							{ return m_vector; };
	v128			QuadwordValue() const	
	{
#ifdef _DEBUG_VECMATH 
		CheckVector();
#endif 
		return m_vector;
	};
	
	// Float Accessors
	const float&	X() const
	{
#ifdef _DEBUG_VECMATH
		CheckVector();
#endif
		const vector_components *vComp = reinterpret_cast<const vector_components *>(&m_vector);
		return vComp->components[0];
	};

	const float&	Y() const
	{
#ifdef _DEBUG_VECMATH
		CheckVector();
#endif
		const vector_components *vComp = reinterpret_cast<const vector_components *>(&m_vector);
		return vComp->components[1];
	};

	const float&	Z() const
	{
#ifdef _DEBUG_VECMATH
		CheckVector();
#endif
		const vector_components *vComp = reinterpret_cast<const vector_components *>(&m_vector);
		return vComp->components[2];
	};

	const float&	W() const
	{
#ifdef _DEBUG_VECMATH
		CheckVector();
#endif
		const vector_components *vComp = reinterpret_cast<const vector_components *>(&m_vector);
		return vComp->components[3];
	};
	// Float Accessors
	float&	X() 
	{
#ifdef _DEBUG_VECMATH
		CheckVector();
#endif
		vector_components *vComp = reinterpret_cast<vector_components *>(&m_vector);
		return vComp->components[0];
	};

	float&	Y() 
	{
#ifdef _DEBUG_VECMATH
		CheckVector();
#endif
		vector_components *vComp = reinterpret_cast<vector_components *>(&m_vector);
		return vComp->components[1];
	};

	float&	Z() 
	{
#ifdef _DEBUG_VECMATH
		CheckVector();
#endif
		vector_components *vComp = reinterpret_cast<vector_components *>(&m_vector);
		return vComp->components[2];
	};

	float&	W() 
	{
#ifdef _DEBUG_VECMATH
		CheckVector();
#endif
		vector_components *vComp = reinterpret_cast<vector_components *>(&m_vector);
		return vComp->components[3];
	};
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



	static	v128	SIMDDp3( v128_arg qVector0, v128_arg qVector1 );
	static	v128	SIMDDp4( v128_arg qVector0, v128_arg qVector1 );
	static	v128	SIMDDiv( v128_arg qVector0, v128_arg qVector1 );
	static	v128	SIMDCross( v128_arg qVector0, v128_arg qVector1 );
	static	v128	SIMDRcp( v128_arg qVector0 );
	static	v128	SIMDSqrt( v128_arg qVector0 );
	static	v128	SIMDRSqrt( v128_arg qVector0 );



protected:

	// Initialisation
	void	Clear();
	
	// this union is useful when we need to work on scalar values..
	union	vector_components
	{
		float	components[ 4 ];
		v128	vector;
	};

private:

	v128 m_vector;

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

    CMatrix( CVector const& row1, CVector const& row2, CVector const& row3, CVector const& row4 );

	CMatrix( v128_arg row0, v128_arg row1, v128_arg row2, v128_arg row3 );

	// Explicit Constructors
	explicit	CMatrix( CLEAR_CONSTRUCT_MODE eClear );
	explicit	CMatrix( IDENTITY_CONSTRUCT_MODE eIdentity );
	explicit	CMatrix( const float* pfData );
			
	
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
CMatrix operator * ( CMatrix const& obMatrixL, CMatrix const& obMatrixR );
CVector		operator *	( const CVector& obVector, const CMatrix& obMatrix );
CDirection	operator *	( const CDirection& obDirection, const CMatrix& obMatrix );
CPoint		operator *	( const CPoint& obPoint, const CMatrix& obMatrix );


/***************************************************************************************************
*
*	CLASS			CVectorBase	-	Inlines
*
***************************************************************************************************/

// ---- Constructors

inline	CVectorBase::CVectorBase()
{
#ifdef	_DEBUG_VECMATH
	Quadword() = vec_ld(0, (v128*)&CVecMath::m_gauiInvalidValue );
#endif	//_DEBUG_VECMATH
}

// ---- Initialisation

inline	void CVectorBase::Clear( void )
{
	Quadword() = vec_ld(0, &CVecMath::m_vZero );
}

// ---- Array access

inline	const float& CVectorBase::operator[] ( int iIndex ) const
{
	ntAssert( iIndex < 4 );
	ALIGNTO_PREFIX(16) const vector_components *vComp ALIGNTO_POSTFIX(16) = reinterpret_cast<const vector_components*>(&m_vector);
	return *( float *)( &vComp->components[ iIndex ] );
}

inline	float&	CVectorBase::operator[] ( int iIndex )
{
	ntAssert( iIndex < 4 );
	ALIGNTO_PREFIX(16) const vector_components *vComp ALIGNTO_POSTFIX(16) = reinterpret_cast<const vector_components*>(&m_vector);
	return *( float *)( &vComp->components[ iIndex ] );
}

inline	const float& CVectorBase::Index( int iIndex ) const
{
	ntAssert( iIndex < 4 );
	ALIGNTO_PREFIX(16) const vector_components *vComp ALIGNTO_POSTFIX(16) = reinterpret_cast<const vector_components*>(&m_vector);
	return *( float *)( &vComp->components[ iIndex ] );
}

inline	float&	CVectorBase::Index( int iIndex )
{
	ntAssert( iIndex < 4 );
	ALIGNTO_PREFIX(16) const vector_components *vComp ALIGNTO_POSTFIX(16) = reinterpret_cast<const vector_components*>(&m_vector);
	return *( float *)( &vComp->components[ iIndex ] );
}

inline	void	CVectorBase::SetFromNTColor( uint32_t col )
{
	NTCOLOUR_EXTRACT_FLOATS( col, X(), Y(), Z(), W() );
}

inline	uint32_t	CVectorBase::GetNTColor( void ) const
{
	return NTCOLOUR_FROM_FLOATS( X(), Y(), Z(), W() );	
}



inline	v128	CVectorBase::SIMDDiv( v128_arg qVector0, v128_arg qVector1 )
{
	v128	estimate	= vec_re( qVector1 );
	v128	result;

	// Perform single Newton-Raphson step to increase accuracy of result.
	result	= vec_nmsub( estimate, qVector1, ( v128 )( 1.0f ) );
	result	= vec_madd( result, estimate, estimate );																	

	// We now have reciprocal of vectorArg1, so now multiply to perform the final divide
	result	= vec_madd( result, qVector0, CVecMath::m_vZero );

	return result;
}

inline	v128	CVectorBase::SIMDRcp( v128_arg qVector0 )
{
	v128	estimate	= vec_re( qVector0 );
	v128	result;

	// Perform single Newton-Raphson step to increase accuracy of result.
	result	= vec_nmsub( estimate, qVector0, ( v128 )( 1.0f ) );
	result	= vec_madd( result, estimate, estimate );																	

	return result;
}

inline	v128	CVectorBase::SIMDSqrt( v128_arg qVector0 )
{
	v128	estimate	= vec_rsqrte( qVector0 );
	v128	estSquared	= vec_madd( estimate, estimate, CVecMath::m_vZero );
	v128	result;

	// Perform single Newton-Raphson step to increase accuracy of result.
	result	= vec_nmsub( qVector0, estSquared, ( v128 )( 1.0f ) );
	result	= vec_madd( result, vec_madd( estimate, ( v128 )( 0.5f ), CVecMath::m_vZero  ), estimate );

	// We have reciprocal square root, so now multiply to give 'normal' square root
	result	= vec_madd( result, qVector0, CVecMath::m_vZero  );

	// We have to handle the case where our input component is zero.
	result	= vec_sel( ( v128 )( 0.0f ), result, vec_cmpgt( qVector0, ( v128 )( 0.0f ) ) ); 

	return result;
}


inline	v128	CVectorBase::SIMDRSqrt( v128_arg qVector0 )
{
	v128	estimate	= vec_rsqrte( qVector0 );
	v128	estSquared	= vec_madd( estimate, estimate, CVecMath::m_vZero );
	v128	result;

	// Perform single Newton-Raphson step to increase accuracy of result.
	result	= vec_nmsub( qVector0, estSquared, ( v128 )( 1.0f ) );
	result	= vec_madd( result, vec_madd( estimate, ( v128 )( 0.5f ), CVecMath::m_vZero  ), estimate );

	return result;
}

inline	v128	CVectorBase::SIMDCross( v128_arg qVector0, v128_arg qVector1 )
{
	// Implemented to do the cross product in y, z, x order and save two shuffles..
	v128	shuffled0;
	v128	shuffled1;
	v128	outputVector;

	shuffled0	= vec_perm( qVector0, qVector0, CVecMath::m_vShuffleYZXX );
	shuffled1	= vec_perm( qVector1, qVector1, CVecMath::m_vShuffleYZXX );

	outputVector = vec_madd( qVector0, shuffled1, CVecMath::m_vZero  );
	outputVector = vec_nmsub( shuffled0, qVector1, outputVector );
	outputVector = vec_perm( outputVector, outputVector, CVecMath::m_vShuffleYZXX );

	return outputVector;
}

inline	v128	CVectorBase::SIMDDp3( v128_arg qVector0, v128_arg qVector1 )
{
	v128 qTemp, result;	
	qTemp	= vec_madd( qVector0, qVector1, CVecMath::m_vZero );
	result	= vec_add( qTemp, vec_sld( qTemp, qTemp, 0x04 ) );	
	result	= vec_add( result, vec_sld( qTemp, qTemp, 0x08 ) );
	return result;
}

inline	v128	CVectorBase::SIMDDp4( v128_arg qVector0, v128_arg qVector1 )
{
	v128 qTemp, result;	
	qTemp	= vec_madd( qVector0, qVector1, CVecMath::m_vZero);
	result	= vec_add( qTemp, vec_sld( qTemp, qTemp, 0x04 ) );	
	result	= vec_add( result, vec_sld( qTemp, qTemp, 0x08 ) );
	result	= vec_add( result, vec_sld( qTemp, qTemp, 0x0c ) );
	return result;
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
	ALIGNTO_PREFIX(16) v128  vConstruct ALIGNTO_POSTFIX(16) = { *pfData, *(pfData+1), *(pfData+2), *(pfData+3)};
	Quadword() = vConstruct;
}

inline	CVector::CVector( float fX, float fY, float fZ, float fW )
{
	ALIGNTO_PREFIX(16) v128 vConstruct ALIGNTO_POSTFIX(16) = { fX, fY, fZ, fW };
	Quadword() = vConstruct;
}

inline	CVector::CVector( float fValue )
{
	ALIGNTO_PREFIX(16) v128  vConstruct ALIGNTO_POSTFIX(16) = { fValue, fValue, fValue, fValue };
	Quadword() = vConstruct;
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
	Quadword() = vec_add( QuadwordValue(), obVector.QuadwordValue() );
	return *this;
}

inline	CVector&	CVector::operator -= ( const CVector& obVector )
{
	Quadword() = vec_sub( QuadwordValue(), obVector.QuadwordValue() );
	return *this;
}

inline	CVector&	CVector::operator *= ( const CVector& obVector )
{
	Quadword() = vec_madd( QuadwordValue(), obVector.QuadwordValue(), CVecMath::m_vZero );
	return *this;
}

inline	CVector&	CVector::operator *= ( float fScalar )
{
	ALIGNTO_PREFIX(16) vector_components elems ALIGNTO_POSTFIX(16);
	elems.components[0] = fScalar;
    Quadword() = vec_madd( QuadwordValue(), vec_splat(vec_ld(0, elems.components), 0), CVecMath::m_vZero  );
	return *this;
}

inline	CVector&	CVector::operator /= ( const CVector& obVector )
{	
	v128 qTemp = CVectorBase::SIMDRcp(obVector.QuadwordValue());
	Quadword() = vec_madd( QuadwordValue(), qTemp, CVecMath::m_vZero );
	return *this;
}

inline	CVector&	CVector::operator /= ( float fDivisor )
{
	ALIGNTO_PREFIX(16) vector_components elems ALIGNTO_POSTFIX(16);
	elems.components[0] = fDivisor;
	v128 qTemp = CVectorBase::SIMDRcp(vec_splat(vec_ld(0, elems.components), 0));
	Quadword() = vec_madd( QuadwordValue(), qTemp, CVecMath::m_vZero );
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
	obOutput.Quadword() = vec_sub(CVecMath::m_vZero, QuadwordValue() );
	return obOutput;
}

// ---- Binary operators

inline	CVector		CVector::operator + ( const CVector& obVector ) const
{
	CVector obOutput;
	obOutput.Quadword() = vec_add( QuadwordValue(), obVector.QuadwordValue() );
	return obOutput;
}

inline	CVector		CVector::operator - ( const CVector& obVector ) const
{
	CVector obOutput;
	obOutput.Quadword() = vec_sub( QuadwordValue(), obVector.QuadwordValue() );
	return obOutput;
}

inline	CVector		CVector::operator * ( const CVector& obVector ) const
{
	CVector obOutput;
	obOutput.Quadword() = vec_madd( QuadwordValue(), obVector.QuadwordValue(), CVecMath::m_vZero);
	return obOutput;
}

inline	CVector		CVector::operator * ( float fScalar ) const
{
	CVector	obOutput;
	ALIGNTO_PREFIX(16) vector_components elems ALIGNTO_POSTFIX(16);
	elems.components[0] = fScalar;
    obOutput.Quadword() = vec_madd( QuadwordValue(), vec_splat(vec_ld(0, elems.components), 0), CVecMath::m_vZero  );
	return obOutput;
}

inline	CVector		CVector::operator /	 ( const CVector& obVector ) const
{
	CVector	obOutput;
	v128 qTemp = CVectorBase::SIMDRcp(obVector.QuadwordValue());
	obOutput.Quadword() = vec_madd( QuadwordValue(), qTemp, CVecMath::m_vZero );
	return obOutput;
}

inline	CVector		CVector::operator / ( float fDivisor ) const
{
	CVector	obOutput;
	ALIGNTO_PREFIX(16) vector_components elems ALIGNTO_POSTFIX(16);
	elems.components[0] = fDivisor;
	v128 qTemp = CVectorBase::SIMDRcp(vec_splat(vec_ld(0, elems.components), 0));
	obOutput.Quadword() = vec_madd( QuadwordValue(), qTemp, CVecMath::m_vZero );
	return obOutput;
}

// ---- Friends

inline	CVector		operator * ( float fScalar, const CVector& obVector )
{
	CVector	obOutput;
	ALIGNTO_PREFIX(16) CVectorBase::vector_components elems ALIGNTO_POSTFIX(16);
	elems.components[0] = fScalar;
    obOutput.Quadword() = vec_madd( obVector.QuadwordValue(), vec_splat(vec_ld(0, elems.components), 0), CVecMath::m_vZero  );
	return obOutput;
}

// ---- Const operations

inline	float	CVector::Length( void ) const
{
	vector_components vComp;
	v128	qTemp = CVectorBase::SIMDDp4( QuadwordValue(), QuadwordValue() );
	vComp.vector = CVectorBase::SIMDSqrt(qTemp);
	return vComp.components[0];
}

inline	float	CVector::LengthSquared( void ) const
{
	vector_components vComp;
	v128	qTemp = CVectorBase::SIMDDp4( QuadwordValue(), QuadwordValue() );
	vComp.vector = qTemp;
	return vComp.components[0];
}

inline	float	CVector::Dot( const CVector& obVector ) const
{
	vector_components vComp;
	v128	qTemp = CVectorBase::SIMDDp4( QuadwordValue(), obVector.QuadwordValue() );
	vComp.vector = qTemp;
	return vComp.components[0];
}

inline	CVector	CVector::Abs( void ) const
{
	CVector	obOutput;
	obOutput.Quadword() = vec_and(QuadwordValue(), *( v128* )&CVecMath::m_gauiMaskOffSign );
	return obOutput;
}

inline	CVector	CVector::Min( const CVector& obVector ) const
{
	CVector	obOutput;
	obOutput.Quadword() = vec_min( QuadwordValue(), obVector.QuadwordValue() );
	return obOutput;
}

inline	CVector	CVector::Max( const CVector& obVector ) const
{
	CVector	obOutput;
	obOutput.Quadword() = vec_max( QuadwordValue(), obVector.QuadwordValue() );
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
	ALIGNTO_PREFIX(16) v128 vConstruct  ALIGNTO_POSTFIX(16) = { *pfData, *(pfData+1), *(pfData+2), 0.0f};
	Quadword() = vConstruct;
}

inline	CDirection::CDirection( float fX, float fY, float fZ )
{
	ALIGNTO_PREFIX(16) v128  vConstruct ALIGNTO_POSTFIX(16) = { fX, fY, fZ, 0.0f};
	Quadword() = vConstruct;
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
	Quadword() = vec_add( QuadwordValue(), obDirection.QuadwordValue() );
	return *this;
}

inline	CDirection&	CDirection::operator -= ( const CDirection& obDirection )
{
	Quadword() = vec_sub( QuadwordValue(), obDirection.QuadwordValue() );
	return *this;
}

inline	CDirection&	CDirection::operator *= ( const CDirection& obDirection )
{
	Quadword() = vec_madd( QuadwordValue(), obDirection.QuadwordValue(), CVecMath::m_vZero );
	return *this;
}

inline	CDirection&	CDirection::operator *= ( float fScalar )
{
	CVector	obOutput;
	ALIGNTO_PREFIX(16) vector_components elems ALIGNTO_POSTFIX(16);
	elems.components[0] = fScalar;
    Quadword() = vec_madd( QuadwordValue(), vec_splat(vec_ld(0, elems.components), 0), CVecMath::m_vZero  );
	return *this;
}

inline	CDirection&	CDirection::operator /= ( float fDivisor )
{
	ALIGNTO_PREFIX(16) vector_components elems ALIGNTO_POSTFIX(16);
	elems.components[0] = fDivisor;
	v128 qTemp = CVectorBase::SIMDRcp(vec_splat(vec_ld(0, elems.components), 0));
	Quadword() = vec_madd( QuadwordValue(), qTemp, CVecMath::m_vZero );
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
	obOutput.Quadword() = vec_sub( CVecMath::m_vZero , QuadwordValue() );
	return obOutput;
}

// ---- Binary operators

inline	CDirection		CDirection::operator + ( const CDirection& obDirection ) const
{
	CDirection obOutput;
	obOutput.Quadword() = vec_add( QuadwordValue(), obDirection.QuadwordValue() );
	return obOutput;
}

inline	CDirection		CDirection::operator - ( const CDirection& obDirection ) const
{
	CDirection obOutput;
	obOutput.Quadword() = vec_sub( QuadwordValue(), obDirection.QuadwordValue() );
	return obOutput;
}

inline	CDirection		CDirection::operator * ( const CDirection& obDirection ) const
{
	CDirection obOutput;
	obOutput.Quadword() = vec_madd( QuadwordValue(), obDirection.QuadwordValue(), CVecMath::m_vZero  );
	return obOutput;
}

inline	CDirection		CDirection::operator * ( float fScalar ) const
{
	CDirection	obOutput;
	ALIGNTO_PREFIX(16) vector_components elems ALIGNTO_POSTFIX(16);
	elems.components[0] = fScalar;
    obOutput.Quadword() = vec_madd( QuadwordValue(), vec_splat(vec_ld(0, elems.components), 0), CVecMath::m_vZero  );
	return obOutput;
}

inline	CDirection		CDirection::operator / ( float fDivisor ) const
{
	CDirection	obOutput;
	ALIGNTO_PREFIX(16) vector_components elems ALIGNTO_POSTFIX(16);
	elems.components[0] = fDivisor;
	v128 qTemp = CVectorBase::SIMDRcp(vec_splat(vec_ld(0, elems.components), 0));
	obOutput.Quadword() = vec_madd( QuadwordValue(), qTemp, CVecMath::m_vZero );
	return obOutput;
}

inline	CPoint		CDirection::operator * ( const CPoint& obPoint ) const
{
	CPoint obOutput;
	obOutput.Quadword() = vec_madd( QuadwordValue(), obPoint.QuadwordValue(), CVecMath::m_vZero  );
	return obOutput;
}

inline	CPoint		CDirection::operator + ( const CPoint& obPoint ) const
{
	CPoint obOutput;
	obOutput.Quadword() = vec_add( QuadwordValue(), obPoint.QuadwordValue() );
	return obOutput;
}

inline	CPoint		CDirection::operator - ( const CPoint& obPoint ) const
{
	CPoint obOutput;
	obOutput.Quadword() = vec_sub( QuadwordValue(), obPoint.QuadwordValue() );
	return obOutput;
}

// ---- Friends

inline	CDirection		operator * ( float fScalar, const CDirection& obDirection )
{
	CDirection	obOutput;
	ALIGNTO_PREFIX(16) CVectorBase::vector_components elems ALIGNTO_POSTFIX(16);
	elems.components[0] = fScalar;
    obOutput.Quadword() = vec_madd( obDirection.QuadwordValue(), vec_splat(vec_ld(0, elems.components), 0), CVecMath::m_vZero  );
	return obOutput;
}

// ---- Const operations

inline	float	CDirection::Length( void ) const
{
	vector_components vComp;
	v128	qTemp = CVectorBase::SIMDDp3( QuadwordValue(), QuadwordValue() );
	vComp.vector = CVectorBase::SIMDSqrt(qTemp);
	return vComp.components[0];
}

inline	float	CDirection::LengthSquared( void ) const
{
	vector_components vComp;
	v128	qTemp = CVectorBase::SIMDDp3( QuadwordValue(), QuadwordValue() );
	vComp.vector = qTemp;
	return vComp.components[0];
}

inline	float	CDirection::Dot( const CDirection& obDirection ) const
{
	vector_components vComp;
	v128	qTemp = CVectorBase::SIMDDp3( QuadwordValue(), obDirection.QuadwordValue() );
	vComp.vector = qTemp;
	return vComp.components[0];
}

inline	CDirection	CDirection::Abs( void ) const
{
	CDirection	obOutput;
	obOutput.Quadword() = vec_and( QuadwordValue(), *(v128* )&CVecMath::m_gauiMaskOffSign );
	return obOutput;
}

inline	CDirection	CDirection::Min( const CDirection& obDirection ) const
{
	CDirection	obOutput;
	obOutput.Quadword() = vec_min( QuadwordValue(), obDirection.QuadwordValue() );
	return obOutput;
}

inline	CDirection	CDirection::Max( const CDirection& obDirection ) const
{
	CDirection	obOutput;
	obOutput.Quadword() = vec_max( QuadwordValue(), obDirection.QuadwordValue() );
	return obOutput;
}

inline	CDirection	CDirection::Cross( const CDirection& obDirection1 ) const
{
	CDirection obOutput;
	obOutput.Quadword() = CVectorBase::SIMDCross( QuadwordValue(), obDirection1.QuadwordValue() );
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
	vector_components Output;
	v128	qTemp;

	qTemp	= vec_sub( obDirection.QuadwordValue(), QuadwordValue() );
	qTemp	= CVectorBase::SIMDDp3( qTemp, qTemp );
	Output.vector = qTemp;

	if ( Output.components[0] < fSquaredTolerance )
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
	ALIGNTO_PREFIX(16) v128  vConstruct ALIGNTO_POSTFIX(16) = { *pfData, *(pfData+1), *(pfData+2), 0.0f};
	Quadword() = vConstruct;
}

inline	CPoint::CPoint( float fX, float fY, float fZ )
{
	ALIGNTO_PREFIX(16) v128	 vConstruct ALIGNTO_POSTFIX(16)= { fX, fY, fZ, 0.0f } ;
	Quadword() = vConstruct;
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
	Quadword() = vec_add( QuadwordValue(), obPoint.QuadwordValue() );
	return *this;
}

inline	CPoint&	CPoint::operator -= ( const CPoint& obPoint )
{
	Quadword() = vec_sub( QuadwordValue(), obPoint.QuadwordValue() );
	return *this;
}

inline	CPoint&	CPoint::operator *= ( const CPoint& obPoint )
{
	Quadword() = vec_madd( QuadwordValue(), obPoint.QuadwordValue(), CVecMath::m_vZero );
	return *this;
}

inline	CPoint&	CPoint::operator += ( const CDirection& obDirection )
{
	Quadword() = vec_add( QuadwordValue(), obDirection.QuadwordValue() );
	return *this;
}

inline	CPoint&	CPoint::operator -= ( const CDirection& obDirection )
{
	Quadword() = vec_sub( QuadwordValue(), obDirection.QuadwordValue() );
	return *this;
}

inline	CPoint&	CPoint::operator *= ( float fScalar )
{
	ALIGNTO_PREFIX(16) vector_components elems ALIGNTO_POSTFIX(16);
	elems.components[0] = fScalar;
    Quadword() = vec_madd( QuadwordValue(), vec_splat(vec_ld(0, elems.components), 0), CVecMath::m_vZero  );
	return *this;
}

inline	CPoint&	CPoint::operator /= ( float fDivisor )
{
	ALIGNTO_PREFIX(16) vector_components elems ALIGNTO_POSTFIX(16);
	elems.components[0] = fDivisor;
	v128 qTemp = CVectorBase::SIMDRcp(vec_splat(vec_ld(0, elems.components), 0));
	Quadword() = vec_madd( QuadwordValue(), qTemp, CVecMath::m_vZero );
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
	obOutput.Quadword() = vec_sub( CVecMath::m_vZero, QuadwordValue() );
	return obOutput;
}

// ---- Binary operators

inline	CPoint		CPoint::operator + ( const CPoint& obPoint ) const
{
	CPoint obOutput;
	obOutput.Quadword() = vec_add( QuadwordValue(), obPoint.QuadwordValue() );
	return obOutput;
}

inline	CPoint		CPoint::operator - ( const CPoint& obPoint ) const
{
	CPoint obOutput;
	obOutput.Quadword() = vec_sub( QuadwordValue(), obPoint.QuadwordValue() );
	return obOutput;
}

inline	CPoint		CPoint::operator * ( const CPoint& obPoint ) const
{
	CPoint obOutput;
	obOutput.Quadword() = vec_madd( QuadwordValue(), obPoint.QuadwordValue(), CVecMath::m_vZero );
	return obOutput;
}

inline	CPoint		CPoint::operator * ( float fScalar ) const
{
	CPoint	obOutput;
	ALIGNTO_PREFIX(16) vector_components elems ALIGNTO_POSTFIX(16);
	elems.components[0] = fScalar;
 	obOutput.Quadword() = vec_madd( QuadwordValue(), vec_splat(vec_ld(0, elems.components), 0), CVecMath::m_vZero  );
	return obOutput;
}

inline	CPoint		CPoint::operator / ( float fDivisor ) const
{
	CPoint	obOutput;
	ALIGNTO_PREFIX(16) vector_components elems ALIGNTO_POSTFIX(16);
	elems.components[0] = fDivisor;
	v128 qTemp = CVectorBase::SIMDRcp(vec_splat(vec_ld(0, elems.components), 0));
	obOutput.Quadword() = vec_madd( QuadwordValue(), qTemp, CVecMath::m_vZero );
	return obOutput;
}

inline	CPoint		CPoint::operator * ( const CDirection& obDirection ) const
{
	CPoint obOutput;
	obOutput.Quadword() = vec_madd( QuadwordValue(), obDirection.QuadwordValue(), CVecMath::m_vZero );
	return obOutput;
}

inline	CPoint		CPoint::operator + ( const CDirection& obDirection ) const
{
	CPoint obOutput;
	obOutput.Quadword() = vec_add( QuadwordValue(), obDirection.QuadwordValue() );
	return obOutput;
}

inline	CPoint		CPoint::operator - ( const CDirection& obDirection ) const
{
	CPoint obOutput;
	obOutput.Quadword() = vec_sub( QuadwordValue(), obDirection.QuadwordValue() );
	return obOutput;
}

// ---- Special Binary operators

inline	CDirection	CPoint::operator ^ ( const CPoint& obPoint ) const
{
	CDirection obOutput;
	obOutput.Quadword() = vec_sub( QuadwordValue(), obPoint.QuadwordValue() );
	return obOutput;
}

// ---- Friends

inline	CPoint		operator * ( float fScalar, const CPoint& obPoint )
{
	CPoint	obOutput;
	ALIGNTO_PREFIX(16) CVectorBase::vector_components elems ALIGNTO_POSTFIX(16);
	elems.components[0] = fScalar;
 	obOutput.Quadword() = vec_madd( obPoint.QuadwordValue(), vec_splat(vec_ld(0, elems.components), 0), CVecMath::m_vZero  );
	return obOutput;
}

// ---- Const operations

inline	float	CPoint::Length( void ) const
{
	vector_components vComp;
	v128 qTemp = CVectorBase::SIMDDp3( QuadwordValue(), QuadwordValue() );
	qTemp = CVectorBase::SIMDSqrt( qTemp );
	vComp.vector = qTemp;
	return vComp.components[0];
}

inline	float	CPoint::LengthSquared( void ) const
{
	vector_components vComp;
	v128 qTemp = CVectorBase::SIMDDp3( QuadwordValue(), QuadwordValue() );
	vComp.vector = qTemp;
	return vComp.components[0];
}

inline	float	CPoint::Dot( const CPoint& obPoint ) const
{
	vector_components vComp;
	v128 qTemp = CVectorBase::SIMDDp3( QuadwordValue(), obPoint.QuadwordValue() );
	vComp.vector = qTemp;
	return vComp.components[0];
}

inline	float	CPoint::Dot( const CDirection &direction ) const
{
	vector_components vComp;
	v128 qTemp = CVectorBase::SIMDDp3( QuadwordValue(), direction.QuadwordValue() );
	vComp.vector = qTemp;
	return vComp.components[0];
}

inline	CPoint	CPoint::Abs( void ) const
{
	CPoint	obOutput;
	obOutput.Quadword() = vec_and( QuadwordValue(), *(v128* )&CVecMath::m_gauiMaskOffSign );
	return obOutput;
}

inline	CPoint	CPoint::Min( const CPoint& obPoint ) const
{
	CPoint	obOutput;
	obOutput.Quadword() = vec_min( QuadwordValue(), obPoint.QuadwordValue() );
	return obOutput;
}

inline	CPoint	CPoint::Max( const CPoint& obPoint ) const
{
	CPoint	obOutput;
	obOutput.Quadword() = vec_max( QuadwordValue(), obPoint.QuadwordValue() );
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
	vector_components Output;
	v128	qTemp;

	qTemp	= vec_sub( obPoint.QuadwordValue(), QuadwordValue() );
	qTemp	= CVectorBase::SIMDDp3( qTemp, qTemp );
	Output.vector = qTemp;

	if ( Output.components[0] < fSquaredTolerance )
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
	Quadword() = vec_ld(0, &CVecMath::m_vOne);
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
	ALIGNTO_PREFIX(16) v128  vConstruct ALIGNTO_POSTFIX(16) = { *pfData, *(pfData+1), *(pfData+2), *(pfData+3)};
	Quadword() = vConstruct;
}

inline	CQuat::CQuat( float fX, float fY, float fZ, float fC )
{
	ALIGNTO_PREFIX(16) v128  vConstruct ALIGNTO_POSTFIX(16) = { fX, fY, fZ, fC};
	Quadword() = vConstruct;
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
	Quadword() = vec_ld(0, (v128*)&CVecMath::m_gauiIdentityMatrix[3]);
}


// ---- Assignment

inline	CQuat&	CQuat::operator = ( const CQuat& obQuat )
{
	Quadword() = obQuat.QuadwordValue();
	return *this;
}

inline	CQuat&	CQuat::operator += ( const CQuat& obQuat )
{
	Quadword() = vec_add( QuadwordValue(), obQuat.QuadwordValue() );
	return *this;
}

inline	CQuat&	CQuat::operator -= ( const CQuat& obQuat )
{
	Quadword() = vec_sub( QuadwordValue(), obQuat.QuadwordValue() );
	return *this;
}

inline	CQuat&	CQuat::operator *= ( const CQuat& obQuat )
{	
	CQuat	obOutput;
	/*
	v128	result, minus, leftQuat, rightQuat;

	leftQuat = QuadwordValue();
	rightQuat = obQuat.QuadwordValue();
	minus = vec_sub( CVecMath::m_vZero, obQuat.QuadwordValue() );

	// this full SIMD implementation (1 sub, 4 splats, 3 perms and 4 madds) should be much faster than the previous 
	// full scalar implementation ;) It should work even better on SPEs
	// due to the co-issue of splats/perms with pure math instructions
	result = vec_madd( vec_splat( leftQuat, 3 ), obQuat.QuadwordValue(), CVecMath::m_vZero );
	result = vec_madd( vec_splat( leftQuat, 0 ), vec_perm( rightQuat, minus, CVecMath::m_vShuffleWzYx), result);
	result = vec_madd( vec_splat( leftQuat, 1 ), vec_perm( rightQuat, minus, CVecMath::m_vShuffleZWxy), result);
	result = vec_madd( vec_splat( leftQuat, 2 ), vec_perm( rightQuat, minus, CVecMath::m_vShuffleyXWz), result);
	
	Quadword() = result;*/

	CQuat	obThis = *this;

	X() = ( obThis.W() * obQuat.X() ) + ( obThis.X() * obQuat.W() ) + ( obThis.Y() * obQuat.Z() ) - ( obThis.Z() * obQuat.Y() );
	Y() = ( obThis.W() * obQuat.Y() ) - ( obThis.X() * obQuat.Z() ) + ( obThis.Y() * obQuat.W() ) + ( obThis.Z() * obQuat.X() );
	Z() = ( obThis.W() * obQuat.Z() ) + ( obThis.X() * obQuat.Y() ) - ( obThis.Y() * obQuat.X() ) + ( obThis.Z() * obQuat.W() );
	W() = ( obThis.W() * obQuat.W() ) - ( obThis.X() * obQuat.X() ) - ( obThis.Y() * obQuat.Y() ) - ( obThis.Z() * obQuat.Z() );

	return *this; 
}

inline	CQuat&	CQuat::operator *= ( float fScalar )
{
	ALIGNTO_PREFIX(16) vector_components elems ALIGNTO_POSTFIX(16);
	elems.components[0] = fScalar;
    Quadword() = vec_madd( QuadwordValue(), vec_splat(vec_ld(0, elems.components), 0), CVecMath::m_vZero  );
	return *this;
}

inline	CQuat&	CQuat::operator /= ( float fDivisor )
{
	ALIGNTO_PREFIX(16) vector_components elems ALIGNTO_POSTFIX(16);
	elems.components[0] = fDivisor;
	v128 qTemp = CVectorBase::SIMDRcp(vec_splat(vec_ld(0, elems.components), 0));
    Quadword() = vec_madd( QuadwordValue(), qTemp, CVecMath::m_vZero  );
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
	obOutput.Quadword() = vec_sub( CVecMath::m_vZero , QuadwordValue() );
	return obOutput;
}

inline	CQuat		CQuat::operator ~ () const
{
	CQuat	obOutput;
	obOutput.Quadword() = vec_xor( QuadwordValue(), *(v128* )&CVecMath::m_gauiQuatConjugateMask );
	return obOutput;
}

// ---- Binary operators

inline	CQuat		CQuat::operator + ( const CQuat& obQuat ) const
{
	CQuat obOutput;
	obOutput.Quadword() = vec_add( QuadwordValue(), obQuat.QuadwordValue() );
	return obOutput;
}

inline	CQuat		CQuat::operator - ( const CQuat& obQuat ) const
{
	CQuat obOutput;
	obOutput.Quadword() = vec_sub( QuadwordValue(), obQuat.QuadwordValue() );
	return obOutput;
}

inline	CQuat		CQuat::operator * ( const CQuat& obQuat ) const
{
	CQuat	obOutput;
	/*
	v128	result, minus, leftQuat, rightQuat;

	leftQuat = QuadwordValue();
	rightQuat = obQuat.QuadwordValue();
	minus = vec_sub( CVecMath::m_vZero, obQuat.QuadwordValue() );

	// this full SIMD implementation (1 sub, 4 splats, 3 perms and 4 madds) should be much faster than the previous 
	// full scalar implementation ;) It should work even better on SPEs
	// due to the double issue of splats/perms with pure math instructions
	result = vec_madd( vec_splat( leftQuat, 3 ), obQuat.QuadwordValue(), CVecMath::m_vZero );
	result = vec_madd( vec_splat( leftQuat, 0 ), vec_perm( rightQuat, minus, CVecMath::m_vShuffleWzYx), result);
	result = vec_madd( vec_splat( leftQuat, 1 ), vec_perm( rightQuat, minus, CVecMath::m_vShuffleZWxy), result);
	result = vec_madd( vec_splat( leftQuat, 2 ), vec_perm( rightQuat, minus, CVecMath::m_vShuffleyXWz), result);
	
	obOutput.Quadword() = result; */


	obOutput.X() = ( W() * obQuat.X() ) + ( X() * obQuat.W() ) + ( Y() * obQuat.Z() ) - ( Z() * obQuat.Y() );
	obOutput.Y() = ( W() * obQuat.Y() ) - ( X() * obQuat.Z() ) + ( Y() * obQuat.W() ) + ( Z() * obQuat.X() );
	obOutput.Z() = ( W() * obQuat.Z() ) + ( X() * obQuat.Y() ) - ( Y() * obQuat.X() ) + ( Z() * obQuat.W() );
	obOutput.W() = ( W() * obQuat.W() ) - ( X() * obQuat.X() ) - ( Y() * obQuat.Y() ) - ( Z() * obQuat.Z() );

	return obOutput;
}

inline	CQuat		CQuat::operator * ( float fScalar ) const
{
	CQuat	obOutput;
	ALIGNTO_PREFIX(16) vector_components elems ALIGNTO_POSTFIX(16);
	elems.components[0] = fScalar;
    obOutput.Quadword() = vec_madd( QuadwordValue(), vec_splat(vec_ld(0, elems.components), 0), CVecMath::m_vZero  );
	return obOutput;
}

inline	CQuat		CQuat::operator / ( float fDivisor ) const
{
	CQuat	obOutput;
	ALIGNTO_PREFIX(16) vector_components elems ALIGNTO_POSTFIX(16);
	elems.components[0] = fDivisor;
	v128 qTemp = CVectorBase::SIMDRcp(vec_splat(vec_ld(0, elems.components), 0));
    obOutput.Quadword() = vec_madd( QuadwordValue(), qTemp, CVecMath::m_vZero  );
	return obOutput;
}

// ---- Friends

inline	CQuat		operator * ( float fScalar, const CQuat& obQuat )
{
	CQuat	obOutput;
	ALIGNTO_PREFIX(16) CVectorBase::vector_components elems ALIGNTO_POSTFIX(16);
	elems.components[0] = fScalar;
	obOutput.Quadword() = vec_madd( obQuat.QuadwordValue(), vec_splat(vec_ld(0, elems.components), 0), CVecMath::m_vZero  );
	return obOutput;
}

// ---- Const operations

inline	float	CQuat::Dot( const CQuat& obQuat ) const
{
	vector_components vComp;
	v128	qTemp = CVectorBase::SIMDDp4( QuadwordValue(), obQuat.QuadwordValue() );
	vComp.vector = qTemp;
	return vComp.components[0];
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
	Row( 0 ).Quadword() = vec_ld(0, (v128*)&CVecMath::m_gauiIdentityMatrix[0]);
	Row( 1 ).Quadword() = vec_ld(0, (v128*)&CVecMath::m_gauiIdentityMatrix[1]);
	Row( 2 ).Quadword() = vec_ld(0, (v128*)&CVecMath::m_gauiIdentityMatrix[2]);
	Row( 3 ).Quadword() = vec_ld(0, (v128*)&CVecMath::m_gauiIdentityMatrix[3]);
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

inline CMatrix::CMatrix( CVector const& row1, CVector const& row2, CVector const& row3, CVector const& row4 )
{
    Row( 0 ) = row1;
    Row( 1 ) = row2;
    Row( 2 ) = row3;
    Row( 3 ) = row4;
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
	Row( 0 ).Quadword() = vec_ld(0, (v128*)&CVecMath::m_gauiIdentityMatrix[0]);
	Row( 1 ).Quadword() = vec_ld(0, (v128*)&CVecMath::m_gauiIdentityMatrix[1]);
	Row( 2 ).Quadword() = vec_ld(0, (v128*)&CVecMath::m_gauiIdentityMatrix[2]);
	Row( 3 ).Quadword() = vec_ld(0, (v128*)&CVecMath::m_gauiIdentityMatrix[3]);
}

// ---- Casting

inline	CMatrix::operator float* ()
{
	return reinterpret_cast<float*>(&Row( 0 ));
}

inline	CMatrix::operator const float* () const
{
	return reinterpret_cast<const float*>(&Row( 0 ));
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
	Row( 0 ).Quadword() = vec_and( obXAxis.QuadwordValue(), *( v128* )&CVecMath::m_gauiMaskOffW );
}

inline	const	CDirection&	CMatrix::GetYAxis() const
{
	return *( CDirection* )&Row( 1 );
}

inline	void	CMatrix::SetYAxis( const CDirection& obYAxis )
{
	Row( 1 ).Quadword() = vec_and( obYAxis.QuadwordValue(), *( v128* )&CVecMath::m_gauiMaskOffW );
}

inline	const	CDirection&	CMatrix::GetZAxis() const
{
	return *( CDirection* )&Row( 2 );
}

inline	void	CMatrix::SetZAxis( const CDirection& obZAxis )
{
	Row( 2 ).Quadword() = vec_and( obZAxis.QuadwordValue(), *( v128* )&CVecMath::m_gauiMaskOffW );
}

inline	const	CPoint&	CMatrix::GetTranslation() const
{
	return *( CPoint* )&Row( 3 );
}

inline	void	CMatrix::SetTranslation( const CPoint& obTranslation )
{
	Row( 3 ).Quadword() = vec_or( vec_and( obTranslation.QuadwordValue(), *(v128* )&CVecMath::m_gauiMaskOffW ), *( v128* )&CVecMath::m_gauiOneInW );
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



//////////////////////////////


/***************************************************************************************************
*
*	FUNCTION		operator *	( const CVector& obVector, const CMatrix& obMatrix )
*
*	DESCRIPTION		Performs CVector * CMatrix multiplication, returning a new CVector object. Don't
*					be alarmed by the fact it returns by value - this will be optimised away in
*					release builds due to the fact that a CVector can be safely held in an __m128
*					register.
*	
*	NOTES			Shamelessly taken from Simon's SSE matrix class. Intentionally in global scope.
*
***************************************************************************************************/

inline CVector		operator *	( const CVector& obVector, const CMatrix& obMatrix )
{
	CVector	obResult;
	v128 result;
	v128_arg vrow0 = obMatrix.Row( 0 ).QuadwordValue();
	v128_arg vrow1 = obMatrix.Row( 1 ).QuadwordValue();
	v128_arg vrow2 = obMatrix.Row( 2 ).QuadwordValue();
	v128_arg vrow3 = obMatrix.Row( 3 ).QuadwordValue();

	result = vec_madd( vec_splat(obVector.QuadwordValue(), 0), vrow0, CVecMath::m_vZero );
	result = vec_madd( vec_splat(obVector.QuadwordValue(), 1), vrow1, result );
	result = vec_madd( vec_splat(obVector.QuadwordValue(), 2), vrow2, result );
	result = vec_madd( vec_splat(obVector.QuadwordValue(), 3), vrow3, result );

	obResult.Quadword() = result;

	return obResult;
}


/***************************************************************************************************
*
*	FUNCTION		operator *	( const CDirection& obDirection, const CMatrix& obMatrix )
*
*	DESCRIPTION		Performs CDirection * CMatrix multiplication, returning a new CVector object.
*					Don't be alarmed by the fact it returns by value - this will be optimised away in
*					release builds due to the fact that a CVecCDirectiontor can be safely held in an
*					__m128 register.
*	
*	NOTES			Shamelessly taken from Simon's SSE matrix class. Intentionally in global scope.
*
***************************************************************************************************/

inline CDirection	operator *	( const CDirection& obDirection, const CMatrix& obMatrix )
{

	CDirection	obResult;
	v128 result;
	v128_arg vrow0 = obMatrix.Row( 0 ).QuadwordValue();
	v128_arg vrow1 = obMatrix.Row( 1 ).QuadwordValue();
	v128_arg vrow2 = obMatrix.Row( 2 ).QuadwordValue();

	result = vec_madd( vec_splat(obDirection.QuadwordValue(), 0), vrow0, CVecMath::m_vZero );
	result = vec_madd( vec_splat(obDirection.QuadwordValue(), 1), vrow1, result );
	result = vec_madd( vec_splat(obDirection.QuadwordValue(), 2), vrow2, result );

	obResult.Quadword() = result;

	return obResult;
}

/***************************************************************************************************
*
*	FUNCTION		operator *	( const CPoint& obPoint, const CMatrix& obMatrix )
*
*	DESCRIPTION		Performs CPoint * CMatrix multiplication, returning a new CPoint object. Don't
*					be alarmed by the fact it returns by value - this will be optimised away in
*					release builds due to the fact that a CPoint can be safely held in an __m128
*					register.
*	
*	NOTES			Shamelessly taken from Simon's SSE matrix class.
*
***************************************************************************************************/

inline CPoint				operator *	( const CPoint& obPoint, const CMatrix& obMatrix )
{

	CPoint	obResult;
	v128 result;
	v128_arg vrow0 = obMatrix.Row( 0 ).QuadwordValue();
	v128_arg vrow1 = obMatrix.Row( 1 ).QuadwordValue();
	v128_arg vrow2 = obMatrix.Row( 2 ).QuadwordValue();
	v128_arg vrow3 = obMatrix.Row( 3 ).QuadwordValue();

	result = vec_madd( vec_splat(obPoint.QuadwordValue(), 0), vrow0, CVecMath::m_vZero );
	result = vec_madd( vec_splat(obPoint.QuadwordValue(), 1), vrow1, result );
	result = vec_madd( vec_splat(obPoint.QuadwordValue(), 2), vrow2, result );
	result = vec_add( vrow3, result);

	obResult.Quadword() = result;

	return obResult;
}



#endif //_VECMATH_PS3_H

