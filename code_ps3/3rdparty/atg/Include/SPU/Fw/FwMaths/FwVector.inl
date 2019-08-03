//--------------------------------------------------------------------------------------------------
/**
	@file		
	
	@brief		SIMD Vector

	@note		(c) Copyright Sony Computer Entertainment 2005. All Rights Reserved.	
**/
//--------------------------------------------------------------------------------------------------

#ifndef FW_VECTOR_INL
#define FW_VECTOR_INL

//--------------------------------------------------------------------------------------------------
/**
	@brief			Default constructor
**/
//--------------------------------------------------------------------------------------------------

inline	FwVector::FwVector( )
{
	_VEC_INITIALISE_QUADWORD( m_value );
}

//--------------------------------------------------------------------------------------------------
/**
	@brief			Constructs from 3 floats
**/
//--------------------------------------------------------------------------------------------------

inline	FwVector::FwVector( float x, float y, float z )
{
	_VEC_SET_FLOATS( m_value, x, y, z, z );
	_VEC_VALIDATE_QUADWORD_XYZ( m_value );
}

//--------------------------------------------------------------------------------------------------
/**
	@brief			Copy constructor
**/
//--------------------------------------------------------------------------------------------------

#ifdef	_MSC_VER
inline	FwVector::FwVector( const FwVector& rhs )
{
	_VEC_VALIDATE_QUADWORD_XYZ( rhs.m_value );
	m_value = rhs.m_value;
}
#endif	//_MSC_VER

//--------------------------------------------------------------------------------------------------
/**
	@brief			Constructs from 3 floats located at the specified address
**/
//--------------------------------------------------------------------------------------------------

inline	FwVector::FwVector( const float* pData )
{
	assert( pData );
	_VEC_SET_FLOATS( m_value, pData[ 0 ], pData[ 1 ], pData[ 2 ], pData[ 2 ] );
	_VEC_VALIDATE_QUADWORD_XYZ( m_value );
}

//--------------------------------------------------------------------------------------------------
/**
	@brief			Constructs from a raw quadword
**/
//--------------------------------------------------------------------------------------------------

inline	FwVector::FwVector( v128 rhs )
{
	_VEC_VALIDATE_QUADWORD_XYZ( rhs );
	m_value = rhs;
}

//--------------------------------------------------------------------------------------------------
/**
	@brief			Constructs from a scalar value
**/
//--------------------------------------------------------------------------------------------------

inline	FwVector::FwVector( FwScalar_arg rhs )
{
	m_value = rhs.QuadwordValue();
}

//--------------------------------------------------------------------------------------------------
/**
	@brief			Constructs from a 4 element vector (W is effectively discarded)
**/
//--------------------------------------------------------------------------------------------------

inline	FwVector::FwVector( FwVector4_arg rhs )
{
	m_value = rhs.QuadwordValue();
}

//--------------------------------------------------------------------------------------------------
/**
	@brief			Constructs to object containing zero
**/
//--------------------------------------------------------------------------------------------------

inline	FwVector::FwVector( const FwMaths::ZeroTag& )
{
	m_value = _VEC_SET_ZERO();
}

//--------------------------------------------------------------------------------------------------
/**
	@brief			Constructs to object containing the unit X axis (1, 0, 0)
**/
//--------------------------------------------------------------------------------------------------

inline	FwVector::FwVector( const FwMaths::UnitXAxisTag& )
{
	m_value = _VEC_GET_UNIT_X();
}

//--------------------------------------------------------------------------------------------------
/**
	@brief			Constructs to object containing the unit Y axis (0, 1, 0)
**/
//--------------------------------------------------------------------------------------------------

inline	FwVector::FwVector( const FwMaths::UnitYAxisTag& )
{
	m_value = _VEC_GET_UNIT_Y();
}

//--------------------------------------------------------------------------------------------------
/**
	@brief			Constructs to object containing the unit Z axis (0, 0, 1)
**/
//--------------------------------------------------------------------------------------------------

inline	FwVector::FwVector( const FwMaths::UnitZAxisTag& )
{
	m_value = _VEC_GET_UNIT_Z();
}

//--------------------------------------------------------------------------------------------------
/**
	@brief			Assigns current object to FwMaths::kZero  -  (0, 0, 0, 0)
**/
//--------------------------------------------------------------------------------------------------

inline	FwVector	FwVector::operator = ( const FwMaths::ZeroTag& )
{
	m_value = _VEC_SET_ZERO();
	return *this;
}

