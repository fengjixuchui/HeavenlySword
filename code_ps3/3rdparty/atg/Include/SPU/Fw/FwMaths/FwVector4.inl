//--------------------------------------------------------------------------------------------------
/**
	@file		
	
	@brief		SIMD Vector

	@note		(c) Copyright Sony Computer Entertainment 2005. All Rights Reserved.	
**/
//--------------------------------------------------------------------------------------------------

#ifndef FW_VECTOR4_INL
#define FW_VECTOR4_INL

//--------------------------------------------------------------------------------------------------
/**
	@brief			Default constructor
**/
//--------------------------------------------------------------------------------------------------

inline	FwVector4::FwVector4( )
{
	_VEC_INITIALISE_QUADWORD( m_value );
}

//--------------------------------------------------------------------------------------------------
/**
	@brief			Constructs from 4 floats
**/
//--------------------------------------------------------------------------------------------------

inline	FwVector4::FwVector4( float x, float y, float z, float w )
{
	_VEC_SET_FLOATS( m_value, x, y, z, w );
	_VEC_VALIDATE_QUADWORD( m_value );
}

//--------------------------------------------------------------------------------------------------
/**
	@brief			Copy constructor
**/
//--------------------------------------------------------------------------------------------------

#ifdef	_MSC_VER
inline	FwVector4::FwVector4( const FwVector4& rhs )
{
	_VEC_VALIDATE_QUADWORD( rhs.m_value );
	m_value = rhs.m_value;
}
#endif	//_MSC_VER

//--------------------------------------------------------------------------------------------------
/**
	@brief			Constructs from 4 floats located at the specified address
**/
//--------------------------------------------------------------------------------------------------

inline	FwVector4::FwVector4( const float* pData )
{
	assert( pData );
	_VEC_SET_FLOATS( m_value, pData[ 0 ], pData[ 1 ], pData[ 2 ], pData[ 3 ] );
	_VEC_VALIDATE_QUADWORD( m_value );
}

//--------------------------------------------------------------------------------------------------
/**
	@brief			Constructs from a raw quadword
**/
//--------------------------------------------------------------------------------------------------

inline	FwVector4::FwVector4( v128 rhs )
{
	_VEC_VALIDATE_QUADWORD( rhs );
	m_value = rhs;
}

//--------------------------------------------------------------------------------------------------
/**
	@brief			Constructs from a scalar value
**/
//--------------------------------------------------------------------------------------------------

inline	FwVector4::FwVector4( FwScalar_arg rhs )
{
	_VEC_VALIDATE_QUADWORD( rhs.m_value );
	m_value = rhs.m_value;
}

//--------------------------------------------------------------------------------------------------
/**
	@brief			Constructs to object containing zero
**/
//--------------------------------------------------------------------------------------------------

inline	FwVector4::FwVector4( const FwMaths::ZeroTag& )
{
	m_value = _VEC_SET_ZERO();
}

//--------------------------------------------------------------------------------------------------
/**
	@brief			Constructs to object containing the unit X axis (1, 0, 0, 0)
**/
//--------------------------------------------------------------------------------------------------

inline	FwVector4::FwVector4( const FwMaths::UnitXAxisTag& )
{
	m_value = _VEC_GET_UNIT_X();
}

//--------------------------------------------------------------------------------------------------
/**
	@brief			Constructs to object containing the unit Y axis (0, 1, 0, 0)
**/
//--------------------------------------------------------------------------------------------------

inline	FwVector4::FwVector4( const FwMaths::UnitYAxisTag& )
{
	m_value = _VEC_GET_UNIT_Y();
}

//--------------------------------------------------------------------------------------------------
/**
	@brief			Constructs to object containing the unit Z axis (0, 0, 1, 0)
**/
//--------------------------------------------------------------------------------------------------

inline	FwVector4::FwVector4( const FwMaths::UnitZAxisTag& )
{
	m_value = _VEC_GET_UNIT_Z();
}

//--------------------------------------------------------------------------------------------------
/**
	@brief			Constructs to object containing the unit W axis (0, 0, 0, 1)
**/
//--------------------------------------------------------------------------------------------------

inline	FwVector4::FwVector4( const FwMaths::UnitWAxisTag& )
{
	m_value = _VEC_GET_UNIT_W();
}

//--------------------------------------------------------------------------------------------------
/**
	@brief			Assigns current object to FwMaths::kZero  -  (0, 0, 0, 0)
**/
//--------------------------------------------------------------------------------------------------

inline	FwVector4	FwVector4::operator = ( const FwMaths::ZeroTag& )
{
	m_value = _VEC_SET_ZERO();
	return *this;
}

//--------------------------------------------------------------------------------------------------
/**
	@brief			Assigns current object to FwMaths::kUnitXAxis, which sets object to (1, 0, 0, 0).
**/
//--------------------------------------------------------------------------------------------------

inline	FwVector4	FwVector4::operator = ( const FwMaths::UnitXAxisTag& )
{
	m_value = _VEC_GET_UNIT_X();
	return *this;
}

