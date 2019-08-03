//--------------------------------------------------
//!
//!	\file interactioncomponent.cpp
//!	This is a dummy holding a few doxygen comments.
//!
//--------------------------------------------------

#include "Physics/config.h"
#include "Physics/system.h"

// Necessary includes
#include "objectdatabase/dataobject.h"
#include "game/interactioncomponent.h"
#include "game/messagehandler.h"
#include "game/entityinfo.h"
#include "game/entity.h"
#include "game/entityboss.h"
#include "game/entity.inl"
#include "game/entityinterabtablespawner.h"
#include "anim/hierarchy.h"
#include "game/movementcontrollerinterface.h"
#include "core/visualdebugger.h"
#include "effect/psystem_utils.h"
#include "effect/psystem_simple.h"
#include "effect/effect_manager.h"

#ifndef _RELEASE
#define DEBUGRENDER_TRAPEZOIDCHECK	//Uncomment to render trapezoid check zones (cone-ends not included).
#endif

//------------------------------------------------------------------------------------------
// Constants
//------------------------------------------------------------------------------------------
static const float NO_SUITABLE_USE_POINT_SCORE = -99999.9f;

START_STD_INTERFACE(Attr_Interactable)
	PUBLISH_PTR_AS( m_pobUsePointAttrs,				SharedUseParams ) 		
END_STD_INTERFACE

START_STD_INTERFACE(CUsePointAttr)
	PUBLISH_CONTAINER_AS(m_obUsePointAnimList			, 		UsePointAnimList)
	DECLARE_POSTCONSTRUCT_CALLBACK( OnPostConstruct )	
END_STD_INTERFACE

START_STD_INTERFACE(CUsePoint)
	PUBLISH_VAR_AS(m_Name, m_Name);
	PUBLISH_VAR_AS(m_UsePointPfxName, m_PfxName)
	PUBLISH_VAR_AS(m_obLocalPosition,	 													m_fTranslation)
	PUBLISH_VAR_AS(m_obLocalOrientation, 													m_fRotation)
	PUBLISH_VAR_AS(m_obLocalRotationAxis,													RotationNormal)
	PUBLISH_VAR_AS(m_obLocalFacingNormal, 													FacingNormal)
	PUBLISH_VAR_WITH_DEFAULT_AS(m_obLocalOffsetMoveTo,	CVector(0.0f, 0.0f, 0.0f, 0.0f), 	MoveToOffset)
	PUBLISH_VAR_WITH_DEFAULT_AS(m_fAngleOfUseMargin,  	0.0f,  								AngleMargin)
	PUBLISH_VAR_WITH_DEFAULT_AS(m_bHeroCanUse, 	 true  	, 									HeroUseable)
	PUBLISH_VAR_WITH_DEFAULT_AS(m_bArcherCanUse, true	, 									ArcherUseable)
	PUBLISH_VAR_WITH_DEFAULT_AS(m_bEnemyAICanUse, true	, 									EnemyUseable)
	PUBLISH_VAR_WITH_DEFAULT_AS(m_bAllyAICanUse, true	, 									AllyUseable)
	PUBLISH_VAR_WITH_DEFAULT_AS(m_bHasFacingRequirements, true,								HasUseFacingNavRequirement)

	PUBLISH_VAR_WITH_DEFAULT_AS(m_fDirectionHeldUseRadius,	4.0f,							DirectionHeldUseRadius)
	PUBLISH_VAR_WITH_DEFAULT_AS(m_fUseRadius,				2.0f,							UseRadius)
	PUBLISH_VAR_WITH_DEFAULT_AS(m_fUseHeight,				0.5f,							UseHeight)

	PUBLISH_VAR_WITH_DEFAULT_AS(m_fBaseHalfWidth,		0.5f,								UseZoneBaseHalfWidth)
	PUBLISH_VAR_WITH_DEFAULT_AS(m_fZBackwardsOffset,	0.4f,								UseZoneBackwardsOffset)
	DECLARE_POSTCONSTRUCT_CALLBACK( OnPostConstruct )	

END_STD_INTERFACE

//--------------------------------------------------
//!
//!	Short Function description.
//!	Long function description
//!	\return 
//!	\param a  
//!
//--------------------------------------------------
CInteractionComponent::CInteractionComponent (CEntity* pobParent) :
	m_fEntityHeight(0.0f),
	m_eInteractionPriority(NONE),
	m_pobParentEntity(pobParent)	
{
	ATTACH_LUA_INTERFACE(CInteractionComponent);

	// Setup the use points on the entity
	SetupUsePoints();

	SetupUsePointPfx();

	ntAssert(pobParent);
}

CInteractionComponent::~CInteractionComponent ()
{
	while(!m_obCollisionFilterList.empty())
	{
		AllowCollisionWith(m_obCollisionFilterList.back());
	}

	// Delete all manually created use-points
	UsePointArray::iterator obIt;
	UsePointArray::iterator obEndIt = m_obManuallyCreatedUsePointArray.end();

	for ( obIt = m_obManuallyCreatedUsePointArray.begin(); obIt != obEndIt; obIt++ )
	{
		NT_DELETE_CHUNK( Mem::MC_ENTITY, (*obIt) );
	}
}

void CInteractionComponent::ExcludeCollisionWith (CEntity* pobEntity)
{
	if (pobEntity && pobEntity->GetInteractionComponent())
	{
		AddToCollisionFilter(pobEntity); // Add entity to this interaction list

		pobEntity->GetInteractionComponent()->AddToCollisionFilter(m_pobParentEntity); // Add this to other entities interaction list

		// Force a collision filter update on both entities
		if( m_pobParentEntity->GetPhysicsSystem() )
		{
			m_pobParentEntity->GetPhysicsSystem()->UpdateCollisionFilter();
		}

		if( pobEntity->GetPhysicsSystem() )
		{
			pobEntity->GetPhysicsSystem()->UpdateCollisionFilter();
		}
	}	
	else
	{
		//ntPrintf("Not able to exclude collision with entity.\n");
	}
}


void CInteractionComponent::AllowCollisionWith (CEntity* pobEntity)
{
	if (pobEntity && pobEntity->GetInteractionComponent() && !CanCollideWith(pobEntity)) // Check to make sure we are associated first
	{
		pobEntity->GetInteractionComponent()->RemoveFromCollisionFilter(m_pobParentEntity); // Remove parent from target entities list

		RemoveFromCollisionFilter(pobEntity); // Remove entity from this entities list
	}
	else
	{
		//ntPrintf("Not able to allow collision with entity.\n");
	}
}

bool CInteractionComponent::CanCollideWith (CEntity* pobEntity)
{
	if (pobEntity && !m_obCollisionFilterList.empty())
	{
		for(ntstd::List<CEntity*>::iterator obIt=m_obCollisionFilterList.begin(); obIt!=m_obCollisionFilterList.end(); ++obIt)
		{
			if ((*obIt)==pobEntity)
				return false;
		}
	}

	if (pobEntity && !m_obNotifyOnInteractionWithList.empty())
	{
		for ( ntstd::List<CEntity*>::iterator obIt = m_obNotifyOnInteractionWithList.begin(); obIt != m_obNotifyOnInteractionWithList.end(); ++obIt)
		{
			if ((*obIt) == pobEntity)
			{
				// Need to tell parent they've come into contact with this
				// At the moment only bosses have a use for this, might be more general in future
				if (m_pobParentEntity->IsBoss())
				{
					Boss* pobBoss = (Boss*)m_pobParentEntity;
					pobBoss->NotifyInteractionWith(pobEntity);
				}
			}
		}
	}
	
	return true;
}

