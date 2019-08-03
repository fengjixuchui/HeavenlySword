//------------------------------------------------------------------------------------------
//!
//!	\file aiformation_line.h
//!
//------------------------------------------------------------------------------------------

#ifndef _AIFORMATION_LINE_INC
#define _AIFORMATION_LINE_INC


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
//!	AILineRank_Formation
//!	Exposed rank for formation line
//!
//------------------------------------------------------------------------------------------
class AILineRank_Formation
{
	HAS_INTERFACE(AILineRank_Formation)

public:
	AILineRank_Formation() {fSpacing = 1.0f; fBaseAngle = -PI; iSlotsMax = 0; }

	float fDistance;	// The distance away this rank is.
	float fSpacing;		// Gaps between slots in a rank.
	float fBaseAngle;	// Initial rotation
	int iSlotsMax;		// Maximum number of slots in this rank
	int iSlotOffset;	// Offset to this rank in the formation slot array
};


//------------------------------------------------------------------------------------------
//!
//!	AIFormation_Line
//!	A circular formation...
//!
//------------------------------------------------------------------------------------------
class AIFormation_Line : public AIFormation
{
	HAS_INTERFACE(AIFormation_Line)

	typedef ntstd::List<AILineRank_Formation*, Mem::MC_AI>		RankList;

public:

	// Xml Construction
	AIFormation_Line() {}

	// Lua construction
	AIFormation_Line(const NinjaLua::LuaObject& def);

	virtual ~AIFormation_Line() 
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

	AILineRank_Formation& GetRank(int Index)
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

	// LUA defined ranks
	AILineRank_Formation* m_aLUARanks;

	// Total number of ranks.
	int m_iRanks;

	friend class AIFormationSlot;
};

#endif //_AIFORMATION_LINE_INC
