//------------------------------------------------------------------------------------------
//!
//!	\file jumpmenu.cpp
//!
//------------------------------------------------------------------------------------------

#include "core/debug_hud.h"
#include "game/testmenu.h"
#include "game/entitycharacter.h"
#include "game/keybinder.h"
#include "game/entitymanager.h"
#include "game/attacks.h"
#include "game/query.h"
#include "game/comboinspector.h"
#include "game/inputcomponent.h"

//------------------------------------------------------------------------------------------
//!
//!	callback for the debug hud
//!
//------------------------------------------------------------------------------------------

bool TestMenu::TestMenuSelect( DebugHudItem* pItem, bool bSelect )
{
// The input component does not have any PlaySequence/PlayMove method in release build, so
// our callback shouldn't do anything in release mode either!
#ifdef _RELEASE
	FW_UNUSED(pItem);
	FW_UNUSED(bSelect);
#else
	if( bSelect )
	{
		const CAttackLink* pAttackLink = (const CAttackLink*)pItem->pSelectedUserUserData;
		ntError_p(pAttackLink, ("No attack associated with menu item!"));
		GetP()->ResetCharacters(); // Go into the actual singleton instance to perform these operations
		if (GetP()->m_bSingleMoveMode)
		{
			GetP()->m_pMenuTarget->GetInputComponent()->PlayMove(pAttackLink);
		}
		else
		{
			GetP()->m_pMenuTarget->GetInputComponent()->PlaySequence(pAttackLink);
		}
	}
#endif
	return false;
}

//------------------------------------------------------------------------------------------
//!
//!	ctors
//!
//------------------------------------------------------------------------------------------
TestMenu::TestMenu() :
	m_pMenuTarget(0),
	m_iUsedString(0),
	m_pDebugHudArray(0),
	m_bShowMenu(false),
	m_bSingleMoveMode(false),
	m_sSingleMoveHeader(0),
	m_sFullComboHeader(0),
	m_pOriginalActionLink(0)
{
}

TestMenu::~TestMenu()
{
	if (m_bShowMenu)
	{
		ToggleMenu();
	}
	CleanDebugHud();
	ClearCharacters();
}

TestMenu::CharacterData::CharacterData(Character* pCharacter) : m_pCharacter(pCharacter)
{
	m_obPosition = NT_NEW CPoint(m_pCharacter->GetPosition());
	m_obRotation = NT_NEW CQuat(m_pCharacter->GetRotation());
}

TestMenu::CharacterData::~CharacterData()
{
	NT_DELETE( m_obPosition );
	NT_DELETE( m_obRotation );
}

void TestMenu::CleanDebugHud()
{
	CleanDebugHudRecursively(m_pDebugHudArray);
	m_pDebugHudArray = 0;
}

void TestMenu::CleanDebugHudRecursively(DebugHudItem* pDebugHudArray)
{
	if (pDebugHudArray)
	{
		for ( DebugHudItem* hudEntry = pDebugHudArray; hudEntry->eType != DebugHudItem::DHI_NONE; ++hudEntry)
		{
			CleanDebugHudRecursively(hudEntry->pChildMenu);
		}
		NT_DELETE_ARRAY(pDebugHudArray);
	}
}

//------------------------------------------------------------------------------------------
//!
//! Simple linear allocator
//!
//------------------------------------------------------------------------------------------
const char* TestMenu::AllocString( const char* pToCopy )
{
	int iLen = strlen( pToCopy );
	ntAssert_p( (m_iUsedString + iLen+1) < STRING_BUFFER_SIZE, ("Out of string buffer space in TestMenu::AllocString") );
	char* pReturn = &m_StringBuffer[m_iUsedString];
	strcpy( pReturn , pToCopy );
	m_iUsedString += iLen+1; // one more for the null
	return pReturn;
}

