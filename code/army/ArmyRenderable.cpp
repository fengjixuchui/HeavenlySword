/***************************************************************************************************
*
*	DESCRIPTION		Class to represent a renderable, animated army object.
*
*	NOTES
*
***************************************************************************************************/

//**************************************************************************************
//	Includes files.
//**************************************************************************************
#include "ArmyRenderable.h"
#include "tbd/functor.h"

#include "core/exportstruct_anim.h"
#include "army/grunt.h"
#include "anim/animloader.h"
#include "anim/hierarchy.h"
#include "anim/animation.h"

#include "game/entityai.h"
#include "game/entity.inl"
#include "game/randmanager.h"

#include "gfx/clump.h"
#include "gfx/levelofdetail.h"

#include "game/renderablecomponent.h"


float ArmyRenderable::s_fMaxRandomDurationVariationAnim = 0.1f;
int ArmyRenderable::s_iVisibleFrameLimit = 10;

//**************************************************************************************
//	
//**************************************************************************************
void ArmyRenderable::Update( float time_step, AnimatorBatchUpdate *batch_updater )
{
	if( !IsRenderingDisabled() )
	{
		if ( m_Animator != NULL /*&& !IsAnimatorDisabled()*/ )
		{
			if ( batch_updater != NULL )
			{
				batch_updater->AddAnimator( m_Animator, time_step );
			}
			else
			{
				m_Animator->CreateBlends( time_step );
			}
		}
	}
}

void ArmyRenderable::UpdatePartTwo()
{
	if( !IsRenderingDisabled() )
	{
		if ( m_Animator != NULL /*&& !IsAnimatorDisabled() */ )
		{
			m_Animator->UpdateResults();
		} else 
		{
			m_Hierarchy->GetRootTransform()->ForceResynchronise();
		}
	}
}

//**************************************************************************************
//	
//**************************************************************************************
void ArmyRenderable::DetachAnimator()
{
	UninstallAnimator();
}

//**************************************************************************************
//	
//**************************************************************************************
void ArmyRenderable::EnableRendering()
{
	m_RenderableComponent->AddRemoveAll_Game( true );
	m_iVisibleFrameCount = s_iVisibleFrameLimit;
}

//**************************************************************************************
//	
//**************************************************************************************
void ArmyRenderable::DisableRendering()
{
	m_RenderableComponent->AddRemoveAll_Game( false );
	m_iVisibleFrameCount = -1;
}

//**************************************************************************************
//	
//**************************************************************************************
bool ArmyRenderable::IsRenderingDisabled() const
{
	return (m_RenderableComponent->DisabledByGame() || m_RenderableComponent->DisabledByArea() );
}

//**************************************************************************************
//	
//**************************************************************************************
void ArmyRenderable::InstallClump( const char* clump_name )
{
	m_ClumpHeader = CClumpLoader::Get().LoadClump_Neutral( clump_name, true );
	ntError_p( m_ClumpHeader != NULL, ("Could not find clump %s", clump_name) );
}

//**************************************************************************************
//	
//**************************************************************************************
void ArmyRenderable::InstallLOD( const char* clump_name )
{
	ntError_p( m_LODComponent == NULL, ("We've already got a LOD component - about to leak memory.") );
	ntError_p( m_RenderableComponent != NULL, ("There must be a renderable component before creating a lod component.") );

	const CLODComponentDef * const lod_def( CLODManager::Get().GetLODPreset( clump_name ) );
	if ( lod_def != NULL )
	{
		m_LODComponent = NT_NEW CLODComponent( m_RenderableComponent, lod_def );
	} else
	{
		ntPrintf( "No LOD Component for %s\n", clump_name );
	}
}

