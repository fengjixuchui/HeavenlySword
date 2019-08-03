/***************************************************************************************************
*
*	$Header:: /game/camvolumes.cpp 2   12/08/03 16:21 Wil                    $
*
*
*
*	CHANGES
*
*	11/8/2003	Wil	Created
*
***************************************************************************************************/

#include "camera/camvolumes.h"
#include "camera/camutils.h"
#include "camera/camgeometry.h"

#include "core/frustum.h"
#include "core/visualdebugger.h"
#include "game/randmanager.h"

#include "objectdatabase/dataobject.h"

#ifdef PLATFORM_PS3
	#include <calloca>
#else
	#include <malloc.h>
#endif

START_CHUNKED_INTERFACE(CamVolumeSphere, Mem::MC_CAMERA)
	PUBLISH_VAR_WITH_DEFAULT_AS(m_obPosition, CPoint(0.0f, 0.0f, 0.0f), Position)
	PUBLISH_VAR_WITH_DEFAULT_AS(m_fRadius, 1.0f, Radius)
	PUBLISH_VAR_WITH_DEFAULT_AS(m_bUsesPOI, true, UsesPOI)
END_STD_INTERFACE

START_CHUNKED_INTERFACE(CamVolumeCylinder, Mem::MC_CAMERA)
	PUBLISH_VAR_WITH_DEFAULT_AS(m_obPosition, CPoint(0.0f, 0.0f, 0.0f), Position)
	PUBLISH_VAR_WITH_DEFAULT_AS(m_fHeight, 1.0f, Height)
	PUBLISH_VAR_WITH_DEFAULT_AS(m_fRadius, 1.0f, Radius)
	PUBLISH_VAR_WITH_DEFAULT_AS(m_obAxis, CDirection(CVecMath::GetYAxis()), Axis)
	PUBLISH_VAR_WITH_DEFAULT_AS(m_bUsesPOI, true, UsesPOI)
	DECLARE_POSTCONSTRUCT_CALLBACK( PostConstruct )
END_STD_INTERFACE

START_CHUNKED_INTERFACE(CamVolumeBox, Mem::MC_CAMERA)
	PUBLISH_VAR_WITH_DEFAULT_AS(m_obPosition, CPoint(0.0f, 0.0f, 0.0f), Position)
	PUBLISH_VAR_WITH_DEFAULT_AS(m_obOrientation, CQuat(0.0f, 0.0f, -1.0f, 0.0f), Orientation)
	PUBLISH_VAR_WITH_DEFAULT_AS(m_obDimensions, CPoint(0.0f, 0.0f, 0.0f), Dimensions)
	PUBLISH_VAR_WITH_DEFAULT_AS(m_bUsesPOI, true, UsesPOI)
	DECLARE_POSTCONSTRUCT_CALLBACK( PostConstruct )
	DECLARE_EDITORCHANGEVALUE_CALLBACK(OnEdit)
END_STD_INTERFACE

START_CHUNKED_INTERFACE(CamVolumeSet, Mem::MC_CAMERA)
	PUBLISH_PTR_CONTAINER_AS(m_obVolumes,Volumes)
	PUBLISH_PTR_CONTAINER_AS(m_obExVolumes,ExVolumes)
	PUBLISH_VAR_WITH_DEFAULT_AS(m_bUsesPOI, true, UsesPOI)
END_STD_INTERFACE


/******************************************************************************************************************
*	
*	FUNCTION		CIntersectResults::SetIfNearFar
*
*	DESCRIPTION	 	Returns true only if we're already set
*						If true is flagged, it means it's setting 
*						a point (obSurface) nearer to (or further from,, depending on bNear) to obRefPos
*
*
********************************************************************************************************************/

bool	CamVolume::CIntersectResults::SetIfNearFar( const CPoint& obSurface, 
										 const CDirection& obNormal, 
										 bool bNear)
{
	if(!IsValid())
	{
		SetResults(obSurface,obNormal);
		return false;
	}
	else
	{
		float fToCurrent = (m_obSurface - m_obRefPoint).LengthSquared();
		float fToNew = (obSurface - m_obRefPoint).LengthSquared();
		
		if(bNear ? (fToNew < fToCurrent) : (fToNew > fToCurrent))
			SetResults(obSurface,obNormal);
		return true;
	}
}

/***************************************************************************************************
*	
*	FUNCTION		CamVolumeBox Constructor
*
***************************************************************************************************/
CamVolumeBox::CamVolumeBox() :
	m_obSrcMat(CONSTRUCT_IDENTITY),
	m_obDimensions(1.0f, 1.0f, 1.0f),
	m_bDirty(true)
{
}

/***************************************************************************************************
*	
*	FUNCTION		CamVolumeBox Constructor
*
***************************************************************************************************/
CamVolumeBox::CamVolumeBox(const CMatrix& obTransform, const CPoint& obDimension) :
	m_obSrcMat(obTransform),
	m_obDimensions(obDimension),
	m_bDirty(true)
{
}

/***************************************************************************************************
*	
*	FUNCTION		CamVolumeBox::PostConstruct
*
*	DESCRIPTION		Finalise our Volume Box by converting exported degree based eulers.
*
***************************************************************************************************/
void CamVolumeBox::PostConstruct()
{
	if((m_obDimensions.X() > 0.0f) && (m_obDimensions.Y() > 0.0f) && (m_obDimensions.Z() > 0.0f))
	{

		CCamUtil::MatrixFromEuler_XYZ(m_obSrcMat,
									  m_obOrientation.X()*DEG_TO_RAD_VALUE,
									  m_obOrientation.Y()*DEG_TO_RAD_VALUE,
									  m_obOrientation.Z()*DEG_TO_RAD_VALUE);
		m_obSrcMat.SetTranslation(m_obPosition);

		m_bDirty = true;
	}
}

/***************************************************************************************************
*	
*	FUNCTION		CamVolumeBox::RebuildMat
*
*	DESCRIPTION		rebuild our matrix
*
***************************************************************************************************/
void CamVolumeBox::RebuildMat() const
{
	m_bDirty = false;
	if((m_obDimensions.X() > 0.0f) && (m_obDimensions.Y() > 0.0f) && (m_obDimensions.Z() > 0.0f))
	{
		CCamUtil::MatrixFromEuler_XYZ(m_obSrcMat,
									  m_obOrientation.X()*DEG_TO_RAD_VALUE,
									  m_obOrientation.Y()*DEG_TO_RAD_VALUE,
									  m_obOrientation.Z()*DEG_TO_RAD_VALUE);

		m_obSrcMat.SetTranslation(m_obPosition);

		m_bDirty = true;
	}

	if(m_bDirty)
	{
		CMatrix obMat(m_obDimensions.X() * 0.5f, 0.0f, 0.0f, 0.0f,
						0.0f, m_obDimensions.Y() * 0.5f, 0.0f, 0.0f,
						0.0f, 0.0f, m_obDimensions.Z() * 0.5f, 0.0f,
						0.0f, 0.0f, 0.0f, 1.0f);
		
		m_obLocalToWorld = obMat * m_obSrcMat;
		m_obWorldToLocal = m_obLocalToWorld.GetFullInverse();
		m_bDirty = false;
	}
}

/***************************************************************************************************
*	
*	FUNCTION		CamVolumeBox::Inside
*
*	DESCRIPTION		returns true if point is within box
*
***************************************************************************************************/
bool	CamVolumeBox::Inside(const CPoint& obPos) const
{
	m_obLastTestPoint = obPos;
	CPoint obResult = obPos * GetWorldToLocal();

	if	(
		(obResult.X() < -1.0f) || (obResult.X() > 1.0f) ||
		(obResult.Y() < -1.0f) || (obResult.Y() > 1.0f) ||
		(obResult.Z() < -1.0f) || (obResult.Z() > 1.0f)
		)
		return false;
	
	return true;
}

/***************************************************************************************************
*	
*	FUNCTION		CamVolumeBox::ClampToSurf
*
*	DESCRIPTION		returns true if point is outside of box and fills in surface and normal
*
***************************************************************************************************/
int CamVolumeBox::GetSurfaceID(const CDirection& obLocalNormal ) const
{

	int iSurfaceIDResult = 
		obLocalNormal.X()>0.0f ? 2 : (obLocalNormal.X()<0.0f ? 1 : 0);

	iSurfaceIDResult*=3;
	iSurfaceIDResult+= obLocalNormal.Y()>0.0f ? 2 : (obLocalNormal.Y()<0.0f ? 1 : 0);
	iSurfaceIDResult*=3;
	iSurfaceIDResult+= obLocalNormal.Z()>0.0f ? 2 : (obLocalNormal.Z()<0.0f ? 1 : 0);

	return iSurfaceIDResult;
}

/***************************************************************************************************
*	
*	FUNCTION		CamVolumeBox::ClampToSurf
*
*	DESCRIPTION		returns true if point is outside of box and fills in surface and normal
*
***************************************************************************************************/
bool	CamVolumeBox::ClampToSurf(const CPoint& obPos, 
								  const CLAMP_ENV_INFO& obCInfo, 
								  CIntersectResults& obIRResult) const
{
	UNUSED(obCInfo);
	ntAssert(!Inside(obPos));

	m_obLastTestPoint = obPos;

	CPoint obResult = obPos * GetWorldToLocal();

	// clamp the point to the volume surface
	obResult.X() = ntstd::Clamp(obResult.X(), -1.0f, 1.0f);
	obResult.Y() = ntstd::Clamp(obResult.Y(), -1.0f, 1.0f);
	obResult.Z() = ntstd::Clamp(obResult.Z(), -1.0f, 1.0f);
	CPoint obSurf = obResult * GetLocalToWorld();

	CDirection obNormal(CONSTRUCT_CLEAR);

	// normal is determined by largest coordinate
	if(fabsf(obResult.X()) > fabsf(obResult.Y()))
		obNormal = (fabsf(obResult.X()) > fabsf(obResult.Z())) ?
					CDirection(obResult.X(), 0.0f, 0.0f) : CDirection(0.0f, 0.0f, obResult.Z());
	else
		obNormal = (fabsf(obResult.Y()) > fabsf(obResult.Z())) ?
					CDirection(0.0f, obResult.Y(), 0.0f) : CDirection(0.0f, 0.0f, obResult.Z());

	obIRResult.m_obInfo.m_iSurfaceID = GetSurfaceID(obNormal);
	obIRResult.m_obInfo.m_pobVolume = this;

	obNormal = obNormal * GetRotTrans();
	obNormal.Normalise();

	obIRResult.SetResults(obSurf,obNormal);


	return true;
}

