#ifndef _ANONYMOUSCOMPONENT_H_
#define _ANONYMOUSCOMPONENT_H_

class CAnonymousEntComponentPointer;
class CAnonymousEntComponentMap;

//--------------------------------------------------
//!
//!	virtual base class that defines interface of anonymous components.
//!
//--------------------------------------------------
class CAnonymousEntComponent
{
friend class CAnonymousEntComponentPointer;
friend class CAnonymousEntComponentMap;
private:
	ntstd::String m_name;
public:

	bool HasName() {return !m_name.empty();};
	ntstd::String& GetName() {return m_name;};
	void SetName(const ntstd::String& name) { m_name = name; } 
	CAnonymousEntComponent();
	CAnonymousEntComponent(const ntstd::String& name);
	virtual ~CAnonymousEntComponent();
	virtual void Update( float fTimeStep ) {UNUSED(fTimeStep);};
};


// < operator
struct ConstCharPointerLower
{
	bool operator()(const char* s1, const char* s2) const
	{
		return strcmp(s1, s2) < 0;
	}
};

//--------------------------------------------------
//!
//!	Anonymous component map, used by CEntity, taking inventory of all the named CAnonymousEntComponent
//!
//--------------------------------------------------
class CAnonymousEntComponentMap
{
private:
	// Warning, the const char* is actually a pointer to an ntstd::string internal c_srt in
	// CAnonymousEntComponent*. Be aware and keep this map private.
	typedef ntstd::Map<const char*,CAnonymousEntComponent*,ConstCharPointerLower> Container;
	Container m_container;
public:
	CAnonymousEntComponentMap();
	~CAnonymousEntComponentMap();
	// return false is already exist, ntAssert if no name
	bool Add(CAnonymousEntComponent*);
	// \return false if component not found
	bool Remove( CAnonymousEntComponent* pobComp );
	bool Remove(const char* pcCompName );
	// return 0 is not found
	CAnonymousEntComponent* Find(const ntstd::String& name) const;
}; // end of class CAnonymousEntComponentMap


#endif // end of _ANONYMOUSCOMPONENT_H_
