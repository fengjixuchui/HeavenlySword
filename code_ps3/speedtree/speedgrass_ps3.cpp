#include <limits>

#include "core/boundingvolumes.h"
#include "objectdatabase/dataobject.h"
#include "area/arearesourcedb.h"
#include "game/entity.h"
#include "game/entity.inl"
#include "gfx/sector.h"
#include "gfx/texturemanager.h"
#include "camera/camman.h"
#include "camera/camview.h"

#include "speedtreeutil_ps3.h"
#include "speedtreerandom_ps3.h"
#include "speedgrass_renderable.h"
#include "speedgrassdatainterface_ps3.h"
#include "tempfileread.h"
#include "uniqueptrcontainer.h"
#include "speedgrass_limits_ps3.h"

#ifdef _SPEEDTREE
#include "game/shelldebug.h"
#endif

#include "core/profiling.h"

//#define SPEEDGRASS_LOADTIME_PROFILER

#ifdef	SPEEDGRASS_LOADTIME_PROFILER
#define SG_START_PROFILER( X ) START_LOAD_TIME_PROFILER( X )
#define SG_STOP_PROFILER( X ) STOP_LOAD_TIME_PROFILER( X )
#else
#define SG_START_PROFILER( X ) 
#define SG_STOP_PROFILER( X ) 
#endif

class CSpeedGrassManager;

typedef FwStd::IntrusivePtr<CSpeedGrassManager>	SpeedGrassManagerPtr;

// Disk structures
struct __attribute__ ((__packed__)) CDataFileHeader
{
	char m_version[4];
};																 

struct __attribute__ ((__packed__)) CDataEntry
{
	float		m_position[3];
	float		m_normal[3];
	uint8_t		m_color[3];
	float		m_height;

};



class SpeedGrassInstance
{
	//HAS_INTERFACE(SpeedGrassInstance);

public:
	enum CONSTRUCT_NULL_OBJECT
	{
		CONSTRUCT_NULL
	};


	SpeedGrassInstance() {}

	SpeedGrassInstance(CONSTRUCT_NULL_OBJECT)
		: m_position(std::numeric_limits<float>::max(), std::numeric_limits<float>::max(), std::numeric_limits<float>::max())
	{
	}

	SpeedGrassInstance(const CPoint& pos, const CDirection& normal, const uint8_t	(&color)[3])
		: m_position(pos)
		, m_normal(normal)
	{
		m_color[0] = color[0];
		m_color[1] = color[1];
		m_color[2] = color[2];
		m_color[3] = 255;
	}

	friend CAABB AddPoint(CAABB aabb, SpeedGrassInstance* inst)
	{
		aabb.Union(inst -> m_position);
		return aabb;
	}

	int GetX(float minX, float sideX) const
	{
		return (int)((m_position.X() - minX) / sideX);
	}
	int GetZ(float minZ, float sideZ) const
	{
		return (int)((m_position.Z() - minZ) / sideZ);
	}



public:
	CPoint		m_position;
	CDirection	m_normal;
	uint8_t		m_color[4];

	static SpeedGrassInstance	nullObject_;

};

class SpeedGrassField  : public ISpeedGrassData
{
	HAS_INTERFACE(SpeedGrassField);

	typedef ntstd::Vector<SpeedGrassInstance*>		InstanceList;
	typedef ntstd::Vector<CSpeedGrassRenderable*>	Renderables;

	struct CCellInfo
	{
		CCellInfo(InstanceList::iterator start, float minY, float maxY)
			:	startIter_(start)
			,	minY_(minY)
			,	maxY_(maxY)
		{
		}

		InstanceList::iterator			startIter_;
		float							minY_;
		float							maxY_;
	};

public:
	~SpeedGrassField();

	void PostConstruct();
	void UpdateLOD();
	void OnAreaLoad(void* fileData, size_t fileSize);
	bool OnAreaUnload();

	void GetVRAMFootprint(CUniquePtrContainer& texCont, unsigned int& vertexSize, unsigned int& indexSize, unsigned int& textureSize);

