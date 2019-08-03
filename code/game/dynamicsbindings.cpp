/***************************************************************************************************
*
*	FILE			dynamicsbindings.cpp
*
*	DESCRIPTION		
*
***************************************************************************************************/

#include "Physics/config.h"
#include "Physics/havokincludes.h"

#include "game/dynamicsbindings.h"

#include "Physics/world.h"
#include "Physics/collisionbitfield.h"
#include "Physics/triggervolume.h"
#include "Physics/rigidbody.h"
#include "core/exportstruct_anim.h"

#ifdef PLATFORM_PC // FIXME_WIL no forcefield on PS3
#include "hair/forcefield.h"
#endif

#include "anim/hierarchy.h"
#include "game/entitymanager.h"
#include "game/entityinfo.h"
#include "game/luaglobal.h"
#include "game/luaexptypes.h"
#include "game/renderablecomponent.h"
#include "camera/camutils.h"
#include "game/attacks.h"
#include "interactioncomponent.h"
#include "awareness.h" // For targeting stuff
#include "game/query.h" // For ricochet stuff
#include "anim/animator.h"
#include "game/messagehandler.h" // Explosion stuff
#include "game/luaattrtable.h"
#include "objectdatabase/dataobject.h"
#include "game/syncdcombat.h"
#include "core/visualdebugger.h"
#include "game/randmanager.h"
#include "game/movement.h"
#include "camera/camman.h"
#include "camera/camview.h"
#include "core/exportstruct_clump.h"

#ifndef _PS3_RUN_WITHOUT_HAVOK_BUILD
#include <hkcollide/castutil/hkLinearCastInput.h>
#include <hkcollide/collector/pointcollector/hkAllCdPointCollector.h>
#include <hkcollide/shape/box/hkBoxShape.h>
#include <hkdynamics/world/hkWorld.h>
#include <hkdynamics/Motion/hkMotion.h>
#include <hkdynamics/entity/hkRigidBodyCinfo.h>
#include <hkcollide/shape/sphere/hkSphereShape.h>
#include <hkcollide/collector/bodypaircollector/hkAllCdBodyPairCollector.h>
#include <hkdynamics/entity/hkRigidBody.h>
#include <hkmath/basetypes/hkMotionState.h>
#include <hkcollide/agent/hkCollidable.h>
#include <hkdynamics/phantom/hkSimpleShapePhantom.h>
#endif

//#include "simpletransition.h"
#include "interactiontransitions.h"

#include "Physics/system.h"
#include "Physics/animatedlg.h"
#include "Physics/staticlg.h"
#include "Physics/singlerigidlg.h"
#include "Physics/compoundlg.h"
#include "Physics/spearlg.h"
#include "Physics/rigidbodybehavior.h"
#include "Physics/projectilelg.h"
#include "Physics/maths_tools.h"
#include "Physics/advancedcharactercontroller.h"
#include "Physics/softlg.h"
#include "physics/verletdef.h"

#ifndef _PS3_RUN_WITHOUT_HAVOK_BUILD
#include "Physics/advancedcharactercontroller.h"
#include "Physics/advancedragdoll.h"
#include <process.h>

#endif

#include "aftertouchcontroller.h"

//#define AUTOMATIC_NEW_SYSTEM_CONVERSION


