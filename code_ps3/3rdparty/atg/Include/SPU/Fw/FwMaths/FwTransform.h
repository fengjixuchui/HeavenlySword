//--------------------------------------------------------------------------------------------------
/**
	@file		
	
	@brief		SIMD Affine Transform

	@note		(c) Copyright Sony Computer Entertainment 2005. All Rights Reserved.	
**/
//--------------------------------------------------------------------------------------------------

#ifndef FW_TRANSFORM_H
#define FW_TRANSFORM_H

#include <Fw/FwMaths/FwScalar.h>
#include <Fw/FwMaths/FwVector4.h>
#include <Fw/FwMaths/FwMatrix44.h>
#include <Fw/FwMaths/FwVector.h>
#include <Fw/FwMaths/FwPoint.h>
#include <Fw/FwMaths/FwQuat.h>

//--------------------------------------------------------------------------------------------------
/**
	@class			FwTransform
	
	@brief		
**/
//--------------------------------------------------------------------------------------------------

class	FwTransform
{
public:
	// standard constructors
	FwTransform();
	FwTransform( const FwTransform& rhs );
	FwTransform( FwVector_arg row0, FwVector_arg row1, FwVector_arg row2, FwPoint_arg row3 );

	// construct from known values
	FwTransform( const FwMaths::ZeroTag& );
	FwTransform( const FwMaths::IdentityTag& );

	// construct from quaternion and translation
	FwTransform( FwQuat_arg rotation, FwPoint_arg translation );

	// construct from rotation angle & axis
	FwTransform( FwVector_arg axis, float angle );

	// construct from 4x4 matrix
	explicit FwTransform( const FwMatrix44& mat );
	
	// construct from point
	explicit FwTransform( FwPoint_arg trans );

	// set entire matrix for a transformation
	void			SetScale( FwVector4_arg scale );
	void			SetRotateX( float angle );
	void			SetRotateY( float angle );
	void			SetRotateZ( float angle );

	// read access to axes & translation within matrix
	FwVector		GetAxis( int index ) const;
	FwVector		GetXAxis( void ) const;
	FwVector		GetYAxis( void ) const;
	FwVector		GetZAxis( void ) const;
	FwPoint			GetTranslation( void ) const;
	
	// write access to axes and translation
	void			SetAxis( int index, FwVector_arg value );
	void			SetXAxis( FwVector_arg xAxis );
	void			SetYAxis( FwVector_arg yAxis );
	void			SetZAxis( FwVector_arg zAxis );
	void			SetTranslation( FwPoint_arg translation );

	// get, set elements
	FwScalar		Get( int row, int col ) const;
	void			Set( int row, int col, FwScalar_arg val );

	// assignments
	FwTransform&		operator = ( const FwTransform& rhs );
	FwTransform&		operator = ( const FwMaths::ZeroTag& );
	FwTransform&		operator = ( const FwMaths::IdentityTag& );

	// matrix matrix multiplies
	FwTransform			operator * ( const FwTransform& rhs ) const;
	FwTransform&		operator *= ( const FwTransform& rhs );

	// return an FwMatrix44, massaged to have a 4th column of (0,0,0,1)
	FwMatrix44			GetMatrix44( void ) const;
	
	// return an FwMatrix44, with an undefined 4th column
	FwMatrix44			GetRawMatrix44( void ) const;

	// non-member functions
	friend	FwVector4	operator * ( FwVector4_arg lhs, const FwTransform& rhs );
	friend	FwVector	operator * ( FwVector_arg lhs, const FwTransform& rhs );
	friend	FwPoint		operator * ( FwPoint_arg lhs, const FwTransform& rhs );
	friend	FwTransform	Inverse( const FwTransform& mat );
	friend	FwTransform	OrthogonalInverse( const FwTransform& mat );
	friend	FwMatrix44	MulTransformMatrix( const FwTransform& trans, const FwMatrix44& mat );

private:
	FwMatrix44		m_matrix;
};

// Include our inlines
#include <Fw/FwMaths/FwTransform.inl>

#endif	// FW_TRANSFORM_H
