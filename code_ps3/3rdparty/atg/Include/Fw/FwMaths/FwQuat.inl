//--------------------------------------------------------------------------------------------------
/**
	@file		
	
	@brief		SIMD Quaternions

	@note		(c) Copyright Sony Computer Entertainment 2005. All Rights Reserved.	
**/
//--------------------------------------------------------------------------------------------------

#ifndef FW_QUAT_INL
#define FW_QUAT_INL

//--------------------------------------------------------------------------------------------------
/**
	@brief			Default constructor
**/
//--------------------------------------------------------------------------------------------------

inline	FwQuat::FwQuat( )
{
	_VEC_INITIALISE_QUADWORD( m_value );
}

//--------------------------------------------------------------------------------------------------
/**
	@brief			Constructor from 4 float values
**/
//--------------------------------------------------------------------------------------------------

inline	FwQuat::FwQuat( float x, float y, float z, float w )
{
	_VEC_SET_FLOATS( m_value, x, y, z, w );
	_VEC_VALIDATE_QUADWORD( m_value );
}

//--------------------------------------------------------------------------------------------------
/**
	@brief			Constructor from axis and angle
**/
//--------------------------------------------------------------------------------------------------

inline	FwQuat::FwQuat( FwVector4_arg axis, float angle )
{
	v128	sinHalfAngle;
	v128	cosHalfAngle;
	v128	tempResult;
	sincosf4( FwScalar( angle * 0.5f ).QuadwordValue(), &sinHalfAngle, &cosHalfAngle );
	tempResult = ( axis * FwVector4( sinHalfAngle ) ).QuadwordValue();
	_VEC_SET_W( tempResult, cosHalfAngle );
	_VEC_NORMALIZE4( m_value, tempResult );
}

//--------------------------------------------------------------------------------------------------
/**
	@brief			Copy constructor
**/
//--------------------------------------------------------------------------------------------------

#ifdef	_MSC_VER
inline	FwQuat::FwQuat( const FwQuat& rhs )
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

inline	FwQuat::FwQuat( const float* pData )
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

inline	FwQuat::FwQuat( v128 rhs )
{
	_VEC_VALIDATE_QUADWORD( rhs );
	m_value = rhs;
}

//--------------------------------------------------------------------------------------------------
/**
	@brief			Constructs from an FwVector4
**/
//--------------------------------------------------------------------------------------------------

inline	FwQuat::FwQuat( FwVector4_arg rhs )
{
	m_value = rhs.QuadwordValue();
}

//--------------------------------------------------------------------------------------------------
/**
	@brief			Constructs to object containing zero
**/
//--------------------------------------------------------------------------------------------------

inline	FwQuat::FwQuat( const FwMaths::ZeroTag& )
{
	m_value = _VEC_SET_ZERO();
}

//--------------------------------------------------------------------------------------------------
/**
	@brief			Constructs to object containing I
**/
//--------------------------------------------------------------------------------------------------

inline	FwQuat::FwQuat( const FwMaths::QuatITag& )
{
	m_value = _VEC_GET_UNIT_X();
}

//--------------------------------------------------------------------------------------------------
/**
	@brief			Constructs to object containing J
**/
//--------------------------------------------------------------------------------------------------

inline	FwQuat::FwQuat( const FwMaths::QuatJTag& )
{
	m_value = _VEC_GET_UNIT_Y();
}

//--------------------------------------------------------------------------------------------------
/**
	@brief			Constructs to object containing K
**/
//--------------------------------------------------------------------------------------------------

inline	FwQuat::FwQuat( const FwMaths::QuatKTag& )
{
	m_value = _VEC_GET_UNIT_Z();
}

//--------------------------------------------------------------------------------------------------
/**
	@brief			Constructs to object containing identity (0, 0, 0, 1)
**/
//--------------------------------------------------------------------------------------------------

inline	FwQuat::FwQuat( const FwMaths::IdentityTag& )
{
	m_value = _VEC_GET_UNIT_W();
}

