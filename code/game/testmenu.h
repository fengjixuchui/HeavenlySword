//------------------------------------------------------------------------------------------
//!
//!	\file testmenu.h
//!
//------------------------------------------------------------------------------------------
#if !defined( HS_TESTMENU_H )
#define HS_TESTMENU_H

#include "game/commandresult.h"
#include "editable/enumlist.h"

struct DebugHudItem;
class Character;
class CAttackLink;

class TestMenu : public Singleton<TestMenu>
{
public:
	TestMenu();
	~TestMenu();

	void ConstructMenu(Character* pCharacter);
	void RecordCharacters(const Character* pCharacter);
	void ResetCharacters();
	void ClearCharacters();
	static bool TestMenuSelect( DebugHudItem* pItem, bool bSelect );
	COMMAND_RESULT ToggleMenu();
	COMMAND_RESULT ToggleSingleMoveMode();
	COMMAND_RESULT DumpCategorizedAttacks();
	COMMAND_RESULT MoveBackwards();
	COMMAND_RESULT MoveForwards();
	int GetMenuWidth();

private:
	struct CharacterData
	{
		CharacterData(Character* pCharacter);
		~CharacterData();

		Character* m_pCharacter;
		CPoint* m_obPosition;
		CQuat* m_obRotation;
	};
	
	struct CategoryComparator
	{
		bool operator()(const CHashedString& obFirstCategory, const CHashedString& obSecondCategory) const
		{
			// Reverse alphabetical order
			return ntstd::String(ntStr::GetString(obFirstCategory)).compare(ntstd::String(ntStr::GetString(obSecondCategory))) > 0;
		}
	};
	
	typedef ntstd::Vector<const CAttackLink*> AttackLinkCollection;
	typedef ntstd::Vector<const CAttackLink*>::const_iterator AttackLinkCollectionIterator;
	typedef ntstd::Map<const CHashedString, AttackLinkCollection, CategoryComparator> CategorizedAttackLinkMap;
	typedef ntstd::List<const CharacterData*> CharacterDataCollection;

	void CleanDebugHud();
	void CleanDebugHudRecursively(DebugHudItem* pDebugHud);
	int GetMenuWidthRecursively(DebugHudItem* pDebugHud);

	AttackLinkCollection GatherAttacks(const Character* pCharacter) const;
	void FollowAttackLinksRecursively(const CAttackLink* pCurrentAttackLink, AttackLinkCollection &obFoundAttacks) const;
	CategorizedAttackLinkMap CategorizeAttacks(const AttackLinkCollection& obUncategorizedAttacks) const;
	const Character* FindClosestEnemy(const Character* pSource) const;
	float GetDistance(const Character* pChar1, const Character* pChar2) const;
	void UpdateHeaderText();

	static CHashedString LookUpAttackLinkName(const CAttackLink* pAttackLink);
	static bool CompareAttackLinks(const CAttackLink* pFirstLink, const CAttackLink* pSecondLink);

	// allocates and copy the string onto the internal string buffer
	const char* AllocString( const char* pToCopy );

	CharacterDataCollection m_obCharacterData;
	Character* m_pMenuTarget;
	static const int STRING_BUFFER_SIZE = 4096;		// 4K
	char m_StringBuffer[STRING_BUFFER_SIZE];	//!< we need some temp space for strings
	int m_iUsedString;							//!< how much of our string buffer have we used

	DebugHudItem* m_pDebugHudArray;
	bool m_bShowMenu;
	bool m_bSingleMoveMode;
	const char *m_sSingleMoveHeader, *m_sFullComboHeader; // so we don't have to keep reallocating memory for them
	const CAttackLink* m_pOriginalActionLink;
};

#endif // end HS_TESTMENU_H
