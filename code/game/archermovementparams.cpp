//------------------------------------------------------------------------------------------
//!
//!	\file archermovementparams.cpp
//!
//------------------------------------------------------------------------------------------

#include "core/boundingvolumes.h"

#include "game/archermovementparams.h"
#include "game/entityarcher.h"
#include "game/inputcomponent.h"
#include "game/staticentity.h"
#include "game/renderablecomponent.h"
#include "game/randmanager.h"
#include "game/vaultingtransition.h"
#include "game/movement.h"
#include "ai/ainavigationsystem/ainavigsystemman.h"
#include "objectdatabase/dataobject.h"

#include "core/visualdebugger.h"

//------------------------------------------------------------------------------------------
//!
//!	Constants and default values.
//!
//------------------------------------------------------------------------------------------
static const float Def_VaultRayCastLength	= 1.5f;
static const float Def_VaultRayAngle		= 45.0f;
static const float Def_VaultUnitHeight		= 0.75f;
static const float Def_VaultRayCastZOffset	= 0.6f;
static const float Def_CrouchVolumeLength	= 2.0f;
static const float Def_CrouchVolumeRadius	= 1.0f;
static const float Def_VaultCacheTimeOut	= 0.75f;


//------------------------------------------------------------------------------------------
//!
//!	Define how our parameters are exposed within XML.
//!
//------------------------------------------------------------------------------------------
START_STD_INTERFACE	( ArcherMovementParams )
	PUBLISH_VAR_WITH_DEFAULT_AS			( m_VaultRayCastLength,	Def_VaultRayCastLength,		VaultRayCastLength )
	PUBLISH_VAR_WITH_DEFAULT_AS			( m_CosVaultRayAngle,	Def_VaultRayAngle,			VaultRayAngle )			// NOTE: In XML, this is in degrees - in PostConstruct we convert to radians and take the cosine.
	PUBLISH_VAR_WITH_DEFAULT_AS			( m_VaultUnitHeight,	Def_VaultUnitHeight,		VaultUnitHeight )
	PUBLISH_PTR_CONTAINER_AS			( m_HighVaultAnims,		HighVaultAnims )
	PUBLISH_PTR_CONTAINER_AS			( m_LowVaultAnims,		LowVaultAnims )
	PUBLISH_VAR_WITH_DEFAULT_AS			( m_CrouchVolumeLength,	Def_CrouchVolumeLength,		CrouchVolumeLength )
	PUBLISH_VAR_WITH_DEFAULT_AS			( m_CrouchVolumeRadius,	Def_CrouchVolumeRadius,		CrouchVolumeRadius )
	PUBLISH_VAR_WITH_DEFAULT_AS			( m_VaultCacheTimeOut,	Def_VaultCacheTimeOut,		VaultCacheTimeOut )
	DECLARE_POSTCONSTRUCT_CALLBACK		( PostConstruct )
	DECLARE_EDITORCHANGEVALUE_CALLBACK	( ValueChanged )
	DECLARE_DELETE_CALLBACK				( DeleteCallback )
END_STD_INTERFACE

//------------------------------------------------------------------------------------------
//!
//!	Define how our parameters are exposed within XML.
//!
//------------------------------------------------------------------------------------------
START_STD_INTERFACE	( ArcherVaultParams )
	PUBLISH_VAR_AS					( m_bEnabled,			Enabled )
	PUBLISH_VAR_AS					( m_DistanceScaling,	DistanceScaling )
	PUBLISH_VAR_AS					( m_VaultAnimName,		AnimName )
	PUBLISH_VAR_AS					( m_NextVaultPopout,	NextPopout )			
	PUBLISH_VAR_AS					( m_MovementPopout,		MovementPopout )			
	PUBLISH_VAR_WITH_DEFAULT_AS		( m_fJumpDistanceRequired,	0.5f,	JumpDistanceRequired )
	PUBLISH_VAR_WITH_DEFAULT_AS		( m_fJumpDistanceLimit,		2.5f,	JumpDistanceLimit )
	PUBLISH_VAR_WITH_DEFAULT_AS		( m_fBlendOutTime,		0.15f,	BlendOutTime)
