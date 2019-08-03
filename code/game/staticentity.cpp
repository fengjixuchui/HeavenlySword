/***************************************************************************************************
*
*	DESCRIPTION		Static (walls etc.) Entity
*
*	NOTES
*
***************************************************************************************************/
#include "Physics/config.h"

#include "objectdatabase/dataobject.h"
#include "game/luaglobal.h"
#include "game/luaattrtable.h"
#include "game/staticentity.h"
#include "game/entitymanager.h"
#include "gfx/clump.h"
#include "game/interactioncomponent.h"
#include "gfx/levelofdetail.h"
#include "area/areasystem.h"
#include "Physics/system.h"

START_STD_INTERFACE(Static)
	PUBLISH_ACCESSOR_WITH_DEFAULT( CPoint, Position, GetPosition, SetPosition, CPoint(0.0f, 0.0f, 0.0f) )
	PUBLISH_ACCESSOR_WITH_DEFAULT( CQuat, Orientation, GetRotation, SetRotation, CQuat(0.0f, 0.0f, 0.0f, 1.0f) )
	PUBLISH_ACCESSOR( ntstd::String, Clump, GetClumpString, SetClumpString )

	PUBLISH_VAR_WITH_DEFAULT_AS( m_bCastShadows, true, CastShadows )
	PUBLISH_VAR_WITH_DEFAULT_AS( m_bRecieveShadows, true, RecieveShadows )
	PUBLISH_VAR_WITH_DEFAULT_AS( m_bDisableRender, false, DisableRender )
	PUBLISH_VAR_WITH_DEFAULT_AS( m_bCollideOnlyWithCC, false, CollideOnlyWithEntities )
	PUBLISH_VAR_WITH_DEFAULT_AS( m_IsVaultable, false, IsVaultable )
	PUBLISH_VAR_WITH_DEFAULT_AS( m_ShouldVaultThroughCentre, false, ShouldVaultThroughCentre )
	PUBLISH_VAR_WITH_DEFAULT_AS( m_CanCrouchNextTo, false, CanCrouchNextTo )

	PUBLISH_VAR_WITH_DEFAULT_AS( m_DefaultDynamics, "Static", DefaultDynamics )

	PUBLISH_VAR_WITH_DEFAULT_AS( m_iMappedAreaInfo, 0, SectorBits )
	PUBLISH_VAR_WITH_DEFAULT_AS( m_replacesEnt, false, ReplacesEnt )
	PUBLISH_VAR_AS( m_entToReplace, SectorLODEnt )

	DECLARE_POSTCONSTRUCT_CALLBACK( OnPostConstruct )
END_STD_INTERFACE

Static::Static()
:	m_bCollideOnlyWithCC		( false )
,	m_IsVaultable				( false )
,	m_ShouldVaultThroughCentre	( false )
,	m_CanCrouchNextTo			( false )
{
	m_DefaultDynamics = "Static";
	m_eType = EntType_Static;
}
Static::~Static()
{
	NT_DELETE( m_obAttributeTable );
}

void Static::OnPostConstruct()
{
	// initialise our area info object
	m_areaInfo = AreaInfo(m_iMappedAreaInfo);

	// this is an non Lua attribute table!
	if( m_obAttributeTable == 0 )
	{
		m_obAttributeTable = NT_NEW LuaAttributeTable( LuaAttributeTable::NO_LUA_TABLE );
		DataObject* pDO = ObjectDatabase::Get().GetDataObjectFromPointer( this );
		ntAssert( pDO != 0 );
		m_obAttributeTable->SetDataObject( pDO );
	}
	
	// Save the name of the entity, this is so we can use it as a key generator for our
	// hashing inside the QuickPtrList that stores us. Can also look at this in the debugger!
	m_Name = GetName().c_str();

	// Add to the entity manager as a static
	CEntityManager::Get().Add( this );

	// Add to area system
	AreaManager::Get().AddEntity( this );

	// Use parents area settings
	if (m_pobParentEntity)
		AreaManager::Get().SetToEntityAreas( m_pobParentEntity, this );

	NinjaLua::LuaObject clumpAttribute = m_obAttributeTable->GetAttribute( "Clump" );
	if( clumpAttribute.IsString() )
	{
		ntstd::String clumpName = clumpAttribute.GetString();
		if( !clumpName.empty() && clumpName != "NULL" )
		{
			LoadClumpHeader( clumpName.c_str() );
			InstallHierarchy();
			InstallGfxComponents( clumpName.c_str() );
		}
	}

	SetPosition( m_InitialPosition );
	SetRotation( m_InitialRotation );

	if(m_bCollideOnlyWithCC)
		m_DefaultDynamics = "StaticEntityOnly";

	m_pobPhysicsSystem = Physics::System::ConstructSystemFromClump( this );

	// Add an interaction component
	// m_pobInteractionComponent = NT_NEW CInteractionComponent(this); Why? - JML

	SetPostConstructed(true);
}