//--------------------------------------------------------------------------------------------------
/**
	@brief			Assigns current object to FwMaths::kZero  -  (0, 0, 0, 0)
	@param			ZeroTag			Used to associate assignment operator. Contents are unused.
**/
//--------------------------------------------------------------------------------------------------

inline	FwQuat		FwQuat::operator = ( const FwMaths::ZeroTag& )
{
	m_value = _VEC_SET_ZERO();
	return *this;
}

//--------------------------------------------------------------------------------------------------
/**
	@brief			Assigns current object to FwMaths::kQuatI  -  (1, 0, 0, 0)
	@param			QuatITag		Used to associate assignment operator. Contents are unused.
**/
//--------------------------------------------------------------------------------------------------

inline	FwQuat		FwQuat::operator = ( const FwMaths::QuatITag& )
{
	m_value = _VEC_GET_UNIT_X();
	return *this;
}

//--------------------------------------------------------------------------------------------------
/**
	@brief			Assigns current object to FwMaths::kQuatJ  -  (0, 1, 0, 0)
	@param			QuatJTag		Used to associate assignment operator. Contents are unused.
**/
//--------------------------------------------------------------------------------------------------

inline	FwQuat		FwQuat::operator = ( const FwMaths::QuatJTag& )
{
	m_value = _VEC_GET_UNIT_Y();
	return *this;
}

//--------------------------------------------------------------------------------------------------
/**
	@brief			Assigns current object to FwMaths::kQuatK  -  (0, 0, 1, 0)
	@param			QuatKTag		Used to associate assignment operator. Contents are unused.
**/
//--------------------------------------------------------------------------------------------------

inline	FwQuat		FwQuat::operator = ( const FwMaths::QuatKTag& )
{
	m_value = _VEC_GET_UNIT_Z();
	return *this;
}

//--------------------------------------------------------------------------------------------------
/**
	@brief			Assigns current object to FwMaths::kIdentity, which sets object to (0, 0, 0, 1).
	@param			IdentityTag		Used to associate assignment operator. Contents are unused.
**/
//--------------------------------------------------------------------------------------------------

inline	FwQuat		FwQuat::operator = ( const FwMaths::IdentityTag& )
{
	m_value = _VEC_GET_UNIT_W();
	return *this;
}

//--------------------------------------------------------------------------------------------------
/**
	@brief			'this = rhs'
**/
//--------------------------------------------------------------------------------------------------

inline	FwQuat		FwQuat::operator = ( FwQuat_arg rhs )
{
	_VEC_VALIDATE_QUADWORD( rhs.m_value );
	m_value = rhs.m_value;
	return *this;
}

//--------------------------------------------------------------------------------------------------
/**
	@brief			Return 'this + rhs'
**/
//--------------------------------------------------------------------------------------------------

inline	FwQuat		FwQuat::operator + ( FwQuat_arg rhs ) const
{
	_VEC_VALIDATE_QUADWORD( m_value );
	_VEC_VALIDATE_QUADWORD( rhs.m_value );
	return FwQuat( _VEC_ADD( m_value, rhs.m_value ) );
}

//--------------------------------------------------------------------------------------------------
/**
	@brief			Return 'this - rhs'
**/
//--------------------------------------------------------------------------------------------------

inline	FwQuat		FwQuat::operator - ( FwQuat_arg rhs ) const
{
	_VEC_VALIDATE_QUADWORD( m_value );
	_VEC_VALIDATE_QUADWORD( rhs.m_value );
	return FwQuat( _VEC_SUB( m_value, rhs.m_value ) );
}

//--------------------------------------------------------------------------------------------------
/**
	@brief			Return 'this * rhs'
**/
//--------------------------------------------------------------------------------------------------

