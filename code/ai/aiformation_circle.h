//------------------------------------------------------------------------------------------
//!
//!	\file aiformation_circle.h
//!
//------------------------------------------------------------------------------------------

#ifndef _AIFORMATION_CIRCLE_INC
#define _AIFORMATION_CIRCLE_INC


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
//!	AICircleRank_Formation
//!	Exposed rank for formation circle
//!
//------------------------------------------------------------------------------------------
class AICircleRank_Formation
{
	HAS_INTERFACE(AICircleRank_Formation)

public:
	AICircleRank_Formation() {fCompression = 1.0f; fBaseAngle = -PI; iSlotsMax = 0; }

	float fRadius; // The radius of this rank
	float fCompression;		 // Squeeze the rand
	float fBaseAngle;		 // Initial rotation
	int iSlotsMax; // Maximum number of slots in this rank
	int iSlotOffset; // Offset to this rank in the formation slot array
};


//------------------------------------------------------------------------------------------
//!
//!	AIFormation_Circle
//!	A circular formation...
//!
//------------------------------------------------------------------------------------------
class AIFormation_Circle : public AIFormation
{
	HAS_INTERFACE(AIFormation_Circle)

	typedef ntstd::List<AICircleRank_Formation*, Mem::MC_AI> RankList;

public:

	// Xml Construction
	AIFormation_Circle() {}

	// Lua construction
	AIFormation_Circle(const NinjaLua::LuaObject& def);

	virtual ~AIFormation_Circle() 
	{ 
		ClearSlots(true); 

		if (m_aLUARanks)
		{
			NT_DELETE_ARRAY_CHUNK(Mem::MC_AI, m_aLUARanks); 
			m_aLUARanks = 0;
		}
	}

	void ValidateData();

protected:

	virtual void PostXmlConstruct();

private:

	virtual void CalculateSlotPositions();

	AIFormationSlot* GetSlot(int iRank, int iSlot);

	AICircleRank_Formation& GetRank(int Index)
	{
		if (m_aLUARanks)
		{
			ntAssert(Index >= 0 && Index < m_iRanks);
			return m_aLUARanks[Index];
		}

		int i = 0;

		// Find the rank from the list. 
		RankList::iterator obIt = m_aXMLRanks.begin();
		while (i != Index)
		{
			++obIt;
			++i;
		}
		ntAssert(obIt != m_aXMLRanks.end());
		return *(*obIt);
	}

	// Return the number of ranks in the formation
	int GetRankCount() const
	{
		return m_iRanks;
	}

private:

	// Xml defined ranks. 
	RankList m_aXMLRanks;

	AICircleRank_Formation* m_aLUARanks;

	// Total number of ranks.
	int m_iRanks;

	friend class AIFormationSlot;
};

#endif //_AIFORMATION_CIRCLE_INC
