//--------------------------------------------------------------------------------------------------
/**
	@file		
	
	@brief		SIMD 3x4 Matrix

	@note		(c) Copyright Sony Computer Entertainment 2005. All Rights Reserved.	
**/
//--------------------------------------------------------------------------------------------------

#ifndef FW_MATRIX34_H
#define FW_MATRIX34_H

#include <Fw/FwMaths/FwVector4.h>
#include <Fw/FwMaths/FwMatrix44.h>

//--------------------------------------------------------------------------------------------------
/**
	@class			FwMatrix34
	
	@brief		
**/
//--------------------------------------------------------------------------------------------------

class	FwMatrix34
{
public:
	// standard constructors
	FwMatrix34();
	FwMatrix34( const FwMatrix34& rhs );
	FwMatrix34( FwVector4_arg row0, FwVector4_arg row1, FwVector4_arg row2 );

	// construct from known values
	FwMatrix34( const FwMaths::ZeroTag& );
	FwMatrix34( const FwMaths::IdentityTag& );

	// construct from an affine 4x4 matrix
	explicit FwMatrix34( const FwMatrix44& mat );
	
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

	// assignments
	FwMatrix34&		operator = ( const FwMatrix34& rhs );
	FwMatrix34&		operator = ( const FwMaths::ZeroTag& );
	FwMatrix34&		operator = ( const FwMaths::IdentityTag& );

	// return an FwMatrix44, with a correct 4th column of (0,0,0,1)
	FwMatrix44		GetMatrix44( void ) const;

private:
	FwVector4 m_row[3];
};

// Include our inlines
#include <Fw/FwMaths/FwMatrix34.inl>

#endif	// FW_MATRIX34_H
