//--------------------------------------------------------------------------------------------------
/**
	@file		
	
	@brief		SIMD Scalar

	@note		(c) Copyright Sony Computer Entertainment 2005. All Rights Reserved.	
**/
//--------------------------------------------------------------------------------------------------

#ifndef FW_SCALAR_INL
#define FW_SCALAR_INL

//--------------------------------------------------------------------------------------------------
/**
	@brief			Default constructor
**/
//--------------------------------------------------------------------------------------------------

inline	FwScalar::FwScalar()
{
	_VEC_INITIALISE_QUADWORD( m_value );
}

//--------------------------------------------------------------------------------------------------
/**
	@brief			Constructs from a single float
**/
//--------------------------------------------------------------------------------------------------

inline	FwScalar::FwScalar( float f )
{
	m_value = _VEC_REPLICATE_FLOAT( f );
	_VEC_VALIDATE_QUADWORD( m_value );
}

//--------------------------------------------------------------------------------------------------
/**
	@brief			Copy constructor
**/
//--------------------------------------------------------------------------------------------------

#ifdef	_MSC_VER
inline	FwScalar::FwScalar( const FwScalar& rhs )
{
	_VEC_VALIDATE_QUADWORD( rhs.m_value );
	m_value = rhs.m_value;
}
#endif	//_MSC_VER

//--------------------------------------------------------------------------------------------------
/**
	@brief			Constructs from a raw quadword
**/
//--------------------------------------------------------------------------------------------------

inline	FwScalar::FwScalar( v128 vec )
{
	_VEC_VALIDATE_QUADWORD( vec );
	m_value = vec;
}

//--------------------------------------------------------------------------------------------------
/**
	@brief			Constructs to object containing zero
**/
//--------------------------------------------------------------------------------------------------

inline	FwScalar::FwScalar( const FwMaths::ZeroTag& )
{
	m_value = _VEC_SET_ZERO();
}

//--------------------------------------------------------------------------------------------------
/**
	@brief			Conversion to a floating point number
**/
//--------------------------------------------------------------------------------------------------

inline	FwScalar::operator float() const
{
	_VEC_VALIDATE_QUADWORD( m_value );
	float result; _VEC_GET_FLOAT_X( result, m_value );
	return result;
}

//--------------------------------------------------------------------------------------------------
/**
	@brief			Assigns current object to kZero  -  (0, 0, 0, 0)
**/
//--------------------------------------------------------------------------------------------------

inline	FwScalar	FwScalar::operator =( const FwMaths::ZeroTag& )
{
	m_value = _VEC_SET_ZERO();
	return *this;
}

//--------------------------------------------------------------------------------------------------
/**
	@brief			'this = rhs'
**/
//--------------------------------------------------------------------------------------------------

inline	FwScalar	FwScalar::operator =( FwScalar_arg v )
{
	_VEC_VALIDATE_QUADWORD( v.m_value );
	m_value = v.m_value;
	return *this;
}

//--------------------------------------------------------------------------------------------------
/**
	@brief			Returns the quadword in the current object by value
**/
//--------------------------------------------------------------------------------------------------

inline	v128	FwScalar::QuadwordValue( void ) const
{
	_VEC_VALIDATE_QUADWORD( m_value );
	return 	m_value;
}

//--------------------------------------------------------------------------------------------------
/**
	@brief			Return 'this + rhs'
**/
//--------------------------------------------------------------------------------------------------

inline	FwScalar	FwScalar::operator +( FwScalar_arg r ) const
{
	_VEC_VALIDATE_QUADWORD( m_value );
	_VEC_VALIDATE_QUADWORD( r.m_value );
	return FwScalar( _VEC_ADD( m_value, r.m_value ) );
}

//--------------------------------------------------------------------------------------------------
/**
	@brief			Return 'this - rhs'
**/
//--------------------------------------------------------------------------------------------------

inline	FwScalar	FwScalar::operator -( FwScalar_arg r ) const
{
	_VEC_VALIDATE_QUADWORD( m_value );
	_VEC_VALIDATE_QUADWORD( r.m_value );
	return FwScalar( _VEC_SUB( m_value, r.m_value ) );
}

