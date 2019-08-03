//--------------------------------------------------
//!
//!	\file game/entitykaisprite.cpp
//!	Definition of the kai sprite entity object
//!
//--------------------------------------------------

#include "objectdatabase/dataobject.h"
#include "game/luaattrtable.h"
#include "Physics/system.h"
#include "physics/collisionbitfield.h"
#include "messagehandler.h"
#include "core/exportstruct_anim.h"
#include "game/entitymanager.h"

#include "game/entitykaisprite.h"

void ForceLinkFunctionKaiSprite()
{
	ntPrintf("!ATTN! Calling ForceLinkFunctionKaiSprite() !ATTN!\n");
}


START_STD_INTERFACE(KaiSprite)
	DEFINE_INTERFACE_INHERITANCE(CEntity)
	COPY_INTERFACE_FROM(CEntity)

	OVERRIDE_DEFAULT(DefaultDynamics, "Animated")

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
//! Kai Sprite state machine.
//!
//--------------------------------------------------
STATEMACHINE(KAI_SPRITE_FSM, KaiSprite)
	KAI_SPRITE_FSM()
	{
		SET_INITIAL_STATE(DEFAULT);
	}

	STATE(DEFAULT)
		BEGIN_EVENTS
			ON_ENTER
			{
				//Each time we get into default state, we check to see if we have any anims left in our queue. If so, play one!
				if(ME->m_obAnimQueue.size() != 0)
				{
					//Play the first anim in the queue.
					ntstd::List<CHashedString>::iterator obIt = ME->m_obAnimQueue.begin();
					CHashedString obAnimName = *obIt;
					ME->m_obAnimToPlay = obAnimName;
					ME->m_bForceLoop = false;
					ME->m_fAnimPlaySpeed = 1.0f;
					if(!ntStr::IsNull(ME->m_obAnimToPlay))	//Shouldn't ever be null.
					{
						SET_STATE(PLAYANIM);
					}
				}
			}
			END_EVENT(true)

			EVENT(msg_sprite_queueanim)
			{
				int iNumAnimsBeforeAdding = ME->m_obAnimQueue.size();

				CHashedString obAnimName = msg.GetHashedString("AnimName");
				int iNumLoops = msg.GetInt("NumLoops");
				if(iNumLoops <= 0)
				{
					ntError_p(false, ("msg_sprite_queuenum used without passing in an NumLoops value!"));
				}

				if(!ntStr::IsNull(obAnimName))
				{
					for(int i = 0 ; i < iNumLoops ; i++)
					{
						ME->m_obAnimQueue.push_back(obAnimName);
					}
				}

				//If this is the only animation that has just been pushed on, then go into PLAYANIM state with that first anim.
				if(iNumAnimsBeforeAdding == 0)
				{
					CEntity* pSprite = (CEntity*)ME;
					pSprite->Show();	//So we can see!
					ME->m_obAnimToPlay = obAnimName;
					ME->m_bForceLoop = false;
					ME->m_fAnimPlaySpeed = 1.0f;
					if(!ntStr::IsNull(ME->m_obAnimToPlay))	//Shouldn't ever be null anyway.
					{
						SET_STATE(PLAYANIM);
					}
				}
			}
			END_EVENT(true)

			EVENT(msg_sprite_notifyoncomplete)
			{
				CEntity* obEntityToNotify = msg.GetEnt("EntityToNotify");
				ME->m_pobNotifyScriptEntity = obEntityToNotify;
				if(ME->m_pobNotifyScriptEntity != NULL)
				{
					ntPrintf("Sprite will now notify entity [%s] when all anims are finished\n", ME->m_pobNotifyScriptEntity->GetName().c_str());
					ME->m_bNotifyAllAnimsDone = true;
				}
				else
				{
					ntPrintf("WARNING: Passed in entity to notify was NULL, so no entity will be notified when all anims are complete\n");
				}
			}
			END_EVENT(true)
		END_EVENTS
	END_STATE

	STATE(PLAYANIM)
		BEGIN_EVENTS
			ON_ENTER
			{
				if (!ntStr::IsNull( ME->m_obAnimToPlay ) )
				{
					ntAssert_p( ME->GetAnimator(), ("KaiSprite %s without anim container\n", ntStr::GetString( ME->GetName() ) ) );
					ME->AnimPlay( ME->m_obAnimToPlay, ME->m_fAnimPlaySpeed, true, ME->m_bForceLoop );
					if (!ME->m_bForceLoop)
					{
						ME->Lua_AnimMessageOnCompletion( ME->m_obAnimToPlay );
					}
				}
			}
			END_EVENT(true)

			EVENT(msg_animdone)
			{
				//When an animation has completed, remove the first anim in our queue.
				ME->m_obAnimQueue.erase(ME->m_obAnimQueue.begin());

				//If we now have no items left in the list, then give feedback to our script entity saying so.
				if(ME->m_obAnimQueue.size() == 0)
				{
					if(ME->m_bNotifyAllAnimsDone)
					{
						//Notify m_pobNotifyScriptEntity entity with a set message it's waiting for.
						if(ME->m_pobNotifyScriptEntity && ME->m_pobNotifyScriptEntity->GetMessageHandler())
						{
							Message obAllAnimsDone(msg_animdone);
							ME->m_pobNotifyScriptEntity->GetMessageHandler()->QueueMessage(obAllAnimsDone);
						}
						ME->m_bNotifyAllAnimsDone = false;
					}
					//Hide! (taken back out at request of Stan).
//					CEntity* pSprite = (CEntity*)ME;
//					pSprite->Hide();
					SET_STATE(DEFAULT);
				}
				else
				{
					//If we have more anims, then just queue up the next one here (avoid the frame-delay going to default and back)
					//Play the first anim in the queue.
					ntstd::List<CHashedString>::iterator obIt = ME->m_obAnimQueue.begin();
					CHashedString obAnimName = *obIt;
					ME->m_obAnimToPlay = obAnimName;
					ME->m_bForceLoop = false;
					ME->m_fAnimPlaySpeed = 1.0f;
					if(!ntStr::IsNull(ME->m_obAnimToPlay))	//Shouldn't ever be null.
					{
						ntAssert_p( ME->GetAnimator(), ("KaiSprite %s without anim container\n", ntStr::GetString( ME->GetName() ) ) );
						ME->AnimPlay( ME->m_obAnimToPlay, ME->m_fAnimPlaySpeed, true, ME->m_bForceLoop );
						if (!ME->m_bForceLoop)
						{
							ME->Lua_AnimMessageOnCompletion( ME->m_obAnimToPlay );
						}
					}
				}
			}
			END_EVENT(true)

			EVENT(msg_anim_stop)
			{
				CAnimator* pobAnimator = ME->GetAnimator();
				if (pobAnimator)
				{
					pobAnimator->RemoveAnimation(ME->m_pobAnim);
				}
				//Clear the list of animations here if we've manually stopped the animations.
				ME->m_obAnimQueue.clear();
				//Also hide the sprite (taken out at request of Stan).
//				CEntity* pSprite = (CEntity*)ME;
//				pSprite->Hide();

				//Back to default state for us!
				SET_STATE(DEFAULT);
			}
			END_EVENT(true)

			EVENT(msg_sprite_queueanim)
			{
				CHashedString obAnimName = msg.GetHashedString("AnimName");
				int iNumLoops = msg.GetInt("NumLoops");
				if(iNumLoops <= 0)
				{
					ntError_p(false, ("msg_sprite_queuenum used without passing in an NumLoops value!"));
				}

				if(!ntStr::IsNull(obAnimName))
				{
					for(int i = 0 ; i < iNumLoops ; i++)
					{
						ME->m_obAnimQueue.push_back(obAnimName);
					}
				}
			}
			END_EVENT(true)

			EVENT(msg_sprite_notifyoncomplete)
			{
				CEntity* obEntityToNotify = msg.GetEnt("EntityToNotify");
				ME->m_pobNotifyScriptEntity = obEntityToNotify;
				if(ME->m_pobNotifyScriptEntity != NULL)
				{
					ntPrintf("Sprite will now notify entity [%s] when all anims are finished\n", ME->m_pobNotifyScriptEntity->GetName().c_str());
					ME->m_bNotifyAllAnimsDone = true;
				}
				else
				{
					ntPrintf("WARNING: Passed in entity to notify was NULL, so no entity will be notified when all anims are complete\n");
				}
			}
			END_EVENT(true)
		END_EVENTS
	END_STATE

