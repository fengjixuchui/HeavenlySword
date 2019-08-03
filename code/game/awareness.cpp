//------------------------------------------------------------------------------------------
//!
//!	\file awareness.cpp
//!
//------------------------------------------------------------------------------------------

// Necessary includes
#include "aicomponent.h"
#include "movementstate.h"

#include "game/awareness.h"
#include "game/entityinfo.h"
#include "game/query.h"
#include "game/entitymanager.h"
#include "game/movement.h"
#include "game/movementcontrollerinterface.h"
#include "game/inputcomponent.h"
#include "game/messagehandler.h"
#include "interactioncomponent.h"

#include "objectdatabase/dataobject.h"

#include "editable/enumlist.h"

// For debug stuff
#include "core/visualdebugger.h"
#include "core/gatso.h"

#include "camera/camutils.h"

START_STD_INTERFACE	( CAttackTargetingData )
	IHELP		( "Core data for the attack targeting" )
	IFLOAT		( CAttackTargetingData, AutoLockOnAngle )
	IFLOAT		( CAttackTargetingData, AutoLockOnDistance )
	IFLOAT		( CAttackTargetingData, AwareAngle )
	IFLOAT		( CAttackTargetingData, AwareDistance )
	IFLOAT		( CAttackTargetingData, ToLockOnAngle )
	IFLOAT		( CAttackTargetingData, ToLockOnDistance )
	IFLOAT		( CAttackTargetingData, MediumLockOnAngle )
	IFLOAT		( CAttackTargetingData, MediumLockOnDistance )
	IFLOAT		( CAttackTargetingData, LongLockOnAngle )
	IFLOAT		( CAttackTargetingData, LongLockOnDistance )
	IFLOAT		( CAttackTargetingData, InnerLockonAngle )
	IFLOAT		( CAttackTargetingData, InnerLockonDistance )
	IFLOAT		( CAttackTargetingData, LaunchLockOnAngle )
	IFLOAT		( CAttackTargetingData, LaunchLockOnDistance )
	IFLOAT		( CAttackTargetingData, LaunchInnerLockOnDistance )
	IFLOAT		( CAttackTargetingData, ThrowLockOnAngle )
	IFLOAT		( CAttackTargetingData, ThrowLockOnDistance )
	IFLOAT		( CAttackTargetingData, PickUpLockOnAngle )
	IFLOAT		( CAttackTargetingData, PickUpLockOnDistance )
	IFLOAT		( CAttackTargetingData, RunningPickupLockOnAngle )
	IFLOAT		( CAttackTargetingData, RunningPickupLockOnDistance )
	IFLOAT		( CAttackTargetingData, CatchLockOnAngle )
	IFLOAT		( CAttackTargetingData, CatchLockOnDistance )
	IFLOAT		( CAttackTargetingData, EvadeLockOnDistance )
	IFLOAT		( CAttackTargetingData, EvadeLockOnAngle )
	IFLOAT		( CAttackTargetingData, AerialComboDistance )
	IFLOAT		( CAttackTargetingData, AerialComboAngle )
	IFLOAT		( CAttackTargetingData, AerialComboHeight )
	IFLOAT		( CAttackTargetingData, AerialComboHeightBottom )
	IFLOAT		( CAttackTargetingData, SpeedExtraDistance )
	IFLOAT		( CAttackTargetingData, SpeedExtraAngle )
	IFLOAT		( CAttackTargetingData, DirectionLockonMagnitude )
	IFLOAT		( CAttackTargetingData, MinimumBreakAwayAngle )
	IFLOAT		( CAttackTargetingData, BreakAwayMagnitude )
	IFLOAT		( CAttackTargetingData, AngleWeighting )
	IFLOAT		( CAttackTargetingData, DistanceWeighting )
	IFLOAT		( CAttackTargetingData, MaximumLockonSpeed )
	IFLOAT		( CAttackTargetingData, UpperHeight )
	IFLOAT		( CAttackTargetingData, LowerHeight )
	IFLOAT		( CAttackTargetingData, UpperJuggleHeight )
	IFLOAT		( CAttackTargetingData, LowerJuggleHeight )
	IFLOAT		( CAttackTargetingData, GroundAttackInnerDistance )
	IFLOAT		( CAttackTargetingData, GroundAttackOuterDistance )
	IFLOAT		( CAttackTargetingData, GroundAttackAngle )
	IFLOAT		( CAttackTargetingData, GroundAttackHeightTop )
	IFLOAT		( CAttackTargetingData, GroundAttackHeightBottom )
	PUBLISH_VAR_WITH_DEFAULT_AS( m_fNextNearestInnerDistance, 0.0f, NextNearestInnerDistance )
	PUBLISH_VAR_WITH_DEFAULT_AS( m_fNextNearestOuterDistance, 10.0f, NextNearestOuterDistance )
	PUBLISH_VAR_WITH_DEFAULT_AS( m_fNextNearestAngle, 360.0f, NextNearestAngle )
END_STD_INTERFACE

// Some debug rendering for targeting
// #define _DEBUG_TARGET
#ifdef _DEBUG_TARGET
	#include "gfx/renderer.h"
	CPoint obDebugTargetStart(CONSTRUCT_CLEAR);
	CPoint obDebugTargetEnd(CONSTRUCT_CLEAR);
#endif