//--------------------------------------------------------------------------------------------------
/**
	@brief			Return 'this * rhs'
**/
//--------------------------------------------------------------------------------------------------

inline	FwScalar	FwScalar::operator *( FwScalar_arg r ) const
{
	_VEC_VALIDATE_QUADWORD( m_value );
	_VEC_VALIDATE_QUADWORD( r.m_value );
	return FwScalar( _VEC_MUL( m_value, r.m_value ) );
}

//--------------------------------------------------------------------------------------------------
/**
	@brief			Return 'this / rhs'
**/
//--------------------------------------------------------------------------------------------------

inline	FwScalar	FwScalar::operator /( FwScalar_arg r ) const
{
	_VEC_VALIDATE_QUADWORD( m_value );
	_VEC_VALIDATE_QUADWORD( r.m_value );
	v128 divResult; _VEC_DIV( divResult, m_value, r.m_value );
	return FwScalar( divResult );
}

//--------------------------------------------------------------------------------------------------
/**
	@brief			Return 'this + rhs float'
**/
//--------------------------------------------------------------------------------------------------

inline	FwScalar	FwScalar::operator +( float r ) const
{
	_VEC_VALIDATE_QUADWORD( m_value );
	return FwScalar( _VEC_ADD( m_value, _VEC_REPLICATE_FLOAT( r ) ) );
}

//--------------------------------------------------------------------------------------------------
/**
	@brief			Return 'this - rhs float'
**/
//--------------------------------------------------------------------------------------------------

inline	FwScalar	FwScalar::operator -( float r ) const
{
	_VEC_VALIDATE_QUADWORD( m_value );
	return FwScalar( _VEC_SUB( m_value, _VEC_REPLICATE_FLOAT( r ) ) );
}

//--------------------------------------------------------------------------------------------------
/**
	@brief			Return 'this * rhs float'
**/
//--------------------------------------------------------------------------------------------------

inline	FwScalar	FwScalar::operator *( float r ) const
{
	_VEC_VALIDATE_QUADWORD( m_value );
	return FwScalar( _VEC_MUL( m_value, _VEC_REPLICATE_FLOAT( r ) ) );
}

//--------------------------------------------------------------------------------------------------
/**
	@brief			Return 'this / rhs float'
**/
//--------------------------------------------------------------------------------------------------

inline	FwScalar	FwScalar::operator /( float r ) const
{
	_VEC_VALIDATE_QUADWORD( m_value );
	v128 divResult; _VEC_DIV( divResult, m_value, _VEC_REPLICATE_FLOAT( r ) );
	return FwScalar( divResult );
}

//--------------------------------------------------------------------------------------------------
/**
	@brief			Operators to keep scalar behaviour when scalar is on rhs.
**/
//--------------------------------------------------------------------------------------------------

inline	FwScalar	operator +( float f, FwScalar_arg v )
{
	_VEC_VALIDATE_QUADWORD( v.m_value );
	return FwScalar( _VEC_ADD( _VEC_REPLICATE_FLOAT( f ), v.m_value ) );
}

inline	FwScalar	operator -( float f, FwScalar_arg v )
{
	_VEC_VALIDATE_QUADWORD( v.m_value );
	return FwScalar( _VEC_SUB( _VEC_REPLICATE_FLOAT( f ), v.m_value ) );
}

inline	FwScalar	operator *( float f, FwScalar_arg v )
{
	_VEC_VALIDATE_QUADWORD( v.m_value );
	return FwScalar( _VEC_MUL( _VEC_REPLICATE_FLOAT( f ), v.m_value ) );
}

inline	FwScalar	operator /( float f, FwScalar_arg v )
{
	_VEC_VALIDATE_QUADWORD( v.m_value );
	v128 divResult; _VEC_DIV( divResult, _VEC_REPLICATE_FLOAT( f ), v.m_value );
	return FwScalar( divResult );
}

