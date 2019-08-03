//------------------------------------------------------------------------------------------
//!
//!	\file movement_lua.cpp
//!
//------------------------------------------------------------------------------------------

#include "game/awareness_lua.h"
#include "game/awareness.h"
#include "game/luaglobal.h"
#include "game/entity.h"
#include "game/entity_lua.h"
#include "game/entity.inl"
#include "game/entityai.h"
#include "game/entityinfo.h"
#include "game/entitymanager.h"
#include "game/inputcomponent.h" // Running target check
#include "game/query.h"
#include "game/movementcontrollerinterface.h"
#include "game/renderablecomponent.h"
#include "game/aicomponent.h"

#include "physics/system.h"

#include "gfx/renderable.h"

#include "core/boundingvolumes.h"

#include "anim/hierarchy.h"

#include "interactioncomponent.h"

#include "luaattrtable.h"
#include "lua\ninjalua.h"
#include "luaexptypes.h"


//#define _AWAREBINDINGS_DEBUG

//------------------------------------------------------------------------------------------
// CAwareness - Lua Interface
//------------------------------------------------------------------------------------------
LUA_EXPOSED_START(AwarenessComponent)
	
	// Expose Awareness_Lua Methods
	LUA_EXPOSED_METHOD(FindInteractionTarget,		Lua_FindInteractionTarget, "Find the most appropriate interaction target.", "", "")
	LUA_EXPOSED_METHOD(FindNamedInteractionTarget,	Lua_FindNamedInteractionTarget, "Find the most appropriate interaction target with a given substring", "substring", "part of object name, e.g. 'crossbow'")
LUA_EXPOSED_END(AwarenessComponent)


//-------------------------------------------------------------------------------------------------
// BINDFUNC:	entity FindInteractionTarget()
// DESCRIPTION:	Find the most appropriate interaction target.
//-------------------------------------------------------------------------------------------------
CEntity* Awareness_Lua::Lua_FindInteractionTarget(	int iCharacterType	)
{
	return FindInteractionTarget(iCharacterType).m_pobInteractingEnt;
}



