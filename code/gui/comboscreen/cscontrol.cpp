/***************************************************************************************************
*
*	DESCRIPTION		This is the class that does all the fancy stuff for the combo screen.
*
*	NOTES		This is the Grand master of all
*	This makes, fills, controls and arbitrates the entire combo menu. It pulls all the strings, holds all the keys and sees all.
*
***************************************************************************************************/

// Includes
#include "cscontrol.h"
//#include "cslistobjecttext.h"
#include "cslistobjectcombobuttons.h"
#include "gui/guimanager.h"
#include "game/entitymanager.h"
#include "gui/guisettings.h"
#include "game/hitcounter.h"

static const float		kLeftXPosistion				= 220.0f;
static const float		kTopYPosistion				= 200.0f;
static const float		kRightXPosistion			= 1100.0f;
static const float		kBottomYPosistion			= 550.0f;
static const float		kTopYInfoBoxPosistion		= 540.0f;
static const float		kBottomYInfoBoxPosistion	= 620.0f;
static const float		kNewTagXPosistion			= 510.0f;
static const float		kComboTagXPosistion			= 720.0f;
static const float		kComboTagSuperXPosistion	= 480.0f;
static const float		kComboTrippleXPosistion		= 520.0f;
static const int		kNumItemsInListToShow		= 6;
static const float		kItemDistance				= 50.0f;
static const float		kMultiCtrlX					= 640.0f;
static const float		kMultiCtrlY					= 130.0f;
static const float		kMultiCtrlLeftArrow			= 350.0f;
static const float		kMultiCtrlRightArrow		= 970.0f;
static const float		kComboSupperNewXPosition	= 1000.0f;



static CXMLElement* ConstructWrapper( void ) { return NT_NEW CSControl(); }

// Register this class under it's XML tag
bool g_bCSLISTCONTROL= CGuiManager::Register( "COMBOCTRL", &ConstructWrapper );


/***************************************************************************************************
*
*	DESCRIPTION		These are the move data struct registering functions.
*
*	NOTES			
*
***************************************************************************************************/
void CS_MOVE_INFO_BASIC::RegisterBasicMove( void )
{
	CGuiManager::Get().GetComboScreenMoveInfoBasicList()->push_back( this );
}

void CS_MOVE_INFO_SPEED::RegisterSpeedMove( void )
{
	CGuiManager::Get().GetComboScreenMoveInfoSpeedList()->push_back( this );
}

void CS_MOVE_INFO_RANGE::RegisterRangeMove( void )
{
	CGuiManager::Get().GetComboScreenMoveInfoRangeList()->push_back( this );
}

void CS_MOVE_INFO_POWER::RegisterPowerMove( void )
{
	CGuiManager::Get().GetComboScreenMoveInfoPowerList()->push_back( this );
}

void CS_MOVE_INFO_AERIAL::RegisterPowerMove( void )
{
	CGuiManager::Get().GetComboScreenMoveInfoAerialList()->push_back( this );
}

void CS_MOVE_INFO_SUPERSTYLE::RegisterPowerMove( void )
{
	CGuiManager::Get().GetComboScreenMoveInfoSuperStyleList()->push_back( this );
}

/***************************************************************************************************
*
*	DESCRIPTION		These are the move data struct XML Descriptors.
*
*	NOTES			
*
***************************************************************************************************/

START_CHUNKED_INTERFACE(CS_MOVE_INFO_BASIC,Mem::MC_MISC)
	PUBLISH_VAR_AS(cpName, Name)
	PUBLISH_VAR_AS(cpMoveDesc, MoveInfo)
	PUBLISH_PTR_AS(pComboUnlockData, ComboLink)
	DECLARE_POSTCONSTRUCT_CALLBACK(RegisterBasicMove)
END_STD_INTERFACE 

START_CHUNKED_INTERFACE(CS_MOVE_INFO_SPEED,Mem::MC_MISC)
	PUBLISH_VAR_AS(cpName, Name)
	PUBLISH_VAR_AS(cpMoveDesc, MoveInfo)
	PUBLISH_PTR_AS(pComboUnlockData, ComboLink)
	DECLARE_POSTCONSTRUCT_CALLBACK(RegisterSpeedMove)
END_STD_INTERFACE 

START_CHUNKED_INTERFACE(CS_MOVE_INFO_RANGE,Mem::MC_MISC)
	PUBLISH_VAR_AS(cpName, Name)
	PUBLISH_VAR_AS(cpMoveDesc, MoveInfo)
	PUBLISH_PTR_AS(pComboUnlockData, ComboLink)
	DECLARE_POSTCONSTRUCT_CALLBACK(RegisterRangeMove)
END_STD_INTERFACE 

START_CHUNKED_INTERFACE(CS_MOVE_INFO_POWER,Mem::MC_MISC)
	PUBLISH_VAR_AS(cpName, Name)
	PUBLISH_VAR_AS(cpMoveDesc, MoveInfo)
	PUBLISH_PTR_AS(pComboUnlockData, ComboLink)
	DECLARE_POSTCONSTRUCT_CALLBACK(RegisterPowerMove)
END_STD_INTERFACE 

START_CHUNKED_INTERFACE(CS_MOVE_INFO_AERIAL,Mem::MC_MISC)
	PUBLISH_VAR_AS(cpName, Name)
	PUBLISH_VAR_AS(cpMoveDesc, MoveInfo)
	PUBLISH_PTR_AS(pComboUnlockData, ComboLink)
	DECLARE_POSTCONSTRUCT_CALLBACK(RegisterPowerMove)
END_STD_INTERFACE 

START_CHUNKED_INTERFACE(CS_MOVE_INFO_SUPERSTYLE,Mem::MC_MISC)
	PUBLISH_VAR_AS(cpName, Name)
	PUBLISH_VAR_AS(cpMoveDesc, MoveInfo)
	PUBLISH_PTR_AS(pComboUnlockData, ComboLink)
	DECLARE_POSTCONSTRUCT_CALLBACK(RegisterPowerMove)
END_STD_INTERFACE 

/***************************************************************************************************
*
*	FUNCTION		CSControl::CSControl
*
*	DESCRIPTION		Construction
*
***************************************************************************************************/

CSControl::CSControl( void )
{

	m_bHasSeenBasic		= true;
	m_bHasSeenRange		= false;
	m_bHasSeenSpeed		= false;
	m_bHasSeenPower		= false;
	m_bHasSeenAerial	= false;
	m_bHasSeenSuper		= false;
}

/***************************************************************************************************
*
*	FUNCTION		CSControl::CSControl
*
*	DESCRIPTION		Destruction
*
***************************************************************************************************/

