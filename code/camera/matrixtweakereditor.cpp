/***************************************************************************************************
*
*	DESCRIPTION
*
*	NOTES
*
***************************************************************************************************/

// Necessary includes
#include "camera/camtools.h"
#include "camera/matrixtweakereditor.h"
#include "objectdatabase/dataobject.h"

START_CHUNKED_INTERFACE	( CMTEditorManual, Mem::MC_CAMERA )
	PUBLISH_VAR_WITH_DEFAULT_AS(m_iParentPad, 0, ParentPad)
	PUBLISH_VAR_WITH_DEFAULT_AS(m_fMaxLong, 0.0f, MaxLong)
	PUBLISH_VAR_WITH_DEFAULT_AS(m_fMaxLat, 0.0f, MaxLat)
	PUBLISH_VAR_WITH_DEFAULT_AS(m_bPreserveY, false, PreserveY)
	PUBLISH_VAR_WITH_DEFAULT_AS(m_bReverseY, false, ReverseY)
	DECLARE_POSTCONSTRUCT_CALLBACK( PostConstruct )
	DECLARE_EDITORCHANGEVALUE_CALLBACK( EditorChangeValue )
END_STD_INTERFACE

START_CHUNKED_INTERFACE	( CMTEditorElastic, Mem::MC_CAMERA )
	PUBLISH_VAR_WITH_DEFAULT_AS(m_iParentPad, 0, ParentPad)
	PUBLISH_VAR_WITH_DEFAULT_AS(m_fMaxLong, 20.0f, MaxLong)
	PUBLISH_VAR_WITH_DEFAULT_AS(m_fMaxLat, 10.0f, MaxLat)
	PUBLISH_VAR_WITH_DEFAULT_AS(m_bPreserveY, true, PreserveY)
	PUBLISH_VAR_WITH_DEFAULT_AS(m_bReverseY, false, ReverseY)
	DECLARE_POSTCONSTRUCT_CALLBACK( PostConstruct )
	DECLARE_EDITORCHANGEVALUE_CALLBACK( EditorChangeValue )
END_STD_INTERFACE




/***************************************************************************************************
*
*	FUNCTION		CMatrixTweakerEditor::CMatrixTweakerEditor
*
*	DESCRIPTION		Construction
*
***************************************************************************************************/

CMatrixTweakerEditor::CMatrixTweakerEditor( void )
:	CMatrixTweaker( CMatrixTweakerDef( "XML Created" ) ),
	m_pobMatrixTweaker( 0 ),
	m_iParentPad( 0 ),
	m_fMaxLong( 0.0f ),
	m_fMaxLat( 0.0f ),
	m_bPreserveY( false ),
	m_bReverseY( false )
{
}


/***************************************************************************************************
*
*	FUNCTION		CMatrixTweakerEditor::~CMatrixTweakerEditor
*
*	DESCRIPTION		Destruction
*
***************************************************************************************************/

CMatrixTweakerEditor::~CMatrixTweakerEditor( void )
{
	NT_DELETE_CHUNK(Mem::MC_CAMERA, m_pobMatrixTweaker );
}


/***************************************************************************************************
*
*	FUNCTION		CMatrixTweakerEditor::Reset
*
*	DESCRIPTION		If we have a valid pointer to the item we are editing then use it, otherwise
*					return something harmless - this needs to run whilst editing
*
***************************************************************************************************/

void CMatrixTweakerEditor::Reset( void )
{
	if (m_pobMatrixTweaker )
		m_pobMatrixTweaker->Reset();
}


/***************************************************************************************************
*
*	FUNCTION		CMatrixTweakerEditor::ApplyTweak
*
*	DESCRIPTION		If we have a valid pointer to the item we are editing then use it, otherwise
*					return something harmless - this needs to run whilst editing
*
***************************************************************************************************/

CMatrix CMatrixTweakerEditor::ApplyTweak( const CMatrix& obSrc, float fTimeChange )
{
	if ( m_pobMatrixTweaker )
		return m_pobMatrixTweaker->ApplyTweak( obSrc, fTimeChange );
	else
		return obSrc;
}


/***************************************************************************************************
*
*	FUNCTION		CMatrixTweakerEditor::Render
*
*	DESCRIPTION		If we have a valid pointer to the item we are editing then use it, otherwise
*					return something harmless - this needs to run whilst editing
*
***************************************************************************************************/

void CMatrixTweakerEditor::Render( void )
{
	if ( m_pobMatrixTweaker )
		m_pobMatrixTweaker->Render();
}


/***************************************************************************************************
*
*	FUNCTION		CMatrixTweakerEditor::RenderInfo
*
*	DESCRIPTION		If we have a valid pointer to the item we are editing then use it, otherwise
*					return something harmless - this needs to run whilst editing
*
***************************************************************************************************/