TestMenu::AttackLinkCollection TestMenu::GatherAttacks(const Character* pCharacter) const
{
	AttackLinkCollection obFoundAttacks;
	const CClusterStructure* pClusterStructure = pCharacter->GetAttackComponent()->GetAttackDefinition()->m_pobClusterStructure;
	FollowAttackLinksRecursively(pClusterStructure->m_pobLeadCluster, obFoundAttacks);
	FollowAttackLinksRecursively(pClusterStructure->m_pobGroundClusterFront, obFoundAttacks);
	FollowAttackLinksRecursively(pClusterStructure->m_pobGroundClusterBack, obFoundAttacks);
	FollowAttackLinksRecursively(pClusterStructure->m_pobInstantKORecoverAttackCluster, obFoundAttacks);
	FollowAttackLinksRecursively(pClusterStructure->m_pobInterceptCluster, obFoundAttacks);
	FollowAttackLinksRecursively(pClusterStructure->m_pobRisingCluster, obFoundAttacks);
	FollowAttackLinksRecursively(pClusterStructure->m_pobInterceptCluster, obFoundAttacks);
	FollowAttackLinksRecursively(pClusterStructure->m_pobShortRangeCluster, obFoundAttacks);
	FollowAttackLinksRecursively(pClusterStructure->m_pobMediumRangeCluster, obFoundAttacks);
	FollowAttackLinksRecursively(pClusterStructure->m_pobLongRangeCluster, obFoundAttacks);
	FollowAttackLinksRecursively(pClusterStructure->m_pobBlockedGrab, obFoundAttacks);
	FollowAttackLinksRecursively(pClusterStructure->m_pobOpenFreeEvades, obFoundAttacks);
	FollowAttackLinksRecursively(pClusterStructure->m_pobComboFreeEvades, obFoundAttacks);
	return obFoundAttacks;
}

void TestMenu::FollowAttackLinksRecursively(const CAttackLink* pCurrentAttackLink, AttackLinkCollection &obFoundAttacks) const
{
	if (pCurrentAttackLink)
	{
		if (ntstd::find(obFoundAttacks.begin(), obFoundAttacks.end(), pCurrentAttackLink) == obFoundAttacks.end())
		{
			if (pCurrentAttackLink->GetAttackDataP())
			{
				obFoundAttacks.push_back(pCurrentAttackLink);
			}

			for (int iIndex = 0; iIndex < AM_NONE; ++iIndex)
			{
				if (pCurrentAttackLink->m_pobLinks[iIndex])
				{
					FollowAttackLinksRecursively(pCurrentAttackLink->m_pobLinks[iIndex], obFoundAttacks);
				}
			}

			// Special case that for some reason isn't part of the above array of attack links.
			if (pCurrentAttackLink->m_pobButtonHeldAttack)
			{
				FollowAttackLinksRecursively(pCurrentAttackLink->m_pobButtonHeldAttack, obFoundAttacks);
			}
		}
	}
}

TestMenu::CategorizedAttackLinkMap TestMenu::CategorizeAttacks(const AttackLinkCollection &obUncategorizedAttacks) const
{
	CategorizedAttackLinkMap categorizedAttacks;
	const GlobalEnum& genum = ObjectDatabase::Get().GetGlobalEnum( "ATTACK_CLASS" ); // we're categorizing by attack class
	for (AttackLinkCollectionIterator attackLinkIt = obUncategorizedAttacks.begin(); attackLinkIt != obUncategorizedAttacks.end(); ++attackLinkIt)
	{
		const CAttackLink* attackLink = (*attackLinkIt);
		const ATTACK_CLASS attackClass(attackLink->GetAttackDataP()->m_eAttackClass);
		categorizedAttacks[genum.GetName( attackClass ).c_str()].push_back(attackLink);
	}

	// Now that we've got our result, sort both the categories as well as the entries by alphabet
	for (CategorizedAttackLinkMap::iterator categoryIt = categorizedAttacks.begin(); categoryIt != categorizedAttacks.end(); ++categoryIt)
	{
		ntstd::sort((*categoryIt).second.begin(), (*categoryIt).second.end(), TestMenu::CompareAttackLinks);
	}

	return categorizedAttacks;
}