/***************************************************************************************************
*	
*	FUNCTION		CamVolumeBox::ExClampToSurf
*
*	DESCRIPTION		returns true if point is outside of a box (acting as an exclusion volume), and returns the 
*									nearest face and corresponding normal
*
***************************************************************************************************/
bool	CamVolumeBox::ExClampToSurf(const CPoint& obPos, 
									const CLAMP_ENV_INFO& obCInfo, 
									CIntersectResults& obIRResult) const
{
	UNUSED(obCInfo);
	m_obLastTestPoint = obPos;
	ntAssert(Inside(obPos));

	CPoint obResult = obPos * GetWorldToLocal();
	CDirection obNormal(CONSTRUCT_CLEAR);

	const CPoint obAbsVals = obResult.Abs();
	const float fXVal = obAbsVals.X();
	const float fYVal = obAbsVals.Y();
	const float fZVal = obAbsVals.Z();

	if ((fXVal+fYVal+fZVal)<(0.000001f))
	{
		// pathological case when the values are too small,
		//  or all zero, to have any meaningfull result.
		obIRResult.SetResults(CPoint(1.0f,0.0f,0.0f),CDirection(1.0f, 0.0f, 0.0f));
	}
	else
	{
		if (obCInfo.m_pobPOIPlaneNormal==NULL)
		{
			if ( fXVal > fYVal )
			{
				if ( fXVal > fZVal )
				{
					obResult.X() =	obResult.X()/fXVal;
					obNormal = 	CDirection(obResult.X(), 0.0f, 0.0f);
				}
				else
				{
					obResult.Z() =	obResult.Z()/fZVal;
					obNormal = 	CDirection(0.0f, 0.0f, obResult.Z());
				}
			}
			else
			{
				if ( fYVal > fZVal )
				{
					obResult.Y() =	obResult.Y()/fYVal;
					obNormal = 	CDirection(0.0f, obResult.Y(), 0.0f);
				}
				else
				{
					obResult.Z() =	obResult.Z()/fZVal;
					obNormal = 	CDirection(0.0f, 0.0f, obResult.Z());
				}
			}
		}
		else
		{
			// normal is determined by largest coordinate
			// projected result is current point projected onto nearest face (keeping proportions of other components)
			// scee.sbashow: todo, am going to use planenormal, to filter out upwards directions in this.
			if ( fXVal > fYVal )
			{
				if ( fXVal > fZVal )
				{
					obResult*=	1.0f/fXVal;
					obNormal = 	CDirection(obResult.X(), 0.0f, 0.0f);
				}
				else
				{
					obResult*=	1.0f/fZVal;
					obNormal = 	CDirection(0.0f, 0.0f, obResult.Z());
				}
			}
			else
			{
				if ( fYVal > fZVal )
				{
					obResult*=	1.0f/fYVal;
					obNormal = 	CDirection(0.0f, obResult.Y(), 0.0f);
				}
				else
				{
					obResult*=	1.0f/fZVal;
					obNormal = 	CDirection(0.0f, 0.0f, obResult.Z());
				}
			}
		}
	}

	obNormal = obNormal * GetRotTrans();
	obNormal.Normalise();
	obNormal = -obNormal;

	obIRResult.SetResults( obResult * GetLocalToWorld(), obNormal);

	return true;

}


/***************************************************************************************************
*	
*	FUNCTION		CamVolumeBox::Intersect
*
*	DESCRIPTION		returns true if line segment intersects the box surface
*
***************************************************************************************************/
bool	CamVolumeBox::Intersect(const CPoint& obStart, const CPoint& obEnd) const
{
	static CPlane aobFaces[] = 
	{
		CPlane(CDirection(1.0f, 0.0f, 0.0f), 1.0f),
		CPlane(CDirection(0.0f, 1.0f, 0.0f), 1.0f),
		CPlane(CDirection(0.0f, 0.0f, 1.0f), 1.0f),
		CPlane(CDirection(-1.0f, 0.0f, 0.0f), 1.0f),
		CPlane(CDirection(0.0f,-1.0f, 0.0f), 1.0f),
		CPlane(CDirection(0.0f, 0.0f,-1.0f), 1.0f)
	};
	
	CPoint obLocalStart(obStart * GetWorldToLocal());
	CPoint obLocalEnd(obEnd * GetWorldToLocal());

	CPoint obTemp;
	for(int i = 0; i < 6; i++)
	{
		if(aobFaces[i].IntersectsLineSeg(obLocalStart,obLocalEnd,obTemp))
		{
			// check bounds
			if	(
				((obTemp.X() < -1.0f) || (obTemp.X() > 1.0f)) ||
				((obTemp.Y() < -1.0f) || (obTemp.Y() > 1.0f)) ||
				((obTemp.Z() < -1.0f) || (obTemp.Z() > 1.0f))
				)
				continue;

			return true;
		}
	}
	return false;
}

/***************************************************************************************************
*	
*	FUNCTION		CamVolumeBox::GetIntersectAndNormal
*
*	DESCRIPTION		returns true if line segment intersects the box surface
*					returns the closest intersection point to start if true, and the normal at that point
*
***************************************************************************************************/
bool	CamVolumeBox::GetIntersectAndNormal(const CPoint& obStart, 
											const CPoint& obEnd, 
											CIntersectResults& obIRResult,
											const CamVolumeSet* pobParentSet) const
{
	static CPlane aobFaces[] = 
	{
		CPlane(CDirection(1.0f, 0.0f, 0.0f), 1.0f),
		CPlane(CDirection(0.0f, 1.0f, 0.0f), 1.0f),
		CPlane(CDirection(0.0f, 0.0f, 1.0f), 1.0f),
		CPlane(CDirection(-1.0f, 0.0f, 0.0f), 1.0f),
		CPlane(CDirection(0.0f,-1.0f, 0.0f), 1.0f),
		CPlane(CDirection(0.0f, 0.0f,-1.0f), 1.0f)
	};

	CPoint obLocalStart(obStart * GetWorldToLocal());
	CPoint obLocalRef(obIRResult.GetRefPos() * GetWorldToLocal());
	CPoint obLocalEnd(obEnd * GetWorldToLocal());

	CPoint obTemp;
	CIntersectResults obResults(obLocalRef);

	for(int i = 0; i < 6; i++)
	{
		if(aobFaces[i].IntersectsLineSeg(obLocalStart,obLocalEnd,obTemp))
		{
			// check bounds
			// scee.sbashow: ?? this is unstable, as one is on the surface.
			//if	(
				//((obTemp.X() < -1.0f) || (obTemp.X() > 1.0f)) ||
				//((obTemp.Y() < -1.0f) || (obTemp.Y() > 1.0f)) ||
				//((obTemp.Z() < -1.0f) || (obTemp.Z() > 1.0f))
				//)
				//continue;

			// scee.sbashow: so make sure inside the cutting face, before doing boundary tests...
			const CPoint obTempInsideCuttingFace = obTemp - aobFaces[i].GetNormal();

			if (((obTempInsideCuttingFace.X() < -1.0f) || (obTempInsideCuttingFace.X() > 1.0f)) ||
				((obTempInsideCuttingFace.Y() < -1.0f) || (obTempInsideCuttingFace.Y() > 1.0f)) ||
				((obTempInsideCuttingFace.Z() < -1.0f) || (obTempInsideCuttingFace.Z() > 1.0f))
				)
				continue;

			if (pobParentSet && 
				!pobParentSet->IntersectPointValid(obTemp*GetLocalToWorld()))
				continue;

			if(obResults.SetIfNearFar(obTemp, aobFaces[i].GetNormal(), true))
				break;
		}
	}

	if(obResults.IsValid())
	{
		const CDirection obNormal =  obResults.GetNormal();

		CDirection obNorm = obNormal * GetRotTrans();
		obNorm.Normalise();
        obIRResult.SetResults(obResults.GetSurface() * GetLocalToWorld(),obNorm);

		// use local normal, obNormal, to determine id.
		obIRResult.m_obInfo.m_iSurfaceID = GetSurfaceID(obNormal);
		obIRResult.m_obInfo.m_pobVolume = this;


		return true;
	}
	return false;
}

/***************************************************************************************************
*	
*	FUNCTION		CamVolumeBox::GetRandomPoint
*
*	DESCRIPTION		generates a random point within the box
*
***************************************************************************************************/
void CamVolumeBox::GetRandomPoint(CPoint& obPos) const
{	
	obPos.X() = grandf(2.0f) - 1.0f;
	obPos.Y() = grandf(2.0f) - 1.0f;
	obPos.Z() = grandf(2.0f) - 1.0f;

	obPos = obPos * GetLocalToWorld();
}

/***************************************************************************************************
*	
*	FUNCTION		CamVolumeBox::Render
*
*	DESCRIPTION		display of the box
*
***************************************************************************************************/
void CamVolumeBox::Render(float fRed, float fGreen, float fBlue, float fAlpha) const
{
	if(Inside(m_obLastTestPoint))
	{
		fGreen = 0.0f; fBlue = 0.0f;
	}

	CPoint aobCorners[] = 
	{
		 CPoint(-1.0f, -1.0f, 1.0f),
		 CPoint(-1.0f, 1.0f, 1.0f),
		 CPoint(1.0f, 1.0f, 1.0f),
		 CPoint(1.0f, -1.0f, 1.0f),
		 CPoint(-1.0f, -1.0f, -1.0f),
		 CPoint(-1.0f, 1.0f, -1.0f),
		 CPoint(1.0f, 1.0f, -1.0f),
		 CPoint(1.0f, -1.0f, -1.0f)
	};

	for(int i = 0; i < 8; i++)
		aobCorners[i] = aobCorners[i] * GetLocalToWorld();

	for(int i = 0; i < 4; i++)
	{
		int iWrapped = i + 1; if(iWrapped == 4) iWrapped = 0;

		CCamUtil::Render_Line(aobCorners[i], aobCorners[iWrapped], fRed, fGreen, fBlue, fAlpha);
		CCamUtil::Render_Line(aobCorners[i+4], aobCorners[iWrapped+4], fRed, fGreen, fBlue, fAlpha);
		CCamUtil::Render_Line(aobCorners[i], aobCorners[i+4], fRed, fGreen, fBlue, fAlpha);
	}
}

void CamVolumeBox::Render(bool /*bParentSelected*/, bool /*bSiblingSelected*/) const
{
	/*
	if(IsSelected() || bParentSelected)
		Render(1.0f, 1.0f, 1.0f, 1.0f);
	else if(bSiblingSelected)
		Render(0.75f, 0.75f, 0.75f, 0.75f);
	*/
}

/***************************************************************************************************
*	
*	FUNCTION		CamVolumeBox::Render
*
*	DESCRIPTION		render stuff
*
***************************************************************************************************/
void CamVolumeBox::RenderInfo(int iX, int iY) const
{
	CCamUtil::DebugPrintf(iX, iY, "CamVolumeBox: %s ",  ntStr::GetString(ObjectDatabase::Get().GetNameFromPointer( this )));
}

