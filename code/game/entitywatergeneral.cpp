#include "entitywatergeneral.h"
#include "fsm.h"

#include "game/entitymanager.h"
#include "objectdatabase/dataobject.h"
#include "core/timer.h"
#include "game/attacks.h"
#include "game/awareness.h"	
#include "Physics/system.h"
#include "Physics/compoundlg.h"
#include "physics/world.h"
#include "movement.h"
#include "camera/camman.h"
#include "camera/camview.h"
#include "anim/animator.h"
#include "game/strike.h"
#include "game/randmanager.h"
#include "messagehandler.h"
#include "game/interactioncomponent.h"
#include "core/visualdebugger.h"
#include "targetedtransition.h"
#include "simpletransition.h"
#include "continuationtransition.h"
#include "inputcomponent.h"
#include "camera/camutils.h"
#include "hud/hudmanager.h"
#include "hud/buttonhinter.h"

#include "water/watermanager.h"
#include "water/waterdmadata.h"
#include "water/waterwaveemitter.h"
#include "water/waterinstance.h"
#include "water/waterinstancedef.h"
#include "water/waterbuoyproxy.h"
#include "physics/advancedcharactercontroller.h"

START_STD_INTERFACE(WaterGeneral)
	COPY_INTERFACE_FROM(Boss)
	DEFINE_INTERFACE_INHERITANCE(Boss)

	OVERRIDE_DEFAULT(Description, "boss,watergeneral")
	OVERRIDE_DEFAULT(Clump, "Characters\\watergeneral\\watergeneral.clump")
	OVERRIDE_DEFAULT(CombatDefinition, "WaterGeneral_AttackDefinition")
	OVERRIDE_DEFAULT(AwarenessDefinition, "WaterGeneral_AttackTargetingData")
	OVERRIDE_DEFAULT(AnimationContainer, "WaterGeneralAnimContainer")
	OVERRIDE_DEFAULT(CollisionHeight, "1.21")
	OVERRIDE_DEFAULT(CollisionRadius, "0.6")
	OVERRIDE_DEFAULT(Health, "500")
	OVERRIDE_DEFAULT(InitialAttackPhase, "WaterGeneral_Phase1_Attack")

	PUBLISH_VAR_WITH_DEFAULT_AS(m_aobWaveJumpSpecialStartPoints[0], CPoint(-30.03f, 56.11f, 44.46f), WaveJumpSpecialStartPoints[0])
	PUBLISH_VAR_WITH_DEFAULT_AS(m_aobWaveJumpSpecialStartPoints[1], CPoint(-50.24f, 56.11f, 20.59f), WaveJumpSpecialStartPoints[1])
	PUBLISH_VAR_WITH_DEFAULT_AS(m_aobWaveJumpSpecialStartPoints[2], CPoint( -9.98f, 56.11f, 27.61f), WaveJumpSpecialStartPoints[2])
	PUBLISH_VAR_WITH_DEFAULT_AS(m_aobWaveJumpSpecialStartPoints[3], CPoint(-30.06f, 56.11f,  3.59f), WaveJumpSpecialStartPoints[3])

	PUBLISH_PTR_CONTAINER_AS(m_obSwimToPoints, SwimToPoints)

	DECLARE_POSTPOSTCONSTRUCT_CALLBACK(OnPostPostConstruct)
END_STD_INTERFACE

START_STD_INTERFACE(WaterGeneralSingleWaveLashSpecial)
	COPY_INTERFACE_FROM(BossSpecialAttack)
	DEFINE_INTERFACE_INHERITANCE(BossSpecialAttack)

	PUBLISH_VAR_AS(m_iMaxNumberOfLashes, MaxNumberOfLashes)
	PUBLISH_VAR_AS(m_iMaxNumberOfLashesAdjust, MaxNumberOfLashesAdjust)
	PUBLISH_PTR_AS(m_pobLashStart, LashStart)
	PUBLISH_PTR_AS(m_pobLashCycle, LashCycle)
	PUBLISH_PTR_AS(m_pobLashEnd, LashEnd)
	PUBLISH_PTR_AS(m_pobSwimmyMovement, SwimmyMovement)
	PUBLISH_VAR_AS(m_fPlayerTooCloseToSwimToPointThreshold, PlayerTooCloseToSwimToPointThreshold)

	PUBLISH_VAR_AS(m_obWaveJumpClusterStructureName, WaveJumpClusterStructureName)
	PUBLISH_VAR_AS(m_fInnerDistanceFromWaveToSwapInWaveJumpCluster, InnerDistanceFromWaveToSwapInWaveJumpCluster)
	PUBLISH_VAR_AS(m_fOuterDistanceFromWaveToSwapInWaveJumpCluster, OuterDistanceFromWaveToSwapInWaveJumpCluster)
	IENUM_d		(CAttackData, WaveJumpButtonHint, VIRTUAL_BUTTON_TYPE, AB_NUM)	

	PUBLISH_VAR_AS(m_obWaterInstanceName, WaterInstanceName)
	PUBLISH_VAR_AS(m_aobWaveEmitterNames[0], WaveEmitterName1)
	PUBLISH_VAR_AS(m_aobWaveEmitterNames[1], WaveEmitterName2)
	PUBLISH_VAR_AS(m_aobWaveEmitterNames[2], WaveEmitterName3)
	PUBLISH_VAR_AS(m_aobWaveEmitterNames[3], WaveEmitterName4)
	PUBLISH_VAR_AS(m_aobWaveEmitterNames[4], WaveEmitterName5)
	PUBLISH_VAR_AS(m_aobWaveEmitterNames[5], WaveEmitterName6)
	PUBLISH_VAR_AS(m_aobWaveEmitterNames[MAX_WAVE_LASH_EMITTERS-1], WaveEmitterName7)
END_STD_INTERFACE

START_STD_INTERFACE(WaterGeneralWaveJumpSpecial)
	COPY_INTERFACE_FROM(BossSpecialAttack)
	DEFINE_INTERFACE_INHERITANCE(BossSpecialAttack)

	PUBLISH_PTR_AS(m_pobSwimmyMovement, SwimmyMovement)
	PUBLISH_VAR_AS(m_obWaterfallClimbAnim, WaterfallClimbAnim)
	PUBLISH_VAR_AS(m_obWaterfallJumpAnim, WaterfallJumpAnim)
	PUBLISH_PTR_AS(m_pobLandAttack, LandAttack)

	PUBLISH_VAR_AS(m_obWaterInstanceName, WaterInstanceName)
	PUBLISH_VAR_AS(m_aobWaveEmitterNames[0], WaveEmitterName1)
	PUBLISH_VAR_AS(m_aobWaveEmitterNames[1], WaveEmitterName2)
	PUBLISH_VAR_AS(m_aobWaveEmitterNames[2], WaveEmitterName3)
END_STD_INTERFACE

START_STD_INTERFACE(WaterGeneralSwimAwayTransitioningMovement)
	COPY_INTERFACE_FROM(BossTransitioningMovement)
	DEFINE_INTERFACE_INHERITANCE(BossTransitioningMovement)

	PUBLISH_VAR_AS(m_obSwimStartAnim, SwimStartAnim)
	PUBLISH_VAR_AS(m_obSwimCycleAnim, SwimCycleAnim)
	PUBLISH_VAR_AS(m_obSwimStopAnim, SwimStopAnim)
	PUBLISH_VAR_AS(m_fPlayerTooCloseToSwimToPointThreshold, m_fPlayerTooCloseToSwimToPointThreshold)
