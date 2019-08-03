

#include "blendshapes/anim/bsanimator.h"
#include "blendshapes/anim/blendshapes_anim.h"
#include "blendshapes/blendshapes.h"
#include "blendshapes/blendedmeshinstance.h"
#include "blendshapes/blendshapes_constants.h"
#include "blendshapes/anim/bsanimcontainer.h"
#include "game/entity.h"
#include "game/entity.inl"
#include "anim/hierarchy.h"
#include "game/luaglobal.h"


//--------------------------------------------------
//
//					Lua Interface
//
//--------------------------------------------------
LUA_EXPOSED_START(BSAnimator)
	LUA_EXPOSED_METHOD( Play, Lua_Play, "plays a bsanimation", "string shortname, number speed, bool looping", "" )
	LUA_EXPOSED_METHOD( PlayAndHold, Lua_PlayAndHold, "plays a bsanimation and holds the position of the last keyframe", "string shortname, number speed", "" )
	LUA_EXPOSED_METHOD( Stop, Lua_Stop, "stops a bsanimation", "string shortname", "" )
	LUA_EXPOSED_METHOD( StopAll, Lua_StopAll, "stops all playing animations", "", "" )
	LUA_EXPOSED_METHOD( StopFinished, Lua_StopAllFinished, "stops all finished animations", "", "" )
LUA_EXPOSED_END(BSAnimator)

//--------------------------------------------------
//
//					Some Helpers
//
//--------------------------------------------------

void BlendBSAnimTargetWeights( BSSetPtr_t pBSSet, BSAnimation* pBSAnim, float scaling );

inline bool IsCompatible( BSSetPtr_t pBSSet, const CEntity* pEntity )
{
	return ( pBSSet->GetClumpHierarchyKey() == pEntity->GetHierarchy()->GetHierarchyKey() );
}

inline bool IsCompatible( BSSetPtr_t pBSSet , BSAnimation* pBSAnim )
{
	return ( pBSSet->GetNameHash() == pBSAnim->GetBlendShapesNameHash() );
}

inline bool IsCompatible( BSSetPtr_t pBSSet, BSAnimHeaderPtr_t pBSAnimHeader )
{
	return pBSSet->GetNameHash() == pBSAnimHeader->m_blendShapesNameHash;
}


struct HashCmp
{
	uint32_t m_hash;
	HashCmp( uint32_t nameHash ) : m_hash(nameHash) {}
	bool operator()( const BSAnimation* pBSAnim ) const
	{
		return ( pBSAnim->GetNameHash() == m_hash );
	}
};



inline float weight_blend_absmax( float a, float b )
{
	return abs(a) > abs(b) ? a : b;
}


inline float weight_blend_avg( float a, float b )
{
	return 0.5f * ( a + b );
}

//--------------------------------------------------
//
//					Lua stuff
//
//--------------------------------------------------

bool BSAnimator::Lua_Play( const char* pcShortName, float fSpeed, bool bLooping )
{
	ntAssert( pcShortName );
	return Play( pcShortName, fSpeed, bLooping ? kBSAF_Looping : 0 );
}


bool BSAnimator::Lua_PlayAndHold( const char* pcShortName, float fSpeed )
{
	ntAssert( pcShortName );
	return Play( pcShortName, fSpeed, kBSAF_NoDelete );
}


void BSAnimator::Lua_Stop( const char* pcShortName )
{
	ntAssert( pcShortName );
	Stop( pcShortName );
}


void BSAnimator::Lua_StopAll( void )
{
	StopAll();
}	


void BSAnimator::Lua_StopAllFinished( void )
{
	StopAllFinished();
}


//--------------------------------------------------
//
//				BSAnimator Stuff 
//
//--------------------------------------------------

//! number of bsanims playable at the same time
static const unsigned int BS_MAX_BSANIMS = 5;


BSAnimator::BSAnimator( CEntity* pEntity, BSSetPtr_t pBlendShapes, BSAnimShortcutContainer* pBSAnimContainer )
:	m_anims(),
	m_bEnabled( true ),
	m_pEntity( pEntity ),
	m_pBlendShapes( pBlendShapes ),
	m_pobBSAnimContainer( pBSAnimContainer )
{
	ATTACH_LUA_INTERFACE(BSAnimator);
	ntError( pEntity );
	ntError( pBlendShapes );
	ntError_p( IsCompatible( pBlendShapes, pEntity ), ("BlendShapes set is not compatible with entity %s. Hierarchy keys are different!\n", ntStr::GetString(pBlendShapes->GetEntity()->GetName()) ) );
}


BSAnimator::~BSAnimator( void )
{
	//Reset();
	StopAll();
}


