#ifndef SPEEDTREEBILLBOARD_H
#define SPEEDTREEBILLBOARD_H

#include "anim/transform.h"

class CRenderableSpeedTreeCell;
class CSpeedTreeWrapper;
class CRenderable;
class SpeedTreeBillboardVertex;

namespace SpeedTreeBillboard
{
	const unsigned int c_numVertsPerBillboard = 4;

	class CCell
	{
		typedef ntstd::Vector<CSpeedTreeWrapper*>	TreeList;
		static const int	c_bufferIndexDisabled	= -1;
		static const int	c_bufferIndexEnqueued	= -2;


	public:

		CCell();

		~CCell();

		void SetPosition(float x0, float y0, float z0, float dx, float dy, float dz);
		void SetVertexBuffers(CSpeedTreeBillboardBuffers* vb);

		unsigned int SubmitGeometry(SpeedTreeBillboardVertex* dest, unsigned int treeIndex) const;

		CRenderable*				GetRenderable();

		void AddTree(CSpeedTreeWrapper* tree)
		{
			m_list.push_back(tree);
		}

		unsigned int GetNumTrees() const
		{
			return m_list.size();
		}


		int			GetBufferIndex() const;

		void		SetDisabled();

		void		SetEnqueued();

		bool		IsDisabled() const;

		bool		IsEnqueued() const;

		void		SetBufferIndex(int index);

		void		EnableRendering();

	private:
		SpeedTreeBillboardVertex* DoInstance(SpeedTreeBillboardVertex* dest, const CSpeedTreeWrapper* tree, float width, float height, unsigned int numImages) const;
		int CCell::GetFirstInstanceIndex(const CSpeedTreeWrapper* treeWrapper, int start, int end) const;

		Transform					m_transform;
		CRenderableSpeedTreeCell*	m_renderable;
		TreeList					m_list;
		TreeList					m_baseList;
	};

}

#endif
