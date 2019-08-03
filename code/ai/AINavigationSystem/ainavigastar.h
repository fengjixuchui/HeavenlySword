//! -------------------------------------------
//! AINavigAStar.h
//!
//! A* Class for the Navigation Graph 
//!
//! Author: Dario Sancho-Pradel (DSP)
//!
//!--------------------------------------------

#ifndef _AINAVIGASTAR_H
#define _AINAVIGASTAR_H

#include "ainavignode.h"
#include "ainavigpath.h"

//! -------------------------------------------------
//! SNavigAStarNode
//!
//!	Navigation Grpah's A* Node Structure 
//! -------------------------------------------------

typedef struct _SNavigAStarNode
{
	_SNavigAStarNode(): pParentNode(NULL), pobNavigNode(NULL), c(0.f), s(0.f), h(0.f) {}
	_SNavigAStarNode(_SNavigAStarNode* pPN, CAINavigNode* pNN, float c_in, float s_in, float h_in) : 
					pParentNode(pPN), pobNavigNode(pNN), c(c_in), s(s_in), h(h_in) {}

	bool IsEqual(_SNavigAStarNode* pASN)	{ return (pASN->pobNavigNode == pobNavigNode); }
	//bool IsValid( void )					{ return (pobNavigNode != NULL); }

	_SNavigAStarNode*	pParentNode;
	CAINavigNode*		pobNavigNode;
	float				c,s,h;			// Total cost (c) = Cost from Start (s) + Heuristic (h)

} SNavigAStarNode;
typedef ntstd::List<SNavigAStarNode*, Mem::MC_AI> SNavigAStarNodeList;

//! -------------------------------------------------
//! CNavigAStar
//!
//!	A* Class for the Navigation Graph 
//! -------------------------------------------------

class CNavigAStar
{
	public:
		// Ctor et al.
		CNavigAStar();
		~CNavigAStar();

		bool MakePath ( CAINavigNode*, CAINavigNode*, CAINavigPath* );
		bool MakePath ( const CPoint&, CAINavigNode*, CAINavigPath* );
		bool MakePath ( const CPoint&, const CPoint&, CAINavigPath* );

	private:
		
		enum ENUM_ASTAR_LIST_TYPE 
		{
			ASTAR_OPEN_LIST		= 0,
			ASTAR_CLOSED_LIST	= 1,
		};
		void				FreeData		( void );
		inline void			SetStartEndNodes( CAINavigNode* pobStart, CAINavigNode* pobEnd );
		bool				MakePath		( CAINavigPath* );	// True -> Path found, False otherwise	

		SNavigAStarNode*	HasNode							( CAINavigNode*		, ENUM_ASTAR_LIST_TYPE );
		//bool				HasNode							( SNavigAStarNode*	, ENUM_ASTAR_LIST_TYPE ); // !!! - Needed?
		void				AddAStarNode					( SNavigAStarNode*	, ENUM_ASTAR_LIST_TYPE );
		SNavigAStarNode*	GetAStarNodeContaining			( CAINavigNode*		, ENUM_ASTAR_LIST_TYPE) const; // !!! - Needed?
		SNavigAStarNode*	GetLowestCostNodeFromOpenList	( void )									const;


	private:

		//	Prevent copying.

		CNavigAStar( const CNavigAStar & )				NOT_IMPLEMENTED;
		CNavigAStar &operator = ( const CNavigAStar & )	NOT_IMPLEMENTED;

	private:
		typedef ntstd::List<SNavigAStarNode*, Mem::MC_AI> SNavigAStarNodeList;

		SNavigAStarNodeList				m_OpenList;			// List of A* Open Nodes
		SNavigAStarNodeList				m_ClosedList;		// List of A* Closed Nodes
		SNavigAStarNode*				m_pobStartAStarNode;	//
		SNavigAStarNode*				m_pobGoalAStarNode;	//
		bool							m_bIsInitialised;	// Are lists empty and no dynamic data, etc...


	
};

#endif // _AINAVIGASTAR_H