//------------------------------------------------------------------------------------------
//!
//!	CAttackTargetingData::CAttackTargetingData
//!	Construction
//!
//------------------------------------------------------------------------------------------
CAttackTargetingData::CAttackTargetingData( void )
:	m_fAutoLockOnAngle( 45.0f ),
	m_fAutoLockOnDistance( 1.0f ),
	m_fAwareAngle( 270.0f ),
	m_fAwareDistance( 15.0f ),
	m_fToLockOnAngle( 90.0f ),
	m_fToLockOnDistance( 4.5f ),
	m_fMediumLockOnAngle( 80.0f ),
	m_fMediumLockOnDistance( 7.0f ),
	m_fLongLockOnAngle( 70.0f ),
	m_fLongLockOnDistance( 10.0f ),
	m_fInnerLockonAngle( 120.0f ),
	m_fInnerLockonDistance( 2.0f ),
	m_fLaunchLockOnAngle( 90.0f ),
	m_fLaunchLockOnDistance( 9.0f ),
	m_fLaunchInnerLockOnDistance( 4.0f ),
	m_fThrowLockOnAngle( 90.0f ),
	m_fThrowLockOnDistance( 20.0f ),
	m_fPickUpLockOnAngle( 90.0f ),
	m_fPickUpLockOnDistance( 1.0f ),
	m_fEvadeLockOnDistance( 1.0f ),
	m_fEvadeLockOnAngle( 60.0f ),
	m_fAerialComboDistance( 10.0f ),
	m_fAerialComboAngle( 180.0f ),
	m_fAerialComboHeight( 5.0f ),
	m_fAerialComboHeightBottom( 1.0f ),
	m_fSpeedExtraDistance( 7.0f ),
	m_fSpeedExtraAngle( 60.0f ),
	m_fDirectionLockonMagnitude( 0.5f ),
	m_fMinimumBreakAwayAngle( 160.0f ),
	m_fBreakAwayMagnitude( 0.8f ),
	m_fAngleWeighting( 1.0f ),
	m_fDistanceWeighting( 1.0f ),
	m_fMaximumLockonSpeed( 2.0f ),
	m_fUpperHeight( 2.5f ),
	m_fLowerHeight( -0.5f ),
	m_fUpperJuggleHeight( 2.5f ),
	m_fLowerJuggleHeight( -0.5f ),
	m_fGroundAttackInnerDistance( 0.0f ),
	m_fGroundAttackOuterDistance( 1.0f ),
	m_fGroundAttackAngle( 360.0f ),
	m_fGroundAttackHeightTop( 0.5f ),
	m_fGroundAttackHeightBottom( -0.5f )
{
}


//------------------------------------------------------------------------------------------
//!
//!	CAttackTargetingData::~CAttackTargetingData
//!	Destruction
//!
//------------------------------------------------------------------------------------------
CAttackTargetingData::~CAttackTargetingData( void )
{
}


//------------------------------------------------------------------------------------------
//!
//!	AwarenessComponent::AwarenessComponent
//!	Construction
//!
//------------------------------------------------------------------------------------------
AwarenessComponent::AwarenessComponent( CEntity* pobParent, const CAttackTargetingData* pobTargetData )
:	m_pobAutoLockonEntity( 0 ),
	m_pobAttackTargetingData( pobTargetData ),
	m_pobParent( pobParent ),
	m_fMovementMagnitude( 0.0f ),
	m_obMovementDirection( CONSTRUCT_CLEAR ),
	m_pobDirectTarget( 0 ),
	m_pobAwarenessEntity( 0 ),
	m_bDebugRender( false )
{
	// Some quick sanity checks on the data
	ntAssert( pobTargetData->m_fAwareAngle >= pobTargetData->m_fAutoLockOnAngle );
	ntAssert( pobTargetData->m_fAwareDistance >= pobTargetData->m_fAutoLockOnDistance );

	ATTACH_LUA_INTERFACE(AwarenessComponent);
}


//------------------------------------------------------------------------------------------
//!
//!	AwarenessComponent::AwarenessComponent
//!	Destruction
//!
//------------------------------------------------------------------------------------------
AwarenessComponent::~AwarenessComponent( void )
{
}

//------------------------------------------------------------------------------------------
//!
//!	AwarenessComponent::GetDetailedTarget
//!	To be expanded so that external systems have more control over the data that they
//!	are using for targeting - at the moment this is not the best bit of code - GH
//!
//!	- it's mainly a copy and paste of GetLockon
//!
//------------------------------------------------------------------------------------------
CEntity* AwarenessComponent::GetDetailedTarget(	ATTACK_TARGET_TYPE	eTargetType,
												const CEntity*		pobEntity,
												float				fOuterRadius,
												float				fInnerRadius, 
												float				fTargetAngle ) const
{
	// To remove - don't want to directly access input component - need to use for AI too?
	CDirection obDirection( m_obMovementDirection );

	// Parameters required by the search
	float fInnerDistance = fInnerRadius;
	float fOuterDistance = fOuterRadius;
	float fAngle = fTargetAngle;

	// If we are not locked on to anything and we don't have a strong directional input
	// then look to see if we have anything in the cone ahead of us
	if ( m_fMovementMagnitude < m_pobAttackTargetingData->m_fDirectionLockonMagnitude )
	{
		// Find a possible lockon using this characters current orientation
		CEntity* pobTarget = FindTarget(	eTargetType,
											pobEntity->GetMatrix(), 
											pobEntity,
											fInnerDistance,
											fOuterDistance,
											fAngle, 
											0 );

		// Return our new lockon
		return pobTarget;
	}

	// We can assume that if we are here that we have a strong enough magnitude to change 
	// lockon but i'll still put the condition here for clarity
	if ( m_fMovementMagnitude >= m_pobAttackTargetingData->m_fDirectionLockonMagnitude )
	{
		// Take our input direction and remove any Y badness
		CDirection obSearchDirection( obDirection.X(), 0.0f, obDirection.Z() );
		obSearchDirection.Normalise();

		// Build a matrix that represents the character were it facing in the pad input direction
		CDirection obXDir = obSearchDirection.Cross( CVecMath::GetYAxis() );
		obXDir.Normalise();

		CMatrix obPossibleMatrix(	obXDir.X(),							0.0f,								obXDir.Z(),							0.0f,
									0.0f,								1.0f,								0.0f,								0.0f,
									obSearchDirection.X(),				0.0f,								obSearchDirection.Z(),				0.0f,
									pobEntity->GetPosition().X(),		pobEntity->GetPosition().Y(),		pobEntity->GetPosition().Z(),		1.0f );

		// If we are debuging we can render what this matrix represents
#ifdef _DEBUG_TARGET
		obDebugTargetStart = pobEntity->GetPosition();
		obDebugTargetEnd = pobEntity->GetPosition() + ( obSearchDirection * 2.0f );
#endif

		// See if we can find a lockon using this matrix
		CEntity* pobTarget = FindTarget(	eTargetType,
											obPossibleMatrix, 
											pobEntity,
											fInnerDistance,
											fOuterDistance,
											fAngle,
											0 );
		// Return our target
		return pobTarget;
	}

	// If we are here something is wrong
	ntAssert( 0 );
	return 0;
}


