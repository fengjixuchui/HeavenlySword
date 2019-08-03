/***************************************************************************************************
*
*	$Header:: /game/vecmath.cpp 32    19/08/03 11:53 Simonb                                        $
*
*	Vector Maths
*
*	CHANGES
*
*	18/11/2002	Dean	Created
*
***************************************************************************************************/

// Static vectors - all in CVecMath scope, to avoid cluttering up vector types

const	ALIGNTO_PREFIX(16) vector unsigned int	CVecMath::m_gauiIdentityMatrix[ 4 ] ALIGNTO_POSTFIX(16)	=	{ {0x3f800000, 0x00000000, 0x00000000, 0x00000000},
																				{0x00000000, 0x3f800000, 0x00000000, 0x00000000},
																				{0x00000000, 0x00000000, 0x3f800000, 0x00000000},
																				{0x00000000, 0x00000000, 0x00000000, 0x3f800000} };

const	ALIGNTO_PREFIX(16) vector unsigned int	CVecMath::m_gauiZero ALIGNTO_POSTFIX(16)			=	{ 0x00000000, 0x00000000, 0x00000000, 0x00000000 };

const	ALIGNTO_PREFIX(16) vector unsigned int	CVecMath::m_gauiMaskOffW	ALIGNTO_POSTFIX(16)		=	{ 0xffffffff, 0xffffffff, 0xffffffff, 0x00000000 };
const	ALIGNTO_PREFIX(16) vector unsigned int	CVecMath::m_gauiOneInW ALIGNTO_POSTFIX(16)			=	{ 0x00000000, 0x00000000, 0x00000000, 0x3f800000 };
const	ALIGNTO_PREFIX(16) vector unsigned int	CVecMath::m_gauiMaskOffSign  ALIGNTO_POSTFIX(16)		= 	{ 0x7fffffff, 0x7fffffff, 0x7fffffff, 0x7fffffff };
const	ALIGNTO_PREFIX(16) vector unsigned int	CVecMath::m_gauiInvalidValue  ALIGNTO_POSTFIX(16)		= 	{ 0xffffffff, 0xffffffff, 0xffffffff, 0xffffffff };
const	ALIGNTO_PREFIX(16) vector unsigned int	CVecMath::m_gauiQuatConjugateMask  ALIGNTO_POSTFIX(16)	=	{ 0x80000000, 0x80000000, 0x80000000, 0x00000000 };
const	ALIGNTO_PREFIX(16) vector unsigned int	CVecMath::m_gauiPNPNMask ALIGNTO_POSTFIX(16)			=	{ 0x00000000, 0x80000000, 0x00000000, 0x80000000 };
const	ALIGNTO_PREFIX(16) vector unsigned int	CVecMath::m_gauiNPNPMask  ALIGNTO_POSTFIX(16)			=	{ 0x80000000, 0x00000000, 0x80000000, 0x00000000 };
// used to compute a cross product
const	ALIGNTO_PREFIX(16) vector unsigned char	CVecMath::m_vShuffleYZXX  ALIGNTO_POSTFIX(16)	= {0x04,0x05,0x06,0x07,0x08,0x09,0x0a,0x0b,0x00,0x01,0x02,0x03,0x00,0x01,0x02,0x03};
// used to compute a quat by quat mul
const	ALIGNTO_PREFIX(16) vector unsigned char	CVecMath::m_vShuffleWZYX  ALIGNTO_POSTFIX(16)	= {0x0c,0x0d,0x0e,0x0f,0x08,0x09,0x0a,0x0b,0x04,0x05,0x06,0x07,0x00,0x01,0x02,0x03};
const	ALIGNTO_PREFIX(16) vector unsigned char	CVecMath::m_vShuffleZWXY  ALIGNTO_POSTFIX(16)	= {0x08,0x09,0x0a,0x0b,0x0c,0x0d,0x0e,0x0f,0x00,0x01,0x02,0x03,0x04,0x05,0x06,0x07};
const	ALIGNTO_PREFIX(16) vector unsigned char	CVecMath::m_vShuffleYXWZ  ALIGNTO_POSTFIX(16)	= {0x04,0x05,0x06,0x07,0x00,0x01,0x02,0x03,0x0c,0x0d,0x0e,0x0f,0x08,0x09,0x0a,0x0b};
const	ALIGNTO_PREFIX(16) vector unsigned char	CVecMath::m_vShuffleWzYx  ALIGNTO_POSTFIX(16)	= {0x0c,0x0d,0x0e,0x0f,0x18,0x19,0x1a,0x1b,0x04,0x05,0x06,0x07,0x10,0x11,0x12,0x13};
const	ALIGNTO_PREFIX(16) vector unsigned char	CVecMath::m_vShuffleZWxy  ALIGNTO_POSTFIX(16)	= {0x08,0x09,0x0a,0x0b,0x0c,0x0d,0x0e,0x0f,0x10,0x11,0x12,0x13,0x14,0x15,0x16,0x17};
const	ALIGNTO_PREFIX(16) vector unsigned char	CVecMath::m_vShuffleyXWz  ALIGNTO_POSTFIX(16)	= {0x14,0x15,0x16,0x17,0x00,0x01,0x02,0x03,0x0c,0x0d,0x0e,0x0f,0x18,0x19,0x1a,0x1b};

const	ALIGNTO_PREFIX(16) vector float		CVecMath::m_vMulMaskPNPN ALIGNTO_POSTFIX(16)	=	{ 1.0f, 0.0f, 1.0f, 0.0f };
const	ALIGNTO_PREFIX(16) vector float		CVecMath::m_vMulMaskPPNN ALIGNTO_POSTFIX(16)	=	{ 1.0f, 1.0f, 0.0f, 0.0f };
const	ALIGNTO_PREFIX(16) vector float		CVecMath::m_vMulMaskNPPN ALIGNTO_POSTFIX(16)	=	{ 0.0f, 1.0f, 1.0f, 0.0f };

const	ALIGNTO_PREFIX(16) vector float		CVecMath::m_vZero ALIGNTO_POSTFIX(16)	=	{ 0.0f, 0.0f, 0.0f, 0.0f };
const	ALIGNTO_PREFIX(16) vector float		CVecMath::m_vOne  ALIGNTO_POSTFIX(16)	=	{ 1.0f, 1.0f, 1.0f, 1.0f };

#ifdef	_DEBUG_VECMATH

/***************************************************************************************************
*
*	FUNCTION		CVectorBase::CheckVector
*
*	DESCRIPTION		When enabled, this function checks the input vector for validity, trapping if
*					it contains uninitialised components.
*
*	RESULT			Function returns if all is good.. else it traps.
*
***************************************************************************************************/

void	CVectorBase::CheckVector( void ) const
{
	ntAssert(vec_all_numeric(m_vector))
}

#endif	//_DEBUG_VECMATH