//**************************************************************************************
//	
//**************************************************************************************
void ArmyRenderable::InstallHierarchy( Transform *parent_transform )
{
	ntError_p( m_Hierarchy == NULL, ("We've already got an hierarchy component - about to leak memory.") );
	ntError_p( m_ClumpHeader != NULL, ("There must be a clump header loaded before we install a hierarchy.") );

	// Create the hierarchy from the clump header.
	m_Hierarchy = CHierarchy::Create( m_ClumpHeader );
	ntError_p( m_Hierarchy != NULL, ("Could not create hierarchy.") );

	if ( parent_transform != NULL )
	{
		parent_transform->AddChild( m_Hierarchy->GetRootTransform() );
	}
	else
	{
		CHierarchy::GetWorld()->GetRootTransform()->AddChild( m_Hierarchy->GetRootTransform() );
	}

	m_Hierarchy->GetRootTransform()->SetLocalMatrix( CVecMath::GetIdentity() );
}

//**************************************************************************************
//	
//**************************************************************************************
CAnimationPtr ArmyRenderable::MakeAnimation( GruntAnimState eAnimType )
{
	static uint32_t s_loopMask = (1 << GAM_IDLE) | (1 << GAM_WALK) | (1 << GAM_RUN) | (1 << GAM_DEAD) | (1 << GAM_TAUNT);
	CAnimationPtr result = m_Animator->CreateAnimation( m_animNames[ eAnimType ] );

	if ((1 << eAnimType) & s_loopMask)
	{
		result->SetFlags( ANIMF_LOOPING );
	}

	return result;
}