CSControl::~CSControl( void )
{
	//clean up the transforms
	m_TempTransform.RemoveFromParent();
	m_MultiTranform.RemoveFromParent();

	//if we are not in purgatory, update our points then.
	if( NULL == CEntityManager::Get().FindEntity( "PurgatoryManager" ) )
	{
		//clear the current new states of combos.
		StanceStylePoints TempPoints = CheckpointManager::Get().GetUniqueSaveData().GetStylePointsUsedForComboMenu(); 
		StanceStylePoints NewPoints;
		StyleManager::Get().GetCurrentStylePoints( NewPoints );

		//did the player look at the basic screen?
		if( m_bHasSeenBasic )
		{
			//we update the points they last viewed at to the current points
			TempPoints.m_iStylePointsMisc = NewPoints.m_iStylePointsMisc;		
		}
		//did the player look at.......
		if( m_bHasSeenRange )
		{
			TempPoints.m_iStylePointsRange = NewPoints.m_iStylePointsRange;
		}
		if( m_bHasSeenSpeed )
		{
			TempPoints.m_iStylePointsSpeed = NewPoints.m_iStylePointsSpeed;
		}
		if( m_bHasSeenPower )
		{
			TempPoints.m_iStylePointsPower = NewPoints.m_iStylePointsPower;
		}
		if( m_bHasSeenAerial )
		{
			TempPoints.m_iStylePointsAerial = NewPoints.m_iStylePointsAerial;
		}
		//Um yeah this is going to need some work
		if( m_bHasSeenSuper )
		{
			//hmm ooh dear what do I do here? martin.....
			CheckpointManager::Get().GetUniqueSaveData().SetSuperStyleLevel( StyleManager::Get().GetHitCounter()->GetCurrentStyleProgressionLevel() );
		}

		//store the points as last seen till next time.
		CheckpointManager::Get().GetUniqueSaveData().SetStylePointsUsedForComboMenu( TempPoints );
	}
}

/***************************************************************************************************
*
*	FUNCTION		CSControl::Render
*
*	DESCRIPTION		This draws everthing
*
***************************************************************************************************/

bool	CSControl::Render( void )
{
	Renderer::Get().SetBlendMode( GFX_BLENDMODE_LERP );

	//draw the windows
	for( int j = 0 ; j < 9 ; ++j )
	{
		m_aobBackgroundWindow1[j].Render();
	}
	for( int j = 0 ; j < 9 ; ++j )
	{
		m_aobBackgroundWindow2[j].Render();
	}

	Renderer::Get().SetBlendMode( GFX_BLENDMODE_OVERWRITE );

	//draw the control
	m_ListControl.Render();
	//draw the stance string
	m_MultiControl.Render();
	//draw the list box
	m_Info.Render();

	return true;
}

/***************************************************************************************************
*
*	FUNCTION		CSControl::MoveLeftAction
*
*	DESCRIPTION		This will check to see if we can move left a page, if so, tell the control to update
*					then change the pages over. If not don't do anything.
*
***************************************************************************************************/

bool	CSControl::MoveLeftAction( int /*iPads */)
{
	m_MultiControl.MovePrev();
	return true;
}

/***************************************************************************************************
*
*	FUNCTION		CSControl::MoveRightAction
*
*	DESCRIPTION		This will check to see if we can move right a page, if so, tell the control to update
*					then change the pages over. If not don't do anything.
*
***************************************************************************************************/

bool	CSControl::MoveRightAction( int /*iPads */)
{
	m_MultiControl.MoveNext();
	return true;
}

/***************************************************************************************************
*
*	FUNCTION		CSControl::MoveDownAction
*
*	DESCRIPTION		This will tell the current list object to scroll down
*
***************************************************************************************************/

bool	CSControl::MoveDownAction( int /*iPads*/ )
{
	m_ListControl.ScrollDown();
	return true;
}

/***************************************************************************************************
*
*	FUNCTION		CSControl::MoveUpAction
*
*	DESCRIPTION		This will tell the current list object to scroll up
*
***************************************************************************************************/

bool	CSControl::MoveUpAction( int /*iPads */)
{
	m_ListControl.ScrollUp();
	return true;
}

/***************************************************************************************************
*
*	FUNCTION		CSControl::BeginEnter
*
*	DESCRIPTION		Set up everything
*
***************************************************************************************************/

bool	CSControl::BeginEnter( bool bForce )
{
	//call parent
	return super::BeginEnter( bForce );
}

/***************************************************************************************************
*
*	FUNCTION		CSControl::BeginIdle
*
*	DESCRIPTION		Do the once of setting for the idle checks
*
***************************************************************************************************/

bool	CSControl::BeginIdle( bool bForce )
{
	//call parent
	return super::BeginIdle( bForce );
}

/***************************************************************************************************
*
*	FUNCTION		CSControl::BeginFocus
*
*	DESCRIPTION		Does this ever happen?
*
***************************************************************************************************/

bool	CSControl::BeginFocus( bool bForce )
{
	//call parent
	return super::BeginFocus( bForce );
}

/***************************************************************************************************
*
*	FUNCTION		CSControl::BeginExit
*
*	DESCRIPTION		Kill it all
*
***************************************************************************************************/

bool	CSControl::BeginExit( bool bForce )
{
	//call parent
	return super::BeginExit( bForce );
}

/***************************************************************************************************
*
*	FUNCTION		CSControl::UpdateIdle
*
*	DESCRIPTION		Check to see if anything has changed that needs to change
*
***************************************************************************************************/

void	CSControl::UpdateIdle( void )
{

	if( m_MultiControl.HasChanged() )
	{
		switch( m_MultiControl.GetCurrentSelection() )
		{
		case 0:
			//pump the list box with correct moves
			FillListBoxWithBasic();
			//flag that we have seen them.
			m_bHasSeenBasic = true;
			break;
		case 1:
			FillListBoxWithSpeed();
			m_bHasSeenSpeed = true;
			break;
		case 2:
			FillListBoxWithPower();
			m_bHasSeenPower = true;
			break;
		case 3:
			FillListBoxWithRange();
			m_bHasSeenRange = true;
			break;
		case 4:
			FillListBoxWithAerial();
			m_bHasSeenAerial = true;
			break;
		case 5:
			FillListBoxWithSuper();
			m_bHasSeenSuper = true;
			break;
		}
		m_MultiControl.ClearChangedState();

	}
}  

