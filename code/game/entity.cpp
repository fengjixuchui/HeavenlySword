/***************************************************************************************************
*
*	DESCRIPTION		Core Entity System Implementation
*
*	NOTES
*
***************************************************************************************************/
#include "Physics/config.h"
#include "Physics/system.h"

#include "game/entity.h"
#include "game/entity.inl"
#include "game/entitymanager.h"
#include "game/entityinfo.h"
#include "game/attacks.h"
#include "game/luaattrtable.h"
#include "game/interactioncomponent.h"
#include "game/renderablecomponent.h"
#include "game/aicomponent.h"
#include "game/movement.h"
#include "game/awareness.h"
#include "game/anonymouscomponent.h"
#include "game/luaexptypes.h"
#include "game/entitycharacter.h"
#include "game/entityai.h"
#include "game/fsm.h"
#include "gui/guimanager.h"

#include "anim/animator.h"
#include "anim/hierarchy.h"
#include "camera/sceneelementcomponent.h"
#include "gfx/clump.h"
#include "gfx/levelofdetail.h"
#include "lua/ninjalua.h"
#include "objectdatabase/dataobject.h"
#include "area/areasystem.h"

#include "tbd/functor.h"

// includes specifically for bind functions. do these really belong here?
#include "ai/ainavgraphmanager.h"
#include "ai/airepulsion.h"
#include "ai/aiformationcomponent.h"
#include "camera/camutils.h"
#include "game/aimcontroller.h"
#include "game/inputcomponent.h"
#include "jamnet/netman.h"
#include "hair/chaincore.h"

// Crappy Ragdogs can't trust their root transforms
#include "physics/advancedcharactercontroller.h"


#include "blendshapes/BlendShapesComponent.h"
#include "blendshapes/xpushapeblending.h"
#include "physics/LookAtComponent.h"
#include "physics/LookAtInfo.h"
#include "gfx/meshinstance.h"
#include "area/arearesourcedb.h"

extern bool g_bAllowMissingData;

START_CHUNKED_INTERFACE(CEntity, Mem::MC_ENTITY)
	PUBLISH_ACCESSOR_WITH_DEFAULT( CPoint, Position, GetPosition, SetPosition, CPoint(0.0f, 0.0f, 0.0f) )
	PUBLISH_ACCESSOR_WITH_DEFAULT( CQuat, Orientation, GetRotation, SetRotation, CQuat(0.0f, 0.0f, 0.0f, 1.0f) )
	PUBLISH_PTR_AS( m_pobParentEntity, ParentEntity )
	PUBLISH_VAR_WITH_DEFAULT_AS( m_ParentTransform, "ROOT", ParentTransform )
	PUBLISH_VAR_AS( m_ConstructionScript, ConstructionScript )
	PUBLISH_VAR_AS( m_DestructionScript, DestructionScript )
	PUBLISH_ACCESSOR( ntstd::String, Clump, GetClumpString, SetClumpString )
	//PUBLISH_ACCESSOR( CHashedString, PhysicsSystem, GetPhysicsSystemString, SetPhysicsSystemString )
	PUBLISH_VAR_AS( m_DefaultDynamics, DefaultDynamics )
	PUBLISH_VAR_WITH_DEFAULT_AS( m_bCastShadows, true, CastShadows )
	PUBLISH_VAR_WITH_DEFAULT_AS( m_bRecieveShadows, true, RecieveShadows )
	PUBLISH_VAR_WITH_DEFAULT_AS( m_bDisableRender, false, DisableRender )

	PUBLISH_VAR_WITH_DEFAULT_AS( m_iMappedAreaInfo, 0, SectorBits )
	PUBLISH_VAR_WITH_DEFAULT_AS( m_replacesEnt, false, ReplacesEnt )
	PUBLISH_VAR_AS( m_entToReplace, SectorLODEnt )

	PUBLISH_PTR_AS( m_pobGameEventsList, Events )
	PUBLISH_PTR_AS( m_pobLookAtInfo, LookAtInfo )

	DECLARE_POSTCONSTRUCT_CALLBACK( OnPostConstruct )
	DECLARE_POSTPOSTCONSTRUCT_CALLBACK( OnPostPostConstruct )
END_STD_INTERFACE


/***************************************************************************************************
*
*	FUNCTION		CEntity::CEntity
*
*	DESCRIPTION		
*
***************************************************************************************************/
CEntity::CEntity() 
:	EntityAnimSet(),
	m_obNewPos( CONSTRUCT_CLEAR ),
	m_obLastPos( CONSTRUCT_CLEAR ),
	m_obCalculatedVel( CONSTRUCT_CLEAR ),
	m_InitialPosition( CONSTRUCT_CLEAR ),
	m_InitialRotation( CONSTRUCT_IDENTITY ),
	m_uiEntityFlags(0),
	m_iMappedAreaInfo( 0 ),
	m_pobParentEntity( 0 ),
	m_pobHierarchy( 0 ),
	m_pobAnimator( 0 ),
	m_pobMovement( 0 ),
	m_pobRenderableComponent( 0 ),
	m_pobMessageHandler( 0 ),
	m_pobPhysicsSystem( 0 ),
	m_pobAwarenessComponent( 0 ),
	m_pobAttackComponent( 0 ),
	m_pobEntityAudioChannel( 0 ),
	m_pobLODComponent( 0 ),
	m_pobInteractionComponent( 0 ),
	m_pSceneElementComponent(0),
	m_pobBlendShapesComponent( 0 ),
	m_pobLookAtComponent( 0 ),
	m_pobFormationComponent(0),
	m_pobAimingComponent(0),
	m_pobOneChain(0),
	m_pFSM( 0 ),
	m_fTimeMultiplier( 1.0f ),
	m_fLastTimeChange( 0.0f ),
	m_obAttributeTable( 0 ),
	m_pobClumpHeader(0),
	m_uiClumpFileHash(0),
	m_bCastShadows( true ),
	m_bRecieveShadows( true ),
	m_bDisableRender( false ),
	m_bHasThink( false ),
	m_pobGameEventsList( 0 ),
	m_eType(EntType_Unknown),
	m_pobLookAtInfo( 0 ),
	m_bLevelActiveConstruction(false)
{
	ATTACH_LUA_INTERFACE(CEntity);
}

