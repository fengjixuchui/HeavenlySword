//------------------------------------------------------
//!
//!	\file army/army_section_setup.cpp
//! army section needed a bit of a tidy this was the 
//! fastest way, move all setup function here and
//! leave other stuff in army_section.cpp
//!
//------------------------------------------------------

#include "ai/AINavigationSystem/aiworldvolumes.h"
#include "army/army_ppu_spu.h"
#include "army/armydef.h"
#include "army/armymanager.h"
#include "army/army_section.h"
#include "army/battlefield.h"
#include "army/battlefield_chunk.h"
#include "army/battalion.h"
#include "army/general.h"
#include "army/grunt.h"
#include "army/unit.h"
#include "army/armyrenderable.h"
#include "army/armyrenderpool.h"
#include "core/gatso.h"
#include "core/timer.h"

#include "camera/camutils.h"

#include "core/visualdebugger.h"
#include "core/exportstruct_anim.h"

// SPU-exec related includes
#include "exec/ppu/exec_ps3.h"
#include "exec/ppu/ElfManager.h"
#include "exec/ppu/dmabuffer_ps3.h"
#include "exec/ppu/sputask_ps3.h"
#include "exec/execspujobadder_ps3.h"
#include "jobapi/joblist.h"
#include "game/randmanager.h"
#include "game/entityplayer.h"
#include "game/entitymanager.h"
#include "game/entityspawnpoint.h"
#include "game/entityprojectile.h"
#include "game/entitymanager.h"
#include "objectdatabase/dataobject.h"
#include "physics/world.h"

#include "physics/verletManager_ps3.h"
#include "physics/verletdef_ps3.h"
#include "physics/verletinstance_ps3.h"
#include "physics/verletrenderable_ps3.h"
#include "physics/softflag.h"

// includes required for audio hooks
#include "audio/audiosystem.h"

void CreateBattlefield( const BattlefieldHeader* pHeader, const float* pFloats, Battlefield* pBattlefield )
{
	// how many chunks?
	const uint16_t iWidthInChunks = (pHeader->m_TotalWidth / BF_CHUNK_WIDTH);
	const uint16_t iHeightInChunks = (pHeader->m_TotalHeight / BF_CHUNK_HEIGHT);
	ntAssert( (iWidthInChunks*iHeightInChunks) <= MAX_BATTLEFIELD_CHUNKS );

	// heightfield must be divisible by chunk size.
	ntAssert( iWidthInChunks*BF_CHUNK_WIDTH == pHeader->m_TotalWidth );
	ntAssert( iHeightInChunks*BF_CHUNK_HEIGHT == pHeader->m_TotalHeight );

	pBattlefield->m_pChunksHeader = (BattlefieldChunksHeader*) DMABuffer::DMAAlloc(	
									sizeof( BattlefieldChunksHeader ) + ((iWidthInChunks*iHeightInChunks)*sizeof(BattlefieldChunk*)),
									0,
									Mem::MC_ARMY );
	pBattlefield->m_pChunksHeader->m_iNumChunks = (iWidthInChunks*iHeightInChunks);
	BattlefieldChunk** pChunks = pBattlefield->m_pChunksHeader->m_pChunks;

	// copy the header over
	NT_MEMCPY( pBattlefield->m_pHeader, pHeader, sizeof( BattlefieldHeader) );
	pBattlefield->m_pHeader->m_WidthInChunks = iWidthInChunks; 
	pBattlefield->m_pHeader->m_HeightInChunks = iHeightInChunks; 

	for( uint16_t cy=0; cy < iHeightInChunks; ++cy )
	{
		for( uint16_t cx=0; cx < iWidthInChunks; ++cx )
		{
			uint16_t chunkId = (cy*iWidthInChunks)+cx;
			pChunks[chunkId] = (BattlefieldChunk*) DMABuffer::DMAAlloc( sizeof(BattlefieldChunk), 0, Mem::MC_ARMY );
			pChunks[chunkId]->m_TopCoord = cy * BF_CHUNK_HEIGHT;
			pChunks[chunkId]->m_LeftCoord = cx * BF_CHUNK_WIDTH;
			pChunks[chunkId]->m_ChunkId = chunkId;

			// copy the heightfield
			for( int16_t iY=-1;iY < BF_CHUNK_HEIGHT+1;iY++ )
			{
				int16_t y = pChunks[chunkId]->m_TopCoord + iY;
				// copy the extra lines
				if( y < 0 )
				{
					y = 0;
				} else
				{
					if( y == pHeader->m_TotalHeight )
					{
						y = pHeader->m_TotalHeight-1;
					}
				}

				for( int16_t iX=-1;iX < BF_CHUNK_WIDTH+1;iX++ )
				{
					int16_t x = pChunks[chunkId]->m_LeftCoord + iX;

					// copy the extra lines
					if( x < 0 )
					{
						x = 0;
					} else
					{
						if( x == pHeader->m_TotalWidth )
						{
							x = pHeader->m_TotalWidth-1;
						}
					}
					float h = pFloats[ (y *pHeader->m_TotalWidth) + x ];

					pChunks[chunkId]->m_HeightField[((iY+1) * (BF_CHUNK_WIDTH+2)) + (iX+1) ] = h;
				}
			}
		}
	}

}

//**************************************************************************************
//!	
//! returns a complete heightfield header that the caller is responsible for afterwards
//!
//**************************************************************************************
HeightfieldFileHeader* GenerateHeightfield( const CPoint &origin, const CDirection &diagonal, unsigned int width, unsigned int height )
{
	//
	//	This path should be moved into MrEd at some point.
	//

	HeightfieldFileHeader* pHeightfieldHeader = (HeightfieldFileHeader*) NT_ALLOC_CHUNK( Mem::MC_MISC, sizeof(HeightfieldFileHeader) + 
																		sizeof(float)*height*width );
	pHeightfieldHeader->m_Width = width;
	pHeightfieldHeader->m_Height = height;
	float* pHeightfield = (float*)(pHeightfieldHeader+1);

	const CDirection sq_diag( diagonal.X() / pHeightfieldHeader->m_Width, diagonal.Y(), diagonal.Z() / pHeightfieldHeader->m_Height );

	for ( unsigned int x=0;x<pHeightfieldHeader->m_Width;x++ )
	{
		for ( unsigned int z=0;z<pHeightfieldHeader->m_Height;z++ )
		{
			CPoint start( origin + CDirection( sq_diag.X() * float( x ), 1000.0f, sq_diag.Z() * float( z ) ) );
			CPoint end( origin + CDirection( sq_diag.X() * float( x ), -1000.0f, sq_diag.Z() * float( z ) ) );

			Physics::TRACE_LINE_QUERY query;

			Physics::RaycastCollisionFlag flag;
			flag.base = 0;
			flag.flags.i_am = Physics::STATIC_ENVIRONMENT_BIT;
			flag.flags.i_collide_with = ( Physics::LARGE_INTERACTABLE_BIT | Physics::SMALL_INTERACTABLE_BIT );

			float height( 0.0f );
			if ( Physics::CPhysicsWorld::Get().TraceLine( start, end, NULL, query, flag ) )
			{
				height = query.obIntersect.Y();
			}

			pHeightfield[ z * pHeightfieldHeader->m_Width + x ] = height;
		}
	}

	return pHeightfieldHeader;
}

