/***************************************************************************************************
*
*
***************************************************************************************************/

//**************************************************************************************
//	Includes files.
//**************************************************************************************
#include "ntlib_spu/vecmath_spu_ps3.h"
#include "ntlib_spu/util_spu.h"
#include "ntlib_spu/debug_spu.h"
#include "ntlib_spu/fixups_spu.h"
#include "ntlib_spu/exec_spu.h"

#include "army/army_ppu_spu.h"
#include "army/army_math_spu.h"
#include "army/battalion.h"
#include "army/battlefield.h"
#include "army/unit.h"
#include "army/grunt.h"


// this is a form of the polygon collision that takes a inset value that contracts along each edges normal to make a smaller
// polygon for the purposes of battalion collision
bool IntersectsStaticSegments ( const BattlefieldHeader* restrict pBattlefieldHeader, const v128 p0, const v128 p1, const float inset )
{
	const v128 simdP = spu_shuffle( p0, p1, VECTORMATH_MAKE_SHUFFLE( VECTORMATH_SHUF_X, VECTORMATH_SHUF_Y, VECTORMATH_SHUF_A, VECTORMATH_SHUF_B) );

	vector unsigned int inside = spu_splats( (unsigned int) 0 );
	vector unsigned int outside = spu_splats( (unsigned int) 0xFFFFFFFF );

	const v128 vInset = spu_splats( inset );

	uint16_t iStartIndex = 0;

	for( uint8_t j=0;j < pBattlefieldHeader->m_iNumStaticPolygons;++j )
	{
		vector unsigned int flags = spu_splats( (unsigned int) 0);
		// lets just check the battalion, before we project it forward
		for( uint8_t i=0;i < pBattlefieldHeader->m_iNumStaticSegments[j];++i )
		{
			// unpack the incoming segment into vector registers
			const ArmyStaticAISegment* restrict obSeg1 = &pBattlefieldHeader->m_StaticSegments[i];

			// todo optimise using vector instructions (2 maybe 3 instructions..)
			v128 seg1P0 = (v128){obSeg1->P0.m_X, obSeg1->P0.m_Y, obSeg1->P0.m_X, obSeg1->P0.m_Y};
			v128 seg1Norm = (v128){	-(obSeg1->P1.m_Y-obSeg1->P0.m_Y), (obSeg1->P1.m_X-obSeg1->P0.m_X), 
									-(obSeg1->P1.m_Y-obSeg1->P0.m_Y), (obSeg1->P1.m_X-obSeg1->P0.m_X) };

			// adjust the point on the edge by the inset value along the edge normal
			seg1P0 = spu_madd( Normalise2D( seg1Norm ), vInset, seg1P0 );

			v128 delta01 = spu_sub( simdP, seg1P0  );

			// <0 = back side of line >0 front side of line
			v128 d01 = Dot2D( delta01, seg1Norm );

			// extract the sign (top bit of each element)
			vector unsigned int signd01 = Intrinsics::SPU_Sign( d01 );

			// if either point backfaces any segment the relevant field will be non-zero
			flags = spu_or( flags, signd01 );

			// not really used we could yet but this would give a bit field of each segment
			// tho should keep it in vectors if we were to do this
			//unsigned int iRes = spu_extract( signd01, 2 );
			//flags |= (iRes >> 31) << i;
		}
		iStartIndex += pBattlefieldHeader->m_iNumStaticSegments[j];


		// each bit of flags is compared to 0 
		// or'ed into inside if equal
		// and'ed into outside if equal
		// inside is a component wise if the element inside a polygon (will be >0 if so)
		// outside is a component sie if the element is outside a polygon (will be zero if so)
		inside = spu_or( inside, spu_cmpeq(flags,0) );
		outside = spu_and( outside, spu_cmpeq(flags, 0) );

	}

	// are we starting inside a polygon?
	if( spu_extract(inside,0) != 0 )
	{
		// is the dest point outside this polygon
		if( spu_extract(outside,2) == 0 )
		{
			return true;
		} else
		{
			return false;
		}
		
		return false;
	} else
	{
		return true;
	}
}