//--------------------------------------------------------------------------------------------------
/**
	@brief			Assigns current object to FwMaths::kUnitYAxis, which sets object to (0, 1, 0, 0).
**/
//--------------------------------------------------------------------------------------------------

inline	FwVector4	FwVector4::operator = ( const FwMaths::UnitYAxisTag& )
{
	m_value = _VEC_GET_UNIT_Y();
	return *this;
}

//--------------------------------------------------------------------------------------------------
/**
	@brief			Assigns current object to FwMaths::kUnitZAxis, which sets object to (0, 0, 1, 0).
**/
//--------------------------------------------------------------------------------------------------

inline	FwVector4	FwVector4::operator = ( const FwMaths::UnitZAxisTag& )
{
	m_value = _VEC_GET_UNIT_Z();
	return *this;
}

//--------------------------------------------------------------------------------------------------
/**
	@brief			Assigns current object to FwMaths::kUnitWAxis, which sets object to (0, 0, 0, 1).
**/
//--------------------------------------------------------------------------------------------------

inline	FwVector4	FwVector4::operator = ( const FwMaths::UnitWAxisTag& )
{
	m_value = _VEC_GET_UNIT_W();
	return *this;
}

//--------------------------------------------------------------------------------------------------
/**
	@brief			'this = rhs'
**/
//--------------------------------------------------------------------------------------------------

inline	FwVector4	FwVector4::operator = ( FwVector4_arg v )
{
	_VEC_VALIDATE_QUADWORD( v.m_value );
	m_value = v.m_value;
	return *this;
}

//--------------------------------------------------------------------------------------------------
/**
	@brief			Return 'this + rhs'
**/
//--------------------------------------------------------------------------------------------------

inline	FwVector4	FwVector4::operator + ( FwVector4_arg rhs ) const
{
	_VEC_VALIDATE_QUADWORD( m_value );
	_VEC_VALIDATE_QUADWORD( rhs.m_value );
	return FwVector4( _VEC_ADD( m_value, rhs.m_value ) );
}

//--------------------------------------------------------------------------------------------------
/**
	@brief			Return 'this + rhs scalar'
**/
//--------------------------------------------------------------------------------------------------

inline	FwVector4	FwVector4::operator + ( FwScalar_arg rhs ) const
{
	_VEC_VALIDATE_QUADWORD( m_value );
	_VEC_VALIDATE_QUADWORD( rhs.m_value );
	return FwVector4( _VEC_ADD( m_value, rhs.m_value ) );
}

//--------------------------------------------------------------------------------------------------
/**
	@brief			Return 'this - rhs'
**/
//--------------------------------------------------------------------------------------------------

inline	FwVector4	FwVector4::operator - ( FwVector4_arg rhs ) const
{
	_VEC_VALIDATE_QUADWORD( m_value );
	_VEC_VALIDATE_QUADWORD( rhs.m_value );
	return FwVector4( _VEC_SUB( m_value, rhs.m_value ) );
}

//--------------------------------------------------------------------------------------------------
/**
	@brief			Return 'this - rhs scalar'
**/
//--------------------------------------------------------------------------------------------------

inline	FwVector4	FwVector4::operator - ( FwScalar_arg rhs ) const
{
	_VEC_VALIDATE_QUADWORD( m_value );
	_VEC_VALIDATE_QUADWORD( rhs.m_value );
	return FwVector4( _VEC_SUB( m_value, rhs.m_value ) );
}

//--------------------------------------------------------------------------------------------------
/**
	@brief			Return 'this * rhs'
**/
//--------------------------------------------------------------------------------------------------

inline	FwVector4	FwVector4::operator * ( FwVector4_arg rhs ) const
{
	_VEC_VALIDATE_QUADWORD( m_value );
	_VEC_VALIDATE_QUADWORD( rhs.m_value );
	return FwVector4( _VEC_MUL( m_value, rhs.m_value ) );
}

//--------------------------------------------------------------------------------------------------
/**
	@brief			Return 'this * rhs scalar'
**/
//--------------------------------------------------------------------------------------------------

inline	FwVector4	FwVector4::operator * ( FwScalar_arg rhs ) const
{
	_VEC_VALIDATE_QUADWORD( m_value );
	_VEC_VALIDATE_QUADWORD( rhs.m_value );
	return FwVector4( _VEC_MUL( m_value, rhs.m_value ) );
}

//--------------------------------------------------------------------------------------------------
/**
	@brief			Return 'this / rhs'
**/
//--------------------------------------------------------------------------------------------------

inline	FwVector4	FwVector4::operator / ( FwVector4_arg rhs ) const
{
	_VEC_VALIDATE_QUADWORD( m_value );
	_VEC_VALIDATE_QUADWORD( rhs.m_value );
	v128 divResult; _VEC_DIV( divResult, m_value, rhs.m_value );
	return FwVector4( divResult );
}

//--------------------------------------------------------------------------------------------------
/**
	@brief			Return 'this / rhs scalar'
**/
//--------------------------------------------------------------------------------------------------

inline	FwVector4	FwVector4::operator / ( FwScalar_arg rhs ) const
{
	_VEC_VALIDATE_QUADWORD( m_value );
	_VEC_VALIDATE_QUADWORD( rhs.m_value );
	v128 divResult; _VEC_DIV( divResult, m_value, rhs.m_value );
	return FwVector4( divResult );
}

