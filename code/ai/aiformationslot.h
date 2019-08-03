//------------------------------------------------------------------------------------------
//!
//!	\file aiformationslot.h
//!
//------------------------------------------------------------------------------------------

#ifndef _AIFORMATIONSLOT_INC
#define _AIFORMATIONSLOT_INC

#include "core/util.h"

//------------------------------------------------------------------------------------------
// External Decls.                           
//------------------------------------------------------------------------------------------

class AI;
class AIFormation;



//------------------------------------------------------------------------------------------
//!
//!	AIFormationSlot
//!	A slot for an AI to fill in a formation.
//!
//------------------------------------------------------------------------------------------
class AIFormationSlot
{
	friend class AIFormation;
	HAS_INTERFACE( AIFormationSlot );

public:
	AIFormationSlot()						                   : m_pt(CONSTRUCT_CLEAR),m_bDelete(false)	{m_pEnt = 0; m_pFormation = 0; m_bInPosition = false; m_bNavigable = false; m_bClearOfOtherSlots = false; m_bInRange = false;}
	AIFormationSlot(AIFormation* pFormation)                   : m_pt(CONSTRUCT_CLEAR),m_bDelete(true)	{m_pEnt = 0; m_pFormation = pFormation; m_bInPosition = false; m_bNavigable = false; m_bClearOfOtherSlots = false; m_bInRange = false;}
	AIFormationSlot(AIFormation* pFormation, const CPoint& pt) : m_pt(pt),m_bDelete(true)				{m_pEnt = 0; m_pFormation = pFormation; m_bInPosition = false; m_bNavigable = false; m_bClearOfOtherSlots = false; m_bInRange = false;}
	virtual ~AIFormationSlot();

	// Can the slot be deleted, only slots that don't call the default constructor will be deleted.
	bool CanDelete(void) const { return m_bDelete; }

	// Accessors.
	void SetEntity(AI* pEnt) { m_pEnt = pEnt; }
	AI* GetEntity() const { return m_pEnt; }

	void SetLocalPoint(const CPoint& pt) {m_pt = pt;}
	CPoint GetLocalPoint() const    {return m_pt;}
	CPoint GetWorldPoint() const;

	void SetNavigable(bool bNavigable) { m_bNavigable = bNavigable; }
	void SetClearOfOtherSlots(bool bClearOfOtherSlots) { m_bClearOfOtherSlots = bClearOfOtherSlots; }
	bool IsValid() const { return m_bNavigable && m_bClearOfOtherSlots; }

	// Are we in a position?
	void SetInPosition(bool bInPosition) const { m_bInPosition = bInPosition; }
	bool IsInPosition() const { return m_bInPosition; }

	// Equality operators.
	bool operator==(const AIFormationSlot& rhs) const {return this == &rhs;}
	bool operator!=(const AIFormationSlot& rhs) const {return this != &rhs;}

	// Is the entity allowed to attack?
	virtual bool IsEntityAllowedToAttack(void) const { return true; }

	// Set the formation
	void SetFormation(AIFormation* pFormation) { m_pFormation = pFormation; }
	AIFormation* GetFormation() { return m_pFormation; }

	// Is entity in formation range?
	void UpdateInRange();
	void SetInRange(bool bInRange) { m_bInRange = bInRange; }
	bool IsInRange() const { return m_bInRange; }

#ifndef _RELEASE
	static void RenderSquare(CPoint const& Point, float fSize, unsigned int uiFormationColour);
	static void RenderCross(CPoint const& Point, float fSize, unsigned int uiFormationColour);
	static void RenderPlus(CPoint const& Point, float fSize, unsigned int uiFormationColour);
	void RenderDebugInfo(unsigned int uiFormationColour);
#endif

protected:

	// The entity in this slot.
	AI* m_pEnt;             

	// Our parent formation.
	AIFormation* m_pFormation;

private:

	// This slots local position.
	CPoint m_pt;               

	// As the data can create slots, the destruction needs to know if this can be deleted
	bool m_bDelete;			

	// Clear path from target to slot.
	bool m_bNavigable;

	// No higher priority formation's slots intersect with this slot.
	bool m_bClearOfOtherSlots;

	// Entity is in position.
	mutable bool m_bInPosition;

	bool m_bInRange;
};

#endif // _AIFORMATIONSLOT_INC
