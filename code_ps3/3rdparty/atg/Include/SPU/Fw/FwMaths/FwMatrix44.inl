//--------------------------------------------------------------------------------------------------
/**
	@file		
	
	@brief		SIMD 4x4 Matrix

	@note		(c) Copyright Sony Computer Entertainment 2005. All Rights Reserved.	
**/
//--------------------------------------------------------------------------------------------------

#ifndef FW_MATRIX44_INL
#define FW_MATRIX44_INL

//--------------------------------------------------------------------------------------------------
/**
	@brief			Default constructor
**/
//--------------------------------------------------------------------------------------------------

inline	FwMatrix44::FwMatrix44()
{
}

//--------------------------------------------------------------------------------------------------
/**
	@brief			Copy constructor
**/
//--------------------------------------------------------------------------------------------------

inline	FwMatrix44::FwMatrix44( const FwMatrix44& rhs )
{
	m_row[0] = rhs.m_row[0];
	m_row[1] = rhs.m_row[1];
	m_row[2] = rhs.m_row[2];
	m_row[3] = rhs.m_row[3];
}

//--------------------------------------------------------------------------------------------------
/**
	@brief			Construct from 4 vectors
**/
//--------------------------------------------------------------------------------------------------

inline	FwMatrix44::FwMatrix44( FwVector4_arg row0, FwVector4_arg row1, FwVector4_arg row2, FwVector4_arg row3 )
{
	m_row[0] = row0;
	m_row[1] = row1;
	m_row[2] = row2;
	m_row[3] = row3;
}

//--------------------------------------------------------------------------------------------------
/**
	@brief			Constructs to zero
**/
//--------------------------------------------------------------------------------------------------

inline	FwMatrix44::FwMatrix44( const FwMaths::ZeroTag& )
{
	m_row[0] = FwMaths::kZero;
	m_row[1] = FwMaths::kZero;
	m_row[2] = FwMaths::kZero;
	m_row[3] = FwMaths::kZero;
}

//--------------------------------------------------------------------------------------------------
/**
	@brief			Constructs to identity
**/
//--------------------------------------------------------------------------------------------------

inline	FwMatrix44::FwMatrix44( const FwMaths::IdentityTag& )
{
	m_row[0] = FwMaths::kUnitXAxis;
	m_row[1] = FwMaths::kUnitYAxis;
	m_row[2] = FwMaths::kUnitZAxis;
	m_row[3] = FwMaths::kUnitWAxis;
}

//--------------------------------------------------------------------------------------------------
/**
	@brief			Constructs from an axis and angle
**/
//--------------------------------------------------------------------------------------------------

inline	FwMatrix44::FwMatrix44( FwVector4_arg axis, float angle )
{
	v128	axisVec;
	v128	sinAngle;
	v128	cosAngle;
	v128	oneMinusC;
	v128	axisS;
	v128	negAxisS;
	v128	xxxx;
	v128	yyyy;
	v128	zzzz;
	v128	tmp0;
	v128	tmp1;
	v128	tmp2;

	axisVec = axis.QuadwordValue();
	sincosf4( _VEC_REPLICATE_FLOAT( angle ), &sinAngle, &cosAngle );
	xxxx		= _VEC_REPLICATE_X( axisVec );
	yyyy		= _VEC_REPLICATE_Y( axisVec );
	zzzz		= _VEC_REPLICATE_Z( axisVec );
	oneMinusC	= _VEC_SUB( ( vector float )( 1.0f ), cosAngle );
	axisS		= _VEC_MUL( axisVec, sinAngle );
	negAxisS	= _VEC_NEG( axisS );

	tmp0	= vec_perm( axisS, negAxisS, _VEC_GET_CONSTANT( kShufXZBX ) );
	tmp1	= vec_perm( axisS, negAxisS, _VEC_GET_CONSTANT( kShufCXXX ) );
	tmp2	= vec_perm( axisS, negAxisS, _VEC_GET_CONSTANT( kShufYAXX ) );
	tmp0	= vec_sel( tmp0, cosAngle, ( vector unsigned int )_VEC_GET_MASK_X() );
	tmp1	= vec_sel( tmp1, cosAngle, ( vector unsigned int )_VEC_GET_MASK_Y() );
	tmp2	= vec_sel( tmp2, cosAngle, ( vector unsigned int )_VEC_GET_MASK_Z() );

	axisVec = _VEC_AND( axisVec, _VEC_GET_MASKOFF_W() );
	tmp0	= _VEC_AND( tmp0, _VEC_GET_MASKOFF_W() );
	tmp1	= _VEC_AND( tmp1, _VEC_GET_MASKOFF_W() );
	tmp2	= _VEC_AND( tmp2, _VEC_GET_MASKOFF_W() );

	m_row[0] = FwVector4( _VEC_MADD( _VEC_MUL( axisVec, xxxx ), oneMinusC, tmp0 ) );
	m_row[1] = FwVector4( _VEC_MADD( _VEC_MUL( axisVec, yyyy ), oneMinusC, tmp1 ) );
	m_row[2] = FwVector4( _VEC_MADD( _VEC_MUL( axisVec, zzzz ), oneMinusC, tmp2 ) );
	m_row[3] = FwMaths::kUnitWAxis;
}
	
