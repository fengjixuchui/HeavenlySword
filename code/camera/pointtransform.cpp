/***************************************************************************************************
*
*	$Header:: /game/pointtransform.cpp 1     11/08/03 16:30 Wil                                    $
*
*
*
*	CHANGES
*
*	11/8/2003	Wil	Created
*
***************************************************************************************************/

#include "camera/pointtransform.h"
#include "camera/camutils.h"
#include "camera/elementmanager.h"
#include "camera/curveeditor.h"
#include "camera/camvolumes.h"
#include "camera/converger.h"

#include "objectdatabase/dataobject.h"

START_CHUNKED_INTERFACE( PTGuideDef, Mem::MC_CAMERA)
	PUBLISH_PTR_AS(m_pobGuideCurve, GuideCurve)
END_STD_INTERFACE

START_CHUNKED_INTERFACE( PTRangeDef, Mem::MC_CAMERA)
	PUBLISH_VAR_WITH_DEFAULT_AS(m_obClampMin, CPoint(0.0f, 0.0f, 0.0f), ClampMin)
	PUBLISH_VAR_WITH_DEFAULT_AS(m_obClampMax, CPoint(0.0f, 0.0f, 0.0f), ClampMax)
END_STD_INTERFACE

START_CHUNKED_INTERFACE( PTVolumeDef, Mem::MC_CAMERA)
	PUBLISH_PTR_AS(m_pobVolume, Volume)
	PUBLISH_VAR_WITH_DEFAULT_AS(m_fConvergeDamp, 0.2f, ConvergeDamp)
	PUBLISH_VAR_WITH_DEFAULT_AS(m_fConvergeSpeed, 12.0f, ConvergeSpeed)
	PUBLISH_VAR_WITH_DEFAULT_AS(m_fConvergeSpring, 5.0f, ConvergeSpring)
END_STD_INTERFACE


//------------------------------------------------------------------------------------------
//!                                                                                         
//!	PTGuideDef::Create                                                                      
//!	Create a PTGuide from this definition.                                                  
//!                                                                                         
//------------------------------------------------------------------------------------------
PointTransform* PTGuideDef::Create()
{
	return NT_NEW_CHUNK( Mem::MC_CAMERA ) PTGuide(*this);
}


/***************************************************************************************************
*	
*	FUNCTION		PTGuide::PTGuide
*
*	DESCRIPTION		Construct the point transform
*
***************************************************************************************************/
PTGuide::PTGuide(const PTGuideDef& def)
: m_uiGuideSubdivs(20),
	m_obDef(def)
{
}

/***************************************************************************************************
*	
*	FUNCTION		PTGuide::Update
*
*	DESCRIPTION		calc position based on input pos
*
***************************************************************************************************/
CPoint	PTGuide::Update(const CPoint& obTracking, 
						const CamSceneElementMan& obMan,
						float fTimeChange)
{
	UNUSED(obMan);
	UNUSED(fTimeChange);

    m_bGuideInit = true;

	SetLastTracked(obTracking);

	float fDistSq = 0.0f;
	m_fTargetVal = 
		m_obDef.m_pobGuideCurve->GetNearestLinear(GetLastTracked(), 
										  fDistSq, 
										  2, 
										  m_uiGuideSubdivs);

	SetLastTransform(m_obDef.m_pobGuideCurve->GetPoint(m_fTargetVal));

	return GetLastTransform();
}
	
/***************************************************************************************************
*	
*	FUNCTION		PTGuide::Render
*
*	DESCRIPTION		render stuff
*
***************************************************************************************************/
void	PTGuide::Render(void)
{
	m_obDef.m_pobGuideCurve->Render();

	if(TrackingValid())
		CCamUtil::Render_Line(GetLastTracked(), GetLastTransform(), 1.0f,1.0,1.0f,1.0f);
}
	






//------------------------------------------------------------------------------------------
//!                                                                                         
//!	PTRangeDef::Create                                                                      
//!	Create a PTRange from this definition.                                                  
//!                                                                                         
//------------------------------------------------------------------------------------------
PointTransform* PTRangeDef::Create()
{
	return NT_NEW_CHUNK( Mem::MC_CAMERA ) PTRange(*this);
}


/***************************************************************************************************
*	
*	FUNCTION		PTRange::PTRange
*
*	DESCRIPTION		Construct the point transform
*
***************************************************************************************************/
PTRange::PTRange(const PTRangeDef& def)
:	m_obClampMin(def.m_obClampMin),
	m_obClampMax(def.m_obClampMax)
{
}


/***************************************************************************************************
*	
*	FUNCTION		PTRange::Update
*
*	DESCRIPTION		calc position based on input pos
*
***************************************************************************************************/
CPoint	PTRange::Update(const CPoint& obTracking, 
						const CamSceneElementMan& obMan,
						float fTimeChange)
{
	UNUSED(obMan);
	UNUSED(fTimeChange);

	SetLastTracked(obTracking);

	CPoint obTemp;
	obTemp.X() = ntstd::Clamp(GetLastTracked().X(), m_obClampMin.X(), m_obClampMax.X());
	obTemp.Y() = ntstd::Clamp(GetLastTracked().Y(), m_obClampMin.Y(), m_obClampMax.Y());
	obTemp.Z() = ntstd::Clamp(GetLastTracked().Z(), m_obClampMin.Z(), m_obClampMax.Z());

	SetLastTransform(obTemp);

	return GetLastTransform();
}
	