void CInteractionComponent::SetupNotifyOnInteractionWith (CEntity* pobEntity)
{
	AddToNotifyOnInteractionWith(pobEntity); // Add entity to this interaction list
}


void CInteractionComponent::StopNotifyOnInteractionWith (CEntity* pobEntity)
{
	RemoveFromNotifyOnInteractionWith(pobEntity); // Remove entity from this entities list
}

void CInteractionComponent::AddToNotifyOnInteractionWith (CEntity* pobEntity)
{
	m_obNotifyOnInteractionWithList.push_back(pobEntity);
}

void CInteractionComponent::RemoveFromNotifyOnInteractionWith (CEntity* pobEntity)
{
	m_obNotifyOnInteractionWithList.remove(pobEntity);
}

void CInteractionComponent::AddToCollisionFilter (CEntity* pobEntity)
{
	if (CanCollideWith(pobEntity)) // We don't want duplicate entities in the list!!
	{
		m_obCollisionFilterList.push_back(pobEntity);
	}
}

void CInteractionComponent::RemoveFromCollisionFilter (CEntity* pobEntity)
{
	m_obCollisionFilterList.remove(pobEntity);
}

//------------------------------------------------------------------------------------------
//!
//!	CInteractionComponent::GetUsePoint
//! Gets a use point from an index number
//!
//------------------------------------------------------------------------------------------
CUsePoint* CInteractionComponent::GetUsePoint( unsigned int nIndex )
{
	ntAssert( nIndex >= 0 );

	// Convert to an assert sometime???
	if ( nIndex >= m_obUsePointArray.size() )
	{
		return 0;
	}

	UsePointArray::const_iterator obEndIt = m_obUsePointArray.end();
	unsigned int nCount = 0;
	for ( UsePointArray::const_iterator obIt = m_obUsePointArray.begin(); obIt != obEndIt; ++obIt )
	{
		if (nCount == nIndex)
		{
			return (*obIt);
		}

        nCount++;
	}

	// Shouldn't get here
	return 0;
}

//------------------------------------------------------------------------------------------
//!
//!	CInteractionComponent::GetUsePointByName
//! Gets a use point based on a name
//!
//------------------------------------------------------------------------------------------
CUsePoint* CInteractionComponent::GetUsePointByName(const CHashedString& Name)
{
	// Check the name is okay
	ntAssert( !ntStr::IsNull(Name) );

	UsePointArray::const_iterator obEndIt = m_obUsePointArray.end();

	for ( UsePointArray::const_iterator obIt = m_obUsePointArray.begin(); obIt != obEndIt; ++obIt )
	{
		if ( (*obIt)->GetName() == Name )
		{
			return (*obIt);
		}
	}

	// Could not find the use point
	return 0;
}

//------------------------------------------------------------------------------------------
//!
//!	CInteractionComponent::GetClosestUsePoint
//! Gets a use point based on a name
//!
//------------------------------------------------------------------------------------------
CUsePoint* CInteractionComponent::GetClosestUsePoint(const CPoint& obCharacterPos)
{
	float fBestDist = 9999.99f;
	CUsePoint* pobBestUsePoint = 0;

	CPoint obUsePointLocalPosition(CONSTRUCT_CLEAR);
	CPoint obUsePointWorldPosition(CONSTRUCT_CLEAR);
	CPoint obRelativePosition(CONSTRUCT_CLEAR);
	float fDistanceSquared;

	// Iterate over all the use points in the list
	UsePointArray::const_iterator obEndIt = m_obUsePointArray.end();

	for ( UsePointArray::const_iterator obIt = m_obUsePointArray.begin(); obIt != obEndIt; ++obIt )
	{
		// Calculate relative position to character (XZ plane only)
		obUsePointLocalPosition = (*obIt)->GetLocalPosition();
		obUsePointWorldPosition = obUsePointLocalPosition * m_pobParentEntity->GetRootTransformP()->GetWorldMatrix();
		obRelativePosition = obUsePointWorldPosition - obCharacterPos;
		//obRelativePosition.Y() = 0.0f;

		fDistanceSquared = obRelativePosition.LengthSquared();

		// Check if use point best
		if ( fDistanceSquared <= fBestDist )
		{
			pobBestUsePoint = *obIt;
			fBestDist = fDistanceSquared;
		}
	}

	return pobBestUsePoint;
}


//------------------------------------------------------------------------------------------
//!
//!	CInteractionComponent::RemoveUsePointPfxData
//! Removes any use point data and transforms that were added to the entity's hierarchy for particles
//!
//------------------------------------------------------------------------------------------
void CInteractionComponent::RemoveUsePointPfxData( void )
{
	// For each use point
	UsePointPfxDataArray::iterator obEndIt = m_obUsePointPfxDataArray.end();

	for ( UsePointPfxDataArray::iterator obIt = m_obUsePointPfxDataArray.begin(); obIt != obEndIt; obIt++ )
	{
		CUsePointPfxData* pobPfxData = (*obIt);
		ntAssert( pobPfxData );

		pobPfxData->m_obTransform.RemoveFromParent();

		NT_DELETE_CHUNK( Mem::MC_ENTITY, pobPfxData );
	}

	m_obUsePointPfxDataArray.clear();
}


//------------------------------------------------------------------------------------------
//!
//!	CInteractionComponent::GetHeightFromUsePoints
//! Works out a height from the use points on it
//!
//------------------------------------------------------------------------------------------
float CInteractionComponent::GetHeightFromUsePoints() const
{
	float fMinY = 99999.9f;
	float fMaxY = -99999.9f;

	// There must be more than one use point to determine a height
	if ( m_obUsePointArray.size() < 2 )
		return 0.0f;

	CPoint obUsePointWorldPosition(CONSTRUCT_CLEAR);

	// Cycle through the use points and work out the highest and lowest use points
	UsePointArray::const_iterator obEndIt = m_obUsePointArray.end();

	for ( UsePointArray::const_iterator obIt = m_obUsePointArray.begin(); obIt != obEndIt; ++obIt )
	{
		// Calculate relative position to character (XZ plane only)
		obUsePointWorldPosition = (*obIt)->GetLocalPosition() * m_pobParentEntity->GetRootTransformP()->GetWorldMatrix();

		if ( obUsePointWorldPosition.Y() < fMinY )
			fMinY = obUsePointWorldPosition.Y();

		if ( obUsePointWorldPosition.Y() > fMaxY )
			fMaxY = obUsePointWorldPosition.Y();
	}

	return fMaxY - fMinY;
}


