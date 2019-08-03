/***************************************************************************************************
*
*	DESCRIPTION		A general purpose XML parser and base XML element for building trees
*
*	NOTES			The parser builds a tree of XML Elements from a given file.  Items to be
*					created by parsing should be derived from the CXMLElement object.
*
***************************************************************************************************/

// Includes
#include "guiparse.h"
#include "guiresource.h"
#include "guimanager.h"

// Setting for handling the parser
// #define USING_CHARACTER_DATA

// Setting for debugging what is parsed
// #define DEBUG_PARSER_RESULTS

// Include the Expat XML Parser
#include "expat\xmlparse.h"

/***************************************************************************************************
*
*	FUNCTION		CXMLElement::CXMLElement
*
*	DESCRIPTION		Construction
*
***************************************************************************************************/

CXMLElement::CXMLElement( void )
{
	// Initialise the member pointers
	m_pobParent = 0;
	m_pcElementName = 0;
	m_pcCharacterData = 0;
}


/**************************************************************************************************
*
*	FUNCTION		CXMLElement::~CXMLElement
*
*	DESCRIPTION		Destruction - this must clean up children too
*
***************************************************************************************************/

CXMLElement::~CXMLElement( void )
{
	// Get rid of the name data
	NT_DELETE_ARRAY( m_pcElementName );

#ifdef _DEBUG

	// Get rid of any character data
	NT_DELETE_ARRAY( m_pcCharacterData );

	// Clean out the attributes
	for( ntstd::List< XML_ATTRIBUTE* >::iterator obIt = m_obAttributes.begin(); obIt != m_obAttributes.end(); ++obIt)
	{
		NT_DELETE_ARRAY( ( *obIt )->pcTitle );
		NT_DELETE_ARRAY( ( *obIt )->pcValue );
		NT_DELETE( ( *obIt ) );
	}

	m_obAttributes.clear();

#endif // _DEBUG

	// Clear all the clildren of this element
	for( ntstd::List< CXMLElement* >::iterator obIt = m_obChildren.begin(); obIt != m_obChildren.end(); ++obIt)
	{
		NT_DELETE( ( *obIt ) );
	}

	m_obChildren.clear();

}


/**************************************************************************************************
*
*	FUNCTION		CXMLElement::SetName
*
*	DESCRIPTION		Always called once when setting up an element
*
***************************************************************************************************/

void CXMLElement::SetName( const char* pcName )
{
	// Copy the string to our own storage
	char* pcNewName = NT_NEW char[ strlen( pcName ) + 1 ];
	strcpy( pcNewName, pcName );

	m_pcElementName = pcNewName;

	if ( !ProcessName( pcName ) )
	{ 
		// ntAssert( 0 );
	}
}

/**************************************************************************************************
*
*	FUNCTION		CXMLElement::FreeNameStrings
*
*	DESCRIPTION		Allows for early freeing of name strings.
*
*	INPUTS			pobRoot Root node of tree. It is then walked.
*
*	NOTES			The CXMLElement dtor will free this strings aswell, so this call isnt necessary.
*
***************************************************************************************************/

void CXMLElement::FreeNameStrings(CXMLElement* pobRoot)
{
	NT_DELETE_ARRAY( pobRoot->m_pcElementName );
	pobRoot->m_pcElementName = 0;

	// Process children
	for( ntstd::List< CXMLElement* >::iterator obIt = pobRoot->m_obChildren.begin(); obIt != pobRoot->m_obChildren.end(); ++obIt)
	{
		FreeNameStrings( *obIt );
	}
}

/**************************************************************************************************
*
*	FUNCTION		CXMLElement::GetAttribute
*
*	DESCRIPTION		Queries the attribute table for the requested attribute.
*
*	INPUTS			szAttr The required attribute.
*
*	OUTPUT			The value of szAttr, otherwise NULL.
*
***************************************************************************************************/

const char* CXMLElement::GetAttribute( const char* szAttr ) const
{
	for( ntstd::List< XML_ATTRIBUTE* >::const_iterator obIt = m_obAttributes.begin(); obIt != m_obAttributes.end(); ++obIt)
	{
		if ( strcmp((*obIt)->pcTitle, szAttr) == 0)
			return (*obIt)->pcValue;
	}
	return NULL;
}

