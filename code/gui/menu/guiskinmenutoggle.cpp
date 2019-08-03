/***************************************************************************************************
*
*	DESCRIPTION		
*
*	NOTES			
*
***************************************************************************************************/

// Includes
#include "guiskinmenutoggle.h"
#include "gui/guimanager.h"

#include "gui/guilua.h"
#include "game/playeroptions.h"
#include "gui/guiscreen.h"
#include "gui/guisettings.h"

/***************************************************************************************************
*
*	The bobbins below allows us to register this class along with its XML tag before main is called.
*	This means that the parts constructing it don't have to know about it.
*
***************************************************************************************************/

static CXMLElement* ConstructWrapper( void ) { return NT_NEW CGuiSkinMenuToggle(); }

// Register this class under it's XML tag
bool g_bMENUTOGGLE = CGuiManager::Register( "MENUTOGGLE", &ConstructWrapper );

/***************************************************************************************************
*
*	FUNCTION		CGuiSkinMenuToggle::CGuiSkinMenuToggle
*
*	DESCRIPTION		Construction
*
***************************************************************************************************/

CGuiSkinMenuToggle::CGuiSkinMenuToggle( void )
	: m_pcOnString(NULL)
	, m_pcOffString(NULL)
	, m_pcCallback(NULL)
	, m_bOn(false)
	, m_pobValueString(NULL)
	, m_pOtherString(NULL)
	, m_fValuePositionX( 0.0f )
	, m_fGapBetweenText( 0.0f )
{}


/***************************************************************************************************
*
*	FUNCTION		CGuiSkinMenuToggle::~CGuiSkinMenuToggle
*
*	DESCRIPTION		Destruction
*
***************************************************************************************************/

CGuiSkinMenuToggle::~CGuiSkinMenuToggle( void )
{
	NT_DELETE_ARRAY(m_pcOnString);
	NT_DELETE_ARRAY(m_pcOffString);
	NT_DELETE_ARRAY(m_pcCallback);

	if (m_pobValueString)
		CStringManager::Get().DestroyString(m_pobValueString);

	if( NULL != m_pOtherString )
		CStringManager::Get().DestroyString( m_pOtherString );
}


/***************************************************************************************************
*
*	FUNCTION		CGuiSkinMenuToggle::ProcessAttribute
*
*	DESCRIPTION		Passes attribute values on to specific handlers based on their title.
*
***************************************************************************************************/

bool CGuiSkinMenuToggle::ProcessAttribute( const char* pcTitle, const char* pcValue )
{
	if ( !super::ProcessAttribute( pcTitle, pcValue ) )
	{
		if ( strcmp( pcTitle, "on" ) == 0 )
		{
			return GuiUtil::SetString( pcValue, &m_pcOnString);
		}
		else if ( strcmp( pcTitle, "off" ) == 0 )
		{
			return GuiUtil::SetString( pcValue, &m_pcOffString );
		}
		else if ( strcmp( pcTitle, "callback" ) == 0 )
		{
			return GuiUtil::SetString( pcValue, &m_pcCallback);
		}
		else if ( strcmp( pcTitle, "playeroptionsbool" ) == 0 )
		{
			m_bOn = CPlayerOptions::Get().GetBool( pcValue );
			return true;
		}
		else if ( strcmp( pcTitle, "valuebaseposition" ) == 0 )
		{
			ProcessValueBasePositionValue( pcValue );
			return true;
		}
		else if( strcmp( pcTitle, "textgap" ) == 0 )
		{
			GuiUtil::SetFloat( pcValue, &m_fGapBetweenText );
			return true;
		}

		return false;
	}

	return true;
}

/***************************************************************************************************
*
*	FUNCTION		CGuiSkinMenuToggle::ProcessEnd
*
*	DESCRIPTION		Carries out setup tasks that are valid only after we know all attributes are set
*
***************************************************************************************************/

bool CGuiSkinMenuToggle::ProcessEnd( void )
{
	ntAssert(m_pcOnString);
	ntAssert(m_pcOffString);
	ntAssert(m_pcCallback);

	super::ProcessEnd();

	CreateValueString();

	return true;
}

/***************************************************************************************************
*
*	FUNCTION		CGuiSkinMenuToggle::MoveLeftAction
*
*	DESCRIPTION		
*
***************************************************************************************************/

bool CGuiSkinMenuToggle::MoveLeftAction( int iPads )
{
	UNUSED(iPads);
	Toggle();
	return true;
}

/***************************************************************************************************
*
*	FUNCTION		CGuiSkinMenuToggle::MoveRightAction
*
*	DESCRIPTION		Triggers lua callback
*
***************************************************************************************************/

bool CGuiSkinMenuToggle::MoveRightAction( int iPads )
{
	UNUSED(iPads);
	Toggle();
	return true;
}

/***************************************************************************************************
*
*	FUNCTION		CGuiSkinMenuToggle::StartAction
*
*	DESCRIPTION		Triggers lua callback
*
***************************************************************************************************/

bool CGuiSkinMenuToggle::StartAction( int iPads )
{
	UNUSED(iPads);
	Toggle();
	return true;
}

/***************************************************************************************************
*
*	FUNCTION		CGuiSkinMenuToggle::SelectAction
*
*	DESCRIPTION		Triggers lua callback
*
***************************************************************************************************/

bool CGuiSkinMenuToggle::SelectAction( int iPads )
{
	UNUSED(iPads);
	Toggle();
	return true;
}