/***************************************************************************************************
*
*	FUNCTION		CEntity::LoadClumpHeader
*
*	DESCRIPTION		Unified place for loading the clump header
*
***************************************************************************************************/
void CEntity::LoadClumpHeader( const char* pClumpName )
{
	if (g_bAllowMissingData)
	{
		if ( CClumpLoader::Get().GetClumpFromCache_Neutral( pClumpName ) == false)
		{
#ifndef _RELEASE
			// this clump must be missing from the arm file
			ntPrintf( Debug::DCU_CLUMP, "*****************************************************************\n" );
			ntPrintf( Debug::DCU_CLUMP, "* Area %d: WARNING! missing clump %s.\n", AreaResourceDB::Get().DebugGetLastAreaLoaded(), pClumpName);
			ntPrintf( Debug::DCU_CLUMP, "* REGENERATE ARM!\n" );
			ntPrintf( Debug::DCU_CLUMP, "*****************************************************************\n" );

			ntPrintf( Debug::DCU_RESOURCES, "*****************************************************************\n" );
			ntPrintf( Debug::DCU_RESOURCES, "* Area %d: WARNING! missing clump %s.\n", AreaResourceDB::Get().DebugGetLastAreaLoaded(), pClumpName);
			ntPrintf( Debug::DCU_RESOURCES, "* REGENERATE ARM!\n" );
			ntPrintf( Debug::DCU_RESOURCES, "*****************************************************************\n" );
#endif
			// add entry to resource data base
			//---------------------------------------------------
			uint32_t areaMask = m_iMappedAreaInfo;
			if (m_iMappedAreaInfo == 0)
			{
				if (AreaManager::Get().LevelActive())
					areaMask = 1 << ( AreaResourceDB::Get().DebugGetLastAreaLoaded()-1 );
				else
					areaMask = 0xffffffff;
			}
			uint32_t resID = AreaResourceDB::Get().AddAreaResource( pClumpName, AreaResource::CLUMP, areaMask );
			
			AreaResource* pResource = AreaResourceDB::Get().GetEntry( resID );
			ntAssert( pResource );

			pResource->Request( AreaResource::CLUMP_LOAD_HEADER );
	
			if (AreaManager::Get().LevelActive() && pResource->GetStatus() != AreaResource::LOADED)
				pResource->Request( AreaResource::LOAD_SYNC );
		}
	}

	user_error_p( CClumpLoader::Get().GetClumpFromCache_Neutral( pClumpName ), ("Area %d: WARNING! missing clump %s. REGENERATE ARM!\n", AreaResourceDB::Get().DebugGetLastAreaLoaded(), pClumpName) );
	m_pobClumpHeader = CClumpLoader::Get().LoadClump_Neutral( pClumpName, false );
}

/***************************************************************************************************
*
*	FUNCTION		CEntity::ReleaseClumpHeader
*
*	DESCRIPTION		Unified place for unloading the clump header
*
***************************************************************************************************/
void CEntity::ReleaseClumpHeader()
{
	CClumpLoader::Get().UnloadClump( m_pobClumpHeader );
}

/***************************************************************************************************
*
*	FUNCTION		CEntity::FixedUpAreaResources
*
*	DESCRIPTION		Called after an areas resources have been loaded by the area system, before
*					we make our entity visible
*
***************************************************************************************************/
void CEntity::FixUpAreaResources()
{
	if ((m_pobClumpHeader == 0) || IsAreaResFixedUp())
		return;

	// as our clump and texture data must be loaded here for this to work, this is a good
	// place to validate that they are before we start to render with them

#	ifdef CLUMP_USE_DEBUG_TAG
	{
		ntError_p( m_pobClumpHeader->m_pAdditionalData->m_bHasVRAMResources,
			("Our VRAM resources for clump (%s) in sectors (0x%x) are not created yet! Likely ARM files need regenerating",
			m_pobClumpHeader->m_pAdditionalData->m_pDebugTag,
			this->m_iMappedAreaInfo) );

		ntError_p( m_pobClumpHeader->m_pAdditionalData->m_bHasTexResources,
			("Our VRAM resources for clump (%s) in sectors (0x%x) are not created yet! Likely ARM files need regenerating",
			m_pobClumpHeader->m_pAdditionalData->m_pDebugTag,
			this->m_iMappedAreaInfo) );
	}
#	else
	{
		ntError_p( m_pobClumpHeader->m_pAdditionalData->m_bHasVRAMResources,
			("Our VRAM resources for a clump in sectors (0x%x) are not created yet! Likely ARM files need regenerating",
			this->m_iMappedAreaInfo) );

		ntError_p( m_pobClumpHeader->m_pAdditionalData->m_bHasTexResources,
			("Our VRAM resources for a clump in sectors (0x%x) are not created yet! Likely ARM files need regenerating",
			this->m_iMappedAreaInfo) );
	}
#	endif

	ntError_p( m_pobRenderableComponent, ("Must have a renderable component already") );

	m_pobRenderableComponent->CreateAreaResources();

	SetAreaResFixedUp(true);
}

/***************************************************************************************************
*
*	FUNCTION		CEntity::ReleaseAreaResources
*
*	DESCRIPTION		Called before an areas resources have been unloaded by the area system, before
*					we make our entity invisible
*
***************************************************************************************************/
void CEntity::ReleaseAreaResources()
{
	if ((m_pobClumpHeader == 0) || !IsAreaResFixedUp())
		return;

	// delete our gfx components.
	ntAssert_p( m_pobRenderableComponent, ("Must have a renderable component already") );
	m_pobRenderableComponent->ReleaseAreaResources();

	SetAreaResFixedUp(false);
}

//------------------------------------------------------------------------------------------
//!  public virtual  CanRemoveFromWorld
//!
//!  @return bool	Just returns true
//!
//!  @author GavB @date 19/09/2006
//------------------------------------------------------------------------------------------
bool CEntity::CanRemoveFromWorld(void)
{
	return true;
}

