//--------------------------------------------------------------------------------------------------
/**
	@file
	
	@brief		Object Manager

	@note		(c) Copyright Sony Computer Entertainment 2005. All Rights Reserved.
**/
//--------------------------------------------------------------------------------------------------

#ifndef	FP_OBJECT_MANAGER_H
#define	FP_OBJECT_MANAGER_H

#include <Fw/FwStd/FwStdMap.h>
#include <Fw/FwStd/FwStdIntrusivePtr.h>
#include <Fw/FwStd/FwHashedString.h>
#include <Fp/FpResource/FpResource.h>


//--------------------------------------------------------------------------------------------------
/**
	@class			FpObjectManager
	
	@brief

	General points.. used to manage sharing of C++ objects with hashed strings, dishing out handles
	the objects in question.

	By default, objects must have a static Create() call taking an FwResourceHandle	object, and 
	return a handle based on FwStd's intrusive pointer class.  This default behaviour can be
	overridden in derived classes by assigning a new function to m_pCreationFunc.

	If objects point to FwResourceHandle's data, then they *must* contain a copy of the handle 
	to prevent the resource system from deleting it! 

	Applications use Get() - with optional 'autoLoad' parameter - to acquire a handle to a shared
	resource (texture, shader etc). 

	Updating the shared object **WILL AFFECT ALL OTHERS REFERRING TO THAT OBJECT**. If you want a
	unique object, then this is not for you!

**/
//--------------------------------------------------------------------------------------------------

template <typename T>
class FpObjectManager
{
public:
	// Construction
	FpObjectManager();

	// Types
	typedef	FwStd::IntrusivePtr<T>						ObjectHandle;
	typedef	FwStd::Map<FwHashedString, ObjectHandle>	ObjectMap;

	// Standard retrieval, potentially adding if necessary
	ObjectHandle		GetObject( const char* pName, bool autoLoad = true );
	ObjectHandle		GetObject( FwHashedString hashedName );

	// Users may want to add constructed objects directly (via name/handle group)
	void				Add( const char* pName, const ObjectHandle& handle );
	void				Add( FwHashedString hashedName, const ObjectHandle& handle );

	// Similarly, they will eventually need to remove the entries from the list
	void				Remove( const char* pName );
	void				Remove( FwHashedString hashedName );

	// Users will probably want to query how many objects are being managed
	int					GetObjectCount( void ) const;

protected:
	// The creation function to use to create objects from resources
	typedef ObjectHandle ( *CreationFunc )( const FwResourceHandle& );
	CreationFunc		m_pCreationFunc;

	// The default creation function that calls T::Create
	static ObjectHandle	GenericCreateObject( const FwResourceHandle& hResource );

private:

	// Here's our map associating hashed strings with C++ object handles..
	ObjectMap			m_objectMap;
};

//--------------------------------------------------------------------------------------------------
/**
	@brief			Constructs the object manager with the default creation function.
**/
//--------------------------------------------------------------------------------------------------

template <typename T>
inline FpObjectManager<T>::FpObjectManager()
{
	m_pCreationFunc = &FpObjectManager<T>::GenericCreateObject;
}

//--------------------------------------------------------------------------------------------------
/**
	@brief			Retrieves (and possibly loads) a handle to a shared object via the name.

	@param			pName			-	A valid item number
	@param			autoLoad		-	If true then the object will be loaded if it's not already in
										memory. If false then we will assert if we can't find the
										object in the manager.

	@result			handle			-	A handle to the object.
**/
//--------------------------------------------------------------------------------------------------

template <typename T>
inline typename FpObjectManager<T>::ObjectHandle FpObjectManager<T>::GetObject( const char* pName, bool autoLoad )
{
	// This keeps the compiler happy on RELEASE builds.
	FW_UNUSED( autoLoad );
	
	FwHashedString	hashedName( pName );

	// Look for the object..
	typename ObjectMap::iterator position = m_objectMap.find( hashedName );

	if ( position != m_objectMap.end() )
	{
		// We found it.. so return a handle to the object..
		return position->second;
	}
	else
	{
		// If it's not there, we've got to either bail, or load it.. 
		FW_ASSERT( autoLoad );
	
		// Load the resource up, and add the created object to the 
		ObjectHandle	objHandle = m_pCreationFunc( FwResourceHandle( pName ) );
		m_objectMap.insert( typename ObjectMap::value_type( hashedName, objHandle ) );
		
		// Finally, return the handle.
		return objHandle;
	}
}


