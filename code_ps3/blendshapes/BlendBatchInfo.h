//--------------------------------------------------
//!
//!	\file BlendBatchInfo.h
//!	This is a dummy holding a few doxygen comments.
//!
//--------------------------------------------------

#ifndef _BLENDBATCHINFO_H_
#define _BLENDBATCHINFO_H_


struct BSBlendBatchInfo
{
	CMatrix		m_aStreamMatrices[2];		//!< recontruction matrix & inverse

	void*		m_pVB;						//!< vb ptr
	u_int		m_iVBOffset;				//!< vb ptr offset
	u_int		m_iNumOfVerts;				//!< num of verts in this batch
	u_int		m_iVertexStride;			//!< vertex stride
	
	void*		m_pBSTargets;				//!< bstarget array ptr
	float*		m_pBSTargetWeights;			//!< bstarget weight array ptr
	u_int		m_iNumOfBSTargets;			//!< num of bstargets
	float		m_fBlendThreshold;			//!< blend threshold
};	

#endif // end of _BLENDBATCHINFO_H_
