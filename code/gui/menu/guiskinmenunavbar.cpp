/***************************************************************************************************
*
*	DESCRIPTION		
*
*	NOTES			
*
***************************************************************************************************/

// Includes
#include "guiskinmenunavbar.h"
#include "gui/guimanager.h"
#include "gui/guisettings.h"

#include "gfx/renderer.h"
#include "gfx/renderstates.h"
#include "gfx/graphicsdevice.h"
#include "effect/renderstate_block.h"

#include "anim/hierarchy.h"

#ifdef DEBUG_NAV_FADE
#	include "core/visualdebugger.h"
#endif

/***************************************************************************************************
*
*	The bobbins below allows us to register this class along with its XML tag before main is called.
*	This means that the parts constructing it don't have to know about it.
*
***************************************************************************************************/

static CXMLElement* ConstructWrapper( void ) { return NT_NEW CGuiSkinMenuNavigationBar(); }

// Register this class under it's XML tag
bool g_bMENUNAVBAR = CGuiManager::Register( "MENUNAVBAR", &ConstructWrapper );


/***************************************************************************************************
*
*	FUNCTION		GuiNavigationBar::GuiNavigationBar
*
*	DESCRIPTION		Construction
*
***************************************************************************************************/

GuiNavigationBar::GuiNavigationBar()
{
	const GuiSettingsNavBarDef* pobSettings = CGuiManager::Get().GuiSettings()->NavBar();
	float fBBWidth = CGuiManager::Get().BBWidth();

	m_apobString[SLOT_SELECT] = NULL;
	m_aobStringDef[SLOT_SELECT].m_fXOffset = pobSettings->SelectXOffset() * fBBWidth;
	m_aobStringDef[SLOT_SELECT].m_eHJustify = CStringDefinition::JUSTIFY_LEFT;

	m_apobString[SLOT_BACK] = NULL;
	m_aobStringDef[SLOT_BACK].m_fXOffset = pobSettings->BackXOffset() * fBBWidth;
	m_aobStringDef[SLOT_BACK].m_eHJustify = CStringDefinition::JUSTIFY_LEFT;

	m_apobString[SLOT_DESCRIPTION] = NULL;
	m_aobStringDef[SLOT_DESCRIPTION].m_fXOffset = pobSettings->DescriptionXOffset() * fBBWidth;
	m_aobStringDef[SLOT_DESCRIPTION].m_eHJustify = CStringDefinition::JUSTIFY_RIGHT;

	m_pobTransform = NULL;
	CreateTransform();

	m_bShowScriptOverlay = false;
	CreateOverlay();

	for (int i = 0; i < NUM_SLOTS; i++)
	{
		m_afFadeTimers[i] = 0.0f;
		m_abFadeDir[i] = true;
		m_abFadeWaiting[i] = true;
	}
}

/***************************************************************************************************
*
*	FUNCTION		GuiNavigationBar::~GuiNavigationBar
*
*	DESCRIPTION		Destruction
*
***************************************************************************************************/

GuiNavigationBar::~GuiNavigationBar( void )
{
	DestroyString(SLOT_SELECT);
	DestroyString(SLOT_BACK);
	DestroyString(SLOT_DESCRIPTION);

	DestroyTransform();
}

/***************************************************************************************************
*
*	FUNCTION		
*
*	DESCRIPTION		
*
***************************************************************************************************/

void GuiNavigationBar::ShowScriptOverlay(bool bShow)
{
	m_bShowScriptOverlay = bShow;
}

void GuiNavigationBar::ShowTextSlot(SLOT eSlot, bool bShow, const char* pcID)
{
	m_abShow[eSlot] = bShow;

	m_abFadeWaiting[eSlot] = true;
	m_abFadeDir[eSlot] = bShow;

	if (!m_abShow[eSlot])
	{
	//	DestroyString(eSlot);
		return;
	}

	switch (eSlot)
	{
	case SLOT_SELECT:
		CreateString(SLOT_SELECT, CGuiManager::Get().GuiSettings()->NavBar()->SelectTitleId());
		break;
	case SLOT_BACK:
		CreateString(SLOT_BACK, CGuiManager::Get().GuiSettings()->NavBar()->BackTitleId());
		break;
	case SLOT_DESCRIPTION:
        CreateString(SLOT_DESCRIPTION, pcID);
		break;
	default:
		break;
	}
}