void CMatrixTweakerEditor::RenderInfo( int iX, int iY )
{
	if ( m_pobMatrixTweaker )
		m_pobMatrixTweaker->RenderInfo( iX, iY );
}


/***************************************************************************************************
*
*	FUNCTION		CMatrixTweakerEditor::Dump
*
*	DESCRIPTION		If we have a valid pointer to the item we are editing then use it, otherwise
*					return something harmless - this needs to run whilst editing
*
***************************************************************************************************/

const char* CMatrixTweakerEditor::Dump( void )
{
	return "";
}


/***************************************************************************************************
*
*	FUNCTION		CMatrixTweakerEditor::GetLastSrc
*
*	DESCRIPTION		If we have a valid pointer to the item we are editing then use it, otherwise
*					return something harmless - this needs to run whilst editing
*
***************************************************************************************************/

CMatrix CMatrixTweakerEditor::GetLastSrc( void )
{
	if ( m_pobMatrixTweaker )
		return m_pobMatrixTweaker->GetLastSrc();
	else
		return CMatrix( CONSTRUCT_CLEAR );
}


/***************************************************************************************************
*
*	FUNCTION		CMatrixTweakerEditor::GetLastTweaked
*
*	DESCRIPTION		If we have a valid pointer to the item we are editing then use it, otherwise
*					return something harmless - this needs to run whilst editing
*
***************************************************************************************************/

CMatrix CMatrixTweakerEditor::GetLastTweaked( void )
{
	if ( m_pobMatrixTweaker )
		return m_pobMatrixTweaker->GetLastTweaked();
	else
		return CMatrix( CONSTRUCT_CLEAR );
}


/***************************************************************************************************
*
*	FUNCTION		CMatrixTweakerEditor::EditorChangeValue
*
*	DESCRIPTION		
*
***************************************************************************************************/

bool CMatrixTweakerEditor::EditorChangeValue( CallBackParameter, CallBackParameter )
{
	// If the data has changed then we 
	// completely recreate our tweaker
	ClearTweaker();

	// Recreate the controller with the new data
	CreateTweaker();

	// Return true - this just confirms that this has tried something
	return true;
}


/***************************************************************************************************
*
*	FUNCTION		CMatrixTweakerEditor::CreateTweakerDef
*
*	DESCRIPTION		Construction
*
***************************************************************************************************/

void CMatrixTweakerEditor::CreateTweakerDef( CMatrixTweakerDef& obDef )
{
	// Set all the detail to the defining structure
	obDef.SetPad( ( PAD_NUMBER )m_iParentPad );
	obDef.SetMaxLong( m_fMaxLong );
	obDef.SetMaxLat( m_fMaxLat );
	obDef.SetYPreserve( m_bPreserveY );
	obDef.SetYReverse( m_bReverseY );
}


/***************************************************************************************************
*
*	FUNCTION		CMTEditorManual::CMTEditorManual
*
*	DESCRIPTION		Construction
*
***************************************************************************************************/

CMTEditorManual::CMTEditorManual( void )
{
}


/***************************************************************************************************
*
*	FUNCTION		CMTEditorManual::~CMTEditorManual
*
*	DESCRIPTION		Destruction
*
***************************************************************************************************/

CMTEditorManual::~CMTEditorManual( void )
{
}


/***************************************************************************************************
*
*	FUNCTION		CMTEditorManual::CreateTweaker
*
*	DESCRIPTION		
*
***************************************************************************************************/

void CMTEditorManual::CreateTweaker( void )
{
	// Create our definition structure
	CMatrixTweakerDef obDef(  ntStr::GetString(ObjectDatabase::Get().GetNameFromPointer( this ) ) );
	CreateTweakerDef( obDef );

	// Create our contained object
	m_pobMatrixTweaker = NT_NEW_CHUNK( Mem::MC_CAMERA ) CCameraManualTweaker( obDef );
}


/***************************************************************************************************
*
*	FUNCTION		CMTEditorElastic::CMTEditorElastic
*
*	DESCRIPTION		Construction
*
***************************************************************************************************/

CMTEditorElastic::CMTEditorElastic( void )
{
}


/***************************************************************************************************
*
*	FUNCTION		CMTEditorElastic::~CMTEditorElastic
*
*	DESCRIPTION		Destruction
*
***************************************************************************************************/

CMTEditorElastic::~CMTEditorElastic( void )
{
}


/***************************************************************************************************
*
*	FUNCTION		CMTEditorElastic::CreateTweaker
*
*	DESCRIPTION		
*
***************************************************************************************************/

void CMTEditorElastic::CreateTweaker( void )
{
	// Create our definition structure
	CMatrixTweakerDef obDef(  ntStr::GetString(ObjectDatabase::Get().GetNameFromPointer( this )) );
	CreateTweakerDef( obDef );

	// Create our contained object
	m_pobMatrixTweaker = NT_NEW_CHUNK( Mem::MC_CAMERA ) CCameraElasticTweaker( obDef );
}


