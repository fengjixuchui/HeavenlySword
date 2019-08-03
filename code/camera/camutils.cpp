/***************************************************************************************************
*
*	$Header:: /game/camutils.cpp 3     13/08/03 14:40 Wil                                          $
*
*
*
*	CHANGES
*
*	11/8/2003	Wil	Created
*
***************************************************************************************************/

#include "camera/camutils.h"
#include "core/visualdebugger.h"


/***************************************************************************************************
*
*	FUNCTION		CCamUtil::CalcAdjustedLookat
*
*	DESCRIPTION		calc a new lookat based on current matrix z and old lookat point
*
***************************************************************************************************/
CPoint	CCamUtil::CalcAdjustedLookat( const CMatrix& obMat, const CPoint& obLookAt )
{
	CDirection obLookDir = obLookAt ^ obMat.GetTranslation();

	float fDistToTarg = obLookDir.Length();

	obLookDir = obMat.GetZAxis() * fDistToTarg;

	return (obMat.GetTranslation() + obLookDir);
}

/***************************************************************************************************
*
*	FUNCTION		ConvergeBySpringDamp
*
*	DESCRIPTION		Dampen the movement of a variable to a target value via a simple spring
*
*	NOTES			Handles wrapping over the max and min valid values.
*
***************************************************************************************************/
float	CCamUtil::ConvergeBySpringDamp(	bool bValWrap,		// do values wrap round in a loop
											float fValCur,		// our current value
											float fValTarg,		// our target value
											float fSpringConst,	// spring constant of spring
											float fSpringDamp,	// spring dampening
											float& fValVel,		// velocity of value
											float fValMin,		// minimum value
											float fValMax,		// maximum value
											float fTimeChange )	// time delta
{
	if(fTimeChange <= 0.0f)
		return fValCur;

	float fValReturn;

	if (bValWrap)	
	{
		ntAssert( (fValCur >= fValMin) && (fValCur <= fValMax) );		// should have a valid current value
		ntAssert( (fValTarg >= fValMin) && (fValTarg <= fValMax) );	// should have a valid target value

		float fValRange = fValMax - fValMin;				// range of values to take
		float fValDiff = fabsf( fValTarg - fValCur );		// difference between our current and target values

		// decide what is the best direction to move in.. this assumes equidistant value intervals....
		//--------------------------------------------------------------------------------------------	
		bool bIncreaseVal;

		if (fValTarg > fValCur)
			bIncreaseVal = (fValDiff > (fValRange * 0.5f)) ? false : true;
		else
			bIncreaseVal = (fValDiff > (fValRange * 0.5f)) ? true : false;

		// adjust the difference value if we're crossing a boundary
		//---------------------------------------------------------------------------
		if (fValDiff > (fValRange * 0.5f))
			fValDiff = fValRange - fValDiff;

		// WD removed this in time change re-write (14.02.04)
//		fValDiff /= CTimer::Get().GetGameTimeScalar();

		// calculate the new velocity and fix its sign
		//-------------------------------------------------------------------------------------------
		fValVel = fabsf(fValVel);

		if (bIncreaseVal)
			fValVel = (fSpringConst * fValDiff) + (fSpringDamp * fValVel);
		else
			fValVel = (fSpringConst * -fValDiff) + (fSpringDamp * -fValVel);

		// Apply the value increment in the correct direction
		//--------------------------------------------------------------------------------------------
		fValReturn = fValCur + (fValVel * fTimeChange);

		// Compensate for wrapping round the value range
		//--------------------------------------------------------------------------------------------
		while (fValReturn > fValMax)
			fValReturn -= fValRange;

		while (fValReturn < fValMin)
			fValReturn += fValRange;
	}
	else
	{
		// WD removed this in time change re-write (14.02.04)
//		float fDisplacement = (fValCur - fValTarg) / CTimer::Get().GetGameTimeScalar();
		float fDisplacement = (fValCur - fValTarg);

		// calc force exerted by the spring
		fValVel = (fSpringConst * -fDisplacement) + (fSpringDamp * fValVel);

		// calc our new position
		fValReturn = fValCur + (fValVel * fTimeChange);
	}

	return fValReturn;
}