END_STD_INTERFACE

START_STD_INTERFACE(WaterGeneralSwimThroughPointsMovement)
	COPY_INTERFACE_FROM(BossTransitioningMovement)
	DEFINE_INTERFACE_INHERITANCE(BossTransitioningMovement)

	PUBLISH_VAR_AS(m_obSwimStartAnim, SwimStartAnim)
	PUBLISH_VAR_AS(m_obSwimCycleAnim, SwimCycleAnim)
	PUBLISH_VAR_AS(m_obSwimCycleLeanLeftAnim, SwimCycleLeanLeftAnim)
	PUBLISH_VAR_AS(m_obSwimCycleLeanRightAnim, SwimCycleLeanRightAnim)
	PUBLISH_VAR_AS(m_obSwim180Anim, Swim180Anim)
	PUBLISH_VAR_AS(m_obSwimStopAnim, SwimStopAnim)
	PUBLISH_VAR_AS(m_fTimeToSpendOnEachPoint, TimeToSpendOnEachPoint)
	PUBLISH_VAR_AS(m_fSpeed, Speed)
	PUBLISH_VAR_AS(m_fProximityThreshold, ProximityThreshold)
	PUBLISH_VAR_AS(m_fMaxRotationPerSecond, MaxRotationPerSecond)
	PUBLISH_VAR_AS(m_fMaxDirectionChange, MaxDirectionChange)
	PUBLISH_VAR_AS(m_fSlowForTurnScalar, SlowForTurnScalar)
	PUBLISH_VAR_AS(m_fMaxSpeedChange, MaxSpeedChange)
	PUBLISH_VAR_AS(m_fLookAhead, LookAhead)
	PUBLISH_VAR_AS(m_iNumPoints, NumPoints)
	PUBLISH_VAR_AS(m_bEndAlignToPlayer, EndAlignToPlayer)
	PUBLISH_VAR_WITH_DEFAULT_AS(m_f180TurnAngleTrigger, 150.0f, 180TurnAngleTrigger)
END_STD_INTERFACE

START_STD_INTERFACE(WaterGeneralSwimToPoint)
	PUBLISH_VAR_WITH_DEFAULT_AS(m_obPoint, CPoint(0.0f, 0.0f, 0.0f), Point)
	PUBLISH_VAR_WITH_DEFAULT_AS(m_fRadius, 1.0f, Radius)
END_STD_INTERFACE

//--------------------------------------------------
//!
//! WaterGeneral 
//! Any specific construction or updating for the water general
//!
//--------------------------------------------------
WaterGeneral::WaterGeneral()
: Boss()
{
	m_eBossType = BT_WATER_GENERAL;

	// Set up wave jump special and swim to point defaults - based on test level
	m_aobWaveJumpSpecialStartPoints[0] = CPoint( -29.58f, 56.1f, 44.98f );
	m_aobWaveJumpSpecialStartPoints[1] = CPoint( -50.34f, 56.11f, 20.5f );
	m_aobWaveJumpSpecialStartPoints[2] = CPoint( -9.98f, 56.11f, 27.61f );
	m_aobWaveJumpSpecialStartPoints[3] = CPoint( -30.06f, 56.11f, 3.59f );
}

WaterGeneral::~WaterGeneral()
{
}

void WaterGeneral::OnPostPostConstruct()
{
	// This is a cheat - I'm assuming start points are opposite each other, in which case the normals facing the waterfall are just vectors to and from each other
	m_aobWaveJumpSpecialStartNormals[0] = CDirection( m_aobWaveJumpSpecialStartPoints[0] - m_aobWaveJumpSpecialStartPoints[1] );
	m_aobWaveJumpSpecialStartNormals[0].Normalise();
	m_aobWaveJumpSpecialStartNormals[1] = CDirection( m_aobWaveJumpSpecialStartPoints[1] - m_aobWaveJumpSpecialStartPoints[0] );
	m_aobWaveJumpSpecialStartNormals[1].Normalise();
	m_aobWaveJumpSpecialStartNormals[2] = CDirection( m_aobWaveJumpSpecialStartPoints[2] - m_aobWaveJumpSpecialStartPoints[3] );
	m_aobWaveJumpSpecialStartNormals[2].Normalise();
	m_aobWaveJumpSpecialStartNormals[3] = CDirection( m_aobWaveJumpSpecialStartPoints[3] - m_aobWaveJumpSpecialStartPoints[2] );
	m_aobWaveJumpSpecialStartNormals[3].Normalise();

	GetAttackComponent()->SetEnableStrikeVolumeCreation(true);
}

bool WaterGeneral::CanStartAnAttack()
{
	return GetAttackComponent()->AI_Access_GetState() == CS_STANDARD;
}

bool WaterGeneral::CanStartALinkedAttack()
{
	return GetAttackComponent()->IsInNextMoveWindow();
}

void WaterGeneral::FinishWave(WaveEmitter* pobEmitter, WaveDma* pobWaveDma, float fWaveSpeed)
{
	#define WAVE_LIFETIME 1.5f

	WaveFinishingData* pobNewWave = NT_NEW_CHUNK( Mem::MC_MISC ) WaveFinishingData;

	pobNewWave->m_pobWaveEmitter = pobEmitter;
	pobNewWave->m_pobWaveDma = pobWaveDma;
	pobNewWave->m_fWaveSpeed = fWaveSpeed;

	pobNewWave->m_fSpeedReductionRate = pobNewWave->m_fWaveSpeed / WAVE_LIFETIME;
	if (pobNewWave->m_pobWaveEmitter->m_Attack0_fWidth > 0)
		pobNewWave->m_fWidthReductionRate = pobNewWave->m_pobWaveEmitter->m_Attack0_fWidth / WAVE_LIFETIME;
	else
		pobNewWave->m_fWidthReductionRate = pobNewWave->m_pobWaveEmitter->m_Attack2_fRadius / WAVE_LIFETIME;
	pobNewWave->m_fHeightReductionRate = pobNewWave->m_pobWaveEmitter->m_fAvgWaveAmplitude / WAVE_LIFETIME;
	pobNewWave->m_fDepthReductionRate = pobNewWave->m_pobWaveEmitter->m_fAvgWavelength / WAVE_LIFETIME;

	m_apobWaves.push_back(pobNewWave);
}