//------------------------------------------------------------------------------------------
//!
//!	AwarenessComponent::GetTarget
//!	General Lockon Finding
//!
//------------------------------------------------------------------------------------------
CEntity* AwarenessComponent::GetTarget(	ATTACK_TARGET_TYPE	eTargetType,
										const CEntity*		pobEntity,
										bool				bUseFacingDirection,
										bool				bUseSpecificDirection,
										const CDirection&	obSpecificDirection,
										const CEntity*			pobExemptThisEntity ) const
{
	// To remove - don't want to directly access input component - need to use for AI too?
	CDirection obDirection( m_obMovementDirection );

	// If we are to use the facing direction - grab the direction from the character's matrix
	if ( bUseFacingDirection )
		obDirection = m_pobParent->GetMatrix().GetZAxis(); 

	// If we need to use a specific direction - set that instead
	if ( bUseSpecificDirection )
		obDirection = obSpecificDirection;

	// Special case for targets we can only get with a direction - the return should be NULL
	if ( TargetRequiresPadDirection( eTargetType, pobEntity ) && ( m_fMovementMagnitude < m_pobAttackTargetingData->m_fDirectionLockonMagnitude ) )
	{
		// If this is an attack target then...
		if ( eTargetType == AT_TYPE_ATTACK )
		{	
			// Try again with the inner lockon settings - recurse
			// Stert off with a local search, if that fails, to a short to lock on
			CEntity* pobEnt = GetTarget( AT_TYPE_INNER, pobEntity );
			if (pobEnt)
				return pobEnt;
		}

		// If this is for the speed extra attack then...
		if ( eTargetType == AT_TYPE_SPEED_EXTRA )
		{
			// Drop out - we don't have an inner
			return 0;
		}

		// Our work is done
		return 0; 
	}

	// Parameters required by the search
	float fInnerDistance = 0.0f;
	float fOuterDistance = 0.0f;
	float fAngle = 0.0f;

	// Set up the parameters based on input type
	switch( eTargetType )
	{
	case AT_TYPE_ATTACK:
		fInnerDistance = 0.0f;
		fOuterDistance = m_pobAttackTargetingData->m_fToLockOnDistance;
		fAngle = m_pobAttackTargetingData->m_fToLockOnAngle;
		break;

	case AT_TYPE_DOWN:
		fInnerDistance = m_pobAttackTargetingData->m_fGroundAttackInnerDistance;
		fOuterDistance = m_pobAttackTargetingData->m_fGroundAttackOuterDistance;
		fAngle = m_pobAttackTargetingData->m_fGroundAttackAngle;
		break;

	case AT_TYPE_MEDIUM_ATTACK:
		fInnerDistance = 0.0f;
		fOuterDistance = m_pobAttackTargetingData->m_fMediumLockOnDistance;
		fAngle = m_pobAttackTargetingData->m_fMediumLockOnAngle;
		break;

	case AT_TYPE_LONG_ATTACK:
		fInnerDistance = 0.0f;
		fOuterDistance = m_pobAttackTargetingData->m_fLongLockOnDistance;
		fAngle = m_pobAttackTargetingData->m_fLongLockOnAngle;
		break;

	case AT_TYPE_AERIAL_COMBO:
		fInnerDistance = 0.0f;
		fOuterDistance = m_pobAttackTargetingData->m_fAerialComboDistance;
		fAngle = m_pobAttackTargetingData->m_fAerialComboAngle;
		break;

	case AT_TYPE_LAUNCH:	
		fInnerDistance = m_pobAttackTargetingData->m_fLaunchInnerLockOnDistance;
		fOuterDistance = m_pobAttackTargetingData->m_fLaunchLockOnDistance;
		fAngle = m_pobAttackTargetingData->m_fLaunchLockOnAngle;
		break;

	case AT_TYPE_THROW:		
		fInnerDistance = 0.0f;
		fOuterDistance = m_pobAttackTargetingData->m_fThrowLockOnDistance;
		fAngle = m_pobAttackTargetingData->m_fThrowLockOnAngle;
		break;

	case AT_TYPE_AUTO:
		fInnerDistance = 0.0f;
		fOuterDistance = m_pobAttackTargetingData->m_fAutoLockOnDistance;
		fAngle = m_pobAttackTargetingData->m_fAutoLockOnAngle;
		break;

	case AT_TYPE_EVADE:
		fInnerDistance = 0.0f;
		fOuterDistance = m_pobAttackTargetingData->m_fEvadeLockOnDistance;
		fAngle = m_pobAttackTargetingData->m_fEvadeLockOnAngle;
		break;

	case AT_TYPE_INNER:
		fInnerDistance = 0.0f;
		fOuterDistance = m_pobAttackTargetingData->m_fInnerLockonDistance;
		fAngle = m_pobAttackTargetingData->m_fInnerLockonAngle;
		break;

	case AT_TYPE_DOWN_INNER:
		fInnerDistance = 0.0f;
		fOuterDistance = m_pobAttackTargetingData->m_fGroundAttackInnerDistance;
		fAngle = m_pobAttackTargetingData->m_fGroundAttackAngle;
		break;

	case AT_TYPE_SPEED_EXTRA:
		fInnerDistance = 0.0f;
		fOuterDistance = m_pobAttackTargetingData->m_fSpeedExtraDistance;
		fAngle = m_pobAttackTargetingData->m_fSpeedExtraAngle;
		break;

	case AT_TYPE_NEXT_NEAREST:
		fInnerDistance = m_pobAttackTargetingData->m_fNextNearestInnerDistance;
		fOuterDistance = m_pobAttackTargetingData->m_fNextNearestOuterDistance;
		fAngle = m_pobAttackTargetingData->m_fNextNearestAngle;
		break;

	case AT_TYPE_PLAYER:
		return CEntityManager::Get().GetPlayer();

	default:					
		ntAssert( 0 );
		break;
	}

	// If we are not locked on to anything and we don't have a strong directional input
	// then look to see if we have anything in the cone ahead of us
	if ( ( m_fMovementMagnitude < m_pobAttackTargetingData->m_fDirectionLockonMagnitude )
		 &&
		 ( !bUseSpecificDirection )
		 &&
		 eTargetType != AT_TYPE_NEXT_NEAREST )
	{
		// Find a possible lockon using this characters current orientation
		CEntity* pobTarget = FindTarget(	eTargetType,
											pobEntity->GetMatrix(), 
											pobEntity,
											fInnerDistance,
											fOuterDistance,
											fAngle,
											pobExemptThisEntity );

		// Return our new lockon
		return pobTarget;
	}

	// If we are here then we can use the data set up at the beginning of this function
	else
	{
		// Take our input direction and remove any Y badness
		CDirection obSearchDirection( obDirection.X(), 0.0f, obDirection.Z() );
		obSearchDirection.Normalise();

		// Build a matrix that represents the character were it facing in the pad input direction
		CDirection obXDir = obSearchDirection.Cross( CVecMath::GetYAxis() );
		obXDir.Normalise();

		CMatrix obPossibleMatrix(	obXDir.X(),							0.0f,								obXDir.Z(),							0.0f,
									0.0f,								1.0f,								0.0f,								0.0f,
									obSearchDirection.X(),				0.0f,								obSearchDirection.Z(),				0.0f,
									pobEntity->GetPosition().X(),		pobEntity->GetPosition().Y(),		pobEntity->GetPosition().Z(),		1.0f );

		// If we are debuging we can render what this matrix represents
#ifdef _DEBUG_TARGET
		obDebugTargetStart = pobEntity->GetPosition();
		obDebugTargetEnd = pobEntity->GetPosition() + ( obSearchDirection * 2.0f );
#endif

		// See if we can find a lockon using this matrix
		CEntity* pobTarget = FindTarget(	eTargetType,
											obPossibleMatrix, 
											pobEntity,
											fInnerDistance,
											fOuterDistance,
											fAngle,
											pobExemptThisEntity );
		// Return our target
		return pobTarget;
	}

	// If we are here something is wrong
	ntAssert( 0 );
	return 0;
}

