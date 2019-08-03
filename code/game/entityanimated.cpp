//--------------------------------------------------
//!
//!	\file game/entityanimated.cpp
//!	Definition of the Animated entity object
//!
//--------------------------------------------------

#include "objectdatabase/dataobject.h"
#include "game/luaattrtable.h"
#include "Physics/system.h"
#include "physics/collisionbitfield.h"
#include "messagehandler.h"
#include "core/exportstruct_anim.h"

#include "game/entityanimated.h"

void ForceLinkFunctionAnimated()
{
	ntPrintf("!ATTN! Calling ForceLinkFunctionAnimated() !ATTN!\n");
}


START_STD_INTERFACE(Object_Animated)
	DEFINE_INTERFACE_INHERITANCE(CEntity)
	COPY_INTERFACE_FROM(CEntity)

	OVERRIDE_DEFAULT(DefaultDynamics, "Animated")

	PUBLISH_VAR_WITH_DEFAULT_AS( m_bPlayAnimOnStart, true, PlayAnimOnStart)
	PUBLISH_VAR_WITH_DEFAULT_AS( m_obAnimToPlay, "Default", AnimToPlay)
	PUBLISH_VAR_WITH_DEFAULT_AS( m_fAnimPlaySpeed, 1.0, AnimPlaySpeed)
	PUBLISH_VAR_WITH_DEFAULT_AS( m_bForceLoop, true, ForceLoop)

	PUBLISH_VAR_AS(m_Description, Description)
	PUBLISH_VAR_AS(m_InitialState, InitialState)
	PUBLISH_VAR_AS(m_AnimationContainer, AnimationContainer)

	DECLARE_POSTCONSTRUCT_CALLBACK( OnPostConstruct )
END_STD_INTERFACE


//--------------------------------------------------
//!
//! Interactable Rigid Body State Machine
//!
//--------------------------------------------------
STATEMACHINE(OBJECT_ANIMATED_FSM, Object_Animated)
	OBJECT_ANIMATED_FSM(bool bPlayAnimOnStart)
	{
		if ( bPlayAnimOnStart )
			// We want to play the assigned anim immediately
			SET_INITIAL_STATE(PLAYANIM);
		else
			SET_INITIAL_STATE(DEFAULT);
	}

	STATE(DEFAULT)
		BEGIN_EVENTS
			EVENT(PlayAnim)
				// Only play the anim if it hasn't already been set to play on startup
				if (!ME->m_bPlayAnimOnStart)
					SET_STATE(PLAYANIM);
			END_EVENT(true)

			EVENT(msg_anim_play)
				if (msg.IsHash("Animation"))
					ME->m_obAnimToPlay = msg.GetHashedString("Animation");
	
				if (msg.IsBool("Looping"))
					ME->m_bForceLoop = msg.GetBool("Looping");

				if (msg.IsFloat("Speed"))
					ME->m_fAnimPlaySpeed = msg.GetFloat("Speed");

				// Only play the anim if it hasn't already been set to play on startup
				if (!ME->m_bPlayAnimOnStart)
					SET_STATE(PLAYANIM);
			END_EVENT(true)

			EVENT(msg_animated_settoend)
			{
				ntPrintf("msg_animated_settoend recieved, setting anim to the end\n");

				if (msg.IsHash("Animation"))
					ME->m_obAnimToPlay = msg.GetHashedString("Animation");

				bool bLocomoting = false;
				if (msg.IsBool("Locomoting"))
					bLocomoting = msg.GetBool("Locomoting");

				ntError_p(!ntStr::IsNull(ME->m_obAnimToPlay), ("msg_animated_settoend called with no 'Animation' parameter (the anim to jump to the end of"));
				ntError_p(ME->GetAnimator(), ("Object_Animated %s without anim container\n", ntStr::GetString(ME->GetName())));
				ME->AnimPlay(ME->m_obAnimToPlay, 1.0f, bLocomoting, false, 1.0f);
			}

			END_EVENT(true);
			
		END_EVENTS
	END_STATE

	STATE(PLAYANIM)
		BEGIN_EVENTS
			ON_ENTER
				if (!ntStr::IsNull( ME->m_obAnimToPlay ) )
				{
					ntAssert_p( ME->GetAnimator(), ("Object_Animated %s without anim container\n", ntStr::GetString( ME->GetName() ) ) );
					ME->AnimPlay( ME->m_obAnimToPlay, ME->m_fAnimPlaySpeed, true, ME->m_bForceLoop );
					if (!ME->m_bForceLoop)
						ME->Lua_AnimMessageOnCompletion( ME->m_obAnimToPlay );
				}
			END_EVENT(true)

			EVENT(msg_animdone)
				SET_STATE(DEFAULT);
			END_EVENT(true)

			EVENT(msg_anim_stop)
			{
				CAnimator* pobAnimator = ME->GetAnimator();
				if (pobAnimator)
				{
					pobAnimator->RemoveAnimation(ME->m_pobAnim);
				}
				SET_STATE(DEFAULT);
			}
			END_EVENT(true)

			EVENT(msg_anim_play)
			{
				CAnimator* pobAnimator = ME->GetAnimator();
				if (pobAnimator)
				{
					pobAnimator->RemoveAnimation(ME->m_pobAnim);
				}

				if (msg.IsHash("Animation"))
					ME->m_obAnimToPlay = msg.GetHashedString("Animation");
	
				if (msg.IsBool("Looping"))
					ME->m_bForceLoop = msg.GetBool("Looping");

				if (msg.IsFloat("Speed"))
					ME->m_fAnimPlaySpeed = msg.GetFloat("Speed");

				if (!ntStr::IsNull( ME->m_obAnimToPlay ) )
				{
					ntAssert_p( ME->GetAnimator(), ("Object_Animated %s without anim container\n", ntStr::GetString( ME->GetName() ) ) );
					ME->AnimPlay( ME->m_obAnimToPlay, ME->m_fAnimPlaySpeed, true, ME->m_bForceLoop );
					if (!ME->m_bForceLoop)
						ME->Lua_AnimMessageOnCompletion( ME->m_obAnimToPlay );
				}
			}
			END_EVENT(true)

			EVENT(msg_anim_speed)
			{
				if (msg.IsNumber("Speed"))
					ME->m_fAnimPlaySpeed = msg.GetFloat("Speed");

				ME->m_pobAnim->SetSpeed(ME->m_fAnimPlaySpeed);
			}
			END_EVENT(true)

			EVENT(msg_anim_looping)
			{
				if (msg.IsBool("Looping"))
					ME->m_bForceLoop = msg.GetBool("Looping");

				int iFlags=ME->m_pobAnim->GetFlags();

				
				if (ME->m_bForceLoop)
				{
					iFlags|=ANIMF_LOOPING;
				}
				else
				{
					iFlags &= !ANIMF_LOOPING;
				}

				ME->m_pobAnim->SetFlags(iFlags);

			}
			END_EVENT(true)

			EVENT(msg_animated_settoend)
			{
				ntPrintf("msg_animated_settoend recieved, setting anim to the end\n");

				if (msg.IsHash("Animation"))
					ME->m_obAnimToPlay = msg.GetHashedString("Animation");

				bool bLocomoting = false;
				if (msg.IsBool("Locomoting"))
					bLocomoting = msg.GetBool("Locomoting");

				ntError_p(!ntStr::IsNull(ME->m_obAnimToPlay), ("msg_animated_settoend called with no 'Animation' parameter (the anim to jump to the end of"));
				ntError_p(ME->GetAnimator(), ("Object_Animated %s without anim container\n", ntStr::GetString(ME->GetName())));
				ME->AnimPlay(ME->m_obAnimToPlay, 1.0f, bLocomoting, false, 1.0f);
				ME->Lua_AnimMessageOnCompletion( ME->m_obAnimToPlay );	//So that it drops back to DEFAULT afterwards.
			}
			END_EVENT(true);

		END_EVENTS
	END_STATE