// this copies 2 commands (current and next) over to the interupt stack
// then adds in a iret, after this you can patch in your interupt command at pBattalion->m_CommandNum
void AddInteruptCommand( Battalion* pBattalion )
{
	ntAssert( pBattalion->m_CommandNum < MAX_BATTALION_COMMANDS );
	pBattalion->m_InteruptStack[0] = pBattalion->m_BattalionCommands[ pBattalion->m_CommandNum ];
	pBattalion->m_InteruptStack[1] = pBattalion->m_BattalionCommands[ pBattalion->m_CommandNum+1 ];
	pBattalion->m_InteruptStack[2] = pBattalion->m_BattalionCommands[ pBattalion->m_CommandNum+2 ];

	// this allows for single op interupt or 2 op interupt
	pBattalion->m_BattalionCommands[ pBattalion->m_CommandNum+1 ].m_iCommand = BL_JUMP;
	pBattalion->m_BattalionCommands[ pBattalion->m_CommandNum+1 ].m_iParam0 = pBattalion->m_CommandNum+2;
	pBattalion->m_BattalionCommands[ pBattalion->m_CommandNum+2 ].m_iCommand = BL_INTERUPT_RET;
	pBattalion->m_InteruptReg = pBattalion->m_curParam0;
	pBattalion->m_BattalionFlags |= BF_IN_INTERUPT;
}

void DoInteruptReturnCommand(  Battalion* pBattalion )
{
	pBattalion->m_BattalionCommands[ pBattalion->m_CommandNum - 2 ] = pBattalion->m_InteruptStack[0];
	pBattalion->m_BattalionCommands[ pBattalion->m_CommandNum - 1 ] = pBattalion->m_InteruptStack[1];
	pBattalion->m_BattalionCommands[ pBattalion->m_CommandNum ] = pBattalion->m_InteruptStack[2];
	pBattalion->m_CommandNum = pBattalion->m_CommandNum - 2;
	pBattalion->m_curParam0 = pBattalion->m_InteruptReg;
	pBattalion->m_BattalionFlags &= ~BF_IN_INTERUPT;
}

void SwitchCommand( Battalion* pBattalion, bool bIncr = true )
{
	if( bIncr )
	{
		pBattalion->m_CommandNum++;
		ntAssert( pBattalion->m_CommandNum < (MAX_BATTALION_COMMANDS+2) );
	}
//	ntPrintf( "Switching Battalion ID %i to command %i", pBattalion->m_BattalionID, pBattalion->m_CommandNum );

JumpBack:
	BattalionCommand& command = pBattalion->m_BattalionCommands[ pBattalion->m_CommandNum ];

	// default we also move param0 over
	pBattalion->m_curParam0 = command .m_iParam0;

	switch( command.m_iCommand )
	{
	case BL_JUMP:
		// we have switched to a jump so change the instruction pointer
		pBattalion->m_CommandNum = command.m_iParam0;
		goto JumpBack;
		break;
	case BL_NOOP:
		pBattalion->m_Orders = BO_TAUNT;
		break;
	case BL_HOLD_POSITION:
		pBattalion->m_timeDecr = command.m_iParam0;
		break;
	case BL_MARCH_TO:
		break;
	case BL_SET_SPEED_MULT:
		break;
	case BL_RESPAWN:
		break;
	case BL_SET_COHERSION:
		break;
	case BL_TAUNT:
		pBattalion->m_timeDecr = command.m_iParam0;
		break;
	case BL_WAIT_ON_FLAG:
		break;
	case BL_STAGING_POST:
		break;
	case BL_SET_PLAYER_TRACKING:
		break;
	case BL_SET_PLAYER_CIRCLE:
		break;
	case BL_INTERUPT_RET:
		break;
	case BL_EXPLODE:
		break;

	default:
		// do nothing 
		break;
	}
}

void DoHoldPositionCommand( BattlefieldHeader* pBattlefieldHeader, Battalion* pBattalion )
{
	// pass the order on to all the grunts... and see if we have timed out
	pBattalion->m_Orders = BO_HOLD_POSITION;

	float timer = pBattalion->m_timeDecr;
	timer -= pBattlefieldHeader->m_deltaTime;
	if( timer < 0 )
	{
		SwitchCommand( pBattalion );
	} else
	{
		pBattalion->m_timeDecr = (uint16_t) timer;
	}
}

