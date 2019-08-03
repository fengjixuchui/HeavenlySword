//------------------------------------------------------
//!
//!	\file army/army_chap5_4.cpp
//!
//------------------------------------------------------

#include "army/army_ppu_spu.h"
#include "army/armydef.h"
#include "army/army_section.h"
#include "army/armyrenderable.h"
#include "army/battalion.h"
#include "army/battlefield.h"
#include "army/battlefield_chunk.h"
#include "army/general.h"
#include "army/grunt.h"
#include "army/unit.h"

#include "camera/motioncontroller.h"
#include "exec/ppu/exec_ps3.h"
#include "exec/ppu/elfmanager.h"
#include "exec/ppu/dmabuffer_ps3.h"
#include "exec/execspujobadder_ps3.h"
#include "jobapi/joblist.h"
#include "objectdatabase/dataobject.h"
#include "game/entity.h"
#include "game/entity.inl"
#include "game/entityarmymessagehub.h"
#include "game/entitymanager.h"



#include "army/army_chap5_4.h"


#define NUM_POLES 20

//------------------------------------------------------
//!
//! Set up this section
//! cos of the time when physics is done we can't
//! do much until first frame
//!
//------------------------------------------------------
ArmyChap5_4::ArmyChap5_4( const ArmyBattlefield* pArmyArena ) :
	m_bDoneConstruct( false ),
	m_bCharge( false ),
	m_bCatapultTriggered( false ),
	m_fGameTimeInSecs( 0.f )
{
	for( int i=0; i < 3;i++)
	{
		m_pCatapult[i] = 0;
		m_iCatapultObstacleIndex[i] = -1;
	}

	m_pArmyArena = pArmyArena;
	ntAssert( m_pArmyArena );

}


void ArmyChap5_4::Create_fodder_square_36( const ArmyBattalion* pBattalionDef, const BattlefieldHeader* pBattlefieldHeader, Battalion* pBattalion, TempGruntVector& grunts )
{
	pBattalion->m_FormationOrder = BFO_DISORDER;		
	pBattalion->m_Shape = BS_UNORDERED_SQUARE;				// made up of an unordered square
	pBattalion->m_Orders = BO_FORWARD_MARCH;				// start with a forward march
	// of this many grunts and this set of units
	pBattalion->m_ShapeData.m_UnorderedSquare.m_NumUnitTypes = 1;
	pBattalion->m_ShapeData.m_UnorderedSquare.m_NumGrunts = 36;
	pBattalion->m_ShapeData.m_UnorderedSquare.m_UnitType[0] = NIBBLE_PACK( BUT_FODDER , 0 );
	pBattalion->m_ShapeData.m_UnorderedSquare.m_Percentage[0] = 255;
	FillInUnOrderedSquareBattalion( pBattalionDef, pBattlefieldHeader, pBattalion, grunts );
}
void ArmyChap5_4::Create_shieldsmen_square_36( const ArmyBattalion* pBattalionDef, const BattlefieldHeader* pBattlefieldHeader, Battalion* pBattalion, TempGruntVector& grunts )
{
	pBattalion->m_FormationOrder = BFO_DISORDER;		
	pBattalion->m_Shape = BS_UNORDERED_SQUARE;				// made up of an unordered square
	pBattalion->m_Orders = BO_FORWARD_MARCH;				// start with a forward march
	// of this many grunts and this set of units
	pBattalion->m_ShapeData.m_UnorderedSquare.m_NumUnitTypes = 1;
	pBattalion->m_ShapeData.m_UnorderedSquare.m_NumGrunts = 36;
	pBattalion->m_ShapeData.m_UnorderedSquare.m_UnitType[0] = NIBBLE_PACK( BUT_SHIELDSMAN , 0 );
	pBattalion->m_ShapeData.m_UnorderedSquare.m_Percentage[0] = 255;
	FillInUnOrderedSquareBattalion( pBattalionDef, pBattlefieldHeader, pBattalion, grunts );
}