void WaterGeneral::UpdateBossSpecifics(float fTimeDelta)
{    	
	if (m_apobWaves.size() > 0)
	{
		// Go through our possibly active emitters...
		for (unsigned int i = 0; i < m_apobWaves.size(); ++i)
		{
			// And if we have an emitter and valid corresponding transform from an active strike volume...
			if ( m_apobWaves[i]->m_pobWaveEmitter && m_apobWaves[i]->m_pobWaveDma )
			{
				if ( !m_apobWaves[i]->m_pobWaveDma->IsValid() )
				{
					m_apobWaves[i]->m_pobWaveDma->Validate();
				}
				
				// We need to create a new wave with the new position/orientation of the strike volumes transform.
				m_apobWaves[i]->m_pobWaveEmitter->m_obPos += m_apobWaves[i]->m_pobWaveEmitter->m_obAvgWaveDir * m_apobWaves[i]->m_fWaveSpeed * fTimeDelta;
				m_apobWaves[i]->m_pobWaveEmitter->m_Attack2_fRadius += m_apobWaves[i]->m_fWaveSpeed * fTimeDelta;
				m_apobWaves[i]->m_fWaveSpeed -= m_apobWaves[i]->m_fSpeedReductionRate * fTimeDelta;

				m_apobWaves[i]->m_pobWaveEmitter->m_Attack0_fWidth -= m_apobWaves[i]->m_fWidthReductionRate * fTimeDelta;
				m_apobWaves[i]->m_pobWaveEmitter->m_fAvgWaveAmplitude -= m_apobWaves[i]->m_fHeightReductionRate * fTimeDelta; 
				m_apobWaves[i]->m_pobWaveEmitter->m_fAvgWavelength -= m_apobWaves[i]->m_fDepthReductionRate * fTimeDelta;
				
				//ntPrintf("Wave %i: Spd %f, Wid %f, Amp %f, Wln %f.\n", i, m_apobWaves[i]->m_fWaveSpeed, m_apobWaves[i]->m_pobWaveEmitter->m_Attack0_fWidth, m_apobWaves[i]->m_pobWaveEmitter->m_fAvgWaveAmplitude, m_apobWaves[i]->m_pobWaveEmitter->m_fAvgWavelength );

				if (m_apobWaves[i]->m_fWaveSpeed > 0.0f)
				{
					m_apobWaves[i]->m_pobWaveEmitter->CreateWave(m_apobWaves[i]->m_pobWaveDma, true);
				}
				else
				{
					m_apobWaves[i]->m_pobWaveDma->Clear();

					ntstd::Vector<WaveFinishingData*>::iterator it = m_apobWaves.begin();

					while (it != m_apobWaves.end())
					{
						if ((*it) == m_apobWaves[i])
							break;
						++it;
					}

					if ( it != m_apobWaves.end() )
					{
						NT_DELETE_CHUNK( Mem::MC_MISC, m_apobWaves[i] );
						m_apobWaves.erase( it );
						--i;
					}									
				}
			}
		}
	}
}

void WaterGeneral::DebugRenderBossSpecifics( CPoint& obScreenLocation, float fXOffset, float fYOffset )
{
#ifndef _GOLD_MASTER
	g_VisualDebug->Printf3D(m_aobWaveJumpSpecialStartPoints[0],0,7,DC_GREEN,0,"m_aobWaveJumpSpecialStartPoints[0]");
	g_VisualDebug->RenderPoint(m_aobWaveJumpSpecialStartPoints[0],5,DC_GREEN);
	g_VisualDebug->Printf3D(m_aobWaveJumpSpecialStartPoints[1],0,7,DC_GREEN,0,"m_aobWaveJumpSpecialStartPoints[1]");
	g_VisualDebug->RenderPoint(m_aobWaveJumpSpecialStartPoints[1],5.0f,DC_GREEN);
	g_VisualDebug->Printf3D(m_aobWaveJumpSpecialStartPoints[2],0.0f,7,DC_GREEN,0,"m_aobWaveJumpSpecialStartPoints[2]");
	g_VisualDebug->RenderPoint(m_aobWaveJumpSpecialStartPoints[2],5.0f,DC_GREEN);
	g_VisualDebug->Printf3D(m_aobWaveJumpSpecialStartPoints[3],0.0f,7,DC_GREEN,0,"m_aobWaveJumpSpecialStartPoints[3]");
	g_VisualDebug->RenderPoint(m_aobWaveJumpSpecialStartPoints[3],5.0f,DC_GREEN);

	g_VisualDebug->RenderLine(m_aobWaveJumpSpecialStartPoints[0],m_aobWaveJumpSpecialStartPoints[0]+m_aobWaveJumpSpecialStartNormals[0],DC_GREEN);
	g_VisualDebug->RenderLine(m_aobWaveJumpSpecialStartPoints[1],m_aobWaveJumpSpecialStartPoints[1]+m_aobWaveJumpSpecialStartNormals[1],DC_GREEN);
	g_VisualDebug->RenderLine(m_aobWaveJumpSpecialStartPoints[2],m_aobWaveJumpSpecialStartPoints[2]+m_aobWaveJumpSpecialStartNormals[2],DC_GREEN);
	g_VisualDebug->RenderLine(m_aobWaveJumpSpecialStartPoints[3],m_aobWaveJumpSpecialStartPoints[3]+m_aobWaveJumpSpecialStartNormals[3],DC_GREEN);

	int iIndex = 0;
	for (ntstd::List<WaterGeneralSwimToPoint*>::iterator obIt = m_obSwimToPoints.begin();
		obIt != m_obSwimToPoints.end();
		obIt++)
	{
		g_VisualDebug->Printf3D((*obIt)->m_obPoint,0.0f,7,DC_GREEN,0,"m_aobSwimToPoints[%i]",iIndex);
		CMatrix obSurface( CONSTRUCT_IDENTITY );
		obSurface.SetTranslation( (*obIt)->m_obPoint );
		obSurface.SetTranslation( CPoint( obSurface.GetTranslation().X(), obSurface.GetTranslation().Y() + 0.1f, obSurface.GetTranslation().Z() ) );
		g_VisualDebug->RenderArc( obSurface, (*obIt)->m_fRadius, 360.0f, DC_GREEN );

		++iIndex;
	}
#endif
}

CPoint WaterGeneral::GetSwimToPoint(int i)
{
	int iIndex = 0;
	CPoint obPoint(CONSTRUCT_CLEAR);
	float fRadius = 0.0f;
	for (ntstd::List<WaterGeneralSwimToPoint*>::iterator obIt = m_obSwimToPoints.begin();
		obIt != m_obSwimToPoints.end();
		obIt++)
	{
		if (iIndex == i)
		{
			obPoint = (*obIt)->m_obPoint;
			fRadius = (*obIt)->m_fRadius;
		}

		iIndex++;
	}

	float fRandomRadius = grandf(fRadius);
	CDirection obRandomVector( grandf(2.0f)-1.0f , 0.0f ,grandf(2.0f)-1.0f );
	obRandomVector.Normalise();
	obRandomVector *= fRandomRadius;
	obPoint += obRandomVector;

	return obPoint;
}

//--------------------------------------------------
//!
//! WaterGeneralSingleWaveLashSpecial 
//! Water generals special attack where she makes single waves in succession
//!
//--------------------------------------------------
WaterGeneralSingleWaveLashSpecial::WaterGeneralSingleWaveLashSpecial()
{
	m_iLashesRemaining = -1;

	m_iMaxNumberOfLashes = 3;
	m_iMaxNumberOfLashesAdjust = 1;

	m_pobLashStart = m_pobLashCycle = m_pobLashEnd = 0;

	m_bStartLashing = false;

	m_fPlayerTooCloseToSwimToPointThreshold = 2.5f;
}

bool WaterGeneralSingleWaveLashSpecial::IsVulnerableTo( CStrike* pobStrike )
{
	if  (!m_pobSwimmyMovement->IsDone())
		return m_pobSwimmyMovement->GetVulnerableDuring();
	else
		return true;
}

bool WaterGeneralSingleWaveLashSpecial::IsVulnerableTo( const CAttackData* pobAttackData )
{
	if  (!m_pobSwimmyMovement->IsDone())
		return m_pobSwimmyMovement->GetVulnerableDuring();
	else
		return true;
}

