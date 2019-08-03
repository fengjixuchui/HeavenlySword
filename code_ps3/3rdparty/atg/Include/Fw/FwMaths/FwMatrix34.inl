//--------------------------------------------------------------------------------------------------
/**
	@file		
	
	@brief		SIMD 3x4 Matrix

	@note		(c) Copyright Sony Computer Entertainment 2005. All Rights Reserved.	
**/
//--------------------------------------------------------------------------------------------------

#ifndef FW_MATRIX34_INL
#define FW_MATRIX34_INL

//--------------------------------------------------------------------------------------------------
/**
	@brief			Default constructor
**/
//--------------------------------------------------------------------------------------------------

inline	FwMatrix34::FwMatrix34()
{
}

//--------------------------------------------------------------------------------------------------
/**
	@brief			Copy constructor
**/
//--------------------------------------------------------------------------------------------------

inline	FwMatrix34::FwMatrix34( const FwMatrix34& rhs )
{
	m_row[0] = rhs.m_row[0];
	m_row[1] = rhs.m_row[1];
	m_row[2] = rhs.m_row[2];
}

//--------------------------------------------------------------------------------------------------
/**
	@brief			Construct from 3 vectors
**/
//--------------------------------------------------------------------------------------------------

inline	FwMatrix34::FwMatrix34( FwVector4_arg row0, FwVector4_arg row1, FwVector4_arg row2 )
{
	m_row[0] = row0;
	m_row[1] = row1;
	m_row[2] = row2;
}

//--------------------------------------------------------------------------------------------------
/**
	@brief			Constructs to zero
**/
//--------------------------------------------------------------------------------------------------

inline	FwMatrix34::FwMatrix34( const FwMaths::ZeroTag& )
{
	m_row[0] = FwMaths::kZero;
	m_row[1] = FwMaths::kZero;
	m_row[2] = FwMaths::kZero;
}

//--------------------------------------------------------------------------------------------------
/**
	@brief			Constructs to identity 
**/
//--------------------------------------------------------------------------------------------------

inline	FwMatrix34::FwMatrix34( const FwMaths::IdentityTag& )
{
	m_row[0] = FwMaths::kUnitXAxis;
	m_row[1] = FwMaths::kUnitYAxis;
	m_row[2] = FwMaths::kUnitZAxis;
}

//--------------------------------------------------------------------------------------------------
/**
	@brief			Constructs from an affine 4x4 matrix 
**/
//--------------------------------------------------------------------------------------------------

inline	FwMatrix34::FwMatrix34( const FwMatrix44& mat )
{
	v128	row0	= mat.GetRow(0).QuadwordValue();
	v128	row1	= mat.GetRow(1).QuadwordValue();
	v128	row2	= mat.GetRow(2).QuadwordValue();
	v128	row3	= mat.GetRow(3).QuadwordValue();

	vector float	temp0	= vec_mergeh( row0, row2 );
	vector float	temp1	= vec_mergeh( row1, row3 );
	vector float	temp2	= vec_mergel( row0, row2 );
	vector float	temp3	= vec_mergel( row1, row3 );

	m_row[0] = FwVector4( vec_mergeh( temp0, temp1 ) );
	m_row[1] = FwVector4( vec_mergel( temp0, temp1 ) );
	m_row[2] = FwVector4( vec_mergeh( temp2, temp3 ) );
}
	
//--------------------------------------------------------------------------------------------------
/**
	@brief			Returns the row associated with a specific index
	@param			i				Index within current object (between 0 and 2)
**/
//--------------------------------------------------------------------------------------------------

inline	const FwVector4	FwMatrix34::operator []( int index ) const
{
	assert( ( index >= 0 ) && ( index <= 2 ) );
	return m_row[ index ];
}

//--------------------------------------------------------------------------------------------------
/**
	@brief			Returns the row associated with a specific index
	@param			i				Index within current object (between 0 and 2)
**/
//--------------------------------------------------------------------------------------------------

inline	FwVector4	FwMatrix34::GetRow( int row ) const
{
	assert( ( row >= 0 ) && ( row <= 2 ) );
	return m_row[ row ];
}

//--------------------------------------------------------------------------------------------------
/**
	@brief			Returns the column associated with a specific index
	@param			i				Index within current object (between 0 and 3)
**/
//--------------------------------------------------------------------------------------------------