//------------------------------------------------------------------------------------------
//!
//!	AwarenessComponent::TargetRequiresPadDirection
//!	We need to remove this sort of knowledge - i think external data should be passed in
//! here.
//!
//------------------------------------------------------------------------------------------
bool AwarenessComponent::TargetRequiresPadDirection( ATTACK_TARGET_TYPE eTargetType, const CEntity* pobEntity ) const
{
	// If this is AI we never need a direction
	if ( pobEntity->IsAI() )
		return false;

	// Some targets can only be aquired with a pad direction
	if ( ( eTargetType == AT_TYPE_LAUNCH )
		 ||
		 ( eTargetType == AT_TYPE_ATTACK )
		 ||
		 ( eTargetType == AT_TYPE_MEDIUM_ATTACK )
		 ||
		 ( eTargetType == AT_TYPE_LONG_ATTACK )
		 ||
		 ( eTargetType == AT_TYPE_SPEED_EXTRA ) ) 
	{
		return true;
	}

	// Others don't need that at all
	return false;
}

//------------------------------------------------------------------------------------------
//!
//!	AwarenessComponent::GetVulnerabilityZoneTarget
//!	
//!	Check everything in the given zone to see if it matches the attacker entity
//------------------------------------------------------------------------------------------
bool AwarenessComponent::IsEntityInZone(const CEntity*	pobZoneOwner, 
										const CEntity*	pobZoneEnterer,
										float fZoneAngle,
										float fZoneSweep,
										float fInnerDistance,
										float fOuterDistance ) const
{
	CPoint obOwnerPosition = pobZoneOwner->GetPosition();
	CPoint obEntererPosition = pobZoneEnterer->GetPosition();

	CDirection obOwnerToEnterer = CDirection( obEntererPosition - obOwnerPosition );
	float fDistance = obOwnerToEnterer.Length();

	// Check distance
	if (fDistance < fInnerDistance || fDistance > fOuterDistance)
		return false;

	// Check cone
	CDirection obOwnerZ = pobZoneOwner->GetMatrix().GetZAxis();
	obOwnerToEnterer.Normalise();

	CMatrix obRotY( CONSTRUCT_IDENTITY );
	CCamUtil::MatrixFromEuler_XYZ(obRotY, 0.0f, fZoneAngle * DEG_TO_RAD_VALUE, 0.0f );
	obOwnerZ = obOwnerZ * obRotY;

	//We perform a cone-check as well as a box-check for those close-to-originating-point zone-checks.
	//If the entity is inside of either of the checks then this will return true.
	bool bInZone = false;	//False by default.

	//Cone check.
	bInZone |= !(acos(obOwnerToEnterer.Dot(obOwnerZ)) * RAD_TO_DEG_VALUE > fZoneSweep*0.5f);

	//Box check (only performed if the cone-check didn't already validate their presence in the zone)
	//Note that the width of the box is a fixed 1-metre, and the box is shorter than the cone.
	if(!bInZone)
	{
		float fDotVal = obOwnerToEnterer.Dot(obOwnerZ);
		//We build a 2D representation of this approach vector in CUsePoint space using obUsePointFacingDir as Y.
		//We then check the x and y offsets from the use point to see if the character is within our box-area.
		float fXOffset2D = 0.0f;
		float fYOffset2D = 0.0f;

		float fOwnerToEntererLen = CDirection( obEntererPosition - obOwnerPosition ).Length();
		fXOffset2D = fOwnerToEntererLen * acosf(fDotVal);
		//Is this x-offset to the left or right? Adjust accordingly.
		CDirection obRightVectorNorm = obOwnerToEnterer.Cross(CDirection(0.0f, 1.0f, 0.0)); obRightVectorNorm.Normalise();
		CDirection obLeftVectorNorm = -obRightVectorNorm;
		if(obRightVectorNorm.Dot(obOwnerToEnterer) > obLeftVectorNorm.Dot(obOwnerToEnterer))
		{
			fXOffset2D = -fXOffset2D;
		}
		fYOffset2D = fOwnerToEntererLen * asinf(fDotVal);

		//Perform our box check.
		if((fXOffset2D <= 1.0f) && (fXOffset2D >= -1.0f) && (fYOffset2D >= fInnerDistance) && (fYOffset2D <= fOuterDistance))
		{
			//It's in the box.
			bInZone = true;

		}
	}

	//Return the result.
	return bInZone;
}