/**************************************************************************************************
*
*	FUNCTION		CXMLElement::RemoveChild
*
*	DESCRIPTION		Removes the supplied child from this element.
*
*	INPUTS			pobChild An existing child node to remove.
*
*	NOTES			pobChild must be a child of this element.
*
***************************************************************************************************/

void CXMLElement::RemoveChild( CXMLElement* pobChild )
{
	ntstd::List< CXMLElement* >::iterator obIt = ntstd::find(m_obChildren.begin(), m_obChildren.end(), pobChild);
	ntAssert(obIt != m_obChildren.end());

	m_obChildren.erase(obIt);
}

/**************************************************************************************************
*
*	FUNCTION		CXMLElement::SetCharacterData
*
*	DESCRIPTION		May be called multiple times for a single element.  So we need to concatinate
*					any info onto anything already stored.
*
***************************************************************************************************/

void CXMLElement::SetCharacterData( const char* pcCharacterData )
{
#ifdef _DEBUG
	// No Debug Information here
#endif // _DEBUG

#ifdef USING_CHARACTER_DATA

	// Is this the first time this has been called for this element?
	if ( m_pcCharacterData == 0 )
	{
		char* pcNewchar_tacterData = NT_NEW char[ strlen( pcCharacterData ) + 1 ];
		strcpy( pcNewchar_tacterData, pcCharacterData );

		m_pcCharacterData = pcNewchar_tacterData;
	}

	// If not, there'll be some concatination action
	else
	{
		// Create a big string
		char* pcNewchar_tacterData = NT_NEW char[ strlen( pcCharacterData ) + strlen( m_pcCharacterData ) ];

		// Copy the new stuff into it
		strcpy( pcNewchar_tacterData, m_pcCharacterData );
		strcat( pcNewchar_tacterData, pcCharacterData );

		// Clear up the old string
		NT_DELETE_ARRAY( m_pcCharacterData );

		// Repoint the pointer
		m_pcCharacterData = pcNewchar_tacterData;
	}

	if ( !ProcessCharacterData( pcCharacterData ) )
	{ 
		ntAssert( 0 );
	}

#else

	// Our formal parameter is not used
	UNUSED( pcCharacterData );

#endif // USING_CHARACTER_DATA

}


/**************************************************************************************************
*
*	FUNCTION		CXMLElement::SetAttribute
*
*	DESCRIPTION		
*
***************************************************************************************************/

void CXMLElement::SetAttribute( const char* pcTitle, const char* pcValue )
{
#ifdef _DEBUG

	// Store the given strings
	char* pcNewTitle = NT_NEW char[ strlen( pcTitle ) + 1 ];
	strcpy( pcNewTitle, pcTitle );

	char* pcNewValue = NT_NEW char[ strlen( pcValue ) + 1 ];
	strcpy( pcNewValue, pcValue );

	// Now collect them up in our list
	XML_ATTRIBUTE* pstrNewAttribute = NT_NEW XML_ATTRIBUTE;
	pstrNewAttribute->pcTitle = pcNewTitle;
	pstrNewAttribute->pcValue = pcNewValue;

	m_obAttributes.push_back( pstrNewAttribute );

#endif // _DEBUG

	if ( !ProcessAttribute( pcTitle, pcValue ) )
	{ 
		// An attribute in the imported file has not been caught and processed
		ntAssert_p( 0, ("Attribute not parsed: Title %s Value %s", pcTitle, pcValue) );
	}
}


/**************************************************************************************************
*
*	FUNCTION		CXMLElement::SetChild
*
*	DESCRIPTION		
*
***************************************************************************************************/

void CXMLElement::SetChild( CXMLElement* pobChild, CXMLElement* pobInsertBefore )
{
#ifdef _DEBUG
	// No Debug Information here
#endif // _DEBUG

	// Add a pointer to this child to the child list
	if (pobInsertBefore == NULL)
	{
		m_obChildren.push_back( pobChild );
	}
	else
	{
		ntstd::List< CXMLElement* >::iterator obIt = ntstd::find(m_obChildren.begin(), m_obChildren.end(), pobInsertBefore);
		ntAssert( obIt != m_obChildren.end() );

		m_obChildren.insert(obIt, pobChild);
	}
	

	if ( !ProcessChild( pobChild ) )
	{ 
		ntAssert( 0 );
	}
}