/***************************************************************************************************
*	
*	FUNCTION		PTRange::Render
*
*	DESCRIPTION		render stuff
*
***************************************************************************************************/
void	PTRange::Render(void)
{
	CCamUtil::Render_AABB(m_obClampMin, m_obClampMax, 1.0f, 0.0, 0.0f, 1.0f);

	if(TrackingValid())
		CCamUtil::Render_Line(GetLastTracked(), GetLastTransform(), 1.0f,1.0,1.0f,1.0f);
}






//------------------------------------------------------------------------------------------
//!                                                                                         
//!	PTVolumeDef::Create                                                                     
//!	Create a PTVolume from this definition.                                                 
//!                                                                                         
//------------------------------------------------------------------------------------------
PointTransform* PTVolumeDef::Create()
{
	return NT_NEW_CHUNK( Mem::MC_CAMERA ) PTVolume(*this);
}


/***************************************************************************************************
*	
*	FUNCTION		PTVolume::PTVolume
*
*	DESCRIPTION		Construct the point transform
*
***************************************************************************************************/
PTVolume::PTVolume(const PTVolumeDef& def)
{
	m_pobVolume = def.m_pobVolume;

	CConvergerDef obPCDef("");

	obPCDef.SetDamp(def.m_fConvergeDamp);
	obPCDef.SetSpring(def.m_fConvergeSpring);
	obPCDef.SetSpeed(def.m_fConvergeSpeed);
    
	m_pobPointConverger = NT_NEW_CHUNK(Mem::MC_CAMERA) CPointConverger(obPCDef);

	PTVolume::Reset();

}

PTVolume::~PTVolume()
{
	NT_DELETE_CHUNK(Mem::MC_CAMERA,m_pobPointConverger);
}

/***************************************************************************************************
*	
*	FUNCTION		PTVolume::Reset
*
*	DESCRIPTION		calc position based on input pos
*
***************************************************************************************************/

void PTVolume::Reset()
{
	m_pobPointConverger->Reset();
	m_pobNewVolumeClamped = m_pobLastVolumeClamped = NULL;
	m_iLastSurface = m_iNewSurface = -1;
	m_bToLastKnown = m_bPointConvergerInUse = false;
}