//------------------------------------------------------------------------------------------
//!
//!	AwarenessComponent::FindTarget
//!	
//!
//------------------------------------------------------------------------------------------
CEntity* AwarenessComponent::FindTarget(	ATTACK_TARGET_TYPE	eTargetType,
											const CMatrix&		obThisCharacter, 
											const CEntity*		pobThis, 
											float 				fInnerDistance, 
											float 				fOuterDistance, 
											float 				fAngle,
											const CEntity*		pobExemptThisEntity ) const
{

	// Only allow the query to execute on AI's and players
	int iEntTypeMask = CEntity::EntType_Character;

	// Find all that is in the lock on range
	CEQCProximitySegment obProximitySub;
	obProximitySub.SetRadius( fOuterDistance );
	obProximitySub.SetAngle( fAngle * DEG_TO_RAD_VALUE );
	obProximitySub.SetMatrix( obThisCharacter );

	// Set up some height bounds so we don't get any gippyness
	CEQCHeightRange obHeightSub;	
	obHeightSub.SetBottom( m_pobAttackTargetingData->m_fLowerHeight );
	obHeightSub.SetTop( m_pobAttackTargetingData->m_fUpperHeight );
	obHeightSub.SetRelativeY( obThisCharacter.GetTranslation().Y() );

	// Create my query object
	CEntityQuery obQuery;
	obQuery.AddClause( obProximitySub );

	CEQCIsGroundAttackable obGroundAttackable;
	// Where necessary check that we have a sensible height range on the targeting
	if ( ( eTargetType == AT_TYPE_DOWN ) 
		|| 
		( eTargetType == AT_TYPE_DOWN_INNER ) )
	{		
		obHeightSub.SetBottom( m_pobAttackTargetingData->m_fGroundAttackHeightBottom );
		obHeightSub.SetTop( m_pobAttackTargetingData->m_fGroundAttackHeightTop );
		obQuery.AddClause(obGroundAttackable);
	}	
	else
	{
		// Exclude those in floored or rise wait state
		obQuery.AddUnClause(obGroundAttackable);
	}

	// Make sure we target only KOed or dying people in aerial
	CEQCIsAerialComboable obAerial;
	if (eTargetType == AT_TYPE_AERIAL_COMBO)
	{
		obHeightSub.SetTop(m_pobAttackTargetingData->m_fAerialComboHeight);
		obHeightSub.SetBottom( m_pobAttackTargetingData->m_fAerialComboHeightBottom );
		obQuery.AddClause(obAerial);
	}

	obQuery.AddClause( obHeightSub );

	// These are the other sub queries which may be added - they all have to be here
	// because of the cheap way in which i wrote the query system - all the sub queries
	// have to sit on the stack
	CEQCIsLockonable obLockonSub;
	CEQCIsThrownTarget obThrownTargetSub;
	CEQCIsCanTakeStandardHit obStandardHit;
	CEQCIsCanTakeSpeedExtraHit obSpeedExtraHit;
	CEQCIsTargetableByPlayer obTargettableByPlayerSub;
	CEQCIsTargetableForEvade obTargettableForEvadeSub;

	// Determine if this is a target for a thrown object
	if ( eTargetType == AT_TYPE_THROW )
	{
		// Others must be living characters
		obQuery.AddClause( obThrownTargetSub );
	}
	else
	{
		// Others must be living characters
		obQuery.AddClause( obLockonSub );
	}

	// Only target entities that are enemies, currently used by allies so they don't hit the player. 
	CEQCIsEnemy	obEnemySub;

	// always ignore ninja sequence entities
	CEQCIsInNinjaSequence obNinjaSequence;
	obQuery.AddUnClause( obNinjaSequence );

	// Make sure the entity is ready to receive attacks, ie not in an extrenal controlled state. 
	CEQCIsCombatComponentActive obTargetCombatComponentActive;
	obQuery.AddClause( obTargetCombatComponentActive );

	// Make sure the entity is allowing itself to recieve attacks
	CEQCIsCombatTargetingDisabled obCombatTargetingDisabled;
	obQuery.AddUnClause( obCombatTargetingDisabled );

	// Select enemy or player based on our parent
	if ( pobThis->IsEnemy() )
	{
		if	(
			( pobThis->IsAI() ) &&
			( ((AI*)pobThis)->GetAIComponent()->GetCombatComponent().IsAttackingPlayer() )
			)
		{
			iEntTypeMask = CEntity::EntType_Player;
		}
		else
		{
			obQuery.AddClause( obTargettableByPlayerSub );
		}
	}
	else if (pobThis->IsPlayer() && eTargetType != AT_TYPE_EVADE)
	{
		obQuery.AddClause( obTargettableByPlayerSub );
	}
	// So we still do fancy evades if they're floored etc
	else if (pobThis->IsPlayer() && eTargetType == AT_TYPE_EVADE)
	{
		obQuery.AddClause( obTargettableForEvadeSub );
	}
	// Else the entity type must be a ally, don't allow ally's to hit the player. 
	else
	{
		obQuery.AddClause( obEnemySub );
	}

	// For these target types we only target a sub set of combat states
	if (	( eTargetType == AT_TYPE_MEDIUM_ATTACK )
		 || ( eTargetType == AT_TYPE_LONG_ATTACK )
		 || ( eTargetType == AT_TYPE_EVADE ) )
	{
		obQuery.AddClause( obStandardHit );
	}

	// For speed extra attacks we have target specific states
	else if ( eTargetType == AT_TYPE_SPEED_EXTRA )
	{
		obQuery.AddClause( obSpeedExtraHit );
	}

	// If there is an inner distance specified remove exclude items within it
	CEQCProximityColumn obColumnSub;

	if ( fInnerDistance > 0.0f )
	{
		// Use a column check for removal - quicker
		obColumnSub.SetRadius( fInnerDistance );
		obColumnSub.SetMatrix( obThisCharacter );

		// Add this as a negative sub query
		obQuery.AddUnClause( obColumnSub );
	}

	// Exclude ourselves
	ntAssert( pobThis );
	obQuery.AddExcludedEntity( *pobThis);

	// And anyone else we need to
	if (pobExemptThisEntity)
		obQuery.AddExcludedEntity( *pobExemptThisEntity);

	// Send my query to the entity manager
	CEntityManager::Get().FindEntitiesByType( obQuery, iEntTypeMask );

	// Find the best result from the set returned
	return FindBestResult(	obQuery.GetResults(), 
							obThisCharacter, 
							fInnerDistance, 
							fOuterDistance, 
							fAngle * DEG_TO_RAD_VALUE );
}

