/***************************************************************************************************
*
*	$Header:: /game/camutils.h 2     13/08/03 14:40 Wil                                            $
*
*
*
*	CHANGES
*
*	11/8/2003	Wil	Created
*
***************************************************************************************************/

#ifndef _CAM_UTILS_H
#define _CAM_UTILS_H

// Necessary includes
#include "input/inputhardware.h"

/***************************************************************************************************
*
*	ENUM			eROT_TYPE
*
*	DESCRIPTION		Quick way of specifying rotation order
*
***************************************************************************************************/
enum ROT_TYPE
{
	ROT_ORDER_XYZ,
	ROT_ORDER_ZXY,
	ROT_ORDER_MAX,
};

/***************************************************************************************************
*
*	CLASS			CCamUtil
*
*	DESCRIPTION		Collection of misc utility functions
*
***************************************************************************************************/
class CCamUtil
{
public:
	static CPoint	CalcAdjustedLookat( const CMatrix& obMat, const CPoint& obLookAt );

	// convergence routines
	static float	ConvergeByFixedDegree(	bool bValWrap,
											float fValCur,
											float fValTarg,
											float fValInc,
											float fValMin,
											float fValMax );

	static float	ConvergeBySpringDamp(	bool bValWrap,
											float fValCur,
											float fValTarg,
											float fSpringConst,
											float fSpringDamp,
											float& fValVel,
											float fValMin,
											float fValMax,
											float fTimeChange );

	static float	Sigmoid(float fVal, float fTotal)
	{
		float fRatio = ntstd::Clamp(fVal / fTotal, 0.f, 1.f);

		fRatio = (fRatio - .5f) * 8.f;                                // Mapping fn for Sigmoid fn.
		fRatio = 1.f / (1.f + pow(MATH_E, -fRatio));                  // Sigmoid fn.
		fRatio = ntstd::Clamp(fRatio * 1.02f/.98f - 0.02f, 0.f, 1.f); // Re-limiting correction.

		return fRatio;
	}

	// spherical coords to cartesian vectors
	static	CDirection	CartesianFromSpherical( float fX, float fY );
	static	void		SphericalFromCartesian( const CDirection& obDir, float& fX, float& fY );

	// quaternions from euler rotations
	static	CQuat		QuatFromEuler_XYZ( float fX, float fY, float fZ );
	static	CQuat		QuatFromEuler_ZXY( float fX, float fY, float fZ );
	
	// rotation matrices from euler rotations
	static	void		MatrixFromEuler_XYZ( CMatrix& obMat, float fX, float fY, float fZ );
	static	void		MatrixFromEuler_ZXY( CMatrix& obMat, float fX, float fY, float fZ );

	// euler rotations from rotation matrix
	static	void		EulerFromMat_XYZ( const CMatrix& obMat, float& fX, float& fY, float& fZ );
	static	void		EulerFromMat_ZXY( const CMatrix& obMat, float& fX, float& fY, float& fZ );

	// euler rotations from quaternions
	static	void		EulerFromQuat_XYZ( const CQuat& obQuat, float& fX, float& fY, float& fZ );
	static	void		EulerFromQuat_ZXY( const CQuat& obQuat, float& fX, float& fY, float& fZ );

	// matrix construction routines
	static	void		CreateFromPoints( CMatrix& obMat, const CPoint& obFrom, const CPoint& obTo, const CDirection& obUp = CVecMath::GetYAxis() );
	static	void		MatrixLookAt( CMatrix& obMat, const CPoint& obEye, const CPoint& obAt, const CDirection& obUp = CVecMath::GetYAxis() );

	// render functions
	static void			Render_Line( const CPoint& obStart, const CPoint& obEnd, float fRed, float fGreen, float fBlue, float fAlpha );
	static void			Render_Matrix( const CMatrix& obMat );
	static void			Render_Sphere( const CPoint& obOrigin, float fRadius, float fRed, float fGreen, float fBlue, float fAlpha );
	static void			Render_Frustum( const CMatrix& obMat, float fFOV, float fAspect, float fRed, float fGreen, float fBlue, float fAlpha );
	static void			Render_Cross( const CPoint& obPos, float fScale, float fRed, float fGreen, float fBlue, float fAlpha );
	static void			Render_AABB( const CPoint& obMin, const CPoint& obMax, float fRed, float fGreen, float fBlue, float fAlpha );
	
	static void			DebugPrintf( int iX, int iY, const char* pcBuffer, ...);
	static void			DebugPrintf( int iX, int iY, uint32_t dwColour, const char* pcBuffer, ...);

	static void			DumpMatrix( const CMatrix& obMat );

	// utility function wrappers
#define ROTATION_ORDER_XYZ

#ifdef ROTATION_ORDER_XYZ

	static	CQuat		QuatFromYawPitchRoll( float fYaw, float fPitch, float fRoll )						{ return QuatFromEuler_XYZ( fPitch, fYaw, fRoll ); }
	static	void		MatrixFromYawPitchRoll( CMatrix& obMat, float fYaw, float fPitch, float fRoll )		{ return MatrixFromEuler_XYZ( obMat, fPitch, fYaw, fRoll ); }
	static	void		YawPitchRollFromMat( const CMatrix& obMat, float& fYaw, float& fPitch, float& fRoll )	{ return EulerFromMat_XYZ( obMat, fPitch, fYaw, fRoll ); }
	static	void		YawPitchRollFromQuat( const CQuat& obQuat, float& fYaw, float& fPitch, float& fRoll )	{ return EulerFromQuat_XYZ( obQuat, fPitch, fYaw, fRoll ); }

#else

	static	CQuat		QuatFromYawPitchRoll( float fYaw, float fPitch, float fRoll )						{ return QuatFromEuler_ZXY( fPitch, fYaw, fRoll ); }
	static	void		MatrixFromYawPitchRoll( CMatrix& obMat, float fYaw, float fPitch, float fRoll )		{ return MatrixFromEuler_ZXY( obMat, fPitch, fYaw, fRoll ); }
	static	void		YawPitchRollFromMat( const CMatrix& obMat, float& fYaw, float& fPitch, float& fRoll )	{ return EulerFromMat_ZXY( obMat, fPitch, fYaw, fRoll ); }
	static	void		YawPitchRollFromQuat( const CQuat& obQuat, float& fYaw, float& fPitch, float& fRoll )	{ return EulerFromQuat_ZXY( obQuat, fPitch, fYaw, fRoll ); }

#endif

	static	CQuat		QuatForCameraFromYawPitchRoll( float fYaw, float fPitch, float fRoll )					{ return QuatFromEuler_XYZ( -fPitch, fYaw + PI, fRoll ); }
	static	void		MatixForCameraFromYawPitchRoll( CMatrix& obMat, float fYaw, float fPitch, float fRoll )	{ return MatrixFromEuler_XYZ( obMat, -fPitch, fYaw + PI, fRoll ); }
};

#endif // _CAM_UTILS_H