//------------------------------------------------------------------------------------------
//!
//!	CInteractionComponent::GetInteractionScore
//! Gets the interaction score for the particular entity
//!
//------------------------------------------------------------------------------------------
float CInteractionComponent::GetInteractionScore(const CPoint& obCharacterPos, 
												 const CDirection& obCharacterDirfloat, 
												 const CDirection& obCharacterPlaneNormal, 
												 bool bDirectionHeld,
												 CUsePoint::InteractingCharacterType eCharcInterType,
												 CUsePoint*& pobUPointBest) const
{
	pobUPointBest = 0;

	// If there are no use points on the object then it can't be interacted with
	if ( m_obUsePointArray.empty() || m_eInteractionPriority == NONE  || !IsCharacterUseable( eCharcInterType ) )
	{
		return -99999.9f;
	}

	// If the entity doesn't have a hierarchy, then return.
	if ( m_pobParentEntity && !m_pobParentEntity->GetHierarchy() )
	{
		return -99999.9f;
	}

	// Score is based on three things.
	// ----------------------------------------------------------------------

	// 1. Distance from character (too far and it will be ignored)
	float fDistanceScore = 0.0f;
	// 2. Angle from character direction
	float fAngleScore = 0.0f;
	// 3. Type of entity - Scores are biased based on it's interaction priority
	float fInteractionPriorityScore = 0.0f;

	// 4. If use point has an angle of use, this measures how normalised with the object you are
	//	  Currently, it is just 1.0f if you are within the use angle range, so does not affect scoring
	float fUsabilityAngleScore = 0.0f;

	// Distance and angle scoring
	// ----------------------------------------------------------------------

	const float fMAX_ANGLE = 90.0f * DEG_TO_RAD_VALUE;
	CPoint obUsePointLocalPosition(CONSTRUCT_CLEAR);
	CPoint obUsePointWorldPosition(CONSTRUCT_CLEAR);
	CPoint obRelativePosition(CONSTRUCT_CLEAR);
	float fDistanceSquared;
	float fCharacterAngle;

	bool bFoundSuitableUsePoint = false;


	const CMatrix obEntWorldMatrix = m_pobParentEntity->GetRootTransformP()->GetWorldMatrix();
	const CMatrix obLocalVectorTransform = 
		obEntWorldMatrix.GetAffineInverse().GetTranspose();

	// Iterate over all the use points in the list
	UsePointArray::const_iterator obEndIt = m_obUsePointArray.end();

	for ( UsePointArray::const_iterator obIt = m_obUsePointArray.begin(); obIt != obEndIt; ++obIt )
	{
		CUsePoint* const pobUPoint = (*obIt);

		if ( pobUPoint->IsCharacterFriendly(eCharcInterType) )
		{
			// Use radius - Squared for length comparison without sqrt
			float fMAX_DISTANCE = pobUPoint->GetUseRadius( bDirectionHeld );
			fMAX_DISTANCE *= fMAX_DISTANCE;

			// Calculate relative position to character (XZ plane only)
			obUsePointLocalPosition = pobUPoint->GetLocalPosition();
			obUsePointWorldPosition = obUsePointLocalPosition * obEntWorldMatrix;
			obRelativePosition = obUsePointWorldPosition - obCharacterPos;
			obRelativePosition.Y() = 0.0f;

			fDistanceSquared = obRelativePosition.LengthSquared();

			// Check if use point is in range
			if ( fDistanceSquared <= fMAX_DISTANCE )
			{
				//If our point has no base-length, then we just take the actual use-point and offset it backwards to calculate fCharacterAngle.
				if(pobUPoint->GetBaseHalfWidth() == 0.0f)
				{
					//The angle to the character needs to be calculated from the offset use-point position (to reflect the fact that
					//use points can be pushed back into an object a little bit to stop you getting so close that you can't use it anymore).
					CPoint obUsePointWorldPositionAdjusted = obUsePointWorldPosition - (obEntWorldMatrix.GetZAxis() * pobUPoint->GetZBackOffset());
					CPoint obRelativePositionAdjusted = obUsePointWorldPositionAdjusted - obCharacterPos;
					fCharacterAngle = MovementControllerUtilities::RotationAboutY( obCharacterDirfloat, CDirection( obRelativePositionAdjusted) );
				}
				else
				{
					//Otherwise, we take a point at each end of the base, offset both backwards,check both of them and take the 'best' one.
					CPoint obUsePointWorldPositionLeft = obUsePointWorldPosition + (obEntWorldMatrix.GetXAxis() * pobUPoint->GetBaseHalfWidth());
					obUsePointWorldPositionLeft = obUsePointWorldPositionLeft - (obEntWorldMatrix.GetZAxis() * pobUPoint->GetZBackOffset());
					CPoint obUsePointWorldPositionRight = obUsePointWorldPosition - (obEntWorldMatrix.GetXAxis() * pobUPoint->GetBaseHalfWidth());
					obUsePointWorldPositionLeft = obUsePointWorldPositionLeft - (obEntWorldMatrix.GetZAxis() * pobUPoint->GetZBackOffset());

					CPoint obRelativePositionLeft = obUsePointWorldPositionLeft - obCharacterPos;
					CPoint obRelativePositionRight = obUsePointWorldPositionRight - obCharacterPos;

					float fCharacterAngleLeft = MovementControllerUtilities::RotationAboutY( obCharacterDirfloat, CDirection(obRelativePositionLeft) );
					float fCharacterAngleRight = MovementControllerUtilities::RotationAboutY( obCharacterDirfloat, CDirection(obRelativePositionRight) );
					if(fCharacterAngleLeft < 0.0f) { fCharacterAngleLeft = -fCharacterAngleLeft; }
					if(fCharacterAngleRight < 0.0f) { fCharacterAngleRight = -fCharacterAngleRight; }

					//Select the smallest as the "best".
					fCharacterAngle = (fCharacterAngleLeft <= fCharacterAngleRight) ? fCharacterAngleLeft : fCharacterAngleRight;
				}

				if ( fCharacterAngle < 0.0f )
					fCharacterAngle = -fCharacterAngle;

				// Check if use point is in front of character
				if ( fCharacterAngle < fMAX_ANGLE )
				{
					// HEIGHT CHECKING
					// Use points should be standardised now, and be at floor level for all objects
					const float fHEIGHT_LOWER_OFFSET = -pobUPoint->GetUseHeight(); // Look upto m_fUseHeight below the character root
					const float fHEIGHT_UPPER_OFFSET = pobUPoint->GetUseHeight();  // and upto m_fUseHeight above
					float fCharacterFeetHeight = obCharacterPos.Y();

					// Check the use point is within a certain height range of the characters feet
					if ( fCharacterFeetHeight >= (obUsePointWorldPosition.Y() + fHEIGHT_LOWER_OFFSET) && fCharacterFeetHeight <= (obUsePointWorldPosition.Y() + fHEIGHT_UPPER_OFFSET) )
					{

						// now check in case it has an angle of use defined
						float fUsabilityOnApproachTemp = 0.0f;
						//If our base side is just a point, then perform a single cone-check for usability-from-approach
						if(pobUPoint->GetBaseHalfWidth() == 0.0f)
						{
							fUsabilityOnApproachTemp = pobUPoint->DetermineUsabilityFactorFromApproach(obCharacterPos,
																			obCharacterPlaneNormal,
																			obLocalVectorTransform,
																			obEntWorldMatrix);
						}
						//Otherwise, perform an isosceles trapezoid check instead to account for the width of the base.
						else
						{
							fUsabilityOnApproachTemp = pobUPoint->DetermineUsabilityFactorFromIsoscelesTrapezoid(obCharacterPos,
																			obCharacterPlaneNormal,
																			obLocalVectorTransform,
																			obEntWorldMatrix, fMAX_DISTANCE);
						}

						const float fUsabilityOnApproach = fUsabilityOnApproachTemp;	//Just to make sure we're not changing it anywhere below.

						if (fUsabilityOnApproach>0.0f)
						{
							float fNewDistanceScore = 1.0f - ( fDistanceSquared / fMAX_DISTANCE );

							// Find the angle of the character as a percentage of the input angle
							float fNewAngleScore = 1.0f - ( fCharacterAngle / fMAX_ANGLE );
							fNewAngleScore *= 2.0f;

							float fNewUsabilityAngleScore = fUsabilityOnApproach;

							// If new combined scores are better, store them
							if ( (fNewDistanceScore + fNewAngleScore + fNewUsabilityAngleScore) > (fDistanceScore + fAngleScore + fUsabilityAngleScore) )
							{
								fDistanceScore = fNewDistanceScore;
								fAngleScore = fNewAngleScore;
								fUsabilityAngleScore = fNewUsabilityAngleScore;

								pobUPointBest = pobUPoint;
							}

							bFoundSuitableUsePoint = true;

						}
					}
				}
			}

		}
	}

	// If none of the mutliple use points were suitable, return a horrific score.
	if (!bFoundSuitableUsePoint)
	{
		return NO_SUITABLE_USE_POINT_SCORE;
	}

	// Bias score based on entity's interaction priority
	// ----------------------------------------------------------------------

	switch( m_eInteractionPriority )
	{
		case USE:		fInteractionPriorityScore = 1.5f;	break;
		case CATCH:		fInteractionPriorityScore = 1.0f;	break;
		case PUSH:		fInteractionPriorityScore = 0.5f;	break;
		case PICKUP:	fInteractionPriorityScore = 0.25f;	break;
		default:
			// GCC requires all cases to be handled in switch statements.
			break;
	}

	// Return total interaction score
	return ( fDistanceScore + fAngleScore + fUsabilityAngleScore + fInteractionPriorityScore );
}