//------------------------------------------------------------------------------------------
//!
//!	AwarenessComponent::FindBestResult
//!	Finds the best target from a given list.  Angle input should be in radians
//!
//------------------------------------------------------------------------------------------
CEntity* AwarenessComponent::FindBestResult(	const QueryResultsContainerType&	obEntities,
												const CMatrix&				obCharacterMatrix,
												float						fInnerDistance,
												float						fOuterDistance,
												float						fAngle ) const
{
	// Create a return result
	CEntity* pobBestEntity = 0;

	// Create values to compare the results
	float fBestEntityScore = FLT_MAX;

	// Square the values so we don't need squareroots
	fInnerDistance *= fInnerDistance;
	fOuterDistance *= fOuterDistance;

	ntstd::Map< float, const CEntity* > obTargets;
	// DEBUGRENDERING //////////////////////////////////////////////////////////////////////////////////

	// Loop through the given set and find a score for each of the items
	QueryResultsContainerType::const_iterator obEndIt = obEntities.end();
	for ( QueryResultsContainerType::const_iterator obIt = obEntities.begin(); obIt != obEndIt; ++obIt )
	{
		//If this is a boss targetting another boss, then we don't even process it as an option.
		if(m_pobParent->IsBoss() && (*obIt)->IsBoss())
		{
			continue;
		}

		// Find the relative position of the character from us
		CPoint obRelativePosition( ( *obIt )->GetPosition() - obCharacterMatrix.GetTranslation() );
		obRelativePosition.Y() = 0.0f;

		// Find the distance of the character from us
		float fCharacterDistance = obRelativePosition.LengthSquared();

		// Find the distance as a percentage between the given distance
		float fDistanceFactor = ( ( fCharacterDistance - fInnerDistance ) / ( fOuterDistance - fInnerDistance ) );

		// Find the angle of the character from us
		float fCharacterAngle = MovementControllerUtilities::RotationAboutY( obCharacterMatrix.GetZAxis(), CDirection( obRelativePosition ) );
		if ( fCharacterAngle < 0.0f ) fCharacterAngle = -fCharacterAngle;

		// Find the angle of the character as a percentage of the input angle
		float fAngleFactor = ( fCharacterAngle / fAngle );

		// Find a 'score' for this entity
		float fEntityScore = ( ( fDistanceFactor * m_pobAttackTargetingData->m_fDistanceWeighting ) + ( fAngleFactor * m_pobAttackTargetingData->m_fAngleWeighting ) );

		// DEBUGRENDERING //////////////////////////////////////////////////////////////////////////////////
		if ( m_bDebugRender )
		{
			obTargets[fEntityScore] = *obIt;
		}
		// DEBUGRENDERING //////////////////////////////////////////////////////////////////////////////////

		// If this is the best score so far update the best score and the 'winning' entity
		if ( fEntityScore < fBestEntityScore )
		{
			pobBestEntity = *obIt;
			fBestEntityScore = fEntityScore;
		}
	}

	// DEBUGRENDERING //////////////////////////////////////////////////////////////////////////////////
#ifndef _GOLD_MASTER
	if ( m_bDebugRender )
	{
		// Display the targeting order above the characters in the list
		int iTargetPreference = 1;
		ntstd::Map< float, const CEntity* >::iterator obEndIt = obTargets.end();
		for ( ntstd::Map< float, const CEntity* >::iterator obIt = obTargets.begin(); obIt != obEndIt; ++obIt )
		{
			// Get the entities position and offset to above them
			CPoint obPosition( obIt->second->GetPosition() );
			obPosition.Y() += 1.7f;
			
			// Render the target preference number
			char acString [10];
			sprintf( acString, "%d" , iTargetPreference );
			g_VisualDebug->Printf3D( obPosition, 0xffffffff, DTF_ALIGN_HCENTRE, acString );

			// Progress the target preference
			iTargetPreference++;
		}
	}
#endif
	// DEBUGRENDERING //////////////////////////////////////////////////////////////////////////////////

	// Return the result - this may be null
	return pobBestEntity;
}


//------------------------------------------------------------------------------------------
//!
//!	AwarenessComponent::FindAwareOfEntities
//!	Find a list of entities that the character is aware of using basic distance parameters
//!
//------------------------------------------------------------------------------------------
const CEntity* AwarenessComponent::FindAwareOfEntities( CEntityQuery& obQuery )
{
//	CGatso::Start2( "FindAwareOfEntities - 1" ); // #### 10%
	// The entities we find also have to be lockonable
	CEQCIsLockonable obLockonSub;
	obQuery.AddClause( obLockonSub );

	// always ignore ninja sequence entities
	CEQCIsInNinjaSequence obNinjaSequence;
	obQuery.AddUnClause( obNinjaSequence );

	// Set a proximity sub query
	CEQCProximitySegment obProximitySub;
	obProximitySub.SetRadius( m_pobAttackTargetingData->m_fAwareDistance );
	obProximitySub.SetAngle( m_pobAttackTargetingData->m_fAwareAngle * DEG_TO_RAD_VALUE );
	obProximitySub.SetMatrix( m_pobParent->GetMatrix() );
	obQuery.AddClause( obProximitySub );

	// Exclude ourselves
	ntAssert( m_pobParent );
	obQuery.AddExcludedEntity( *m_pobParent );

//	CGatso::Start2( "FindAwareOfEntities - 1.1" ); // #### 8%
	// Send my query to the entity manager
	CEntityManager::Get().FindEntitiesByType( obQuery, CEntity::EntType_Character );
//	CGatso::Stop2( "FindAwareOfEntities - 1.1" );

//	CGatso::Stop2( "FindAwareOfEntities - 1" );

	// Find the best result from the set returned
	return FindBestResult(	obQuery.GetResults(), 
							m_pobParent->GetMatrix(), 
							0.0f, 
							m_pobAttackTargetingData->m_fAwareDistance, 
							m_pobAttackTargetingData->m_fAwareAngle * DEG_TO_RAD_VALUE );
}