bool	CSControl::ProcessEnd( void )
{
	CGuiUnit::ProcessEnd();

	//add the tranforms
	m_pobBaseTransform->AddChild( &m_TempTransform );
	m_pobBaseTransform->AddChild( &m_MultiTranform );

	//clear the tranform
	m_TempTransform.SetLocalTranslation( CPoint( 0.0f, 0.0f, 0.0f ) );

	//set up the list control for initial display
	m_ListControl.NumItemsPerPage( kNumItemsInListToShow );
	m_ListControl.SetControlingTransform( &m_TempTransform );
	m_ListControl.SetItemHeight( kItemDistance );
	m_ListControl.SetUpAndDownArrowItems( kRightXPosistion, kTopYPosistion, kBottomYPosistion ); 
	m_ListControl.SetTopXAndYOfControl( kLeftXPosistion, kTopYPosistion );

	//set up the multi selection at the top
	m_Info.SetPosition( 255.0f + ( ( 1025.0f - 255.0f ) / 2.0f )	, kTopYInfoBoxPosistion );
	m_Info.SetSize( 1025.0f - 255.0f, kBottomYInfoBoxPosistion - kTopYInfoBoxPosistion );
	m_Info.SetTransform( &m_MultiTranform );

	//we have basic always so add it to the list box
	FillListBoxWithBasic();
	//give the multi select the basic option
	FillMultiControlWithBasic();
	//are we in purgatory?
	if( NULL == CEntityManager::Get().FindEntity( "PurgatoryManager" ) )
	{
		//make sure its the hero and not the other girl( what ever her name is ¬_¬ ) This should never happen, but just in case ^_~
		if( CEntityManager::Get().GetPlayer()->IsHero() )
		{
			//does the hero have the Heavenly sword and hence all the other sweet stances?
			if( CEntityManager::Get().GetPlayer()->ToHero()->HasHeavenlySword() )
			{
				AddStancesToMultiControl();
			}
		}
	}
	else
	{
		//when we are in purgatory we display them all.
		AddStancesToMultiControl();
	}

	//create the windows
	SetUpWindows();
	//position the left right arrows for the multi select
	m_MultiControl.SetArrowPosition( kMultiCtrlY, kMultiCtrlLeftArrow, kMultiCtrlRightArrow );

	return true;
}

void	CSControl::FlushListBox()
{
	m_ListControl.DeleteAllChildren();
	m_ListControl.JumpToStart();
}

void	CSControl::FillListBoxWithBasic()
{
	//basic has single line entries
	m_ListControl.SetItemHeight( kItemDistance );
	//we can fit 6 items on the screen this way
	m_ListControl.NumItemsPerPage( 6 );
	//get the list of all the potential moves
	ntstd::List< CS_MOVE_INFO_PARENT* >* pList = CGuiManager::Get().GetComboScreenMoveInfoBasicList();
	ntAssert( NULL != pList );
	//clear the current data
	FlushListBox();
	//pump it full of the new data
	FillListFromPointer( pList, BASIC );
	//make the seperators reflect this change
	m_ListControl.MoveSeperators();
	//set the info box
	m_Info.SetSize( 1025.0f - 255.0f, kBottomYInfoBoxPosistion - kTopYInfoBoxPosistion );
	m_Info.SetTextString( "COMBOMENU_BASIC_INFO", CStringDefinition::JUSTIFY_CENTRE, CStringDefinition::JUSTIFY_TOP );
}	  

void	CSControl::FillListBoxWithSpeed()
{
	//speed has single line entries
	m_ListControl.SetItemHeight( kItemDistance );
	//we can fit 6 items on the screen this way
	m_ListControl.NumItemsPerPage( 6 );
	//get the list of all the potential moves
	ntstd::List< CS_MOVE_INFO_PARENT* >* pList = CGuiManager::Get().GetComboScreenMoveInfoSpeedList();
	ntAssert( NULL != pList );
	//clear the current data
	FlushListBox();
	//pump it full of the new data
	FillListFromPointer( pList, SPEED );
	//make the seperators reflect this change
	m_ListControl.MoveSeperators();
	//set the info box
	m_Info.SetSize( 1025.0f - 255.0f, kBottomYInfoBoxPosistion - kTopYInfoBoxPosistion );
	m_Info.SetTextString( "COMBOMENU_SPEED_INFO", CStringDefinition::JUSTIFY_CENTRE, CStringDefinition::JUSTIFY_TOP  );
}

void	CSControl::FillListBoxWithRange()
{
	//range has double height entries
	m_ListControl.SetItemHeight( kItemDistance * 2.0f );
	//we can fit 3 double items on the screen
	m_ListControl.NumItemsPerPage( 3 );
	//get the list of all the potential moves
	ntstd::List< CS_MOVE_INFO_PARENT* >* pList = CGuiManager::Get().GetComboScreenMoveInfoRangeList();
	ntAssert( NULL != pList );
	//clear the current data
	FlushListBox();
	//pump it full of the new data
	FillListFromPointer( pList, RANGE );
	//make the seperators reflect this change
	m_ListControl.MoveSeperators();
	//set the info box
	m_Info.SetSize( 1025.0f - 255.0f, kBottomYInfoBoxPosistion - kTopYInfoBoxPosistion );
	m_Info.SetTextString( "COMBOMENU_RANGE_INFO", CStringDefinition::JUSTIFY_CENTRE, CStringDefinition::JUSTIFY_TOP  );
}

void	CSControl::FillListBoxWithPower()
{
	//Power has single line entries
	m_ListControl.SetItemHeight( kItemDistance );
	//we can fit 6 items on the screen this way
	m_ListControl.NumItemsPerPage( 6 );
	//get the list of all the potential moves
	ntstd::List< CS_MOVE_INFO_PARENT* >* pList = CGuiManager::Get().GetComboScreenMoveInfoPowerList();
	ntAssert( NULL != pList );
	//clear the current data
	FlushListBox();
	//pump it full of the new data
	FillListFromPointer( pList, POWER );
	//make the seperators reflect this change
	m_ListControl.MoveSeperators();
	//set the info box
	m_Info.SetSize( 1025.0f - 255.0f, kBottomYInfoBoxPosistion - kTopYInfoBoxPosistion );
	m_Info.SetTextString( "COMBOMENU_POWER_INFO", CStringDefinition::JUSTIFY_CENTRE, CStringDefinition::JUSTIFY_TOP  );
}

