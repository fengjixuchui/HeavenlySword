//--------------------------------------------------------------------------------------------------
/**
	@file		
	
	@brief		SIMD Point

	@note		(c) Copyright Sony Computer Entertainment 2005. All Rights Reserved.	
**/
//--------------------------------------------------------------------------------------------------

#ifndef FW_POINT_INL
#define FW_POINT_INL

//--------------------------------------------------------------------------------------------------
/**
	@brief			Default constructor
**/
//--------------------------------------------------------------------------------------------------

inline	FwPoint::FwPoint( )
{
	_VEC_INITIALISE_QUADWORD( m_value );
}

//--------------------------------------------------------------------------------------------------
/**
	@brief			Constructs from 3 floats
**/
//--------------------------------------------------------------------------------------------------

inline	FwPoint::FwPoint( float x, float y, float z )
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
inline	FwPoint::FwPoint( const FwPoint& rhs )
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

inline	FwPoint::FwPoint( const float* pData )
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

inline	FwPoint::FwPoint( v128 rhs )
{
	_VEC_VALIDATE_QUADWORD_XYZ( rhs );
	m_value = rhs;
}

//--------------------------------------------------------------------------------------------------
/**
	@brief			Constructs from a scalar value
**/
//--------------------------------------------------------------------------------------------------

inline	FwPoint::FwPoint( FwScalar_arg rhs )
{
	m_value = rhs.QuadwordValue();
}

//--------------------------------------------------------------------------------------------------
/**
	@brief			Constructs from a 4 element vector (W is effectively discarded)
**/
//--------------------------------------------------------------------------------------------------

inline	FwPoint::FwPoint( FwVector4_arg rhs )
{
	m_value = rhs.QuadwordValue();
}

//--------------------------------------------------------------------------------------------------
/**
	@brief			Constructs to object containing zero
**/
//--------------------------------------------------------------------------------------------------

inline	FwPoint::FwPoint( const FwMaths::ZeroTag& )
{
	m_value = _VEC_SET_ZERO();
}

//--------------------------------------------------------------------------------------------------
/**
	@brief			Assigns current object to FwMaths::kZero  -  (0, 0, 0, 0)
**/
//--------------------------------------------------------------------------------------------------

inline	FwPoint		FwPoint::operator = ( const FwMaths::ZeroTag& )
{
	m_value = _VEC_SET_ZERO();
	return *this;
}

//--------------------------------------------------------------------------------------------------
/**
	@brief			'this = rhs'
**/
//--------------------------------------------------------------------------------------------------

inline	FwPoint		FwPoint::operator = ( FwPoint_arg v )
{
	_VEC_VALIDATE_QUADWORD_XYZ( v.m_value );
	m_value = v.m_value;
	return *this;
}

//--------------------------------------------------------------------------------------------------
/**
	@brief			Return 'this + rhs vector'
**/
//--------------------------------------------------------------------------------------------------

inline	FwPoint		FwPoint::operator + ( FwVector_arg rhs ) const
{
	_VEC_VALIDATE_QUADWORD_XYZ( m_value );
	return FwPoint( _VEC_ADD( m_value, rhs.QuadwordValue() ) );
}

//--------------------------------------------------------------------------------------------------
/**
	@brief			Return 'this - rhs'
**/
//--------------------------------------------------------------------------------------------------

inline	FwVector	FwPoint::operator - ( FwPoint_arg rhs ) const
{
	_VEC_VALIDATE_QUADWORD_XYZ( m_value );
	_VEC_VALIDATE_QUADWORD_XYZ( rhs.m_value );
	return FwVector( _VEC_SUB( m_value, rhs.m_value ) );
}

//--------------------------------------------------------------------------------------------------
/**
	@brief			Return 'this - rhs vector'
**/
//--------------------------------------------------------------------------------------------------

inline	FwPoint		FwPoint::operator - ( FwVector_arg rhs ) const
{
	_VEC_VALIDATE_QUADWORD_XYZ( m_value );
	return FwPoint( _VEC_SUB( m_value, rhs.QuadwordValue() ) );
}