//-------------------------------------------------------------------------------------------------
// BINDFUNC: Dynamics_ConstructRigidFromClump( luaobject Properties )
// DESCRIPTION: Add InAir state for an entity so it can be thrown.
//-------------------------------------------------------------------------------------------------
static int Dynamics_ConstructRigidFromClump ( NinjaLua::LuaState* pobState )
{
	NinjaLua::LuaStack args( pobState );
	NinjaLua::LuaObject obProperties( args[1] );

	// if we are just a normal lua table this will provide a LuaAttributeTable interface to it
	LuaAttributeTable temp( obProperties ); 
	// get a lua attribute table, if we were passed a normal lua table handle that as well

	LuaAttributeTable* pAttrTable;
	if(args[1].IsString())
	{
		DataObject* pDO = ObjectDatabase::Get().GetDataObjectFromName( args[1].GetString() );
		temp.SetDataObject( pDO );
		pAttrTable = &temp;
	}
	else if (args[1].IsTable())
	{
		pAttrTable = &temp;
	}
	else
	{
		pAttrTable = (LuaAttributeTable*)lua_touserdata(*pobState , 1); // args[1].Get<LuaAttributeTable*>();
		if( pAttrTable == 0 )
		{
			pAttrTable = &temp;
		}
	}
#ifndef _PS3_RUN_WITHOUT_HAVOK_BUILD
	CEntity* pobEnt = CLuaGlobal::Get().GetTarg();
#endif

	if (obProperties.IsNil()) // No properties have been defined so create rigid state from defaults
	{		
	
#ifndef _PS3_RUN_WITHOUT_HAVOK_BUILD

#ifdef AUTOMATIC_NEW_SYSTEM_CONVERSION

		//static char commandLine[ MAX_PATH * 2 ];
		static char acFileName[ MAX_PATH * 2 ];
		static char psFileName[ MAX_PATH ];
		memset( acFileName, 0 , sizeof( acFileName ) );

			CHashedString clumpString = pobEnt->GetClumpString();
			strcpy( psFileName, *clumpString);
			int stringLength = strlen(psFileName);
			strcpy( psFileName + stringLength - 5, "ps.xml\0" );
			ntPrintf("Converting %s to %s...\n",*clumpString, psFileName);

			ntstd::string temp = g_ShellOptions->m_obContentPlatform;
			temp += "\\";
			temp += *clumpString;

			ntstd::string temp2 = g_ShellOptions->m_contentNeutral;
			temp2 += "\\";
			temp2 += psFileName;

			temp += " ";
			temp += temp2;

			//Util::GetFiosFilePath( temp.c_str(), acFileName );

			//ntPrintf("Command line %s...\n", acFileName);
			//int ret = spawnl( P_WAIT, "C:\\alienbrainWork\\HS\\content_pc\\ClumpToPSXML.exe", "ClumpToPSXML.exe", acFileName );

			temp2 = "C:\\alienbrainWork\\HS\\content_pc\\ClumpToPSXML.exe ";
			temp2 += temp;

			Util::GetFiosFilePath( temp2.c_str(), acFileName );

			int ret = system( acFileName ) ;

			if( -1 == ret )
			{
				ntPrintf("Failed to call ClumpToPSXML.exe!...\n");
			}
#endif
		// 2 - Create a group
		//Physics::SingleRigidLG* lg = Physics::ClumpTools::ConstructRigidLGFromClump( pobEnt, 0 );
		Physics::SingleRigidLG* lg = NT_NEW Physics::SingleRigidLG( pobEnt->GetName(),  pobEnt);
		lg->Load();

		// 3 - Create a system
		if( pobEnt->GetPhysicsSystem() == 0 )
		{
			Physics::System* system = NT_NEW_CHUNK(Mem::MC_ENTITY) Physics::System( pobEnt, pobEnt->GetName() );
			pobEnt->SetPhysicsSystem( system );
		}
		// 4 - Add the group
		pobEnt->GetPhysicsSystem()->AddGroup( lg );
		lg->Activate();
#endif
	}
	else // Create rigid body from user defined parameters
	{
		CVector obVector(pAttrTable->GetVector("CenterOfMass"));
#ifndef _PS3_RUN_WITHOUT_HAVOK_BUILD
		hkRigidBodyCinfo obInfo;

		obInfo.m_motionType = hkMotion::MOTION_DYNAMIC;
		obInfo.m_qualityType = HK_COLLIDABLE_QUALITY_MOVING;
		
		obInfo.m_mass = pAttrTable->GetNumber("Mass");
		obInfo.m_centerOfMass.set(obVector.X(),obVector.Y(),obVector.Z());
		obInfo.m_restitution = pAttrTable->GetNumber("Restitution");
		obInfo.m_friction = pAttrTable->GetNumber("Friction");
		obInfo.m_linearDamping = pAttrTable->GetNumber("LinearDamping");
		obInfo.m_angularDamping = pAttrTable->GetNumber("AngularDamping");
		//HACK to save art reexport
		if (obInfo.m_angularDamping < 0.05f)
			obInfo.m_angularDamping = 0.05f;
		obInfo.m_angularDamping *= 6;
		obInfo.m_maxLinearVelocity = pAttrTable->GetNumber("MaxLinearVelocity");
		obInfo.m_maxAngularVelocity = pAttrTable->GetNumber("MaxAngularVelocity");

		NinjaLua::LuaObject LuaMotionType = pAttrTable->GetAttribute("MotionType");
		if (LuaMotionType.IsString())
		{
			if (strcmp(	LuaMotionType.GetString(), "FIXED" )==0)
			{
				obInfo.m_motionType = hkMotion::MOTION_FIXED;
				obInfo.m_qualityType = HK_COLLIDABLE_QUALITY_FIXED;
			}
			else if (strcmp(	LuaMotionType.GetString(), "KEYFRAMED" )==0)
			{
				obInfo.m_motionType = hkMotion::MOTION_KEYFRAMED;
				obInfo.m_qualityType = HK_COLLIDABLE_QUALITY_KEYFRAMED;
			}
			else if (strcmp(	LuaMotionType.GetString(), "DYNAMIC" )==0)
			{
				obInfo.m_motionType = hkMotion::MOTION_DYNAMIC;
				obInfo.m_qualityType = HK_COLLIDABLE_QUALITY_MOVING;
			}
		}

#ifdef AUTOMATIC_NEW_SYSTEM_CONVERSION

		//static char commandLine[ MAX_PATH * 2 ];
		static char acFileName[ MAX_PATH * 2 ];
		static char psFileName[ MAX_PATH ];
		memset( acFileName, 0 , sizeof( acFileName ) );

			CHashedString clumpString = pobEnt->GetClumpString();
			strcpy( psFileName, *clumpString);
			int stringLength = strlen(psFileName);
			strcpy( psFileName + stringLength - 5, "ps.xml\0" );
			ntPrintf("Converting %s to %s...\n",*clumpString, psFileName);

			ntstd::string temp = *g_ShellOptions->m_obContentPlatform;
			temp += "\\";
			temp += *clumpString;

			ntstd::string temp2 = g_ShellOptions->m_contentNeutral;
			temp2 += "\\";
			temp2 += psFileName;

			temp += " ";
			temp += temp2;

			//Util::GetFiosFilePath( temp.c_str(), acFileName );

			//ntPrintf("Command line %s...\n", acFileName);
			//int ret = spawnl( P_WAIT, "C:\\alienbrainWork\\HS\\content_pc\\ClumpToPSXML.exe", "ClumpToPSXML.exe", acFileName );

			temp2 = "C:\\alienbrainWork\\HS\\content_pc\\ClumpToPSXML.exe ";
			temp2 += temp;

			Util::GetFiosFilePath( temp2.c_str(), acFileName );

			int ret = system( acFileName ) ;

			if( -1 == ret )
			{
				ntPrintf("Failed to call ClumpToPSXML.exe!...\n");
			}
#endif

		// 2 - Create a group
		//Physics::SingleRigidLG* lg = Physics::ClumpTools::ConstructRigidLGFromClump( pobEnt,  &obInfo );
		Physics::SingleRigidLG* lg = NT_NEW Physics::SingleRigidLG( pobEnt->GetName(),  pobEnt);
		lg->Load();

		// 3 - Create a system
		if( pobEnt->GetPhysicsSystem() == 0 )
		{
			Physics::System* system = NT_NEW_CHUNK(Mem::MC_ENTITY) Physics::System( pobEnt, pobEnt->GetName() );
			pobEnt->SetPhysicsSystem( system );
		}
		// 4 - Add the group
		pobEnt->GetPhysicsSystem()->AddGroup( lg );
		lg->Activate();
#endif
	}

	return 0;
}


// FIXME: remove this function and use proper XML def
// eventually simplify Mus Physic system
static int Dynamics_ConstructSoftFromClump ( NinjaLua::LuaState* pobState )
{
	return 0;
}