//--------------------------------------------------------------------------------------------------
/**
	@brief			Assigns current object to FwMaths::kUnitXAxis, which sets object to (1, 0, 0).
**/
//--------------------------------------------------------------------------------------------------

inline	FwVector	FwVector::operator = ( const FwMaths::UnitXAxisTag& )
{
	m_value = _VEC_GET_UNIT_X();
	return *this;
}

//--------------------------------------------------------------------------------------------------
/**
	@brief			Assigns current object to FwMaths::kUnitYAxis, which sets object to (0, 1, 0).
**/
//--------------------------------------------------------------------------------------------------

inline	FwVector	FwVector::operator = ( const FwMaths::UnitYAxisTag& )
{
	m_value = _VEC_GET_UNIT_Y();
	return *this;
}

//--------------------------------------------------------------------------------------------------
/**
	@brief			Assigns current object to FwMaths::kUnitZAxis, which sets object to (0, 0, 1).
**/
//--------------------------------------------------------------------------------------------------

inline	FwVector	FwVector::operator = ( const FwMaths::UnitZAxisTag& )
{
	m_value = _VEC_GET_UNIT_Z();
	return *this;
}

//--------------------------------------------------------------------------------------------------
/**
	@brief			'this = rhs'
**/
//--------------------------------------------------------------------------------------------------

inline	FwVector	FwVector::operator = ( FwVector_arg v )
{
	_VEC_VALIDATE_QUADWORD_XYZ( v.m_value );
	m_value = v.m_value;
	return *this;
}

//--------------------------------------------------------------------------------------------------
/**
	@brief			Return 'this + rhs'
**/
//--------------------------------------------------------------------------------------------------

inline	FwVector	FwVector::operator + ( FwVector_arg rhs ) const
{
	_VEC_VALIDATE_QUADWORD_XYZ( m_value );
	_VEC_VALIDATE_QUADWORD_XYZ( rhs.m_value );
	return FwVector( _VEC_ADD( m_value, rhs.m_value ) );
}

//--------------------------------------------------------------------------------------------------
/**
	@brief			Return 'this + rhs scalar'
**/
//--------------------------------------------------------------------------------------------------

inline	FwVector	FwVector::operator + ( FwScalar_arg rhs ) const
{
	_VEC_VALIDATE_QUADWORD_XYZ( m_value );
	return FwVector( _VEC_ADD( m_value, rhs.QuadwordValue() ) );
}

//--------------------------------------------------------------------------------------------------
/**
	@brief			Return 'this - rhs'
**/
//--------------------------------------------------------------------------------------------------

inline	FwVector	FwVector::operator - ( FwVector_arg rhs ) const
{
	_VEC_VALIDATE_QUADWORD_XYZ( m_value );
	_VEC_VALIDATE_QUADWORD_XYZ( rhs.m_value );
	return FwVector( _VEC_SUB( m_value, rhs.m_value ) );
}

//--------------------------------------------------------------------------------------------------
/**
	@brief			Return 'this - rhs scalar'
**/
//--------------------------------------------------------------------------------------------------

inline	FwVector	FwVector::operator - ( FwScalar_arg rhs ) const
{
	_VEC_VALIDATE_QUADWORD_XYZ( m_value );
	return FwVector( _VEC_SUB( m_value, rhs.QuadwordValue() ) );
}

//--------------------------------------------------------------------------------------------------
/**
	@brief			Return 'this * rhs'
**/
//--------------------------------------------------------------------------------------------------

inline	FwVector	FwVector::operator * ( FwVector_arg rhs ) const
{
	_VEC_VALIDATE_QUADWORD_XYZ( m_value );
	_VEC_VALIDATE_QUADWORD_XYZ( rhs.m_value );
	return FwVector( _VEC_MUL( m_value, rhs.m_value ) );
}

//--------------------------------------------------------------------------------------------------
/**
	@brief			Return 'this * rhs scalar'
**/
//--------------------------------------------------------------------------------------------------

inline	FwVector	FwVector::operator * ( FwScalar_arg rhs ) const
{
	_VEC_VALIDATE_QUADWORD_XYZ( m_value );
	return FwVector( _VEC_MUL( m_value, rhs.QuadwordValue() ) );
}

//--------------------------------------------------------------------------------------------------
/**
	@brief			Return 'this / rhs'
**/
//--------------------------------------------------------------------------------------------------

