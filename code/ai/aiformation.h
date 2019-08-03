//------------------------------------------------------------------------------------------
//!                                                                                         
//!	\file aiformation.h                                                                     
//!                                                                                         
//------------------------------------------------------------------------------------------

#ifndef _AIFORMATION_INC
#define _AIFORMATION_INC

//------------------------------------------------------------------------------------------
// Includes for inheritance or for members                                                  
//------------------------------------------------------------------------------------------

#include "core/boostarray.h"
#include "core/boundingvolumes.h"
#include "lua/ninjalua.h"
#include "editable/enums_formations.h"

//------------------------------------------------------------------------------------------
// Constants                                                                                
//------------------------------------------------------------------------------------------

#define USE_DEFAULT									-999.0f

#define EXIT_DISTANCE_THRESHOLD					2.0f;
#define ENTRY_DISTANCE_THRESHOLD				2.0f;

//------------------------------------------------------------------------------------------
// External Decls.                                                                          
//------------------------------------------------------------------------------------------

class AIFormationAttack;
class AIFormationSlot;
class CEntity;
class AI;
class AISafeZone;
class FormationComponent;

//------------------------------------------------------------------------------------------
// Enumerations, would like to access in LUA                                                
//------------------------------------------------------------------------------------------

enum AI_FORMATION_ATTACK_STATE
{
	AFAS_NOTVALID,
	AFAS_VALID,
	AFAS_WAITING_FOR_VALID_PLAYER,
	AFAS_WAITING_FOR_PLAYER_TO_GETUP,
	AFAS_NO_VALID_ENTITIES,
	AFAS_NOT_ENOUGH_VALID_ENTITIES,
	AFAS_NOT_ENOUGH_ENTITIES,
};

//------------------------------------------------------------------------------------------
//!                                                                                         
//!	AIFormation                                                                             
//!	The abstract base class for all types of formation.                                     
//!                                                                                         
//------------------------------------------------------------------------------------------
class AIFormation
{
	friend class FormationComponent;
	friend class AIBehaviour_Formation;
	HAS_INTERFACE(AIFormation)

public:
	AIFormation();
	virtual ~AIFormation();

	HAS_LUA_INTERFACE()

	// Is the formation active. 
	bool IsActive() const { return m_bActive; }
	void SetActive(bool bState, FormationComponent* pFormationComponent);

	// Return the priority of the formation.
	float GetPriority(void) const { return m_fPriority; }

	// Can this formation be updated this frame with the current ticker value?
	bool CanUpdate(uint64_t uiTestTicker) 
	{ 
		if(m_uiUpdateTicker != uiTestTicker)
		{
			m_uiUpdateTicker = uiTestTicker;
			return true;
		}
		return false;
	}

	// Method to construct the formation if required
	void XmlConstruct()
	{
		if (!m_bIsXMLConstructed)
		{
			m_bIsXMLConstructed = true;
			PostXmlConstruct();
			AllocateSlots();
		}
	}

private:

	// Slot management.
	/////////////////////////////////////////////////////////////////

	void UnassignSlot(AIFormationSlot& slot);

	AIFormationSlot* AssignSlot(AI* pEntity, bool bTestOnly = false);

protected:

	virtual void PostXmlConstruct();

public:

	void AllocateSlots();

	void RemoveSlot(AIFormationSlot& slot);

	bool RemoveEntity(AI* pEntity);

	const AIFormationSlot* FindSlot(const AI& entity) const;

	int EntityCount() {return m_iEntsInFormation;}

	AI* GetEntity(int Index);

	// Formation Updates
	virtual void Update(float fTimeDelta, unsigned int uiFormationColour, FormationComponent* pFormationComponent);

	virtual void UpdatePositions(float) { }

	virtual void CalculateSlotPositions() {};

	void SwitchSlotsToReduceTravel();

	// Target points for the formation and entities in the formation
	const CPoint GetTarget() const;
	const CPoint GetLockonTarget() const;

	// Formation Attacks.
	int GetEntsReadyForAttack(ntstd::List<AI*>& rEntityList) const;

	// Name access
	const ntstd::String& GetName(void) const { return m_Name; }
	void SetName(const char* pcName) { m_Name = pcName; }

	const ntstd::String& GetIdleAnim(void) const { return m_IdleAnim; }
	virtual bool IsAgressive(void) const { return m_bAggressive; }