//-------------------------------------------------------------------------------------------------
// BINDFUNC: Dynamics_ConstructCompoundRigidFromClump( luaobject Properties )
// DESCRIPTION: Add InAir state for an entity so it can be thrown.
//-------------------------------------------------------------------------------------------------
static int Dynamics_ConstructCompoundRigidFromClump ( NinjaLua::LuaState* pobState )
{
	NinjaLua::LuaStack args( pobState );
	NinjaLua::LuaObject obProperties( args[1] );

	// if we are just a normal lua table this will provide a LuaAttributeTable interface to it
	LuaAttributeTable temp( obProperties ); 
	// get a lua attribute table, if we were passed a normal lua table handle that as well

	LuaAttributeTable* pAttrTable;
	if(args[1].IsString())
	{
		DataObject* pDO = ObjectDatabase::Get().GetDataObjectFromName( args[1].GetString() );
		temp.SetDataObject( pDO );
		pAttrTable = &temp;
	}
	else if (args[1].IsTable())
	{
		pAttrTable = &temp;
	}
	else
	{
		pAttrTable = (LuaAttributeTable*)lua_touserdata(*pobState , 1); //pAttrTable = (LuaAttributeTable*) args[1].Get<LuaAttributeTable*>(); 
		if( pAttrTable == 0 )
		{
			pAttrTable = &temp;
		}
	}
#ifndef _PS3_RUN_WITHOUT_HAVOK_BUILD
	CEntity* pobEnt = CLuaGlobal::Get().GetTarg();
#endif
	//ntAssert( pobEnt->GetDynamics() );

	if (obProperties.IsNil()) // No properties have been defined so create rigid state from defaults
	{

#ifndef _PS3_RUN_WITHOUT_HAVOK_BUILD
#ifdef AUTOMATIC_NEW_SYSTEM_CONVERSION

		//static char commandLine[ MAX_PATH * 2 ];
		static char acFileName[ MAX_PATH * 2 ];
		static char psFileName[ MAX_PATH ];
		memset( acFileName, 0 , sizeof( acFileName ) );

			CHashedString clumpString = pobEnt->GetClumpString();
			strcpy( psFileName, *clumpString);
			int stringLength = strlen(psFileName);
			strcpy( psFileName + stringLength - 5, "ps.xml\0" );
			ntPrintf("Converting %s to %s...\n",*clumpString, psFileName);

			ntstd::string temp = *g_ShellOptions->m_obContentPlatform;
			temp += "\\";
			temp += *clumpString;

			ntstd::string temp2 = g_ShellOptions->m_contentNeutral;
			temp2 += "\\";
			temp2 += psFileName;

			temp += " ";
			temp += temp2;

			//Util::GetFiosFilePath( temp.c_str(), acFileName );

			//ntPrintf("Command line %s...\n", acFileName);
			//int ret = spawnl( P_WAIT, "C:\\alienbrainWork\\HS\\content_pc\\ClumpToPSXML.exe", "ClumpToPSXML.exe", acFileName );

			temp2 = "C:\\alienbrainWork\\HS\\content_pc\\ClumpToPSXML.exe ";
			temp2 += temp;

			Util::GetFiosFilePath( temp2.c_str(), acFileName );

			int ret = system( acFileName ) ;

			if( -1 == ret )
			{
				ntPrintf("Failed to call ClumpToPSXML.exe!...\n");
			}
#endif

		// 2 - Create a group
		//Physics::CompoundLG* lg = Physics::ClumpTools::ConstructCompoundLGFromClump( pobEnt, 0 );
		Physics::CompoundLG* lg = NT_NEW Physics::CompoundLG(pobEnt->GetName(), pobEnt); 
		lg->Load(); 

		// 3 - Create a system
		if( pobEnt->GetPhysicsSystem() == 0 )
		{
			Physics::System* system = NT_NEW_CHUNK(Mem::MC_ENTITY) Physics::System( pobEnt, pobEnt->GetName() );
			pobEnt->SetPhysicsSystem( system );
		}
		// 4 - Add the group
		pobEnt->GetPhysicsSystem()->AddGroup( lg );
		lg->Activate();
#endif
	}
	else // Create rigid body from user defined parameters
	{
		CVector obVector(pAttrTable->GetVector("CenterOfMass"));
#ifndef _PS3_RUN_WITHOUT_HAVOK_BUILD
		hkRigidBodyCinfo obInfo;
		obInfo.m_motionType = hkMotion::MOTION_BOX_INERTIA;
		obInfo.m_mass = pAttrTable->GetNumber("Mass");
		obInfo.m_centerOfMass = hkVector4(obVector.X(),obVector.Y(),obVector.Z());
		obInfo.m_restitution = pAttrTable->GetNumber("Restitution");
		obInfo.m_friction = pAttrTable->GetNumber("Friction");
		obInfo.m_linearDamping = pAttrTable->GetNumber("LinearDamping");
		obInfo.m_angularDamping = pAttrTable->GetNumber("AngularDamping");
		//HACK to save art reexport
		if (obInfo.m_angularDamping < 0.05f)
			obInfo.m_angularDamping = 0.05f;
		obInfo.m_angularDamping *= 6;
		obInfo.m_maxLinearVelocity = pAttrTable->GetNumber("MaxLinearVelocity");
		obInfo.m_maxAngularVelocity = pAttrTable->GetNumber("MaxAngularVelocity");

		NinjaLua::LuaObject LuaMotionType = pAttrTable->GetAttribute("MotionType");
		if (LuaMotionType.IsString())
		{
			if (strcmp(	LuaMotionType.GetString(), "FIXED" )==0)
			{
				obInfo.m_motionType = hkMotion::MOTION_FIXED;
				obInfo.m_qualityType = HK_COLLIDABLE_QUALITY_FIXED;
			}
			else if (strcmp(	LuaMotionType.GetString(), "KEYFRAMED" )==0)
			{
				obInfo.m_motionType = hkMotion::MOTION_KEYFRAMED;
				obInfo.m_qualityType = HK_COLLIDABLE_QUALITY_KEYFRAMED;
			}
			else if (strcmp(	LuaMotionType.GetString(), "DYNAMIC" )==0)
			{
				obInfo.m_motionType = hkMotion::MOTION_DYNAMIC;
				obInfo.m_qualityType = HK_COLLIDABLE_QUALITY_MOVING;
			}
		}
		else
		{
			obInfo.m_motionType = hkMotion::MOTION_DYNAMIC;
			obInfo.m_qualityType = HK_COLLIDABLE_QUALITY_MOVING;
		}


#ifdef AUTOMATIC_NEW_SYSTEM_CONVERSION

		//static char commandLine[ MAX_PATH * 2 ];
		static char acFileName[ MAX_PATH * 2 ];
		static char psFileName[ MAX_PATH ];
		memset( acFileName, 0 , sizeof( acFileName ) );

			CHashedString clumpString = pobEnt->GetClumpString();
			strcpy( psFileName, *clumpString);
			int stringLength = strlen(psFileName);
			strcpy( psFileName + stringLength - 5, "ps.xml\0" );
			ntPrintf("Converting %s to %s...\n",*clumpString, psFileName);

			ntstd::string temp = *g_ShellOptions->m_obContentPlatform;
			temp += "\\";
			temp += *clumpString;

			ntstd::string temp2 = g_ShellOptions->m_contentNeutral;
			temp2 += "\\";
			temp2 += psFileName;

			temp += " ";
			temp += temp2;

			//Util::GetFiosFilePath( temp.c_str(), acFileName );

			//ntPrintf("Command line %s...\n", acFileName);
			//int ret = spawnl( P_WAIT, "C:\\alienbrainWork\\HS\\content_pc\\ClumpToPSXML.exe", "ClumpToPSXML.exe", acFileName );

			temp2 = "C:\\alienbrainWork\\HS\\content_pc\\ClumpToPSXML.exe ";
			temp2 += temp;

			Util::GetFiosFilePath( temp2.c_str(), acFileName );

			int ret = system( acFileName ) ;

			if( -1 == ret )
			{
				ntPrintf("Failed to call ClumpToPSXML.exe!...\n");
			}
#endif

		// 2 - Create a group
		//Physics::CompoundLG* lg = Physics::ClumpTools::ConstructCompoundLGFromClump( pobEnt, &obInfo );
		Physics::CompoundLG* lg = NT_NEW Physics::CompoundLG(pobEnt->GetName(), pobEnt); 
		lg->Load(); 

		// 3 - Create a system
		if( pobEnt->GetPhysicsSystem() == 0 )
		{
			Physics::System* system = NT_NEW_CHUNK(Mem::MC_ENTITY) Physics::System( pobEnt, pobEnt->GetName() );
			pobEnt->SetPhysicsSystem( system );
		}
		// 4 - Add the group
		pobEnt->GetPhysicsSystem()->AddGroup( lg );
		lg->Activate();
#endif
	}

	return 0;
}

