//! -------------------------------------------
//! AINavigNode.h
//!
//! Node element for the AIs navigation graph 
//!
//! Author: Dario Sancho-Pradel (DSP)
//!
//!--------------------------------------------

#ifndef _AINAVIGNODE_H
#define _AINAVIGNODE_H

#include <limits.h>
#include "game/entityai.h"

// Forward declaration
class CAINavigNode;
class CAINavigGraph;
class GameEvent;
class CAINavigArrow;
class CAINavigCoverPoint;
class CAIMovement;

typedef ntstd::List<CHashedString		, Mem::MC_MISC>	HashedStringVector;
typedef ntstd::List<CAINavigArrow*		, Mem::MC_MISC>	AINavigArrowList;
typedef ntstd::List<CAINavigNode*		, Mem::MC_MISC>	AINavigNodeList;
typedef ntstd::Vector<CAINavigNode*		, Mem::MC_AI>	AINavigNodeVector;
typedef ntstd::List<CAINavigCoverPoint*	, Mem::MC_AI>	AINavigCoverPointList;

//!--------------------------------------------
//! Enumerations: Arrow, Node and Distance Type
//!--------------------------------------------
enum AINAVIGARROW_TYPE {
	NAT_INVALID = -1,
	NAT_DEFAULT = 0,
	NAT_COVER_WAY,
	NAT_DOOR,
	NAT_LADDER,
	NAT_CORRIDOR,
	NAT_NUM_ITEMS,
};
enum AINAVIGNODE_TYPE {
	NDT_INVALID = -1,
	NDT_DEFAULT = 0,
	NDT_DOOR,
	NDT_LADDER,
	NDT_CORRIDOR,
	NDT_NUM_ITEMS,
};

enum ENUM_DISTANCE_TYPE {
	DT_MANHATTAN = 0,
	DT_EUCLIDES_SQR,
	DT_EUCLIDES,
};

enum ENUM_COVER_ANIM_TYPE {
	ANIM_IN_COVER = 0,
	ANIM_TO_COVER ,
	ANIM_BREAK_COVER,
	ANIM_PEEK_COVER,
};

enum ENUM_TC_ANIM_TYPE {
	ANIM_TC_INVALID = -1,
	ANIM_TC_BREAK_COVER_LEFT = 0,
	ANIM_TC_ENTER_COVER_RIGHT,
	ANIM_TC_BREAK_COVER_RIGHT,
	ANIM_TC_ENTER_COVER_LEFT,
};

//! -------------------------------------------------
//! CAINavigCoverPoint
//!
//!	Cover Point linked to a NavigGraph
//! -------------------------------------------------
typedef struct _SCoverPointAnimSets
{
	void PostConstruct( void );

	HashedStringVector listhsPeekFromCoverAnims;
	HashedStringVector listhsBreakCoverAnims;
	HashedStringVector listhsTakeCoverAnims;
	HashedStringVector listhsInCoverAnims;

	HashedStringVector listhsLongLeftTakeCoverAnims;
	HashedStringVector listhsShortLeftTakeCoverAnims;

	HashedStringVector listhsLongRearTakeCoverAnims;
	HashedStringVector listhsShortRearTakeCoverAnims;

	HashedStringVector listhsLongRightTakeCoverAnims;
	HashedStringVector listhsShortRightTakeCoverAnims;

} SCoverPointAnimSets;


class CAINavigCoverPoint
{
	public:

		HAS_INTERFACE(CAINavigCoverPoint)

		CAINavigCoverPoint() :	m_obValidDir(CONSTRUCT_CLEAR),m_obPos(CONSTRUCT_CLEAR), m_ksName("EMPTY"), m_pobNavigGraph(NULL), 
								m_fRadiusSQR(0.1f),m_bUseValidDir(false), m_fChanceOfUse(0.0f), m_bAvailable(true), m_bDestroyed(false),
								m_pClaimer(NULL), m_bJumpOver(false) {}

		void	PostConstruct	( void );

		const CHashedString	GetName			( void ) const { return m_ksName; }
		bool				IsAvailabe		( void ) const { return m_bAvailable; }
		float				GetChanceOfUse	( void ) const { return m_fChanceOfUse; }
		CPoint				GetPos			( void ) const { return m_obPos; }
		float				GetRadiusSQR	( void ) const { return m_fRadiusSQR; }