/***************************************************************************************************
*
*	FUNCTION		CDirection::CDirection
*
*	DESCRIPTION		Constructs a unit length vector from spherical polar coordinates
*
***************************************************************************************************/

CDirection::CDirection( float fPhi, float fTheta )
{
	float fSinPhi, fCosPhi, fSinTheta, fCosTheta;
	CMaths::SinCos(fPhi, fSinPhi, fCosPhi);
	CMaths::SinCos(fTheta, fSinTheta, fCosTheta);
	X() = fCosPhi*fCosTheta;
	Y() = fSinTheta;
	Z() = fSinPhi*fCosTheta;
	W() = 0.0f;
};

/***************************************************************************************************
*
*	FUNCTION		CDirection::Normalise
*
*	DESCRIPTION		Normalises the directional vector in the current object.
*
*	RESULTS			If the vector of the current object has zero length, then the vector is cleared
*					to zero. In this case, the function will return 'true' indicating that this 
*					processing has occurred. In all other cases, the current vector is normalised.
*
***************************************************************************************************/

bool	CDirection::Normalise( void )
{
	ALIGNTO_PREFIX(16) vector_components fLengthSquared ALIGNTO_POSTFIX(16);
	v128	qTemp = CVectorBase::SIMDDp3( QuadwordValue(), QuadwordValue() );
	fLengthSquared.vector = qTemp;

	if ( fLengthSquared.components[0] > ( EPSILON * EPSILON ) )
	{
		qTemp = CVectorBase::SIMDRSqrt( qTemp );
		Quadword()	= vec_madd( QuadwordValue(), vec_splat(qTemp, 0), CVecMath::m_vZero );
		return false;
	}
	else
	{
		Quadword() = vec_ld(0, (v128*)&CVecMath::GetZAxis());
		return true;
	}	
}


/***************************************************************************************************
*
*	FUNCTION		CDirection::GetPerpendicular
*
*	DESCRIPTION		Returns an arbitrary normalised vector perpendicular to the current vector.
*
*	RESULTS			The perpendicular vector.
*
***************************************************************************************************/

CDirection	CDirection::GetPerpendicular( void ) const
{
	// this picks the most non-degenerate perpendicular vector from this one
	CDirection obResult( CONSTRUCT_CLEAR );
	if( fabsf( X() ) > fabsf( Y() ) )
	{
		obResult.X() = Z();
		obResult.Z() = -X();
	}
	else
	{
		obResult.Y() = Z();
		obResult.Z() = -Y();
	}
	
	// normalise it and return it
	obResult.Normalise();
	return obResult;
}


/***************************************************************************************************
*
*	FUNCTION		CDirection::CalculateRotationAxis
*
*	DESCRIPTION		Determines the rotation axis to rotate obRotateFrom to obRotateTo
*
*	RESULTS			This stores the axis and the angle from obRotateFrom to obRotateTo is returned 
*					as a result
*
***************************************************************************************************/

float	CDirection::CalculateRotationAxis( const CDirection& obRotateFrom, const CDirection& obRotateTo )
{
	// compute the rotation axis using a cross product
	*this = obRotateFrom.Cross( obRotateTo );

	// get the length squared
	float fNormSq = LengthSquared();

	// check for zero length (i.e. the from and to vectors are nearly colinear)
	if( fNormSq <= EPSILON*EPSILON )
	{
		// pick any perpendicular axis, and choose either 0 or 180 degree rotation
		*this = obRotateFrom.GetPerpendicular();

		// check for a 180 degree rotation
		return ( obRotateFrom.Dot( obRotateTo ) < 0.0f ) ? PI : 0.0f;
	}
	else
	{
		// normalise this vector
		Normalise();

		// compute the sine of the angle from the cross product rule
		float fSineAngle = fsqrtf( fNormSq / ( obRotateFrom.LengthSquared() * obRotateTo.LengthSquared() ) );
		if( fSineAngle > 1.0f )
			fSineAngle = 1.0f;

		// get angle
		float fAngle = fasinf( fSineAngle );

		// adjust the angle to deal with obtuse angles
		if( obRotateFrom.Dot( obRotateTo ) < 0.0f )
			fAngle = PI - fAngle;

		return fAngle;
	}
};


/***************************************************************************************************
*
*	FUNCTION		CQuat::CQuat( const CDirection& obAxis, float fAngle )
*
*	DESCRIPTION		Constructs a quaternion from the specified axis and angle.
*
***************************************************************************************************/

CQuat::CQuat( const CDirection& obAxis, float fAngle )
{
	float	fHalfAngle = fAngle * 0.5f;
	float	fSin;
	float	fCos;

	CMaths::SinCos( fHalfAngle, fSin, fCos );

	CDirection	obTemp = obAxis * fSin;
	Quadword()	= obTemp.QuadwordValue();
	W()			= fCos;

	Normalise();
}


/***************************************************************************************************
*
*	FUNCTION		CQuat::CQuat( const CMatrix& obMatrix )
*
*	DESCRIPTION		Constructs a quaternion from the specified rotation matrix.
*
***************************************************************************************************/

CQuat::CQuat( const CMatrix& obMatrix )
{
	float	fTrace = obMatrix[0][0] + obMatrix[1][1] + obMatrix[2][2];
	float	fS;

	// Check the diagonal

	if ( fTrace > 0.0f )
	{
		fS	= fsqrtf( fTrace + 1.0f );
		W()	= fS / 2.0f;
		fS	= 0.5f / fS;
		X()	= ( obMatrix[1][2] - obMatrix[2][1] ) * fS;
		Y()	= ( obMatrix[2][0] - obMatrix[0][2] ) * fS;
		Z()	= ( obMatrix[0][1] - obMatrix[1][0] ) * fS;
	}
	else
	{
		static const int aiNext[3] = { 1, 2, 0 };
		int	i, j, k;

		// Diagonal is negative
		i = 0;

		if ( obMatrix[1][1] > obMatrix[0][0] )
			i = 1;

		if ( obMatrix[2][2] > obMatrix[i][i] )
			i = 2;

		j = aiNext[i];
		k = aiNext[j];

		fS = fsqrtf( ( obMatrix[i][i] - ( obMatrix[j][j] + obMatrix[k][k] ) ) + 1.0f );

		Index( i )	= fS * 0.5f;

		if ( fS != 0.0f )
			fS = 0.5f / fS;

		Index( 3 )	= ( obMatrix[j][k] - obMatrix[k][j] ) * fS;
		Index( j )	= ( obMatrix[i][j] + obMatrix[j][i] ) * fS;
		Index( k )	= ( obMatrix[i][k] + obMatrix[k][i] ) * fS;
	}

	Normalise();
}


/***************************************************************************************************
*
*	FUNCTION		CQuat::GetAxisAndAngle
*
*	DESCRIPTION		Returns the angle & axis that form the current quaternion. The angle returned
*					is always in the range -PI to PI.
*
***************************************************************************************************/

