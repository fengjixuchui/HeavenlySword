/* 
 * 
 * Confidential Information of Telekinesys Research Limited (t/a Havok).  Not for disclosure or distribution without Havok's
 * prior written consent.This software contains code, techniques and know-how which is confidential and proprietary to Havok.
 * Level 2 and Level 3 source code contains trade secrets of Havok. Havok Software (C) Copyright 1999-2006 Telekinesys Research Limited t/a Havok. All Rights Reserved. Use of this software is subject to the terms of an end user license agreement.
 * 
 */
#ifndef HK_MATH_MATRIX4_H
#define HK_MATH_MATRIX4_H

#ifndef HK_MATH_MATH_H
#	error Please include hkmath/hkMath.h instead of this file.
#endif

/// A 4x4 matrix of hkReals.
/// This class is only used in our tool chain.
/// It is not optimized for runtime use. Use hkTransform instead.
/// Internal storage is 16 hkReals in a 4x4 matrix.
/// Elements are stored in column major format.<br>
/// i.e. contiguous memory locations are (x00, x10, x20, x30), (x01, x11,...) 
/// where x10 means row 1, column 0 for example.
class hkMatrix4
{
	public:

		HK_DECLARE_NONVIRTUAL_CLASS_ALLOCATOR(HK_MEMORY_CLASS_MATH, hkMatrix4);

			/// Empty constructor.  The elements of the matrix are not initialized.
		HK_FORCE_INLINE hkMatrix4() { }

			/// Gets a read-write reference to the i'th column.
		HK_FORCE_INLINE hkVector4& getColumn(int i);

			/// Gets a read-only reference to the i'th column.
		HK_FORCE_INLINE const hkVector4& getColumn(int i) const;

			/// Returns a copy of the i'th row.
		HK_FORCE_INLINE void getRow( int row, hkVector4& r) const;

			/// Gets read-write access to the specified element.
		HK_FORCE_INLINE hkReal& operator() (int row, int col);

			/// Gets read-only access to the specified elements.
		HK_FORCE_INLINE const hkReal& operator() (int row, int col) const;

			/// Sets all rows at once.
		HK_FORCE_INLINE void setRows( const hkVector4& r0, const hkVector4& r1, const hkVector4& r2, const hkVector4& r3);

			/// Writes the rows 0 to 3 in to the parameters r0, r1, r2, r3 respectively.
		HK_FORCE_INLINE void getRows( hkVector4& r0, hkVector4& r1, hkVector4& r2, hkVector4& r3) const;

			/// Sets all columns of the current matrix.  Where column is set to r0 and so on.
		HK_FORCE_INLINE void setCols( const hkVector4& c0, const hkVector4& c1, const hkVector4& c2, const hkVector4& c3);

			/// Writes the columns 0 to 3 into the parameters c0, c1, c2 and c3 respectively.
		HK_FORCE_INLINE void getCols( hkVector4& c0, hkVector4& c1, hkVector4& c2, hkVector4& c3) const;

			/// Zeroes all values in this matrix.
		HK_FORCE_INLINE void setZero();

			/// Sets the specified diagonal values, zeroes the non-diagonal values.
		HK_FORCE_INLINE void setDiagonal( hkReal m00, hkReal m11, hkReal m22, hkReal m33 = 1.0f );

			/// Sets the diagonal values to 1, zeroes the non-diagonal values.
		HK_FORCE_INLINE void setIdentity();

			/// Returns a global identity transform.
		HK_FORCE_INLINE static const hkMatrix4& HK_CALL getIdentity();

			/// Set the contents based on the given hkTransform. Will set the bottom row to (0,0,0,1) in this hkMatrix4 as 
			/// it is undefined in a hkTransform (not used)
		void set(const hkTransform& t);

			/// Writes a 4x4 matrix suitable for rendering into p.
		void get4x4ColumnMajor(hkReal* p) const;

			/// Reads a 4x4 matrix from p. 
		void set4x4ColumnMajor(const hkReal* p);

			/// Writes a 4x4 matrix suitable for rendering into p.
		void get4x4RowMajor(hkReal* p) const;

			/// Reads a 4x4 matrix from p. 
		void set4x4RowMajor(const hkReal* p);

			/// Checks if this matrix is equal to m within an optional epsilon.
		hkBool isApproximatelyEqual( const hkMatrix4& m, hkReal epsilon=1e-3f ) const;

			/// Inverts the matrix. This function returns HK_SUCCESS if the determinant is greater than epsilon. Otherwise it returns HK_FAILURE and the matrix values are undefined.
		hkResult invert(hkReal epsilon);

			/// Transposes this matrix in place.
		void transpose();

			/// set to the transpose of another matrix
		void setTranspose( const hkMatrix4& s );
		
			/// Set this matrix to be the product of a and b.  (this = a * b)
		void setMul( const hkMatrix4& a, const hkMatrix4& b );

			/// Sets this matrix to be the product of a and the inverse of b.  (this = a * b^-1)
		void setMulInverse(  const hkMatrix4& a, const hkMatrix4& b );

			/// Sets this matrix to be the product of a and scale (this = a * scale)
		void setMul( hkSimdRealParameter scale, const hkMatrix4& a );

			/// Modifies this matrix by adding the matrix a to it.  (this += a)
		void add( const hkMatrix4& a );

			/// Modifies this matrix by subtracting the matrix a from it.  (this += a)
		void sub( const hkMatrix4& a );

			/// Modifies this matrix by post multiplying it by the matrix a. (this = this*a)
		void mul( const hkMatrix4& a);

			/// Modifies this matrix by multiplying by scale (this *= scale)
		void mul( hkSimdRealParameter scale );

			/// Copies all elements from a into this matrix.
		inline void operator= ( const hkMatrix4& a );

			/// Checks for bad values (denormals or infinities)
		hkBool isOk() const;

	protected:

		hkVector4 m_col0;
		hkVector4 m_col1;
		hkVector4 m_col2;
		hkVector4 m_col3;
};

#endif // HK_MATH_MATRIX4_H

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