		void				SetAvailabe		( bool b )	   { m_bAvailable = b; }
		void				Destroy			( void )	   { m_bDestroyed = true; }	
		bool				IsDestroyed		( void ) const { return m_bDestroyed; }

		const CHashedString GetAnimation	( ENUM_COVER_ANIM_TYPE, bool *, CEntity* = NULL );
		CDirection			GetValidDir		( void ) const { return m_obValidDir; }
		bool				IsValidDirUsed	( void ) const { return m_bUseValidDir; }
		bool				IsCoverInEnemysDirection ( const CPoint & pt3D );
		const CEntity*		GetClaimer		( void ) const	{ return m_pClaimer; }
		void				SetClaimer		( CEntity* pC )	{ m_pClaimer = pC; }
		void				SetJumpOver		( bool b )		{ m_bJumpOver = b; }
		bool				IsJumpOver		( void ) const	{ return m_bJumpOver; }


	private:

		SCoverPointAnimSets*	m_pSCoverPointAnimSets;

		CDirection			m_obValidDir;		
		CPoint				m_obPos;
		CHashedString		m_ksName;
		CAINavigGraph*		m_pobNavigGraph;	// Belongs to this navig graph
		float				m_fRadiusSQR;		// Radius (Threshold)
		bool				m_bUseValidDir; 
		float				m_fChanceOfUse;		// Probaility to be used [0,1]
		bool				m_bAvailable;		// Already occupied by another AI?
		bool				m_bDestroyed;		// If true, the cover point desapears
		CEntity*			m_pClaimer;			// Entity that books the Cover Point
		bool				m_bJumpOver;		// The Cover point is accessed by jumping over the cover
};


//! -------------------------------------------------
//! CAINavigArrow
//!
//!	Defines a connection arrow between two nodes
//! -------------------------------------------------

class CAINavigArrow
{
	public:

		HAS_INTERFACE(CAINavigArrow)

		~CAINavigArrow();
		void PostConstruct();

		static const int AINAVIGNODE_MAX_WEIGHT = INT_MAX-2;

		AIEntityList	m_listAcceptEntityTypes;	// List of AI types to be accepted (e.g. Shieldsman_Type)
		AIEntityList	m_listRejectEntityTypes;	// List of AI types to be rejected (e.g. Ninja_Type)
		AIEntityList	m_listAcceptEntity;			// List of AI entities to be accepted (e.g. Main_Commander)
		AIEntityList	m_listRejectEntity;			// List of AI entities to be rejected (e.g. General)

		CHashedString			m_ksName;			// Arrow Name !!! - Needed?
		CAINavigNode*			m_pobSrcNode;		// X ---> O (Source Node)
		CAINavigNode*			m_pobTgtNode;		// O ---> X (Target Node)
		CAINavigCoverPoint*		m_pobCoverPoint;
		CEntity*				m_pobEntity;		// Pointer to the Arrow Entity (e.g. Entity Door or Ladder)
		float					m_fDistance;		// Distance between to m_pobSrcNode and m_pobTgtNode
		float					m_fWidth;			
		AINAVIGARROW_TYPE		m_eType;			// Arrow Type (Default, Door, Ladder)
		float					m_iWeight;			// Connection Weight
		bool					m_bIsEnabled;		// Can this arrow be travelled
		bool					m_bIsBusy;			// Is the connection busy (e.g. Ladder in use)
		bool					m_bRejectTypeOn;	// Consider the rejection type list rather than the acceptance list
		bool					m_bRejectEntityOn;	// Consider the rejection entity list rather than the acceptance list
		CEntity*				m_pobEntityToUse;	// Pointer to the Entity to be used if traversing this arrow in follow_path_cover behaviour

		bool					m_bToCoverPoint;	// Does the arrow leads to a cover point?
		

	public:

		CAINavigArrow() :	m_ksName(""), m_pobSrcNode(NULL), m_pobTgtNode(NULL), m_fDistance(0.0f), m_fWidth(0.0f), m_eType(NAT_DEFAULT),
							m_iWeight(0.0f), m_bIsEnabled(true), m_bIsBusy(false), m_bRejectTypeOn(true), m_bRejectEntityOn(true), 
							m_pobEntityToUse(NULL), m_bToCoverPoint(false) {}

