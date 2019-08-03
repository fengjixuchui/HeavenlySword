#ifndef _TRIANGLE_H_
#define _TRIANGLE_H_


#include "core/explicittemplate.h"


class Triangle
{
public:
	Array<CVector,3> m_vertices;
	
	Triangle();
	
	Triangle(const CVector& a, const CVector& b, const CVector& c);
		
	void Subdivide(Array<Triangle,4>& res) const;
	
	void Scale(const CVector& v);
	
	void Transform(const CMatrix& m);
	
	void Translate(const CVector& v);
	
	void DivideByW();
	
	bool IsInfinite();
	
	
	CVector Middle() const;
	
	void DebugDraw(uint32_t color);
}; // end of class Triangle
typedef ntstd::List<Triangle*,Mem::MC_AI>						AITriangleList;


#endif // end of _TRIANGLE_H_