/***************************************************************************************************
*
*	FUNCTION		ConvergeByFixedDegree
*
*	DESCRIPTION		Move to a target value from a current value by a fixed amount
*
*	NOTES			Handles wrapping over the max and min valid values.
*
***************************************************************************************************/
float	CCamUtil::ConvergeByFixedDegree(	bool bValWrap,		// do values wrap round in a loop
											float fValCur,		// our current value
											float fValTarg,		// our target value
											float fValInc,		// amount to change value by
											float fValMin,		// minimum value
											float fValMax )		// maximum value
{
	float fValReturn;
	float fValDiff = fabsf( fValTarg - fValCur );		// difference between our current and target values

	// cap the increment to make sure we dont overshoot
	//-------------------------------------------------
	if (fValDiff < fValInc)	
		fValInc = fValDiff;

	if (bValWrap)	
	{
		ntAssert( (fValCur >= fValMin) && (fValCur <= fValMax) );		// should have a valid current value
		ntAssert( (fValTarg >= fValMin) && (fValTarg <= fValMax) );	// should have a valid target value

		float fValRange = fValMax - fValMin;				// range of values to take

		// cap the increment to make sure we dont overshoot when wrapping
		//--------------------------------------------------------------
		if ( (fValDiff > (fValRange * 0.5f)) && ((fValRange - fValDiff) < fValInc) )
			fValInc = fValRange - fValDiff;

		// decide what is the best direction to move in.. this assumes equidistant value intervals....
		//--------------------------------------------------------------------------------------------	
		bool bIncreaseVal;

		if (fValTarg > fValCur)
			bIncreaseVal = (fValDiff > (fValRange * 0.5f)) ? false : true;
		else
			bIncreaseVal = (fValDiff > (fValRange * 0.5f)) ? true : false;

		// Apply the value increment in the correct direction
		//--------------------------------------------------------------------------------------------
		if (bIncreaseVal)
			fValReturn = fValCur + fValInc;
		else
			fValReturn = fValCur - fValInc;

		// Compensate for wrapping round the value range
		//--------------------------------------------------------------------------------------------
		while (fValReturn > fValMax)
			fValReturn -= fValRange;

		while (fValReturn < fValMin)
			fValReturn += fValRange;
	}
	else
	{
		if (fValTarg > fValCur)
			fValReturn = fValCur + fValInc;
		else
			fValReturn = fValCur - fValInc;
	}

	return fValReturn;
}


/***************************************************************************************************
*
*	FUNCTION		CartesianFromSpherical
*
*	DESCRIPTION		Converts input spherical angles into normalised direction (Y is pole)
*
*	INPUTS			latitude(fX) in the range -HALF_PI to HALF_PI
*					longditude(fY) in the range 0 to TWO_PI
*
*	EXAMPLES		Vary latitude from -90 degrees to 90 degrees, get YAxis -> ZAxis -> -YAxis
*					( 0,  1,  0) = CartesianFromSpherical( -HALF_PI, 0 );
*					( 0,  0,  1) = CartesianFromSpherical( 0, 0 );
*					( 0, -1,  0) = CartesianFromSpherical( HALF_PI, 0 );
*
*					Vary longditude from 0 degrees to 360 degrees, get ZAxis -> XAxis -> -ZAxis -> -XAxis -> ZAxis
*					( 0,  0,  1) = CartesianFromSpherical( 0, 0 );
*					( 1,  0,  0) = CartesianFromSpherical( 0, HALF_PI );
*					( 0,  0, -1) = CartesianFromSpherical( 0, PI );
*					(-1,  0,  0) = CartesianFromSpherical( 0, PI + HALF_PI );
*					( 0,  0,  1) = CartesianFromSpherical( 0, TWO_PI );
*
***************************************************************************************************/
CDirection	CCamUtil::CartesianFromSpherical( float fX, float fY )
{
	float SX, SY, CX, CY;

	CMaths::SinCos( fX + HALF_PI, SX, CX );
	CMaths::SinCos( fY, SY, CY );

	CDirection obResult( SX*SY, CX, SX*CY );
	obResult.Normalise();

	return obResult;
}