/**************************************************************************************************
*
*	FUNCTION		CXMLElement::SetParent
*
*	DESCRIPTION		
*
***************************************************************************************************/

void CXMLElement::SetParent( CXMLElement* pobParent )
{
#ifdef _DEBUG
	// No Debug Information here
#endif // _DEBUG

	// Save what we have been passed
//	ntAssert( pobParent );
	m_pobParent = pobParent;

	// Do we need to do any more?
	if ( !ProcessParent( pobParent ) )
	{ 
		ntAssert( 0 );
	}
}


/**************************************************************************************************
*
*	FUNCTION		CXMLElement::SetEnd
*
*	DESCRIPTION		Called at when a tag ends, so a good time to check that an element has all that
*					it needs to get on with its job.
*
***************************************************************************************************/

void CXMLElement::SetEnd( void )
{
#ifdef _DEBUG
	// No Debug Information here
#endif // _DEBUG

	// Is the element we have set up happy
	if ( !ProcessEnd() )
	{ 
		ntAssert( 0 );
	}

	PostProcessEnd();
}


/**************************************************************************************************
*
*	FUNCTION		CXMLElement::ProcessName
*
*	DESCRIPTION		
*
***************************************************************************************************/

bool CXMLElement::ProcessName( const char* pcName )
{
	// Unused formal parameters
	UNUSED( pcName );

	return true;
}


/**************************************************************************************************
*
*	FUNCTION		CXMLElement::ProcessCharacterData
*
*	DESCRIPTION		
*
***************************************************************************************************/

bool CXMLElement::ProcessCharacterData( const char* pcCharacterData )
{
	// Unused formal parameters
	UNUSED( pcCharacterData );

	return true;
}


/**************************************************************************************************
*
*	FUNCTION		CXMLElement::ProcessAttribute
*
*	DESCRIPTION		Attribute processing needs to be filtered through the heirarchy of derived items
*					so we should always return false if nothing has been used on each particular
*					level.
*
***************************************************************************************************/

bool CXMLElement::ProcessAttribute( const char* pcTitle, const char* pcValue )
{
	// Unused formal parameters
	UNUSED( pcTitle );
	UNUSED( pcValue );

	return false;
}


/**************************************************************************************************
*
*	FUNCTION		CXMLElement::ProcessChild
*
*	DESCRIPTION		
*
***************************************************************************************************/

bool CXMLElement::ProcessChild( CXMLElement* pobChild )
{
	// Unused formal parameters
	UNUSED( pobChild );

	return true;
}


/**************************************************************************************************
*
*	FUNCTION		CXMLElement::ProcessParent
*
*	DESCRIPTION		
*
***************************************************************************************************/

bool CXMLElement::ProcessParent( CXMLElement* pobParent )
{
	// Unused formal parameters
	UNUSED( pobParent );

	return true;
}


/**************************************************************************************************
*
*	FUNCTION		CXMLElement::ProcessEnd
*
*	DESCRIPTION		
*
***************************************************************************************************/

bool CXMLElement::ProcessEnd( void )
{
	return false;
}

/**************************************************************************************************
*
*	FUNCTION		CXMLElement::PostProcessEnd
*
*	DESCRIPTION		
*
***************************************************************************************************/

void CXMLElement::PostProcessEnd( void )
{
}

/**************************************************************************************************
*
*	FUNCTION		CXMLElement::GetElementDepth
*
*	DESCRIPTION		Returns how far down the heirarchy the element is by tracing parent pointers.
*					This is only called on debug builds.
*
***************************************************************************************************/

int CXMLElement::GetElementDepth( void )
{
	// If the element has no parents then it sits at level zero
	int iLevel = 0;

	// Now see how far back we can go through parents
	CXMLElement* pobElement = this;

	while ( pobElement->GetParent() )
	{
		iLevel++;
		pobElement = pobElement->GetParent();
	}

	return iLevel;
}


/**************************************************************************************************
*
*	FUNCTION		CXMLElement::ProcessParent
*
*	DESCRIPTION		
*
***************************************************************************************************/