void ArmySection::FillInOrderedLineBattalion( const ArmyBattalion* pBattalionDef, const BattlefieldHeader* pBattlefieldHeader, Battalion* pBattalion, TempGruntVector& grunts )
{
	// now allocate the units this calcutions are only valid for BS_ORDERED_LINES (hence the assert)
	ntAssert( pBattalion->m_Shape == BS_ORDERED_LINES );

	// fill in initial player tracking and circling bits
	pBattalion->m_BattalionFlags |= (pBattalionDef->m_bPlayerTracking) ? BF_PLAYER_TRACKING : 0;
	pBattalion->m_BattalionFlags |= (pBattalionDef->m_bPlayerCircle) ? BF_PLAYER_CIRCLE : 0;

	// is this relative to a parent?
	pBattalion->m_BattalionFlags |= (pBattalionDef->m_pParentEntity != 0) ? BF_RELATIVE_COMMANDS : 0;

	uint32_t iNumUnitTypes = 0;
	uint16_t iHasUnitOfType[ BUT_MAX ];

	memset( iHasUnitOfType, 0, BUT_MAX * sizeof(uint16_t) );
	// count actual unit types
	for( uint32_t i=0;i < pBattalion->m_ShapeData.m_OrderedLine.m_NumLines;i++)
	{
		uint8_t iUnitType;
		if( i & 0x1 )
		{
			iUnitType = NIBBLE_EXTRACT_TOP( pBattalion->m_ShapeData.m_OrderedLine.m_UnitInLine[ i>>1 ] );
		} else
		{
			iUnitType = NIBBLE_EXTRACT_BOTTOM( pBattalion->m_ShapeData.m_OrderedLine.m_UnitInLine[ i>>1 ] );
		}
		if( iHasUnitOfType[ iUnitType ] == 0 )
		{
			iNumUnitTypes++;
		}
		iHasUnitOfType[ iUnitType ]++;
	}

	ntAssert( iNumUnitTypes < MAX_UNITS_PER_BATTALION );

	pBattalion->m_iNumUnits = iNumUnitTypes;
	Unit* pFirstUnit = pBattalion->m_Units;
	Unit* pUnit = pFirstUnit;
	Unit* pUnitOfType[ BUT_MAX ];
	memset( pUnitOfType, 0, BUT_MAX * sizeof(Unit*) );

	pBattalion->m_iNumGrunts = (pBattalion->m_ShapeData.m_OrderedLine.m_NumLines * 
									pBattalion->m_ShapeData.m_OrderedLine.m_DepthPerLine * 
									pBattalion->m_ShapeData.m_OrderedLine.m_Width );
	uint16_t iCurGruntNum = grunts.size();
	// increase by the total number of grunts
	grunts.resize( grunts.size() + pBattalion->m_iNumGrunts );

	for( uint32_t i=0;i < pBattalion->m_ShapeData.m_OrderedLine.m_NumLines;i++)
	{
		uint8_t iUnitType;
		if( i & 0x1 )
		{
			iUnitType = NIBBLE_EXTRACT_TOP( pBattalion->m_ShapeData.m_OrderedLine.m_UnitInLine[ i>>1 ] );
		} else
		{
			iUnitType = NIBBLE_EXTRACT_BOTTOM( pBattalion->m_ShapeData.m_OrderedLine.m_UnitInLine[ i>>1 ] );
		}
		if( pUnitOfType[iUnitType] == 0 )
		{
			ntAssert( (uint32_t)(pUnit - pFirstUnit) < iNumUnitTypes );

			pUnitOfType[iUnitType] = pUnit;
			pUnit->m_UnitID = pUnit - pFirstUnit;
			pUnit->m_UnitType = iUnitType;
			pUnit->m_NumGrunts = 0;
			pUnit->m_BattalionID = pBattalion->m_BattalionID;
			pUnit++;
		}
		// count how many grunts are in each unit type
		pUnitOfType[iUnitType]->m_NumGrunts += (pBattalion->m_ShapeData.m_OrderedLine.m_DepthPerLine * pBattalion->m_ShapeData.m_OrderedLine.m_Width);
	}

	ntError( iNumUnitTypes < MAX_UNITS_PER_BATTALION );

	// now go thro each unit and allocate a cotinous block of grunts and mark the start
	uint16_t iUnitGruntIDTracker[ BUT_MAX ];
	for( uint32_t i=0;i < iNumUnitTypes; ++i )
	{
		Unit* pUnit = pFirstUnit + i;
		pUnit->m_FirstGruntID = iCurGruntNum;
		iUnitGruntIDTracker[ pUnit->m_UnitType ] = pUnit->m_FirstGruntID;
		iCurGruntNum += pUnit->m_NumGrunts;
	}

	// now build the grunts data in the formation 
	CDirection horiz( pBattalionDef->m_MaxCorner.X() - pBattalionDef->m_MinCorner.X(), 0, 0 );
	CDirection vert( 0, 0, pBattalionDef->m_MaxCorner.Z() - pBattalionDef->m_MinCorner.Z() );
	CPoint center = pBattalionDef->m_MinCorner + (horiz * 0.5f) + (vert * 0.5f);

	char pBattalionName[ 256 ] = "\0";
	UNUSED( pBattalionName );
#if !defined( _RELEASE )
	sprintf( pBattalionName, "%s", ObjectDatabase::Get().GetDataObjectFromPointer( pBattalionDef )->GetName().GetString() );
#endif


	BF_Position bf_center;
	bf_center.m_X = 0;
	bf_center.m_Y = 0;
	bool ret;
	UNUSED(ret);
	ret = 	BF_WorldSpaceToBF_Position( center, pBattlefieldHeader, &bf_center );
	user_error_p( ret, ("Center of Battalion %s Off the Battlefield\n", pBattalionName ) );

	for( unsigned int i=0;i < pBattalion->m_ShapeData.m_OrderedLine.m_NumLines;i++)
	{
		uint8_t iUnitType;
		if( i & 0x1 )
		{
			iUnitType = NIBBLE_EXTRACT_TOP( pBattalion->m_ShapeData.m_OrderedLine.m_UnitInLine[ i>>1 ] );
		} else
		{
			iUnitType = NIBBLE_EXTRACT_BOTTOM( pBattalion->m_ShapeData.m_OrderedLine.m_UnitInLine[ i>>1 ] );
		}

		Unit* pUnit = pUnitOfType[ iUnitType ];
		ntAssert( pUnit->m_UnitType == iUnitType );

		const float tOffset = 0.5f / (float)pBattalion->m_ShapeData.m_OrderedLine.m_Width;
		for( unsigned int j=0;j < pBattalion->m_ShapeData.m_OrderedLine.m_DepthPerLine;j++)
		{
			float tHeight = (float)((i*pBattalion->m_ShapeData.m_OrderedLine.m_DepthPerLine)+j) /
								(float)(pBattalion->m_ShapeData.m_OrderedLine.m_NumLines*pBattalion->m_ShapeData.m_OrderedLine.m_DepthPerLine);

			// cos positive Y is towards the player I want to fill backwards...
			tHeight = 1.0f - tHeight;

			for( unsigned int k=0; k < pBattalion->m_ShapeData.m_OrderedLine.m_Width;k++)
			{
				GruntGameState& gr = grunts[ iUnitGruntIDTracker[ pUnit->m_UnitType ] ]; 

				float tWidth = (float)k / (float)pBattalion->m_ShapeData.m_OrderedLine.m_Width;

				switch( pBattalion->m_FormationOrder )
				{
				case BFO_REGULAR: break;
				case BFO_SEMI_REGULAR:
					tWidth += (grandf(0.05f)-0.025f);
					tHeight += (grandf(0.02f)-0.01f);
					break;
				case BFO_BARBARIAN:
					tWidth += (grandf(0.1f)-0.05f);
					tHeight += (grandf(0.1f)-0.05f);
					break;
				case BFO_DISORDER:
					tWidth += (grandf(0.5f)-0.25f);
					tHeight += (grandf(0.5f)-0.25f);
					break;
				}
				if( pBattalion->m_ShapeData.m_OrderedLine.m_OffsetAlternateLines )
				{
					tWidth += (k & 0x1) ? tOffset : 0.f;
				}
				// wrap mode
				while( tWidth > 1.0f )
				{
					tWidth -= 1.0f;
				}
				while( tWidth < 0.f )
				{
					tWidth += 1.0f;
				}
				while( tHeight > 1.0f )
				{
					tHeight -= 1.0f;
				}
				while( tHeight < 0.f )
				{
					tHeight += 1.0f;
				}


				CPoint worldPos = pBattalionDef->m_MinCorner + (tWidth * horiz) + (tHeight * vert);
				CPoint offset = worldPos - center;
				bool ret;
				UNUSED(ret);
				ret = BF_WorldSpaceToBF_Position( worldPos, pBattlefieldHeader, &gr.m_Position );
				user_error_p( ret, ("Grunt of Battalion %s starting off the Battlefield\n", pBattalionName) );

				BF_WorldSpaceToBF_Position( worldPos, pBattlefieldHeader, &gr.m_Position );
				gr.m_Orientation.m_Rotation = (uint16_t)(0.25f * 65535.f); // TODO 
				gr.m_GruntID		= iUnitGruntIDTracker[ pUnit->m_UnitType ]++;
				gr.m_BattalionID	= pUnit->m_BattalionID;
				gr.m_UnitID			= pUnit->m_UnitID;
				gr.m_GruntMajorState = GMS_NORMAL; // start people off alive and not shit scared :-D
				gr.m_BattalionOffsetX = (int16_t)gr.m_Position.m_X - (int16_t)bf_center.m_X;
				gr.m_BattalionOffsetY = (int16_t)gr.m_Position.m_Y - (int16_t)bf_center.m_Y;
				gr.m_IntendedPos.m_X = gr.m_Position.m_X;
				gr.m_IntendedPos.m_Y = gr.m_Position.m_Y;
			}
		}
	}
}

