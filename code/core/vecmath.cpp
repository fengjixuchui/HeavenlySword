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

const	ALIGNTO_PREFIX(16) uint32_t	CVecMath::m_gauiIdentityMatrix[ 16 ] ALIGNTO_POSTFIX(16)	=	{ 0x3f800000, 0x00000000, 0x00000000, 0x00000000,
																		  0x00000000, 0x3f800000, 0x00000000, 0x00000000,
																		  0x00000000, 0x00000000, 0x3f800000, 0x00000000,
																		  0x00000000, 0x00000000, 0x00000000, 0x3f800000 };

const	ALIGNTO_PREFIX(16) uint32_t	CVecMath::m_gauiZero[ 4 ]	ALIGNTO_POSTFIX(16)			=	{ 0x00000000, 0x00000000, 0x00000000, 0x00000000 };

const	ALIGNTO_PREFIX(16)	uint32_t	CVecMath::m_gauiMaskOffW[ 4 ]	ALIGNTO_POSTFIX(16)		=	{ 0xffffffff, 0xffffffff, 0xffffffff, 0x00000000 };
const	ALIGNTO_PREFIX(16) uint32_t	CVecMath::m_gauiOneInW[ 4 ]	ALIGNTO_POSTFIX(16)			=	{ 0x00000000, 0x00000000, 0x00000000, 0x3f800000 };
const	ALIGNTO_PREFIX(16) uint32_t	CVecMath::m_gauiMaskOffSign[ 4 ] ALIGNTO_POSTFIX(16)		= 	{ 0x7fffffff, 0x7fffffff, 0x7fffffff, 0x7fffffff };
const	ALIGNTO_PREFIX(16) uint32_t	CVecMath::m_gauiInvalidValue[ 4 ] ALIGNTO_POSTFIX(16)		= 	{ 0xffffffff, 0xffffffff, 0xffffffff, 0xffffffff };
const	ALIGNTO_PREFIX(16)	uint32_t	CVecMath::m_gauiQuatConjugateMask[ 4 ] ALIGNTO_POSTFIX(16)	=	{ 0x80000000, 0x80000000, 0x80000000, 0x00000000 };
const	ALIGNTO_PREFIX(16) uint32_t	CVecMath::m_gauiPNPNMask[ 4 ] ALIGNTO_POSTFIX(16)			=	{ 0x00000000, 0x80000000, 0x00000000, 0x80000000 };
const	ALIGNTO_PREFIX(16) uint32_t	CVecMath::m_gauiNPNPMask[ 4 ] ALIGNTO_POSTFIX(16)			=	{ 0x80000000, 0x00000000, 0x80000000, 0x00000000 };


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
	const float*	pfElements = ( const float* )this;
	for ( int iLoop = 0; iLoop < 4; iLoop++ )
	{
		ntAssert( _finite( pfElements[ iLoop ] ) );
	}
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
	float	fLengthSquared;
	__m128	qTemp = CVectorBase::SIMDDp3( QuadwordValue(), QuadwordValue() );
	_mm_store_ss( &fLengthSquared, qTemp );

	if ( fLengthSquared > ( EPSILON * EPSILON ) )
	{
		qTemp = _mm_rsqrt_ss( qTemp );
		qTemp = _mm_shuffle_ps( qTemp, qTemp, _MM_SSE_SHUFFLE( 0, 0, 0, 0 ) );
		Quadword()	= _mm_mul_ps( QuadwordValue(), qTemp );
		return false;
	}
	else
	{
		Quadword() = CVecMath::GetZAxis().QuadwordValue();
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
	float	fLengthSquared;
	__m128	qTemp = CVectorBase::SIMDDp4( QuadwordValue(), QuadwordValue() );
	_mm_store_ss( &fLengthSquared, qTemp );

	if ( fLengthSquared > EPSILON )
	{
		qTemp = _mm_rsqrt_ss( qTemp );
		qTemp = _mm_shuffle_ps( qTemp, qTemp, _MM_SSE_SHUFFLE( 0, 0, 0, 0 ) );
		Quadword()	= _mm_mul_ps( QuadwordValue(), qTemp );
		return false;
	}
	else
	{
		Quadword() = _mm_load_ps( ( const float* )CVecMath::m_gauiOneInW );
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
	float fSize = X()*X()+Y()*Y()+Z()*Z()+W()*W();
	return fabsf(fSize-1.0f)<0.1f;
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
	Row(3) = CVecMath::GetIdentity().Row(3);
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
CMatrix::CMatrix( const CQuat& obRotation, const CPoint& obTranslation )
{
	SetFromQuat(obRotation);
	SetTranslation(obTranslation);
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
*	DESCRIPTION		Returns the transpose of this matrix as a NEW CMatrix object.
*
***************************************************************************************************/

CMatrix	CMatrix::GetTranspose( void ) const
{
	// build the intermediate matrix:
	//		e00, e01, e10, e11
	//		e20, e21, e30, e31
	//		e02, e03, e12, e13
	//		e22, e23, e32, e33

	__m128 xmmTemp0 = _mm_shuffle_ps( Row( 0 ).QuadwordValue(), Row( 1 ).QuadwordValue(), _MM_SSE_SHUFFLE( 0, 1, 0, 1 ) );
	__m128 xmmTemp1 = _mm_shuffle_ps( Row( 2 ).QuadwordValue(), Row( 3 ).QuadwordValue(), _MM_SSE_SHUFFLE( 0, 1, 0, 1 ) );
	__m128 xmmTemp2 = _mm_shuffle_ps( Row( 0 ).QuadwordValue(), Row( 1 ).QuadwordValue(), _MM_SSE_SHUFFLE( 2, 3, 2, 3 ) );
	__m128 xmmTemp3 = _mm_shuffle_ps( Row( 2 ).QuadwordValue(), Row( 3 ).QuadwordValue(), _MM_SSE_SHUFFLE( 2, 3, 2, 3 ) );

	// return the matrix:
	//		e00, e10, e20, e30
	//		e01, e11, e21, e31
	//		e02, e12, e22, e32
	//		e03, e13, e23, e33
	
	CMatrix	obOutput;

	obOutput.Row( 0 ).Quadword() = _mm_shuffle_ps( xmmTemp0, xmmTemp1, _MM_SSE_SHUFFLE( 0, 2, 0, 2 ) );
	obOutput.Row( 1 ).Quadword() = _mm_shuffle_ps( xmmTemp0, xmmTemp1, _MM_SSE_SHUFFLE( 1, 3, 1, 3 ) );
	obOutput.Row( 2 ).Quadword() = _mm_shuffle_ps( xmmTemp2, xmmTemp3, _MM_SSE_SHUFFLE( 0, 2, 0, 2 ) );
	obOutput.Row( 3 ).Quadword() = _mm_shuffle_ps( xmmTemp2, xmmTemp3, _MM_SSE_SHUFFLE( 1, 3, 1, 3 ) );

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
*	DESCRIPTION		Multiply obMatrixL by obMatrixR, returning the result in a NEW matrix.
*
*	RESULT			NEW matrix = obMatrixL * obMatrixR
*
*	NOTES			This is intentionally in global scope.
*
***************************************************************************************************/

CMatrix		operator *	( const CMatrix& obMatrixL, const CMatrix& obMatrixR )
{
	CMatrix	obTemp;

	obTemp.Row( 0 ) = obMatrixL.Row( 0 ) * obMatrixR;
	obTemp.Row( 1 ) = obMatrixL.Row( 1 ) * obMatrixR;
	obTemp.Row( 2 ) = obMatrixL.Row( 2 ) * obMatrixR;
	obTemp.Row( 3 ) = obMatrixL.Row( 3 ) * obMatrixR;

	return obTemp;
}


/***************************************************************************************************
*
*	FUNCTION		operator *	( const CVector& obVector, const CMatrix& obMatrix )
*
*	DESCRIPTION		Performs CVector * CMatrix multiplication, returning a NEW CVector object. Don't
*					be alarmed by the fact it returns by value - this will be optimised away in
*					release builds due to the fact that a CVector can be safely held in an __m128
*					register.
*	
*	NOTES			Shamelessly taken from Simon's SSE matrix class. Intentionally in global scope.
*
***************************************************************************************************/

CVector		operator *	( const CVector& obVector, const CMatrix& obMatrix )
{
	CVector	obResult;
	CVector	obTemp;

	// perform v.xxxx * row0

	obResult.Quadword() =	_mm_mul_ps(	_mm_shuffle_ps( obVector.QuadwordValue(), obVector.QuadwordValue(), _MM_SSE_SHUFFLE( 0, 0, 0, 0 ) ),
										obMatrix.Row( 0 ).QuadwordValue() );

	// add v.yyyy * row1

	obTemp.Quadword()	=	_mm_mul_ps(	_mm_shuffle_ps( obVector.QuadwordValue(), obVector.QuadwordValue(), _MM_SSE_SHUFFLE( 1, 1, 1, 1 ) ),
										obMatrix.Row( 1 ).QuadwordValue() );

	obResult.Quadword() =	_mm_add_ps( obResult.QuadwordValue(), obTemp.Quadword() );

	// add v.zzzz * row2

	obTemp.Quadword()	=	_mm_mul_ps(	_mm_shuffle_ps( obVector.QuadwordValue(), obVector.QuadwordValue(), _MM_SSE_SHUFFLE( 2, 2, 2, 2 ) ),
										obMatrix.Row( 2 ).QuadwordValue() );

	obResult.Quadword() =	_mm_add_ps( obResult.QuadwordValue(), obTemp.Quadword() );

	// add v.wwww * row3

	obTemp.Quadword()	=	_mm_mul_ps(	_mm_shuffle_ps( obVector.QuadwordValue(), obVector.QuadwordValue(), _MM_SSE_SHUFFLE( 3, 3, 3, 3 ) ),
										obMatrix.Row( 3 ).QuadwordValue() );

	obResult.Quadword() =	_mm_add_ps( obResult.QuadwordValue(), obTemp.Quadword() );

	// return the result

	return obResult;
}


/***************************************************************************************************
*
*	FUNCTION		operator *	( const CDirection& obDirection, const CMatrix& obMatrix )
*
*	DESCRIPTION		Performs CDirection * CMatrix multiplication, returning a NEW CVector object.
*					Don't be alarmed by the fact it returns by value - this will be optimised away in
*					release builds due to the fact that a CVecCDirectiontor can be safely held in an
*					__m128 register.
*	
*	NOTES			Shamelessly taken from Simon's SSE matrix class. Intentionally in global scope.
*
***************************************************************************************************/

CDirection	operator *	( const CDirection& obDirection, const CMatrix& obMatrix )
{
	CDirection	obResult;
	CDirection	obTemp;

	// perform v.xxxx * row0

	obResult.Quadword() =	_mm_mul_ps(	_mm_shuffle_ps( obDirection.QuadwordValue(), obDirection.QuadwordValue(), _MM_SSE_SHUFFLE( 0, 0, 0, 0 ) ),
										obMatrix.Row( 0 ).QuadwordValue() );

	// add v.yyyy * row1

	obTemp.Quadword()	=	_mm_mul_ps(	_mm_shuffle_ps( obDirection.QuadwordValue(), obDirection.QuadwordValue(), _MM_SSE_SHUFFLE( 1, 1, 1, 1 ) ),
										obMatrix.Row( 1 ).QuadwordValue() );

	obResult.Quadword() =	_mm_add_ps( obResult.QuadwordValue(), obTemp.Quadword() );

	// add v.zzzz * row2

	obTemp.Quadword()	=	_mm_mul_ps(	_mm_shuffle_ps( obDirection.QuadwordValue(), obDirection.QuadwordValue(), _MM_SSE_SHUFFLE( 2, 2, 2, 2 ) ),
										obMatrix.Row( 2 ).QuadwordValue() );

	obResult.Quadword() =	_mm_add_ps( obResult.QuadwordValue(), obTemp.Quadword() );

	// return the result

	return obResult;
}

/***************************************************************************************************
*
*	FUNCTION		operator *	( const CPoint& obPoint, const CMatrix& obMatrix )
*
*	DESCRIPTION		Performs CPoint * CMatrix multiplication, returning a NEW CPoint object. Don't
*					be alarmed by the fact it returns by value - this will be optimised away in
*					release builds due to the fact that a CPoint can be safely held in an __m128
*					register.
*	
*	NOTES			Shamelessly taken from Simon's SSE matrix class.
*
***************************************************************************************************/

CPoint				operator *	( const CPoint& obPoint, const CMatrix& obMatrix )
{
	CPoint	obResult;
	CPoint	obTemp;

	// perform v.xxxx * row0

	obResult.Quadword() =	_mm_mul_ps(	_mm_shuffle_ps( obPoint.QuadwordValue(), obPoint.QuadwordValue(), _MM_SSE_SHUFFLE( 0, 0, 0, 0 ) ),
										obMatrix.Row( 0 ).QuadwordValue() );

	// add v.yyyy * row1

	obTemp.Quadword()	=	_mm_mul_ps(	_mm_shuffle_ps( obPoint.QuadwordValue(), obPoint.QuadwordValue(), _MM_SSE_SHUFFLE( 1, 1, 1, 1 ) ),
										obMatrix.Row( 1 ).QuadwordValue() );

	obResult.Quadword() =	_mm_add_ps( obResult.QuadwordValue(), obTemp.Quadword() );

	// add v.zzzz * row2

	obTemp.Quadword()	=	_mm_mul_ps(	_mm_shuffle_ps( obPoint.QuadwordValue(), obPoint.QuadwordValue(), _MM_SSE_SHUFFLE( 2, 2, 2, 2 ) ),
										obMatrix.Row( 2 ).QuadwordValue() );

	obResult.Quadword() =	_mm_add_ps( obResult.QuadwordValue(), obTemp.Quadword() );

	// add row3 (CPoint objects have an implicit 1.0f in the W() component)

	obResult.Quadword() =	_mm_add_ps( obResult.QuadwordValue(), obMatrix.Row( 3 ).QuadwordValue() );

	// return the result

	return obResult;
}


/***************************************************************************************************
*
*	FUNCTION		CMatrix::GetAffineInverse
*
*	DESCRIPTION		Returns a NEW matrix containing the affine inverse of the current matrix
*
***************************************************************************************************/

CMatrix	CMatrix::GetAffineInverse( void ) const
{
	CMatrix	obOutput;

	// build the intermediate matrix:
	//		e00, e01, e10, e11
	//		e20, e21, e30, e31
	//		e02, e03, e12, e13
	//		e22, e23, e32, e33

	__m128 xmmTemp0 = _mm_shuffle_ps( Row( 0 ).QuadwordValue(), Row( 1 ).QuadwordValue(), _MM_SSE_SHUFFLE( 0, 1, 0, 1 ) );
	__m128 xmmTemp1 = _mm_shuffle_ps( Row( 2 ).QuadwordValue(), Row( 3 ).QuadwordValue(), _MM_SSE_SHUFFLE( 0, 1, 0, 1 ) );
	__m128 xmmTemp2 = _mm_shuffle_ps( Row( 0 ).QuadwordValue(), Row( 1 ).QuadwordValue(), _MM_SSE_SHUFFLE( 2, 3, 2, 3 ) );
	__m128 xmmTemp3 = _mm_shuffle_ps( Row( 2 ).QuadwordValue(), Row( 3 ).QuadwordValue(), _MM_SSE_SHUFFLE( 2, 3, 2, 3 ) );

	// return the matrix:
	//		e00, e10, e20, e30
	//		e01, e11, e21, e31
	//		e02, e12, e22, e32
	//
	// Each row of the matrix has the W component masked off (as the resultant matrix represents an affine transformation)
	
	obOutput.Row( 0 ).Quadword() = _mm_and_ps( _mm_shuffle_ps( xmmTemp0, xmmTemp1, _MM_SSE_SHUFFLE( 0, 2, 0, 2 ) ), *( __m128* )&CVecMath::m_gauiMaskOffW );
	obOutput.Row( 1 ).Quadword() = _mm_and_ps( _mm_shuffle_ps( xmmTemp0, xmmTemp1, _MM_SSE_SHUFFLE( 1, 3, 1, 3 ) ), *( __m128* )&CVecMath::m_gauiMaskOffW );
	obOutput.Row( 2 ).Quadword() = _mm_and_ps( _mm_shuffle_ps( xmmTemp2, xmmTemp3, _MM_SSE_SHUFFLE( 0, 2, 0, 2 ) ),	*( __m128* )&CVecMath::m_gauiMaskOffW );

	// Compute our NEW translation, but instead of the cack above this comment we'll use the already transposed result in order
	// to save cycles. Note we reuse the xmmTemp0/xmmTemp1 variables..
	//
	// General behaviour:
	//		perform ( v.xxxx * row0 )
	//		add		( v.yyyy * row1 )
	//		add		( v.zzzz * row2 )
	//		Mask off result W component, and put in 1.0f (resultant matrix represents an affine transformation)

	CPoint	obTranslation = -GetTranslation();
	xmmTemp0	=	_mm_mul_ps(	_mm_shuffle_ps( obTranslation.QuadwordValue(), obTranslation.QuadwordValue(), _MM_SSE_SHUFFLE( 0, 0, 0, 0 ) ), obOutput.Row( 0 ).QuadwordValue() );
	xmmTemp1	=	_mm_mul_ps(	_mm_shuffle_ps( obTranslation.QuadwordValue(), obTranslation.QuadwordValue(), _MM_SSE_SHUFFLE( 1, 1, 1, 1 ) ), obOutput.Row( 1 ).QuadwordValue() );
	xmmTemp0	=	_mm_add_ps( xmmTemp0, xmmTemp1 );
	xmmTemp1	=	_mm_mul_ps(	_mm_shuffle_ps( obTranslation.QuadwordValue(), obTranslation.QuadwordValue(), _MM_SSE_SHUFFLE( 2, 2, 2, 2 ) ), obOutput.Row( 2 ).QuadwordValue() );
	obOutput.Row( 3 ).Quadword() = _mm_or_ps( _mm_and_ps( _mm_add_ps( xmmTemp0, xmmTemp1 ), *( __m128* )&CVecMath::m_gauiMaskOffW ), *( __m128* )&CVecMath::m_gauiOneInW );

	return obOutput;
}


/***************************************************************************************************
*
*	FUNCTION		CMatrix::GetFullInverse
*
*	DESCRIPTION		Returns a NEW matrix containing the full inverse of the current 4x4 matrix. This
*					code was taken from a sample by Intel Architecture Labs, made available on both 
*					their developer support site and as an part of an source package that went with
*					their Gamasutra article.
*
*	NOTES			If it's crap/bugged, it's not my fault. The implementation seems pretty cool to
*					me though...
*
***************************************************************************************************/

CMatrix	CMatrix::GetFullInverse( void ) const
{
	CMatrix	obMinterms;
	__m128	qVa;
	__m128	qVb;
	__m128	qVc;
	__m128	qTemp1;
	__m128	qTemp2;
	__m128	qR1;
	__m128	qR2;
	__m128	qR3;
	__m128	qSum;
	__m128	qDet;
	__m128	qRDet;

	// Calculate minterms for the first line

	qTemp1 = Row( 3 ).QuadwordValue();
	qTemp2 = _mm_ror_ps( Row( 2 ).QuadwordValue(), 1 );
	qVa	= _mm_mul_ps( qTemp2, _mm_ror_ps( qTemp1, 2 ) );	// V3'·V4"
	qVb	= _mm_mul_ps( qTemp2, _mm_ror_ps( qTemp1, 3 ) );	// V3'·V4^
	qVc	= _mm_mul_ps( qTemp2, _mm_ror_ps( qTemp1, 0 ) );	// V3'·V4
		
	qR1 = _mm_sub_ps( _mm_ror_ps( qVa, 1 ), _mm_ror_ps( qVc, 2 ) );		// V3"·V4^ - V3^·V4"	
	qR2 = _mm_sub_ps( _mm_ror_ps( qVb, 2 ), _mm_ror_ps( qVb, 0 ) );		// V3^·V4' - V3'·V4^
	qR3 = _mm_sub_ps( _mm_ror_ps( qVa, 0 ), _mm_ror_ps( qVc, 1 ) );		// V3'·V4" - V3"·V4'

	qTemp1 = Row( 1 ).QuadwordValue();
	qVa = _mm_ror_ps( qTemp1, 1 );	qSum = _mm_mul_ps( qVa, qR1 );
	qVb = _mm_ror_ps( qTemp1, 2 );	qSum = _mm_add_ps( qSum, _mm_mul_ps( qVb, qR2 ) );
	qVc = _mm_ror_ps( qTemp1, 3 );	qSum = _mm_add_ps( qSum, _mm_mul_ps( qVc, qR3 ) );

	// Calculating the determinant
	
	qDet = _mm_mul_ps( qSum, Row( 0 ).QuadwordValue() );
	qDet = _mm_add_ps( qDet, _mm_movehl_ps( qDet, qDet ) );
	obMinterms.Row( 0 ).Quadword() = _mm_xor_ps( qSum, *( ( __m128* )CVecMath::m_gauiPNPNMask ) );

	// Calculating the minterms of the second line (using previous results)
	
	qTemp1 = _mm_ror_ps( Row( 0 ).QuadwordValue(), 1 );	qSum = _mm_mul_ps( qTemp1, qR1 );
	qTemp1 = _mm_ror_ps( qTemp1, 1 );					qSum = _mm_add_ps( qSum, _mm_mul_ps( qTemp1, qR2 ) );
	qTemp1 = _mm_ror_ps( qTemp1, 1 );					qSum = _mm_add_ps( qSum, _mm_mul_ps( qTemp1, qR3 ) );
	obMinterms.Row( 1 ).Quadword() = _mm_xor_ps( qSum, *( ( __m128* )CVecMath::m_gauiNPNPMask ) );

	// Testing the determinant
	qDet = _mm_sub_ss( qDet, _mm_shuffle_ps( qDet, qDet, 1 ) );
	int	iFlag = _mm_comieq_ss( qDet, _mm_sub_ss( qTemp1, qTemp1 ) );

	// Calculating the minterms of the third line

	qTemp1 = _mm_ror_ps( Row( 0 ).QuadwordValue(), 1 );
	qVa = _mm_mul_ps( qTemp1, qVb );									// V1'·V2"
	qVb = _mm_mul_ps( qTemp1, qVc );									// V1'·V2^
	qVc = _mm_mul_ps( qTemp1, Row( 1 ).QuadwordValue() );				// V1'·V2

	qR1 = _mm_sub_ps( _mm_ror_ps( qVa, 1 ), _mm_ror_ps( qVc, 2 ) );		// V1"·V2^ - V1^·V2"
	qR2 = _mm_sub_ps( _mm_ror_ps( qVb, 2 ), _mm_ror_ps( qVb, 0 ) );		// V1^·V2' - V1'·V2^
	qR3 = _mm_sub_ps( _mm_ror_ps( qVa, 0 ), _mm_ror_ps( qVc, 1 ) );		// V1'·V2" - V1"·V2'

	qTemp1 = _mm_ror_ps( Row( 3 ).QuadwordValue(), 1 );	qSum = _mm_mul_ps( qTemp1, qR1 );
	qTemp1 = _mm_ror_ps( qTemp1, 1 );					qSum = _mm_add_ps( qSum, _mm_mul_ps( qTemp1, qR2 ) );
	qTemp1 = _mm_ror_ps( qTemp1, 1 );					qSum = _mm_add_ps( qSum, _mm_mul_ps( qTemp1, qR3 ) );
	obMinterms.Row( 2 ).Quadword() =  _mm_xor_ps( qSum, *( ( __m128* )CVecMath::m_gauiPNPNMask ) );

	// Dividing is FASTER than rcp_nr! (Because rcp_nr causes many register-memory RWs).
	qRDet = _mm_div_ss( _mm_set_ss( 1.0f ), qDet );
	qRDet = _mm_shuffle_ps( qRDet, qRDet, 0x00 );

	// Divide the first 12 minterms with the determinant.
	obMinterms.Row( 0 ).Quadword() = _mm_mul_ps( obMinterms.Row( 0 ).QuadwordValue(), qRDet );
	obMinterms.Row( 1 ).Quadword() = _mm_mul_ps( obMinterms.Row( 1 ).QuadwordValue(), qRDet );
	obMinterms.Row( 2 ).Quadword() = _mm_mul_ps( obMinterms.Row( 2 ).QuadwordValue(), qRDet );

	// Calculate the minterms of the fourth line and divide by the determinant.
	qTemp1 = _mm_ror_ps( Row( 2 ).QuadwordValue(), 1 );	qSum = _mm_mul_ps( qTemp1, qR1 );
	qTemp1 = _mm_ror_ps( qTemp1, 1 );					qSum = _mm_add_ps( qSum, _mm_mul_ps( qTemp1, qR2 ) );
	qTemp1 = _mm_ror_ps( qTemp1, 1 );					qSum = _mm_add_ps( qSum, _mm_mul_ps( qTemp1, qR3 ) );
	obMinterms.Row( 3 ).Quadword() = _mm_xor_ps( qSum, *( ( __m128* )CVecMath::m_gauiNPNPMask ) );
	obMinterms.Row( 3 ).Quadword() = _mm_mul_ps(obMinterms.Row( 3 ).QuadwordValue(), qRDet );

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
		qTrns0 = _mm_unpacklo_ps( obMinterms.Row( 0 ).QuadwordValue(), obMinterms.Row( 1 ).QuadwordValue() );
		qTrns1 = _mm_unpacklo_ps( obMinterms.Row( 2 ).QuadwordValue(), obMinterms.Row( 3 ).QuadwordValue() );
		qTrns2 = _mm_unpackhi_ps( obMinterms.Row( 0 ).QuadwordValue(), obMinterms.Row( 1 ).QuadwordValue() );
		qTrns3 = _mm_unpackhi_ps( obMinterms.Row( 2 ).QuadwordValue(), obMinterms.Row( 3 ).QuadwordValue() );
		obOutput.Row( 0 ).Quadword() = _mm_movelh_ps( qTrns0, qTrns1 );
		obOutput.Row( 1 ).Quadword() = _mm_movehl_ps( qTrns1, qTrns0 );
		obOutput.Row( 2 ).Quadword() = _mm_movelh_ps( qTrns2, qTrns3 );
		obOutput.Row( 3 ).Quadword() = _mm_movehl_ps( qTrns3, qTrns2 );
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