inline	FwQuat		FwQuat::operator * ( FwQuat_arg rhs ) const
{
	_VEC_VALIDATE_QUADWORD( m_value );
	_VEC_VALIDATE_QUADWORD( rhs.m_value );

    vector float a = m_value;
    vector float b = rhs.m_value;

    vector float zero = (vector float)( 0.0f );

    vector float na = vec_sub(zero, a);

    vector float bx = _VEC_REPLICATE_X( b );
    vector float by = _VEC_REPLICATE_Y( b );
    vector float bz = _VEC_REPLICATE_Z( b );
    vector float bw = _VEC_REPLICATE_W( b );

    vector float r0 = vec_perm(a, na, _VEC_GET_CONSTANT( kShufWZBA) );
    vector float r1 = vec_perm(a, na, _VEC_GET_CONSTANT( kShufCWXB) );
    vector float r2 = vec_perm(a, na, _VEC_GET_CONSTANT( kShufYAWC) );

    vector float res;

    res = vec_madd( a, bw, zero);  // r  = ( ax,  ay,  az,  aw) * bw
    res = vec_madd(r0, bx, res);   // r += (+aw, +az, -ay, -ax) * bx
    res = vec_madd(r1, by, res);   // r += (-az, +aw, +ax, -ay) * by
    res = vec_madd(r2, bz, res);   // r += ( ay, -ax,  aw, -az) * bz

    return FwQuat(res);
}

//--------------------------------------------------------------------------------------------------
/**
	@brief			Return 'this * rhs scalar'
**/
//--------------------------------------------------------------------------------------------------

inline	FwQuat		FwQuat::operator * ( FwScalar_arg rhs ) const
{
	_VEC_VALIDATE_QUADWORD( m_value );
	return FwQuat( _VEC_MUL( m_value, rhs.QuadwordValue() ) );
}

//--------------------------------------------------------------------------------------------------
/**
	@brief			Return 'this / rhs scalar'
**/
//--------------------------------------------------------------------------------------------------

inline	FwQuat		FwQuat::operator / ( FwScalar_arg rhs ) const
{
	_VEC_VALIDATE_QUADWORD( m_value );
	v128 divResult; _VEC_DIV( divResult, m_value, rhs.QuadwordValue() );
	return FwQuat( divResult );
}

//--------------------------------------------------------------------------------------------------
/**
	@brief			Sets current object to 'this + rhs'
**/
//--------------------------------------------------------------------------------------------------

inline	FwQuat		FwQuat::operator += ( FwQuat_arg rhs )
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

inline	FwQuat		FwQuat::operator -= ( FwQuat_arg rhs )
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

inline	FwQuat		FwQuat::operator *= ( FwQuat_arg rhs )
{
	*this = *this * rhs;
	return *this;
}

//--------------------------------------------------------------------------------------------------
/**
	@brief			Sets current object to 'this * rhs scalar'
**/
//--------------------------------------------------------------------------------------------------

inline	FwQuat		FwQuat::operator *= ( FwScalar_arg rhs )
{
	_VEC_VALIDATE_QUADWORD( m_value );
	m_value = _VEC_MUL( m_value, rhs.QuadwordValue() );
	return *this;
}

//--------------------------------------------------------------------------------------------------
/**
	@brief			Sets current object to 'this / rhs scalar'
**/
//--------------------------------------------------------------------------------------------------

inline	FwQuat		FwQuat::operator /= ( FwScalar_arg rhs )
{
	_VEC_VALIDATE_QUADWORD( m_value );
	v128 divResult; _VEC_DIV( divResult, m_value, rhs.QuadwordValue() );
	_VEC_VALIDATE_QUADWORD( divResult );
	m_value = divResult;
	return *this;
}

//--------------------------------------------------------------------------------------------------
/**
	@brief			Returns '-this'
**/
//--------------------------------------------------------------------------------------------------

inline	FwQuat	FwQuat::operator - () const
{
	_VEC_VALIDATE_QUADWORD( m_value );
	return FwQuat( _VEC_NEG( m_value ) );
}

//--------------------------------------------------------------------------------------------------
/**
	@brief			Retrieves the X component as a scalar obect
**/
//--------------------------------------------------------------------------------------------------