//--------------------------------------------------------------------------------------------------
/**
	@brief			Sets current object to 'this + rhs vector'
**/
//--------------------------------------------------------------------------------------------------

inline	FwPoint		FwPoint::operator += ( FwVector_arg rhs )
{
	_VEC_VALIDATE_QUADWORD_XYZ( m_value );
	m_value = _VEC_ADD( m_value, rhs.QuadwordValue() );
	return *this;
}

//--------------------------------------------------------------------------------------------------
/**
	@brief			Sets current object to 'this - rhs vector'
**/
//--------------------------------------------------------------------------------------------------

inline	FwPoint		FwPoint::operator -= ( FwVector_arg rhs )
{
	_VEC_VALIDATE_QUADWORD_XYZ( m_value );
	m_value = _VEC_SUB( m_value, rhs.QuadwordValue() );
	return *this;
}

//--------------------------------------------------------------------------------------------------
/**
	@brief			Returns '-this'
**/
//--------------------------------------------------------------------------------------------------

inline	FwPoint		FwPoint::operator - () const
{
	_VEC_VALIDATE_QUADWORD_XYZ( m_value );
	return FwPoint( _VEC_NEG( m_value ) );
}

//--------------------------------------------------------------------------------------------------
/**
	@brief			Retrieves the X component as a scalar obect
**/
//--------------------------------------------------------------------------------------------------

inline	FwScalar	FwPoint::X( void ) const
{
	_VEC_VALIDATE_QUADWORD_XYZ( m_value );
	return FwScalar( _VEC_REPLICATE_X( m_value ) );
}

//--------------------------------------------------------------------------------------------------
/**
	@brief			Retrieves the Y component as a scalar obect
**/
//--------------------------------------------------------------------------------------------------

inline	FwScalar	FwPoint::Y( void ) const
{
	_VEC_VALIDATE_QUADWORD_XYZ( m_value );
	return FwScalar( _VEC_REPLICATE_Y( m_value ) );
}

//--------------------------------------------------------------------------------------------------
/**
	@brief			Retrieves the Z component as a scalar obect
**/
//--------------------------------------------------------------------------------------------------

inline	FwScalar	FwPoint::Z( void ) const
{
	_VEC_VALIDATE_QUADWORD_XYZ( m_value );
	return FwScalar( _VEC_REPLICATE_Z( m_value ) );
}

//--------------------------------------------------------------------------------------------------
/**
	@brief			Set the X component of a vector using a scalar value
**/
//--------------------------------------------------------------------------------------------------

inline	void		FwPoint::SetX( FwScalar_arg x )
{
	_VEC_VALIDATE_QUADWORD_XYZ( m_value );
	_VEC_SET_X( m_value, x.QuadwordValue() );
}

//--------------------------------------------------------------------------------------------------
/**
	@brief			Set the Y component of a vector using a scalar value
**/
//--------------------------------------------------------------------------------------------------

inline	void		FwPoint::SetY( FwScalar_arg y )
{
	_VEC_VALIDATE_QUADWORD_XYZ( m_value );
	_VEC_SET_Y( m_value, y.QuadwordValue() );
}

//--------------------------------------------------------------------------------------------------
/**
	@brief			Set the Z component of a vector using a scalar value
**/
//--------------------------------------------------------------------------------------------------

inline	void		FwPoint::SetZ( FwScalar_arg z )
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

inline	FwScalar	FwPoint::Get( int index ) const
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
			
inline	void		FwPoint::Set( int index, FwScalar_arg val )
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

inline	const FwScalar	FwPoint::operator []( int index ) const
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

inline	v128	FwPoint::QuadwordValue( void ) const
{
	_VEC_VALIDATE_QUADWORD_XYZ( m_value );
	return 	m_value;
}

//--------------------------------------------------------------------------------------------------
/**
	@brief			Returns a 4 component vector. The W component is initialised to 1.0f. 
**/
//--------------------------------------------------------------------------------------------------