void ArmySection::FillInUnOrderedSquareBattalion( const ArmyBattalion* pBattalionDef, const BattlefieldHeader* pBattlefieldHeader, Battalion* pBattalion, TempGruntVector& grunts )
{
	// now allocate the units this calcutions are only valid for BS_UNORDERED_SQUARE (hence the assert)
	ntAssert( pBattalion->m_Shape == BS_UNORDERED_SQUARE );

	// fill in initial player tracking and circling bits
	pBattalion->m_BattalionFlags |= (pBattalionDef->m_bPlayerTracking) ? BF_PLAYER_TRACKING : 0;
	pBattalion->m_BattalionFlags |= (pBattalionDef->m_bPlayerCircle) ? BF_PLAYER_CIRCLE : 0;

	uint32_t iNumUnitTypes = 0;
	uint16_t iHasUnitOfType[ BUT_MAX ];

	memset( iHasUnitOfType, 0, BUT_MAX * sizeof(uint16_t) );
	// count actual unit types
	for( uint32_t i=0;i < pBattalion->m_ShapeData.m_UnorderedSquare.m_NumUnitTypes;i++)
	{
		uint8_t iUnitType;
		if( i & 0x1 )
		{
			iUnitType = NIBBLE_EXTRACT_TOP( pBattalion->m_ShapeData.m_UnorderedSquare.m_UnitType[ i>>1 ] );
		} else
		{
			iUnitType = NIBBLE_EXTRACT_BOTTOM( pBattalion->m_ShapeData.m_UnorderedSquare.m_UnitType[ i>>1 ] );
		}

		if( iHasUnitOfType[ iUnitType ] == 0 )
		{
			iNumUnitTypes++;
		}
		iHasUnitOfType[ iUnitType ]++;
	}

	ntAssert( iNumUnitTypes < MAX_UNITS_PER_BATTALION );

	pBattalion->m_iNumUnits = iNumUnitTypes;
	Unit* pFirstUnit = pBattalion->m_Units;
	Unit* pUnit = pFirstUnit;
	Unit* pUnitOfType[ BUT_MAX ];
	memset( pUnitOfType, 0, BUT_MAX * sizeof(Unit*) );

	pBattalion->m_iNumGrunts = pBattalion->m_ShapeData.m_UnorderedSquare.m_NumGrunts;
	uint16_t iCurGruntNum = grunts.size();
	// increase by the total number of grunts
	grunts.resize( grunts.size() + pBattalion->m_iNumGrunts );


	uint16_t iByteCheck = 0; // check our byte percetages add up to 255
	uint16_t iAccumGruntCount = 0;
	for( uint32_t i=0;i < pBattalion->m_ShapeData.m_UnorderedSquare.m_NumUnitTypes;i++)
	{
		uint8_t iUnitType;
		uint8_t iBytePercent;

		if( i & 0x1 )
		{
			iUnitType = NIBBLE_EXTRACT_TOP( pBattalion->m_ShapeData.m_UnorderedSquare.m_UnitType[ i>>1 ] );
		} else
		{
			iUnitType = NIBBLE_EXTRACT_BOTTOM( pBattalion->m_ShapeData.m_UnorderedSquare.m_UnitType[ i>>1 ] );
		}

		iBytePercent = pBattalion->m_ShapeData.m_UnorderedSquare.m_Percentage[ i ];
		iByteCheck += (uint16_t) iBytePercent;
		float fT = ((float)iBytePercent) * (1.f/255.f);

		if( pUnitOfType[iUnitType] == 0 )
		{
			ntAssert( (uint32_t)(pUnit - pFirstUnit) < iNumUnitTypes );

			pUnitOfType[iUnitType] = pUnit;
			pUnit->m_UnitID = pUnit - pFirstUnit;
			pUnit->m_UnitType = iUnitType;
			pUnit->m_NumGrunts = 0;
			pUnit->m_BattalionID = pBattalion->m_BattalionID;
			pUnit++;
		}
		// count how many grunts are in each unit type
		pUnitOfType[iUnitType]->m_NumGrunts += (uint16_t)(pBattalion->m_ShapeData.m_UnorderedSquare.m_NumGrunts * fT);
		iAccumGruntCount += pUnitOfType[iUnitType]->m_NumGrunts;

		// if the last one account for any rounding error
		if( i == (uint32_t) (pBattalion->m_ShapeData.m_UnorderedSquare.m_NumUnitTypes-1) )
		{
			int rounding = pBattalion->m_ShapeData.m_UnorderedSquare.m_NumGrunts - iAccumGruntCount;

			pUnitOfType[iUnitType]->m_NumGrunts += rounding;
		}
	}

	UNUSED( iByteCheck );
	ntError_p( iByteCheck == 255, ("all the percetnages must add upto 255, we got %i for %s", iByteCheck, pBattalionDef->m_Command.GetDebugString() ) );

	ntError( iNumUnitTypes < MAX_UNITS_PER_BATTALION );

	// now go thro each unit and allocate a cotinous block of grunts and mark the start
	uint16_t iUnitGruntIDTracker[ BUT_MAX ];
	for( uint32_t i=0;i < iNumUnitTypes; ++i )
	{
		Unit* pUnit = pFirstUnit + i;
		pUnit->m_FirstGruntID = iCurGruntNum;
		iUnitGruntIDTracker[ pUnit->m_UnitType ] = pUnit->m_FirstGruntID;
		iCurGruntNum += pUnit->m_NumGrunts;

	}

	// now build the grunts data in the formation 
	CDirection horiz( pBattalionDef->m_MaxCorner.X() - pBattalionDef->m_MinCorner.X(), 0, 0 );
	CDirection vert( 0, 0, pBattalionDef->m_MaxCorner.Z() - pBattalionDef->m_MinCorner.Z() );
	CPoint center = pBattalionDef->m_MinCorner + (horiz * 0.5f) + (vert * 0.5f);

	char pBattalionName[ 256 ] = "\0";
	UNUSED( pBattalionName );
#if !defined( _RELEASE )
	sprintf( pBattalionName, "%s", ObjectDatabase::Get().GetDataObjectFromPointer( pBattalionDef )->GetName().GetString() );
#endif


	BF_Position bf_center;
	bf_center.m_X = 0;
	bf_center.m_Y = 0;
	bool ret;
	UNUSED(ret);
	ret = 	BF_WorldSpaceToBF_Position( center, pBattlefieldHeader, &bf_center );
	user_error_p( ret, ("Center of Battalion %s Off the Battlefield\n", pBattalionName ) );

	for( uint32_t i=0;i < iNumUnitTypes; ++i )
	{
		Unit* pUnit = pFirstUnit + i;
		for( unsigned int k=0; k < pUnit->m_NumGrunts;k++)
		{
			GruntGameState& gr = grunts[ iUnitGruntIDTracker[ pUnit->m_UnitType ] ]; 

			float tHeight = grandf(1.f);
			float tWidth = grandf(1.f);

			// not really needed by just cut and pasted and might help break
			// clusters up even more...
			switch( pBattalion->m_FormationOrder )
			{
			case BFO_REGULAR: break;
			case BFO_SEMI_REGULAR:
				tWidth += (grandf(0.05f)-0.025f);
				tHeight += (grandf(0.02f)-0.01f);
				break;
			case BFO_BARBARIAN:
				tWidth += (grandf(0.1f)-0.05f);
				tHeight += (grandf(0.1f)-0.05f);
				break;
			case BFO_DISORDER:
				tWidth += (grandf(0.5f)-0.25f);
				tHeight += (grandf(0.5f)-0.25f);
				break;
			}
			// wrap mode
			while( tWidth > 1.0f )
			{
				tWidth -= 1.0f;
			}
			while( tWidth < 0.f )
			{
				tWidth += 1.0f;
			}
			while( tHeight > 1.0f )
			{
				tHeight -= 1.0f;
			}
			while( tHeight < 0.f )
			{
				tHeight += 1.0f;
			}


			CPoint worldPos = pBattalionDef->m_MinCorner + (tWidth * horiz) + (tHeight * vert);
			CPoint offset = worldPos - center;
			bool ret;
			UNUSED(ret);
			ret = BF_WorldSpaceToBF_Position( worldPos, pBattlefieldHeader, &gr.m_Position );
			user_error_p( ret, ("Grunt of Battalion %s starting off the Battlefield\n", pBattalionName) );

			BF_WorldSpaceToBF_Position( worldPos, pBattlefieldHeader, &gr.m_Position );
			gr.m_Orientation.m_Rotation = (uint16_t)(0.25f * 65535.f); // TODO 
			gr.m_GruntID		= iUnitGruntIDTracker[ pUnit->m_UnitType ]++;
			gr.m_BattalionID	= pUnit->m_BattalionID;
			gr.m_UnitID			= pUnit->m_UnitID;
			gr.m_GruntMajorState = GMS_NORMAL; // start people off alive and not shit scared :-D
			gr.m_BattalionOffsetX = (int16_t)gr.m_Position.m_X - (int16_t)bf_center.m_X;
			gr.m_BattalionOffsetY = (int16_t)gr.m_Position.m_Y - (int16_t)bf_center.m_Y;
			gr.m_IntendedPos.m_X = gr.m_Position.m_X;
			gr.m_IntendedPos.m_Y = gr.m_Position.m_Y;
		}
	}
}

