/*
 * Copyright (c) 2003-2005 Naughty Dog, Inc.
 * A Wholly Owned Subsidiary of Sony Computer Entertainment, Inc.
 * Use and distribution without consent strictly prohibited
 */

#ifndef ICE_JOINTTRANSFORM_H
#define ICE_JOINTTRANSFORM_H

namespace Ice
{
	namespace Anim
	{
        /*!
         * The JointTransform is used as both input and output from the concatenation
         * (parenting) process. This is the (S, M) matrix the documentation talks about.
		 * It is basically a matrix that is storage transposed with respect to the regular
         * matrices. Instead of storing [0, 0, 0, 1] as the last column, we store the scale.
		 * This the (S,M) data is also 4 quadwords in size.
         */
        class JointTransform
        {
		  protected:
			SMath::Vec4 m_col0;  // M00, M10, M20, Tx  (I.e. column 0 of a regular matrix.)
			SMath::Vec4 m_col1;  // M01, M11, M21, Ty  (I.e. column 1 of a regular matrix.)
			SMath::Vec4 m_col2;  // M20, M21, M22, Tz  (I.e. column 2 of a regular matrix.)
			SMath::Vector m_scale; // Sx, Sy, Sz, -

		  public:
			/*!
			 * Return the transform as a proper 4-by-4 matrix. This will put the transpose
			 * of our columns into the matrix and then scale the rows by the components of 
			 * the scale.
			 */
			SMath::Mat44 GetTransform() const;

			/*!
			 * Set both the scale and the matrix part to a specific value.
			 */
			void Set(SMath::Mat44 const &matrix, SMath::Vector_arg scale);

			/*!
			 * Apply a point to the transform. (Will scale, rotate and translate it.)
			 */			
			SMath::Vec4 ApplyPoint(SMath::Vec4_arg pos) const;

			/*!
			 * Apply a point to the transform. (Will scale and rotate it.)
			 */		   
			SMath::Vec4 ApplyVector(SMath::Vec4_arg pos) const;


			/*!
			 * Set the transform to identity.
			 */
			void Reset();

			/*!
			 * Return the 4-by-4 matrix-part representing the transform.
			 * This will not scale the matrix by the local scale factor (m_scale).
			 */
			SMath::Mat44 GetMatrix() const;

			/*!
			 * Return the scale part of the transform.
			 */
			SMath::Vector GetScale() const;

			/*!
			 * Returns the x-axis, y-axis, or z-axis row of the transform.
			 * Note that this is a slow operation, as this transform is stored by columns.
			 */
			SMath::Vec4 GetXAxis() const;
			SMath::Vec4 GetYAxis() const; 
			SMath::Vec4 GetZAxis() const; 

			/*!
			 * Returns the translation row of the transform.
			 * Note that this is a slow operation, as this transform is stored by columns.
			 */
			SMath::Point GetTranslation() const;

			/*!
			 * Returns the i'th column of this transform (from 0 to 2).
			 */
			SMath::Vec4  GetCol(unsigned iCol) const;
        };

		inline void JointTransform::Reset() {
			m_col0  = SMath::kUnitXAxis;
			m_col1  = SMath::kUnitYAxis;
			m_col2  = SMath::kUnitZAxis;
			m_scale = SMath::Vector(1.0f, 1.0f, 1.0f);
		}
		
		inline SMath::Mat44 JointTransform::GetMatrix() const {
			return SMath::Transpose(SMath::Mat44(m_col0, m_col1, m_col2, SMath::kUnitWAxis));
		}

		inline SMath::Vector JointTransform::GetScale() const {
			return m_scale;
		}
		
		inline SMath::Vec4 JointTransform::GetXAxis() const {
			return SMath::Vec4(m_col0.X(), m_col1.X(), m_col2.X(), 0.0f);
		}
		
		inline SMath::Vec4 JointTransform::GetYAxis() const {
			return SMath::Vec4(m_col0.Y(), m_col1.Y(), m_col2.Y(), 0.0f);
		}
		
		inline SMath::Vec4 JointTransform::GetZAxis() const {
			return SMath::Vec4(m_col0.Z(), m_col1.Z(), m_col2.Z(), 0.0f);
		}
		
		inline SMath::Point JointTransform::GetTranslation() const {
			return SMath::Point(m_col0.W(), m_col1.W(), m_col2.W());
		}

		inline SMath::Vec4  JointTransform::GetCol(unsigned iCol) const {
			return (&m_col0)[iCol];
		}

		// We simply need to transpose our matrix and multiply in the scale.
		// Note that the scale is to be pre-multiplied, so each row is to be
		// scaled.
		inline SMath::Mat44 JointTransform::GetTransform() const 
		{
			SMath::Mat44 tmp(GetMatrix());
			return SMath::Mat44(m_scale.X() * tmp.GetRow(0),
								m_scale.Y() * tmp.GetRow(1),
								m_scale.Z() * tmp.GetRow(2),
								tmp.GetRow(3));
		}
		
		inline void JointTransform::Set(SMath::Mat44 const &matrix, SMath::Vector_arg scale)
		{
			SMath::Mat44 mtx = SMath::Transpose(matrix);
			m_col0 = mtx.GetRow(0);
			m_col1 = mtx.GetRow(1);
			m_col2 = mtx.GetRow(2);
			m_scale = scale;
		}
		
		inline SMath::Vec4 JointTransform::ApplyPoint(SMath::Vec4_arg pos) const
		{
			SMath::Vec4 scale = SMath::Vec4(m_scale.X(), m_scale.Y(), m_scale.Z(), 1.0f);
			SMath::Vec4 p = scale * pos;
			return SMath::MulPointMatrix(p, GetMatrix());
		}
		
		inline SMath::Vec4 JointTransform::ApplyVector(SMath::Vec4_arg vec) const
		{
			SMath::Vec4 scale = SMath::Vec4(m_scale.X(), m_scale.Y(), m_scale.Z(), 1.0f);
			SMath::Vec4 v = scale * vec;
			return SMath::MulVectorMatrix(v, GetMatrix());
		}
	}
}

#endif // ICE_ICEJOINTANIM