/***************************************************************************************************
*	
*	FUNCTION		PTVolume::Update
*
*	DESCRIPTION		calc position based on input pos
*
***************************************************************************************************/
CPoint	PTVolume::Update(const CPoint& obTracking,  
						 const CamSceneElementMan& obMan, 
						 float fTimeChange)
{
	SetLastTracked(obTracking);

	// if we're outside the volume set, clamp to its surface, as 
	// defined by the addition volume set, or that of an exclusion 
	// volume from a set of exclusions which must be entirely within 
	// the (non-broken) addition set.
	
	#ifdef CAM_VOLUMES_DEBUG
		m_pobVolume->Render(1.0f,0.0f,0.0f,1.0f);
	#endif

	if(!m_pobVolume->Inside(obTracking))
	{
		CamVolume::CIntersectResults obIRResult(obTracking);
		CamVolume::CLAMP_ENV_INFO obCEI;
		obCEI.m_pobPOI = &obMan.GetPointOfInterest();
		obCEI.m_pobPOIPlaneNormal = NULL;
        if (!m_pobVolume->ClampToSurf(obTracking, obCEI, obIRResult))
		{
			ntError(0);
		}
		
		#ifdef CAM_VOLUMES_DEBUG

		const float fRed = 1.0f;
		const float fGreen = 0.0f;
		const float fBlue = 0.0f;
		CCamUtil::Render_Line(obIRResult.GetSurface(), obMan.GetPointOfInterest(), fRed, fGreen, fBlue, 1.0f);
		CCamUtil::Render_Sphere(obIRResult.GetSurface(),2.0f,1.0f,1.0f,0.0f,1.0f);
		m_pobVolume->Render(fRed,fGreen,1.0f,1.0f);
		
		#endif

		if (m_pobPointConverger)
		{
			int iSurfaceID;
			const CamVolume* pobVolumeClamped  =
				obIRResult.GetIRVolumeAndSurface(iSurfaceID);

			ntError_p(pobVolumeClamped, ("Should not happen, that a clamp generates no volume!! -- flag with scee.sbashow"));

			if (!m_bPointConvergerInUse)
			{
				m_pobLastVolumeClamped = m_pobNewVolumeClamped = pobVolumeClamped;
				m_iLastSurface = m_iNewSurface = iSurfaceID;
				m_bToLastKnown = false;
			}
			else
			if (m_pobNewVolumeClamped!=pobVolumeClamped || 
				iSurfaceID!=m_iNewSurface)
			{
				if (m_pobNewVolumeClamped)
				{
					m_bToLastKnown = true;
					m_obLastKnownPoint = obIRResult.GetSurface();
					m_pobLastVolumeClamped = m_pobNewVolumeClamped;
					m_iLastSurface = m_iNewSurface;
				}
				else
				{
					m_bToLastKnown = false;
				}

				m_pobNewVolumeClamped = pobVolumeClamped;
				m_iNewSurface = iSurfaceID;

				//m_pobPointConverger->Reset();
				//SetLastTransform(obIRResult.GetSurface());
				#ifdef CAM_VOLUMES_DEBUG
					ntPrintf("Warning, Vol is %d, vol surface is now = %d!!\n",*(int*)&pobVolumeClamped,iSurfaceID);
				#endif
			}

            if (m_bToLastKnown)
			{
				m_pobPointConverger->Update(m_obLastKnownPoint,fTimeChange);
				const CDirection obToLastKnown = (m_obLastKnownPoint^m_pobPointConverger->GetDamped());
				if (obToLastKnown.LengthSquared()<sqr(m_pobPointConverger->GetDef().GetSpeed()*fTimeChange))
				{
					m_bToLastKnown = false;
				}

				pobVolumeClamped = m_pobLastVolumeClamped;
			}
			else
			{
				m_pobPointConverger->Update(obIRResult.GetSurface(),fTimeChange);
				SetLastTransform(m_pobPointConverger->GetDamped());
			}

			obCEI.m_pobPOI = NULL;
			obCEI.m_pobPOIPlaneNormal = NULL;

			CamVolume::CIntersectResults 
				obIRConvergerPosSurfaceBound(m_pobPointConverger->GetDamped());

			m_bPointConvergerInUse = true;
			m_obLastConvergedPoint = m_pobPointConverger->GetDamped();

			if (pobVolumeClamped->IsAdd())
			{

				if (!pobVolumeClamped->Inside(m_obLastConvergedPoint))
				{
					pobVolumeClamped->ClampToSurf(m_obLastConvergedPoint,
												  obCEI,obIRConvergerPosSurfaceBound);
					m_obLastConvergedPoint = obIRConvergerPosSurfaceBound.GetSurface();
				}
			}
			else
			{
				if (pobVolumeClamped->Inside(m_obLastConvergedPoint))
				{
					pobVolumeClamped->ExClampToSurf(m_obLastConvergedPoint,
													obCEI,obIRConvergerPosSurfaceBound);
					m_obLastConvergedPoint = obIRConvergerPosSurfaceBound.GetSurface();
				}
			}

			SetLastTransform(m_obLastConvergedPoint);
			#ifdef CAM_VOLUMES_DEBUG
					CCamUtil::Render_Sphere(m_obLastConvergedPoint,0.1f,0.0f,0.0f,1.0f,1.0f);
			#endif

		}
		else
		{
			SetLastTransform(obIRResult.GetSurface());
		}
	}
	else
	{
		if (m_bPointConvergerInUse)
		{
			if (m_pobNewVolumeClamped)
			{
                m_pobNewVolumeClamped = NULL;
				m_pobPointConverger->Reset();
				m_pobPointConverger->Initialise(m_obLastConvergedPoint);
			}

			m_pobPointConverger->Update(obTracking,fTimeChange);

			const CDirection obToLastKnown = (obTracking^m_pobPointConverger->GetDamped());
			m_bPointConvergerInUse = 
				(obToLastKnown.LengthSquared()>sqr(m_pobPointConverger->GetDef().GetSpeed()*0.25*fTimeChange));

			#ifdef CAM_VOLUMES_DEBUG
					CCamUtil::Render_Sphere(m_pobPointConverger->GetDamped(),0.1f,0.0f,0.0f,1.0f,1.0f);
			#endif
			SetLastTransform(m_pobPointConverger->GetDamped());
			if (!m_bPointConvergerInUse)
			{
				m_pobPointConverger->Reset();
			}
		}
		else
		{
			#ifdef CAM_VOLUMES_DEBUG
				CCamUtil::Render_Sphere(obTracking,2.0f,1.0f,1.0f,0.0f,1.0f);
			#endif
	
			SetLastTransform(obTracking);
		}
	}

	#ifdef CAM_VOLUMES_DEBUG
		CCamUtil::Render_Sphere(obTracking,1.0f,1.0f,1.0f,1.0f,1.0f);
	#endif


	return GetLastTransform();
}
	
/***************************************************************************************************
*	
*	FUNCTION		PTVolume::Render
*
*	DESCRIPTION		render stuff
*
***************************************************************************************************/
void	PTVolume::Render(void)
{
	m_pobVolume->Render(1.0f, 0.0f, 0.0f, 1.0f);

	if(TrackingValid())
		CCamUtil::Render_Line(GetLastTracked(), GetLastTransform(), 1.0f,1.0,1.0f,1.0f);
	
}