//------------------------------------------------------------------------------------------
//!  public virtual  RemoveFromWorld
//!
//!  @return bool 
//!
//!  @remarks: 
//!				Ask the entity to be removed from the world - in the case where the 
//!				entity is spawned the process will be instant else the process will
//!				happen during the next update of the entity manager. 
//!				If the entity is removed or will be removed then the return is true,
//!				else the entity wont be removed just yet. 
//!  @author GavB @date 19/09/2006
//------------------------------------------------------------------------------------------
bool CEntity::RemoveFromWorld(bool bSafeRemove) 
{
		// If told not to destroy/deactivate the entity now
	if( bSafeRemove )
	{
		// To be used during the game, to flag an entity for destruction (use with caution!!)
		SetToDestroy(true);

		// an entity might be registered with the AI's dynamic avoidance system, so try and remove from that
		CAINavGraphManager::Get().GetRepulsion()->RemoveEntity(this);

		return true;
	}

	ntError( GetMessageHandler() == 0 || GetMessageHandler()->Processing() == false );

	// Destroy any projectiles that may be parented to this object
	if( GetPhysicsSystem() )
		GetPhysicsSystem()->Lua_RemoveChildEntities(); 

	// Interacting with something?
	if( GetInteractionComponent() )
		GetInteractionComponent()->SetInteractionType(NONE);

	// deactivate the physics system for the entity
	if (GetPhysicsSystem())
		GetPhysicsSystem()->Deactivate();

	// Clear out any message queues
	if( GetMessageHandler() )
		GetMessageHandler()->ClearMessageQueues();

	// Create a safety check - I'm sure this will cause code will cause a problem at some point
	static bool gbSafetyCheck = false;

	// Check that the safety flag is set to false. 
	ntError( gbSafetyCheck == false );

	// fill the safety check bool
	gbSafetyCheck = true;

	// If the entity has a state machine, then process the exit transitions all the way
	// up to the top state...
	if( GetFSM() )
		GetFSM()->SetState(0);

	// clear the safety check code.
	gbSafetyCheck = false;

	return true;
}

/***************************************************************************************************
*
*	FUNCTION		CEntity::InstallGfxComponents
*
*	DESCRIPTION		Unified place to create graphics components
*
***************************************************************************************************/
void	CEntity::InstallGfxComponents( const char* pClumpName )
{
	const CLODComponentDef* pLodDef = CLODManager::Get().GetLODPreset( pClumpName );

	ntAssert_p( m_pobRenderableComponent == 0, ("Already have a renderable component") );
	ntAssert_p( m_pobHierarchy != 0, ("MUST have a heirachy already here") );

	m_pobRenderableComponent = NT_NEW_CHUNK(Mem::MC_ENTITY) CRenderableComponent( m_pobHierarchy, !m_bDisableRender, m_bRecieveShadows, m_bCastShadows, false );

	if (pLodDef)
	{
		ntAssert_p( m_pobLODComponent == 0, ("Already have a LOD component") );
		m_pobLODComponent = NT_NEW_CHUNK(Mem::MC_ENTITY) CLODComponent(m_pobRenderableComponent,pLodDef);
	}
}

/***************************************************************************************************
*
*	FUNCTION		CEntity::ReleaseAreaResources
*
*	DESCRIPTION		Called before an areas resources have been unloaded by the area system, before
*					we make our entity invisible
*
***************************************************************************************************/
void CEntity::DeleteGfxComponents()
{
	if ( m_pobRenderableComponent )
	{
		NT_DELETE_CHUNK(Mem::MC_ENTITY, m_pobRenderableComponent);
		m_pobRenderableComponent = 0;
	}

	if ( m_pobLODComponent )
	{
		NT_DELETE_CHUNK(Mem::MC_ENTITY, m_pobLODComponent);
		m_pobLODComponent = 0;
	}
}

/***************************************************************************************************
*
*	FUNCTION		CEntity::OnPostConstruct
*
*	DESCRIPTION		Called after serialisation is complete
*
***************************************************************************************************/
void CEntity::OnPostConstruct()
{
	// initialise our area info object
	m_areaInfo = AreaInfo(m_iMappedAreaInfo);

	// Pass the GetName function (as a functor object) to the EntityAnimSet base.
	// The ownership issue of this functor is a bit messy - created here, but destroyed by EntityAnimSet. :/
	EntityAnimSet::InstallGetName( NT_NEW SpecificFunctor< CEntity, ntstd::String, true >( this, &CEntity::GetName ) );

	// if we don't have an attribute table construct one from the XML
	// TODO ForceLuaTable stuff
	if( m_obAttributeTable == 0 )
	{
		DataObject* pDO = ObjectDatabase::Get().GetDataObjectFromPointer(this);

		ntAssert_p( pDO, ("Entity is not registered with object database?") );

		m_obAttributeTable = CLuaGlobal::Get().TableFromBaseObject( pDO );
	}

	// Save the name of the entity, this is so we can use it as a key generator for our
	// hashing inside the QuickPtrList that stores us. Can also look at this in the debugger!
	m_Name = GetName().c_str();

	// Check to see if an entity with same name already exists
	user_error_p( !CEntityManager::Get().FindEntity( m_Name ), ("Entity of name %s already exists", GetName().c_str() ) );

	// Add to the entity manager
	CEntityManager::Get().Add( this );

	// Initialise the clump, hierarchy and renderable components
	if(!m_Clump.empty() && m_Clump != "NULL")
	{
		// Helpful debugging for when the level is slow (used with the comment in renderablecomponent.cpp
		//ntPrintf( "EntityName: %s Clump Name: %s\n", GetName().c_str(), m_Clump.c_str() );

		LoadClumpHeader(m_Clump.c_str());

		InstallHierarchy();
		InstallGfxComponents(m_Clump.c_str());

		// Calculate the clump file hash
		const char* pcClumpString=m_Clump.c_str(); // Pointer to the clump file path
		int len=m_Clump.length() - 6; // Length of the path minus the .clump extension
		int offset=len;
		
		while(offset>0) // Start scanning the path starting from the end for the path seperator
		{
			if (pcClumpString[offset]=='/' || pcClumpString[offset]=='\\') // Found it
				break;

			offset--;
		}

		offset++;
		len-=offset; // Now we can calculate how long the clump file name is (minus the extension)

		char acTemp [64];
		int index=0;

		while(index<len)
		{
			char c=pcClumpString[offset+index]; // Get the next character

			if (c >= 'A' && c<= 'Z') // This character needs to be converted to lowercase
				c += 'a' - 'A';

			acTemp[index]=c; // Copy to temp array

			index++;
		}
				
		acTemp[index]='\0'; // Stick a null terminator on the end

		m_uiClumpFileHash=CHashedString(acTemp).GetHash(); // Get the hash value for this string
		
		//ntPrintf("Clump file string=%s  Hash=%d\n",acTemp,m_uiClumpFileHash);
	}

	// Set up our keywords from the entity description - originally carried out in the CEntityInfo
	const ntstd::String& obDescription = m_obAttributeTable->GetString( "Description" );
	if ( obDescription.length() )
		m_obKeywords.Set( obDescription.c_str() );

	SetPosition( m_InitialPosition );
	SetRotation( m_InitialRotation );

	// Add an interaction component
	m_pobInteractionComponent = NT_NEW_CHUNK(Mem::MC_ENTITY) CInteractionComponent(this);
	
	// Add to area system
	// NOTE! must be done after component creation, as this will set visibility and
	// activity status if ent created when level is running.
	AreaManager::Get().AddEntity( this );
	if(AreaManager::Get().LevelActive())
	{
		m_bLevelActiveConstruction = true;
	}

	// Make sure that the parent entity data has been set correctly - this call should be removed
	// when we have the ability to macro up lua accessors to pointers - i.e. PUBLISH_PTR_ACCESSOR
	CEntity* pobTemp = m_pobParentEntity;
	m_pobParentEntity = 0;
	SetParentEntity( pobTemp );

	SetPostConstructed(true);
}

