//! -------------------------------------------
//! AINavigGraph.h
//!
//! New AIs Navigation Grpah
//!
//! Author: Dario Sancho-Pradel (DSP)
//!
//!--------------------------------------------

#ifndef _AINAVIGGRAPH_H
#define _AINAVIGGRAPH_H

#include "ainavignode.h"
#include "game/entity.h"
#include "game/entity.inl"
#include "area/areasystem.h"

// Forward declarations
//class CAINavigGraph;
class CAINavigNodesSets;

typedef struct _SNavigGraphLink SNavigGraphLink;
typedef ntstd::List<SNavigGraphLink*	, Mem::MC_AI>	SNavigGraphLinkList;
typedef ntstd::List<CAINavigGraph*		, Mem::MC_AI>	AINavigGraphList;
typedef ntstd::List<CAINavigNodesSets*	, Mem::MC_AI>	AIAINavigNodesSetsList;


class CAINavigNodesSets
{
	public:
		HAS_INTERFACE(CAINavigNodesSets)
		CAINavigNodesSets() : m_pobNavigGraph(NULL) {}
		
		AINavigNodeList*	GetIntermediateNodesList()	{ return (&m_listIntermediateNodes); }
		CHashedString		GetName()	const			{ return m_hsName; }
        
	private:
	
		AINavigNodeList		m_listIntermediateNodes;
		CHashedString		m_hsName;
		CAINavigGraph*		m_pobNavigGraph;
};


class CAINavigGraph 
{

	public:

		HAS_INTERFACE(CAINavigGraph)

		// Ctor et al.

		CAINavigGraph();

		void PostConstruct( void );
		void PostPostConstruct( void );


		// Accessors et al.

		CAINavigNode*		GetNode			( CEntity*			);	// !!! - Needed?
		CAINavigNode*		GetNodeWithName (const CHashedString & );
		AINAVIGNODE_TYPE	GetNodeType		( CEntity* );	// !!! - Needed?
		const CHashedString	GetName			( void				) const { return m_ksName; }
		bool				IsPatrolGraph	( void				) const { return m_bIsPatrolGraph; }
		bool				IsOpenPatrolGraph ( void			) const { return m_bIsOpenPatrolGraph; }
		bool				IsQueueGraph	( void				) const { return m_bIsQueueGraph; }
		bool				IsLocatorGraph	( void				) const	{ return m_bIsLocatorGraph; }
		CEntity*			GetObjectToUse	( void				) { return m_pObjectToUse; }

		// Methods
		// Note: Enable means [DOOR -> Open, LADDER -> In place]

		bool							IsDoorOpen			( CEntity* );
		bool							IsLadderAvailable	( CEntity* );
		bool							SetEnableDoor		( CEntity*, bool );
		bool							SetEnableLadder		( CEntity*, bool );
		bool							DeleteNode			( CEntity* );
		bool							IsDoor				( CEntity* pEnt) { return ( GetNodeType(pEnt) == NDT_DOOR ); }
		bool							IsLadder			( CEntity* pEnt) { return ( GetNodeType(pEnt) == NDT_LADDER ); }					
		CAINavigArrow*					GetDoor				( CEntity* );
		CAINavigArrow*					GetLadder			( CEntity* );
		CAINavigNode*					GetClosestNode		( const CPoint&, float* );
		CAINavigNode*					GetPatrolLocator	( void ) const { return m_pPatrolGraphLocator; }
		AINavigArrowList*				GetDoors			( void );			// Get all the Door arrows
		AINavigArrowList*				GetLadders			( void );			// Get all the Ladders arrows
		const AINavigArrowList*			GetTgtArrows		( CAINavigNode* ) const;
		const AINavigNodeList*			GetNodesList		( void ) { return (&m_listNodes); }
		const AINavigCoverPointList*	GetNavigCoverPoints	( void ) { return (&m_listCoverPoints); }
		
		CAINavigCoverPoint*				GetClosestCoverPointInMinMaxRange	( AI* , const CEntity*  );
		CAINavigNode*					GetANodeLinkedToCoverPoint	( CAINavigCoverPoint* ) const;

		CAINavigNode*					GetFirstVisibleNode		( const CPoint & );
		CAINavigNode*					GetAbsoluteClosestNode	( const CPoint &, float* );
		
		bool							IsNavigGraphActiveLUA	( void )	const	{ return m_bLUAEnabled; }
		void							SetNavigGraphActiveLUA	( bool b )			{ m_bLUAEnabled = b; }

		unsigned int					GetSectorBits			( void )	const	{ return m_iMappedAreaInfo; }
		bool							IsInActiveSector		( void )	const	{ return true; } //{ return (((1 << (AreaManager::Get().GetCurrActiveArea()-1)) & m_iMappedAreaInfo) != 0); }
		bool							HasInvalidSectorBits	( void )	const	{ return ( m_iMappedAreaInfo== 0 ); }
		// Extension For Set of Intermediate Nodes
		//AINavigNodeList*				GetIntermediateNodeList	( CHashedString );
	
		// DebugRender
		void							DebugRender				( void );
		void							DebugRenderQueueGraph	( void );

	private:

		//	Prevent copying.

		CAINavigGraph( const CAINavigGraph & )				NOT_IMPLEMENTED;
		CAINavigGraph &operator = ( const CAINavigGraph & )	NOT_IMPLEMENTED;	

	private:
		
		AINavigNodeList			m_listNodes;
		AINavigCoverPointList	m_listCoverPoints;
		AIAINavigNodesSetsList	m_listNodesSets;

		AINavigArrowList	m_listDoorArrows;
		AINavigArrowList	m_listLadderArrows;

		SNavigGraphLinkList	m_listNavigGraphAreaLinks;
		SNavigGraphLinkList	m_listPreviousNavigGraphAreaLinks;

		CHashedString		m_ksName;
		int					m_iRegionIndex;			// Indexing withing the CAINavigGraphManager
		bool				m_bIsPatrolGraph;		// Is it a patrolling path?
		bool				m_bIsOpenPatrolGraph;	// Is the patrolling path open ?
		CAINavigNode*		m_pPatrolGraphLocator;	// Node used to A* to the Patrol Path if far away
		CEntity*			m_pObjectToUse;
		bool				m_bIsQueueGraph;
		bool				m_bIsLocatorGraph;
		bool				m_bLUAEnabled;			// It can be disabled from LUA
		unsigned int		m_iMappedAreaInfo;		// Sector Bits
};

//! -------------------------------------------------
//! SNavigGraphLink (struct)
//!
//!	Defines the Link between two CAINavigGraph areas
//! -------------------------------------------------


typedef struct _SNavigGraphLink
{
	_SNavigGraphLink() : pobFromNavigNode(NULL), pobToNavigNode(NULL), pobToNavigGraphArea(NULL) {}

	CAINavigNode*	pobFromNavigNode;	// Link Node that belongs to this NavigGraph (Area)
	CAINavigNode*	pobToNavigNode;		// Link Node that belongs to pobNavigGraphArea
	CAINavigGraph*	pobToNavigGraphArea;// Connection with pobNavigGraphArea area
} SNavigGraphLink;



#endif // _AINAVIGGRAPH_H

