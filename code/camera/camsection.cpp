//------------------------------------------------------------------------------------------
//!                                                                                         
//!	\file camsection.cpp
//!                                                                                         
//------------------------------------------------------------------------------------------


////////////////////////////
// Required Includes
////////////////////////////
#include "camera/camsection.h"
#include "camera/camman.h"
#include "camera/basiccamera.h"
#include "core/osddisplay.h"
#include "objectdatabase/dataobject.h"

//////////////////////////////
// CamSection Interface
//////////////////////////////
START_CHUNKED_INTERFACE(CamSection, Mem::MC_CAMERA)
	PUBLISH_PTR_CONTAINER_AS( m_obCombatCameraList, CombatCameraList )
	PUBLISH_PTR_CONTAINER_AS( m_obCameraList, CameraList)	
	DECLARE_POSTCONSTRUCT_CALLBACK( PostConstruct )
END_STD_INTERFACE


//------------------------------------------------------------------------------------------
//!
//!	CamSection::CamSection
//!	Construction
//!
//------------------------------------------------------------------------------------------
CamSection::CamSection()
{
}


//------------------------------------------------------------------------------------------
//!
//!	CamSection::~CamSection
//!	Destruction
//!
//------------------------------------------------------------------------------------------
CamSection::~CamSection()
{
	OSD::Add(OSD::CAMERA, DC_WHITE, "CamSection Destructing\n");

	// Remove the section from the manager.
	if(CamMan::Exists())
		CamMan::Get().RemoveCameraSection(this);
}


//------------------------------------------------------------------------------------------
//!
//!	CamSection::PostConstruct
//!	Called after the item is loaded from XML
//!
//------------------------------------------------------------------------------------------
void CamSection::PostConstruct()
{
	for(ntstd::List<BasicCameraTemplate*, Mem::MC_CAMERA>::const_iterator it = m_obCameraList.begin(); it != m_obCameraList.end(); it++)
	{
		BasicCameraTemplate* pCam = *it;
		ntPrintf("Adding Camera: %s\n", ntStr::GetString(pCam->GetCameraName()));
	}

	CamMan::Get().AddCameraSection(this);
}


//------------------------------------------------------------------------------------------
//!
//!	CamSection::FindCamera
//!	Find a camera with a given name.
//!
//------------------------------------------------------------------------------------------
BasicCameraTemplate* CamSection::FindCamera(const CHashedString& obName)
{
	for(ntstd::List<BasicCameraTemplate*, Mem::MC_CAMERA>::const_iterator it = m_obCameraList.begin(); 
		it != m_obCameraList.end(); it++)
	{
		if(obName==(*it)->GetCameraName())
			return *it;
	}

	return 0;
}