void DoTauntCommand( BattlefieldHeader* pBattlefieldHeader, Battalion* pBattalion )
{
	// pass the order on to all the grunts... and see if we have timed out
	pBattalion->m_Orders = BO_TAUNT;

	float timer = pBattalion->m_timeDecr;
	timer -= pBattlefieldHeader->m_deltaTime;
	if( timer < 0 )
	{
		SwitchCommand( pBattalion );
	} else
	{
		pBattalion->m_timeDecr = (uint16_t) timer;
	}
}

void DoWaitOnFlagCommand( BattlefieldHeader* pBattlefieldHeader, Battalion* pBattalion )
{
	const BattalionCommand& command = pBattalion->m_BattalionCommands[ pBattalion->m_CommandNum ];
	// pass the order on to all the grunts... 
	pBattalion->m_Orders = BO_TAUNT;

	if( pBattlefieldHeader->m_iGlobalEventFlags & (0x1 << command.m_iParam0) )
	{
		SwitchCommand( pBattalion );
	}
}

void DoMarchToCommand( BattlefieldHeader* pBattlefieldHeader, Battalion* pBattalion )
{
	BattalionCommand& command = pBattalion->m_BattalionCommands[ pBattalion->m_CommandNum ];

	// get the center point of the battalion
	v128 minExtent = (vector float){ pBattalion->m_TopLeft.m_X , pBattalion->m_TopLeft.m_Y , pBattalion->m_TopLeft.m_X , pBattalion->m_TopLeft.m_Y };
	v128 maxExtent = (vector float){ pBattalion->m_BotRight.m_X , pBattalion->m_BotRight.m_Y , pBattalion->m_BotRight.m_X , pBattalion->m_BotRight.m_Y };
	v128 center = spu_add( minExtent, maxExtent );
	center = spu_mul( center, (vector float){ 0.5f, 0.5f, 0.5f, 0.5f } );

	if( command.m_iParam0 != 0 )
	{
		pBattalion->m_BattalionSpeedPercentageMultiplier =  ((float)command.m_iParam0 * (1.f/100.f));
	} else
	{
		pBattalion->m_BattalionSpeedPercentageMultiplier =  1.f;
	}
	float fTargetRadiusSqr = 750.f * 750.f;
	if( command.m_iParam1 != 0 )
	{
		fTargetRadiusSqr = (float) command.m_iParam1;
		fTargetRadiusSqr*= fTargetRadiusSqr;
	}

	// are we there yet?
	v128 targetpt = (vector float){ command.m_Pos.m_X, command.m_Pos.m_Y , command.m_Pos.m_X , command.m_Pos.m_Y };
	if( pBattalion->m_BattalionFlags & BF_RELATIVE_COMMANDS )
	{
		targetpt = spu_add( targetpt, (vector float) { pBattalion->m_RelativePnt.m_X, pBattalion->m_RelativePnt.m_Y, pBattalion->m_RelativePnt.m_X, pBattalion->m_RelativePnt.m_Y } );
	}

	v128 diff = spu_sub( targetpt, center );
	v128 dot = Dot2D( diff, diff );
	if( spu_extract(dot,0) < fTargetRadiusSqr )
	{
		// yep then move to next command
		SwitchCommand( pBattalion );
		return;
	}
//	ntPrintf( "center <%f,%f>\n",spu_extract( center, 0 ), spu_extract( center, 1 ) );

	// right we need to vector towards our target
	v128 dirToTravel = Normalise2DFast(diff);
	pBattalion->m_BattalionRotation = (uint16_t)(spu_extract(Vec2ToOrientation( dirToTravel ), 0) * 65535.f);
	pBattalion->m_Orders = BO_FORWARD_MARCH;

}

void DoMarchAndRotate( BattlefieldHeader* pBattlefieldHeader, Battalion* pBattalion, GruntArray* pGruntArray )
{
	// TODO 
}
void DoRespawn( BattlefieldHeader* pBattlefieldHeader, Battalion* pBattalion, GruntArray* pGruntArray )
{
	const BattalionCommand& command = pBattalion->m_BattalionCommands[ pBattalion->m_CommandNum ];
	pBattalion->m_POI.m_X = command.m_Pos.m_X;
	pBattalion->m_POI.m_Y = command.m_Pos.m_Y;
	pBattalion->m_Orders = BO_RESPAWN;
	pBattalion->m_BotRight.m_X = command.m_Pos.m_X;
	pBattalion->m_BotRight.m_Y = command.m_Pos.m_Y;
	pBattalion->m_TopLeft.m_X = command.m_Pos.m_X;
	pBattalion->m_TopLeft.m_Y = command.m_Pos.m_Y;

	// do we also want to jump
	if( command.m_iParam0 != 0xFFFF )
	{
		pBattalion->m_CommandNum = command.m_iParam0;
	} else
	{
		SwitchCommand( pBattalion );
	}
}