/***************************************************************************************************
*
*	FUNCTION		SphericalFromCartesian
*
*	DESCRIPTION		Converts input direction into latitude(fX) and longditude(fY)
*
***************************************************************************************************/
void	CCamUtil::SphericalFromCartesian( const CDirection& obDir, float& fX, float& fY )
{
	float fProjection = fsqrtf((obDir.X() * obDir.X()) + (obDir.Z() * obDir.Z()));

	fX = -atan2f( obDir.Y(), fProjection );
	fY = atan2f( obDir.X(), obDir.Z() );
}

/***************************************************************************************************
*
*	FUNCTION		QuatFromEuler_XYZ
*
*	DESCRIPTION		Generate a quaternion from euler angles in XYZ order
*
***************************************************************************************************/
CQuat CCamUtil::QuatFromEuler_XYZ( float fX, float fY, float fZ )
{
	float SX, CX, SY, CY, SZ, CZ;

	CMaths::SinCos( fX * 0.5f, SX, CX );
	CMaths::SinCos( fY * 0.5f, SY, CY );
	CMaths::SinCos( fZ * 0.5f, SZ, CZ );

	float Q1 = CZ*CY;
	float Q2 = SZ*SY;
	float Q3 = CZ*SY;
	float Q4 = SZ*CY;

	return CQuat( (Q1*SX)-(Q2*CX), (Q3*CX)+(Q4*SX), (Q4*CX)-(Q3*SX), (Q1*CX)+(Q2*SX) );
}

/***************************************************************************************************
*
*	FUNCTION		QuatFromEuler_ZXY
*
*	DESCRIPTION		Converts x, y and z euler angles into a quaternion with the rotation order ZXY
*
***************************************************************************************************/
CQuat	QuatFromEuler_ZXY( float fX, float fY, float fZ )
{
	float SX, CX, SY, CY, SZ, CZ;

	CMaths::SinCos( fX * 0.5f, SX, CX );
	CMaths::SinCos( fY * 0.5f, SY, CY );
	CMaths::SinCos( fZ * 0.5f, SZ, CZ );

	float Q1 = CX*CY;
	float Q2 = SX*CY;
	float Q3 = CX*SY;
	float Q4 = SX*SY;

	return CQuat( (Q2*CZ)+(Q3*SZ), (Q3*CZ)-(Q2*SZ), (Q1*SZ)-(Q4*CZ), (Q1*CZ)+(Q4*SZ) );
}

/***************************************************************************************************
*
*	FUNCTION		MatrixFromEuler_XYZ
*
*	DESCRIPTION		Generate a matrix from euler angles in XYZ order
*
***************************************************************************************************/
void	CCamUtil::MatrixFromEuler_XYZ( CMatrix& obMat, float fX, float fY, float fZ )
{
	float SX, CX, SY, CY, SZ, CZ;

	CMaths::SinCos( fX, SX, CX );
	CMaths::SinCos( fY, SY, CY );
	CMaths::SinCos( fZ, SZ, CZ );

	obMat.SetXAxis( CDirection(	CY*CZ,	CY*SZ,	-SY ) );
	obMat.SetYAxis( CDirection(	(SX*SY*CZ)-(CX*SZ),	(SX*SY*SZ)+(CX*CZ),	SX*CY ) );
	obMat.SetZAxis( CDirection(	(CX*SY*CZ)+(SX*SZ),	(CX*SY*SZ)-(SX*CZ),	CX*CY ) );
	obMat.SetTranslation( CVecMath::GetZeroPoint() );
}

/***************************************************************************************************
*
*	FUNCTION		MatrixFromEuler_ZXY
*
*	DESCRIPTION		Converts x, y and z euler angles into a rotation matrix with rotation order ZXY
*
***************************************************************************************************/
void	CCamUtil::MatrixFromEuler_ZXY( CMatrix& obMat, float fX, float fY, float fZ )
{
	float SX, CX, SY, CY, SZ, CZ;

	CMaths::SinCos( fX, SX, CX );
	CMaths::SinCos( fY, SY, CY );
	CMaths::SinCos( fZ, SZ, CZ );

	obMat.SetXAxis( CDirection(	(CY*CZ)+(SX*SY*SZ),	CX*SZ,	(SX*CY*SZ)-(SY*CZ) ) );
	obMat.SetYAxis( CDirection(	(SX*SY*CZ)-(CY*SZ), CX*CZ,	(SY*SZ)+(SX*CY*CZ) ) );
	obMat.SetZAxis( CDirection(	CX*SY, -SX,	CX*CY ) );
	obMat.SetTranslation( CVecMath::GetZeroPoint() );
}

