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
#include "ntlib_spu/syncprims_spu.h"

#include "army/army_ppu_spu.h"
#include "army/army_math_spu.h"
#include "army/battalion.h"
#include "army/battlefield.h"
#include "army/battlefield_chunk.h"
#include "army/grunt.h"

#include <stdlib.h> // for bsearch

//-----------------------------------------------------
//!
//! How far this grunt to this specified event
//! returns true if inside the radius of the event
//!
//-----------------------------------------------------
inline bool DistanceToEvent( const v128 gruntPos, const BattlefieldEvent* event, v128& diff, v128& distsqr )
{
	v128 eventPos = (vector float){ event->m_EventLocation.m_X, event->m_EventLocation.m_Y, event->m_EventLocation.m_X, event->m_EventLocation.m_Y };
	diff = spu_sub( eventPos, gruntPos );

	v128 radisqr = (vector float){ event->m_iRadius, event->m_iRadius, event->m_iRadius, event->m_iRadius };
	radisqr = spu_mul( radisqr, radisqr );
	distsqr = Dot2D( diff, diff );

	if( spu_extract( distsqr, 0 ) < spu_extract( radisqr, 0 ) )
	{
		return true;
	} else
	{
		return false;
	}
}

//-----------------------------------------------------
//!
//! simple little function to say if a particular anim is playing
//!
//-----------------------------------------------------
inline bool IsAnimPlaying( GruntGameState* pGrunt, GruntAnimState state )
{
	return( pGrunt->m_AnimPlayingBits & (1 << state) );
}

//-----------------------------------------------------
//!
//! mark that we want to play this animation unless we already
//! are playing 
//!
//-----------------------------------------------------
inline void SetAnimState( GruntGameState* pGrunt, GruntAnimState state )
{
	if( IsAnimPlaying(pGrunt, state) )
	{
		return;
	}
//	ntPrintf( "m_AnimPlayingBits 0x%x\n", pGrunt->m_AnimPlayingBits );

	pGrunt->m_AnimToPlayBits |= (1 << state);
	pGrunt->m_AnimPlayingBits |= (1 << state); // now claim its playing (its not actually but will be in 1 frames time)
}

//-----------------------------------------------------
//!
//! AvoidThing is the central logic of all avoidance code
//! if just force adjust the grunt from the 'thing' forward only
//! with some hard coded hakery in places to get it good enough
//!
//-----------------------------------------------------
inline v128 AvoidThing( v128 thisPos, v128 obPos, v128 r, v128 heading, v128 perp )
{
	static const v128 rScalar = (vector float){ 1.2f, 1.2f, 1.2f,1.2f };
	r = spu_mul( r, rScalar );

	static const v128 oneVec = (vector float){ 1.0f, 0.0f, 1.0f, 1.0f };
	v128 diff = spu_sub( thisPos, obPos );
	v128 magd = Magnitude2DFast( diff );
	v128 t = Intrinsics::SPU_Div( magd, r );

	t = Intrinsics::SPU_Min( t, oneVec );
	t = spu_sub( oneVec, t );

	v128 dirVec = Normalise2DFast( diff );
	v128 headDot = Dot2D( dirVec, heading );

	if( spu_extract( headDot, 0 ) < 0.0f )
	{
		thisPos = spu_add( thisPos, spu_mul( t, spu_mul( dirVec, r ) ) );
	}

	return thisPos;
}

//-----------------------------------------------------
//!
//! we need to know where we should be this function does that, just provide your currnet speed
//! we essential keep track of each grunts position in an ideal world and then we can vector
//! back to that as required. Its this that allows the units to essentially reform and continue
//! on there original path without any real path finding
//!
//-----------------------------------------------------
inline v128 UpdateIntendedPosition(  GruntGameState* pGrunt, const BattlefieldHeader* pBattlefieldHeader, const Battalion* pBattalion, const v128 vvel )
{
	// where we should be
	v128 ipos = (vector float){ pGrunt->m_IntendedPos.m_X, pGrunt->m_IntendedPos.m_Y, pGrunt->m_IntendedPos.m_X, pGrunt->m_IntendedPos.m_Y };
	float intendedRot = pBattalion->m_BattalionRotation * (1.f/65535.f);
	v128 intendedVRot = (vector float){ intendedRot, intendedRot, intendedRot, intendedRot };
	v128 irvec = Orientation2Vec( intendedVRot );
	v128 nipos = spu_madd( vvel, irvec, ipos );

	pGrunt->m_IntendedPos.m_X = (uint16_t)( spu_extract( nipos, 0 ) );
	pGrunt->m_IntendedPos.m_Y = (uint16_t)( spu_extract( nipos, 1 ) );

	return nipos;
}
//-----------------------------------------------------
//!
//! due to the 'interesting' way I handle updates, this is certainly far, far from perfect but meh should be good enough )
//! for each objectable avoid it
//!
//-----------------------------------------------------
v128 AvoidObstacle( GruntGameState* pThisGrunt, const BattlefieldHeader* pBattlefieldHeader, v128 thisPos, v128 heading )
{
	v128 perp = PerpCW2D( heading );

	for( int i=0;i < pBattlefieldHeader->m_iNumObstacles;i++)
	{
		v128 obPos = (vector float){	pBattlefieldHeader->m_Obstacles[i].m_Location.m_X, pBattlefieldHeader->m_Obstacles[i].m_Location.m_Y, 
										pBattlefieldHeader->m_Obstacles[i].m_Location.m_X, pBattlefieldHeader->m_Obstacles[i].m_Location.m_Y };
		v128 r = (vector float){	pBattlefieldHeader->m_Obstacles[i].m_Radius, pBattlefieldHeader->m_Obstacles[i].m_Radius, 
									pBattlefieldHeader->m_Obstacles[i].m_Radius, pBattlefieldHeader->m_Obstacles[i].m_Radius };

		thisPos = AvoidThing( thisPos, obPos, r, heading, perp );
	}
	return thisPos;
}