inline	FwVector	FwVector::operator / ( FwVector_arg rhs ) const
{
	_VEC_VALIDATE_QUADWORD_XYZ( m_value );
	_VEC_VALIDATE_QUADWORD_XYZ( rhs.m_value );
	v128 divResult; _VEC_DIV( divResult, m_value, rhs.m_value );
	return FwVector( divResult );
}

//--------------------------------------------------------------------------------------------------
/**
	@brief			Return 'this / rhs scalar'
**/
//--------------------------------------------------------------------------------------------------

inline	FwVector	FwVector::operator / ( FwScalar_arg rhs ) const
{
	_VEC_VALIDATE_QUADWORD_XYZ( m_value );
	v128 divResult; _VEC_DIV( divResult, m_value, rhs.QuadwordValue() );
	return FwVector( divResult );
}

//--------------------------------------------------------------------------------------------------
/**
	@brief			Sets current object to 'this + rhs'
**/
//--------------------------------------------------------------------------------------------------

inline	FwVector	FwVector::operator += ( FwVector_arg rhs )
{
	_VEC_VALIDATE_QUADWORD_XYZ( m_value );
	_VEC_VALIDATE_QUADWORD_XYZ( rhs.m_value );
	m_value = _VEC_ADD( m_value, rhs.m_value );
	return *this;
}

//--------------------------------------------------------------------------------------------------
/**
	@brief			Sets current object to 'this + rhs scalar'
**/
//--------------------------------------------------------------------------------------------------

inline	FwVector	FwVector::operator += ( FwScalar_arg rhs )
{
	_VEC_VALIDATE_QUADWORD_XYZ( m_value );
	m_value = _VEC_ADD( m_value, rhs.QuadwordValue() );
	return *this;
}

//--------------------------------------------------------------------------------------------------
/**
	@brief			Sets current object to 'this - rhs'
**/
//--------------------------------------------------------------------------------------------------

inline	FwVector	FwVector::operator -= ( FwVector_arg rhs )
{
	_VEC_VALIDATE_QUADWORD_XYZ( m_value );
	_VEC_VALIDATE_QUADWORD_XYZ( rhs.m_value );
	m_value = _VEC_SUB( m_value, rhs.m_value );
	return *this;
}

//--------------------------------------------------------------------------------------------------
/**
	@brief			Sets current object to 'this - rhs scalar'
**/
//--------------------------------------------------------------------------------------------------

inline	FwVector	FwVector::operator -= ( FwScalar_arg rhs )
{
	_VEC_VALIDATE_QUADWORD_XYZ( m_value );
	m_value = _VEC_SUB( m_value, rhs.QuadwordValue() );
	return *this;
}

//--------------------------------------------------------------------------------------------------
/**
	@brief			Sets current object to 'this * rhs'
**/
//--------------------------------------------------------------------------------------------------

inline	FwVector	FwVector::operator *= ( FwVector_arg rhs )
{
	_VEC_VALIDATE_QUADWORD_XYZ( m_value );
	_VEC_VALIDATE_QUADWORD_XYZ( rhs.m_value );
	m_value = _VEC_MUL( m_value, rhs.m_value );
	return *this;
}

//--------------------------------------------------------------------------------------------------
/**
	@brief			Sets current object to 'this * rhs scalar' 
**/
//--------------------------------------------------------------------------------------------------

inline	FwVector	FwVector::operator *= ( FwScalar_arg rhs )
{
	_VEC_VALIDATE_QUADWORD_XYZ( m_value );
	m_value = _VEC_MUL( m_value, rhs.QuadwordValue() );
	return *this;
}

//--------------------------------------------------------------------------------------------------
/**
	@brief			Sets current object to 'this / rhs'
**/
//--------------------------------------------------------------------------------------------------

inline	FwVector	FwVector::operator /= ( FwVector_arg rhs )
{
	_VEC_VALIDATE_QUADWORD_XYZ( m_value );
	_VEC_VALIDATE_QUADWORD_XYZ( rhs.m_value );
	v128 divResult; _VEC_DIV( divResult, m_value, rhs.m_value );
	_VEC_VALIDATE_QUADWORD_XYZ( divResult );
	m_value = divResult;
	return *this;
}

//--------------------------------------------------------------------------------------------------
/**
	@brief			Sets current object to 'this / rhs scalar'
**/
//--------------------------------------------------------------------------------------------------