//------------------------------------------------------------------------------------------
//!
//!	CInteractionComponent::SetupUsePointPfx
//! Sets up the pfx on the use point if necessary.
//!
//------------------------------------------------------------------------------------------
void CInteractionComponent::SetupUsePointPfx( void )
{
	// For each use point
	UsePointArray::iterator obEndIt = m_obUsePointArray.end();

	for ( UsePointArray::iterator obIt = m_obUsePointArray.begin(); obIt != obEndIt; ++obIt )
	{
		CUsePoint* const pobUPoint = (*obIt);
		
		// If it has a particle name assigned to it
		if( !ntStr::IsNull( pobUPoint->GetPfxName() ) )
		{
			// Create a transform for the particle effect position
			CUsePointPfxData* pNewPfxData = NT_NEW_CHUNK( Mem::MC_ENTITY ) CUsePointPfxData();
			ntAssert( pNewPfxData );

			// Transform
			CMatrix localMatrix( CONSTRUCT_IDENTITY );
			localMatrix.SetTranslation( pobUPoint->GetLocalPosition() );
			pNewPfxData->m_obTransform.SetLocalMatrix( localMatrix );
			m_pobParentEntity->GetHierarchy()->GetRootTransform()->AddChild( &pNewPfxData->m_obTransform );

			// Pfx Name
			pNewPfxData->m_UsePointPfxName = pobUPoint->GetPfxName();

			// Add it to the array
			m_obUsePointPfxDataArray.push_back( pNewPfxData );
		}
	}
}


//------------------------------------------------------------------------------------------
//!
//!	CInteractionComponent::UpdateUsePointPfx
//! Updates the pfx on the use point if necessary.
//!
//------------------------------------------------------------------------------------------
void CInteractionComponent::UpdateUsePointPfx( void )
{
	// For each use point
	UsePointPfxDataArray::iterator obEndIt = m_obUsePointPfxDataArray.end();

	for ( UsePointPfxDataArray::iterator obIt = m_obUsePointPfxDataArray.begin(); obIt != obEndIt; ++obIt )
	{
		CUsePointPfxData* pobPfxData = (*obIt);
		ntAssert( pobPfxData );

		// Turn on effect if interaction priority is USE
		if ( m_eInteractionPriority == USE )
		{
			if ( pobPfxData->m_uPfxID == 0 )
			{
				void* pPfxDef = ObjectDatabase::Get().GetPointerFromName<void*>( pobPfxData->m_UsePointPfxName );
				if ( pPfxDef )
				{
					pobPfxData->m_uPfxID = PSystemUtils::ConstructParticleEffect( pPfxDef, &pobPfxData->m_obTransform );
				}
			}
		}
		else
		{
			// Turn effect off if its there
			if ( pobPfxData->m_uPfxID != 0 )
			{
				EffectManager::Get().KillEffectWhenReady( pobPfxData->m_uPfxID );
				pobPfxData->m_uPfxID = 0;
			}
		}
	}
}

//------------------------------------------------------------------------------------------
//!
//!	CInteractionComponent::SetupUsePoints
//! Sets up the use-points on the object, by attempting to load an .int.xml file related to
//! the entity.
//!
//------------------------------------------------------------------------------------------
void CInteractionComponent::SetupUsePoints(void)
{
	// Generate the name of the expected interaction XML file
	ntstd::String clumpFileName = m_pobParentEntity->GetClumpString();

	// Swap the extension to .int.xml
	ntstd::String interactionXMLFile( clumpFileName, 0, strlen( clumpFileName.c_str() ) - 5 );
	interactionXMLFile += ntstd::String( "int.xml\0" );

	// Make the filename all lower-case and replace / with \.
	ntstd::transform( interactionXMLFile.begin(), interactionXMLFile.end(), interactionXMLFile.begin(), &ntstd::Tolower );
	ntstd::transform( interactionXMLFile.begin(), interactionXMLFile.end(), interactionXMLFile.begin(), &ntstd::ConvertSlash );

	// Extend the filename
	static char TempFilename[ 2048 ];
	Util::GetFiosFilePath( interactionXMLFile.c_str(), TempFilename );
	interactionXMLFile = TempFilename;

	// Attempt to load it and read in at least 1 use-point
	if ( !AttemptToReadInteractionXMLFile( interactionXMLFile ) )
	{
		// Didn't manage to read in a use-point so create a default one at the root
		// @todo [scee_st] points can be allocated in all sorts of different chunks :(
		CUsePoint* pobDefaultUsePoint = NT_NEW_CHUNK ( Mem::MC_ENTITY ) CUsePoint( CHashedString("UsePoint_Default"), CPoint(CONSTRUCT_CLEAR), CQuat(CONSTRUCT_CLEAR) );

		pobDefaultUsePoint->SetParentIntComponent( this );

		// Add it to the list
		m_obUsePointArray.push_back( pobDefaultUsePoint );

		// Add it to the manually created list, so we know we can delete this one in the destructor
		m_obManuallyCreatedUsePointArray.push_back( pobDefaultUsePoint );
	}
}

