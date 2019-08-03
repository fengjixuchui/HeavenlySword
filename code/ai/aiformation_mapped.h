//------------------------------------------------------------------------------------------
//!
//!	\file aiformation_mapped.h
//!
//------------------------------------------------------------------------------------------

#ifndef _AIFORMATION_MAPPED_INC
#define _AIFORMATION_MAPPED_INC


/////////////////////////////////////////////
// Includes for inheritance or for members 
/////////////////////////////////////////////
#include "ai/aiformation.h"
#include "ai/aiformationslot.h"
#include "ai/ainavgraphmanager.h"

#include "game/randmanager.h"

class CAIComponent;

//------------------------------------------------------------------------------------------
//!
//!	AIFormation_Mapped
//!	A circular formation...
//!
//------------------------------------------------------------------------------------------
class AIFormation_Mapped : public AIFormation
{
	HAS_INTERFACE(AIFormation_Mapped)

	typedef ntstd::Vector<CPoint, Mem::MC_AI>		PointList;

public:

	// Xml Construction
	AIFormation_Mapped() {}

	// Lua construction
	AIFormation_Mapped(const NinjaLua::LuaObject& def);

	virtual ~AIFormation_Mapped() 
	{ 
		ClearSlots(true);

		if (m_aLUAPoints)
		{
			NT_DELETE_ARRAY_CHUNK(Mem::MC_AI, m_aLUAPoints); 
			m_aLUAPoints = NULL;
		}
	}

	void ValidateData();

protected:

	virtual void PostXmlConstruct();

private:

	virtual void CalculateSlotPositions();

	AIFormationSlot* GetSlot(int iPoint);

	CPoint* GetPoint(int Index)
	{
		if (m_aLUAPoints)
		{
			ntAssert(Index >= 0 && Index < m_iPoints);
			return &m_aLUAPoints[Index];
		}

		return &m_aXMLPoints[Index];
	}

	// Return the number of points in the formation
	int GetPoints() const
	{
		return m_iPoints;
	}

private:

	// Xml defined points. 
	PointList m_aXMLPoints;

	// LUA defined points.
	CPoint* m_aLUAPoints;

	// Total number of points.
	int m_iPoints;

	friend class AIFormationSlot;
};

#endif //_AIFORMATION_MAPPED_INC