inline	FwVector	FwVector::operator /= ( FwScalar_arg rhs )
{
	_VEC_VALIDATE_QUADWORD_XYZ( m_value );
	v128 divResult; _VEC_DIV( divResult, m_value, rhs.QuadwordValue() );
	_VEC_VALIDATE_QUADWORD_XYZ( divResult );
	m_value = divResult;
	return *this;
}

//--------------------------------------------------------------------------------------------------
/**
	@brief			Returns '-this'
**/
//--------------------------------------------------------------------------------------------------

inline	FwVector	FwVector::operator - () const
{
	_VEC_VALIDATE_QUADWORD_XYZ( m_value );
	return FwVector( _VEC_NEG( m_value ) );
}

//--------------------------------------------------------------------------------------------------
/**
	@brief			Retrieves the X component as a scalar obect
**/
//--------------------------------------------------------------------------------------------------

inline	FwScalar	FwVector::X( void ) const
{
	_VEC_VALIDATE_QUADWORD_XYZ( m_value );
	return FwScalar( _VEC_REPLICATE_X( m_value ) );
}

//--------------------------------------------------------------------------------------------------
/**
	@brief			Retrieves the Y component as a scalar obect
**/
//--------------------------------------------------------------------------------------------------

inline	FwScalar	FwVector::Y( void ) const
{
	_VEC_VALIDATE_QUADWORD_XYZ( m_value );
	return FwScalar( _VEC_REPLICATE_Y( m_value ) );
}

//--------------------------------------------------------------------------------------------------
/**
	@brief			Retrieves the Z component as a scalar obect
**/
//--------------------------------------------------------------------------------------------------

inline	FwScalar	FwVector::Z( void ) const
{
	_VEC_VALIDATE_QUADWORD_XYZ( m_value );
	return FwScalar( _VEC_REPLICATE_Z( m_value ) );
}

//--------------------------------------------------------------------------------------------------
/**
	@brief			Set the X component of a vector using a scalar value
**/
//--------------------------------------------------------------------------------------------------

inline	void		FwVector::SetX( FwScalar_arg x )
{
	_VEC_VALIDATE_QUADWORD_XYZ( m_value );
	_VEC_SET_X( m_value, x.QuadwordValue() );
}

//--------------------------------------------------------------------------------------------------
/**
	@brief			Set the Y component of a vector using a scalar value
**/
//--------------------------------------------------------------------------------------------------

inline	void		FwVector::SetY( FwScalar_arg y )
{
	_VEC_VALIDATE_QUADWORD_XYZ( m_value );
	_VEC_SET_Y( m_value, y.QuadwordValue() );
}

//--------------------------------------------------------------------------------------------------
/**
	@brief			Set the Z component of a vector using a scalar value
**/
//--------------------------------------------------------------------------------------------------

inline	void		FwVector::SetZ( FwScalar_arg z )
{
	_VEC_VALIDATE_QUADWORD_XYZ( m_value );
	_VEC_SET_Z( m_value, z.QuadwordValue() );
}

//--------------------------------------------------------------------------------------------------
/**
	@brief			Returns the value of the element at the specified index (0=X, 1=Y, 2=Z)

	@param			index				Index of the element (Valid values are 0, 1, 2)

	@result			A scalar containing the element.
**/
//--------------------------------------------------------------------------------------------------

inline	FwScalar	FwVector::Get( int index ) const
{	
	assert( ( index >= 0 ) && ( index <= 2 ) );
	_VEC_VALIDATE_QUADWORD_XYZ( m_value );
	_VEC_RETURN_ELEMENT( m_value, index );
}

//--------------------------------------------------------------------------------------------------
/**
	@brief			Returns the value of the element at the specified index (0=X, 1=Y, 2=Z)

	@param			index				Index of the element (Valid values are 0, 1, 2)
					val					Scalar value to insert at the specified element
**/
//--------------------------------------------------------------------------------------------------
			
inline	void		FwVector::Set( int index, FwScalar_arg val )
{
	assert( ( index >= 0 ) && ( index <= 2 ) );
	_VEC_VALIDATE_QUADWORD_XYZ( m_value );
	_VEC_SET_ELEMENT( m_value, index, val.QuadwordValue() );
}

//--------------------------------------------------------------------------------------------------
/**
	@brief			Returns the value of the element at the specified index (0=X, 1=Y, 2=Z)

	@param			index				Index of the element (Valid values are 0, 1, 2)

	@result			A const scalar containing the element, the constness of which allows us to 
					error at compile time if someone attempts to do use operator [] to write a value.
**/
//--------------------------------------------------------------------------------------------------

