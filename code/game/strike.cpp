/***************************************************************************************************
*
*	DESCRIPTION		The strike class. Used to pass blow information between players.
*
*	NOTES
*
***************************************************************************************************/

// Necessary includes
#include "game/strike.h"
#include "game/entity.h"
#include "game/entity.inl"
#include "game/attacks.h"

/***************************************************************************************************
*
*	FUNCTION		CStrike::CStrike
*
*	DESCRIPTION		Construction
*
***************************************************************************************************/
CStrike::CStrike(	const CEntity*		pobOriginator,		
					const CEntity*		pobTarget,			
					const CAttackData*	pobAttackData,		
					float				fAttackTimeScalar,	
					float				fAttackRange,		
					bool				bPreStrike,
					bool				bCounter,
					bool				bIncidental,
					bool				bWithinExclusionDistance,
					bool				bSyncronise,
					bool				bSpecificAttackVulnerabilityZone,
					CEntity*			pobProjectile,
					const CPoint&		obInitialPosition,
					int					iHitArea)


:	m_pobOriginator( pobOriginator ),
	m_pobTarget( pobTarget ),	
	m_pobAttackData( pobAttackData ),
	m_fAttackTimeScalar( fAttackTimeScalar ),
	m_fAttackRange( fAttackRange ),
	m_bPreStrike( bPreStrike ),
	m_bCounter( bCounter ),
	m_bSyncronise( bSyncronise ),
	m_obInitialPosition( obInitialPosition ),
	m_bIncidentalStrike( bIncidental ),
	m_bWithinExclusionDistance( bWithinExclusionDistance ),
	m_bSpecificAttackVulnerabilityZone( bSpecificAttackVulnerabilityZone ),
	m_pobProjectile( pobProjectile ),
	m_iHitArea(iHitArea)
{
}


/***************************************************************************************************
*
*	FUNCTION		CStrike::CStrike
*
*	DESCRIPTION		Construction
*
***************************************************************************************************/

CStrike::CStrike( const CStrike& obOther )
:	m_pobOriginator( obOther.m_pobOriginator ),
	m_pobTarget( obOther.m_pobTarget ),	
	m_pobAttackData( obOther.m_pobAttackData ),
	m_fAttackTimeScalar( obOther.m_fAttackTimeScalar ),
	m_fAttackRange( obOther.m_fAttackRange ),
	m_bPreStrike( obOther.m_bPreStrike ),
	m_bCounter( obOther.m_bCounter ),
	m_bSyncronise( obOther.m_bSyncronise ),
	m_obInitialPosition( obOther.m_obInitialPosition ),
	m_iHitArea(obOther.m_iHitArea)
{
}


/***************************************************************************************************
*
*	FUNCTION		CStrike::CStrike
*
*	DESCRIPTION		Construction
*
***************************************************************************************************/

CStrike& CStrike::operator=( const CStrike& obOther )
{
	if ( this != &obOther )
	{
		m_pobOriginator = obOther.m_pobOriginator;
		m_pobTarget = obOther.m_pobTarget;
		m_pobAttackData = obOther.m_pobAttackData;
		m_fAttackTimeScalar = obOther.m_fAttackTimeScalar;
		m_fAttackRange = obOther.m_fAttackRange;
		m_bPreStrike = obOther.m_bPreStrike;
		m_bCounter = obOther.m_bCounter;
		m_bSyncronise = obOther.m_bSyncronise;
		m_obInitialPosition = obOther.m_obInitialPosition;
		m_pobProjectile = obOther.m_pobProjectile;
		m_iHitArea = obOther.m_iHitArea;
	}

	// Return a reference to the new us
	return *this;
}


/***************************************************************************************************
*
*	FUNCTION		CStrike::~CStrike
*
*	DESCRIPTION		Destruction
*
***************************************************************************************************/

CStrike::~CStrike()
{
}


/***************************************************************************************************
*
*	FUNCTION		CStrike::GetAttackTime
*
*	DESCRIPTION		how long am i?
*
***************************************************************************************************/
float	CStrike::GetAttackTime() const
{
	return m_pobAttackData->GetAttackTime( m_fAttackTimeScalar );
}

/***************************************************************************************************
*
*	FUNCTION		CStrike::GetAttackRecoverTime
*
*	DESCRIPTION		how long does it take me to recover?
*
***************************************************************************************************/
float	CStrike::GetAttackRecoverTime() const
{
	return m_pobAttackData->GetAttackRecoverTime( m_fAttackTimeScalar );
}