inline	FwScalar	FwQuat::X( void ) const
{
	_VEC_VALIDATE_QUADWORD( m_value );
	return FwScalar( _VEC_REPLICATE_X( m_value ) );
}

//--------------------------------------------------------------------------------------------------
/**
	@brief			Retrieves the Y component as a scalar obect
**/
//--------------------------------------------------------------------------------------------------

inline	FwScalar	FwQuat::Y( void ) const
{
	_VEC_VALIDATE_QUADWORD( m_value );
	return FwScalar( _VEC_REPLICATE_Y( m_value ) );
}

//--------------------------------------------------------------------------------------------------
/**
	@brief			Retrieves the Z component as a scalar obect
**/
//--------------------------------------------------------------------------------------------------

inline	FwScalar	FwQuat::Z( void ) const
{
	_VEC_VALIDATE_QUADWORD( m_value );
	return FwScalar( _VEC_REPLICATE_Z( m_value ) );
}

//--------------------------------------------------------------------------------------------------
/**
	@brief			Retrieves the W component as a scalar obect
**/
//--------------------------------------------------------------------------------------------------

inline	FwScalar	FwQuat::W( void ) const
{
	_VEC_VALIDATE_QUADWORD( m_value );
	return FwScalar( _VEC_REPLICATE_W( m_value ) );
}

//--------------------------------------------------------------------------------------------------
/**
	@brief			Set the X component of a vector using a scalar value
**/
//--------------------------------------------------------------------------------------------------

inline	void		FwQuat::SetX( FwScalar_arg x )
{
	_VEC_VALIDATE_QUADWORD( m_value );
	_VEC_SET_X( m_value, x.QuadwordValue() );
}

//--------------------------------------------------------------------------------------------------
/**
	@brief			Set the Y component of a vector using a scalar value
**/
//--------------------------------------------------------------------------------------------------

inline	void		FwQuat::SetY( FwScalar_arg y )
{
	_VEC_VALIDATE_QUADWORD( m_value );
	_VEC_SET_Y( m_value, y.QuadwordValue() );
}

//--------------------------------------------------------------------------------------------------
/**
	@brief			Set the Z component of a vector using a scalar value
**/
//--------------------------------------------------------------------------------------------------

inline	void		FwQuat::SetZ( FwScalar_arg z )
{
	_VEC_VALIDATE_QUADWORD( m_value );
	_VEC_SET_Z( m_value, z.QuadwordValue() );
}

//--------------------------------------------------------------------------------------------------
/**
	@brief			Set the W component of a vector using a scalar value
**/
//--------------------------------------------------------------------------------------------------

inline	void		FwQuat::SetW( FwScalar_arg w )
{
	_VEC_VALIDATE_QUADWORD( m_value );
	_VEC_SET_W( m_value, w.QuadwordValue() );
}

//--------------------------------------------------------------------------------------------------
/**
	@brief			Returns the value of the element at the specified index (0=X, 1=Y, 2=Z, 3=W)

	@param			index				Index of the element (Valid values are 0, 1, 2, 3)

	@result			A scalar containing the element.
**/
//--------------------------------------------------------------------------------------------------

inline	FwScalar	FwQuat::Get( int index ) const
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
		
inline	void		FwQuat::Set( int index, FwScalar_arg val )
{
	assert( ( index >= 0 ) && ( index <= 3 ) );
	_VEC_VALIDATE_QUADWORD( m_value );
	_VEC_SET_ELEMENT( m_value, index, val.QuadwordValue() );
}

//--------------------------------------------------------------------------------------------------
/**
	@brief			Returns the value of the element at the specified index (0=X, 1=Y, 2=Z, 3=W)

	@param			index				Index of the element (Valid values are 0, 1, 2, 3)

	@result			A const scalar containing the element, the constness of which allows us to 
					error at compile time if someone attempts to do use operator [] to write a value.
**/
//--------------------------------------------------------------------------------------------------

inline	const FwScalar	FwQuat::operator []( int index ) const
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

