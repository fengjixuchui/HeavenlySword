
#include "listviewwindow.h"
#include "gui/guitext.h"
#include "gui/guitextinternals.h"
#include "gui/guiutil.h"
#include "core/visualdebugger.h"

CListViewWindow::CListViewWindow(CBaseWindow* pParent)
	: CPanelWindow(pParent)
{
	m_obStrDef.m_pobFont = CFontManager::Get().GetFont("Comic");

	m_obStrDef.m_eVJustify = CStringDefinition::JUSTIFY_TOP;
	m_obStrDef.m_eHJustify = CStringDefinition::JUSTIFY_LEFT;
}

CListViewWindow::~CListViewWindow(void)
{
}

void CListViewWindow::OnRender()
{
	CPanelWindow::OnRender();

	CFont::GLYPH_ATTR_2D* pAttr = 0;
	Texture::Ptr pTex;	//unused
	m_obStrDef.m_pobFont->GetGlyphDetails2D(L'>', &pAttr, &pTex);

	float fFontHeight = m_obStrDef.m_pobFont->GetFontLineHeight();

	float fXBorder = (float)pAttr->iWidth*3;
	float fYBorder = 10.0f;

	float fX = m_ScreenExtents.fX + fXBorder;
	float fY = m_ScreenExtents.fY + fYBorder;

	int iTotalItems = m_Contents.size();
	int iMaxItemsDisplayed = (int)((m_ScreenExtents.fHeight + fYBorder*2)/fFontHeight);
	iMaxItemsDisplayed = min(iMaxItemsDisplayed, iTotalItems);
	
	int iOffset;
	if (m_iCurrentItem <= iMaxItemsDisplayed/2)
		iOffset = 0;
	else if (m_iCurrentItem > (iTotalItems - 1 - iMaxItemsDisplayed/2))
		iOffset = iTotalItems - iMaxItemsDisplayed;
	else
		iOffset = m_iCurrentItem - iMaxItemsDisplayed/2;

	uint32_t col1 = CVector(1.0f, 0.4314f, 0.0f, 1.0f).GetNTColor();
	uint32_t col2 = CVector(0.314f, 0.569f, 0.627f, 1.0f).GetNTColor();

//	ntPrintf("m_iCurrentItem = %d ", m_iCurrentItem);
//	ntPrintf("iMaxItemsDisplayed = %d ", iMaxItemsDisplayed);
//	ntPrintf("iTotalItems = %d ", iTotalItems);
//	ntPrintf("iOffset = %d\n", iOffset);

	for (int i = 0; i < iMaxItemsDisplayed; i++)
	{
		int iIndex = i + iOffset;

		uint32_t col;
		if (iIndex == m_iCurrentItem)
		{
			GuiUtil::RenderString(fX - fXBorder, fY, &m_obStrDef, ">");
			col = col1;
		}
		else
			col = col2;

		//ntPrintf("iIndex = %d\n", iIndex);

		if (iIndex < (int)m_Contents.size())
		{
			GuiUtil::RenderString(fX, fY, &m_obStrDef, m_Contents[iIndex].c_str());
		//	g_VisualDebug->Printf2D(fX, fY, col, 0, m_Contents[iIndex].c_str());
		}

		fY += fFontHeight;
	}

//	fX = CGuiManager::Get().BBWidth()*0.5f;
//	fY = CGuiManager::Get().BBHeight()*0.75f;
//	g_VisualDebug->Printf2D(fX, fY, DC_RED, 0, "m_iCurrentItem = %d", m_iCurrentItem);			fY += fFontHeight;
//	g_VisualDebug->Printf2D(fX, fY, DC_RED, 0, "iMaxItemsDisplayed = %d", iMaxItemsDisplayed);	fY += fFontHeight;
//	g_VisualDebug->Printf2D(fX, fY, DC_RED, 0, "iTotalItems = %d", iTotalItems);				fY += fFontHeight;
//	g_VisualDebug->Printf2D(fX, fY, DC_RED, 0, "iOffset = %d", iOffset);						fY += fFontHeight;
}

void CListViewWindow::SetContents(const ContentsList& items)
{
	m_Contents = items;
	m_iCurrentItem = 0;
}

int CListViewWindow::GetSelection()
{
	return m_iCurrentItem;
}

const ntstd::String& CListViewWindow::GetSelectedItem()
{
	return m_Contents[m_iCurrentItem];
}

int CListViewWindow::GetSelectedItemIndex()
{
	return m_iCurrentItem;
}

int CListViewWindow::MoveSelection(int offset)
{
	if (offset != 0)
	{
		m_iCurrentItem += offset;

		if (offset > 0)
		{
			int iTotalItems = m_Contents.size();
			if (m_iCurrentItem >= iTotalItems)
				m_iCurrentItem = iTotalItems-1;
		}
		else
		{
			if (m_iCurrentItem < 0)
				m_iCurrentItem = 0;
		}
	}

	return m_iCurrentItem;
}