/***************************************************************************************************
*	
*	FUNCTION		CamVolumeBox::Render
*
*	DESCRIPTION		render stuff
*
***************************************************************************************************/
void CamVolumeBox::DebugRender ()
{
#ifndef _GOLD_MASTER

	float fRed,fGreen,fBlue,fAlpha = 1.0f;

	if(Inside(m_obLastTestPoint))
	{
		fRed = 1.0f;
		fGreen = 0.0f;
		fBlue = 0.0f;
	}
	else
	{
		fRed = 1.0f;
		fGreen = 1.0f;
		fBlue = 1.0f;		
	}

	CPoint aobCorners[] = 
	{
		 CPoint(-1.0f, -1.0f, 1.0f),
		 CPoint(-1.0f, 1.0f, 1.0f),
		 CPoint(1.0f, 1.0f, 1.0f),
		 CPoint(1.0f, -1.0f, 1.0f),
		 CPoint(-1.0f, -1.0f, -1.0f),
		 CPoint(-1.0f, 1.0f, -1.0f),
		 CPoint(1.0f, 1.0f, -1.0f),
		 CPoint(1.0f, -1.0f, -1.0f)
	};

	for(int i = 0; i < 8; i++)
		aobCorners[i] = aobCorners[i] * GetLocalToWorld();

	for(int i = 0; i < 4; i++)
	{
		int iWrapped = i + 1; if(iWrapped == 4) iWrapped = 0;

		CCamUtil::Render_Line(aobCorners[i], aobCorners[iWrapped], fRed, fGreen, fBlue, fAlpha);
		CCamUtil::Render_Line(aobCorners[i+4], aobCorners[iWrapped+4], fRed, fGreen, fBlue, fAlpha);
		CCamUtil::Render_Line(aobCorners[i], aobCorners[i+4], fRed, fGreen, fBlue, fAlpha);
	}

	// Some debug information
	g_VisualDebug->Printf3D( GetLocalToWorld().GetTranslation(), NTCOLOUR_FROM_FLOATS(fRed,fGreen,fBlue,fAlpha), DTF_ALIGN_HCENTRE, "CamVolumeBox: %s",  ntStr::GetString(ObjectDatabase::Get().GetNameFromPointer( this )));

#endif
}





/***************************************************************************************************
*	
*	FUNCTION		CamVolumeSphere Constructor
*
***************************************************************************************************/
CamVolumeSphere::CamVolumeSphere() :
	m_obPosition(CONSTRUCT_CLEAR),
	m_fRadius(1.0f)
{
}

/***************************************************************************************************
*	
*	FUNCTION		CamVolumeSphere Constructor
*
***************************************************************************************************/
CamVolumeSphere::CamVolumeSphere(const CPoint& obPosition, float fRadius) :
	m_obPosition(obPosition),
	m_fRadius(fRadius)
{
}


/***************************************************************************************************
*	
*	FUNCTION		CamVolumeSphere::Inside
*
*	DESCRIPTION		returns true if point is within sphere
*
***************************************************************************************************/
bool	CamVolumeSphere::Inside(const CPoint& obPos) const
{
	m_obLastTestPoint = obPos;

	CPoint obResult(obPos);
	obResult -= GetPosition();

	if(obResult.LengthSquared() <= GetRadiusSq())
		return true;
	else
		return false;
}

/***************************************************************************************************
*	
*	FUNCTION		CamVolumeSphere::ExClampToSurf
*
*	DESCRIPTION		sets the surface and normal for a clamp from a point outside
*
***************************************************************************************************/
bool	CamVolumeSphere::ClampToSurf(const CPoint& obPos, 
									 const CLAMP_ENV_INFO& obCInfo, 
									 CIntersectResults& obIRResult) const
{
	UNUSED(obCInfo);
	m_obLastTestPoint = obPos;

	ntAssert(!Inside(obPos));

	CPoint obResult(obPos);
	obResult -= GetPosition();

	CDirection obNormal = CDirection(obResult);
	obNormal.Normalise();
	CPoint obSurf = (obNormal * GetRadius()) + GetPosition();

	obIRResult.SetResults(obSurf,obNormal);

	obIRResult.m_obInfo.m_iSurfaceID = 0;
	obIRResult.m_obInfo.m_pobVolume = this;

	return true;
}

/***************************************************************************************************
*	
*	FUNCTION		CamVolumeSphere::ExClampToSurf
*
*	DESCRIPTION		sets the surface and normal for a clamp from a point inside
*
***************************************************************************************************/
bool	CamVolumeSphere::ExClampToSurf(const CPoint& obPos, 
									 const CLAMP_ENV_INFO& obCInfo,
									 CIntersectResults& obIRResult) const
{
	UNUSED(obCInfo);
	m_obLastTestPoint = obPos;

	ntAssert(Inside(obPos));


	CPoint obResult(obPos);
	obResult -= GetPosition();

	CDirection obNormal = CDirection(obResult);
	obNormal.Normalise();
	obIRResult.SetResults((obNormal * GetRadius()) + GetPosition(),-obNormal);

	obIRResult.m_obInfo.m_iSurfaceID = 0;
	obIRResult.m_obInfo.m_pobVolume = this;

	return true;
}

/***************************************************************************************************
*	
*	FUNCTION		CamVolumeSphere::Intersect
*
*	DESCRIPTION		returns true if line segment intersects the sphere surface
*
***************************************************************************************************/
bool	CamVolumeSphere::Intersect(const CPoint& obStart, const CPoint& obEnd) const
{
	CDirection obLineSeg(obEnd ^ obStart);
	CDirection obToStart(obStart ^ GetPosition());

	float fA = obLineSeg.LengthSquared();
	float fB = 2.0f * obLineSeg.Dot(obToStart);
	float fC = GetPosition().LengthSquared() + obStart.LengthSquared() - (2.0f * GetPosition().Dot(obStart)) - GetRadiusSq();

	float fDiscriminant = (fB * fB) - (4 * fA * fC);

	// no results
	//-----------
	if(fDiscriminant < 0.0f) 
		return false;

	// line is tangetial, may have a result
	//-------------------------------------
	if(fabsf(fDiscriminant) < EPSILON)
	{
		if(fabsf(fA) < EPSILON)
		{
			if((obStart.LengthSquared() - GetRadiusSq()) < EPSILON)
				return true;
			return false;
		}

		float fTime = -fB / (2.0f * fA);
		if((fTime >= 0.0f) && (fTime <= 1.0f))
			return true;
		return false;
	}

	// line crosses sphere, may have 0, 1, or 2 results.
	//-------------------------------------------------
	fDiscriminant = fsqrtf(fDiscriminant);

	float fTime = (-fB + fDiscriminant) / (2.0f * fA);
	if((fTime >= 0.0f) && (fTime <= 1.0f))
		return true;

	fTime = (-fB - fDiscriminant) / (2.0f * fA);
	if((fTime >= 0.0f) && (fTime <= 1.0f))
		return true;

	return false;
}

/***************************************************************************************************
*	
*	FUNCTION		CamVolumeSphere::GetIntersectAndNormal
*
*	DESCRIPTION		returns true if line segment intersects the sphere surface
*					returns the closest intersection point to start if true, and the normal at that point
*
***************************************************************************************************/
bool	CamVolumeSphere::GetIntersectAndNormal(const CPoint& obStart, 
											   const CPoint& obEnd, 
											   CIntersectResults& obIRResult,
											   const CamVolumeSet* pobParentSet) const
{
	CDirection obLineSeg(obEnd ^ obStart);
	CDirection obToStart(obStart ^ GetPosition());

	float fA = obLineSeg.LengthSquared();
	float fB = 2.0f * obLineSeg.Dot(obToStart);
	float fC = GetPosition().LengthSquared() + obStart.LengthSquared() - (2.0f * GetPosition().Dot(obStart)) - GetRadiusSq();

	float fDiscriminant = (fB * fB) - (4 * fA * fC);

	// no results
	//-----------
	if(fDiscriminant < 0.0f) 
		return false;

	// line is tangetial, may have a result
	//-------------------------------------
	if(fabsf(fDiscriminant) < EPSILON)
	{
		if(fabsf(fA) < EPSILON)
		{
			if((obStart.LengthSquared() - GetRadiusSq()) < EPSILON)
			{
				CDirection obNorm = obToStart;
				obNorm.Normalise();
				obIRResult.SetResults(obStart,obNorm);

				obIRResult.m_obInfo.m_iSurfaceID = 0;
				obIRResult.m_obInfo.m_pobVolume = this;

				return true;
			}
			return false;
		}

		float fTime = -fB / (2.0f * fA);
		if((fTime >= 0.0f) && (fTime <= 1.0f))
		{
			CPoint obSurf = obStart + (obLineSeg * fTime);
            CDirection obNorm = obSurf ^ GetPosition();
			obNorm.Normalise();
			obIRResult.SetResults(obSurf,obNorm);

			obIRResult.m_obInfo.m_iSurfaceID = 0;
			obIRResult.m_obInfo.m_pobVolume = this;

			return true;
		}
		return false;
	}

	CIntersectResults obResults(obIRResult.GetRefPos());

	// line crosses sphere, may have 0, 1, or 2 results.
	//-------------------------------------------------
	fDiscriminant = fsqrtf(fDiscriminant);

	// does 1st point lie on line?
	float fTime = (-fB + fDiscriminant) / (2.0f * fA);
	if((fTime >= 0.0f) && (fTime <= 1.0f))
	{
		CPoint obTemp = obStart + (obLineSeg * fTime);

		if (!pobParentSet ||
			pobParentSet->IntersectPointValid(obTemp))
		{
			obResults.SetResults(obTemp, obTemp ^ GetPosition());

			obResults.m_obInfo.m_iSurfaceID = 0;
			obResults.m_obInfo.m_pobVolume = this;
		}
	}

	// does 2nd point lie on line?
	fTime = (-fB - fDiscriminant) / (2.0f * fA);
	if((fTime >= 0.0f) && (fTime <= 1.0f))
	{
		CPoint obTemp = obStart + (obLineSeg * fTime);

		if (!pobParentSet ||
			pobParentSet->IntersectPointValid(obTemp))
		{
			obResults.SetIfNearFar(obTemp, obTemp ^ GetPosition(), true);

			obResults.m_obInfo.m_iSurfaceID = 0;
			obResults.m_obInfo.m_pobVolume = this;
		}
	}

	if(obResults.IsValid())
	{
		CDirection obNorm = obResults.GetNormal();
		obNorm.Normalise();
		obIRResult.SetResults(obResults.GetSurface(),obNorm);

        obIRResult.m_obInfo.m_pobVolume = 
			obResults.GetIRVolumeAndSurface(obIRResult.m_obInfo.m_iSurfaceID);

		return true;
	}
	return false;
}

/***************************************************************************************************
*	
*	FUNCTION		CamVolumeSphere::GetRandomPoint
*
*	DESCRIPTION		generates a random point within the sphere
*
***************************************************************************************************/
void CamVolumeSphere::GetRandomPoint(CPoint& obPos) const
{	
	CDirection obTemp(CONSTRUCT_CLEAR);
	
	obTemp.X() = drandf(1.0f) - 0.5f;
	obTemp.Y() = drandf(1.0f) - 0.5f;
	obTemp.Z() = drandf(1.0f) - 0.5f;
	obTemp.Normalise();
	
	obPos = CPoint(obTemp * GetRadius()) + GetPosition();
}