/***************************************************************************************************
*
*	FUNCTION		EulerFromMat_XYZ
*
*	DESCRIPTION		recover euler rotations from an XYZ constructed rotation matrix
*
***************************************************************************************************/
void	CCamUtil::EulerFromMat_XYZ( const CMatrix& obMat, float& fX, float& fY, float& fZ )
{
	CMatrix obLocal( obMat );
	obLocal.SetTranslation( CVecMath::GetZeroPoint() );

	// recover z rotation from matrix first
	fZ = atan2f( obLocal.GetXAxis().Y(), obLocal.GetXAxis().X() );

	if ( obLocal.GetXAxis().X() < 0.0f )
		fZ -= PI;

	// rotate matrix back by this amount
	obLocal = obLocal * CMatrix( CVecMath::GetZAxis(), -fZ );

	// recover Y rotation next
	fY = atan2f( obLocal.GetZAxis().X(), obLocal.GetZAxis().Z() );

	// rotate matrix back by this amount
	obLocal = obLocal * CMatrix( CVecMath::GetYAxis(), -fY );

	// finally recover the X rotation
	fX = atan2f( obLocal.GetZAxis().Z(), obLocal.GetZAxis().Y() ) - HALF_PI;
}

/***************************************************************************************************
*
*	FUNCTION		EulerFromMat_ZXY
*
*	DESCRIPTION		recover euler rotations from an ZXY constructed rotation matrix
*
***************************************************************************************************/
void	CCamUtil::EulerFromMat_ZXY( const CMatrix& obMat, float& fX, float& fY, float& fZ )
{
	CMatrix obLocal( obMat );
	obLocal.SetTranslation( CVecMath::GetZeroPoint() );

	// recover y rotation from matrix first
	fY = atan2f( obLocal.GetZAxis().X(), obLocal.GetZAxis().Z() );

	// rotate matrix back by this amount
	obLocal = obLocal * CMatrix( CVecMath::GetYAxis(), -fY );

	// recover X rotation next
	fX = atan2f( obLocal.GetZAxis().Z(), obLocal.GetZAxis().Y() ) - HALF_PI;

	// rotate matrix back by this amount
	obLocal = obLocal * CMatrix( CVecMath::GetXAxis(), -fX );

	// finally recover the Z rotation
	fZ = atan2f( obLocal.GetXAxis().Y(), obLocal.GetXAxis().X() );
}

/***************************************************************************************************
*
*	FUNCTION		EulerFromQuat_XYZ
*
*	DESCRIPTION		recover euler rotations from an ZXY constructed rotation matrix
*
***************************************************************************************************/
void	CCamUtil::EulerFromQuat_XYZ( const CQuat& obQuat, float& fX, float& fY, float& fZ )
{
	CMatrix obLocal( obQuat, CVecMath::GetZeroPoint() );
	EulerFromMat_XYZ( obLocal, fX, fY, fZ );
}

/***************************************************************************************************
*
*	FUNCTION		EulerFromQuat_ZXY
*
*	DESCRIPTION		recover euler rotations from an ZXY constructed rotation matrix
*
***************************************************************************************************/
void	CCamUtil::EulerFromQuat_ZXY( const CQuat& obQuat, float& fX, float& fY, float& fZ )
{
	CMatrix obLocal( obQuat,  CVecMath::GetZeroPoint() );
	EulerFromMat_ZXY( obLocal, fX, fY, fZ );
}

/***************************************************************************************************
*
*	FUNCTION		CreateFromPoints
*
*	DESCRIPTION		Generate a matrix pointing from one point to another with the given up vector
*
***************************************************************************************************/
void	CCamUtil::CreateFromPoints( CMatrix& obMat, const CPoint& obFrom, const CPoint& obTo, const CDirection& obUp )
{	
	CDirection	obZAxis( obTo ^ obFrom );
	
	if( obZAxis.Normalise() )
		obMat.SetZAxis( CVecMath::GetZAxis() ); 
	else
		obMat.SetZAxis( obZAxis ); 

	CDirection	obXAxis = obUp.Cross( obMat.GetZAxis() );

	if( obXAxis.Normalise() )
		obMat.SetXAxis( CVecMath::GetXAxis() ); 
	else
		obMat.SetXAxis( obXAxis );

	obMat.BuildYAxis();
	obMat.SetTranslation( obFrom );
}