	CHashedString GetHash()
	{
		return m_grassClumpName;
	}

public:
	// ISpeedGrassData
	float	GetLODCutoff()
	{
		return	m_cutoffDistance;
	}
	float	GetViewTransactionLength()
	{
		return m_fadeTransactionLength;
	}

	float	GetWindPeriod()
	{
		return m_windPeriod;
	}

	float GetWindSpeed()
	{
		return m_windSpeed;
	}

private:
	void GetGrassClumpName(ntstd::String& name);
	int LoadData(const CDataEntry* tempData, int numElems);
	void CreateGeometry(int numBlades, Texture::Ptr texture);
	Texture::Ptr LoadTexture();
	void DestroyGeometry();
	void Setup(const CDataEntry* data, int numElems, const char* grassFieldName);
	void BuildCell(const InstanceList::iterator& start, const InstanceList::iterator& end, CSpeedGrassRenderable*	newRenderable);

public:

	int		m_hqBladesInBunch;

	 // wind parameters
	 float	m_minBladeNoise;
	 float	m_maxBladeNoise;
	 float	m_minBladeThrow;
	 float	m_maxBladeThrow;

	 float	m_minBladeSize;
	 float	m_maxBladeSize;

	 float	m_cutoffDistance;
	 float  m_fadeTransactionLength;
	 float	m_windPeriod;
	 float  m_windSpeed;

	 ntstd::String	m_textureName;
	 int	m_numImages;
	 CEntity*	m_parentObj;

	 InstanceList	m_list;

private:
	Renderables				m_renderables;
	SpeedGrassInstance*		m_grassData;
	SpeedGrassManagerPtr	m_manager;
	CHashedString			m_grassClumpName;
};

class CSpeedGrassManager : public Singleton<CSpeedGrassManager, SPEEDGRASS_MEMORY_CHUNK>
{
	typedef ntstd::Vector<SpeedGrassField*>	GrassFields;
public:
	CSpeedGrassManager();
	~CSpeedGrassManager();

	void Update(float elapsedTime);
	void OnAreaLoad(CHashedString clumpName, void* fileData, size_t fileSize);
	bool OnAreaUnload(CHashedString clumpName);

	void Register(SpeedGrassField* grassField);
	void Unregister(SpeedGrassField* grassField);

	void GetVRAMFootprint(unsigned int& vertexSize, unsigned int& indexSize, unsigned int& textureSize);

private:
	friend void IntrusivePtrAddRef(CSpeedGrassManager* obj)
	{
		ntAssert(obj -> m_refCount < 200000);
		++ obj -> m_refCount;
	}

	friend void IntrusivePtrRelease(CSpeedGrassManager* obj)
	{
		ntAssert(obj -> m_refCount != 0);
		if ( -- obj -> m_refCount == 0)
		{
			CSpeedGrassManager::Kill();
		}
	}

	inline SpeedGrassField*	FindField(CHashedString clumpName);


private:
	unsigned int		m_refCount;
	GrassFields			m_fields;

};


class ListSortFunctor : public std::binary_function<SpeedGrassInstance const*, SpeedGrassInstance const*, bool> 
{
public:
	ListSortFunctor(const CAABB& aabb, const float (&sides)[2], int cellsX) 
		: m_aabb(aabb)
		, m_sides(sides)
		, m_cellsX(cellsX)
	{
	}

	void CalculateCellIndex(SpeedGrassInstance const* lhs, SpeedGrassInstance const* rhs, int& cellIndexL, int& cellIndexR) const
	{
		int zL = lhs -> GetZ(m_aabb.Min().Z(), m_sides[1]);
		int zR = rhs -> GetZ(m_aabb.Min().Z(), m_sides[1]);

		int xL = lhs -> GetX(m_aabb.Min().X(), m_sides[0]);
		int xR = rhs -> GetX(m_aabb.Min().X(), m_sides[0]);

		cellIndexL = zL * m_cellsX + xL;
		cellIndexR = zR * m_cellsX + xR;

		ntAssert(cellIndexL < m_cellsX * m_cellsX);
		ntAssert(cellIndexR < m_cellsX * m_cellsX);
	}