//--------------------------------------------------------------------------------------------------
/**
	@brief			Sets current object to 'this + rhs'
**/
//--------------------------------------------------------------------------------------------------

inline	FwScalar	FwScalar::operator +=( FwScalar_arg r )
{
	_VEC_VALIDATE_QUADWORD( m_value );
	_VEC_VALIDATE_QUADWORD( r.m_value );
	m_value = _VEC_ADD( m_value, r.m_value );
	return *this;
}

//--------------------------------------------------------------------------------------------------
/**
	@brief			Sets current object to 'this - rhs'
**/
//--------------------------------------------------------------------------------------------------

inline	FwScalar	FwScalar::operator -=( FwScalar_arg r )
{
	_VEC_VALIDATE_QUADWORD( m_value );
	_VEC_VALIDATE_QUADWORD( r.m_value );
	m_value = _VEC_SUB( m_value, r.m_value );
	return *this;
}

//--------------------------------------------------------------------------------------------------
/**
	@brief			Sets current object to 'this * rhs'
**/
//--------------------------------------------------------------------------------------------------

inline	FwScalar	FwScalar::operator *=( FwScalar_arg r )
{
	_VEC_VALIDATE_QUADWORD( m_value );
	_VEC_VALIDATE_QUADWORD( r.m_value );
	m_value = _VEC_MUL( m_value, r.m_value );
	return *this;
}

//--------------------------------------------------------------------------------------------------
/**
	@brief			Sets current object to 'this / rhs'
**/
//--------------------------------------------------------------------------------------------------

inline	FwScalar	FwScalar::operator /=( FwScalar_arg r )
{
	_VEC_VALIDATE_QUADWORD( m_value );
	_VEC_VALIDATE_QUADWORD( r.m_value );
	v128 divResult; _VEC_DIV( divResult, m_value, r.m_value );
	_VEC_VALIDATE_QUADWORD( divResult );
	m_value = divResult;
	return *this;
}

//--------------------------------------------------------------------------------------------------
/**
	@brief			Sets current object to 'this + rhs float'
**/
//--------------------------------------------------------------------------------------------------

inline	FwScalar	FwScalar::operator +=( float f )
{
	_VEC_VALIDATE_QUADWORD( m_value );
	m_value = _VEC_ADD( m_value, _VEC_REPLICATE_FLOAT( f ) );
	return *this;
}

//--------------------------------------------------------------------------------------------------
/**
	@brief			Sets current object to 'this - rhs float'
**/
//--------------------------------------------------------------------------------------------------

inline	FwScalar	FwScalar::operator -=( float f )
{
	_VEC_VALIDATE_QUADWORD( m_value );
	m_value = _VEC_SUB( m_value, _VEC_REPLICATE_FLOAT( f ) );
	return *this;
}

//--------------------------------------------------------------------------------------------------
/**
	@brief			Sets current object to 'this * rhs float'
**/
//--------------------------------------------------------------------------------------------------

inline	FwScalar	FwScalar::operator *=( float f )
{
	_VEC_VALIDATE_QUADWORD( m_value );
	m_value = _VEC_MUL( m_value, _VEC_REPLICATE_FLOAT( f ) );
	return *this;
}

//--------------------------------------------------------------------------------------------------
/**
	@brief			Sets current object to 'this / rhs float'
**/
//--------------------------------------------------------------------------------------------------

inline	FwScalar	FwScalar::operator /=( float f )
{
	_VEC_VALIDATE_QUADWORD( m_value );
	v128 divResult; _VEC_DIV( divResult, m_value, _VEC_REPLICATE_FLOAT( f ) );
	_VEC_VALIDATE_QUADWORD( divResult );
	m_value = divResult;
	return *this;
}

//--------------------------------------------------------------------------------------------------
/**
	@brief			FwScalar '-this'
**/
//--------------------------------------------------------------------------------------------------

inline	FwScalar	FwScalar::operator - () const
{
	_VEC_VALIDATE_QUADWORD( m_value );
	return FwScalar( _VEC_NEG( m_value ) );
}