//--------------------------------------------------------------------------------------------------
/**
	@brief			Returns the row associated with a specific index
	@param			i				Index within current object (between 0 and 3)
**/
//--------------------------------------------------------------------------------------------------

inline	const FwVector4	FwMatrix44::operator []( int index ) const
{
	assert( ( index >= 0 ) && ( index <= 3 ) );
	return m_row[ index ];
}

//--------------------------------------------------------------------------------------------------
/**
	@brief			Returns the row associated with a specific index
	@param			i				Index within current object (between 0 and 3)
**/
//--------------------------------------------------------------------------------------------------

inline	FwVector4	FwMatrix44::GetRow( int row ) const
{
	assert( ( row >= 0 ) && ( row <= 3 ) );
	return m_row[ row ];
}

//--------------------------------------------------------------------------------------------------
/**
	@brief			Returns the column associated with a specific index
	@param			i				Index within current object (between 0 and 3)
**/
//--------------------------------------------------------------------------------------------------

inline	FwVector4	FwMatrix44::GetCol( int col ) const
{
	assert( ( col >= 0 ) && ( col <= 3 ) );
	return FwVector4( m_row[0].Get( col ), m_row[1].Get( col ), m_row[2].Get( col ), m_row[3].Get( col ) );
}

//--------------------------------------------------------------------------------------------------
/**
	@brief			Sets the row associated with a specific index
**/
//--------------------------------------------------------------------------------------------------

inline	void		FwMatrix44::SetRow( int row, FwVector4_arg rhs )
{
	assert( ( row >= 0 ) && ( row <= 3 ) );
	m_row[ row ] = rhs;
}

//--------------------------------------------------------------------------------------------------
/**
	@brief			Sets the column associated with a specific index
**/
//--------------------------------------------------------------------------------------------------

inline	void		FwMatrix44::SetCol( int col, FwVector4_arg rhs )
{
	assert( ( col >= 0 ) && ( col <= 3 ) );
	m_row[0].Set( col, rhs.X() );
	m_row[1].Set( col, rhs.Y() );
	m_row[2].Set( col, rhs.Z() );
	m_row[3].Set( col, rhs.W() );
}

//--------------------------------------------------------------------------------------------------
/**
	@brief			Returns the float a specific row & column in the object
**/
//--------------------------------------------------------------------------------------------------

inline	FwScalar	FwMatrix44::Get( int row, int col ) const
{
	assert( ( row >= 0 ) && ( row <= 3 ) );
	assert( ( col >= 0 ) && ( col <= 3 ) );
	return m_row[ row ].Get(col);
}

//--------------------------------------------------------------------------------------------------
/**
	@brief			Sets the float at a specific row & column in the object
**/
//--------------------------------------------------------------------------------------------------

inline	void	FwMatrix44::Set( int row, int col, FwScalar_arg val )
{
	assert( ( row >= 0 ) && ( row <= 3 ) );
	assert( ( col >= 0 ) && ( col <= 3 ) );
	FwVector4 work = m_row[ row ];
	work.Set( col, val );
	m_row[ row ] = work;
}

//--------------------------------------------------------------------------------------------------
/**
	@brief			Sets the matrix to hold an object that can scale all 4 axes
**/
//--------------------------------------------------------------------------------------------------

