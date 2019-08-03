#if !defined(LISTVIEWWINDOW_H)
#define LISTVIEWWINDOW_H

#include "gui/debug/panelwindow.h"

#include "gui/guitext.h"

#include "core/nt_std.h"

class CListViewWindow :
	public CPanelWindow
{
public:
	typedef ntstd::Vector<ntstd::String> ContentsList;

	CListViewWindow(CBaseWindow* pParent);
	virtual ~CListViewWindow(void);
	
	void SetContents(const ContentsList& items);

	int MoveSelection(int offset);
	int GetSelection();

	const ntstd::String& GetSelectedItem();
	int GetSelectedItemIndex();

private:

	virtual void OnRender();

	int m_iCurrentItem;

	CStringDefinition m_obStrDef;

	ContentsList m_Contents;
};

#endif // LISTVIEWWINDOW_H
