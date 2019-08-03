//------------------------------------------------------------------------------------------
//!
//!	\file jumpmenu.cpp
//!
//------------------------------------------------------------------------------------------

#include "core/debug_hud.h"
#include "game/luaglobal.h"
#include "game/jumpmenu.h"
#include "area/areasystem.h"
#ifndef PLATFORM_PS3
#include "hair/effectchain.h"
#endif

namespace
{
//------------------------------------------------------------------------------------------
//!
//!	callback for the debug hud to call a lua function
//!
//------------------------------------------------------------------------------------------

bool JumpMenuSelect( DebugHudItem* pItem, bool bSelect )
{
	if( bSelect )
	{
		// Obtain a reference to the global lua state
		NinjaLua::LuaObject obGlobals = CLuaGlobal::Get().State().GetGlobals();
		
		// Get a reference to the specified function
		const NinjaLua::LuaObject &obFuncOb = obGlobals[(char*)pItem->pSelectedUserUserData];	// Made const as GCC complains about assigned temporary to non-const ref [ARV].

		if( !obFuncOb.IsNil() && obFuncOb.IsFunction() )
		{
			// Typecast the object to a function
			NinjaLua::LuaFunction obFunction(obFuncOb);

			// Call the bound function function. 
			obFunction();

#ifndef PLATFORM_PS3
			// reset hair
			if(ChainRessource::Exists())
			{
				ChainRessource::Get().Reset();
			}
#endif
		}
	}
	return false;
}

}

// BINDFUNC:		ResetAreaSystem( int iArea )
// DESCRIPTION:		Resets the area system with the given area identifier
static void ResetAreaSystem( int iArea )
{
	if (!AreaSystem::IsValidAreaNumber(iArea))
	{
		user_warn_p( 0, ("Invalid area number just in ResetAreaSystem() :%d", iArea) );
		return;
	}

	ntAssert( AreaManager::Get().Exists() );
	AreaManager::Get().ForceReactivateLevel( iArea );
}

//------------------------------------------------------------------------------------------
//!
//!	ctor
//!
//------------------------------------------------------------------------------------------
JumpMenu::JumpMenu() :
	m_pDebugHudArray(0),
	m_iUsedString(0),
	m_bShowMenu( false )
{
	NinjaLua::LuaObject obGlobals = CLuaGlobal::Get().State().GetGlobals();

	obGlobals.Register( "ResetAreaSystem", ResetAreaSystem );
}

//------------------------------------------------------------------------------------------
//!
//!	dtor
//!
//------------------------------------------------------------------------------------------
JumpMenu::~JumpMenu()
{
	NT_DELETE_ARRAY( m_pDebugHudArray );
}

//------------------------------------------------------------------------------------------
//!
//! Simple linear allocator to keep our debug hud string nice and easy
//!
//------------------------------------------------------------------------------------------
const char* JumpMenu::AllocString( const char* pToCopy )
{
	int iLen = strlen( pToCopy );
	ntAssert_p( (m_iUsedString + iLen+1) < STRING_BUFFER_SIZE, ("Out of string buffer space in JumpMenu::AllocString") );
	char* pReturn = &m_StringBuffer[m_iUsedString];
	strcpy( pReturn , pToCopy );
	m_iUsedString += iLen+1; // one more for the null
	return pReturn;
}

COMMAND_RESULT JumpMenu::ToggleMenu()
{
	m_bShowMenu = !m_bShowMenu;

	if( m_bShowMenu )
	{
		DebugHUD::Get().UseMenu( m_pDebugHudArray );
	} else
	{
		DebugHUD::Get().UseMenu( 0 );
	}

	return CR_SUCCESS;
}

