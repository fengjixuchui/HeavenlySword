#include <speedtreert.h>

#include "core/timer_ps3.h"
#include "anim/hierarchy.h"
#include "gfx/renderer.h"
#include "gfx/rendercontext.h"
#include "gfx/depthhazeconsts.h"
#include "gfx/debugshadergraph_ps3.h"
#include "speedtreeutil_ps3.h"
#include "speedgrass_renderable.h"
#include "speedgrassdatainterface_ps3.h"
#include "speedgrass_limits_ps3.h"

#include <Fw/FwEndian.h>

// CLASSES ///////////////////////////////////////////////////////////////////////////////////

// Define shader graph
class CSpeedGrassShading : public CDebugShaderGraph
{
public:
	struct ShaderDescription : public CDebugShaderGraph::ShaderDescriptionBase<2>
	{
		ShaderDescription(unsigned int pass, unsigned int shaderType)
		{
			m_levels[0] = pass;
			m_levels[1] = shaderType;
		}

	};

	CSpeedGrassShading()
		: CDebugShaderGraph(SPEEDGRASS_MEMORY_CHUNK)
	{
		CreateGraph("speedgrass_");
	}

private:
	virtual void InitGraphDefinitions()
	{
		BEGIN_GRAPH_LEVEL "material_", "depth_"										END_GRAPH_LEVEL
		BEGIN_GRAPH_LEVEL "vp", "fp"												END_GRAPH_LEVEL
	}

};

namespace
{
	CSpeedGrassShading* g_speedGrassShaders;
}


template <unsigned int Pass, unsigned int ShaderType>
class CSpeedGrassShader : public CDebugShaderBase
{
public:
	CSpeedGrassShader()
	{
		ntAssert(g_speedGrassShaders);
		m_shader = g_speedGrassShaders -> GetShader(CSpeedGrassShading::ShaderDescription(Pass, ShaderType));
	}

};

// Limits  of vertex elements for compression
struct CVertexProps
{
	CLimits<CPoint>		pos_;
	CLimits<CDirection>	normal_;
	CLimits<Vec2>		texcoords_;
	CLimits<float>		bladeSize_;
	CLimits<float>		windWeight_;
	CLimits<float>		noiseFactor_;
};

union CCompressedVector
{
	CCompressedVector(uint32_t init)
		: vInt_(init)
	{
	}

	uint32_t		vInt_;
	unsigned char	vChar_[4];
};

struct CCompressedVertex
{
	CCompressedVertex()
		:	pos_(0)
		,	normalTex_(0)
		,	params_(0)
		,	color_(0)
	{
	}

	CCompressedVector	pos_;
	CCompressedVector	normalTex_;
	CCompressedVector	params_;
	uint32_t			color_;

};

// FREE FUNCTIONS ////////////////////////////////////////////////////////////////////////////////

// Global initialization function
void CreateSpeedGrassShaders()
{
	ntAssert(!g_speedGrassShaders);
	g_speedGrassShaders = NT_NEW_CHUNK(SPEEDGRASS_MEMORY_CHUNK) CSpeedGrassShading;
}

// Global deinitializaion function
void DestroySpeedGrassShaders()
{
	ntAssert(g_speedGrassShaders);
	NT_DELETE_CHUNK(SPEEDGRASS_MEMORY_CHUNK, g_speedGrassShaders);
	g_speedGrassShaders = NULL;
}

// vertex compression utility functions
namespace
{
	// Compress a floating point value
	inline uint32_t CompressValue(float value, float minValue, float maxValue, unsigned int index)
	{
		ntAssert(minValue < maxValue);
		ntAssert(value <= maxValue && minValue <= value);
		ntAssert(index < 4);

		float p = (value - minValue) / (maxValue - minValue);

		uint32_t result = (uint32_t)(p * 255.f);
		result &= 0xff;
		result <<= index * 8;

		return result;
	}

	// compress a vector-like type
	template <class T>
	void CompressValue(CCompressedVector& dest, size_t index, size_t size, const CLimits<T>& limits, const T& value)
	{
		ntAssert(size <= sizeof(T) / 4);
		ntAssert(index + size <= 4);

		for (size_t i = 0; i < size; ++ i)
		{
			dest.vInt_ |= CompressValue(value[i], limits.min_[i], limits.max_[i], index + i);
		}
	}