CHashedString TestMenu::LookUpAttackLinkName(const CAttackLink* pAttackLink)
{
	if (!pAttackLink)
		return "<null>";

	DataObject* pDataObject = ObjectDatabase::Get().GetDataObjectFromPointer(pAttackLink);
	CHashedString result(pDataObject->GetName());
	if (!ntStr::IsNull(result))
	{
		return result;
	}
	else
	{
		return CHashedString(pAttackLink->GetAttackDataP() ? pAttackLink->GetAttackDataP()->m_obAttackAnimName : "<unknown>");
	}
}

bool TestMenu::CompareAttackLinks(const CAttackLink* pFirstLink, const CAttackLink* pSecondLink)
{
	// Construct two String objects based on the attack link names char data, so we can compare them lexically
	return ntstd::String(ntStr::GetString(LookUpAttackLinkName(pFirstLink))).compare(ntstd::String(ntStr::GetString(LookUpAttackLinkName(pSecondLink)))) < 0;
}

void TestMenu::ConstructMenu(Character* pCharacter)
{
	static const unsigned int COL_RED			= NTCOLOUR_ARGB(0xFF,0xFF,0x00,0x00);
	static const unsigned int COL_GREEN			= NTCOLOUR_ARGB(0xFF,0x00,0xFF,0x00);
	static const unsigned int COL_BLUE			= NTCOLOUR_ARGB(0xFF,0x00,0x00,0xFF);
	static const unsigned int COL_WHITE			= NTCOLOUR_ARGB(0xFF,0xFF,0xFF,0xFF);

	// Reset everything
	CleanDebugHud();
	m_iUsedString = 0;
	m_sSingleMoveHeader = 0;
	m_sFullComboHeader = 0;

	m_pMenuTarget = pCharacter;

	// Gather what we need to display in the menu
	AttackLinkCollection obFoundAttacks(GatherAttacks(pCharacter));
	CategorizedAttackLinkMap obCategorizedAttacks(CategorizeAttacks(obFoundAttacks));

	// Save the position (and other data) of all nearby characters!
	RecordCharacters(pCharacter);


	// Actually build the menu from the data we gathered
	DebugHudItem baseItem = { DebugHudItem::DHI_TEXT, {0},		-1,			{COL_GREEN},	0, 0,				0,	0 };
	DebugHudItem funcItem = { DebugHudItem::DHI_TEXT, {0},		0,			{COL_GREEN},	0, TestMenuSelect,	0,	0 };
	DebugHudItem endItem =	{ DebugHudItem::DHI_NONE, {0},		0,			{0},			0,					0,	0 };

	int iNumItems = obCategorizedAttacks.size();
	m_pDebugHudArray = NT_NEW DebugHudItem[ iNumItems + 4 ];
	m_pDebugHudArray[ 0 ] = baseItem;
	UpdateHeaderText();
	m_pDebugHudArray[ 1 ] = baseItem;
	m_pDebugHudArray[ 1 ].uiColour = COL_BLUE;
	m_pDebugHudArray[ 1 ].pText = "--------";

	//------- Header finished
	int iIndex = 0;
	for (CategorizedAttackLinkMap::const_iterator categoryIt = obCategorizedAttacks.begin(); categoryIt != obCategorizedAttacks.end(); ++categoryIt, ++iIndex)
	{
		m_pDebugHudArray[ iIndex + 2 ] = baseItem; // iIndex + 2 is the offset from the start, skipping the first two header entries
		m_pDebugHudArray[ iIndex + 2 ].pText = AllocString( (*categoryIt).first.GetDebugString() );
		m_pDebugHudArray[ iIndex + 2 ].iIndex = iIndex;
		m_pDebugHudArray[ iIndex + 2 ].uiColour = COL_WHITE;

		// Build the submenu for this attack category
		int iNumSubItems = (*categoryIt).second.size();
		DebugHudItem *pSubDebugHudArray = NT_NEW DebugHudItem[ iNumSubItems + 4 ];
		pSubDebugHudArray[ 0 ] = baseItem;
		pSubDebugHudArray[ 0 ].pText = AllocString(ntStr::GetString((*categoryIt).first));
		pSubDebugHudArray[ 1 ] = baseItem;
		pSubDebugHudArray[ 1 ].uiColour = COL_BLUE;
		pSubDebugHudArray[ 1 ].pText = "--------";

		int iSubIndex = 0;
		for (AttackLinkCollectionIterator subIt = (*categoryIt).second.begin(); subIt != (*categoryIt).second.end(); ++subIt, ++iSubIndex)
		{
			pSubDebugHudArray[ iSubIndex + 2 ] = funcItem; // iSubIndex + 2 is the offset from the start, skipping the first two header entries
			pSubDebugHudArray[ iSubIndex + 2 ].pText = AllocString( ntStr::GetString(LookUpAttackLinkName(*subIt)) );
			pSubDebugHudArray[ iSubIndex + 2 ].iIndex = iSubIndex;
			if (ComboInspector::SearchForComboPath(pCharacter->GetAttackComponent()->GetAttackDefinition()->m_pobClusterStructure->m_pobLeadCluster, *subIt).empty())
			{
				// Combo not reachable from lead cluster, give special colour
				pSubDebugHudArray[ iSubIndex + 2 ].uiColour = COL_RED;
			}
			else
			{
				pSubDebugHudArray[ iSubIndex + 2 ].uiColour = COL_WHITE;
			}
			pSubDebugHudArray[ iSubIndex + 2 ].pSelectedUserUserData = (void*)*subIt;
		}

		pSubDebugHudArray[ iNumSubItems + 2 ] = baseItem; // iNumSubItems + 2 is the one-before-last index
		pSubDebugHudArray[ iNumSubItems + 2 ].pText = "--------";
		pSubDebugHudArray[ iNumSubItems + 2 ].uiColour = COL_BLUE;

		pSubDebugHudArray[ iNumSubItems + 3 ] = endItem; // iNumSubItems + 3 is the last index
		
		// Assign the submenu to the actual category
		m_pDebugHudArray[ iIndex + 2 ].pChildMenu = &pSubDebugHudArray[0];
	}

	////------- Footer start
	m_pDebugHudArray[ iNumItems + 2 ] = baseItem; // iNumItems + 2 is the one-before-last index
	m_pDebugHudArray[ iNumItems + 2 ].pText = AllocString("--------");
	m_pDebugHudArray[ iNumItems + 2 ].uiColour = COL_BLUE;

	m_pDebugHudArray[ iNumItems + 3 ] = endItem; // iNumItems + 3 is the last index
}