//------------------------------------------------------------------------------------------
//!
//!	Load - called to load the jump menu data
//!
//------------------------------------------------------------------------------------------
void JumpMenu::Load( const char* pLevelName )
{
	ntError_p( pLevelName, ("Must have a valid level name here") );

	static const unsigned int COL_GREEN			= NTCOLOUR_ARGB(0xFF,0x00,0xFF,0x00);
	static const unsigned int COL_BLUE			= NTCOLOUR_ARGB(0xFF,0xFF,0x00,0x00);
	static const unsigned int COL_WHITE			= NTCOLOUR_ARGB(0xFF,0xFF,0xFF,0xFF);

	// reset everything
	NT_DELETE_ARRAY( m_pDebugHudArray );
	m_pDebugHudArray = 0;
	m_iUsedString = 0;
	m_bShowMenu = false;
	DebugHUD::Get().UseMenu( 0 );

	char acLevelFile[ MAX_PATH ];
	char acFileName[MAX_PATH];
	snprintf( acLevelFile, MAX_PATH, "levels/%s_jumpmenu.lua", pLevelName );
	Util::GetFiosFilePath( acLevelFile, acFileName );

	// if file exists, try to source it.
	if( !File::Exists( acFileName ) )
	{
		return;
	}

	CLuaGlobal::Get().InstallFile( acFileName, false );

	// Obtain a reference to the global lua state
	NinjaLua::LuaObject obGlobals = CLuaGlobal::Get().State().GetGlobals();
	// Get a reference to the JumpMenu table
	const NinjaLua::LuaObject& obJumpMenu = obGlobals["JumpMenu"];							// Made const as GCC complains about assigned temporary to non-const ref [ARV].
	user_warn_p( obJumpMenu.IsTable(), ("JumpMenu isn't a lua table, Ignoring" ) );
	if( !obJumpMenu.IsTable() )
	{
		return;
	}

	DebugHudItem baseItem = { DebugHudItem::DHI_TEXT, {0},		-1,			{COL_GREEN},	0, 0,				0,	0 };
	DebugHudItem funcItem = { DebugHudItem::DHI_TEXT, {0},		0,			{COL_GREEN},	0, JumpMenuSelect,	0,	0 };
	DebugHudItem endItem =	{ DebugHudItem::DHI_NONE, {0},		0,			{0},			0,					0,	0 };

	int iNumItems = obJumpMenu.GetSize() + 4;
	m_pDebugHudArray = NT_NEW DebugHudItem[ iNumItems ];
	int iCurItem = 0;
	m_pDebugHudArray[ iCurItem ] = baseItem;
	m_pDebugHudArray[ iCurItem ].pText = AllocString( "JumpMenu" );
	iCurItem++;
	m_pDebugHudArray[ iCurItem ] = baseItem;
	m_pDebugHudArray[ iCurItem ].uiColour = COL_BLUE;
	m_pDebugHudArray[ iCurItem ].pText = AllocString( "--------" );
	iCurItem++;
	//------- Header finished
	for( int i=iCurItem, iIndex=0; i < iNumItems; ++i,++iIndex)
	{
		if( obJumpMenu[(iIndex*2)+1].IsString() && obJumpMenu[(iIndex*2)+2].IsString() )
		{
			m_pDebugHudArray[ i ] = funcItem;
			m_pDebugHudArray[ i ].pText = AllocString( obJumpMenu[(iIndex*2)+1].GetString() );
			m_pDebugHudArray[ i ].iIndex = iIndex;
			m_pDebugHudArray[ iCurItem ].uiColour = COL_WHITE;
			m_pDebugHudArray[ i ].pSelectedUserUserData = (void*) AllocString( obJumpMenu[(iIndex*2)+2].GetString() );
			iCurItem++;
		} else
		{
			break; // any error break the loop to be sure
		}
	}

	//------- Footer start
	m_pDebugHudArray[ iCurItem ] = baseItem;
	m_pDebugHudArray[ iCurItem ].pText = AllocString( "--------" );
	m_pDebugHudArray[ iCurItem ].uiColour = COL_BLUE;

	iCurItem++;
	m_pDebugHudArray[ iCurItem ] = endItem;


}