bool WaterGeneralSingleWaveLashSpecial::Initialise(Boss* pobBoss, CEntity* pobPlayer)
{
	WaterGeneral* pobWGen = (WaterGeneral*)pobBoss;

	pobWGen->GetBossMovement()->m_obTargetPoint = pobPlayer->GetPosition();
	pobWGen->GetBossMovement()->m_bTargetPointSet = true;

	m_iLashesRemaining = m_iMaxNumberOfLashes;
	m_iLashesRemaining -= grand() % (m_iMaxNumberOfLashesAdjust + 1);
	if (m_iLashesRemaining <= 0)
		m_iLashesRemaining = 1;
	
	// Kick off with a swim
	CPoint obWinnerSwimToPoint( CONSTRUCT_CLEAR );
	CDirection obWinnerSwimToVector( CONSTRUCT_CLEAR );
	float fWinnerDistanceSquared = 10000000.0f;
	float fWinnerDistanceFromPlayerSquared = 10000000.0f;
	int iWinnerIndex = -1;
	for (int i = 0; i < pobWGen->GetNumberOfSwimToPoints(); i++)
	{
		CPoint obSwimToPoint = pobWGen->GetSwimToPoint(i);
		CDirection obSwimToVector = CDirection( obSwimToPoint - pobWGen->GetPosition() );
		CDirection obSwimToPlayer = CDirection( obSwimToPoint - pobPlayer->GetPosition() );
		float fDistanceSquared = obSwimToVector.LengthSquared();
		float fDistanceFromPlayerSquared = obSwimToPlayer.LengthSquared();
		// Any closer
		if (fDistanceSquared < fWinnerDistanceSquared && fDistanceFromPlayerSquared > m_fPlayerTooCloseToSwimToPointThreshold*m_fPlayerTooCloseToSwimToPointThreshold && fDistanceSquared > m_fPlayerTooCloseToSwimToPointThreshold*m_fPlayerTooCloseToSwimToPointThreshold)
		{
			fWinnerDistanceSquared = obSwimToVector.LengthSquared();
			fWinnerDistanceFromPlayerSquared = fDistanceFromPlayerSquared;
			obWinnerSwimToPoint = obSwimToPoint;
			obWinnerSwimToVector = obSwimToVector;
			iWinnerIndex = i;
		}
	}

	ntError( iWinnerIndex != -1 );

	m_pobSwimmyMovement->Initialise(pobBoss,pobPlayer);
	m_pobSwimmyMovement->SetEndPoint(obWinnerSwimToPoint);

	m_bStartLashing = false;

	// Prepare some water vars for usage
	if (m_obWaterInstanceName.IsNull())
		m_pobWaterInstance = 0;
	else
        m_pobWaterInstance = WaterManager::Get().GetWaterInstance(m_obWaterInstanceName);

	m_iNextWaveEmmiterToUse = 0;

	for ( int i = 0; i < MAX_WAVE_LASH_EMITTERS; ++i )
	{
		m_apobWaveEmitterStrikeVolumes[i] = 0;

		if ( m_pobWaterInstance )
		{
			m_apobWaveEmitters[i] = ObjectDatabase::Get().GetPointerFromName<WaveEmitter*>( m_aobWaveEmitterNames[i] );
			if ( m_apobWaveEmitters[i] )
			{
				//m_apobWaveEmitters[i]->InstallParentWaterInstance( m_pobWaterInstance );
				m_apobWaveDmas[i] = m_pobWaterInstance->GetFirstAvailableWaveSlot();
				if (m_apobWaveDmas[i])
					m_apobWaveDmas[i]->Validate();
			}
			else
				m_apobWaveDmas[i] = 0;
		}
		else
		{
			m_apobWaveEmitters[i] = 0;
			m_apobWaveDmas[i] = 0;
		}
	}

	return true;
}

void WaterGeneralSingleWaveLashSpecial::NotifyCreatedStrikeVolume(CombatPhysicsStrikeVolume* pobVol)
{
	// Take a pointer to the transform so we can start a wave and keep it up to date
	if (m_iNextWaveEmmiterToUse < MAX_WAVE_LASH_EMITTERS && 
		m_apobWaveEmitters[m_iNextWaveEmmiterToUse] && 
		m_apobWaveDmas[m_iNextWaveEmmiterToUse] && 
		!m_apobWaveEmitterStrikeVolumes[m_iNextWaveEmmiterToUse])
	{
		m_apobWaveEmitterStrikeVolumes[m_iNextWaveEmmiterToUse] = pobVol;
		m_iNextWaveEmmiterToUse++;
	}
}

void WaterGeneralSingleWaveLashSpecial::NotifyRemovedStrikeVolume(CombatPhysicsStrikeVolume* pobVol)
{
	// Find the wave index
	for (int i = 0; i < MAX_WAVE_LASH_EMITTERS; ++i)
	{
		if (m_apobWaveEmitterStrikeVolumes[i] == pobVol)
		{
			// Null out the transform so we stop updating it
			m_apobWaveEmitterStrikeVolumes[i] = 0;
		}
	}
}

void WaterGeneralSingleWaveLashSpecial::NotifyAttackFinished()
{
	m_iLashesRemaining--;
}

void WaterGeneralSingleWaveLashSpecial::NotifyAttackAutoLinked()
{
	m_iLashesRemaining--;
}

void WaterGeneralSingleWaveLashSpecial::NotifyMovementDone()
{
	if (!m_pobSwimmyMovement->IsDone())
		m_pobSwimmyMovement->NotifyMovementDone();

	m_bStartLashing = true;
}