void TestMenu::UpdateHeaderText()
{
	if (m_pDebugHudArray)
	{
		if (m_bSingleMoveMode)
		{
			if (!m_sSingleMoveHeader)
			{
				m_sSingleMoveHeader = AllocString(ntStr::GetString(ntstd::String("TestMenu ") + "(simple)"));
			}
			m_pDebugHudArray[ 0 ].pText = m_sSingleMoveHeader;
		}
		else
		{
			if (!m_sFullComboHeader)
			{
				m_sFullComboHeader = AllocString(ntStr::GetString(ntstd::String("TestMenu ") + "(combos)"));
			}
			m_pDebugHudArray[ 0 ].pText = m_sFullComboHeader;
		}
	}
}

int TestMenu::GetMenuWidth()
{
	if (!m_bShowMenu || !m_pDebugHudArray)
	{
		return 0;
	}
	else
	{
		return GetMenuWidthRecursively(m_pDebugHudArray);
	}
}

int TestMenu::GetMenuWidthRecursively(DebugHudItem* pDebugHud)
{
	int iMaxWidth = 0;
	for ( DebugHudItem* hudEntry = pDebugHud; hudEntry->eType != DebugHudItem::DHI_NONE; ++hudEntry)
	{
		// "Guess" that the glyph-width is 12, best we can do here really. Also, we don't have text callback
		// menu items in the test menu, so no need to worry about those.
		int entryWidth = (int)(hudEntry->eType == DebugHudItem::DHI_TEXT ? strlen(hudEntry->pText) * 12.0f : 0);
		if (hudEntry->pChildMenu)
		{
			entryWidth += GetMenuWidthRecursively(hudEntry->pChildMenu);
		}
		if (entryWidth > iMaxWidth)
		{
			iMaxWidth = entryWidth;
		}
	}
	return iMaxWidth;
}