inline	const FwScalar	FwVector::operator []( int index ) const
{
	assert( ( index >= 0 ) && ( index <= 2 ) );
	_VEC_VALIDATE_QUADWORD_XYZ( m_value );
	_VEC_RETURN_ELEMENT( m_value, index );
}

//--------------------------------------------------------------------------------------------------
/**
	@brief			Returns the quadword in the current object by value
**/
//--------------------------------------------------------------------------------------------------

inline	v128	FwVector::QuadwordValue( void ) const
{
	_VEC_VALIDATE_QUADWORD_XYZ( m_value );
	return 	m_value;
}

//--------------------------------------------------------------------------------------------------
/**
	@brief			Returns a 4 component vector. The W component is initialised to zero. 
**/
//--------------------------------------------------------------------------------------------------

inline	FwVector4	FwVector::GetVector4( void ) const
{
	v128 result = m_value;
	result = _VEC_AND( result, _VEC_GET_MASKOFF_W() );
	return FwVector4( result );
}

// -------------------------------------------------------------------------------------------------
// Non-member functions

//--------------------------------------------------------------------------------------------------
/**
	@brief			Determines the largest value held in the input object
**/
//--------------------------------------------------------------------------------------------------

inline	FwScalar	MaxComp( FwVector_arg vec )
{
	_VEC_VALIDATE_QUADWORD_XYZ( vec.m_value );
	v128	tempVector;
	tempVector = _VEC_MAX( _VEC_REPLICATE_X( vec.m_value ), _VEC_REPLICATE_Y( vec.m_value ) );
	tempVector = _VEC_MAX( tempVector, _VEC_REPLICATE_Z( vec.m_value ) );
	return FwScalar( tempVector );
}

//--------------------------------------------------------------------------------------------------
/**
	@brief			Determines the smallest value held in the input object
**/
//--------------------------------------------------------------------------------------------------

inline	FwScalar	MinComp( FwVector_arg vec )
{
	_VEC_VALIDATE_QUADWORD_XYZ( vec.m_value );
	v128	tempVector;
	tempVector = _VEC_MIN( _VEC_REPLICATE_X( vec.m_value ), _VEC_REPLICATE_Y( vec.m_value ) );
	tempVector = _VEC_MIN( tempVector, _VEC_REPLICATE_Z( vec.m_value ) );
	return FwScalar( tempVector );
}

//--------------------------------------------------------------------------------------------------
/**
	@brief			Compute the sum of X, Y, and Z elements within input object
**/
//--------------------------------------------------------------------------------------------------

inline	FwScalar	Sum( FwVector_arg vec )
{
	_VEC_VALIDATE_QUADWORD_XYZ( vec.m_value );
	v128 tempVector = _VEC_ADD( _VEC_REPLICATE_X( vec.m_value ), _VEC_REPLICATE_Y( vec.m_value ) );
	return FwScalar( _VEC_ADD( tempVector, _VEC_REPLICATE_Z( vec.m_value ) ) );
}

//--------------------------------------------------------------------------------------------------
/**
	@brief			Compute the squared length of the input object
**/	
//--------------------------------------------------------------------------------------------------

inline	FwScalar	LengthSqr( FwVector_arg vec )
{
	_VEC_VALIDATE_QUADWORD_XYZ( vec.m_value );
	v128 lenSqrResult; _VEC_DOT3( lenSqrResult, vec.m_value, vec.m_value );
	return FwScalar( _VEC_REPLICATE_X( lenSqrResult ) );
}

//--------------------------------------------------------------------------------------------------
/**
	@brief			Compute the reciprocal of the length of the input object
**/
//--------------------------------------------------------------------------------------------------

inline	FwScalar	LengthRcp( FwVector_arg vec )
{
	_VEC_VALIDATE_QUADWORD_XYZ( vec.m_value );
	v128 lenSqrResult; _VEC_DOT3( lenSqrResult, vec.m_value, vec.m_value );
	v128 lenRcpResult; _VEC_RSQRT( lenRcpResult, _VEC_REPLICATE_X( lenSqrResult ) );
	return FwScalar( lenRcpResult );	
}

//--------------------------------------------------------------------------------------------------
/**
	@brief			Compute the length of the input object
**/
//--------------------------------------------------------------------------------------------------