void	CQuat::GetAxisAndAngle( CDirection& obAxis, float& fAngle ) const
{
	// get the axis * sin( halfangle )
	obAxis.Quadword() = QuadwordValue();

	// use tan( halfangle ) = sin / cos
	// [ therefore halfangle = atan2f( sin, cos ) = atan2f( y, x ) ]
	float fHalfAngle = atan2f( obAxis.Length(), W() );

	// because obAxis.Length() > 0 (i.e. y > 0) the halfangle is in the range [ 0, PI ]
	// be careful picking the shortest route for angle
	if( fHalfAngle > HALF_PI )
        fAngle = 2.0f*( fHalfAngle - PI );
	else 
		fAngle = 2.0f*fHalfAngle;

	// normalise the axis
	obAxis.Normalise();
}


/***************************************************************************************************
*
*	FUNCTION		CQuat::Normalise
*
*	DESCRIPTION		Normalises a quaternion.
*
*	RESULTS			If the sum of the squares of all the terms is zero, then the quaternion is 
*					initialised to (0,0,0,1). In this case, the function will return 'true'
*					indicating that this processing has occurred. In all other cases, the current
*					quaternion is normalised.
*
***************************************************************************************************/

bool	CQuat::Normalise( void )
{
	ALIGNTO_PREFIX(16) vector_components fLengthSquared ALIGNTO_POSTFIX(16);
	v128	qTemp = CVectorBase::SIMDDp4( QuadwordValue(), QuadwordValue() );
	fLengthSquared.vector = qTemp;

	if ( fLengthSquared.components[0] > ( EPSILON ) )
	{
		qTemp = CVectorBase::SIMDRSqrt( qTemp );
		Quadword()	= vec_madd( QuadwordValue(), vec_splat(qTemp, 0), CVecMath::m_vZero );
		return false;
	}
	else
	{
		Quadword() = vec_ld(0, ( v128* )&CVecMath::m_gauiOneInW);
		return true;
	}	
}

/***************************************************************************************************
*
*	FUNCTION		CQuat::IsNormalised
*
*	DESCRIPTION		
*
*	RESULTS			
*
***************************************************************************************************/
bool	CQuat::IsNormalised( void ) const
{
//	float fSize = X()*X()+Y()*Y()+Z()*Z()+W()*W();
//	return fabsf(fSize-1.0f)<0.1f;

	ALIGNTO_PREFIX( 16 ) vector_components temp ALIGNTO_POSTFIX( 16 );
	temp.vector = CVectorBase::SIMDDp4( QuadwordValue(), QuadwordValue() );
	temp.vector = vec_and( temp.vector, *( v128* )&CVecMath::m_gauiMaskOffSign );

	temp.vector = vec_sub( temp.vector, (v128){ 1.1f, 1.1f, 1.1f, 1.1f } );

	return temp.components[ 0 ] < 0.0f;
}

/***************************************************************************************************
*
*	FUNCTION		CQuat::RotationBetween
*
*	DESCRIPTION		Sets the CQuat to represent a rotation between the two specified directional
*					vectors.
*
***************************************************************************************************/

CQuat	CQuat::RotationBetween( const CDirection& obRotateFrom, const CDirection& obRotateTo )
{
	CDirection	obAxis;
	float fAngle = obAxis.CalculateRotationAxis(obRotateFrom,obRotateTo);

	return CQuat( obAxis, fAngle );
}


/***************************************************************************************************
*
*	FUNCTION		CQuat::Slerp
*
*	DESCRIPTION		Performs spherical linear interpolation between the two input quaternions. As
*					fT goes from 0.0f to 1.0f, the result interpolates between obQuat0 and obQuat1.
*
*	NOTES			This function determines the shorter arc along the 4D hypersphere to avoid
*					interpolation errors, as described in Advanced Animation & Rendering Techniques
*					by Watt & Watt, Page 365.
*
***************************************************************************************************/

CQuat	CQuat::Slerp( const CQuat& obQuat0, const CQuat& obQuat1, float fT )
{
	ntError_p( obQuat0.IsNormalised(), ("SLERP: obQuat0 is not normalised, length is %f", CVector( obQuat0 ).Length()) );
	ntError_p( obQuat1.IsNormalised(), ("SLERP: obQuat1 is not normalised, length is %f", CVector( obQuat1 ).Length()) );

	CQuat	obOutput;

	// Calculate ( obRot0 - obRot1 ) and ( obRot0 + obRot1 )
	CQuat	obTempQuat0 = obQuat0 - obQuat1;
	CQuat	obTempQuat1 = obQuat0 + obQuat1;

	// Calculate magnitude of ( obTemp0 dot obTemp1 ), and ( obTemp1 dot obTemp1 )
	float	fMag0 = obTempQuat0.Dot( obTempQuat0 );
	float	fMag1 = obTempQuat1.Dot( obTempQuat1 );

	// Determine our real target quaternion based on the differences in calculated magnitude	
	if ( fMag0 <= fMag1 )
		obTempQuat1 = obQuat1;
	else
		obTempQuat1 = -obQuat1;

	// Now do the standard slerp processing..
	float fOmega, fCosom, fSinom, fSclp, fSclq, ftOmega;

	fCosom = obQuat0.Dot( obTempQuat1 );

	if ((1.0f + fCosom) > EPSILON)
	{
		if ((1.0f - fCosom) > EPSILON)
		{
			fOmega = facosf(fCosom);
			fSinom = fsinf(fOmega);
			ftOmega = fT*fOmega;

//			fSclq = fsinf( ftOmega );
//			fSclp = fcosf( ftOmega );
			CMaths::SinCos( ftOmega, fSclq, fSclp );

			fSclq /= fSinom;
			fSclp -= fCosom*fSclq;
		}
		else
		{
			fSclp = 1.0f - fT;
			fSclq = fT;
		}

		obOutput = ( fSclp * obQuat0 ) + ( fSclq * obTempQuat1 );
	}
	else
	{
		ftOmega = fT*HALF_PI;

//		fSclq = fsinf( ftOmega );
//		fSclp = fcosf( ftOmega );
		CMaths::SinCos( ftOmega, fSclq, fSclp );

		obOutput.X() =	fSclp*obQuat0.X() - fSclq*obQuat0.Y();
		obOutput.Y() =	fSclp*obQuat0.Y() + fSclq*obQuat0.X();
		obOutput.Z() =	fSclp*obQuat0.Z() - fSclq*obQuat0.W();
		obOutput.W() =	obQuat0.Z();
	}

	ntError_p( obOutput.IsNormalised(), ("SLERP is returning non-normalised quats! Length is %f\n", CVector( obOutput ).Length()) );

	return obOutput;
}