		void				SetEnable		(bool b) { m_bIsEnabled = b; }
		void				SetBusy			(bool b) { m_bIsBusy = b; }
		bool				IsBusy			( void ) const	{ return m_bIsBusy; }
		bool				IsEnabled		( void ) const	{ return m_bIsEnabled; }
		CAINavigNode*		GetSrcNode		( void ) const	{ return m_pobSrcNode; }
		CAINavigNode*		GetTgtNode		( void ) const	{ return m_pobTgtNode; }
		const CEntity*		GetEntity		( void ) const	{ return m_pobEntity; }
		float				GetDistance		( void ) const	{ return m_fDistance; }
		float				GetWidth		( void ) const	{ return m_fWidth; }
		AINAVIGARROW_TYPE	GetType			( void ) const	{ return m_eType; }

		bool				GetLeadsToCover ( void ) const	{ return m_bToCoverPoint; }
		CAINavigCoverPoint* GetCoverPoint	( void ) const  { return m_pobCoverPoint; }

		CEntity*			GetEntityToUse	( void ) const	{ return m_pobEntityToUse; }
		void				SetEntityToUse	( CEntity* pE )	{ m_pobEntityToUse = pE; }
};



//! -------------------------------------------------
//! CAINavigNode
//!
//!	Defines a Navigation NODE
//! -------------------------------------------------
typedef struct _STCAnimSets
{
	//void PostConstruct( void );

	HashedStringVector listhsBreakLeftTCAnims;
	HashedStringVector listhsBreakRightTCAnims;
	HashedStringVector listhsReturnLeftTCAnims;
	HashedStringVector listhsReturnRightTCAnims;
	HashedStringVector listhsHideCrouchingTCAnims;
	ENUM_TC_ANIM_TYPE  eReturnToCoverSide;

} STCAnimSets;

class CAINavigNode {

	public:

		HAS_INTERFACE(CAINavigNode)
		
		// Ctor, et al.
		CAINavigNode() :	m_pSTCAnimSets(NULL), m_obPos(CONSTRUCT_CLEAR), m_pobEntity(NULL), m_pobEntityToUse(NULL), m_eNodeType(NDT_DEFAULT), 
							m_pobNavigGraph(NULL), m_pobGameEvent(NULL), m_fNodeRadiusSQR(1.0f), m_fNodeRadius(1.0f),
							m_fNoWallDetectRadiusSQR(1.0f), 
							m_bDisableWallDetFlag(false), m_bIsQueueNode(false), m_iQueueIndex(0), m_iQueueRow(0),
							m_bDynaicallyAllocated(false), m_obWhackAMoleDir(CONSTRUCT_CLEAR) {}

		~CAINavigNode();
		void PostConstruct( void );

		//	Prevent copying.

		CAINavigNode( const CAINavigNode & )				NOT_IMPLEMENTED;
		CAINavigNode &operator = ( const CAINavigNode & )	NOT_IMPLEMENTED;

		// Accessors et al.
		