//**************************************************************************************
//	
//**************************************************************************************
void ArmyRenderable::InstallAnimator( const CHashedString& animContainerName )
{
	ntError_p( m_Animator == NULL, ("We've already got an animator component - about to leak memory.") );
	ntError_p( m_Hierarchy != NULL, ("There must be a hierarchy before creating an animator.") );
	
	m_Animator = CAnimator::Create( static_cast< EntityAnimSet * >( this ), m_Hierarchy );
	m_Animator->SetFlagBits( ANIMATORF_SIMPLE_ANIMATOR );

	EntityAnimSet::InstallAnimator( m_Animator, animContainerName );

	m_animNames[ GAM_IDLE ] = "army_stand";
	m_animNames[ GAM_DIVE_LEFT ] = "army_dive_left";
	m_animNames[ GAM_DIVE_RIGHT ] = "army_dive_right";
	m_animNames[ GAM_GETTING_UP ] = "army_getup1"; 

	switch( (int)erandf(1.9999f) )
	{
	case 0:		m_animNames[ GAM_WALK ] = "army_walk_01"; break;
	case 1:		m_animNames[ GAM_WALK ] = "army_walk_02"; break;
	default:	ntError_p( false, ("Bad Mod op\n") );
	}

	switch( (int)erandf(1.999f) )
	{
	case 0:		m_animNames[ GAM_RUN ] = "army_run_01"; break;
	case 1:		m_animNames[ GAM_RUN ] = "army_run_02"; break;
	default:	ntError_p( false, ("Bad Mod op\n") );
	}

	switch( (int)erandf(6.9999f) )
	{
	case 0:		m_animNames[ GAM_KO_BLUNT ] = "army_explode";  break;
	case 1:		m_animNames[ GAM_KO_BLUNT ] = "army_explode2"; break;
	case 2:		m_animNames[ GAM_KO_BLUNT ] = "army_explode3"; break;
	case 3:		m_animNames[ GAM_KO_BLUNT ] = "army_explode4"; break;
	case 4:		m_animNames[ GAM_KO_BLUNT ] = "army_explode8"; break;
	case 5:		m_animNames[ GAM_KO_BLUNT ] = "army_explode6"; break;
	case 6:		m_animNames[ GAM_KO_BLUNT ] = "army_explode7"; break;
	default:	ntError_p( false, ("Bad Mod op\n") );
	}

	switch( (int)erandf(1.9999f) )
	{
	case 0:		m_animNames[ GAM_DEAD ] = "army_dead1";  break;
	case 1:		m_animNames[ GAM_DEAD ] = "army_dead2"; break;
	default:	ntError_p( false, ("Bad Mod op\n") );
	}

	switch( (int)erandf(9.9999f) )
	{
	case 0:		m_animNames[ GAM_KO_WAKE ] = "army_wake_ko_01"; break;
	case 1:		m_animNames[ GAM_KO_WAKE ] = "army_wake_ko_02"; break;
	case 2:		m_animNames[ GAM_KO_WAKE ] = "army_wake_ko_03"; break;
	case 3:		m_animNames[ GAM_KO_WAKE ] = "army_wake_ko_04"; break;
	case 4:		m_animNames[ GAM_KO_WAKE ] = "army_wake_ko_05"; break;
	case 5:		m_animNames[ GAM_KO_WAKE ] = "army_wake_ko_06"; break;
	case 6:		m_animNames[ GAM_KO_WAKE ] = "army_wake_ko_07"; break;
	case 7:		m_animNames[ GAM_KO_WAKE ] = "army_ko_face_up"; break;
	case 8:		m_animNames[ GAM_KO_WAKE ] = "army_ko_face_up_2"; break;
	case 9:		m_animNames[ GAM_KO_WAKE ] = "army_ko_face_up_3"; break;
	default:	ntError_p( false, ("Bad Mod op\n") );
	}

	switch( (int)erandf(6.9999f) )
	{
	case 0:		m_animNames[ GAM_TAUNT ] = "army_aggtaunt_01"; break;
	case 1:		m_animNames[ GAM_TAUNT ] = "army_aggtaunt_02"; break;
	case 2:		m_animNames[ GAM_TAUNT ] = "army_aggtaunt_03"; break;
	case 3:		m_animNames[ GAM_TAUNT ] = "army_aggtaunt_04"; break;
	case 4:		m_animNames[ GAM_TAUNT ] = "army_stomp_01"; break;
	case 5:		m_animNames[ GAM_TAUNT ] = "army_stomp_02"; break;
	case 6:		m_animNames[ GAM_TAUNT ] = "army_stomp_03"; break;
	default:	ntError_p( false, ("Bad Mod op\n") );
	}

	switch( (int)erandf(2.9999f) )
	{
	case 0:		m_animNames[ GAM_BLOCK ] = "army_block_up"; break;
	case 1:		m_animNames[ GAM_BLOCK ] = "army_block_left"; break;
	case 2:		m_animNames[ GAM_BLOCK ] = "army_block_right"; break;
	default:	ntError_p( false, ("Bad Mod op\n") );
	}

	switch( (int)erandf(2.9999f) )
	{
	case 0:		m_animNames[ GAM_RECOIL ] = "army_recoil_up"; break;
	case 1:		m_animNames[ GAM_RECOIL ] = "army_recoil_left"; break;
	case 2:		m_animNames[ GAM_RECOIL ] = "army_recoil_right"; break;
	default:	ntError_p( false, ("Bad Mod op\n") );
	}
}

//**************************************************************************************
//	
//**************************************************************************************
void ArmyRenderable::InstallRenderable()
{
	ntError_p( m_RenderableComponent == NULL, ("We've already got a renderable component - about to leak memory.") );
	ntError_p( m_Hierarchy != NULL, ("There must be a hierarchy before creating an animator.") );

	static const bool bRenderOpaque		= true;
	static const bool bShadowRecieve	= true;
	static const bool bShadowCast		= true;

	m_RenderableComponent = NT_NEW CRenderableComponent( m_Hierarchy, bRenderOpaque, bShadowRecieve, bShadowCast );
}

//**************************************************************************************
//	
//**************************************************************************************
void ArmyRenderable::UninstallClump()
{
	if ( m_ClumpHeader )
	{
		CClumpLoader::Get().UnloadClump( const_cast< CClumpHeader * >( m_ClumpHeader ) );	// Bit dirty, but usage is const elsewhere in this class.
		m_ClumpHeader = NULL;
	}
}