/***************************************************************************************************
*
*	FUNCTION		CMatrix::CMatrix
*
*	DESCRIPTION		Construct a matrix from an axis and an angle
*
***************************************************************************************************/
CMatrix::CMatrix( const CDirection& obAxis, float fAngle )
{
	SetFromAxisAndAngle(obAxis,fAngle);
	Row(3).Quadword() = vec_ld(0, ( v128* )&CVecMath::m_gauiIdentityMatrix[3]);
}

/***************************************************************************************************
*
*	FUNCTION		CMatrix::CMatrix
*
*	DESCRIPTION		Construct a matrix from 4 v128 regs
*
***************************************************************************************************/
CMatrix::CMatrix( v128_arg row0,  v128_arg row1, v128_arg row2, v128_arg row3 )
{
	Row( 0 ).Quadword() = row0;
	Row( 1 ).Quadword() = row1;
	Row( 2 ).Quadword() = row2;
	Row( 3 ).Quadword() = row3;
}

/***************************************************************************************************
*
*	FUNCTION		CMatrix::SetFromAxisAndAngle
*
*	DESCRIPTION		set orientation of matrix from an axis and an angle
*
***************************************************************************************************/
void CMatrix::SetFromAxisAndAngle( const CDirection& obAxis, float fAngle )
{
	float fSin;
	float fCos;
	CDirection	obAxisSquared;
	CDirection	obAxisSin;

	// Generate sin/cos of the angle
	CMaths::SinCos( fAngle, fSin, fCos );

	// Generate a squared axis, and the axis scaled by the sine of fAngle
	obAxisSquared	= obAxis * obAxis;
	obAxisSin		= obAxis * fSin;
	
	// Construct the matrix
	Row(0)[0] = obAxisSquared.X() + ( fCos * ( 1.0f - obAxisSquared.X() ) );
	Row(1)[0] = Row(0)[1] = obAxis.X() * obAxis.Y() * ( 1.0f - fCos );
	Row(1)[0] -= obAxisSin.Z();
	Row(0)[1] += obAxisSin.Z();
	
	Row(1)[1] = obAxisSquared.Y() + ( fCos * ( 1.0f - obAxisSquared.Y() ) );
	Row(2)[0] = Row(0)[2] = obAxis.Z() * obAxis.X() * ( 1.0f - fCos );
	Row(2)[0] += obAxisSin.Y();
	Row(0)[2] -= obAxisSin.Y();

	Row(2)[2] = obAxisSquared.Z() + ( fCos * ( 1.0f - obAxisSquared.Z() ) );
	Row(2)[1] = Row(1)[2] = obAxis.Y() * obAxis.Z() * ( 1.0f - fCos );
	Row(2)[1] -= obAxisSin.X();
	Row(1)[2] += obAxisSin.X();

	Row(0)[3] = 0.0f;
	Row(1)[3] = 0.0f;
	Row(2)[3] = 0.0f;
}

/***************************************************************************************************
*
*	FUNCTION		CMatrix::CMatrix
*
*	DESCRIPTION		Construct a matrix from a quaternion rotation and translation
*
***************************************************************************************************/

#define	_VEC_REPLICATE_W( vec )				vec_splat( ( vec ), 3 )
#define	_VEC_GET_SHUF_YZX()					( ( vector unsigned char )vec_sld( vec_perm( vec_lvsl( 0, ( float* )0 ), vec_lvsl( 0, ( float* )0 ), vec_lvsr( 4, ( float* )0 ) ), vec_lvsl( 0, ( float* )0 ), 0x08 ) )
#define	_VEC_GET_SHUF_ZXY()					( ( vector unsigned char )vec_sld( vec_perm( vec_lvsl( 0, ( float* )0 ), vec_lvsl( 0, ( float* )0 ), vec_lvsr( 4, ( float* )0 ) ), vec_lvsl( 0, ( float* )0 ), 0x0c ) )

#define	_VEC_GET_MASKOFF_W()				( vec_sld( ( vector float )vec_vspltisw( -1 ), ( vector float )( 0.0f ), 0x04 ) )
#define	_VEC_GET_MASKOFF_W()				( vec_sld( ( vector float )vec_vspltisw( -1 ), ( vector float )( 0.0f ), 0x04 ) )

#define	_VEC_GET_MASK_X()					( vec_sld( ( vector float )vec_vspltisw( -1 ), ( vector float )( 0.0f ), 0x0c ) )
#define	_VEC_GET_MASK_Y()					( vec_sld( ( vector float )( 0.0f ), vec_sld( ( vector float )vec_vspltisw( -1 ), ( vector float )( 0.0f ), 0x0c ), 0x0c ) )
#define	_VEC_GET_MASK_Z()					( vec_sld( ( vector float )( 0.0f ), vec_sld( ( vector float )vec_vspltisw( -1 ), ( vector float )( 0.0f ), 0x0c ), 0x08 ) )
#define _VEC_GET_MASK_W()					( vec_sld( ( vector float )( 0.0f ), ( vector float )vec_vspltisw( -1 ), 0x04 ) )

#define	_VEC_MUL( lhs, rhs )				vec_madd( ( lhs ), ( rhs ), ( vector float )( 0.0f ) )

#define	_VEC_GET_UNIT_W()					( vec_ctf( ( vec_sld( ( vector signed int )( 0 ), vec_vspltisw( 1 ), 0x04 ) ), 0 ) )

CMatrix::CMatrix( const CQuat& obRotation, const CPoint& obTranslation ) 
{
//	SetFromQuat(obRotation);
//	SetTranslation(obTranslation);

	vector float	xyzw_2;
	vector float	yzx0;
	vector float	zxy0;
	vector float	wwww_2;
	vector float	yzx0_2;
	vector float	zxy0_2;

	xyzw_2	= vec_add( obRotation.QuadwordValue(), obRotation.QuadwordValue() );
	yzx0	= vec_perm( obRotation.QuadwordValue(), obRotation.QuadwordValue(), _VEC_GET_SHUF_YZX() );
	zxy0	= vec_perm( obRotation.QuadwordValue(), obRotation.QuadwordValue(), _VEC_GET_SHUF_ZXY() );
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

	
	Row(0).Quadword() = vec_sel( temp3, temp2, ( vector unsigned int )_VEC_GET_MASK_Z() );
	Row(1).Quadword() = vec_sel( temp4, temp0, ( vector unsigned int )_VEC_GET_MASK_Z() );
	Row(2).Quadword() = vec_sel( temp5, temp1, ( vector unsigned int )_VEC_GET_MASK_Z() );
	Row(3).Quadword() = vec_sel( obTranslation.QuadwordValue(), _VEC_GET_UNIT_W(), ( vector unsigned int )_VEC_GET_MASK_W() );

}