		const CHashedString					GetName					( void ) const	{ return m_ksName; }
		const CEntity*						GetEntity				( void ) const	{ return m_pobEntity; }	// !!! - Needed?
		CEntity*							GetEntityToUse			( void ) const	{ return m_pobEntityToUse; }
		CPoint								GetPos					( void ) const	{ return m_obPos; }
		int									GetQueueIndex			( void ) const	{ return m_iQueueIndex; }
		int									GetQueueRow				( void ) const	{ return m_iQueueRow; }
		bool								IsQueueNode				( void ) const  { return m_bIsQueueNode; }
		AINAVIGNODE_TYPE					GetType					( void ) const	{ return m_eNodeType; }
		float								GetDistanceToPoint		( const CPoint&, ENUM_DISTANCE_TYPE) const;
		const AINavigArrowList*				GetTgtArrows			( void ) const	{ return &m_listTargetArrows; }
		const AINavigArrowList*				GetSourceArrows			( void ) const	{ return &m_listSourceArrows; }
		float								GetDistanceToSourceNode ( const CAINavigNode* ) const;
		float								GetRadiusSQR			( void ) const	{ return m_fNodeRadiusSQR; }
		float								GetRadius				( void ) const	{ return m_fNodeRadius; }
		GameEvent*							GetPatrolEvent			( void )		{ return m_pobGameEvent; }
		const CAINavigGraph*				GetParentNavigGraph		( void ) const	{ return m_pobNavigGraph; }
		float								GetNoWallDetectRadiusSQR( void ) const	{ return m_fNoWallDetectRadiusSQR; }
		bool								HasDisableWallDetFlag	( void ) const	{ return m_bDisableWallDetFlag; }
		CAINavigArrow*						GetTargetArrow			( const CAINavigNode* ) const;
		void								GetUseEntities			( ntstd::List<CEntity*>* ) const;
		float								GetWidthPercentage		( void ) const	{ return m_fNodeWithPercentage; }
		bool								HasWidthPercentage		( void ) const	{ return (m_fNodeWithPercentage < 0.0f || m_fNodeWithPercentage > 0.901f) ? false : true; }

		// Methods

		bool	HasName				( const CHashedString & obNodeKName ) { return (m_ksName == obNodeKName); }
		void	DetachSourceLinks	( void );	// Detach it from sources in order to delete this node

		// Cover Point related

		CAINavigCoverPoint*			GetAvailableCoverPoint	( CAIMovement* );
		void						GetCoverPoints			( AINavigCoverPointList* ); // !!! - Needed ?
		const AINavigArrowList*		GetCoverPointArrows		( void ) const	{ return &m_listCoverArrows; }
		bool						IsLinkedToCoverPoint	( CAINavigCoverPoint* );

		// Dynamically Allocated Node
		bool	HasBeenDynamicallyAllocated ( void ) const { return m_bDynaicallyAllocated; }

		void	SetGoAroundNodeParams ( const CPoint& pos, float fRadius, const CHashedString& name = CHashedString("No-Name"))
		{
			m_obPos = pos;
			m_fNodeRadius = fRadius;
			m_fNodeRadiusSQR = fRadius * fRadius;
			m_ksName = name;
			m_bDynaicallyAllocated = true;
		}

		// Whack-a-mole shooting
		const CHashedString GetTCBreakCoverAnimation	( bool *	);
		const CHashedString GetTCReturnToCoverAnimation	( bool *	);
		CDirection			GetWhackAMoleDir			( void		) const { return m_obWhackAMoleDir; }
		void				SetWhackAMoleDir			( const CDirection& dir )		{ m_obWhackAMoleDir = dir; }

	private:
				
		AINavigArrowList	m_listTargetArrows;	// List of arrows that point to nodes I can go to
		AINavigArrowList	m_listSourceArrows;	// List of arrows that point to nodes that come to me
		AINavigArrowList	m_listCoverArrows;	// List of arrows that lead to a cover point

		STCAnimSets*		m_pSTCAnimSets;		// List of Whack-a-Mole Actions

		CPoint						m_obPos;
		CHashedString				m_ksName;
		CEntity*					m_pobEntity;		// !!! - Needed?
		CEntity*					m_pobEntityToUse;
		AINAVIGNODE_TYPE			m_eNodeType;
		CAINavigGraph*				m_pobNavigGraph;	// Belongs to this navig graph
		GameEvent*					m_pobGameEvent;		// The game event that should be trigger when a patrolling AI reaches this node
		float						m_fNodeRadiusSQR;	// Radius (Threshold)
		float						m_fNodeRadius;		
		float						m_fNoWallDetectRadiusSQR;
		bool						m_bDisableWallDetFlag;
		//bool						m_bIsWhackAMoleNode; // Needed ?
		bool						m_bIsQueueNode; // Needed ?
		int							m_iQueueIndex;
		int							m_iQueueRow;
		bool						m_bDynaicallyAllocated;	// Used in GoAroundVolumes. It needs to be manually deallocated
		CDirection					m_obWhackAMoleDir;
		float						m_fNodeWithPercentage; // radius percentage used in follow path cover. if < 0 gets disregarded
	
};

#endif // _AINAVIGNODE_H