/***************************************************************************************************
*
*	FUNCTION		MatrixLookAt
*
*	DESCRIPTION		Generate a look at matrix
*
***************************************************************************************************/
void	CCamUtil::MatrixLookAt( CMatrix& obMat, const CPoint& obEye, const CPoint& obAt, const CDirection& obUp )
{
	CreateFromPoints( obMat, obEye, obAt, obUp );
	obMat = obMat.GetAffineInverse();
}





/***************************************************************************************************
*
*	FUNCTION		RenderLine
*
*	DESCRIPTION		render a line
*
***************************************************************************************************/
void	CCamUtil::Render_Line( const CPoint& obStart, const CPoint& obEnd, float fRed, float fGreen, float fBlue, float fAlpha )
{
#ifndef _GOLD_MASTER
	CVector col( fRed, fGreen, fBlue, fAlpha );
	g_VisualDebug->RenderLine( obStart, obEnd, col.GetNTColor(), 0 );
#endif
}

/***************************************************************************************************
*
*	FUNCTION		RenderMatrix
*
*	DESCRIPTION		Draw the three axis of the matrix
*
***************************************************************************************************/
void	CCamUtil::Render_Matrix( const CMatrix& obMat )
{
	CCamUtil::Render_Line(	obMat.GetTranslation(),
							obMat.GetTranslation() + obMat.GetXAxis(),
							1.0f, 0.0f, 0.0f, 0.0f );
	
	CCamUtil::Render_Line(	obMat.GetTranslation(),
							obMat.GetTranslation() + obMat.GetYAxis(),
							0.0f, 1.0f, 0.0f, 0.0f );
	
	CCamUtil::Render_Line(	obMat.GetTranslation(),
							obMat.GetTranslation() + obMat.GetZAxis(),
							0.0f, 0.0f, 1.0f, 0.0f );
}

/***************************************************************************************************
*
*	FUNCTION		RenderSphere
*
*	DESCRIPTION		Draw a sphere
*
***************************************************************************************************/
void	CCamUtil::Render_Sphere( const CPoint& obOrigin, float fRadius, float fRed, float fGreen, float fBlue, float fAlpha )
{
#ifndef _GOLD_MASTER
	CVector col( fRed, fGreen, fBlue, fAlpha );
	g_VisualDebug->RenderSphere( CVecMath::GetQuatIdentity(), obOrigin, fRadius, col.GetNTColor(), DPF_WIREFRAME );
#endif
}