	bool operator() (SpeedGrassInstance const* lhs, SpeedGrassInstance const* rhs)   const
	{
		int cellIndexL;
		int cellIndexR;

		CalculateCellIndex(lhs, rhs, cellIndexL, cellIndexR);
		
		return cellIndexL < cellIndexR;

	}

	bool IsEqual(SpeedGrassInstance const* lhs, SpeedGrassInstance const* rhs) const
	{
		int cellIndexL;
		int cellIndexR;

		if (lhs == rhs)
		{
			return true;
		}

		if (lhs == &SpeedGrassInstance::nullObject_ || rhs == &SpeedGrassInstance::nullObject_ )
		{
			return false;
		}

		CalculateCellIndex(lhs, rhs, cellIndexL, cellIndexR);

		return cellIndexL == cellIndexR;

	}

private:
	const CAABB		&m_aabb;
	const float		(&m_sides)[2];
	const int		m_cellsX;
};

// XML definitions ////////////////////////////////////////////////////

START_STD_INTERFACE(SpeedGrassField)
	PUBLISH_VAR_WITH_DEFAULT_AS(m_minBladeNoise, 0.0f, MinBladeNoise)
	PUBLISH_VAR_WITH_DEFAULT_AS(m_maxBladeNoise, 0.0f, MaxBladeNoise)
	PUBLISH_VAR_WITH_DEFAULT_AS(m_minBladeThrow, 0.0f, MinBladeThrow)
	PUBLISH_VAR_WITH_DEFAULT_AS(m_maxBladeThrow, 0.0f, MaxBladeThrow)
	PUBLISH_VAR_WITH_DEFAULT_AS(m_minBladeSize, 0.0f, MinBladeSize)
	PUBLISH_VAR_WITH_DEFAULT_AS(m_maxBladeSize, 0.0f, MaxBladeSize)
	PUBLISH_VAR_WITH_DEFAULT_AS(m_cutoffDistance, 0.0f, ViewDistance)
	PUBLISH_VAR_WITH_DEFAULT_AS(m_fadeTransactionLength, 0.0f, FadeTransactionLength)
	PUBLISH_VAR_WITH_DEFAULT_AS(m_windPeriod, 0.0f, WindPeriod)
	PUBLISH_VAR_WITH_DEFAULT_AS(m_windSpeed, 1.0f, WindSpeed)
	PUBLISH_VAR_AS(m_textureName, TextureName)
	PUBLISH_VAR_WITH_DEFAULT_AS(m_numImages, 0, NumImages)
	PUBLISH_VAR_WITH_DEFAULT_AS(m_hqBladesInBunch, 1, NumBladesInBunch)

	//PUBLISH_PTR_CONTAINER_AS(m_list, List)
	PUBLISH_PTR_AS(m_parentObj, StaticObject)
	DECLARE_POSTCONSTRUCT_CALLBACK(PostConstruct)
END_STD_INTERFACE

// Statics ////////////////////////////////////////////

SpeedGrassInstance	SpeedGrassInstance::nullObject_(SpeedGrassInstance::CONSTRUCT_NULL);  

// Free functions ////////////////////////////////////
bool IsEnabled()
{
#ifdef _SPEEDTREE
	return g_ShellOptions->m_bUseSpeedTree;
#else
	return false;
#endif
}


void ForceLinkFunctionSpeedGrass()
{
	ntPrintf("!ATTN! Calling ForceLinkFunctionSpeedGrass() !ATTN!\n");
}

namespace SpeedGrass
{
	void Update(float elapsedTime)
	{
		if (CSpeedGrassManager::Exists())
		{
			CSpeedGrassManager::Get().Update(elapsedTime);
		}
	}

	void OnAreaLoad(CHashedString clumpName, void* fileData, size_t fileSize)
	{
		if (CSpeedGrassManager::Exists())
		{
			CSpeedGrassManager::Get().OnAreaLoad(clumpName, fileData, fileSize);
		}

	}

	bool OnAreaUnload(CHashedString clumpName)
	{
		if (CSpeedGrassManager::Exists())
		{
			return CSpeedGrassManager::Get().OnAreaUnload(clumpName);
		}

		return false;
	}