void ArmySection::InstallWelderHookup()
{
	// clear the army unit parameters pointer first
	memset( m_pArmyUnitParameters, 0, sizeof(ArmyUnitParameters*) * MAX_UNIT_TYPES );

	// now lets try and install a pointer to a specific named object
	{
		DataObject* pFodderDO = ObjectDatabase::Get().GetDataObjectFromName( "ArmyUnitParam_Fodder" );
		if( pFodderDO && strcmp( pFodderDO->GetClassName(), "ArmyUnitParameters")== 0 )
		{
			m_pArmyUnitParameters[ BUT_FODDER ] = (ArmyUnitParameters*) pFodderDO->GetBasePtr();

#ifndef _RELEASE
			ntError( m_pArmyUnitParameters[ BUT_FODDER ]->m_SpawnPool );
			DataObject* pSpawnerDO = ObjectDatabase::Get().GetDataObjectFromPointer( m_pArmyUnitParameters[ BUT_FODDER ]->m_SpawnPool );
			ntError_p( strcmp( pSpawnerDO->GetClassName(), "SpawnPool") == 0, ("BUT_FODDER has invalid spawner def (wrong class)") );
#endif
			m_pArmyUnitParameters[ BUT_FODDER ]->m_SpawnPool->BuildPool();
		}
	}
	{
		DataObject* pSwordsmanDO = ObjectDatabase::Get().GetDataObjectFromName( "ArmyUnitParam_Swordsman" );
		if( pSwordsmanDO && strcmp( pSwordsmanDO->GetClassName(), "ArmyUnitParameters")== 0 )
		{
			m_pArmyUnitParameters[ BUT_SWORDSMAN ] = (ArmyUnitParameters*) pSwordsmanDO->GetBasePtr();

#ifndef _RELEASE
			ntError( m_pArmyUnitParameters[ BUT_SWORDSMAN ]->m_SpawnPool );
			DataObject* pSpawnerDO = ObjectDatabase::Get().GetDataObjectFromPointer( m_pArmyUnitParameters[ BUT_SWORDSMAN ]->m_SpawnPool );
			ntError_p( strcmp( pSpawnerDO->GetClassName(), "SpawnPool") == 0, ("BUT_SWORDSMAN has invalid spawner def (wrong class)") );
#endif
			m_pArmyUnitParameters[ BUT_SWORDSMAN ]->m_SpawnPool->BuildPool();

		}
	}
	{
		DataObject* pShieldsmanDO = ObjectDatabase::Get().GetDataObjectFromName( "ArmyUnitParam_Shieldsman" );
		if( pShieldsmanDO && strcmp( pShieldsmanDO->GetClassName(), "ArmyUnitParameters")== 0 )
		{
			m_pArmyUnitParameters[ BUT_SHIELDSMAN ] = (ArmyUnitParameters*) pShieldsmanDO->GetBasePtr();

#ifndef _RELEASE
			ntError( m_pArmyUnitParameters[ BUT_SHIELDSMAN ]->m_SpawnPool );
			DataObject* pSpawnerDO = ObjectDatabase::Get().GetDataObjectFromPointer( m_pArmyUnitParameters[ BUT_SHIELDSMAN ]->m_SpawnPool );
			ntError_p( strcmp( pSpawnerDO->GetClassName(), "SpawnPool") == 0, ("BUT_SHIELDSMAN has invalid spawner def (wrong class)") );
#endif
			m_pArmyUnitParameters[ BUT_SHIELDSMAN ]->m_SpawnPool->BuildPool();

		}
	}
	{
		DataObject* pAxeDO = ObjectDatabase::Get().GetDataObjectFromName( "ArmyUnitParam_Axeman" );
		if( pAxeDO && strcmp( pAxeDO->GetClassName(), "ArmyUnitParameters")== 0 )
		{
			m_pArmyUnitParameters[ BUT_AXEMAN ] = (ArmyUnitParameters*) pAxeDO->GetBasePtr();

#ifndef _RELEASE
			ntError( m_pArmyUnitParameters[ BUT_AXEMAN ]->m_SpawnPool );
			DataObject* pSpawnerDO = ObjectDatabase::Get().GetDataObjectFromPointer( m_pArmyUnitParameters[ BUT_AXEMAN ]->m_SpawnPool );
			ntError_p( strcmp( pSpawnerDO->GetClassName(), "SpawnPool") == 0, ("AXEMAN has invalid spawner def (wrong class)") );
#endif
			m_pArmyUnitParameters[ BUT_AXEMAN ]->m_SpawnPool->BuildPool();
		}
	}
	{
		DataObject* pBombDO = ObjectDatabase::Get().GetDataObjectFromName( "ArmyUnitParam_Bomberman" );
		if( pBombDO && strcmp( pBombDO->GetClassName(), "ArmyUnitParameters")== 0 )
		{
			m_pArmyUnitParameters[ BUT_BOMBERMAN ] = (ArmyUnitParameters*) pBombDO->GetBasePtr();

#ifndef _RELEASE
			ntError( m_pArmyUnitParameters[ BUT_BOMBERMAN ]->m_SpawnPool );
			DataObject* pSpawnerDO = ObjectDatabase::Get().GetDataObjectFromPointer( m_pArmyUnitParameters[ BUT_BOMBERMAN ]->m_SpawnPool );
			ntError_p( strcmp( pSpawnerDO->GetClassName(), "SpawnPool") == 0, ("BOMBERMAN has invalid spawner def (wrong class)") );
#endif
			m_pArmyUnitParameters[ BUT_BOMBERMAN ]->m_SpawnPool->BuildPool();
		}
	}

	DataObject* pArmyDataDO = ObjectDatabase::Get().GetDataObjectFromName( "ArmyParam_Generic" );
	if( pArmyDataDO && strcmp( pArmyDataDO->GetClassName(), "ArmyGenericParameters")== 0 )
	{
		m_pArmyGenericParameters = (ArmyGenericParameters*) pArmyDataDO->GetBasePtr();
	} else
	{
		// if hasn't been provided make our own one
		m_pArmyGenericParameters = ObjectDatabase::Get().Construct<ArmyGenericParameters>( "ArmyGenericParameters" );
	}
	// first time (in _RELEASE this is the only time this gets moved over...)
	UpdateFromWelder();
}

