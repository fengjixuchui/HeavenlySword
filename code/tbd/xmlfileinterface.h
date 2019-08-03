#ifndef _XMLFILEINTERFACE_H_
#define _XMLFILEINTERFACE_H_

class FileDate;
class ObjectContainer;
class DataObject;





class KeepMeContainer
{
public:
	class KeepMe
	{
	public:
		KeepMe();
		virtual ~KeepMe();
	}; // end of class KeepMe
private:
	// pointer to data which must be preserved while reloading
	mutable KeepMe* m_pKeepMe;

public:

	// keep me stuff
	inline bool HasKeepMe() const {return m_pKeepMe!=0;}
	inline KeepMe* GetKeepMe() const {ntAssert(HasKeepMe());return m_pKeepMe;}
	inline void SetKeepMe(KeepMe* pKeepMe) const { m_pKeepMe=pKeepMe;}
	inline void SetKeepMeToZero() const { m_pKeepMe=0;}
	void ReleaseKeepMe();

	//! constructor
	KeepMeContainer();
	virtual ~KeepMeContainer();
}; // end of class KeepMeContainer


//--------------------------------------------------
//!
//!	Xml file interface
//!	One Xml file <-> one C++ class, reload it when you want.
//!	Exciting class with lots of stuff to describe
//!
//--------------------------------------------------


class XmlFileInterface: public CNonCopyable, public KeepMeContainer
{
public:
	

	
	// file path
	ntstd::String m_filePath;
	
	// keep track of file date for update
	FileDate* m_pFile;
	
	// main object determined by file name
	DataObject* m_pMainObject;

	
public:

	// constructor
	XmlFileInterface();
	
	// destructor
	virtual ~XmlFileInterface();
	
	// reload 
	static bool Reload(XmlFileInterface*& pObject);
	
	// create new one
	static XmlFileInterface* Create(const char* objectName, const char* pathName);
	
	// create new one
	static XmlFileInterface* Create(const char* pathName);
	
	// get main opbject
	DataObject* GetMainObject() {ntAssert(m_pMainObject);return m_pMainObject;}
protected:	

	// reload pointer, rewrite pointer
	virtual void PropagateChange() {};
	
	// load object
	static XmlFileInterface* Load(const char* objectName, const char* pathName);

	// free object
	static bool Delete(XmlFileInterface* pObject);

	// get container from object
	static ntstd::String GetContainerNameFromObjectName(const ntstd::String& objectName)
	{
		return ntstd::String(objectName) + ntstd::String("Cont");
	}
	
	// get container from object
	static ntstd::String GetFileNameFromObjectName(const char* objectName, const char* pathName)
	{
		return ntstd::String(pathName) + ntstd::String(objectName) + ntstd::String(".xml");
	}
	
	// finalise (post construct), called after new or load
	virtual void Finalise() {}
		
	// return true if a file is associated to this object
	bool HasFile();

/*	// save current object
	// NOT FULLY TESTED
	bool Save(const char* filePath = 0);
	
	// new object created in the game aide
	// NOT FULLY TESTED
	XmlFileInterface* New(const char* objectName, const char* objectType);
*/
};

#endif // end of _XMLFILEINTERFACE_H_