//-------------------------------------------------------------------------------------------------
// BINDFUNC: Dynamics_ConstructSpearFromClump( luaobject Properties )
// DESCRIPTION: Add InAir state for an entity so it can be thrown.
//-------------------------------------------------------------------------------------------------
static int Dynamics_ConstructSpearFromClump ( NinjaLua::LuaState* pobState )
{
	NinjaLua::LuaStack args( pobState );
	NinjaLua::LuaObject obProperties( args[1] );

	// if we are just a normal lua table this will provide a LuaAttributeTable interface to it
	LuaAttributeTable temp( obProperties ); 
	// get a lua attribute table, if we were passed a normal lua table handle that as well

	LuaAttributeTable* pAttrTable;
	if(args[1].IsString())
	{
		DataObject* pDO = ObjectDatabase::Get().GetDataObjectFromName( args[1].GetString() );
		temp.SetDataObject( pDO );
		pAttrTable = &temp;
	}
	else if (args[1].IsTable())
	{
		pAttrTable = &temp;
	}
	else
	{
		pAttrTable = (LuaAttributeTable*)lua_touserdata(*pobState , 1); //pAttrTable = (LuaAttributeTable*) args[1].Get<LuaAttributeTable*>();
		if( pAttrTable == 0 )
		{
			pAttrTable = &temp;
		}
	}
#ifndef _PS3_RUN_WITHOUT_HAVOK_BUILD
	CEntity* pobEnt = CLuaGlobal::Get().GetTarg();
#endif

	//ntAssert( pobEnt->GetDynamics() );

	if (obProperties.IsNil()) // No properties have been defined so create rigid state from defaults
	{
#ifndef _PS3_RUN_WITHOUT_HAVOK_BUILD
#ifdef AUTOMATIC_NEW_SYSTEM_CONVERSION

		//static char commandLine[ MAX_PATH * 2 ];
		static char acFileName[ MAX_PATH * 2 ];
		static char psFileName[ MAX_PATH ];
		memset( acFileName, 0 , sizeof( acFileName ) );

			CHashedString clumpString = pobEnt->GetClumpString();
			strcpy( psFileName, *clumpString);
			int stringLength = strlen(psFileName);
			strcpy( psFileName + stringLength - 5, "ps.xml\0" );
			ntPrintf("Converting %s to %s...\n",*clumpString, psFileName);

			ntstd::string temp = *g_ShellOptions->m_obContentPlatform;
			temp += "\\";
			temp += *clumpString;

			ntstd::string temp2 = g_ShellOptions->m_contentNeutral;
			temp2 += "\\";
			temp2 += psFileName;

			temp += " ";
			temp += temp2;

			//Util::GetFiosFilePath( temp.c_str(), acFileName );

			//ntPrintf("Command line %s...\n", acFileName);
			//int ret = spawnl( P_WAIT, "C:\\alienbrainWork\\HS\\content_pc\\ClumpToPSXML.exe", "ClumpToPSXML.exe", acFileName );

			temp2 = "C:\\alienbrainWork\\HS\\content_pc\\ClumpToPSXML.exe ";
			temp2 += temp;

			Util::GetFiosFilePath( temp2.c_str(), acFileName );

			int ret = system( acFileName ) ;

			if( -1 == ret )
			{
				ntPrintf("Failed to call ClumpToPSXML.exe!...\n");
			}
#endif
		// 2 - Create a group
		//Physics::SpearLG* lg = Physics::ClumpTools::ConstructSpearLGFromClump( pobEnt, 0 );
		Physics::SpearLG* lg = NT_NEW Physics::SpearLG(pobEnt->GetName(), pobEnt);
		lg->Load();

		// 3 - Create a system
		if( pobEnt->GetPhysicsSystem() == 0 )
		{
			Physics::System* system = NT_NEW_CHUNK(Mem::MC_ENTITY) Physics::System( pobEnt, pobEnt->GetName() );
			pobEnt->SetPhysicsSystem( system );
		}
		// 4 - Add the group
		pobEnt->GetPhysicsSystem()->AddGroup( lg );
		lg->Activate();
#endif
	}
	else // Create rigid body from user defined parameters
	{
		CVector obVector(pAttrTable->GetVector("CenterOfMass"));
#ifndef _PS3_RUN_WITHOUT_HAVOK_BUILD
		hkRigidBodyCinfo obInfo;

		obInfo.m_motionType = hkMotion::MOTION_DYNAMIC;
		obInfo.m_qualityType = HK_COLLIDABLE_QUALITY_MOVING;
		
		obInfo.m_mass = pAttrTable->GetNumber("Mass");
		obInfo.m_centerOfMass.set(obVector.X(),obVector.Y(),obVector.Z());
		obInfo.m_restitution = pAttrTable->GetNumber("Restitution");
		obInfo.m_friction = pAttrTable->GetNumber("Friction");
		obInfo.m_linearDamping = pAttrTable->GetNumber("LinearDamping");
		obInfo.m_angularDamping = pAttrTable->GetNumber("AngularDamping");
		//HACK to save art reexport
		if (obInfo.m_angularDamping < 0.05f)
			obInfo.m_angularDamping = 0.05f;
		obInfo.m_angularDamping *= 6;
		obInfo.m_maxLinearVelocity = pAttrTable->GetNumber("MaxLinearVelocity");
		obInfo.m_maxAngularVelocity = pAttrTable->GetNumber("MaxAngularVelocity");

		NinjaLua::LuaObject LuaMotionType = pAttrTable->GetAttribute("MotionType");
		if (LuaMotionType.IsString())
		{
			if (strcmp(	LuaMotionType.GetString(), "FIXED" )==0)
			{
				obInfo.m_motionType = hkMotion::MOTION_FIXED;
				obInfo.m_qualityType = HK_COLLIDABLE_QUALITY_FIXED;
			}
			else if (strcmp(	LuaMotionType.GetString(), "KEYFRAMED" )==0)
			{
				obInfo.m_motionType = hkMotion::MOTION_KEYFRAMED;
				obInfo.m_qualityType = HK_COLLIDABLE_QUALITY_KEYFRAMED;
			}
			else if (strcmp(	LuaMotionType.GetString(), "DYNAMIC" )==0)
			{
				obInfo.m_motionType = hkMotion::MOTION_DYNAMIC;
				obInfo.m_qualityType = HK_COLLIDABLE_QUALITY_MOVING;
			}
		}

#ifdef AUTOMATIC_NEW_SYSTEM_CONVERSION

		//static char commandLine[ MAX_PATH * 2 ];
		static char acFileName[ MAX_PATH * 2 ];
		static char psFileName[ MAX_PATH ];
		memset( acFileName, 0 , sizeof( acFileName ) );

			CHashedString clumpString = pobEnt->GetClumpString();
			strcpy( psFileName, *clumpString);
			int stringLength = strlen(psFileName);
			strcpy( psFileName + stringLength - 5, "ps.xml\0" );
			ntPrintf("Converting %s to %s...\n",*clumpString, psFileName);

			ntstd::string temp = *g_ShellOptions->m_obContentPlatform;
			temp += "\\";
			temp += *clumpString;

			ntstd::string temp2 = g_ShellOptions->m_contentNeutral;
			temp2 += "\\";
			temp2 += psFileName;

			temp += " ";
			temp += temp2;

			//Util::GetFiosFilePath( temp.c_str(), acFileName );

			//ntPrintf("Command line %s...\n", acFileName);
			//int ret = spawnl( P_WAIT, "C:\\alienbrainWork\\HS\\content_pc\\ClumpToPSXML.exe", "ClumpToPSXML.exe", acFileName );

			temp2 = "C:\\alienbrainWork\\HS\\content_pc\\ClumpToPSXML.exe ";
			temp2 += temp;

			Util::GetFiosFilePath( temp2.c_str(), acFileName );

			int ret = system( acFileName ) ;

			if( -1 == ret )
			{
				ntPrintf("Failed to call ClumpToPSXML.exe!...\n");
			}
#endif

		// 2 - Create a group
		//Physics::SpearLG* lg = Physics::ClumpTools::ConstructSpearLGFromClump( pobEnt, &obInfo );
		Physics::SpearLG* lg = NT_NEW Physics::SpearLG(pobEnt->GetName(), pobEnt);
		lg->Load();

		// 3 - Create a system
		if( pobEnt->GetPhysicsSystem() == 0 )
		{
			Physics::System* system = NT_NEW_CHUNK(Mem::MC_ENTITY) Physics::System( pobEnt, pobEnt->GetName() );
			pobEnt->SetPhysicsSystem( system );
		}
		// 4 - Add the group
		pobEnt->GetPhysicsSystem()->AddGroup( lg );
		lg->Activate();
#endif
	}

	return 0;
}

