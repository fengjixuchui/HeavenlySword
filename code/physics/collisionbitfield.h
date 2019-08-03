//---------------------------------------------------------------------------------------------------------
//!
//!	\file physics/collisionbitfield.h
//!	
//!	DYNAMICS COMPONENT:
//!		This is the definition of the collision bitfield we want to use in our game
//!
//!	Author: Mustapha Bismi (mustapha.bismi@ninjatheory.com)
//! Created: 2005.02.21
//!
//---------------------------------------------------------------------------------------------------------

#ifndef _DYNAMICS_COLLISIONFIELD_INC
#define _DYNAMICS_COLLISIONFIELD_INC

#include "config.h"

//#ifndef _PS3_RUN_WITHOUT_HAVOK_BUILD
#include <hkbase/types/hkBaseTypes.h>
//#endif

// Main dynamics component namespace
namespace Physics {

//--------------------------------------------------------------------------------------------------------
//!
//!	PhysicsProperty.
//!		Rigid Bodies and phantoms should get some properties attached to them.
//!
//---------------------------------------------------------------------------------------------------------

typedef enum {

	PROPERTY_ENTITY_PTR					= 1,	//!< This is the first property attached to a body. This enable us to get the entity ptr, if their is any. 
	PROPERTY_RAGDOLL_TRANSFORM_PTR,
	PROPERTY_RAGDOLL_OFFSET_SIZE_INT,
	PROPERTY_FILTER_EXCEPTION_INT,				//!< This property contain the per entity filter exception controller
	PROPERTY_FIRST_CONTACT_INT,					//!< This property is used to define if at the first body contact this body should be turned to dynamics
	PROPERTY_SPAWN_PARTICLES_INT				//!< Add this property on a rigid body to spawn some predefined particles

} PhysicsProperty;

//--------------------------------------------------------------------------------------------------------
//!
//!	RagdollMaterial.
//!		Let's define some ragdoll material to use specifically.
//!		At the moment, there is 11 of them, so I will use the last 4 bits of the extra_flags
//!
//---------------------------------------------------------------------------------------------------------

typedef enum {

	RAGDOLL_HEAD_MATERIAL		= 0,
	RAGDOLL_SPINE_00_MATERIAL,
	RAGDOLL_PELVIS_MATERIAL,
	RAGDOLL_L_ARM_MATERIAL,
	RAGDOLL_R_ARM_MATERIAL,
	RAGDOLL_L_ELBOW_MATERIAL,
	RAGDOLL_R_ELBOW_MATERIAL,
	RAGDOLL_L_LEG_MATERIAL,
	RAGDOLL_R_LEG_MATERIAL,
	RAGDOLL_L_KNEE_MATERIAL,
	RAGDOLL_R_KNEE_MATERIAL,
	RAGDOLL_NO_COLLIDE

} RagdollMaterial;

//--------------------------------------------------------------------------------------------------------
//!
//!	EntityCollisionFlag.
//!		Define the bitfield layout for a rigid or a phantom rigid body.
//!
//---------------------------------------------------------------------------------------------------------
typedef union EntityCollisionFlag {

	struct {
		unsigned i_am : 8;
		unsigned i_collide_with : 8;
		unsigned extra_flags : 16;
	} flags;

	hkUint32 base;

	EntityCollisionFlag()
	{
		base = 0;
	};

} EntityCollisionFlag;

//--------------------------------------------------------------------------------------------------------
//!
//!	ChatacterControllerCollisionFlag.
//!		Define the bitfield layout for a character controler bodies
//!
//---------------------------------------------------------------------------------------------------------
typedef union ChatacterControllerCollisionFlag {

	struct {
		unsigned i_am : 8;
		unsigned i_collide_with : 8;	
		unsigned extra_flags : 13;
		unsigned i_am_important : 1;
		unsigned i_am_in_KO_state : 2; 
	} flags;

	hkUint32 base;

	ChatacterControllerCollisionFlag()
	{
		base = 0;
	};

} ChatacterControllerCollisionFlag;

//--------------------------------------------------------------------------------------------------------
//!
//!	RagdollCollisionFlag.
//!		Define the specific bitfield layout for the ragdoll rigid bodies.
//!
//---------------------------------------------------------------------------------------------------------
typedef union RagdollCollisionFlag {

	struct {
		unsigned i_am : 8;
		unsigned i_collide_with : 8;			
		unsigned extra_flags : 8;
		unsigned character_dead : 1;
		unsigned ragdoll_material : 4;
		unsigned i_am_important : 1;
		unsigned i_am_in_KO_state : 2;
	} flags;

	hkUint32 base;

	RagdollCollisionFlag()
	{
		base = 0;
	};

} RagdollCollisionFlag;

//--------------------------------------------------------------------------------------------------------
//!
//!	AIWallCollisionFlag.
//!		Define the specific bitfield layout for the AIwalls rigid bodies.
//!
//---------------------------------------------------------------------------------------------------------
typedef union AIWallCollisionFlag {

	struct {
		unsigned i_am : 8;
		unsigned i_collide_with : 8;
		unsigned extra_flags : 4;
		unsigned one_sided : 1; // wall collide only from one side of triangles
		unsigned not_collide_with_dead_ragdolls : 1;
		unsigned kill_passing_KO : 1; // if 1 then if character in KO state is passing the wall it will kill him
		unsigned not_collide_with_KO_states_unimportant : 2; 
		unsigned not_collide_with_KO_states_important : 2;
		unsigned raycast_material : 5;
	} flags;

	hkUint32 base;

	AIWallCollisionFlag()
	{
		base = 0;
	};

} AIWallCollisionFlag;

//--------------------------------------------------------------------------------------------------------
//!
//!	RaycastCollisionFlag.
//!		Define the bitfield layout for a raycast.
//!
//---------------------------------------------------------------------------------------------------------
union RaycastCollisionFlag
{
	struct
	{
		unsigned i_am : 5;
		unsigned i_collide_with : 8;
		unsigned extra_flags : 19;
	} flags;