//-----------------------------------------------------
//!
//! due to the 'interesting' way I handle updates, this is certainly far, far from perfect but meh should be good enough )
//! for each other grunt in the same chunk upto a speed limit avoid it
//!
//-----------------------------------------------------
v128 AvoidOtherGrunts( GruntGameState* pThisGrunt, 
						const BattlefieldHeader* pBattlefieldHeader, 
						const BattalionArray* pBattalionArray, 							
						const GruntGameState* pFirstGrunt,
						const GruntGameState* pLastGrunt, v128 thisPos, 
						v128 heading )
{
	const Battalion* pThisBattalion = &pBattalionArray->m_BattalionArray[ pThisGrunt->m_BattalionID ];
	const Unit* pThisUnit = &pThisBattalion->m_Units[ pThisGrunt->m_UnitID ] ;
	v128 thisRadii = (vector float){ pThisUnit->m_PersonalSpace, pThisUnit->m_PersonalSpace, pThisUnit->m_PersonalSpace, pThisUnit->m_PersonalSpace };
	thisRadii = spu_mul( thisRadii, (vector float){ 0.5f, 0.5f, 0.5f, 0.5f } );
	const v128 perp = PerpCW2D( heading );

	// to stop catastrophic speed failures we only ever to a fixed amount of avoidance.. too many is pointless and
	// kill framerate
	static const int MAX_OTHER_GRUNT_CHECKS = 30;

	int iNumGruntChecks = 0;

	// for each grunt in this chunk (this can be dual pipelined with some cleverness TODO
	for( 	const GruntGameState* pThatGrunt = pFirstGrunt;
			(pThatGrunt < pLastGrunt);
			pThatGrunt++ )
	{
		// early out
		if( iNumGruntChecks > MAX_OTHER_GRUNT_CHECKS )
		{
			return thisPos;
		}

		// don't self collide
		if( pThatGrunt != pThisGrunt )
		{
			const v128 thatPos = (vector float){ pThatGrunt->m_Position.m_X, pThatGrunt->m_Position.m_Y, 
													pThatGrunt->m_Position.m_X, pThatGrunt->m_Position.m_Y };
			const Battalion* pThatBattalion = &pBattalionArray->m_BattalionArray[ pThatGrunt->m_BattalionID ];
			const Unit* pThatUnit = &pThatBattalion->m_Units[ pThatGrunt->m_UnitID ] ;
			v128 thatRadii = (vector float){ pThatUnit->m_PersonalSpace, pThatUnit->m_PersonalSpace, pThatUnit->m_PersonalSpace, pThatUnit->m_PersonalSpace };
			thatRadii = spu_mul( thatRadii, (vector float){ 0.5f, 0.5f, 0.5f, 0.5f } );

			v128 radii = spu_add( thisRadii, thatRadii );
			static const v128 rScalar = (vector float){ 1.0f, 1.0f, 1.0f, 1.0f };
			radii = spu_mul( radii, rScalar );

			thisPos = AvoidThing( thisPos, thatPos, radii, heading, perp );
			++iNumGruntChecks;
		}
	}

	return thisPos;
}
//-----------------------------------------------------
//!
//! due to the 'interesting' way I handle updates, this is certainly far, far from perfect but meh should be good enough )
//! avoid the player
//!
//-----------------------------------------------------
inline v128 AvoidPlayer( GruntGameState* pThisGrunt, const BattlefieldHeader* pBattlefieldHeader, v128 thisPos, v128 heading )
{
	v128 perp = PerpCW2D( heading );

	v128 obPos = (vector float){	pBattlefieldHeader->m_PlayerBFPos.m_X, pBattlefieldHeader->m_PlayerBFPos.m_Y, 
									pBattlefieldHeader->m_PlayerBFPos.m_X, pBattlefieldHeader->m_PlayerBFPos.m_Y };
	v128 r = (vector float){	pBattlefieldHeader->m_PlayerBFRadius, pBattlefieldHeader->m_PlayerBFRadius, pBattlefieldHeader->m_PlayerBFRadius, pBattlefieldHeader->m_PlayerBFRadius };
	static const v128 playerAvoidScalar = (vector float){ 0.2f, 0.2f, 0.2f, 0.2f };
	r = spu_mul( r, playerAvoidScalar );

	return AvoidThing( thisPos, obPos, r, heading, perp );
}