//! resets all managed animations to zero
void BSAnimator::Reset( void )
{
	for ( BSAnimationPtrCollection_t::iterator animIt = m_anims.begin(); animIt != m_anims.end(); ++animIt )
	{
		animIt->second->Reset();
	}
}


void BSAnimator::StopAll( void ) 
{
	for ( BSAnimationPtrCollection_t::iterator animIt = m_anims.begin(); animIt != m_anims.end(); ++animIt )
	{
		NT_DELETE_CHUNK( Mem::MC_PROCEDURAL, animIt->second );
	}

	m_anims.clear();
}


void BSAnimator::StopAllFinished( void )
{
	for ( BSAnimationPtrCollection_t::iterator animIt = m_anims.begin(); animIt != m_anims.end();  )
	{
		if ( animIt->second->IsFinished() )
		{
			animIt = RemoveFromPlayingBSAnimations( animIt );
		}
		else
		{
			++animIt;
		}
	}
}


bool BSAnimator::IsPlaying( const CHashedString& shortName ) 
{
	return m_anims.find(shortName) != m_anims.end();
}


bool BSAnimator::Play( const CHashedString& shortName, float fSpeed, int iFlags )
{
	user_warn_p( IsEnabled(), (("BSANIMATION - %s BSAnimator is disabled. Cannot play %s"), m_pEntity->GetName().c_str(), ntStr::GetString(shortName)) );
	if ( IsEnabled() && m_pBlendShapes )
	{
		BSAnimation* pBSAnim = GetPlayingBSAnimation( shortName );
		if ( pBSAnim )
		{
			//user_warn_msg(("%s is already playing! I'll reset it instead\n", ntStr::GetString(shortName)));
			pBSAnim->Reset();
			pBSAnim->SetFlags( iFlags );
			pBSAnim->SetSpeed( fSpeed );
			return true;
		}
		else if ( m_anims.size() < BS_MAX_BSANIMS )
		{
			BSAnimHeaderPtr_t pHeader = m_pobBSAnimContainer->GetBSAnimHeader( shortName );
			if ( pHeader )
			{
				if ( IsCompatible(m_pBlendShapes, pHeader) )
				{
					BSAnimation* pBSAnim = NT_NEW_CHUNK (Mem::MC_PROCEDURAL) BSAnimation( pHeader );
					pBSAnim->SetFlags( iFlags );
					pBSAnim->SetSpeed( fSpeed );
					m_anims[shortName] = pBSAnim;
					return true;
				}
				else 
				{
					user_warn_msg(("BSANIMATION - Couldn't play %s. BSAnim header clumpName hash mismatch( bsclump: %i, bsanim: %i ). Ignoring...\n",ntStr::GetString(shortName), m_pBlendShapes->GetClumpNameHash(), pHeader->m_nameHash));
				}
			}
			else
			{
				user_warn_msg(("BSANIMATION - BSAnim header shortcut %s couldn't be found. Ignoring... \n", ntStr::GetString(shortName)));
			}
		}
	}
	else
	{
		user_warn_msg(("BSANIMATION - Can't play %s without a valid blendshapes set\n", ntStr::GetString(shortName)));
	}
	
	return false;
}

void BSAnimator::Stop( const CHashedString& shortName )
{
	BSAnimationPtrCollection_t::iterator it = m_anims.find( shortName );
	if ( it != m_anims.end() )
	{
		RemoveFromPlayingBSAnimations( it );
	}
	else
	{
		user_warn_msg( ("%s was not playing in the first place!\n",ntStr::GetString(shortName)) );
	}
}



inline void BlendBSTargetWeight( BSSetPtr_t pSet, u_int targetIndex, float targetWeight, float blendFactor )
{
	const float prevWeight = pSet->GetTargetWeightByIndex( targetIndex );
	pSet->SetTargetWeightByIndex( targetIndex, prevWeight + blendFactor * targetWeight );
}

 inline void BlendWrinkleAreaWeight( BSSetPtr_t pSet, u_int areaIndex, float areaWeight, float blendFactor )
{
	const float prevWeight = pSet->GetWrinkleAreaWeight( areaIndex );
	pSet->SetWrinkleAreaWeight( areaIndex, prevWeight + blendFactor * areaWeight );
}


void BSAnimator::Update( float timeStep )
{
	//! early out if disabled or no valid bsset present
	if ( !(IsEnabled() && m_pBlendShapes) )
		return;

	//! reset local blendshape target weights 
	m_pBlendShapes->ResetWeights();

	for ( BSAnimationPtrCollection_t::iterator animIt = m_anims.begin(); animIt != m_anims.end(); ++animIt )
	{
		animIt->second->Update( timeStep );
		BlendBSAnimTargetWeights( m_pBlendShapes, animIt->second, 1.0f );
	}

	// remove anims marked for deletion
	StopAllFinished();
}