inline	void	FwMatrix44::SetScale( FwVector4_arg scale )
{
	v128	scaleValue = scale.QuadwordValue();
	m_row[0] = FwVector4( _VEC_AND( scaleValue, _VEC_GET_MASK_X() ) );
	m_row[1] = FwVector4( _VEC_AND( scaleValue, _VEC_GET_MASK_Y() ) );
	m_row[2] = FwVector4( _VEC_AND( scaleValue, _VEC_GET_MASK_Z() ) );
	m_row[3] = FwVector4( _VEC_AND( scaleValue, _VEC_GET_MASK_W() ) );
}

//--------------------------------------------------------------------------------------------------
/**
	@brief			Sets the matrix to hold a matrix representing a rotation around X
**/
//--------------------------------------------------------------------------------------------------

inline	void	FwMatrix44::SetRotateX( float angle )
{
	v128	sinAngle;
	v128	cosAngle;
	v128	row1;
	v128	row2;

	sincosf4( _VEC_REPLICATE_FLOAT( angle ), &sinAngle, &cosAngle );
	row1 = vec_sel( ( vector float )( 0.0f ), cosAngle, ( vector unsigned int )_VEC_GET_MASK_Y() );
	row1 = vec_sel( row1, sinAngle, ( vector unsigned int )_VEC_GET_MASK_Z() );
	
	row2 = vec_sel( ( vector float )( 0.0f ), _VEC_NEG( sinAngle ), ( vector unsigned int )_VEC_GET_MASK_Y() );
	row2 = vec_sel( row2, cosAngle, ( vector unsigned int )_VEC_GET_MASK_Z() );

	m_row[0] = FwMaths::kUnitXAxis;
	m_row[1] = FwVector4( row1 );
	m_row[2] = FwVector4( row2 );
	m_row[3] = FwMaths::kUnitWAxis;
}

//--------------------------------------------------------------------------------------------------
/**
	@brief			Sets the matrix to hold a matrix representing a rotation around Y
**/
//--------------------------------------------------------------------------------------------------

inline	void	FwMatrix44::SetRotateY( float angle )
{
	v128	sinAngle;
	v128	cosAngle;
	v128	row0;
	v128	row2;

	sincosf4( _VEC_REPLICATE_FLOAT( angle ), &sinAngle, &cosAngle );
	row0 = vec_sel( ( vector float )( 0.0f ), cosAngle, ( vector unsigned int )_VEC_GET_MASK_X() );
	row0 = vec_sel( row0, _VEC_NEG( sinAngle ), ( vector unsigned int )_VEC_GET_MASK_Z() );
	
	row2 = vec_sel( ( vector float )( 0.0f ), sinAngle, ( vector unsigned int )_VEC_GET_MASK_X() );
	row2 = vec_sel( row2, cosAngle, ( vector unsigned int )_VEC_GET_MASK_Z() );

	m_row[0] = FwVector4( row0 );
	m_row[1] = FwMaths::kUnitYAxis;
	m_row[2] = FwVector4( row2 );
	m_row[3] = FwMaths::kUnitWAxis;
}

//--------------------------------------------------------------------------------------------------
/**
	@brief			Sets the matrix to hold a matrix representing a rotation around Z
**/
//--------------------------------------------------------------------------------------------------

inline	void	FwMatrix44::SetRotateZ( float angle )
{
	v128	sinAngle;
	v128	cosAngle;
	v128	row0;
	v128	row1;

	sincosf4( _VEC_REPLICATE_FLOAT( angle ), &sinAngle, &cosAngle );
	row0 = vec_sel( ( vector float )( 0.0f ), cosAngle, ( vector unsigned int )_VEC_GET_MASK_X() );
	row0 = vec_sel( row0, sinAngle, ( vector unsigned int )_VEC_GET_MASK_Y() );
	
	row1 = vec_sel( ( vector float )( 0.0f ), _VEC_NEG( sinAngle ), ( vector unsigned int )_VEC_GET_MASK_X() );
	row1 = vec_sel( row1, cosAngle, ( vector unsigned int )_VEC_GET_MASK_Y() );

	m_row[0] = FwVector4( row0 );
	m_row[1] = FwVector4( row1 );
	m_row[2] = FwMaths::kUnitZAxis;
	m_row[3] = FwMaths::kUnitWAxis;
}
	
//--------------------------------------------------------------------------------------------------
/**
	@brief			'this = rhs'
**/
//--------------------------------------------------------------------------------------------------