COMMAND_RESULT TestMenu::ToggleMenu()
{
	if( !m_bShowMenu ) // this means the menu is now becoming visible
	{
		ConstructMenu( CEntityManager::Get().GetPlayer() ); // for now just rebuild the menu for the player character
		DebugHUD::Get().UseMenu( m_pDebugHudArray );
	}
	else
	{
		if (m_bSingleMoveMode)
		{
			// This is so that we're sure that the action link has always been reset
			ToggleSingleMoveMode();
		}

		ClearCharacters();
		CleanDebugHud();
		m_pMenuTarget = 0;
		DebugHUD::Get().UseMenu( 0 );
	}

	// Only change this at the very end
	m_bShowMenu = !m_bShowMenu;
	return CR_SUCCESS;
}

COMMAND_RESULT TestMenu::ToggleSingleMoveMode()
{
	// This is only a valid action if the menu is visible
	if (m_bShowMenu)
	{
		m_bSingleMoveMode = !m_bSingleMoveMode;
		UpdateHeaderText();
		ntAssert_p(m_pMenuTarget && m_pMenuTarget->GetAttackComponent(), ("Must have a valid menu target by now!"));
		if (m_bSingleMoveMode)
		{
			// Remember the original action link on the menu target's lead cluster
			m_pOriginalActionLink = m_pMenuTarget->GetAttackComponent()->GetAttackDefinition()->m_pobClusterStructure->m_pobLeadCluster->m_pobLinks[AM_ACTION];
		}
		else
		{
			// Restore the action link we remembered
			m_pMenuTarget->GetAttackComponent()->GetAttackDefinition()->m_pobClusterStructure->m_pobLeadCluster->m_pobLinks[AM_ACTION] = m_pOriginalActionLink;
		}
		return CR_SUCCESS;
	}
	else
	{
		return CR_FAILED;
	}
}

COMMAND_RESULT TestMenu::DumpCategorizedAttacks()
{
	AttackLinkCollection obFoundAttacks(GatherAttacks(CEntityManager::Get().GetPlayer()));
	CategorizedAttackLinkMap obCategorizedAttacks(CategorizeAttacks(obFoundAttacks));
	
	for (CategorizedAttackLinkMap::const_iterator categoryIt = obCategorizedAttacks.begin(); categoryIt != obCategorizedAttacks.end(); ++categoryIt)
	{
		ntPrintf("\n%s\n", ntStr::GetString((*categoryIt).first));
		for (AttackLinkCollectionIterator subIt = (*categoryIt).second.begin(); subIt != (*categoryIt).second.end(); ++subIt)
		{
			ntPrintf("%s\n", ntStr::GetString(LookUpAttackLinkName(*subIt)));
		}
	}
	return CR_SUCCESS;
}

COMMAND_RESULT TestMenu::MoveBackwards()
{
	Character* pPlayer = CEntityManager::Get().GetPlayer();
	const Character* pClosestEnemy = FindClosestEnemy(pPlayer);
	if (!pClosestEnemy)
	{
		return CR_FAILED;
	}
	else
	{
		if (m_bShowMenu) ResetCharacters();
		CDirection moveDirection(pPlayer->GetPosition() - pClosestEnemy->GetPosition());
		moveDirection.Normalise();
		pPlayer->SetPosition(pPlayer->GetPosition() + (moveDirection * 0.1f));
		if (m_bShowMenu) RecordCharacters(pPlayer);
		ntPrintf("Moving away from '%s', distance is now: %f\n", ntStr::GetString(pClosestEnemy->GetName()), GetDistance(pPlayer, pClosestEnemy));
		return CR_SUCCESS;
	}
}