	float GetExitDistance() const { return m_fExitDistance; }

	float GetEntryDistance() const { return m_fEntryDistance; }

	// Get into formation within this distance.
	float m_fEntryDistance;

	// Out of formation beyond this distance.
	float m_fExitDistance;

	// Formation rotates relative to camera. 
	bool m_bCameraRelative;

	// Instead of filling slots in order, entities should go to the nearest valid ones.
	bool m_bClosestSlot;

	bool m_bOverlapChecked;

	// Health Changed
	void HealthChanged(CEntity*, float fNewHealth, float fBaseHealth);

	// Owner access
	void SetOwner(CEntity* pEntity) { m_pOwner = pEntity; }
	CEntity* GetOwner() const { return m_pOwner; }

#ifndef _RELEASE
	static void RenderPlus(CPoint const& Point, float fSize, unsigned int uiFormationColour);
	void RenderDebugInfo(unsigned int uiFormationColour);
#endif

////////////////////////////////////////////////////////////////////////
// Helper Functions                                                   
////////////////////////////////////////////////////////////////////////
protected:
	
	// Slot Management.
	/////////////////////////////////////////////////////////////////
	void ClearSlots(bool bDestructing=false);
	AIFormationSlot* FindSlot(const AI& entity);
	bool IsSlotAvailable() const {return m_iEntsInFormation < m_iSlotsInFormation;}

////////////////////////////////////////////////////////////////////////
// Members
////////////////////////////////////////////////////////////////////////
protected:

	// The entity this formation is managed by
	CEntity* m_pOwner;

	// Our ID
	ntstd::String m_Name;

	// Our ticker ID
	uint64_t m_uiUpdateTicker;

	// Active?
	bool m_bActive;

	// Made from XML?
	bool m_bIsXMLConstructed;

	// Formation Slots
	AIFormationSlot** m_pSlots;

	// Total slots
	int m_iSlotsInFormation; 

	// Slots filled
	int m_iEntsInFormation;      

	// Is the formation relative to another entity?
	CEntity* m_pTargetEntity;

	CEntity* m_pLockonEntity;

	CPoint   m_ptOffset;

	float m_fConstraintAngle;

	// Priority of the formation. This is only used when formations overlap, the lesser priority formation must give way
	float m_fPriority;

	// Does the formation have incidentals to play on entities?
	NinjaLua::LuaObject m_IncidentalAnimList;

	// Idle formation anim
	ntstd::String m_IdleAnim;

protected:

	bool m_bAggressive;
	float m_fIdleAnimSpeed;

public:

	float GetIdleAnimSpeed() const { return m_fIdleAnimSpeed; }

private:

	// Our manager is our friend.
	friend class AIFormationManager;

	// This is so I can get the slots
	friend class AIFormationSet;
};
typedef ntstd::List<AIFormation*, Mem::MC_AI>		AIFormationList;

LV_DECLARE_USERDATA(AIFormation);

//------------------------------------------------------------------------------------------
// Formation Definition Strings                                                             
//------------------------------------------------------------------------------------------

#define DEF_AGGRESSIVE			"aggressive"
#define DEF_ANGLE				"angle"
#define DEF_ANIM_SPEED_TEST		"AnimSpeedTest"
#define DEF_BASE_ANGLE			"base_angle"
#define DEF_CAMERARELATIVE     	"camerarelative"
#define DEF_CLOSEST_SLOT     	"closest_slot"
#define DEF_COMPRESS        	"compress"
#define DEF_CONSTRAINT_ANGLE    "constraint_angle"
#define DEF_DISTANCE			"distance"
#define DEF_IDLE_ANIM			"idle_anim"
#define DEF_LENGTH          	"length"
#define DEF_LOCKON_ENTITY   	"lockon_entity"
#define DEF_PLACES				"places"
#define DEF_PRIORITY     		"priority"
#define DEF_RADIUS				"radius"
#define DEF_RANKS           	"ranks"
#define DEF_SLOTCOUNT       	"slotcount"
#define DEF_SPACING         	"spacing"
#define DEF_TARGET_ENTITY     	"target_entity"
#define DEF_TARGET_OFFSET     	"target_offset"
#define DEF_TYPE            	"type"

#endif //_AIFORMATION_INC
