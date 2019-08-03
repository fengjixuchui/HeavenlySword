//------------------------------------------------------
//!
//!	\file army/army_chap5_4.cpp
//!
//------------------------------------------------------

#include "army/army_ppu_spu.h"
#include "army/armydef.h"
#include "army/army_section.h"
#include "army/battalion.h"
#include "army/battlefield.h"
#include "army/battlefield_chunk.h"
#include "army/general.h"
#include "army/grunt.h"
#include "army/unit.h"

#include "game/entityarmymessagehub.h"
#include "exec/ppu/exec_ps3.h"
#include "exec/ppu/elfmanager.h"
#include "exec/ppu/dmabuffer_ps3.h"
#include "exec/execspujobadder_ps3.h"
#include "jobapi/joblist.h"
#include "objectdatabase/dataobject.h"

#include "army/army_chap1_3.h"


// Win/lose condition numbers.
namespace
{
	static const uint16_t TRIGGER_INFANTRY_RUN = (1 << 1);
}


//------------------------------------------------------
//!
//! Set up this section
//! cos of the time when physics is done we can't
//! do much until first frame
//!
//------------------------------------------------------
ArmyChap1_3::ArmyChap1_3( const ArmyBattlefield* pArmyArena ) :
	m_bDoneConstruct( false )
{
	m_pArmyArena = pArmyArena;
	ntAssert( m_pArmyArena );
}

void ArmyChap1_3::CreateSmallSquare( const ArmyBattalion* pBattalionDef, const BattlefieldHeader* pBattlefieldHeader, Battalion* pBattalion, TempGruntVector& grunts )
{

	// wavey 6x10 fodder fodder shieldman
	pBattalion->m_FormationOrder = BFO_SEMI_REGULAR;				// a tightly controlled unit
	pBattalion->m_Shape = BS_ORDERED_LINES;					// made up of ordered lines
	pBattalion->m_Orders = BO_FORWARD_MARCH;				// start with a forward march
	// of this many grunts and this set of units
	pBattalion->m_ShapeData.m_OrderedLine.m_Width = 5;
	pBattalion->m_ShapeData.m_OrderedLine.m_NumLines = 5;
	pBattalion->m_ShapeData.m_OrderedLine.m_DepthPerLine = 1;
	pBattalion->m_ShapeData.m_OrderedLine.m_UnitInLine[0] = NIBBLE_PACK( BUT_FODDER, 0 );

	FillInOrderedLineBattalion( pBattalionDef, pBattlefieldHeader, pBattalion, grunts );
}

void ArmyChap1_3::CreateMediumSquare( const ArmyBattalion* pBattalionDef, const BattlefieldHeader* pBattlefieldHeader, Battalion* pBattalion, TempGruntVector& grunts )
{

	// wavey 6x10 fodder fodder shieldman
	pBattalion->m_FormationOrder = BFO_SEMI_REGULAR;				// a tightly controlled unit
	pBattalion->m_Shape = BS_ORDERED_LINES;					// made up of ordered lines
	pBattalion->m_Orders = BO_FORWARD_MARCH;				// start with a forward march
	// of this many grunts and this set of units
	pBattalion->m_ShapeData.m_OrderedLine.m_Width = 8;
	pBattalion->m_ShapeData.m_OrderedLine.m_NumLines = 4;
	pBattalion->m_ShapeData.m_OrderedLine.m_DepthPerLine = 2;
	pBattalion->m_ShapeData.m_OrderedLine.m_UnitInLine[0] = NIBBLE_PACK( BUT_FODDER, BUT_SHIELDSMAN );

	FillInOrderedLineBattalion( pBattalionDef, pBattlefieldHeader, pBattalion, grunts );
}