void ArmyChap5_4::Create_fodder_square_36_Axemen( const ArmyBattalion* pBattalionDef, const BattlefieldHeader* pBattlefieldHeader, Battalion* pBattalion, TempGruntVector& grunts )
{
	pBattalion->m_FormationOrder = BFO_DISORDER;		
	pBattalion->m_Shape = BS_UNORDERED_SQUARE;				// made up of an unordered square
	pBattalion->m_Orders = BO_FORWARD_MARCH;				// start with a forward march
	// of this many grunts and this set of units
	pBattalion->m_ShapeData.m_UnorderedSquare.m_NumUnitTypes = 2;
	pBattalion->m_ShapeData.m_UnorderedSquare.m_NumGrunts = 36;
	pBattalion->m_ShapeData.m_UnorderedSquare.m_UnitType[0] = NIBBLE_PACK( BUT_FODDER , BUT_AXEMAN );
	pBattalion->m_ShapeData.m_UnorderedSquare.m_Percentage[0] = 230;
	pBattalion->m_ShapeData.m_UnorderedSquare.m_Percentage[1] = 25;
	FillInUnOrderedSquareBattalion( pBattalionDef, pBattlefieldHeader, pBattalion, grunts );
}
void ArmyChap5_4::Create_shieldsmen_square_36_Axemen( const ArmyBattalion* pBattalionDef, const BattlefieldHeader* pBattlefieldHeader, Battalion* pBattalion, TempGruntVector& grunts )
{
	pBattalion->m_FormationOrder = BFO_DISORDER;		
	pBattalion->m_Shape = BS_UNORDERED_SQUARE;				// made up of an unordered square
	pBattalion->m_Orders = BO_FORWARD_MARCH;				// start with a forward march
	// of this many grunts and this set of units
	pBattalion->m_ShapeData.m_UnorderedSquare.m_NumUnitTypes = 2;
	pBattalion->m_ShapeData.m_UnorderedSquare.m_NumGrunts = 36;
	pBattalion->m_ShapeData.m_UnorderedSquare.m_UnitType[0] = NIBBLE_PACK( BUT_SHIELDSMAN , BUT_AXEMAN );
	pBattalion->m_ShapeData.m_UnorderedSquare.m_Percentage[0] = 230;
	pBattalion->m_ShapeData.m_UnorderedSquare.m_Percentage[1] = 25;
	FillInUnOrderedSquareBattalion( pBattalionDef, pBattlefieldHeader, pBattalion, grunts );
}

void ArmyChap5_4::Create_fodder_square_36_DoubleAxemen( const ArmyBattalion* pBattalionDef, const BattlefieldHeader* pBattlefieldHeader, Battalion* pBattalion, TempGruntVector& grunts )
{
	pBattalion->m_FormationOrder = BFO_DISORDER;		
	pBattalion->m_Shape = BS_UNORDERED_SQUARE;				// made up of an unordered square
	pBattalion->m_Orders = BO_FORWARD_MARCH;				// start with a forward march
	// of this many grunts and this set of units
	pBattalion->m_ShapeData.m_UnorderedSquare.m_NumUnitTypes = 2;
	pBattalion->m_ShapeData.m_UnorderedSquare.m_NumGrunts = 36;
	pBattalion->m_ShapeData.m_UnorderedSquare.m_UnitType[0] = NIBBLE_PACK( BUT_FODDER , BUT_AXEMAN );
	pBattalion->m_ShapeData.m_UnorderedSquare.m_Percentage[0] = 200;
	pBattalion->m_ShapeData.m_UnorderedSquare.m_Percentage[1] = 55;
	FillInUnOrderedSquareBattalion( pBattalionDef, pBattlefieldHeader, pBattalion, grunts );
}
void ArmyChap5_4::Create_shieldsmen_square_36_DoubleAxemen( const ArmyBattalion* pBattalionDef, const BattlefieldHeader* pBattlefieldHeader, Battalion* pBattalion, TempGruntVector& grunts )
{
	pBattalion->m_FormationOrder = BFO_DISORDER;		
	pBattalion->m_Shape = BS_UNORDERED_SQUARE;				// made up of an unordered square
	pBattalion->m_Orders = BO_FORWARD_MARCH;				// start with a forward march
	// of this many grunts and this set of units
	pBattalion->m_ShapeData.m_UnorderedSquare.m_NumUnitTypes = 2;
	pBattalion->m_ShapeData.m_UnorderedSquare.m_NumGrunts = 36;
	pBattalion->m_ShapeData.m_UnorderedSquare.m_UnitType[0] = NIBBLE_PACK( BUT_SHIELDSMAN , BUT_AXEMAN );
	pBattalion->m_ShapeData.m_UnorderedSquare.m_Percentage[0] = 200;
	pBattalion->m_ShapeData.m_UnorderedSquare.m_Percentage[1] = 55;
	FillInUnOrderedSquareBattalion( pBattalionDef, pBattlefieldHeader, pBattalion, grunts );
}