/***************************************************************************************************
*
*	FUNCTION		RenderFrustum
*
*	DESCRIPTION		render a frustum
*
***************************************************************************************************/
void	CCamUtil::Render_Frustum( const CMatrix& obMat, float fFOV, float fAspect, float fRed, float fGreen, float fBlue, float fAlpha )
{
	static const float fFakeZNear = 0.2f;
	static const float fFakeZFar = 50.0f;

	CDirection m_obBoundsMin = CDirection(-1.0f, -1.0f, 0.0f);
	CDirection m_obBoundsMax = CDirection( 1.0f,  1.0f, 1.0f);

	const float fYScale = ftanf( fFOV * DEG_TO_RAD_VALUE );
	const float fXScale = fAspect * fYScale;

	CPoint aobQuad[] = 
	{
		CPoint( fXScale*m_obBoundsMin.X(), fYScale*m_obBoundsMin.Y(), 1.0f), 
		CPoint( fXScale*m_obBoundsMax.X(), fYScale*m_obBoundsMin.Y(), 1.0f), 
		CPoint( fXScale*m_obBoundsMax.X(), fYScale*m_obBoundsMax.Y(), 1.0f), 
		CPoint( fXScale*m_obBoundsMin.X(), fYScale*m_obBoundsMax.Y(), 1.0f)
	};

	const float fZNear = fFakeZNear*(1.0f-m_obBoundsMin.Z()) + fFakeZFar*m_obBoundsMin.Z();
	const float fZFar = fFakeZNear*(1.0f-m_obBoundsMax.Z()) + fFakeZFar*m_obBoundsMax.Z();

	CPoint obPositions[8];
	obPositions[0] = fZNear*aobQuad[0];
	obPositions[1] = fZNear*aobQuad[1];
	obPositions[2] = fZNear*aobQuad[2];
	obPositions[3] = fZNear*aobQuad[3];
	obPositions[4] = fZFar*aobQuad[0];
	obPositions[5] = fZFar*aobQuad[1];
	obPositions[6] = fZFar*aobQuad[2];
	obPositions[7] = fZFar*aobQuad[3];

	// convert to world space
	for(int iVertex = 0; iVertex < 8; ++iVertex)
        obPositions[iVertex] = obPositions[iVertex]*obMat;

	CCamUtil::Render_Line( obPositions[0], obPositions[1], fRed, fGreen, fBlue, fAlpha );
	CCamUtil::Render_Line( obPositions[1], obPositions[2], fRed, fGreen, fBlue, fAlpha );
	CCamUtil::Render_Line( obPositions[2], obPositions[3], fRed, fGreen, fBlue, fAlpha );
	CCamUtil::Render_Line( obPositions[3], obPositions[0], fRed, fGreen, fBlue, fAlpha );

	CCamUtil::Render_Line( obPositions[4], obPositions[5], fRed, fGreen, fBlue, fAlpha );
	CCamUtil::Render_Line( obPositions[5], obPositions[6], fRed, fGreen, fBlue, fAlpha );
	CCamUtil::Render_Line( obPositions[6], obPositions[7], fRed, fGreen, fBlue, fAlpha );
	CCamUtil::Render_Line( obPositions[7], obPositions[4], fRed, fGreen, fBlue, fAlpha );

	CCamUtil::Render_Line( obPositions[0], obPositions[4], fRed, fGreen, fBlue, fAlpha );
	CCamUtil::Render_Line( obPositions[1], obPositions[5], fRed, fGreen, fBlue, fAlpha );
	CCamUtil::Render_Line( obPositions[2], obPositions[6], fRed, fGreen, fBlue, fAlpha );
	CCamUtil::Render_Line( obPositions[3], obPositions[7], fRed, fGreen, fBlue, fAlpha );
}

/***************************************************************************************************
*
*	FUNCTION		RenderCross
*
*	DESCRIPTION		Render the point with three intersection world space axis lines
*
***************************************************************************************************/
void	CCamUtil::Render_Cross( const CPoint& obPos, float fScale, float fRed, float fGreen, float fBlue, float fAlpha )
{	
	CCamUtil::Render_Line(	obPos + CPoint( fScale, 0.0f, 0.0f ),
							obPos - CPoint( fScale, 0.0f, 0.0f ),
							fRed, fGreen, fBlue, fAlpha );

	CCamUtil::Render_Line(	obPos + CPoint( 0.0f, fScale, 0.0f ),
							obPos - CPoint( 0.0f, fScale, 0.0f ),
							fRed, fGreen, fBlue, fAlpha );

	CCamUtil::Render_Line(	obPos + CPoint( 0.0f, 0.0f, fScale ),
							obPos - CPoint( 0.0f, 0.0f, fScale ),
							fRed, fGreen, fBlue, fAlpha );
}

