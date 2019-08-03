/***************************************************************************************************
*
*	DESCRIPTION		Provides a simple interface for combat related movement requests
*
*	NOTES
*
***************************************************************************************************/

#ifndef _ATTACKMOVEMENT_H
#define _ATTACKMOVEMENT_H

#include "editable/enumlist.h"

// Forward declarations
class CStrike;
class CEntity;
class CAttackData;
class CAttackKO;
class CAttackBlock;
class CHashedString;
class AerialDetailsDef;

/***************************************************************************************************
*
*	CLASS			CAttackMovement
*
*	DESCRIPTION		Some of the methods here require default movement descriptions as arguments.
*					This is a temporary thing whilst the game data is being built and developed.
*					These arguments should eventually be removed from the methods all together.
*
***************************************************************************************************/

class CAttackMovementBlendInTimes
{
	HAS_INTERFACE( CAttackMovementBlendValues );

public:
	CAttackMovementBlendInTimes() :
		m_fAttackBlendInTime( 0.15f ),
		m_fAttackRecoverBlendInTime( 0.15f ),
		m_fBlockBlendInTime( 0.15f ),
		m_fEndBlockBlendInTime( 0.15f ),
		m_fStanceSwitchBlendInTime( 0.15f ),
		m_fDeflectBlendInTime( 0.0f ),
		m_fRecoilBlendInTime( 0.0f ),
		m_fStaggerBlendInTime( 0.0f ),
		m_fStaggerRecoverBlendInTime( 0.15f ),
		m_fKOBlendInTime( 0.0f ),
		m_fKOFallBlendInTime( 0.15f ),
		m_fKOImpactBlendInTime( 0.075f ),
		m_fFlooredBlendInTime( 0.6f ),
		m_fRiseBlendInTime( 0.15f ),
		m_fKillBlendInTime( 0.0f )
	{};

	float m_fAttackBlendInTime,
		m_fAttackRecoverBlendInTime,
		m_fBlockBlendInTime,
		m_fEndBlockBlendInTime,
		m_fStanceSwitchBlendInTime,
		m_fDeflectBlendInTime,
		m_fRecoilBlendInTime,
		m_fStaggerBlendInTime,
		m_fStaggerRecoverBlendInTime,
		m_fKOBlendInTime,
		m_fKOFallBlendInTime,
		m_fKOImpactBlendInTime,
		m_fFlooredBlendInTime,
		m_fRiseBlendInTime,
		m_fKillBlendInTime;
};

class CAttackMovement
{
public:

	// Construction destruction
	CAttackMovement( CEntity* pobParent, CAttackMovementBlendInTimes* pobBlendInTimes );
	~CAttackMovement( void );

	// Set a message on the movement system to be broadcast on movement completion
	void SetMovementMessage( const char* pcMessage, bool bSetInterruptToo = false );
	void SetMovementCallback( void ( *CompletionCallback )( CEntity* ), bool bSetInterruptToo = false );

	// Attacking moves
	void StartAttackMovement( const CStrike& obStrike, const CDirection& obEvadeDirection, bool bNoBlend = false );
	void StartAttackRecoverMovement( const CStrike& obStrike, const AerialDetailsDef* pobAerialDetails, bool bStrikeLanded );
	
	// Basic blocking actions
	void StartBlockMovement( const CAttackBlock* pobBlock, float fTimeTillStrike );
	void StartBlockEndMovement( const CAttackBlock* pobBlock );

	// Stance switching
	void StartSwitchMovement( const CHashedString& obAnimName );

	// Movement of deflections
	void StartDeflectingMovement( const CHashedString& obAnimName, float fTimeScalar );

	// Movement of reactions
	void StartRecoilMovement( const CHashedString& obAnimName, REACTION_ZONE eReactionZone, float fTimeScalar, bool bYawRotation = true );

	// Movement related to Staggers
	void StartStaggerMovement( const CHashedString& obAnimName, float fTimeScalar );
	void StartStaggerRecoverMovement( const CHashedString& obAnimName );

	// Movement related to KOs
	void StartKOMovement( const CHashedString& obAnimName, const CAttackKO* pobKO, REACTION_ZONE eReactionZone, float fTimeScalar,	bool bSyncInitialHeight, bool bUseSpecificAngleToTarget, float fSpecificAngleToTarget, bool bNeedFallAndFlooredImpact );
	void StartKOAftertouchMovement( const CHashedString& obAnimName, const CAttackKO* pobKO, REACTION_ZONE eReactionZone, float fTimeScalar, bool bUseSpecificAngleToTarget, float fSpecificAngleToTarget, const CEntity* pobAttacker, bool bNeedFallAndFlooredImpact );
	void StartFlooredMovement( const CHashedString& obFlooredAnimName, const CHashedString& obWaitAnimName, bool bWasFullyRagdolledInKO, bool bNeedFallAndFlooredImpact, bool bChainIt );
	void StartRiseMovement( const CHashedString& obAnimName );

	// For dealing with KOs that have started with relative movement - adds a CHAINED controller
	void StartKOFallMovement( const CAttackKO* pobKO, bool bInterruptedSync );
	void StartPausedKOFallMovement( const CAttackKO* pobKO );

	// Movement related to getting killed
	void StartKillMovement( const CHashedString& obAnimName, REACTION_ZONE eReactionZone, float fTimeScalar, bool bChainOnFall = false, CAttackKO* pobKO = 0 );

	void StartImpactMovement( const CHashedString& obAnimName, bool bChainIt );

	const CAttackMovementBlendInTimes* GetAttackMovementBlendInTimes() { return m_pobBlendInTimes; };
private:

	CAttackMovementBlendInTimes* m_pobBlendInTimes;
	bool m_bOwnsBlendInTimes;

	// Members
	CEntity* m_pobParentEntity;
};

#endif // _ATTACKMOVEMENT_H
