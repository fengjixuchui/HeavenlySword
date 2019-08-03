//------------------------------------------------------------------------------------------
//!                                                                                         
//!	\file camsection.h
//!                                                                                         
//------------------------------------------------------------------------------------------

#ifndef _CAMSECTION_INC
#define _CAMSECTION_INC

/////////////////////////////
// Required Includes
/////////////////////////////

class BasicCameraTemplate;
class CCombatCamDef;
//#include "basiccamera.h"
//#include "combatcamdef.h"

//------------------------------------------------------------------------------------------
//!
//!	CamSection
//!	A section of cameras for a level
//!
//------------------------------------------------------------------------------------------
class CamSection
{
public:
	HAS_INTERFACE(CamSection)
	CamSection();
	virtual ~CamSection();

	virtual void PostConstruct();

	//BasicCameraTemplate* GetBestCamera(const CPoint& POI) const;
	BasicCameraTemplate* FindCamera(const CHashedString& obName);

// Welder Elements
private:
	ntstd::List<BasicCameraTemplate*, Mem::MC_CAMERA> m_obCameraList;
	ntstd::List<CCombatCamDef*, Mem::MC_CAMERA> m_obCombatCameraList;
};

#endif  //_CAMSECTION_INC
