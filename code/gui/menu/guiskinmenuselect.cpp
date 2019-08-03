/***************************************************************************************************
*
*	DESCRIPTION		
*
*	NOTES			
*
***************************************************************************************************/

// Includes
#include "guiskinmenuselect.h"
#include "gui/guimanager.h"
#include "gui/guisound.h"
#include "gui/guisettings.h"

#include "gui/guiscreen.h"
#include "gui/guiflow.h"
#include "lua/ninjalua.h"

#include "guiskinmenuitem.h"

#include "gfx/renderer.h"
#include "gfx/renderstates.h"
#include "gfx/graphicsdevice.h"
#include "effect/renderstate_block.h"

/***************************************************************************************************
*
*	The bobbins below allows us to register this class along with its XML tag before main is called.
*	This means that the parts constructing it don't have to know about it.
*
***************************************************************************************************/

static CXMLElement* ConstructWrapper( void ) { return NT_NEW CGuiSkinMenuSelect(); }

// Register this class under it's XML tag
bool g_bMENUSELECT = CGuiManager::Register( "MENUSELECT", &ConstructWrapper );

CGuiSkinMenuSelect::CGuiSkinMenuSelect(void)
{
	m_fCursorWidth = 0.0f;
	m_fCursorHeight = 0.0f;
	m_fCursorXOffset = 15.0f;
}

CGuiSkinMenuSelect::~CGuiSkinMenuSelect(void)
{
}

bool CGuiSkinMenuSelect::ProcessAttribute( const char* pcTitle, const char* pcValue )
{
	if ( !super::ProcessAttribute( pcTitle, pcValue ) )
	{
		if ( strcmp( pcTitle, "synctocontents" ) == 0 )
		{
			SyncToContents();
			return true;
		}
		else if ( strcmp( pcTitle, "setselecteditem" ) == 0 )
		{
			GuiUtil::SetInt( pcValue, &m_iSelectedUnit );
			CalcCursorPosition();
			return true;
		}

		return false;
	}
	return true;
}

bool CGuiSkinMenuSelect::ProcessEnd( void )
{
	super::ProcessEnd();

	const GuiSettingsMenuSelectDef* pobSettings = CGuiManager::Get().GuiSettings()->MenuSelect();
	m_obImage.SetTexture(pobSettings->CursorTexture());

	m_fCursorWidth = (float)m_obImage.GetTextureWidth();
	m_fCursorHeight = (float)m_obImage.GetTextureHeight();

	m_obImage.SetWidth( m_fCursorWidth );
	m_obImage.SetHeight( m_fCursorHeight );

	LoadCurrentPosition();

	LoadBackground();
	SyncToContents();

	return true;
}

bool CGuiSkinMenuSelect::MoveDownAction( int iPads )
{
	if (super::MoveDownAction(iPads))
	{
		CalcCursorPosition();
		SaveCurrentPosition();

		CGuiSoundManager::Get().PlaySound(CGuiSound::ACTION_DOWN);

		return true;
	}
	return false;
}

bool CGuiSkinMenuSelect::MoveUpAction( int iPads )
{
	if (super::MoveUpAction(iPads))
	{
		CalcCursorPosition();
		SaveCurrentPosition();

		CGuiSoundManager::Get().PlaySound(CGuiSound::ACTION_UP);

		return true;
	}
	return false;
}

void CGuiSkinMenuSelect::SaveCurrentPosition()
{
	GetParentScreen()->GetScreenHeader()->GetScreenStore().Set("menuCurrentSelection", m_iSelectedUnit);
}

void CGuiSkinMenuSelect::LoadCurrentPosition()
{
	NinjaLua::LuaObject obTmp = GetParentScreen()->GetScreenHeader()->GetScreenStore()["menuCurrentSelection"];
	if (obTmp.IsNumber())
	{
		m_iSelectedUnit = obTmp.GetInteger();

		if (m_iSelectedUnit < 0)
			m_iSelectedUnit = 0;

		if (m_iSelectedUnit >= (int)m_obSelectableUnits.size())
			m_iSelectedUnit = ((int)m_obSelectableUnits.size()) - 1;
	}
}

//monitor external selection change
bool CGuiSkinMenuSelect::Update()
{
	NinjaLua::LuaObject obTmp = GetParentScreen()->GetScreenHeader()->GetScreenStore()["menuCurrentSelection"];
	if (obTmp.IsNumber())
	{
		if (m_iSelectedUnit != obTmp.GetInteger())
		{
			LoadCurrentPosition();
			CalcCursorPosition();
		}
	}

	return super::Update();
}

void CGuiSkinMenuSelect::CalcCursorPosition()
{
	CGuiSkinMenuItem* pobUnit = static_cast<CGuiSkinMenuItem*>( m_papobSelectableUnits[m_iSelectedUnit] );

	GuiExtents ex;
	pobUnit->GetExtents(ex);

	const GuiSettingsMenuSelectDef* pobSettings = CGuiManager::Get().GuiSettings()->MenuSelect();
    const CVector& obOffset = pobSettings->CursorOffset();

	CPoint obPos(ex.fX - m_fCursorWidth/2.0f + obOffset.X(), ex.fY + ex.fHeight/2 + obOffset.Y(), 0.0f);
	m_obImage.SetPosition( obPos );
}