void CEntity::OnPostPostConstruct()
{
	// Do any hierarchy reparenting if necessary

	if( m_pobHierarchy && m_pobParentEntity )
	{
		ntAssert_p( m_pobParentEntity->GetHierarchy(), ("Parent entity %s does not have a hierarchy/n",m_pobParentEntity->GetName().c_str()) );
			
		const char* obParentTransformName = m_obAttributeTable->GetAttribute( "ParentTransform" ).ToString();

		CHashedString obHashedParentTransformName(obParentTransformName);

		Transform* pobParentTransform = m_pobParentEntity->GetHierarchy()->GetTransform(obHashedParentTransformName);

		ntAssert_p( pobParentTransform, ("Transform %s on parent entity %s not found/n",obParentTransformName,m_pobParentEntity->GetName().c_str()) );

		GetHierarchy()->GetRootTransform()->RemoveFromParent();

		pobParentTransform->AddChild(GetHierarchy()->GetRootTransform());

		SetPosition( m_InitialPosition );
		SetRotation( m_InitialRotation );
	}

	// ----- Call entity lua OnConstruct if defined -----

	if(!ntStr::IsNull(m_ConstructionScript) && (m_eType != EntType_Unknown && m_eType != EntType_Player))
	{
		user_warn_msg(("Construction Script on Hard Objects is no longer supported, %s\n", GetName().c_str()));
	}
	else if(!ntStr::IsNull(m_ConstructionScript))
	{
		// call Lua script to construct ent
		NinjaLua::LuaObject ctor = CLuaGlobal::Get().State().GetGlobals().Get<NinjaLua::LuaObject>(m_ConstructionScript);
		//NinjaLua::LuaObject obCtor = CLuaGlobal::Get().GetState().GetGlobal( constScript.GetString() );

		CLuaGlobal::Get().SetMessage( 0 );
		CLuaGlobal::Get().SetTarg( this );

		// Construct field can either refer to a global lua function or a table (where it assumes there is a function in that table called OnConstruct)
		if(ctor.IsFunction())
		{
			NinjaLua::LuaFunction func(ctor);
			func(GetAttrib());
		}
		else if(ctor.IsTable()) // If the ConstructionScript field does not correspond to a lua function, then check to see if it corresponds to a lua table containing a function called OnConstruct
		{
			NinjaLua::LuaObject obConstructFunc = ctor.Get<NinjaLua::LuaObject>("OnConstruct");

			if(obConstructFunc.IsFunction())
			{
				NinjaLua::LuaFunction func(obConstructFunc);
				func(GetAttrib());
			}
			else
			{
				user_error_p(0, ("Entity construction failed - '%s.OnConstruct' is not a valid function", ntStr::GetString(m_ConstructionScript)));
			}
		}
		else
		{
			user_error_p(0, ("Entity construction failed - '%s' is not a valid global function", ntStr::GetString(m_ConstructionScript)));
		}

		CLuaGlobal::Get().SetTarg( 0 );
	}

	// Moved out of AreaSystem - JML
	if(m_bLevelActiveConstruction)
	{
		OnLevelStart();
	}
}




/***************************************************************************************************
*
*	FUNCTION		CEntity::AddChildEntity
*
*	DESCRIPTION		Maintain a list of pointers to our child entities
*
***************************************************************************************************/
void CEntity::AddChildEntity( CEntity* pobChild )
{
	// We can't do anything with an empty pointer
	if ( !pobChild )
	{
		ntAssert_p( 0, ( "Request to add a NULL pointer to an entity child list.\n" ) );
		return;
	}

#ifdef _DEBUG

	// Make sure that we don't already have this child in our list - that would be bad
	ntstd::List<CEntity*>::iterator obEndIt = m_obChildEntities.end();
	for( ntstd::List<CEntity*>::iterator obIt = m_obChildEntities.begin(); obIt != obEndIt; ++obIt )
	{
		// If we find it warn some people
		if ( ( *obIt ) == pobChild )
		{
			ntAssert_p( 0, ( "Request to add a child to an entity child list that is already present.\n" ) );
			return;
		}
	}

#endif

	// Add the new child to our list
	m_obChildEntities.push_back( pobChild );

}


/***************************************************************************************************
*
*	FUNCTION		CEntity::RemoveChildEntity
*
*	DESCRIPTION		Maintain a list of pointers to our child entities
*
***************************************************************************************************/
void CEntity::RemoveChildEntity( CEntity* pobChild )
{
	// We can't do anything with an empty pointer
	if ( !pobChild )
	{
		ntAssert_p( 0, ( "Request to remove a NULL pointer from an entity child list.\n" ) );
		return;
	}

	// Run through our list of children and find the offender
	ntstd::List<CEntity*>::iterator obEndIt = m_obChildEntities.end();
	for( ntstd::List<CEntity*>::iterator obIt = m_obChildEntities.begin(); obIt != obEndIt; ++obIt )
	{
		// If we find it, remove it and get out of here
		if ( ( *obIt ) == pobChild )
		{
			m_obChildEntities.erase( obIt );
			return;
		}
	}

	// If we are here then something has gone wrong
	ntAssert_p( 0, ( "We have tried to remove a child entity that wasn't there.\n" ) );
}