//--------------------------------------------------------------------------------------------------
/**
	@brief			Returns whether the scalar is positive
**/
//--------------------------------------------------------------------------------------------------

inline	bool		IsPositive( FwScalar_arg vec )
{
	_VEC_VALIDATE_QUADWORD( vec.m_value );
	return _VEC_IS_POSITIVE( vec.m_value );
}	

//--------------------------------------------------------------------------------------------------
/**
	@brief			Returns whether the scalar is negative
**/
//--------------------------------------------------------------------------------------------------

inline	bool		IsNegative( FwScalar_arg vec )
{
	_VEC_VALIDATE_QUADWORD( vec.m_value );
	return _VEC_IS_NEGATIVE( vec.m_value );
}			

//--------------------------------------------------------------------------------------------------
/**
	@brief			Returns the absolute value of the scalar
**/
//--------------------------------------------------------------------------------------------------

inline	FwScalar	Abs( FwScalar_arg vec )
{
	_VEC_VALIDATE_QUADWORD( vec.m_value );
	return FwScalar( _VEC_ABS( vec.m_value ) );
}

//--------------------------------------------------------------------------------------------------
/**
	@brief			Returns an object containing the sign of the scalar (-1,1)
**/
//--------------------------------------------------------------------------------------------------

inline	FwScalar	Sign( FwScalar_arg vec )
{
	_VEC_VALIDATE_QUADWORD( vec.m_value );
	v128 signResult; _VEC_SIGN( signResult, vec.m_value );
	return FwScalar( signResult );
}

//--------------------------------------------------------------------------------------------------
/**
	@brief			Returns an object containing the signum of the scalar (-1,0,1)
**/
//--------------------------------------------------------------------------------------------------

inline	FwScalar	Signum( FwScalar_arg vec )
{
	_VEC_VALIDATE_QUADWORD( vec.m_value );
	v128 signumResult; _VEC_SIGNUM( signumResult, vec.m_value );
	return FwScalar( signumResult );
}

//--------------------------------------------------------------------------------------------------
/**
	@brief			Returns the reciprocal of the scalar
**/
//--------------------------------------------------------------------------------------------------

inline	FwScalar	Recip( FwScalar_arg vec )
{
	_VEC_VALIDATE_QUADWORD( vec.m_value );
	v128 recipResult; _VEC_RCP( recipResult, vec.m_value );
	return FwScalar( recipResult );
}

//--------------------------------------------------------------------------------------------------
/**
	@brief			Returns the square root of the scalar
**/
//--------------------------------------------------------------------------------------------------

inline	FwScalar	Sqrt( FwScalar_arg vec )
{
	_VEC_VALIDATE_QUADWORD( vec.m_value );
	v128 sqrtResult; _VEC_SQRT( sqrtResult, vec.m_value );
	return FwScalar( sqrtResult );
}

//--------------------------------------------------------------------------------------------------
/**
	@brief			Returns the reciprocal square root of the scalar
**/
//--------------------------------------------------------------------------------------------------

inline	FwScalar	RecipSqrt( FwScalar_arg vec )
{
	_VEC_VALIDATE_QUADWORD( vec.m_value );
	v128 rsqrtResult; _VEC_RSQRT( rsqrtResult, vec.m_value );
	return FwScalar( rsqrtResult );
}

//--------------------------------------------------------------------------------------------------
/**
	@brief			Returns the larger of two scalars
**/
//--------------------------------------------------------------------------------------------------

inline	FwScalar	Max( FwScalar_arg lhs, FwScalar_arg rhs )
{
	_VEC_VALIDATE_QUADWORD( lhs.m_value );
	_VEC_VALIDATE_QUADWORD( rhs.m_value );
	return FwScalar( _VEC_MAX( lhs.m_value, rhs.m_value ) );
}

//--------------------------------------------------------------------------------------------------
/**
	@brief			Returns the lesser of two scalars
**/
//--------------------------------------------------------------------------------------------------

inline	FwScalar	Min( FwScalar_arg lhs, FwScalar_arg rhs )
{
	_VEC_VALIDATE_QUADWORD( lhs.m_value );
	_VEC_VALIDATE_QUADWORD( rhs.m_value );
	return FwScalar( _VEC_MIN( lhs.m_value, rhs.m_value ) );
}