bool CGuiSkinMenuSelect::Render()
{
	Renderer::Get().SetBlendMode( GFX_BLENDMODE_LERP );

	uint32_t uiCol = CVector(1.0f, 1.0f, 1.0f, ScreenFade()).GetNTColor();

	for (int i = 0; i < 9; i++)
	{
		m_aobBackground[i].SetColour( uiCol );
		m_aobBackground[i].Render();
	}

	m_obImage.SetColour( uiCol );
	m_obImage.Render();

	Renderer::Get().SetBlendMode( GFX_BLENDMODE_OVERWRITE );

	return super::Render();
}

void CGuiSkinMenuSelect::LoadBackground()
{
	const GuiSettingsMenuSelectDef* pobSettings = CGuiManager::Get().GuiSettings()->MenuSelect();

	m_aobBackground[0].SetTexture( pobSettings->TopLeftTexture() );
	m_aobBackground[1].SetTexture( pobSettings->TopTexture() );
	m_aobBackground[2].SetTexture( pobSettings->TopRightTexture() );
	m_aobBackground[3].SetTexture( pobSettings->RightTexture() );
	m_aobBackground[4].SetTexture( pobSettings->BottomRightTexture() );
	m_aobBackground[5].SetTexture( pobSettings->BottomTexture() );
	m_aobBackground[6].SetTexture( pobSettings->BottomLeftTexture() );
	m_aobBackground[7].SetTexture( pobSettings->LeftTexture() );
	m_aobBackground[8].SetTexture( pobSettings->CentreTexture() );
}

void CGuiSkinMenuSelect::PositionBackground()
{
	CPoint obMin(CGuiManager::Get().BBWidth(), CGuiManager::Get().BBHeight(), 0);
	CPoint obMax(0, 0, 0);

	GuiExtents ext;
	for( ntstd::List< CGuiUnit* >::iterator obIt = m_obSelectableUnits.begin(); obIt != m_obSelectableUnits.end(); ++obIt )
	{
		static_cast<CGuiSkinMenuItem*>( *obIt )->GetExtents(ext);

		if (ext.fX < obMin.X())
			obMin.X() = ext.fX;
		if (ext.fX + ext.fWidth > obMax.X())
			obMax.X() = ext.fX + ext.fWidth;

		if (ext.fY < obMin.Y())
			obMin.Y() = ext.fY;
		if (ext.fY + ext.fHeight > obMax.Y())
			obMax.Y() = ext.fY + ext.fHeight;
	}
	const GuiSettingsMenuSelectDef* pobSettings = CGuiManager::Get().GuiSettings()->MenuSelect();

	const CPoint& obPad = CPoint(pobSettings->BorderPadding());
	const CPoint& obBorderSize = CPoint(pobSettings->BorderSize());

	CPoint obBodySize = obMax - obMin + obPad*2.0f;
	CPoint obCentre = obMin + obBodySize/2.0f - obPad;

	//centre
	m_aobBackground[8].SetWidth( obBodySize.X() );
	m_aobBackground[8].SetHeight( obBodySize.Y() );
	m_aobBackground[8].SetPosition( obCentre );


	CPoint obHalfBorderSize = obBorderSize/2.0f;
	CPoint obHalfBodySize = obBodySize/2.0f;

	//topleft
	m_aobBackground[0].SetWidth( obBorderSize.X() );
	m_aobBackground[0].SetHeight( obBorderSize.Y() );
	m_aobBackground[0].SetPosition( obCentre - obHalfBodySize - obHalfBorderSize );

	//top
	m_aobBackground[1].SetWidth( obBodySize.X() );
	m_aobBackground[1].SetHeight( obBorderSize.Y() );
	m_aobBackground[1].SetPosition( obCentre - CPoint(0, obHalfBodySize.Y() + obHalfBorderSize.Y(), 0) );

	//topright
	m_aobBackground[2].SetWidth( obBorderSize.X() );
	m_aobBackground[2].SetHeight( obBorderSize.Y() );
	m_aobBackground[2].SetPosition( obCentre + CPoint(obHalfBodySize.X() + obHalfBorderSize.X(), -(obHalfBodySize.Y() + obHalfBorderSize.Y()), 0) );

	//right
	m_aobBackground[3].SetWidth( obBorderSize.X() );
	m_aobBackground[3].SetHeight( obBodySize.Y() );
	m_aobBackground[3].SetPosition( obCentre + CPoint(obHalfBodySize.X() + obHalfBorderSize.X(), 0, 0) );

	//bottomright
	m_aobBackground[4].SetWidth( obBorderSize.X() );
	m_aobBackground[4].SetHeight( obBorderSize.Y() );
	m_aobBackground[4].SetPosition( obCentre + obHalfBodySize + obHalfBorderSize );

	//bottom
	m_aobBackground[5].SetWidth( obBodySize.X() );
	m_aobBackground[5].SetHeight( obBorderSize.Y() );
	m_aobBackground[5].SetPosition( obCentre + CPoint(0, obHalfBodySize.Y() + obHalfBorderSize.Y(), 0) );

	//bottomleft
	m_aobBackground[6].SetWidth( obBorderSize.X() );
	m_aobBackground[6].SetHeight( obBorderSize.Y() );
	m_aobBackground[6].SetPosition( obCentre - CPoint(obHalfBodySize.X() + obHalfBorderSize.X(), -(obHalfBodySize.Y() + obHalfBorderSize.Y()), 0) );

	//left
	m_aobBackground[7].SetWidth( obBorderSize.X() );
	m_aobBackground[7].SetHeight( obBodySize.Y() );
	m_aobBackground[7].SetPosition( obCentre - CPoint(obHalfBodySize.X() + obHalfBorderSize.X(), 0, 0) );
}

void CGuiSkinMenuSelect::SyncToContents()
{
	PositionBackground();
	CalcCursorPosition();
}