/***************************************************************************************************
*
*	FUNCTION		CEntity::SetParentEntity
*
*	DESCRIPTION		Re-parent ourselves, including re-sectoring work
*
***************************************************************************************************/
void CEntity::SetParentEntity( CEntity* pobParentEntity )
{
	// If we are already set then drop out
	if ( pobParentEntity == m_pobParentEntity )
		return;

	// If we already have a parent entity remove ourselves as children
	if ( m_pobParentEntity )
		m_pobParentEntity->RemoveChildEntity( this );

	// Store the new given parent
	m_pobParentEntity = pobParentEntity;

	// If the parent entity has change we need to change our area settings
	if ( m_pobParentEntity )
		AreaManager::Get().SetToEntityAreas( m_pobParentEntity, this );

	// As we have a new parent - they have a new child
	if ( m_pobParentEntity )
		m_pobParentEntity->AddChildEntity( this );
}


/***************************************************************************************************
*
*	FUNCTION		CEntity::SetClumpString
*
*	DESCRIPTION		Part of the system to allow clump to be changed in welder
*
***************************************************************************************************/
void CEntity::SetClumpString( const ntstd::String& clump)
{
	if( IsPostConstructed() )
	{
		CPoint curPosition = GetPosition();
		CQuat curOrientation = GetRotation();
		// destroy any existing object (this won't work if the script has installed any components that
		// have snarfed a look a hierachy, this needs fixing for armys anyway)
		if(m_pobClumpHeader)
		{
			DeleteGfxComponents();

			if ( m_pobHierarchy->GetRootTransform()->GetParent() )
				m_pobHierarchy->GetRootTransform()->RemoveFromParent();
			
			CHierarchy::Destroy( m_pobHierarchy ); m_pobHierarchy = 0;

			ReleaseClumpHeader();
		}

		// load the new header
		LoadClumpHeader( ntStr::GetString(clump) );
		InstallHierarchy();
		InstallGfxComponents( ntStr::GetString(clump) );

		SetPosition( curPosition );
		SetRotation( curOrientation );
	}

	m_Clump = clump;
}


/***************************************************************************************************
*
*	FUNCTION		CEntity::~CEntity
*
*	DESCRIPTION		Destructor for an entity.
*
***************************************************************************************************/
CEntity::~CEntity()
{
	ntPrintf("Destructing %s\n", ntStr::GetString(m_Name));
	// call Lua script to destruct stuff Is optional
	if( !ntStr::IsNull(m_DestructionScript))
	{	
		NinjaLua::LuaObject obDtor = CLuaGlobal::Get().State().GetGlobals()[ m_DestructionScript ];
		ntError_p( !obDtor.IsNil(), ("Entity DestructionScript %s not found", ntStr::GetString(m_DestructionScript) ) );
		CLuaGlobal::Get().SetMessage( 0 );
		CLuaGlobal::Get().SetTarg( this );

		NinjaLua::LuaFunction obFunc = NinjaLua::LuaFunction(obDtor);
		obFunc(	m_obAttributeTable->GetLuaObjectWrapper() );
		CLuaGlobal::Get().SetTarg( 0 );
	}

	// If there is an interaction component, then we get it to remove any use point transforms
	// from the hierarchy, before the hierarchy and animation system are destructed
	if ( m_pobInteractionComponent )
	{
		m_pobInteractionComponent->RemoveUsePointPfxData();
	}

	NT_DELETE(m_pobOneChain);
	m_pobOneChain = 0;

	NT_DELETE_CHUNK(Mem::MC_ENTITY, m_pobLookAtComponent );
	m_pobLookAtComponent = 0;

	// Destroy any Scene element component we might have.
	NT_DELETE_CHUNK(Mem::MC_ENTITY, m_pSceneElementComponent );
	m_pSceneElementComponent = 0;

    // Delete the formation component
	NT_DELETE_CHUNK(Mem::MC_ENTITY, m_pobFormationComponent );
    m_pobFormationComponent = 0;

	if(m_pobAimingComponent)
	{
		NT_DELETE_CHUNK(Mem::MC_ENTITY, m_pobAimingComponent);
	}

	// If we have an FSM, destroy it
	if (m_pFSM)
	{
		NT_DELETE_CHUNK(Mem::MC_ENTITY, m_pFSM);
	}

	// If we have a message handler component, destroy it
	if ( m_pobMessageHandler )
	{
		NT_DELETE_CHUNK(Mem::MC_ENTITY, m_pobMessageHandler );
		m_pobMessageHandler = 0;
	}

	// If we have a movement component, destroy it
	if ( m_pobMovement )
	{
		NT_DELETE_CHUNK(Mem::MC_ENTITY, m_pobMovement );
		m_pobMovement = 0;
	}

	// If we have an awareness component destroy it
	if ( m_pobAwarenessComponent )
	{
		NT_DELETE_CHUNK(Mem::MC_ENTITY, m_pobAwarenessComponent );
		m_pobAwarenessComponent = 0;
	}

	// If we have an attack component, destroy it
	if ( m_pobAttackComponent )
	{
		NT_DELETE_CHUNK(Mem::MC_ENTITY, m_pobAttackComponent );
		m_pobAttackComponent = 0;
	}

	// If we have a dynamics component destroy it (after the combat, which needs to kill some physics stuff)
	if ( m_pobPhysicsSystem )
	{
		NT_DELETE_CHUNK(Mem::MC_ENTITY, m_pobPhysicsSystem );
		m_pobPhysicsSystem = 0;
	}

	UninstallBlendShapesComponent();

	// if we have and GFX components still installed, delete them
	DeleteGfxComponents();

	// If we have an animator component, destroy it..
	if ( m_pobAnimator )
	{
		CAnimator::Destroy( m_pobAnimator );
		m_pobAnimator = 0;
	}

	// If we have a hierarchy component, remove from any parent and then destroy..
	if ( m_pobHierarchy )
	{
		if ( m_pobHierarchy->GetRootTransform()->GetParent() )
			m_pobHierarchy->GetRootTransform()->RemoveFromParent();
		
		CHierarchy::Destroy( m_pobHierarchy );
		m_pobHierarchy = 0;
	}

	// if we have an info block, free it
	if ( m_pobEntityAudioChannel )
	{
		NT_DELETE_CHUNK(Mem::MC_ENTITY, m_pobEntityAudioChannel );
		m_pobEntityAudioChannel = 0;
	}

	if ( m_pobLODComponent )
	{
		NT_DELETE_CHUNK(Mem::MC_ENTITY, m_pobLODComponent );
		m_pobLODComponent = 0;
	}

	if ( m_pobInteractionComponent )
	{
		NT_DELETE_CHUNK(Mem::MC_ENTITY, m_pobInteractionComponent );
		m_pobInteractionComponent = 0;
	}

	if(m_pobClumpHeader)
	{
		ReleaseClumpHeader();
	}

	if (!IsToDestroy()) // If this flag is set, it means we want to remove the entity from the manager manually
	{
		// Remove from the entity manager
		CEntityManager::Get().Remove( this );

		// Remove from the area system
		AreaManager::Get().RemoveEntity( this );
	}

	// Disassociate this entity from any child entities - this needs to be performed before we do RemoveChildEntity!
	// HC: I've added this to prevent situations whereby an entity still has m_pobParentEntity pointer to an entity which was already deleted
	// Bug fix 05/01/06: Set parent entity pointer to 0 directly instead of using SetParentEntity(0), since the old way removes the child entity from the list anyway!
	while(m_obChildEntities.size()>0)
	{
		if (m_obChildEntities.back()->m_pobParentEntity == this)
		{
			m_obChildEntities.back()->m_pobParentEntity = 0;
		}

		m_obChildEntities.pop_back();
	}

	// If we have a parent entity remove ourselves as children
	if ( m_pobParentEntity )
	{
		m_pobParentEntity->RemoveChildEntity( this );
	}
	ntPrintf("Done %s\n", ntStr::GetString(m_Name));
}