/***************************************************************************************************
*	
*	FUNCTION		CamVolumeSphere::Render
*
*	DESCRIPTION		display of the sphere
*
***************************************************************************************************/
void CamVolumeSphere::Render(float fRed, float fGreen, float fBlue, float fAlpha) const
{
	if(Inside(m_obLastTestPoint))
	{
		fGreen = 0.0f; fBlue = 0.0f;
	}

	CCamUtil::Render_Sphere(GetPosition(), GetRadius(), fRed, fGreen, fBlue, fAlpha);
}

void CamVolumeSphere::Render(bool /*bParentSelected*/, bool /*bSiblingSelected*/) const
{
	/*
	if(IsSelected() || bParentSelected)
		Render(1.0f, 1.0f, 1.0f, 1.0f);
	else if(bSiblingSelected)
		Render(0.75f, 0.75f, 0.75f, 0.75f);
	*/
}

/***************************************************************************************************
*	
*	FUNCTION		CamVolumeSphere::Render
*
*	DESCRIPTION		render stuff
*
***************************************************************************************************/
void CamVolumeSphere::RenderInfo(int iX, int iY) const
{
	CCamUtil::DebugPrintf(iX, iY, "CamVolumeSphere: %s ",  ntStr::GetString(ObjectDatabase::Get().GetNameFromPointer( this )));
}

/***************************************************************************************************
*	
*	FUNCTION		CamVolumeSphere::DebugRender
*
*	DESCRIPTION		render stuff
*
***************************************************************************************************/
void CamVolumeSphere::DebugRender ()
{
#ifndef _GOLD_MASTER

	float fRed,fGreen,fBlue,fAlpha = 1.0f;

	if(Inside(m_obLastTestPoint))
	{
		fRed = 1.0f;
		fGreen = 0.0f;
		fBlue = 0.0f;
	}
	else
	{
		fRed = 1.0f;
		fGreen = 1.0f;
		fBlue = 1.0f;		
	}

	CCamUtil::Render_Sphere(GetPosition(), GetRadius(), fRed, fGreen, fBlue, fAlpha);

	// Some debug information
	g_VisualDebug->Printf3D( GetPosition(), NTCOLOUR_FROM_FLOATS(fRed,fGreen,fBlue,fAlpha), DTF_ALIGN_HCENTRE, "CamVolumeSphere: %s", ntStr::GetString(ObjectDatabase::Get().GetNameFromPointer( this )));

#endif
}





/***************************************************************************************************
*	
*	FUNCTION		CamVolumeCylinder Constructor
*
***************************************************************************************************/
CamVolumeCylinder::CamVolumeCylinder() :
	m_bDirty(true),
	m_obAxis(CVecMath::GetYAxis()),
	m_fRadius(1.0f),
	m_fHeight(1.0f)
{
	// the position goes at the bottom of the cylinder -on construction-
	// this will be fixed up in OnPostConstruct;
	// The reason for this is that MrEd sets the cylinder position to be the bottom of the cylinder.
	m_obPosition = CPoint(0.0f,-m_fHeight*0.5f,0.0f);
}

/***************************************************************************************************
*	
*	FUNCTION		CamVolumeCylinder Constructor
*
***************************************************************************************************/
CamVolumeCylinder::CamVolumeCylinder(const CPoint& obPosition, const CDirection& obAxis, float fRadius, float fHeight) :
	m_bDirty(true),
	m_obPosition(obPosition),
	m_obAxis(obAxis),
	m_fRadius(fRadius),
	m_fHeight(fHeight)
{
}

void CamVolumeCylinder::PostConstruct()
{
	// the position recentered to the middle of the cylinder, as the rest of the code expects.
	m_obPosition += CDirection(0.0f,m_fHeight*0.5f,0.0f);
}

/***************************************************************************************************
*	
*	FUNCTION		CamVolumeCylinder::RebuildMat
*
*	DESCRIPTION		rebuilds the current matrix set
*
***************************************************************************************************/
void CamVolumeCylinder::RebuildMat() const
{
	if(m_bDirty)
	{
		CPoint obPos = m_obPosition;

		// offset Position by a lookat point that is furthest from our axis in direction.
		if(fabsf(m_obAxis.X()) < fabsf(m_obAxis.Y()))
			obPos += (fabsf(m_obAxis.X()) < fabsf(m_obAxis.Z())) ? CVecMath::GetXAxis() : CVecMath::GetZAxis();
		else
			obPos += (fabsf(m_obAxis.Y()) < fabsf(m_obAxis.Z())) ? CVecMath::GetYAxis() : CVecMath::GetZAxis();

		// construct a lookat matrix with our 'up' as our axis
		CCamUtil::CreateFromPoints(m_obSrcMat, m_obPosition, obPos, m_obAxis);

		// generate a scale matrix for our dimensions
		CMatrix obMat(	m_fRadius, 0.0f, 0.0f, 0.0f,
						0.0f, m_fHeight * 0.5f, 0.0f, 0.0f,
						0.0f, 0.0f, m_fRadius, 0.0f,
						0.0f, 0.0f, 0.0f, 1.0f);
		
		m_obLocalToWorld = obMat * m_obSrcMat;
		m_obWorldToLocal = m_obLocalToWorld.GetFullInverse();
		m_bDirty = false;
	}
}

/***************************************************************************************************
*	
*	FUNCTION		CamVolumeCylinder::Inside
*
*	DESCRIPTION		returns true if point is within cylinder
*
***************************************************************************************************/
bool	CamVolumeCylinder::Inside(const CPoint& obPos) const
{
	m_obLastTestPoint = obPos;

	CPoint obResult = obPos * GetWorldToLocal();

	const float fYVal = obResult.Y();

	if((fYVal*fYVal)<= 1.0f)
	{
		obResult.Y() = 0.0f;
		if(obResult.LengthSquared() <= 1.0f)
			return true;
	}

	return false;
}

/***************************************************************************************************
*	
*	FUNCTION		CamVolumeCylinder::ClampToSurf
*
*	DESCRIPTION		sets the surface and normal for a clamp from a point outside
*
***************************************************************************************************/
bool	CamVolumeCylinder::ClampToSurf(const CPoint& obPos, 
									   const CLAMP_ENV_INFO& obCInfo, 
									   CIntersectResults& obIRResult) const
{
	UNUSED(obCInfo);
	m_obLastTestPoint = obPos;

	ntAssert(!Inside(obPos));

	CPoint obTemp = obPos * GetWorldToLocal();

	const float fOldY = obTemp.Y();
	obTemp.Y()=0.0f;

	// clamp x and z to be on the surface of the cylinder
	CDirection obResult(obTemp);
	obResult.Normalise();
	CDirection obNormal(CONSTRUCT_CLEAR);

	const bool bTop = (fOldY > 1.0f);
	const bool bBot = (fOldY < -1.0f);

	// get the normal ant set Y coord
	if((bTop) || (bBot))
	{
		obResult.Y() = ntstd::Clamp(fOldY, -1.0f, 1.0f);
		obNormal = CDirection(0.0f, obResult.Y(), 0.0f);
	}
	else
	{
		obNormal = obResult;
		obResult.Y() = fOldY;
	}

	CPoint obSurf = CPoint(obResult) * GetLocalToWorld();
	obNormal = obNormal * GetRotTrans();
	obNormal.Normalise();

	obIRResult.SetResults(obSurf,obNormal);

	obIRResult.m_obInfo.m_iSurfaceID = bTop ? 1 : (bBot ? 2 : 0);
	obIRResult.m_obInfo.m_pobVolume = this;

	return true;
}

/***************************************************************************************************
*	
*	FUNCTION		CamVolumeCylinder::ExClampToSurf
*
*	DESCRIPTION		sets the surface and normal for a clamp for a point inside
*
***************************************************************************************************/
bool	CamVolumeCylinder::ExClampToSurf(const CPoint& obPos, 
									   const CLAMP_ENV_INFO& obCInfo,
									   CIntersectResults& obIRResult) const
{
	UNUSED(obCInfo);
	m_obLastTestPoint = obPos;

	ntAssert(Inside(obPos));

	CPoint obTemp = obPos * GetWorldToLocal();

	const float fobTempLSqr = obTemp.LengthSquared();

	const float fOldY = obTemp.Y();
	obTemp.Y()=0.0f;

	// clamp x and z to be on the surface of the cylinder
	CDirection obResult(obTemp);

	CDirection obNormal(CONSTRUCT_CLEAR);

	bool bTop = false;
	bool bBot = false;

	if (fobTempLSqr<=sqr(fOldY))
	{
		bTop = fOldY > 0.0f;
		bBot = !bTop;
		const float fNewY = bTop?1.0f:-1.0f;

		obNormal = CDirection(0.0f,fNewY,0.0f);
		obResult.Y() = fNewY;
	}
	else
	{
		obResult.Normalise();
		obNormal = obResult;
		obResult.Y() = fOldY;
	}

	obNormal = obNormal * GetRotTrans();
	obNormal.Normalise();

	obIRResult.SetResults(CPoint(obResult) * GetLocalToWorld(),-obNormal);

	obIRResult.m_obInfo.m_iSurfaceID = bTop ? 1 : (bBot ? 2 : 0);
	obIRResult.m_obInfo.m_pobVolume = this;

	return true;
}


/***************************************************************************************************
*	
*	FUNCTION		CamVolumeCylinder::Intersect
*
*	DESCRIPTION		returns true if line segment intersects the cylinder surface
*
***************************************************************************************************/
bool	CamVolumeCylinder::Intersect(const CPoint& obStart, const CPoint& obEnd) const
{
	static CPlane obTop(CDirection(0.0f, 1.0f, 0.0f), 1.0f);
	static CPlane obBot(CDirection(0.0f, -1.0f, 0.0f), 1.0f);
	
	CPoint obLocalStart(obStart * GetWorldToLocal());
	CPoint obLocalEnd(obEnd * GetWorldToLocal());

	// check top and bottom
	CPoint obTemp;
	if(obTop.IntersectsLineSeg(obLocalStart,obLocalEnd,obTemp))
	{
		obTemp.Y() = 0.0f;
		if(obTemp.LengthSquared() <= 1.0f)
			return true;
	}

	if(obBot.IntersectsLineSeg(obLocalStart,obLocalEnd,obTemp))
	{
		obTemp.Y() = 0.0f;
		if(obTemp.LengthSquared() <= 1.0f)
			return true;
	}

	// check sides
	static CCircle obTestCircle(0.0f,0.0f,1.0f);
	C2DLineSegment obTestLine(obLocalStart.X(), obLocalStart.Z(), obLocalEnd.X(), obLocalEnd.Z());

	if(obTestLine.IsPoint()) // we're aligned with the cylinder, no more intersections
		return false;

	CPoint obTemp1, obTemp2;
	CCircle::INTERSECT_TYPE eType = obTestCircle.GetIntersection(obTestLine, obTemp1, obTemp2);

	if(eType != CCircle::NO_INTERSECT)
	{
		// we have one or two hits
		float fTime = obTestLine.ProjectPoint(obTemp1.X(), obTemp1.Y());
		float fYTest = obLocalStart.Y() + (fTime * (obLocalEnd.Y() - obLocalStart.Y()));
		
		if(fabsf(fYTest)<= 1.0f)
			return true;

		if(eType == CCircle::INTERSECTS_2)
		{
			float fTime = obTestLine.ProjectPoint(obTemp2.X(), obTemp2.Y());
			float fYTest = obLocalStart.Y() + (fTime * (obLocalEnd.Y() - obLocalStart.Y()));

			if(fabsf(fYTest)<= 1.0f)
				return true;
		}
	}
	return false;
}