void DoStagingPostCommand( BattlefieldHeader* pBattlefieldHeader, Battalion* pBattalion )
{
	const BattalionCommand& command = pBattalion->m_BattalionCommands[ pBattalion->m_CommandNum ];

	// is it our turn to move to the stagin post?
	if( pBattlefieldHeader->m_iStagingPostLine >= command.m_iParam0 )
	{
		// move to the staging post
		// are we there yet?
		v128 targetpt = (vector float){ command.m_Pos.m_X, command.m_Pos.m_Y , command.m_Pos.m_X , command.m_Pos.m_Y };
		if( pBattalion->m_BattalionFlags & BF_RELATIVE_COMMANDS )
		{
			targetpt = spu_add( targetpt, (vector float) { pBattalion->m_RelativePnt.m_X, pBattalion->m_RelativePnt.m_Y, pBattalion->m_RelativePnt.m_X, pBattalion->m_RelativePnt.m_Y } );
		}
		// get the center point of the battalion
		v128 minExtent = (vector float){ pBattalion->m_TopLeft.m_X , pBattalion->m_TopLeft.m_Y , pBattalion->m_TopLeft.m_X , pBattalion->m_TopLeft.m_Y };
		v128 maxExtent = (vector float){ pBattalion->m_BotRight.m_X , pBattalion->m_BotRight.m_Y , pBattalion->m_BotRight.m_X , pBattalion->m_BotRight.m_Y };
		v128 center = spu_add( minExtent, maxExtent );
		center = spu_mul( center, (vector float){ 0.5f, 0.5f, 0.5f, 0.5f } );

		const static float fTargetRadiusSqr = 750.f * 750.f;
		v128 diff = spu_sub( targetpt, center );
		v128 dot = Dot2D( diff, diff );


		if( spu_extract(dot,0) < fTargetRadiusSqr )
		{
			// we are here now taunt until we are told to move...
			pBattalion->m_Orders = BO_TAUNT;
			pBattalion->m_BattalionFlags |= BF_AT_STAGING_POST;

			if( pBattlefieldHeader->m_iGlobalEventFlags & (0x1 << command.m_iParam1) )
			{
				// clear the AT_STAGING_POST flag
				pBattalion->m_BattalionFlags &= ~BF_AT_STAGING_POST;

				SwitchCommand( pBattalion );
			}

			return;
		}

		// right we need to vector towards our target
		v128 dirToTravel = Normalise2DFast(diff);
		pBattalion->m_BattalionRotation = (uint16_t)(spu_extract(Vec2ToOrientation( dirToTravel ), 0) * 65535.f);
		pBattalion->m_Orders = BO_FORWARD_MARCH;
	} else
	{
		// nop our line isn't up yet
		// just jeer while waiting
		pBattalion->m_Orders = BO_TAUNT;
	}
}

void DoExplodeCommand( Battalion* pBattalion )
{
	pBattalion->m_Orders = BO_EXPLODE;
}

void DoSetPlayerTracking( BattlefieldHeader* pBattlefieldHeader, Battalion* pBattalion )
{
	const BattalionCommand& command = pBattalion->m_BattalionCommands[ pBattalion->m_CommandNum ];

	if( command.m_iParam0 == 0 )
	{
		pBattalion->m_BattalionFlags &= ~BF_PLAYER_TRACKING;
	} else
	{
		pBattalion->m_BattalionFlags |= BF_PLAYER_TRACKING;
	}

	SwitchCommand( pBattalion );
}