/***************************************************************************************************
*
*	FUNCTION		CMatrix::SetFromQuat
*
*	DESCRIPTION		Set rotation component only from a quat
*
***************************************************************************************************/
void CMatrix::SetFromQuat( const CQuat& obRotation )
{
	// TODO: This is going to need a hand-coded assembly version

//#define	EXPERIMENTAL_CODE

#ifdef	EXPERIMENTAL_CODE
	CVector	obRot2;
	obRot2.Quadword() = obRotation.QuadwordValue();
	obRot2 += obRot2;

	CVector obRotSwiz;
	obRotSwiz.Quadword() = _mm_shuffle_ps( obRotation.QuadwordValue(), obRotation.QuadwordValue(), _MM_SSE_SHUFFLE( 1, 2, 0, 3) );
	obRotSwiz *= obRot2;

	CVector obRot2W;
	obRot2W = obRot2 * obRotation.W();

	CVector	obRot2Rot;
	obRot2Rot.Quadword() = obRotation.QuadwordValue();
	obRot2Rot *= obRot2;

	Row(0)[0] = 1.0f - ( obRot2Rot.Y() ) - ( obRot2Rot.Z() );			//	Row(0)[0] = 1.0f - ( fY2 * obRotation.Y() ) - ( fZ2 * obRotation.Z() );			
	Row(0)[1] = ( obRotSwiz.X() ) + ( obRot2W.Z() );					//	Row(0)[1] = ( fX2 * obRotation.Y() ) + ( obRotation.W() * fZ2 );
	Row(0)[2] = ( obRotSwiz.Z() ) - ( obRot2W.Y() );					//	Row(0)[2] = ( fZ2 * obRotation.X() ) - ( obRotation.W() * fY2 );
	Row(0)[3] = 0.0f;													
																			
	Row(1)[0] = ( obRotSwiz.X() ) - ( obRot2W.Z() );					//	Row(1)[0] = ( fX2 * obRotation.Y() ) - ( obRotation.W() * fZ2 );
	Row(1)[1] = 1.0f - ( obRot2Rot.X() ) - ( obRot2Rot.Z() );			//	Row(1)[1] = 1.0f - ( fX2 * obRotation.X() ) - ( fZ2 * obRotation.Z() );
	Row(1)[2] = ( obRotSwiz.Y() ) + ( obRot2W.X() );					//	Row(1)[2] = ( fY2 * obRotation.Z() ) + ( obRotation.W() * fX2 );
	Row(1)[3] = 0.0f;													
																		
	Row(2)[0] = ( obRotSwiz.Z() ) + ( obRot2W.Y() );					//	Row(2)[0] = ( fZ2 * obRotation.X() ) + ( obRotation.W() * fY2 )
	Row(2)[1] = ( obRotSwiz.Y() ) - ( obRot2W.X() );					//	Row(2)[1] = ( fY2 * obRotation.Z() ) - ( obRotation.W() * fX2 )
	Row(2)[2] = 1.0f - ( obRot2Rot.X() ) - ( obRot2Rot.Y() );			//	Row(2)[2] = 1.0f - ( fX2 * obRotation.X() ) - ( fY2 * obRotation.Y() )
	Row(2)[3] = 0.0f;													

#else

	float fX2 = obRotation.X() + obRotation.X();
	float fY2 = obRotation.Y() + obRotation.Y();
	float fZ2 = obRotation.Z() + obRotation.Z();
	
	// fill in the non-diagonal entries first

	Row(0)[1] = Row(1)[0] = fX2 * obRotation.Y();	// Generate obRot2 * swizzled obRotation
	Row(1)[2] = Row(2)[1] = fY2 * obRotation.Z();
	Row(2)[0] = Row(0)[2] = fZ2 * obRotation.X();
	
	float t = obRotation.W() * fX2;					// Generate W() * obRot2 and use
	Row(1)[2] += t; Row(2)[1] -= t;
	t = obRotation.W() * fY2;
	Row(2)[0] += t; Row(0)[2] -= t;
	t = obRotation.W() * fZ2;
	Row(0)[1] += t; Row(1)[0] -= t;
	
	// fill in the diagonals

	Row(0)[0] = Row(1)[1] = Row(2)[2] = 1.0f;
	t = fX2 * obRotation.X();						// Generate obRot2 * obRotation
	Row(1)[1] -= t; Row(2)[2] -= t;
	t = fY2 * obRotation.Y();
	Row(0)[0] -= t; Row(2)[2] -= t;
	t = fZ2 * obRotation.Z();
	Row(0)[0] -= t; Row(1)[1] -= t;
	
	// fill in the rest
	Row(0)[3] = Row(1)[3] = Row(2)[3] = 0.0f;
#endif
}



/***************************************************************************************************
*
*	FUNCTION		CMatrix::GetTranspose
*
*	DESCRIPTION		Returns the transpose of this matrix as a new CMatrix object.
*
***************************************************************************************************/

CMatrix	CMatrix::GetTranspose( void ) const
{
	CMatrix obOutput;

	v128_arg vrow1 = Row( 0 ).QuadwordValue();
	v128_arg vrow2 = Row( 1 ).QuadwordValue();
	v128_arg vrow3 = Row( 2 ).QuadwordValue();
	v128_arg vrow4 = Row( 3 ).QuadwordValue();

	v128 vtemp1 = vec_mergel( vrow1, vrow3 );
	v128 vtemp2 = vec_mergeh( vrow1, vrow3 );
	v128 vtemp3 = vec_mergel( vrow2, vrow4 );
	v128 vtemp4 = vec_mergeh( vrow2, vrow4 );

	obOutput.Row( 0 ).Quadword() = vec_mergeh( vtemp2, vtemp4 );
	obOutput.Row( 1 ).Quadword() = vec_mergel( vtemp2, vtemp4 );
	obOutput.Row( 2 ).Quadword() = vec_mergeh( vtemp1, vtemp3 );
	obOutput.Row( 3 ).Quadword() = vec_mergel( vtemp1, vtemp3 );

	return obOutput;
}


/***************************************************************************************************
*
*	FUNCTION		CMatrix::operator *=
*
*	DESCRIPTION		Multiply this matrix by obMatrix, storing the result in this matrix.
*
*	RESULT			this = this * obMatrix;
*
***************************************************************************************************/

CMatrix&	CMatrix::operator *= ( const CMatrix& obMatrix )
{
	Row( 0 ) = Row( 0 ) * obMatrix;
	Row( 1 ) = Row( 1 ) * obMatrix;
	Row( 2 ) = Row( 2 ) * obMatrix;
	Row( 3 ) = Row( 3 ) * obMatrix;
	return *this;
}

/***************************************************************************************************
*
*	FUNCTION		operator *
*
*	DESCRIPTION		Multiply obMatrixL by obMatrixR, returning the result in a new matrix.
*
*	RESULT			new matrix = obMatrixL * obMatrixR
*
*	NOTES			This is intentionally in global scope.
*
***************************************************************************************************/

