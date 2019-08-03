//------------------------------------------------------
//!
//!	\file army/armydef.h
//! all the army class that have interface definations
//! the interface between welder and mred and the game
//!
//------------------------------------------------------

#ifndef ARMY_DEF_H
#define ARMY_DEF_H

class ArmyBattalion;
class ArmyObstacle;
class ArmyBattalionCommand;
class SpawnPool;
class ArmyMessageHub;
class CEntity;

//! the main army defination class
//! having this in a level will fire up an army section
//! TODO sector bits etc.
class ArmyBattlefield
{
public:
	//! which army section is this for
	ntstd::String m_SectionName;
	//! the two points that make up the extents of the heightfield
	CPoint m_MinCorner;
	CPoint m_MaxCorner;

	//! the name of the heightfield (not currently used)
	CHashedString m_HeightfieldFilename;
	//! the width and height of the heightfield
	unsigned int m_HFWidth;
	unsigned int m_HFHeight;

	//! the name of the AI Volume to use for the army containment field
	//! we can't use a pointer as MrEd has a 'feature' that you can't link
	//! to sub entities
	CHashedString m_ContainmentFieldAIVol0;
	CHashedString m_ContainmentFieldAIVol1;
	CHashedString m_ContainmentFieldAIVol2;
	CHashedString m_ContainmentFieldAIVol3;

	typedef ntstd::List<ArmyBattalion*, Mem::MC_ARMY> ArmyBattalionDefList;
	typedef ntstd::List<ArmyObstacle*, Mem::MC_ARMY> ArmyObstacleDefList;

	ArmyBattalionDefList m_ArmyBattalionDefs;
	ArmyObstacleDefList m_ArmyObstacleDefs;

	// Event sounds
	CKeyString m_obEventSoundExplosion;
	CKeyString m_obEventSoundRocketFire;
	CKeyString m_obEventSoundJeer;


	// this unpublished and just used to quickly send messages back to the game
	// its set directly by the hub who will be receiving the messages
	ArmyMessageHub* m_pMessageHub;
};

class ArmyBattalionSoundDef
{
public:

	CKeyString m_obStateAdvanceSound;
	CKeyString m_obStateHoldSound;
	CKeyString m_obStateRetreatSound;
	CKeyString m_obStateFleeSound;
	CKeyString m_obStateAttackSound;
	CKeyString m_obStateChargeSound;
};

class ArmyBattalion
{
public:
	CHashedString m_Command;
	CPoint m_MinCorner;
	CPoint m_MaxCorner;
	CQuat m_Orientation;

	CHashedString m_SoundDef;

	typedef ntstd::List<ArmyBattalionCommand*, Mem::MC_ARMY> CommandList;
	CommandList		m_ArmyBattalionCommands;
	ArmyBattalion*	m_pBattalionToCopyCommandsFrom;
	CEntity*			m_pParentEntity; // TODO

	bool	m_bPlayerTracking;
	bool	m_bPlayerCircle;
	bool	m_bPlayerAttacking;
	bool	m_bHasFlag;
};


class ArmyObstacle
{
public:
	CPoint	m_Centre;
	float	m_Radius;
};

class ArmyUnitParameters
{
public:
	float m_fRunSpeed;
	float m_fWalkSpeed;
	float m_fDiveSpeed;

	int				m_iPersonalSpace;
	SpawnPool*		m_SpawnPool;
	int				m_iMaxPoolSize;
	float			m_fHalfHeight;

	// sprite defs
	CKeyString	m_IdleColourSprite;
	CKeyString	m_IdleNormalSprite;
	float		m_IdleCycleTime;
	CKeyString	m_WalkColourSprite;
	CKeyString	m_WalkNormalSprite;
	float		m_WalkCycleTime;
	CKeyString	m_RunColourSprite;
	CKeyString	m_RunNormalSprite;
	float		m_RunCycleTime;
	CKeyString	m_DeadColourSprite;
	CKeyString	m_DeadNormalSprite;

	// bomb def
	CKeyString m_BombName;

	// inner and outer player tracking
	float		m_OuterPlayerTrackRadius;
	float		m_InnerPlayerTrackRadius;

	// Circle-player behaviour radius.
	float		m_CirclePlayerRadius;
};

class ArmyGenericParameters
{
public:
	float	m_fPlayerInRadius;
	float	m_fBazookaInnerThreatRadius;
	float	m_fBazookaThreatRadius;
	float	m_fBazookaExplodeRadius;
	float	m_fCamaraFarRadius;
	int		m_iMaxPeeps;
	int		m_iMaxAIDudes;
	int		m_iVisibleFrameLimit;

	int		m_iPlayerTrackWallInset;
	int		m_iPlayerTrackWallMultipler;
	int		m_iPlayerTrackWallRandomishFactor;

	int		m_iStagePostBodyCountWithBazooka;
	int		m_iStagePostBodyCountWithSwords;

	float	m_fMaxRandomDurationVariationAnim;

	CKeyString m_FlagTemplateName;
	CKeyString m_FlagPoleClumpName;
	float		m_fFlagPoleVerticalOffset;
	float		m_fFlagPoleHeight;
};


class ArmyBattalionCommand
{
public:
	CPoint	m_vPos;				//!< the spatial point this command is connected to
	int		m_iSequence;		//!< the id for this command
	int		m_iCommand;			//!< note should probably be a proper welder enum but this data also goes on SPU so for now its BATTALION_LOGIC in pog form
	int		m_iParam0;			//!< 16 bit int params specific to each command -1 = 0xFFFF
	int		m_iParam1;			//!< 10 bit int param (< 1024!!!)
};


//------------------------------------------------------
//!
//! Simple heightfield file format
//!
//------------------------------------------------------
struct HeightfieldFileHeader
{
	uint32_t m_Width;
	uint32_t m_Height;

	uint32_t m_padd[2];
}; // followed m_Width * m_Height * sizeof(float)

#endif