void GuiNavigationBar::Render()
{
	if (m_bShowScriptOverlay)
	{
		Renderer::Get().SetBlendMode( GFX_BLENDMODE_LERP );
		m_aobScriptOverlay[0].Render();
		m_aobScriptOverlay[1].Render();
		Renderer::Get().SetBlendMode( GFX_BLENDMODE_OVERWRITE );
	}


	CVector obColour = CGuiManager::Get().GuiSettings()->DefaultTextColour();

	for (int i = 0; i < NUM_SLOTS; i++)
	{
		if (m_apobString[i])
		{
			obColour.W() = min(1.0f, m_afFadeTimers[i]);

			m_apobString[i]->SetColour(obColour);
			m_apobString[i]->Render();
		}
	}

#ifdef DEBUG_NAV_FADE
	g_VisualDebug->Printf2D(500, 20, 0xffffffff, 0, "m_afFadeTimers[SLOT_SELECT]: %f", m_afFadeTimers[SLOT_SELECT]);
	g_VisualDebug->Printf2D(500, 35, 0xffffffff, 0, "m_abFadeWaiting[SLOT_SELECT]: %d", m_abFadeWaiting[SLOT_SELECT]);
	g_VisualDebug->Printf2D(500, 50, 0xffffffff, 0, "m_abFadeDir[SLOT_SELECT]: %d", m_abFadeDir[SLOT_SELECT]);
	g_VisualDebug->Printf2D(500, 70, 0xffffffff, 0, "m_afFadeTimers[SLOT_BACK]: %f", m_afFadeTimers[SLOT_BACK]);
	g_VisualDebug->Printf2D(500, 85, 0xffffffff, 0, "m_abFadeDir[SLOT_BACK]: %d", m_abFadeDir[SLOT_BACK]);
	g_VisualDebug->Printf2D(500, 100, 0xffffffff, 0, "m_abFadeWaiting[SLOT_BACK]: %d", m_abFadeWaiting[SLOT_BACK]);
	g_VisualDebug->Printf2D(500, 120, 0xffffffff, 0, "m_afFadeTimers[SLOT_DESCRIPTION]: %f", m_afFadeTimers[SLOT_DESCRIPTION]);
	g_VisualDebug->Printf2D(500, 135, 0xffffffff, 0, "m_abFadeDir[SLOT_DESCRIPTION]: %d", m_abFadeDir[SLOT_DESCRIPTION]);
	g_VisualDebug->Printf2D(500, 150, 0xffffffff, 0, "m_abFadeWaiting[SLOT_DESCRIPTION]: %d", m_abFadeWaiting[SLOT_DESCRIPTION]);
#endif

}

void GuiNavigationBar::Update()
{
	float fTimerChange = CTimer::Get().GetSystemTimeChange() * (1.0f / CGuiManager::Get().GuiSettings()->NavBar()->FadeTime());
	for (int i = 0; i < NUM_SLOTS; i++)
	{
		if (m_abFadeWaiting[i] == false)	//ie, running
		{
			if (m_abFadeDir[i] == true)	//fade in
			{
				m_afFadeTimers[i] += fTimerChange;
				if (m_afFadeTimers[i] > 1.0f)
				{
					m_afFadeTimers[i] = 1.0f;
					m_abFadeWaiting[i] = true;
				}
			}
			else /*if (m_abFadeDir[i] == false) */	//fade out
			{
				m_afFadeTimers[i] -= fTimerChange;
				if (m_afFadeTimers[i] < 0.0f)
				{
					m_afFadeTimers[i] = 0.0f;
					m_abFadeWaiting[i] = true;
					DestroyString((SLOT)i);
				}
			}
		}
	}

}