END_STATEMACHINE // KAI_SPRITE_FSM


//--------------------------------------------------
//!
//!	KaiSprite::KaiSprite()
//!	Default constructor
//!
//--------------------------------------------------
KaiSprite::KaiSprite()
{
	m_eType = EntType_Static;
	//m_pobAnim = 0;
}


//--------------------------------------------------
//!
//!	KaiSprite::OnPostConstruct()
//!	Post Construct
//!
//--------------------------------------------------
void KaiSprite::OnPostConstruct()
{
	CEntity::OnPostConstruct();
	
	// Create components
	InstallMessageHandler();
	InstallDynamics();

	if ( !ntStr::IsNull( m_AnimationContainer ) )
		InstallAnimator(m_AnimationContainer);

	// Create and attach the statemachine
	KAI_SPRITE_FSM* pFSM = NT_NEW_CHUNK(Mem::MC_ENTITY) KAI_SPRITE_FSM();
	ATTACH_FSM(pFSM);
}


//--------------------------------------------------
//!
//!	KaiSprite::~KaiSprite()
//!	Default destructor
//!
//--------------------------------------------------
KaiSprite::~KaiSprite()
{
}


//--------------------------------------------------
//!
//!	KaiSprite::AnimPlay(CHashedString pcAnim, float fSpeed, bool bLocomoting, bool bLooping)
//!
//--------------------------------------------------
void KaiSprite::AnimPlay (CHashedString pcAnim, float fSpeed, bool bLocomoting, bool bLooping)
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
	}
}
