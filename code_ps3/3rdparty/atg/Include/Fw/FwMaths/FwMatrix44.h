//--------------------------------------------------------------------------------------------------
/**
	@file		
	
	@brief		SIMD 4x4 Matrix

	@note		(c) Copyright Sony Computer Entertainment 2005. All Rights Reserved.	
**/
//--------------------------------------------------------------------------------------------------

#ifndef FW_MATRIX44_H
#define FW_MATRIX44_H

#include <Fw/FwMaths/FwScalar.h>
#include <Fw/FwMaths/FwVector4.h>

//--------------------------------------------------------------------------------------------------
// Forward references

class	FwVector4;
class	FwMatrix44;

//--------------------------------------------------------------------------------------------------
/**
	@class			FwMatrix44
	
	@brief			Holds a full 4x4 matrix
**/
//--------------------------------------------------------------------------------------------------

class	FwMatrix44
{
public:
	// standard constructors
	FwMatrix44();
	FwMatrix44( const FwMatrix44& rhs );
	FwMatrix44( FwVector4_arg row0, FwVector4_arg row1, FwVector4_arg row2, FwVector4_arg row3 );

	// construction to known values
	FwMatrix44( const FwMaths::ZeroTag& );
	FwMatrix44( const FwMaths::IdentityTag& );

	// construct from rotation angle & axis
	FwMatrix44( FwVector4_arg axis, float angle );
	
	// read access to elements using array operator
	const FwVector4	operator []( int index ) const;

	// get, set rows and columns
	FwVector4		GetRow( int row ) const;
	FwVector4		GetCol( int col ) const;
	void			SetRow( int row, FwVector4_arg rhs );
	void			SetCol( int col, FwVector4_arg rhs );

	// get, set elements
	FwScalar		Get( int row, int col ) const;
	void			Set( int row, int col, FwScalar_arg val );

	// set entire matrix for a transformation
	void			SetScale( FwVector4_arg scale );
	void			SetRotateX( float angle );
	void			SetRotateY( float angle );
	void			SetRotateZ( float angle );

	// obtain euler angles
	void			GetEulerAngles( float& x, float& y, float& z ) const;
	
	// assignments
	FwMatrix44&		operator = ( const FwMatrix44& rhs );
	FwMatrix44&		operator = ( const FwMaths::ZeroTag& );
	FwMatrix44&		operator = ( const FwMaths::IdentityTag& );

	// add, subtract, negate, absolute value
	FwMatrix44		operator + ( const FwMatrix44& rhs ) const;
	FwMatrix44		operator - ( const FwMatrix44& rhs ) const;
	FwMatrix44&		operator += ( const FwMatrix44& rhs );
	FwMatrix44&		operator -= ( const FwMatrix44& rhs );
	FwMatrix44		operator - () const;

	// matrix scalar multiplies
	FwMatrix44		operator * ( FwScalar_arg rhs ) const;
	FwMatrix44&		operator *= ( FwScalar_arg rhs );

	// matrix matrix multiplies
	FwMatrix44		operator * ( const FwMatrix44& rhs ) const;
	FwMatrix44&		operator *= ( const FwMatrix44& rhs );

	// non-member functions
	friend	FwVector4		operator * ( FwVector4_arg lhs, const FwMatrix44& rhs );
	friend	FwMatrix44		MulTransformMatrix( const FwMatrix44& trans, const FwMatrix44& mat );
	friend	FwVector4		MulPointMatrix( FwVector4_arg point, const FwMatrix44& mat );		
	friend	FwVector4		MulVectorMatrix( FwVector4_arg vec, const FwMatrix44& mat );
	friend	FwVector4		MulVector4Transform( FwVector4_arg vec, const FwMatrix44& mat );

	friend	FwMatrix44		Abs( const FwMatrix44& mat );
	friend	FwMatrix44		Transpose( const FwMatrix44& mat );
	friend	FwMatrix44		Inverse( const FwMatrix44& mat );
	friend	FwMatrix44		Adjoint( const FwMatrix44& mat );
	friend	FwScalar		Determinant3( const FwMatrix44& mat );
	friend	FwScalar		Determinant4( const FwMatrix44& mat );
	friend	FwMatrix44		AffineInverse( const FwMatrix44& mat );
	friend	FwMatrix44		AffineOrthogonalInverse( const FwMatrix44& mat );
	friend	FwMatrix44		operator * ( FwScalar_arg scalar, const FwMatrix44& mat );

private:
	FwVector4	m_row[4];
};

// Include our inlines
#include <Fw/FwMaths/FwMatrix44.inl>

#endif	// FW_MATRIX44_H