/***************************************************************************************************
*	
*	FUNCTION		CamVolumeCylinder::GetIntersectAndNormal
*
*	DESCRIPTION		returns true if line segment intersects the cylinder surface
*					returns the closest intersection point to start if true, and the normal at that point
*
***************************************************************************************************/
bool	CamVolumeCylinder::GetIntersectAndNormal(const CPoint& obStart,  
												 const CPoint& obEnd, 
												 CIntersectResults& obIRResult,
												 const CamVolumeSet* pobParentSet) const
{
	static CCircle obTestCircle(0.0f,0.0f,1.0f);
	static CPlane obTop(CDirection(0.0f, 1.0f, 0.0f), 1.0f);
	static CPlane obBot(CDirection(0.0f, -1.0f, 0.0f), 1.0f);
	
	CPoint obLocalStart(obStart * GetWorldToLocal());
	CPoint obLocalRef(obIRResult.GetRefPos() * GetWorldToLocal());
	CPoint obLocalEnd(obEnd * GetWorldToLocal());

	CPoint obTemp;
	CIntersectResults obResults(obLocalRef);

	// check top plane
	if	(
		(obTop.IntersectsLineSeg(obLocalStart,obLocalEnd,obTemp)) &&		// intersects top plane
		(CPoint(obTemp.X(), 0.0f, obTemp.Z()).LengthSquared() <= 1.0f)	&& // within radius
		(!pobParentSet || pobParentSet->IntersectPointValid(obTemp*GetLocalToWorld()))
		)
	{
		obResults.SetResults(obTemp,obTop.GetNormal());

		obResults.m_obInfo.m_iSurfaceID = 1;
		obResults.m_obInfo.m_pobVolume = this;
	}

	// check bottom plane
	if	(
		(obBot.IntersectsLineSeg(obLocalStart,obLocalEnd,obTemp)) &&			// intersects bottom plane
		(CPoint(obTemp.X(), 0.0f, obTemp.Z()).LengthSquared() <= 1.0f) &&		// within radius
		(!pobParentSet || pobParentSet->IntersectPointValid(obTemp*GetLocalToWorld())) &&		// 
		(obResults.SetIfNearFar(obTemp, obBot.GetNormal(), true))	// is second point
		)
	{
		CDirection obNorm = obResults.GetNormal() * GetRotTrans();
		obNorm.Normalise();
		obIRResult.SetResults(obResults.GetSurface() * GetLocalToWorld(),obNorm);

		obIRResult.m_obInfo.m_iSurfaceID = 2;
		obIRResult.m_obInfo.m_pobVolume = this;

		return true;
	}

	// check sides
	C2DLineSegment obTestLine(obLocalStart.X(), obLocalStart.Z(), obLocalEnd.X(), obLocalEnd.Z());

	if(!obTestLine.IsPoint()) // we're aligned with the cylinder, no more intersections
	{
		CPoint obTemp1, obTemp2;

		CCircle::INTERSECT_TYPE eType = obTestCircle.GetIntersection(obTestLine, obTemp1, obTemp2);

		if(eType!= CCircle::NO_INTERSECT)	// we have one or two hits
		{
			// get actual 3D intersect point and normal of 1st point
			obTemp = obLocalStart + (obTestLine.ProjectPoint(obTemp1.X(), obTemp1.Y()) * (obLocalEnd ^ obLocalStart));
			CDirection obNormal(obTemp1.X(), 0.0f, obTemp1.Y());

			if	(
				(fabsf(obTemp.Y()) <= 1.0f) &&								// is it within our height range?
				(!pobParentSet || pobParentSet->IntersectPointValid(obTemp*GetLocalToWorld()) )&&	
				(obResults.SetIfNearFar(obTemp, obNormal, true))	// is second point
				)
			{
				CDirection obNorm = obResults.GetNormal() * GetRotTrans();
				obNorm.Normalise();
				obIRResult.SetResults(obResults.GetSurface() * GetLocalToWorld(),obNorm);

				obIRResult.m_obInfo.m_iSurfaceID = 0;
				obIRResult.m_obInfo.m_pobVolume = this;

				return true;
			}

			if(eType == CCircle::INTERSECTS_2)
			{
				// get actual 3D intersect point and normal of 2nd point
				obTemp = obLocalStart + (obTestLine.ProjectPoint(obTemp2.X(), obTemp2.Y()) * (obLocalEnd ^ obLocalStart));
				CDirection obNormal(obTemp2.X(), 0.0f, obTemp2.Y());

				if ((fabsf(obTemp.Y()) <= 1.0f) &&								// is it within our height range?
					(!pobParentSet || pobParentSet->IntersectPointValid(obTemp*GetLocalToWorld()) )
					)
				{
					obResults.SetIfNearFar(obTemp, obNormal, true);

					obResults.m_obInfo.m_iSurfaceID = 0;
					obResults.m_obInfo.m_pobVolume = this;
				}
			}
		}
	}

	if(obResults.IsValid())
	{
		CDirection obNorm = obResults.GetNormal() * GetRotTrans();
		obNorm.Normalise();

		obIRResult.SetResults(obResults.GetSurface() * GetLocalToWorld(),obNorm);
		obIRResult.m_obInfo.m_pobVolume = 
			obResults.GetIRVolumeAndSurface(obIRResult.m_obInfo.m_iSurfaceID);
        
		return true;
	}
	return false;
}

/***************************************************************************************************
*	
*	FUNCTION		CamVolumeCylinder::GetRandomPoint
*
*	DESCRIPTION		generates a random point within the cylinder
*
***************************************************************************************************/
void CamVolumeCylinder::GetRandomPoint(CPoint& obPos) const
{	
	float fAngle = grandf(TWO_PI);
	obPos.X() = fsinf(fAngle);
	obPos.Y() = drandf(2.0f) - 1.0f;
	obPos.Z() = fcosf(fAngle);

	obPos = obPos * GetLocalToWorld();
}

/***************************************************************************************************
*	
*	FUNCTION		CamVolumeCylinder::Render
*
*	DESCRIPTION		display of the cylinder
*
***************************************************************************************************/
void CamVolumeCylinder::Render(float fRed, float fGreen, float fBlue, float fAlpha) const
{
	if(Inside(m_obLastTestPoint))
	{
		fGreen = 0.0f; fBlue = 0.0f;
	}

	static const int iNUM_SEGS = 20;

	for(int i = 0; i < iNUM_SEGS; i++)
	{
		float fAngleOld = (i * TWO_PI) / iNUM_SEGS;
		float fAngleNew = ((i+1) * TWO_PI) / iNUM_SEGS;
		
		CPoint obPos1 = CPoint(fsinf(fAngleOld), 1.0f, fcosf(fAngleOld)) * GetLocalToWorld();
		CPoint obPos2 = CPoint(fsinf(fAngleNew), 1.0f, fcosf(fAngleNew)) * GetLocalToWorld();
		CPoint obPos3 = CPoint(fsinf(fAngleOld), -1.0f, fcosf(fAngleOld)) * GetLocalToWorld();
		CPoint obPos4 = CPoint(fsinf(fAngleNew), -1.0f, fcosf(fAngleNew)) * GetLocalToWorld();

		CCamUtil::Render_Line(obPos1, obPos2, fRed, fGreen, fBlue, fAlpha);
		CCamUtil::Render_Line(obPos3, obPos4, fRed, fGreen, fBlue, fAlpha);
		CCamUtil::Render_Line(obPos2, obPos4, fRed, fGreen, fBlue, fAlpha);
	}
}


void CamVolumeCylinder::Render(bool /*bParentSelected*/, bool /*bSiblingSelected*/) const
{
	/*
	if(IsSelected() || bParentSelected)
		Render(1.0f, 1.0f, 1.0f, 1.0f);
	else if(bSiblingSelected)
		Render(0.75f, 0.75f, 0.75f, 0.75f);
	*/
}

/***************************************************************************************************
*	
*	FUNCTION		CamVolumeCylinder::Render
*
*	DESCRIPTION		render stuff
*
***************************************************************************************************/
void CamVolumeCylinder::RenderInfo(int iX, int iY) const
{
	CCamUtil::DebugPrintf(iX, iY, "CamVolumeCylinder: %s ",  ntStr::GetString(ObjectDatabase::Get().GetNameFromPointer( this )));
}

/***************************************************************************************************
*	
*	FUNCTION		CamVolumeCylinder::DebugRender
*
*	DESCRIPTION		render stuff
*
***************************************************************************************************/
void CamVolumeCylinder::DebugRender ()
{
#ifndef _GOLD_MASTER

	float fRed,fGreen,fBlue,fAlpha = 1.0f;

	if(Inside(m_obLastTestPoint))
	{
		fRed = 1.0f;
		fGreen = 0.0f;
		fBlue = 0.0f;
	}
	else
	{
		fRed = 1.0f;
		fGreen = 1.0f;
		fBlue = 1.0f;		
	}

	static const int iNUM_SEGS = 20;

	for(int i = 0; i < iNUM_SEGS; i++)
	{
		float fAngleOld = (i * TWO_PI) / iNUM_SEGS;
		float fAngleNew = ((i+1) * TWO_PI) / iNUM_SEGS;
		
		CPoint obPos1 = CPoint(fsinf(fAngleOld), 1.0f, fcosf(fAngleOld)) * GetLocalToWorld();
		CPoint obPos2 = CPoint(fsinf(fAngleNew), 1.0f, fcosf(fAngleNew)) * GetLocalToWorld();
		CPoint obPos3 = CPoint(fsinf(fAngleOld), -1.0f, fcosf(fAngleOld)) * GetLocalToWorld();
		CPoint obPos4 = CPoint(fsinf(fAngleNew), -1.0f, fcosf(fAngleNew)) * GetLocalToWorld();

		CCamUtil::Render_Line(obPos1, obPos2, fRed, fGreen, fBlue, fAlpha);
		CCamUtil::Render_Line(obPos3, obPos4, fRed, fGreen, fBlue, fAlpha);
		CCamUtil::Render_Line(obPos2, obPos4, fRed, fGreen, fBlue, fAlpha);
	}

	// Some debug information
	g_VisualDebug->Printf3D(GetLocalToWorld().GetTranslation(), NTCOLOUR_FROM_FLOATS(fRed,fGreen,fBlue,fAlpha), DTF_ALIGN_HCENTRE, "CamVolumeCylinder: %s", ntStr::GetString(ObjectDatabase::Get().GetNameFromPointer( this )));

#endif
}













/***************************************************************************************************
*	
*	FUNCTION		CamVolumeSet constructor
*
***************************************************************************************************/
CamVolumeSet::CamVolumeSet(): 	m_eIntersectMode(ITSCT_MODE_NONE),
								m_pobExcluded(NULL)
{
}

