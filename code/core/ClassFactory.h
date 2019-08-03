//--------------------------------------------------
//!
//!	\file ClassFactory.h
//!	Class Factory Base Class
//! John Lusty - 2004
//!
//--------------------------------------------------

#ifndef _CLASS_FACTORY_H
#define _CLASS_FACTORY_H

//--------------------------------------------------
//!
//!	CInstancerBase
//!	Abstract Base Class for Factory Instancers
//!
//--------------------------------------------------
template<class TBASE>
class CInstancerBase
{
public:
	CInstancerBase() {;}
	virtual ~CInstancerBase(){};
	virtual TBASE* Create() const = 0;
};

//--------------------------------------------------
//!
//!	CFactoryStorageArray
//!	Array Storage for Instancers
//!	An example storage class base on a char indexed
//! array.
//!
//--------------------------------------------------
template<class TBASEINSTANCER, typename TARG>
class CFactoryStorageArray
{
public:
	CFactoryStorageArray()
	{
		for(int i = 0; i < 256; i++) m_pData[i] = 0;
	}

	void Store(TARG arg, TBASEINSTANCER* pobj)
	{
#ifdef _DEBUG
		int iTest = (int)arg;
		ntAssert(iTest >= 0 && iTest < 256);
#endif
		ntAssert(!m_pData[arg]);
		m_pData[arg] = pobj;
	}

	TBASEINSTANCER* Lookup(TARG arg) const
	{
#ifdef _DEBUG
		int iTest = (int)arg;
		ntAssert(iTest >= 0 && iTest < 256);
#endif
		ntAssert(m_pData[arg]);
		return m_pData[arg];
	}

private:
	TBASEINSTANCER*	m_pData[256];
};

//--------------------------------------------------
//!
//!	CFactoryStorageMap
//!	Map Storage for instancers.
//!	Example storage based around the STL map class
//!
//--------------------------------------------------
template<class TBASEINSTANCER, typename TARG>
class CFactoryStorageMap
{
public:

	void Store(TARG arg, TBASEINSTANCER* pobj)
	{
		typename ntstd::Map<TARG, TBASEINSTANCER*>::const_iterator it;
		it = m_data.find(arg);
		ntAssert(it == m_data.end());

		m_data.insert(ntstd::pair<TARG,TBASEINSTANCER*>(arg,pobj));
	}

	TBASEINSTANCER* Lookup(TARG arg) const
	{
		typename ntstd::Map<TARG,TBASEINSTANCER*>::const_iterator it;
		it = m_data.find(arg);
		if(it == m_data.end())
			return 0;
		return it->second;
	}

private:
	ntstd::Map<TARG, TBASEINSTANCER*> m_data;
};


//--------------------------------------------------
//!
//!	The Class Factory Implementation
//!	Provides a mechanism for instancing a class
//!	dependant on a look up value.
//!
//! TSTORAGE - A class providing storage for the factories instancers
//! TBASE	- The base type of classes made by this factory
//! TARG		- The lookup argument type
//!
//--------------------------------------------------
template<class TSTORAGE, typename TBASE, typename TARG>
class CFactory : public TSTORAGE /*: public CFactoryBase<TBASE, TARG>,*/
{
public:
	void AddInstancer(TARG arg, CInstancerBase<TBASE>* pobInstancer)
	{
		ntAssert(this); 
		Store(arg, pobInstancer);
	}

	TBASE* GetInstance(TARG arg) const 
	{
		if(!this)
			return 0;

		CInstancerBase<TBASE>* pobInstancer = Lookup(arg);
		if(pobInstancer)
			return pobInstancer->Create();
		return 0;
	}
};


//--------------------------------------------------
//!
//!	Instancer for a particular class
//!	Registers itself with the factory and
//!	provides a mechanism to instantiate the specific
//! class.
//!
//! TBASE - base class
//!	T     - actual type to be instanced
//!	TARG  - referencing argument type
//!
//--------------------------------------------------
template<class TBASE, typename T, typename TARG, class TSTORAGE>
class CInstancer : public CInstancerBase<TBASE>
{
public:
	CInstancer(CFactory<TSTORAGE,TBASE,TARG>*& pFact, TARG arg)
	{
		static CFactory<TSTORAGE, TBASE, TARG> sFact;

		if(!pFact)
			pFact = &sFact;

		pFact->AddInstancer(arg, (CInstancerBase<TBASE>*)this);
	}
	virtual TBASE* Create() const {return NT_NEW T;}
};

//////////////////////////////////////////////////////////////////////////////////////////////////////////
// Macros make life easier
//////////////////////////////////////////////////////////////////////////////////////////////////////////

// Create a class factory:
#define DECLARE_CLASS_FACTORY(storagetype, basetype, argtype, name) \
			CFactory<storagetype<CInstancerBase<basetype>,argtype>, basetype, argtype>* name = 0

// Publish a class factory:
#define PUBLISH_CLASS_FACTORY(storagetype, basetype, argtype, name) \
			extern CFactory<storagetype<CInstancerBase<basetype>,argtype>, basetype, argtype>* name

// Register a class with a factory:
#define REGISTER_CLASS(basetype, argtype, storagetype, factory, type, key) \
			CInstancer<basetype, type, argtype, storagetype<CInstancerBase<basetype>, argtype> > factory##type ##__LINE__(factory, key)

#endif // _CLASS_FACTORY_H
