#include "anim/hierarchy.h"
#include "gfx/sector.h"
#include "speedtreert.h"
#include "speedtreerenderable_ps3.h"
#include "speedtreewrapper_ps3.h"

#include "core/gatso.h"


#include "speedtreebillboard.h"




namespace SpeedTreeBillboard
{

	CCell::CCell()
		: m_renderable(NT_NEW_CHUNK(SPEEDTREE_MEMORY_CHUNK) CRenderableSpeedTreeCell(&m_transform))
	{
		CSector::Get().GetRenderables().AddRenderable(m_renderable);
		m_renderable -> SetBufferIndex(c_bufferIndexDisabled);
	}

	CCell::~CCell()
	{
		if (m_transform.GetParent())
		{
			m_transform.RemoveFromParent();
		}
		CSector::Get().GetRenderables().RemoveRenderable(m_renderable);
		NT_DELETE_CHUNK(SPEEDTREE_MEMORY_CHUNK, m_renderable);
	}


	void CCell::SetPosition(float x0, float y0, float z0, float dx, float dy, float dz)
	{
		ntAssert(m_renderable);
		CPoint pos(x0, y0, z0);
		m_transform.SetLocalMatrix( CMatrix( CQuat( CONSTRUCT_IDENTITY ), pos ) );
		CHierarchy::GetWorld()->GetRootTransform()->AddChild( &m_transform );
		//m_transform.ClearFlagBits( TRANSF_WORLD_MATRIX_INVALID );
		//m_renderable -> SetBounds(CAABB( pos, pos + CPoint(dx, dy, dz)));
		m_renderable -> SetBounds(CAABB( CPoint(0, 0, 0), CPoint(dx, dy, dz)));
	}

	void CCell::SetVertexBuffers(CSpeedTreeBillboardBuffers* vb)
	{
		if (m_renderable)
		{
			// At this point the cell should have trees in it if there are any
			if (!m_list.empty())
			{
				ntstd::sort(m_list.begin(), m_list.end(), &CSpeedTreeWrapper::SortPredicate);
				TreeList::iterator	iter = m_list.begin();
				//m_baseList.push_back(*iter++);
				while(iter != m_list.end())
				{
					if (!(*iter) -> GetParent())
					{
						m_baseList.push_back(*iter++);
						continue;
					}
					//ntstd::pair<TreeList::iterator, TreeList::iterator> range = std::equal_range(iter, m_list.end(), *iter, &CSpeedTreeWrapper::SortPredicate);
					for (TreeList::iterator baseIter = m_baseList.begin(); baseIter != m_baseList.end(); ++ baseIter)
					{
						if ((*iter) -> GetParent() == *baseIter )
						{
							goto BaseFound;
						}
					}
					m_baseList.push_back((*iter) -> GetParent());
BaseFound:			++ iter;
				}
				
				m_renderable -> SetTreeData(vb, &m_baseList[0], m_baseList.size());
			}
		}
	}

	SpeedTreeBillboardVertex* CCell::DoInstance(SpeedTreeBillboardVertex* dest, const CSpeedTreeWrapper* tree, float width, float height, unsigned int numImages) const
	{
		const float* pos = tree -> GetPosition();
		float azimuth = tree -> GetRotation();

		CSpeedTreeRT::SLodValues Lod;
		CSpeedTreeRT* theTree = tree -> GetSpeedTree();
		//theTree -> ComputeLodLevel();
		theTree -> GetLodValues(Lod);

		float fade = Lod.m_fBillboardFadeOut;

		if (fade > 0)
		{
			for (unsigned int vertex = 0; vertex < c_numVertsPerBillboard; ++ vertex)
			{
				NT_PLACEMENT_NEW(dest ++) SpeedTreeBillboardVertex(pos, vertex, width, height, azimuth, fade, numImages);
			}
		}
		
		return dest;
	}

	int CCell::GetFirstInstanceIndex(const CSpeedTreeWrapper* treeWrapper, int start, int end) const
	{
		if (start == end)
		{
			return - 1;
		}

		for (int index = start; index < end; ++ index)
		{
			if (m_list[index]->GetParent() == treeWrapper)
			{
				return index;
			}
		}

		return -1;
	}