inline	FwScalar	Length( FwVector_arg vec )
{
	_VEC_VALIDATE_QUADWORD_XYZ( vec.m_value );
	v128 lenSqrResult; _VEC_DOT3( lenSqrResult, vec.m_value, vec.m_value );
	v128 lenSqrtResult; _VEC_SQRT( lenSqrtResult, _VEC_REPLICATE_X( lenSqrResult ) );
	return FwScalar( lenSqrtResult );	
}

//--------------------------------------------------------------------------------------------------
/**
	@brief			Returns an object containing absolute values of each element in input object
**/
//--------------------------------------------------------------------------------------------------

inline	FwVector	Abs( FwVector_arg vec )
{
	_VEC_VALIDATE_QUADWORD_XYZ( vec.m_value );
	return FwVector( _VEC_ABS( vec.m_value ) );
}

//--------------------------------------------------------------------------------------------------
/**
	@brief			Returns an object containing the sign of each element in input object (-1,1)
**/
//--------------------------------------------------------------------------------------------------

inline	FwVector	Sign( FwVector_arg vec )
{
	_VEC_VALIDATE_QUADWORD_XYZ( vec.m_value );
	v128 signResult; _VEC_SIGN( signResult, vec.m_value );
	return FwVector( signResult );
}

//--------------------------------------------------------------------------------------------------
/**
	@brief			Returns an object containing the signum of each element in input object (-1,0,1)
**/
//--------------------------------------------------------------------------------------------------

inline	FwVector	Signum( FwVector_arg vec )
{
	_VEC_VALIDATE_QUADWORD_XYZ( vec.m_value );
	v128 signumResult; _VEC_SIGNUM( signumResult, vec.m_value );
	return FwVector( signumResult );
}

//--------------------------------------------------------------------------------------------------
/**
	@brief			Returns an object containing reciprocal of each element in input object
**/
//--------------------------------------------------------------------------------------------------

inline	FwVector	Recip( FwVector_arg vec )
{
	_VEC_VALIDATE_QUADWORD_XYZ( vec.m_value );
	v128 recipResult; _VEC_RCP( recipResult, vec.m_value );
	return FwVector( recipResult );
}

//--------------------------------------------------------------------------------------------------
/**
	@brief			Returns the square root of each element in input object
**/
//--------------------------------------------------------------------------------------------------

inline	FwVector	Sqrt( FwVector_arg vec )
{
	_VEC_VALIDATE_QUADWORD_XYZ( vec.m_value );
	v128 sqrtResult; _VEC_SQRT( sqrtResult, vec.m_value );
	return FwVector( sqrtResult );
}

//--------------------------------------------------------------------------------------------------
/**
	@brief			Returns the reciprocal square root of each element in input object
**/
//--------------------------------------------------------------------------------------------------

inline	FwVector	RecipSqrt( FwVector_arg vec )
{
	_VEC_VALIDATE_QUADWORD_XYZ( vec.m_value );
	v128 rsqrtResult; _VEC_RSQRT( rsqrtResult, vec.m_value );
	return FwVector( rsqrtResult );
}

//--------------------------------------------------------------------------------------------------
/**
	@brief			Returns an object containing element-wise maximum values of two input objects
**/
//--------------------------------------------------------------------------------------------------

inline	FwVector	Max( FwVector_arg lhs, FwVector_arg rhs )
{
	_VEC_VALIDATE_QUADWORD_XYZ( lhs.m_value );
	_VEC_VALIDATE_QUADWORD_XYZ( rhs.m_value );
	return FwVector( _VEC_MAX( lhs.m_value, rhs.m_value ) );
}

//--------------------------------------------------------------------------------------------------
/**
	@brief			Returns an object containing element-wise minimum values of two input objects
**/
//--------------------------------------------------------------------------------------------------

inline	FwVector	Min( FwVector_arg lhs, FwVector_arg rhs )
{
	_VEC_VALIDATE_QUADWORD_XYZ( lhs.m_value );
	_VEC_VALIDATE_QUADWORD_XYZ( rhs.m_value );
	return FwVector( _VEC_MIN( lhs.m_value, rhs.m_value ) );
}

//--------------------------------------------------------------------------------------------------
/**
	@brief			Returns an object containing clamped values based on minimum and maximum limits
**/
//--------------------------------------------------------------------------------------------------