void ArmyChap5_4::Create_axemen_square_36( const ArmyBattalion* pBattalionDef, const BattlefieldHeader* pBattlefieldHeader, Battalion* pBattalion, TempGruntVector& grunts )
{
	// 6x6 axeman
	pBattalion->m_FormationOrder = BFO_BARBARIAN;	
	pBattalion->m_Shape = BS_ORDERED_LINES;					// made up of ordered lines
	pBattalion->m_Orders = BO_FORWARD_MARCH;				// start with a forward march
	// of this many grunts and this set of units
	pBattalion->m_ShapeData.m_OrderedLine.m_Width = 5;
	pBattalion->m_ShapeData.m_OrderedLine.m_NumLines = 1;
	pBattalion->m_ShapeData.m_OrderedLine.m_DepthPerLine = 4;
	pBattalion->m_ShapeData.m_OrderedLine.m_UnitInLine[0] = NIBBLE_PACK( BUT_AXEMAN, 0 );

	FillInOrderedLineBattalion( pBattalionDef, pBattlefieldHeader, pBattalion, grunts );
}

void ArmyChap5_4::Create_mixed_rectangle_40( const ArmyBattalion* pBattalionDef, const BattlefieldHeader* pBattlefieldHeader, Battalion* pBattalion, TempGruntVector& grunts )
{
	pBattalion->m_FormationOrder = BFO_DISORDER;		
	pBattalion->m_Shape = BS_UNORDERED_SQUARE;				// made up of an unordered square
	pBattalion->m_Orders = BO_FORWARD_MARCH;				// start with a forward march
	// of this many grunts and this set of units
	pBattalion->m_ShapeData.m_UnorderedSquare.m_NumUnitTypes = 3;
	pBattalion->m_ShapeData.m_UnorderedSquare.m_NumGrunts = 40;
	pBattalion->m_ShapeData.m_UnorderedSquare.m_UnitType[0] = NIBBLE_PACK( BUT_SHIELDSMAN , BUT_FODDER );
	pBattalion->m_ShapeData.m_UnorderedSquare.m_UnitType[1] = NIBBLE_PACK( BUT_AXEMAN, 0 );
	pBattalion->m_ShapeData.m_UnorderedSquare.m_Percentage[0] = 115;
	pBattalion->m_ShapeData.m_UnorderedSquare.m_Percentage[1] = 115;
	pBattalion->m_ShapeData.m_UnorderedSquare.m_Percentage[2] = 25;
	FillInUnOrderedSquareBattalion( pBattalionDef, pBattlefieldHeader, pBattalion, grunts );
}

