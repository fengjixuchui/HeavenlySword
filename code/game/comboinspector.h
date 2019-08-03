//------------------------------------------------------------------------------------------
//!
//!	\file comboinspector.h
//!
//------------------------------------------------------------------------------------------
#if !defined( HS_COMBOINSPECTOR_H )
#define HS_COMBOINSPECTOR_H

// forward decl
class CAttackLink;

// The combo inspector parses all combos out of a given combat move data set, and will find
// (at most) one combo for every move given, provided it can be found from the "idle stance"
// attack cluster. Implementation uses an A* search algorithm.
class ComboInspector
{
public:
	typedef ntstd::List<const CAttackLink*> AttackLinkCollection;
	typedef ntstd::List<const CAttackLink*>::const_iterator AttackLinkCollectionIterator;
	typedef ntstd::Map<const CHashedString, AttackLinkCollection > AttackToComboMap;
	
	static AttackToComboMap GatherCombos(const CAttackLink* pIdleStance, const AttackLinkCollection& obAllAttacks);
	static AttackLinkCollection SearchForComboPath(const CAttackLink* pStart, const CAttackLink* pGoal);
private:
	// Simple class to encapsulates a path to a goal, with the path travelled so far and the intended goal.
	class PathToGoal
	{
	public:
		AttackLinkCollection m_obPath;
		const CAttackLink* m_pGoal;
		
		bool PathToGoal::operator<( PathToGoal &other )
		{
			return m_obPath.size() + CostToGoalHeuristic() < other.m_obPath.size() + other.CostToGoalHeuristic();
		}
	private:
		int CostToGoalHeuristic();
	};

	// hide the constructor
	ComboInspector();
};

#endif // end HS_COMBOINSPECTOR_H