END_STD_INTERFACE

//------------------------------------------------------------------------------------------
//!
//!	Expose an archer 3rd person attack state interface
//!
//------------------------------------------------------------------------------------------
START_STD_INTERFACE	(ThirdPersonAttackState)
	PUBLISH_VAR_AS( m_IdleAnim,							IdleAnim )
	PUBLISH_VAR_AS( m_FireAnim,							FireAnim )
	PUBLISH_VAR_AS( m_IdleUpAnim,						IdleUpAnim )
	PUBLISH_VAR_AS( m_FireUpAnim,						FireUpAnim )
	PUBLISH_VAR_AS( m_IdleRightAim,						IdleRightAim )
	PUBLISH_VAR_AS( m_FireRightAim,						FireRightAim )
	PUBLISH_VAR_AS( m_IdleRightUpAim,					IdleRightUpAim )
	PUBLISH_VAR_AS( m_FireRightUpAim,					FireRightUpAim )
	PUBLISH_VAR_AS( m_IdleLeftAim,						IdleLeftAim )
	PUBLISH_VAR_AS( m_FireLeftAim,						FireLeftAim )
	PUBLISH_VAR_AS( m_IdleLeftUpAim,					IdleLeftUpAim )
	PUBLISH_VAR_AS( m_FireLeftUpAim,					FireLeftUpAim )
	PUBLISH_VAR_AS( m_PopoutAnim,						PopoutAnim )
	PUBLISH_VAR_AS( m_ReloadAnim,						ReloadAnim )
	PUBLISH_VAR_AS( m_WeaponIdle,						WeaponIdle )
	PUBLISH_VAR_AS( m_WeaponFire,						WeaponFire )
	PUBLISH_VAR_AS( m_WeaponReload,						WeaponReload )
	PUBLISH_VAR_WITH_DEFAULT_AS( m_AngleRange,			120.0f,			AngleRange )
	PUBLISH_VAR_WITH_DEFAULT_AS( m_VerticalAngleLimit,	30.0f,			VerticalAngleLimit )
	PUBLISH_VAR_WITH_DEFAULT_AS( m_RotationSpeedLimit,	500.0f,			RotationSpeedLimit )
	PUBLISH_VAR_WITH_DEFAULT_AS( m_LockOnThreshold,		-1.0f,			LockOnThreshold )
	PUBLISH_PTR_AS( m_PopoutTransition,					PopoutTransition )
	PUBLISH_VAR_AS( m_FirePopout,						FirePopout )
	PUBLISH_VAR_AS( m_PopoutTime,						PopoutTime )
	PUBLISH_VAR_AS( m_PopoutTimeMag1,					PopoutTimeMag1 )
	PUBLISH_PTR_CONTAINER_AS( m_Transitions,			Transitions )
	PUBLISH_PTR_AS( m_Targeting,						Targeting )
END_STD_INTERFACE

//------------------------------------------------------------------------------------------
//!
//!	Expose an archer 3rd person attack transition interface
//!
//------------------------------------------------------------------------------------------
START_STD_INTERFACE	(ThirdPersonAttackTransition)
	PUBLISH_VAR_AS( m_InputMagnitude, InputMagnitudenitude )
	PUBLISH_VAR_AS( m_HorizontalAngleOffset, HorizontalAngleOffset )
	PUBLISH_VAR_AS( m_HorizontalAngleRange, HorizontalAngleRange )
	PUBLISH_VAR_AS( m_TransitionAnim, TransitionAnim )
	PUBLISH_VAR_AS( m_TransitionAnimSpeed, TransitionAnimSpeed )
	PUBLISH_VAR_AS( m_BlendInTime, BlendInTime )
	PUBLISH_VAR_AS( m_EarlyOutTime, EarlyOutTime )
	PUBLISH_VAR_AS( m_StateUpTimeCheck, StateUpTimeCheck )
	PUBLISH_PTR_AS( m_TransitionTo, TransitionTo )