//------------------------------------------------------------------------------------------
//!
//!	CInteractionComponent::RegisterWithUsePoints
//! In post post construction, the use point is notified of its parent.
//!
//------------------------------------------------------------------------------------------
void CInteractionComponent::RegisterWithUsePoints(void)
{
	// Iterate over all the use points in the list
	UsePointArray::iterator obEndIt = m_obUsePointArray.end();

	for ( UsePointArray::iterator obIt = m_obUsePointArray.begin(); obIt != obEndIt; ++obIt )
	{
		CUsePoint* const pobUPoint = (*obIt);
		pobUPoint->RegisterParentAs(this);
	}

}

//------------------------------------------------------------------------------------------
//!
//!	CInteractionComponent::AttemptToReadInteractionXMLFile
//! Sets up the use-points on the object
//!
//------------------------------------------------------------------------------------------
bool CInteractionComponent::AttemptToReadInteractionXMLFile(const ntstd::String& XMLFileName)
{
	// Check that the file exists
	if( !File::Exists( XMLFileName.c_str() ) )
	{
		return false;
	}

	// We need to know at the end whether we loaded at least one use point or not
	bool bLoadedAUsePoint = false;

	DataObject* obj = ObjectDatabase::Get().GetDataObjectFromName( CHashedString(XMLFileName) );
	
	if ( obj == 0 )
	{
		// Tell people what we're up to
		ntPrintf("XML loading \'%s\'\n", XMLFileName.c_str());

		// Open the XML file in memory
		FileBuffer obFile( XMLFileName.c_str(), true );

		// Load the file
		ObjectDatabase::Get().LoadDataObject( &obFile, XMLFileName );
		obj = ObjectDatabase::Get().GetDataObjectFromName( CHashedString(XMLFileName) );
	}

	if ( obj )
	{
		ObjectContainer* current = static_cast<ObjectContainer*>( obj->GetBasePtr() );
		for (	ObjectContainer::ObjectList::iterator obIt = current->m_ContainedObjects.begin(); 
				obIt != current->m_ContainedObjects.end(); 
				obIt++ )
		{
			if ( 0 == strcmp( (*obIt)->GetClassName(), "CUsePoint") )
			{
				CUsePoint* pUsePoint = (CUsePoint*)((*obIt)->GetBasePtr());
				m_obUsePointArray.push_back(pUsePoint);
				bLoadedAUsePoint = true;
			}
		}
	}

	// Return whether we loaded a use point or not
	return bLoadedAUsePoint;
}

//------------------------------------------------------------------------------------------
//!
//!	CInteractionComponent::DebugRenderUsePoints
//! Debug Rendering of use points
//!
//------------------------------------------------------------------------------------------
#include "game/entitymanager.h"

void CInteractionComponent::DebugRenderUsePoints()
{
#ifndef _GOLD_MASTER

	CMatrix obEntWorldMatrix = m_pobParentEntity->GetRootTransformP()->GetWorldMatrix();
	const CMatrix obLocalVectorTransform = 
		obEntWorldMatrix.GetAffineInverse().GetTranspose();


	CDirection obXOffset = CVecMath::GetXAxis() * obEntWorldMatrix;
	CDirection obYOffset = CVecMath::GetYAxis() * obEntWorldMatrix;
	CDirection obZOffset = CVecMath::GetZAxis() * obEntWorldMatrix;

	obXOffset*=0.2f;
	obYOffset*=0.2f;
	obZOffset*=0.2f;

	int iIndex = 1;
	UsePointArray::const_iterator obEndIt = m_obUsePointArray.end();

	for ( UsePointArray::const_iterator obIt = m_obUsePointArray.begin(); obIt != obEndIt; ++obIt )
	{
		// Calculate relative position to character (XZ plane only)
		const CUsePoint* const pobUPoint = (*obIt);


		CPoint obUsePointWorldPosition;
		CDirection obUsePointFacingDir;

		pobUPoint->GetPosAndFacingNormalWS(obLocalVectorTransform,
										   obEntWorldMatrix,
										   obUsePointWorldPosition,
										   obUsePointFacingDir);

		CPoint obXEnd(	obUsePointWorldPosition + obXOffset );

		CPoint obYEnd(	obUsePointWorldPosition + obYOffset );

		CPoint obZEnd(	obUsePointWorldPosition + obZOffset );

		g_VisualDebug->RenderLine(obUsePointWorldPosition,obXEnd,0xffff0000);
		g_VisualDebug->RenderLine(obUsePointWorldPosition,obYEnd,0xff00ff00);
		g_VisualDebug->RenderLine(obUsePointWorldPosition,obZEnd,0xff0000ff);
		g_VisualDebug->Printf3D( obUsePointWorldPosition, 0xffffffff,0,"Use %d", iIndex );

		if (pobUPoint->HasUseAngle())
		{
			const CPoint obUPEnd(obUsePointWorldPosition + obUsePointFacingDir);

			g_VisualDebug->RenderLine( obUsePointWorldPosition, obUPEnd, 0x7fffff00);
			g_VisualDebug->Printf3D( obUPEnd, 0xffffffff,0,"Use point facing tip %d, Angle of use = %f degrees", iIndex, pobUPoint->GetUseAngle()*180.0f/PI );


			// scee.sbashow : is there a 'find player entity', rather than having to use "Hero"?
			CEntity* const pobPlayer = CEntityManager::Get().GetPlayer();

			const CPoint obWorldPosPlayer = 
				pobPlayer->GetRootTransformP()->GetWorldMatrix().GetTranslation();

			if(pobUPoint->GetBaseHalfWidth() == 0)
			{
				if (pobUPoint->DetermineUsabilityFactorFromApproach(obWorldPosPlayer,
																	pobPlayer->GetRootTransformP()->GetWorldMatrix().GetYAxis(),
																	obLocalVectorTransform,
																	obEntWorldMatrix) > 0.0f)
				{
					g_VisualDebug->RenderLine( obUsePointWorldPosition, obWorldPosPlayer, 0xff00ff00);
				}
				else
				{
					g_VisualDebug->RenderLine( obUsePointWorldPosition, obWorldPosPlayer, 0x7fff0000);
				}
			}
			else
			{
				//NOTE: I use 4.0f instead of fMAX_DISTANCE here (seeing as it's not available). It's only for debug-rendering anyway!
				if(pobUPoint->DetermineUsabilityFactorFromIsoscelesTrapezoid(obWorldPosPlayer,
																			pobPlayer->GetRootTransformP()->GetWorldMatrix().GetYAxis(),
																			obLocalVectorTransform,
																			obEntWorldMatrix, 4.0f) > 0.0f)
				{
					g_VisualDebug->RenderLine( obUsePointWorldPosition, obWorldPosPlayer, 0xff00ff00);
				}
				else
				{
					g_VisualDebug->RenderLine( obUsePointWorldPosition, obWorldPosPlayer, 0x7fff0000);
				}
			}

			//CDirection obPerp = 
			//	CDirection(obDrToUP).GetPerpendicular();


		}
		else
		{
			g_VisualDebug->RenderArc(CMatrix (CQuat(CONSTRUCT_CLEAR), obUsePointWorldPosition), pobUPoint->GetUseRadius(true), TWO_PI, DC_RED);
			g_VisualDebug->RenderArc(CMatrix (CQuat(CONSTRUCT_CLEAR), obUsePointWorldPosition), pobUPoint->GetUseRadius(false), TWO_PI, DC_GREEN);
	
		}
		
		iIndex++;
	}
#endif
}

