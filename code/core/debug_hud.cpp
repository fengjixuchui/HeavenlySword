/***************************************************************************************************
*
* debug_hud.cpp
* Simple menu class to make a cool, handy debug menu system
*
***************************************************************************************************/

#include "core/debug_hud.h"
#include "game/shelldebug.h"
#include "gfx/graphing.h"
#include "input/inputhardware.h"
#include "core/visualdebugger.h"
#include "game/shelldebug.h"

DebugHUD::DebugHUD()
{
	m_fStartX = sfDebugLeftBorder;
	m_fStartY = sfContextTopBorder;
	m_pStartMenuItem = 0;
	m_iSelected[0] = 0;
}

void DebugHUD::UseMenu( DebugHudItem* pMenu )
{
	m_pStartMenuItem = pMenu;
	m_pParent = 0;
	// find current menu
	m_iSelected[0] = 0;
	m_SelectedLevel = 0;

	SelectFirstValid( m_pStartMenuItem );
}

DebugHudItem* DebugHUD::GetSelected()
{
	int level = 0;
	DebugHudItem* pMenuItem;

	do
	{
		pMenuItem = GetSelectedAtLevel(level);
		if( level == m_SelectedLevel )
		{
			return pMenuItem;
		} else
		{
			level++;
		}
	} while( pMenuItem->pChildMenu != 0 );

	return pMenuItem;
}

DebugHudItem* DebugHUD::GetSelectedFrom( DebugHudItem* pMenuItem, int iSelected )
{
	if( pMenuItem == 0 )
		return 0;
	while( pMenuItem->eType != DebugHudItem::DHI_NONE )
	{
		if( pMenuItem->iIndex == iSelected )
		{
			return pMenuItem;
		}
		pMenuItem = pMenuItem+1;
	}
	return 0;
}

DebugHudItem* DebugHUD::GetSelectedAtLevel( int level )
{
	if( level == 0 )
	{
		return GetSelectedFrom( m_pStartMenuItem, m_iSelected[0] );
	} else
	{
		DebugHudItem* pMenuItem = GetSelectedAtLevel( level-1 );
		if( pMenuItem && pMenuItem->pChildMenu ) 
		{
			pMenuItem = GetSelectedFrom( pMenuItem->pChildMenu, m_iSelected[level] );
		}
		return pMenuItem;
	}
}

void DebugHUD::SelectFirstValid( DebugHudItem* pMenuItem )
{
	if( pMenuItem == 0 )
		return;
	while( pMenuItem->eType != DebugHudItem::DHI_NONE )
	{
		if( pMenuItem->iIndex >= 0 )
		{
			m_iSelected[m_SelectedLevel] = pMenuItem->iIndex;
			return;		
		}
		pMenuItem = pMenuItem+1;
	}
}

void DebugHUD::SelectNext()
{
	DebugHudItem* pMenuItem = GetSelected();
	if( pMenuItem == 0 )
		return;
	pMenuItem = pMenuItem+1;
	while( pMenuItem->eType != DebugHudItem::DHI_NONE )
	{
		if( pMenuItem->iIndex >= 0 )
		{
			m_iSelected[m_SelectedLevel] = pMenuItem->iIndex;
			return;
		}
		pMenuItem = pMenuItem+1;
	}
}

void DebugHUD::SelectPrev()
{
	DebugHudItem* pMenuItem = GetSelected();
	if( pMenuItem == 0 )
		return;
	DebugHudItem* pStartMenuItem;
	if( m_SelectedLevel == 0 )
	{
		pStartMenuItem = m_pStartMenuItem;
	} else
	{
		pStartMenuItem = GetSelectedAtLevel(m_SelectedLevel-1)->pChildMenu;
	}

	if( pMenuItem != pStartMenuItem )
	{
		pMenuItem = pMenuItem-1;
	}

	while( pMenuItem != pStartMenuItem )
	{
		if( pMenuItem->iIndex >= 0 )
		{
			m_iSelected[m_SelectedLevel] = pMenuItem->iIndex;
			return;
		}
		pMenuItem = pMenuItem-1;
	}
}


void DebugHUD::Update()
{
	if( m_pStartMenuItem == 0 )
		return;

	DoUserInterface();

	// display the menu to the user
	DrawMenu( m_pStartMenuItem, m_fStartX, m_fStartY, 0 ); 
}