void CXMLElement::DebugPrintElement( bool bSiblings )
{	
#ifdef DEBUG_PARSER_RESULTS

	// If someone is trying to print before the element is named
	// then they are a muppet, because that serves no purpose whatsoever.
	ntAssert( m_pcElementName );

	// Print the tag name - indent the data
	for ( int iIndent = 0; iIndent < GetElementDepth(); iIndent++ )
		ntPrintf( "\t" );

	// Print the name
	ntPrintf( m_pcElementName );

	// Finish off the line
	ntPrintf( "\n" );

	// Print out the attribute data
	for( ntstd::List< XML_ATTRIBUTE* >::iterator obIt = m_obAttributes.begin(); obIt != m_obAttributes.end(); ++obIt)
	{
		// Indent the data
		for ( int iIndent = 0; iIndent < GetElementDepth(); iIndent++ )
			ntPrintf( "\t" );

		// Print the title
		ntPrintf( ( *obIt )->pcTitle );

		// Space it out
		ntPrintf( " = " );

		// Print out the value
		ntPrintf( ( *obIt )->pcValue );

		// Finish off the line
		ntPrintf( "\n" );
	}

	// Print the character data, this doesn't necessarily exist
	if ( m_pcCharacterData )
	{
#ifdef USING_CHARACTER_DATA

		// Indent accordingly
		for ( int iIndent = 0; iIndent < GetElementDepth(); iIndent++ )
			ntPrintf( "\t" );

		// Print the data
		ntPrintf( m_pcCharacterData );

		// Finish off the line
		ntPrintf( "\n" );

#endif // USING_CHARACTER_DATA
	}
	
	// Now go and print the child elements if requested
	if ( bSiblings )
	{
		for( ntstd::List< CXMLElement* >::iterator obIt = m_obChildren.begin(); obIt != m_obChildren.end(); ++obIt)
		{
			( *obIt )->DebugPrintElement( true );
		}
	}
#else

	// Unused formal parameters
	UNUSED( bSiblings );

#endif // _DEBUG_PARSER_RESULTS
}


/***************************************************************************************************
*
*	FUNCTION		StaticHandlerEndTag
*
***************************************************************************************************/

static void StaticHandlerEndTag( void* pUserData, const char* pcElementName )
{
	CXMLParse::Get().HandlerEndTag( pUserData, pcElementName );
}

/***************************************************************************************************
*
*	FUNCTION		StaticHandlerComment
*
***************************************************************************************************/

static void StaticHandlerComment( void* pUserData, const char* pcComment )
{
	CXMLParse::Get().HandlerComment( pUserData, pcComment );
}

/***************************************************************************************************
*
*	FUNCTION		StaticHandlerProcess
*
***************************************************************************************************/

static void StaticHandlerProcess( void* pUserData, const char* pcTarget, const char* pcData )
{
	CXMLParse::Get().HandlerProcess( pUserData, pcTarget, pcData );
}

/***************************************************************************************************
*
*	FUNCTION		StaticHandlerStartTag
*
***************************************************************************************************/

static void StaticHandlerStartTag( void* pUserData, const char* pcElementName, const char** apcAttributes )
{
	CXMLParse::Get().HandlerStartTag( pUserData, pcElementName, apcAttributes );
}

/***************************************************************************************************
*
*	FUNCTION		StaticHandlerCharacterData
*
***************************************************************************************************/

static void StaticHandlerCharacterData ( void* pUserData, const char* pcCharacterData, int iLength )
{
	CXMLParse::Get().HandlerCharacterData( pUserData, pcCharacterData, iLength );
}


/***************************************************************************************************
*
*	FUNCTION		StaticHandlerExternalEntity
*
***************************************************************************************************/

static int StaticHandlerExternalEntity( XML_Parser obParser, const char* pcContext, const char* pcBase, const char* pcSystemID, const char* pcPublicID )
{
	return CXMLParse::Get().HandlerExternalEntity( obParser, pcContext, pcBase, pcSystemID, pcPublicID );
}


/***************************************************************************************************
*
*	FUNCTION		CXMLParse::CXMLParse
*
*	DESCRIPTION		Construction
*
***************************************************************************************************/

CXMLParse::CXMLParse( void )
{

}


/***************************************************************************************************
*
*	FUNCTION		CXMLParse::~CXMLParse
*
*	DESCRIPTION		Destruction
*
***************************************************************************************************/