//--------------------------------------------------------------------------------------------------
/**
	@brief			Sets current object to 'this + rhs'
**/
//--------------------------------------------------------------------------------------------------

inline	FwVector4	FwVector4::operator += ( FwVector4_arg rhs )
{
	_VEC_VALIDATE_QUADWORD( m_value );
	_VEC_VALIDATE_QUADWORD( rhs.m_value );
	m_value = _VEC_ADD( m_value, rhs.m_value );
	return *this;
}

//--------------------------------------------------------------------------------------------------
/**
	@brief			Sets current object to 'this + rhs scalar'
**/
//--------------------------------------------------------------------------------------------------

inline	FwVector4	FwVector4::operator += ( FwScalar_arg rhs )
{
	_VEC_VALIDATE_QUADWORD( m_value );
	_VEC_VALIDATE_QUADWORD( rhs.m_value );
	m_value = _VEC_ADD( m_value, rhs.m_value );
	return *this;
}

//--------------------------------------------------------------------------------------------------
/**
	@brief			Sets current object to 'this - rhs'
**/
//--------------------------------------------------------------------------------------------------

inline	FwVector4	FwVector4::operator -= ( FwVector4_arg rhs )
{
	_VEC_VALIDATE_QUADWORD( m_value );
	_VEC_VALIDATE_QUADWORD( rhs.m_value );
	m_value = _VEC_SUB( m_value, rhs.m_value );
	return *this;
}

//--------------------------------------------------------------------------------------------------
/**
	@brief			Sets current object to 'this - rhs scalar'
**/
//--------------------------------------------------------------------------------------------------

inline	FwVector4	FwVector4::operator -= ( FwScalar_arg rhs )
{
	_VEC_VALIDATE_QUADWORD( m_value );
	_VEC_VALIDATE_QUADWORD( rhs.m_value );
	m_value = _VEC_SUB( m_value, rhs.m_value );
	return *this;
}

//--------------------------------------------------------------------------------------------------
/**
	@brief			Sets current object to 'this * rhs'
**/
//--------------------------------------------------------------------------------------------------

inline	FwVector4	FwVector4::operator *= ( FwVector4_arg rhs )
{
	_VEC_VALIDATE_QUADWORD( m_value );
	_VEC_VALIDATE_QUADWORD( rhs.m_value );
	m_value = _VEC_MUL( m_value, rhs.m_value );
	return *this;
}

//--------------------------------------------------------------------------------------------------
/**
	@brief			Sets current object to 'this * rhs scalar' 
**/
//--------------------------------------------------------------------------------------------------

inline	FwVector4	FwVector4::operator *= ( FwScalar_arg rhs )
{
	_VEC_VALIDATE_QUADWORD( m_value );
	_VEC_VALIDATE_QUADWORD( rhs.m_value );
	m_value = _VEC_MUL( m_value, rhs.m_value );
	return *this;
}

//--------------------------------------------------------------------------------------------------
/**
	@brief			Sets current object to 'this / rhs'
**/
//--------------------------------------------------------------------------------------------------

inline	FwVector4	FwVector4::operator /= ( FwVector4_arg rhs )
{
	_VEC_VALIDATE_QUADWORD( m_value );
	_VEC_VALIDATE_QUADWORD( rhs.m_value );
	v128 divResult; _VEC_DIV( divResult, m_value, rhs.m_value );
	_VEC_VALIDATE_QUADWORD( divResult );
	m_value = divResult;
	return *this;
}

//--------------------------------------------------------------------------------------------------
/**
	@brief			Sets current object to 'this / rhs scalar'
**/
//--------------------------------------------------------------------------------------------------

inline	FwVector4	FwVector4::operator /= ( FwScalar_arg rhs )
{
	_VEC_VALIDATE_QUADWORD( m_value );
	_VEC_VALIDATE_QUADWORD( rhs.m_value );
	v128 divResult; _VEC_DIV( divResult, m_value, rhs.m_value );
	_VEC_VALIDATE_QUADWORD( divResult );
	m_value = divResult;
	return *this;
}

//--------------------------------------------------------------------------------------------------
/**
	@brief			Returns '-this'
**/
//--------------------------------------------------------------------------------------------------

inline	FwVector4	FwVector4::operator - () const
{
	_VEC_VALIDATE_QUADWORD( m_value );
	return FwVector4( _VEC_NEG( m_value ) );
}

//--------------------------------------------------------------------------------------------------
/**
	@brief			Retrieves the X component as a scalar obect
**/
//--------------------------------------------------------------------------------------------------

inline	FwScalar	FwVector4::X( void ) const
{
	_VEC_VALIDATE_QUADWORD( m_value );
	return FwScalar( _VEC_REPLICATE_X( m_value ) );
}

//--------------------------------------------------------------------------------------------------
/**
	@brief			Retrieves the Y component as a scalar obect
**/
//--------------------------------------------------------------------------------------------------

inline	FwScalar	FwVector4::Y( void ) const
{
	_VEC_VALIDATE_QUADWORD( m_value );
	return FwScalar( _VEC_REPLICATE_Y( m_value ) );
}

