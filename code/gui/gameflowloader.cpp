/***************************************************************************************************
*
*	DESCRIPTION		
*
*	NOTES			
*
***************************************************************************************************/

// Includes
#include "gameflowloader.h"
#include "gui/guimanager.h"
#include "gui/guiparse.h"
#include "gui/guiflow.h"
#include "game/shellconfig.h"

CScreenHeader* CGameflowLoader::Load(const char* szFilename)
{
	CXMLElement* pFlow = CXMLParse::Get().CreateTree( szFilename );

	//create a dummy node to make processing easier
	CXMLElement* pobDummy = NT_NEW CXMLElement();
	pobDummy->SetName("DUMMY");
	pobDummy->SetChild(pFlow);
	pFlow->SetParent(pobDummy);

	ntstd::List<CXMLElement*> obGroups;

	CollectGroups(pobDummy, obGroups);

	while (obGroups.size() > 0)
	{
		for(ntstd::List< CXMLElement* >::iterator obIt = obGroups.begin(); obIt != obGroups.end(); ++obIt)
		{
			ExpandGroup( *obIt );
		}

		obGroups.clear();
		
		CollectGroups(pobDummy, obGroups);
	}

	ntAssert(pobDummy->GetChildren().size() == 1);

	CScreenHeader* pobGameFlow = static_cast<CScreenHeader*>( pobDummy->GetChildren().front() );
	pobGameFlow->SetParent(NULL);

	pobDummy->GetChildren().clear();
	NT_DELETE(pobDummy);

	//inject screen calibration
	if ( g_ShellOptions->m_bEnableScreenCalibration )
	{
		const char* szSC = "gui\\screenCalibration\\screencalibrationflow.xml";
		CXMLElement* pobSC = CXMLParse::Get().CreateTree( szSC );
		if (pobSC)
		{
			CScreenHeader* pobSCHeader = static_cast<CScreenHeader*>(pobSC);
			pobGameFlow->SetParent(pobSCHeader);
			pobSCHeader->SetChild(pobGameFlow);
			pobGameFlow = pobSCHeader;
		}
		else
		{
			ntPrintf("Screen Calibration definition not found [%s]\n", szSC);
		}
	}

//	PrintFlow(pobGameFlow);
	UpdateHeaderChildren(pobGameFlow);

	CXMLElement::FreeNameStrings(pobGameFlow);

	return pobGameFlow;
}

void CGameflowLoader::PrintFlow(CXMLElement* pobNode, int indent)
{
	for (int i = 0; i < indent; i++)
		ntPrintf("\t");

	const char* name = pobNode->GetAttribute("filename");
	UNUSED( name );
	ntPrintf(name?name:"");
	ntPrintf("\t");
	ntPrintf(pobNode->GetName());
	if (strcmp(pobNode->GetName(), "SCREENHEADER") == 0)
	{
		ntPrintf("\t");
		ntPrintf(static_cast<CScreenHeader*>(pobNode)->GetTag());
	}

	ntPrintf("\tUs: %p\tParent: %p", pobNode, pobNode->GetParent());
	ntPrintf("\n");

	for ( ntstd::List< CXMLElement* >::iterator obIt = pobNode->GetChildren().begin(); obIt != pobNode->GetChildren().end(); ++obIt)
		PrintFlow(*obIt, indent+1);
}

void CGameflowLoader::UpdateHeaderChildren(CXMLElement* pobNode)
{
	static_cast<CScreenHeader*>( pobNode )->SyncChildren();

	for ( ntstd::List< CXMLElement* >::iterator obIt = pobNode->GetChildren().begin(); obIt != pobNode->GetChildren().end(); ++obIt)
		UpdateHeaderChildren(*obIt);
}

void CGameflowLoader::CollectGroups(CXMLElement* pobRoot, ntstd::List<CXMLElement*>& obGroups)
{
	if (strcmp(pobRoot->GetName(), "SCREENGROUP") == 0)
	{
		obGroups.push_back(pobRoot);
	}

	for( ntstd::List< CXMLElement* >::iterator obIt = pobRoot->GetChildren().begin(); obIt != pobRoot->GetChildren().end(); ++obIt)
	{
		CollectGroups( *obIt, obGroups );
	}
}

void CGameflowLoader::ExpandGroup(CXMLElement* pobNode)
{
	CScreenGroup* pobGroup = static_cast<CScreenGroup*>(pobNode);
	CXMLElement* pExpandedGroup = CXMLParse::Get().CreateTree( pobGroup->GetGroupFilename() );

	// swap out this xml node and replace it with the above tree

	//copy tag
	pExpandedGroup->SetAttribute("tag", pobGroup->GetTag());

	//parent subtree
	pExpandedGroup->SetParent(pobGroup->GetParent());

	//find current node
	ntstd::List< CXMLElement* >& obChildren = pobGroup->GetParent()->GetChildren();
	ntstd::List< CXMLElement* >::iterator obIt = ntstd::find(obChildren.begin(), obChildren.end(), pobGroup);
	
	pobGroup->GetParent()->SetChild(pExpandedGroup, *obIt);
	pobGroup->GetParent()->RemoveChild(*obIt);

	//get the 'last' node in the group
	CXMLElement* pobLastChild = pExpandedGroup;
	while (pobLastChild->GetChildren().size() != 0)
		pobLastChild = pobLastChild->GetChildren().back();

	// reparent children
	for( ntstd::List< CXMLElement* >::iterator obIt = pobGroup->GetChildren().begin(); obIt != pobGroup->GetChildren().end(); ++obIt)
	{
		(*obIt)->SetParent(pobLastChild);
		pobLastChild->SetChild(*obIt);
	}

	// completely disconnect from surroundings
	pobGroup->SetParent(NULL);
	pobGroup->GetChildren().clear();
	// and drop
	NT_DELETE(pobGroup);
}