CXMLParse::~CXMLParse( void )
{
	
}


/***************************************************************************************************
*
*	FUNCTION		CXMLParse::Initialise
*
*	DESCRIPTION		That's right, it initialises it.
*
***************************************************************************************************/

void CXMLParse::Initialise( void )
{
}


/***************************************************************************************************
*
*	FUNCTION		CXMLParse::CreateTree
*
*	DESCRIPTION		This will create a tree and return a pointer to the root.  Callers of this
*					are responsible for destroying the tree, done by simply deleting the root.
*
***************************************************************************************************/

CXMLElement* CXMLParse::CreateTree( const char* pcResource )
{
	// Create an expat parser
	XML_Parser obParser = XML_ParserCreate( 0 );

	// Make sure that that worked
	ntAssert_p( obParser, ( "I couldn't create an expat parser" ) );

	// Create a struct to manage this parse job
	XML_PARSE strParseInfo = { 0 };

	// Set the parser in the structure
	strParseInfo.obParser = obParser;

	// Set a pointer so the handlers can see this information
	XML_SetUserData( obParser, &strParseInfo );

	// Set the handler for processing instruction tabs
	XML_SetProcessingInstructionHandler( obParser, StaticHandlerProcess );

	// Set the handlers for starting and ending tags
	// These will deal with the element names and attribute pairs
	XML_SetElementHandler( obParser, StaticHandlerStartTag, StaticHandlerEndTag );

	// A call to handle the character data 
	XML_SetCharacterDataHandler( obParser, StaticHandlerCharacterData );

	// A call to handle the file's comments 
	XML_SetCommentHandler( obParser, StaticHandlerComment );

	// Allow external DTD information to be parsed
	XML_SetParamEntityParsing( obParser, XML_PARAM_ENTITY_PARSING_UNLESS_STANDALONE );

	// Set a handler to deal with external DTDs
	XML_SetExternalEntityRefHandler( obParser, StaticHandlerExternalEntity );

	// Load up the xml file
	int iByteFileSize = 0;
	char* pcData = ( char* )CGuiResource::Get().GetResource( pcResource, &iByteFileSize, true );

	// Now get on and do some parsing action
	if ( !XML_Parse( obParser, pcData, iByteFileSize, 1 ) )
	{
		// Show us the ntError
		ntPrintf( "Parse ntError at line %d:\n%s\n", XML_GetCurrentLineNumber( obParser ), XML_ErrorString( XML_GetErrorCode( obParser ) ) );

		// Clean up our mess
		CGuiResource::Get().ReleaseResource( pcResource );
		NT_DELETE( strParseInfo.pobRootElement );
		XML_ParserFree( obParser );

		// Stop here, something is bad
		ntError(0);
	}

	// Give us a look
	strParseInfo.pobRootElement->DebugPrintElement( true );

	// Get rid of the expat parser
	XML_ParserFree( obParser );

	// Release the resource
	CGuiResource::Get().ReleaseResource( pcResource );

	// Point to what we have built - it's theirs now
	return strParseInfo.pobRootElement;
}


/***************************************************************************************************
*
*	FUNCTION		CXMLParse::HandlerProcess
*
*	DESCRIPTION		Deals with any processing instructions found by expat.
*
***************************************************************************************************/

void CXMLParse::HandlerProcess ( void* pUserData, const char* pcTarget, const char* pcData ) 
{
	// Unused formal parameters
	UNUSED( pUserData );
	UNUSED( pcTarget );
	UNUSED( pcData );
} 


/***************************************************************************************************
*
*	FUNCTION		CXMLParse::HandlerStartTag
*
*	DESCRIPTION		Deals with any openning tags found by expat.
*
***************************************************************************************************/