	unsigned int GetVRAMFootprint(unsigned int& vertexSize, unsigned int& indexSize, unsigned int& textureSize)
	{
		indexSize = 0;
		vertexSize = 0;
		textureSize = 0;

		if (CSpeedGrassManager::Exists())
		{
			CSpeedGrassManager::Get().GetVRAMFootprint(vertexSize, indexSize, textureSize);
		}

		return indexSize + vertexSize + textureSize;
	}
}

// CSpeedGrassManager /////////////////////////////////////////////////////

CSpeedGrassManager::CSpeedGrassManager()
	: m_refCount(1)
{
}

CSpeedGrassManager::~CSpeedGrassManager()
{
	ntAssert(m_fields.empty());
}

void CSpeedGrassManager::Register(SpeedGrassField* field)
{
	m_fields.push_back(field);
}

void CSpeedGrassManager::Unregister(SpeedGrassField* field)
{
	GrassFields::iterator pos = ntstd::find(m_fields.begin(), m_fields.end(), field);
	if (pos != m_fields.end())
	{
		m_fields.erase(pos);
	}
	else
	{
		ntAssert(!"A grass field has already been unregistered");
	}
}

void CSpeedGrassManager::Update(float)
{
	// this will call UpdateLOD on all objects in the vector 
	ntstd::for_each(m_fields.begin(), m_fields.end(), ntstd::mem_fun(&SpeedGrassField::UpdateLOD));
}

inline SpeedGrassField*	CSpeedGrassManager::FindField(CHashedString clumpName)
{
	// linear search for now
	for (GrassFields::iterator iter = m_fields.begin(); iter != m_fields.end(); ++ iter)
	{
		if ((*iter) -> GetHash() == clumpName )
		{
			return *iter;
		}
	}

	ntError_p(0, ("SPEEDGRASS : Speedgrass area resource not found\n"));
	return NULL;
}

void CSpeedGrassManager::OnAreaLoad(CHashedString clumpName, void* fileData, size_t fileSize)
{
	SpeedGrassField*	fieldToLoad = FindField(clumpName);
	if (fieldToLoad)
	{
		fieldToLoad -> OnAreaLoad(fileData, fileSize);
	}
}

bool CSpeedGrassManager::OnAreaUnload(CHashedString clumpName)
{
	SpeedGrassField*	fieldToLoad = FindField(clumpName);
	if (fieldToLoad)
	{
		return fieldToLoad -> OnAreaUnload();
	}

	return false;
}


void CSpeedGrassManager::GetVRAMFootprint(unsigned int& vertexSize, unsigned int& indexSize, unsigned int& textureSize)
{
	CUniquePtrContainer	uniqueTextures;

	for (GrassFields::iterator iter = m_fields.begin(); iter != m_fields.end(); ++ iter)
	{
		(*iter) -> GetVRAMFootprint(uniqueTextures, vertexSize, indexSize, textureSize);
	}

}

// CSpeedGrassField /////////////////////////////////////////////////////

SpeedGrassField::~SpeedGrassField()
{
	if (IsEnabled())
	{
		DestroyGeometry();
		m_manager -> Unregister(this);
	}
}

void SpeedGrassField::GetGrassClumpName(ntstd::String& name)
{
	static const char* grassFileExtension = "grass";

	if (!m_parentObj)
	{
		return;
	}

	ntstd::String clumpName = m_parentObj -> GetClumpString();
	unsigned int extPos = clumpName.rfind('.');

	if (extPos == ntstd::String::npos)
	{
		return;
	}

	clumpName.erase(extPos + 1, clumpName.length() - extPos - 1);
	clumpName += grassFileExtension;

	Util::SetToPlatformResources();

	char fullPath[1024];
	Util::GetFiosFilePath(ntStr::GetString(clumpName), fullPath);

	Util::SetToNeutralResources();

	name = fullPath;

}