/***************************************************************************************************
*
*	FUNCTION		CEntity::DoThinkBehaviour
*
*	DESCRIPTION		
*
***************************************************************************************************/
void CEntity::DoThinkBehaviour ()
{
#ifdef CHECK_LUAPLUS_GC
	CWilDebug::CloseSystem();
#endif

	NinjaLua::LuaObject think = m_obAttributeTable->GetAttribute( "think" );

	// Make sure think is actually a LUA function!
	if (think.IsNil())
		return;

	ntError( false && "Is there still a need for this thinks? if you get this please see gav b" );
	NinjaLua::LuaObject nextthink = m_obAttributeTable->GetAttribute( "nextthink" );

	ntAssert( think.IsFunction() );
	ntAssert( nextthink.IsNumber() );

	float fNextThink = nextthink.GetFloat();

	if (fNextThink>0.0f) // Decrement the nextthink counter
	{
		fNextThink-=GetLastTimeChange();

		if (fNextThink>0.0f)
		{
			m_obAttributeTable->SetNumber( "nextthink", fNextThink );
		}
	}
		
	if (fNextThink<0.0f) // It's time to call our think function
	{
		// Clear the nextthink, that way we enforce it having to be set in each update
		m_obAttributeTable->SetNumber( "nextthink", 0.0f );
		// Execute the function assigned to the think property
		NinjaLua::LuaFunction obFunc( think );
		CLuaGlobal::Get().SetTarg( this );
		obFunc();
		CLuaGlobal::Get().SetTarg( NULL );
	}
}



/***************************************************************************************************
*
*	FUNCTION		CEntity::InstallHierarchy
*
*	DESCRIPTION		
*
***************************************************************************************************/
void CEntity::InstallHierarchy()
{
	ntAssert( m_pobHierarchy == 0 );	// already got one?

	// We don't actually need any XML instance data if creating fully via a lua table
	if( m_pobClumpHeader )
	{
		m_pobHierarchy = CHierarchy::Create(m_pobClumpHeader);
		CHierarchy::GetWorld()->GetRootTransform()->AddChild( GetHierarchy()->GetRootTransform() );
		GetHierarchy()->GetRootTransform()->SetLocalMatrix(CVecMath::GetIdentity());
	}
}



/***************************************************************************************************
*
*	FUNCTION		CEntity::InstallAnimator
*
*	DESCRIPTION		
*
***************************************************************************************************/
void CEntity::InstallAnimator(const CHashedString& AnimContainerName)
{
	ntAssert( m_pobAnimator==0 );	// already got one?
	ntAssert( m_pobHierarchy );	// animator requires a hierarchy.
	m_pobAnimator = CAnimator::Create( static_cast< EntityAnimSet * >( this ), m_pobHierarchy, this );

	// Pass on the animator to the EntityAnimSet.
	EntityAnimSet::InstallAnimator( m_pobAnimator, AnimContainerName );
}


/***************************************************************************************************
*
*	FUNCTION		CEntity::InstallBlendShapesComponent
*
*	DESCRIPTION		
*
*	NOTE	Rendereables MUST be installed first
*			
*
***************************************************************************************************/
void CEntity::InstallBlendShapesComponent( const char* bsclumpFileName )
{
	ntError_p( m_pobRenderableComponent, ("RenderableComponent should be present before installing BlendShapeComponent") );
	ntError_p( !m_pobBlendShapesComponent, ("BlendShapesComponent already present") );

	if ( XPUShapeBlending::Get().IsEnabled() )
	{
		//! create new component
        m_pobBlendShapesComponent = NT_NEW_CHUNK(Mem::MC_ENTITY) BlendShapesComponent( this );
		//! add bsclump
		if ( bsclumpFileName )
		{
			m_pobBlendShapesComponent->PushBSSet( bsclumpFileName );
		}
	}
}


void CEntity::UninstallBlendShapesComponent( )
{
	if ( m_pobBlendShapesComponent )
	{
		NT_DELETE_CHUNK( Mem::MC_ENTITY, m_pobBlendShapesComponent );
		m_pobBlendShapesComponent = 0;
	}
}