	// specialization of the above for floats
	void CompressValue(CCompressedVector& dest, size_t index, size_t, const CLimits<float>& limits, const float& value)
	{
		ntAssert(index < 4);

		dest.vInt_ |= CompressValue(value, limits.min_, limits.max_, index);
	}

	// a value is decompressed by : DecompressedValue = min + CompressedValue / 255.f * (max - min)
	// so we will need to give a vertex shader a min value and a factor == (max - min) / 255
	template <class T>
	inline T ComputeCompressionFactor(const CLimits<T>& limits)
	{
		const float Inv255 = 1.f / 255.f;//0.00392156862f; // 1 / 255

		return T((limits.max_ - limits.min_) * Inv255);
	}

	inline float ComputeCompressionFactor(float minVal, float maxVal)
	{
		const float Inv255 = 1.f / 255.f;//0.00392156862f; // 1 / 255

		ntAssert(minVal <= maxVal);

		return (maxVal - minVal) * Inv255;
	}

}

// CSpeedGrassRenderable //////////////////////////////////////////////////////////////////////////////////////

CSpeedGrassRenderable::CSpeedGrassRenderable(Texture::Ptr texture, unsigned int numBlades, ISpeedGrassData* dataInterface)
	: CRenderable(&m_transform, true, false, false, RT_SPEED_GRASS)
	, m_numVerts(0)
	, m_texture(texture)
	, m_dataInterface(dataInterface)
	, m_vertexProps(NT_NEW_CHUNK(SPEEDGRASS_MEMORY_CHUNK) CVertexProps)
{
	unsigned int numVerts = c_numVertsPerBlade * numBlades;

	//const unsigned int numFields = 5;
	//GcStreamField	fields[numFields];
	//NT_PLACEMENT_NEW (&fields[0]) GcStreamField( FwHashedString("input.vPosition"), 0, Gc::kFloat, 3 );
	//NT_PLACEMENT_NEW (&fields[1]) GcStreamField( FwHashedString("input.vTexCoords"), 12, Gc::kFloat, 2 );
	//NT_PLACEMENT_NEW (&fields[2]) GcStreamField( FwHashedString("input.vParams"), 20, Gc::kFloat, 4 );
	//NT_PLACEMENT_NEW (&fields[3]) GcStreamField( FwHashedString("input.vNormal"), 36, Gc::kFloat, 3 );
	//NT_PLACEMENT_NEW (&fields[4]) GcStreamField( FwHashedString("input.Color"), 48, Gc::kUByte, 4, true );


	//m_vertexBuffer = RendererPlatform::CreateVertexStream(	numVerts, sizeof(CSpeedGrassVertex),
	//															numFields,
	//															fields,
	//															Gc::kStaticBuffer );
	const unsigned int numFields = 4;
	GcStreamField	fields[numFields];
	NT_PLACEMENT_NEW (&fields[0]) GcStreamField( FwHashedString("input.Position"), 0, Gc::kUByte, 4 );
	NT_PLACEMENT_NEW (&fields[1]) GcStreamField( FwHashedString("input.NormalTex"), 4, Gc::kUByte, 4 );
	NT_PLACEMENT_NEW (&fields[2]) GcStreamField( FwHashedString("input.Params"), 8, Gc::kUByte, 4 );
	NT_PLACEMENT_NEW (&fields[3]) GcStreamField( FwHashedString("input.Color"), 12, Gc::kUByte, 4, true );

	m_vertexBuffer = RendererPlatform::CreateVertexStream(	numVerts, sizeof(CCompressedVertex),
																numFields,
																fields,
																Gc::kStaticBuffer );


	
}

CSpeedGrassRenderable::~CSpeedGrassRenderable()
{
	if (m_transform.GetParent())
	{
		m_transform.RemoveFromParent();
	}

	NT_DELETE_CHUNK(SPEEDGRASS_MEMORY_CHUNK, m_vertexProps);
}


unsigned int CSpeedGrassRenderable::GetVertexFootprint()
{
	return (unsigned int)GcStreamBuffer::QueryResourceSizeInBytes(m_vertexBuffer -> GetCount(), m_vertexBuffer -> GetStride() );	
}