	unsigned int CCell::SubmitGeometry(SpeedTreeBillboardVertex* dest, unsigned int treeIndex) const
	{
		if (IsDisabled())
		{
			ntAssert(0);
			return 0;
		}

		if (m_list.empty())
		{
			ntAssert_p(0, ("SPEEDTREE : Empty billboard list\n"));
			return 0;
		}

		if (!(treeIndex < m_baseList.size()))
		{
			ntAssert_p(0, ("SPEEDTREE : Billboard tree index %d out of range. Base list size: %d list size: %d\n", treeIndex, m_baseList.size(), m_list.size()));
			return 0;
		}

		CGatso::Start( "SPEEDTREE_Cell::SubmitGeometry" );

		SpeedTreeBillboardVertex* start = dest;

		CSpeedTreeWrapper* treeWrapper = m_baseList[treeIndex];


		CSpeedTreeRT*	speedTree = treeWrapper -> GetSpeedTree();
		ntAssert(speedTree);

		CSpeedTreeRT::SGeometry sGeometry;
		speedTree -> GetGeometry(sGeometry, SpeedTree_BillboardGeometry);

		unsigned int numImages = sGeometry.m_s360Billboard.m_nNumImages;

		// see if the base tree is in the cell
		for (TreeList::const_iterator iter = m_list.begin(); iter != m_list.end() && NULL == (*iter) -> GetParent(); ++ iter)
		{
			if (*iter == treeWrapper)
			{
				float width = sGeometry.m_s360Billboard.m_fWidth * treeWrapper -> GetScale();
				float height = sGeometry.m_s360Billboard.m_fHeight * treeWrapper -> GetScale();
				dest = DoInstance(dest, treeWrapper, width, height, numImages);

				break;
			}
		}

		//if (0 == treeIndex) m_searchIndex = m_baseList.size();

		//ntstd::pair<TreeList::const_iterator, TreeList::const_iterator> range = std::equal_range(m_list.begin(), m_list.end(), treeWrapper, &CSpeedTreeWrapper::SortPredicate);

		int startIndex = GetFirstInstanceIndex(treeWrapper, 0, m_list.size());

		if (-1 != startIndex)
		{
			for (unsigned int index = startIndex; index < m_list.size() && m_list[index]->GetParent() == treeWrapper; ++ index)
			{
				CSpeedTreeWrapper* treeInstance = m_list[index];

				float width = sGeometry.m_s360Billboard.m_fWidth * treeInstance -> GetScale();
				float height = sGeometry.m_s360Billboard.m_fHeight * treeInstance -> GetScale();

				dest = DoInstance(dest, treeInstance, width, height, numImages);

				ntAssert(dest - start <=  (int)CSpeedTreeBillboardBuffers::m_scratchSize);
			}
		}

		CGatso::Stop( "SPEEDTREE_Cell::SubmitGeometry" );

		//ntAssert_p((dest - start), ("SPEEDTREE : Failed to submit billboard geometry\n"));
		return	dest - start;

	}

	CRenderable*	CCell::GetRenderable()
	{
		return m_renderable;
	}

	int			CCell::GetBufferIndex() const
	{
		return m_renderable -> GetBufferIndex(); 
	}

	void		CCell::SetDisabled()
	{
		m_renderable -> SetBufferIndex(c_bufferIndexDisabled);
	}

	void		CCell::SetEnqueued()
	{
		m_renderable -> SetBufferIndex(c_bufferIndexEnqueued);
	}

	bool		CCell::IsDisabled() const
	{
		return GetBufferIndex() < 0;
	}

	bool		CCell::IsEnqueued() const
	{
		return GetBufferIndex() == c_bufferIndexEnqueued;
	}

	void		CCell::SetBufferIndex(int index)
	{
		// TODO: check bounds
		m_renderable -> SetBufferIndex(index);
	}

	void		CCell::EnableRendering()
	{
		ntAssert(!m_list.empty());

		m_renderable -> DisableRendering(false);
	}



}