BossAttack* WaterGeneralSingleWaveLashSpecial::Update(float fTimeDelta, Boss* pobBoss, CEntity* pobPlayer)
{
	// If we've got water...
	if (m_pobWaterInstance)
	{
		// Go through our possibly active emitters...
		for (int i = 0; i < m_iNextWaveEmmiterToUse && i < MAX_WAVE_LASH_EMITTERS; ++i)
		{
			// And if we have an emitter and valid corresponding transform from an active strike volume...
			if ( m_apobWaveEmitters[i] && m_apobWaveEmitterStrikeVolumes[i] && m_apobWaveDmas[i] )
			{
				if ( !m_apobWaveDmas[i]->IsValid() )
				{
					m_apobWaveDmas[i]->Validate();
				}

				// We need to create a new wave with the new position/orientation of the strike volumes transform.
				m_apobWaveEmitters[i]->m_obPos = m_apobWaveEmitterStrikeVolumes[i]->GetTransform()->GetWorldMatrix().GetTranslation() * m_pobWaterInstance->GetWorldToWaterMatrix();
				
				m_apobWaveEmitters[i]->m_Attack0_fWidth = m_apobWaveEmitterStrikeVolumes[i]->GetWidth();
				m_apobWaveEmitters[i]->m_Attack2_fRadius = 0; // This needs to be nulltastic so we do'nt get confused as to what kind of wave it is when we pass it to the wgen to fade out
				m_apobWaveEmitters[i]->m_fAvgWaveAmplitude = m_apobWaveEmitterStrikeVolumes[i]->GetHeight() * 0.5f; // Height / 2 cos water is quite far off the ground
				m_apobWaveEmitters[i]->m_fAvgWavelength = m_apobWaveEmitterStrikeVolumes[i]->GetDepth();
				
				m_apobWaveEmitters[i]->m_obAvgWaveDir = m_apobWaveEmitterStrikeVolumes[i]->GetTransform()->GetWorldMatrix().GetZAxis() * m_pobWaterInstance->GetWorldToWaterMatrix();
				m_afWaveDisplacementsLastFrame[i] = m_apobWaveEmitterStrikeVolumes[i]->GetDisplacementLastFrame().Length();
				m_apobWaveEmitters[i]->CreateWave(m_apobWaveDmas[i], true);
			}
			else if ( m_apobWaveEmitters[i] && m_apobWaveDmas[i] )
			{
				// We've lost a strike volume so need to finish the wave
				WaterGeneral* pobWGen = (WaterGeneral*)pobBoss;
				pobWGen->FinishWave(m_apobWaveEmitters[i], m_apobWaveDmas[i], m_afWaveDisplacementsLastFrame[i] * (1.0f / fTimeDelta) );			
				m_apobWaveEmitters[i] = 0;
				m_apobWaveDmas[i] = 0;
			}
		}
	}

	if (m_pobSwimmyMovement->IsDone())
	{
		pobBoss->GetBossMovement()->m_obTargetPoint = pobPlayer->GetPosition();
		pobBoss->GetBossMovement()->m_bTargetPointSet = true;

		if (m_bStartLashing && pobBoss->GetAttackComponent()->Boss_Command_RequestDirectAttack(m_pobLashStart))
		{			
			pobBoss->GetMessageHandler()->ReceiveMsg<msg_buttonattack>();
			m_bStartLashing = false;
        }

		CPoint obPlayerPosition(pobPlayer->GetPosition());
		float fDistance = pobBoss->GetAttackComponent()->GetDistanceToClosestStrikeVolume(obPlayerPosition);
		if ( fDistance < m_fOuterDistanceFromWaveToSwapInWaveJumpCluster &&
			fDistance > m_fInnerDistanceFromWaveToSwapInWaveJumpCluster )
		{
			if (!pobPlayer->GetAttackComponent()->GetLeadClusterChanged())
			{
				// Swap in cluster with jump in it
				DataObject* pobDOB = ObjectDatabase::Get().GetDataObjectFromName(m_obWaveJumpClusterStructureName);

				if (pobDOB && strcmp(pobDOB->GetClassName(),"CClusterStructure") == 0)
				{
					CAttackComponent* pobAttack = const_cast< CAttackComponent* >(pobPlayer->GetAttackComponent());
					pobAttack->ChangeLeadClusterTo((CClusterStructure*)pobDOB->GetBasePtr());
				}
				else
				{
					ntPrintf("Failed to change lead cluster! Was the name %s right?\n",m_obWaveJumpClusterStructureName.GetDebugString() );
				}
			}

			// Render a button hint for it
			if (m_eWaveJumpButtonHint != AB_NUM && CHud::Get().GetCombatHUDElements()->m_pobButtonHinter)
				CHud::Get().GetCombatHUDElements()->m_pobButtonHinter->SetHintForButton(m_eWaveJumpButtonHint, true);
		}
		else
		{
			CAttackComponent* pobAttack = const_cast< CAttackComponent* >(pobPlayer->GetAttackComponent());
			pobAttack->ResetLeadCluster();

			if (m_eWaveJumpButtonHint != AB_NUM && CHud::Get().GetCombatHUDElements()->m_pobButtonHinter)
				CHud::Get().GetCombatHUDElements()->m_pobButtonHinter->SetHintForButton(m_eWaveJumpButtonHint, false);
		}

		if (m_iLashesRemaining < 0 && pobBoss->CanStartAnAttack())
		{
			// We've finished
			for (int i = 0; i < MAX_WAVE_LASH_EMITTERS; ++i)
			{
				if ( m_apobWaveEmitterStrikeVolumes[i] && m_apobWaveDmas[i] )
				{
					CAttackComponent* pobAttack = const_cast< CAttackComponent* >(pobPlayer->GetAttackComponent());
					pobAttack->ResetLeadCluster();

					if (m_eWaveJumpButtonHint != AB_NUM && CHud::Get().GetCombatHUDElements()->m_pobButtonHinter)
						CHud::Get().GetCombatHUDElements()->m_pobButtonHinter->SetHintForButton(m_eWaveJumpButtonHint, false);

					WaterGeneral* pobWGen = (WaterGeneral*)pobBoss;

					pobWGen->FinishWave(m_apobWaveEmitters[i], m_apobWaveDmas[i], m_afWaveDisplacementsLastFrame[i] * (1.0f / fTimeDelta) );

					// Destroy volume and wave
					m_apobWaveEmitterStrikeVolumes[i]->DestroyNow();
				}
			}

			return 0;
		}

		if (m_iLashesRemaining == 0)
		{
			if (pobBoss->GetAttackComponent()->Boss_Command_RequestDirectAttack(m_pobLashEnd,true))
			{			
				pobBoss->GetMessageHandler()->ReceiveMsg<msg_buttonattack>();

				m_iLashesRemaining--;
			}
		}
	}
	else
	{
		m_pobSwimmyMovement->DoMovement(fTimeDelta,pobBoss,pobPlayer);
	}

	return this;
}

void WaterGeneralSingleWaveLashSpecial::DebugRender(CPoint& obScreenLocation, float fXOffset, float fYOffset)
{
#ifndef _GOLD_MASTER
	g_VisualDebug->Printf3D(obScreenLocation,fXOffset,fYOffset,DC_CYAN,0,"Single Wave Lash Special %s: %i lashes remaining", ObjectDatabase::Get().GetNameFromPointer(this).GetString(), m_iLashesRemaining );
#endif
}

//--------------------------------------------------
//!
//! WaterGeneralWaveJumpSpecial 
//! Water generals special attack where she makes a big wave for hero to jump over
//!
//--------------------------------------------------
WaterGeneralWaveJumpSpecial::WaterGeneralWaveJumpSpecial()
{
	m_pobLandAttack = 0;
}

