#ifndef _DEEPLIST_H_
#define _DEEPLIST_H_

class DeepList; // forward decl.

class DeepListIterator
{
public:
	DeepListIterator(const DeepList* pDeepList);

	~DeepListIterator();

	// Has the iterator finished?
	operator bool() const;

	// Move on to the next element in the array
	bool operator++();

	void Reset(const DeepList* pDeepList);

	const void* GetValue();
private:
	void TunnelDownContainers();

	typedef ntstd::List< DataObject*, Mem::MC_ODB >::const_iterator InternalIterator;
	// The following two represent the "stack" of iterators, where the topmost (== first)
	// iterator is the container that is currently being iterated. The "end iterators" stack
	// mirrors the "actual iterators" stack, so we can tell if the current iterator has finished.
	ntstd::List< InternalIterator > m_obActualIterators;
	ntstd::List< InternalIterator > m_obEndIterators;
};

class DeepList
{
friend class DeepListIterator;
friend class DeepListDIF;

public:
	DeepList();
	virtual ~DeepList();

	void LinkObject(void* pObject);

	void UnlinkObject(void* pObject);

	DeepListIterator DeepIterator() const;

private:
	void ValidateFixup() const;

	void LinkDataObject(DataObject *pObject);

	void UnlinkDataObject(DataObject *pObject);

	typedef ntstd::List< DataObject*, Mem::MC_ODB > ObjectList;
	
	typedef ntstd::List< void* > FixupList;

	// mutable because the Validate Fixup may modify these
	mutable ObjectList m_obObjects;
	mutable FixupList m_obFixupList;
};

#endif // _DEEPLIST_H_