//--------------------------------------------------------------------------------------------------
/**
	@brief			Clamps a scalar between a specified range.
**/
//--------------------------------------------------------------------------------------------------

inline	FwScalar	Clamp( FwScalar_arg vec, FwScalar_arg mn, FwScalar_arg mx )
{
	_VEC_VALIDATE_QUADWORD( vec.m_value );
	_VEC_VALIDATE_QUADWORD( mn.m_value );
	_VEC_VALIDATE_QUADWORD( mx.m_value );
	return FwScalar( _VEC_CLAMP( vec.m_value, mn.m_value, mx.m_value ) );
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

inline	FwScalar	Lerp( FwScalar_arg lhs, FwScalar_arg rhs, FwScalar_arg alpha )
{
	_VEC_VALIDATE_QUADWORD( lhs.m_value );
	_VEC_VALIDATE_QUADWORD( rhs.m_value );
	_VEC_VALIDATE_QUADWORD( alpha.m_value );
	return FwScalar( _VEC_MADD( _VEC_SUB( rhs.m_value, lhs.m_value ), alpha.m_value, lhs.m_value ) );
}

//--------------------------------------------------------------------------------------------------
/**
	@brief			Scalar/Scalar comparisons
**/
//--------------------------------------------------------------------------------------------------

inline	bool     operator < ( FwScalar_arg l, FwScalar_arg r ) 
{ 
		return _VEC_ALL_COMPONENTS_LT( l.m_value, r.m_value ); 
} 

inline	bool     operator <=( FwScalar_arg l, FwScalar_arg r ) 
{ 
		return _VEC_ALL_COMPONENTS_LE( l.m_value, r.m_value ); 
} 

inline	bool     operator > ( FwScalar_arg l, FwScalar_arg r ) 
{ 
		return _VEC_ALL_COMPONENTS_GT( l.m_value, r.m_value ); 
} 

inline	bool     operator >=( FwScalar_arg l, FwScalar_arg r ) 
{ 
		return _VEC_ALL_COMPONENTS_GE( l.m_value, r.m_value ); 
} 

inline	bool     operator ==( FwScalar_arg l, FwScalar_arg r ) 
{ 
		return _VEC_ALL_COMPONENTS_EQ( l.m_value, r.m_value ); 
}

inline	bool     operator !=( FwScalar_arg l, FwScalar_arg r ) 
{ 
		return !_VEC_ALL_COMPONENTS_EQ( l.m_value, r.m_value ); 
}

//--------------------------------------------------------------------------------------------------
/**
	@brief			Scalar/Float comparisons
**/
//--------------------------------------------------------------------------------------------------

inline	bool     operator < ( FwScalar_arg l, float r ) 
{ 
		return float(l) < r; 
}

inline	bool     operator <=( FwScalar_arg l, float r ) 
{ 
		return float(l) <= r; 
}

inline	bool     operator > ( FwScalar_arg l, float r ) 
{
		return float(l) > r; 
}

inline	bool     operator >=( FwScalar_arg l, float r ) 
{
		return float(l) >= r; 
}

inline	bool     operator ==( FwScalar_arg l, float r) 
{ 
		return float(l) == r; 
}

inline	bool     operator !=( FwScalar_arg l, float r) 
{ 
		return float(l) != r; 
}

inline bool     operator < ( float r, FwScalar_arg l )
{
		return r < float(l); 
}

inline bool     operator <=( float r, FwScalar_arg l )
{
		return r <= float(l); 
}

inline bool     operator > ( float r, FwScalar_arg l )
{
		return r > float(l); 
}

inline bool     operator >=( float r, FwScalar_arg l )
{
		return r >= float(l); 
}

inline bool     operator ==( float r, FwScalar_arg l )
{
		return r == float(l); 
}

inline bool     operator !=( float r, FwScalar_arg l )
{
		return r != float(l); 
}

#endif	// FW_SCALAR_INL