//**************************************************************************************
//	
//**************************************************************************************
void ArmyRenderable::UninstallLOD()
{
	if (m_LODComponent)
	{
		NT_DELETE( m_LODComponent );
		m_LODComponent = NULL;
	}
}

//**************************************************************************************
//	
//**************************************************************************************
void ArmyRenderable::UninstallHierarchy()
{
	if ( m_Hierarchy != NULL )
	{
		if ( m_Hierarchy->GetRootTransform()->GetParent() != NULL )
		{
			m_Hierarchy->GetRootTransform()->RemoveFromParent();
		}

		CHierarchy::Destroy( m_Hierarchy );
		m_Hierarchy = NULL;
	}
}

//**************************************************************************************
//	
//**************************************************************************************
void ArmyRenderable::UninstallAnimator()
{
	if (m_Animator)
	{
		CAnimator::Destroy( m_Animator );
		m_Animator = NULL;
	}
}

//**************************************************************************************
//	
//**************************************************************************************
void ArmyRenderable::UninstallRenderable()
{
	NT_DELETE( m_RenderableComponent );
	m_RenderableComponent = NULL;
}

//**************************************************************************************
//	
//**************************************************************************************
void ArmyRenderable::Init( const ArmyRenderableDef &def )
{
	ntError_p( !def.m_ClumpName.IsNull(), ("You must specify a clump name.") );

	// Pass the GetName function (as a functor object) to the EntityAnimSet base.
	// The ownership issue of this functor is a bit messy - created here, but destroyed by EntityAnimSet. :/
	InstallGetName( NT_NEW SpecificFunctor< ArmyRenderable, ntstd::String, true >( this, &ArmyRenderable::GetName ) );

	InstallClump( def.m_ClumpName.GetString() );
	InstallHierarchy( def.m_ParentTransform );
	if( def.m_bAnimatable )
	{
		InstallAnimator( CHashedString(def.m_AnimContainer) );
	}
	InstallRenderable();
	if( def.m_bLodable )
	{
		InstallLOD( def.m_ClumpName.GetString() );
	}

}

ntstd::String ArmyRenderable::GetName() const
{
	return ntstd::String("ArmyRenderable");
}

//**************************************************************************************
//	
//**************************************************************************************
ArmyRenderable::ArmyRenderable	()
:	m_ClumpHeader				( NULL )
,	m_LODComponent				( NULL )
,	m_Hierarchy					( NULL )
,	m_Animator					( NULL )
,	m_RenderableComponent		( NULL )
,	m_bShadowsDisabled			( false )
,	m_pRealDudeEntity			( NULL )
,	m_iVisibleFrameCount		( s_iVisibleFrameLimit )
,	m_bLastKnownAIStateWasDead	( false )
,	m_bFlagBearer( false )
{
}

//**************************************************************************************
//	
//**************************************************************************************
ArmyRenderable::ArmyRenderable	( const ArmyRenderableDef &def )
:	m_ClumpHeader				( NULL )
,	m_LODComponent				( NULL )
,	m_Hierarchy					( NULL )
,	m_Animator					( NULL )
,	m_RenderableComponent		( NULL )
,	m_bShadowsDisabled			( false )
,	m_pRealDudeEntity			( NULL )
,	m_iVisibleFrameCount		( s_iVisibleFrameLimit )
,	m_bLastKnownAIStateWasDead	( false )
,	m_bFlagBearer( false )
{
	Init( def );
}

//**************************************************************************************
//	
//**************************************************************************************
ArmyRenderable::~ArmyRenderable()
{
	UninstallLOD();
	UninstallRenderable();
	UninstallAnimator();
	UninstallHierarchy();
	UninstallClump();
}

//**************************************************************************************
//	
//**************************************************************************************
bool ArmyRenderable::IsAnimPlaying( GruntAnimState animState )
{
	if (( m_lastAnim == animState ) && ( m_currAnim ))
		return m_currAnim->IsActive();
	return false;
}