int SpeedGrassField::LoadData(const CDataEntry* tempData, int numElems)
{
	if (!m_grassData)
	{
		ntAssert(0);
		return 0;
	}

	CPoint orig = m_parentObj -> GetPosition();

	for (int i = 0; i < numElems; ++ i)
	{
		const CDataEntry&	grassClumpFileData = tempData[i];

		CPoint		pos(grassClumpFileData.m_position[0], grassClumpFileData.m_position[1], grassClumpFileData.m_position[2]);
		CDirection	normal(grassClumpFileData.m_normal[0], grassClumpFileData.m_normal[1], grassClumpFileData.m_normal[2]);
		pos += orig;

		const uint8_t	(&color)[3] = grassClumpFileData.m_color;
																														   
		NT_PLACEMENT_NEW(&m_grassData[i]) SpeedGrassInstance(pos, normal, color);

		m_list.push_back(&m_grassData[i]);
	}

	return numElems;

}

void SpeedGrassField::BuildCell(const InstanceList::iterator& start, const InstanceList::iterator& end, CSpeedGrassRenderable*	newRenderable)
{
//	const float bladeRotationDelta = PI / m_hqBladesInBunch; 

	for (InstanceList::iterator iter = start; iter != end; ++ iter)
	{
		ntAssert((unsigned int)(iter._Myptr) <= (unsigned int)&m_list.back());

		SpeedGrassInstance*	blade = *iter;
		float	bladeThrow = GetRandom(m_minBladeThrow, m_maxBladeThrow);
		float	bladeNoise = GetRandom(m_minBladeNoise, m_maxBladeNoise);
		float	bladeSize  = GetRandom(m_minBladeSize, m_maxBladeSize);

		int imageIndex = GetRandom(0, m_numImages - 1);

		const float c_fClamp = 2.0f / 256.0f;
		float s1 = float(imageIndex) / m_numImages + c_fClamp;
		float s2 = float(imageIndex + 1) / m_numImages - c_fClamp;

		uint8_t bladeColor[4];
		*(uint32_t*)bladeColor = *(uint32_t*)blade -> m_color;

		for (int bladeIndex = 0; bladeIndex < m_hqBladesInBunch; ++ bladeIndex)
		{
			//float bladeAngle = bladeRotationDelta * bladeIndex;
		for (unsigned int corner = 0; corner < 4; ++ corner)
		{
			float	texCoords[2];
			float windWeight = 0;

			switch (corner)
			{
			case 0:
				texCoords[0] = s2;
				texCoords[1] = 0.0f + c_fClamp;
				windWeight = bladeThrow;
				break;
			case 1:
				texCoords[0] = s1;
				texCoords[1] = 0.0f + c_fClamp;
				windWeight = bladeThrow;
				break;
			case 2:
				texCoords[0] = s1;
				texCoords[1] = 1.0f - c_fClamp;
				bladeColor[0] /= 2;
				bladeColor[1] /= 2;
				bladeColor[2] /= 2;
				break;
			case 3:
				texCoords[0] = s2;
				texCoords[1] = 1.0f - c_fClamp;
				bladeColor[0] /= 2;
				bladeColor[1] /= 2;
				bladeColor[2] /= 2;
				break;
			}

			CSpeedGrassVertex	vertex(blade -> m_position, blade -> m_normal, texCoords, (float)corner, bladeSize, windWeight, bladeNoise, bladeColor);
			newRenderable -> AddBlade(vertex);
		}
	}
	}

}