/***************************************************************************************************
*
*	FUNCTION		RenderAABB
*
*	DESCRIPTION		Render the Axis Aligned bounding box denoted by the min and max points
*
***************************************************************************************************/
void	CCamUtil::Render_AABB( const CPoint& obMin, const CPoint& obMax, float fRed, float fGreen, float fBlue, float fAlpha )
{
	CPoint obBot1, obBot2, obBot3, obBot4;
	CPoint obTop1, obTop2, obTop3, obTop4;

	obBot1.Y() = obBot2.Y() = obBot3.Y() = obBot4.Y() = obMin.Y();
	obTop1.Y() = obTop2.Y() = obTop3.Y() = obTop4.Y() = obMax.Y();
	
	obBot1.X() = obBot2.X() = obTop1.X() = obTop2.X() = obMin.X();
	obBot3.X() = obBot4.X() = obTop3.X() = obTop4.X() = obMax.X();

	obBot1.Z() = obBot3.Z() = obTop1.Z() = obTop3.Z() = obMin.Z();
	obBot2.Z() = obBot4.Z() = obTop2.Z() = obTop4.Z() = obMax.Z();

	CCamUtil::Render_Line( obBot1, obTop1, fRed, fGreen, fBlue, fAlpha );
	CCamUtil::Render_Line( obBot2, obTop2, fRed, fGreen, fBlue, fAlpha );
	CCamUtil::Render_Line( obBot3, obTop3, fRed, fGreen, fBlue, fAlpha );
	CCamUtil::Render_Line( obBot4, obTop4, fRed, fGreen, fBlue, fAlpha );

	CCamUtil::Render_Line( obBot1, obBot2, fRed, fGreen, fBlue, fAlpha );
	CCamUtil::Render_Line( obTop1, obTop2, fRed, fGreen, fBlue, fAlpha );
	CCamUtil::Render_Line( obBot3, obBot4, fRed, fGreen, fBlue, fAlpha );
	CCamUtil::Render_Line( obTop3, obTop4, fRed, fGreen, fBlue, fAlpha );

	CCamUtil::Render_Line( obBot1, obBot3, fRed, fGreen, fBlue, fAlpha );
	CCamUtil::Render_Line( obBot2, obBot4, fRed, fGreen, fBlue, fAlpha );
	CCamUtil::Render_Line( obTop1, obTop3, fRed, fGreen, fBlue, fAlpha );
	CCamUtil::Render_Line( obTop2, obTop4, fRed, fGreen, fBlue, fAlpha );
}

/***************************************************************************************************
*	
*	FUNCTION		CCamUtil::DebugPrintf
*
*	DESCRIPTION		render stuff
*
***************************************************************************************************/
void	CCamUtil::DebugPrintf( int iX, int iY, const char* pcBuffer, ...)
{
#ifndef _RELEASE
	static char	acBuffer[MAX_PATH*2];
	acBuffer[sizeof(acBuffer) - 1] = 0;

	va_list	stArgList;
	va_start(stArgList, pcBuffer);
	vsnprintf(acBuffer, sizeof(acBuffer) - 1, pcBuffer, stArgList);
	va_end(stArgList);

	g_VisualDebug->Printf2D( (float)iX, (float)iY, DC_WHITE, 0, "%s", acBuffer );
#else
	UNUSED(iX);
	UNUSED(iY);
	UNUSED(pcBuffer);
#endif
}


/***************************************************************************************************
*	
*	FUNCTION		CCamUtil::DebugPrintf
*
*	DESCRIPTION		with coulour
*
***************************************************************************************************/
void	CCamUtil::DebugPrintf( int iX, int iY, uint32_t dwColour, const char* pcBuffer, ...)
{
#ifndef _RELEASE
	static char	acBuffer[MAX_PATH*2];
	acBuffer[sizeof(acBuffer) - 1] = 0;

	va_list	stArgList;
	va_start(stArgList, pcBuffer);
	vsnprintf(acBuffer, sizeof(acBuffer) - 1, pcBuffer, stArgList);
	va_end(stArgList);

	g_VisualDebug->Printf2D( (float)iX, (float)iY, dwColour, 0, "%s", acBuffer );
#else
	UNUSED(iX);
	UNUSED(iY);
	UNUSED(pcBuffer);
	UNUSED(dwColour);
#endif
}

/***************************************************************************************************
*	
*	FUNCTION		CCamUtil::DumpMatrix
*
*	DESCRIPTION		
*
***************************************************************************************************/
void	CCamUtil::DumpMatrix( const CMatrix& obMat )
{
	ntPrintf("[ %f,\t%f,\t%f,\t%f ]\n",obMat.GetXAxis().X(),obMat.GetXAxis().Y(),obMat.GetXAxis().Z(),obMat.GetXAxis().W());
	ntPrintf("[ %f,\t%f,\t%f,\t%f ]\n",obMat.GetYAxis().X(),obMat.GetYAxis().Y(),obMat.GetYAxis().Z(),obMat.GetYAxis().W());
	ntPrintf("[ %f,\t%f,\t%f,\t%f ]\n",obMat.GetZAxis().X(),obMat.GetZAxis().Y(),obMat.GetZAxis().Z(),obMat.GetZAxis().W());
	ntPrintf("[ %f,\t%f,\t%f,\t%f ]\n",obMat.GetTranslation().X(),obMat.GetTranslation().Y(),obMat.GetTranslation().Z(),obMat.GetTranslation().W());
}