void DoSetPlayerCircling( BattlefieldHeader *pBattlefieldHeader, Battalion *pBattalion )
{
	const BattalionCommand &command = pBattalion->m_BattalionCommands[ pBattalion->m_CommandNum ];

	if ( command.m_iParam0 == 0 )
	{
		pBattalion->m_BattalionFlags &= ~BF_PLAYER_CIRCLE;
	}
	else
	{
		pBattalion->m_BattalionFlags |= BF_PLAYER_CIRCLE;
	}

	SwitchCommand( pBattalion );
}

bool DoPlayerRelatedLogic( BattlefieldHeader* pBFHeader, Battalion* pBattalion, GruntArray* pGruntArray, const v128 center )
{
	// Work out if we're tracking or circling...
	bool should_track_player = ( ( pBattalion->m_BattalionFlags & BF_PLAYER_TRACKING ) != 0 );
	UNUSED( should_track_player );

	// find the distance from the player to the center of the battalion (this rely on the battalion being fairly
	// cohersive...
	v128 worldCenter = BF_PositionToWorldSpace( center, pBFHeader );

	v128 diff = spu_sub( pBFHeader->m_PlayerPos.QuadwordValue(), worldCenter );
	// kill height 
	diff = (v128) spu_and( (vector unsigned int)diff, (vector unsigned int){ 0xffffffff, 0, 0xffffffff, 0xffffffff } );
	v128 distsqr = Intrinsics::SPU_DP3( diff, diff );
	v128 dist = rsqrtf4fast( distsqr );

	// We add on 10m to the circle radius for circling the player as we only want to march
	// to within a certain distance of the desired radius, then we seek to each grunt's specific
	// position on the circle from within the BO_RUN_TO_CIRCLE behaviour.
	v128 player_circle_radius = spu_splats( should_track_player ? pBattalion->m_fMinInnerPlayerTrackRadius : pBattalion->m_MinCirclePlayerRadius+30.0f ) ;
	v128 t0 = recipf4fast( spu_mul( dist, player_circle_radius ) );

	if ( should_track_player )
	{
		v128 t1 = recipf4fast( spu_mul( dist, spu_splats(  pBattalion->m_fMaxOuterPlayerTrackRadius ) ) );

		// too far to care, so just run normal commands
		if( spu_extract(t1, 0) >= 1.f )
		{
			return false;
		}
	}

	if( spu_extract(t0, 0) > 1.f )
	{
		// Outside the circle... March towards the player.

		// make back hemisphere grunts further away than front hemisphere ones
		v128 v1 = spu_madd( pBFHeader->m_CameraFacing.QuadwordValue(), (vector float){0.f, 0.f, 0.f, 0.f}, worldCenter );
		v1 = spu_sub( v1, pBFHeader->m_PlayerPos.QuadwordValue() );
		v128 v2 = (v128) spu_and( (vector unsigned int)pBFHeader->m_CameraFacing.QuadwordValue(), (vector unsigned int){ 0xffffffff, 0, 0xffffffff, 0xffffffff } );
		v1 = (v128) spu_and( (vector unsigned int)v1, (vector unsigned int){ 0xffffffff, 0, 0xffffffff, 0xffffffff } );
		v1 = spu_mul( Intrinsics::SPU_RSqrt( Intrinsics::SPU_DP3(v1,v1) ), v1 );
		v2 = spu_mul( Intrinsics::SPU_RSqrt( Intrinsics::SPU_DP3(v2,v2) ), v2 );
		v128 viewdot = Intrinsics::SPU_DP3( v1, v2 );

		uint16_t randomish = pBattalion->m_BattalionID % 5;

		if( spu_extract(viewdot,0) > 0.f )
		{
			randomish += 7;
		}

		// right we need to vector towards our target
		v128 player = (vector float) { pBFHeader->m_PlayerBFPos.m_X, pBFHeader->m_PlayerBFPos.m_Y, 0, 0 };
		v128 target = spu_add( player, (vector float){ 0, -(randomish + pBFHeader->m_iPlayerTrackWallRandomishFactor) * pBFHeader->m_iPlayerTrackWallMultipler, 0, 0 } );

		// lets see if the target is inside the polygon world
		if( IntersectsStaticSegments( pBFHeader, target, target, pBFHeader->m_iPlayerTrackWallInset ) == false )
		{
			AddInteruptCommand( pBattalion );
			BattalionCommand& command = pBattalion->m_BattalionCommands[ pBattalion->m_CommandNum ];
			command.m_iCommand = BL_MARCH_TO;
			command.m_iParam0 = 0;
			command.m_iParam1 = 750;
			command.m_Pos.m_X = (uint16_t) spu_extract( target, 0 );
			command.m_Pos.m_Y = (uint16_t) spu_extract( target, 1 );
			// and a small wait to help performance and generally settle things doen
			BattalionCommand& command2 = pBattalion->m_BattalionCommands[ pBattalion->m_CommandNum+1 ];
			command2.m_iCommand = BL_TAUNT;
			command2.m_iParam0 = 15 + pBattalion->m_BattalionID; // taunt for a 0.5 second + a small bit of psuedo-randomity to stop beating

			SwitchCommand( pBattalion, false );
			return false;
		}
		// if not just taunt for a bit
	}
	else if ( !should_track_player )
	{
		// We're within 30m of the circle-player zone, so change the orders to circle-player mode.
		// This allows individual grunts to find an appropriate place around the circle.
		pBattalion->m_Orders = BO_RUN_TO_CIRCLE;
		return false;
	}

	AddInteruptCommand( pBattalion );
	BattalionCommand& command = pBattalion->m_BattalionCommands[ pBattalion->m_CommandNum ];
	command.m_iCommand = BL_TAUNT;
	command.m_iParam0 = 150 + pBattalion->m_BattalionID; // taunt for a 5 second + a small bit of psuedo-randomity to stop beating
	SwitchCommand( pBattalion, false );
	return false;
}