bool WaterGeneralWaveJumpSpecial::Initialise(Boss* pobBoss, CEntity* pobPlayer)
{
	WaterGeneral* pobWGen = (WaterGeneral*)pobBoss;
	
	// Kick off movement to a waterfall point, find the closest one and go for it
	CPoint obWinnerWaterfallPoint( CONSTRUCT_CLEAR );
	CDirection obWinnerWaterfallVector( CONSTRUCT_CLEAR );
	float fWinnerDistanceFromPlayerSquared = -1.0f;
	int iWinnerIndex = -1;
	for (int i = 0; i < pobWGen->GetNumberOfWaterfallPoints(); i++)
	{
		CPoint obWaterfallPoint = pobWGen->GetWaterfallPoint(i);
		CDirection obWaterfallVector = CDirection( obWaterfallPoint - pobWGen->GetPosition() );
		CDirection obWaterfallToPlayer = CDirection( obWaterfallPoint - pobPlayer->GetPosition() );
		float fDistanceFromPlayerSquared = obWaterfallToPlayer.LengthSquared();
		if (fDistanceFromPlayerSquared > fWinnerDistanceFromPlayerSquared)
		{
			fWinnerDistanceFromPlayerSquared = fDistanceFromPlayerSquared;
			obWinnerWaterfallPoint = obWaterfallPoint;
			obWinnerWaterfallVector = obWaterfallVector;
			iWinnerIndex = i;
		}
	}

	ntError( iWinnerIndex != -1 );
	CDirection obWinnerWaterfallNormal = pobWGen->GetWaterfallNormal(iWinnerIndex);

	m_pobSwimmyMovement->Initialise(pobBoss,pobPlayer);
	m_pobSwimmyMovement->SetEndPoint(obWinnerWaterfallPoint);
	m_pobSwimmyMovement->SetEndAlignToZ(obWinnerWaterfallNormal);

	m_bStartAttack = false;
	m_bClimbStarted = false;
	m_bAttackStarted = false;

	// Prepare some water vars for usage
	m_pobWaterInstance = WaterManager::Get().GetWaterInstance(m_obWaterInstanceName);
	m_iNextWaveEmmiterToUse = 0;

	for ( int i = 0; i < MAX_WAVE_JUMP_EMITTERS; ++i )
	{
		m_apobWaveEmitterStrikeVolumes[i] = 0;

		if ( m_pobWaterInstance )
		{
			m_apobWaveEmitters[i] = ObjectDatabase::Get().GetPointerFromName<WaveEmitter*>( m_aobWaveEmitterNames[i] );
			if ( m_apobWaveEmitters[i] )
			{
				//m_apobWaveEmitters[i]->InstallParentWaterInstance( m_pobWaterInstance );
				m_apobWaveDmas[i] = m_pobWaterInstance->GetFirstAvailableWaveSlot();
				m_apobWaveDmas[i]->Validate();
			}
			else
				m_apobWaveDmas[i] = 0;
		}
		else
		{
			m_apobWaveEmitters[i] = 0;
			m_apobWaveDmas[i] = 0;
		}
	}

	return true;
}

void WaterGeneralWaveJumpSpecial::NotifyCreatedStrikeVolume(CombatPhysicsStrikeVolume* pobVol)
{
	// Take a pointer to the transform so we can start a wave and keep it up to date
	if (m_iNextWaveEmmiterToUse < MAX_WAVE_JUMP_EMITTERS && 
		m_apobWaveEmitters[m_iNextWaveEmmiterToUse] && 
		m_apobWaveDmas[m_iNextWaveEmmiterToUse] && 
		!m_apobWaveEmitterStrikeVolumes[m_iNextWaveEmmiterToUse])
	{
		m_apobWaveEmitterStrikeVolumes[m_iNextWaveEmmiterToUse] = pobVol;
		m_iNextWaveEmmiterToUse++;
	}
}

void WaterGeneralWaveJumpSpecial::NotifyRemovedStrikeVolume(CombatPhysicsStrikeVolume* pobVol)
{
	// Find the wave index
	for (int i = 0; i < MAX_WAVE_JUMP_EMITTERS; ++i)
	{
		if (m_apobWaveEmitterStrikeVolumes[i] == pobVol)
		{
			// Null out the transform so we stop updating it
			m_apobWaveEmitterStrikeVolumes[i] = 0;
		}
	}
}

void WaterGeneralWaveJumpSpecial::NotifyAttackFinished()
{
}

void WaterGeneralWaveJumpSpecial::NotifyMovementDone()
{
	if (!m_pobSwimmyMovement->IsDone())
		m_pobSwimmyMovement->NotifyMovementDone();

	if (m_bClimbStarted)
		m_bStartAttack = true;
}

BossAttack* WaterGeneralWaveJumpSpecial::Update(float fTimeDelta, Boss* pobBoss, CEntity* pobPlayer)
{
	// If we've got water...
	if (m_pobWaterInstance)
	{
		// Go through our possibly active emitters...
		for (int i = 0; i < m_iNextWaveEmmiterToUse && i < MAX_WAVE_JUMP_EMITTERS; ++i)
		{
			// And if we have an emitter and valid corresponding transform from an active strike volume...
			if ( m_apobWaveEmitters[i] && m_apobWaveEmitterStrikeVolumes[i] && m_apobWaveDmas[i] )
			{
				if ( !m_apobWaveDmas[i]->IsValid() )
				{
					m_apobWaveDmas[i]->Validate();
				}

				// We need to create a new wave with the new position/orientation of the strike volumes transform.
				m_apobWaveEmitters[i]->m_obPos = m_apobWaveEmitterStrikeVolumes[i]->GetTransform()->GetWorldMatrix().GetTranslation() * m_pobWaterInstance->GetWorldToWaterMatrix();
				
				m_apobWaveEmitters[i]->m_Attack2_fRadius = m_apobWaveEmitterStrikeVolumes[i]->GetWidth();
				m_apobWaveEmitters[i]->m_Attack0_fWidth = 0; // This needs to be nulltastic so we do'nt get confused as to what kind of wave it is when we pass it to the wgen to fade out
				m_apobWaveEmitters[i]->m_fAvgWaveAmplitude = m_apobWaveEmitterStrikeVolumes[i]->GetHeight();
				m_apobWaveEmitters[i]->m_fAvgWavelength = m_apobWaveEmitterStrikeVolumes[i]->GetDepth();
				
				m_apobWaveEmitters[i]->CreateWave(m_apobWaveDmas[i], true);
			}
		}
	}

	if ( m_pobSwimmyMovement->IsDone() )
	{		
		// Time to climb?
		if (!m_bClimbStarted)
		{
			SimpleTransitionDef obClimbDef;
			obClimbDef.SetDebugNames("WGen Climb","SimpleTransitionDef");
			obClimbDef.m_bApplyGravity = false;
			obClimbDef.m_bLooping = false;
			obClimbDef.m_obAnimationName = m_obWaterfallClimbAnim;

			SimpleTransitionDef obJumpDef;
			obJumpDef.SetDebugNames("WGen Jump","SimpleTransitionDef");
			obJumpDef.m_bApplyGravity = false;
			obJumpDef.m_bLooping = false;
			obJumpDef.m_obAnimationName = m_obWaterfallJumpAnim;

			//pobBoss->GetMovement()->ClearControllers();
			pobBoss->GetMovement()->AddChainedController( obClimbDef, CMovement::DMM_STANDARD, CMovement::MOVEMENT_BLEND );
			pobBoss->GetMovement()->AddChainedController( obJumpDef, CMovement::DMM_STANDARD, CMovement::MOVEMENT_BLEND );
			
			Message obMovementMessage(msg_movementdone);
			pobBoss->GetMovement()->SetCompletionMessage(obMovementMessage,pobBoss);

			m_bClimbStarted = true;
		}
		// Time to attack?
		else if (m_bStartAttack && pobBoss->CanStartAnAttack() && !m_bAttackStarted)
		{
			if (pobBoss->GetAttackComponent()->Boss_Command_RequestDirectAttack(m_pobLandAttack))
			{			
				pobBoss->GetMessageHandler()->ReceiveMsg<msg_buttonattack>();
				m_bAttackStarted = true;
			}
		}
		// Time to manage the attack?
		else if (m_bAttackStarted && m_bClimbStarted && m_bStartAttack && !pobBoss->CanStartAnAttack())
		{
			// Errr... boobies?
		}
		else if (m_bAttackStarted && m_bClimbStarted && m_bStartAttack && pobBoss->CanStartAnAttack())
		{			
			// We've finished
			WaterGeneral* pobWGen = (WaterGeneral*)pobBoss;
			for (int i = 0; i < MAX_WAVE_JUMP_EMITTERS; ++i)
			{
				if ( m_apobWaveEmitterStrikeVolumes[i] )
				{
					// Destroy volume
					m_apobWaveEmitterStrikeVolumes[i]->DestroyNow();
				}

				if ( m_apobWaveDmas[i] )
				{
					pobWGen->FinishWave(m_apobWaveEmitters[i], m_apobWaveDmas[i], m_apobWaveEmitterStrikeVolumes[i]->GetDisplacementLastFrame().Length() * (1.0f / fTimeDelta) );
				}
			}

			return 0;		
		}
	}
	else
	{
		m_pobSwimmyMovement->DoMovement(fTimeDelta,pobBoss,pobPlayer);
	}

	return this;
}