void CSpeedGrassRenderable::SetPosition(float x0, float y0, float z0, float dx, float dy, float dz)
{
	CPoint pos(x0, y0, z0);
	m_transform.SetLocalMatrix( CMatrix( CQuat( CONSTRUCT_IDENTITY ), pos ) );
	CHierarchy::GetWorld()->GetRootTransform()->AddChild( &m_transform );
	m_obBounds = (CAABB( CPoint(0, 0, 0), CPoint(dx, dy, dz)));
}

// It adds a single vertex actually
void CSpeedGrassRenderable::AddBlade(const CSpeedGrassVertex& vertex)
{
	if (m_numVerts < m_vertexBuffer -> GetCount())
	{
		//m_vertexBuffer -> Write(&vertex, m_numVerts * sizeof(CSpeedGrassVertex), sizeof(CSpeedGrassVertex));

		CCompressedVertex	hwVertex;

		hwVertex.pos_.vInt_ = 0;

		CompressValue(hwVertex.pos_, 0, 3, m_vertexProps -> pos_, vertex.m_position);
		CompressValue(hwVertex.normalTex_, 0, 2, m_vertexProps -> normal_, vertex.m_normal);
		CompressValue(hwVertex.normalTex_, 2, 2, m_vertexProps -> texcoords_, vertex.m_texCoords);

		hwVertex.params_ = (uint32_t)vertex.m_params[0];	// corner index

		CompressValue(hwVertex.params_, 1, 1, m_vertexProps -> bladeSize_, vertex.m_params[1]);
		CompressValue(hwVertex.params_, 2, 1, m_vertexProps -> windWeight_, vertex.m_params[2]);
		CompressValue(hwVertex.params_, 3, 1, m_vertexProps -> noiseFactor_, vertex.m_params[3]);

		hwVertex.color_ = vertex.m_color;

		m_vertexBuffer -> Write(&hwVertex, m_numVerts * sizeof(CCompressedVertex), sizeof(CCompressedVertex));

		++ m_numVerts;
	}
	else
	{
		ntAssert(!"Too many blades");
	}
}

void CSpeedGrassRenderable::SetLightingConstants(Shader* VS, Shader* PS) const
{
	DECLARE_VS_CONSTANT(g_vOSLightDir)
	DECLARE_VS_CONSTANT(g_SHCoeffs)
	DECLARE_PS_CONSTANT(g_VPOStoUV)
	DECLARE_VS_CONSTANT(g_worldViewMatrix)
	DECLARE_VS_CONSTANT(g_eyePosObjSpace)
	DECLARE_VS_CONSTANT(g_aConsts)
	DECLARE_VS_CONSTANT(g_gConsts)
	DECLARE_VS_CONSTANT(g_sunDir)
	DECLARE_VS_CONSTANT(g_beta1PlusBeta2)
	DECLARE_VS_CONSTANT(g_betaDash1)
	DECLARE_VS_CONSTANT(g_betaDash2)
	DECLARE_VS_CONSTANT(g_oneOverBeta1PlusBeta2)
	DECLARE_VS_CONSTANT(g_sunColor)
	DECLARE_VS_CONSTANT(g_keyDirColour);

	CMatrix	(&SHM)[3] = RenderingContext::Get()->m_SHMatrices.m_aChannelMats;

	VS -> SetVSConstant(g_SHCoeffs, SHM, 12);
	VS -> SetVSConstant(g_vOSLightDir, (float*)&RenderingContext::Get()->m_toKeyLight, 1);

	VS -> SetVSConstant(g_aConsts, CDepthHazeSetting::GetAConsts());
	VS -> SetVSConstant(g_gConsts, CDepthHazeSetting::GetGConsts());
	VS -> SetVSConstant(g_sunDir, CDepthHazeSetting::GetSunDir());
	VS -> SetVSConstant(g_beta1PlusBeta2, CDepthHazeSetting::GetBeta1PlusBeta2());
	VS -> SetVSConstant(g_betaDash1, CDepthHazeSetting::GetBetaDash1());
	VS -> SetVSConstant(g_betaDash2, CDepthHazeSetting::GetBetaDash2());
	VS -> SetVSConstant(g_oneOverBeta1PlusBeta2, CDepthHazeSetting::GetOneOverBeta1PlusBeta2());
	VS -> SetVSConstant(g_sunColor, CDepthHazeSetting::GetSunColour());
	VS -> SetVSConstant(g_eyePosObjSpace, RenderingContext::Get()->GetEyePos());
	VS -> SetVSConstant(g_worldViewMatrix, RenderingContext::Get()->m_worldToView);
	VS -> SetVSConstant(g_keyDirColour, RenderingContext::Get()->m_keyColour );

	CVector posToUV( CONSTRUCT_CLEAR );
	posToUV.X() = 1.f / (Renderer::Get().m_targetCache.GetWidth() - 1);
	posToUV.Y() = 1.f / (Renderer::Get().m_targetCache.GetHeight() - 1);

	PS -> SetPSConstant( g_VPOStoUV, posToUV );

	Renderer::Get().SetTexture(1, RenderingContext::Get()->m_pStencilTarget, true );
	Renderer::Get().SetSamplerFilterMode(1, TEXTUREFILTER_BILINEAR);
	Renderer::Get().SetSamplerAddressMode( 1, TEXTUREADDRESS_CLAMPALL );

}

