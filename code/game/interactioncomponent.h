//--------------------------------------------------
//!
//!	\file interactioncomponent.h
//!	This is a dummy holding a few doxygen comments.
//!
//--------------------------------------------------

#ifndef _INTERACTIONCOMPONENT_H
#define _INTERACTIONCOMPONENT_H

#include "editable/enumlist.h"
#include "game/interaction_lua.h"
#include "lua/ninjalua.h"
#include "anim/transform.h"


// Forward declarations
class CEntity;
class CInteractionComponent;

#include "game/interactableparams.h"


//--------------------------------------------------
//!
//!	CUsePoint
//!	A use point is a point on the object that characters
//! interact with.
//!
//! NOTE: Positions and orientations are local to the object.
//!
//--------------------------------------------------
class CUsePoint
{
	HAS_INTERFACE(CUsePoint)

public:

	friend class CInteractionComponent;
	// Constructor and Destructor
	CUsePoint()
	{
		m_bCharacterControlToMP = false;

		m_pobIntComponentParent = 0;
	}

	CUsePoint(const CHashedString& Name, const CPoint& obLocalPosition, const CQuat& obLocalOrientation)
		:	m_obLocalOrientation ( obLocalOrientation )
		,	m_obLocalPosition ( obLocalPosition )
		,	m_obLocalOffsetMoveTo ( CVector(CONSTRUCT_CLEAR) )
		,	m_Name ( Name )
		,	m_UsePointPfxName ( CHashedString("") )
		,	m_fAngleOfUseMargin			 ( 0.0f )
		,	m_fCosAngleOFactor			 ( 0.0f )
		,	m_fBaseHalfWidth			 ( 0.5f )
		,	m_fZBackwardsOffset			 ( 0.4f )
		,	m_bHasFacingRequirements	 ( false )
		,	m_fDirectionHeldUseRadius	 ( 4.0f )
		,	m_fUseRadius				 ( 2.0f )
		,	m_fUseHeight				 ( 0.5f )
		,	m_bHeroCanUse				 ( true )
		,	m_bArcherCanUse				 ( true )
		,	m_bEnemyAICanUse			 ( true )
		,	m_bAllyAICanUse				 ( true )
		,	m_bCharacterControlToMP		 ( false )
		,	m_pobIntComponentParent		 ( 0 )
		{;};

	~CUsePoint() {};

	// Accessors
	const CHashedString	GetName() const			{ return m_Name; }
	const CPoint&	GetLocalPosition() const	{ return m_obLocalPosition; }

	const CHashedString& GetPfxName() const		{ return m_UsePointPfxName; }

	// Post Construct
	void OnPostConstruct();
	
	enum UseMovesState
	{
		UP_Walk,
		UP_Run,
		UP_TotalMove
    };

	enum InteractingCharacterType
	{
		ICT_Undefined	= 0,
		ICT_Archer		= (1<<0),
		ICT_Hero		= (1<<1),
		ICT_Enemy		= (1<<2),
		ICT_Ally		= (1<<3),
	};

	bool IsCharacterFriendly(InteractingCharacterType eCharType)	const ;

	// the anim state to go into after moving to the use point - character type dependent.
	CHashedString	GetUseAnim(int iCharacterType, UseMovesState eMoveState) const ;
	
	float GetUseAngle() const					{ return m_fAngleOfUseMargin; }
	const CVector	GetLocalUseFacing() const	{ return m_obLocalFacingNormal; }
	const CVector	GetMoveToOffset()	const 	{ return m_obLocalOffsetMoveTo; }
	float GetBaseHalfWidth() const				{ return m_fBaseHalfWidth; }
	float GetZBackOffset() const				{ return m_fZBackwardsOffset; }

    float DetermineUsabilityFactorFromApproach(const CPoint& 		obPosApproach, 
											   const CDirection& 	obPlaneNormalOfApproachingObject,
											   const CMatrix& 		obParentTransformAffineInverse,
											    const CMatrix& 		obParentTransform) const;

    float DetermineUsabilityFactorFromIsoscelesTrapezoid(const CPoint& 		obPosApproach, 
											   const CDirection& 	obPlaneNormalOfApproachingObject,
											   const CMatrix& 		obParentTransformAffineInverse,
											    const CMatrix& 		obParentTransform,
												float fMaxDistance) const;