inline	FwMatrix44&		FwMatrix44::operator = ( const FwMatrix44& rhs )
{
	m_row[0] = rhs.m_row[0];
	m_row[1] = rhs.m_row[1];
	m_row[2] = rhs.m_row[2];
	m_row[3] = rhs.m_row[3];
	return *this;
}

//--------------------------------------------------------------------------------------------------
/**
	@brief			Assigns current object to FwMaths::kZero, setting matrix to zero
**/
//--------------------------------------------------------------------------------------------------

inline	FwMatrix44&		FwMatrix44::operator = ( const FwMaths::ZeroTag& )
{
	m_row[0] = FwMaths::kZero;
	m_row[1] = FwMaths::kZero;
	m_row[2] = FwMaths::kZero;
	m_row[3] = FwMaths::kZero;
	return *this;
}

//--------------------------------------------------------------------------------------------------
/**
	@brief			Assigns current object to FwMaths::kIdentity, setting matrix to identity
**/
//--------------------------------------------------------------------------------------------------

inline	FwMatrix44&		FwMatrix44::operator = ( const FwMaths::IdentityTag& )
{
	m_row[0] = FwMaths::kUnitXAxis;
	m_row[1] = FwMaths::kUnitYAxis;
	m_row[2] = FwMaths::kUnitZAxis;
	m_row[3] = FwMaths::kUnitWAxis;
	return *this;
}

//--------------------------------------------------------------------------------------------------
/**
	@brief			Returns 'this + rhs'
**/
//--------------------------------------------------------------------------------------------------

inline	FwMatrix44		FwMatrix44::operator + ( const FwMatrix44& rhs ) const
{
	return FwMatrix44(	m_row[0] + rhs.m_row[0],
						m_row[1] + rhs.m_row[1],
						m_row[2] + rhs.m_row[2],
						m_row[3] + rhs.m_row[3] );
}

//--------------------------------------------------------------------------------------------------
/**
	@brief			Returns 'this - rhs'
**/
//--------------------------------------------------------------------------------------------------

inline	FwMatrix44		FwMatrix44::operator - ( const FwMatrix44& rhs ) const
{
	return FwMatrix44(	m_row[0] - rhs.m_row[0],
						m_row[1] - rhs.m_row[1],
						m_row[2] - rhs.m_row[2],
						m_row[3] - rhs.m_row[3] );
}

//--------------------------------------------------------------------------------------------------
/**
	@brief			Sets current object to 'this + rhs'
**/
//--------------------------------------------------------------------------------------------------

inline	FwMatrix44&		FwMatrix44::operator += ( const FwMatrix44& rhs )
{
	m_row[0] += rhs.m_row[0];
	m_row[1] += rhs.m_row[1];
	m_row[2] += rhs.m_row[2];
	m_row[3] += rhs.m_row[3];
	return *this;
}

//--------------------------------------------------------------------------------------------------
/**
	@brief			Sets current object to 'this - rhs'
**/
//--------------------------------------------------------------------------------------------------

inline	FwMatrix44&		FwMatrix44::operator -= ( const FwMatrix44& rhs )
{
	m_row[0] -= rhs.m_row[0];
	m_row[1] -= rhs.m_row[1];
	m_row[2] -= rhs.m_row[2];
	m_row[3] -= rhs.m_row[3];
	return *this;
}

//--------------------------------------------------------------------------------------------------
/**
	@brief			Returns '-this'
**/
//--------------------------------------------------------------------------------------------------

inline	FwMatrix44		FwMatrix44::operator - () const
{
	return FwMatrix44(	-m_row[0], -m_row[1], -m_row[2], -m_row[3] );
}

//--------------------------------------------------------------------------------------------------
/**
	@brief			Returns 'this * rhs scalar'
**/
//--------------------------------------------------------------------------------------------------

inline	FwMatrix44		FwMatrix44::operator * ( FwScalar_arg rhs ) const
{
	return FwMatrix44(	m_row[0] * rhs,
						m_row[1] * rhs,
						m_row[2] * rhs,
						m_row[3] * rhs );
}

//--------------------------------------------------------------------------------------------------
/**
	@brief			Sets current object to 'this * rhs scalar'
**/
//--------------------------------------------------------------------------------------------------