//-----------------------------------------------------
//!
//! The armies can be kept inside polygon boundraries now
//! you can have 4 convex polygons and if inside them you will not be able to leave
//! NOTE currently only the first polygons seems to work
//!
//-----------------------------------------------------
inline bool IntersectsStaticSegments ( const BattlefieldHeader* restrict pBattlefieldHeader, const v128 p0, const v128 p1 )
{
	const v128 simdP = spu_shuffle( p0, p1, VECTORMATH_MAKE_SHUFFLE( VECTORMATH_SHUF_X, VECTORMATH_SHUF_Y, VECTORMATH_SHUF_A, VECTORMATH_SHUF_B) );

	vector unsigned int inside = spu_splats( (unsigned int) 0 );
	vector unsigned int outside = spu_splats( (unsigned int) 0xFFFFFFFF );

	uint16_t iStartIndex = 0;

	for( uint8_t j=0;j < pBattlefieldHeader->m_iNumStaticPolygons;++j )
	{
		vector unsigned int flags = spu_splats( (unsigned int) 0);
		// lets just check the battalion, before we project it forward
		for( uint8_t i=0;i < pBattlefieldHeader->m_iNumStaticSegments[j];++i )
		{
			// unpack the incoming segment into vector registers
			const ArmyStaticAISegment* restrict obSeg1 = &pBattlefieldHeader->m_StaticSegments[iStartIndex+i];

			// todo optimise using vector instructions (2 maybe 3 instructions..)
			v128 seg1P0 = (v128){obSeg1->P0.m_X, obSeg1->P0.m_Y, obSeg1->P0.m_X, obSeg1->P0.m_Y};
			v128 seg1Norm = (v128){	-(obSeg1->P1.m_Y-obSeg1->P0.m_Y), (obSeg1->P1.m_X-obSeg1->P0.m_X), 
									-(obSeg1->P1.m_Y-obSeg1->P0.m_Y), (obSeg1->P1.m_X-obSeg1->P0.m_X) };

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

// kill our grunt
void DieMofo( GruntGameState* pGrunt, const BattlefieldEvent* event )
{				
	// can't die if we are already dead
	if( pGrunt->m_GruntMajorState != GMS_DYING )
	{
		// die mofo
		pGrunt->m_GruntMajorState = GMS_DYING;
		SetAnimState( pGrunt, GAM_KO_BLUNT );
		pGrunt->m_POI.m_X = event->m_EventLocation.m_X;
		pGrunt->m_POI.m_Y = event->m_EventLocation.m_Y;
		pGrunt->m_EventRadius = event->m_iRadius;
	}
}

void DoDyingGruntState(	GruntGameState* pGrunt, 
							const BattlefieldHeader* pBattlefieldHeader, 
							const Battalion* pBattalion, 
							const Unit* pUnit)
{
	if( !IsAnimPlaying( pGrunt, GAM_KO_BLUNT ) )
	{
		BattlefieldInfo	info;

		// update the battlefield stats note the increment only happens once even tho
		// it may actually happen more than once, the line is re-read back from main memory
		// each time the atomic update fails... trust me this is m_iCurDead = m_iCurDead +1 !!
		DisableInterrupts();
		do
		{
			AtomicLockCacheLine( (uint32_t)pBattlefieldHeader->m_eaInfo, &info );
			info.m_iCurDead++;
			info.m_iTotalDeadCount++;
			if( pGrunt->m_SelfExplode )
			{
				info.m_iExplodeCount++;
			}
		}
		while( AtomicStoreConditionalCacheLine( (uint32_t)pBattlefieldHeader->m_eaInfo, &info ) );
		EnableInterrupts();

		SetAnimState( pGrunt, GAM_DEAD );
		pGrunt->m_GruntMajorState = GMS_DEAD;
	}

	v128 pos = (vector float){ pGrunt->m_Position.m_X, pGrunt->m_Position.m_Y, pGrunt->m_Position.m_X, pGrunt->m_Position.m_Y };
	v128 vpoi = (vector float){ pGrunt->m_POI.m_X,pGrunt->m_POI.m_Y, pGrunt->m_POI.m_X, pGrunt->m_POI.m_Y };
	v128 dir = spu_sub( pos, vpoi );
	dir = Normalise2DFast( dir );

	float speed = pUnit->m_DiveSpeed * pBattlefieldHeader->m_deltaTime;
	v128 vspeed = (vector float){ speed, speed, speed, speed };


	{
		v128 vec_dist = spu_sub( pos, vpoi );
		vec_dist = Intrinsics::SPU_Sqrt( Intrinsics::SPU_DP3( vec_dist, vec_dist ) );

		// Remap distance from explosion from (0,+inf) to (1,0) with new '0' being at distance m_EventRadius.
		v128 scalar = spu_splats( (float) pGrunt->m_EventRadius );
		scalar = spu_mul( scalar, spu_splats( 10.f ) );
		v128 multiplier = Intrinsics::SPU_Div( vec_dist, scalar );

		multiplier = spu_sub( spu_splats( 1.f ), multiplier );
		multiplier = Intrinsics::SPU_Max( multiplier, spu_splats( 0.0f ) );

		vspeed = spu_mul( vspeed, spu_mul(multiplier, spu_splats(1.f)) );
	}





	// integrate velocity to position
	pos = spu_madd( vspeed, dir, pos );

	// move back into the grunt structure
	pGrunt->m_Position.m_X = (uint16_t)( spu_extract( pos, 0 ) );
	pGrunt->m_Position.m_Y = (uint16_t)( spu_extract( pos, 1 ) );
	pGrunt->m_Orientation.m_Rotation = (uint16_t)(spu_extract(Vec2ToOrientation( dir ), 0) * 65535.f);
	UpdateIntendedPosition( pGrunt, pBattlefieldHeader, pBattalion, vspeed );
}


void DoNormalGruntState( GruntGameState* pGrunt, 
						const BattlefieldHeader* pBattlefieldHeader, 
						const Battalion* pBattalion, 
						const BattalionArray* pBattalionArray, 
						const GruntGameState* pFirstGrunt,
						const GruntGameState* pLastGrunt,
						const Unit* pUnit )
{
	float speed = 0.f;
	float fakespeed = 0.f;

	switch( pBattalion->m_Orders )
	{
	case BO_NO_CONTROL:
		// each grunt for itself... 
		// TODO most will run, some attack each other, some keep attacking enemy
		// for now all will run away at there max speed (ignore battalion speed)
		fakespeed = speed = pUnit->m_RunSpeed * pBattlefieldHeader->m_deltaTime;
		SetAnimState( pGrunt, GAM_RUN );
		break;
	case BO_FORWARD_MARCH:
		fakespeed = speed = pBattalion->m_BattalionSpeed * pBattlefieldHeader->m_deltaTime;
		SetAnimState( pGrunt, GAM_RUN );
		break;
	case BO_HOLD_POSITION:
		SetAnimState( pGrunt, GAM_IDLE );
		break;
	case BO_RETREAT:
		fakespeed = speed = pBattalion->m_BattalionSpeed * pBattlefieldHeader->m_deltaTime;
		SetAnimState( pGrunt, GAM_RUN );
		break;
	case BO_RUN_FOR_YOUR_LIVES:
		// still under some control but run away at max speed
		fakespeed = speed = pUnit->m_RunSpeed * pBattlefieldHeader->m_deltaTime;
		SetAnimState( pGrunt, GAM_RUN );
		break;
	case BO_ATTACK:
		fakespeed = speed = pBattalion->m_BattalionSpeed * 1.5f * pBattlefieldHeader->m_deltaTime;
		SetAnimState( pGrunt, GAM_RUN );
		break;
	case BO_CHARGE:
		// attack at max, speed better have a good commander who doesn't
		// put slow troops up front...
		fakespeed = speed = pUnit->m_RunSpeed * pBattlefieldHeader->m_deltaTime;
		SetAnimState( pGrunt, GAM_RUN );
		break;
	case BO_TAUNT:
//		fakespeed = pUnit->m_WalkSpeed* pBattlefieldHeader->m_deltaTime;
		SetAnimState( pGrunt, GAM_TAUNT );
		break;
	case BO_EXPLODE:
		{
			// fake up a battlefield event 
			BattlefieldEvent event;
			event.m_EventLocation = pGrunt->m_Position;
			event.m_iRadius = 1000; // fakery this will confuse and annoy wil some point in the future!!
			pGrunt->m_SelfExplode = 1;
			DieMofo( pGrunt, &event );
			return;
		}
	case BO_RUN_TO_CIRCLE:
		fakespeed = speed = pBattalion->m_BattalionSpeed * pBattlefieldHeader->m_deltaTime;
		SetAnimState( pGrunt, GAM_RUN );
		break;
	}

	// speed and position
	v128 pos = (vector float){ pGrunt->m_Position.m_X, pGrunt->m_Position.m_Y, pGrunt->m_Position.m_X, pGrunt->m_Position.m_Y };
	v128 vvel = (vector float){ fakespeed, fakespeed, fakespeed, fakespeed };

	
	
	
	v128 player_pos;
	v128 circle_radius;
	if ( pBattalion->m_Orders == BO_RUN_TO_CIRCLE )
	{

		// TODO: Make them run to their place in the circle...

		player_pos		= (vector float){	pBattlefieldHeader->m_PlayerBFPos.m_X, pBattlefieldHeader->m_PlayerBFPos.m_Y,
											pBattlefieldHeader->m_PlayerBFPos.m_X, pBattlefieldHeader->m_PlayerBFPos.m_Y };
		circle_radius	= (vector float){	pBattalion->m_MinCirclePlayerRadius, pBattalion->m_MinCirclePlayerRadius,
											pBattalion->m_MinCirclePlayerRadius, pBattalion->m_MinCirclePlayerRadius };

		// Find the angle around the circle that is "unique" for this grunt.
		v128 grunt_uid	= (vector float){ pGrunt->m_GruntID, pGrunt->m_GruntID, pGrunt->m_GruntID, pGrunt->m_GruntID };
		v128 num_grunts	= (vector float){ pBattalion->m_iNumGrunts, pBattalion->m_iNumGrunts, pBattalion->m_iNumGrunts, pBattalion->m_iNumGrunts };

		v128 circle_pos_angle = Intrinsics::SPU_Div( grunt_uid, num_grunts );		// This is now on [0,1).
		circle_pos_angle = spu_mul( circle_pos_angle, VECTORMATH_TWO_PI );

		// Find the sine and cosine of the circle-pos angle.
		v128 sin_angle, cos_angle;
		Intrinsics::SPU_SinCos( circle_pos_angle, &sin_angle, &cos_angle );

		// Work out a unique position on the circle - first assume the circle is at the origin...
		v128 circle_offset = spu_mul( circle_radius, spu_shuffle( sin_angle, cos_angle, VECTORMATH_SHUF_XAYB ) );

		// Now offset to the player's position. We now have our desired target on the circle.
		v128 grunt_target_pos = spu_add( circle_offset, player_pos );

		// Work out the direction to the target position.
		v128 to_target = spu_sub( grunt_target_pos, pos );

		// normalise this direction and then find the orientation.
		to_target = Normalise2DFast( to_target );
		v128 desired_orientation = Vec2ToOrientation( to_target );

		// Set the new orientation for the grunt...
		pGrunt->m_Orientation.m_Rotation = uint16_t( spu_extract( desired_orientation, 0 ) * 65536.0f );

		CPoint temp_a( BF_PositionToWorldSpace( spu_add( player_pos, (vector float){ 0.0f, -1.0f, 0.0f, -1.0f } ), pBattlefieldHeader ) );
		CPoint temp_b( BF_PositionToWorldSpace( spu_add( player_pos, (vector float){ 0.0f, 1.0f, 0.0f, 1.0f } ), pBattlefieldHeader ) );
		temp_a += CDirection( 0.0f, 50.0f, 0.0f );
		temp_b += CDirection( 0.0f, 50.0f, 0.0f );
		BF_DrawDebugLine( pBattlefieldHeader, temp_a, temp_b, 0xffffffff );
		temp_a = CPoint( BF_PositionToWorldSpace( spu_add( player_pos, (vector float){ -1.0f, 0.0f, -1.0f, 0.0f } ), pBattlefieldHeader ) );
		temp_b = CPoint( BF_PositionToWorldSpace( spu_add( player_pos, (vector float){ 1.0f, 0.0f, 1.0f, 0.0f } ), pBattlefieldHeader ) );
		temp_a += CDirection( 0.0f, 50.0f, 0.0f );
		temp_b += CDirection( 0.0f, 50.0f, 0.0f );
		BF_DrawDebugLine( pBattlefieldHeader, temp_a, temp_b, 0xffffffff );

		temp_a = CPoint( BF_PositionToWorldSpace( spu_add( grunt_target_pos, (vector float){ 0.0f, -1.0f, 0.0f, -1.0f } ), pBattlefieldHeader ) );
		temp_b = CPoint( BF_PositionToWorldSpace( spu_add( grunt_target_pos, (vector float){ 0.0f, 1.0f, 0.0f, 1.0f } ), pBattlefieldHeader ) );
		temp_a += CDirection( 0.0f, 50.0f, 0.0f );
		temp_b += CDirection( 0.0f, 50.0f, 0.0f );
		BF_DrawDebugLine( pBattlefieldHeader, temp_a, temp_b, 0xffffffff );
		temp_a = CPoint( BF_PositionToWorldSpace( spu_add( grunt_target_pos, (vector float){ -1.0f, 0.0f, -1.0f, 0.0f } ), pBattlefieldHeader ) );
		temp_b = CPoint( BF_PositionToWorldSpace( spu_add( grunt_target_pos, (vector float){ 1.0f, 0.0f, 1.0f, 0.0f } ), pBattlefieldHeader ) );
		temp_a += CDirection( 0.0f, 50.0f, 0.0f );
		temp_b += CDirection( 0.0f, 50.0f, 0.0f );
		BF_DrawDebugLine( pBattlefieldHeader, temp_a, temp_b, 0xffffffff );

		

		temp_a = CPoint( BF_PositionToWorldSpace( player_pos, pBattlefieldHeader ) );
		temp_b = CPoint( BF_PositionToWorldSpace( grunt_target_pos, pBattlefieldHeader ) );
		temp_a += CDirection( 0.0f, 50.0f, 0.0f );
		temp_b += CDirection( 0.0f, 50.0f, 0.0f );
		BF_DrawDebugLine( pBattlefieldHeader, temp_a, temp_b, 0xffffffff );
	}






	v128 nipos = UpdateIntendedPosition( pGrunt, pBattlefieldHeader, pBattalion, vvel );

	// current orientation as a vector
	float curRot = pGrunt->m_Orientation.m_Rotation * (1.f/65535.f);
	v128 curVRot = (vector float){	curRot, curRot, curRot, curRot };
	v128 curVec = Orientation2Vec( curVRot );

	// the direction we should be moving to get us where we are going
	static const v128 oneVec = (vector float){ 1.0f, 1.0f, 1.0f, 1.0f };
	static const v128 angularScalar = (vector float){ 0.55f, 0.55f, 0.55f, 0.55f };
	v128 headVec = spu_sub( nipos, pos );
	headVec = Normalise2D( headVec );
	curVec = spu_add( spu_mul( headVec, angularScalar ), spu_mul( curVec, spu_sub( oneVec, angularScalar) ) );

	vvel = (vector float){ speed, speed, speed, speed };
	// where we would get to on our current heading
	v128 pred_pos = spu_madd( vvel, curVec, pos );

	// where would we be, if we hit something if we keep on this course
	pred_pos = AvoidOtherGrunts( pGrunt, pBattlefieldHeader, pBattalionArray, pFirstGrunt, pLastGrunt, pred_pos, curVec );
	pred_pos = AvoidObstacle( pGrunt, pBattlefieldHeader, pred_pos, curVec );

	if ( pBattalion->m_Orders == BO_RUN_TO_CIRCLE )
	{
		// Avoid the circle around the player...
		v128 perp = PerpCW2D( curVec );
		pred_pos = AvoidThing( pred_pos, player_pos, circle_radius, curVec, perp );
	}

	pred_pos = AvoidPlayer( pGrunt, pBattlefieldHeader, pred_pos, curVec );

	// if we are gonna hit the hard wall, stop (TODO slide)
	if( IntersectsStaticSegments( pBattlefieldHeader, pos, pred_pos ) )
	{
		pred_pos = pos;
	}

	// a new cur vector
	v128 npos;
	v128 newVec = spu_sub( pred_pos, pos );
	if( spu_extract( Magnitude2DFast( newVec ), 0 ) > 0.01f )
	{
		curVec = newVec;
		// now reform a vector to here and then re-multiple by our speed
		curVec = Normalise2D( curVec );
		npos = spu_madd( vvel, curVec, pos );
	} else
	{
		// we ended up not moving so err don't
		npos = pos;

	}
	if( pBattalion->m_Orders == BO_HOLD_POSITION || pBattalion->m_Orders == BO_TAUNT )
	{
		v128 player = (vector float) { pBattlefieldHeader->m_PlayerBFPos.m_X, pBattlefieldHeader->m_PlayerBFPos.m_Y, 0, 0 };
		curVec = spu_sub( player, npos );
		curVec = Normalise2D( curVec );
	}

	// move back into the grunt structure
	pGrunt->m_Position.m_X = (uint16_t)( spu_extract( npos, 0 ) );
	pGrunt->m_Position.m_Y = (uint16_t)( spu_extract( npos, 1 ) );
	pGrunt->m_Orientation.m_Rotation = (uint16_t)(spu_extract( Vec2ToOrientation( curVec ) , 0) * 65535.f);
}

void DoScaredGruntState(	GruntGameState* pGrunt, 
							const BattlefieldHeader* pBattlefieldHeader, 
							const Battalion* pBattalion, 
							const Unit* pUnit, 
							const GruntGameState* pFirstGrunt,
							const GruntGameState* pLastGrunt,
							const BattlefieldEvent* event,
							uint32_t eventId,
							const v128 eventDiff, 
							const v128 eventDistSqr )
{
	// lets see if we are in the inner thread radius, if so we will try and jump out of way else just run
	v128 innerRadissqr = (vector float){ event->m_iParam1, event->m_iParam1, event->m_iParam1, event->m_iParam1 };
	innerRadissqr = spu_mul( innerRadissqr, innerRadissqr );
	// its fairly far away so just run like the wind away from it
	float speed = pUnit->m_RunSpeed * pBattlefieldHeader->m_deltaTime;

	v128 pos = (vector float){ pGrunt->m_Position.m_X,pGrunt-> m_Position.m_Y, pGrunt->m_Position.m_X, pGrunt->m_Position.m_Y };
	v128 vpoi = (vector float){ pGrunt->m_POI.m_X,pGrunt->m_POI.m_Y, pGrunt->m_POI.m_X, pGrunt->m_POI.m_Y };
	v128 dir = spu_sub( pos, vpoi );
	dir = Normalise2DFast( dir );

	// lets stop being scared... we all brave boys
	if( eventId == 0xFFFFFFFF )
	{
		pGrunt->m_GruntMajorState = GMS_NORMAL;
		return;
	}
	if( spu_extract( eventDistSqr,0 ) < spu_extract( innerRadissqr, 0 ) )
	{
		// this isn't the fastest thing in the world but given its only to limit the number of divers...
		// for now it will do.. We dot the camera facing with a vector from the camera to this grunt and
		// if not in a cone don't dive
		v128 worldPos = BF_PositionToWorldSpace( pos, pBattlefieldHeader );
		v128 worldPOI = BF_PositionToWorldSpace( vpoi, pBattlefieldHeader );

		v128 v1 = spu_madd( pBattlefieldHeader->m_CameraFacing.QuadwordValue(), (vector float){0.f, 0.f, 0.f, 0.f}, worldPos );
		v1 = spu_sub( v1, worldPOI );
		v128 v2 = pBattlefieldHeader->m_CameraFacing.QuadwordValue();
		v1 = spu_mul( Intrinsics::SPU_RSqrt( Intrinsics::SPU_DP3(v1,v1) ), v1 );

		v128 viewdot = Intrinsics::SPU_DP3( v1, v2 );


		// 0.2 is just a guess - the number corresponds to the cosine of the maximum
		// angle you wish grunts to respond at, 0.2 = 78.5 degrees.
		if ( spu_extract( viewdot, 0 ) > 0.99f )
		{
			if ( event != 0 )
			{
				pGrunt->m_POI.m_X = event->m_EventLocation.m_X;
				pGrunt->m_POI.m_Y = event->m_EventLocation.m_Y;
			}

			pGrunt->m_GruntMajorState = GMS_DIVING;

			float curRot = pGrunt->m_Orientation.m_Rotation * (1.f/65535.f);

			v128 worldPos = BF_PositionToWorldSpace( pos, pBattlefieldHeader );
			v128 worldPOI = BF_PositionToWorldSpace( vpoi, pBattlefieldHeader );
			v128 dir = spu_sub( worldPos, worldPOI );
			dir = spu_mul( Intrinsics::SPU_RSqrt( Intrinsics::SPU_DP3(dir,dir) ), dir );

			v128 cam_right = spu_shuffle( pBattlefieldHeader->m_CameraFacing.QuadwordValue(), pBattlefieldHeader->m_CameraFacing.QuadwordValue(), VECTORMATH_SHUF_ZYXW );
			cam_right = spu_mul( cam_right, (vector float){ 1.0f, 1.0f, -1.0f, 1.0f } );

			v128 cam_right_dot_dir = Intrinsics::SPU_DP3( cam_right, dir );

			if ( spu_extract( cam_right_dot_dir, 0 ) > 0 )
			{
				if ( curRot < 0.75 && curRot > 0.25 )
				{
					SetAnimState( pGrunt, GAM_DIVE_LEFT );
				}
				else
				{
					SetAnimState( pGrunt, GAM_DIVE_RIGHT );
				}
			}
			else
			{
				if ( curRot < 0.75 && curRot > 0.25 )
				{
					SetAnimState( pGrunt, GAM_DIVE_RIGHT );
				}
				else
				{
					SetAnimState( pGrunt, GAM_DIVE_LEFT );
				}
			}

			return;
		}
	} 

	SetAnimState( pGrunt, GAM_RUN );



	v128 vvel = (vector float){ speed, speed, speed, speed };
	UpdateIntendedPosition( pGrunt, pBattlefieldHeader, pBattalion, vvel );

	// where we would get to on our current heading
	v128 pred_pos = spu_madd( vvel, dir, pos );

	// where would we be, if we hit something if we keep on this course
	pred_pos = AvoidObstacle( pGrunt, pBattlefieldHeader, pred_pos, dir );
	pred_pos = AvoidPlayer( pGrunt, pBattlefieldHeader, pred_pos, dir );

	// if we are gonna hit the hard wall, stop (TODO slide)
	if( IntersectsStaticSegments( pBattlefieldHeader, pos, pred_pos ) )
	{
		pred_pos = pos;
	}

	// a new cur vector
	v128 npos;
	v128 newVec = spu_sub( pred_pos, pos );
	if( spu_extract( Magnitude2DFast( newVec ), 0 ) > 0.01f )
	{
		// now reform a vector to here and then re-multiple by our speed
		dir = Normalise2D( newVec );
		npos = spu_madd( vvel, dir, pos );
	} else
	{
		// we ended up not moving so err don't
		npos = pos;

	}

	// move back into the grunt structure
	pGrunt->m_Position.m_X = (uint16_t)( spu_extract( npos, 0 ) );
	pGrunt->m_Position.m_Y = (uint16_t)( spu_extract( npos, 1 ) );
	pGrunt->m_Orientation.m_Rotation = (uint16_t)(spu_extract(Vec2ToOrientation( dir ), 0) * 65535.f);

}

void DoDivingGruntState(	GruntGameState* pGrunt, 
							const BattlefieldHeader* pBattlefieldHeader, 
							const Battalion* pBattalion, 
							const Unit* pUnit )
{

	// when the dive is over go back to normal state (tho possible die...)
	if( (!IsAnimPlaying( pGrunt, GAM_DIVE_LEFT)) && (!IsAnimPlaying( pGrunt, GAM_DIVE_RIGHT)) )
	{
		// we want to get up afterwards... but getting up is built into the dive anim so happens automatically...
		pGrunt->m_GruntMajorState = GMS_NORMAL;
	}

	v128 pos = (vector float){ pGrunt->m_Position.m_X, pGrunt->m_Position.m_Y, pGrunt->m_Position.m_X, pGrunt->m_Position.m_Y };

	v128 dir;
	float curRot = pGrunt->m_Orientation.m_Rotation * (1.f/65535.f);
	v128 curVRot = (vector float){ curRot, curRot, curRot, curRot };
	v128 grunt_facing = Orientation2Vec( curVRot );
	v128 grunt_right = PerpCW2D( grunt_facing );
	if ( IsAnimPlaying( pGrunt, GAM_DIVE_LEFT ) )
	{
		dir = grunt_right;
	}
	else
	{
		dir = Intrinsics::SPU_Negate( grunt_right );
	}

	float speed = pUnit->m_DiveSpeed * pBattlefieldHeader->m_deltaTime;

	v128 vspeed = (vector float){ speed, speed, speed, speed };
	// integrate velocity to position
	pos = spu_madd( vspeed, dir, pos );

	// move back into the grunt structure
	pGrunt->m_Position.m_X = (uint16_t)( spu_extract( pos, 0 ) );
	pGrunt->m_Position.m_Y = (uint16_t)( spu_extract( pos, 1 ) );

	// the dive anims already have the orientation baked in, so force forward z
	dir = (vector float){ 0, -1, 0, 0};
	pGrunt->m_Orientation.m_Rotation = (uint16_t)(spu_extract(Vec2ToOrientation( dir ), 0) * 65535.f);
	UpdateIntendedPosition( pGrunt, pBattlefieldHeader, pBattalion, vspeed );
}

void RespawnGrunt(	GruntGameState* pGrunt, 
					const BattlefieldHeader* pBattlefieldHeader,
					const Battalion* pBattalion )
{
	v128 targetpt = (vector float){ pBattalion->m_POI.m_X, pBattalion->m_POI.m_Y , pBattalion->m_POI.m_X , pBattalion->m_POI.m_Y };
	v128 offset = (vector float){ pGrunt->m_BattalionOffsetX, pGrunt->m_BattalionOffsetY , pGrunt->m_BattalionOffsetX , pGrunt->m_BattalionOffsetY };
	v128 pt = spu_add( targetpt, offset );

	pGrunt->m_Position.m_X = (uint16_t) spu_extract( pt, 0 );
	pGrunt->m_Position.m_Y = (uint16_t) spu_extract( pt, 1 );
	pGrunt->m_IntendedPos.m_X = (uint16_t) spu_extract( pt, 0 );
	pGrunt->m_IntendedPos.m_Y = (uint16_t) spu_extract( pt, 1 );

	if( pGrunt->m_GruntMajorState != GMS_REAL_DUDE )
	{
		if( pGrunt->m_GruntMajorState == GMS_DEAD )
		{
			AtomicDecrement( ((uint32_t)pBattlefieldHeader->m_eaInfo) + 1 );
		}
		pGrunt->m_GruntMajorState = GMS_NORMAL;
	}
	pGrunt->m_Orientation.m_Rotation = 0;
}


void DoGruntLogic(	GruntGameState* pGrunt, 
					const BattlefieldHeader* pBattlefieldHeader, 
					const BattalionArray* pBattalionArray, 
					const GruntGameState* pFirstGrunt,
					const GruntGameState* pLastGrunt )
{
	const Battalion* pBattalion = &pBattalionArray->m_BattalionArray[pGrunt->m_BattalionID];

	// respawn is a special case as it should effect everybody including dead guys
	if( pBattalion->m_Orders == BO_RESPAWN )
	{
//		ntPrintf( "RESPAWN\n" );
		RespawnGrunt( pGrunt, pBattlefieldHeader, pBattalion );
		return;
	}


	v128 pos = (vector float){ pGrunt->m_Position.m_X, pGrunt->m_Position.m_Y, pGrunt->m_Position.m_X, pGrunt->m_Position.m_Y };

	// these will be computed if we find an even like death of scared
	uint32_t eventId = 0xFFFFFFFF;

	// do absoluletly nothing
	if( pGrunt->m_GruntMajorState == GMS_REAL_DUDE )
		return;

	// dead men don't wear plaid...
	if( pGrunt->m_GruntMajorState == GMS_DEAD )
	{
		return;
	}


	v128 eventDir = spu_splats(0.f);
	v128 eventDistSqr = spu_splats(0.f);
	// scan for events break out on first one if its 'fatal' storing eventId
	for( uint32_t i=0;i < pBattlefieldHeader->m_iNumEvents;++i)
	{
		const BattlefieldEvent* event = &pBattlefieldHeader->m_Events[i];
		if( DistanceToEvent( pos, event, eventDir, eventDistSqr ) == true )
		{
			switch( event->m_iType )
			{
			case BET_BAZOOKA_SHOT:
				if( eventId == 0xFFFFFFFF && (pGrunt->m_GruntMajorState == GMS_NORMAL || pGrunt->m_GruntMajorState == GMS_SCARED) )
				{
					// always run from the first time we saw it
					if( pGrunt->m_GruntMajorState != GMS_SCARED )
					{
						pGrunt->m_POI.m_X = event->m_EventLocation.m_X;
						pGrunt->m_POI.m_Y = event->m_EventLocation.m_Y;

						// big threat!!
						pGrunt->m_GruntMajorState = GMS_SCARED;
					}

					eventId = i;
				}
				if( spu_extract( eventDistSqr,0 ) < 10000.f )
				{
					AtomicDecrement( (uint32_t)pBattlefieldHeader->m_eaEventUpdates[ i ], 1 );
				}
				break;
			case BET_SYNC_ATTACK_WAKE:
				if( eventId == 0xFFFFFFFF && pGrunt->m_GruntMajorState == GMS_NORMAL )
				{
//					ntPrintf( "BET_SYNC_ATTACK_WAKE\n" );
					// all type get KO-ed by a SYNC attack wake
					pGrunt->m_GruntMajorState = GMS_KOED;
					pGrunt->m_POI.m_X = event->m_EventLocation.m_X;
					pGrunt->m_POI.m_Y = event->m_EventLocation.m_Y;
					eventId = i;
				}
				break;
			case BET_RANGE_ATTACK_WAKE:
			case BET_SPEED_ATTACK_WAKE:
				if( eventId == 0xFFFFFFFF && pGrunt->m_GruntMajorState == GMS_NORMAL )
				{
//					ntPrintf( "BET_RANGE_ATTACK_WAKE\n" );
					// fodders get ko'ed everybody else blocks
					const Battalion* pBattalion = &pBattalionArray->m_BattalionArray[ pGrunt->m_BattalionID ];
					const Unit* pUnit = &pBattalion->m_Units[ pGrunt->m_UnitID ] ;

					switch( pUnit->m_UnitType )
					{
					case BUT_FODDER:
						SetAnimState( pGrunt, GAM_KO_WAKE );
						pGrunt->m_GruntMajorState = GMS_KOED;
						break;
					default:
					case BUT_SHIELDSMAN:
					case BUT_AXEMAN:
						SetAnimState( pGrunt, GAM_BLOCK );
						pGrunt->m_GruntMajorState = GMS_BLOCK;
						break;
					}
					pGrunt->m_POI.m_X = event->m_EventLocation.m_X;
					pGrunt->m_POI.m_Y = event->m_EventLocation.m_Y;
					eventId = i;
				}
				break;
			case BET_POWER_ATTACK_WAKE:
				if( eventId == 0xFFFFFFFF && pGrunt->m_GruntMajorState == GMS_NORMAL )
				{
//					ntPrintf( "BET_POWER_ATTACK_WAKE\n" );
					// fodders get ko'ed everybody else blocks
					const Battalion* pBattalion = &pBattalionArray->m_BattalionArray[ pGrunt->m_BattalionID ];
					const Unit* pUnit = &pBattalion->m_Units[ pGrunt->m_UnitID ] ;

					switch( pUnit->m_UnitType )
					{
					case BUT_FODDER:
						SetAnimState( pGrunt, GAM_KO_WAKE );
						pGrunt->m_GruntMajorState = GMS_KOED;
						break;
					default:
					case BUT_SHIELDSMAN:
					case BUT_AXEMAN:
						SetAnimState( pGrunt, GAM_RECOIL );
						pGrunt->m_GruntMajorState = GMS_RECOIL;
						break;
					}
					pGrunt->m_POI.m_X = event->m_EventLocation.m_X;
					pGrunt->m_POI.m_Y = event->m_EventLocation.m_Y;
					eventId = i;
				}
				break;

			case BET_EXPLOSION:
				DieMofo( pGrunt, event );
				eventId = i;
				break;
			default:
				break;
			};

		}
	}

	const Unit* pUnit = &pBattalion->m_Units[ pGrunt->m_UnitID ] ;

	switch( pGrunt->m_GruntMajorState )
	{
	case GMS_NORMAL:
		DoNormalGruntState( pGrunt, pBattlefieldHeader, pBattalion, pBattalionArray, pFirstGrunt, pLastGrunt, pUnit );
		break;
	case GMS_DYING:
		DoDyingGruntState( pGrunt, 
							pBattlefieldHeader, 
							pBattalion, 
							pUnit );		
		break;
	case GMS_SCARED:
		{
			const BattlefieldEvent* event = 0;
			if( eventId != 0xFFFFFFFF )
			{
				event = &pBattlefieldHeader->m_Events[eventId];
				// only care bout scary events
				if( event->m_iType != BET_BAZOOKA_SHOT )
				{
					event = 0;
					eventId = 0xFFFFFFFF;
				}
			}

			DoScaredGruntState( pGrunt, 
								pBattlefieldHeader, 
								pBattalion, 
								pUnit, 
								pFirstGrunt, 
								pLastGrunt, 
								event, 
								eventId,
								eventDir, 
								eventDistSqr );
		}
		break;
	case GMS_DIVING:
		// diving peeps 
		DoDivingGruntState( pGrunt, 
							pBattlefieldHeader, 
							pBattalion, 
							pUnit );		
		break;
	case GMS_GETTING_UP:
		// when we have finished getting up return to normal
		if( !IsAnimPlaying( pGrunt, GAM_GETTING_UP ) )
		{
			pGrunt->m_GruntMajorState = GMS_NORMAL;
		}
		break;
	case GMS_KOED:
		// when we have finished being koed get back up
		if( !IsAnimPlaying( pGrunt, GAM_KO_WAKE ) )
		{
			// we want to get up afterwards
			SetAnimState( pGrunt, GAM_GETTING_UP );
			pGrunt->m_GruntMajorState = GMS_GETTING_UP;
		}		
		break;
	case GMS_BLOCK:
		// when we have finished blocking
		if( !IsAnimPlaying( pGrunt, GAM_BLOCK ) )
		{
			pGrunt->m_GruntMajorState = GMS_NORMAL;
		}
		break;
	case GMS_RECOIL:
		// when we have finished blocking
		if( !IsAnimPlaying( pGrunt, GAM_RECOIL ) )
		{
			pGrunt->m_GruntMajorState = GMS_NORMAL;
		}
		break;
	case GMS_DEAD:
	default:
		break;
	}
}



//------------------------------------------------------
//!
//!
//------------------------------------------------------
void GruntExecute( SPUArgumentList &params )
{
	GetArrayInput( BattalionArray*, pBattalionArray,		SRTA_BATTALION_ARRAY );
	GetArrayInput( BattlefieldHeader*, pBattlefieldHeader,	SRTA_BATTLEFIELD_HEADER );
	GetArrayInput( GruntArray*, pGruntArray,				SRTA_GRUNTS );
	GetArrayInput( BattlefieldChunk*, pBattlefieldChunk,	SRTA_BATTLEFIELD_CHUNK );
	GetU32Input( iFirstGruntIndex,							SRTA_FIRST_GRUNT_INDEX );
	ntAssert( ((uint16_t)iFirstGruntIndex) != 0xFFFF );

	uint16_t iNumGrunts = 0;
	GruntGameState* pGrunt0 = (GruntGameState*) (pGruntArray+1);
	GruntGameState* pFirstGrunt = pGrunt0 + iFirstGruntIndex; 
	GruntGameState* pRealLastGrunt = pGrunt0 + pGruntArray->m_iNumGrunts;

	// scan to get the end or our grunt array
	GruntGameState* pLastGrunt = pFirstGrunt;
	while( (pLastGrunt->m_ChunkIndex == pBattlefieldChunk->m_ChunkId) && (pLastGrunt < pRealLastGrunt) )
	{
		pLastGrunt++;
		iNumGrunts++;
	}

	// skip all interesting process, if time is zero or very close to it
	if( pBattlefieldHeader->m_deltaTime > 1e-5f )
	{
		// for each grunt in this
		for( 	GruntGameState* pGrunt = pFirstGrunt;
				pGrunt < pLastGrunt;
				pGrunt++ )
		{

			pGrunt->m_CurHeight = BFC_GetLocalHeight( pBattlefieldChunk, pBattlefieldHeader, &pGrunt->m_Position );
			DoGruntLogic( pGrunt, pBattlefieldHeader, pBattalionArray, pFirstGrunt, pLastGrunt );
		}
	}


	// dma updated grunt back to main mem
	if( iNumGrunts > 0 )
	{
		uint32_t eaAddr =	g_DMAEffectiveAddresses[ SRTA_GRUNTS ] + 
							sizeof(GruntArray) + 
							(pFirstGrunt - pGrunt0)*sizeof(GruntGameState);

//		ntPrintf( "First Grunt %lu, eaAddr - 0x%x, iNumGrunts %d, DMA transfer size %lu\n", (pFirstGrunt - pGrunt0), eaAddr, iNumGrunts, iNumGrunts*sizeof(GruntGameState) );
		// we are going to DMA back just our chunk worth of grunts! this should really be double buffered
		// do be completely deterministic for surrounding checks but fuck it, whats good about determinism
		ntDMA::Params gruntsDmaParams;
		ntDMA_ID gruntsId = ntDMA::GetFreshID();
		gruntsDmaParams.Init32( pFirstGrunt, eaAddr, iNumGrunts * sizeof(GruntGameState) , gruntsId );
		ntDMA::DmaToPPU( gruntsDmaParams );
 		ntDMA::StallForCompletion( gruntsId );
		ntDMA::FreeID( gruntsId );
	}

}
