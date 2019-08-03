//--------------------------------------------------------------------------------------------------
/**
	@file		
	
	@brief		SIMD Quaternions

	@note		(c) Copyright Sony Computer Entertainment 2005. All Rights Reserved.	
**/
//--------------------------------------------------------------------------------------------------

#ifndef FW_QUAT_H
#define FW_QUAT_H

#include <Fw/FwMaths/FwScalar.h>
#include <Fw/FwMaths/FwVector4.h>
#include <Fw/FwMaths/FwMatrix44.h>

//--------------------------------------------------------------------------------------------------
// Forward references

class	FwQuat;

//--------------------------------------------------------------------------------------------------
// Argument typedefs

#ifdef	_MSC_VER
typedef	const FwQuat&		FwQuat_arg;
#else
typedef	const FwQuat		FwQuat_arg;
#endif	//_MSC_VER

//--------------------------------------------------------------------------------------------------
/**
	@class			FwQuat
	
	@brief			Holds a quaternion rotation in X, Y, Z, and W			
**/
//--------------------------------------------------------------------------------------------------

class	FwQuat
{
public:
	// standard constructors
	FwQuat( );
	FwQuat( float x, float y, float z, float w );
	FwQuat( FwVector4_arg axis, float angle );

	// copy constructor
#ifdef	_MSC_VER
	FwQuat( const FwQuat& rhs );
#endif	//_MSC_VER

	// additional constructors
	explicit	FwQuat( const float* pData );
	explicit	FwQuat( v128 rhs );
	explicit	FwQuat( FwVector4_arg rhs );
	explicit	FwQuat( const FwMatrix44& rotation );

	// rotation order construction
	enum RotationOrder
	{
		kXYZ, 
		kYZX, 
		kZXY, 
		kXZY, 
		kYXZ, 
		kZYX,
	};
	FwQuat( float x, float y, float z, RotationOrder rotOrder = kXYZ );

	// construction to known values
	FwQuat( const FwMaths::ZeroTag& );
	FwQuat( const FwMaths::QuatITag& );
	FwQuat( const FwMaths::QuatJTag& );
	FwQuat( const FwMaths::QuatKTag& );
	FwQuat( const FwMaths::IdentityTag& );

	// assign common fields of rhs
	FwQuat		operator = ( const FwMaths::ZeroTag& );
	FwQuat		operator = ( const FwMaths::QuatITag& );
	FwQuat		operator = ( const FwMaths::QuatJTag& );
	FwQuat		operator = ( const FwMaths::QuatKTag& );
	FwQuat		operator = ( const FwMaths::IdentityTag& );
	FwQuat		operator = ( FwQuat_arg rhs );

	// operations with floats are broadcast operations
	FwQuat		operator + ( FwQuat_arg rhs ) const;
	FwQuat		operator - ( FwQuat_arg rhs ) const;
	FwQuat		operator * ( FwQuat_arg rhs ) const;
	FwQuat		operator * ( FwScalar_arg rhs ) const;
	FwQuat		operator / ( FwScalar_arg rhs ) const;

	FwQuat		operator += ( FwQuat_arg rhs );
	FwQuat		operator -= ( FwQuat_arg rhs );
	FwQuat		operator *= ( FwQuat_arg rhs );
	FwQuat		operator *= ( FwScalar_arg rhs );
	FwQuat		operator /= ( FwScalar_arg rhs );

	// negate
	FwQuat		operator - () const;

	// general member & array accessors
	FwScalar		X( void ) const;
	FwScalar		Y( void ) const;
	FwScalar		Z( void ) const;
	FwScalar		W( void ) const;	

	void			SetX( FwScalar_arg x );
	void			SetY( FwScalar_arg y );
	void			SetZ( FwScalar_arg z );
	void			SetW( FwScalar_arg w );

	FwScalar		Get( int index ) const;			
	void			Set( int index, FwScalar_arg val );
	const FwScalar	operator []( int index ) const;

	// read-only access to quadword value
	v128			QuadwordValue( void ) const;

	// return an equivalent FwVector4
	FwVector4		GetVector4( void ) const;

	// obtain axis & angle
	void			GetAxisAndAngle( FwVector4& rAxis, float& rAngle ) const;

	// non-member functions, which need to be friends
	friend	FwMatrix44	BuildTransform( FwQuat_arg rotation, FwVector4_arg translation );
	friend	FwQuat		Conjugate( FwQuat_arg quat );
	friend	FwScalar	Dot( FwQuat_arg lhs, FwQuat_arg rhs );
	friend	FwQuat		Normalize( FwQuat_arg quat );
	friend	FwQuat		SafeNormalize( FwQuat_arg quat, FwQuat_arg fallback );
	friend	FwQuat		Lerp( FwQuat_arg lhs, FwQuat_arg rhs, FwScalar_arg alpha );
	friend	FwQuat		Slerp( FwQuat_arg unitQuat0, FwQuat_arg unitQuat1, FwScalar_arg t );
	friend	FwQuat		Squad( FwQuat_arg unitQuat0, FwQuat_arg unitQuat1, FwQuat_arg unitQuat2, FwQuat_arg unitQuat3, float t );
	friend	FwQuat		RotationBetween( FwVector4_arg rotateFrom, FwVector4_arg rotateTo );
	friend	FwQuat		operator * ( FwScalar_arg scalar, FwQuat_arg vec );

private:
	v128		m_value;
};

// Include our inlines
#include <Fw/FwMaths/FwQuat.inl>

#endif	// FW_QUAT_H