void	CSControl::FillListBoxWithAerial()
{
	//Areial has triple height items
	m_ListControl.SetItemHeight( kItemDistance * 3.0f );
	// we can fit 2 on screen
	m_ListControl.NumItemsPerPage( 2 );
	//get the list of all the potential moves
	ntstd::List< CS_MOVE_INFO_PARENT* >* pList = CGuiManager::Get().GetComboScreenMoveInfoAerialList();
	ntAssert( NULL != pList );
	//clear the current data
	FlushListBox();
	//pump it full of the new data
	FillListFromPointer( pList, AERIAL );
	//make the seperators reflect this change
	m_ListControl.MoveSeperators();
	//set the info box
	m_Info.SetSize( 1025.0f - 255.0f, kBottomYInfoBoxPosistion - kTopYInfoBoxPosistion );
	m_Info.SetTextString( "COMBOMENU_AERIAL_INFO", CStringDefinition::JUSTIFY_CENTRE, CStringDefinition::JUSTIFY_TOP  );
}

void	CSControl::FillListBoxWithSuper()
{
	//Super has hexel height items, well it is super... ¬_¬
	m_ListControl.SetItemHeight( kItemDistance * 6.0f );
	//There can only be one of them at once
	m_ListControl.NumItemsPerPage( 1 );
	//get a list of allthe potential moves
	ntstd::List< CS_MOVE_INFO_PARENT* >* pList = CGuiManager::Get().GetComboScreenMoveInfoSuperStyleList();
	ntstd::List< CS_MOVE_INFO_PARENT* >::iterator Iter = pList->begin();
	ntAssert( NULL != pList );
	//clear current data
	FlushListBox();

	//build item one  
	CSListObjectText* pTempText;

	//okay how many do we have
	HIT_LEVEL TempHL = HL_ZERO;
	HIT_LEVEL OldHL = HL_ZERO;

	//are we in purgatory
	if( NULL == CEntityManager::Get().FindEntity( "PurgatoryManager" ) )
	{
		//no, the how big is the hero's bar?
		TempHL = StyleManager::Get().GetHitCounter()->GetCurrentStyleProgressionLevel();
		OldHL = CheckpointManager::Get().GetUniqueSaveData().GetSuperStyleLevel();
	}
	else
	{
		//yes, all all of them
		TempHL	= HL_THREE;
		OldHL	= HL_THREE;
	}

	CSListObjectTri* pOne = NULL;
	CSListObjectTri* pTwo = NULL;
	CSListObjectTri* pThree = NULL;

	if( TempHL == HL_ONE ||
		TempHL == HL_TWO ||
		TempHL == HL_THREE ||
		TempHL == HL_FOUR ||
		TempHL == HL_SPECIAL )
	{

		pOne = NT_NEW CSListObjectTri;

		pTempText = NT_NEW CSListObjectText;
		pTempText->SetPosition( kLeftXPosistion, kTopYPosistion + ( 3.0f * kItemDistance ));
		pTempText->SetSize( 1000.0f, 300.0f );
		pTempText->SetTransform( &m_TempTransform );
		pTempText->SetTextString( "COMBOMENU_SUPER_STYLE_HEADING_1" );

		pOne->SetControl( pTempText, 0 );
		for( int j = 0 ; j < 4 ; ++j )
		{
			pTempText = NT_NEW CSListObjectText;
			pTempText->SetPosition( kComboTagSuperXPosistion, kTopYPosistion + ( kItemDistance * ( j + 1 ) ) );
			pTempText->SetSize( 1000.0f, 300.0f );
			pTempText->SetTransform( &m_TempTransform );
			pTempText->SetTextString( (*Iter)->cpMoveDesc.c_str() );

			pOne->SetControl( pTempText, 1 + ( j * 2 ) );

			pTempText = NT_NEW CSListObjectText;
			pTempText->SetPosition( kComboTagXPosistion, kTopYPosistion + ( kItemDistance * ( j + 1 ) ) );
			pTempText->SetSize( 1000.0f, 300.0f );
			pTempText->SetTransform( &m_TempTransform );
			pTempText->SetTextString( (*Iter)->cpName.c_str() );

			pOne->SetControl( pTempText, 2 + ( j * 2 ) );

			Iter++;
		}

		//have we just got this?
		if( TempHL > OldHL )
		{
			//add a new item then.
			CSListObjectNew* pNew = NT_NEW CSListObjectNew();
			pNew->SetTransform( &m_TempTransform );
			pNew->SetPosition( kComboSupperNewXPosition, kTopYPosistion + ( 3.0f * kItemDistance ) );
			pNew->Init();

			pOne->SetControl( pNew, 9 );
		}  

		m_ListControl.AddItem( pOne );

	}

	if( TempHL == HL_TWO ||
		TempHL == HL_THREE ||
		TempHL == HL_FOUR ||
		TempHL == HL_SPECIAL )
	{
		const float fOffsetForTwo = kItemDistance * 6.0f; 

		pTwo = NT_NEW CSListObjectTri;

		pTempText = NT_NEW CSListObjectText;
		pTempText->SetPosition( kLeftXPosistion, kTopYPosistion + ( 3.0f * kItemDistance ) + fOffsetForTwo );
		pTempText->SetSize( 1000.0f, 300.0f );
		pTempText->SetTransform( &m_TempTransform );
		pTempText->SetTextString( "COMBOMENU_SUPER_STYLE_HEADING_2" );

		pTwo->SetControl( pTempText, 0 );
		for( int j = 0 ; j < 4 ; ++j )
		{
			pTempText = NT_NEW CSListObjectText;
			pTempText->SetPosition( kComboTagSuperXPosistion, kTopYPosistion + ( kItemDistance * ( j + 1 ) ) + fOffsetForTwo );
			pTempText->SetSize( 1000.0f, 300.0f );
			pTempText->SetTransform( &m_TempTransform );
			pTempText->SetTextString( (*Iter)->cpMoveDesc.c_str() );

			pTwo->SetControl( pTempText, 1 + ( j * 2 ) );

			pTempText = NT_NEW CSListObjectText;
			pTempText->SetPosition( kComboTagXPosistion, kTopYPosistion + ( kItemDistance * ( j + 1 ) ) + fOffsetForTwo );
			pTempText->SetSize( 1000.0f, 300.0f );
			pTempText->SetTransform( &m_TempTransform );
			pTempText->SetTextString( (*Iter)->cpName.c_str() );

			pTwo->SetControl( pTempText, 2 + ( j * 2 ) );

			Iter++;
		}

		//have we just got this?
		if( TempHL > OldHL )
		{
			//add a new item then.
			CSListObjectNew* pNew = NT_NEW CSListObjectNew();
			pNew->SetTransform( &m_TempTransform );
			pNew->SetPosition( kComboSupperNewXPosition, kTopYPosistion + ( 3.0f * kItemDistance ) + fOffsetForTwo  );
			pNew->Init();

			pTwo->SetControl( pNew, 9 );
		}  

		m_ListControl.AddItem( pTwo );

	}
		//now for the next one
	//if we have special( what ever that is ) or Four( I though there was only 3 ) or Three, add em all then
	//change me so I don't enter the backwards^_^
	if( TempHL == HL_THREE ||
		TempHL == HL_FOUR ||
		TempHL == HL_SPECIAL )
	{
		const float fOffsetForThree = kItemDistance * 12.0f; 

		//new object
		pThree = NT_NEW CSListObjectTri;
		//make the level string
		pTempText = NT_NEW CSListObjectText;
		pTempText->SetPosition( kLeftXPosistion, kTopYPosistion + ( 3.0f * kItemDistance )  + fOffsetForThree );
		pTempText->SetSize( 1000.0f, 300.0f );
		pTempText->SetTransform( &m_TempTransform );
		pTempText->SetTextString( "COMBOMENU_SUPER_STYLE_HEADING_3" );
		//make it the first item so the selection cursor will fall on it
		pThree->SetControl( pTempText, 0 );

		//add the 4 other move names and stance requirements
		for( int j = 0 ; j < 4 ; ++j )
		{
			pTempText = NT_NEW CSListObjectText;
			pTempText->SetPosition( kComboTagSuperXPosistion, kTopYPosistion + ( kItemDistance * ( j + 1 ) ) + fOffsetForThree  );
			pTempText->SetSize( 1000.0f, 300.0f );
			pTempText->SetTransform( &m_TempTransform );
			pTempText->SetTextString( (*Iter)->cpMoveDesc.c_str() );

			pThree->SetControl( pTempText, 1 + ( j * 2 ) );

			pTempText = NT_NEW CSListObjectText;
			pTempText->SetPosition( kComboTagXPosistion, kTopYPosistion + ( kItemDistance * ( j + 1 ) ) + fOffsetForThree );
			pTempText->SetSize( 1000.0f, 300.0f );
			pTempText->SetTransform( &m_TempTransform );
			pTempText->SetTextString( (*Iter)->cpName.c_str() );

			pThree->SetControl( pTempText, 2 + ( j * 2 ) );

			Iter++;
		}

		//have we just got this?
		if( TempHL > OldHL )
		{
			//add a new item then.
			CSListObjectNew* pNew = NT_NEW CSListObjectNew();
			pNew->SetTransform( &m_TempTransform );
			pNew->SetPosition( kComboSupperNewXPosition, kTopYPosistion + ( 3.0f * kItemDistance ) + fOffsetForThree );
			pNew->Init();

			pThree->SetControl( pNew, 9 );
		}

		m_ListControl.AddItem( pThree );
	}

	m_Info.SetSize( 1025.0f - 255.0f, kBottomYInfoBoxPosistion - kTopYInfoBoxPosistion );
	m_Info.SetTextString( "COMBOMENU_SUPER_INFO", CStringDefinition::JUSTIFY_CENTRE, CStringDefinition::JUSTIFY_TOP  );

	m_ListControl.MoveSeperators();
}