//**************************************************************************************
//	
//**************************************************************************************
void ArmyRenderable::PlayAnimation( GruntAnimState animState )
{
	// already playing
	if( IsAnimPlaying(animState) )
		return;

	// nope, kill last one if present
	StopAnimation();

	// special case
	if( m_bFlagBearer )
	{
		if( animState == GAM_IDLE || animState == GAM_TAUNT )
		{
			m_currAnim = m_Animator->CreateAnimation( "army_flag_bearer_idle" );
			m_currAnim->SetFlags( ANIMF_LOOPING );
		} else if (animState == GAM_WALK || animState == GAM_RUN )
		{
			m_currAnim = m_Animator->CreateAnimation( "army_flag_bearer_walk" );
			m_currAnim->SetFlags( ANIMF_LOOPING );
		} else
		{
			m_currAnim = MakeAnimation( animState );
		}
	} else
	{
		// now create a new one
		m_currAnim = MakeAnimation( animState );
	}

	if( m_currAnim )
	{
		m_Animator->AddAnimation( m_currAnim );
		m_currAnim->SetPercentage( erandf(s_fMaxRandomDurationVariationAnim)  );
		if( animState == GAM_DIVE_LEFT || animState == GAM_DIVE_RIGHT )
		{
			m_currAnim->SetSpeed( 3.f );
		}
	}

	m_lastAnim = animState;
}

//**************************************************************************************
//	
//**************************************************************************************
void ArmyRenderable::StopAnimation()
{
	if (m_currAnim && m_currAnim->IsActive() )
		m_Animator->RemoveAnimation( m_currAnim ); 

	m_lastAnim = MAX_GRUNT_ANIM;
	m_currAnim = CAnimationPtr();
}

//**************************************************************************************
//	
//**************************************************************************************
void ArmyRenderable::UpdateState( float fTime )
{
	UNUSED( fTime );

	if( IsVisibleInternal() )
	{
		m_iVisibleFrameCount = s_iVisibleFrameLimit;
	} else
	{
		--m_iVisibleFrameCount;
	}
/*	if( m_AnimQueue.m_iAnimState != GAM_NONE )
	{
		m_AnimQueue.m_fSecondBlendTime -= fTime;
		if( m_AnimQueue.m_fSecondBlendTime < 0 )
		{
			FinishAnim( m_AnimQueue );
		}

		// TODO update the blend weights
	}*/
}

void ArmyRenderable::TurnIntoRealDude( AI* pEntity )
{
	ntError( pEntity );

	// for now just store. TODO sync animator and hierachy
	m_pRealDudeEntity = pEntity;
	m_pRealDudeEntity->SetArmyRenderable( this );

	// assume real_dudes start alive...
	m_bLastKnownAIStateWasDead = false;

	// now let the normal system do it
	DisableRendering();

}

void ArmyRenderable::TurnFromRealDude(bool bIsDead)
{
	ntError( m_pRealDudeEntity );
	m_vLastKnownAIPosition = m_pRealDudeEntity->GetPosition();
	m_vLastKnownAIOrientation = m_pRealDudeEntity->GetRotation();

	// TODO sync hierachy for dead body handling and do weapon stuff
	EnableRendering();

	m_bLastKnownAIStateWasDead = bIsDead;
	m_pRealDudeEntity = 0;
}

bool	ArmyRenderable::IsVisibleInternal() 
{ 
	if( m_LODComponent != NULL )
	{
		return m_LODComponent->Visible();
	} else
	{
		return true;
	}
}

const CPoint		ArmyRenderable::LastKnownAIPosition() const
{
	if( GetRealDudeEntity() )
	{
		return GetRealDudeEntity()->GetPosition();
	} else
	{
		return m_vLastKnownAIPosition;
	}
}

const CQuat		ArmyRenderable::LastKnownAIOrientation() const
{
	if( GetRealDudeEntity() )
	{
		return GetRealDudeEntity()->GetRotation();
	} else
	{
		return m_vLastKnownAIOrientation;
	}
}