bool CInteractionComponent::IsCharacterUseable(CUsePoint::InteractingCharacterType eCharType)	const 
{
	ntAssert ( m_pobParentEntity->IsInteractable() );

	Interactable* pobInteract = m_pobParentEntity->ToInteractable();
	switch (eCharType)
	{
	case CUsePoint::ICT_Undefined:
		ntAssert_p(0, ("Undefined character type attempting to use %s\n", ntStr::GetString ( m_pobParentEntity->GetName() ) ));
		return true;
		break;

	case CUsePoint::ICT_Archer:
		return pobInteract->ArcherCanUse();
		break;
		
	case CUsePoint::ICT_Hero:
		return pobInteract->HeroCanUse();	
		break;

	case CUsePoint::ICT_Enemy:
		return pobInteract->EnemyAICanUse();	
		break;

	case CUsePoint::ICT_Ally:
		return pobInteract->AllyAICanUse();	
		break;

	default:
		ntAssert_p(0, ("Unexpected character type attempting to use %s\n", ntStr::GetString ( m_pobParentEntity->GetName() ) ));
		return false;
		break;
	}
	return false;
}

void CUsePoint::GetPosAndFacingNormalWS(const CMatrix& 		obParentTransformAffineInverse,
										const CMatrix& 		obParentTransform,
										CPoint&				obPointWS,
										CDirection&			obDirFacingWS) const
{	
	
	obPointWS = GetLocalPosition() * obParentTransform;
	obDirFacingWS = CDirection( GetLocalUseFacing() * obParentTransformAffineInverse );
	return;
}

// scee.sbashow : currently simply returns 1.0f or 
	//				0.0f if within UseAngle or without, respectively.

float CUsePoint::DetermineUsabilityFactorFromApproach(const CPoint& 		obPosApproach, 
													  const CDirection& 	obPlaneNormalOfApproachingObject,
													  const CMatrix& 		obParentTransformAffineInverse,
													  const CMatrix& 		obParentTransform) const
{

	if (!HasUseAngle())
	{
		return  1.0f;
	}

	CPoint obUsePointWorldPosition;
	CDirection obUsePointFacingDir;


	GetPosAndFacingNormalWS(obParentTransformAffineInverse,
							obParentTransform,
							obUsePointWorldPosition,
							obUsePointFacingDir);

	const CVector obDrToApproachCharacter = CVector(obPosApproach) - CVector(obUsePointWorldPosition);

	CDirection obDirToApproachCharacter(obDrToApproachCharacter);
	const CDirection obDirFacing(obUsePointFacingDir);


	// scee.sbashow : make sure the vector from the use point to the character has no component along 'obPlaneNormalOfApproachingObject'
	obDirToApproachCharacter-=obPlaneNormalOfApproachingObject*(obPlaneNormalOfApproachingObject.Dot(obDirToApproachCharacter));

	bool bUseable = false;

	const float fDotVal = obDirToApproachCharacter.Dot(obDirFacing);

	if (fDotVal>0.0f)
	{
		// scee.sbashow : ok, make sure 0.0f < m_fAngleOfUseMargin <= 180.0f
		if (m_fCosAngleOFactor>=0.0f)
		{
			bUseable = 
				((fDotVal*fDotVal)>(obDirToApproachCharacter.LengthSquared()*obDirFacing.LengthSquared()*m_fCosAngleOFactor*m_fCosAngleOFactor));
		}
	}
	else
	{
		// scee.sbashow : ok, check for cases where m_fAngleOfUseMargin > 180.0f
		// (this has not been tested btw)
		if (m_fCosAngleOFactor<0.0f)
		{
			bUseable = 
				((fDotVal*fDotVal)<(obDirToApproachCharacter.LengthSquared()*obDirFacing.LengthSquared()*m_fCosAngleOFactor*m_fCosAngleOFactor));
		}
	}

	// scee.sbashow : for now, just return binary 1.0f or 0.0f. 
	//					May have a distribution depending on how 'normalised' with the facing normal later?

	return bUseable? 1.0f : 0.0f;
}


