//! -------------------------------------------
//! aiworldvolumes.h
//!
//! Volumes used to describe the AI's world
//!
//! Author: Dario Sancho-Pradel (DSP)
//!
//!--------------------------------------------

#ifndef _AIWOLRDVOLUMES_H
#define _AIWOLRDVOLUMES_H

#include "aiVoultingvolumeparams.h"
#include "area/areasystem.h"

// Forward Declarations

class CAINavigPath;

typedef struct _SAIVolSegment SAIVolSegment;
typedef struct _SGoAroundNodes SGoAroundNodes;
//typedef struct _SVaultingParams SVaultingParams;
typedef ntstd::Vector<SGoAroundNodes, Mem::MC_AI> SGoAroundNodesVector;
typedef ntstd::List<SGoAroundNodes*, Mem::MC_AI> SGoAroundNodesList;

//!--------------------------------------------
//! Class CAIWorldVolume
//!--------------------------------------------
class CAIWorldVolume
{
	HAS_INTERFACE(CAIWorldVolume)

	public:

		enum eVolumeType
		{
			VTYPE_POLYHEDRON = 0,
			VTYPE_CYLINDER
		};

		typedef ntstd::Vector<SAIVolSegment, Mem::MC_AI> SAIVolSegmentVector;
		
	public:
	
		// Ctor, et al.
		CAIWorldVolume() :	m_fHeight(0.0f), m_fRadius(0.0f), m_fRadiusSQR(0.0f), m_fRadiusPOLY(0.0f),
							m_uiNumberOfVertex(0), m_eVolumeType(VTYPE_POLYHEDRON),
							m_bContRegion(false), m_bTransparent(false), m_bVaultingVolume(false), m_bGoAroundVolume(false),
							m_fGoAroundMinDst(2.0f), m_fGoAroundDistFactor(1.0f), m_fGoAroundNodeRadius(1.0f), m_iPathFindMatrix(NULL),
							m_iMappedAreaInfo(0), m_bDebugMe(false), m_pVaultingParams(NULL) {}
		~CAIWorldVolume();

		void PostConstruct ( void );
		
		bool	IsInside						( const CPoint&													) const;
		bool	IntersectsAIVolume				( const CPoint&, const CPoint&, CPoint* , CDirection*			) const;
		bool	IntersectsAIExclusionVolume		( const CPoint&, const CPoint&, CPoint* , CDirection*			) const;
		bool	IntersectsAIContainmentVolume	( const CPoint&, const CPoint&, CPoint* , CDirection*			) const;
		bool	IntersectsSegments				( const CPoint&, const CPoint&, const SAIVolSegment*, CPoint*	) const;
		bool	IntersectsCircle				( const CPoint&, const CPoint&, CPoint*							) const;
		bool	PreventsLineofSight				( const CPoint&, const CPoint& ) const;
		bool	IsTransparent					( void ) const { return m_bTransparent; }
		bool	IsGoAroundVolume				( void ) const { return m_bGoAroundVolume; }
		bool	IsVaultingVolume				( void ) const { return m_bVaultingVolume; }
		bool	IsContainmentRegion				( void ) const { return m_bContRegion; }
		bool	IsInProcessingDistance			( const CPoint& obFrom, const CPoint& obTo ) const;
		bool	IsInProcessingDistance			( const CPoint& obFrom, float fCheckRadius ) const;
		bool	SetCentre						( const CPoint& obPos) { if (m_eVolumeType == VTYPE_CYLINDER) { m_obCentre=obPos; return true; } else { return false; } }
		const char*	GetName						( void ) const { return m_tmp_Name; }
	
		CPoint	GetCentre						( void ) const { return m_obCentre; }
		float	GetPolyRadius					( void ) const { return (m_eVolumeType == VTYPE_POLYHEDRON ? m_fRadiusPOLY : -1.0f); }
		
		unsigned int GetSectorBits				( void ) const { return m_iMappedAreaInfo; }
		
		// Go around volumes
		bool	GetPathAroundVolume				( const CPoint&, const CPoint&,  CAINavigPath* );

		// Vaulting Volumes
		const SVaultingParams*	GetVaultingParams ( void ) const { return m_pVaultingParams; }

		void	DebugRender		( void );