void SpeedGrassField::CreateGeometry(int numBlades, Texture::Ptr texture)
{
	const unsigned int c_maxBladesPerCell = 64 * 8;

#ifndef SPEEDGRASS_HQ
	m_hqBladesInBunch = 1;
#endif
	if (m_hqBladesInBunch < 1) m_hqBladesInBunch = 1;

SG_START_PROFILER( SG_GET_BOUNDS )
	CAABB	fieldBoundaries(CONSTRUCT_INFINITE_NEGATIVE);
	fieldBoundaries = ntstd::accumulate(m_list.begin(), m_list.end(), fieldBoundaries, &AddPoint);

	const float boundAdjust = 0.5f;
	fieldBoundaries.Min().X() -= boundAdjust;
	fieldBoundaries.Min().Z() -= boundAdjust;
	fieldBoundaries.Max().X() += boundAdjust;
	fieldBoundaries.Max().Z() += boundAdjust;

	float size[2] = { fieldBoundaries.Max().X() - fieldBoundaries.Min().X(), fieldBoundaries.Max().Z() - fieldBoundaries.Min().Z() };
	unsigned int numCellsXZ[2];
	unsigned int numCells = ComputeCellDimensions( size, numBlades, c_maxBladesPerCell, numCellsXZ ); 

	m_renderables.reserve(numCells);

	float cellSide[2] = { size[0] / (float)numCellsXZ[0], size[1] / (float)numCellsXZ[1] };

SG_STOP_PROFILER( SG_GET_BOUNDS )

SG_START_PROFILER( SG_SORT )
	ListSortFunctor sortFunctor(fieldBoundaries, cellSide, numCellsXZ[0]);

	ntstd::sort(m_list.begin(), m_list.end(), sortFunctor);
SG_STOP_PROFILER( SG_SORT )

	typedef ntstd::Vector<CCellInfo>	CellInfoVector;
	CellInfoVector cellInfos;
	cellInfos.reserve(numCells + 1);

	// end marker (just to avoid additional check inside the loop)
	m_list.push_back(&SpeedGrassInstance::nullObject_);

	// go through all blades to find the boundaries for each cell in the blade list as well as Y dimensions of each cell
	for (InstanceList::iterator	startIter = m_list.begin(); startIter != m_list.end() - 1; )
	{
		float	minY = fieldBoundaries.Max().Y();
		float	maxY = fieldBoundaries.Min().Y();

		InstanceList::iterator	iter = startIter; 
		do
		{
			SpeedGrassInstance*	blade = *iter;
			if (blade -> m_position.Y() < minY)
			{
				minY = blade -> m_position.Y();
			}

			if (blade -> m_position.Y() > maxY)
			{
				maxY = blade -> m_position.Y();
			}
			
		} while( sortFunctor.IsEqual(*(++iter), *startIter ) ); 

		cellInfos.push_back(CCellInfo(startIter, minY, maxY));
		startIter = iter;
	}
	cellInfos.push_back(CCellInfo(m_list.end() - 1, 0, 0));

	float bladeSizeX = m_maxBladeSize * 0.5f;
	float bladeSizeZ = m_maxBladeSize * 0.5f;

	srand(328);

	// set up the cells now
	for (CellInfoVector::iterator cellIter = cellInfos.begin(); cellIter != cellInfos.end() - 1; ++ cellIter)
	{
		InstanceList::iterator start	= cellIter -> startIter_;
		InstanceList::iterator end		= (cellIter + 1) -> startIter_;

		size_t bladesInCell = std::distance(start, end);

		if (bladesInCell > 0)
		{
			CSpeedGrassRenderable*	newRenderable = NT_NEW_CHUNK(SPEEDGRASS_MEMORY_CHUNK) CSpeedGrassRenderable(texture, bladesInCell, this);
			m_renderables.push_back(newRenderable);

			CSector::Get().GetRenderables().AddRenderable(newRenderable);

			int thisCellX = (*start) -> GetX(fieldBoundaries.Min().X(), cellSide[0]);
			int thisCellZ = (*start) -> GetZ(fieldBoundaries.Min().Z(), cellSide[1]);

			CPoint minPos(fieldBoundaries.Min().X() + ((float)thisCellX) * cellSide[0] - bladeSizeX, cellIter -> minY_, fieldBoundaries.Min().Z() + ((float)thisCellZ) * cellSide[1] - bladeSizeZ);
			CPoint deltaPos(cellSide[0] + 2.f * bladeSizeX, cellIter -> maxY_ - cellIter -> minY_ + m_maxBladeSize, cellSide[1] + 2.f * bladeSizeZ);


			newRenderable -> SetPosition( 
					minPos.X(),
					minPos.Y(),
					minPos.Z(),
					deltaPos.X(),
					deltaPos.Y(),
					deltaPos.Z() );

			newRenderable -> SetLimits( Limits(minPos, minPos + deltaPos), Limits(CDirection(-1.f, -1.f, -1.f), CDirection(1.f, 1.f, 1.f)),
				Limits(Vec2(0.f,0.f), Vec2(1.f, 1.f)), Limits(m_minBladeSize, m_maxBladeSize), Limits(0.f, m_maxBladeThrow), Limits(m_minBladeNoise, m_maxBladeNoise) );

			BuildCell(start, end, newRenderable);

		}

	}

}