//-------------------------------------------------------------------------------------------------
// BINDFUNC: Dynamics_ConstructProjectile (NinjaLua::LuaObject obTable)
// DESCRIPTION: 
//-------------------------------------------------------------------------------------------------
static void Dynamics_ConstructProjectile (NinjaLua::LuaObject obTable)
{
	CEntity* pobSelf=CLuaGlobal::Get().GetTarg();

	//ntAssert( pobSelf->GetPhysicsSystem() );

	const char* pcProjectileProperties=obTable["ProjectileProperties"].GetString();

	ProjectileProperties* pobProperties=ObjectDatabase::Get().GetPointerFromName<ProjectileProperties*>(pcProjectileProperties);
	ntAssert(pobProperties);
	
	CPoint obPosition(obTable["Position"]["x"].GetFloat(),obTable["Position"]["y"].GetFloat(),obTable["Position"]["z"].GetFloat());
	CDirection obDirection;

	if (obTable["FirstPersonAim"].IsBoolean() && obTable["FirstPersonAim"].GetBoolean()==true)
	{
		const CMatrix& obCameraMatrix=CamMan::Get().GetPrimaryView()->GetCurrMatrix();

		CDirection obForward=CDirection(0.0f,0.0f,50.0f) * obCameraMatrix;

		CPoint obEnd=obCameraMatrix.GetTranslation() + obForward;

		Physics::RaycastCollisionFlag obFlag; obFlag.base = 0;
		obFlag.flags.i_am = Physics::COLLISION_ENVIRONMENT_BIT;
		obFlag.flags.i_collide_with = (	//Physics::CHARACTER_CONTROLLER_PLAYER_BIT	| 
										Physics::CHARACTER_CONTROLLER_ENEMY_BIT	|
										Physics::RAGDOLL_BIT						|
										Physics::SMALL_INTERACTABLE_BIT				|
										Physics::LARGE_INTERACTABLE_BIT				);

		Physics::TRACE_LINE_QUERY stQuery;

		if (Physics::CPhysicsWorld::Get().TraceLine(obCameraMatrix.GetTranslation(),obEnd,pobSelf,stQuery,obFlag))
		{
			obDirection.X()=stQuery.obIntersect.X()-obPosition.X();
			obDirection.Y()=stQuery.obIntersect.Y()-obPosition.Y();
			obDirection.Z()=stQuery.obIntersect.Z()-obPosition.Z();
			obDirection.Normalise();
		}
		else
		{
			obDirection.X()=obEnd.X()-obPosition.X();
			obDirection.Y()=obEnd.Y()-obPosition.Y();
			obDirection.Z()=obEnd.Z()-obPosition.Z();
			obDirection.Normalise();
		}
	}
	else
	{
		obDirection.X()=obTable["Direction"]["x"].GetFloat();
		obDirection.Y()=obTable["Direction"]["y"].GetFloat();
		obDirection.Z()=obTable["Direction"]["z"].GetFloat();
	}

	//pobSelf->GetDynamics()->ConstructProjectile("Projectile",pobProperties,obPosition,obDirection);
	// 2 - Create a group
	Physics::ProjectileLG* lg = Physics::ProjectileLG::Construct( pobSelf, pobProperties, obPosition, obDirection );
	// 3 - Create a system
	if( pobSelf->GetPhysicsSystem() == 0 )
	{
		Physics::System* system = NT_NEW_CHUNK(Mem::MC_ENTITY) Physics::System( pobSelf, pobSelf->GetName() );
		pobSelf->SetPhysicsSystem( system );
	}
	// 4 - Add the group
	pobSelf->GetPhysicsSystem()->AddGroup( lg );
	lg->Activate();
}