void ArmyChap1_3::CreateLargeSquare( const ArmyBattalion* pBattalionDef, const BattlefieldHeader* pBattlefieldHeader, Battalion* pBattalion, TempGruntVector& grunts )
{

	// wavey 6x10 fodder fodder shieldman
	pBattalion->m_FormationOrder = BFO_SEMI_REGULAR;				// a tightly controlled unit
	pBattalion->m_Shape = BS_ORDERED_LINES;					// made up of ordered lines
	pBattalion->m_Orders = BO_FORWARD_MARCH;				// start with a forward march
	// of this many grunts and this set of units
	pBattalion->m_ShapeData.m_OrderedLine.m_Width = 10;
	pBattalion->m_ShapeData.m_OrderedLine.m_NumLines = 2;
	pBattalion->m_ShapeData.m_OrderedLine.m_DepthPerLine = 5;
	pBattalion->m_ShapeData.m_OrderedLine.m_UnitInLine[0] = NIBBLE_PACK( BUT_FODDER, BUT_FODDER );
	pBattalion->m_ShapeData.m_OrderedLine.m_UnitInLine[1] = NIBBLE_PACK( BUT_SHIELDSMAN, BUT_SHIELDSMAN );
	pBattalion->m_ShapeData.m_OrderedLine.m_UnitInLine[2] = NIBBLE_PACK( BUT_AXEMAN, 0 );

	FillInOrderedLineBattalion( pBattalionDef, pBattlefieldHeader, pBattalion, grunts );
}
void ArmyChap1_3::CreateSmallRectangle( const ArmyBattalion* pBattalionDef, const BattlefieldHeader* pBattlefieldHeader, Battalion* pBattalion, TempGruntVector& grunts )
{

	// wavey 6x10 fodder fodder shieldman
	pBattalion->m_FormationOrder = BFO_SEMI_REGULAR;				// a tightly controlled unit
	pBattalion->m_Shape = BS_ORDERED_LINES;					// made up of ordered lines
	pBattalion->m_Orders = BO_FORWARD_MARCH;				// start with a forward march
	// of this many grunts and this set of units
	pBattalion->m_ShapeData.m_OrderedLine.m_Width = 3;
	pBattalion->m_ShapeData.m_OrderedLine.m_NumLines = 5;
	pBattalion->m_ShapeData.m_OrderedLine.m_DepthPerLine = 1;
	pBattalion->m_ShapeData.m_OrderedLine.m_UnitInLine[0] = NIBBLE_PACK( BUT_FODDER, 0 );

	FillInOrderedLineBattalion( pBattalionDef, pBattlefieldHeader, pBattalion, grunts );
}

void ArmyChap1_3::CreateMediumRectangle( const ArmyBattalion* pBattalionDef, const BattlefieldHeader* pBattlefieldHeader, Battalion* pBattalion, TempGruntVector& grunts )
{

	// wavey 6x10 fodder fodder shieldman
	pBattalion->m_FormationOrder = BFO_SEMI_REGULAR;				// a tightly controlled unit
	pBattalion->m_Shape = BS_ORDERED_LINES;					// made up of ordered lines
	pBattalion->m_Orders = BO_FORWARD_MARCH;				// start with a forward march
	// of this many grunts and this set of units
	pBattalion->m_ShapeData.m_OrderedLine.m_Width = 4;
	pBattalion->m_ShapeData.m_OrderedLine.m_NumLines = 2;
	pBattalion->m_ShapeData.m_OrderedLine.m_DepthPerLine = 2;
	pBattalion->m_ShapeData.m_OrderedLine.m_UnitInLine[0] = NIBBLE_PACK( BUT_FODDER, BUT_SHIELDSMAN );

	FillInOrderedLineBattalion( pBattalionDef, pBattlefieldHeader, pBattalion, grunts );
}

void ArmyChap1_3::CreateLargeRectangle( const ArmyBattalion* pBattalionDef, const BattlefieldHeader* pBattlefieldHeader, Battalion* pBattalion, TempGruntVector& grunts )
{

	// wavey 6x10 fodder fodder shieldman
	pBattalion->m_FormationOrder = BFO_SEMI_REGULAR;				// a tightly controlled unit
	pBattalion->m_Shape = BS_ORDERED_LINES;					// made up of ordered lines
	pBattalion->m_Orders = BO_FORWARD_MARCH;				// start with a forward march
	// of this many grunts and this set of units
	pBattalion->m_ShapeData.m_OrderedLine.m_Width = 5;
	pBattalion->m_ShapeData.m_OrderedLine.m_NumLines = 2;
	pBattalion->m_ShapeData.m_OrderedLine.m_DepthPerLine = 5;
	pBattalion->m_ShapeData.m_OrderedLine.m_UnitInLine[0] = NIBBLE_PACK( BUT_FODDER, BUT_FODDER );
	pBattalion->m_ShapeData.m_OrderedLine.m_UnitInLine[1] = NIBBLE_PACK( BUT_SHIELDSMAN, BUT_SHIELDSMAN );
	pBattalion->m_ShapeData.m_OrderedLine.m_UnitInLine[2] = NIBBLE_PACK( BUT_AXEMAN, 0 );

	FillInOrderedLineBattalion( pBattalionDef, pBattlefieldHeader, pBattalion, grunts );
}