	bool HasUseAngle() const {	return m_fAngleOfUseMargin>0.0f; }
	bool HasFacingRequirements() const	{	return m_bHasFacingRequirements;	}
	bool CharacterControlsToMoveToPoint() const { return m_bCharacterControlToMP; }

	float GetUseRadius (bool bDirectionHeld) const { return bDirectionHeld ? m_fDirectionHeldUseRadius : m_fUseRadius; }
	float GetUseHeight () const { return m_fUseHeight; }



	void GetPosAndFacingNormalWS(const CMatrix& 		obParentTransformAffineInverse,
								 const CMatrix& 		obParentTransform,
								 CPoint&				obPointWS,
								 CDirection&			obDirFacinfWS) const;

	void SetParentIntComponent( CInteractionComponent* pobIntComponent ) { m_pobIntComponentParent = pobIntComponent; }

protected:

	// should index into this according to character type
	// but unlikely to change across characters, so left for the moment.
	CHashedString m_obAnimWalk[1];
	CHashedString m_obAnimRun[1];

private:
	// Sets up what anims to use depending on attached object (and for every interacting character type) - called during postpost stage.
	void RegisterParentAs(CInteractionComponent* pobIntComponentParent);

	CQuat	m_obLocalOrientation;	// Local orientation of use-point 													   --- currently not used. tbdeprctd?
	CVector m_obLocalRotationAxis;	// local space defined axes around which any use-point facing normal would be rotated. --- currently not used. tbdeprctd?
	


	CPoint	m_obLocalPosition;				// Local position of use-point relative to root of object
	CVector	m_obLocalOffsetMoveTo; 			// Local space defined offset from use point pos, where character moves to. 
											//(Character could add to this her/its own additional offset depending on type)
	CVector m_obLocalFacingNormal;			// local space defined normal which is use point centre facing direction.
		
	CHashedString	m_Name;					// Name of the use point

	CHashedString	m_UsePointPfxName;		// Particle system to display at use point, not required.
	
	float  m_fAngleOfUseMargin;				// angle of use around centre facing  - if x rads, then x/2 each side of m_obLocalFacingNormal
	float  m_fCosAngleOFactor;				// cached, and useful
	float  m_fBaseHalfWidth;				// How wide the base of the trapezoid is from middle to side (used in box-check and cone-check position)
	float  m_fZBackwardsOffset;				// How far back (into the object) to push the use-check zones (so when you're close you can still use them).
	bool   m_bHasFacingRequirements;		// should not try to face the use normal if this is not set - character will merely nav to use point using the simple move transition.

	float m_fDirectionHeldUseRadius;		// Radius for this usepoint if the player has a direction held
	float m_fUseRadius;						// Radius for this usepoint
	float m_fUseHeight;						// Height tolerance for this use point
    
	bool 	m_bHeroCanUse;					// flags whether a hero character should consider it.
	bool 	m_bArcherCanUse;				// flags whether an archer character should consider it.
	bool 	m_bEnemyAICanUse;				// flags whether an enemy character should consider it.
	bool 	m_bAllyAICanUse;				// flags whether an ally character should consider it.
	
	// a transitional flag for the new system of the character controlling the move-to behaviour.
	bool 			m_bCharacterControlToMP;
		
	// the interaction component that the use point is associated with.
	CInteractionComponent* m_pobIntComponentParent;

};


//--------------------------------------------------
//!
//!	Use Point Particle Data
//!	Data for particle systems on the use points,
//! stored in the interaction component
//!
//--------------------------------------------------
class CUsePointPfxData
{
public:
	CUsePointPfxData()
	{
		m_uPfxID = 0;
		m_UsePointPfxName = "";
	};

	~CUsePointPfxData() {};

	CHashedString	m_UsePointPfxName;
	Transform		m_obTransform;
	unsigned int	m_uPfxID;
};


//--------------------------------------------------
//!
//!	Interaction Component
//!	Long Class Description
//!	Exciting class with lots of stuff to describe
//!
//--------------------------------------------------
class CInteractionComponent : public Interaction_Lua
{
public:

	// Typedef for convenience now that it's been chunked.
	typedef ntstd::Vector<CUsePoint*, Mem::MC_ENTITY>   UsePointArray;
	typedef ntstd::Vector<CUsePointPfxData*, Mem::MC_ENTITY>	UsePointPfxDataArray;
		

	// Construction / destruction
	CInteractionComponent( CEntity* pobParent );
	~CInteractionComponent( void );

	HAS_LUA_INTERFACE()

	void SetupNotifyOnInteractionWith(CEntity* pobEntity);
	void StopNotifyOnInteractionWith(CEntity* pobEntity);
	void ExcludeCollisionWith (CEntity* pobEntity); // Prevent two entities from colliding with each other
	void AllowCollisionWith (CEntity* pobEntity); // Re-enable collision between two entities
	bool CanCollideWith (CEntity* pobEntity); // Check to see if we can collide with a specific entity

	void SetInteractionType (INTERACTION_PRIORITY ePriority) { m_eInteractionPriority=ePriority; UpdateUsePointPfx(); }
	INTERACTION_PRIORITY GetInteractionPriority () const { return m_eInteractionPriority; }
	float GetInteractionScore(const CPoint& obCharacterPos, 
							  const CDirection& obCharacterDir, 
							  const CDirection& obCharacterPlaneNormal, 
							  bool bDirectionHeld,
							  CUsePoint::InteractingCharacterType eCharcInterType,
							  CUsePoint*& pobUPointBest) const;

	void SetEntityHeight (float fHeight) { m_fEntityHeight=fHeight; }
	float GetEntityHeight () const { return m_fEntityHeight; }
	
	// Use points
	int			GetNumberOfUsePoints( void ) { return m_obUsePointArray.size(); }
	CUsePoint*	GetUsePoint( unsigned int nIndex );
	CUsePoint*	GetUsePointByName(const CHashedString& Name);
	float		GetHeightFromUsePoints() const;
	void		DebugRenderUsePoints();
	CUsePoint*  GetClosestUsePoint(const CPoint& obCharacterPos);
	void		RemoveUsePointPfxData( void );
	
	CEntity* 	GetParentEntity( void ) const { return m_pobParentEntity; }
	void 		RegisterWithUsePoints(void);

	bool CInteractionComponent::IsCharacterUseable(CUsePoint::InteractingCharacterType eCharType)	const;

private:

	void SetupUsePointPfx(void);
	void UpdateUsePointPfx(void);

	// Attempts to load in an interaction XML file (.int.xml) for the use-points.
	void SetupUsePoints(void);
	bool AttemptToReadInteractionXMLFile(const ntstd::String& XMLFileName); // Returns true if it managed to read in 1 or more use-points.

	void AddToCollisionFilter (CEntity* pobEntity);
	void RemoveFromCollisionFilter (CEntity* pobEntity);
	void AddToNotifyOnInteractionWith (CEntity* pobEntity);
	void RemoveFromNotifyOnInteractionWith (CEntity* pobEntity);

	unsigned int			m_uiInteractionPriority; // 0 means no interaction, higher value means higher priority
	float					m_fEntityHeight;
	INTERACTION_PRIORITY	m_eInteractionPriority;
	CEntity*				m_pobParentEntity; // The entity that owns this component
	ntstd::List<CEntity*>	m_obEntityList; // List of entities that this entity is currently interacting with
	ntstd::List<CEntity*>	m_obCollisionFilterList; // List of entities that are excluded from colliding with this entity
	ntstd::List<CEntity*>	m_obNotifyOnInteractionWithList; // List of entities that we should tell our parent about if we hit them
	
	UsePointPfxDataArray	m_obUsePointPfxDataArray;	// Array of use point particle data
	UsePointArray			m_obUsePointArray; // List of use-points on the entity. Allocates in MC_ENITTY
	UsePointArray			m_obManuallyCreatedUsePointArray;	// Only used by destructor, and only these can be deleted

	friend class Interaction_Lua;
};

LV_DECLARE_USERDATA(CInteractionComponent);

#endif // _INTERACTIONCOMPONENT_H