//--------------------------------------------------------------------------------------------------
/**
	@brief			Retrieves the Z component as a scalar obect
**/
//--------------------------------------------------------------------------------------------------

inline	FwScalar	FwVector4::Z( void ) const
{
	_VEC_VALIDATE_QUADWORD( m_value );
	return FwScalar( _VEC_REPLICATE_Z( m_value ) );
}

//--------------------------------------------------------------------------------------------------
/**
	@brief			Retrieves the W component as a scalar obect
**/
//--------------------------------------------------------------------------------------------------

inline	FwScalar	FwVector4::W( void ) const
{
	_VEC_VALIDATE_QUADWORD( m_value );
	return FwScalar( _VEC_REPLICATE_W( m_value ) );
}

//--------------------------------------------------------------------------------------------------
/**
	@brief			Set the X component of a vector using a scalar value
**/
//--------------------------------------------------------------------------------------------------

inline	void		FwVector4::SetX( FwScalar_arg x )
{
	_VEC_VALIDATE_QUADWORD( m_value );
	_VEC_VALIDATE_QUADWORD( x.m_value );
	_VEC_SET_X( m_value, x.m_value );
}

//--------------------------------------------------------------------------------------------------
/**
	@brief			Set the Y component of a vector using a scalar value
**/
//--------------------------------------------------------------------------------------------------

inline	void		FwVector4::SetY( FwScalar_arg y )
{
	_VEC_VALIDATE_QUADWORD( m_value );
	_VEC_VALIDATE_QUADWORD( y.m_value );
	_VEC_SET_Y( m_value, y.m_value );
}

//--------------------------------------------------------------------------------------------------
/**
	@brief			Set the Z component of a vector using a scalar value
**/
//--------------------------------------------------------------------------------------------------

inline	void		FwVector4::SetZ( FwScalar_arg z )
{
	_VEC_VALIDATE_QUADWORD( m_value );
	_VEC_VALIDATE_QUADWORD( z.m_value );
	_VEC_SET_Z( m_value, z.m_value );
}

//--------------------------------------------------------------------------------------------------
/**
	@brief			Set the W component of a vector using a scalar value
**/
//--------------------------------------------------------------------------------------------------

inline	void		FwVector4::SetW( FwScalar_arg w )
{
	_VEC_VALIDATE_QUADWORD( m_value );
	_VEC_VALIDATE_QUADWORD( w.m_value );
	_VEC_SET_W( m_value, w.m_value );
}

//--------------------------------------------------------------------------------------------------
/**
	@brief			Returns the value of the element at the specified index (0=X, 1=Y, 2=Z, 3=W)

	@param			index				Index of the element (Valid values are 0, 1, 2, 3)

	@result			A scalar containing the element.
**/
//--------------------------------------------------------------------------------------------------

inline	FwScalar	FwVector4::Get( int index ) const
{	
	assert( ( index >= 0 ) && ( index <= 3 ) );
	_VEC_VALIDATE_QUADWORD( m_value );
	_VEC_RETURN_ELEMENT( m_value, index );
}

//--------------------------------------------------------------------------------------------------
/**
	@brief			Returns the value of the element at the specified index (0=X, 1=Y, 2=Z, 3=W)

	@param			index				Index of the element (Valid values are 0, 1, 2, 3)
					val					Scalar value to insert at the specified element
**/
//--------------------------------------------------------------------------------------------------
			
inline	void		FwVector4::Set( int index, FwScalar_arg val )
{
	assert( ( index >= 0 ) && ( index <= 3 ) );
	_VEC_VALIDATE_QUADWORD( m_value );
	_VEC_VALIDATE_QUADWORD( val.m_value );
	_VEC_SET_ELEMENT( m_value, index, val.m_value );
}

//--------------------------------------------------------------------------------------------------
/**
	@brief			Returns the value of the element at the specified index (0=X, 1=Y, 2=Z, 3=W)

	@param			index				Index of the element (Valid values are 0, 1, 2, 3)

	@result			A const scalar containing the element, the constness of which allows us to 
					error at compile time if someone attempts to do use operator [] to write a value.
**/
//--------------------------------------------------------------------------------------------------

inline	const FwScalar	FwVector4::operator []( int index ) const
{
	assert( ( index >= 0 ) && ( index <= 3 ) );
	_VEC_VALIDATE_QUADWORD( m_value );
	_VEC_RETURN_ELEMENT( m_value, index );
}

//--------------------------------------------------------------------------------------------------
/**
	@brief			Returns the quadword in the current object by value
**/
//--------------------------------------------------------------------------------------------------

inline	v128	FwVector4::QuadwordValue( void ) const
{
	_VEC_VALIDATE_QUADWORD( m_value );
	return 	m_value;
}

//--------------------------------------------------------------------------------------------------
/**
	@brief			Returns the quadword in the current object by value, without any validation
					This typically exists for those cases where the FwVector4 may have come from 
					a 3 component source such as point or vector, where the W component should not
					undergo 4-component  validation. 

	@note			This is primarily for internal use, but having it generally accessible saves us
					the headache of friending explicit non-member matrix functions, which then 
					ties the two classes together in an undesirable way. 
**/
//--------------------------------------------------------------------------------------------------