END_STD_INTERFACE

//------------------------------------------------------------------------------------------
//!
//!	Expose an archer targeting segments
//!
//------------------------------------------------------------------------------------------
START_STD_INTERFACE	(ThirdPersonTargetingSegment)
	PUBLISH_VAR_AS( m_fBaseRotation, BaseRotation )
	PUBLISH_VAR_AS( m_fAngle,  Angle )
	PUBLISH_VAR_AS( m_fRadius, Radius )
	PUBLISH_VAR_AS( m_fHeight, Height )
	PUBLISH_VAR_AS( m_vDebugColour, DebugColour )
END_STD_INTERFACE

//------------------------------------------------------------------------------------------
//!
//!	Expose an archer targeting segments
//!
//------------------------------------------------------------------------------------------
START_STD_INTERFACE	( ThirdPersonTargeting )
	PUBLISH_PTR_CONTAINER_AS( m_SegmentList, SegmentList )
	PUBLISH_PTR_CONTAINER_AS( m_ActiveTargetSegmentList, ActiveTargetSegmentList )
END_STD_INTERFACE


//------------------------------------------------------------------------------------------
//!
//!	Constructor.
//!
//------------------------------------------------------------------------------------------
ArcherMovementParams::ArcherMovementParams()
:	m_VaultRayCastLength	( Def_VaultRayCastLength )
,	m_CosVaultRayAngle		( Def_VaultRayAngle )
,	m_VaultUnitHeight		( Def_VaultUnitHeight )
,   m_VaultRayCastZOffset	( Def_VaultRayCastZOffset )
,	m_CrouchVolumeLength	( Def_CrouchVolumeLength )
,	m_CrouchVolumeRadius	( Def_CrouchVolumeRadius )
,	m_VaultCacheTimeOut		( Def_VaultCacheTimeOut )
{}

//------------------------------------------------------------------------------------------
//!
//!	Post-construction function.
//!
//------------------------------------------------------------------------------------------
void ArcherMovementParams::PostConstruct()
{
	ValueChanged( NULL, NULL );
}

//------------------------------------------------------------------------------------------
//!
//!	Delete callback function.
//!
//------------------------------------------------------------------------------------------
void ArcherMovementParams::DeleteCallback()
{
}

//------------------------------------------------------------------------------------------
//!
//!	ValueChanged function.
//!
//------------------------------------------------------------------------------------------
bool ArcherMovementParams::ValueChanged( CallBackParameter, CallBackParameter )
{
	ntError_p( m_VaultRayCastLength > 0.0f, ("VaultRayCastLength should be greater than zero.") );
	ntError_p( m_VaultUnitHeight > 0.0f, ("VaultUnitHeight should be greater than zero.") );

	ntError_p( m_CosVaultRayAngle >= 0.0f && m_CosVaultRayAngle <= 90.0f, ("VaultRayAngle must be between 0 and 90 degress - resetting to 45 degrees.") );
	if ( m_CosVaultRayAngle < 0.0f || m_CosVaultRayAngle > 90.0f )
	{
		m_CosVaultRayAngle = Def_VaultRayAngle;
	}

	m_CosVaultRayAngle = fcosf( m_CosVaultRayAngle * PI / 180.0f );

	return true;
}

//------------------------------------------------------------------------------------------
//!
//!	ThirdPersonAttackTransition::ThirdPersonAttackTransition
//!	Construction
//!
//------------------------------------------------------------------------------------------
ThirdPersonAttackTransition::ThirdPersonAttackTransition(void)
{
	m_InputMagnitude = 0.0f;
	m_HorizontalAngleOffset = 0.0f;
	m_HorizontalAngleRange = 0.0f;
	m_TransitionAnimSpeed = 1.0f;
	m_BlendInTime = 0.0f;
	m_EarlyOutTime = -1.0f;
	m_StateUpTimeCheck = 0.0f;
	m_TransitionTo = 0;
}