inline	FwMatrix44&		FwMatrix44::operator *= ( FwScalar_arg rhs )
{
	m_row[0] *= rhs;
	m_row[1] *= rhs;
	m_row[2] *= rhs;
	m_row[3] *= rhs;
	return *this;
}

//--------------------------------------------------------------------------------------------------
/**
	@brief			Returns 'this * rhs'
**/
//--------------------------------------------------------------------------------------------------

inline	FwMatrix44		FwMatrix44::operator * ( const FwMatrix44& rhs ) const
{
	return FwMatrix44(	m_row[0] * rhs,
						m_row[1] * rhs,
						m_row[2] * rhs,
						m_row[3] * rhs );
}

//--------------------------------------------------------------------------------------------------
/**
	@brief			Sets current object to 'this * rhs'
**/
//--------------------------------------------------------------------------------------------------

inline	FwMatrix44&		FwMatrix44::operator *= ( const FwMatrix44& rhs )
{
	*this = *this * rhs;
	return *this;
}

//--------------------------------------------------------------------------------------------------
/**
	@brief			Obtains euler angles from a rotation matrix (assuming X,Y,Z rotation order)

	@param			x					Reference to float that will contain X rotation
	@param			y					Reference to float that will contain Y rotation
	@param			z					Reference to float that will contain Z rotation

	@note			We may get round to supporting other rotation orders at some point in the future
**/
//--------------------------------------------------------------------------------------------------

inline	void	FwMatrix44::GetEulerAngles( float& x, float& y, float& z ) const
{
	FwVector4	rowWork = m_row[0] * m_row[0];
	float		cY = sqrtf( rowWork.X() + rowWork.Y() );
	
	if ( cY > 1e-6f )
	{
		x = atan2f( m_row[1].Z(), m_row[2].Z() );
		y = atan2f(-m_row[0].Z(), cY);
		z = atan2f( m_row[0].Y(), m_row[0].X() );
	}
	else
	{
		x = atan2f(-m_row[2].Y(), m_row[1].Y() );
		y = atan2f(-m_row[0].Z(), cY );
		z = 0.0f;
	}
}

//--------------------------------------------------------------------------------------------------
/**
	@brief			Returns 'lhs * rhs' (FwVector4 * FwMatrix44)
**/
//--------------------------------------------------------------------------------------------------

inline	FwVector4		operator * ( FwVector4_arg lhs, const FwMatrix44& rhs )
{
	v128	result;
	v128	lhsValue = lhs.QuadwordValue();
	result	= _VEC_MUL( rhs.m_row[0].QuadwordValue(), _VEC_REPLICATE_X( lhsValue ) );
	result	= _VEC_MADD( rhs.m_row[1].QuadwordValue(), _VEC_REPLICATE_Y( lhsValue ), result );
	result	= _VEC_MADD( rhs.m_row[2].QuadwordValue(), _VEC_REPLICATE_Z( lhsValue ), result );
	result	= _VEC_MADD( rhs.m_row[3].QuadwordValue(), _VEC_REPLICATE_W( lhsValue ), result );

	return FwVector4( result );
}

//--------------------------------------------------------------------------------------------------
/**
	@brief			Returns 'affine trans * mat' (FwMatrix44 * FwMatrix44)

	@note			This assumes that the 'trans' matrix represents an affine transform. The function
					behaves as though the 4th column is (0,0,0,1), and will return a fully initialised
					FwMatrix44.
**/
//--------------------------------------------------------------------------------------------------