inline	FwVector4	FwPoint::GetVector4( void ) const
{
	float	one = 1.0f;
	v128	result = m_value;
	_VEC_SET_W( result, _VEC_REPLICATE_FLOAT( one ) );
	return FwVector4( result );
}

// -------------------------------------------------------------------------------------------------
// Non-member functions

//--------------------------------------------------------------------------------------------------
/**
	@brief			Determines the largest value held in the input object
**/
//--------------------------------------------------------------------------------------------------

inline	FwScalar	MaxComp( FwPoint_arg vec )
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

inline	FwScalar	MinComp( FwPoint_arg vec )
{
	_VEC_VALIDATE_QUADWORD_XYZ( vec.m_value );
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

inline	FwScalar	Sum( FwPoint_arg vec )
{
	_VEC_VALIDATE_QUADWORD_XYZ( vec.m_value );
	v128 tempVector = _VEC_ADD( _VEC_REPLICATE_X( vec.m_value ), _VEC_REPLICATE_Y( vec.m_value ) );
	return FwScalar( _VEC_ADD( tempVector, _VEC_REPLICATE_Z( vec.m_value ) ) );
}

//--------------------------------------------------------------------------------------------------
/**
	@brief			Returns an object containing absolute values of each element in input object
**/
//--------------------------------------------------------------------------------------------------

inline	FwPoint		Abs( FwPoint_arg vec )
{
	_VEC_VALIDATE_QUADWORD_XYZ( vec.m_value );
	return FwPoint( _VEC_ABS( vec.m_value ) );
}

//--------------------------------------------------------------------------------------------------
/**
	@brief			Returns an object containing the sign of each element in input object (-1,1)
**/
//--------------------------------------------------------------------------------------------------

inline	FwPoint		Sign( FwPoint_arg vec )
{
	_VEC_VALIDATE_QUADWORD_XYZ( vec.m_value );
	v128 signResult; _VEC_SIGN( signResult, vec.m_value );
	return FwPoint( signResult );
}

//--------------------------------------------------------------------------------------------------
/**
	@brief			Returns an object containing the signum of each element in input object (-1,0,1)
**/
//--------------------------------------------------------------------------------------------------

inline	FwPoint		Signum( FwPoint_arg vec )
{
	_VEC_VALIDATE_QUADWORD_XYZ( vec.m_value );
	v128 signumResult; _VEC_SIGNUM( signumResult, vec.m_value );
	return FwPoint( signumResult );
}

//--------------------------------------------------------------------------------------------------
/**
	@brief			Returns an object containing reciprocal of each element in input object
**/
//--------------------------------------------------------------------------------------------------

inline	FwPoint		Recip( FwPoint_arg vec )
{
	_VEC_VALIDATE_QUADWORD_XYZ( vec.m_value );
	v128 recipResult; _VEC_RCP( recipResult, vec.m_value );
	return FwPoint( recipResult );
}

//--------------------------------------------------------------------------------------------------
/**
	@brief			Returns the square root of each element in input object
**/
//--------------------------------------------------------------------------------------------------

inline	FwPoint		Sqrt( FwPoint_arg vec )
{
	_VEC_VALIDATE_QUADWORD_XYZ( vec.m_value );
	v128 sqrtResult; _VEC_SQRT( sqrtResult, vec.m_value );
	return FwPoint( sqrtResult );
}

//--------------------------------------------------------------------------------------------------
/**
	@brief			Returns the reciprocal square root of each element in input object
**/
//--------------------------------------------------------------------------------------------------

inline	FwPoint		RecipSqrt( FwPoint_arg vec )
{
	_VEC_VALIDATE_QUADWORD_XYZ( vec.m_value );
	v128 rsqrtResult; _VEC_RSQRT( rsqrtResult, vec.m_value );
	return FwPoint( rsqrtResult );
}

//--------------------------------------------------------------------------------------------------
/**
	@brief			Returns an object containing element-wise maximum values of two input objects
**/
//--------------------------------------------------------------------------------------------------

inline	FwPoint		Max( FwPoint_arg lhs, FwPoint_arg rhs )
{
	_VEC_VALIDATE_QUADWORD_XYZ( lhs.m_value );
	_VEC_VALIDATE_QUADWORD_XYZ( rhs.m_value );
	return FwPoint( _VEC_MAX( lhs.m_value, rhs.m_value ) );
}

//--------------------------------------------------------------------------------------------------
/**
	@brief			Returns an object containing element-wise minimum values of two input objects
**/
//--------------------------------------------------------------------------------------------------

inline	FwPoint		Min( FwPoint_arg lhs, FwPoint_arg rhs )
{
	_VEC_VALIDATE_QUADWORD_XYZ( lhs.m_value );
	_VEC_VALIDATE_QUADWORD_XYZ( rhs.m_value );
	return FwPoint( _VEC_MIN( lhs.m_value, rhs.m_value ) );
}

//--------------------------------------------------------------------------------------------------
/**
	@brief			Returns an object containing clamped values based on minimum and maximum limits
**/
//--------------------------------------------------------------------------------------------------

inline	FwPoint		Clamp( FwPoint_arg vec, FwPoint_arg mn, FwPoint_arg mx )
{
	_VEC_VALIDATE_QUADWORD_XYZ( vec.m_value );
	_VEC_VALIDATE_QUADWORD_XYZ( mn.m_value );
	_VEC_VALIDATE_QUADWORD_XYZ( mx.m_value );
	return FwPoint( _VEC_CLAMP( vec.m_value, mn.m_value, mx.m_value ) );
}

//--------------------------------------------------------------------------------------------------
/**
	@brief			Returns the 3-element dot product of input objects
**/
//--------------------------------------------------------------------------------------------------

inline	FwScalar	Dot( FwPoint_arg lhs, FwPoint_arg rhs )
{
	_VEC_VALIDATE_QUADWORD_XYZ( lhs.m_value );
	_VEC_VALIDATE_QUADWORD_XYZ( rhs.m_value );
	v128 dot3Result; _VEC_DOT3( dot3Result, lhs.m_value, rhs.m_value );
	return FwScalar( _VEC_REPLICATE_X( dot3Result ) );
}

//--------------------------------------------------------------------------------------------------
/**
	@brief			Compute the squared distance between the input object
**/
//--------------------------------------------------------------------------------------------------

inline	FwScalar	DistSqr( FwPoint_arg lhs, FwPoint_arg rhs )
{
	_VEC_VALIDATE_QUADWORD_XYZ( lhs.m_value );
	_VEC_VALIDATE_QUADWORD_XYZ( rhs.m_value );
	v128 pointDiff = _VEC_SUB( rhs.m_value, lhs.m_value );
	v128 distSqrResult; _VEC_DOT3( distSqrResult, pointDiff, pointDiff );
	return FwScalar( _VEC_REPLICATE_X( distSqrResult ) );
}

//--------------------------------------------------------------------------------------------------
/**
	@brief			Compute the reciprocal of the distance between the input objects
**/
//--------------------------------------------------------------------------------------------------

inline	FwScalar	DistRcp( FwPoint_arg lhs, FwPoint_arg rhs )
{
	_VEC_VALIDATE_QUADWORD_XYZ( lhs.m_value );
	_VEC_VALIDATE_QUADWORD_XYZ( rhs.m_value );
	v128 pointDiff = _VEC_SUB( rhs.m_value, lhs.m_value );
	v128 distSqrResult; _VEC_DOT3( distSqrResult, pointDiff, pointDiff );
	v128 distRcpResult; _VEC_RSQRT( distRcpResult, _VEC_REPLICATE_X( distSqrResult ) );
	return FwScalar( distRcpResult );	
}

//--------------------------------------------------------------------------------------------------
/**
	@brief			Compute the distance between the input objects
**/
//--------------------------------------------------------------------------------------------------

inline	FwScalar	Dist( FwPoint_arg lhs, FwPoint_arg rhs )
{
	_VEC_VALIDATE_QUADWORD_XYZ( lhs.m_value );
	_VEC_VALIDATE_QUADWORD_XYZ( rhs.m_value );
	v128 pointDiff = _VEC_SUB( rhs.m_value, lhs.m_value );
	v128 distSqrResult; _VEC_DOT3( distSqrResult, pointDiff, pointDiff );
	v128 distResult; _VEC_SQRT( distResult, _VEC_REPLICATE_X( distSqrResult ) );
	return FwScalar( distResult );
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

inline	FwPoint		Lerp( FwPoint_arg lhs, FwPoint_arg rhs, FwScalar_arg alpha )
{
	_VEC_VALIDATE_QUADWORD_XYZ( lhs.m_value );
	_VEC_VALIDATE_QUADWORD_XYZ( rhs.m_value );
	_VEC_VALIDATE_QUADWORD_XYZ( alpha.QuadwordValue() );
	return FwPoint( _VEC_MADD( _VEC_SUB( rhs.m_value, lhs.m_value ), alpha.QuadwordValue(), lhs.m_value ) );
}

//--------------------------------------------------------------------------------------------------
/**
	@brief			Returns whether all components in lhs are < equivalent components in rhs
*/
//--------------------------------------------------------------------------------------------------

inline	bool		AllComponentsLessThan( FwPoint_arg lhs, FwPoint_arg rhs )
{
	_VEC_VALIDATE_QUADWORD_XYZ( lhs.m_value );
	_VEC_VALIDATE_QUADWORD_XYZ( rhs.m_value );
	v128	lhsTemp = lhs.m_value;
	v128	rhsTemp = rhs.m_value;
	_VEC_SET_W( lhsTemp, _VEC_REPLICATE_Z( lhsTemp ) );
	_VEC_SET_W( rhsTemp, _VEC_REPLICATE_Z( rhsTemp ) );
	return _VEC_ALL_COMPONENTS_LT( lhsTemp, rhsTemp );
}

//--------------------------------------------------------------------------------------------------
/**
	@brief			Returns whether all components in lhs are <= equivalent components in rhs
*/
//--------------------------------------------------------------------------------------------------

inline	bool		AllComponentsLessThanOrEqual( FwPoint_arg lhs, FwPoint_arg rhs )
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

inline	bool		AllComponentsGreaterThan( FwPoint_arg lhs, FwPoint_arg rhs )
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

inline	bool		AllComponentsGreaterThanOrEqual( FwPoint_arg lhs, FwPoint_arg rhs )
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

inline	bool		AllComponentsEqual( FwPoint_arg lhs, FwPoint_arg rhs )
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
	@brief			Returns a 'lhs vector + rhs point'.
**/
//--------------------------------------------------------------------------------------------------

inline	FwPoint		operator + ( FwVector_arg lhs, FwPoint_arg rhs )
{
	_VEC_VALIDATE_QUADWORD_XYZ( rhs.m_value );
	return FwPoint( _VEC_ADD( lhs.QuadwordValue(), rhs.m_value ) );
}

//--------------------------------------------------------------------------------------------------
/**
	@brief			Returns a 'lhs point - rhs kOrigin', which allows fast point->vector.
**/
//--------------------------------------------------------------------------------------------------

inline	FwVector	operator - ( FwPoint_arg lhs, const FwMaths::OriginTag& )
{
	return FwVector( lhs.QuadwordValue() );
}

//--------------------------------------------------------------------------------------------------
/**
	@brief			Returns a 'lhs kOrigin + rhs vector', which allows fast vector->point
**/
//--------------------------------------------------------------------------------------------------

inline	FwPoint		operator + ( const FwMaths::OriginTag&, FwVector_arg lhs )
{
	return FwPoint( lhs.QuadwordValue() );
}

#endif	// FW_POINT_INL
