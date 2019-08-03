/***************************************************************************************************
*
*	DESCRIPTION		
*
*	NOTES			
*
***************************************************************************************************/

#ifndef GUIDEBUGSKINLEVELSELECT_H
#define GUIDEBUGSKINLEVELSELECT_H

// Includes
#include "gui/guiunitselect.h"
#include "effect/screensprite.h"
#include "gui/guitext.h"

#include "core/nt_std.h"

#include "gui/debug/PanelWindow.h"
#include "gui/debug/TitleWindow.h"
#include "gui/debug/ScrollBarWindow.h"
#include "gui/debug/ListViewWindow.h"

/***************************************************************************************************
*
*	CLASS			CGuiDebugSkinLevelSelect
*
*	DESCRIPTION		
*
***************************************************************************************************/

class CGuiDebugSkinLevelSelect: public CGuiUnit
{
public:

	// Construction Destruction
	CGuiDebugSkinLevelSelect( void );
	virtual	~CGuiDebugSkinLevelSelect( void );

	bool			Render( void );
	void			UpdateIdle( void );

	static ntstd::String ms_obSelectedLevel;

protected:

	//! To deal with parsing
	virtual bool	ProcessAttribute( const char* pcTitle, const char* pcValue );
	virtual bool	ProcessEnd( void );

	virtual bool MoveDownAction( int iPads );
	virtual bool MoveUpAction( int iPads );
	virtual bool AnyAction( int iPads );
	virtual bool StartAction( int iPads );
	virtual bool SelectAction( int iPads );

	static void StartElementHandler(void *pvUserData, const char *el, const char **attr);
	static void EndElementHandler(void *pvUserData, const char *el);
	static void CommentHandler( void* pvUserData, const char* pcComment );

	struct LevelInfo
	{
		ntstd::String name;
		ntstd::String path;
	};

#ifdef ENUM_FILE_SYS
	void EnumerateLevels(const char* szRootPath);
#else
	void EnumerateLevels(const char* szLevelsFile);
#endif

	void RenderLevels();

	typedef ntstd::Vector<LevelInfo> LevelList;
	LevelList m_obLevelList;

	CPanelWindow* m_pLevelSelect;
	CScrollBarWindow* m_pScrollBar;
	CListViewWindow* m_pLevelListView;

	bool m_bLevelsAvailable;
};

#endif // GUIDEBUGSKINLEVELSELECT_H