inline	v128	FwVector4::QuadwordValueXYZ( void ) const
{
	_VEC_VALIDATE_QUADWORD_XYZ( m_value );
	return 	m_value;
}

// -------------------------------------------------------------------------------------------------
// Non-member functions

//--------------------------------------------------------------------------------------------------
/**
	@brief			Determines the largest value held in the input object (3 component)
**/
//--------------------------------------------------------------------------------------------------

inline	FwScalar	MaxComp3( FwVector4_arg vec )
{
	_VEC_VALIDATE_QUADWORD_XYZ( vec.m_value );
	v128	tempVector;
	tempVector = _VEC_MAX( _VEC_REPLICATE_X( vec.m_value ), _VEC_REPLICATE_Y( vec.m_value ) );
	tempVector = _VEC_MAX( tempVector, _VEC_REPLICATE_Z( vec.m_value ) );
	return FwScalar( tempVector );
}

//--------------------------------------------------------------------------------------------------
/**
	@brief			Determines the largest value held in the input object (4 component)
**/
//--------------------------------------------------------------------------------------------------

inline	FwScalar	MaxComp4( FwVector4_arg vec )
{
	_VEC_VALIDATE_QUADWORD( vec.m_value );
	v128	tempVector;
	tempVector = _VEC_MAX( _VEC_REPLICATE_X( vec.m_value ), _VEC_REPLICATE_Y( vec.m_value ) );
	tempVector = _VEC_MAX( tempVector, _VEC_REPLICATE_Z( vec.m_value ) );
	tempVector = _VEC_MAX( tempVector, _VEC_REPLICATE_W( vec.m_value ) );
	return FwScalar( tempVector );
}

//--------------------------------------------------------------------------------------------------
/**
	@brief			Determines the smallest value held in the input object (3 component)
**/
//--------------------------------------------------------------------------------------------------

inline	FwScalar	MinComp3( FwVector4_arg vec )
{
	_VEC_VALIDATE_QUADWORD_XYZ( vec.m_value );
	v128	tempVector;
	tempVector = _VEC_MIN( _VEC_REPLICATE_X( vec.m_value ), _VEC_REPLICATE_Y( vec.m_value ) );
	tempVector = _VEC_MIN( tempVector, _VEC_REPLICATE_Z( vec.m_value ) );
	return FwScalar( tempVector );
}

//--------------------------------------------------------------------------------------------------
/**
	@brief			Determines the smallest value held in the input object (4 component)
**/
//--------------------------------------------------------------------------------------------------

inline	FwScalar	MinComp4( FwVector4_arg vec )
{
	_VEC_VALIDATE_QUADWORD( vec.m_value );
	v128	tempVector;
	tempVector = _VEC_MIN( _VEC_REPLICATE_X( vec.m_value ), _VEC_REPLICATE_Y( vec.m_value ) );
	tempVector = _VEC_MIN( tempVector, _VEC_REPLICATE_Z( vec.m_value ) );
	tempVector = _VEC_MIN( tempVector, _VEC_REPLICATE_W( vec.m_value ) );
	return FwScalar( tempVector );
}

//--------------------------------------------------------------------------------------------------
/**
	@brief			Compute the sum of X, Y, and Z elements within input object
**/
//--------------------------------------------------------------------------------------------------

inline	FwScalar	Sum3( FwVector4_arg vec )
{
	_VEC_VALIDATE_QUADWORD_XYZ( vec.m_value );
	v128 tempVector = _VEC_ADD( _VEC_REPLICATE_X( vec.m_value ), _VEC_REPLICATE_Y( vec.m_value ) );
	return FwScalar( _VEC_ADD( tempVector, _VEC_REPLICATE_Z( vec.m_value ) ) );
}

//--------------------------------------------------------------------------------------------------
/**
	@brief			Compute the sum of X, Y, Z, and W elements within input object
**/
//--------------------------------------------------------------------------------------------------

inline	FwScalar	Sum4( FwVector4_arg vec )
{
	_VEC_VALIDATE_QUADWORD( vec.m_value );
	v128 tempVec1 = _VEC_ADD( _VEC_REPLICATE_X( vec.m_value ), _VEC_REPLICATE_Y( vec.m_value ) );
	v128 tempVec2 = _VEC_ADD( _VEC_REPLICATE_Z( vec.m_value ), _VEC_REPLICATE_W( vec.m_value ) );
	return FwScalar( _VEC_ADD( tempVec1, tempVec2 ) );
}

//--------------------------------------------------------------------------------------------------
/**
	@brief			Compute the squared length of the input object (3 component)
**/	
//--------------------------------------------------------------------------------------------------

inline	FwScalar	Length3Sqr( FwVector4_arg vec )
{
	_VEC_VALIDATE_QUADWORD_XYZ( vec.m_value );
	v128 lenSqrResult; _VEC_DOT3( lenSqrResult, vec.m_value, vec.m_value );
	return FwScalar( _VEC_REPLICATE_X( lenSqrResult ) );
}