//-------------------------------------------------------------------------------------------------
// BINDFUNC: Dynamics_Explosion (table)
// DESCRIPTION:
//-------------------------------------------------------------------------------------------------
static int Dynamics_Explosion ( NinjaLua::LuaState* pobState )
{
#ifndef _PS3_RUN_WITHOUT_HAVOK_BUILD
	NinjaLua::LuaStack args( pobState );

	ntAssert( args[1].IsTable() );
	NinjaLua::LuaObject obTable( args[1] );

	// Table properties:
	// Position=x,y,z (point of the explosion)
	// Push=float (Initial velocity the entity is thrown by)
	// Dropoff=float (Decides how much to push depending on how far from the explosion)
	// Radius=float (radius of the explosion)
	// Originator=entity (this should have a valid character which an attack component!)

	if (obTable.IsNil())
		return 0;

	CEntity* pobSelf=NULL;
	CPoint obPosition(CONSTRUCT_CLEAR);

	if (obTable["Position"].IsNil())
	{
		pobSelf = CLuaGlobal::Get().GetTarg();
		obPosition=pobSelf->GetPosition();
	}
	else
	{
		obPosition.X()=obTable["Position"]["x"].GetFloat();
		obPosition.Y()=obTable["Position"]["y"].GetFloat();
		obPosition.Z()=obTable["Position"]["z"].GetFloat();
	}


	/*
	// ----- Temporary for E3: Adjust the Y position of the explosion -----
	Physics::RaycastCollisionFlag flag;
	flag.base = 0;
	flag.flags.i_am = Physics::STATIC_ENVIRONMENT_BIT;
	flag.flags.i_collide_with = ( Physics::LARGE_INTERACTABLE_BIT );

	float fHitFraction;
	CDirection	obHitNormal;

	CDirection obOffset(0.0f,10.0f,0.0f);

	if ( CPhysicsWorld::Get().GetClosestIntersectingSurfaceDetails( obPosition, obPosition+obOffset, fHitFraction, obHitNormal, flag ))
	{
		// hitNormal has been filled in
		CPoint obHitPos = obPosition + (obOffset * fHitFraction);

		ntPrintf("Adjust Y offset of explosion from %f to %f\n",obPosition.Y(),obHitPos.Y());

		obPosition.Y()=obHitPos.Y();
	}
	// --------------------------------------------------------------------
	*/

	const float fPush=obTable["Push"].GetFloat(); // Initial velocity added to entity
	const float fRadius=obTable["Radius"].GetFloat(); // Radius of the explosion in metres
	//const float fDropoff=obTable["Dropoff"].GetFloat(); // Reduction in push per metre (radius-(dropoff * distance from explosion))
	//const float fRadiusSquared=fRadius*fRadius;

	// Search through the entities that might be affected
	
	// [MUS] - As we are interested only by two type of entities (characters and bodies), we should use Havok instead of the Entity Query system
	// PS: Havok is an order of magnitude faster thant the current Query system !
	hkSphereShape sphereShape(fRadius);
	hkMotionState motionState;
	motionState.getTransform() = hkTransform(hkQuaternion::getIdentity(),Physics::MathsTools::CPointTohkVector(obPosition));
	hkCollidable sphereCollidable(&sphereShape, &motionState);

	Physics::EntityCollisionFlag obFlag; obFlag.base = 0;
	obFlag.flags.i_am			=			Physics::LARGE_INTERACTABLE_BIT;
	obFlag.flags.i_collide_with = (			Physics::CHARACTER_CONTROLLER_PLAYER_BIT	| 
											Physics::CHARACTER_CONTROLLER_ENEMY_BIT	|
											Physics::RAGDOLL_BIT						|
											Physics::SMALL_INTERACTABLE_BIT				|
											Physics::LARGE_INTERACTABLE_BIT				);

	sphereCollidable.setCollisionFilterInfo(obFlag.base);

	hkAllCdBodyPairCollector obCollector; 
	Physics::CPhysicsWorld::Get().GetPenetrations(&sphereCollidable, (hkCollisionInput&)*Physics::CPhysicsWorld::Get().GetCollisionInput(), obCollector);

	// See if we can find an originating entity for this explosion
	CEntity* pobOriginator = NULL;
	if ( obTable["Originator"].IsLightUserData() )
		pobOriginator = static_cast< CEntity* >( obTable["Originator"].GetLightUserData() );
	else if(obTable["Originator"].IsUserData())
		pobOriginator = *((CEntity**)obTable["Originator"].GetUserData());

	// See if we can find an attack move which provides the strike data for an explosion
	CAttackData* pobAttackData = ObjectDatabase::Get().GetPointerFromName< CAttackData* >( "atk_explosion" );

	//for(ntstd::List<CEntity*>::iterator obIt=obQuery.GetResults().begin(); obIt!=obQuery.GetResults().end(); ++obIt)
	for(int i=0; i < obCollector.getHits().getSize(); ++i)
	{
		CEntity* pobEntity = 0;

		hkRigidBody* obRB = hkGetRigidBody(obCollector.getHits()[i].m_rootCollidableA);
		if(0 == obRB)
			obRB = hkGetRigidBody(obCollector.getHits()[i].m_rootCollidableB);

		if((obRB) && (!obRB->isFixed()))
		{
			pobEntity = (CEntity*) obRB->getProperty(Physics::PROPERTY_ENTITY_PTR).getPtr();	
		} else {

			hkPhantom* obPH = hkGetPhantom(obCollector.getHits()[i].m_rootCollidableA);
			if(0 == obPH)
				obPH = hkGetPhantom(obCollector.getHits()[i].m_rootCollidableB);

			if(obPH) 
				pobEntity = (CEntity*) obPH->getProperty(Physics::PROPERTY_ENTITY_PTR).getPtr();
				
		};

		if ( pobEntity )
		{
			CDirection obDirection(pobEntity->GetPosition() - obPosition);

			{
				if (pobEntity->GetAttackComponent() && pobEntity->ToCharacter()->GetCurrHealth()>0.0f)// Entity should receive a combat strike
				{
					if (pobAttackData && pobEntity!=pobOriginator)
					{
						// Find an originating position
						CPoint obOriginatingPosition = ( pobOriginator ) ? pobOriginator->GetPosition() : CPoint( 0.0f, 0.0f, 0.0f );

						CStrike* pobStrike = NT_NEW CStrike(	0,
															pobEntity,
															pobAttackData,
															1.0f,
															1.0f,
															false,
															false,
															false,
															false, 
															false,
															false,
															0,
															obOriginatingPosition );

						// pobEntity->GetAttackComponent()->ReceiveStrike(pobStrike);
						SyncdCombat::Get().PostStrike( pobStrike );

						if ( pobEntity->GetMessageHandler() )
						{
							pobEntity->GetMessageHandler()->ReceiveMsg<msg_combat_struck>();
						}
					}
				}
				else if (pobEntity->GetPhysicsSystem())
				{
					// NOTE: Need to check for line of sight

					/*
					float fThisPush=fPush-(fDropoff*fsqrtf(fDistanceSquared));

					if (fThisPush<0.0f)
					{
						fThisPush=0.0f;
					}
					*/

					float fThisPush=fPush;

					obDirection.Normalise();
					obDirection*=fThisPush;

					if (pobEntity!=pobSelf)
					{
						// [Mus] - Do not apply impulses over continuous frames !
						pobEntity->GetPhysicsSystem()->ApplyLocalisedLinearImpulse(obDirection,CVector(obPosition));

						//ntPrintf("LUA: explosion hit %s, push is %f\n",(*obIt)->GetName(),fThisPush);

						if ( pobEntity->GetMessageHandler() )
						{
							CMessageSender::SendEmptyMessage( "msg_blast_damage", pobEntity->GetMessageHandler() );
						}
					}
				}
			}
		}
	}
#else
	UNUSED( pobState );
#endif
	return 0;
}


//-------------------------------------------------------------------------------------------------
// BINDFUNC: Dynamics_AddStaticProjectile(CEntity* pobEntity)
// DESCRIPTION: 
//-------------------------------------------------------------------------------------------------
static void Dynamics_AddStaticProjectile (CEntity* pobEntity)
{
	Physics::ProjectileManager::Get().AddStaticProjectile(pobEntity);
}

//-------------------------------------------------------------------------------------------------
// BINDFUNC: Trigger_Activate(string)
// DESCRIPTION: 
//-------------------------------------------------------------------------------------------------
static bool Trigger_Activate (const char* pcName)
{
	CHECK_STR(pcName);
	return CTriggerVolumeManager::Get().ActivateTrigger(pcName);
}