CMatrix		operator *	( const CMatrix& obMatrixL, const CMatrix& obMatrixR )
{
/*	return CMatrix( 
        obMatrixL.Row( 0 ) * obMatrixR,
        obMatrixL.Row( 1 ) * obMatrixR,
        obMatrixL.Row( 2 ) * obMatrixR,
        obMatrixL.Row( 3 ) * obMatrixR ); */

	vector float resultA, resultB, resultC, resultD;
	vector float vRowR0 = obMatrixR.Row( 0 ).QuadwordValue();
	vector float vRowR1 = obMatrixR.Row( 1 ).QuadwordValue();
	vector float vRowR2 = obMatrixR.Row( 2 ).QuadwordValue();
	vector float vRowR3 = obMatrixR.Row( 3 ).QuadwordValue();

	vector float vRowL0 = obMatrixL.Row( 0 ).QuadwordValue();
	vector float vRowL1 = obMatrixL.Row( 1 ).QuadwordValue();
	vector float vRowL2 = obMatrixL.Row( 2 ).QuadwordValue();
	vector float vRowL3 = obMatrixL.Row( 3 ).QuadwordValue();

	resultA = vec_madd( vec_splat( vRowL0, 0 ), vRowR0, CVecMath::m_vZero );
	resultA = vec_madd( vec_splat( vRowL0, 1 ), vRowR1, resultA );
	resultA = vec_madd( vec_splat( vRowL0, 2 ), vRowR2, resultA );
	resultA = vec_madd( vec_splat( vRowL0, 3 ), vRowR3, resultA );

	resultB = vec_madd( vec_splat( vRowL1, 0 ), vRowR0, CVecMath::m_vZero );
	resultB = vec_madd( vec_splat( vRowL1, 1 ), vRowR1, resultB );
	resultB = vec_madd( vec_splat( vRowL1, 2 ), vRowR2, resultB );
	resultB = vec_madd( vec_splat( vRowL1, 3 ), vRowR3, resultB );

	resultC = vec_madd( vec_splat( vRowL2, 0 ), vRowR0, CVecMath::m_vZero );
	resultC = vec_madd( vec_splat( vRowL2, 1 ), vRowR1, resultC );
	resultC = vec_madd( vec_splat( vRowL2, 2 ), vRowR2, resultC );
	resultC = vec_madd( vec_splat( vRowL2, 3 ), vRowR3, resultC );

	resultD = vec_madd( vec_splat( vRowL3, 0 ), vRowR0, CVecMath::m_vZero );
	resultD = vec_madd( vec_splat( vRowL3, 1 ), vRowR1, resultD );
	resultD = vec_madd( vec_splat( vRowL3, 2 ), vRowR2, resultD );
	resultD = vec_madd( vec_splat( vRowL3, 3 ), vRowR3, resultD );

	return CMatrix( resultA, resultB, resultC, resultD );
}

/***************************************************************************************************
*
*	FUNCTION		CMatrix::GetAffineInverse
*
*	DESCRIPTION		Returns a new matrix containing the affine inverse of the current matrix
*
***************************************************************************************************/

CMatrix	CMatrix::GetAffineInverse( void ) const
{
/*	CMatrix obOutput;

	// this nice inverse function is from ATG guys
	v128_arg vrow0 = Row( 0 ).QuadwordValue();
	v128_arg vrow1 = Row( 1 ).QuadwordValue();
	v128_arg vrow2 = Row( 2 ).QuadwordValue();
	v128_arg vrow3 = Row( 3 ).QuadwordValue();

	v128	inv2	= CVectorBase::SIMDCross( vrow0, vrow1 );
	v128	inv0	= CVectorBase::SIMDCross( vrow1, vrow2 );
	v128	inv1	= CVectorBase::SIMDCross( vrow2, vrow0 );
	v128	inv3	= vec_sub( CVecMath::m_vZero, vrow3 );
	
	v128	dotVec	= CVectorBase::SIMDDp3( inv2, vrow2 );
	v128	invDet	= CVectorBase::SIMDRcp( vec_splat( dotVec, 0 ) );

	// Transpose elements..
	v128	temp0	= vec_mergeh( inv0, inv2 );
	v128	temp1	= vec_mergeh( inv1, (v128)( 0.0f ) );
	v128	temp2	= vec_mergel( inv0, inv2 );
	v128	temp3	= vec_mergel( inv1, (v128)( 0.0f ) );

	inv0 = vec_mergeh( temp0, temp1 );
	inv1 = vec_mergel( temp0, temp1 );
	inv2 = vec_mergeh( temp2, temp3 );

	v128	inv3Temp = inv3;

	inv3	= vec_madd( inv0, vec_splat( inv3Temp, 0), CVecMath::m_vZero );
	inv3	= vec_madd( inv1, vec_splat( inv3Temp, 1 ), inv3 );
	inv3	= vec_madd( inv2, vec_splat( inv3Temp, 2 ), inv3 );

	obOutput.Row( 0 ).Quadword() = vec_madd( inv0, invDet, CVecMath::m_vZero  );
	obOutput.Row( 1 ).Quadword() = vec_madd( inv1, invDet, CVecMath::m_vZero  );
	obOutput.Row( 2 ).Quadword() = vec_madd( inv2, invDet, CVecMath::m_vZero  );
	obOutput.Row( 3 ).Quadword() = vec_madd( inv3, invDet, CVecMath::m_vZero  );

	return obOutput;*/

	CMatrix obOutput;

	v128_arg vrow1 = Row(0).QuadwordValue();
	v128_arg vrow2 = Row(1).QuadwordValue();
	v128_arg vrow3 = Row(2).QuadwordValue();
	v128_arg vrow4 = Row(3).QuadwordValue();

	v128 vtemp1 = vec_mergel( vrow1, vrow3 );
	v128 vtemp2 = vec_mergeh( vrow1, vrow3 );
	v128 vtemp3 = vec_mergel( vrow2, vrow4 );
	v128 vtemp4 = vec_mergeh( vrow2, vrow4 );

	obOutput.Row(0).Quadword() = vec_mergeh( vtemp2, vtemp4 );
	obOutput.Row(1).Quadword() = vec_mergel( vtemp2, vtemp4 );
	obOutput.Row(2).Quadword() = vec_mergeh( vtemp1, vtemp3 );

	obOutput.Row(0).Quadword() = vec_and (obOutput.Row(0).QuadwordValue(), *( v128* )&CVecMath::m_gauiMaskOffW);
	obOutput.Row(1).Quadword() = vec_and (obOutput.Row(1).QuadwordValue(), *( v128* )&CVecMath::m_gauiMaskOffW);
	obOutput.Row(2).Quadword() = vec_and (obOutput.Row(2).QuadwordValue(), *( v128* )&CVecMath::m_gauiMaskOffW);


	CPoint	obTranslation = -GetTranslation();
	v128 result;

	result = vec_madd( vec_splat(obTranslation.QuadwordValue(), 0), obOutput.Row(0).QuadwordValue(), CVecMath::m_vZero );
	result = vec_madd( vec_splat(obTranslation.QuadwordValue(), 1), obOutput.Row(1).QuadwordValue(), result );
	result = vec_madd( vec_splat(obTranslation.QuadwordValue(), 2), obOutput.Row(2).QuadwordValue(), result );

	obOutput.Row(3).Quadword() = vec_or( vec_and( result, *(v128* )&CVecMath::m_gauiMaskOffW ), *( v128* )&CVecMath::m_gauiOneInW );

	return obOutput;
}