void WaterGeneralWaveJumpSpecial::DebugRender(CPoint& obScreenLocation, float fXOffset, float fYOffset)
{
#ifndef _GOLD_MASTER
	if (!m_pobSwimmyMovement->IsDone())
		g_VisualDebug->Printf3D(obScreenLocation,fXOffset,fYOffset,DC_CYAN,0,"Wave Jump Special %s: waiting for movements to complete", ObjectDatabase::Get().GetNameFromPointer(this).GetString() );
	else
		g_VisualDebug->Printf3D(obScreenLocation,fXOffset,fYOffset,DC_CYAN,0,"Wave Jump Special %s: attacking", ObjectDatabase::Get().GetNameFromPointer(this).GetString() );
#endif
}

//--------------------------------------------------
//!
//! WaterGeneralSwimAwayTransitioningMovement 
//!
//--------------------------------------------------
BossMovement* WaterGeneralSwimAwayTransitioningMovement::Initialise(Boss* pobBoss, CEntity* pobPlayer)
{
	// Assumes we're ok to do this...
	WaterGeneral* pobWGen = (WaterGeneral*)pobBoss;

	CPoint obWinnerSwimToPoint( CONSTRUCT_CLEAR );
	CDirection obWinnerSwimToVector( CONSTRUCT_CLEAR );
	float fWinnerDistanceSquared = 10000000.0f;
	float fWinnerDistanceFromPlayerSquared = 10000000.0f;
	int iWinnerIndex = -1;
	for (int i = 0; i < pobWGen->GetNumberOfSwimToPoints(); i++)
	{
		CPoint obSwimToPoint = pobWGen->GetSwimToPoint(i);
		CDirection obSwimToVector = CDirection( obSwimToPoint - pobWGen->GetPosition() );
		CDirection obSwimToPlayer = CDirection( obSwimToPoint - pobPlayer->GetPosition() );
		float fDistanceSquared = obSwimToVector.LengthSquared();
		float fDistanceFromPlayerSquared = obSwimToPlayer.LengthSquared();
		// Any closer
		if (fDistanceSquared < fWinnerDistanceSquared && fDistanceFromPlayerSquared > m_fPlayerTooCloseToSwimToPointThreshold*m_fPlayerTooCloseToSwimToPointThreshold && fDistanceSquared > m_fPlayerTooCloseToSwimToPointThreshold*m_fPlayerTooCloseToSwimToPointThreshold)
		{
			fWinnerDistanceSquared = obSwimToVector.LengthSquared();
			fWinnerDistanceFromPlayerSquared = fDistanceFromPlayerSquared;
			obWinnerSwimToPoint = obSwimToPoint;
			obWinnerSwimToVector = obSwimToVector;
			iWinnerIndex = i;
		}
	}

	ntError( iWinnerIndex != -1 );

	ZAxisAlignTargetedTransitionDef obSwimStartAlignDef;
	obSwimStartAlignDef.SetDebugNames("WGen Swim Start","ZAxisAlignTargetedTransitionDef");
	obSwimStartAlignDef.m_bApplyGravity = true;
	obSwimStartAlignDef.m_obAlignZTo = CDirection( obWinnerSwimToPoint - pobWGen->GetPosition() );
	obSwimStartAlignDef.m_obAlignZTo.Normalise();
	obSwimStartAlignDef.m_obAnimationName = m_obSwimStartAnim;

	TargetedTransitionToPointDef obSwimCycle;
	obSwimCycle.SetDebugNames("WGen Swim Continue","TargetedTransitionToPointDef");
	obSwimCycle.m_bApplyGravity = true;
	obSwimCycle.m_fExtraSpeed = m_fSpeed;
	obSwimCycle.m_obAnimationName = m_obSwimCycleAnim;
	obSwimCycle.m_obPoint = obWinnerSwimToPoint;

	ZAxisAlignTargetedTransitionDef obSwimEndAlignDef;
	obSwimEndAlignDef.SetDebugNames("WGen Swim End","ZAxisAlignTargetedTransitionDef");
	obSwimEndAlignDef.m_bApplyGravity = true;
	obSwimEndAlignDef.m_obAlignZTo = CDirection( pobWGen->GetPosition() - obWinnerSwimToPoint );
	obSwimEndAlignDef.m_obAnimationName = m_obSwimStopAnim;

	pobWGen->GetMovement()->BringInNewController( obSwimStartAlignDef, CMovement::DMM_STANDARD, CMovement::MOVEMENT_BLEND );
	pobWGen->GetMovement()->AddChainedController( obSwimCycle, CMovement::DMM_STANDARD, 0.15f );
	pobWGen->GetMovement()->AddChainedController( obSwimEndAlignDef, CMovement::DMM_STANDARD, CMovement::MOVEMENT_BLEND );

	Message obMovementMessage(msg_movementdone);
	pobWGen->GetMovement()->SetCompletionMessage(obMovementMessage,pobBoss);

	ntPrintf("Excluding.\n");
	pobWGen->GetInteractionComponent()->ExcludeCollisionWith(pobPlayer);

	m_bDone = false;

	return this;
}

BossMovement* WaterGeneralSwimAwayTransitioningMovement::DoMovement(float fTimeDelta, Boss* pobBoss, CEntity* pobPlayer)
{
	m_fTimeInMovement += fTimeDelta;
	if (!m_bDone)
		return this;
	else
	{
		ntPrintf("Allowing.\n");
		pobBoss->GetInteractionComponent()->AllowCollisionWith(pobPlayer);
		return 0;
	}
}