void ArmyChap1_3::Create_secondwave( const ArmyBattalion* pBattalionDef, const BattlefieldHeader* pBattlefieldHeader, Battalion* pBattalion, TempGruntVector& grunts )
{
	pBattalion->m_FormationOrder = BFO_DISORDER;		
	pBattalion->m_Shape = BS_UNORDERED_SQUARE;				// made up of an unordered square
	pBattalion->m_Orders = BO_FORWARD_MARCH;				// start with a forward march
	// of this many grunts and this set of units
	pBattalion->m_ShapeData.m_UnorderedSquare.m_NumUnitTypes = 2;
	pBattalion->m_ShapeData.m_UnorderedSquare.m_NumGrunts = 100;
	pBattalion->m_ShapeData.m_UnorderedSquare.m_UnitType[0] = NIBBLE_PACK( BUT_SHIELDSMAN , BUT_FODDER );
	pBattalion->m_ShapeData.m_UnorderedSquare.m_Percentage[0] = 127;
	pBattalion->m_ShapeData.m_UnorderedSquare.m_Percentage[1] = 128;
	FillInUnOrderedSquareBattalion( pBattalionDef, pBattlefieldHeader, pBattalion, grunts );

}


void ArmyChap1_3::SectionConstruct()
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
		if( pBattalionDef->m_Command == CHashedString("small_square") )
		{
			CreateSmallSquare( pBattalionDef, &battlefieldHeader, pBattalion, tmpGrunts );
		} else if( pBattalionDef->m_Command == CHashedString("medium_square") )
		{
			CreateMediumSquare( pBattalionDef, &battlefieldHeader, pBattalion, tmpGrunts );
		} else if( pBattalionDef->m_Command == CHashedString("large_square") )
		{
			CreateLargeSquare( pBattalionDef, &battlefieldHeader, pBattalion, tmpGrunts );
		} else if( pBattalionDef->m_Command == CHashedString("small_rectangle") )
		{
			CreateSmallRectangle( pBattalionDef, &battlefieldHeader, pBattalion, tmpGrunts );
		} else if( pBattalionDef->m_Command == CHashedString("medium_rectangle") )
		{
			CreateMediumRectangle( pBattalionDef, &battlefieldHeader, pBattalion, tmpGrunts );
		} else if( pBattalionDef->m_Command == CHashedString("large_rectangle") )
		{
			CreateLargeRectangle( pBattalionDef, &battlefieldHeader, pBattalion, tmpGrunts );
		} else if( pBattalionDef->m_Command == CHashedString("secondwave") )
		{
			Create_secondwave( pBattalionDef, &battlefieldHeader, pBattalion, tmpGrunts );
		}
		else
		{
			CreateDefaultBattalion( pBattalionDef, &battlefieldHeader, pBattalion, tmpGrunts );
		}
	}
	CreatePostBattalionDMAStructures( battlefieldHeader, pHeightfieldData, tmpGrunts );
}

//------------------------------------------------------
//!
//! Clean up
//!
//------------------------------------------------------
ArmyChap1_3::~ArmyChap1_3()
{
}

//------------------------------------------------------
//!
//! Called once per frame, to tick the army section
//!
//------------------------------------------------------
void ArmyChap1_3::Update( float fTimeStep )
{
	// first frame construct for now
	if( m_bDoneConstruct == false )
	{
		SectionConstruct();
		m_bDoneConstruct = true;
	}

	// kick off SPU stuff
	ArmySection::Update(fTimeStep);

}

//------------------------------------------------------
//!
//! Called once per frame, to render the army section
//!
//------------------------------------------------------
void ArmyChap1_3::Render()
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
void ArmyChap1_3::ProcessGlobalEvent( const CHashedString obGlobalEvent )
{
#if !defined(_RELEASE )
	ntPrintf( "obGlobalEvent \"%s\" triggered\n", obGlobalEvent.GetDebugString() );
#endif
	if( obGlobalEvent == CHashedString("Trigger") )
	{
		// set the flag to make the infantry to run
		m_pBattlefield->m_pHeader->m_iGlobalEventFlags |= TRIGGER_INFANTRY_RUN;
		// also reset bodycounts
		AtomicSet( &m_pBattlefield->m_pHeader->m_eaInfo->m_iCurDead, 0 );
	}
}