/***************************************************************************************************
*
*	FUNCTION		CGuiSkinMenuToggle::CreateValueString
*
*	DESCRIPTION		[Re]creates the string which holds and displays the toggle's value.
*
***************************************************************************************************/

void CGuiSkinMenuToggle::CreateValueString()
{
	if (m_pobValueString)
		CStringManager::Get().DestroyString(m_pobValueString);
	if( NULL != m_pOtherString )
		CStringManager::Get().DestroyString( m_pOtherString );

	CStringDefinition obDef = StringDef();
	obDef.m_fXOffset += m_fValuePositionX - m_pobBaseTransform->GetLocalTranslation().X();
	obDef.m_eHJustify = CStringDefinition::JUSTIFY_LEFT;
	m_pobValueString = CStringManager::Get().MakeString(m_pcOnString, obDef, m_pobBaseTransform, m_eRenderSpace);

	super::GetExtents(m_obExtents);
	m_obExtents.fWidth += obDef.m_fXOffset + m_pobValueString->RenderWidth();

	//make the other string
	CStringDefinition obDefOther = StringDef();
	obDefOther.m_fXOffset += m_fValuePositionX - m_pobBaseTransform->GetLocalTranslation().X() + m_pobValueString->RenderWidth() + m_fGapBetweenText;
	obDefOther.m_eHJustify = CStringDefinition::JUSTIFY_LEFT;
	m_pOtherString = CStringManager::Get().MakeString( m_pcOffString, obDefOther, m_pobBaseTransform, m_eRenderSpace);

	m_obExtents.fWidth += m_fGapBetweenText + m_pOtherString->RenderWidth();

	//set intial colour, would do this in a function but the there is already a set colours one so it would get confusing
	CVector SelectedColour = CGuiManager::Get().GuiSettings()->DefaultTextColour();
	CVector DisabledColour = CGuiManager::Get().GuiSettings()->DefaultTextDisabledColour();
	if( m_bOn )
	{
		m_pobValueString->SetColour( SelectedColour );
		m_pOtherString->SetColour( DisabledColour );
	}
	else
	{
		m_pobValueString->SetColour( DisabledColour );
		m_pOtherString->SetColour( SelectedColour );
	}

}

/***************************************************************************************************
*
*	FUNCTION		CGuiSkinMenuToggle::Toggle
*
*	DESCRIPTION		Flip our state and notify our lua callback
*
***************************************************************************************************/

void CGuiSkinMenuToggle::Toggle()
{
	m_bOn ^= true;

	//CreateValueString();

	CVector SelectedColour = CGuiManager::Get().GuiSettings()->DefaultTextSelectedColour();
	CVector DisabledColour = CGuiManager::Get().GuiSettings()->DefaultTextDisabledColour();
	if( m_bOn )
	{
		m_pobValueString->SetColour( SelectedColour );
		m_pOtherString->SetColour( DisabledColour );
	}
	else
	{
		m_pobValueString->SetColour( DisabledColour );
		m_pOtherString->SetColour( SelectedColour );
	}

	NinjaLua::LuaFunction luaFunc = CGuiLua::GetLuaFunction(m_pcCallback);
	if (!luaFunc.IsNil())
	{
        luaFunc(m_bOn);
	}
	else
	{
		ntPrintf("CGuiSkinMenuToggle: Failed to find lua function \"%s\"\n", m_pcCallback);
	}
}

/***************************************************************************************************
*
*	FUNCTION		CGuiSkinMenuToggle::RenderText
*
*	DESCRIPTION		
*
***************************************************************************************************/

bool CGuiSkinMenuToggle::Render()
{
	super::Render();

	if	( m_pobValueString )
	{
		if	( IsFading() )
		{
			CGuiSkinFader::FadeStringObject( m_pobValueString, ScreenFade() );
			CGuiSkinFader::FadeStringObject( m_pOtherString, ScreenFade() );
		}

		m_pobValueString->Render();
		m_pOtherString->Render();
	}

	return true;
}

/***************************************************************************************************
*
*	FUNCTION		CGuiSkinMenuToggle::ProcessValueBasePositionValue
*
*	DESCRIPTION		Process the position to display the toggle value string.
*
***************************************************************************************************/

void CGuiSkinMenuToggle::ProcessValueBasePositionValue( const char* pcValue )
{
	// We'll get three floats from the string
	float fX, fY, fZ;
	int iResult = sscanf( pcValue, "%f,%f,%f", &fX, &fY, &fZ ); 

	// Make sure we extracted three values
	ntAssert( iResult == 3 );
	UNUSED( iResult );

	// Create a point with the data
	CPoint obGfxBasePoint( fX, fY, fZ );

	// Calc and store the position in screen space.
	m_fValuePositionX = obGfxBasePoint.X() * CGuiManager::Get().BBWidth();
}

/***************************************************************************************************
*
*	FUNCTION		CGuiSkinMenuToggle::SetTextColour
*
*	DESCRIPTION		Process the position to display the toggle value string.
*
***************************************************************************************************/

void CGuiSkinMenuToggle::SetTextColour( const CVector &obColour )
{
	super::SetTextColour(obColour);
	if( true == m_bOn )
	{
		if (m_pobValueString)
			m_pobValueString->SetColour(const_cast<CVector&>(obColour));
	}
	else
	{
		if (m_pOtherString)
			m_pOtherString->SetColour(const_cast<CVector&>(obColour));
	}

	
}

void CGuiSkinMenuToggle::GetExtents(GuiExtents& obExtents)
{
	obExtents = m_obExtents;
}