//--------------------------------------------------------------------------------------------------
/**
	@brief			Compute the reciprocal of the length of the input object (3 component)
**/
//--------------------------------------------------------------------------------------------------

inline	FwScalar	Length3Rcp( FwVector4_arg vec )
{
	_VEC_VALIDATE_QUADWORD_XYZ( vec.m_value );
	v128 lenSqrResult; _VEC_DOT3( lenSqrResult, vec.m_value, vec.m_value );
	v128 lenRcpResult; _VEC_RSQRT( lenRcpResult, _VEC_REPLICATE_X( lenSqrResult ) );
	return FwScalar( lenRcpResult );	
}

//--------------------------------------------------------------------------------------------------
/**
	@brief			Compute the length of the input object (3 component)
**/
//--------------------------------------------------------------------------------------------------

inline	FwScalar	Length3( FwVector4_arg vec )
{
	_VEC_VALIDATE_QUADWORD_XYZ( vec.m_value );
	v128 lenSqrResult; _VEC_DOT3( lenSqrResult, vec.m_value, vec.m_value );
	v128 lenSqrtResult; _VEC_SQRT( lenSqrtResult, _VEC_REPLICATE_X( lenSqrResult ) );
	return FwScalar( lenSqrtResult );	
}

//--------------------------------------------------------------------------------------------------
/**
	@brief			Compute the squared length of the input object (4 component)
**/	
//--------------------------------------------------------------------------------------------------

inline	FwScalar	Length4Sqr( FwVector4_arg vec )
{
	_VEC_VALIDATE_QUADWORD( vec.m_value );
	v128 lenSqrResult; _VEC_DOT4( lenSqrResult, vec.m_value, vec.m_value );
	return FwScalar( _VEC_REPLICATE_X( lenSqrResult ) );
}

//--------------------------------------------------------------------------------------------------
/**
	@brief			Compute the reciprocal of the length of the input object (4 component)
**/
//--------------------------------------------------------------------------------------------------

inline	FwScalar	Length4Rcp( FwVector4_arg vec )
{
	_VEC_VALIDATE_QUADWORD( vec.m_value );
	v128 lenSqrResult; _VEC_DOT4( lenSqrResult, vec.m_value, vec.m_value );
	v128 lenRcpResult; _VEC_RSQRT( lenRcpResult, _VEC_REPLICATE_X( lenSqrResult ) );
	return FwScalar( lenRcpResult );	
}

//--------------------------------------------------------------------------------------------------
/**
	@brief			Compute the length of the input object (4 component)
**/
//--------------------------------------------------------------------------------------------------

inline	FwScalar	Length4( FwVector4_arg vec )
{
	_VEC_VALIDATE_QUADWORD( vec.m_value );
	v128 lenSqrResult; _VEC_DOT4( lenSqrResult, vec.m_value, vec.m_value );
	v128 lenSqrtResult; _VEC_SQRT( lenSqrtResult, _VEC_REPLICATE_X( lenSqrResult ) );
	return FwScalar( lenSqrtResult );	
}

//--------------------------------------------------------------------------------------------------
/**
	@brief			Returns an object containing absolute values of each element in input object
**/
//--------------------------------------------------------------------------------------------------

inline	FwVector4	Abs( FwVector4_arg vec )
{
	_VEC_VALIDATE_QUADWORD( vec.m_value );
	return FwVector4( _VEC_ABS( vec.m_value ) );
}

//--------------------------------------------------------------------------------------------------
/**
	@brief			Returns an object containing the sign of each element in input object (-1,1)
**/
//--------------------------------------------------------------------------------------------------

inline	FwVector4	Sign( FwVector4_arg vec )
{
	_VEC_VALIDATE_QUADWORD( vec.m_value );
	v128 signResult; _VEC_SIGN( signResult, vec.m_value );
	return FwVector4( signResult );
}

//--------------------------------------------------------------------------------------------------
/**
	@brief			Returns an object containing the signum of each element in input object (-1,0,1)
**/
//--------------------------------------------------------------------------------------------------

inline	FwVector4	Signum( FwVector4_arg vec )
{
	_VEC_VALIDATE_QUADWORD( vec.m_value );
	v128 signumResult; _VEC_SIGNUM( signumResult, vec.m_value );
	return FwVector4( signumResult );
}

//--------------------------------------------------------------------------------------------------
/**
	@brief			Returns an object containing reciprocal of each element in input object
**/
//--------------------------------------------------------------------------------------------------

inline	FwVector4	Recip( FwVector4_arg vec )
{
	_VEC_VALIDATE_QUADWORD( vec.m_value );
	v128 recipResult; _VEC_RCP( recipResult, vec.m_value );
	return FwVector4( recipResult );
}

//--------------------------------------------------------------------------------------------------
/**
	@brief			Returns the square root of each element in input object
**/
//--------------------------------------------------------------------------------------------------

inline	FwVector4	Sqrt( FwVector4_arg vec )
{
	_VEC_VALIDATE_QUADWORD( vec.m_value );
	v128 sqrtResult; _VEC_SQRT( sqrtResult, vec.m_value );
	return FwVector4( sqrtResult );
}