void	CSControl::FillListFromPointer( ntstd::List< CS_MOVE_INFO_PARENT* >* pList, LayoutType eType  )
{
	//get an iterator
	ntstd::List< CS_MOVE_INFO_PARENT* >::iterator Iter = pList->begin();
	float fOffset = 0.0f;
	
	StanceStylePoints NewPoints;
	//get the current amounts of points the player has
	

	//for each element in the list
	while ( Iter != pList->end() )
	{
		bool bAdd = false;
		bool bNew = false;

		//are we in purgatory
		if( NULL == CEntityManager::Get().FindEntity( "PurgatoryManager" ) )
		{
			StyleManager::Get().GetCurrentStylePoints( NewPoints );
			//no
			//have we had our combo unlock data param set?
			if( NULL == (*Iter)->pComboUnlockData )
			{
				//add it then
				bAdd = true;
			}
			else
			{
				//with the points we had last time we looked at this screen, could we use this combo?
				if( (*Iter)->pComboUnlockData->CanComboBeUnlocked( CheckpointManager::Get().GetUniqueSaveData().GetStylePointsUsedForComboMenu() ) )
				{
					//yes, then add it
					bAdd = true;
				}
				//no can we add it with the points she currently has
				else if( (*Iter)->pComboUnlockData->CanComboBeUnlocked( NewPoints ) )
				{
					//yes, well add it and flag it as new
					bAdd = true;
					bNew = true;
				}
			}
		}
		else
		{
			//in purgatory, so add it regardless
			bAdd = true;
		}

	//make tri
		if( bAdd )
		{
			CSListObjectTri* pTemp = NULL;
			switch ( eType )
			{
			case BASIC:	//single liners
			case POWER:
			case SPEED:
				pTemp = BuildTriFromStrings( (*Iter)->cpName.c_str(), (*Iter)->cpMoveDesc.c_str(), bNew, fOffset );
				break;
			case AERIAL:  //dual liners
				pTemp = Build3LineFromStrings( (*Iter)->cpName.c_str(), (*Iter)->cpMoveDesc.c_str(), bNew, fOffset );
				break;
			case RANGE:
			case MISC:	 //triple liners
				pTemp = Build2LineFromStrings( (*Iter)->cpName.c_str(), (*Iter)->cpMoveDesc.c_str(), bNew, fOffset );
				break;
			case SUPERSTYLE:
				pTemp = NULL; //just to shut the stupid compiler up!   we will never get here. 
				break;
			}
	//give it to the list control
			m_ListControl.AddItem( pTemp );
	//next
			fOffset += m_ListControl.GetItemHeight();//kItemDistance;
		}
		//next!
		Iter++;
		
	}

}

void	CSControl::FillMultiControlWithBasic()
{
	CSListObjectText*	pOne = NT_NEW CSListObjectText();
	pOne->SetPosition( kMultiCtrlX, kMultiCtrlY );
	pOne->SetSize( 1000.0f, 50.0f );
	pOne->SetTransform( &m_MultiTranform );
	pOne->SetTextString( "COMBOMENU_STANCE_BASIC", CStringDefinition::JUSTIFY_CENTRE );

	m_MultiControl.AddItem(	pOne );

}