inline	FwMatrix44		MulTransformMatrix( const FwMatrix44& trans, const FwMatrix44& mat )
{
	v128	rowResult0;
	v128	rowResult1;
	v128	rowResult2;
	v128	rowResult3;

	v128	transRow0	= trans.m_row[0].QuadwordValueXYZ();
	v128	transRow1	= trans.m_row[1].QuadwordValueXYZ();
	v128	transRow2	= trans.m_row[2].QuadwordValueXYZ();
	v128	transRow3	= trans.m_row[3].QuadwordValueXYZ();

	v128	matRow0		= mat.m_row[0].QuadwordValue();
	v128	matRow1		= mat.m_row[1].QuadwordValue();
	v128	matRow2		= mat.m_row[2].QuadwordValue();
	v128	matRow3		= mat.m_row[3].QuadwordValue();

	rowResult0	= _VEC_MUL( matRow0, _VEC_REPLICATE_X( transRow0 ) );
	rowResult0	= _VEC_MADD( matRow1, _VEC_REPLICATE_Y( transRow0 ), rowResult0 );
	rowResult0	= _VEC_MADD( matRow2, _VEC_REPLICATE_Z( transRow0 ), rowResult0 );

	rowResult1	= _VEC_MUL( matRow0, _VEC_REPLICATE_X( transRow1 ) );
	rowResult1	= _VEC_MADD( matRow1, _VEC_REPLICATE_Y( transRow1 ), rowResult1 );
	rowResult1	= _VEC_MADD( matRow2, _VEC_REPLICATE_Z( transRow1 ), rowResult1 );

	rowResult2	= _VEC_MUL( matRow0, _VEC_REPLICATE_X( transRow2 ) );
	rowResult2	= _VEC_MADD( matRow1, _VEC_REPLICATE_Y( transRow2 ), rowResult2 );
	rowResult2	= _VEC_MADD( matRow2, _VEC_REPLICATE_Z( transRow2 ), rowResult2 );

	rowResult3	= _VEC_MUL( matRow0, _VEC_REPLICATE_X( transRow3 ) );
	rowResult3	= _VEC_MADD( matRow1, _VEC_REPLICATE_Y( transRow3 ), rowResult3 );
	rowResult3	= _VEC_MADD( matRow2, _VEC_REPLICATE_Z( transRow3 ), rowResult3 );
	rowResult3	= _VEC_ADD( rowResult3, matRow3 );

	return FwMatrix44( FwVector4( rowResult0 ), FwVector4( rowResult1 ), FwVector4( rowResult2 ), FwVector4( rowResult3 ) );
}

//--------------------------------------------------------------------------------------------------
/**
	@brief			Returns 'point * mat' (FwVector4 * FwMatrix44)

	@note			Interprets the input FwVector4 as a point (ie with an implicit 1.0f in W), and 
					transforms it through the given matrix. 
**/
//--------------------------------------------------------------------------------------------------

inline	FwVector4		MulPointMatrix( FwVector4_arg point, const FwMatrix44& mat )
{
	v128	result;
	v128	pointValue = point.QuadwordValueXYZ();
	result	= _VEC_MUL( mat.m_row[0].QuadwordValue(), _VEC_REPLICATE_X( pointValue ) );
	result	= _VEC_MADD( mat.m_row[1].QuadwordValue(), _VEC_REPLICATE_Y( pointValue ), result );
	result	= _VEC_MADD( mat.m_row[2].QuadwordValue(), _VEC_REPLICATE_Z( pointValue ), result );
	result	= _VEC_ADD( result, mat.m_row[3].QuadwordValue() );
	
	return FwVector4( result );
}

//--------------------------------------------------------------------------------------------------
/**
	@brief			Returns 'vector * mat' (FwVector4 * FwMatrix44)

	@note			Interprets the input FwVector4 as a vector (ie with an implicit 0.0f in W), and 
					transforms it through the given matrix. 
**/
//--------------------------------------------------------------------------------------------------

inline	FwVector4		MulVectorMatrix( FwVector4_arg vec, const FwMatrix44& mat )
{
	v128	result;
	v128	vecValue = vec.QuadwordValueXYZ();
	result	= _VEC_MUL( mat.m_row[0].QuadwordValue(), _VEC_REPLICATE_X( vecValue ) );
	result	= _VEC_MADD( mat.m_row[1].QuadwordValue(), _VEC_REPLICATE_Y( vecValue ), result );
	result	= _VEC_MADD( mat.m_row[2].QuadwordValue(), _VEC_REPLICATE_Z( vecValue ), result );
	
	return FwVector4( result );
}

//--------------------------------------------------------------------------------------------------
/**
	@brief			Returns 'vec * mat' (FwVector4 * FwMatrix44)

	@note			Interprets the input FwMatrix44 as an affine transform, using it to transform the
					given FwVector4.
**/
//--------------------------------------------------------------------------------------------------