void ProcessCommand(  BattlefieldHeader* pBattlefieldHeader, Battalion* pBattalion, GruntArray* pGruntArray, const v128 center )
{
	// see if this battalion is doing player tracking logic and not already doing an interupt
	// NOTE: Both the player-tracking and player-circling logic uses the player-related-logic code.
	bool use_player_logic =	(pBattalion->m_BattalionFlags & BF_PLAYER_TRACKING) ||
							(pBattalion->m_BattalionFlags & BF_PLAYER_CIRCLE);

	if(  use_player_logic && !(pBattalion->m_BattalionFlags & BF_IN_INTERUPT) )
	{
		if( DoPlayerRelatedLogic( pBattlefieldHeader, pBattalion, pGruntArray, center ) )
		{
			return;
		}
	}

	BattalionCommand& command = pBattalion->m_BattalionCommands[ pBattalion->m_CommandNum ];

	switch( command.m_iCommand )
	{
	case BL_NOOP:
		pBattalion->m_Orders = BO_TAUNT;
		break;
	case BL_JUMP:
		// this case is to cover jumping (or starting) to a jump
		pBattalion->m_CommandNum = command.m_iParam0;
		break;
	case BL_HOLD_POSITION:
		DoHoldPositionCommand( pBattlefieldHeader, pBattalion );
		break;
	case BL_MARCH_TO:
		DoMarchToCommand( pBattlefieldHeader, pBattalion );
		break;
	case BL_SET_SPEED_MULT:
		pBattalion->m_BattalionSpeedPercentageMultiplier =  ((float)command.m_iParam0 * (1.f/100.f));
		SwitchCommand( pBattalion );
		break;
	case BL_MARCH_AND_ROTATE:
		DoMarchAndRotate( pBattlefieldHeader, pBattalion, pGruntArray );
		break;
	case BL_RESPAWN:
		DoRespawn( pBattlefieldHeader, pBattalion, pGruntArray );
		break;
	case BL_SET_COHERSION:
		pBattalion->m_BattalionCohersionFactor =  ((float)command.m_iParam0 * (1.f/100.f));
		SwitchCommand( pBattalion );
		break;
	case BL_TAUNT:
		DoTauntCommand( pBattlefieldHeader, pBattalion );
		break;
	case BL_WAIT_ON_FLAG:
		DoWaitOnFlagCommand( pBattlefieldHeader, pBattalion  );
		break;
	case BL_STAGING_POST:
		DoStagingPostCommand( pBattlefieldHeader, pBattalion  );
		break;
	case BL_SET_PLAYER_TRACKING:
		DoSetPlayerTracking( pBattlefieldHeader, pBattalion  );
		break;
	case BL_SET_PLAYER_CIRCLE:
		DoSetPlayerCircling( pBattlefieldHeader, pBattalion );
		break;
	case BL_INTERUPT_RET:
		DoInteruptReturnCommand( pBattalion );
		break;
	case BL_EXPLODE:
		DoExplodeCommand( pBattalion );
		break;
	default:
		break;
	}

}