COMMAND_RESULT TestMenu::MoveForwards()
{
	Character* pPlayer = CEntityManager::Get().GetPlayer();
	const Character* pClosestEnemy = FindClosestEnemy(pPlayer);
	if (!pClosestEnemy)
	{
		return CR_FAILED;
	}
	else
	{
		if (m_bShowMenu) ResetCharacters();
		CDirection moveDirection(pPlayer->GetPosition() + pClosestEnemy->GetPosition());
		moveDirection.Normalise();
		pPlayer->SetPosition(pPlayer->GetPosition() - (moveDirection * 0.1f));
		if (m_bShowMenu) RecordCharacters(pPlayer);
		ResetCharacters();
		ntPrintf("Moving towards '%s', distance is now: %f\n", ntStr::GetString(pClosestEnemy->GetName()), GetDistance(pPlayer, pClosestEnemy));
		return CR_SUCCESS;
	}
}

const Character* TestMenu::FindClosestEnemy(const Character* pSource) const
{
	Character *pResult = 0;
	CEntityQuery obEnemyQuery;
	CEntityManager::GetP()->FindEntitiesByType(obEnemyQuery, CEntity::EntType_Character);
	QueryResultsContainerType& obAllEntities = obEnemyQuery.GetResults();
	for (QueryResultsContainerType::const_iterator entityIt = obAllEntities.begin(); entityIt != obAllEntities.end(); ++entityIt)
	{
		Character* pCharacter = static_cast<Character*>(*entityIt);
		if (pSource != pCharacter && (!pResult || GetDistance(pSource, pCharacter) < GetDistance(pSource, pResult)))
		{
			pResult = pCharacter;
		}
	}
	return pResult;
}

float TestMenu::GetDistance(const Character* pChar1, const Character* pChar2) const
{
	return CPoint(pChar1->GetPosition() - pChar2->GetPosition()).Length();
}

void TestMenu::ResetCharacters()
{
	for (CharacterDataCollection::const_iterator dataIt = m_obCharacterData.begin(); dataIt != m_obCharacterData.end(); ++dataIt)
	{
		const CharacterData *pCharacterData = *dataIt;
		pCharacterData->m_pCharacter->SetPosition(*pCharacterData->m_obPosition);
		pCharacterData->m_pCharacter->SetRotation(*pCharacterData->m_obRotation);
		pCharacterData->m_pCharacter->SetDead(false);
		pCharacterData->m_pCharacter->SetHealthPerc(100.0f);
	}
}

void TestMenu::RecordCharacters(const Character *pOrigin)
{
	ClearCharacters();
	CEntityQuery obCharacterQuery;
	CEQCProximitySphere obProximitySphere;
	obProximitySphere.Set(pOrigin->GetPosition(), 50.0f); // 50 should be more than enough...
	obCharacterQuery.AddClause(obProximitySphere);
	CEntityManager::GetP()->FindEntitiesByType(obCharacterQuery, CEntity::EntType_Character);
	QueryResultsContainerType& obAllEntities = obCharacterQuery.GetResults();
	for (QueryResultsContainerType::const_iterator entityIt = obAllEntities.begin(); entityIt != obAllEntities.end(); ++entityIt)
	{
		Character* pCharacter = static_cast<Character*>(*entityIt);
		m_obCharacterData.push_back(NT_NEW CharacterData(pCharacter));
	}
}

void TestMenu::ClearCharacters()
{
	for (CharacterDataCollection::const_iterator dataIt = m_obCharacterData.begin(); dataIt != m_obCharacterData.end(); ++dataIt)
	{
		NT_DELETE( *dataIt );
	}
	m_obCharacterData.clear();
}