void GuiNavigationBar::CreateOverlay()
{
	CPoint obPos( m_pobTransform->GetLocalTranslation() );
	CPoint obSize( CGuiManager::Get().GuiSettings()->NavBar()->ScriptSize() );
	CPoint obOffset( CGuiManager::Get().GuiSettings()->NavBar()->ScriptOffset() );
	float fHalfX = obSize.X()*0.5f;

	m_aobScriptOverlay[0].SetTexture( CGuiManager::Get().GuiSettings()->NavBar()->ScriptLeftTexture() );
	m_aobScriptOverlay[0].SetWidth( obSize.X() );
	m_aobScriptOverlay[0].SetHeight( obSize.Y() );
	m_aobScriptOverlay[0].SetPosition( obPos - CPoint(fHalfX, 0, 0) + obOffset );

	m_aobScriptOverlay[1].SetTexture( CGuiManager::Get().GuiSettings()->NavBar()->ScriptRightTexture() );
	m_aobScriptOverlay[1].SetWidth( obSize.X() );
	m_aobScriptOverlay[1].SetHeight( obSize.Y() );
	m_aobScriptOverlay[1].SetPosition(obPos + CPoint(fHalfX, 0, 0) + obOffset );
}

void GuiNavigationBar::CreateString( SLOT eSlot, const char* pcID )
{
	//destroy string
	DestroyString(eSlot);

	//Create new
	if (pcID != NULL)
	{
		//for safety, we need to reassign the font. otherwise the font pointer could be dangling from after a language switch
		CFont* pobFont = GuiUtil::GetFontFromDescription( CGuiManager::Get().GuiSettings()->NavBar()->TextFont() );
		m_aobStringDef[eSlot].m_pobFont = pobFont;

		m_apobString[eSlot] = CStringManager::Get().MakeString(pcID, m_aobStringDef[eSlot], m_pobTransform, CGuiUnit::RENDER_SCREENSPACE);
	}
}

void GuiNavigationBar::DestroyString( SLOT eSlot )
{
	if (!m_apobString[eSlot])
		return;

	CStringManager::Get().DestroyString(m_apobString[eSlot]);
	m_apobString[eSlot] = NULL;
}

void GuiNavigationBar::CreateTransform()
{
	if (m_pobTransform)
		return;

	CPoint obBasePoint(CGuiManager::Get().GuiSettings()->NavBar()->BasePosition());

	obBasePoint.X() *= CGuiManager::Get().BBWidth();
	obBasePoint.Y() *= CGuiManager::Get().BBHeight();

	// Create a matrix with the point
	CMatrix obBaseMatrix( CONSTRUCT_IDENTITY );
	obBaseMatrix.SetTranslation( obBasePoint );

	// Now set our transform object out of all that
	m_pobTransform = NT_NEW Transform();
	m_pobTransform->SetLocalMatrix( obBaseMatrix );

	CHierarchy::GetWorld()->GetRootTransform()->AddChild( m_pobTransform );
}

void GuiNavigationBar::DestroyTransform()
{
	if (!m_pobTransform)
		return;

	m_pobTransform->RemoveFromParent();
	NT_DELETE( m_pobTransform );
	m_pobTransform = NULL;
}

void GuiNavigationBar::NotifyFade()
{
	for (int i = 0; i < NUM_SLOTS; i++)
	{
		if (m_abFadeWaiting[i])
			m_abFadeWaiting[i] = false;
	}
}

/***************************************************************************************************
*
*	FUNCTION		CGuiSkinMenuNavigationBar::CGuiSkinMenuNavigationBar
*
*	DESCRIPTION		Construction
*
***************************************************************************************************/

CGuiSkinMenuNavigationBar::CGuiSkinMenuNavigationBar( void )
{
//	CGuiManager::Get().NavigationBar()->NotifyActivated();

	m_bShowScriptOverlay = false;

	m_bNotifiedFadeIn = false;
	m_bNotifiedFadeOut = false;

	m_pcDescTitleID = NULL;
}

/***************************************************************************************************
*
*	FUNCTION		CGuiSkinMenuNavigationBar::~CGuiSkinMenuNavigationBar
*
*	DESCRIPTION		Destruction
*
***************************************************************************************************/

CGuiSkinMenuNavigationBar::~CGuiSkinMenuNavigationBar( void )
{
//	CGuiManager::Get().NavigationBar()->NotifyDeactivated();
	NT_DELETE_ARRAY(m_pcDescTitleID);
}


