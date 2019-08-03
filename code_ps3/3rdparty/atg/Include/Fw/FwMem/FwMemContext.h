//--------------------------------------------------------------------------------------------------
/**
	@file		
	
	@brief		Memory Contexts

	@note		(c) Copyright Sony Computer Entertainment 2004. All Rights Reserved.	
**/
//--------------------------------------------------------------------------------------------------

#ifndef	FW_MEM_CONTEXT_H
#define	FW_MEM_CONTEXT_H


//--------------------------------------------------------------------------------------------------
/**
	@class			FwMemContext
	
	@brief			Mechanism to allow grouping of allocations into contexts.

	Even though allocations are named in debug builds, there is often a need to group allocations
	into larger collections. For example, you might want to group all allocations involved in
	collision, or in rendering. This can make it easier to identify subsystems whose memory usage
	is becoming excessive. These collections are referred to as 'contexts', with the default context
	being 'Misc'.

	To place allocations within a context, you simply create a FwMemContext object with the desired
	context name prior to making your allocations. When the object is destroyed, the context is
	returned to the one that was in effect before you changed it. Because of the construct/destruct
	behaviour of the class, you might want to consider placing your objects within their own scope,
	to make it clear that you're relying on object lifetimes during your processing

	@code
		{
			FwMemContext	collisionContext( "Collision" );	// Set context to 'Collision'
			FW_NEW CCollision();								// Allocate object in that context
		}														// Context returned to previous.
	@endcode

	It's advised that projects wrap up all of their markers into either a namespace or class, to 
	ensure that all code utilises the same context names.

	@note			The maximum number of distinct contexts is defined as kMaxContextNames. If we
					run out of contexts, this value will need to be increased. No other changes
					should be required.
**/
//--------------------------------------------------------------------------------------------------

class FwMemContext
{
public:

#ifdef	ATG_MEMORY_DEBUG_ENABLED

	// Construction & destruction
	inline	FwMemContext( const char* pContextName );
	inline	~FwMemContext();

	// Retrieval of context information
	inline	static const char*	GetContextName( s16 contextIndex );
	inline	static s16			GetActiveContextIndex( void );
	inline	static const char*	GetActiveContextName( void );
	inline	static s32			GetNumberOfContexts( void );
	static	s16					FindContextIndex( const char* pContextName );

private:

	s16					m_oldContextIndex;

	static	const s32	kMaxContextNames = 32;
	static	s16			ms_activeContextIndex;
	static	const char*	ms_contextNames[ kMaxContextNames ];
	static	s16			ms_numberOfContexts;

	static	s16			MakeContextIndex( const char* pContextName );

#else

	// Stub functions for when the functionality isn't required
	FwMemContext( const char* ) {}
	~FwMemContext() {}
	inline	static const char*	GetContextName( s16 )			{ return NULL; }
	inline	static s16			GetActiveContextIndex( void )	{ return 0; }
	inline	static const char*	GetActiveContextName( void )	{ return NULL; }
	inline	static s32			GetNumberOfContexts( void )		{ return 0; };
	inline	static s16			FindContextIndex( const char* )	{ return 0; };

#endif // ATG_MEMORY_DEBUG_ENABLED
};

#ifdef	ATG_MEMORY_DEBUG_ENABLED

//--------------------------------------------------------------------------------------------------
/**
	@brief			Construction of an FwMemContext object

	@param			pContextName	Name of the context associated with this object
**/
//--------------------------------------------------------------------------------------------------

inline FwMemContext::FwMemContext( const char* pContextName )
{
	s16	contextIndex		= MakeContextIndex( pContextName );
	m_oldContextIndex		= ms_activeContextIndex;
	ms_activeContextIndex	= contextIndex;
}

//--------------------------------------------------------------------------------------------------
/**
	@brief			Destructor for FwMemContext

	On destruction this object restores the previous context.
**/
//--------------------------------------------------------------------------------------------------

inline FwMemContext::~FwMemContext()
{
	ms_activeContextIndex = m_oldContextIndex;
}

//--------------------------------------------------------------------------------------------------
/**
	@brief			Obtain the name associated with a specific context index

	@param			contextIndex	Index of the context within our table. If '-1' is used, then
									a text string describing the fact that no context was given is
									returned to the user.

	@return			Pointer to a read-only string containing the context name.
**/
//--------------------------------------------------------------------------------------------------

inline const char*		FwMemContext::GetContextName( s16 contextIndex )
{
	FW_ASSERT( contextIndex < ms_numberOfContexts );
	if ( contextIndex >= 0 )
		return ms_contextNames[ contextIndex ];
	else
		return "<ALL>";
}

//--------------------------------------------------------------------------------------------------
/**
	@brief			Obtain the current context index

	@return			A 16-bit integer holding the current context index
**/
//--------------------------------------------------------------------------------------------------

inline s16	FwMemContext::GetActiveContextIndex( void )
{
	return	ms_activeContextIndex;
}

//--------------------------------------------------------------------------------------------------
/**
	@brief			Obtain the name associated with the current context

	@return			Pointer to a string containing the context name
**/
//--------------------------------------------------------------------------------------------------

inline const char*	FwMemContext::GetActiveContextName( void )
{
	return GetContextName( GetActiveContextIndex() );
}

//--------------------------------------------------------------------------------------------------
/**
	@brief			Return the number of defined contexts

	@return			The number of contexts available within our table
**/
//--------------------------------------------------------------------------------------------------

inline s32	FwMemContext::GetNumberOfContexts( void )
{
	return ms_numberOfContexts;
}

#endif	// ATG_MEMORY_DEBUG_ENABLED

#endif	// FW_MEM_DEBUG_H
