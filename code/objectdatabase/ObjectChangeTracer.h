//------------------------------------------------------
//!
//!	\file core\objectchangetracer.h
//!
//------------------------------------------------------
#if !defined(CORE_OBJECTCHANGETRACER_H)
#define CORE_OBJECTCHANGETRACER_H

#include "objectdatabase/gameguid.h"

class netstream;

//------------------------------------------------------
//!
//! This tracks changes to the object database for a 
//! remote client. This change delta then get sent when
//! asked for, so the remote client can stay upto date.
//!
//------------------------------------------------------

class ObjectChangeTracker
{
public:
	virtual ~ObjectChangeTracker();

	void ObjectCreated( const GameGUID& guid, const GameGUID& parentGuid )
	{
		ntstd::String command;
		command = "created " + guid.GetAsString() + " " + parentGuid.GetAsString();
		NotifyTracker(command);
	}
	void ObjectDeleted( const GameGUID& guid, const GameGUID& parentGuid )
	{
		ntstd::String command;
		command = "deleted " + guid.GetAsString() + " " + parentGuid.GetAsString();;
		NotifyTracker(command);
	}
	void ObjectChanged( const GameGUID& guid, const GameGUID& parentGuid )
	{
		ntstd::String command;
		command = "changed " + guid.GetAsString() + " " + parentGuid.GetAsString();;
		NotifyTracker(command);
	}

	virtual void NotifyTracker(ntstd::String command) = 0;
};

#endif