	hkUint32 base;

	RaycastCollisionFlag()
	:	base( 0 )
	{}
};

//--------------------------------------------------------------------------------------------------------
//!
//!	FilterExceptionFlag.
//!		Define the set of exceptions on the rules. 0 means that all the rules applies
//!
//---------------------------------------------------------------------------------------------------------
typedef union FilterExceptionFlag {

	struct {
		unsigned exception_set : 14;
		unsigned extra_flags : 18;
	} flags;

	hkUint32 base;

	FilterExceptionFlag()
	{
		base = 0;
	};

} FilterExceptionFlag;

//--------------------------------------------------------------------------------------------------------
//!
//!	ENTITY_COLLISION_FLAGS.
//!		This define the set of flags for an entity (rigid body || phantoms) I_AM or I_COLLIDE_WITH field.
//!
//---------------------------------------------------------------------------------------------------------
enum ENTITY_COLLISION_FLAGS
{
	TRIGGER_VOLUME_BIT					= ( 1 << 0 ),		//  I am a trigger volume
	CHARACTER_CONTROLLER_PLAYER_BIT		= ( 1 << 1 ),		//  I am the player character controller
	CHARACTER_CONTROLLER_ENEMY_BIT		= ( 1 << 2 ),		//  I am a npc character controller
	SMALL_INTERACTABLE_BIT				= ( 1 << 3 ),		//  I am a small interactable
	LARGE_INTERACTABLE_BIT				= ( 1 << 4 ),		//  I am a large interactable	
	RAGDOLL_BIT							= ( 1 << 5 ),		//  I am a ragdoll part
	AI_WALL_BIT							= ( 1 << 6 ),		//  I am an AI wall
	RIGID_PROJECTILE_BIT				= ( 1 << 7 ),		//  I am a rigid projectile
};

//--------------------------------------------------------------------------------------------------------
//!
//!	RAYCAST_COLLISION_FLAGS.
//!		This define the set of flags for a raycast I_AM field.
//!
//---------------------------------------------------------------------------------------------------------
enum RAYCAST_COLLISION_FLAGS
{
	PROJECTILE_RAYCAST_BIT			= ( 1 << 0 ),		//  I am a projectile raycast
	LINE_SIGHT_BIT					= ( 1 << 1 ),		//  I am a line of sight raycast
	COLLISION_ENVIRONMENT_BIT		= ( 1 << 2 ),		//  I am a collision environment raycast
	STATIC_ENVIRONMENT_BIT			= ( 1 << 3 ),		//  I am a static only collision environment raycast: use only if you need only the static geom (ground/walls)
    COOLCAM_AFTERTOUCH_BIT          = ( 1 << 4 ),		//  I am a coolcam aftertouch raycast
};

//--------------------------------------------------------------------------------------------------------
//!
//!	KO_STATE_FLAGS.
//!     This define the set of flags for defining KO state of characters
//!
//---------------------------------------------------------------------------------------------------------
enum KO_States
{
	SMALL_KO_BIT		= ( 1 << 0 ),
	BIG_KO_BIT			= ( 1 << 1 ),
};


//--------------------------------------------------------------------------------------------------------
//!
//!	FILTER_EXCEPTION_FLAGS.
//!		Define the list of exception you can apply per entity for the collision filter.
//!
//---------------------------------------------------------------------------------------------------------
enum FILTER_EXCEPTION_FLAGS
{

	ALWAYS_RETURN_TRUE_BIT			= ( 1 << 0 ),		//  Do not filter the body, always return true. 
	ALWAYS_RETURN_FALSE_BIT			= ( 1 << 1 ),		//  Do not filter the body, always return false. 
	IGNORE_ENTITY_PTR_BIT			= ( 1 << 2 ),		//  Do apply filter even if both bodies are from the same entity.
	IGNORE_ENTITY_INTERACT_BIT		= ( 1 << 3 ),		//  Do apply filter even if both entities interact together.
	IGNORE_ENTITY_FIGHT_BIT			= ( 1 << 4 ),		//  Do apply filter even if both entities fight together 
	CC_AND_RAGDOLL_ALWAYS_BIT		= ( 1 << 5 ),		//  Ragdoll always collide with character controller
	CC_AND_RAGDOLL_NEVER_BIT		= ( 1 << 6 ),		//  Ragdoll never collide with character controller 
	IGNORE_FIXED_GEOM				= ( 1 << 7 ),		//	Force to ignore the static geometry
	ONLY_FIXED_GEOM					= ( 1 << 8 ),		//	Collide only with fixed geometry
	IGNORE_CCs						= ( 1 << 9 ),		//	Ignore CCs
	COLLIDE_WITH_PLAYER_ONLY		= ( 1 << 10 ),		//	Collide only with the player
	COLLIDE_WITH_CCs_ONLY			= ( 1 << 11 ),		//	Collide only with CCs (Player + entities)
	COLLIDE_WITH_NMEs_ONLY			= ( 1 << 12 ),		//	Collide only with enemies

};

} // Physics namespace

#endif // _DYNAMICS_COLLISIONFIELD_INC