void DebugHUD::DrawMenu( DebugHudItem* pMenuItem, float fOrigX, float fOrigY, int level )
{
#ifndef _GOLD_MASTER

	float fX = fOrigX;
	float fY = fOrigY;
	DebugHudItem* pChildMenuItem = 0;

	float maxWidth = 0.f;

	float fHitBoxSX = 0;
	float fHitBoxSY = 0;
	float fHitBoxEX = 0;
	float fHitBoxEY = 0;

	const float fGlypthWidth = 14.f; // debug text doesn't exist a way of getting the size of a line of text
	const float fGlypthHeight = 12.f; // debug text doesn't exist a way of getting the size of a line of text

	char outText[256];
	while( pMenuItem->eType != DebugHudItem::DHI_NONE )
	{
		fHitBoxSX = fX; fHitBoxSY = fY; // most render types don't move the start point of the hit box
		switch( pMenuItem->eType )
		{
		case DebugHudItem::DHI_TEXT:
			g_VisualDebug->Printf2D( fX, fY, pMenuItem->uiColour, 0, pMenuItem->pText );
			fY += fGlypthHeight;
			fHitBoxEX = strlen(pMenuItem->pText) * fGlypthWidth + fHitBoxSX; fHitBoxEY = fY;
			break;
		case DebugHudItem::DHI_TEXT_CALLBACK:
			{
				unsigned int uiColour;
				pMenuItem->pTextCallback( pMenuItem, outText, uiColour );
				g_VisualDebug->Printf2D( fX, fY, uiColour, 0, outText );
				fY += fGlypthHeight;
				fHitBoxEX = strlen(outText) * fGlypthWidth + fHitBoxSX; fHitBoxEY = fY;
			}
			break;
		case DebugHudItem::DHI_DRAW_CALLBACK:
			{
				float width, height;
				pMenuItem->pDrawCallback( pMenuItem, &fX, &fY, &width, &height );
				fHitBoxSX = fX; fHitBoxSY = fY; // a draw may have moved the start coordinates
				fX += width, fY += height;
				fHitBoxEX = fX; fHitBoxEY = fY;
			}
			break;
		default:
			// do nothing
			break;
		}
		if( pMenuItem->iIndex == m_iSelected[level] )
		{
			if( level == m_SelectedLevel )
			{
				if( pMenuItem->pHighLightedFunc == 0 )
				{
					static const unsigned int dwCol = DC_RED;
					CPoint tl(fHitBoxSX, fHitBoxSY,0);
					CPoint tr(fHitBoxEX, fHitBoxSY,0);
					CPoint br(fHitBoxEX, fHitBoxEY,0);
					CPoint bl(fHitBoxSX, fHitBoxEY,0);
					g_VisualDebug->RenderLine( tl, tr, dwCol, DPF_DISPLAYSPACE );
					g_VisualDebug->RenderLine( tr, br, dwCol, DPF_DISPLAYSPACE );
					g_VisualDebug->RenderLine( br, bl, dwCol, DPF_DISPLAYSPACE );
					g_VisualDebug->RenderLine( bl, tl, dwCol, DPF_DISPLAYSPACE );
				} else
				{
					pMenuItem->pHighLightedFunc( pMenuItem );
				}
			}

			if( (level <= m_SelectedLevel) && pMenuItem->pChildMenu != 0 )
			{
				pChildMenuItem = pMenuItem->pChildMenu;
			}
		}
		maxWidth = ntstd::Max( maxWidth, fHitBoxEX );

		pMenuItem = pMenuItem+1;
	}
	if( pChildMenuItem != 0 )
	{
		DrawMenu( pChildMenuItem, maxWidth, fOrigY, ++level  );
	}
#endif
}

void DebugHUD::DoUserInterface()
{
	static const int PAGE_UP_DOWN_NUM = 10;
	if ( CInputHardware::Get().GetKeyboard().IsKeyPressed(KEYC_DOWN_ARROW) )
	{
		DebugHudItem* pMenuItem = GetSelected();
		if( pMenuItem && pMenuItem->pSelectedFunc )
		{
			pMenuItem->pSelectedFunc( pMenuItem, false );
		}

		SelectNext();
	}
	else if ( CInputHardware::Get().GetKeyboard().IsKeyPressed(KEYC_UP_ARROW) )
	{
		DebugHudItem* pMenuItem = GetSelected();
		if( pMenuItem && pMenuItem->pSelectedFunc )
		{
			pMenuItem->pSelectedFunc( pMenuItem, false );
		}

		SelectPrev();
	}
	if ( CInputHardware::Get().GetKeyboard().IsKeyPressed(KEYC_DOWN_ARROW, KEYM_CTRL) )
	{
		// using CTRL-DOWN instead of PAGE_DOWN to avoid conflicts with other PAGE_DOWN uses
		for(int i=0;i < PAGE_UP_DOWN_NUM;i++)
		{
			DebugHudItem* pMenuItem = GetSelected();
			if( pMenuItem && pMenuItem->pSelectedFunc )
			{
				pMenuItem->pSelectedFunc( pMenuItem, false );
			}
			SelectNext();
		}
	}
	else if ( CInputHardware::Get().GetKeyboard().IsKeyPressed(KEYC_UP_ARROW, KEYM_CTRL) )
	{
		// using CTRL-UP instead of PAGE_UP to avoid conflicts with other PAGE_UP uses
		for(int i=0;i < PAGE_UP_DOWN_NUM;i++)
		{
			DebugHudItem* pMenuItem = GetSelected();
			if( pMenuItem && pMenuItem->pSelectedFunc )
			{
				pMenuItem->pSelectedFunc( pMenuItem, false );
			}

			SelectPrev();
		}
	}
	if ( CInputHardware::Get().GetKeyboard().IsKeyPressed(KEYC_RIGHT_ARROW) )
	{
		DebugHudItem* pMenuItem = GetSelected();
		if( pMenuItem && pMenuItem->pSelectedFunc )
		{
			if( pMenuItem->pSelectedFunc( pMenuItem, true ) == false )
			{
				return;
			}
		}

		if( pMenuItem && pMenuItem->pChildMenu != 0 )
		{
			m_SelectedLevel++;
			SelectFirstValid( pMenuItem->pChildMenu );
		} 

	}
	else if ( CInputHardware::Get().GetKeyboard().IsKeyPressed(KEYC_LEFT_ARROW) )
	{
		DebugHudItem* pMenuItem = GetSelected();
		if( pMenuItem && pMenuItem->pSelectedFunc )
		{
			pMenuItem->pSelectedFunc( pMenuItem, false );
		}

		if( m_SelectedLevel > 0 )
		{
			m_iSelected[ m_SelectedLevel ] = 0;
			m_SelectedLevel--;
		}
	}
}