void SpeedGrassField::DestroyGeometry()
{
	for (Renderables::iterator iter = m_renderables.begin(); iter != m_renderables.end(); ++ iter)
	{
		CSector::Get().GetRenderables().RemoveRenderable(*iter);
		NT_DELETE_CHUNK(SPEEDGRASS_MEMORY_CHUNK, *iter);
	}
	m_renderables.clear();
}

Texture::Ptr SpeedGrassField::LoadTexture()
{

	Texture::Ptr texture = TextureManager::Get().LoadTexture_Neutral( ntStr::GetString( Util::BaseName( ntStr::GetString(m_textureName) ) ) );
	texture -> m_Platform.GetTexture() -> SetGammaCorrect( Gc::kGammaCorrectSrgb );

	return texture;
}


void SpeedGrassField::Setup(const CDataEntry* data, int numElems, const char* grassFieldName)
{
	m_list.reserve(numElems);
	m_grassData = NT_NEW_ARRAY_CHUNK(SPEEDGRASS_MEMORY_CHUNK) SpeedGrassInstance[numElems];	

SG_START_PROFILER( SG_LOAD_DATA )
	int numBlades = LoadData(data, numElems);
SG_STOP_PROFILER( SG_LOAD_DATA )

	if (0 != numBlades)
	{
		// Create vertex buffers
SG_START_PROFILER( SG_FILL_BUFFERS )
		CreateGeometry(numBlades, LoadTexture());
SG_STOP_PROFILER( SG_FILL_BUFFERS )
	}
	else
	{
		user_warn_msg(("SPEEDGRASS : %s failed to load data, ignored...\n", grassFieldName));
	}

	NT_DELETE_ARRAY_CHUNK(SPEEDGRASS_MEMORY_CHUNK, m_grassData);
	m_list.clear();
	m_grassData = NULL;

	UNUSED(grassFieldName);
}

void SpeedGrassField::PostConstruct()
{
	SG_START_PROFILER( SG )

	if (!IsEnabled())
	{
		return;
	}

	const char* grassFieldName = ntStr::GetString(ObjectDatabase::Get().GetNameFromPointer(this));

	ntstd::String grassClumpName;

	GetGrassClumpName(grassClumpName);

	if (ntStr::IsNull(grassClumpName))
	{
		user_warn_msg(("SPEEDGRASS : grass field %s is not linked to a valid object\n", grassFieldName));
		return;
	}


	if (ntStr::IsNull(m_textureName))
	{
		user_warn_msg(("SPEEDGRASS : %s  has no texture, ignored...\n", grassFieldName));
		return;
	}

	if (m_numImages < 1)
	{
		user_warn_msg(("SPEEDGRASS : %s should have at least 1 texture image, ignored...\n", grassFieldName));
		return;
	}

	m_grassClumpName = CHashedString(grassClumpName);

	if (!CSpeedGrassManager::Exists())
	{
		m_manager = SpeedGrassManagerPtr(NT_NEW_CHUNK(SPEEDGRASS_MEMORY_CHUNK) CSpeedGrassManager);
	}
	else
	{
		m_manager = SpeedGrassManagerPtr(CSpeedGrassManager::GetP());
		IntrusivePtrAddRef(CSpeedGrassManager::GetP());
	}
	m_manager -> Register(this);

	uint32_t sectorBits = m_parentObj -> GetMappedAreaInfo();

	if ((int)sectorBits <= 0)
	{
		// Load globally
		CScopedFileRead<CDataEntry, SPEEDGRASS_MEMORY_CHUNK, CDataFileHeader> grassClump(grassClumpName);

		if (!grassClump.IsValid())
		{
			user_warn_msg(("SPEEDGRASS : %s failed to load grass clump %s, ignored...\n", grassFieldName, ntStr::GetString(grassClumpName)));
			return;
		}

		// Load geometry data
		int numElements = (int)grassClump.GetNumElements();

		Setup(grassClump.GetData(), numElements, grassFieldName);

	}
	else
	{
		// register with the area system

		// register texture
		const char* fileName = ntStr::GetString(m_textureName);
		if (!ntStr::IsNull(fileName))
		{
			const char* name = ntStr::GetString( Util::BaseName(fileName) );
			AreaResourceDB::Get().AddAreaResource( name, AreaResource::TEXTURE, sectorBits );
		}

		// register grass clump
		AreaResourceDB::Get().AddAreaResource( ntStr::GetString(grassClumpName), AreaResource::SPEEDGRASS, sectorBits );

	}

	UNUSED(grassFieldName);

	SG_STOP_PROFILER( SG )
}

