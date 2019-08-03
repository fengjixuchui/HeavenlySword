/* 
 * 
 * Confidential Information of Telekinesys Research Limited (t/a Havok).  Not for disclosure or distribution without Havok's
 * prior written consent.This software contains code, techniques and know-how which is confidential and proprietary to Havok.
 * Level 2 and Level 3 source code contains trade secrets of Havok. Havok Software (C) Copyright 1999-2006 Telekinesys Research Limited t/a Havok. All Rights Reserved. Use of this software is subject to the terms of an end user license agreement.
 * 
 */

#ifndef HKBASE_SMALL_HKARRAY_H
#define HKBASE_SMALL_HKARRAY_H

	/// Common functionality for all hkSmallArray types.
	/// These are out of line functions to avoid code bloat.
struct hkSmallArrayUtil
{
	static void HK_CALL _reserve(void*, int numElem, int sizeElem);
	static void HK_CALL _reserveMore(void* array, int sizeElem);
	static void HK_CALL _reduce(     void* array, int sizeElem);
};


	/// Space optimised array for arrays likely to be empty.
	///    - The number of objects is limited to 65k
	///    - If this array is empty, it uses only sizeof(void*) instead of sizeof(hkArray) == 12
template <typename T>
class hkSmallArray
{
		friend struct hkSmallArrayUtil;
	public:

		HK_DECLARE_NONVIRTUAL_CLASS_ALLOCATOR(HK_MEMORY_CLASS_ARRAY, hkSmallArray<T>);

			/// Creates a zero length array.
		HK_FORCE_INLINE hkSmallArray();

	public:

			/// Deallocates array memory.
		HK_FORCE_INLINE ~hkSmallArray();

			/// Read/write access to the i'th element.
		HK_FORCE_INLINE T& operator[] (int i);

			/// Read only access to the i'th element.
		HK_FORCE_INLINE const T& operator[] (int i) const;

			/// Returns the size.
		HK_FORCE_INLINE int getSize() const;

			/// Returns the capacity.
		HK_FORCE_INLINE int getCapacity() const;

			/// Checks if the size is zero.
		HK_FORCE_INLINE hkBool isEmpty() const;
		
			/// Sets the size to zero. Only works if the array was not empty before
		HK_FORCE_INLINE void clear();

			/// Sets the size to zero and deallocates storage.
		HK_FORCE_INLINE void clearAndDeallocate();

			/// Tries to reduce the capacity to avoid wasting storage
		HK_FORCE_INLINE void optimizeCapacity( int numFreeElemsLeft );

			/// Removes the element at the specified index.
			/// This is done by moving the last element into index i
			/// and resizing by -1. This is very fast, but note that
			/// the order of elements is changed.
		void removeAt(int index);

			/// Removes the element at the specified index, copying elements down one slot as in the STL array.
			/// Slower than above, but the order is unchanged.
		void removeAtAndCopy(int index);

			/// Returns the index of the first occurrence of t, or -1 if not found.
		int indexOf(const T& t) const;

			/// Removes the last element.
		HK_FORCE_INLINE void popBack();

			/// Adds an element to the end.
		HK_FORCE_INLINE void pushBack(const T& e);

			/// Adds an element to the end. No check for resize.
		HK_FORCE_INLINE void pushBackUnchecked(const T& e);

			/// Ensures no reallocation occurs until at least size n.
		HK_FORCE_INLINE void reserve(int n);

			/// Ensures no reallocation occurs until size n.
		HK_FORCE_INLINE void reserveExactly(int n);

			/// Sets the size to n.
			/// If the array is expanded, new elements are uninitialized.
		HK_FORCE_INLINE void setSize(int n);

			/// Sets the size to n.
			/// If the array is expanded, new elements are uninitialized.
		HK_FORCE_INLINE void setSizeUnchecked(int n);

			/// Increments the size by 1 and returns a reference to the first element created.
		HK_FORCE_INLINE T& expandOne( );

			/// Increments the size by n and returns a pointer to the first element created.
		HK_FORCE_INLINE T* expandBy( int n );

			/// Expands the array by numToInsert at the specified index.
			/// See also getSubarray() and the constructor, which uses an existing
			/// C style array in place.
		HK_FORCE_INLINE T* expandAt( int index, int numToInsert );


			/// 
		typedef T* iterator;
			/// 
		typedef const T* const_iterator;

			/// Returns an STL-like iterator to the first element.
		HK_FORCE_INLINE iterator begin();
			/// Returns an STL-like iterator to the 'one past the last' element.
		HK_FORCE_INLINE iterator end();
			/// Returns an STL-like const iterator to the first element.
		HK_FORCE_INLINE const_iterator begin() const;
			/// Returns an STL-like const iterator to the 'one past the last' element.
		HK_FORCE_INLINE const_iterator end() const;

	public:
		HK_FORCE_INLINE void releaseMemory();

		T*       getData();
		const T* getData() const;

		struct Info
		{
			hkUint16 m_size;
			hkUint16 m_capacity;
			T* getData(){		return reinterpret_cast<T*>(this+1);	}
		};
		Info* m_info;

};
#include <hkbase/htl/hkSmallArray.inl>

#endif // HKBASE_SMALL_HKARRAY_H


/*
* Havok SDK - PUBLIC RELEASE, BUILD(#20060902)
*
* Confidential Information of Havok.  (C) Copyright 1999-2006 
* Telekinesys Research Limited t/a Havok. All Rights Reserved. The Havok
* Logo, and the Havok buzzsaw logo are trademarks of Havok.  Title, ownership
* rights, and intellectual property rights in the Havok software remain in
* Havok and/or its suppliers.
*
* Use of this software for evaluation purposes is subject to and indicates 
* acceptance of the End User licence Agreement for this product. A copy of 
* the license is included with this software and is also available from salesteam@havok.com.
*
*/