//-------------------------------------------------------------------------------------------------
// BINDFUNC: Trigger_Deactivate(string)
// DESCRIPTION: 
//-------------------------------------------------------------------------------------------------
static bool Trigger_Deactivate (const char* pcName)
{
	CHECK_STR(pcName);
	return CTriggerVolumeManager::Get().DeactivateTrigger(pcName);
}

//-------------------------------------------------------------------------------------------------
// BINDFUNC: Dynamics_RangeFastAttack(LifeDuration,Power,BeginPosition,EndPosition)
// DESCRIPTION: 
//-------------------------------------------------------------------------------------------------
static int Dynamics_RangeFastAttack ( NinjaLua::LuaState* pobState)
{
#ifndef _PS3_RUN_WITHOUT_HAVOK_BUILD
	CEntity* pobSelf = CLuaGlobal::Get().GetTarg();

	ntAssert( pobSelf );
	//ntAssert( pobSelf->GetDynamics() );

	const CMatrix obWorldMatrix=pobSelf->GetMatrix();

	const CPoint& obWorldTranslation=obWorldMatrix.GetTranslation();


	const float fFORWARD_OFFSET = 3.0f;
	const float fUP_OFFSET = 0.5f;
	const float fBOX_X = 2.0f;//1.0f;
	const float fBOX_Y = 0.5f;//0.5f;
	const float fBOX_Z = 3.0f;
	const float fMIN_VELOCITY = 10.0f;
	const float fMAX_VELOCITY = 40.0f;
	const float fMIN_Y_VELOCITY = 5.0f;
	const float fMAX_Y_VELOCITY = 10.0f;



	CDirection obForward(obWorldMatrix.GetZAxis());
	obForward*=fFORWARD_OFFSET;
	
	CPoint obCentre(obWorldTranslation);
	obCentre+=obForward;
	obCentre.Y()+=fUP_OFFSET;


//	CMatrix obDebugMatrix(obWorldMatrix);
//	obDebugMatrix.SetTranslation(obCentre);

//	g_VisualDebug->RenderOBB(obDebugMatrix,CDirection(fBOX_X,fBOX_Y,fBOX_Z),0x77ffffff);


	CQuat obQuatRotation(obWorldMatrix);

	hkVector4 obPosition(obCentre.X(),obCentre.Y(),obCentre.Z());

	hkQuaternion obRotation = Physics::MathsTools::CQuatTohkQuaternion(obQuatRotation);

	hkTransform obLocalTransform(obRotation,obPosition);


	hkVector4 obHalfExtents(fBOX_X,fBOX_Y,fBOX_Z);
	hkBoxShape obBoxShape(obHalfExtents);


	hkCollidable obCollidable(&obBoxShape,&obLocalTransform);




	//Get all shapes which are penetrating the collidable Note: If you have call this function every step for a given object, use the hkShapePhantom version.
	hkAllCdBodyPairCollector obCollector;

	Physics::CPhysicsWorld::Get().GetPenetrations(&obCollidable, (hkCollisionInput &)*Physics::CPhysicsWorld::Get().GetCollisionInput(), obCollector);
	
	const hkArray<hkRootCdBodyPair>& obHits = obCollector.getHits();

	for(int iHit=0;iHit<obHits.getSize();iHit++)
	{
		const hkRootCdBodyPair* pobCurrentPair = &obHits[iHit];
		const hkCollidable* pobCollided = pobCurrentPair->m_rootCollidableB;

		if(pobCollided->getCollisionFilterInfo())
		{
			hkRigidBody* obRB = hkGetRigidBody(pobCollided);

			if(obRB)
			{

				float fVelocity=fMIN_VELOCITY+grandf(fMAX_VELOCITY-fMIN_VELOCITY);

				float fVelocityY=fMIN_Y_VELOCITY+grandf(fMAX_Y_VELOCITY-fMIN_Y_VELOCITY);

				hkVector4 obImpulseVelocity(
					obWorldMatrix.GetZAxis().X() * fVelocity,
					(obWorldMatrix.GetZAxis().Y() * fVelocity) + fVelocityY,
					obWorldMatrix.GetZAxis().Z() * fVelocity);

				hkVector4 obCurrentVelocity(obRB->getLinearVelocity());

				hkVector4 obNewVelocity(obImpulseVelocity);

				obNewVelocity.sub4(obCurrentVelocity);

				obNewVelocity.mul4(obRB->getMass());

				obRB->applyLinearImpulse(obNewVelocity);

				CEntity* rbEntity = (CEntity*) obRB->getProperty(Physics::PROPERTY_ENTITY_PTR).getPtr();
				if( rbEntity )
				{
					// Hurt for 5 seconds
					//rbEntity->GetPhysicsSystem()->DoHurtOnCollision( 5.0f );
					rbEntity->GetPhysicsSystem()->GetCollisionStrikeHandler()->Enable(1,5.0f);
				}
			}

/*			else
			{
				hkPhantom* obPH = hkGetPhantom(pobCollided);
	
				if(obPH)
				{
					CEntity* pobEntity = (CEntity*) obPH->getProperty(Physics::PROPERTY_ENTITY_PTR).asPtr();

					if (pobEntity->IsEnemy() && pobEntity->GetCurrHealth() > 0.0f)
					{

					}
				}
			}*/
		}
	}
#else
	UNUSED( pobState );
#endif
	return 0;
}


// parameters: velx, vely, velz, controlling entity, AT parameters

