//---------------------------------------------------------------------------------------------------------
//!
//!	\file physics/maths_tools.h
//!	
//!	DYNAMICS COMPONENT:
//!		Tools to extract shapes from clumps
//!
//!	Author: Mustapha Bismi (mustapha.bismi@ninjatheory.com)
//! Created: 2005.08.08
//!
//---------------------------------------------------------------------------------------------------------

#ifndef _DYNAMICS_MATHS_TOOLS_INC
#define _DYNAMICS_MATHS_TOOLS_INC

#include "config.h"
#ifndef  _PS3_RUN_WITHOUT_HAVOK_BUILD

#include "physics/havokincludes.h"

#include <hkmath/hkmath.h>

namespace Physics
{
	// ---------------------------------------------------------------
	//	Tools class. Do not try to instanciate.
	// ---------------------------------------------------------------
	class MathsTools
	{
	public:

		// hkVector <--> CPoint
		static void			CPointTohkVector(const CPoint& obFrom, hkVector4& obTo);
		static void			hkVectorToCPoint(const hkVector4& obFrom, CPoint& obTo);
		static hkVector4	CPointTohkVector(const CPoint& obFrom);
		static CPoint		hkVectorToCPoint(const hkVector4& obFrom);

		// hkVector <--> CVector
		// Nota: Havok do a terrible job with the W components. Conversion from hkVector force the w component to 1.f
		static void			CVectorTohkVector(const CVector& obFrom, hkVector4& obTo);
		static void			hkVectorToCVector(const hkVector4& obFrom, CVector& obTo);
		static hkVector4	CVectorTohkVector(const CVector& obFrom);
		static CVector		hkVectorToCVector(const hkVector4& obFrom);

		// hkVector <--> CDirection
		// Nota: Havok do a terrible job with the W components. Conversion from hkVector force the w component to 1.f
		static void			CDirectionTohkVector(const CDirection& obFrom, hkVector4& obTo);
		static void			hkVectorToCDirection(const hkVector4& obFrom, CDirection& obTo);
		static hkVector4	CDirectionTohkVector(const CDirection& obFrom);
		static CDirection	hkVectorToCDirection(const hkVector4& obFrom);


		// hkQuaternion <--> CQuat 
		static void			CQuatTohkQuaternion(const CQuat& obFrom, hkQuaternion& obTo);
		static void			hkQuaternionToCQuat(const hkQuaternion& obFrom, CQuat& obTo);
		static hkQuaternion	CQuatTohkQuaternion(const CQuat& obFrom);
		static CQuat		hkQuaternionToCQuat(const hkQuaternion& obFrom);

		// hkQuaternion <--> CMatrix (
		// Nota: Translation component is ignored.
		static void			CMatrixTohkQuaternion(const CMatrix& obFrom, hkQuaternion& obTo);
		static void			hkQuaternionToCMatrix(const hkQuaternion& obFrom, CMatrix& obTo);
		static hkQuaternion	CMatrixTohkQuaternion(const CMatrix& obFrom);
		static CMatrix		hkQuaternionToCMatrix(const hkQuaternion& obFrom);

		// hkQsTransform <--> CMatrix 	
		// Nota: Beware scale is ignored!!! both transform are considered to be orthonormal (rotations). 
		static void			CMatrixTohkQsTransform(const CMatrix& obFrom, hkQsTransform& obTo);
		static void			hkQsTransformToCMatrix(const hkQsTransform& obFrom, CMatrix& obTo);
		static hkQsTransform	CMatrixTohkQsTransform(const CMatrix& obFrom);
		static CMatrix		hkQsTransformToCMatrix(const hkQsTransform& obFrom);

		// hkTransform <--> CMatrix 			
		static void			CMatrixTohkTransform(const CMatrix& obFrom, hkTransform& obTo);
		static void			hkTransformToCMatrix(const hkTransform& obFrom, CMatrix& obTo);
		static hkTransform	CMatrixTohkTransform(const CMatrix& obFrom);
		static CMatrix		hkTransformToCMatrix(const hkTransform& obFrom);


	private:

		MathsTools();
		~MathsTools();
	};

}

#endif
#endif // _DYNAMICS_MATHS_TOOLS_INC