void	CSControl::AddStancesToMultiControl()
{
	CSListObjectText*	pOne = NT_NEW CSListObjectText();
	pOne->SetPosition( kMultiCtrlX, kMultiCtrlY );
	pOne->SetSize( 1000.0f, 25.0f );
	pOne->SetTransform( &m_MultiTranform );
	pOne->SetTextString( "COMBOMENU_STANCE_SPEED", CStringDefinition::JUSTIFY_CENTRE  );

	m_MultiControl.AddItem(	pOne );

	pOne = NT_NEW CSListObjectText();
	pOne->SetPosition( kMultiCtrlX, kMultiCtrlY );
	pOne->SetSize( 1000.0f, 25.0f );
	pOne->SetTransform( &m_MultiTranform );
	pOne->SetTextString( "COMBOMENU_STANCE_POWER", CStringDefinition::JUSTIFY_CENTRE  );

	m_MultiControl.AddItem(	pOne );

	pOne = NT_NEW CSListObjectText();
	pOne->SetPosition( kMultiCtrlX, kMultiCtrlY );
	pOne->SetSize( 1000.0f, 25.0f );
	pOne->SetTransform( &m_MultiTranform );
	pOne->SetTextString( "COMBOMENU_STANCE_RANGE", CStringDefinition::JUSTIFY_CENTRE  );

	m_MultiControl.AddItem(	pOne );

	pOne = NT_NEW CSListObjectText();
	pOne->SetPosition( kMultiCtrlX, kMultiCtrlY );
	pOne->SetSize( 1000.0f, 25.0f );
	pOne->SetTransform( &m_MultiTranform );
	pOne->SetTextString( "COMBOMENU_STANCE_AERIAL", CStringDefinition::JUSTIFY_CENTRE  );

	m_MultiControl.AddItem(	pOne );

	pOne = NT_NEW CSListObjectText();
	pOne->SetPosition( kMultiCtrlX, kMultiCtrlY );
	pOne->SetSize( 1000.0f, 25.0f );
	pOne->SetTransform( &m_MultiTranform );
	pOne->SetTextString( "COMBOMENU_STANCE_SUPERSTYLE", CStringDefinition::JUSTIFY_CENTRE  );

	m_MultiControl.AddItem(	pOne );
}


CSListObjectTri*	CSControl::BuildTriFromStrings( const char* cpName, const char* cpMoveList, bool bNew, float fOffset )
{
	CSListObjectTri*	pTri	= NT_NEW CSListObjectTri();
	CSListObjectText*	pOne	= NT_NEW CSListObjectText();
	CSListObjectNew*	pTwo;
	//are we new
	if( true == bNew )
	{
		//yes make it
		pTwo	= NT_NEW CSListObjectNew();
		pTwo->SetTransform( &m_TempTransform );
	}
	else
	{
		pTwo	= NULL;
	}
	CSListObjectText*	pThree	= NT_NEW CSListObjectText();
	
	ntAssert( NULL != pTri );
	ntAssert( NULL != pOne );
	if( true == bNew )
	{
		ntAssert( NULL != pTwo );
	}
	ntAssert( NULL != pThree );

	//add name
	pOne->SetPosition( kLeftXPosistion, fOffset + kTopYPosistion );
	pOne->SetSize( 1000.0f, 300.0f );
	pOne->SetTransform( &m_TempTransform );
	pOne->SetTextString( cpName );

	//set up new tag if need be
	if( true == bNew )
	{
		/*
		pTwo->SetSize( 1000.0f, 250.0f );
		pTwo->SetTransform( &m_TempTransform );
		pTwo->SetTextString("COMBOMENU_NEW");   */   
		pTwo->SetPosition( kNewTagXPosistion, fOffset + kTopYPosistion );
		pTwo->Init();
	}

	//set up move list
	pThree->SetPosition( kComboTagXPosistion, fOffset + kTopYPosistion );
	pThree->SetSize( 1000.0f, 300.0f );
	pThree->SetTransform( &m_TempTransform );
	pThree->SetTextString( cpMoveList );

	//combine in a tri
	pTri = NT_NEW CSListObjectTri;
	ntAssert( NULL != pTri );
	pTri->SetPosition( 0.0f, 0.0f );
	pTri->SetTransform( &m_TempTransform );
	pTri->SetFirstControl( pOne );
	pTri->SetSecondControl( pTwo );
	pTri->SetThirdControl( pThree );

	return pTri;

}

CSListObjectTri*	CSControl::Build3LineFromStrings( const char* cpName, const char* cpMoveList, bool bNew, float fOffset )
{
 	CSListObjectTri*	pTri	= NT_NEW CSListObjectTri();
	CSListObjectText*	pOne	= NT_NEW CSListObjectText();
	CSListObjectNew*	pTwo;
	if( true == bNew )
	{
		pTwo	= NT_NEW CSListObjectNew();
		pTwo->SetTransform( &m_TempTransform );
	}
	else
	{
		pTwo	= NULL;
	}
	CSListObjectText*	pThree	= NT_NEW CSListObjectText();
	
	ntAssert( NULL != pTri );
	ntAssert( NULL != pOne );
	if( true == bNew )
	{
		ntAssert( NULL != pTwo );
	}
	ntAssert( NULL != pThree );

	pOne->SetPosition( kLeftXPosistion, fOffset + kTopYPosistion + kItemDistance );
	pOne->SetSize( 1000.0f, 300.0f );
	pOne->SetTransform( &m_TempTransform );
	pOne->SetTextString( cpName );

	if( true == bNew )
	{
		pTwo->SetPosition( kNewTagXPosistion, fOffset + kTopYPosistion );
		pTwo->Init();
	}

	pThree->SetPosition( kComboTrippleXPosistion, fOffset + kTopYPosistion );
	pThree->SetSize( 1000.0f, 300.0f );
	pThree->SetTransform( &m_TempTransform );
	pThree->SetTextString( cpMoveList );

	pTri = NT_NEW CSListObjectTri;
	ntAssert( NULL != pTri );
	pTri->SetPosition( 0.0f, 0.0f );
	pTri->SetTransform( &m_TempTransform );
	pTri->SetFirstControl( pOne );
	pTri->SetSecondControl( pTwo );
	pTri->SetThirdControl( pThree );


	return pTri;
}

CSListObjectTri*	CSControl::Build2LineFromStrings( const char* cpName, const char* cpMoveList, bool bNew, float fOffset )
{
	CSListObjectTri*	pTri	= NT_NEW CSListObjectTri();
	CSListObjectText*	pOne	= NT_NEW CSListObjectText();
	CSListObjectNew*	pTwo;
	if( true == bNew )
	{
		pTwo	= NT_NEW CSListObjectNew();
		pTwo->SetTransform( &m_TempTransform );
	}
	else
	{
		pTwo	= NULL;
	}
	CSListObjectText*	pThree	= NT_NEW CSListObjectText();
	
	ntAssert( NULL != pTri );
	ntAssert( NULL != pOne );
	if( true == bNew )
	{
		ntAssert( NULL != pTwo );
	}
	ntAssert( NULL != pThree );

	pOne->SetPosition( kLeftXPosistion, fOffset + kTopYPosistion + ( kItemDistance  / 2.0f ) );
	pOne->SetSize( 1000.0f, 300.0f );
	pOne->SetTransform( &m_TempTransform );
	pOne->SetTextString( cpName );

	if( true == bNew )
	{
		pTwo->SetPosition( kNewTagXPosistion, fOffset + kTopYPosistion );
		pTwo->Init();
	}

	pThree->SetPosition( kComboTagXPosistion, fOffset + kTopYPosistion );
	pThree->SetSize( 1000.0f, 300.0f );
	pThree->SetTransform( &m_TempTransform );
	pThree->SetTextString( cpMoveList );

	pTri = NT_NEW CSListObjectTri;
	ntAssert( NULL != pTri );
	pTri->SetPosition( 0.0f, 0.0f );
	pTri->SetTransform( &m_TempTransform );
	pTri->SetFirstControl( pOne );
	pTri->SetSecondControl( pTwo );
	pTri->SetThirdControl( pThree );


	return pTri;
}