template <class VertexShader>
void CSpeedGrassRenderable::SetCompressionConstants(const VertexShader& VS) const
{
	DECLARE_VS_CONSTANT(g_minPos)
	DECLARE_VS_CONSTANT(g_factorPos)
	DECLARE_VS_CONSTANT(g_minNormalTex)
	DECLARE_VS_CONSTANT(g_factorNormalTex)
	DECLARE_VS_CONSTANT(g_minParams)
	DECLARE_VS_CONSTANT(g_factorParams)

	ntAssert(m_vertexProps);

	VS -> SetVSConstant(g_minPos, m_vertexProps -> pos_.min_);
	VS -> SetVSConstant(g_factorPos, ComputeCompressionFactor(m_vertexProps -> pos_));

	float minNormalTex[4]	 = { m_vertexProps -> normal_.min_.X(), m_vertexProps -> normal_.min_.Y(), m_vertexProps -> texcoords_.min_[0], m_vertexProps -> texcoords_.min_[1] };
	float factorNormalTex[4] = { ComputeCompressionFactor( m_vertexProps -> normal_.min_[0], m_vertexProps -> normal_.max_[0] ),
								 ComputeCompressionFactor( m_vertexProps -> normal_.min_[1], m_vertexProps -> normal_.max_[1] ),
								 ComputeCompressionFactor( m_vertexProps -> texcoords_.min_[0], m_vertexProps -> texcoords_.max_[0] ),
								 ComputeCompressionFactor( m_vertexProps -> texcoords_.min_[1], m_vertexProps -> texcoords_.max_[1] ),
								};

	VS -> SetVSConstant(g_minNormalTex, minNormalTex, 1);
	VS -> SetVSConstant(g_factorNormalTex, factorNormalTex, 1);

	float minParams[4]		= { 0, m_vertexProps -> bladeSize_.min_, m_vertexProps -> windWeight_.min_, m_vertexProps -> noiseFactor_.min_ };
	float factorParams[4]	= { 0, ComputeCompressionFactor(m_vertexProps -> bladeSize_), ComputeCompressionFactor(m_vertexProps -> windWeight_),
								ComputeCompressionFactor(m_vertexProps -> noiseFactor_) };

	VS -> SetVSConstant(g_minParams, minParams, 1);
	VS -> SetVSConstant(g_factorParams, factorParams, 1);

}

