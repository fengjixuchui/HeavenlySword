//--------------------------------------------------------------------------------------------------
/**
	@file		
	
	@brief		SIMD Affine Transform

	@note		(c) Copyright Sony Computer Entertainment 2005. All Rights Reserved.	
**/
//--------------------------------------------------------------------------------------------------

#ifndef FW_TRANSFORM_INL
#define FW_TRANSFORM_INL

//--------------------------------------------------------------------------------------------------
/**
	@brief			Default constructor
**/
//--------------------------------------------------------------------------------------------------

inline	FwTransform::FwTransform()
{
}

//--------------------------------------------------------------------------------------------------
/**
	@brief			Copy constructor
**/
//--------------------------------------------------------------------------------------------------

inline	FwTransform::FwTransform( const FwTransform& rhs )
{
	m_matrix = rhs.m_matrix;
}

//--------------------------------------------------------------------------------------------------
/**
	@brief			Construct from 3 axes and a translation
**/
//--------------------------------------------------------------------------------------------------

inline	FwTransform::FwTransform( FwVector_arg row0, FwVector_arg row1, FwVector_arg row2, FwPoint_arg row3 )
{
	m_matrix = FwMatrix44(	row0.GetVector4(),
							row1.GetVector4(),
							row2.GetVector4(),
							row3.GetVector4() );
}

//--------------------------------------------------------------------------------------------------
/**
	@brief			Constructs to zero
**/
//--------------------------------------------------------------------------------------------------

inline	FwTransform::FwTransform( const FwMaths::ZeroTag& )
{
	m_matrix = FwMaths::kZero;
}

//--------------------------------------------------------------------------------------------------
/**
	@brief			Constructs to identity
**/
//--------------------------------------------------------------------------------------------------

inline	FwTransform::FwTransform( const FwMaths::IdentityTag& )
{
	m_matrix = FwMaths::kIdentity;
}

//--------------------------------------------------------------------------------------------------
/**
	@brief			Constructs from quaternion and a translation
**/
//--------------------------------------------------------------------------------------------------

inline	FwTransform::FwTransform( FwQuat_arg rotation, FwPoint_arg translation )
{
	m_matrix = BuildTransform( rotation, translation.GetVector4() );
}

//--------------------------------------------------------------------------------------------------
/**
	@brief			Constructs from an axis & angle, setting translation to zero
**/
//--------------------------------------------------------------------------------------------------

inline	FwTransform::FwTransform( FwVector_arg axis, float angle )
{
	m_matrix = FwMatrix44( axis.GetVector4(), angle );
}
	
//--------------------------------------------------------------------------------------------------
/**
	@brief			Constructs from a point, setting rotation to identity
**/
//--------------------------------------------------------------------------------------------------

inline	FwTransform::FwTransform( FwPoint_arg trans )
{	
	m_matrix = FwMaths::kIdentity;
	m_matrix.SetRow( 3, trans.GetVector4() );
}

//--------------------------------------------------------------------------------------------------
/**
	@brief			Construct from a 4x4 matrix
**/
//--------------------------------------------------------------------------------------------------

inline	FwTransform::FwTransform( const FwMatrix44& mat )
{
	m_matrix = mat;
}

//--------------------------------------------------------------------------------------------------
/**
	@brief			Sets the matrix to hold an object that can scale all 3 axes. 
**/
//--------------------------------------------------------------------------------------------------

inline	void		FwTransform::SetScale( FwVector4_arg scale )
{
	m_matrix.SetScale( scale );
}

//--------------------------------------------------------------------------------------------------
/**
	@brief			Sets the matrix to hold a matrix representing a rotation around X
**/
//--------------------------------------------------------------------------------------------------

inline	void		FwTransform::SetRotateX( float angle )
{
	m_matrix.SetRotateX( angle );
}

//--------------------------------------------------------------------------------------------------
/**
	@brief			Sets the matrix to hold a matrix representing a rotation around Y
**/
//--------------------------------------------------------------------------------------------------

inline	void		FwTransform::SetRotateY( float angle )
{
	m_matrix.SetRotateY( angle );
}

//--------------------------------------------------------------------------------------------------
/**
	@brief			Sets the matrix to hold a matrix representing a rotation around Z
**/
//--------------------------------------------------------------------------------------------------

inline	void		FwTransform::SetRotateZ( float angle )
{
	m_matrix.SetRotateZ( angle );
}

//--------------------------------------------------------------------------------------------------
/**
	@brief			Retrieves the vector for a given axis (0=X, 1=Y, 2=Z)
**/
//--------------------------------------------------------------------------------------------------

