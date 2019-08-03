
#include "objectdatabase/neteditinterface.h"
#include "objectdatabase/dataobject.h"

/*

Sigh... this whole file is a pile of pants. 

The whole thing should be more object orientated such that objects have methods that can be called on them.
The methods would ideally be registered with the data interface.

E.g. \attacks\att_forward_1.Rename(att_backward1)

Then most of this file would go away! - MB

*/

#define NET_COMMAND_BUFFER_SIZE 256
int g_iNetBufferIndex = 0; //TODO:MB move this
char g_cNetBuffer[NET_COMMAND_BUFFER_SIZE+1];

// Version number of "net protocol", should be updated if any changes are made that change the behaviour of the ODB
const int CNetEditInterface::NET_COMPONENT_VERSION = 5;

void CNetEditInterface::NotifyTracker(ntstd::String command)
{
	if (remoteCallback.isopen())
	{
		remoteCallback << "netupdate " << command.c_str() << "\r\n";
		remoteCallback.EndBlock();
	}
}

void CNetEditInterface::CloseTrackerStream()
{
	if (remoteCallback.isopen())
	{
		remoteCallback << "exit\r\n";
		remoteCallback.EndBlock();
		remoteCallback.close();
		client.close();
	}
}

CNetEditInterface::CNetEditInterface() :
	m_pSelected( 0 ),
	m_pSelectedParent( 0 )
{
	m_bEcho = false;
	Startup();

}


CNetEditInterface::~CNetEditInterface()
{
	ObjectDatabase::Get().RemoveRemoteTracker( this );
	CloseTrackerStream();
	remote.close();
}


bool CNetEditInterface::Startup()
{
#ifdef KEYSTRING_INCLUDE_DEBUG_INFO 
	WinsockStartup();
	g_iNetBufferIndex = 0;

	ObjectDatabase::Get().AddRemoteTracker( this );

	// Place the server in listening state
	return StartListen();
#else
	return false;
#endif
}


bool CNetEditInterface::StartListen()
{
	m_bConnected = false;
	// Place the server in listening state
	if(server.listen(m_iPort) == false)
	{
		ntPrintf("Unable to start server\n");
		return false;
	}
	return true;
}

void CNetEditInterface::Update()
{
	Listen();
	if( m_pSelected )
	{
		StdDataInterface* pInterface = ObjectDatabase::Get().GetInterface( m_pSelected );
		pInterface->CallBack( m_pSelected, "DebugRenderNet" );
	}
}



void CNetEditInterface::Listen()
{
	if (!m_bConnected)
	{
		// See if we can make a connection
		if (server.canaccept())
		{
			if(server.accept(remote))
			{
				// Close the server.
				// Note that this only closes the server socket so new connections
				// cannot be established. It does not close any connection already established.
				server.close();

				// Flag that we are connected
				m_bConnected = true;

				// Say hello.... Lionel Richie stylee
				//remote << "Hello\n";
			}
		}
	}
	else
	{
		// If the connection is suddenly no longer open, canrecv probably noticed
		// that we lost connection to the host and closed it.
		if(remote.isopen() == false)
		{
			ntPrintf("Lost connection to remote host\n");
			CloseTrackerStream(); // make sure this has been closed as well
			StartListen();
			return;
		}

		// Build the command buffer
		if (remote.canrecv() > 0)
		{
			while ((g_iNetBufferIndex < NET_COMMAND_BUFFER_SIZE) && (remote.canrecv() > 0))
			{
				char c;
				// Receive a byte from the stream
				remote >> c;
				if (m_bEcho)
				{
					remote << c;
				}

				g_cNetBuffer[g_iNetBufferIndex] = c;

				if (c != 0x0d)
					g_iNetBufferIndex++;

				if (c == 0x0a)
				{
					// OK process the command
					g_cNetBuffer[g_iNetBufferIndex-1] = 0;
					Dispatch(g_cNetBuffer);
					g_iNetBufferIndex = 0;
				}
			}

			if (g_iNetBufferIndex == NET_COMMAND_BUFFER_SIZE)
			{
				ntPrintf("Command too long");
				// Empty the recieve buffer
				while(remote.canrecv() != 0)
				{
					uint8_t c;
					// Receive a byte from the stream
					remote >> c;
				}

				remote << "Failed\r\n";
				remote.EndBlock();
				g_iNetBufferIndex = 0;

				return;
			}
		}
	}
}




