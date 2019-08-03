#include "gfx/triangle.h"

#include "core/visualdebugger.h"



void Triangle::DebugDraw(uint32_t color)
{
#ifndef _GOLD_MASTER
	g_VisualDebug->RenderLine(
		CPoint(m_vertices[0]),
		CPoint(m_vertices[1]),color);
	g_VisualDebug->RenderLine(
		CPoint(m_vertices[1]),
		CPoint(m_vertices[2]),color);
	g_VisualDebug->RenderLine(
		CPoint(m_vertices[2]),
		CPoint(m_vertices[0]),color);
#endif
}


Triangle::Triangle()
{
	m_vertices[0]=CVector(CONSTRUCT_CLEAR);
	m_vertices[1]=CVector(CONSTRUCT_CLEAR);
	m_vertices[2]=CVector(CONSTRUCT_CLEAR);
}

Triangle::Triangle(const CVector& a, const CVector& b, const CVector& c)
{
	m_vertices[0]=a;
	m_vertices[1]=b;
	m_vertices[2]=c;
}
	
void Triangle::Subdivide(Array<Triangle,4>& res) const
{
	const CVector& a = m_vertices[0];
	const CVector& b = m_vertices[1];
	const CVector& c = m_vertices[2];
	CVector ab = (a+b)*0.5f;
	CVector bc = (b+c)*0.5f;
	CVector ca = (c+a)*0.5f;
	res[0]=Triangle(a,ab,ca);
	res[1]=Triangle(ab,b,bc);
	res[2]=Triangle(bc,c,ca);
	res[3]=Triangle(ca,ab,bc);
}
	
	
void Triangle::Translate(const CVector& v)
{
	for(int iVertice = 0 ; iVertice < 3 ; iVertice++ )
	{
		m_vertices[iVertice] += v;
	}
}

void Triangle::Scale(const CVector& v)
{
	for(int iVertice = 0 ; iVertice < 3 ; iVertice++ )
	{
		m_vertices[iVertice] *= v;
	}
}

void Triangle::Transform(const CMatrix& m)
{
	for(int iVertice = 0 ; iVertice < 3 ; iVertice++ )
	{
		m_vertices[iVertice] = m_vertices[iVertice] * m;
	}
}

void Triangle::DivideByW()
{
	for(int iVertice = 0 ; iVertice < 3 ; iVertice++ )
	{
		m_vertices[iVertice] /= m_vertices[iVertice].W();
	}
}
bool Triangle::IsInfinite()
{
	bool bIsInfinite = false;
	for(int iVertice = 0 ; iVertice < 3 ; iVertice++ )
	{
		bIsInfinite = bIsInfinite || (abs(m_vertices[iVertice].W())<0.000001f);
	}
	return bIsInfinite;
}

CVector Triangle::Middle() const
{
	return (m_vertices[0]+m_vertices[1]+m_vertices[2])*(1.0f/3.0f);
}