/*
float CUsePoint::DetermineUsabilityFactorFromIsoscelesTrapezoid(...) const

This function performs usability factor checks on a trapezoid-style volume on the
use point.
The first check is a box, allowing for the base-width of the trapezoid outwards.
If the character isn't within that box, then just one cone check (as per the
DetermineUsabilityFactorFromApproach() function) is performed, on the side of the
box that the character is on.

Note: I probably went the long way around with this, there are likely some nifty matrix-math tricks to make this much faster.
- Without all the DEBUGRENDER_TRAPEZOIDCHECK bits it's actually fairly compact except for a few
- normalise calls at the start of the function (during set-up).
Note2: Only use-points with an angle-of-use margin between 0 and 180 degrees are supported.
*/
float CUsePoint::DetermineUsabilityFactorFromIsoscelesTrapezoid(const CPoint &obPosApproach, const CDirection &obPlaneNormalOfApproachingObject,
																const CMatrix& obParentTransformAffineInverse, const CMatrix& obParentTransform,
																float fMaxDistance) const
{
	if (!HasUseAngle())
	{
		return  1.0f;
	}

	//The default return is false.
	bool bUseable = false;

	//Set-up storage for, and retrieve, the world-space position and facing-direction for this use point.
	CPoint obUsePointWorldPosition;
	CDirection obUsePointFacingDir;
	GetPosAndFacingNormalWS(obParentTransformAffineInverse, obParentTransform, obUsePointWorldPosition, obUsePointFacingDir);
	//Immediately push back the use-point world-position (along facing-dir) by however much was specified.
	obUsePointWorldPosition = obUsePointWorldPosition - CPoint(obUsePointFacingDir * m_fZBackwardsOffset);
	obUsePointFacingDir.Normalise();	//Normalise this early on.

	//Calculate the vector from the use point to the character attempting to use it.
	CDirection obDirToApproachCharacter(obPosApproach - obUsePointWorldPosition);

	//Make sure the vector from the use point to the character has no component along 'obPlaneNormalOfApproachingObject'
	//E.G. For characters this basically means removing the y-component from the vector.
	obDirToApproachCharacter -= obPlaneNormalOfApproachingObject*(obPlaneNormalOfApproachingObject.Dot(obDirToApproachCharacter));

	//For the box checks we need normalised values (so that we can retrieve absolute x and y offsets from dot-product etc).
	CDirection obDirToApproachCharNorm(obDirToApproachCharacter); obDirToApproachCharNorm.Normalise();
	//We take the right vector as the cross product between the facing direction and the plane-normal of the approaching object (0,1,0 for chars?)
	CDirection obRightVectorNorm = obUsePointFacingDir.Cross(obPlaneNormalOfApproachingObject); obRightVectorNorm.Normalise();

	const float fDotVal = obDirToApproachCharNorm.Dot(obUsePointFacingDir);		//Used for the box checks.

	//Only supporting 0 <= m_fAngleOfUseMargin <= 180.0f for now.
	if(fDotVal >= 0.0f)
	{
		//We build a 2D representation of this approach vector in CUsePoint space using obUsePointFacingDir as Y.
		//We then check the x and y offsets from the use point to see if the character is within our box-area.
		float fXOffset2D = 0.0f;
		float fYOffset2D = 0.0f;

		float fobDirToApproachLen = obDirToApproachCharacter.Length();
		fXOffset2D = fobDirToApproachLen * acosf(fDotVal);
		//Is this x-offset to the left or right? Adjust accordingly.
		CDirection obLeftVectorNorm = -obRightVectorNorm;
		if(obRightVectorNorm.Dot(obDirToApproachCharNorm) > obLeftVectorNorm.Dot(obDirToApproachCharNorm))
		{
			fXOffset2D = -fXOffset2D;
		}
		fYOffset2D = fobDirToApproachLen * asinf(fDotVal);

#ifdef DEBUGRENDER_TRAPEZOIDCHECK
		//Render the box!
		CDirection obRightForBox = obUsePointFacingDir.Cross(obPlaneNormalOfApproachingObject);
		CPoint obBoxLeftSideStart = obUsePointWorldPosition - CPoint(obRightForBox * m_fBaseHalfWidth);
		CPoint obBoxRightSideStart = obUsePointWorldPosition + CPoint(obRightForBox * m_fBaseHalfWidth);
		CPoint obBoxLeftSideEnd = obBoxLeftSideStart + CPoint(obUsePointFacingDir * fMaxDistance);
		CPoint obBoxRightSideEnd = obBoxRightSideStart + CPoint(obUsePointFacingDir * fMaxDistance);

		g_VisualDebug->RenderLine(obBoxLeftSideStart, obBoxRightSideStart, 0xffff00ff);
		g_VisualDebug->RenderLine(obBoxLeftSideStart, obBoxLeftSideEnd, 0xffff00ff);
		g_VisualDebug->RenderLine(obBoxRightSideStart, obBoxRightSideEnd, 0xffff00ff);
		g_VisualDebug->RenderLine(obBoxRightSideEnd, obBoxLeftSideEnd, 0xffff00ff);
#endif

		//Perform our box check.
		if((fXOffset2D <= m_fBaseHalfWidth) && (fXOffset2D >= -m_fBaseHalfWidth) && (fYOffset2D >= 0.0f) && (fYOffset2D <= fMaxDistance))
		{
			//It's in the box.
			bUseable = true;
		}
		//Only perform one of the two cone checks if the player isn't in the box.
		else
		{
#ifdef DEBUGRENDER_TRAPEZOIDCHECK
			//Draw the "right" vector (should be the opposite of the x-axis or something went wrong!).
			CPoint obPointOnRight = CPoint(obUsePointWorldPosition + CPoint(obRightVectorNorm));
			g_VisualDebug->RenderLine( obUsePointWorldPosition, obPointOnRight, 0xffffffff);

			//Render our facing-dir.
			g_VisualDebug->RenderLine(obUsePointWorldPosition, obUsePointWorldPosition + CPoint(obUsePointFacingDir), 0xffffffff);
#endif

			//Just one cone check, originating from obUsePointWorldPosition +- m_fBaseHalfWidth.
			//We only check the cone-area to the side of the box-check that the player is on.
			if(m_fCosAngleOFactor >= 0.0f)
			{
				CPoint obUsePointWorldPositionAdjusted(CONSTRUCT_CLEAR);
				if(fXOffset2D > 0.0f)	//We only want to check the cone on the left.
				{
					obUsePointWorldPositionAdjusted = CPoint(obUsePointWorldPosition - CPoint(obRightVectorNorm * m_fBaseHalfWidth));
				}
				else	//Only check the cone to the right.
				{
					obUsePointWorldPositionAdjusted = CPoint(obUsePointWorldPosition + CPoint(obRightVectorNorm * m_fBaseHalfWidth));
				}

				CDirection obDirToApproachCharacterAdjusted = CDirection(obPosApproach - obUsePointWorldPositionAdjusted);
				obDirToApproachCharacterAdjusted.Normalise();
				float fDotCone = obDirToApproachCharacterAdjusted.Dot(obUsePointFacingDir);	//used for the cone checks.

#ifdef DEBUGRENDER_TRAPEZOIDCHECK
				//Draw the cone...
				float fRenderXOffset = fMaxDistance * acosf(m_fCosAngleOFactor);
				float fRenderYOffset = fMaxDistance * asinf(m_fCosAngleOFactor);
				//Render the cone sides.
				CPoint obRenderEndPointLeftSide = obUsePointWorldPositionAdjusted - (CPoint(obRightVectorNorm * fRenderXOffset))
					+ CPoint(obUsePointFacingDir * fRenderYOffset);
				CPoint obRenderEndPointRightSide = obUsePointWorldPositionAdjusted + (CPoint(obRightVectorNorm * fRenderXOffset))
					+ CPoint(obUsePointFacingDir * fRenderYOffset);
				g_VisualDebug->RenderLine(obUsePointWorldPositionAdjusted, obRenderEndPointLeftSide, 0xfffff000);
				g_VisualDebug->RenderLine(obUsePointWorldPositionAdjusted, obRenderEndPointRightSide, 0xff0000ff);
#endif
				if(fDotCone > m_fCosAngleOFactor)
				{
					bUseable = true;
				}
			}
		}
	}

	return bUseable ? 1.0f : 0.0f;
}

// scee.sbashow :  - Will add use point code here until someone 
		//			tells me where to place the usepoint cpp and h files in the project

void CUsePoint::OnPostConstruct()
{
	if (m_fAngleOfUseMargin>0.0f)
	{
		// Don't cache into welder exposed variables as they may be output to xml
		float fAngleTemp = m_fAngleOfUseMargin;
		fAngleTemp*=PI/180.0f;
		m_fCosAngleOFactor = cosf(fAngleTemp*0.5f);
		m_obLocalRotationAxis.W()=1.0f;
		m_obLocalFacingNormal.W()=1.0f;
	}
}

bool CUsePoint::IsCharacterFriendly(InteractingCharacterType eCharType)	const 
{
	switch (eCharType)
	{
	case ICT_Undefined:
		ntAssert_p(0, ("Undefined character type attempting to use %s\n", ntStr::GetString ( m_pobIntComponentParent->GetParentEntity()->GetName() ) ));
		return true;
		break;

	case ICT_Archer:
		return m_bArcherCanUse;
		break;
		
	case ICT_Hero:
		return m_bHeroCanUse;	
		break;

	case ICT_Enemy:
		return m_bEnemyAICanUse;	
		break;

	case ICT_Ally:
		return m_bAllyAICanUse;	
		break;

	default:
		ntAssert_p(0, ("Unexpected character type attempting to use %s\n", ntStr::GetString ( m_pobIntComponentParent->GetParentEntity()->GetName() ) ));
		return false;
		break;
	}
	return false;
}

CHashedString  CUsePoint::GetUseAnim(int iCharacterType, 
									 UseMovesState eMoveState) const
{

	if (eMoveState == UP_Walk)
	{
		return m_obAnimWalk[0];
	}

	if (eMoveState == UP_Run)
	{
		return m_obAnimRun[0];
	}

	return CHashedString(HASH_STRING_NULL);
}