void CNetEditInterface::Dispatch(const char* pcCommand)
{
	// Parse the input
	StringType obCommand(pcCommand);
	StringList obParseList;
//	obCommand.Parse(obParseList, " \t=");
	ntStr::Parse(obCommand, obParseList, " \t=");
	if (obParseList.empty())
	{
		return;
	}
	// Test connection
	if (strstr(pcCommand, "hello")==pcCommand)
	{
		remote << "Right back at'cha";
		remote.EndBlock();
		return;
	}
	// Quit the connection
	else if (strstr(pcCommand, "exit")==pcCommand)
	{
		remote.close();
		CloseTrackerStream();
		return;
	}
	// toggle the echo flag
	else if (strstr(pcCommand, "echo")==pcCommand)
	{
		m_bEcho = !m_bEcho;
		if (m_bEcho)
		{
			remote << "Echo ON\r\n";
		}
		else
		{
			remote << "Echo OFF\r\n";
		}
		return;
	}
	// Query the version number of the net component
	else if (strstr(pcCommand, "version")==pcCommand)
	{
		remote << NET_COMPONENT_VERSION;
		remote.EndBlock();
		return;
	}

	// More complex things!

	// Allow the client to set up a callback stream on a specified port
	else if (strstr(pcCommand, "callback")==pcCommand)
	{
		if (obParseList.size() != 2) 
		{
			//		   0        1
			remote << "callback portnr\r\n";
			remote.EndBlock();
			return;
		}

		int iPortNumber = 0;
		ntstd::Istringstream stringStream(ExtractString(obParseList, 1));
		stringStream >> iPortNumber;

		if (iPortNumber > 0)
		{
			client.connect(iPortNumber, ntStr::GetString(remote.getpeername()), remoteCallback);
			remote << "OK\r\n";
			remote.EndBlock();
		}
		else
		{
			remote << "Failed\r\n";
			remote.EndBlock();
		}
		return;
	}
	// List objects
	else if (strstr(pcCommand, "list")==pcCommand)
	{
		if (obParseList.size() == 2)
		{
			StringList::iterator obIt= obParseList.begin();
			obIt++;
			GameGUID guid;
			guid.SetFromString(*obIt);

			DataObject* pRoot = ObjectDatabase::Get().GetDataObjectFromGUID( guid );
			if( pRoot == 0 )
			{
				remote << "Failed\r\n";
				remote.EndBlock();
				return;
			}
			ObjectDatabase::Get().SaveDataObject( pRoot, &remote, ntstd::String(), true, true );
		}
		else
		{
			remote << "<Container>";

			ObjectDatabase::Get().SaveAllInterfaces( &remote );
			DataObject* pRoot = ObjectDatabase::Get().GetDataObjectFromPointer( ObjectDatabase::Get().GetGlobalContainer() );
			ObjectDatabase::Get().SaveDataObject( pRoot, &remote );

			remote << "</Container>";
		}
		remote.EndBlock();
		return;
	}

	// Set an object
	else if (strstr(pcCommand, "set")==pcCommand)
	{
		ntPrintf("%s\n", pcCommand);
		if ((obParseList.size() < 3) ||(obParseList.size() > 4)) 
		{
			//		   0   1               2	 3
			remote << "set itemGUID>member=value [parent]\r\n";
			remote.EndBlock();
			return;
		}

		ntstd::String sObjAndMember = ExtractString(obParseList,1);
		ntstd::String sObj;
		ntstd::String sMember;

		ntstd::String::iterator pIt = sObjAndMember.begin();
		// Search for place to terminate the string
		while ((pIt != sObjAndMember.end()) && (*pIt != '>'))
		{
			sObj += *pIt;
			pIt++;
		}
		pIt++;// skip the '>'
		while ( pIt != sObjAndMember.end() )
		{
			sMember += *pIt;
			pIt++;
		}

		CHashedString hashMember(sMember);

		// OK, now handle the value
		bool bResult = true;

		GameGUID sObjGuid;
		sObjGuid.SetFromString(sObj);

		DataObject* pDO = ObjectDatabase::Get().GetDataObjectFromGUID( sObjGuid );
		StdDataInterface* pInterface = ObjectDatabase::Get().GetInterface( pDO );
		ntstd::String sFieldDataString = ExtractString(obParseList,2);
		CHashedString hashedFieldData(sFieldDataString);

		// if the source object doesn't exist
		if( pDO == 0 )
		{
			bResult = false;
		} else
		{
			DataInterfaceField* pDIF = pInterface->GetFieldByName( hashMember );
			if( pDIF == 0 )
			{
				bResult = false;
			} else
			{
				if( pDIF->GetMetaData() == "void*" || pDIF->GetMetaData() == "std::list<void*>" || pDIF->GetMetaData() == "ntstd::List<DataObject*>"  )
				{
					GameGUID guid;
					guid.SetFromString( sFieldDataString );
					if( !guid.IsNull() )
					{
						// if this is a reference field check that the target doesn't exist
						// to stop the brain dead backwards compatibily forward reference rubbish
						DataObject* pTargetDO = ObjectDatabase::Get().GetDataObjectFromGUID( guid );
						if(pTargetDO == 0 )
						{
							bResult = false;
						}
					}
				}
			}
		}

		if( bResult )
		{
			if (obParseList.size() == 3) 
			{
				pInterface->SetData( pDO, hashMember, sFieldDataString );
				pInterface->CallBack( pDO, "EditorChangeValue", hashMember, hashedFieldData );
			}
			else
			{
				StdDataInterface* pInterface = ObjectDatabase::Get().GetInterface( pDO );
				pInterface->SetData( pDO, hashMember, sFieldDataString );
				pInterface->CallBack( pDO, "EditorChangeValue", hashMember, hashedFieldData );

				GameGUID parentGuid;
				parentGuid.SetFromString( ExtractString(obParseList, 3) );
				DataObject* pParentDO = ObjectDatabase::Get().GetDataObjectFromGUID( parentGuid );
				StdDataInterface* pParentInterface = ObjectDatabase::Get().GetInterface( pParentDO );
				pParentInterface->CallBack( pParentDO, "EditorChangeParent" );

			}
			remote << "OK\r\n";
		} else
		{
			remote << "Failed\r\n";
		}
		remote.EndBlock();
		return;
	}

	// Interact with an object
	else if (strstr(pcCommand, "interact")==pcCommand)
	{
		//ntPrintf("%s\n", pcCommand);
		if ((obParseList.size()<3)||(obParseList.size()>4)) 
		{
			//         0        1                   2     3
			remote << "interact itemGUID>member=value [parent]\r\n";
			return;
		}

		ntstd::String sObjAndMember = ExtractString(obParseList,1);
		ntstd::String sObj;
		ntstd::String sMember;

		ntstd::String::iterator pIt = sObjAndMember.begin();
		// Search for place to terminate the string
		while ((pIt != sObjAndMember.end()) && (*pIt != '>'))
		{
			sObj += *pIt;
			pIt++;
		}
		pIt++;// skip the '>'
		while ( pIt != sObjAndMember.end() )
		{
			sMember += *pIt;
			pIt++;
		}

		CHashedString	hashMember(sMember);

		GameGUID sObjGuid;
		sObjGuid.SetFromString(sObj);

		if (obParseList.size() == 3) 
		{
			DataObject* pDO = ObjectDatabase::Get().GetDataObjectFromGUID( sObjGuid );
			if (pDO)
			{
				StdDataInterface* pInterface = ObjectDatabase::Get().GetInterface( pDO );
				pInterface->EditData( pDO, hashMember, ExtractString(obParseList,2) );
				pInterface->CallBack( pDO, "EditorChangeValue", hashMember, CHashedString(ExtractString(obParseList,2)) );
			}
		}
		else
		{
			DataObject* pDO = ObjectDatabase::Get().GetDataObjectFromGUID( sObjGuid );
			if (pDO)
			{
				StdDataInterface* pInterface = ObjectDatabase::Get().GetInterface( pDO );
				pInterface->EditData( pDO, hashMember, ExtractString(obParseList,2) );
				pInterface->CallBack( pDO, "EditorChangeValue", hashMember, CHashedString(ExtractString(obParseList,2)) );

				GameGUID parentGuid;
				parentGuid.SetFromString( ExtractString(obParseList, 3) );
				DataObject* pParentDO = ObjectDatabase::Get().GetDataObjectFromGUID( parentGuid );
				StdDataInterface* pParentInterface = ObjectDatabase::Get().GetInterface( pParentDO );
				pParentInterface->CallBack( pParentDO, "EditorChangeParent" );

			}		
		}

		remote << "OK\r\n";
		remote.EndBlock();

		// NO REPLY FROM INTERACT COMMANDS!!
		return;
	}
	// Create an object
	else if (strstr(pcCommand, "create")==pcCommand)
	{
		ntPrintf("%s\n", pcCommand);
		if (obParseList.size() != 5 && obParseList.size() != 6 ) 
		{
			//			0     1      2    3  4                         5
			remote << "create object name in containerGUID||objectGUID field\r\n";
			return;
		}

		// Create the object
		//const char* pcResult = "Failed\r\n";
		const char* pClassOrig = ExtractString(obParseList,1);
		const char* pName = ExtractString(obParseList,2);
		const char* pParentGuidName = ExtractString(obParseList,4);

        static char pObjectContainerString[] = "ObjectContainer";
		const char* pClass = pClassOrig;

		ntstd::String guidResult;

		if( strcmp(pClassOrig,"Container") == 0 || strcmp(pClassOrig,"ObjectContainer") == 0 )
		{
			if ( !ObjectDatabase::Get().IsValidContainerName( pName ) ) {
				remote << "Failed\r\n";
				remote.EndBlock();
				return;
			}
			pClass = pObjectContainerString; // make sure it's always "ObjectContainer", never "Container"
		} else {	
			if ( !ObjectDatabase::Get().IsValidObjectName( pName ) ) {
				remote << "Failed\r\n";
				remote.EndBlock();
				return;
			}
		}

		GameGUID parentGuid;
		parentGuid.SetFromString( pParentGuidName) ;
		DataObject* pDO = ObjectDatabase::Get().GetDataObjectFromGUID( parentGuid );
		ntAssert( pDO != 0 );
		DataObject* pNewDO;
		if( ntstd::String(pDO->GetClassName()) == "ObjectContainer" )
		{
			pNewDO = ObjectDatabase::Get().ConstructObject( pClass, pName, GameGUID(), (ObjectContainer*) pDO->GetBasePtr() );
		} else
		{
			CHashedString pFieldName = ExtractString(obParseList,5);

			ObjectContainer* pContainer = pDO->GetParent();

			StdDataInterface* pInterface = ObjectDatabase::Get().GetInterface( pDO );
			pNewDO = ObjectDatabase::Get().ConstructObject( pClass, pName, GameGUID(), pContainer );

			ntstd::String data = ntstd::String("A ") + pNewDO->GetGUID().GetAsString();
			pInterface->EditData( pDO, pFieldName, data );
			pInterface->CallBack( pDO, "EditorChangeValue", pFieldName, CHashedString(data.c_str()) );
		}

		guidResult = pNewDO->GetGUID().GetAsString();
		guidResult += "\r\n";

		remote << guidResult.c_str();
		remote.EndBlock();
		return;
	}

	// Delete an object
	else if (strstr(pcCommand, "delete")==pcCommand)
	{
		ntPrintf("%s\n", pcCommand);
		if (obParseList.size() != 2) 
		{
			//			0     1     
			remote << "delete GUID\r\n";
			return;
		}

		// Create the object
		const char* pcResult = "Failed\r\n";

		GameGUID guid;
		guid.SetFromString( ExtractString(obParseList,1) );
		DataObject* pDO = ObjectDatabase::Get().GetDataObjectFromGUID( guid );

		if( pDO == 0 )
		{
 			remote << pcResult;
			remote.EndBlock();
			return;
		}

		// deselect only if we need to
		if (m_pSelected == pDO || m_pSelectedParent == pDO)
		{
			m_pSelected = 0;
			m_pSelectedParent = 0;
		}

		// Let the object know it's about to be destroyed.
		StdDataInterface* pInterface = ObjectDatabase::Get().GetInterface( pDO );
		ntError( pInterface != NULL );
		pInterface->CallBack( pDO, "DeleteObject" );

		ObjectDatabase::Get().DestroyObject( pDO );
		pcResult = "OK\r\n";
		remote << pcResult;
		remote.EndBlock();
		return;
	}


	// Duplicate an object
	else if (strstr(pcCommand, "duplicate")==pcCommand)
	{
		if ((obParseList.size() < 3) || (obParseList.size() >3)) 
		{
			//         0         1          2
			remote << "duplicate sourceGUID destinationGUID\r\n";
			return;
		}

		// Create the object
		const char* pcResult = "Failed\r\n";

		// Get pointers to our source and destination
		GameGUID sourceGuid, destinationGuid;
		sourceGuid.SetFromString( ExtractString(obParseList,1) );
		destinationGuid.SetFromString( ExtractString(obParseList,2) );
		DataObject* pSO = ObjectDatabase::Get().GetDataObjectFromGUID( sourceGuid );
		DataObject* pDO = ObjectDatabase::Get().GetDataObjectFromGUID( destinationGuid );

		if (pSO && pDO)
		{
			// Copy the object
			// Get the interface
			DataInterface* pDestInterface = ObjectDatabase::Get().GetInterface( pDO->GetClassName() );

			DataInterface::iterator destIt = pDestInterface->begin();

			while (destIt != pDestInterface->end())
			{
				pDestInterface->SetData(pDO, (*destIt)->GetName(), (*destIt)->GetData(pSO));
				destIt++;
			}

			pcResult = "OK\r\n";
		}
		
		remote << pcResult;
		remote.EndBlock();
		return;
	}

	// Rename an object
	else if (strstr(pcCommand, "rename")==pcCommand)
	{
		ntPrintf("%s\n", pcCommand);
		if ((obParseList.size() < 3) || (obParseList.size() >3)) 
		{
			//			0     1    2
			remote << "rename GUID newname\r\n";
			return;
		}

		// Create the object
		const char* pcResult = "Failed\r\n";

		GameGUID guid;
		guid.SetFromString( ExtractString(obParseList,1) );
		DataObject* pDO = ObjectDatabase::Get().GetDataObjectFromGUID( guid );

		ntstd::String newName = ExtractString(obParseList,2);
		if (ObjectDatabase::Get().RenameDataObject( pDO, newName ))
		{
			StdDataInterface* pInterface = ObjectDatabase::Get().GetInterface( pDO );
			pInterface->CallBack( pDO, "EditorRenameObject", CHashedString(newName) );
			pcResult = "OK\r\n";
		}

		remote << pcResult;
		remote.EndBlock();
		return;
	}

	// Select an object
	else if (strstr(pcCommand, "select")==pcCommand)
	{
		ntPrintf("%s\n", pcCommand);
		if ((obParseList.size() < 2) || (obParseList.size() >3)) 
		{
			//         0      1    2
			remote << "select GUID [parentGUID]\r\n";
			return;
		}

		// Create the object
		const char* pcResult = "Failed\r\n";

		GameGUID guid;
		guid.SetFromString( ExtractString(obParseList,1) );
		DataObject* pDO = ObjectDatabase::Get().GetDataObjectFromGUID( guid );
		if (pDO)
		{
			StdDataInterface* pInterface = ObjectDatabase::Get().GetInterface( pDO );
			pInterface->CallBack( pDO, "EditorSelect", 1 ); // param1 == 1 for select
			m_pSelected = pDO;

			pcResult = "OK\r\n";
		}

		// If a parent is selected
		if (obParseList.size() == 3)
		{
			guid.SetFromString( ExtractString(obParseList,2) );
			DataObject* pParentDO = ObjectDatabase::Get().GetDataObjectFromGUID( guid );
			if (pParentDO)
			{
				StdDataInterface* pInterface = ObjectDatabase::Get().GetInterface( pParentDO );
				pInterface->CallBack( pParentDO, "EditorSelectParent", 1 ); // param1 == 1 for select
				m_pSelectedParent = pParentDO;

				pcResult = "OK\r\n";
			}
		}

		remote << pcResult;
		remote.EndBlock();
		return;
	}


	// Deselect an object
	else if (strstr(pcCommand, "deselect")==pcCommand)
	{
		ntPrintf("%s\n", pcCommand);

		// Create the object
		const char* pcResult = "Failed\r\n";

		// not entirely sure why this happens but its relatively 
		if( m_pSelected == 0 )
		{
			pcResult = "OK\r\n";
			remote << pcResult;
			remote.EndBlock();
			return;
		}
		if ((obParseList.size() < 2) || (obParseList.size() >3)) 
		{
			//         0        1    2
			remote << "deselect GUID [parentGUID]\r\n";
			return;
		}


		GameGUID guid;
		guid.SetFromString( ExtractString(obParseList,1) );
		DataObject* pDO = ObjectDatabase::Get().GetDataObjectFromGUID( guid );
		if (pDO)
		{
			if ( pDO != m_pSelected )
			{
				remote << "Error\r\n";
				remote.EndBlock();
				return;
			}
			StdDataInterface* pInterface = ObjectDatabase::Get().GetInterface( pDO );
			pInterface->CallBack( m_pSelected, "EditorSelect", (const char*)0 ); // param0 == 0 for deselect
			m_pSelected = 0;

			pcResult = "OK\r\n";
		}
		if (obParseList.size() == 3) 
		{
			guid.SetFromString( ExtractString(obParseList,2) );
			DataObject* pParentDO = ObjectDatabase::Get().GetDataObjectFromGUID( guid );
			if (pParentDO)
			{
				ntAssert( pParentDO == m_pSelectedParent );
				StdDataInterface* pInterface = ObjectDatabase::Get().GetInterface( pParentDO );
				pInterface->CallBack( pParentDO, "EditorSelectParent", (const char*)0 ); // param0 == 0 for deselect
				m_pSelectedParent = 0;
				pcResult = "OK\r\n";
			}

		}

		remote << pcResult;
		remote.EndBlock();
		return;
	}
	// Save a container
	else if (strstr(pcCommand, "save")==pcCommand)
	{
		if (obParseList.size() != 2) 
		{
			//			0     1       
			remote << "save GUID\r\n";
			return;
		}

		const char* pcResult = "Failed\r\n";

		GameGUID guid;
		guid.SetFromString( ExtractString(obParseList,1) );


		//GameGUID guid;
		//guid.SetFromString(
		DataObject* pobCont = ObjectDatabase::Get().GetDataObjectFromGUID( guid );
		if (pobCont)
		{
			// Find the named container

#ifdef PLATFORM_PS3
			char acBuffer[ MAX_PATH ];
			Util::GetFiosFilePath( ntStr::GetString(pobCont->GetName()), acBuffer );
			const char* pcContName = acBuffer;
#else
			const char* pcContName = ntStr::GetString(pobCont->GetName());
#endif

			// Need to ntAssert that the container is a file

#ifndef PLATFORM_PS3
			// Backup the original file
			char cBuffer[512];
			ntAssert(strlen(pcContName) < 507);
			strcpy(cBuffer, pcContName);
			strcat(cBuffer, ".bak");
			CopyFile(pcContName, cBuffer, false);
#endif
			// Write the file
			IOOfstream out(pcContName);
			if (out.good())
			{
				ObjectDatabase::Get().SaveDataObject( pobCont, &out, pcContName );
				pcResult = "OK\r\n";
			}
		}
		else
		{
			remote << "Could not find specified container\r\n";
		}

		// Create the object
		remote << pcResult;
		remote.EndBlock();
		return;
	}

	// Get an object
	else if (strstr(pcCommand, "get")==pcCommand)
	{
		ntPrintf("%s\n", pcCommand);
		if (obParseList.size() != 2) 
		{
			//		   0   1    	
			remote << "get GUID>member\r\n";
			return;
		}

		ntstd::String sObjAndMember = ExtractString(obParseList,1);
		ntstd::String sObj;
		ntstd::String sMember;

		ntstd::String::iterator pIt = sObjAndMember.begin();
		// Search for place to terminate the string
		while ((pIt != sObjAndMember.end()) && (*pIt != '>'))
		{
			sObj += *pIt;
			pIt++;
		}
		pIt++;// skip the '>'
		while ( pIt != sObjAndMember.end() )
		{
			sMember += *pIt;
			pIt++;
		}

		CHashedString hashMember(sMember);

		ntstd::String obResult;

		GameGUID sObjGuid;
		sObjGuid.SetFromString( sObj );
		DataObject* pDO = ObjectDatabase::Get().GetDataObjectFromGUID( sObjGuid );
		if (pDO)
		{
			StdDataInterface* pInterface = ObjectDatabase::Get().GetInterface( pDO );
			obResult = pInterface->GetData( pDO, hashMember);
		}

		// OK, now handle the value
		if ( obResult.empty() )
		{
			remote << "Failed";
		}
		else
		{
			remote << obResult.c_str();
		}
		remote.EndBlock();
		return;
	}

	else if (strstr(pcCommand, "reparent")==pcCommand)
	{
		ntPrintf("%s\n", pcCommand);
		if (obParseList.size() != 3) {
			//         0        1        2
			remote << "reparent GUID-bag GUID-new-parent\r\n";
			return;
		}

		ntstd::String sObj = ExtractString(obParseList, 1);
		ntstd::String sParent = ExtractString(obParseList, 2);

		GameGUID sObjGuid;
		sObjGuid.SetFromString( sObj );
		DataObject* pDataObject = ObjectDatabase::Get().GetDataObjectFromGUID( sObjGuid );
		
		GameGUID sParentGuid;
		sParentGuid.SetFromString( sParent );
		DataObject* pDataObjectParent = ObjectDatabase::Get().GetDataObjectFromGUID( sParentGuid );

		if( pDataObject != 0 && pDataObjectParent != 0 && ntstd::String(pDataObjectParent->GetClassName()) == "ObjectContainer" )
		{
			ObjectDatabase::Get().ReparentObject( pDataObject, (ObjectContainer*)pDataObjectParent->GetBasePtr() );
			m_pSelected = 0;
			m_pSelectedParent = 0;
			remote << "OK\r\n";
		}
		else
		{
			remote << "Failed\r\n";
		}
		remote.EndBlock();
		return;
	}

	else if (strstr(pcCommand, "unlink")==pcCommand)
	{
		ntPrintf("%s\n", pcCommand);
		if (obParseList.size() != 4) {
			//         0        1       2         3
			remote << "unlink GUID-list list-name GUID-element\r\n";
			remote.EndBlock();
			return;
		}

		ntstd::String sList = ExtractString(obParseList, 1);
		CHashedString sVariable = ExtractString(obParseList, 2);
		ntstd::String sElement = ExtractString(obParseList, 3);

		GameGUID obListGuid;
		obListGuid.SetFromString( sList );
		DataObject* pDataObject = ObjectDatabase::Get().GetDataObjectFromGUID( obListGuid );
		StdDataInterface* pInterface = ObjectDatabase::Get().GetInterface( pDataObject );
		pInterface->EditData( pDataObject, sVariable, "D " + sElement );
	
		pInterface->CallBack( pDataObject, "EditorChangeValue", CHashedString(sVariable), CHashedString(sElement) );


		remote << "OK\r\n";
		remote.EndBlock();
		return;
	}

	else if (strstr(pcCommand, "link")==pcCommand)
	{

		ntPrintf("%s\n", pcCommand);
		if (obParseList.size() != 4) {
			//         0        1       2         3
			remote << "link GUID-list list-name GUID-element\r\n";
			remote.EndBlock();
			return;
		}

		ntstd::String sList = ExtractString(obParseList, 1);
		CHashedString sVariable = ExtractString(obParseList, 2);
		ntstd::String sElement = ExtractString(obParseList, 3);

		GameGUID obListGuid;
		obListGuid.SetFromString( sList );
		DataObject* pDataObject = ObjectDatabase::Get().GetDataObjectFromGUID( obListGuid );
		StdDataInterface* pInterface = ObjectDatabase::Get().GetInterface( pDataObject );
		pInterface->EditData( pDataObject, sVariable, "A " + sElement );

		pInterface->CallBack( pDataObject, "EditorChangeValue", CHashedString(sVariable), CHashedString(sElement) );
		
		remote << "OK\r\n";
		remote.EndBlock();
		return;
	}

	else if (strstr(pcCommand, "fill")==pcCommand)
	{
		if (obParseList.size() != 4) {
			//         0    1           2    3
			remote << "fill GUID-bag with GUID-bag\r\n";
			remote.EndBlock();
			return;
		}

		ntstd::String sTarget = ExtractString(obParseList, 1);
		ntstd::String sSource = ExtractString(obParseList, 3);

		GameGUID obTargetGuid;
		obTargetGuid.SetFromString( sTarget );
		GameGUID obSourceGuid;
		obSourceGuid.SetFromString( sSource );
		
		ntstd::String sResult = "Failed\r\n";
		if ( !obTargetGuid.IsNull() && !obSourceGuid.IsNull() )
		{
			DataObject* pTargetObject = ObjectDatabase::Get().GetDataObjectFromGUID( obTargetGuid );
			DataObject* pSourceObject = ObjectDatabase::Get().GetDataObjectFromGUID( obSourceGuid );
			if ( pTargetObject && pSourceObject )
	{
				StdDataInterface* pTargetInterface = ObjectDatabase::Get().GetInterface( pTargetObject );
				StdDataInterface* pSourceInterface = ObjectDatabase::Get().GetInterface( pSourceObject );
				if ( pTargetInterface == pSourceInterface )
				{
					// ready to fill the target with the source!
					DataInterface::iterator targetIt = pTargetInterface->begin();
					while (targetIt != pTargetInterface->end())
		{
						pTargetInterface->SetData(pTargetObject, (*targetIt)->GetName(), (*targetIt)->GetData(pSourceObject));
						targetIt++;
					}

					sResult = "OK\r\n";
				}
			}
		}
		
		remote << sResult.c_str();
		remote.EndBlock();
		return;
	}
	
	else if (strstr(pcCommand, "control-list")==pcCommand)
	{
		if (obParseList.size() < 4 || obParseList.size() > 6) {
			//         0    1            2         3         4       5
			remote << "control-list add|del|edit GUID-list list-name [index] [new-value]\r\n";
			remote.EndBlock();
			return;
		}

		ntstd::String sResult = "OK\r\n";
		ntstd::String sCommandType = ExtractString(obParseList, 1);
		ntstd::String sList = ExtractString(obParseList, 2);
		ntstd::String sVariable = ExtractString(obParseList, 3);
		GameGUID obListGuid;
		obListGuid.SetFromString( sList );
		DataObject* pListDO = ObjectDatabase::Get().GetDataObjectFromGUID( obListGuid );
		if ( pListDO == 0) {
			sResult = "Failed\r\n";
		}
		else
		{
			StdDataInterface* pListInterface = ObjectDatabase::Get().GetInterface( pListDO );
			if (sCommandType == "add")
			{
				ntstd::String sDefaultValue = ExtractString(obParseList, 4);
				pListInterface->EditData( pListDO, CHashedString(sVariable), "A " + sDefaultValue );
			}
			else if (sCommandType == "delete")
			{
				if (obParseList.size() != 5) {
					sResult = "Failed\r\n";
				}
				else
				{
					ntstd::String sIndex = ExtractString(obParseList, 4);
					pListInterface->EditData( pListDO, CHashedString(sVariable), "D " + sIndex );
				}
			}
			else if (sCommandType == "edit")
			{
				if (obParseList.size() != 6) {
					sResult = "Failed\r\n";
				}
				else
				{
					ntstd::String sIndex = ExtractString(obParseList, 4);
					ntstd::String sNewValue = ExtractString(obParseList, 5);
					pListInterface->EditData( pListDO, CHashedString(sVariable), "E " + sIndex + " " + sNewValue);
				}
			}
			else
			{
				sResult = "fill add|del|edit GUID-list list-name [index] [new-value]\r\n";
			}

			// Adding the editor change value call back - JML
			ntstd::String sFieldDataString = ExtractString(obParseList,2);
			CHashedString hashedFieldData(sFieldDataString);
			pListInterface->CallBack( pListDO, "EditorChangeValue", CHashedString(sVariable), hashedFieldData );
		}

		remote << sResult.c_str();
		remote.EndBlock();
		return;
	}

	// Select a member
	else if (strstr(pcCommand, "member")==pcCommand)
	{
		if ((obParseList.size() < 2) || (obParseList.size() >2)) 
		{
			//			0     1		2
			remote << "member name\r\n";
			return;
		}

		remote << "OK\r\n";
		remote.EndBlock();
		return;
	}


	// Handle everything else

	remote << "Unknown command - " << pcCommand;
	remote.EndBlock();
}