/***************************************************************************************************
*
*	FUNCTION		CMatrix::GetFullInverse
*
*	DESCRIPTION		Returns a new matrix containing the full inverse of the current 4x4 matrix. This
*					code was taken from a sample by Intel Architecture Labs, made available on both 
*					their developer support site and as an part of an source package that went with
*					their Gamasutra article.
*
*	NOTES			If it's crap/bugged, it's not my fault. The implementation seems pretty cool to
*					me though...
*
***************************************************************************************************/

// If the standard SSE shuffle parameter macro isn't there, define it..
#ifndef	_MM_SHUFFLE
#define _MM_SHUFFLE(fp3,fp2,fp1,fp0) (((fp3) << 6) | ((fp2) << 4) | ((fp1) << 2) | ((fp0)))
#endif	//_MM_SHUFFLE

// And here are some SIMD macros for rotating quadwords
#define _mm_ror_ps(vec,i)	\
	(((i)%4) ? (_mm_shuffle_ps(vec,vec, _MM_SHUFFLE((unsigned char)(i+3)%4,(unsigned char)(i+2)%4,(unsigned char)(i+1)%4,(unsigned char)(i+0)%4))) : (vec))
#define _mm_rol_ps(vec,i)	\
	(((i)%4) ? (_mm_shuffle_ps(vec,vec, _MM_SHUFFLE((unsigned char)(7-i)%4,(unsigned char)(6-i)%4,(unsigned char)(5-i)%4,(unsigned char)(4-i)%4))) : (vec))