inline	FwVector	FwTransform::GetAxis( int index ) const
{
	assert( index >= 0 && index <= 2 );
	return FwVector( m_matrix.GetRow( index ) );
}

//--------------------------------------------------------------------------------------------------
/**
	@brief			Retrieves a vector containing the X axis of the transform
**/
//--------------------------------------------------------------------------------------------------

inline	FwVector	FwTransform::GetXAxis( void ) const
{
	return FwVector( m_matrix.GetRow( 0 ) );
}

//--------------------------------------------------------------------------------------------------
/**
	@brief			Retrieves a vector containing the Y axis of the transform
**/
//--------------------------------------------------------------------------------------------------

inline	FwVector	FwTransform::GetYAxis( void ) const
{
	return FwVector( m_matrix.GetRow( 1 ) );
}

//--------------------------------------------------------------------------------------------------
/**
	@brief			Retrieves a vector containing the Z axis of the transform
**/
//--------------------------------------------------------------------------------------------------

inline	FwVector	FwTransform::GetZAxis( void ) const
{
	return FwVector( m_matrix.GetRow( 2 ) );
}

//--------------------------------------------------------------------------------------------------
/**
	@brief			Retrieves a point containing the translation of the transform
**/
//--------------------------------------------------------------------------------------------------

inline	FwPoint		FwTransform::GetTranslation( void ) const
{
	return FwPoint( m_matrix.GetRow( 3 ) );
}
	
//--------------------------------------------------------------------------------------------------
/**
	@brief			Sets the vector representing the given axis (0=X, 1=Y, 2=Z)
**/
//--------------------------------------------------------------------------------------------------

inline	void		FwTransform::SetAxis( int index, FwVector_arg value )
{
	assert( index >= 0 && index <= 2 );
	m_matrix.SetRow( index, value.GetVector4() );
}

//--------------------------------------------------------------------------------------------------
/**
	@brief			Sets the vector representing the X axis
**/
//--------------------------------------------------------------------------------------------------

inline	void		FwTransform::SetXAxis( FwVector_arg xAxis )
{
	m_matrix.SetRow( 0, xAxis.GetVector4() );
}

//--------------------------------------------------------------------------------------------------
/**
	@brief			Sets the vector representing the Y axis
**/
//--------------------------------------------------------------------------------------------------

inline	void		FwTransform::SetYAxis( FwVector_arg yAxis )
{
	m_matrix.SetRow( 1, yAxis.GetVector4() );
}

//--------------------------------------------------------------------------------------------------
/**
	@brief			Sets the vector representing the Z axis
**/
//--------------------------------------------------------------------------------------------------

inline	void		FwTransform::SetZAxis( FwVector_arg zAxis )
{
	m_matrix.SetRow( 2, zAxis.GetVector4() );
}

//--------------------------------------------------------------------------------------------------
/**
	@brief			Sets the point representing the translation
**/
//--------------------------------------------------------------------------------------------------

inline	void		FwTransform::SetTranslation( FwPoint_arg translation )
{
	m_matrix.SetRow( 3, translation.GetVector4() );
}

//--------------------------------------------------------------------------------------------------
/**
	@brief			Returns the float a specific row & column in the object

	@note			Intentionally restricts access to 4th column, due to indeterminate stored value.
**/
//--------------------------------------------------------------------------------------------------

inline	FwScalar	FwTransform::Get( int row, int col ) const
{
	assert( ( row >= 0 ) && ( row <= 3 ) );
	assert( ( col >= 0 ) && ( col <= 2 ) );
	return m_matrix.Get( row, col );
}

//--------------------------------------------------------------------------------------------------
/**
	@brief			Sets the float at a specific row & column in the object

	@note			Intentionally restricts access to 4th column, due to indeterminate stored value.
**/
//--------------------------------------------------------------------------------------------------

inline	void	FwTransform::Set( int row, int col, FwScalar_arg val )
{
	assert( ( row >= 0 ) && ( row <= 3 ) );
	assert( ( col >= 0 ) && ( col <= 2 ) );
	m_matrix.Set( row, col, val );
}

//--------------------------------------------------------------------------------------------------
/**
	@brief			'this = rhs'
**/
//--------------------------------------------------------------------------------------------------

inline	FwTransform&	FwTransform::operator = ( const FwTransform& rhs )
{
	m_matrix = rhs.m_matrix;
	return *this;
}

//--------------------------------------------------------------------------------------------------
/**
	@brief			Assigns current object to FwMaths::kZero, setting matrix to zero
**/
//--------------------------------------------------------------------------------------------------

