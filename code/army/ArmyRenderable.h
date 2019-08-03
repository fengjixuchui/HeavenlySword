/***************************************************************************************************
*
*	DESCRIPTION		Class to represent a renderable, animated army object.
*
*	NOTES
*
***************************************************************************************************/

#ifndef ARMYRENDERABLE_H_
#define ARMYRENDERABLE_H_

//**************************************************************************************
//	Includes files.
//**************************************************************************************
#include "game/EntityAnimSet.h"
#include "anim/Animator.h"
#include "anim/animationheader.h"
#include "anim/Hierarchy.h"
#include "army/army_ppu_spu.h"
#include "army/grunt.h"

//**************************************************************************************
//	Forward declarations.
//**************************************************************************************
class	CRenderableComponent;
class	CClumpHeader;
class	CLODComponent;
class	CKeyString;
class	ArmyRenderable;
class	AI;
class	AnimatorBatchUpdate;

//**************************************************************************************
//	Typedefs.
//**************************************************************************************

//**************************************************************************************
//	Structure to define how to create an ArmyRenderable.
//**************************************************************************************
struct ArmyRenderableDef
{
	CKeyString		m_ClumpName;
	CHashedString	m_AnimContainer;
	bool			m_bAnimatable;
	bool			m_bLodable;
	Transform *		m_ParentTransform;

	ArmyRenderableDef( const CKeyString &clump, const CHashedString &animContainer, bool bAnimatable = true, bool bLodable = true, Transform *parent_transform = NULL )
	:	m_ClumpName			( clump )
	,	m_AnimContainer		( animContainer )
	,	m_bAnimatable		( bAnimatable )
	,	m_bLodable			( bLodable )
	,	m_ParentTransform	( parent_transform )
	{}
};

//**************************************************************************************
//	Class to represent a renderable, animated army object.
//**************************************************************************************
class ArmyRenderable : private EntityAnimSet
{
	public:
		//
		//	Update.
		//
		void			Update			( float time_step, AnimatorBatchUpdate *batch_updater );
		void			UpdatePartTwo();
		void			UpdateState( float fTime );

	private:
		CAnimationPtr	MakeAnimation( GruntAnimState eAnimType );
		bool			IsVisibleInternal();

	public:
		void			StopAnimating	()									
		{ 
			ntError( m_Animator != NULL ); 
			m_Animator->ClearAnimWeights(); 
		}
		CHierarchy *	GetHierarchy	()									
		{ 
			return m_Hierarchy; 
		}
		void			DetachAnimator	();

		const CAnimator *		GetAnimator		()		const	{ return m_Animator; }
		CRenderableComponent *	GetRenderable	()		const	{ return m_RenderableComponent; }

		void SetFlagBearer( bool bEnable )
		{
			m_bFlagBearer = bEnable;
		}

	public:
		//
		//	Transform retrieval.
		//
		Transform *		GetRootTransform()								{ ntError( m_Hierarchy != NULL ); return m_Hierarchy->GetRootTransform(); }
		const Transform *	GetRootTransform()						const	{ ntError( m_Hierarchy != NULL ); return m_Hierarchy->GetRootTransform(); }
		Transform *		GetTransform	( const char *name )			{ ntError( m_Hierarchy != NULL ); return m_Hierarchy->GetTransform( name ); }
		const Transform *	GetTransform	( const char *name )	const	{ ntError( m_Hierarchy != NULL ); return m_Hierarchy->GetTransform( name ); }

	public:
		void			TurnIntoRealDude( AI* pEntity );
		void			TurnFromRealDude( bool bIsDead );
		bool			IsVisible()
		{
			if( m_iVisibleFrameCount < 0 )
			{
				return false;
			} else
			{
				return true;
			}
		}
		static float s_fMaxRandomDurationVariationAnim;
		static int		s_iVisibleFrameLimit;

	public:
		//
		//	Enable/Disable rendering.
		//
		void				EnableRendering	();
		void				DisableRendering();
		bool				IsRenderingDisabled() const;

		void				DisableShadows();
		void				EnableShadows();
		bool				IsShadowsDisabled() const { return m_bShadowsDisabled; }

		bool				LastKnownAIStateWasDead() const
		{
			return m_bLastKnownAIStateWasDead;
		}

		const CPoint		LastKnownAIPosition() const;

		const CQuat			LastKnownAIOrientation() const;
	public:
		//
		//	Install our GetName function.
		//
		void				InstallGetName	( StringFunctor *get_name_func )
		{
			EntityAnimSet::InstallGetName( get_name_func );
		}
		ntstd::String GetName() const;

	public:
		//
		//	Ctor, dtor.
		//
		ArmyRenderable( const ArmyRenderableDef &def );
		ArmyRenderable();
		~ArmyRenderable();

		// real ctor
		void Init( const ArmyRenderableDef &def );

		// animation control and status
		void PlayAnimation( GruntAnimState animState );
		void StopAnimation();
		bool IsAnimPlaying( GruntAnimState animState );

		AI*			GetRealDudeEntity() const
		{
			return m_pRealDudeEntity;
		}

	private:
		//
		//	Prevent copying.
		//
		ArmyRenderable( const ArmyRenderable & )				NOT_IMPLEMENTED;
		ArmyRenderable &operator = ( const ArmyRenderable & )	NOT_IMPLEMENTED;

	private:
		//
		//	Helper functions for installing/uninstalling components.
		//
		void		InstallClump			( const char* clump_name );
		void		InstallLOD				( const char* clump_name );
		void		InstallHierarchy		( Transform *parent_transform );
		void		InstallAnimator			( const CHashedString& animContainerName );
		void		InstallRenderable		();

		void		UninstallClump			();
		void		UninstallLOD			();
		void		UninstallHierarchy		();
		void		UninstallAnimator		();
		void		UninstallRenderable		();

	private:
		//
		//	Aggregated members.
		//
		const CClumpHeader *		m_ClumpHeader;				// Pointer to clump header
		CLODComponent *				m_LODComponent;				// The level of detail component.
		CHierarchy *				m_Hierarchy;				// The animation hierarchy.
		CAnimator *					m_Animator;					// The animator component.
		CRenderableComponent *		m_RenderableComponent;		// The renderable component.
		bool						m_bShadowsDisabled;
		AI*							m_pRealDudeEntity;
		int							m_iVisibleFrameCount;

		CHashedString				m_animNames[ MAX_GRUNT_ANIM ];
		CAnimationPtr				m_currAnim;
		GruntAnimState				m_lastAnim;

		bool				m_bLastKnownAIStateWasDead;
		CPoint				m_vLastKnownAIPosition;
		CQuat				m_vLastKnownAIOrientation;

		bool				m_bFlagBearer;
};

#endif	// !ARMYRENDERABLE_H_