//------------------------------------------------------------------------------------------
//!
//!	AwarenessComponent::Update
//!	
//!
//------------------------------------------------------------------------------------------
void AwarenessComponent::Update( float /* fTimeStep */ )
{
//	CGatso::Start2( "Aware - 0" ); // ##### 33%
	// We don't want to do any of this for AI controlled characters
	if ( !m_pobParent->IsAI() )
	{
//		CGatso::Start2( "Aware - 1" ); // ##### 26%
//		CGatso::Start2( "Aware - 1.1" ); // ##### 16%
//		CGatso::Start2( "Aware - 1.1.1" ); // ##### 0.0%
		// Set up our 'input' data based on the character type
		if ( const CInputComponent* pobInput = m_pobParent->GetInputComponent() )
		{
			m_fMovementMagnitude = pobInput->GetInputSpeed();
			m_obMovementDirection = pobInput->GetInputDir();
		}
//		CGatso::Stop2( "Aware - 1.1.1" );

#ifdef _DEBUG_TARGET

		// This provides some debug rendering for the targeting code
		g_VisualDebug->RenderLine(	obDebugTargetStart,
											obDebugTargetEnd,
											0xffff0000, 0 );

#endif

//		CGatso::Start2( "Aware - 1.1.2" ); // ##### 16%
		// See if we have any entities that we are aware of
		CEntityQuery obQuery;
		m_pobAwarenessEntity = FindAwareOfEntities( obQuery ); // 9%
//		CGatso::Stop2( "Aware - 1.1.2" );

		// Store a pointer to the old auto lockon
		CEntity* pobOldLockon = m_pobAutoLockonEntity;
//		CGatso::Stop2( "Aware - 1.1" );

//		CGatso::Start2( "Aware - 1.2" ); // ##### 10%
		// If there is a currently locked on item make sure that it is still in range
		if ( m_pobAutoLockonEntity )
		{
			// Find the position of the entity we are currently locked on to
			CPoint obPosition( m_pobAutoLockonEntity->GetPosition() );

			// Find the position relative to us
			obPosition -= m_pobParent->GetPosition();
			obPosition.Y() = 0.0f;

			// If the distance between us is greater than the lockoff radius ( + 25% for stability ) then remove the lock 
			float fLockOnDistance = m_pobAttackTargetingData->m_fAutoLockOnDistance * 1.25f;
			fLockOnDistance *= fLockOnDistance;
			if ( obPosition.LengthSquared() > fLockOnDistance )
				m_pobAutoLockonEntity = 0;

			// If we are stil locked on here then we should check to see if we have a better option
			if ( ( m_pobAutoLockonEntity )
				 &&
				 ( m_fMovementMagnitude < m_pobAttackTargetingData->m_fBreakAwayMagnitude ) )
			{
				// Save the current lock on
				CEntity* pobCurrent = m_pobAutoLockonEntity;

				// Try for another lockon within the auto parameters - we use a direction here
				m_pobAutoLockonEntity = GetTarget( AT_TYPE_AUTO, m_pobParent );

				// If we didn't find one set the current target back
				if ( m_pobAutoLockonEntity == 0 )
				{
					m_pobAutoLockonEntity = pobCurrent;
				}
				else
				{
					// Update the position information
					obPosition = m_pobAutoLockonEntity->GetPosition();
					obPosition -= m_pobParent->GetPosition();
					obPosition.Y() = 0;
				}
			}

			// If we are still locked on at this point - check to see if we should perform an instant breakaway
			if ( m_pobAutoLockonEntity )
			{
				// Check to see if the pad magnitude is great enough for a breakaway
				if ( m_fMovementMagnitude >= m_pobAttackTargetingData->m_fBreakAwayMagnitude )
				{
					// In which direction is the opponent from us?
					CDirection obOpponentDirection( obPosition );

					// Find the angle betweeen that and the current pad input
					float fRelativeAngle = MovementControllerUtilities::RotationAboutY( obOpponentDirection, m_obMovementDirection ) * RAD_TO_DEG_VALUE;

					// Check the magnitude of this angle against the scripted parameter
					if ( ( fRelativeAngle >= m_pobAttackTargetingData->m_fMinimumBreakAwayAngle ) 
						|| 
						( fRelativeAngle <= -m_pobAttackTargetingData->m_fMinimumBreakAwayAngle ) )
					{
						m_pobAutoLockonEntity = 0;
					}
				}
			}
		}

		// If we are very close to a possible target then we should automatically lock on
		// Don't use a direction to search here
		else
		{
			// We can't get a new lock on target if we are running
			if ( m_fMovementMagnitude < m_pobAttackTargetingData->m_fBreakAwayMagnitude )
				m_pobAutoLockonEntity = GetTarget( AT_TYPE_AUTO, m_pobParent );
		}
//		CGatso::Stop2( "Aware - 1.2" );

//		CGatso::Start2( "Aware - 1.3" ); // ##### 0.0%
		// If we have a target entity then set the position as input to the movement system
		if ( m_pobAutoLockonEntity )
		{
			ntError( m_pobParent && m_pobParent->GetMovement() );
			m_pobParent->GetMovement()->m_obMovementInput.m_obTargetPoint = m_pobAutoLockonEntity->GetPosition();
			m_pobParent->GetMovement()->m_obMovementInput.m_bTargetPointSet = true;
		}

		// ...otherwise if we have an awareness entity we use that as tracking data
		else if ( m_pobAwarenessEntity )
		{
			ntError( m_pobParent && m_pobParent->GetMovement() );
			m_pobParent->GetMovement()->m_obMovementInput.m_obTargetPoint = m_pobAwarenessEntity->GetPosition();
			m_pobParent->GetMovement()->m_obMovementInput.m_bTargetPointSet = true;
		}
		else
		{
			// Must explicitly set not to use a target point to prevent true values from past being used incorrectly
			//m_pobParent->GetMovement()->m_obMovementInput.m_bTargetPointSet = false;
		}

		// We need to tell the state system if we have gained or lost an auto lockon
		if ( m_pobParent->GetMessageHandler() )
		{
			if ( ( pobOldLockon == 0 ) && ( m_pobAutoLockonEntity != 0 ) )
				CMessageSender::SendEmptyMessage( CHashedString(HASH_STRING_MSG_AWARE_GAINED_LOCKON), m_pobParent->GetMessageHandler() );
			else if ( ( pobOldLockon != 0 ) && ( m_pobAutoLockonEntity == 0 ) )
				CMessageSender::SendEmptyMessage( CHashedString(HASH_STRING_MSG_AWARE_LOST_LOCKON), m_pobParent->GetMessageHandler() );
		}
//		CGatso::Stop2( "Aware - 1.3" );
//		CGatso::Stop2( "Aware - 1" );
	}
//	CGatso::Stop2( "Aware - 0" );
}