void ArmyChap5_4::Create_shieldman_square_64( const ArmyBattalion* pBattalionDef, const BattlefieldHeader* pBattlefieldHeader, Battalion* pBattalion, TempGruntVector& grunts )
{
	pBattalion->m_FormationOrder = BFO_DISORDER;		
	pBattalion->m_Shape = BS_UNORDERED_SQUARE;				// made up of an unordered square
	pBattalion->m_Orders = BO_FORWARD_MARCH;				// start with a forward march
	// of this many grunts and this set of units
	pBattalion->m_ShapeData.m_UnorderedSquare.m_NumUnitTypes = 2;
	pBattalion->m_ShapeData.m_UnorderedSquare.m_NumGrunts = 64;
	pBattalion->m_ShapeData.m_UnorderedSquare.m_UnitType[0] = NIBBLE_PACK( BUT_SHIELDSMAN , BUT_AXEMAN );
	pBattalion->m_ShapeData.m_UnorderedSquare.m_Percentage[0] = 230;
	pBattalion->m_ShapeData.m_UnorderedSquare.m_Percentage[1] = 25;
	FillInUnOrderedSquareBattalion( pBattalionDef, pBattlefieldHeader, pBattalion, grunts );

}



void ArmyChap5_4::Create_lillys_big_one( const ArmyBattalion* pBattalionDef, const BattlefieldHeader* pBattlefieldHeader, Battalion* pBattalion, TempGruntVector& grunts )
{
	// straight 5x6 shieldman fodder shieldman
	pBattalion->m_FormationOrder = BFO_DISORDER;		
	pBattalion->m_Shape = BS_UNORDERED_SQUARE;				// made up of an unordered square
	pBattalion->m_Orders = BO_FORWARD_MARCH;				// start with a forward march
	// of this many grunts and this set of units
	pBattalion->m_ShapeData.m_UnorderedSquare.m_NumUnitTypes = 3;
	pBattalion->m_ShapeData.m_UnorderedSquare.m_NumGrunts = 243;
	pBattalion->m_ShapeData.m_UnorderedSquare.m_UnitType[0] = NIBBLE_PACK( BUT_SHIELDSMAN , BUT_AXEMAN );
	pBattalion->m_ShapeData.m_UnorderedSquare.m_UnitType[1] = NIBBLE_PACK( BUT_FODDER , 0 );
	pBattalion->m_ShapeData.m_UnorderedSquare.m_Percentage[0] = 64;
	pBattalion->m_ShapeData.m_UnorderedSquare.m_Percentage[1] = 127;
	pBattalion->m_ShapeData.m_UnorderedSquare.m_Percentage[2] = 64;

	FillInUnOrderedSquareBattalion( pBattalionDef, pBattlefieldHeader, pBattalion, grunts );
}

void ArmyChap5_4::Create_small_flag_square( const ArmyBattalion* pBattalionDef, const BattlefieldHeader* pBattlefieldHeader, Battalion* pBattalion, TempGruntVector& grunts )
{

	// straight 5x6 shieldman fodder shieldman
	pBattalion->m_FormationOrder = BFO_DISORDER;		
	pBattalion->m_Shape = BS_UNORDERED_SQUARE;				// made up of an unordered square
	pBattalion->m_Orders = BO_FORWARD_MARCH;				// start with a forward march
	// of this many grunts and this set of units
	pBattalion->m_ShapeData.m_UnorderedSquare.m_NumUnitTypes = 1;
	pBattalion->m_ShapeData.m_UnorderedSquare.m_NumGrunts = NUM_POLES;
	pBattalion->m_ShapeData.m_UnorderedSquare.m_UnitType[0] = NIBBLE_PACK( BUT_FODDER , 0 );
	pBattalion->m_ShapeData.m_UnorderedSquare.m_Percentage[0] = 255;

	// for now force flags
	const_cast<ArmyBattalion*>(pBattalionDef)->m_bHasFlag = true;
	const_cast<ArmyBattalion*>(pBattalionDef)->m_bPlayerAttacking = true;

	FillInUnOrderedSquareBattalion( pBattalionDef, pBattlefieldHeader, pBattalion, grunts );
}