inline	v128		FwQuat::QuadwordValue( void ) const
{
	_VEC_VALIDATE_QUADWORD( m_value );
	return m_value;
}

//--------------------------------------------------------------------------------------------------
/**
	@brief			Returns a 4 component vector
**/
//--------------------------------------------------------------------------------------------------

inline	FwVector4	FwQuat::GetVector4( void ) const
{
	return FwVector4( m_value );
}

//--------------------------------------------------------------------------------------------------
/**
	@brief			Returns an angle and axis
**/
//--------------------------------------------------------------------------------------------------

inline	void		FwQuat::GetAxisAndAngle( FwVector4& rAxis, float& rAngle ) const
{
	_VEC_VALIDATE_QUADWORD( m_value );
	rAngle	= 2.0f * atan2f( Length3( FwVector4( QuadwordValue() ) ), W() );
	v128	normResult; _VEC_SAFE_NORMALIZE3( normResult, m_value, _VEC_GET_UNIT_X() );
	rAxis	= FwVector4( normResult );
}

//--------------------------------------------------------------------------------------------------
/**
	@brief			Returns an affine transform from a rotation & translation

	@note			
**/
//--------------------------------------------------------------------------------------------------

inline	FwMatrix44	BuildTransform( FwQuat_arg rotation, FwVector4_arg translation )
{
	_VEC_VALIDATE_QUADWORD( rotation.m_value );

	vector float	xyzw_2;
	vector float	yzx0;
	vector float	zxy0;
	vector float	wwww_2;
	vector float	yzx0_2;
	vector float	zxy0_2;

	xyzw_2	= vec_add( rotation.m_value, rotation.m_value );
	yzx0	= vec_perm( rotation.m_value, rotation.m_value, _VEC_GET_SHUF_YZX() );
	zxy0	= vec_perm( rotation.m_value, rotation.m_value, _VEC_GET_SHUF_ZXY() );
	wwww_2	= _VEC_REPLICATE_W( xyzw_2 );

	yzx0_2	= vec_perm( xyzw_2, xyzw_2, _VEC_GET_SHUF_YZX() );
	zxy0_2	= vec_perm( xyzw_2, xyzw_2, _VEC_GET_SHUF_ZXY() );

	vector float	temp0	= _VEC_MUL( yzx0, wwww_2 );   
	vector float	temp1	= vec_nmsub( yzx0, yzx0_2, ( vector float )( 1.0f ) );
	vector float	temp2	= _VEC_MUL( yzx0, xyzw_2 );

	temp0	= vec_madd( zxy0, xyzw_2, temp0 );
	temp1	= vec_nmsub( zxy0, zxy0_2, temp1 );
	temp2	= vec_nmsub( zxy0, wwww_2, temp2 );

	temp0	= vec_and( temp0, _VEC_GET_MASKOFF_W() ); 
	temp1	= vec_and( temp1, _VEC_GET_MASKOFF_W() ); 
	temp2	= vec_and( temp2, _VEC_GET_MASKOFF_W() ); 

	vector float	temp3	= vec_sel( temp0, temp1, ( vector unsigned int )_VEC_GET_MASK_X() );	
	vector float	temp4	= vec_sel( temp1, temp2, ( vector unsigned int )_VEC_GET_MASK_X() );
	vector float	temp5	= vec_sel( temp2, temp0, ( vector unsigned int )_VEC_GET_MASK_X() );

	return FwMatrix44(	FwVector4( vec_sel( temp3, temp2, ( vector unsigned int )_VEC_GET_MASK_Z() ) ),
						FwVector4( vec_sel( temp4, temp0, ( vector unsigned int )_VEC_GET_MASK_Z() ) ),
						FwVector4( vec_sel( temp5, temp1, ( vector unsigned int )_VEC_GET_MASK_Z() ) ),
						FwVector4( vec_sel( translation.QuadwordValue(), _VEC_GET_UNIT_W(), ( vector unsigned int )_VEC_GET_MASK_W() ) ) );
}