static int Dynamics_Ragdoll_StartAimedThrow ( NinjaLua::LuaState* pobState )
{
	//ntPrintf("Dynamics_Ragdoll_StartAimedThrow\n");

	CEntity* pobThis = CLuaGlobal::Get().GetTarg();
	ntAssert( pobThis );

	NinjaLua::LuaStack args(pobState);

	CDirection obVelocity(args[1].GetFloat(),args[2].GetFloat(),args[3].GetFloat());


	const CMatrix& obCameraMatrix=CamMan::GetPrimaryView()->GetCurrMatrix();

	CDirection obDelta(pobThis->GetPosition() - obCameraMatrix.GetTranslation());
	float fDistanceBetweenObjectAndCamera=obDelta.Length();

	CDirection obOffset1=CDirection(0.0f,0.0f,fDistanceBetweenObjectAndCamera) * obCameraMatrix; // Prevent the raycast from hitting stuff thats behind the player, such as a wall
	CDirection obOffset2=CDirection(0.0f,0.0f,50.0f) * obCameraMatrix;

	CPoint obStart(obCameraMatrix.GetTranslation() + obOffset1);
	CPoint obEnd(obCameraMatrix.GetTranslation() + obOffset2);

	Physics::RaycastCollisionFlag obFlag; obFlag.base = 0;
	obFlag.flags.i_am = Physics::COLLISION_ENVIRONMENT_BIT;
	obFlag.flags.i_collide_with = (	//Physics::CHARACTER_CONTROLLER_PLAYER_BIT	| 
									Physics::CHARACTER_CONTROLLER_ENEMY_BIT	|
									Physics::RAGDOLL_BIT						|
									Physics::SMALL_INTERACTABLE_BIT				|
									Physics::LARGE_INTERACTABLE_BIT				);

	Physics::TRACE_LINE_QUERY stQuery;

	if (Physics::CPhysicsWorld::Get().TraceLine(obStart,obEnd,pobThis,stQuery,obFlag))
	{
		CDirection obNewVelocity(stQuery.obIntersect - pobThis->GetPosition());
		obNewVelocity.Normalise();
		obNewVelocity*=obVelocity.Z(); // We are currently only using the Z component from the input velocity vector
		obVelocity=obNewVelocity;
	}
	else
	{
		CDirection obNewVelocity(obEnd - pobThis->GetPosition());
		obNewVelocity.Normalise();
		obNewVelocity*=obVelocity.Z(); // We are currently only using the Z component from the input velocity vector
		obVelocity=obNewVelocity;
	}

	RagdollThrownControllerDef obDef;
	obDef.m_obLinearVelocity=obVelocity;

	if (args[6].IsBoolean() && args[6].GetBoolean()==true)
		obDef.m_pobControllingEntity=args[4].Get<CEntity*>();//static_cast< CEntity* >(args[4].GetLightUserData());//*(CEntity**)args[4].GetUserData();

	obDef.m_pobParameters=ObjectDatabase::Get().GetPointerFromName<AftertouchControlParameters*>(args[5].GetString());

	pobThis->GetMovement()->BringInNewController( obDef, CMovement::DMM_STANDARD, CMovement::COMBAT_BLEND );

	return 0;
}

static int Dynamics_Ragdoll_StartThrow ( NinjaLua::LuaState* pobState )
{
	//ntPrintf("Dynamics_Ragdoll_StartThrow\n");

	CEntity* pobThis = CLuaGlobal::Get().GetTarg();
	ntAssert( pobThis );

	NinjaLua::LuaStack args(pobState);

	CDirection obVelocity(args[1].GetFloat(),args[2].GetFloat(),args[3].GetFloat());


	CEntity* pobControllingEntity=0;

	pobControllingEntity = args[4].Get<CEntity*>();

	/*
	if(args[4].IsLightUserData())
	{
		pobControllingEntity = static_cast< CEntity* >(args[4].GetLightUserData());
	}
	else if(args[4].IsUserData())
	{
		//pobControllingEntity = NinjaLua::LuaValue::Get<CEntity*>( NinjaLua::GetState( pobState->GetCState() ), 4 );

		pobControllingEntity=args[4].Get<CEntity*>();// *(CEntity**)args[4].GetUserData();
	}
	*/

	//CEntity* pobControllingEntity=*(CEntity**)args[4].GetUserData();

	// Get a target based on the direction the character is facing
	CEntity* pobTarget=pobControllingEntity->GetAwarenessComponent()->GetTarget( AT_TYPE_THROW, pobControllingEntity, true );

	CDirection obResultantVelocity;

	if (pobTarget) // We have a target
	{
		CMatrix obLookAtTarget;

		CPoint obFrom(pobThis->GetPosition());
		obFrom.Y()=0.0f;

		CPoint obTo(pobTarget->GetPosition());
		obTo.Y()=0.0f;

		CCamUtil::CreateFromPoints(obLookAtTarget,obFrom,obTo);

		obResultantVelocity=obVelocity * obLookAtTarget;
	}
	else // No target, use the direction the entity is facing in
	{
		obResultantVelocity=obVelocity * pobControllingEntity->GetMatrix();
	}

	RagdollThrownControllerDef obDef;
	obDef.m_obLinearVelocity=obResultantVelocity;

	//if (args[6].IsBoolean() && args[6].GetBoolean()==true)
		obDef.m_pobControllingEntity=pobControllingEntity;

	obDef.m_pobParameters=ObjectDatabase::Get().GetPointerFromName<AftertouchControlParameters*>(args[5].GetString());

	pobThis->GetMovement()->BringInNewController( obDef, CMovement::DMM_STANDARD, CMovement::COMBAT_BLEND );

	return 0;
}

static int Dynamics_SetGravityOnCharacterController( NinjaLua::LuaState* pobState )
{
	CEntity* pobThis = CLuaGlobal::Get().GetTarg();
	ntAssert( pobThis );

	NinjaLua::LuaStack args(pobState);
	ntAssert(args[1].IsBoolean());

	Physics::AdvancedCharacterController* pobAdvCC = (Physics::AdvancedCharacterController*) pobThis->GetPhysicsSystem()->GetFirstGroupByType( Physics::LogicGroup::ADVANCED_CHARACTER_CONTROLLER );
	if (pobAdvCC)
	{
		pobAdvCC->SetApplyCharacterControllerGravity(args[1].GetBoolean());
	}

	return 0;
}


/***************************************************************************************************
*
*	FUNCTION		CDynamicsBindings::Register
*
*	DESCRIPTION		
*
***************************************************************************************************/
void CDynamicsBindings::Register()
{
	NinjaLua::LuaObject obLuaGlobals = CLuaGlobal::Get().State().GetGlobals();

	// Dynamics state construction
	obLuaGlobals.RegisterRaw( 		"Dynamics_ConstructSoftFromClump",				Dynamics_ConstructSoftFromClump );
	obLuaGlobals.Register( 			"Dynamics_ConstructProjectile",					Dynamics_ConstructProjectile );

	obLuaGlobals.RegisterRaw( 		"Dynamics_ConstructRigidFromClump",				Dynamics_ConstructRigidFromClump );
	obLuaGlobals.RegisterRaw(		"Dynamics_ConstructCompoundRigidFromClump",		Dynamics_ConstructCompoundRigidFromClump );
	obLuaGlobals.RegisterRaw(		"Dynamics_ConstructSpearFromClump",				Dynamics_ConstructSpearFromClump );

	// State control
	// General
	obLuaGlobals.RegisterRaw( 		"Dynamics_Explosion", Dynamics_Explosion );

	// Animated

	// Ragdolls
	obLuaGlobals.RegisterRaw(		"Dynamics_Ragdoll_StartAimedThrow",	Dynamics_Ragdoll_StartAimedThrow );
	obLuaGlobals.RegisterRaw(		"Dynamics_Ragdoll_StartThrow",		Dynamics_Ragdoll_StartThrow );

	// Projectiles
	obLuaGlobals.Register( 	"Dynamics_AddStaticProjectile", Dynamics_AddStaticProjectile );

	// Trigger volumes
	obLuaGlobals.Register(	"Trigger_Activate", Trigger_Activate );
	obLuaGlobals.Register(	"Trigger_Deactivate", Trigger_Deactivate );

	// Custom stuff	
	obLuaGlobals.RegisterRaw(			"Dynamics_RangeFastAttack", Dynamics_RangeFastAttack );

	obLuaGlobals.RegisterRaw(			"Dynamics_SetGravityOnCharacterController", Dynamics_SetGravityOnCharacterController );
}

