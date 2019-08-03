//--------------------------------------------------
//!
//!	\file effecttrail_edge.h
//!	This is a dummy holding a few doxygen comments.
//!
//--------------------------------------------------

#ifndef _EFFECTTRAIL_EDGE_H
#define _EFFECTTRAIL_EDGE_H

#include "effect/effect_resourceman.h"

class EffectTrail_EdgeDef;

//--------------------------------------------------
//!
//!	EffectTrail_EdgeNode
//!
//--------------------------------------------------
class EffectTrail_EdgeNode
{
public:
	EffectTrail_EdgeNode();
	virtual ~EffectTrail_EdgeNode(){};

	CPoint	m_obPos;
	float	m_fTexture;

	virtual bool EditorChangeValue( CallBackParameter, CallBackParameter )
	{
		m_bHasChanged = true;
		return true;
	}

	bool HasChanged()
	{
		bool bReturn = m_bHasChanged;
		m_bHasChanged = false;
		return bReturn;
	}

	virtual void DebugRender();

	void	SetParent( EffectTrail_EdgeDef* pParent ) { m_pParent = pParent; }

private:
	bool m_bHasChanged;
	EffectTrail_EdgeDef* m_pParent;
};

//--------------------------------------------------
//!
//!	EffectTrail_EdgeDefResources
//!
//--------------------------------------------------
class EffectTrail_EdgeDefResources : public EffectResource
{
public:
	EffectTrail_EdgeDefResources();
	virtual ~EffectTrail_EdgeDefResources();
	
	virtual void GenerateResources();
	virtual bool ResourcesOutOfDate() const;

	EffectTrail_EdgeDef* m_pParent;

	const CVector*	GetVectorArray() const { return m_pVectors; }
	u_int			GetNumPoints() const { return m_iNumPoints; }

private:
	CVector*	m_pVectors;
	u_int		m_iNumPoints;
};

//--------------------------------------------------
//!
//!	EffectTrail_EdgeDef
//!	Interface defining trail edge
//!
//--------------------------------------------------
class EffectTrail_EdgeDef
{
public:
	EffectTrail_EdgeDef();
	virtual ~EffectTrail_EdgeDef(){};

	virtual void PostConstruct();

	virtual bool EditorChangeValue( CallBackParameter, CallBackParameter )
	{
		m_bHasChanged = true;
		PostConstruct();
		return true;
	}

	virtual void DebugRender();

	bool HasChanged()
	{
		bool bReturn = m_bHasChanged;

		for (	ntstd::List<EffectTrail_EdgeNode*>::iterator it = m_obEdgeNodes.begin();
				it != m_obEdgeNodes.end(); ++it )
		{
			if ((*it)->HasChanged())
				bReturn = true;
		}

		m_bHasChanged = false;
		return bReturn;
	}

	ntstd::List<EffectTrail_EdgeNode*>	m_obEdgeNodes;
	CDirection						m_obEdgeOrientYPR;
	mutable CMatrix					m_debugRenderMat;
	EffectTrail_EdgeDefResources	m_res;

	const CMatrix&	GetEdgeOrient() const { return m_edgeOrient; }

private:
	bool		m_bHasChanged;
	CMatrix		m_edgeOrient;
};

#endif //_EFFECTTRAIL_EDGE_H