void ArmySection::CreatePreBattalionDMAStructures( BattlefieldHeader& battlefieldHeader, HeightfieldFileHeader*& pHeightfieldData )
{
	// requesting 
	m_pSortDependency =  NT_NEW_CHUNK( Mem::MC_ARMY ) DependencyCounter();
	m_pRenderDependency = NT_NEW_CHUNK( Mem::MC_ARMY ) DependencyCounter();
	m_pBattalionDependency = NT_NEW_CHUNK( Mem::MC_ARMY ) DependencyCounter();
	
	// removed as not safe over level restart
//	m_pDoAnimationsPortNum = Exec::AddPPUCallbackHandler( DoAnimationCallbackStatic, 1 );
//	ntAssert( m_pDoAnimationsPortNum != 0xFFFFFFFF );


	// EVIL HACK to test the theory of non-square bf affecting things
	{
		ArmyBattlefield* pEditArmyArena = const_cast<ArmyBattlefield*>(m_pArmyArena);
		float maxm = ntstd::Max( pEditArmyArena->m_MaxCorner.X() - pEditArmyArena->m_MinCorner.X(), pEditArmyArena->m_MaxCorner.Z() - pEditArmyArena->m_MinCorner.Z());
		pEditArmyArena->m_MaxCorner.X() = pEditArmyArena->m_MinCorner.X()  + maxm;
		pEditArmyArena->m_MaxCorner.Z() = pEditArmyArena->m_MinCorner.Z()  + maxm;
	}
/*
	ntPrintf( "Min <%f,%f,%f> Max <%f, %f, %f>\n",	m_pArmyArena->m_MinCorner.X(),
													m_pArmyArena->m_MinCorner.Y(),
													m_pArmyArena->m_MinCorner.Z(),
													m_pArmyArena->m_MaxCorner.X(),
													m_pArmyArena->m_MaxCorner.Y(),
													m_pArmyArena->m_MaxCorner.Z() ); */

	// see if we need to generate the heightfield
//	if( m_pArmyArena->m_HeightfieldFilename.IsNull() )
	{
		pHeightfieldData = GenerateHeightfield( m_pArmyArena->m_MinCorner, 
												m_pArmyArena->m_MaxCorner ^ m_pArmyArena->m_MinCorner, 
												m_pArmyArena->m_HFWidth, 
												m_pArmyArena->m_HFHeight );
		memset( &battlefieldHeader, 0, sizeof( battlefieldHeader ) );

		battlefieldHeader.m_TotalWidth = pHeightfieldData->m_Width;
		battlefieldHeader.m_TotalHeight = pHeightfieldData->m_Height;
		CDirection diag = m_pArmyArena->m_MaxCorner ^ m_pArmyArena->m_MinCorner;
		battlefieldHeader.m_WorldTopLeft = m_pArmyArena->m_MinCorner;
		battlefieldHeader.m_WorldHoriz = diag.X();
		battlefieldHeader.m_WorldVert = diag.Z();

	}/* else
	{
		// TODO load an existing saved out heightfield.. .should this ever be done 
		// should probably just load battlefield chunks directly...
		ntAssert( false );
	}*/

	// lets create the battlefield and 
	m_pBattlefield = NT_NEW_CHUNK( Mem::MC_ARMY) Battlefield();
	memset( m_pBattlefield, 0, sizeof( Battlefield ) );

	m_pBattlefield->m_pBattalions = (BattalionArray*) DMABuffer::DMAAlloc( sizeof( BattalionArray ), 0 , Mem::MC_ARMY );
	memset( m_pBattlefield->m_pBattalions, 0, sizeof( BattalionArray ) );

	user_error_p( m_pArmyArena->m_ArmyBattalionDefs.size() < MAX_BATTALIONS, ("Too many battalions on the battlefield\n") );
	m_pBattlefield->m_pBattalions->m_iNumBattalions = m_pArmyArena->m_ArmyBattalionDefs.size();

	// events
	m_pEvents = NT_NEW_ARRAY_CHUNK( Mem::MC_ARMY ) BattlefieldEvent[ MAX_BATTLEFIELD_EVENTS ];
	m_iNumEvents = 0;
	for( int i=0;i < MAX_BATTLEFIELD_EVENTS;++i )
	{
		m_EventUpdates[i] = (BattlefieldEventUpdate*) DMABuffer::DMAAlloc( sizeof( BattlefieldEventUpdate ), 0, Mem::MC_ARMY, 128 );
		memset( m_EventUpdates[i], 0, sizeof( BattlefieldEventUpdate ) );
	}

	m_uiSoundRocketFireID=0;
	m_uiSoundExplosionID=0;
}