//------------------------------------------------------------------------------------------
//!  public constructor  ThirdPersonAttackState
//!
//!
//!  @author GavB @date 01/06/2006
//------------------------------------------------------------------------------------------
ThirdPersonAttackState::ThirdPersonAttackState(void)
{
	m_PopoutTime = 5.0f;
	m_Targeting = 0;
}

//------------------------------------------------------------------------------------------
//!  public constructor  ThirdPersonTargetingSegment
//!
//!
//!  @author GavB @date 20/06/2006
//------------------------------------------------------------------------------------------
ThirdPersonTargetingSegment::ThirdPersonTargetingSegment(void) :
	m_fBaseRotation(0.0f),
	m_fAngle(180.0f),
	m_fRadius(5.0f),
	m_fHeight(2.0f)
{
	m_vDebugColour.SetFromNTColor( DC_RED );
}

//------------------------------------------------------------------------------------------
//!  public constructor  ThirdPersonTargeting
//!
//!
//!  @author GavB @date 20/06/2006
//------------------------------------------------------------------------------------------
ThirdPersonTargeting::ThirdPersonTargeting()
{
}


//------------------------------------------------------------------------------------------
//!  public  VaultObjectIfAppropriate
//!
//!  @return bool <TODO: insert return value description here>
//!
//!  @remarks <TODO: insert remarks here>
//!
//!  @author GavB @date 03/08/2006
//------------------------------------------------------------------------------------------
bool Archer::VaultObjectIfAppropriate( bool bTestOnly )
{
	// Movement options for the archer. 
	const ArcherMovementParams *pMovementParams = ObjectDatabase::Get().GetPointerFromName< ArcherMovementParams * >( "ArcherMovementParams" );

	// Static magic value that one day could come from data
	static const float Vaulting_DeadZone = 0.1f;

	// If we're not pushing the stick, use our dirFacing direction.
	CDirection dirFacing = (GetInputComponent()->GetInputSpeed() < Vaulting_DeadZone) ? GetMatrix().GetZAxis() : GetInputComponent()->GetInputDir();

	// Adjust the starting point of the ray
	const float fAdjust = pMovementParams->m_VaultRayCastZOffset;

	// Get the archers position and with it construct a ray start and end point	
	CPoint ptArcher			= GetPosition();
	CPoint ptRayOffset		= CPoint(0.0f, pMovementParams->m_VaultUnitHeight * 0.7f, 0.0f);
	CPoint ptRayStart		= ptArcher + ptRayOffset - (dirFacing * fAdjust);
	CPoint ptRayEnd			= ptRayStart + dirFacing * (pMovementParams->m_VaultRayCastLength + fAdjust);

	// Results from the ray test. 
	CPoint ptRetIntersect;
	CDirection dirRetNorm;

	const CAIWorldVolume* pVolume = CAINavigationSystemMan::Get().IntersectsVaultingVolume( ptRayStart, ptRayEnd, &ptRetIntersect, &dirRetNorm );

	if( pVolume )
	{
		CPoint ptOffset(0.3f, 0.0f, 0.0f);
		CPoint		ptResult; 
		CDirection	dirResult; 
		
		// Create another couple of tests to make sure the archer isn't at the corner of a volume.
		if(!(pVolume->IntersectsAIVolume( ptRayStart + ptOffset, ptRayEnd + ptOffset, &ptResult, &dirResult ) && 
			 pVolume->IntersectsAIVolume( ptRayStart - ptOffset, ptRayEnd - ptOffset, &ptResult, &dirResult ) ) )
		{
			return false;
		}

		// Make sure the direction normal is clean
		dirRetNorm.Normalise();

		// Make sure the Archer is facing the direction requested to vault. 
		if( dirRetNorm.Dot( GetMatrix().GetZAxis() ) > -0.5f )
			return false;

		// If the start of the ray is inside the volume, then the results returned will be all bad. 
		if( pVolume->IsInside( ptRayStart ) )
		{
			//ntError( false && "Archer was inside a vaulting volume, please ask for the volume at the archers current position to be made a little smaller" );
			return false;
		}

		const SVaultingParams* pVolumeVaultingParams	= pVolume->GetVaultingParams ( );
		bool	bBigJump								= pVolume->GetHeight() > pMovementParams->m_VaultUnitHeight;
		float	fDistanceToVault						= pVolumeVaultingParams ? pVolumeVaultingParams->fVaultingDistance : 1.5f;

		CPoint		ptNextRayEnd						= ptRetIntersect + (-dirRetNorm * 0.01f);
		CPoint		ptNextRayStart						= ptRetIntersect + (-dirRetNorm * 100.0f);
		CPoint		ptNextIntersection;
		CDirection	dirNextIntersectionNormal;

		// Find an intersection on the other side of the volume the archer is vaulting over. 
		if( pVolume->IntersectsAIVolume( ptNextRayStart, ptNextRayEnd, &ptNextIntersection, &dirNextIntersectionNormal ) ) 
		{
			// If found, then use the value to calculate the width of the jump. 
			fDistanceToVault = (ptNextIntersection - ptRetIntersect).Length();
		}


		// Increase the distance to vault by a little. 
		fDistanceToVault += fDistanceToVault * 0.5f;

#ifndef _GOLD_MASTER
		g_VisualDebug->RenderLine(ptArcher, ptNextIntersection, DC_CYAN);
#endif

		// Based on the height of the volume detected, decide a height anim set to use
		const ntstd::List<ArcherVaultParams*>& rVaultSet = bBigJump ? pMovementParams->m_HighVaultAnims : pMovementParams->m_LowVaultAnims;

		if( rVaultSet.size() <= 0 )
			return false;

		// Create an iterator to scan through all the vaulting anims. 
		ntstd::List<ArcherVaultParams*>::const_iterator obVaultParam = rVaultSet.begin();
		uint uiAttempt = 0;

		// Attemp to find a vaulting animation for the obstruction. 
		do
		{
			// Pick a random anim from the vaulting set. 
			int iIndex = grand() % rVaultSet.size();

			if( m_iLastVaultAnimPlayed == iIndex )
				iIndex = (m_iLastVaultAnimPlayed + 1) % rVaultSet.size();

			m_iLastVaultAnimPlayed = iIndex;

			// Iterate over to the volume. 
			while( iIndex-- )
				++obVaultParam;

			// Check that this animation will suit the distance to vault. 
			if( fDistanceToVault >= (*obVaultParam)->m_fJumpDistanceRequired && fDistanceToVault < (*obVaultParam)->m_fJumpDistanceLimit && (*obVaultParam)->m_bEnabled )
				break;

			// Reset the vaulting anim iterator
			obVaultParam = rVaultSet.begin();

		}while( ++uiAttempt < rVaultSet.size() );

		// Did the search fail to find a valid animation to play?
		if( uiAttempt >= rVaultSet.size() )
			return false;

		// 
		VaultingTransitionDef vault_def;

		vault_def.m_Facing				= -dirRetNorm;
		vault_def.m_FinalPosition		= ptRetIntersect - ptRayOffset + -dirRetNorm * fDistanceToVault;
		vault_def.m_pVaultingParams		= *obVaultParam;

		// TODO: these values should obtained from XML data. 
		vault_def.m_StartScalePercent	= 0.6f;
		vault_def.m_EndScalePercent		= 0.95f;

		// Aim to land at the same height we started at.
		vault_def.m_FinalPosition.Y()	= ptArcher.Y();	

		// Keep the current Y position of the archer as a point for the camera to look at. 
		m_fVaultingCameraY				= ptArcher.Y();	

		if( !IsVaultingSafe() )
			return false;
		
		if( bTestOnly )
			return true;

		if( GetMovement()->BringInNewController( vault_def, CMovement::DMM_STANDARD, 0.1f ) )
		{
			WalkRunBlendInTime( vault_def.m_pVaultingParams->m_fBlendOutTime );
			SetVaulting();
			return true;
		}
	}

	return false;
}