/***************************************************************************************************
*	
*	FUNCTION		CamVolumeSet destructor
*
***************************************************************************************************/
CamVolumeSet::~CamVolumeSet()
{
	/*
	while(!m_obVolumes.empty())
	{
		CamVolume::Destroy(const_cast< CamVolume* >(m_obVolumes.back()));
		m_obVolumes.pop_back();
	}

	while(!m_obExVolumes.empty())
	{
		CamVolume::Destroy(const_cast< CamVolume* >(m_obExVolumes.back()));
		m_obExVolumes.pop_back();
	}
	*/
}

/***************************************************************************************************
*	
*	FUNCTION		CamVolumeSet::IsPointIncluded
*
*	DESCRIPTION		returns true if point is within the volume set - where "within" 
*						can mean 'anywwhere' if there are no inclusion vols.
*						This test must be coupled with IsPointExcluded to test if a point in an add 
*							volume is also excluded by being in a sub volume.
*
***************************************************************************************************/

bool	CamVolumeSet::IsPointIncluded(const CPoint& obPos, 
									  const CamVolume* pobIgnore,
									  const CamVolume** ppobResult ) const
{
	// 

	const int iNumAddVols = 
			m_obVolumes.size();

	// remember - with no add volumes, a point is always 'inside' 
	if ((!iNumAddVols)
		||
		(iNumAddVols==1 && 
		(pobIgnore==m_obVolumes.front())))
	{
		return true;
	}

	for(ntstd::List<CamVolume*, Mem::MC_CAMERA>::const_iterator obVolIt(m_obVolumes.begin());
		obVolIt != m_obVolumes.end(); ++obVolIt)
	{
		if((*obVolIt) == pobIgnore)
			continue;

		if((*obVolIt)->Inside(obPos))
		{
			if (ppobResult)
			{
				*ppobResult = (*obVolIt);
			}

			return true;
		}
	}

	return false;
}

/***************************************************************************************************
*	
*	FUNCTION		CamVolumeSet::IsPointExcluded
*
*	DESCRIPTION		returns true if point is inside any exclusion volumes
*
***************************************************************************************************/
bool	CamVolumeSet::IsPointExcluded(const CPoint& obPos, 
									  const CamVolume* pobIgnore, 
									  const CamVolume** ppobResult ) const
{		
	for(ntstd::List<CamVolume*, Mem::MC_CAMERA>::const_iterator obVolIt(m_obExVolumes.begin());
		obVolIt != m_obExVolumes.end(); ++obVolIt)
	{
		if((*obVolIt) == pobIgnore)
			continue;

		if((*obVolIt)->Inside(obPos))
		{
			if (ppobResult)
			{
				*ppobResult = (*obVolIt);
			}

			return true;
		}
	}

	return false;
}

/***************************************************************************************************
*	
*	FUNCTION		CamVolumeSet::Inside
*
*	DESCRIPTION		returns true if point is within the volume set
*
***************************************************************************************************/
bool	CamVolumeSet::Inside(const CPoint& obPos) const
{
	m_obLastTestPoint = obPos;
	
	// check for exclusion
	if(IsPointExcluded(obPos))
		return false;


	// if no add volumes, then treat 
	// 		everything outside an exclusion as included...
	if (!m_obVolumes.size())
	{
		return true;
	}
	
	if(IsPointIncluded(obPos))
		return true;

	return false;
}

/***************************************************************************************************
*	
*	FUNCTION		CamVolumeSet::ExClampToSurf
*
*	DESCRIPTION     modifies the point to be the closest point outside the exclusion 
*						surface it is currently in
*
*	NOTES			Code assumes the exclusion volumes lie within the addition set.
*
***************************************************************************************************/
bool	CamVolumeSet::ExClampToSurf(const CPoint& obPos, 
									const CLAMP_ENV_INFO& obCInfo, 
									CIntersectResults& obIRResult) const
{
	ntAssert_p(!m_obExVolumes.empty(), ("Invalid to call ExClampToSurf() with exclusion volumes"));

	const float fScale = 1.0f;
	const float fErrorFactor = 1e-03f;

	if (obCInfo.m_pobPOI && UsesPOI())
	{
		CDirection obToInterestPoint =  (*obCInfo.m_pobPOI) ^ obPos;

		//	I really need a scale() method that can help here. Currently assuming 1.0f

		// scee.sbashow: danger is is that this vector could be too small ... this will do for now.
		if (obToInterestPoint.LengthSquared()<sqr(fErrorFactor*fScale))
		{
			// if so, report this by returning false.
			return false;
		}
		else
		{
			obToInterestPoint.Normalise();

			// 	ntPrintf("Cam is in exclusion --- will trace back along Line from Cam to POI\n");
			float fFactor = 500.0f*fScale;

			CIntersectResults obTempIR(*obCInfo.m_pobPOI);

			// scee.sbashow: 
			// find nearest point to *pobPOI between obPos and *pobPOI
			if (!GetIntersectAndNormal(obPos, 
									   *obCInfo.m_pobPOI,
									   obTempIR,NULL))
			{
				int iNumTries = 3;
				CIntersectResults obTempIR(obPos);

				while(iNumTries--)
				{
					
					// scee.sbashow: 
					// It may be that *pobPOI was also inside - in which case,
					// keep extending the line back from the 
					// pos (in the reverse direction to obPos -> *pobPOI)
					// until it hits something.
					if (!GetIntersectAndNormal(obPos - obToInterestPoint*fFactor, 
											   *obCInfo.m_pobPOI,
											   obTempIR,
											   NULL))
					{
						fFactor*=10.0f;
					}
					else
					{
						obIRResult = obTempIR;
						// the normal faces into the exclusion, so need to negate it here.
						obIRResult.InvertNormal();
						return true;
					}
				}

				// scee.sbashow: ray test not long enough ?
				// 			Increase iNumTries, or something could be amiss.
				ntError(0);
				return false;
			}
			else
			{
				obIRResult = obTempIR;
				// the normal faces out of the exclusion, 
				// so no need to negate it here.
				return true;
			}

		}
	}
	else
	{
		// scee.sbashow:
		//					Would only be called if no POI is provided

		const int iWorkSize = m_obExVolumes.size();
		CIntersectResults* paobRawResults = 
			(CIntersectResults*) alloca(sizeof(CIntersectResults)*iWorkSize);

		ntstd::List< CIntersectResults*, Mem::MC_CAMERA > obResults;

		int iNumResultsInside = 0;

		for(ntstd::List<CamVolume*, Mem::MC_CAMERA>::const_iterator obVolIt(m_obExVolumes.begin());
											obVolIt != m_obExVolumes.end();obVolIt++)
		{
			CamVolume* const pobCamExcludeVolume = *obVolIt;

			CIntersectResults obTempIR(obPos);

			// scee.sbashow: ok, is our point inside this exclude volume??
			if(pobCamExcludeVolume->Inside(obPos) &&
				pobCamExcludeVolume->ExClampToSurf(obPos, 
												   obCInfo, 
												   obTempIR))
			{
				const CPoint obTempSurf = obTempIR.GetSurface();

				// is the closest wall point to the point in question still excluded by others?
				if (IsPointExcluded(obTempSurf,pobCamExcludeVolume))
				{
					// use a POI pos which'll still be inside the exclusion volume -- 
					// ie quite close to obTempSurf on the line between it and obPos.
					 CDirection obNormed = (obPos^obTempSurf);

					 const CPoint obPOIPos = 	
						 obTempSurf + obNormed*0.5f;

						// scee.sbashow: make damn sure it actually is excluded, tho' ;-) 
   						// otherwise ExClampToSurf will not project it out the back end 
						// (ie the end in the opposite direction of obNormed ... this opposite direction 
						// 		being the direction of intial push that took us to obTempSurf) 
						// of the volume set we want to push away from.
						// 
					ntError(IsPointExcluded(obPOIPos));


					CLAMP_ENV_INFO obCEI;
					obCEI.m_pobPOI = &obPOIPos;
					obCEI.m_pobPOIPlaneNormal = 
						obCInfo.m_pobPOIPlaneNormal;

					// scee.sbashow: not perfect - curretly projecting out in the direction we 
					// 						initially found was the smallest to an exclude volume's surface
					//							here we continue to project out if this surface point is 
					// 								 contained in other exclude volumes. The resultant may be further
					// 									than trying it from a different original surface point.
					const bool bCouldEx_it = this->ExClampToSurf(obTempSurf,
																 obCEI, //&obPosIter
																 obTempIR);

					UNUSED(bCouldEx_it);

					// scee.sbashow : If this happens, see me (scee.sbashow)-- likely cause is that 
					// 					obLookingPos is not far enough away from obTempTempSurf
					ntError(bCouldEx_it);
				}

				// scee.sbashow: note - CIntersectResults has no destructor 
				// 		that does anything important, so will not need to call destructor on the objects constructed on the stack here.
				CIntersectResults* const pobResult = 
					NT_PLACEMENT_NEW(&paobRawResults[iNumResultsInside++]) CIntersectResults(obPos, 
																							 obTempIR);

				InsertResultOnDist(*pobResult, obResults);
			}
		}

		// must have result!
		ntError(obResults.size()>0);
		obIRResult = *obResults.front();
		return true;
	}
}