void BSAnimator::DebugPrint( void ) const
{
#ifndef _RELEASE

	ntPrintf( "BSAnimator debug print:\n" );

	//for ( BSAnimationPtrCollection_t::const_iterator animIt = m_anims.begin(); animIt != m_anims.end(); ++animIt )
	//{
	//	ntPrintf("    %-24s			Time=%0.3f/%0.3f (%0.2f)  Weight=%0.3f \n",
	//		"",
	//		(*animIt)->GetTime(),
	//		(*animIt)->GetDuration(),
	//		(*animIt)->GetTime() / (*animIt)->GetDuration(),
	//		(*animIt)->GetBlendFactor() );
	//		 
	//}
#endif //_RELEASE
}


inline void BSAnimator::UpdateBSAnims( float timeStep )
{
	for ( BSAnimationPtrCollection_t::iterator animIt = m_anims.begin(); animIt != m_anims.end(); ++animIt )
		animIt->second->Update( timeStep );
}

BSAnimation* BSAnimator::GetPlayingBSAnimation( const CHashedString& shortName )
{
	ntError( m_pobBSAnimContainer );
	BSAnimationPtrCollection_t::iterator it = m_anims.find( shortName );
	return it == m_anims.end() ? 0 : it->second;
}

BSAnimator::BSAnimationPtrCollection_t::iterator BSAnimator::RemoveFromPlayingBSAnimations( BSAnimationPtrCollection_t::iterator& it )
{
	NT_DELETE_CHUNK( Mem::MC_PROCEDURAL, it->second );
	return  m_anims.erase( it );
}

//--------------------------------------------------
//
//				Helpers Definitions
//
//--------------------------------------------------
//--------------------------------------------------
//!
//! note that we used the MAX of the current and new weights
//! this is because the blendshape uses additive morphs and this
//! makes a bit more sense when blending shapes that affect the same areas
//! this way, if you have a bsanim that affects, say, the left eyebrow shape and another
//! bsanim that affects the left AND right eyebrows, the resulting blend 
//! keeps the left one where it was and just raises the right one to its final position
//! note that this only applies to bsanims that affect the same bstargets
//! does this make sense at all? Well apprently not. Since now we support negative weights
//! again I've changed it to the absolute max. This however, looks suspicious to me
//!
//!
//!	\param pBSSet the blendshape set to update
//! \param pBSAnim the animation instance to get the weights from
//! \param scaling additional scaling. Currently not used anywhere but you never know...
//!
//--------------------------------------------------
void BlendBSAnimTargetWeights( BSSetPtr_t pBSSet, BSAnimation* pBSAnim, float scaling )
{
	ntAssert( IsCompatible( pBSSet, pBSAnim ) );

	for ( uint32_t iChannel = 0; iChannel < pBSAnim->GetNumOfChannels(); ++iChannel )
	{
		const int targetIndex = pBSAnim->GetChannelTargetIndex( iChannel );
		ntAssert_p( targetIndex >= 0, ("bsanim channel target index is invalid") );

		const float targetWeight = pBSAnim->GetTargetWeightByIndex( iChannel );
		const float animBlendFactor = pBSAnim->GetBlendFactor();
		const float newWeight = targetWeight * animBlendFactor * scaling;

		switch ( pBSAnim->GetChannelType( iChannel ) )
		{
			case kBSAC_BSTargetWeight: 
			{
				const float currentTotalWeight = pBSSet->GetTargetWeightByIndex( targetIndex );												
				//pBSSet->SetTargetWeightByIndex( targetIndex, ntstd::Max(newWeight,currentTotalWeight) );
				pBSSet->SetTargetWeightByIndex( targetIndex, weight_blend_absmax(currentTotalWeight,newWeight) );
				break;
			}
			case kBSAC_WrinkleWeight:
			{
				const float currentTotalWeight = pBSSet->GetWrinkleAreaWeight( targetIndex );
				//pBSSet->SetWrinkleAreaWeight( targetIndex, ntstd::Max(newWeight,currentTotalWeight) );
				pBSSet->SetWrinkleAreaWeight( targetIndex, weight_blend_absmax(currentTotalWeight,newWeight) );
				break;
			}
			case kBSAC_Invalid:
			{
				ntError_p( false, ("Invalid BSAnimChannel type. This should be detected in the Finaliser!!!\n") );
				break;
			}
		}
	}
}


//eof

