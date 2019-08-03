#ifndef SPEEDGRASS_RENDERABLE
#define SPEEDGRASS_RENDERABLE

#include "core/explicittemplate.h"
#include "gfx/renderable.h"
#include "gfx/vertexdecleration_ps3.h"
#include "anim/transform.h"


struct CSpeedGrassVertex
{
	CSpeedGrassVertex( 
		const CPoint&		point,
		const CDirection&	direction,
		const float*		texCoords,
		float				vertexIndex,
		float				bladeSize,
		float				windWeight,
		float				noiseFactor,
		uint8_t*			color )
		: m_position(point.X(), point.Y(), point.Z())
		, m_texCoords(texCoords[0], texCoords[1])
		, m_params(vertexIndex, bladeSize, windWeight, noiseFactor)
		, m_normal(direction.X(), direction.Y(), direction.Z())
		, m_color(*(uint32_t*)color)
	{
	}

	CPoint		m_position;
	Vec2		m_texCoords;
	Vec4		m_params;
	CDirection	m_normal;
	uint32_t	m_color;
};

struct ISpeedGrassData;

template <class T> class CLimits;

class CVertexProps;

class CSpeedGrassRenderable : public CRenderable
{
	static const unsigned int c_numVertsPerBlade = 4;

public:
	CSpeedGrassRenderable(Texture::Ptr texture, unsigned int numBlades, ISpeedGrassData* dataInterface);
	~CSpeedGrassRenderable();

	void SetPosition(float x0, float y0, float z0, float dx, float dy, float dz);
	void AddBlade(const CSpeedGrassVertex& vertex);
	void SetLimits(const CLimits<CPoint>& pos, const CLimits<CDirection>& normal, const CLimits<Vec2>& texcoords, const CLimits<float>& bladeSize, const CLimits<float>& windWeight, const CLimits<float>& noiseFactor);

	unsigned int GetVertexFootprint();


	virtual void RenderDepth();
	virtual void RenderMaterial();
	virtual void RenderShadowMap() {}
	virtual void RenderShadowOnly() {}

private:
	template <class VertexShader>
	void SetCommonConstants(const VertexShader& VS) const;
	template <class VertexShader>
	void SetCompressionConstants(const VertexShader& VS) const;

	void SetLightingConstants(Shader* VS, Shader* PS) const;
	void SendGeometry(Shader* VS, Shader* PS) const;
	void PostRender() const;

private:
	unsigned int		m_numVerts;
	Texture::Ptr		m_texture;
	Transform			m_transform;
	VBHandle			m_vertexBuffer;
	ISpeedGrassData*	m_dataInterface;
	CVertexProps*		m_vertexProps;

};

#endif
