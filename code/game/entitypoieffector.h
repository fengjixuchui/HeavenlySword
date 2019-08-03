//------------------------------------------------------------------------------------------
//!
//!	\file game/entitypoieffector.h
//!	Definition of the POI effector entity object
//!
//------------------------------------------------------------------------------------------

#ifndef	_ENTITY_POIEFFECTOR_H
#define	_ENTITY_POIEFFECTOR_H

#include "game/entity.h"
#include "camera/sceneelementcomponent.h"

//------------------------------------------------------------------------------------------
//!
//! POI_Effector
//! A very simple entity which just has an influence on camera's POI.
//!
//------------------------------------------------------------------------------------------
class POI_Effector : public CEntity
{
public:
	void OnPostPostConstruct();

	void SetActive(bool bActive);
	
private:
	SceneElementComponentDef  m_def;

	HAS_INTERFACE(POI_Effector)
};

#endif //_ENTITY_POIEFFECTOR_H
