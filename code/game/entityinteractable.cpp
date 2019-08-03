//--------------------------------------------------
//!
//!	\file game/entityinteractable.cpp
//!	Definition of the Interactable entity object
//!
//--------------------------------------------------

#include "objectdatabase/dataobject.h"

#include "game/entityinteractable.h"
#include "interactioncomponent.h"

void ForceLinkFunction18()
{
	ntPrintf("!ATTN! Calling ForceLinkFunction18() !ATTN!\n");
}

START_CHUNKED_INTERFACE(Interactable, Mem::MC_ENTITY)
	DEFINE_INTERFACE_INHERITANCE(CEntity)
	COPY_INTERFACE_FROM(CEntity)

	PUBLISH_VAR_AS(m_obSceneElementDef, SceneElementDef)

	PUBLISH_VAR_WITH_DEFAULT_AS(m_bHeroCanUse,			true,		HeroCanUse)
	PUBLISH_VAR_WITH_DEFAULT_AS(m_bArcherCanUse,		true,		ArcherCanUse)
	PUBLISH_VAR_WITH_DEFAULT_AS(m_bEnemyAICanUse,		true,		EnemyAICanUse)
	PUBLISH_VAR_WITH_DEFAULT_AS(m_bAllyAICanUse,		true,		AllyAICanUse)

	DECLARE_POSTPOSTCONSTRUCT_CALLBACK(OnPostPostConstruct)

END_STD_INTERFACE


//--------------------------------------------------
//!
//!	Interactable::Interactable()
//!	Default constructor
//!
//--------------------------------------------------
Interactable::Interactable()
	:
	m_obSceneElementDef("")
{
	m_Clump = "";
	m_DefaultDynamics = "";
	m_pobParentEntity = 0;
	m_ParentTransform = "ROOT";
	m_bCastShadows = true;
	m_bRecieveShadows = true;
	m_bDisableRender = false;
	m_iMappedAreaInfo = 0;
	m_pobGameEventsList = 0;
	m_eType = EntType_Interactable;
	m_eInteractableType = EntTypeInteractable_Undefined;
	m_bHeroCanUse = true;
	m_bArcherCanUse = true;
	m_bEnemyAICanUse = true;
	m_bAllyAICanUse = true;
}

//--------------------------------------------------
//!
//!	Interactable::~Interactable()
//!	Default destructor
//!
//--------------------------------------------------
Interactable::~Interactable()
{
}


//--------------------------------------------------
//!
//!	Interactable::OnPostConstruct()
//!	scee.sbashow : Note, a number of the entity interactables, like the ladder, do not seem to call this
//! 				in their OnPostConstruct, they call CEntity::OnPostConstruct() directly... is this intentional?
//!					So have made them all go through this. 
//!					(Some did not even derive through Interactable in the first place - have recitified that also)
//--------------------------------------------------
void Interactable::OnPostConstruct()
{
	// We don't want no stinking lua construction function...
	this->m_ConstructionScript = 0;

	CEntity::OnPostConstruct();

	SceneElementComponentDef* pDef = ObjectDatabase::Get().GetPointerFromName<SceneElementComponentDef*>(m_obSceneElementDef);

	if(pDef)
	{
		SceneElementComponent* pSEC = NT_NEW_CHUNK(Mem::MC_ENTITY) SceneElementComponent( (CEntity*)this, pDef );
		SetSceneElementComponent(pSEC);
	}
}

//--------------------------------------------------
//!
//!	Interactable::OnPostPostConstruct()
//!	scee.sbashow : Sets up use points with information, 
//!				dependent on run-time interactable type.
//!				Because this is postPostConstruct, we can assume UsePoints are fully constructed and there.
//!
//--------------------------------------------------
void Interactable::OnPostPostConstruct()
{
	CEntity::OnPostPostConstruct();

	CInteractionComponent* const pobIC = GetInteractionComponent();
	pobIC->RegisterWithUsePoints();
}