inline	FwTransform&	FwTransform::operator = ( const FwMaths::ZeroTag& )
{
	m_matrix = FwMaths::kZero;
	return *this;
}

//--------------------------------------------------------------------------------------------------
/**
	@brief			Assigns current object to FwMaths::kIdentity, setting matrix to identity
**/
//--------------------------------------------------------------------------------------------------

inline	FwTransform&	FwTransform::operator = ( const FwMaths::IdentityTag& )
{
	m_matrix = FwMaths::kIdentity;
	return *this;
}

//--------------------------------------------------------------------------------------------------
/**
	@brief			Returns 'this * rhs' (FwTransform * FwTransform)
**/
//--------------------------------------------------------------------------------------------------

inline	FwTransform		FwTransform::operator * ( const FwTransform& rhs ) const
{
	return FwTransform( MulTransformMatrix( m_matrix, rhs.m_matrix ) );
}

//--------------------------------------------------------------------------------------------------
/**
	@brief			Sets current transform to 'this * rhs'
**/
//--------------------------------------------------------------------------------------------------

inline	FwTransform&	FwTransform::operator *= ( const FwTransform& rhs )
{
	m_matrix = MulTransformMatrix( m_matrix, rhs.m_matrix );
	return *this;
}

//--------------------------------------------------------------------------------------------------
/**
	@brief			Returns a 4x4 matrix with a properly initialised 4th column of (0,0,0,1)
**/
//--------------------------------------------------------------------------------------------------

inline	FwMatrix44		FwTransform::GetMatrix44( void ) const
{
	return FwMatrix44(	FwVector( m_matrix.GetRow( 0 ) ).GetVector4(),
						FwVector( m_matrix.GetRow( 1 ) ).GetVector4(),
						FwVector( m_matrix.GetRow( 2 ) ).GetVector4(),
						FwPoint( m_matrix.GetRow( 3 ) ).GetVector4() );
}
	
//--------------------------------------------------------------------------------------------------
/**
	@brief			Returns a 4x4 matrix with an undefined 4th column
**/
//--------------------------------------------------------------------------------------------------

inline	FwMatrix44		FwTransform::GetRawMatrix44( void ) const
{
	return m_matrix;
}

//--------------------------------------------------------------------------------------------------
/**
	@brief			Returns 'lhs * rhs' (FwVector4 * FwTransform)
**/
//--------------------------------------------------------------------------------------------------

inline	FwVector4	operator * ( FwVector4_arg lhs, const FwTransform& rhs )
{
	return FwVector4( MulVector4Transform( lhs, rhs.m_matrix ) );
}

//--------------------------------------------------------------------------------------------------
/**
	@brief			Returns 'lhs * rhs' (FwVector * FwTransform)
**/
//--------------------------------------------------------------------------------------------------

inline	FwVector	operator * ( FwVector_arg lhs, const FwTransform& rhs )
{
	return FwVector( MulVectorMatrix( lhs.GetVector4(), rhs.m_matrix ) );
}

//--------------------------------------------------------------------------------------------------
/**
	@brief			Returns 'lhs * rhs' (FwPoint * FwTransform)
**/
//--------------------------------------------------------------------------------------------------

inline	FwPoint		operator * ( FwPoint_arg lhs, const FwTransform& rhs )
{
	return FwPoint( MulPointMatrix( lhs.GetVector4(), rhs.m_matrix ) );
}

//--------------------------------------------------------------------------------------------------
/**
	@brief			Returns the inverse of the specified affine transform
**/
//--------------------------------------------------------------------------------------------------

inline	FwTransform	Inverse( const FwTransform& mat )
{
	return FwTransform( AffineInverse( mat.m_matrix ) );
}

//--------------------------------------------------------------------------------------------------
/**
	@brief			Returns the inverse of the specified affine transform 

	@note			This should be used when 3x3 rotation component is orthogonal, as it's faster.
**/
//--------------------------------------------------------------------------------------------------

inline	FwTransform	OrthogonalInverse( const FwTransform& mat )
{
	return FwTransform( AffineOrthogonalInverse( mat.m_matrix ) );
}

//--------------------------------------------------------------------------------------------------
/**
	@brief			Returns 'affine trans * mat' (FwTransform * FwMatrix44)

	@note			See FwMatrix44::MulTransformMatrix()
**/
//--------------------------------------------------------------------------------------------------

inline	FwMatrix44	MulTransformMatrix( const FwTransform& trans, const FwMatrix44& mat )
{
	return MulTransformMatrix( trans.m_matrix, mat );
}

#endif	// FW_TRANSFORM_INL