void ArmySection::CreateStaticAISegments( DataObject* pConFieldDO )
{
	// lets try and get an specific AI volume, that we use as the army containment field
	if( pConFieldDO && strcmp( pConFieldDO->GetClassName(), "CAIWorldVolume")== 0 )
	{
		const CAIWorldVolume* pWorldVol = (const CAIWorldVolume*)pConFieldDO->GetBasePtr();
		const CAIWorldVolume::SAIVolSegmentVector& segments = pWorldVol->GetSegmentList();

		ntAssert( segments.size() < BattlefieldHeader::MAX_STATIC_SEGMENTS );
		ntAssert( m_pBattlefield->m_pHeader->m_iNumStaticPolygons < MAX_STATIC_AI_POLYGONS );

		uint32_t iStartIndex = 0;
		// find where we are in the segment array
		for( unsigned int i=0;i < m_pBattlefield->m_pHeader->m_iNumStaticPolygons;i++)
		{
			iStartIndex += m_pBattlefield->m_pHeader->m_iNumStaticSegments[i];
		}

		// the AI segments are 3D but i use them as 2D to save RAM and cos it makes more sense
		for( unsigned int i=0;i < segments.size();i++ )
		{
			bool ret;
			UNUSED(ret);
			ret = BF_WorldSpaceToBF_Position( segments[i].P0, m_pBattlefield->m_pHeader, &m_pBattlefield->m_pHeader->m_StaticSegments[iStartIndex+i].P0 );
			ntAssert( ret );
			ret = BF_WorldSpaceToBF_Position( segments[i].P1, m_pBattlefield->m_pHeader, &m_pBattlefield->m_pHeader->m_StaticSegments[iStartIndex+i].P1 );
			ntAssert( ret );
		}
		m_pBattlefield->m_pHeader->m_iNumStaticSegments[m_pBattlefield->m_pHeader->m_iNumStaticPolygons] = (uint8_t) segments.size();
		m_pBattlefield->m_pHeader->m_iNumStaticPolygons++;

	}
}
void ArmySection::CreatePostBattalionDMAStructures( const BattlefieldHeader& battlefieldHeader, HeightfieldFileHeader* pHeightfieldData, const TempGruntVector& tmpGrunts ) 
{
	m_pArmyAdder = (ExecSPUJobAdder*) DMABuffer::DMAAlloc( sizeof(ExecSPUJobAdder), 0, Mem::MC_ARMY );
	m_pRenderAdder = (ExecSPUJobAdder*) DMABuffer::DMAAlloc( sizeof(ExecSPUJobAdder), 0, Mem::MC_ARMY );

	ntAssert_p( tmpGrunts.size() < MAX_GRUNTS, ("Max %i grunts\n", MAX_GRUNTS) );
	// allocate the DMA buffer for the maximum number of grunts
	uint32_t iSize = sizeof(GruntArray) + (sizeof(GruntGameState) * tmpGrunts.size()) + 
											(sizeof(uint16_t) * tmpGrunts.size());

	m_pBattlefield->m_pGrunts = (GruntArray*) DMABuffer::DMAAlloc( iSize, 0, Mem::MC_ARMY );
	m_pBattlefield->m_pGrunts->m_iSize = DMABuffer::DMAAllocSize( iSize );
	m_pBattlefield->m_pGrunts->m_iNumGrunts = tmpGrunts.size();
	// copy the grunt data where we really want it 
	GruntGameState* pGrunts = (GruntGameState*) (m_pBattlefield->m_pGrunts+1);
	NT_MEMCPY( pGrunts, &tmpGrunts[0], sizeof(GruntGameState) * tmpGrunts.size() );

	m_pBattlefield->m_pHeader = (BattlefieldHeader*) DMABuffer::DMAAlloc( sizeof( BattlefieldHeader ), 0, Mem::MC_ARMY );
	memset( m_pBattlefield->m_pHeader, 0, sizeof( BattlefieldHeader ) );
	CreateBattlefield( &battlefieldHeader, (float*)(pHeightfieldData+1), m_pBattlefield );
	NT_FREE_CHUNK( Mem::MC_MISC, (uintptr_t)pHeightfieldData );

	// lets try and get an specific AI volume, that we use as the army containment field
	DataObject* pConField0DO = ObjectDatabase::Get().GetDataObjectFromName( m_pArmyArena->m_ContainmentFieldAIVol0 );
	CreateStaticAISegments( pConField0DO );
	DataObject* pConField1DO = ObjectDatabase::Get().GetDataObjectFromName( m_pArmyArena->m_ContainmentFieldAIVol1 );
	CreateStaticAISegments( pConField1DO );
	DataObject* pConField2DO = ObjectDatabase::Get().GetDataObjectFromName( m_pArmyArena->m_ContainmentFieldAIVol2 );
	CreateStaticAISegments( pConField2DO );
	DataObject* pConField3DO = ObjectDatabase::Get().GetDataObjectFromName( m_pArmyArena->m_ContainmentFieldAIVol3 );
	CreateStaticAISegments( pConField3DO );

	ArmyBattlefield::ArmyObstacleDefList::const_iterator odlIt = m_pArmyArena->m_ArmyObstacleDefs.begin();
	for( unsigned int i = 0;
		 odlIt != m_pArmyArena->m_ArmyObstacleDefs.end(); ++odlIt, ++i )
	{
		user_error_p( (i < MAX_BATTLEFIELD_OBSTACLES), ("Too many Army Obstacles\n") );

		BattlefieldHeader* pBFHeader = m_pBattlefield->m_pHeader;
		bool ret;
		UNUSED(ret);
		ret = BF_WorldSpaceToBF_Position( (*odlIt)->m_Centre, pBFHeader, &pBFHeader->m_Obstacles[ i ].m_Location );
		user_error_p( ret, ("Obstacle Off the Battelefield\n") );
		float bfRadius = CalculuteRadiusInBFSpace( (*odlIt)->m_Radius );
		pBFHeader->m_Obstacles[ i ].m_Radius = (uint16_t) bfRadius;
		pBFHeader->m_Obstacles[ i ].m_Attract = 0;
		pBFHeader->m_iNumObstacles = (uint8_t) i;
	}

	ArmyBattlefield::ArmyBattalionDefList::const_iterator bdlIt = m_pArmyArena->m_ArmyBattalionDefs.begin();
	for( unsigned int i = 0;
		 bdlIt != m_pArmyArena->m_ArmyBattalionDefs.end(); ++bdlIt, ++i )
	{
		const ArmyBattalion* pBattalionDef = (*bdlIt);
		Battalion* pBattalion = &m_pBattlefield->m_pBattalions->m_BattalionArray[i];


		char pBattalionName[ 256 ] = "\0";
		UNUSED( pBattalionName );
#if !defined( _RELEASE )
		sprintf( pBattalionName, "%s", ObjectDatabase::Get().GetDataObjectFromPointer( pBattalionDef )->GetName().GetString() );
#endif

		CDirection deltaPos( CONSTRUCT_CLEAR );
		const ArmyBattalion::CommandList* pCommandList;
		if( pBattalionDef->m_pBattalionToCopyCommandsFrom != 0 )
		{
#if !defined( _RELEASE )
			DataObject* pCopyFromDO = ObjectDatabase::Get().GetDataObjectFromPointer( pBattalionDef->m_pBattalionToCopyCommandsFrom );
			bool isCorrectClass = (strcmp( pCopyFromDO->GetClassName(),"ArmyBattalion" ) == 0);
			user_error_p( isCorrectClass, 
				("Battalion %s is trying to copy command from %s but this is not an ArmyBattalion\n", pBattalionName, pCopyFromDO->GetName().GetString() ) );
#endif
			pCommandList = &pBattalionDef->m_pBattalionToCopyCommandsFrom->m_ArmyBattalionCommands;
			deltaPos = pBattalionDef->m_MaxCorner ^ pBattalionDef->m_pBattalionToCopyCommandsFrom->m_MaxCorner;
		} else
		{
			pCommandList = &pBattalionDef->m_ArmyBattalionCommands;
		}

		user_error_p( pCommandList->size()< MAX_BATTALION_COMMANDS, ("Too many command for battalion %s\n",pBattalionName) );

		pBattalion->m_CommandNum = 0;
		pBattalion->m_BattalionSpeedPercentageMultiplier = 1.f;
		pBattalion->m_BattalionCohersionFactor = 0.3f;

		// debug check for duplicate command sequence
		uint8_t bCommandSafety[MAX_BATTALION_COMMANDS];
		memset( bCommandSafety, 0, sizeof(uint8_t) * MAX_BATTALION_COMMANDS );


		ArmyBattalion::CommandList::const_iterator clIt = pCommandList->begin();
		while( clIt != pCommandList->end() )
		{
			bool ret;
			UNUSED(ret);


			// safety checks
			user_error_p( bCommandSafety[ (*clIt)->m_iSequence ] == 0, ("Battalion Command duplicate sequence id %i in %s\n", (*clIt)->m_iSequence, pBattalionName ) );
			bCommandSafety[ (*clIt)->m_iSequence ] = 1;

			user_error_p( (*clIt)->m_iCommand < MAX_BATTALION_COMMAND_INSTR, ("Invalid Battalion command command >64 sequence %i in %s\n",(*clIt)->m_iSequence, pBattalionName ) );
			user_error_p( (*clIt)->m_iParam0 < MAX_BATTALION_COMMAND_PARAM0, ("Invalid Battalion command Param0 >65355 sequence %i in %s\n", (*clIt)->m_iSequence, pBattalionName ) );
			user_error_p( (*clIt)->m_iParam1 < MAX_BATTALION_COMMAND_PARAM1, ("Invalid Battalion command Param1 >1024 sequence %i in %s\n", (*clIt)->m_iSequence, pBattalionName ) );

			BattalionCommand* pBatCom = &pBattalion->m_BattalionCommands[ (*clIt)->m_iSequence ];

			ret = BF_WorldSpaceToBF_Position( ((*clIt)->m_vPos + deltaPos), m_pBattlefield->m_pHeader, &pBatCom->m_Pos );
			user_error_p( ret, ("Invalid position in battalion command sequence %i in %s\n", (*clIt)->m_iSequence, pBattalionName ) );

			pBatCom->m_iCommand = (uint16_t) (*clIt)->m_iCommand;
			pBatCom->m_iParam0 = (uint16_t) (*clIt)->m_iParam0;
			pBatCom->m_iParam1 = (uint16_t) (*clIt)->m_iParam1;

			if( (*clIt)->m_iSequence == 0 )
			{
				pBattalion->m_curParam0 = pBatCom->m_iParam0;
			}

			++clIt;
		}

		if( bCommandSafety[0] != 1 )
		{
			BattalionCommand* pBatCom = &pBattalion->m_BattalionCommands[ 0 ];
			pBatCom->m_iCommand = BL_NOOP;
			pBatCom->m_iParam0 = 0;
			pBatCom->m_iParam1 = 0;
			bCommandSafety[0] = 1;
		}

		bool bGapDetected = false;
		user_error_p( bCommandSafety[0] == 1, ("There must always be a 0th sequence id in a %s's list of commands\n", pBattalionName ) );
		for( int j=0;j < MAX_BATTALION_COMMANDS;++j)
		{
			if( bCommandSafety[j] == 0 && bGapDetected == false )
			{
				bGapDetected = true;
			}
			if( bCommandSafety[j] == 1 && bGapDetected == true )
			{
				user_error_p( false, ("%s's commands sequences numbers are not contigous", pBattalionName) );
			}
		}

	}
	// in non release mode allow some parameters to be changed real-time from welder
	InstallWelderHookup();

//	user_error_p( m_pBattlefield->m_pGrunts->m_iNumGrunts < MAX_ARMY_PEEPS, ("Too many grunts in total!!!") );

	// set battlefield total time to 0
	m_pBattlefield->m_pHeader->m_accumTime = 0;

	// lets now create all the renderables eeek!!!
	uint32_t iUnitTypeCount[ MAX_UNIT_TYPES ];
	memset( iUnitTypeCount, 0, sizeof(uint32_t) * MAX_UNIT_TYPES );

	// we need to count how many of each type of grunt we will need MAX
	for( uint32_t i=0;i < m_pBattlefield->m_pGrunts->m_iNumGrunts;i++)
	{
		GruntGameState* pGrunt = &((GruntGameState*)(m_pBattlefield->m_pGrunts+1))[i];
		const Battalion* pBattalion = &m_pBattlefield->m_pBattalions->m_BattalionArray[pGrunt->m_BattalionID];
		const Unit* pUnit = &pBattalion->m_Units[ pGrunt->m_UnitID ];

		iUnitTypeCount[ pUnit->m_UnitType ]++;
	}

	m_pBattlefield->m_pRenderDestBuffer = (GruntRenderState*) DMABuffer::DMAAlloc( sizeof(GruntRenderState) * m_pBattlefield->m_pGrunts->m_iNumGrunts, 0, Mem::MC_ARMY );
	memset(m_pBattlefield->m_pRenderDestBuffer, 0, sizeof(GruntRenderState) * m_pBattlefield->m_pGrunts->m_iNumGrunts );

	m_pBattlefield->m_pRenderables = (ArmyRenderable**) NT_NEW_CHUNK( Mem::MC_ARMY ) ArmyRenderable*[ m_pBattlefield->m_pGrunts->m_iNumGrunts ];
	memset( m_pBattlefield->m_pRenderables, 0, sizeof(ArmyRenderable**) * m_pBattlefield->m_pGrunts->m_iNumGrunts );

	for( uint32_t i=0;i < MAX_UNIT_TYPES;i++)
	{
		if( iUnitTypeCount[ i ] > 0 )
		{
			ntError_p( m_pArmyUnitParameters[i], ("no unit paramaters for type %i", i) );
			uint32_t iPoolSize = ntstd::Min( (uint32_t) m_pArmyUnitParameters[i]->m_iMaxPoolSize, iUnitTypeCount[i] );
			m_pBattlefield->m_pRenderPools[i] = NT_NEW_CHUNK( Mem::MC_ARMY ) ArmyRenderPool( m_pBattlefield, m_pArmyUnitParameters[i],  iPoolSize, m_pBattlefield->m_pGrunts->m_iNumGrunts );

			// special case for bomber-men
			if( i == BUT_BOMBERMAN )
			{
				m_pBattlefield->m_pRenderPools[i]->AddBombs( m_pArmyUnitParameters[i]->m_BombName );
			}
		} else
		{
			m_pBattlefield->m_pRenderPools[i] = 0;
		}
	}

	m_pBattlefield->m_pHeader->m_eaInfo = (BattlefieldInfo*) DMABuffer::DMAAlloc( sizeof( BattlefieldInfo ), 0, Mem::MC_ARMY, 128 );
	memset( m_pBattlefield->m_pHeader->m_eaInfo, 0, sizeof(BattlefieldInfo) );

	// start with line 0
	m_pBattlefield->m_pHeader->m_iStagingPostLine = 0;

	m_pBattlefield->m_pHeader->m_eaGruntRenderAllocator = (ArmyGruntRenderAllocator*) DMABuffer::DMAAlloc( sizeof(ArmyGruntRenderAllocator), 0, Mem::MC_ARMY, 128 );
	memset( m_pBattlefield->m_pHeader->m_eaGruntRenderAllocator, 0, sizeof(ArmyGruntRenderAllocator) );

#ifndef _NO_DBGMEM_OR_RELEASE
	static_assert( sizeof(SPU_DebugLine) == 48, SPY_DebugLine_not_equal_48_bytes );
	m_pBattlefield->m_pHeader->m_iNumDebugLines = (uint32_t*)DMABuffer::DMAAlloc( sizeof(uint32_t), 0, Mem::MC_DEBUG );
	m_pBattlefield->m_pHeader->m_pDebugLineBuffer = (SPU_DebugLine*)DMABuffer::DMAAlloc( sizeof(SPU_DebugLine) * MAX_ARMY_SPU_DEBUG_LINES, 0, Mem::MC_DEBUG );
	AtomicSet( m_pBattlefield->m_pHeader->m_iNumDebugLines, 0 );
#endif

	{
		ArmyBattlefield::ArmyBattalionDefList::const_iterator bdlIt = m_pArmyArena->m_ArmyBattalionDefs.begin();
		for( unsigned int i = 0;
			bdlIt != m_pArmyArena->m_ArmyBattalionDefs.end(); ++bdlIt, ++i )
		{
			const ArmyBattalion* pBattalionDef = (*bdlIt);
			if( pBattalionDef->m_bHasFlag == true )
			{
				Battalion* pBattalion = &m_pBattlefield->m_pBattalions->m_BattalionArray[i];
				// now scan thro the battalion creating flags for everyone in this battalion
				for( unsigned int j=0;j < tmpGrunts.size();j++)
				{
					const GruntGameState& gr = tmpGrunts[j];
					if( gr.m_BattalionID == pBattalion->m_BattalionID )
					{
						CreateFlagForGrunt( gr.m_GruntID );
					}
				}
			}  
		}
	}

	// slightly memory wasteful (8K or so) but for now only simplist way, think
	// i need to refactor unit_render to have a index array and not rearange the render grunts themself...
	m_pFlagArray = NT_NEW_ARRAY_CHUNK(Mem::MC_ARMY) uint8_t[m_pBattlefield->m_pGrunts->m_iNumGrunts];
	memset( m_pFlagArray, 0xFF, sizeof(uint8_t)*m_pBattlefield->m_pGrunts->m_iNumGrunts );

	ntError( m_ArmyFlags.size() < 255 );
	// update flag positions
	for( unsigned int i=0;i < m_ArmyFlags.size();i++)
	{
		m_pFlagArray[m_ArmyFlags[i].m_GruntID] = (uint8_t)i;
	}
}


