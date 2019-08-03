//------------------------------------------------------
//!
//!	\file core\guidclient.h
//!
//------------------------------------------------------
#if !defined(CORE_GUIDCLIENT_H)
#define CORE_GUIDCLIENT_H


/**************************************************************************************************
*
*	CLASS			GuidClient
*
*	DESCRIPTION
*
*	This used provides a client interface to the Guid Server.
*	The GuidServer was a pain in the arse so now this uses real MS GUID which are bigger but
*   require no communication with any other machine. 
*	Renameing is far less effective though..
*
*   Big problem is that this is Windows Only...
*
*
*
**************************************************************************************************/
class GuidClient : public Singleton<GuidClient>
{
public:
	GuidClient();
	~GuidClient();


	// Look up the name, return guid in pcResultBuf
	bool Lookup( const char* name, char* pcResultBuf, int iResultBufSize );

	// Look up the guid, return string in pcResultBuf
	bool LookupGuid( const char* name, char* pcResultBuf, int iResultBufSize );

	// Tell the server to add the string to its list
	bool Add( const char* pcString );

	bool AddUnique( char* pcString );

	bool Remove( const char* guid );

	bool Rename( const char* guid, const char* newname );

private:
	class GuidClientImpl& m_impl;
};

#endif
