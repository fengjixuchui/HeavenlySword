#include "verletdef_ps3.h"

#include "verletmaterial_ps3.h"
#include "core/boostarray.inl"

namespace Physics
{

VerletMaterialInstanceDef::VerletMaterialInstanceDef(ntstd::String surfaceTextureName, ntstd::String normalTextureName)
	:m_surfaceTextureName(surfaceTextureName)
	,m_normalTextureName(normalTextureName)
{
	m_specularCoef = 1.0f;

	m_specularColor[0] = 1.0f;
	m_specularColor[1] = 1.0f;
	m_specularColor[2] = 1.0f;

	m_diffuseColor[0] = 1.0f;
	m_diffuseColor[1] = 1.0f;
	m_diffuseColor[2] = 1.0f;
}










// rectangle procedural verlet
VerletInstanceDef::VerletInstanceDef(Vec2 realSizeAux, Pixel2 gridSizeAux, float fDefaultPMass, const CMatrix& offset)
	:m_realSize(realSizeAux)
	,m_gridSize(gridSizeAux)
	,m_offset(offset)
{
	static const float g_fEpsilon = FLT_EPSILON;

	// texture name
	ntstd::String m_surfaceTextureName;
	ntstd::String m_normalTextureName; 

	// init particle
	uint32_t uiNbParticles = m_gridSize[0]*m_gridSize[1];
	m_particles = ntstd::Vector<Particle>(uiNbParticles);
	
	// make particle
	{
		Vec2 dec(realSizeAux[0]/(gridSizeAux[0]-1),realSizeAux[1]/(gridSizeAux[1]-1));
		float fInvDefaultPMass = 1.0f/fDefaultPMass;
		for(int i = 0; i < m_gridSize[0]; i++)
		{
			for(int j = 0; j < m_gridSize[1]; j++)
			{
				int iIndex = i*m_gridSize[1] + j;
				CPoint particle(0.f, j*dec[1], i*dec[0]);
				if(j==(m_gridSize[1]-1))
				{
					m_staticPoints.push_back(iIndex);
				}
				m_particles[iIndex]=Particle(particle,fInvDefaultPMass);
			};
		};
	}
	ntstd::sort(&m_staticPoints[0], &m_staticPoints[0]+m_staticPoints.size());

	// make constraint

	// init constraint
	ntstd::Vector<Pixel2> offsetContainer;
	offsetContainer.reserve(6);
	offsetContainer.push_back( Pixel2(0,1) );
	offsetContainer.push_back( Pixel2(1,-1) );
	offsetContainer.push_back( Pixel2(1,0) );
	offsetContainer.push_back( Pixel2(1,1) );
	offsetContainer.push_back( Pixel2(0,2) );
	offsetContainer.push_back( Pixel2(2,0) );
	
	Pixel2 pointIndex;
	for(pointIndex[0] = 0; pointIndex[0] < m_gridSize[0]; pointIndex[0]++)
	{
		for(pointIndex[1] = 0; pointIndex[1] < m_gridSize[1]; pointIndex[1]++)
		{
			for(uint32_t iLocalConstraint = 0 ; iLocalConstraint < offsetContainer.size() ; ++iLocalConstraint )
			{
				Pixel2 offset = offsetContainer[iLocalConstraint];
				Pixel2 link = pointIndex + offset;
				if(link[0]>=0 && link[0]<m_gridSize[0] && link[1]>=0 && link[1]<m_gridSize[1])
				{
					uint32_t uiParticle = pointIndex[0]*m_gridSize[1] + pointIndex[1];
					uint32_t uiLinkIndex = link[0]*m_gridSize[1] + link[1];
					if(!(m_particles[uiParticle].m_fInvMass<g_fEpsilon && m_particles[uiLinkIndex].m_fInvMass<g_fEpsilon))
					{
						m_constraints.push_back(Pixel2(uiParticle,uiLinkIndex));
					}
				}
			}
		}
	}


	// texcorrd
	m_staticBuffer = StaticBuffer(GetNbParticles());
	{
		for(int i = 0; i < m_gridSize[0]; i++)
		{
			for(int j = 0; j < m_gridSize[1]; j++)
			{
				int iIndex = i*m_gridSize[1] + j;
				Vec2 texcoord(float(i) / float(m_gridSize[0]-1), float(j) / float(m_gridSize[1]-1));

				m_staticBuffer[iIndex].m_normalTexcoord[0] = texcoord[0];
				m_staticBuffer[iIndex].m_normalTexcoord[1] = texcoord[1];

				m_staticBuffer[iIndex].m_diffuseTexcoord[0] = texcoord[0];
				m_staticBuffer[iIndex].m_diffuseTexcoord[1] = texcoord[1];
			};
		};
	}

	// mesh connectivity
	{
		uint32_t uiNbTriangles = (m_gridSize[0]-1)*(m_gridSize[1]-1)*2*3;
		m_connectivity = MeshIndices( uiNbTriangles );
		uint16_t tmp[6];
		for(int i = 0; i < m_gridSize[0]-1; i++)
		{
			for(int j = 0; j < m_gridSize[1]-1; j++)
			{
				int iIndex = i*(m_gridSize[1]-1) + j;
				
				tmp[0]=(i+0)*m_gridSize[1] + (j+0);
				tmp[1]=(i+1)*m_gridSize[1] + (j+0);
				tmp[2]=(i+0)*m_gridSize[1] + (j+1);

				tmp[3]=(i+1)*m_gridSize[1] + (j+0);
				tmp[4]=(i+1)*m_gridSize[1] + (j+1);
				tmp[5]=(i+0)*m_gridSize[1] + (j+1);
				
				NT_MEMCPY(&m_connectivity[iIndex*6],tmp,6*sizeof(uint16_t));
			};
		};
	}

	// tangeant plane container
	{
		m_tangeantPlaneIndices = TangeantPlaneContainer(GetNbParticles());
		Pixel2 pointIndex;
		for(pointIndex[0] = 0; pointIndex[0] < m_gridSize[0]; pointIndex[0]++)
		{
			for(pointIndex[1] = 0; pointIndex[1] < m_gridSize[1]; pointIndex[1]++)
			{
				// compute x and y decalage
				int32_t rel_xDec = (pointIndex[0]<m_gridSize[0]-1)?1:-1;
				int32_t rel_yDec = (pointIndex[1]<m_gridSize[1]-1)?1:-1;
				Pixel2 xdec = pointIndex + Pixel2(rel_xDec,0);
				Pixel2 ydec = pointIndex + Pixel2(0,rel_yDec);
				// compute absolute indices
				uint32_t abs_point = pointIndex[0]*m_gridSize[1] + pointIndex[1];
				uint32_t abs_xdec = xdec[0]*m_gridSize[1] + xdec[1];
				uint32_t abs_ydec = ydec[0]*m_gridSize[1] + ydec[1];
				ntAssert(abs_xdec<GetNbParticles());
				ntAssert(abs_ydec<GetNbParticles());
				// store relative value
				m_tangeantPlaneIndices[abs_point][0]=abs_xdec-abs_point;
				m_tangeantPlaneIndices[abs_point][1]=abs_ydec-abs_point;
			}
		}
	}
}




} //Physics