CSListObjectTri*	CSControl::BuildSuperListLineFromStrings( const char* cpName, const char* cpMoveList, bool bNew, float fOffset )
{
	CSListObjectTri*	pTri	= NT_NEW CSListObjectTri();
	CSListObjectText*	pOne	= NT_NEW CSListObjectText();
	CSListObjectNew*	pTwo;
	if( true == bNew )
	{
		pTwo	= NT_NEW CSListObjectNew();
		pTwo->SetTransform( &m_TempTransform );
	}
	else
	{
		pTwo	= NULL;
	}
	CSListObjectText*	pThree	= NT_NEW CSListObjectText();
	
	ntAssert( NULL != pTri );
	ntAssert( NULL != pOne );
	if( true == bNew )
	{
		ntAssert( NULL != pTwo );
	}
	ntAssert( NULL != pThree );

	pOne->SetPosition( kLeftXPosistion, fOffset + kTopYPosistion + ( kItemDistance  / 2.0f ) );
	pOne->SetSize( 1000.0f, 300.0f );
	pOne->SetTransform( &m_TempTransform );
	pOne->SetTextString( cpName );

	if( true == bNew )
	{
		pTwo->SetPosition( kNewTagXPosistion, fOffset + kTopYPosistion );
		pTwo->Init();
	}

	pThree->SetPosition( kComboTagXPosistion, fOffset + kTopYPosistion );
	pThree->SetSize( 1000.0f, 300.0f );
	pThree->SetTransform( &m_TempTransform );
	pThree->SetTextString( cpMoveList );

	pTri = NT_NEW CSListObjectTri;
	ntAssert( NULL != pTri );
	pTri->SetPosition( 0.0f, 0.0f );
	pTri->SetTransform( &m_TempTransform );
	pTri->SetFirstControl( pOne );
	pTri->SetSecondControl( pTwo );
	pTri->SetThirdControl( pThree );


	return pTri;
}