//--------------------------------------------------------------------------------------------------
/**
	@brief			Returns an object containing the conjugate of the input object
**/
//--------------------------------------------------------------------------------------------------

inline	FwQuat		Conjugate( FwQuat_arg quat )
{
	_VEC_VALIDATE_QUADWORD( quat.m_value );
	return FwQuat( _VEC_XOR( quat.m_value, _VEC_GET_MASK_CONJUGATE() ) );
}

//--------------------------------------------------------------------------------------------------
/**
	@brief			Returns the 4-element dot product of input objects
**/
//--------------------------------------------------------------------------------------------------

inline	FwScalar	Dot( FwQuat_arg lhs, FwQuat_arg rhs )
{
	_VEC_VALIDATE_QUADWORD( lhs.m_value );
	_VEC_VALIDATE_QUADWORD( rhs.m_value );
	v128	dotResult; _VEC_DOT4( dotResult, lhs.m_value, rhs.m_value );
	return FwScalar( _VEC_REPLICATE_X( dotResult ) );
}

//--------------------------------------------------------------------------------------------------
/**
	@brief			Returns an object holding normalised version of input object
**/
//--------------------------------------------------------------------------------------------------

inline	FwQuat		Normalize( FwQuat_arg quat )
{
	_VEC_VALIDATE_QUADWORD( quat.m_value );
	v128	result; _VEC_NORMALIZE4( result, quat.m_value );
	return FwQuat( result );
}

//--------------------------------------------------------------------------------------------------
/**
	@brief			Returns an object holding normalised version of quaternion input object, or
					specified fallback value if 'quat' could not be successfully normalised.
**/
//--------------------------------------------------------------------------------------------------

inline	FwQuat		SafeNormalize( FwQuat_arg quat, FwQuat_arg fallback )
{
	_VEC_VALIDATE_QUADWORD( quat.m_value );
	_VEC_VALIDATE_QUADWORD( fallback.m_value );
	v128	result; _VEC_SAFE_NORMALIZE4( result, quat.m_value, fallback.m_value );
	return FwQuat( result );
}

//--------------------------------------------------------------------------------------------------
/**
	@brief			Returns an object formed by linear interpolation between two input objects
	@param			lhs				Start object
	@param			rhs				End object
	@param			alpha			Interpolation factor. 
	@note			When alpha is outside of range 0.0f to 1.0f, extrapolation occurs.
**/
//--------------------------------------------------------------------------------------------------

inline	FwQuat		Lerp( FwQuat_arg lhs, FwQuat_arg rhs, FwScalar_arg alpha )
{
	_VEC_VALIDATE_QUADWORD( lhs.m_value );
	_VEC_VALIDATE_QUADWORD( rhs.m_value );
	return FwQuat( _VEC_MADD( _VEC_SUB( rhs.m_value, lhs.m_value ), alpha.QuadwordValue(), lhs.m_value ) );
}

//--------------------------------------------------------------------------------------------------
/**
	@brief			Spherical quadrangle interpolation between two FwQuat objects
**/
//--------------------------------------------------------------------------------------------------

inline	FwQuat		Squad( FwQuat_arg unitQuat0, FwQuat_arg unitQuat1, FwQuat_arg unitQuat2, FwQuat_arg unitQuat3, float t )
{
	FwQuat tmp0 = Slerp( unitQuat0, unitQuat3, t );
	FwQuat tmp1	= Slerp( unitQuat1, unitQuat2, t );
	return Slerp( tmp0, tmp1, ( 2.0f * t ) * ( 1.0f - t ) );
}

//--------------------------------------------------------------------------------------------------
/**
	@brief			Same as 'FwQuat * FwScalar', but keeps scalar behaviour when scalar is on lhs
**/
//--------------------------------------------------------------------------------------------------

inline FwQuat	operator * ( FwScalar_arg scalar, FwQuat_arg vec )
{
	return vec * scalar;
}

#endif	//FW_QUAT_INL