bool CEntity::IsBlendShapesCapable( void ) const
{
	ntAssert( GetRenderableComponent() );

	if ( GetRenderableComponent() )
	{
		// [scee_st]: CRemderableComponent now has this typedef for the list types anyway (for chunking)
		const CRenderableComponent::MeshInstanceList& rMeshes  = GetRenderableComponent()->GetMeshInstances();
		for ( CRenderableComponent::MeshInstanceList::const_iterator it = rMeshes.begin(); it != rMeshes.end(); it++ )
		{
			if ( (*it)->IsShapeBlended() )
			{
				return true;
			}	
		}
	}

	return false;
}


/***************************************************************************************************
*
*	FUNCTION		CEntity::InstallDynamics
*
*	DESCRIPTION		
*
***************************************************************************************************/
void CEntity::InstallDynamics()
{
	ntAssert( m_pobHierarchy );		// requires a hierarchy.

	/*// If I have an xml file defined, ignore the dynamics component
	if(( m_PhysicsSystem != "" ) && ( !m_PhysicsSystem.IsNull() ))
	{
		m_pobPhysicsSystem = NT_NEW Physics::System( this, GetName() );

		static char acFileName[ MAX_PATH ];
		Util::GetFiosFilePath( *m_PhysicsSystem, acFileName );

		m_pobPhysicsSystem->ReadFromXML( acFileName );

	} else {*/
	
		// If not, try to construct the phsyics system from the clump...
		m_pobPhysicsSystem = Physics::System::ConstructSystemFromClump( this );

		//... If not System is constructed, the clump is either empty, or the DynamicsState is not supported.
	//}	

}




//-----------------------------------------------------------------------------------------------
//!
//! CEntity::InstallLookAtComponent
//!	
//-----------------------------------------------------------------------------------------------
void CEntity::InstallLookAtComponent( CHashedString obCompName )
{
	LookAtComponentDef* pobDef = ObjectDatabase::Get().GetPointerFromName<LookAtComponentDef*>( obCompName );
	if ( pobDef )
	{
		m_pobLookAtComponent = NT_NEW_CHUNK(Mem::MC_ENTITY) LookAtComponent( this, pobDef );
	}
}

//-----------------------------------------------------------------------------------------------
//!
//! Gets the location of an item.  This is a softer representation of where the entity is, if it
//!	has a heirarchy.  Ideally to be used by items that want a fairly central position in the 
//! entities space.
//!
//-----------------------------------------------------------------------------------------------
CPoint CEntity::GetLocation( void ) const
{
	// If we have a heirarchy...
	if ( GetHierarchy() )
	{
		// If the root transform has a single child then use that position
		if ( GetHierarchy()->GetRootTransform()->GetFirstChild() && !GetHierarchy()->GetRootTransform()->GetFirstChild()->GetNextSibling() )
			return GetHierarchy()->GetRootTransform()->GetFirstChild()->GetWorldMatrix().GetTranslation();
		
		//...otherwise if we have no, or multiple, child transforms - return the root position
		else
			return GetHierarchy()->GetRootTransform()->GetWorldMatrix().GetTranslation();
	}
	
	// If there is no heirarchy we get the initial position
	else
		return m_InitialPosition;
}

//--------------------------------------------------
//!
//! Sets the root node to the world position provided.
//! Should update all components (Dynamics etc.) to use
//! this new position correctly
//!
//-------------------------------------------------
void CEntity::SetPosition(const CPoint &obPoint)
{
	if( GetHierarchy() )
	{	
		CMatrix obMat = GetHierarchy()->GetRootTransform()->GetWorldMatrix();
		obMat.SetTranslation(obPoint);
		GetHierarchy()->GetRootTransform()->SetLocalMatrixFromWorldMatrix(obMat);

		// !!!!!!!!!! Nasty fudge !!!!!!!!!!!!!!
		// With recent changes to the re-ordering of the entity update system
		// it's possible for the CMovementState to get out of sync with the real position
		// of the entity causing problems when warping entities. Gav
		if( GetMovement() )
		{
			CMovementStateRef& robMovementState = const_cast<CMovementStateRef&>(GetMovement()->GetCurrentMovementState());
			robMovementState.m_obPosition = obPoint;
		}

		// this should fix the dynamics and movement issues, noted by JML
		if( GetPhysicsSystem() )
		{
			GetPhysicsSystem()->EntityRootTransformHasChanged();
		}
	} else
	{
		m_InitialPosition = obPoint;
	}
}

//--------------------------------------------------
//!
//! Returns the worls space rotation
//!
//-------------------------------------------------
CQuat CEntity::GetRotation( void ) const
{
	if( GetHierarchy() )
	{
		return CQuat( GetHierarchy()->GetRootTransform()->GetWorldMatrix() );
	} else
	{
		return m_InitialRotation;
	}
}

//--------------------------------------------------
//!
//! Sets the world orientation of an entity. 
//! all components should update and start using the new orientation
//!
//-------------------------------------------------
void CEntity::SetRotation(const CQuat& obRot)
{
	if( GetHierarchy() )
	{
		GetHierarchy()->GetRootTransform()->SetLocalMatrixFromWorldMatrix(CMatrix(obRot, GetHierarchy()->GetRootTransform()->GetWorldMatrix().GetTranslation()));

		// !!!!!!!!!! Nasty fudge !!!!!!!!!!!!!!
		// With recent changes to the re-ordering of the entity update system
		// it's possible for the CMovementState to get out of sync with the real position
		// of the entity causing problems when warping entities. Gav
		if( GetMovement() )
		{
			CMovementStateRef& robMovementState = const_cast<CMovementStateRef&>(GetMovement()->GetCurrentMovementState());
			robMovementState.m_obOrientation = obRot;

			// Kludge.
			CMatrix mat(obRot, CPoint(CONSTRUCT_CLEAR));

			robMovementState.m_obFacing = mat.GetZAxis();
		}

		if( GetPhysicsSystem() )
		{
			GetPhysicsSystem()->EntityRootTransformHasChanged();
		}

	} else
	{
		m_InitialRotation = obRot;
	}
}





