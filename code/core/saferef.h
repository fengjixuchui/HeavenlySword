/***************************************************************************************************
*
*	DESCRIPTION		-	SafeRef.h
*
*	NOTES				Implementation of the CSafeRef class
*
***************************************************************************************************/

#ifndef _SAFE_REF_H
#define _SAFE_REF_H

///////////////////////////////////////
// Forward decls
template<class T> class CSafeRefTarget;

enum NULL_REF {NULL_HANDLE};

/***************************************************************************************************
*
*	CLASS			CSafeRef<T>
*
*	DESCRIPTION		A safe reference can be used to keep track of objects you wish to pass around
*					which might become invalidated.  NetClients use SafeRef's so that other systems
*					such as the player manager can have a reference to a NetClient but know when
*					that NetClient is no longer valid.
*
*					Safe References require a CSafeRefServer as a member of the referenced class.  
*					This server can also be set up in garbage collect mode which automatically
*					destroys the referenced object.
*
***************************************************************************************************/
template<class T>
class CSafeRef
{
public:
	CSafeRef(NULL_REF)							{m_pServ = 0;}
	explicit CSafeRef(CSafeRefTarget<T>* pServ) {m_pServ = pServ; if(m_pServ) m_pServ->AddRef();}
	~CSafeRef()									{if(m_pServ) m_pServ->Release();}

	// Copy construction
	CSafeRef(const CSafeRef<T>& obRHS)			{m_pServ = obRHS.m_pServ; if(m_pServ) m_pServ->AddRef();}

	// Assignment
	CSafeRef<T>& operator=(const CSafeRef<T>& obRHS) 
	{
		if(m_pServ)
			m_pServ->Release(); 

		m_pServ = obRHS.m_pServ; 

		if(m_pServ)
			m_pServ->AddRef(); 

		return *this;
	}

	// Comparisons
	bool operator==(const CSafeRef<T>& obRHS) const	{return m_pServ == obRHS.m_pServ;}
	bool operator!=(const CSafeRef<T>& obRHS) const	{return m_pServ != obRHS.m_pServ;}
	bool operator==(const CSafeRefTarget<T>* pRHS) const {return m_pServ == pRHS;}
	bool operator!=(const CSafeRefTarget<T>* pRHS) const {return m_pServ != pRHS;}

	bool Valid() const		{return m_pServ && m_pServ->Get();}
	bool IsNull() const		{return m_pServ==0 || !m_pServ->Get();}
	operator bool() const	{return m_pServ && m_pServ->Get();}

	// Function calling
	T* operator->()				{ntAssert(Valid()); return m_pServ->Get();}
	const T* operator->() const {ntAssert(Valid()); return m_pServ->Get();}
	const T& Get() const		{ntAssert(Valid()); return *m_pServ->Get();}
	const T& operator*() const 	{return Get();}

private:
	CSafeRefTarget<T>* m_pServ;
};


/***************************************************************************************************
*
*	CLASS			CSafeRefTarget<RefType>
*
*	DESCRIPTION		Server for safe references to classes of type RefType
*
***************************************************************************************************/
template<class RefType>
class CSafeRefTarget
{
public:
	explicit CSafeRefTarget(RefType* pTarg, bool bGarbageCollect=false) 
	{
		m_pTarg = pTarg; 
		m_uiCount = 0; 
		m_bGarbageCollect = bGarbageCollect;
	}
	void Invalidate() 
	{
		m_pTarg = 0; 
		if(!m_uiCount) 
		{
			NT_DELETE( this );
		}
	}

	// Ref Counting
	void AddRef() {m_uiCount++;;}
	void Release() 
	{
		if(!--m_uiCount && (m_bGarbageCollect || !m_pTarg)) 
			NT_DELETE( this );
	}

	//
	RefType*			Get()			{return m_pTarg;}
	CSafeRef<RefType>	MakeHandle()	{return CSafeRef<RefType>(this);}

private:
	~CSafeRefTarget()
	{
		if(m_pTarg)
		{
			NT_DELETE( m_pTarg );
		}
	}

private:

	unsigned int m_uiCount;
	RefType*	 m_pTarg;
	bool		 m_bGarbageCollect;
};


//////////////////////
// Server
//////////////////////
template<class T>
class CSafeRefServer
{
public:
	CSafeRefServer()			{m_pTarget = NT_NEW CSafeRefTarget<T>((T*)this); m_pTarget->AddRef();}
	~CSafeRefServer()			{m_pTarget->Invalidate(); m_pTarget->Release();}

	CSafeRef<T> MakeHandle()	{return m_pTarget ? m_pTarget->MakeHandle() : CSafeRef<T>(NULL_HANDLE);}

private:
	CSafeRefTarget<T>* m_pTarget;
};

/////////////////////////////////
// Garbage Collecting Server
/////////////////////////////////
template<class T>
class CGarbageCollectedSafeRefServer
{
public:
	CGarbageCollectedSafeRefServer()	{m_pTarget = NT_NEW CSafeRefTarget<T>((T*)this, true); m_pTarget->AddRef();}
	~CGarbageCollectedSafeRefServer()	{m_pTarget->Release();}

	CSafeRef<T> MakeHandle()			{return m_pTarget ? m_pTarget->MakeHandle() : CSafeRef<T>(NULL_HANDLE);}

private:
	CSafeRefTarget<T>* m_pTarget;
};


//
// This #define is to make the construction of saferefs nice...
// Or maybe it's nasty...  I'm in two minds...
//
#define NULL_REF NULL_HANDLE

#endif // _SAFE_REF_H
