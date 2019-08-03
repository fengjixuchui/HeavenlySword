//------------------------------------------------------------------------------------------
//!
//!	\file comboinspector.cpp
//!
//------------------------------------------------------------------------------------------

#include "comboinspector.h"
#include "editable/enumlist.h"
#include "game/attacks.h"

//------------------------------------------------------------------------------------------
//!
//!	ctor
//!
//------------------------------------------------------------------------------------------
ComboInspector::ComboInspector()
{
}

ComboInspector::AttackToComboMap ComboInspector::GatherCombos(const CAttackLink* pIdleStance, const AttackLinkCollection& obAllAttacks)
{
	AttackToComboMap result;

	for (AttackLinkCollectionIterator attackIt = obAllAttacks.begin(); attackIt != obAllAttacks.end(); ++attackIt)
	{
		result[(*attackIt)->GetAttackDataP()->m_obAttackAnimName] = SearchForComboPath(pIdleStance, *attackIt);
	}

	return result;
}

ComboInspector::AttackLinkCollection ComboInspector::SearchForComboPath(const CAttackLink* pStart, const CAttackLink* pGoal)
{
	// Search for the combo path using an A* algorithm
	typedef ntstd::List< PathToGoal > PathToGoalCollection;
	AttackLinkCollection obInspectedLinks;
	PathToGoalCollection obPathToGoalQueue;
	AttackLinkCollection obInitialPath;
	obInitialPath.push_back(pStart);
	PathToGoal obInitialPathToGoal = { obInitialPath, pGoal };
	obPathToGoalQueue.push_back(obInitialPathToGoal);
	while (!obPathToGoalQueue.empty())
	{
		PathToGoal obPathToGoal(obPathToGoalQueue.front());
		obPathToGoalQueue.pop_front();
		const CAttackLink* pLastPathElement = obPathToGoal.m_obPath.back();
		if (pLastPathElement == obPathToGoal.m_pGoal)
		{
			return obPathToGoal.m_obPath;
		}
		else
		{
			// Find all the "paths" leading off from this one and put them in the priority queue if we haven't
			// visited this "node" on the path before.
			for (int iIndex = 0; iIndex < AM_NONE + 1; ++iIndex) // AM_NONE + 1 because we're sneakily checking the ButtonHeldAttack after the links array
			{
				const CAttackLink* pNextPathElement = iIndex < AM_NONE ? pLastPathElement->m_pobLinks[iIndex] : pLastPathElement->m_pobButtonHeldAttack;
				if (pNextPathElement && ntstd::find(obInspectedLinks.begin(), obInspectedLinks.end(), pNextPathElement) == obInspectedLinks.end())
				{
					obInspectedLinks.push_back(pNextPathElement);
					AttackLinkCollection obNewPath(obPathToGoal.m_obPath);
					obNewPath.push_back(pNextPathElement);
					PathToGoal obNewPathToGoal = { obNewPath, obPathToGoal.m_pGoal }; 
					obPathToGoalQueue.push_back( obNewPathToGoal );
					obPathToGoalQueue.sort(); // sort by priority, as per the A* algorithm
				}
			}
		}
	}
	return AttackLinkCollection();
}

int ComboInspector::PathToGoal::CostToGoalHeuristic()
{
	// Generally speaking, an "f" attack will be further away from "fff" than "ff". Hence, we compare
	// the length of the strings to guess distance. Obviously this won't work for every case, but it's
	// about the best we can get for the A* heuristics.
	int result = strlen(m_pGoal->m_pobAttackData->m_obAttackAnimName.GetDebugString()) - strlen(m_obPath.back()->m_pobAttackData->m_obAttackAnimName.GetDebugString());
	return result < 0 ? 0 : result;
}
