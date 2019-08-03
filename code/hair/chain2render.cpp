#include "hair/chain2render.h"
#include "hair/chaincore.h"
#include "core/visualdebugger.h"


CPoint ChainAnimation2Render::OnePoint::GetWorldPosition() const
{
	return CPoint(m_pElem->GetExtremity().m_position);
}

ChainAnimation2Render::OneCurve::OneCurve(const ExternalCurve& curve)
	:InternalCurveBase(curve.size())
{
	int iCount = 0;
	for(ExternalCurve::const_iterator it = curve.begin();
		it != curve.end();
		++it)
	{
		(*this)[iCount] = OnePoint(*it);
		iCount++;
	}
}


void ChainAnimation2Render::OneCurve::DebugDraw()
{
#ifndef _GOLD_MASTER
	for(u_int i = 1 ; i < size() ; i++ )
	{
		g_VisualDebug->RenderLine((*this)[i].GetWorldPosition(),(*this)[i-1].GetWorldPosition(), DC_WHITE );
	}
#endif
}

ChainAnimation2Render::ChainAnimation2Render()
{
	// nothing
}

ChainAnimation2Render::~ChainAnimation2Render()
{
	for(Container::iterator it = m_container.begin();
		it != m_container.end();
		++it)
	{
		NT_DELETE_CHUNK( Mem::MC_PROCEDURAL, *it );
	}
}

void ChainAnimation2Render::Add(const ExternalCurve& pCurve)
{
	m_container.push_back( NT_NEW_CHUNK ( Mem::MC_PROCEDURAL ) OneCurve(pCurve) );
}


void ChainAnimation2Render::DebugDraw()
{
	for(Container::iterator it = m_container.begin();
		it != m_container.end();
		++it)
	{
		(*it)->DebugDraw();
	}
}