/*****************************************************************************************************************************
*	
*	FUNCTION		CamVolumeSet::CalculateWeightedIntersectionPoint
*
*	DESCRIPTION     Ok, here goes. This method takes the list of intersection results, and uses the nearest three
*						it can find to work out a weighted point. Such a weighted point will be used for projection to mantain
*						continuity unlike what happens with a simple projected point moves across add volume surfaces.
*					obRefPoint is the camera's unclipped pos, and is used as an input to work out the weights 
*						which are based on the closeness of the projected results to this cam ideal pos.
*						The maximum weighting that can be considered is with three relatively close points.
*					
******************************************************************************************************************************/
bool CamVolumeSet::CalculateWeightedIntersectionPoint(const ntstd::List< CIntersectResults*, Mem::MC_CAMERA >& obIntersectionResults, 
											 const CPoint& obRefPoint,
											 float fWeightFactor,
											 CPoint& obWeightedResult) const
{
	if (obIntersectionResults.size()<2)
	{
		return false;
	}

	obWeightedResult = CPoint(CONSTRUCT_CLEAR);

	int iNumToConsider = 0;
	float afWeights[3] = {0.0f,0.0f,0.0f};
	CPoint aobPointsConsidered[3];

	// insert on increasing distance from the refpos.
	for(ntstd::List<CIntersectResults*, Mem::MC_CAMERA>::const_iterator obIt(obIntersectionResults.begin());
		obIt != obIntersectionResults.end(); ++obIt)
	{
		const CIntersectResults* const obResult = *obIt;

		const CPoint obSurfPoint = obResult->GetSurface();
		const CDirection obSurfNormal = obResult->GetNormal();

		const CDirection obDir = obRefPoint^obSurfPoint;

		if (obDir.Dot(obSurfNormal)>0.0f)
		{
			const float fLengthToSurface = obDir.Length();
			const CPoint obTestPoint = 
				obSurfPoint+obSurfNormal*(fLengthToSurface*0.05f);

			CPoint obTempSurf;
			CDirection obTempNorm;

			bool bValidSurfacePoint = true;

			CIntersectResults obTempIR(obRefPoint);

			for(ntstd::List<CamVolume*, Mem::MC_CAMERA>::const_iterator obVolIt(m_obVolumes.begin());
				 obVolIt != m_obVolumes.end(); ++obVolIt)
			{
				if	((*obVolIt)->GetIntersectAndNormal(obTestPoint, 
													   obRefPoint, 
													   obTempIR,
													   NULL))
				{
					// can't consider this obSurfPoint 
					// point - must lie around other side 
					// of volume set or something
					bValidSurfacePoint = false;
					break;
				}
			}


            if (bValidSurfacePoint)
			{

				afWeights[iNumToConsider]				= 1.0f/(fLengthToSurface+0.000001f);
				aobPointsConsidered[iNumToConsider] 	= obSurfPoint;

				if ((++iNumToConsider)==3)
				{
					break;
				}
			}
		}
	}

	if ( iNumToConsider <= 0 )
	{
		user_warn_msg( ("Num Points to consider less than or equal to 0\n") );
		return false;	
	}

	if (iNumToConsider>1)
	{
		#ifdef CAM_VOLUMES_DEBUG

		for (int i=0; i<iNumToConsider; i++)
		{
			for (int j=i+1; j<iNumToConsider; j++)
			{
				CCamUtil::Render_Line(aobPointsConsidered[i], aobPointsConsidered[j], 1.0f, 1.0f, 0.0f, 1.0f);
			}
		}

		#endif

		float fTotalWeight=0.0f;

		for (int i=0; i<iNumToConsider; i++)
		{
			fTotalWeight 	 += 	afWeights[i];
		}

		if (fTotalWeight>0.0f)
		{
			const float fNormFact = (1.0f/fTotalWeight);
			for (int i=0; i<iNumToConsider; i++)
			{
				afWeights[i]*=fNormFact;
			}

			float fMaxWeight = 0.0f;

			for (int i=0; i<iNumToConsider; i++)
			{
				if (afWeights[i]>fMaxWeight)
				{
					fMaxWeight = afWeights[i];
				}
			}

			fTotalWeight = 0.0f;
			for (int i=0; i<iNumToConsider; i++)
			{
				float fWeightConsidered = expf(-sqr(fWeightFactor*(afWeights[i]-fMaxWeight)));
				fTotalWeight+=fWeightConsidered;
				obWeightedResult += aobPointsConsidered[i]*fWeightConsidered;
			}

			obWeightedResult*=1.0f/fTotalWeight;

		#ifdef CAM_VOLUMES_DEBUG
			CCamUtil::Render_Sphere(obWeightedResult,0.2f,1.0f,0.0f,0.0f,1.0f);
		#endif
			return true;
		}
	}

	return false;
}


/***************************************************************************************************
*	
*	FUNCTION		CamVolumeSet::ClampToSurf
*
*	DESCRIPTION     modifies the point to be the closest point on the surface to the test pos
*					and calculates the surface normal at this point
*
***************************************************************************************************/
bool	CamVolumeSet::ClampToSurf(const CPoint& obPos, 
								  const CLAMP_ENV_INFO& obCInfo, 
								  CIntersectResults& obResult) const
{
	m_obLastTestPoint = obPos;

	ntAssert(!Inside(obPos));

	// if no addition volumes are specified, it is assumed that 
	// anything not in the exlcusion volumes is 'inside'.
	// For the camera to be in here then, implies it is in an exclusion volume.
	if (!m_obVolumes.size())
	{
		return ExClampToSurf(obPos, 
							 obCInfo, 
							 obResult);
	}

	const int iWorkSize = m_obVolumes.size()+1;
	CIntersectResults* paobRawResults = 
		(CIntersectResults*) alloca(sizeof(CIntersectResults)*iWorkSize);

	ntstd::List< CIntersectResults*, Mem::MC_CAMERA > obResults;

	int iNumResultsOutside = 0;

	// i. Generate a list of all the intersection points for all our volumes if any
	//-----------------------------------------------------------------------------
	for(ntstd::List<CamVolume*, Mem::MC_CAMERA>::const_iterator obVolIt(m_obVolumes.begin());
		obVolIt != m_obVolumes.end(); ++obVolIt)
	{
		CPoint		obTempSurf;
		CDirection	obTempNorm;

		const bool bInside = ((*obVolIt)->Inside(obPos));

		if (bInside)
		{
			// ok, so for this ClampToSurf function to have been called, it registered that it --the camera-- was not inside() the volume set.
			// That means that it must also be in an exclusion volume if it is inside any addition volume, as is the case just now.
			// So perform an exclusion clamp out. 
			return ExClampToSurf(obPos, 
								obCInfo, 
								obResult);
		}

		CIntersectResults obTempIR(obPos);

		CLAMP_ENV_INFO obCEI;
		obCEI.m_pobPOI = NULL;
		obCEI.m_pobPOIPlaneNormal = obCInfo.m_pobPOIPlaneNormal;

		const bool bClamped = (*obVolIt)->ClampToSurf(obPos, 
													  obCEI, 
													  obTempIR);
		if( bClamped )
		{
			// scee.sbashow: note - CIntersectResults has no destructor 
			// 		that does anything important, so will not need to call destructor on the objects constructed on the stack here.
			CIntersectResults* const pobResult = 
				NT_PLACEMENT_NEW(&paobRawResults[iNumResultsOutside++]) CIntersectResults(obPos,obTempIR);

			InsertResultOnDist(*pobResult, obResults);

		}
	}

	if (obResults.empty())
	{
		return false;
	}

	// ii. return the closest one
	//---------------------------------------------------

	// scee.sbashow: for some add volume constructs, this will
	//					lead to discontinous breaks in the clamped position
	// 					One way to fix this would be to store clamped positions history
	// 						and each time a jump happens, to set the position in a stack of clamped positions
	// 					And navigate around box edges to clear the stack of requested clamp positions.
	// 						But this'll do for now (also, skeptical of 'historical' procedures in cam avoidance code.)

	// scee.sbashow:	This 'historical' method will not be the way I will approach it.
	// 					Guided as I am by the principle that 'history' in camera code is generally a BAD idea.
	// 
	// The following function will use the nearest two or three add-volume viewable-surface projected points, 
	// and construct a line or tri, and choose a point in this line or tri with a position that is 
	// a1*p1+a2*p2+a3*p3, where p1,p2,p3 are the points, and a1,a2,a3 are weights a1+a2+a3=1 proportional to the 
	// distances of the projected points from the original point.
	// 
	// This point will then be used as a POI, and I will project back on a line from the input point 
	// through this POI to the relevant, intersecting, add volume surface. Continuity should thus be acheived, 
	// so long as the add volume setup is fairly simple.
	// 
	// 

	// a1*p1+a2*p2, or a1*p1+a2*p2+a3*p3, depending
	CPoint obWeightedPoint;

	// do we need to interpolate between two or 
	// three nearest points and project back with the result?
	if (CalculateWeightedIntersectionPoint(obResults,
								  obPos,
								  20.0f,
								  obWeightedPoint))
	{
		// result is now in obWeightedPoint
		const CDirection obToWeighted = obWeightedPoint^obPos;
		CPoint obProjectedResult;
		CDirection obProjectedNormal;

		CIntersectResults obTempIR(obWeightedPoint);

		// find nearest point to *pobPOI between obPos and *pobPOI
		if (GetIntersectAndNormal(obPos, 
								   obPos+obToWeighted*50.0f,
									obTempIR,NULL))
		{

			CIntersectResults* const pobResult = 
				NT_PLACEMENT_NEW(&paobRawResults[iNumResultsOutside++]) CIntersectResults(obWeightedPoint,obTempIR);
			obResults.push_front(pobResult);
		}
	}
	
	const CIntersectResults& obIntbestResult = *obResults.front();


	// Should not map exclusion volumes which cut 
	// through addition volumes, if there are any addition volumes.
	// Have to deal with this case if someone does.
	// Will do this by clamping out.
	if (IsPointExcluded(obIntbestResult.GetSurface()))
	{
		if (obCInfo.m_pobPOI)
		{
			const CPoint obClamped = obIntbestResult.GetSurface();

			bool bCouldClampOneDir = ExClampToSurf(	obClamped, 
													obCInfo, 
													obResult);
			ntError(bCouldClampOneDir);
			return bCouldClampOneDir;
		}
		else
		{

			CLAMP_ENV_INFO obCEI;
			obCEI.m_pobPOI = &obPos;
			obCEI.m_pobPOIPlaneNormal = obCInfo.m_pobPOIPlaneNormal;

			// I cannot say if this will necessarily work, but I'm basically performing an exclamp on
			// a POI which is the projected surface point of the add volume, 
			// and the original pos to clamp. 
			// Or reversing the two if it doesn't work that way round the first time.
			const CPoint obPOI = obIntbestResult.GetSurface();

			bool bCouldClampOneDir = ExClampToSurf(	obPOI, 
													obCEI, 
													obResult);
			if (!bCouldClampOneDir)
			{
				obCEI.m_pobPOI = &obPOI;
                bCouldClampOneDir = ExClampToSurf(	obPos, 
													obCEI, 
													obResult);
				ntError(bCouldClampOneDir);

			}
			return bCouldClampOneDir;
		}
	}
	else
	{
		obResult = obIntbestResult;
		return true;
	}

	return false;
}

/***************************************************************************************************
*	
*	FUNCTION		CamVolumeSet::Intersect
*
*	DESCRIPTION		returns true if line segment intersects the volume set surface
*
***************************************************************************************************/
bool CamVolumeSet::Intersect(const CPoint& obStart, const CPoint& obEnd) const
{
	SetIntersectMode(ITSCT_MODE_NONE);

	CIntersectResults obTempIR(obStart);

	return GetIntersectAndNormal(obStart, 
								 obEnd, 
								 obTempIR, NULL);
}

/***************************************************************************************************
*	
*	FUNCTION		CamVolumeSet::IntersectPointValid
*
*	DESCRIPTION		returns true if point is in set, depending on volume set's intersection mode.
*
***************************************************************************************************/
bool	CamVolumeSet::IntersectPointValid( const CPoint& obPoint ) const
{
	switch(m_eIntersectMode)
	{
		case ITSCT_MODE_NONE:
			return true;
		case ITSCT_MODE_PUSH_EXC:
		{
			return (!IsPointExcluded(obPoint,m_pobExcluded)) &&
						IsPointIncluded(obPoint,m_pobExcluded);
		}
		case ITSCT_MODE_PUSH_INC:
		{
			return (!IsPointIncluded(obPoint, m_pobExcluded));
		}

		default:
		ntError(0);
		return true;
	}
}