CInteractionTarget Awareness_Lua::FindInteractionTarget( int iCharacterType )
{
	CInteractionTarget obTargetResult;
	obTargetResult.m_pobInteractingEnt = 0;

	CEntity*	pobThis = ((AwarenessComponent*)this)->m_pobParent;

#ifdef _AWAREBINDINGS_DEBUG
	ntPrintf("Entity %s - Find interaction target\n", pobThis->GetName().c_str() );
#endif
	
	if((pobThis->IsAI() || pobThis->GetInputComponent()) && pobThis->GetAwarenessComponent())
	{
		bool bDirectionHeld = false;

		if((pobThis->IsAI() && ((AI*)pobThis)->GetAIComponent() && ((AI*)pobThis)->GetAIComponent()->GetMovementMagnitude() > 0.3f ) || 
		   (pobThis->GetInputComponent() && pobThis->GetInputComponent()->IsDirectionHeld()))
		{
			bDirectionHeld = true;
		}

		// Get position and targeting direction of this entity
		// ----------------------------------------------------------------------
		CPoint obPosition(pobThis->GetHierarchy()->GetRootTransform()->GetWorldMatrix().GetTranslation());
		CDirection obDirection;

		// If this entity is an AI or the analog stick isn't held, then use their character matrix
		if( pobThis->IsAI() || !pobThis->GetInputComponent()->IsDirectionHeld() )
		{
			obDirection=pobThis->GetHierarchy()->GetRootTransform()->GetWorldMatrix().GetZAxis();
		}
		else // Otherwise use pad direction
		{
			obDirection=pobThis->GetInputComponent()->GetInputDir();
		}

		const CDirection obPlaneNormalOfApproach(pobThis->GetHierarchy()->GetRootTransform()->GetWorldMatrix().GetYAxis());

		// Query object database for all but static entities
		// ----------------------------------------------------------------------
		CEQCIsInteractionTarget obClause;
		CEntityQuery obQuery;
		obQuery.AddClause(obClause);

		// If available - check whether this entity has been told NOT to interact with curtain objects.
		CEQCIsType obIsType;
		if( pobThis->IsCharacter() )
		{
			const Character* prChar = pobThis->ToCharacter();
			CKeywords obTemp( prChar->GetIgnoredInteractions().c_str() );
			obIsType.SetKeywords( obTemp );
			obQuery.AddUnClause(obIsType);
		}

		CEntityManager::Get().FindEntitiesByType( obQuery, CEntity::EntType_Interactable |
														   CEntity::EntType_AI );
		
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
			// Ensure entity is not paused
			if (!(*obIt)->IsPaused() && pobThis != (*obIt))
			{
				CUsePoint* pobBestUPointPossible;

				// Get score, passing in this position and angle
				float fScore = (*obIt)->GetInteractionComponent()->GetInteractionScore( obPosition, 
																						obDirection, 
																						obPlaneNormalOfApproach,
																						bDirectionHeld,
																						(CUsePoint::InteractingCharacterType) iCharacterType,
																						pobBestUPointPossible );
			
#ifdef _AWAREBINDINGS_DEBUG
				ntPrintf("entity %s scores %f\n",(*obIt)->GetName().c_str(),fScore);
#endif

				// If this entity has a better score then remember it
				if (fScore > fBestEntityScore)
				{
					pobBestEntity = (*obIt);
					fBestEntityScore = fScore;
					pobBestUPoint = pobBestUPointPossible;
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
			return obTargetResult;
		}
	}	
	// If score is better than current one, save this new entity and its score.

	// We didn't find a suitable interaction target
	return obTargetResult;
}

/*
CEntity* Awareness_Lua::Lua_FindInteractionTarget ( )
{
	CEntity* pobThis = ((AwarenessComponent*)this)->m_pobParent;

#ifdef _AWAREBINDINGS_DEBUG
	ntPrintf("entity %s find interaction target\n",pobThis->GetName().c_str() );
#endif

	if ((pobThis->GetAIComponent() || pobThis->GetInputComponent()) && pobThis->GetAwarenessComponent())
	{
		//const float fMAX_DISTANCE=(pobThis->GetInputComponent()->IsDirectionHeld() ? 4.0f*4.0f : 2.0f*2.0f);
		const float fMAX_ANGLE = 90.0f * DEG_TO_RAD_VALUE;
		float fMAX_DISTANCE;

		if ( (pobThis->GetAIComponent() && pobThis->GetAIComponent()->GetMovementMagnitude() > 0.3f) || (pobThis->GetInputComponent() && pobThis->GetInputComponent()->IsDirectionHeld()) )
			fMAX_DISTANCE = 4.0f * 4.0f;
		else
			fMAX_DISTANCE = 2.0f * 2.0f;

		// Get position and targeting direction of this entity
		CPoint obPosition(pobThis->GetHierarchy()->GetRootTransform()->GetWorldMatrix().GetTranslation());
		CDirection obDirection;

		if (pobThis->GetAIComponent() || !pobThis->GetInputComponent()->IsDirectionHeld()) // If this entity is an AI or the analog stick isn't held, then use their character matrix
		{
			obDirection=pobThis->GetHierarchy()->GetRootTransform()->GetWorldMatrix().GetZAxis();
		}
		else // Otherwise use pad direction
		{
			obDirection=pobThis->GetInputComponent()->GetInputDir();
		}

		CEQCIsInteractionTarget obClause;
		CEntityQuery obQuery;
		obQuery.AddClause(obClause);
		CEntityManager::Get().FindEntitiesByType( obQuery, CEntity::EntType_AllButStatic );
		
		// Create a return result
		CEntity* pobBestEntity = 0;

		// Create values to compare the results
		float fBestEntityScore = 0.0f;

		// Loop through the given set and find a score for each of the items
		ntstd::List< CEntity* >::const_iterator obEndIt = obQuery.GetResults().end();

		for ( ntstd::List< CEntity* >::const_iterator obIt = obQuery.GetResults().begin(); obIt != obEndIt; ++obIt )
		{
#ifdef _AWAREBINDINGS_DEBUG
			ntPrintf("interaction target=%s\n",(*obIt)->GetName().c_str());
#endif

			if (!(*obIt)->IsPaused()) // Ensure the entity isn't disabled
			{
				// Find the relative position of the character from us
				CPoint obRelativePosition (CONSTRUCT_CLEAR);
				CPoint obAltRelativePosition (CONSTRUCT_CLEAR);
				if ( (*obIt)->GetInteractionComponent()->GetInteractionPriority() == USE_MULTI )
				{
					// Entity has two useable locations
					// Will need to generalise this if we end up with objects with several use points
					// eg a traverser that you can use in both directions and interupt from a third location - TMcK
					obRelativePosition = ( *obIt )->GetTransformP("transform_001")->GetWorldMatrix().GetTranslation() - obPosition;
					obAltRelativePosition = ( *obIt )->GetTransformP("transform_002")->GetWorldMatrix().GetTranslation() - obPosition;

					obRelativePosition.Y() = 0.0f;
					obAltRelativePosition.Y() = 0.0f;

#ifdef _AWAREBINDINGS_DEBUG
					ntPrintf("position X %f Y %f Z %f\n", obPosition.X(), obPosition.Y(), obPosition.Z());
					CPoint Test =( *obIt )->GetTransformP("transform_001")->GetLocalMatrix().GetTranslation();
					ntPrintf("transform_001 X %f Y %f Z %f\n", Test.X(), Test.Y(), Test.Z());
					Test =( *obIt )->GetTransformP("transform_002")->GetLocalMatrix().GetTranslation();
					ntPrintf("transform_002 X %f Y %f Z %f\n", Test.X(), Test.Y(), Test.Z());
#endif
				}
				else
				{
					obRelativePosition = ( *obIt )->GetPosition() - obPosition;
					obRelativePosition.Y() = 0.0f;
				}

				float fDistanceSquared=obRelativePosition.LengthSquared();
				float fAltDistanceSquared=obAltRelativePosition.LengthSquared();

				if ((fDistanceSquared < fMAX_DISTANCE) || 
					( ( (*obIt)->GetInteractionComponent()->GetInteractionPriority() == USE_MULTI ) && (fAltDistanceSquared < fMAX_DISTANCE) ))
					// Check to see if entity is in range
				{
					// Find the angle of the character from us
					float fCharacterAngle = MovementControllerUtilities::RotationAboutY( obDirection, CDirection( obRelativePosition ) );
					float fAltCharacterAngle = MovementControllerUtilities::RotationAboutY( obDirection, CDirection( obRelativePosition ) );

					if (fCharacterAngle < 0.0f)
						fCharacterAngle = -fCharacterAngle;
					if (fAltCharacterAngle < 0.0f)
						fAltCharacterAngle = -fAltCharacterAngle;

					if ( (fCharacterAngle < fMAX_ANGLE) || 
						( ( (*obIt)->GetInteractionComponent()->GetInteractionPriority() == USE_MULTI ) && (fAltCharacterAngle < fMAX_ANGLE) ) )
						// Check to see if the entity is infront of the player
					{
						// Do height check
						bool bHeightCheck = false;

						float fThisLowerY=obPosition.Y(); // Feet position of the character
						float fThisUpperY=fThisLowerY + 1.5f; // Head position of character
						
						float fOtherHeight=(*obIt)->GetInteractionComponent()->GetEntityHeight();
						float fOtherLowerY=(*obIt)->GetPosition().Y();
						float fOtherUpperY=fOtherLowerY + fOtherHeight;

						if ( (*obIt)->GetInteractionComponent()->GetInteractionPriority() == USE_MULTI )
						{
							fOtherLowerY = ( *obIt )->GetTransformP("transform_001")->GetWorldMatrix().GetTranslation().Y();
							fOtherUpperY = ( *obIt )->GetTransformP("transform_002")->GetWorldMatrix().GetTranslation().Y();
						}

						switch((*obIt)->GetInteractionComponent()->GetInteractionPriority())
						{
							case USE:
							{
								if (fOtherHeight>0.0f) // We are dealing with a ladder
								{
									if (fThisLowerY>=(fOtherLowerY-0.25f) && fThisLowerY<(fOtherLowerY+0.25f)) // We are at the bottom
									{
										bHeightCheck = true;
									}
									
									if (fThisLowerY>=(fOtherUpperY-1.0f) && fThisLowerY<(fOtherUpperY+0.25f)) // We are at the top
									{
										bHeightCheck = true;
									}

								}
								else if (fOtherLowerY>=(fThisLowerY-0.1f) && fOtherUpperY<fThisUpperY) // We are dealing with something like a switch, that might be elevated off the ground
								{
									bHeightCheck = true;
								}

								break;
							}

							case USE_MULTI:
							{
								if (fThisLowerY>=(fOtherLowerY-0.25f) && fThisLowerY<(fOtherLowerY+0.25f)) // We are at the bottom
								{
									bHeightCheck = true;
								}
								
								if (fThisLowerY>=(fOtherUpperY-1.0f) && fThisLowerY<(fOtherUpperY+0.25f)) // We are at the top
								{
									bHeightCheck = true;
								}

								break;
							}

							case PUSH:
							case PICKUP:
							{
								assert((*obIt)->GetRenderableComponent());

								const float fHEIGHT_LOWER_OFFSET = -0.5f;
								const float fHEIGHT_UPPER_OFFSET = 0.5f;
								float fTestY;

								if ((*obIt)->GetPhysicsSystem() && (*obIt)->GetPhysicsSystem()->GetFirstGroupByType( Physics::LogicGroup::ADVANCED_CHARACTER_CONTROLLER )) // This is a character
								{
									fTestY=(*obIt)->GetLocation().Y(); // Use the centre point of the ragdoll
								}
								else // This is a non-character entity
								{
									CAABB obWorldAABB((*obIt)->GetRenderableComponent()->GetWorldSpaceAABB()); // Get world AABB of the entity

									fTestY=obWorldAABB.GetCentre().Y()-obWorldAABB.GetHalfLengths().Y(); // Use the lowest point of the entity (which should be the part thats resting on the ground)
								}

								if (fTestY>=(fThisLowerY+fHEIGHT_LOWER_OFFSET) && fTestY<(fThisLowerY+fHEIGHT_UPPER_OFFSET))
								{
									bHeightCheck = true;
								}

								break;
							}

							default:
							{
								// GCC requires all cases to be handled in switch statements.
								break;
							}
						}

						if (bHeightCheck)
						{
							// Find the distance as a percentage between the given distance
							float fDistanceScore = 1.0f - ( fDistanceSquared / fMAX_DISTANCE );
						
							// Find the angle of the character as a percentage of the input angle
							float fAngleScore = 1.0f - ( fCharacterAngle / fMAX_ANGLE );

							fAngleScore *= 2.0f; // Give the angle a more influence over the score than the distance

							float fInteractionPriorityScore = 0.0f;
							
							// These represent the targeting weights for each interaction type
							switch((*obIt)->GetInteractionComponent()->GetInteractionPriority())
							{
								case USE:		fInteractionPriorityScore = 1.5f;	break;
								case USE_MULTI:	fInteractionPriorityScore = 1.5f;	break;
								case CATCH:		fInteractionPriorityScore = 1.0f;	break;
								case PUSH:		fInteractionPriorityScore = 0.5f;	break;
								case PICKUP:	fInteractionPriorityScore = 0.25f;	break;
								default:
									// GCC requires all cases to be handled in switch statements.
									break;
							}

							// Find a 'score' for this entity
							float fEntityScore = fDistanceScore + fAngleScore + fInteractionPriorityScore;
#ifdef _AWAREBINDINGS_DEBUG
							ntPrintf("entity %s scores %f\n",(*obIt)->GetName().c_str(),fEntityScore);
#endif

							// If this is the best score so far update the best score and the 'winning' entity
							if ( fEntityScore > fBestEntityScore )
							{
								pobBestEntity = *obIt;
								fBestEntityScore = fEntityScore;
							}

							if ( (*obIt)->GetInteractionComponent()->GetInteractionPriority() == USE_MULTI)
							{
								float fAltDistanceScore = 1.0f - ( fAltDistanceSquared / fMAX_DISTANCE );
								float fAltAngleScore = 1.0f - ( fAltCharacterAngle / fMAX_ANGLE );
								fAltAngleScore *= 2.0f; // Give the angle a more influence over the score than the distance
								float fAltInteractionPriorityScore = 1.5f;

								// Find a 'score' for this entity
								float fAltEntityScore = fAltDistanceScore + fAltAngleScore + fAltInteractionPriorityScore;
		
#ifdef _AWAREBINDINGS_DEBUG
								ntPrintf("entity %s ( alt ) scores %f\n",(*obIt)->GetName().c_str(),fAltEntityScore);
#endif

								// If this is the best score so far update the best score and the 'winning' entity
								if ( fAltEntityScore > fBestEntityScore )
								{
									pobBestEntity = *obIt;
									fBestEntityScore = fAltEntityScore;
								}
							}
						}
					}
				}
			}
		}

		if (pobBestEntity && pobBestEntity->HasAttributeTable())
		{
#ifdef _AWAREBINDINGS_DEBUG
			ntPrintf("Found interaction target=%s\n",pobBestEntity->GetName().c_str());
#endif

			return pobBestEntity; // We have found an entity
		}
	}

	return 0; // Suitable entity not found so return nil
}
*/

//-------------------------------------------------------------------------------------------------
// BINDFUNC:	entity FindNamedInteractionTarget()
// DESCRIPTION:	Find the most appropriate interaction target with a given substring in it's name (e.g. 'crossbow')
//-------------------------------------------------------------------------------------------------
CEntity* Awareness_Lua::Lua_FindNamedInteractionTarget(const char* pcNameSubstring, int iCharacterType)
{
	return FindNamedInteractionTarget(pcNameSubstring,iCharacterType).m_pobInteractingEnt;
}


CInteractionTarget Awareness_Lua::FindNamedInteractionTarget(const char* pcNameSubstring, int iCharacterType)
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
		bool bDirectionHeld = false;

		if ( (pobThis->IsAI() && ((AI*)pobThis)->GetAIComponent()->GetMovementMagnitude() > 0.3f ) || 
			 (pobThis->GetInputComponent() && pobThis->GetInputComponent()->IsDirectionHeld() ) )
		{
			bDirectionHeld = true;
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
		CEQCIsSubStringInName obNameClause(pcNameSubstring);
		obQuery.AddClause(obNameClause);
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
																						 bDirectionHeld,
																						 (CUsePoint::InteractingCharacterType)iCharacterType,
																						 pobBestUPoint1 );
				// Get a second score, a step or two back (in-case we overstepped slightly when moving to the object)
				float fScore2 = (*obIt)->GetInteractionComponent()->GetInteractionScore( obPosition - CPoint(obDirection), 
																						 obDirection, 
																						 obPlaneNormalOfApproach, 
																						 bDirectionHeld,
																						 (CUsePoint::InteractingCharacterType)iCharacterType,
																						 pobBestUPoint2 );
				// Get a third score, a step or two forwards (in-case we stopped short when moving to the object)
				float fScore3 = (*obIt)->GetInteractionComponent()->GetInteractionScore( obPosition + CPoint(obDirection), 
																						 obDirection, 
																						 obPlaneNormalOfApproach, 
																						 bDirectionHeld,
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
			return obTargetResult;
		}
	}	
	// If score is better than current one, save this new entity and its score.

	// We didn't find a suitable interaction target
	return obTargetResult;
}