void ArmyChap5_4::SectionConstruct()
{
	// FROM THE SCRIPT 
	//-----------------

	// note until we have a proper export path that allows us load battlefield chunks etc
	// directly we have some code to generate the data t load time
	BattlefieldHeader battlefieldHeader;
	HeightfieldFileHeader* pHeightfieldData = 0;

	CreatePreBattalionDMAStructures( battlefieldHeader, pHeightfieldData );

	// we want to allocate all grunts in on
	TempGruntVector tmpGrunts;
	tmpGrunts.reserve( 2000 );

	{
		ArmyBattlefield::ArmyBattalionDefList::const_iterator bdlIt = m_pArmyArena->m_ArmyBattalionDefs.begin();
		for( unsigned int i = 0;
			bdlIt != m_pArmyArena->m_ArmyBattalionDefs.end(); ++bdlIt, ++i )
		{
			const ArmyBattalion* pBattalionDef = (*bdlIt);
			Battalion* pBattalion = &m_pBattlefield->m_pBattalions->m_BattalionArray[i];
			pBattalion->m_BattalionID = i;

			// okay we use the Command string to decide how to fill this battalion...
			// sure this may seem low tech but for now it will do...
			// \todo move to precompiled hashes for the types we are checking against
			if( pBattalionDef->m_Command == CHashedString("test_arv") )
			{
				Create_fodder_square_36( pBattalionDef, &battlefieldHeader, pBattalion, tmpGrunts );
			} else if( pBattalionDef->m_Command == CHashedString("fodder_square_36") )
			{
				Create_fodder_square_36( pBattalionDef, &battlefieldHeader, pBattalion, tmpGrunts );
			} else if( pBattalionDef->m_Command == CHashedString("shieldmen_square_36") )
			{
				Create_shieldsmen_square_36( pBattalionDef, &battlefieldHeader, pBattalion, tmpGrunts );
			} else if( pBattalionDef->m_Command == CHashedString("fodder_square_36_axemen") )
			{
				Create_fodder_square_36_Axemen( pBattalionDef, &battlefieldHeader, pBattalion, tmpGrunts );
			} else if( pBattalionDef->m_Command == CHashedString("shieldmen_square_36_axemen") )
			{
				Create_shieldsmen_square_36_Axemen( pBattalionDef, &battlefieldHeader, pBattalion, tmpGrunts );
			} else if( pBattalionDef->m_Command == CHashedString("fodder_square_36_double_axemen") )
			{
				Create_fodder_square_36_DoubleAxemen( pBattalionDef, &battlefieldHeader, pBattalion, tmpGrunts );
			} else if( pBattalionDef->m_Command == CHashedString("shieldmen_square_36_double_axemen") )
			{
				Create_shieldsmen_square_36_DoubleAxemen( pBattalionDef, &battlefieldHeader, pBattalion, tmpGrunts );
			} else if( pBattalionDef->m_Command == CHashedString("axemen_square_36") )
			{
				Create_axemen_square_36( pBattalionDef, &battlefieldHeader, pBattalion, tmpGrunts );
			} else if( pBattalionDef->m_Command == CHashedString("mixed_rectangle_60") )
			{
				Create_mixed_rectangle_40( pBattalionDef, &battlefieldHeader, pBattalion, tmpGrunts );
			} else if( pBattalionDef->m_Command == CHashedString("shieldmen_square_80") )
			{
				Create_shieldman_square_64( pBattalionDef, &battlefieldHeader, pBattalion, tmpGrunts );
			} else if( pBattalionDef->m_Command == CHashedString("lillys_big_one") )
			{
				Create_lillys_big_one( pBattalionDef, &battlefieldHeader, pBattalion, tmpGrunts );
			} else if( pBattalionDef->m_Command == CHashedString("small_flag_square") )
			{
				Create_small_flag_square( pBattalionDef, &battlefieldHeader, pBattalion, tmpGrunts );
			}  
			else
			{
				CreateDefaultBattalion( pBattalionDef, &battlefieldHeader, pBattalion, tmpGrunts );
			}
		}
	}


	CreatePostBattalionDMAStructures( battlefieldHeader, pHeightfieldData, tmpGrunts );



	//--------------------------------
	// CUSTOM BEHAVIOUR STUFF
	// Some may call it HACKs I say its the quickest
	// way to get exactly what we want
	//--------------------------------
	m_pCatapult[0] = ObjectDatabase::Get().GetPointerFromName<CEntity*>( "catapult1" );
	m_iCatapultObstacleIndex[0] = GetObstacleIndexFromName( "ArmyObstacle_cat_1" );
	m_pCatapult[1] = ObjectDatabase::Get().GetPointerFromName<CEntity*>( "catapult2" );
	m_iCatapultObstacleIndex[1] = GetObstacleIndexFromName( "ArmyObstacle_cat_2" );
	m_pCatapult[2] = ObjectDatabase::Get().GetPointerFromName<CEntity*>( "catapult3" );
	m_iCatapultObstacleIndex[2] = GetObstacleIndexFromName( "ArmyObstacle_cat_3" );

	DataObject* pCOCam1 = ObjectDatabase::Get().GetDataObjectFromName("CatapultCam1_MCBoomDef");
	if( pCOCam1 != 0 && m_pCatapult[0] )
	{
		MCBoomDef* pCam1 = pCOCam1->GetBaseObject<MCBoomDef>();
		m_vBoomOffset = CDirection(pCam1->m_ptPosition.X(), pCam1->m_ptPosition.Y(),pCam1->m_ptPosition.Z() );
	}
}