/***************************************************************************************************
*	
*	FUNCTION		CamVolumeSet::InsertResultOnDist
*	
*	DESCRIPTION		Ordered insert into the list
*					scee.sbashow: didn't write this method, but thinking one could do a binary search ... 
*	 								guess it depends on how many elelments in obList one expects.
*
***************************************************************************************************/
void CamVolumeSet::InsertResultOnDist(CIntersectResults& obResult, 
									  ntstd::List<CIntersectResults*, Mem::MC_CAMERA>& obList)
{
	if(!obList.empty())
	{
		const float fDistSq = 
			(obResult.GetRefPos() - obResult.GetSurface()).LengthSquared();

		// insert on increasing distance from the refpos.
		for(ntstd::List<CIntersectResults*, Mem::MC_CAMERA>::iterator obIt(obList.begin());
			obIt != obList.end(); ++obIt)
		{
			const float fCurrDistSq = 
				(obResult.GetRefPos() - (*obIt)->GetSurface()).LengthSquared();

			if(fCurrDistSq>fDistSq)
			{
				obList.insert(obIt, &obResult);
				return;
			}
		}
	}
	// our one must be furthest away, or the list is empty
	obList.push_back(&obResult);			
}


/***************************************************************************************************
*	
*	FUNCTION		CamVolumeSet::GetIntersectAndNormal
*
*	DESCRIPTION		returns true if line segment intersects the volume set surface
*					returns the closest intersection point to start if true, and the normal at that point
*
***************************************************************************************************/
bool CamVolumeSet::GetIntersectAndNormal(const CPoint& obStart, 
										 const CPoint& obEnd,
										 CIntersectResults& obIRResult,
										 const CamVolumeSet* pobParentSet) const
{

	UNUSED(pobParentSet);

	const int iWorkSize = m_obVolumes.size() + m_obExVolumes.size();
	CIntersectResults* paobRawResults = 
		(CIntersectResults*) alloca(sizeof(CIntersectResults)*iWorkSize);

	int iNumResults = 0;

	// i. Generate a list of all the intersection points for all our volumes if any
	//-----------------------------------------------------------------------------
	ntstd::List< CIntersectResults*, Mem::MC_CAMERA > obFirstResults;
	ntstd::List< CIntersectResults*, Mem::MC_CAMERA > obSecondResults;


	SetIntersectMode(ITSCT_MODE_PUSH_INC);

	for(ntstd::List<CamVolume*, Mem::MC_CAMERA>::const_iterator obVolIt(m_obVolumes.begin());
		obVolIt != m_obVolumes.end(); ++obVolIt)
	{
		SetIntersectExclude(*obVolIt);
		CIntersectResults obTempIR(obIRResult.GetRefPos());

		if	((*obVolIt)->GetIntersectAndNormal(obStart, 
											   obEnd, 
											   obTempIR,
											   this))
		{
			CIntersectResults* pobResult =  
				NT_PLACEMENT_NEW(&paobRawResults[iNumResults++]) CIntersectResults(obIRResult.GetRefPos(), obTempIR);
			InsertResultOnDist(*pobResult, obFirstResults);
		}
	}

	SetIntersectMode(ITSCT_MODE_PUSH_EXC);

	// ii. See if we intersect any exclusion volumes
	//-----------------------------------------------------------------------------
	for(ntstd::List<CamVolume*, Mem::MC_CAMERA>::const_iterator obVolIt(m_obExVolumes.begin());
		obVolIt != m_obExVolumes.end(); ++obVolIt)
	{
		SetIntersectExclude(*obVolIt);

		CIntersectResults obTempIR(obIRResult.GetRefPos());

		if	((*obVolIt)->GetIntersectAndNormal(obStart, 
											   obEnd, 
											   obTempIR,
											   this))
		{
			CIntersectResults* pobResult = 
				NT_PLACEMENT_NEW(&paobRawResults[iNumResults++]) CIntersectResults(obIRResult.GetRefPos(), obTempIR);
			pobResult->InvertNormal(); // NB we use the inverted normal here intentionally
			InsertResultOnDist(*pobResult, obSecondResults);
		}
	}

	if((obFirstResults.empty()) && (obSecondResults.empty()))
		return false;

	// iii. Check to see if any of the pushed out added are excluded, and push out
	//----------------------------------------------------------------------------
	while(!obFirstResults.empty())
	{
		CIntersectResults* const pobNearest = obFirstResults.front();
		const CamVolume* pobExclusion = NULL;
		const bool bExcluded = 
			IsPointExcluded(pobNearest->GetSurface(), NULL, &pobExclusion);
		
		obFirstResults.pop_front();

		if(bExcluded)
		{
			ntError(pobExclusion);

			CPoint		obTempSurf;
			CDirection	obTempNorm;

			SetIntersectExclude(pobExclusion);

			CIntersectResults obTempIR(obIRResult.GetRefPos());

			// back project our point on this volume away from the start, to see if it becomes valid	
			// we get the exit surface point by the order we pass start and end to GetIntersectAndNormal()
			// see if the exit point is within our volume set
			if	(pobExclusion->GetIntersectAndNormal(obEnd, 
													 obStart, 
                                                     obTempIR,	
													 this))
			{
                pobNearest->SetResults(obTempIR); 
				pobNearest->InvertNormal(); // NB we use the inverted normal here intentionally

				InsertResultOnDist(*pobNearest, obSecondResults);
			}
		}
		else
		{
			// it's fit for further evaluation
			InsertResultOnDist(*pobNearest, obSecondResults);
		}
	}

	// make sure we've cleared our first list
	ntAssert(obFirstResults.empty());

	// iv. return the closest one
	//---------------------------------------------------
	if(!obSecondResults.empty())
	{	
		obIRResult = *obSecondResults.front();
		return true;
	}

	return false;
}

/***************************************************************************************************
*	
*	FUNCTION		CamVolumeSet::GetRandomPoint
*
*	DESCRIPTION		returns a random point within the volume set
*
*	NOTES			will attemp a finite number of tries if we have any exclusion volumes,
*					but we cant carry this on for ever, as the exclusion zone may well cover all
*					our valid volumes anyway...which would be stupid, but you justdont know...
*
***************************************************************************************************/
void CamVolumeSet::GetRandomPoint(CPoint& obPos) const 
{
	ntError(!m_obVolumes.empty());

	int iNumTries = 10;

	while(iNumTries--)
	{
		int iVolumeToUse = grand() % m_obVolumes.size();

		ntstd::List<CamVolume*, Mem::MC_CAMERA>::const_iterator obVolIt(m_obVolumes.begin());

		ntstd::advance(obVolIt, iVolumeToUse);

		(*obVolIt)->GetRandomPoint(obPos);

		if(!IsPointExcluded(obPos))
			return;
	}
}

/***************************************************************************************************
*	
*	FUNCTION		CamVolumeSet::Render
*
*	DESCRIPTION		displays all the volumes (all those that dont intersect the last tested point
*					are rendered at half intensity)
*
***************************************************************************************************/
void CamVolumeSet::Render(float fRed, float fGreen, float fBlue, float fAlpha) const
{
	for(ntstd::List<CamVolume*, Mem::MC_CAMERA>::const_iterator obVolIt(m_obVolumes.begin());
		obVolIt != m_obVolumes.end(); ++obVolIt)
	{
		(*obVolIt)->Inside(m_obLastTestPoint); 
		(*obVolIt)->Render(fRed,fGreen,fBlue, fAlpha);
	}

	for(ntstd::List<CamVolume*, Mem::MC_CAMERA>::const_iterator obVolIt(m_obExVolumes.begin());
		obVolIt != m_obExVolumes.end(); ++obVolIt)
	{
		(*obVolIt)->Inside(m_obLastTestPoint); 
		(*obVolIt)->Render(1.0f-fRed,1.0f-fGreen, 1.0f-fBlue, fAlpha);
	}
}


void CamVolumeSet::Render(bool /*bParentSelected*/, bool /*bSiblingSelected*/) const
{
	/*
	bParentSelected = bParentSelected || IsSelected();
	bSiblingSelected = bParentSelected || bSiblingSelected;

	if(!bSiblingSelected)
	{
		for(ntstd::List<CamVolume*>::const_iterator obVolIt(m_obVolumes.begin());
			obVolIt != m_obVolumes.end(); ++obVolIt)
		{
			if((*obVolIt)->IsSelected())
			{
				bSiblingSelected = true;
				break;
			}
		}

		if(!bSiblingSelected)
		{
			for(ntstd::List<CamVolume*>::const_iterator obVolIt(m_obExVolumes.begin());
				obVolIt != m_obExVolumes.end(); ++obVolIt)
			{
				if((*obVolIt)->IsSelected())
				{
					bSiblingSelected = true;
					break;
				}
			}
		}
	}

	for(ntstd::List<CamVolume*>::const_iterator obVolIt(m_obVolumes.begin());
		obVolIt != m_obVolumes.end(); ++obVolIt)
	{
		(*obVolIt)->Inside(m_obLastTestPoint); 
		(*obVolIt)->Render(bParentSelected, bSiblingSelected);
	}

	for(ntstd::List<CamVolume*>::const_iterator obVolIt(m_obExVolumes.begin());
		obVolIt != m_obExVolumes.end(); ++obVolIt)
	{
		(*obVolIt)->Inside(m_obLastTestPoint); 
		(*obVolIt)->Render(bParentSelected, bSiblingSelected);
	}
	*/
}

/***************************************************************************************************
*	
*	FUNCTION		CamVolumeSet::Render
*
*	DESCRIPTION		render stuff
*
***************************************************************************************************/
void CamVolumeSet::RenderInfo(int iX, int iY) const
{
	CCamUtil::DebugPrintf(iX, iY, "CamVolumeSet: %s ",  ntStr::GetString(ObjectDatabase::Get().GetNameFromPointer( this )));
}

/***************************************************************************************************
*	
*	FUNCTION		CamVolumeSet::GetPosition
*
*	DESCRIPTION		returns a weighted [scee.sbashow: where's the weighting???] average of all the volumes Positions
*
*	TBC				this could be pre-calced once we know were finished building
*
***************************************************************************************************/
CPoint CamVolumeSet::GetPosition() const 
{
	ntError(!m_obVolumes.empty());

	CPoint obResult(CONSTRUCT_CLEAR);

	for(ntstd::List<CamVolume*, Mem::MC_CAMERA>::const_iterator obVolIt(m_obVolumes.begin());
		obVolIt != m_obVolumes.end(); ++obVolIt)
	{
		obResult += (*obVolIt)->GetPosition();
	}

	obResult /= (float)m_obVolumes.size();

	return obResult;
}

/***************************************************************************************************
*	
*	FUNCTION		CamVolumeSet::DebugRender
*
*	DESCRIPTION		
*
***************************************************************************************************/
void CamVolumeSet::DebugRender ()
{
	for(ntstd::List<CamVolume*, Mem::MC_CAMERA>::const_iterator obVolIt(m_obVolumes.begin());
		obVolIt != m_obVolumes.end(); ++obVolIt)
	{
		(*obVolIt)->Render( true, true );
	}
	
}











