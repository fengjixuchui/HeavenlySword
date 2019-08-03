//---------------------------------------------------------------------------------------------------------
//!
//!	\file physics/maths_tools.cpp
//!	
//!	DYNAMICS COMPONENT:	
//!
//!	Author: Mustapha Bismi (mustapha.bismi@ninjatheory.com)
//! Created: 2005.08.06
//!
//---------------------------------------------------------------------------------------------------------

#include "config.h"
#include "maths_tools.h"

namespace Physics {
#ifndef _PS3_RUN_WITHOUT_HAVOK_BUILD
	void MathsTools::CPointTohkVector(const CPoint& obFrom, hkVector4& obTo)
	{
		obTo.set( obFrom.X(), obFrom.Y(), obFrom.Z());
		return;
	}

	void MathsTools::hkVectorToCPoint(const hkVector4& obFrom, CPoint& obTo)
	{
		obTo.X() = obFrom(0);
		obTo.Y() = obFrom(1);
		obTo.Z() = obFrom(2);
		obTo.W() = 0.f;
		return;
	}

	hkVector4 MathsTools::CPointTohkVector(const CPoint& obFrom)
	{
		hkVector4 obTo;
		obTo.set( obFrom.X(), obFrom.Y(), obFrom.Z());
		return obTo;
	}

	CPoint MathsTools::hkVectorToCPoint(const hkVector4& obFrom)
	{
		return CPoint(obFrom(0),obFrom(1),obFrom(2));
	}

	void MathsTools::CVectorTohkVector(const CVector& obFrom, hkVector4& obTo)
	{
		obTo.set( obFrom.X(), obFrom.Y(), obFrom.Z());
		return;
	}

	void MathsTools::hkVectorToCVector(const hkVector4& obFrom, CVector& obTo)
	{
		obTo.X() = obFrom(0);
		obTo.Y() = obFrom(1);
		obTo.Z() = obFrom(2);
		obTo.W() = 1.f;
		return;
	}

	hkVector4 MathsTools::CVectorTohkVector(const CVector& obFrom)
	{
		hkVector4 obTo;
		obTo.set( obFrom.X(), obFrom.Y(), obFrom.Z());
		return obTo;
	}

	CVector MathsTools::hkVectorToCVector(const hkVector4& obFrom)
	{
		return CVector(obFrom(0),obFrom(1),obFrom(2),1.f);
	}

	void MathsTools::CDirectionTohkVector(const CDirection& obFrom, hkVector4& obTo)
	{
		obTo.set( obFrom.X(), obFrom.Y(), obFrom.Z());
		return;
	}

	void MathsTools::hkVectorToCDirection(const hkVector4& obFrom, CDirection& obTo)
	{
		obTo.X() = obFrom(0);
		obTo.Y() = obFrom(1);
		obTo.Z() = obFrom(2);
		return;
	}

	hkVector4 MathsTools::CDirectionTohkVector(const CDirection& obFrom)
	{
		hkVector4 obTo;
		obTo.set( obFrom.X(), obFrom.Y(), obFrom.Z());
		return obTo;
	}

	CDirection MathsTools::hkVectorToCDirection(const hkVector4& obFrom)
	{
		return CDirection( obFrom(0),obFrom(1),obFrom(2) );
	}


	void MathsTools::CQuatTohkQuaternion(const CQuat& obFrom, hkQuaternion& obTo)
	{
		obTo.setImag(hkVector4(obFrom.X(), obFrom.Y(), obFrom.Z())); 
		obTo.setReal(obFrom.W()); 
		obTo.normalize();
		return;
	}

	void MathsTools::hkQuaternionToCQuat(const hkQuaternion& obFrom, CQuat& obTo)
	{
		obTo.X() = obFrom.getImag()(0);
		obTo.Y() = obFrom.getImag()(1);
		obTo.Z() = obFrom.getImag()(2);
		obTo.W() = obFrom.getReal();
		return;
	}

	hkQuaternion MathsTools::CQuatTohkQuaternion(const CQuat& obFrom)
	{
		hkQuaternion obTo;
		obTo.setImag(hkVector4(obFrom.X(), obFrom.Y(), obFrom.Z())); 
		obTo.setReal(obFrom.W()); 
		obTo.normalize();
		return obTo;
	}

	CQuat MathsTools::hkQuaternionToCQuat(const hkQuaternion& obFrom)
	{
		return CQuat(obFrom.getImag()(0),obFrom.getImag()(1),obFrom.getImag()(2),obFrom.getReal());
	}

	void MathsTools::CMatrixTohkQuaternion(const CMatrix& obFrom, hkQuaternion& obTo)
	{
		CQuat obQuaternion(obFrom);
		CQuatTohkQuaternion(obQuaternion,obTo);
		return;
	}

	void MathsTools::hkQuaternionToCMatrix(const hkQuaternion& obFrom, CMatrix& obTo)
	{
		obTo.SetFromQuat(hkQuaternionToCQuat(obFrom));
		return;
	}

	hkQuaternion MathsTools::CMatrixTohkQuaternion(const CMatrix& obFrom)
	{
		return CQuatTohkQuaternion(CQuat(obFrom));
	}

	CMatrix MathsTools::hkQuaternionToCMatrix(const hkQuaternion& obFrom)
	{
		return CMatrix(hkQuaternionToCQuat(obFrom));
	}

	// hkQsTransform <--> CMatrix 		
	void MathsTools::CMatrixTohkQsTransform(const CMatrix& obFrom, hkQsTransform& obTo)
	{
		obTo = CMatrixTohkQsTransform(obFrom);
	}

	void MathsTools::hkQsTransformToCMatrix(const hkQsTransform& obFrom, CMatrix& obTo)
	{
		obTo = hkQsTransformToCMatrix(obFrom);
	}

	hkQsTransform MathsTools::CMatrixTohkQsTransform(const CMatrix& obFrom)
	{		
		return hkQsTransform(CPointTohkVector(obFrom.GetTranslation()), CMatrixTohkQuaternion(obFrom));
	}

	CMatrix	MathsTools::hkQsTransformToCMatrix(const hkQsTransform& obFrom)
	{
		return CMatrix(hkQuaternionToCQuat(obFrom.getRotation()), hkVectorToCPoint(obFrom.getTranslation()));		
	}

	// hkTransform <--> CMatrix 		
	void MathsTools::CMatrixTohkTransform(const CMatrix& obFrom, hkTransform& obTo)
	{
		obTo = CMatrixTohkTransform(obFrom);
	}

	void MathsTools::hkTransformToCMatrix(const hkTransform& obFrom, CMatrix& obTo)
	{
		obTo = hkTransformToCMatrix(obFrom);
	}

	hkTransform MathsTools::CMatrixTohkTransform(const CMatrix& obFrom)
	{		
		return hkTransform(CMatrixTohkQuaternion(obFrom), CPointTohkVector(obFrom.GetTranslation()));
	}

	CMatrix	MathsTools::hkTransformToCMatrix(const hkTransform& obFrom)
	{
		return CMatrix(hkQuaternionToCQuat(hkQuaternion(obFrom.getRotation())), hkVectorToCPoint(obFrom.getTranslation()));		
	}
#endif
} // namespace Physics 