//--------------------------------------------------
//!
//! Pauses or Unpauses this entity
//!
//-------------------------------------------------
void CEntity::Pause( bool bPause, bool bFullPhysicsPause )
{
	if ( bPause == IsPaused() && (!bFullPhysicsPause || !GetPhysicsSystem()))		
		return;	

	if( GetPhysicsSystem() )
	{
		if(!bFullPhysicsPause)
		{
			GetPhysicsSystem()->Pause( bPause );
		}
		else
		{
			if(bPause)
			{
				GetPhysicsSystem()->Deactivate();
			}
			else
			{
				GetPhysicsSystem()->Activate();
			}
		}
	}

	SetPaused(bPause);
}

//--------------------------------------------------
//!
//! Update the state of the anim container (used to assist wielder)
//!
//-------------------------------------------------
void CEntity::RebuildAnimEventLists ()
{
#ifndef _RELEASE
	if (m_pobAnimator)
	{
		NinjaLua::LuaObject obObject=m_obAttributeTable->GetAttribute(CHashedString(HASH_STRING_ANIMATIONCONTAINER));
		
		if (obObject.IsString())
		{
			CHashedString sContainer = obObject.GetHashedString();
			EntityAnimSet::RebuildAnimEventLists(sContainer);
		}
	}
#endif
}


//-----------------------------------------------------------------------------
//!
//! CEntity::IsType
//! Is the entity of a type as described in the keywords field
//!
//----------------------------------------------------------------------------
bool CEntity::IsType(const char* pcType) const
{
	// If the entity has an entity info, use that, as it'll be faster
	return GetKeywords().Contains( pcType );
#if 0
	// Attempt to find the keyword description
	const char* pcDesc = m_obAttributeTable->GetString("Description").c_str();

	// If the string isn't valid, then return false. 
	if( !pcDesc )
		return false;

	// Build a keyword set and test for the type
	return CKeywords( pcDesc ).Contains( pcType );
#endif
}


//-----------------------------------------------------------------------------------------------
//!
//! CEntity::SetPhysicsSystem
//!
//-----------------------------------------------------------------------------------------------
void CEntity::SetPhysicsSystem( Physics::System* p_system )
{
	NT_DELETE_CHUNK(Mem::MC_ENTITY, m_pobPhysicsSystem );
	m_pobPhysicsSystem = p_system;
}


/***************************************************************************************************
*
*	FUNCTION		CEntityInfo::RemoveKeyword
*
*	DESCRIPTION		Change Description of a single item
*
***************************************************************************************************/
void CEntity::RemoveKeyword(const char* pcRemove)
	{
	m_obKeywords.RemoveKeyword(pcRemove);
	}

/***************************************************************************************************
*
*	FUNCTION		CEntityInfo::AddKeyword
*
*	DESCRIPTION		Change Description of a single item
*
***************************************************************************************************/
void CEntity::AddKeyword(const char* pcAdd)
{
	m_obKeywords.AddKeyword(pcAdd);
}

//-----------------------------------------------------------------------------------------------
//!
//! CEntity::IsLockonable
//!	
//-----------------------------------------------------------------------------------------------
bool CEntity::IsLockonable( void ) const
{
	if (IsCharacter())
	{
		const Character* pobChar = ToCharacter();
		// An entity can also be lockonable if they are dead in terms of health but still dying in their combat state, just using AI_Access stuff as it's already there - DGF
		return ( (!pobChar->IsDead()) || (GetAttackComponent()->AI_Access_GetState() == CS_DYING) );
	}
	return false;
}


/***************************************************************************************************
*
*	FUNCTION		CEntity::Update
*
*	DESCRIPTION		
*
***************************************************************************************************/
void CEntity::UpdateVelocity( float fTimeStep )
{

	// Try and calculate the velocity of this entity - will be wrong on the first frame
	if ( m_fLastTimeChange > 0.0f )
		m_obCalculatedVel = ( m_obNewPos ^ m_obLastPos ) * ( 1.0f / m_fLastTimeChange );

	// Update the information required to calculate the velocity
	m_obLastPos = m_obNewPos;
	m_obNewPos = GetPosition();
	m_fLastTimeChange = fTimeStep;
}


/***************************************************************************************************
*
*	FUNCTION		CEntity::GetCalcVelocity
*
*	DESCRIPTION		This is derived from the position at the begining of THIS frame minus the same 
*					value from LAST frame divided by the time step from the LAST frame, i.e, this 
*					is one frame behind, and invalid for the first frame. If we need something 
*					better, we need an entity info update at the end of the game update rather than 
*					the begining.
*
***************************************************************************************************/
CDirection CEntity::GetCalcVelocity( void ) const
{
	return m_obCalculatedVel;
}


 


/***************************************************************************************************
*
*	FUNCTION		CEntity::GetLookDirection
*
*	DESCRIPTION		
*
***************************************************************************************************/
void CEntity::GetLookDirection( CDirection& obFacing ) const
{
	obFacing = GetMatrix().GetZAxis();
}


//-----------------------------------------------------------------------------
//!
//! CEntity::Hide
//! Hide this entity
//!
//----------------------------------------------------------------------------
void CEntity::Hide()
{
	// Hide Ourself
	if(m_pobRenderableComponent)
	{
		m_pobRenderableComponent->AddRemoveAll_Game(false);
	}

	// Hide our children
	for(ntstd::List<CEntity*>::iterator it = m_obChildEntities.begin(); it != m_obChildEntities.end(); it++)
	{
		(*it)->Hide();
	}
}


//-----------------------------------------------------------------------------
//!
//! CEntity::Show
//! Show this entity
//!
//----------------------------------------------------------------------------
void CEntity::Show()
{
	// Show ourself
	if(m_pobRenderableComponent)
	{
		m_pobRenderableComponent->AddRemoveAll_Game(true);
	}

	// Show our children
	for(ntstd::List<CEntity*>::iterator it = m_obChildEntities.begin(); it != m_obChildEntities.end(); it++)
	{
		(*it)->Show();
	}

}

//-----------------------------------------------------------------------------
//!
//! CEntity::IsHidden
//! return true if this entity has been hidden
//!
//----------------------------------------------------------------------------
bool CEntity::IsHidden()
{
	// Hide Ourself
	if(m_pobRenderableComponent)
	{
		return m_pobRenderableComponent->DisabledByGame();
	}
	return false;
}