template <class VertexShader>
void CSpeedGrassRenderable::SetCommonConstants(const VertexShader& VS)   const
{
	DECLARE_VS_CONSTANT(g_mModelViewProj)
	DECLARE_VS_CONSTANT(g_mBBUnitSquare)
	DECLARE_VS_CONSTANT(g_viewPoint)
	DECLARE_VS_CONSTANT(g_windDirection)
	DECLARE_VS_CONSTANT(g_params)
	DECLARE_VS_CONSTANT(g_cameraProps)

	VS -> SetVSConstant(g_mModelViewProj, RenderingContext::Get()->m_worldToScreen, 4 );

	CPoint viewPoint = RenderingContext::Get()->GetEyePos();
	VS -> SetVSConstant(g_viewPoint, viewPoint);

	float windDirection[4] = {1.f, 0, 0, 0};
	VS -> SetVSConstant(g_windDirection, windDirection, 1);

	float cameraProps[4] = {0};

	CSpeedTreeRT::GetCameraAngles(cameraProps[0], cameraProps[1]);
	cameraProps[0] = DegToRad(cameraProps[0]);
	cameraProps[1] = DegToRad(cameraProps[1]);
	VS -> SetVSConstant(g_cameraProps, cameraProps, 1);


	double time = CTimer::Get().GetGameTime();
	float  lodDistance = m_dataInterface -> GetLODCutoff();
	float  transitionLength = m_dataInterface -> GetViewTransactionLength();
	float  windPeriod		= m_dataInterface -> GetWindPeriod();
	float  windSpeed		= m_dataInterface -> GetWindSpeed();

	time *= windSpeed;

	//float params[4] = { 300.f, 50.f, (float)time, 5.f};
	float params[4] = { lodDistance, transitionLength, (float)time, windPeriod};
	VS -> SetVSConstant(g_params, params, 1);

	static const CMatrix unitBillboard(
		0, 0.5f, 1.0f, 0,
		0, -0.5f, 1.f, 0,
		0, -.5f, 0, 0,
		0, 0.5f, 0, 0);

	VS -> SetVSConstant(g_mBBUnitSquare, unitBillboard);

	Renderer::Get().SetTexture(0, m_texture, true);

	Renderer::Get().SetCullMode( GFX_CULLMODE_NORMAL );
	Renderer::Get().SetSamplerAddressMode( 0, TEXTUREADDRESS_CLAMPALL );

	SetCompressionConstants(VS);

}

void CSpeedGrassRenderable::SendGeometry(Shader* VS, Shader* PS) const
{
	ntAssert(m_numVerts > 0);

	Renderer::Get().SetVertexShader(VS, true);
	Renderer::Get().SetPixelShader(PS, true);

	Renderer::Get().m_Platform.SetStream( m_vertexBuffer ); 
	Renderer::Get().m_Platform.DrawPrimitives( Gc::kQuads, 0, m_numVerts );
}

void CSpeedGrassRenderable::PostRender() const 
{
	Renderer::Get().m_Platform.ClearStreams();
	Renderer::Get().SetCullMode( GFX_CULLMODE_NORMAL);
}

void CSpeedGrassRenderable::RenderMaterial()
{
	static CSpeedGrassShader<SPEEDTREE_PASS_MATERIAL, SHADERTYPE_VERTEX>	VS;
	static CSpeedGrassShader<SPEEDTREE_PASS_MATERIAL, SHADERTYPE_PIXEL>		PS;

	SetCommonConstants(VS);
	SetLightingConstants(VS.GetShader(), PS.GetShader());

	SendGeometry(VS.GetShader(), PS.GetShader());

	PostRender();

}

void CSpeedGrassRenderable::RenderDepth()
{
	static CSpeedGrassShader<SPEEDTREE_PASS_DEPTH, SHADERTYPE_VERTEX>	VS;
	static CSpeedGrassShader<SPEEDTREE_PASS_DEPTH, SHADERTYPE_PIXEL>	PS;

	SetCommonConstants(VS);

	SendGeometry(VS.GetShader(), PS.GetShader());

	PostRender();
}

void CSpeedGrassRenderable::SetLimits(const CLimits<CPoint>& pos, const CLimits<CDirection>& normal, const CLimits<Vec2>& texcoords, const CLimits<float>& bladeSize, const CLimits<float>& windWeight, const CLimits<float>& noiseFactor)
{
	m_vertexProps -> pos_		= pos;
	m_vertexProps -> normal_	= normal;
	m_vertexProps -> texcoords_	= texcoords;
	m_vertexProps -> bladeSize_	= bladeSize;
	m_vertexProps -> windWeight_ = windWeight;
	m_vertexProps -> noiseFactor_ = noiseFactor;
}