inline	FwVector4	FwMatrix34::GetCol( int col ) const
{
	assert( ( col >= 0 ) && ( col <= 3 ) );
	return FwVector4( m_row[0].Get( col ), m_row[1].Get( col ), m_row[2].Get( col ), 0.0f );
}

//--------------------------------------------------------------------------------------------------
/**
	@brief			Sets the row associated with a specific index
**/
//--------------------------------------------------------------------------------------------------

inline	void		FwMatrix34::SetRow( int row, FwVector4_arg rhs )
{
	assert( ( row >= 0 ) && ( row <= 2 ) );
	m_row[ row ] = rhs;
}

//--------------------------------------------------------------------------------------------------
/**
	@brief			Sets the column associated with a specific index
**/
//--------------------------------------------------------------------------------------------------

inline	void		FwMatrix34::SetCol( int col, FwVector4_arg rhs )
{
	assert( ( col >= 0 ) && ( col <= 3 ) );
	m_row[0].Set( col, rhs.X() );
	m_row[1].Set( col, rhs.Y() );
	m_row[2].Set( col, rhs.Z() );
}

//--------------------------------------------------------------------------------------------------
/**
	@brief			Returns the float a specific row & column in the object
**/
//--------------------------------------------------------------------------------------------------

inline	FwScalar	FwMatrix34::Get( int row, int col ) const
{
	assert( ( row >= 0 ) && ( row <= 2 ) );
	assert( ( col >= 0 ) && ( col <= 3 ) );
	return m_row[ row ].Get(col);
}

//--------------------------------------------------------------------------------------------------
/**
	@brief			Sets the float at a specific row & column in the object
**/
//--------------------------------------------------------------------------------------------------

inline	void		FwMatrix34::Set( int row, int col, FwScalar_arg val )
{
	assert( ( row >= 0 ) && ( row <= 2 ) );
	assert( ( col >= 0 ) && ( col <= 3 ) );
	FwVector4 work = m_row[ row ];
	work.Set( col, val );
	m_row[ row ] = work;
}

//--------------------------------------------------------------------------------------------------
/**
	@brief			'this = rhs'
**/
//--------------------------------------------------------------------------------------------------

inline	FwMatrix34&	FwMatrix34::operator = ( const FwMatrix34& rhs )
{
	m_row[0] = rhs.m_row[0];
	m_row[1] = rhs.m_row[1];
	m_row[2] = rhs.m_row[2];
	return *this;
}

//--------------------------------------------------------------------------------------------------
/**
	@brief			Assigns current object to FwMaths::kZero, setting matrix to zero
**/
//--------------------------------------------------------------------------------------------------

inline	FwMatrix34&	FwMatrix34::operator = ( const FwMaths::ZeroTag& )
{
	m_row[0] = FwMaths::kZero;
	m_row[1] = FwMaths::kZero;
	m_row[2] = FwMaths::kZero;
	return *this;
}

//--------------------------------------------------------------------------------------------------
/**
	@brief			Assigns current object to FwMaths::kIdentity, setting matrix to identity
**/
//--------------------------------------------------------------------------------------------------

inline	FwMatrix34&	FwMatrix34::operator = ( const FwMaths::IdentityTag& )
{
	m_row[0] = FwMaths::kUnitXAxis;
	m_row[1] = FwMaths::kUnitYAxis;
	m_row[2] = FwMaths::kUnitZAxis;
	return *this;
}

//--------------------------------------------------------------------------------------------------
/**
	@brief			Returns a 4x4 matrix with a properly initialised 4th column of (0,0,0,1)
**/
//--------------------------------------------------------------------------------------------------

inline	FwMatrix44	FwMatrix34::GetMatrix44( void ) const
{
	v128	row0	= m_row[0].QuadwordValue();
	v128	row1	= m_row[1].QuadwordValue();
	v128	row2	= m_row[2].QuadwordValue();
	v128	row3	= _VEC_GET_UNIT_W();

	vector float	temp0	= vec_mergeh( row0, row2 );
	vector float	temp1	= vec_mergeh( row1, row3 );
	vector float	temp2	= vec_mergel( row0, row2 );
	vector float	temp3	= vec_mergel( row1, row3 );

	return FwMatrix44(	FwVector4( vec_mergeh( temp0, temp1 ) ),
						FwVector4( vec_mergel( temp0, temp1 ) ),
						FwVector4( vec_mergeh( temp2, temp3 ) ),
						FwVector4( vec_mergel( temp2, temp3 ) ) );
}
	
#endif	// FW_MATRIX34_INL
