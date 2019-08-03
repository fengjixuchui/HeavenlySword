/***************************************************************************************************
*
*	DESCRIPTION		
*
*	NOTES			
*
***************************************************************************************************/

#ifndef _GAMEFLOWLOADER_H_
#define _GAMEFLOWLOADER_H_

// Includes
#include "core/nt_std.h"

// Forward Declarations
class CScreenHeader;
class CXMLElement;

/***************************************************************************************************
*
*	CLASS			CGameflowLoader
*
*	DESCRIPTION		
*
***************************************************************************************************/

class CGameflowLoader
{
public:

	static CScreenHeader* Load(const char* szFilename);

private:
	CGameflowLoader( void ) {}
	~CGameflowLoader( void ) {}

	static void PrintFlow(CXMLElement* pobNode, int indent = 0);
	static void UpdateHeaderChildren(CXMLElement* pobNode);
	static void CollectGroups(CXMLElement* pobRoot, ntstd::List<CXMLElement*>& obGroups);
	static void ExpandGroup(CXMLElement* pobNode);
};

#endif // _GAMEFLOWLOADER_H_