//--------------------------------------------------------------------------------------------------
/**
	@brief			Returns the reciprocal square root of each element in input object
**/
//--------------------------------------------------------------------------------------------------

inline	FwVector4	RecipSqrt( FwVector4_arg vec )
{
	_VEC_VALIDATE_QUADWORD( vec.m_value );
	v128 rsqrtResult; _VEC_RSQRT( rsqrtResult, vec.m_value );
	return FwVector4( rsqrtResult );
}

//--------------------------------------------------------------------------------------------------
/**
	@brief			Returns an object containing element-wise maximum values of two input objects
**/
//--------------------------------------------------------------------------------------------------

inline	FwVector4	Max( FwVector4_arg lhs, FwVector4_arg rhs )
{
	_VEC_VALIDATE_QUADWORD( lhs.m_value );
	_VEC_VALIDATE_QUADWORD( rhs.m_value );
	return FwVector4( _VEC_MAX( lhs.m_value, rhs.m_value ) );
}

//--------------------------------------------------------------------------------------------------
/**
	@brief			Returns an object containing element-wise minimum values of two input objects
**/
//--------------------------------------------------------------------------------------------------

inline	FwVector4	Min( FwVector4_arg lhs, FwVector4_arg rhs )
{
	_VEC_VALIDATE_QUADWORD( lhs.m_value );
	_VEC_VALIDATE_QUADWORD( rhs.m_value );
	return FwVector4( _VEC_MIN( lhs.m_value, rhs.m_value ) );
}

//--------------------------------------------------------------------------------------------------
/**
	@brief			Returns an object containing clamped values based on minimum and maximum limits
**/
//--------------------------------------------------------------------------------------------------

inline	FwVector4	Clamp( FwVector4_arg vec, FwVector4_arg mn, FwVector4_arg mx )
{
	_VEC_VALIDATE_QUADWORD( vec.m_value );
	_VEC_VALIDATE_QUADWORD( mn.m_value );
	_VEC_VALIDATE_QUADWORD( mx.m_value );
	return FwVector4( _VEC_CLAMP( vec.m_value, mn.m_value, mx.m_value ) );
}

//--------------------------------------------------------------------------------------------------
/**
	@brief			Returns the 3-element dot product of input objects
**/
//--------------------------------------------------------------------------------------------------

inline	FwScalar	Dot3( FwVector4_arg lhs, FwVector4_arg rhs )
{
	_VEC_VALIDATE_QUADWORD_XYZ( lhs.m_value );
	_VEC_VALIDATE_QUADWORD_XYZ( rhs.m_value );
	v128 dot3Result; _VEC_DOT3( dot3Result, lhs.m_value, rhs.m_value );
	return FwScalar( _VEC_REPLICATE_X( dot3Result ) );
}

//--------------------------------------------------------------------------------------------------
/**
	@brief			Returns the 4-element dot product of input objects
**/
//--------------------------------------------------------------------------------------------------

inline	FwScalar	Dot4( FwVector4_arg lhs, FwVector4_arg rhs )
{
	_VEC_VALIDATE_QUADWORD( lhs.m_value );
	_VEC_VALIDATE_QUADWORD( rhs.m_value );
	v128 dot4Result; _VEC_DOT4( dot4Result, lhs.m_value, rhs.m_value );
	return FwScalar( _VEC_REPLICATE_X( dot4Result ) );
}

//--------------------------------------------------------------------------------------------------
/**
	@brief			Returns the cross product of the input object
**/
//--------------------------------------------------------------------------------------------------

inline	FwVector4	Cross( FwVector4_arg lhs, FwVector4_arg rhs )
{
	_VEC_VALIDATE_QUADWORD( lhs.m_value );
	_VEC_VALIDATE_QUADWORD( rhs.m_value );
	v128 crossResult; _VEC_CROSS( crossResult, lhs.m_value, rhs.m_value );
	crossResult = _VEC_AND( crossResult, _VEC_GET_MASKOFF_W() );
	return FwVector4( crossResult );
}

//--------------------------------------------------------------------------------------------------
/**
	@brief			Returns an object holding normalised version of input object (3 component)
**/
//--------------------------------------------------------------------------------------------------

inline	FwVector4	Normalize3( FwVector4_arg vec )
{
	_VEC_VALIDATE_QUADWORD_XYZ( vec.m_value );
	v128	result; _VEC_NORMALIZE3( result, vec.m_value );
	result = _VEC_AND( result, _VEC_GET_MASKOFF_W() );
	return FwVector4( result );
}

//--------------------------------------------------------------------------------------------------
/**
	@brief			Returns an object holding normalised version of 3 component input object, or
					specified fallback value if 'vec' could not be successfully normalised.
**/
//--------------------------------------------------------------------------------------------------

inline	FwVector4	SafeNormalize3( FwVector4_arg vec, FwVector4_arg fallback )
{
	_VEC_VALIDATE_QUADWORD_XYZ( vec.m_value );
	_VEC_VALIDATE_QUADWORD_XYZ( fallback.m_value );
	v128	result; _VEC_SAFE_NORMALIZE3( result, vec.m_value, fallback.m_value );
	return FwVector4( result );
}