void CXMLParse::HandlerStartTag ( void* pUserData, const char* pcElementName, const char** apcAttributes ) 
{
	// Start a tag - create a new element - this needs to create an object based
	// on the class registered with the element name
	CXMLElement* pobNewElement = CGuiManager::Get().CreateGuiElement( pcElementName );

	// Tell it what it's called
	pobNewElement->SetName( pcElementName );

	// Sort out it's attributes
	for ( int iAttribute = 0; ( apcAttributes[iAttribute] != 0 ); iAttribute += 2 )
	{
		pobNewElement->SetAttribute( apcAttributes[iAttribute], apcAttributes[iAttribute + 1] );
	}

	// Set up the pointers to what's what
	XML_PARSE* strParseInfo = ( XML_PARSE* )pUserData;

	// Do we have a root element yet?
	if ( strParseInfo->pobRootElement == 0 ) 
	{
		strParseInfo->pobRootElement = pobNewElement;
	}
	
	else
	{ 
		// We know there are existing elements
		ntAssert( strParseInfo->pobCurrentElement );

		// Set the new element as a child of the current
		strParseInfo->pobCurrentElement->SetChild( pobNewElement );

		// Set the current as the parent of the new
		pobNewElement->SetParent( strParseInfo->pobCurrentElement );
	}

	// Always set the new element to the current position
	strParseInfo->pobCurrentElement = pobNewElement;
} 


/***************************************************************************************************
*
*	FUNCTION		CXMLParse::HandlerEndTag
*
*	DESCRIPTION		Deals with any tag ends found by expat.
*
***************************************************************************************************/

void CXMLParse::HandlerEndTag ( void* pUserData, const char* pcElementName )
{
	// Unused formal parameters
	UNUSED( pcElementName );

	// Nothing else will be added to this element - time for some checking
	( ( XML_PARSE* )pUserData )->pobCurrentElement->SetEnd();

	// Set the parent of the current item as current
	// It doesn't matter if there isn't one since it will set to null
	( ( XML_PARSE* )pUserData )->pobCurrentElement = ( ( XML_PARSE* )pUserData )->pobCurrentElement->GetParent();
}


/***************************************************************************************************
*
*	FUNCTION		CXMLParse::HandlerCharacterData
*
*	DESCRIPTION		Deals with any character data found by expat.
*
***************************************************************************************************/

void CXMLParse::HandlerCharacterData ( void* pUserData, const char* pcCharacterData, int iLength ) 
{
	// The string we are given isn't null terminated
	char* pcTerminatedCharacterData = NT_NEW char[ iLength + 1 ];
	NT_MEMCPY( pcTerminatedCharacterData, pcCharacterData, iLength * sizeof( char ) );
	pcTerminatedCharacterData[iLength] = 0;

	( ( XML_PARSE* )pUserData )->pobCurrentElement->SetCharacterData( pcTerminatedCharacterData );

	NT_DELETE_ARRAY( pcTerminatedCharacterData );
}


/***************************************************************************************************
*
*	FUNCTION		CXMLParse::HandlerComment
*
*	DESCRIPTION		Deals with any comments found by expat.
*
***************************************************************************************************/

void CXMLParse::HandlerComment ( void* pUserData, const char* pcComment )
{
	// Unused formal parameters
	UNUSED( pUserData );
	UNUSED( pcComment );
}


/***************************************************************************************************
*
*	FUNCTION		CXMLParse::HandlerExternalEntity
*
*	DESCRIPTION		Deals with externally linked files found by expat
*
***************************************************************************************************/

int CXMLParse::HandlerExternalEntity( XML_Parser obParser, const char* pcContext, const char* pcBase, const char* pcSystemID, const char* pcPublicID )
{
	// Unused formal parameters
	UNUSED( pcBase );
	UNUSED( pcPublicID );

	// Create an external expat parser - built from the settings of the existing parser
	XML_Parser obExternalParser = XML_ExternalEntityParserCreate( obParser, pcContext, 0 );

	// Make sure that that worked
	if ( !obExternalParser ) 
		ntPrintf( "I couldn't create an external expat parser" );

	// Load up the xml file
	int iByteFileSize = 0;
	char* pcData = ( char* )CGuiResource::Get().GetResource( pcSystemID, &iByteFileSize, true );
	
	// Now get on and do some parsing action
	if ( !XML_Parse( obExternalParser, pcData, iByteFileSize, 1 ) )
	{
		ntPrintf( "Parse ntError at line %d:\n%s\n", XML_GetCurrentLineNumber( obExternalParser ), XML_ErrorString( XML_GetErrorCode( obExternalParser ) ) );

		// ntError(0);
		return 0;
	}

	// Get rid of the expat parser
	XML_ParserFree( obExternalParser );

	// Release the resource
	CGuiResource::Get().ReleaseResource( pcSystemID );

	return 1;
}