inline	FwVector	Clamp( FwVector_arg vec, FwVector_arg mn, FwVector_arg mx )
{
	_VEC_VALIDATE_QUADWORD_XYZ( vec.m_value );
	_VEC_VALIDATE_QUADWORD_XYZ( mn.m_value );
	_VEC_VALIDATE_QUADWORD_XYZ( mx.m_value );
	return FwVector( _VEC_CLAMP( vec.m_value, mn.m_value, mx.m_value ) );
}

//--------------------------------------------------------------------------------------------------
/**
	@brief			Returns the 3-element dot product of input objects
**/
//--------------------------------------------------------------------------------------------------

inline	FwScalar	Dot( FwVector_arg lhs, FwVector_arg rhs )
{
	_VEC_VALIDATE_QUADWORD_XYZ( lhs.m_value );
	_VEC_VALIDATE_QUADWORD_XYZ( rhs.m_value );
	v128 dot3Result; _VEC_DOT3( dot3Result, lhs.m_value, rhs.m_value );
	return FwScalar( _VEC_REPLICATE_X( dot3Result ) );
}

//--------------------------------------------------------------------------------------------------
/**
	@brief			Returns the cross product of the input object
**/
//--------------------------------------------------------------------------------------------------

inline	FwVector	Cross( FwVector_arg lhs, FwVector_arg rhs )
{
	_VEC_VALIDATE_QUADWORD_XYZ( lhs.m_value );
	_VEC_VALIDATE_QUADWORD_XYZ( rhs.m_value );
	v128 crossResult; _VEC_CROSS( crossResult, lhs.m_value, rhs.m_value );
	return FwVector( crossResult );
}

//--------------------------------------------------------------------------------------------------
/**
	@brief			Returns an object holding normalised version of input object (3 component)
**/
//--------------------------------------------------------------------------------------------------

inline	FwVector	Normalize( FwVector_arg vec )
{
	_VEC_VALIDATE_QUADWORD_XYZ( vec.m_value );
	v128	result; _VEC_NORMALIZE3( result, vec.m_value );
	return FwVector( result );
}

//--------------------------------------------------------------------------------------------------
/**
	@brief			Returns an object holding normalised version of 3 component input object, or
					specified fallback value if 'vec' could not be successfully normalised.
**/
//--------------------------------------------------------------------------------------------------

inline	FwVector	SafeNormalize( FwVector_arg vec, FwVector_arg fallback )
{
	_VEC_VALIDATE_QUADWORD_XYZ( vec.m_value );
	_VEC_VALIDATE_QUADWORD_XYZ( fallback.m_value );
	v128	result; _VEC_SAFE_NORMALIZE3( result, vec.m_value, fallback.m_value );
	return FwVector( result );
}

//--------------------------------------------------------------------------------------------------
/**
	@brief			Returns an object formed by linear interpolation between two input objects
	@param			lhs				Start object
	@param			rhs				End object
	@param			alpha			Interpolation factor. 
	@note			When alpha is outside of range 0.0f to 1.0f, extrapolation occurs
**/
//--------------------------------------------------------------------------------------------------

inline	FwVector	Lerp( FwVector_arg lhs, FwVector_arg rhs, FwScalar_arg alpha )
{
	_VEC_VALIDATE_QUADWORD_XYZ( lhs.m_value );
	_VEC_VALIDATE_QUADWORD_XYZ( rhs.m_value );
	_VEC_VALIDATE_QUADWORD_XYZ( alpha.QuadwordValue() );
	return FwVector( _VEC_MADD( _VEC_SUB( rhs.m_value, lhs.m_value ), alpha.QuadwordValue(), lhs.m_value ) );
}

//--------------------------------------------------------------------------------------------------
/**
	@brief			Returns whether all components in lhs are < equivalent components in rhs
*/
//--------------------------------------------------------------------------------------------------

inline	bool		AllComponentsLessThan( FwVector_arg lhs, FwVector_arg rhs )
{
	_VEC_VALIDATE_QUADWORD_XYZ( lhs.m_value );
	_VEC_VALIDATE_QUADWORD_XYZ( rhs.m_value );
	v128	lhsTemp = lhs.m_value;
	v128	rhsTemp = rhs.m_value;
	_VEC_SET_W( lhsTemp, _VEC_REPLICATE_Z( lhsTemp ) );
	_VEC_SET_W( rhsTemp, _VEC_REPLICATE_Z( rhsTemp ) );
	return _VEC_ALL_COMPONENTS_LT( lhsTemp, rhsTemp );}

//--------------------------------------------------------------------------------------------------
/**
	@brief			Returns whether all components in lhs are <= equivalent components in rhs
*/
//--------------------------------------------------------------------------------------------------