//--------------------------------------------------------------------------------------------------
/**
	@brief			Returns an object holding normalised version of input object (4 component)
**/
//--------------------------------------------------------------------------------------------------

inline	FwVector4	Normalize4( FwVector4_arg vec )
{
	_VEC_VALIDATE_QUADWORD( vec.m_value );
	v128	result; _VEC_NORMALIZE4( result, vec.m_value );
	return FwVector4( result );
}

//--------------------------------------------------------------------------------------------------
/**
	@brief			Returns an object holding normalised version of 4 component input object, or
					specified fallback value if 'vec' could not be successfully normalised.
**/
//--------------------------------------------------------------------------------------------------

inline	FwVector4	SafeNormalize4( FwVector4_arg vec, FwVector4_arg fallback )
{
	_VEC_VALIDATE_QUADWORD( vec.m_value );
	_VEC_VALIDATE_QUADWORD( fallback.m_value );
	v128	result; _VEC_SAFE_NORMALIZE4( result, vec.m_value, fallback.m_value );
	return FwVector4( result );
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

inline	FwVector4	Lerp( FwVector4_arg lhs, FwVector4_arg rhs, FwScalar_arg alpha )
{
	_VEC_VALIDATE_QUADWORD( lhs.m_value );
	_VEC_VALIDATE_QUADWORD( rhs.m_value );
	_VEC_VALIDATE_QUADWORD( alpha.QuadwordValue() );
	return FwVector4( _VEC_MADD( _VEC_SUB( rhs.m_value, lhs.m_value ), alpha.QuadwordValue(), lhs.m_value ) );
}

//--------------------------------------------------------------------------------------------------
/**
	@brief			Returns whether all components in lhs are < equivalent components in rhs
*/
//--------------------------------------------------------------------------------------------------

inline	bool		AllComponentsLessThan( FwVector4_arg lhs, FwVector4_arg rhs )
{
	_VEC_VALIDATE_QUADWORD( lhs.m_value );
	_VEC_VALIDATE_QUADWORD( rhs.m_value );
	return _VEC_ALL_COMPONENTS_LT( lhs.m_value, rhs.m_value );
}

//--------------------------------------------------------------------------------------------------
/**
	@brief			Returns whether all components in lhs are <= equivalent components in rhs
*/
//--------------------------------------------------------------------------------------------------

inline	bool		AllComponentsLessThanOrEqual( FwVector4_arg lhs, FwVector4_arg rhs )
{
	_VEC_VALIDATE_QUADWORD( lhs.m_value );
	_VEC_VALIDATE_QUADWORD( rhs.m_value );
	return _VEC_ALL_COMPONENTS_LE( lhs.m_value, rhs.m_value );
}

//--------------------------------------------------------------------------------------------------
/**
	@brief			Returns whether all components in lhs are > equivalent components in rhs
*/
//--------------------------------------------------------------------------------------------------

inline	bool		AllComponentsGreaterThan( FwVector4_arg lhs, FwVector4_arg rhs )
{
	_VEC_VALIDATE_QUADWORD( lhs.m_value );
	_VEC_VALIDATE_QUADWORD( rhs.m_value );
	return _VEC_ALL_COMPONENTS_GT( lhs.m_value, rhs.m_value );
}

//--------------------------------------------------------------------------------------------------
/**
	@brief			Returns whether all components in lhs are >=  equivalent components in rhs
*/
//--------------------------------------------------------------------------------------------------

inline	bool		AllComponentsGreaterThanOrEqual( FwVector4_arg lhs, FwVector4_arg rhs )
{
	_VEC_VALIDATE_QUADWORD( lhs.m_value );
	_VEC_VALIDATE_QUADWORD( rhs.m_value );
	return _VEC_ALL_COMPONENTS_GE( lhs.m_value, rhs.m_value );
}

//--------------------------------------------------------------------------------------------------
/**
	@brief			Returns whether all components in lhs are ==  equivalent components in rhs
*/
//--------------------------------------------------------------------------------------------------

inline	bool		AllComponentsEqual( FwVector4_arg lhs, FwVector4_arg rhs )
{
	_VEC_VALIDATE_QUADWORD( lhs.m_value );
	_VEC_VALIDATE_QUADWORD( rhs.m_value );
	return _VEC_ALL_COMPONENTS_EQ( lhs.m_value, rhs.m_value );
}

//--------------------------------------------------------------------------------------------------
/**
	@brief			Same as 'FwVector4 * FwScalar', but keeps scalar behaviour when scalar is on lhs
**/
//--------------------------------------------------------------------------------------------------

inline	FwVector4	operator * ( FwScalar_arg scalar, FwVector4_arg vec )
{
	return vec * scalar;
}

//--------------------------------------------------------------------------------------------------
/**
	@brief			Same as 'FwVector4 + FwScalar', but keeps scalar behaviour when scalar is on lhs
**/
//--------------------------------------------------------------------------------------------------

inline	FwVector4	operator + ( FwScalar_arg scalar, FwVector4_arg vec )
{
	return vec + scalar;
}

#endif	// FW_VECTOR4_INL