void CSControl::SetUpWindows( void )
{
	//copied code from Mike Blaha's code, so ask him^_^
	m_aobBackgroundWindow1[0].SetTexture( "gui/frontend/textures/comboscreen/ow_combotextboxcorner4_colour_alpha_nomip.tga" );
	m_aobBackgroundWindow1[1].SetTexture( "gui/frontend/textures/comboscreen/ow_combotextboxstraight1_colour_alpha_nomip.tga");
	m_aobBackgroundWindow1[2].SetTexture( "gui/frontend/textures/comboscreen/ow_combotextboxcorner1_colour_alpha_nomip.tga" );
	m_aobBackgroundWindow1[3].SetTexture( "gui/frontend/textures/comboscreen/ow_combotextboxstraight2_colour_alpha_nomip.tga" );
	m_aobBackgroundWindow1[4].SetTexture( "gui/frontend/textures/comboscreen/ow_combotextboxcorner2_colour_alpha_nomip.tga" );
	m_aobBackgroundWindow1[5].SetTexture( "gui/frontend/textures/comboscreen/ow_combotextboxstraight3_colour_alpha_nomip.tga" );
	m_aobBackgroundWindow1[6].SetTexture( "gui/frontend/textures/comboscreen/ow_combotextboxcorner3_colour_alpha_nomip.tga" );
	m_aobBackgroundWindow1[7].SetTexture( "gui/frontend/textures/comboscreen/ow_combotextboxstraight4_colour_alpha_nomip.tga" );
	m_aobBackgroundWindow1[8].SetTexture( "gui/frontend/textures/comboscreen/ow_combotextboxcentre_colour_alpha_nomip.tga" );

	m_aobBackgroundWindow2[0].SetTexture( "gui/frontend/textures/comboscreen/ow_combotextboxcorner4_colour_alpha_nomip.tga" );
	m_aobBackgroundWindow2[1].SetTexture( "gui/frontend/textures/comboscreen/ow_combotextboxstraight1_colour_alpha_nomip.tga");
	m_aobBackgroundWindow2[2].SetTexture( "gui/frontend/textures/comboscreen/ow_combotextboxcorner1_colour_alpha_nomip.tga" );
	m_aobBackgroundWindow2[3].SetTexture( "gui/frontend/textures/comboscreen/ow_combotextboxstraight2_colour_alpha_nomip.tga" );
	m_aobBackgroundWindow2[4].SetTexture( "gui/frontend/textures/comboscreen/ow_combotextboxcorner2_colour_alpha_nomip.tga" );
	m_aobBackgroundWindow2[5].SetTexture( "gui/frontend/textures/comboscreen/ow_combotextboxstraight3_colour_alpha_nomip.tga" );
	m_aobBackgroundWindow2[6].SetTexture( "gui/frontend/textures/comboscreen/ow_combotextboxcorner3_colour_alpha_nomip.tga" );
	m_aobBackgroundWindow2[7].SetTexture( "gui/frontend/textures/comboscreen/ow_combotextboxstraight4_colour_alpha_nomip.tga" );
	m_aobBackgroundWindow2[8].SetTexture( "gui/frontend/textures/comboscreen/ow_combotextboxcentre_colour_alpha_nomip.tga" );
    
	CPoint obMin( 160.0f, 100.0f, 0.0f );
	CPoint obMax( 1120.0f, 630.0f, 0.0f );

	const GuiSettingsMenuSelectDef* pobSettings = CGuiManager::Get().GuiSettings()->MenuSelect();

	const CPoint& obPad = CPoint(pobSettings->BorderPadding());
	const CPoint& obBorderSize = CPoint(pobSettings->BorderSize());

	CPoint obBodySize = obMax - obMin + obPad*2.0f;
	CPoint obCentre = obMin + obBodySize/2.0f - obPad;

	//centre
	m_aobBackgroundWindow1[8].SetWidth( obBodySize.X() );
	m_aobBackgroundWindow1[8].SetHeight( obBodySize.Y() );
	m_aobBackgroundWindow1[8].SetPosition( obCentre );


	CPoint obHalfBorderSize = obBorderSize/2.0f;
	CPoint obHalfBodySize = obBodySize/2.0f;

	//topleft
	m_aobBackgroundWindow1[0].SetWidth( obBorderSize.X() );
	m_aobBackgroundWindow1[0].SetHeight( obBorderSize.Y() );
	m_aobBackgroundWindow1[0].SetPosition( obCentre - obHalfBodySize - obHalfBorderSize );

	//top
	m_aobBackgroundWindow1[1].SetWidth( obBodySize.X() );
	m_aobBackgroundWindow1[1].SetHeight( obBorderSize.Y() );
	m_aobBackgroundWindow1[1].SetPosition( obCentre - CPoint(0, obHalfBodySize.Y() + obHalfBorderSize.Y(), 0) );

	//topright
	m_aobBackgroundWindow1[2].SetWidth( obBorderSize.X() );
	m_aobBackgroundWindow1[2].SetHeight( obBorderSize.Y() );
	m_aobBackgroundWindow1[2].SetPosition( obCentre + CPoint(obHalfBodySize.X() + obHalfBorderSize.X(), -(obHalfBodySize.Y() + obHalfBorderSize.Y()), 0) );

	//right
	m_aobBackgroundWindow1[3].SetWidth( obBorderSize.X() );
	m_aobBackgroundWindow1[3].SetHeight( obBodySize.Y() );
	m_aobBackgroundWindow1[3].SetPosition( obCentre + CPoint(obHalfBodySize.X() + obHalfBorderSize.X(), 0, 0) );

	//bottomright
	m_aobBackgroundWindow1[4].SetWidth( obBorderSize.X() );
	m_aobBackgroundWindow1[4].SetHeight( obBorderSize.Y() );
	m_aobBackgroundWindow1[4].SetPosition( obCentre + obHalfBodySize + obHalfBorderSize );

	//bottom
	m_aobBackgroundWindow1[5].SetWidth( obBodySize.X() );
	m_aobBackgroundWindow1[5].SetHeight( obBorderSize.Y() );
	m_aobBackgroundWindow1[5].SetPosition( obCentre + CPoint(0, obHalfBodySize.Y() + obHalfBorderSize.Y(), 0) );

	//bottomleft
	m_aobBackgroundWindow1[6].SetWidth( obBorderSize.X() );
	m_aobBackgroundWindow1[6].SetHeight( obBorderSize.Y() );
	m_aobBackgroundWindow1[6].SetPosition( obCentre - CPoint(obHalfBodySize.X() + obHalfBorderSize.X(), -(obHalfBodySize.Y() + obHalfBorderSize.Y()), 0) );

	//left
	m_aobBackgroundWindow1[7].SetWidth( obBorderSize.X() );
	m_aobBackgroundWindow1[7].SetHeight( obBodySize.Y() );
	m_aobBackgroundWindow1[7].SetPosition( obCentre - CPoint(obHalfBodySize.X() + obHalfBorderSize.X(), 0, 0) );

	//and the other one
	obMin = CPoint( 255.0f, 510.0f, 0.0f );
	obMax = CPoint( 1025.0f, 600.0f, 0.0f );

	obBodySize = obMax - obMin + obPad*2.0f;
	obCentre = obMin + obBodySize/2.0f - obPad;

	//centre
	m_aobBackgroundWindow2[8].SetWidth( obBodySize.X() );
	m_aobBackgroundWindow2[8].SetHeight( obBodySize.Y() );
	m_aobBackgroundWindow2[8].SetPosition( obCentre );


	obHalfBorderSize = obBorderSize/2.0f;
	obHalfBodySize = obBodySize/2.0f;

	//topleft
	m_aobBackgroundWindow2[0].SetWidth( obBorderSize.X() );
	m_aobBackgroundWindow2[0].SetHeight( obBorderSize.Y() );
	m_aobBackgroundWindow2[0].SetPosition( obCentre - obHalfBodySize - obHalfBorderSize );

	//top
	m_aobBackgroundWindow2[1].SetWidth( obBodySize.X() );
	m_aobBackgroundWindow2[1].SetHeight( obBorderSize.Y() );
	m_aobBackgroundWindow2[1].SetPosition( obCentre - CPoint(0, obHalfBodySize.Y() + obHalfBorderSize.Y(), 0) );

	//topright
	m_aobBackgroundWindow2[2].SetWidth( obBorderSize.X() );
	m_aobBackgroundWindow2[2].SetHeight( obBorderSize.Y() );
	m_aobBackgroundWindow2[2].SetPosition( obCentre + CPoint(obHalfBodySize.X() + obHalfBorderSize.X(), -(obHalfBodySize.Y() + obHalfBorderSize.Y()), 0) );

	//right
	m_aobBackgroundWindow2[3].SetWidth( obBorderSize.X() );
	m_aobBackgroundWindow2[3].SetHeight( obBodySize.Y() );
	m_aobBackgroundWindow2[3].SetPosition( obCentre + CPoint(obHalfBodySize.X() + obHalfBorderSize.X(), 0, 0) );

	//bottomright
	m_aobBackgroundWindow2[4].SetWidth( obBorderSize.X() );
	m_aobBackgroundWindow2[4].SetHeight( obBorderSize.Y() );
	m_aobBackgroundWindow2[4].SetPosition( obCentre + obHalfBodySize + obHalfBorderSize );

	//bottom
	m_aobBackgroundWindow2[5].SetWidth( obBodySize.X() );
	m_aobBackgroundWindow2[5].SetHeight( obBorderSize.Y() );
	m_aobBackgroundWindow2[5].SetPosition( obCentre + CPoint(0, obHalfBodySize.Y() + obHalfBorderSize.Y(), 0) );

	//bottomleft
	m_aobBackgroundWindow2[6].SetWidth( obBorderSize.X() );
	m_aobBackgroundWindow2[6].SetHeight( obBorderSize.Y() );
	m_aobBackgroundWindow2[6].SetPosition( obCentre - CPoint(obHalfBodySize.X() + obHalfBorderSize.X(), -(obHalfBodySize.Y() + obHalfBorderSize.Y()), 0) );

	//left
	m_aobBackgroundWindow2[7].SetWidth( obBorderSize.X() );
	m_aobBackgroundWindow2[7].SetHeight( obBodySize.Y() );
	m_aobBackgroundWindow2[7].SetPosition( obCentre - CPoint(obHalfBodySize.X() + obHalfBorderSize.X(), 0, 0) );
}