inline	FwVector4		MulVector4Transform( FwVector4_arg vec, const FwMatrix44& mat )	
{
	v128	result;
	v128	vecValue = vec.QuadwordValue();
	result	= _VEC_MUL( mat.m_row[0].QuadwordValueXYZ(), _VEC_REPLICATE_X( vecValue ) );
	result	= _VEC_MADD( mat.m_row[1].QuadwordValueXYZ(), _VEC_REPLICATE_Y( vecValue ), result );
	result	= _VEC_MADD( mat.m_row[2].QuadwordValueXYZ(), _VEC_REPLICATE_Z( vecValue ), result );
	result	= _VEC_MADD( mat.m_row[3].QuadwordValueXYZ(), _VEC_REPLICATE_W( vecValue ), result );
	
	return FwVector4( vec_sel( result, vecValue, ( vector unsigned int )_VEC_GET_MASK_W() ) );
}

//--------------------------------------------------------------------------------------------------
/**
	@brief			Returns an object containing absolute values of each element in current object
**/
//--------------------------------------------------------------------------------------------------

inline	FwMatrix44		Abs( const FwMatrix44& mat )
{
	v128	row0Abs = _VEC_ABS( mat.m_row[0].QuadwordValue() );
	v128	row1Abs = _VEC_ABS( mat.m_row[1].QuadwordValue() );
	v128	row2Abs = _VEC_ABS( mat.m_row[2].QuadwordValue() );
	v128	row3Abs = _VEC_ABS( mat.m_row[3].QuadwordValue() );

	return FwMatrix44( FwVector4( row0Abs ), FwVector4( row1Abs ), FwVector4( row2Abs ), FwVector4( row3Abs ) );
}

//--------------------------------------------------------------------------------------------------
/**
	@brief			Transposes the specified matrix, returning transposed result
**/
//--------------------------------------------------------------------------------------------------

inline	FwMatrix44		Transpose( const FwMatrix44& mat )
{
	v128	row0	= mat.m_row[0].QuadwordValue();
	v128	row1	= mat.m_row[1].QuadwordValue();
	v128	row2	= mat.m_row[2].QuadwordValue();
	v128	row3	= mat.m_row[3].QuadwordValue();

	v128	temp0	= vec_mergeh( row0, row2 );
	v128	temp1	= vec_mergeh( row1, row3 );
	v128	temp2	= vec_mergel( row0, row2 );
	v128	temp3	= vec_mergel( row1, row3 );

	v128	res0 = vec_mergeh( temp0, temp1 );
	v128	res1 = vec_mergel( temp0, temp1 );
	v128	res2 = vec_mergeh( temp2, temp3 );
	v128	res3 = vec_mergel( temp2, temp3 );

	return FwMatrix44( FwVector4( res0 ), FwVector4( res1 ), FwVector4( res2 ), FwVector4( res3 ) );
}

//--------------------------------------------------------------------------------------------------
/**
	@brief			Returns the determinant of the top-left 3x3 section of the current 4x4 matrix
**/
//--------------------------------------------------------------------------------------------------

inline	FwScalar		Determinant3( const FwMatrix44& mat )
{
	v128	crossRes;	_VEC_CROSS( crossRes, mat.m_row[0].QuadwordValue(), mat.m_row[1].QuadwordValue() );
	v128	dot3Res;	_VEC_DOT3( dot3Res, mat.m_row[2].QuadwordValue(), crossRes );
	return	FwScalar( _VEC_REPLICATE_X( dot3Res ) );
}

//--------------------------------------------------------------------------------------------------
/**
	@brief			Returns the inverse of the specified affine transform 
	
	@note			This assumes that the input matrix represents an affine transform. The function
					behaves as though the 4th column is (0,0,0,1), and will return a fully initialised
					FwMatrix44.
**/
//--------------------------------------------------------------------------------------------------