inline	bool		AllComponentsLessThanOrEqual( FwVector_arg lhs, FwVector_arg rhs )
{
	_VEC_VALIDATE_QUADWORD_XYZ( lhs.m_value );
	_VEC_VALIDATE_QUADWORD_XYZ( rhs.m_value );
	v128	lhsTemp = lhs.m_value;
	v128	rhsTemp = rhs.m_value;
	_VEC_SET_W( lhsTemp, _VEC_REPLICATE_Z( lhsTemp ) );
	_VEC_SET_W( rhsTemp, _VEC_REPLICATE_Z( rhsTemp ) );
	return _VEC_ALL_COMPONENTS_LE( lhsTemp, rhsTemp );
}

//--------------------------------------------------------------------------------------------------
/**
	@brief			Returns whether all components in lhs are > equivalent components in rhs
*/
//--------------------------------------------------------------------------------------------------

inline	bool		AllComponentsGreaterThan( FwVector_arg lhs, FwVector_arg rhs )
{
	_VEC_VALIDATE_QUADWORD_XYZ( lhs.m_value );
	_VEC_VALIDATE_QUADWORD_XYZ( rhs.m_value );
	v128	lhsTemp = lhs.m_value;
	v128	rhsTemp = rhs.m_value;
	_VEC_SET_W( lhsTemp, _VEC_REPLICATE_Z( lhsTemp ) );
	_VEC_SET_W( rhsTemp, _VEC_REPLICATE_Z( rhsTemp ) );
	return _VEC_ALL_COMPONENTS_GT( lhsTemp, rhsTemp );
}

//--------------------------------------------------------------------------------------------------
/**
	@brief			Returns whether all components in lhs are >=  equivalent components in rhs
*/
//--------------------------------------------------------------------------------------------------

inline	bool		AllComponentsGreaterThanOrEqual( FwVector_arg lhs, FwVector_arg rhs )
{
	_VEC_VALIDATE_QUADWORD_XYZ( lhs.m_value );
	_VEC_VALIDATE_QUADWORD_XYZ( rhs.m_value );
	v128	lhsTemp = lhs.m_value;
	v128	rhsTemp = rhs.m_value;
	_VEC_SET_W( lhsTemp, _VEC_REPLICATE_Z( lhsTemp ) );
	_VEC_SET_W( rhsTemp, _VEC_REPLICATE_Z( rhsTemp ) );
	return _VEC_ALL_COMPONENTS_GE( lhsTemp, rhsTemp );
}

//--------------------------------------------------------------------------------------------------
/**
	@brief			Returns whether all components in lhs are ==  equivalent components in rhs
*/
//--------------------------------------------------------------------------------------------------

inline	bool		AllComponentsEqual( FwVector_arg lhs, FwVector_arg rhs )
{
	_VEC_VALIDATE_QUADWORD_XYZ( lhs.m_value );
	_VEC_VALIDATE_QUADWORD_XYZ( rhs.m_value );
	v128	lhsTemp = lhs.m_value;
	v128	rhsTemp = rhs.m_value;
	_VEC_SET_W( lhsTemp, _VEC_REPLICATE_Z( lhsTemp ) );
	_VEC_SET_W( rhsTemp, _VEC_REPLICATE_Z( rhsTemp ) );
	return _VEC_ALL_COMPONENTS_EQ( lhsTemp, rhsTemp );
}

//--------------------------------------------------------------------------------------------------
/**
	@brief			Same as 'FwVector * FwScalar', but keeps scalar behaviour when scalar is on lhs
**/
//--------------------------------------------------------------------------------------------------

inline	FwVector	operator * ( FwScalar_arg scalar, FwVector_arg vec )
{
	_VEC_VALIDATE_QUADWORD_XYZ( vec.m_value );
	return FwVector( _VEC_MUL( scalar.QuadwordValue(), vec.m_value ) );
}

//--------------------------------------------------------------------------------------------------
/**
	@brief			Same as 'FwVector + FwScalar', but keeps scalar behaviour when scalar is on lhs
**/
//--------------------------------------------------------------------------------------------------

inline	FwVector	operator + ( FwScalar_arg scalar, FwVector_arg vec )
{
	_VEC_VALIDATE_QUADWORD_XYZ( vec.m_value );
	return FwVector( _VEC_ADD( scalar.QuadwordValue(), vec.m_value ) );
}

#endif	// FW_VECTOR_INL