END_STATEMACHINE // OBJECT_ANIMATED_FSM


//--------------------------------------------------
//!
//!	Object_Animated::Object_Animated()
//!	Default constructor
//!
//--------------------------------------------------
Object_Animated::Object_Animated()
{
	m_eType = EntType_Static;
	//m_pobAnim = 0;
}

//--------------------------------------------------
//!
//!	Object_Animated::OnPostConstruct()
//!	Post Construct
//!
//--------------------------------------------------
void Object_Animated::OnPostConstruct()
{
	CEntity::OnPostConstruct();
	
	// Create components
	InstallMessageHandler();
	InstallDynamics();

	if ( !ntStr::IsNull( m_AnimationContainer ) )
		InstallAnimator(m_AnimationContainer);
}

//--------------------------------------------------
//!
//!	Object_Animated::OnLevelStart()
//!	Called for each ent on level startup
//!
//--------------------------------------------------
void Object_Animated::OnLevelStart()
{
	// Create and attach the statemachine. Must be done AFTER anim containers fixed up by area system
	// i.e. after XML serialisation. OR this shouldnt play an animation
	OBJECT_ANIMATED_FSM* pFSM = NT_NEW_CHUNK(Mem::MC_ENTITY) OBJECT_ANIMATED_FSM(m_bPlayAnimOnStart);
	ATTACH_FSM(pFSM);
}

//--------------------------------------------------
//!
//!	Object_Animated::~Object_Animated()
//!	Default destructor
//!
//--------------------------------------------------
Object_Animated::~Object_Animated()
{
}

//--------------------------------------------------
//!
//!	Object_Animated::AnimPlay(CHashedString pcAnim, float fSpeed, bool bLocomoting, bool bLooping)
//!
//--------------------------------------------------
void Object_Animated::AnimPlay (CHashedString pcAnim, float fSpeed, bool bLocomoting, bool bLooping, float fPercentage)
{
	// Parameters:
	// Name - Short name for the anim
	// Speed - Speed multiplier for the anim, default is 1.0
	// Locomoting - Is the anim playing relative to its current world position? If nil is specified, the default for the anim is applied.
	// Looping - Does this anim loop? If nil is specified, the default for the anim is applied.

	ntAssert( ! ntStr::IsNull(pcAnim) );

	CAnimator* pobAnimator = GetAnimator();
	ntAssert( pobAnimator );

	m_pobAnim = pobAnimator->CreateAnimation( pcAnim );

	if (m_pobAnim!=0)
	{
		m_pobAnim->SetSpeed(fSpeed);

		int iFlags=0;

		if (bLocomoting)
		{
			iFlags|=ANIMF_LOCOMOTING;
		}

		if (bLooping)
		{
			iFlags|=ANIMF_LOOPING;
		}
	
		m_pobAnim->SetFlagBits( iFlags );

		pobAnimator->AddAnimation( m_pobAnim );
		if(fPercentage != 0.0f)
		{
			m_pobAnim->SetPercentage(fPercentage);
		}
	}
}