//------------------------------------------------------
//!
//!
//------------------------------------------------------
void BattalionUpdate( SPUArgumentList &params )
{
	GetArrayInput( BattalionArray*, pBattalionArray,		SRTA_BATTALION_ARRAY );
	GetArrayInput( BattlefieldHeader*, pBattlefieldHeader,	SRTA_BATTLEFIELD_HEADER );
	GetArrayInput( GruntArray*, pGruntArray,				SRTA_GRUNTS );
	GetU32Input( iBattalionID,								SRTA_BATTALION_ID );

	Battalion* pBattalion = &pBattalionArray->m_BattalionArray[iBattalionID];
	// get some info back from out grunts
	GruntGameState* pGrunts = (GruntGameState*) (pGruntArray+1);
	uint16_t* pGruntIndex = (uint16_t*)(pGrunts + pGruntArray->m_iNumGrunts);

	// bigger than the battlefield 
	v128 minExtent, maxExtent;
	minExtent = spu_splats( 65536.f * 2 );
	maxExtent = spu_splats( -65536.f * 2 );
	pBattalion->m_BattalionSpeed = 255;

	for( uint16_t i=0;i < pBattalion->m_iNumUnits;i++)
	{
		const Unit* pUnit = &pBattalion->m_Units[i];
		uint16_t iGruntID = pUnit->m_FirstGruntID;

		// calculate the battalion speed (in future factor in morales?)
		pBattalion->m_BattalionSpeed = (pUnit->m_RunSpeed < pBattalion->m_BattalionSpeed) ? pUnit->m_RunSpeed : pBattalion->m_BattalionSpeed;

		for( uint16_t j=0;j < pUnit->m_NumGrunts;j++, iGruntID++)
		{
			// do the double index to get this grunt
			GruntGameState* pGrunt = &pGrunts[ pGruntIndex[ iGruntID ] ];

			// don't consider the dead or scared as part of the battalion
			if( pGrunt->m_GruntMajorState == GMS_NORMAL )
			{
				v128 vfPos = (vector float){ pGrunt->m_Position.m_X, pGrunt->m_Position.m_Y, 0, 0 };
				minExtent = Intrinsics::SPU_Min( minExtent, vfPos );
				maxExtent = Intrinsics::SPU_Max( maxExtent, vfPos );
			}
		}
	}


	pBattalion->m_BattalionSpeed = (uint8_t) (pBattalion->m_BattalionSpeed * pBattalion->m_BattalionSpeedPercentageMultiplier);

	pBattalion->m_TopLeft.m_X = (uint16_t) spu_extract( minExtent, 0 );
	pBattalion->m_TopLeft.m_Y = (uint16_t) spu_extract( minExtent, 1 );
	pBattalion->m_BotRight.m_X = (uint16_t) spu_extract( maxExtent, 0 );
	pBattalion->m_BotRight.m_Y = (uint16_t) spu_extract( maxExtent, 1 );

	v128 center = spu_add( minExtent, maxExtent );
	center = spu_mul( center, spu_splats( 0.5f ) );

	ProcessCommand( pBattlefieldHeader, pBattalion, pGruntArray, center );

/*

	// if the battalion is in disorder, each grunt for itself
	if( pBattalion->m_FormationOrder == BFO_DISORDER )
	{
		pBattalion->m_Orders = BO_NO_CONTROL;
	}
*/

	// we are going to DMA back our battalion we just updated
	uint32_t eaAddr =	g_DMAEffectiveAddresses[ SRTA_BATTALION_ARRAY ] + (((uint8_t*)pBattalion) - ((uint8_t*)pBattalionArray));

	ntDMA::Params batDmaParams;
	ntDMA_ID batId = ntDMA::GetFreshID();
	batDmaParams.Init32( pBattalion, eaAddr, sizeof(Battalion) , batId );
	ntDMA::DmaToPPU( batDmaParams );
 	ntDMA::StallForCompletion( batId );
	ntDMA::FreeID( batId );
}