//--------------------------------------------------------------------------------------------------
/**
	@brief			Retrieves (and possibly loads) a handle to a shared object via a hashed string.

	@param			hashedName		-	Hashed name constructed from the objects textual name.

	@result			handle			-	A handle to the object.
**/
//--------------------------------------------------------------------------------------------------

template <typename T>
inline typename FpObjectManager<T>::ObjectHandle FpObjectManager<T>::GetObject( FwHashedString hashedName )
{
	// Look for the object..
	typename ObjectMap::iterator position = m_objectMap.find( hashedName );

	if ( position != m_objectMap.end() )
	{
		// We found it.. so return a handle to the object..
		return position->second;
	}
	else
	{
		// We didn't find it.. so for now we'll assert. 
		FW_ASSERT( false );
		return ObjectHandle();
	}
}	


//--------------------------------------------------------------------------------------------------
/**
	@brief			Adds a pre-constructed object to the manager with a given name

	@param			pName			-	Name associated with the object (must not exist already)
	@param			handle			-	Handle to the object which we want to manage.
**/
//--------------------------------------------------------------------------------------------------

template <typename T>
inline	void	FpObjectManager<T>::Add( const char* pName, const typename FpObjectManager<T>::ObjectHandle& handle )
{
	FwHashedString hashedName( pName );

	typename ObjectMap::iterator position = m_objectMap.find( hashedName );
	if ( position == m_objectMap.end() )
	{
		m_objectMap.insert( typename ObjectMap::value_type( hashedName, handle ) );
	}
	else
	{
		FW_ASSERT_MSG( false, ( "%s : Attempt to add an object (%s) that already exists.\n", __FUNCTION__, pName ) );
	}
}


//--------------------------------------------------------------------------------------------------
/**
	@brief			Adds a pre-constructed object to the manager with a given hashed name

	@param			hashedName		-	Hashed name associated with the object (must not exist already)
	@param			handle			-	Handle to the object which we want to manage.
**/
//--------------------------------------------------------------------------------------------------

template <typename T>
inline	void	FpObjectManager<T>::Add( FwHashedString hashedName, const typename FpObjectManager<T>::ObjectHandle& handle )
{
	typename ObjectMap::iterator position = m_objectMap.find( hashedName );
	if ( position == m_objectMap.end() )
	{
		m_objectMap.insert( typename ObjectMap::value_type( hashedName, handle ) );
	}
	else
	{
		FW_ASSERT_MSG( false, ( "%s : Attempt to add an object (Hash: 0x%08x) that already exists.\n", __FUNCTION__, hashedName.Get() ) );
	}
}

//--------------------------------------------------------------------------------------------------
/**
	@brief			Removes an (unreferenced) item from the manager.

	@param			pName			-	Name of the object we want to remove. The object must be 
										currently managed, and must have no outstanding references.
**/
//--------------------------------------------------------------------------------------------------

template <typename T>
inline	void	FpObjectManager<T>::Remove( const char* pName )
{
	typename ObjectMap::iterator position = m_objectMap.find( FwHashedString( pName ) );
	FW_ASSERT( position != m_objectMap.end() );
	m_objectMap.erase( position );
}

//--------------------------------------------------------------------------------------------------
/**
	@brief			Removes an unreferenced item from the manager, accessed by hashed string

	@param			hashedName		-	Hashed name of the object we want to remove. Object must be
										currently managed, and must have no outstanding references.
**/
//--------------------------------------------------------------------------------------------------

template <typename T>
inline	void	FpObjectManager<T>::Remove( FwHashedString hashedName )
{
	typename ObjectMap::iterator position = m_objectMap.find( hashedName );
	FW_ASSERT( position != m_objectMap.end() );
	m_objectMap.erase( position );
}

//--------------------------------------------------------------------------------------------------
/**
	@brief			Returns the number of objects currently being managed.

	@param			objectCount		-	Number of objects currently being managed.
**/
//--------------------------------------------------------------------------------------------------

template <typename T>
inline	int	FpObjectManager<T>::GetObjectCount( void ) const
{
	return m_objectMap.size();
}

//--------------------------------------------------------------------------------------------------
/**
	@brief			Creates the object using the Create method and the resource.

	@param			
**/
//--------------------------------------------------------------------------------------------------

template <typename T>
inline typename FpObjectManager<T>::ObjectHandle FpObjectManager<T>::GenericCreateObject( const FwResourceHandle& hResource )
{
	return T::Create( hResource );
}

#endif	// FP_OBJECT_MANAGER_H
