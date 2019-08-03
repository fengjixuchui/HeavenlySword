/***************************************************************************************************
*
*	DESCRIPTION		A general purpose XML parser and base XML element for building trees
*
*	NOTES			The parser builds a tree of XML Elements from a given file.  Items to be
*					created by parsing should be derived from the CXMLElement object.
*
***************************************************************************************************/

#ifndef	_GUIPARSE_H
#define	_GUIPARSE_H

// Forward declarations
typedef void* XML_Parser;

/***************************************************************************************************
*
*	CLASS			CXMLElement
*
*	DESCRIPTION		A simple class to hold the string information representing an XML element.  
*
*					Anything that is built from an XML file should inherit from these.
*
***************************************************************************************************/

class CXMLElement
{
public:

	//! Construction Destruction
	CXMLElement( void );
	virtual ~CXMLElement( void );

	//! Setup
	void	SetName( const char* pcName );
	void	SetCharacterData( const char* pcCharacterData );
	void	SetAttribute( const char* pcTitle, const char* pcValue );
	void	SetChild( CXMLElement* pobChild, CXMLElement* pobInsertBefore = NULL );
	void	SetParent( CXMLElement* pobParent );
	void	SetEnd( void );

	//! Access to element contents
	CXMLElement*	GetParent( void ) { return m_pobParent; }

	//! Debugging assistance
	void	DebugPrintElement( bool bSiblings );

	//get element name
	const char* GetName( void ) const				{ return m_pcElementName; }
	//get our children
	ntstd::List<CXMLElement*>& GetChildren( void )	{ return m_obChildren; }
	//Lookup an attribute
	const char* GetAttribute( const char* szAttr ) const;

	//remove one of our children (doesnt delete)
	void RemoveChild( CXMLElement* pobChild );

	// Free the element name string of pobRoot and its children
	static void FreeNameStrings(CXMLElement* pobRoot);

	virtual void	PostProcessEnd( void );

protected:
	
	//! Hooks for items derived from this to do their own processing 
	virtual bool	ProcessName( const char* pcName );
	virtual bool	ProcessCharacterData( const char* pcCharacterData );
	virtual bool	ProcessAttribute( const char* pcTitle, const char* pcValue );
	virtual bool	ProcessChild( CXMLElement* pobChild );
	virtual bool	ProcessParent( CXMLElement* pobParent );
	virtual bool	ProcessEnd( void );

	//! Helpers
	int				GetElementDepth( void );

	//! Structure to hold attribute pairs
	struct XML_ATTRIBUTE
	{
		const char*		pcTitle;
		const char*		pcValue;
	};

	//! The name of the element
	const char*				m_pcElementName;

	//! Any character data it may contain
	const char*				m_pcCharacterData;

	//! The attributes of this element
	ntstd::List<XML_ATTRIBUTE*>	m_obAttributes;

	//! A list of this elements children
	ntstd::List<CXMLElement*>		m_obChildren;

	//! The parent of this element
	CXMLElement*			m_pobParent;

};


/***************************************************************************************************
*
*	CLASS			CXMLParse
*
*	DESCRIPTION		A wrapper for the utilised XML parser.
*
***************************************************************************************************/

class CXMLParse : public Singleton<CXMLParse>
{
public:

	//! Construction Destruction
	CXMLParse( void );
	~CXMLParse( void );

	// Methods
	void			Initialise( void );
	CXMLElement*	CreateTree( const char* pcResource );

	// Handlers for parser events
	void	HandlerEndTag ( void* pUserData, const char* pcElementName );
	void	HandlerComment ( void* pUserData, const char* pcComment );
	void	HandlerProcess ( void* pUserData, const char* pcTarget, const char* pcData );
	void	HandlerStartTag ( void* pUserData, const char* pcElementName, const char** apcAttributes );
	void	HandlerCharacterData ( void* pUserData, const char* pcCharacterData, int iLength );
	int		HandlerExternalEntity( XML_Parser obParser, const char* pcContext, const char* pcBase, const char* pcSystemID, const char* pcPublicID );

protected:

	// Our parser user data
	struct XML_PARSE
	{
		CXMLElement*	pobCurrentElement;
		CXMLElement*	pobRootElement;
		XML_Parser		obParser;
	};

};

#endif // _GUIPARSE_H