/***************************************************************************************************
*
*	FUNCTION		CGuiSkinMenuNavigationBar::ProcessAttribute
*
*	DESCRIPTION		Passes attribute values on to specific handlers based on their title.
***************************************************************************************************/

bool CGuiSkinMenuNavigationBar::ProcessAttribute( const char* pcTitle, const char* pcValue )
{
	if ( !super::ProcessAttribute( pcTitle, pcValue ) )
	{
		if ( strcmp( pcTitle, "descriptiontitleid" ) == 0 )
		{
			NT_DELETE_ARRAY(m_pcDescTitleID);
			m_pcDescTitleID = NULL;
			if (strlen(pcValue) == 0)
			{
				m_abShow[GuiNavigationBar::SLOT_DESCRIPTION] = false;
			}
			else
			{
				m_abShow[GuiNavigationBar::SLOT_DESCRIPTION] = true;
				GuiUtil::SetString(pcValue, &m_pcDescTitleID);
			}
			return true;
		}
		else if ( strcmp( pcTitle, "showselect" ) == 0 )
		{
			return GuiUtil::SetBool(pcValue, &m_abShow[GuiNavigationBar::SLOT_SELECT]);
		}
		else if ( strcmp( pcTitle, "showback" ) == 0 )
		{
			return GuiUtil::SetBool(pcValue, &m_abShow[GuiNavigationBar::SLOT_BACK]);
		}
		else if ( strcmp( pcTitle, "showscriptoverlay" ) == 0 )
		{
			GuiUtil::SetBool(pcValue, &m_bShowScriptOverlay);
			CGuiManager::Get().NavigationBar()->ShowScriptOverlay(m_bShowScriptOverlay);
			return true;
		}

		return false;
	}

	return true;
}

/***************************************************************************************************
*
*	FUNCTION		CGuiSkinMenuNavigationBar::ProcessEnd
*
*	DESCRIPTION		Carries out setup tasks that are valid only after we know all attributes are set
*
***************************************************************************************************/

bool CGuiSkinMenuNavigationBar::ProcessEnd( void )
{
	super::ProcessEnd();

	if (!m_bShowScriptOverlay)
		CGuiManager::Get().NavigationBar()->ShowScriptOverlay(false);

	SyncAttributes();

	return true;
}

bool CGuiSkinMenuNavigationBar::SetData(const char* pcName, void* pvData)
{
	if (!super::SetData(pcName, pvData))
	{
		if (0 == strcmp(pcName, "syncattributes"))
		{
			SyncAttributes();

			//if done this way then we want to trigger a fade.
			CGuiManager::Get().NavigationBar()->NotifyFade();

			return true;
		}
	}
	return false;
}

void CGuiSkinMenuNavigationBar::SyncAttributes()
{
	CGuiManager::Get().NavigationBar()->ShowTextSlot(GuiNavigationBar::SLOT_SELECT, m_abShow[GuiNavigationBar::SLOT_SELECT]);
	CGuiManager::Get().NavigationBar()->ShowTextSlot(GuiNavigationBar::SLOT_BACK, m_abShow[GuiNavigationBar::SLOT_BACK]);
	CGuiManager::Get().NavigationBar()->ShowTextSlot(GuiNavigationBar::SLOT_DESCRIPTION, m_abShow[GuiNavigationBar::SLOT_DESCRIPTION], m_pcDescTitleID);
}
void CGuiSkinMenuNavigationBar::UpdateFadeIn()
{
	super::UpdateFadeIn();

	if (!m_bNotifiedFadeIn && ScreenFade() > 0.0f)
	{
		CGuiManager::Get().NavigationBar()->NotifyFade();
		m_bNotifiedFadeIn = true;
	}
}

void CGuiSkinMenuNavigationBar::UpdateFadeOut()
{
	super::UpdateFadeOut();

	if (!m_bNotifiedFadeOut && ScreenFade() < 1.0f)
	{
		for (int i = 0; i < GuiNavigationBar::NUM_SLOTS; i++)
		{
			m_abShow[i] = false;
		}

		SyncAttributes();

		CGuiManager::Get().NavigationBar()->NotifyFade();
		m_bNotifiedFadeOut = true;
	}
}