//------------------------------------------------------
//!
//! Clean up
//!
//------------------------------------------------------
ArmyChap5_4::~ArmyChap5_4()
{
}

//------------------------------------------------------
//!
//! Called once per frame, to tick the army section
//!
//------------------------------------------------------
void ArmyChap5_4::Update( float fTimeStep )
{
	// first frame construct for now
	if( m_bDoneConstruct == false )
	{
		SectionConstruct();
		m_bDoneConstruct = true;
		return;
	}

	m_fGameTimeInSecs += fTimeStep;

	if( m_pCatapult[0] && (m_iCatapultObstacleIndex[0] != -1) )
	{
		BF_WorldSpaceToBF_Position( m_pCatapult[0]->GetPosition() , m_pBattlefield->m_pHeader, &m_pBattlefield->m_pHeader->m_Obstacles[ m_iCatapultObstacleIndex[0] ].m_Location );
	}
	if( m_pCatapult[1] && (m_iCatapultObstacleIndex[1] != -1) )
	{
		BF_WorldSpaceToBF_Position( m_pCatapult[1]->GetPosition() , m_pBattlefield->m_pHeader, &m_pBattlefield->m_pHeader->m_Obstacles[ m_iCatapultObstacleIndex[1] ].m_Location );
	}
	if( m_pCatapult[2] && (m_iCatapultObstacleIndex[2] != -1) )
	{
		BF_WorldSpaceToBF_Position( m_pCatapult[2]->GetPosition() , m_pBattlefield->m_pHeader, &m_pBattlefield->m_pHeader->m_Obstacles[ m_iCatapultObstacleIndex[2] ].m_Location );
	}
/*
	DataObject* pCOCam1 = ObjectDatabase::Get().GetDataObjectFromName("CatapultCam1_MCBoomDef");
	if( pCOCam1 != 0 && m_pCatapult[0] )
	{
		MCBoomDef* pCam1 = pCOCam1->GetBaseObject<MCBoomDef>();
		pCam1->m_ptPosition = m_vBoomOffset + m_pCatapult[0]->GetPosition();
	}
	DataObject* pCOCam2 = ObjectDatabase::Get().GetDataObjectFromName("CatapultCam2_MCBoomDef");
	if( pCOCam2 != 0 && m_pCatapult[1] )
	{
		MCBoomDef* pCam2 = pCOCam1->GetBaseObject<MCBoomDef>();
		pCam2->m_ptPosition = m_vBoomOffset + m_pCatapult[1]->GetPosition();
	}
	DataObject* pCOCam3 = ObjectDatabase::Get().GetDataObjectFromName("CatapultCam3_MCBoomDef");
	if( pCOCam3 != 0 && m_pCatapult[2] )
	{
		MCBoomDef* pCam3 = pCOCam3->GetBaseObject<MCBoomDef>();
		pCam3->m_ptPosition = m_vBoomOffset + m_pCatapult[2]->GetPosition();
	}
*/

	// reset the staging post flag
	static const uint16_t STAGING_POST_FLAG = (1 << 0);
	m_pBattlefield->m_pHeader->m_iGlobalEventFlags &= ~STAGING_POST_FLAG;

	bool bHasBazooka = true;
	// does the player have the bazooka or not?
	Player* pPlayer = CEntityManager::Get().GetPlayer();
	if( (pPlayer->GetRangedWeapon() == 0 && pPlayer->GetInteractionTarget() == 0) )
	{
		bHasBazooka = false;
	}

	uint32_t iCurTargetDeadCount = 0;

	// whats the target
	// for now 50 * m_iStagingPostLine but will take from data table eventually
	iCurTargetDeadCount = m_pBattlefield->m_pHeader->m_iStagingPostLine * 
		(bHasBazooka ? m_pArmyGenericParameters->m_iStagePostBodyCountWithBazooka : m_pArmyGenericParameters->m_iStagePostBodyCountWithSwords);

	uint32_t iWaitAtStagingPost = 0;
	// count how many battalions are in awaiting at the staging post
	for( uint32_t i=0;i < m_pBattlefield->m_pBattalions->m_iNumBattalions;i++)
	{
		const Battalion* pBattalion = &m_pBattlefield->m_pBattalions->m_BattalionArray[ i ];
		iWaitAtStagingPost += (pBattalion->m_BattalionFlags & BF_AT_STAGING_POST);
	}

	// pause the stage posts for 25 seconds while the cutscene runs
	if( m_fGameTimeInSecs > 25.f )
	{
		if( (iWaitAtStagingPost >= 3 ) &&
			( m_pBattlefield->m_pHeader->m_eaInfo->m_iTotalDeadCount >= iCurTargetDeadCount || m_bCharge) )
		{
			m_pBattlefield->m_pHeader->m_iGlobalEventFlags |= STAGING_POST_FLAG;
			m_pBattlefield->m_pHeader->m_iStagingPostLine++;
		}
	}

	static const unsigned int iCatapultTriggerBodyCount = 100;
	if( m_pBattlefield->m_pHeader->m_eaInfo->m_iTotalDeadCount >= iCatapultTriggerBodyCount && (m_bCatapultTriggered == false) )
	{
		m_bCatapultTriggered = true;
		ntError( m_pArmyArena != NULL );
		ntError( m_pArmyArena->m_pMessageHub != NULL );
		ntError( m_pArmyArena->m_pMessageHub->GetMessageHandler() != NULL );

		m_pArmyArena->m_pMessageHub->GetMessageHandler()->ProcessEvent( "OnComplete" );
	}

	// when the player drops the bazooka, trigger a charge, only do it once they have kill at least one guy
	// HACK the interaction target is a fix for borked GetRangedWeapon() behavior
//	Player* pPlayer = CEntityManager::Get().GetPlayer();
//	if( (pPlayer->GetRangedWeapon() == 0 && pPlayer->GetInteractionTarget() == 0) && ((m_pBattlefield->m_pHeader->m_eaInfo->m_iTotalDeadCount > 0) && (m_bCharge == false)) )
//	{
//		m_bCharge = true;
//	}



	// kick off SPU stuff
	ArmySection::Update(fTimeStep);
}

//------------------------------------------------------
//!
//! Called once per frame, to render the army section
//!
//------------------------------------------------------
void ArmyChap5_4::Render()
{

	// thunk thro to our parent render function
	ArmySection::Render();
}

//------------------------------------------------------
//!
//! takes a hashed string 
//! The simpliest one set a global event flag
//! in the battlefield so the actual armies can respond in
//! some simple way
//! but other things are possible (disable obstacles etc.)
//!
//------------------------------------------------------
void ArmyChap5_4::ProcessGlobalEvent( const CHashedString obGlobalEvent )
{
	UNUSED( obGlobalEvent );
}