ArmySection::ArmySection() :
	m_pArmyArena( 0 ),
	m_iDebugMode( 0 ),
	m_pSortDependency( 0 ),
	m_pRenderDependency( 0 ),
	m_pBattalionDependency( 0 ),
	m_pBattlefield( 0 ),
	m_pArmyAdder( 0 ),
	m_pRenderAdder( 0 ),
	m_iNumEvents( 0 ),
	m_pEvents( 0 ),
	m_uiSoundRocketFireID( 0 ),
	m_uiSoundExplosionID( 0 ),
	m_pArmyGenericParameters( 0 ),
	m_bFirstFrameDone( false )
{
	memset( m_EventUpdates, 0, sizeof(BattlefieldEventUpdate*) * MAX_BATTLEFIELD_EVENTS );
	memset( m_pArmyUnitParameters, 0, sizeof(ArmyUnitParameters*) * MAX_UNIT_TYPES );
}

ArmySection::~ArmySection()
{
	if( m_pBattlefield )
	{
		// clean up flags
		for( unsigned int i=0;i < m_ArmyFlags.size();i++)
		{

			m_ArmyFlags[i].m_pFlag->SetParentTransform( 0 );
			if( m_ArmyFlags[i].m_pFlagPole->GetRootTransform()->GetParent() )
			{
				m_ArmyFlags[i].m_pFlagPole->GetRootTransform()->RemoveFromParent();
			}

			NT_DELETE_CHUNK( Mem::MC_ARMY, m_ArmyFlags[i].m_pFlagPole );
		}
		NT_DELETE_ARRAY_CHUNK( Mem::MC_ARMY, m_pFlagArray );


		// make sure any AI have been removed 
		for ( uint32_t i=0; i < m_pBattlefield->m_pGrunts->m_iNumGrunts;++i )
		{
			if( m_pBattlefield->m_pRenderables[i] != 0 )
			{
				AI* pAI = m_pBattlefield->m_pRenderables[i]->GetRealDudeEntity();
				if( pAI )
				{
					pAI->RemoveFromWorld( false );
				}
			}
		}

		for( uint32_t i=0;i < MAX_UNIT_TYPES;i++)
		{
			if ( m_pBattlefield->m_pRenderPools[i])
			{
				NT_DELETE_CHUNK( Mem::MC_ARMY, m_pBattlefield->m_pRenderPools[i] );
			}
		}

		NT_DELETE_CHUNK( Mem::MC_ARMY, m_pBattlefield->m_pRenderables );

		DMABuffer::DMAFree( m_pBattlefield->m_pRenderDestBuffer, Mem::MC_ARMY );
	}

	for( int i=0;i < MAX_BATTLEFIELD_EVENTS;i++)
	{
		DMABuffer::DMAFree( m_EventUpdates[i], Mem::MC_ARMY );
	}
	NT_DELETE_CHUNK( Mem::MC_ARMY, m_pEvents );

	if( m_pBattlefield )
	{
		DMABuffer::DMAFree( m_pBattlefield->m_pHeader->m_eaGruntRenderAllocator, Mem::MC_ARMY );
		DMABuffer::DMAFree( m_pBattlefield->m_pHeader->m_eaInfo, Mem::MC_ARMY );

#ifndef _NO_DBGMEM_OR_RELEASE
		DMABuffer::DMAFree( m_pBattlefield->m_pHeader->m_iNumDebugLines, Mem::MC_DEBUG );
		DMABuffer::DMAFree( m_pBattlefield->m_pHeader->m_pDebugLineBuffer, Mem::MC_DEBUG );
#endif
		DMABuffer::DMAFree( m_pBattlefield->m_pBattalions, Mem::MC_ARMY );

	//	BattlefieldChunk** pChunks = (BattlefieldChunk**)(m_pBattlefield->m_pChunksHeader+1);
		BattlefieldChunk** pChunks = (BattlefieldChunk**)(m_pBattlefield->m_pChunksHeader->m_pChunks);
		for( uint16_t i=0; i < m_pBattlefield->m_pChunksHeader->m_iNumChunks; ++i )
		{
			DMABuffer::DMAFree( pChunks[i], Mem::MC_ARMY );
		}
		DMABuffer::DMAFree( m_pBattlefield->m_pChunksHeader, Mem::MC_ARMY );

		DMABuffer::DMAFree( m_pBattlefield->m_pGrunts, Mem::MC_ARMY );
		DMABuffer::DMAFree( m_pBattlefield->m_pHeader, Mem::MC_ARMY );

		NT_DELETE_CHUNK( Mem::MC_ARMY, m_pBattlefield );
		m_pBattlefield = 0;
	}

	DMABuffer::DMAFree( m_pRenderAdder, Mem::MC_ARMY );
	DMABuffer::DMAFree( m_pArmyAdder, Mem::MC_ARMY );

	// killed
//	Exec::RemovePPUCallbackHandler( m_pDoAnimationsPortNum );
	NT_DELETE_CHUNK( Mem::MC_ARMY, m_pBattalionDependency );
	NT_DELETE_CHUNK( Mem::MC_ARMY, m_pSortDependency );
	NT_DELETE_CHUNK( Mem::MC_ARMY, m_pRenderDependency );
}