inline	FwMatrix44		AffineInverse( const FwMatrix44& mat )
{
	v128	row0	= mat.m_row[0].QuadwordValue();
	v128	row1	= mat.m_row[1].QuadwordValue();
	v128	row2	= mat.m_row[2].QuadwordValue();
	v128	row3	= mat.m_row[3].QuadwordValue();

	v128	inv2;	_VEC_CROSS( inv2, row0, row1 );
	v128	inv0;	_VEC_CROSS( inv0, row1, row2 );
	v128	inv1;	_VEC_CROSS( inv1, row2, row0 );
	v128	inv3 =	_VEC_NEG( row3 );
	
	v128	dotVec;	_VEC_DOT3( dotVec, inv2, row2 );
	v128	invDet; _VEC_RCP( invDet, _VEC_REPLICATE_X( dotVec ) );

	// Transpose elements..
	v128	temp0	= vec_mergeh( inv0, inv2 );
	v128	temp1	= vec_mergeh( inv1, ( vector float )( 0.0f ) );
	v128	temp2	= vec_mergel( inv0, inv2 );
	v128	temp3	= vec_mergel( inv1, ( vector float )( 0.0f ) );

	inv0 = vec_mergeh( temp0, temp1 );
	inv1 = vec_mergel( temp0, temp1 );
	inv2 = vec_mergeh( temp2, temp3 );

	v128	inv3Temp = inv3;

	inv3	= _VEC_MUL( inv0, _VEC_REPLICATE_X( inv3Temp ) );
	inv3	= vec_madd( inv1, _VEC_REPLICATE_Y( inv3Temp ), inv3 );
	inv3	= vec_madd( inv2, _VEC_REPLICATE_Z( inv3Temp ), inv3 );

	invDet	= vec_and( invDet, _VEC_GET_MASKOFF_W() );

	inv0	= _VEC_MUL( inv0, invDet );
	inv1	= _VEC_MUL( inv1, invDet );
	inv2	= _VEC_MUL( inv2, invDet );
	inv3	= _VEC_MUL( inv3, invDet );
	inv3	= vec_add( inv3, _VEC_GET_UNIT_W() );
	
	return FwMatrix44( FwVector4( inv0 ), FwVector4( inv1 ), FwVector4( inv2 ), FwVector4( inv3 ) );
}

//--------------------------------------------------------------------------------------------------
/**
	@brief			Returns the inverse of the specified affine transform 

	@note			This assumes that the input matrix represents an affine transform. The function
					behaves as though the 4th column is (0,0,0,1), and will return a fully initialised
					FwMatrix44.
					
					This should be used when 3x3 rotation component is orthogonal, as it's faster.
**/
//--------------------------------------------------------------------------------------------------

inline	FwMatrix44		AffineOrthogonalInverse( const FwMatrix44& mat )
{
	v128	row0	= mat.m_row[0].QuadwordValue();
	v128	row1	= mat.m_row[1].QuadwordValue();
	v128	row2	= mat.m_row[2].QuadwordValue();
	v128	row3	= mat.m_row[3].QuadwordValue();

	v128	temp0	= vec_mergeh( row0, row2 );
	v128	temp1	= vec_mergeh( row1, ( vector float )( 0.0f ) );
	v128	temp2	= vec_mergel( row0, row2 );
	v128	temp3	= vec_mergel( row1, ( vector float )( 0.0f ) );

	v128	inv0 	= vec_mergeh( temp0, temp1 );
	v128	inv1 	= vec_mergel( temp0, temp1 );
	v128	inv2 	= vec_mergeh( temp2, temp3 );

	v128	inv3 = _VEC_NEG( row3 );

	v128	inv3Temp = inv3;

	inv3	= _VEC_MUL( inv0, _VEC_REPLICATE_X( inv3Temp ) );
	inv3	= vec_madd( inv1, _VEC_REPLICATE_Y( inv3Temp ), inv3 );
	inv3	= vec_madd( inv2, _VEC_REPLICATE_Z( inv3Temp ), inv3 );

	inv0	= _VEC_AND( inv0, _VEC_GET_MASKOFF_W() );
	inv1	= _VEC_AND( inv1, _VEC_GET_MASKOFF_W() );
	inv2	= _VEC_AND( inv2, _VEC_GET_MASKOFF_W() );
	inv3	= _VEC_AND( inv3, _VEC_GET_MASKOFF_W() );
	inv3	= vec_or( inv3, _VEC_GET_UNIT_W() );

	return FwMatrix44( FwVector4( inv0 ), FwVector4( inv1 ), FwVector4( inv2 ), FwVector4( inv3 ) );
}

//--------------------------------------------------------------------------------------------------
/**
	@brief			Same as 'FwMatrix44 * FwScalar', but keeps scalar behaviour when scalar is on lhs
**/
//--------------------------------------------------------------------------------------------------

inline	FwMatrix44		operator * ( FwScalar_arg scalar, const FwMatrix44& mat )
{
	return mat * scalar;
}

#endif	// FW_MATRIX44_INL