BossMovement* WaterGeneralSwimThroughPointsMovement::Initialise(Boss* pobBoss, CEntity* pobPlayer)
{
	ntError( m_iNumPoints < 25 );

	WaterGeneral* pobWGen = (WaterGeneral*)pobBoss;
	
	// When we start swimming, we turn towards the waterfall point
	ZAxisAlignTargetedTransitionDef obSwimStartAlignDef;
	obSwimStartAlignDef.SetDebugNames("WGen Swim Start","ZAxisAlignTargetedTransitionDef");
	obSwimStartAlignDef.m_bApplyGravity = true;
	obSwimStartAlignDef.m_obAlignZTo = pobWGen->GetMatrix().GetZAxis();
	obSwimStartAlignDef.m_obAlignZTo.Normalise();
	obSwimStartAlignDef.m_obAnimationName = m_obSwimStartAnim;

	// While we're swimming we continue till we get there, always facing towards it
	InputFollowingTransitionDef obSwimCycle;
	obSwimCycle.SetDebugNames("WGen Swim Continue","InputFollowingTransitionDef");
	obSwimCycle.m_bApplyGravity = true;
	obSwimCycle.m_fExtraSpeed = m_fSpeed;
	obSwimCycle.m_obAnimName = m_obSwimCycleAnim;
	obSwimCycle.m_obLeanLeftAnimName = m_obSwimCycleLeanLeftAnim;
	obSwimCycle.m_obLeanRightAnimName = m_obSwimCycleLeanRightAnim;
	obSwimCycle.m_fMaxRotationPerSecond = m_fMaxRotationPerSecond * DEG_TO_RAD_VALUE;
	obSwimCycle.m_fMaxDirectionChange = m_fMaxDirectionChange * DEG_TO_RAD_VALUE;
	obSwimCycle.m_fSlowForTurnScalar = m_fSlowForTurnScalar;
	obSwimCycle.m_fMaxSpeedChange = m_fMaxSpeedChange;

	pobWGen->GetMovement()->BringInNewController( obSwimStartAlignDef, CMovement::DMM_STANDARD, CMovement::MOVEMENT_BLEND );
	pobWGen->GetMovement()->AddChainedController( obSwimCycle, CMovement::DMM_STANDARD, CMovement::MOVEMENT_BLEND );

	for (int i = 0; i < m_iNumPoints; i++)
	{
		m_aobPathPoints[i] = pobWGen->GetSwimToPoint(i%pobWGen->GetNumberOfSwimToPoints());
	}

	m_bEndAlignToZSet = false;
	m_fTimeOnCurrentPoint = 0.0f;
	m_iCurrentPoint = 0;
	m_bDone = false;
	m_bEndStarted = false;
	m_bDone180OnCurrentPoint = false;

	ntPrintf("Excluding.\n");
	pobWGen->GetInteractionComponent()->ExcludeCollisionWith(pobPlayer);

	return this;
}

void WaterGeneralSwimThroughPointsMovement::SetEndPoint(CPoint& obPoint)
{
	m_aobPathPoints[m_iNumPoints-1] = obPoint;
}

void WaterGeneralSwimThroughPointsMovement::SetEndAlignToZ(CDirection& obAlignTo)
{
	m_bEndAlignToZSet = true;
	m_obEndAlignToZ = obAlignTo;
}

void WaterGeneralSwimThroughPointsMovement::NotifyMovementDone() 
{ 
	m_bDone = true; 
}

BossMovement* WaterGeneralSwimThroughPointsMovement::DoMovement(float fTimeDelta, Boss* pobBoss, CEntity* pobPlayer)
{
	m_fTimeOnCurrentPoint += fTimeDelta;

	WaterGeneral* pobWGen = (WaterGeneral*)pobBoss;
	
	CDirection obVector( m_aobPathPoints[m_iCurrentPoint] - pobWGen->GetPosition() );
	float fLength = obVector.Length();
	if (fLength < m_fProximityThreshold || (m_fTimeOnCurrentPoint > m_fTimeToSpendOnEachPoint && m_iCurrentPoint != m_iNumPoints-1))
	{
		m_bDone180OnCurrentPoint = false;
		m_iCurrentPoint++;
		m_fTimeOnCurrentPoint = 0.0f;
		return this->DoMovement(fTimeDelta,pobBoss,pobPlayer);
	}
	else if (m_iCurrentPoint < m_iNumPoints)
	{
		//g_VisualDebug->RenderPoint(m_aobPathPoints[m_iCurrentPoint],10.0f,DC_PURPLE);

		float fLookAhead = fLength;
		fLookAhead > m_fLookAhead ? fLookAhead = m_fLookAhead : fLookAhead = fLookAhead;
		CDirection obFacing = pobWGen->GetMatrix().GetZAxis();
		obVector.Normalise();
		float fAngle = MovementControllerUtilities::RotationAboutY(obFacing, obVector);
		if (!m_bDone180OnCurrentPoint && !m_obSwim180Anim.IsNull() && fabs(fAngle * RAD_TO_DEG_VALUE) > m_f180TurnAngleTrigger)
		{
			m_bDone180OnCurrentPoint = true;

			SimpleTransitionDef obTurn;
			obTurn.m_obAnimationName = m_obSwim180Anim;
			obTurn.SetDebugNames("WGen Swim Turn","SimpleTransitionDef");

			InputFollowingTransitionDef obSwimCycle;
			obSwimCycle.SetDebugNames("WGen Swim Continue","InputFollowingTransitionDef");
			obSwimCycle.m_bApplyGravity = true;
			obSwimCycle.m_fExtraSpeed = m_fSpeed;
			obSwimCycle.m_obAnimName = m_obSwimCycleAnim;
			obSwimCycle.m_obLeanLeftAnimName = m_obSwimCycleLeanLeftAnim;
			obSwimCycle.m_obLeanRightAnimName = m_obSwimCycleLeanRightAnim;
			obSwimCycle.m_fMaxRotationPerSecond = m_fMaxRotationPerSecond * DEG_TO_RAD_VALUE;
			obSwimCycle.m_fMaxDirectionChange = m_fMaxDirectionChange * DEG_TO_RAD_VALUE;
			obSwimCycle.m_fSlowForTurnScalar = m_fSlowForTurnScalar;
			obSwimCycle.m_fMaxSpeedChange = m_fMaxSpeedChange;

			pobWGen->GetMovement()->BringInNewController( obTurn, CMovement::DMM_STANDARD, CMovement::MOVEMENT_BLEND );
			pobWGen->GetMovement()->AddChainedController( obSwimCycle, CMovement::DMM_STANDARD, CMovement::MOVEMENT_BLEND );
		}
		else
		{
			pobWGen->GetBossMovement()->m_fMoveSpeed = 1.0f;
			if (pobWGen->GetNavMan() && !pobWGen->GetNavMan()->IsPointInAvoidanceArea(m_aobPathPoints[m_iCurrentPoint]) )
			{
				pobWGen->GetNavMan()->AugmentVector(pobWGen->GetPosition(), obVector, pobWGen->GetBossMovement()->m_fMoveSpeed, fLookAhead, true);
			}
		}

		pobWGen->GetBossMovement()->m_obMoveDirection = obVector;

		return this;
	}
	else if (m_iCurrentPoint >= m_iNumPoints)
	{
		if (!m_bEndStarted)
		{
			ZAxisAlignTargetedTransitionDef obSwimEndAlignDef;
			obSwimEndAlignDef.SetDebugNames("WGen Swim End","ZAxisAlignTargetedTransitionDef");
			obSwimEndAlignDef.m_bApplyGravity = true;
			if (m_bEndAlignToZSet)
				obSwimEndAlignDef.m_obAlignZTo = m_obEndAlignToZ;
			else
				obSwimEndAlignDef.m_obAlignZTo = pobWGen->GetMatrix().GetZAxis() * -1;
			if (m_bEndAlignToPlayer)
				obSwimEndAlignDef.m_pobEntityAlignZTowards = pobPlayer;
			obSwimEndAlignDef.m_obAnimationName = m_obSwimStopAnim;

			pobWGen->GetMovement()->BringInNewController( obSwimEndAlignDef, CMovement::DMM_STANDARD, CMovement::MOVEMENT_BLEND );

			Message obMovementMessage(msg_movementdone);
			pobWGen->GetMovement()->SetCompletionMessage(obMovementMessage,pobBoss);

			ntPrintf("Allowing.\n");
			pobWGen->GetInteractionComponent()->AllowCollisionWith(pobPlayer);

			m_bEndStarted = true;
		}

		if (m_bDone)
		{
			return 0;
		}
		else
			return this;
	}
	else
	{
		ntAssert(0);
		return 0;
	}
}