void ArmySection::CreateDefaultBattalion( const ArmyBattalion* pBattalionDef, const BattlefieldHeader* pBattlefieldHeader, Battalion* pBattalion, TempGruntVector& grunts )
{

	// wavey 6x10 fodder fodder shieldman
	pBattalion->m_FormationOrder = BFO_SEMI_REGULAR;				// a tightly controlled unit
	pBattalion->m_Shape = BS_ORDERED_LINES;					// made up of ordered lines
	pBattalion->m_Orders = BO_FORWARD_MARCH;				// start with a forward march
	// of this many grunts and this set of units
	pBattalion->m_ShapeData.m_OrderedLine.m_Width = 8;
	pBattalion->m_ShapeData.m_OrderedLine.m_NumLines = 2;
	pBattalion->m_ShapeData.m_OrderedLine.m_DepthPerLine = 2;
	pBattalion->m_ShapeData.m_OrderedLine.m_UnitInLine[0] = NIBBLE_PACK( BUT_FODDER, BUT_FODDER );
	pBattalion->m_ShapeData.m_OrderedLine.m_UnitInLine[1] = NIBBLE_PACK( BUT_FODDER, 0 );

	FillInOrderedLineBattalion( pBattalionDef, pBattlefieldHeader, pBattalion, grunts );
}



int32_t ArmySection::GetObstacleIndexFromName( const CHashedString& str )
{
	ArmyObstacle* const pObstacle = ObjectDatabase::Get().GetPointerFromName<ArmyObstacle*>( str );

	if( pObstacle == 0 )
		return -1;

	ArmyBattlefield::ArmyObstacleDefList::const_iterator odlIt = m_pArmyArena->m_ArmyObstacleDefs.begin();
	for( unsigned int i = 0;
		 odlIt != m_pArmyArena->m_ArmyObstacleDefs.end(); ++odlIt, ++i )
	{
		if( (*odlIt) == pObstacle )
			return i;
	}

	return -1;
}
void ArmySection::CreateFlagForGrunt( uint16_t gruntID )
{
	if( Physics::VerletManager::IsEnabled() == true )
	{
		char sName[128];
		sprintf(sName, "%s_%d", "ArmyFlag", m_ArmyFlags.size() );

		DataObject* pFlagObjToClone = ObjectDatabase::Get().GetDataObjectFromName( CHashedString(m_pArmyGenericParameters->m_FlagTemplateName) );
		DataObject* pFlagDO = ObjectDatabase::Get().CloneObject( pFlagObjToClone, sName );
		Template_Flag* pFlag = pFlagDO->GetBaseObject<Template_Flag>();

		ArmyRenderable* pRenderable = NT_NEW_CHUNK( Mem::MC_ARMY ) ArmyRenderable();
		ArmyRenderableDef def( m_pArmyGenericParameters->m_FlagPoleClumpName, CHashedString(), false, false );
		pRenderable->Init( def );

		CPoint flagVec = CPoint(0, m_pArmyGenericParameters->m_fFlagPoleHeight, 0 );
		pFlag->SetParentTransform( pRenderable->GetRootTransform() ); 
		pFlag->SetPosition( flagVec );


		ArmyFlagEntry flag;
		flag.m_GruntID = gruntID;
		flag.m_pFlag = pFlag;
		flag.m_pFlagPole = pRenderable;
		m_ArmyFlags.push_back( flag );
	}
}