void SpeedGrassField::OnAreaLoad(void* fileData, size_t fileSize)
{
	const char* grassFieldName = ntStr::GetString(ObjectDatabase::Get().GetNameFromPointer(this));
	ntError_p(sizeof(CDataFileHeader) < fileSize, ("SPEEDGRASS : %s grass clump file loaded by the area system too short\n", grassFieldName));

	const CDataEntry*	grassData = (const CDataEntry*)(((char*)fileData) + sizeof(CDataFileHeader));
	int dataSize = fileSize - sizeof(CDataFileHeader);
	ntError_p((dataSize % sizeof(CDataEntry) == 0), ("SPEEDGRASS : %s wrong data in the clump file loaded by the area system\n", grassFieldName));
	int numElements = dataSize / sizeof(CDataEntry);

	Setup(grassData, numElements, grassFieldName);

	UNUSED(grassFieldName);
}

bool SpeedGrassField::OnAreaUnload()
{
	DestroyGeometry();

	return true;
}

void SpeedGrassField::UpdateLOD()
{
	// this will call UpdateLOD on all objects in the vector passing cutoff distance as a parameter
	//ntstd::for_each(m_renderables.begin(), m_renderables.end(), std::bind2nd(ntstd::mem_fun(&CSpeedGrassRenderable::UpdateLOD), m_cutoffDistance));

	const CCamera* camera = CamMan::GetPrimaryView();

	if (!camera)
	{
		return;
	}

	const CPoint& viewPoint = camera -> GetEyePos();
	float farClip = m_cutoffDistance + m_fadeTransactionLength;

	for (Renderables::iterator iter = m_renderables.begin(); iter != m_renderables.end(); ++ iter)
	{
		CAABB	aabb = (*iter) -> GetWorldSpaceAABB();

		CPoint centre = aabb.GetCentre();
		CDirection radiusVector = aabb.GetHalfLengths();

		CDirection cameraVector = centre ^ viewPoint;

		float squaredLen = cameraVector.LengthSquared();
		float squaredRadius = radiusVector.LengthSquared();
		float squaredClipDistance = farClip * farClip;
		if (squaredLen < squaredClipDistance)
		{
			// Visible
			if (!(*iter) -> IsRendering())
			{
				(*iter) -> DisableRendering(false);
			}

		}
		else
		{
			float len = fsqrtf(squaredLen);
			float radius = fsqrtf(squaredRadius);

			if (len - radius > farClip)
			{
				// not Visible
				if ((*iter) -> IsRendering())
				{
					(*iter) -> DisableRendering(true);
				}
			}
			else
			{
				// Partly visible
				if (!(*iter) -> IsRendering())
				{
					(*iter) -> DisableRendering(false);
				}
			}
		}

	}
}

void SpeedGrassField::GetVRAMFootprint(CUniquePtrContainer& texCont, unsigned int& vertexSize, unsigned int& indexSize, unsigned int& textureSize)
{
	if (!m_renderables.empty())
	{
		Texture::Ptr texture = TextureManager::Get().LoadTexture_Neutral( ntStr::GetString( Util::BaseName( ntStr::GetString(m_textureName) ) ) );

		if (texture.IsValid())
		{
			if (!texCont.GetPtr(texture.Get()))
			{
				textureSize += texture -> CalculateVRAMFootprint();
				texCont.AddPtr(texture.Get());
			}
		}


		for (Renderables::iterator renderableIter = m_renderables.begin(); renderableIter != m_renderables.end(); ++ renderableIter)
		{
			vertexSize += (*renderableIter) -> GetVertexFootprint();
		}
	}
}