CMatrix	CMatrix::GetFullInverse( void ) const
{
	// this code is still wrapped around SSE emul..but nonetheless it's using PPU's VMX unit 
	CMatrix	obMinterms;
	__m128	qVa;
	__m128	qVb;
	__m128	qVc;
	__m128	qTemp1;
	__m128	qTemp2;
	__m128  qTemp3;
	__m128	qR1;
	__m128	qR2;
	__m128	qR3;
	__m128	qSum;
	__m128	qDet;
	__m128	qRDet;


	// Calculate minterms for the first line

	qTemp1.vReg = Row( 3 ).QuadwordValue();
	qTemp3.vReg = Row( 2 ).QuadwordValue();
	qTemp2 = _mm_ror_ps( qTemp3, 1 );
	qVa	= _mm_mul_ps( qTemp2, _mm_ror_ps( qTemp1, 2 ) );	// V3'·V4"
	qVb	= _mm_mul_ps( qTemp2, _mm_ror_ps( qTemp1, 3 ) );	// V3'·V4^
	qVc	= _mm_mul_ps( qTemp2, _mm_ror_ps( qTemp1, 0 ) );	// V3'·V4
	
	qR1 = _mm_sub_ps( _mm_ror_ps( qVa, 1 ), _mm_ror_ps( qVc, 2 ) );		// V3"·V4^ - V3^·V4"	
	qR2 = _mm_sub_ps( _mm_ror_ps( qVb, 2 ), _mm_ror_ps( qVb, 0 ) );		// V3^·V4' - V3'·V4^
	qR3 = _mm_sub_ps( _mm_ror_ps( qVa, 0 ), _mm_ror_ps( qVc, 1 ) );		// V3'·V4" - V3"·V4'

	qTemp1.vReg = Row( 1 ).QuadwordValue();
	qVa = _mm_ror_ps( qTemp1, 1 );	qSum = _mm_mul_ps( qVa, qR1 );
	qVb = _mm_ror_ps( qTemp1, 2 );	qSum = _mm_add_ps( qSum, _mm_mul_ps( qVb, qR2 ) );
	qVc = _mm_ror_ps( qTemp1, 3 );	qSum = _mm_add_ps( qSum, _mm_mul_ps( qVc, qR3 ) );

	// Calculating the determinant
	qTemp3.vReg = Row( 0 ).QuadwordValue();
	qDet = _mm_mul_ps( qSum, qTemp3 );
	qDet = _mm_add_ps( qDet, _mm_movehl_ps( qDet, qDet ) );
	qTemp3.vReg = *( v128* )&CVecMath::m_gauiPNPNMask;
	obMinterms.Row( 0 ).Quadword() = (_mm_xor_ps( qSum, qTemp3 )).vReg;

	// Calculating the minterms of the second line (using previous results)
	
	qTemp3.vReg = Row( 0 ).QuadwordValue();
	qTemp1 = _mm_ror_ps( qTemp3, 1 );	qSum = _mm_mul_ps( qTemp1, qR1 );
	qTemp1 = _mm_ror_ps( qTemp1, 1 );					qSum = _mm_add_ps( qSum, _mm_mul_ps( qTemp1, qR2 ) );
	qTemp1 = _mm_ror_ps( qTemp1, 1 );					qSum = _mm_add_ps( qSum, _mm_mul_ps( qTemp1, qR3 ) );
	qTemp3.vReg = *( v128* )&CVecMath::m_gauiNPNPMask;
	obMinterms.Row( 1 ).Quadword() = (_mm_xor_ps( qSum, qTemp3 )).vReg;

	// Testing the determinant
	qDet = _mm_sub_ss( qDet, _mm_shuffle_ps( qDet, qDet, 1 ) );
	int	iFlag = _mm_comieq_ss( qDet, _mm_sub_ss( qTemp1, qTemp1 ) );

	// Calculating the minterms of the third line

	qTemp3.vReg = Row( 0 ).QuadwordValue();
	qTemp1 = _mm_ror_ps( qTemp3, 1 );
	qVa = _mm_mul_ps( qTemp1, qVb );									// V1'·V2"
	qVb = _mm_mul_ps( qTemp1, qVc );									// V1'·V2^
	qTemp3.vReg = Row( 1 ).QuadwordValue();
	qVc = _mm_mul_ps( qTemp1, qTemp3);				// V1'·V2

	qR1 = _mm_sub_ps( _mm_ror_ps( qVa, 1 ), _mm_ror_ps( qVc, 2 ) );		// V1"·V2^ - V1^·V2"
	qR2 = _mm_sub_ps( _mm_ror_ps( qVb, 2 ), _mm_ror_ps( qVb, 0 ) );		// V1^·V2' - V1'·V2^
	qR3 = _mm_sub_ps( _mm_ror_ps( qVa, 0 ), _mm_ror_ps( qVc, 1 ) );		// V1'·V2" - V1"·V2'

	qTemp3.vReg = Row( 3 ).QuadwordValue();
	qTemp1 = _mm_ror_ps( qTemp3, 1 );	qSum = _mm_mul_ps( qTemp1, qR1 );
	qTemp1 = _mm_ror_ps( qTemp1, 1 );					qSum = _mm_add_ps( qSum, _mm_mul_ps( qTemp1, qR2 ) );
	qTemp1 = _mm_ror_ps( qTemp1, 1 );					qSum = _mm_add_ps( qSum, _mm_mul_ps( qTemp1, qR3 ) );
	qTemp3.vReg = *( v128* )&CVecMath::m_gauiPNPNMask;
	obMinterms.Row( 2 ).Quadword() =  (_mm_xor_ps( qSum, qTemp3 )).vReg;

	// Dividing is FASTER than rcp_nr! (Because rcp_nr causes many register-memory RWs).
	qRDet = _mm_div_ss( _mm_set_ss( 1.0f ), qDet );
	qRDet = _mm_shuffle_ps( qRDet, qRDet, 0x00 );

	// Divide the first 12 minterms with the determinant.
	qTemp3.vReg = obMinterms.Row( 0 ).QuadwordValue();
	obMinterms.Row( 0 ).Quadword() = (_mm_mul_ps( qTemp3, qRDet )).vReg;
	qTemp3.vReg = obMinterms.Row( 1 ).QuadwordValue();
	obMinterms.Row( 1 ).Quadword() = (_mm_mul_ps( qTemp3, qRDet )).vReg;
	qTemp3.vReg = obMinterms.Row( 2 ).QuadwordValue();
	obMinterms.Row( 2 ).Quadword() = (_mm_mul_ps( qTemp3, qRDet )).vReg;

	// Calculate the minterms of the fourth line and divide by the determinant.
	qTemp3.vReg = Row( 2 ).QuadwordValue();
	qTemp1 = _mm_ror_ps( qTemp3, 1 );	qSum = _mm_mul_ps( qTemp1, qR1 );
	qTemp1 = _mm_ror_ps( qTemp1, 1 );					qSum = _mm_add_ps( qSum, _mm_mul_ps( qTemp1, qR2 ) );
	qTemp1 = _mm_ror_ps( qTemp1, 1 );					qSum = _mm_add_ps( qSum, _mm_mul_ps( qTemp1, qR3 ) );
	qTemp3.vReg = *( v128* )&CVecMath::m_gauiNPNPMask;
	obMinterms.Row( 3 ).Quadword() = (_mm_xor_ps( qSum, qTemp3 )).vReg;
	qTemp3.vReg = obMinterms.Row( 3 ).QuadwordValue();
	obMinterms.Row( 3 ).Quadword() = (_mm_mul_ps(qTemp3, qRDet )).vReg;

	// Check if the matrix is inversable.
	// Uses a delayed branch here, so the test would not interfere the calculations.
	// Assuming most of the matrices are inversable, the previous calculations are  not wasted.
	// It is faster this way.

	CMatrix	obOutput;

	if ( iFlag )
	{
		obOutput.Clear();
	}
	else
	{
		__m128	qTrns0;
		__m128	qTrns1;
		__m128	qTrns2;
		__m128	qTrns3;

		// Now we just have to transpose the minterms matrix.
		qTemp1.vReg = obMinterms.Row( 0 ).QuadwordValue();
		qTemp2.vReg = obMinterms.Row( 1 ).QuadwordValue();
		qTrns0 = _mm_unpacklo_ps( qTemp1, qTemp2 );
		qTemp1.vReg = obMinterms.Row( 2 ).QuadwordValue();
		qTemp2.vReg = obMinterms.Row( 3 ).QuadwordValue();
		qTrns1 = _mm_unpacklo_ps( qTemp1, qTemp2 );
		qTemp1.vReg = obMinterms.Row( 0 ).QuadwordValue();
		qTemp2.vReg = obMinterms.Row( 1 ).QuadwordValue();
		qTrns2 = _mm_unpackhi_ps( qTemp1, qTemp2 );
		qTemp1.vReg = obMinterms.Row( 2 ).QuadwordValue();
		qTemp2.vReg = obMinterms.Row( 3 ).QuadwordValue();
		qTrns3 = _mm_unpackhi_ps( qTemp1, qTemp2 );
		obOutput.Row( 0 ).Quadword() = (_mm_movelh_ps( qTrns0, qTrns1 )).vReg;
		obOutput.Row( 1 ).Quadword() = (_mm_movehl_ps( qTrns1, qTrns0 )).vReg;
		obOutput.Row( 2 ).Quadword() = (_mm_movelh_ps( qTrns2, qTrns3 )).vReg;
		obOutput.Row( 3 ).Quadword() = (_mm_movehl_ps( qTrns3, qTrns2 )).vReg;
	}
	
	return obOutput;
}

/***************************************************************************************************
*
*	FUNCTION		CMatrix::Perspective
*
*	DESCRIPTION		Setup symmetric perspective projection matrix
*
***************************************************************************************************/

void	CMatrix::Perspective( float fFieldOfViewY, float fAspectRatio, float fNear, float fFar )
{
	float fRcpTanFOV = 1.0f / ftanf( fFieldOfViewY / 2.0f );
	float fQ = fFar / ( fFar - fNear );

	// diag(-1, 1, 1, 1) * normal projection matrix
	Row( 0 ) = CVector( -fRcpTanFOV / fAspectRatio, 0.0f, 0.0f, 0.0f );
	Row( 1 ) = CVector( 0.0f, fRcpTanFOV, 0.0f, 0.0f );
	Row( 2 ) = CVector( 0.0f, 0.0f, fQ, 1.0f ); 
	Row( 3 ) = CVector( 0.0f, 0.0f, -fNear * fQ, 0.0f );
}

/***************************************************************************************************
*
*	FUNCTION		CMatrix::Perspective
*
*	DESCRIPTION		Setup symmetric perspective projection matrix (based on D3DXMatrixPerspectiveLH) with x flipped
*
***************************************************************************************************/

void	CMatrix::Perspective2( float fWidth, float fHeight, float fNear, float fFar )
{
	// diag(-1, 1, 1, 1) * normal projection matrix
	Row( 0 ) = CVector( -(2*fNear)/fWidth, 0.f, 0.f, 0.f);
	Row( 1 ) = CVector( 0.f, (2*fNear)/fHeight, 0.f, 0.f);
	Row( 2 ) = CVector( 0.0f, 0.0f, fFar / ( fFar - fNear ), 1.0f ); 
	Row( 3 ) = CVector( 0.0f, 0.0f, fNear*fFar / ( fNear - fFar ), 0.0f );
}
