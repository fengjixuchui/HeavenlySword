#ifndef _VERLETDEF_PS3_H_
#define _VERLETDEF_PS3_H_



//--------------------------------------------------
//!
//!	\file verletdef.h
//!	This is a dummy holding a few doxygen comments.
//!
//--------------------------------------------------


#include "spu/flags/data.h"
#include "core/explicittemplate.h"

namespace Physics
{
//--------------------------------------------------
//!
//!	\file matrerial instance def, should be coming from an XML file
//!
//--------------------------------------------------
class VerletMaterialInstanceDef
{
public:
	// texture name
	ntstd::String m_surfaceTextureName;
	ntstd::String m_normalTextureName;
	// speuclar
	float m_specularCoef;
	float m_specularColor[3];
	float m_diffuseColor[3];
public:
	VerletMaterialInstanceDef(ntstd::String surfaceTextureName, ntstd::String normalTextureName);
}; // end of class VerletMaterialInstanceDef

//--------------------------------------------------
//!
//--------------------------------------------------
class VerletInstanceDef
{
public:
	class Particle
	{
	public:
		CPoint m_position;
		float m_fInvMass;
	public:
		Particle() 
			:m_position(CONSTRUCT_CLEAR)
			,m_fInvMass(0.0f)
		{
			// nothing
		}
		Particle(CPoint& position, float fInvMass) 
			:m_position(position)
			,m_fInvMass(fInvMass)
		{
			// nothing
		}
	}; // end of class Particle
public:
	// physics
	ntstd::Vector<Particle> m_particles;
	ntstd::Vector<Pixel2> m_constraints;
	
	// mesh
	typedef ntstd::Vector<uint16_t> MeshIndices;
	MeshIndices m_connectivity;

	// mesh
	typedef ntstd::Vector<uint16_t> StaticPoint;
	StaticPoint m_staticPoints;
	
	// grid size
	Vec2 m_realSize;
	Pixel2 m_gridSize;

	// static buffer
	typedef ntstd::Vector<FlagBinding::VertexStatic> StaticBuffer;
	StaticBuffer m_staticBuffer;

	// connectivity for tangeant plane
	typedef ntstd::Vector<Pixel2> TangeantPlaneContainer;
	TangeantPlaneContainer m_tangeantPlaneIndices;

	
	// offset
	CMatrix m_offset;
public:
	// get nb particles
	uint32_t GetNbParticles() const  {return m_particles.size();}
	// rectangle procedural verlet
	VerletInstanceDef(Vec2 realSize, Pixel2 gridSize, float fDefaultPMass, const CMatrix& offset);
}; // end of class VerletInstanceDef






} //Physics


#endif // end of _VERLETDEF_PS3_H_