#include "game/entityinteractableladder.h"
#include "game/entityinteractablethrown.h"
#include "game/interactiontransitions.h"

void CUsePointAttr::OnPostConstruct()
{

	enum UsePointKeyLocTypes
	{
		UP_Std,
		UP_Top,
		UP_Bot,
		UP_TotalType
	};

	struct KEY_TYPES
	{
		UsePointKeyLocTypes	m_eType;
		const char* pString;
	};

	//scee.sbashow - The use point attribute (which new shared attributes should inherit from) contains a list of identifiers followed by anim types.
	//				So for the ladder, eg, we would have "Top_Walk","Ladder_MountTop","Bot_Walk","Ladder_MountBottom"
	//				This ilist is generated as the hash value of each string, from the object database.

	const KEY_TYPES aobKeyLocTypes[]	=	{	{UP_Std,"Std_"},
												{UP_Top,"Top_"},
												{UP_Bot,"Bot_"}};

#if 0
	//scee.sbashow - the thrown objects are also anim-type sensitive to the following characteristics
	// this is not implemented, but perhaps later. So currently Att_Thrown is not inheriting InteractableParams (which contains a CUsePointAttr instance)
	enum UsePointKeyObjectForms
	{
		UPOF_Normal,
		UPOF_Small,
		UPOF_Big,
		UPOF_Flat,
		UPOF_Handle,
		UPOF_Axey,
		UPOF_Meaty,
		UPOF_TotalType
	};

	struct KEY_FORMS
	{
		UsePointKeyObjectForms	m_eType;
		const char* pString;
	};
	const KEY_FORMS aobKeyObjectForms[]	=	{	{UPOF_Normal,	"Normal_"},
												{UPOF_Small,	"Small_"},
												{UPOF_Big,		"Big_"},
												{UPOF_Flat,		"Flat_"},
												{UPOF_Handle,	"Handle_"},
												{UPOF_Axey,		"Axey_"},
												{UPOF_Meaty,	"Meaty_"}};
#endif
	
	struct KEY_MOVES
	{
		CUsePoint::UseMovesState	m_eMove;
		const char* pString;
	};
	
	const KEY_MOVES aobKeyMoves[]	=	{	{CUsePoint::UP_Walk,"Walk"},
											{CUsePoint::UP_Run,"Run"}};


	CHashedString obCombinedStrings[UP_TotalType][CUsePoint::UP_TotalMove];


	for (int i=0; i<UP_TotalType; i++)
	{
		for (int j=0; j<CUsePoint::UP_TotalMove; j++)
		{
			ntstd::String obType(aobKeyLocTypes[i].pString);
			obType+=ntstd::String(aobKeyMoves[j].pString);

			obCombinedStrings[i][j]=CHashedString(obType.c_str());
		}
	}

	ntstd::List<CHashedString>::const_iterator obItr = 
			m_obUsePointAnimList.begin();

	while(obItr!=m_obUsePointAnimList.end())
	{
	
		bool bFound = false;
		for (int i=0; (!bFound) && i<UP_TotalType; i++)
		{
			for (int j=0; j<CUsePoint::UP_TotalMove; j++)
			{
				if ((*obItr)==obCombinedStrings[i][j])
				{
					obItr++;
					m_obCharUseAnimMap[obCombinedStrings[i][j]] = *obItr++;
					bFound = true;
					break;
				}
			}
		}

		if (!bFound)
		{
			obItr++;
			obItr++;
		}
	} 										
											
											
}


void CUsePoint::RegisterParentAs(CInteractionComponent* pobIntComponentParent)
{
	m_pobIntComponentParent = pobIntComponentParent;

	if ( m_pobIntComponentParent )
	{
		CEntity* pobParent = m_pobIntComponentParent->GetParentEntity();

		if (pobParent->GetEntType()&CEntity::EntType_Interactable)
		{
			Interactable* const pobInteractable =  static_cast<Interactable*>(pobParent);

			const CUsePointAttr* const pUsePointAttrs = 
						pobInteractable->GetUsePointAttributes();

			/*
			if (pUsePointAttrs)
			{
				m_iCharacterInteractingFriendliness = 0;

				if (pUsePointAttrs->ArcherCanUse())
				{
					m_iCharacterInteractingFriendliness |=	CUsePoint::ICT_Archer;
				}

				if (pUsePointAttrs->HeroCanUse())
				{
					m_iCharacterInteractingFriendliness |=	CUsePoint::ICT_Hero;
				}
			}
			else
			{
				m_iCharacterInteractingFriendliness = (((int)CUsePoint::ICT_Archer) |  
													   ((int)CUsePoint::ICT_Hero));
			}
			*/

			
			switch (pobInteractable->GetInteractableType())
			{
				case Interactable::EntTypeInteractable_Ladder:
				{
					Interactable_Ladder* const pobladder = static_cast<Interactable_Ladder*>(pobParent);

					const LadderParameters* const pobLadderParams = 
						ObjectDatabase::Get().GetPointerFromName<LadderParameters*>(pobladder->m_LadderParams);
                    

					if ( GetName() == CHashedString("UsePoint_Top") )
					{
						m_obAnimWalk[0] = 
							m_obAnimRun[0]	 = 
								pobLadderParams->m_pobUsePointAttrs->GetAnim("Top_Walk");
					}
					else
					{
						m_obAnimWalk[0] = 
							m_obAnimRun[0]	 = 
								pobLadderParams->m_pobUsePointAttrs->GetAnim("Bot_Walk");
					}

					m_bCharacterControlToMP = true;
				}
				break;

				case Interactable::EntTypeInteractable_Thrown:
				{
					Interactable_Thrown* const pobThrown = static_cast<Interactable_Thrown*>(pobParent);

					// the old system - Att_Thrown stores move to/ run to as explicit variables
					// convert this to the new scheme by having Att_Thrown : public InteractableParams
					// and copying the example for the ladder (above) or button mash (below), 
					// see xml files for defining anims for eg container name = Ladders in specialobjects.xml

					const Att_Thrown* const pobThrownAtt = pobThrown->GetSharedAttributes();

					m_obAnimWalk[0] = 		pobThrownAtt->m_obAnimPlayerMoveTo;
					m_obAnimRun[0] =	 	pobThrownAtt->m_obAnimPlayerRunTo;
				}
				break;

				case Interactable::EntTypeInteractable_Simple_Usable:
				{
				}
				break;

				case Interactable::EntTypeInteractable_ButtonMash:
				{
					if(pUsePointAttrs)
					{
						// the new system - InteractableParams as a base class that the shared attributes
						// derive from.

						m_obAnimWalk[0] = 		
							pUsePointAttrs->GetAnim("Std_Walk");
						m_obAnimRun[0]	 = 		
							pUsePointAttrs->GetAnim("Std_Run");
						m_bCharacterControlToMP = true;
					}
				}
				break;

				case Interactable::EntTypeInteractable_Spawner:
				{
					m_obAnimWalk[0]  = ((EntityInteractableSpawner*)pobParent)->GetWalkToAnim();
					m_obAnimRun[0]	 = ((EntityInteractableSpawner*)pobParent)->GetRunToAnim();
					m_bCharacterControlToMP = true;
				}
				break;

				default:
				{
				}

			}
		}
	}
}