CEntity* AwarenessComponent::FindRangedWeaponInteractionTarget(RANGED_WEAPON_TYPE eWeaponType, int iCharacterType)
{
//	ntPrintf("Named interaction target substring is %s", pcNameSubstring);
	CInteractionTarget obTargetResult;
	obTargetResult.m_pobInteractingEnt = 0;

	CEntity*	pobThis = ((AwarenessComponent*)this)->m_pobParent;

#ifdef _AWAREBINDINGS_DEBUG
	ntPrintf("Entity %s - Find interaction target\n", pobThis->GetName().c_str() );
#endif

	if ((pobThis->IsAI() || pobThis->GetInputComponent()) && pobThis->GetAwarenessComponent())
	{
		// Determine max search distance based on movement magnitude.
		// ----------------------------------------------------------------------
		float fMAX_DISTANCE;

		if ( (pobThis->IsAI() && ((AI*)pobThis)->GetAIComponent()->GetMovementMagnitude() > 0.3f ) || 
			 (pobThis->GetInputComponent() && pobThis->GetInputComponent()->IsDirectionHeld() ) )
		{
			fMAX_DISTANCE = 4.0f * 4.0f;
		}
		else
		{
			fMAX_DISTANCE = 2.0f * 2.0f;
		}

		// Get position and targeting direction of this entity
		// ----------------------------------------------------------------------
		CPoint obPosition(pobThis->GetHierarchy()->GetRootTransform()->GetWorldMatrix().GetTranslation());
		CDirection obDirection;

		// If this entity is an AI or the analog stick isn't held, then use their character matrix
		if ( pobThis->IsAI() || !pobThis->GetInputComponent()->IsDirectionHeld() )
		{
			obDirection=pobThis->GetHierarchy()->GetRootTransform()->GetWorldMatrix().GetZAxis();
		}
		else // Otherwise use pad direction
		{
			obDirection=pobThis->GetInputComponent()->GetInputDir();
		}

		const CDirection obPlaneNormalOfApproach(pobThis->GetHierarchy()->GetRootTransform()->GetWorldMatrix().GetYAxis());

		// Query object database for all entities with a matching substring.
		// ----------------------------------------------------------------------
		CEQCIsInteractionTarget obClause;
		CEntityQuery obQuery;
		obQuery.AddClause(obClause);
		CEQCIsRangedWeaponWithType obRangedWeaponType(eWeaponType);
		obQuery.AddClause(obRangedWeaponType);
		CEntityManager::Get().FindEntitiesByType( obQuery, CEntity::EntType_AllButStatic );
		
		// Create a return result
		CEntity* pobBestEntity = 0;
		CUsePoint* pobBestUPoint = 0;

		// Create values to compare the results
		float fBestEntityScore = 0.0f;

		// Go through each entity in the results that isn't paused
		// ----------------------------------------------------------------------
		QueryResultsContainerType::const_iterator obEndIt = obQuery.GetResults().end();
		for ( QueryResultsContainerType::const_iterator obIt = obQuery.GetResults().begin(); obIt != obEndIt; ++obIt )
		{
			CEntity* thisEnt = *obIt;
			UNUSED(thisEnt);
			ntPrintf("Padding");

			// Ensure entity is not paused
			if (!(*obIt)->IsPaused() && pobThis != (*obIt))
			{
				float fThreeTestBestScore = 0.0f;


				CUsePoint* pobBestUPoint1,*pobBestUPoint2,*pobBestUPoint3,*pobUPBestOf3;

				// Get score, passing in this position and angle
				float fScore1 = (*obIt)->GetInteractionComponent()->GetInteractionScore( obPosition, 
																						 obDirection, 
																						 obPlaneNormalOfApproach, 
																						 fMAX_DISTANCE,
																						 (CUsePoint::InteractingCharacterType)iCharacterType,
																						 pobBestUPoint1 );
				// Get a second score, a step or two back (in-case we overstepped slightly when moving to the object)
				float fScore2 = (*obIt)->GetInteractionComponent()->GetInteractionScore( obPosition - CPoint(obDirection), 
																						 obDirection, 
																						 obPlaneNormalOfApproach, 
																						 fMAX_DISTANCE,
																						 (CUsePoint::InteractingCharacterType)iCharacterType,
																						 pobBestUPoint2 );
				// Get a third score, a step or two forwards (in-case we stopped short when moving to the object)
				float fScore3 = (*obIt)->GetInteractionComponent()->GetInteractionScore( obPosition + CPoint(obDirection), 
																						 obDirection, 
																						 obPlaneNormalOfApproach, 
																						 fMAX_DISTANCE,
																						 (CUsePoint::InteractingCharacterType)iCharacterType,
																						 pobBestUPoint3 );

				//Store the best of these scores.
				if (fScore1 > fScore2)
				{
					fThreeTestBestScore = fScore1;
					pobUPBestOf3 = pobBestUPoint1;
				}
				else
				{
					fThreeTestBestScore = fScore2;
					pobUPBestOf3 = pobBestUPoint2;
				}

				if (fScore3 > fThreeTestBestScore)
				{
					fThreeTestBestScore = fScore3;
					pobUPBestOf3 = pobBestUPoint3;
				}

#ifdef _AWAREBINDINGS_DEBUG
				ntPrintf("entity %s scores %f\n",(*obIt)->GetName().c_str(),fThreeTestBestScore);
#endif

				// If this entity has a better score then remember it
				if (fThreeTestBestScore > fBestEntityScore)
				{
					pobBestEntity = (*obIt);
					fBestEntityScore = fThreeTestBestScore;
					pobBestUPoint = pobUPBestOf3;
				}
			}
		}

		if (pobBestEntity)
		{
#ifdef _AWAREBINDINGS_DEBUG
			ntPrintf("Found interaction target=%s\n",pobBestEntity->GetName().c_str());
#endif
			obTargetResult.m_pobInteractingEnt = pobBestEntity;
			obTargetResult.m_pobClosestUsePoint = pobBestUPoint;
			return obTargetResult.m_pobInteractingEnt;
		}
	}	
	// If score is better than current one, save this new entity and its score.

	// We didn't find a suitable interaction target
	return obTargetResult.m_pobInteractingEnt;
}