		//! the army stuff wants to extract the volume segments for SPU usage, so this is a read-only accessor
		const SAIVolSegmentVector& GetSegmentList() const { return m_vpSegments; }

		// Return the height of the volume
		float	GetHeight			( void ) const	{ return m_fHeight; }

		bool	IsInActiveSector	( void ) const	{ return true; } //{ return (((1 << (AreaManager::Get().GetCurrActiveArea()-1)) & m_iMappedAreaInfo) != 0); }
		bool	HasInvalidSectorBits( void ) const	{ return ( m_iMappedAreaInfo== 0 ); }

		// The calculated perpendicular is:
		//			Perpendicular
		//           |
		//           |
		//  A|---------------->B

		CDirection GetPerpendicular(const CDirection& obDir) { return CDirection(obDir.Z(),obDir.Y(),-obDir.X()); }

	private:

		void		GeneratePathFindTable	( void );
		int			GetFirstVertexFromShortestPath ( unsigned int uiFrom, unsigned int uiTo );
		
		
	private:
		typedef ntstd::Vector<CPoint, Mem::MC_AI> AIPointVector;
		AIPointVector					m_vectorpVertex;	// List of Vertices
		SAIVolSegmentVector				m_vpSegments;		// List of Segments
		SGoAroundNodesVector			m_vpGoAroundNodes;
		CPoint							m_obCentre;			// Base Centre
		float							m_fHeight;			// Hieight
		float							m_fRadius;			// Radius (for CYLINDER)
		float							m_fRadiusSQR;		// Radius SQR (for CYLINDER)
		float							m_fRadiusPOLY;		// Radius (for POLYHEDRON)
		unsigned int					m_uiNumberOfVertex;	// Number of vertices (for colynomial volumes)
		eVolumeType						m_eVolumeType;		// Convex, Convcave or CYLINDER
		bool							m_bContRegion;		// False(Wall-like Volume), True(Containement Region)
		bool							m_bRelaxedIsect;	// True(only contiguous segment are checked for multiple intersections)
		bool							m_bTransparent;		// Can I see through it?
		bool							m_bVaultingVolume;
		bool							m_bGoAroundVolume;// Is transparent, so AIs can see the player and chase around
		float							m_fGoAroundMinDst;
		float							m_fGoAroundDistFactor;
		float							m_fGoAroundNodeRadius;
		int*							m_iPathFindMatrix;
		unsigned int					m_iMappedAreaInfo;  // Sector Bits
		bool							m_bDebugMe;
		char							m_tmp_Name[64];
		SVaultingParams*				m_pVaultingParams;
		
};

typedef ntstd::List<CAIWorldVolume*, Mem::MC_AI> AIWorldVolumeList;

typedef struct _SAIVolSegment
{
	_SAIVolSegment() : P0(CONSTRUCT_CLEAR), P1(CONSTRUCT_CLEAR), Normal(CONSTRUCT_CLEAR), Dir(CONSTRUCT_CLEAR), Centre(CONSTRUCT_CLEAR) {}

	_SAIVolSegment(const CPoint& p0, const CPoint& p1, const CDirection& n, const CDirection& d) :
					P0(p0), P1(p1), Normal(n), Dir(d), Centre((p0+p1)/2) {}

	// X(t,x,y) = P(x,z)+t*Dir, where X = Point in the segment
	CPoint		P0;		// Origin of the segment
	CPoint		P1;		// End of the segment
	CDirection	Normal;	// Segments Normal
	CDirection	Dir;	// Segments Direction = P1-P0
	CPoint		Centre;	// Segment's centre
} SAIVolSegment;

typedef struct _SGoAroundNodes
{
	_SGoAroundNodes(const CPoint& p0, const CDirection& d) :
					Vertex(p0), FirstNodePos(CPoint(p0+3*d)), CornerDir(d), VertexUsage(0) {}

	CPoint		Vertex;			// Vertex Point
	CPoint		FirstNodePos;			// 
	CDirection	CornerDir;		// Normal in the direction of the corner outwards
	int			VertexUsage;	// Number of entities targeting this vertex
} SGoAroundNodes;

//typedef struct _SVaultingParams
//{
//	_SVaultingParams() : fVaultingDistance(0.0f) {}
//
//	float fVaultingDistance;
//} SVaultingParams;
//
#endif // _AIWOLRDVOLUMES_H

